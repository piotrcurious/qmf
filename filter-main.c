#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "qmf.h"

// Original filter-main.c used M=4 but QMF is usually M=2.
// I will keep M=2 to be consistent with qmf.c implementation.
#undef M
#define M 2

int main()
{
    // Declare and initialize an array of pseudo-random sequence
    double seq[N] = {1.0, -1.0, -1.0, -1.0, -1.0, 1.0, -1.0, -1.0};

    // Declare an array to store the wavelet coefficients
    double h[N];

    // Call the daub function to generate the wavelet coefficients
    daub(seq, h);

    // Declare and initialize an array of input signal (a sine wave with frequency 2*pi/LEN)
    double x[LEN];
    for (int i = 0; i < LEN; i++)
    {
        x[i] = sin(2 * M_PI * i / LEN);
    }

    // Declare arrays to store the output signals (the low-pass and high-pass filtered signals)
    double yl[LEN / M];
    double yh[LEN / M];

    // Call the qmf function to perform QMF filtering on the input signal
    qmf(x, LEN, h, yl, yh);

    // Print the output signals
    printf("The low-pass filtered signal is:\n");
    for (int i = 0; i < LEN / M; i++)
    {
        printf("%f\n", yl[i]);
    }

    printf("The high-pass filtered signal is:\n");
    for (int i = 0; i < LEN / M; i++)
    {
        printf("%f\n", yh[i]);
    }

    return 0;
}
