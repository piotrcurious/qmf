
#include <stdio.h>

#include <stdlib.h>

#include <math.h>

// Define the length of filter coefficients

#define N 8

// Define the decimation factor

#define M 4

// Define the length of input signal

#define LEN 64

// Function to normalize a vector by its Euclidean norm

void normalize(double *vec, int len);

// Function to generate Daubechies wavelet coefficients from a pseudo-random sequence

void daub(double *seq, double *h);

// Function to perform QMF filtering on an input signal using Daubechies wavelet coefficients

void qmf(double *x, double *h, double *yl, double *yh);

int main()

{

    // Declare and initialize an array of pseudo-random sequence

    double seq[N] = {1.0,-1.0,-1.0,-1.0,-1.0,1.0,-1.0,-1.0};

    // Declare an array to store the wavelet coefficients

    double h[N];

    // Call the daub function to generate the wavelet coefficients

    daub(seq,h);

    // Declare and initialize an array of input signal (a sine wave with frequency 2*pi/LEN)

    double x[LEN];

    for (int i = 0; i < LEN; i++)

    {

        x[i] = sin(2*M_PI*i/LEN);

    }

    // Declare arrays to store the output signals (the low-pass and high-pass filtered signals)

    double yl[LEN/M];

    double yh[LEN/M];

    // Call the qmf function to perform QMF filtering on the input signal using Daubechies wavelet coefficients

    qmf(x,h,yl,yh);

    // Print the output signals

    printf("The low-pass filtered signal is:\n");

    for (int i = 0; i < LEN/M; i++)

    {

        printf("%f\n",yl[i]);

    }

    

     printf("The high-pass filtered signal is:\n");

     for (int i = 0; i < LEN/M; i++)

     {

         printf("%f\n",yh[i]);

     }

     return 0;

}
