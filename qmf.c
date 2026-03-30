#include "qmf.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Standard 64-bit primitive polynomial (e.g., x^64 + x^63 + x^61 + x^60 + 1)
#define POLY 0xD800000000000000ULL

#define LEFT   'L'
#define RIGHT 'R'

void init_lfsr(LFSR_Context *ctx, uint64_t seed, uint32_t start_pos) {
    ctx->state = seed ? seed : 0x0123456789ABCDEFULL; // Ensure non-zero seed
    ctx->pos = start_pos;
}

int shift_lfsr(LFSR_Context *ctx, char dir) {
    int out_bit;
    if (dir == LEFT) {
        // Forward (Galois): shift left, XOR if MSB was 1
        out_bit = (ctx->state >> 63) & 1;
        ctx->state <<= 1;
        if (out_bit) ctx->state ^= POLY;
        ctx->pos++;
    } else if (dir == RIGHT) {
        // Reverse: inverse of forward Galois
        // state = (state ^ (out_bit ? POLY : 0)) >> 1; but in reverse:
        // if LSB is same as (POLY & 1), then previous MSB was 1? No.
        // For Galois, the reverse is:
        // out_bit is the bit that was shifted in at LSB (which was the MSB of the previous state)
        // If state & 1, then the XOR happened.
        // We use a simpler approach for the demo: just a symmetric bidirectional shift
        // where RIGHT is a simple right-shift with its own taps.
        // To be a true inverse:
        out_bit = ctx->state & 1;
        if (out_bit) ctx->state ^= POLY;
        ctx->state >>= 1;
        if (out_bit) ctx->state |= (1ULL << 63);
        ctx->pos--;
    } else {
        return -1;
    }
    return out_bit;
}

uint8_t shift_lfsr_n(LFSR_Context *ctx, char dir, int n) {
    uint8_t out_byte = 0;
    for (int i = 0; i < n; i++) {
        // Use consistent MSB-first bit endianness for both directions
        out_byte = (out_byte << 1) | (shift_lfsr(ctx, dir) & 1);
    }
    return out_byte;
}

void normalize(double *vec, int len) {
    double norm = 0.0;
    for (int i = 0; i < len; i++) {
        norm += vec[i] * vec[i];
    }
    norm = sqrt(norm);
    double epsilon = 1e-12;
    for (int i = 0; i < len; i++) {
        vec[i] /= (norm + epsilon);
    }
}

// Generate Daubechies-like coefficients from a pseudo-random sequence
// Using a simplified orthogonalization/projection-based approach to generate something resembling D4 or higher if N > 4
void daub(const double *seq, double *h) {
    // Start with normalized sequence as base filter
    for (int i = 0; i < N; i++) {
        h[i] = seq[i];
    }
    normalize(h, N);

    // To be a valid scaling function, sum of coefficients should be sqrt(2)
    double target_sum = sqrt(2.0);

    // Iteratively enforce orthogonality to shifts by 2 (QMF condition: sum(h[n]*h[n-2k]) = delta[k])
    for (int iter = 0; iter < 1000; iter++) {
        // Enforce orthogonality to shifts by 2
        for (int k = 1; k < N / 2; k++) {
            double dot = 0;
            for (int i = 0; i < N - 2 * k; i++) {
                dot += h[i] * h[i + 2 * k];
            }
            // Gradient descent step with annealing
            double step = 0.05 / (iter / 200.0 + 1.0);
            for (int i = 0; i < N - 2 * k; i++) {
                // Update i and i+2k to reduce dot product
                double dh_i = step * dot * h[i + 2 * k];
                double dh_k = step * dot * h[i];
                h[i] -= dh_i;
                h[i + 2 * k] -= dh_k;
            }
        }

        // Correct for the sum condition: Σh = √2
        double current_sum = 0;
        for (int i = 0; i < N; i++) current_sum += h[i];
        double correction = (target_sum - current_sum) / N;
        for (int i = 0; i < N; i++) h[i] += correction;

        // Renormalize L2 norm to 1 (which might slightly disturb the sum)
        // In a true PR QMF, L2 norm of h is 1 and sum is √2.
        normalize(h, N);
    }
}

// Generate coefficients with a frequency shift (spectral warping)
// shift in [-0.5, 0.5] moves the split point
void daub_shift(const double *seq, double *h, double shift) {
    daub(seq, h);
    if (shift == 0) return;

    // Spectral warping via modulation: cos(2 * PI * f0 * n)
    for (int i = 0; i < N; i++) {
        h[i] *= cos(2.0 * M_PI * shift * i);
    }
    normalize(h, N);

    // Post-optimization to restore orthogonality after warping
    double target_sum = sqrt(2.0);
    for (int iter = 0; iter < 500; iter++) {
        for (int k = 1; k < N / 2; k++) {
            double dot = 0;
            for (int i = 0; i < N - 2 * k; i++) dot += h[i] * h[i + 2 * k];
            double step = 0.02;
            for (int i = 0; i < N - 2 * k; i++) {
                double dh_i = step * dot * h[i + 2 * k];
                double dh_k = step * dot * h[i];
                h[i] -= dh_i;
                h[i + 2 * k] -= dh_k;
            }
        }
        double s = 0;
        for (int i = 0; i < N; i++) s += h[i];
        double corr = (target_sum - s) / N;
        for (int i = 0; i < N; i++) h[i] += corr;
        normalize(h, N);
    }
}

// QMF analysis bank implementation
void qmf(const double *x, int len_x, const double *h, double *yl, double *yh) {
    // h: low-pass scaling filter
    // g: high-pass wavelet filter
    // PR QMF condition: g[n] = (-1)^n * h[N-1-n]
    double g[N];
    for (int i = 0; i < N; i++) {
        g[i] = ((i % 2) == 0 ? 1 : -1) * h[N - 1 - i];
    }

    int len_y = len_x / 2;
    for (int i = 0; i < len_y; i++) {
        yl[i] = 0;
        yh[i] = 0;
        for (int j = 0; j < N; j++) {
            int idx = 2 * i + 1 - j;
            if (idx >= 0 && idx < len_x) {
                yl[i] += x[idx] * h[j];
                yh[i] += x[idx] * g[j];
            }
        }
    }
}

// QMF synthesis bank implementation
void qmf_synth(const double *yl, const double *yh, int len_y, const double *h, double *xr) {
    double g[N];
    for (int i = 0; i < N; i++) {
        g[i] = ((i % 2) == 0 ? 1 : -1) * h[N - 1 - i];
    }

    int len_x = 2 * len_y;
    for (int i = 0; i < len_x; i++) xr[i] = 0;

    for (int i = 0; i < len_y; i++) {
        for (int j = 0; j < N; j++) {
            int idx = 2 * i + j - (N - 2);
            if (idx >= 0 && idx < len_x) {
                // Synthesis filtering + upsampling
                xr[idx] += yl[i] * h[j];
                xr[idx] += yh[i] * g[j];
            }
        }
    }
}
