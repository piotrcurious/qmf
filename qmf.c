#include "qmf.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Define tap positions for 64-bit LFSR
#define TAP1 63
#define TAP2 62
#define TAP3 60
#define TAP4 59

// Define feedback masks for each byte using ULL for 64-bit safety
#define MASK0 ((1ULL << (TAP1 % 8)) ^ (1ULL << (TAP2 % 8)) ^ (1ULL << (TAP3 % 8)) ^ (1ULL << (TAP4 % 8)))
#define MASK1 ((MASK0 << 8) | (MASK0 >> (64 - 8)))
#define MASK2 ((MASK1 << 8) | (MASK1 >> (64 - 8)))
#define MASK3 ((MASK2 << 8) | (MASK2 >> (64 - 8)))
#define MASK4 ((MASK3 << 8) | (MASK3 >> (64 - 8)))
#define MASK5 ((MASK4 << 8) | (MASK4 >> (64 - 8)))
#define MASK6 ((MASK5 << 8) | (MASK5 >> (64 - 8)))
#define MASK7 ((MASK6 << 8) | (MASK6 >> (64 - 8)))

#define LEFT   'L'
#define RIGHT 'R'

void init_lfsr(LFSR_Context *ctx, uint64_t seed, uint32_t start_pos) {
    ctx->state = seed;
    ctx->pos = start_pos;
}

int shift_lfsr(LFSR_Context *ctx, char dir) {
    int out_bit;
    if (dir == LEFT) {
        out_bit = (ctx->state >> TAP1) & 1;
        ctx->state <<= 1;
        switch (ctx->pos / 8) {
            case 0: ctx->state ^= MASK0 & (uint64_t)-(int64_t)out_bit; break;
            case 1: ctx->state ^= MASK1 & (uint64_t)-(int64_t)out_bit; break;
            case 2: ctx->state ^= MASK2 & (uint64_t)-(int64_t)out_bit; break;
            case 3: ctx->state ^= MASK3 & (uint64_t)-(int64_t)out_bit; break;
            case 4: ctx->state ^= MASK4 & (uint64_t)-(int64_t)out_bit; break;
            case 5: ctx->state ^= MASK5 & (uint64_t)-(int64_t)out_bit; break;
            case 6: ctx->state ^= MASK6 & (uint64_t)-(int64_t)out_bit; break;
            case 7: ctx->state ^= MASK7 & (uint64_t)-(int64_t)out_bit; break;
        }
        ctx->pos = (ctx->pos + 1) % 64;
    } else if (dir == RIGHT) {
        out_bit = ctx->state & 1;
        ctx->state >>= 1;
        uint64_t mask = 0;
        switch (ctx->pos / 8) {
            case 0: mask = MASK7; break;
            case 1: mask = MASK6; break;
            case 2: mask = MASK5; break;
            case 3: mask = MASK4; break;
            case 4: mask = MASK3; break;
            case 5: mask = MASK2; break;
            case 6: mask = MASK1; break;
            case 7: mask = MASK0; break;
        }
        if (out_bit) ctx->state |= mask;
        else ctx->state &= ~mask;
        ctx->pos = (ctx->pos + 63) % 64;
    } else {
        return -1;
    }
    return out_bit;
}

uint8_t shift_lfsr_n(LFSR_Context *ctx, char dir, int n) {
    uint8_t out_byte = 0;
    for (int i = 0; i < n; i++) {
        if (dir == LEFT) {
            out_byte = (out_byte << 1) | shift_lfsr(ctx, dir);
        } else {
            out_byte = (out_byte >> 1) | (shift_lfsr(ctx, dir) << 7);
        }
    }
    return out_byte;
}

void normalize(double *vec, int len) {
    double norm = 0.0;
    for (int i = 0; i < len; i++) {
        norm += vec[i] * vec[i];
    }
    norm = sqrt(norm);
    double epsilon = 1e-9;
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
    double sum = 0;
    for (int i = 0; i < N; i++) sum += h[i];
    double target_sum = sqrt(2.0);
    double correction = (target_sum - sum) / N;
    for (int i = 0; i < N; i++) h[i] += correction;
    normalize(h, N);

    // Iteratively enforce orthogonality to shifts by 2 (QMF condition: sum(h[n]*h[n-2k]) = delta[k])
    for (int iter = 0; iter < 100; iter++) {
        for (int k = 1; k < N / 2; k++) {
            double dot = 0;
            for (int i = 0; i < N - 2 * k; i++) {
                dot += h[i] * h[i + 2 * k];
            }
            // Simple gradient descent to reduce non-orthogonality
            for (int i = 0; i < N - 2 * k; i++) {
                h[i] -= 0.5 * dot * h[i + 2 * k];
                h[i + 2 * k] -= 0.5 * dot * h[i];
            }
        }
        // Re-enforce sum condition and normalization
        sum = 0;
        for (int i = 0; i < N; i++) sum += h[i];
        correction = (target_sum - sum) / N;
        for (int i = 0; i < N; i++) h[i] += correction;
        normalize(h, N);
    }
}

// Generate coefficients with a frequency shift (spectral warping)
// shift in [-0.5, 0.5] moves the split point
void daub_shift(const double *seq, double *h, double shift) {
    daub(seq, h);
    if (shift == 0) return;

    // Simple spectral warping via modulation
    for (int i = 0; i < N; i++) {
        h[i] *= cos(M_PI * shift * i);
    }
    normalize(h, N);
}

// QMF analysis bank implementation
void qmf(const double *x, int len_x, const double *h, double *yl, double *yh) {
    // h is the low-pass filter (scaling coefficients)
    // g is the high-pass filter (wavelet coefficients)
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
    for (int i = 0; i < len_x; i++) {
        xr[i] = 0;
    }

    for (int i = 0; i < len_y; i++) {
        for (int j = 0; j < N; j++) {
            int idx = 2 * i + j - (N - 2);
            if (idx >= 0 && idx < len_x) {
                xr[idx] += yl[i] * h[N - 1 - j];
                xr[idx] += yh[i] * g[N - 1 - j];
            }
        }
    }
}
