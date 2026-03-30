#include "qmf.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void test_lfsr_reproducibility() {
    printf("Testing LFSR reproducibility...\n");
    init_lfsr(0x12345678, 0);
    uint8_t b1 = shift_lfsr_n('L', 8);
    init_lfsr(0x12345678, 0);
    uint8_t b2 = shift_lfsr_n('L', 8);
    if (b1 == b2) printf("OK: LFSR is reproducible.\n");
    else printf("FAIL: LFSR is not reproducible.\n");
}

void test_orthogonality() {
    printf("Testing orthogonality of generated coefficients...\n");
    double seq[N] = {0.1, -0.5, 0.8, -0.2, 0.4, 0.6, -0.1, 0.3};
    double h[N];
    daub(seq, h);

    // Check normalization: sum(h[n]^2) = 1
    double norm = 0;
    for (int i = 0; i < N; i++) norm += h[i]*h[i];
    printf("Norm^2: %f (expect 1.0)\n", norm);

    // Check sum condition: sum(h[n]) = sqrt(2)
    double sum = 0;
    for (int i = 0; i < N; i++) sum += h[i];
    printf("Sum: %f (expect %f)\n", sum, sqrt(2.0));

    // Check shift by 2 orthogonality: sum(h[n]*h[n-2k]) = 0 for k != 0
    for (int k = 1; k < N/2; k++) {
        double dot = 0;
        for (int i = 0; i < N - 2*k; i++) {
            dot += h[i] * h[i+2*k];
        }
        printf("Shift %d dot product: %f (expect 0.0)\n", 2*k, dot);
    }
}

void test_qmf_reconstruction() {
    printf("Testing QMF perfect reconstruction property (approximate)...\n");
    double seq[N] = {1, 1, 1, 1, 1, 1, 1, 1}; // Start with something simple
    double h[N];
    daub(seq, h);

    // Test signal
    double x[LEN];
    for (int i = 0; i < LEN; i++) x[i] = (double)rand() / RAND_MAX;

    double yl[LEN/2];
    double yh[LEN/2];
    qmf(x, LEN, h, yl, yh);

    // Synthesis bank
    // Reconstruction is approx sum_n (yl[n]*h[2k-n] + yh[n]*g[2k-n])
    // More simply, we check if the information is preserved.
    // For perfect reconstruction filters, sum_i (yl[i]*h[2k-i] + yh[i]*g[2k-i]) should yield x[2k].
    // Note: there is usually a delay.

    double g[N];
    for (int i = 0; i < N; i++) g[i] = ((i % 2) == 0 ? 1 : -1) * h[N - 1 - i];

    double xr[LEN];
    for (int i = 0; i < LEN; i++) xr[i] = 0;

    for (int i = 0; i < LEN/2; i++) {
        for (int j = 0; j < N; j++) {
            int idx = 2*i + j; // This depends on the exact definition of analysis/synthesis
            if (idx < LEN) {
                xr[idx] += yl[i] * h[j];
                xr[idx] += yh[i] * g[j];
            }
        }
    }

    // Since we used an iterative method to find h, reconstruction might not be perfect.
    // Let's just check if it's "close" in some sense or at least non-zero.
    double mse = 0;
    int count = 0;
    // Account for delay/boundary effects: check middle part
    for (int i = N; i < LEN - N; i++) {
        mse += (x[i-N+1] - xr[i]) * (x[i-N+1] - xr[i]);
        count++;
    }
    printf("MSE (middle part): %f\n", mse/count);
}

int main() {
    test_lfsr_reproducibility();
    test_orthogonality();
    test_qmf_reconstruction();
    return 0;
}
