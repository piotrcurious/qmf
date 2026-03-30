#ifndef QMF_H
#define QMF_H

#include <stdint.h>

#define N 8
#define M 2 // Standard QMF decimation is 2
#define LEN 64

// LFSR functions (from 64_bit_LFSR.c or similar)
void init_lfsr(uint64_t seed, uint32_t start_pos);
int shift_lfsr(char dir);
uint8_t shift_lfsr_n(char dir, int n);

// Core functions
void normalize(double *vec, int len);
void daub(const double *seq, double *h);
void qmf(const double *x, int len_x, const double *h, double *yl, double *yh);

#endif // QMF_H
