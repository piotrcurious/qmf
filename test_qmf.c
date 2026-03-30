#include "qmf.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main() {
    printf("Starting LFSR Test...\n");
    init_lfsr(1234567890123456789UL, 0);
    uint64_t initial_state = 1234567890123456789UL;

    // Simple test for reproducibility
    int bit1 = shift_lfsr('L');
    int bit2 = shift_lfsr('L');
    printf("First two bits (Left): %d, %d\n", bit1, bit2);

    init_lfsr(initial_state, 0);
    uint8_t byte = shift_lfsr_n('L', 8);
    printf("First byte (Left): %02X\n", byte);

    printf("LFSR Test Complete.\n");

    printf("\nStarting Wavelet/QMF Test...\n");
    double seq[N] = {1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0};
    double h[N];
    daub(seq, h);
    printf("Generated coefficients:\n");
    for (int i = 0; i < N; i++) {
        printf("h[%d] = %f\n", i, h[i]);
    }

    double x[LEN];
    for (int i = 0; i < LEN; i++) {
        x[i] = sin(2 * M_PI * i / 16.0); // Sine wave with period 16
    }

    double yl[LEN / 2];
    double yh[LEN / 2];
    qmf(x, LEN, h, yl, yh);

    printf("\nFirst 8 samples of Analysis bank outputs:\n");
    printf("idx | low-pass | high-pass\n");
    for (int i = 0; i < 8; i++) {
        printf("%3d | %8.4f | %8.4f\n", i, yl[i], yh[i]);
    }

    printf("Wavelet/QMF Test Complete.\n");
    return 0;
}
