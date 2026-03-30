#ifndef QMF_H
#define QMF_H

#include <stdint.h>

#define N 8
#define M 2 // Standard QMF decimation is 2
#define LEN 64

// LFSR state structure for reentrancy
typedef struct {
    uint64_t state;
    uint32_t pos;
} LFSR_Context;

// LFSR functions
void init_lfsr(LFSR_Context *ctx, uint64_t seed, uint32_t start_pos);
int shift_lfsr(LFSR_Context *ctx, char dir);
uint8_t shift_lfsr_n(LFSR_Context *ctx, char dir, int n);

// Core functions
void normalize(double *vec, int len);
void daub(const double *seq, double *h);
void daub_shift(const double *seq, double *h, double shift);
void qmf(const double *x, int len_x, const double *h, double *yl, double *yh);
void qmf_synth(const double *yl, const double *yh, int len_y, const double *h, double *xr);

#endif // QMF_H
