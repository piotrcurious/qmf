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
        // Reverse Galois shift
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
    double epsilon = 1e-15;
    for (int i = 0; i < len; i++) {
        vec[i] /= (norm + epsilon);
    }
}

// Refined Daubechies coefficient generation using an improved iterative algorithm
void daub(const double *seq, double *h) {
    for (int i = 0; i < N; i++) h[i] = seq[i];
    normalize(h, N);

    double target_sum = sqrt(2.0);

    // Higher iteration count and adaptive step size for better convergence
    for (int iter = 0; iter < 2000; iter++) {
        double step = 0.02 / (1.0 + iter / 500.0);

        // Enforce orthogonality: sum(h[n]*h[n-2k]) = 0 for k > 0
        for (int k = 1; k < N / 2; k++) {
            double dot = 0;
            for (int i = 0; i < N - 2 * k; i++) dot += h[i] * h[i + 2 * k];

            for (int i = 0; i < N - 2 * k; i++) {
                double hi = h[i], hik = h[i + 2 * k];
                h[i] -= step * dot * hik;
                h[i + 2 * k] -= step * dot * hi;
            }
        }

        // Enforce sum condition: sum(h[n]) = sqrt(2)
        double current_sum = 0;
        for (int i = 0; i < N; i++) current_sum += h[i];
        double correction = (target_sum - current_sum) / N;
        for (int i = 0; i < N; i++) h[i] += correction;

        // Re-normalize L2 norm to 1
        normalize(h, N);
    }
}

void daub_shift(const double *seq, double *h, double shift) {
    daub(seq, h);
    if (shift == 0) return;

    for (int i = 0; i < N; i++) h[i] *= cos(2.0 * M_PI * shift * i);
    normalize(h, N);

    // Re-optimize lightly to maintain orthogonality after shift
    double target_sum = sqrt(2.0);
    for (int iter = 0; iter < 500; iter++) {
        double step = 0.01;
        for (int k = 1; k < N / 2; k++) {
            double dot = 0;
            for (int i = 0; i < N - 2 * k; i++) dot += h[i] * h[i + 2 * k];
            for (int i = 0; i < N - 2 * k; i++) {
                double hi = h[i], hik = h[i + 2 * k];
                h[i] -= step * dot * hik;
                h[i + 2 * k] -= step * dot * hi;
            }
        }
        double s = 0;
        for (int i = 0; i < N; i++) s += h[i];
        double corr = (target_sum - s) / N;
        for (int i = 0; i < N; i++) h[i] += corr;
        normalize(h, N);
    }
}

void qmf(const double *x, int len_x, const double *h, double *yl, double *yh) {
    double g[N];
    for (int i = 0; i < N; i++) g[i] = ((i % 2) == 0 ? 1 : -1) * h[N - 1 - i];

    int len_y = len_x / 2;
    for (int i = 0; i < len_y; i++) {
        yl[i] = 0; yh[i] = 0;
        for (int j = 0; j < N; j++) {
            int idx = 2 * i + 1 - j;
            if (idx >= 0 && idx < len_x) {
                yl[i] += x[idx] * h[j];
                yh[i] += x[idx] * g[j];
            }
        }
    }
}

void qmf_synth(const double *yl, const double *yh, int len_y, const double *h, double *xr) {
    double g[N];
    for (int i = 0; i < N; i++) g[i] = ((i % 2) == 0 ? 1 : -1) * h[N - 1 - i];

    int len_x = 2 * len_y;
    for (int i = 0; i < len_x; i++) xr[i] = 0;

    for (int i = 0; i < len_y; i++) {
        for (int j = 0; j < N; j++) {
            int idx = 2 * i + j - (N - 2);
            if (idx >= 0 && idx < len_x) {
                xr[idx] += yl[i] * h[j];
                xr[idx] += yh[i] * g[j];
            }
        }
    }
}
