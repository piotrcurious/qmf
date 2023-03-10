
#include <stdio.h>

#include <stdlib.h>

#include <math.h>

// Define the length of filter coefficients

#define N 8

// Function to normalize a vector by its Euclidean norm

void normalize(double *vec, int len);

// Function to generate Daubechies wavelet coefficients from a pseudo-random sequence

void daub(double *seq, double *h);

int main()

{

    // Declare and initialize an array of pseudo-random sequence

    double seq[N] = {1.0,-1.0,-1.0,-1.0,-1.0,1.0,-1.0,-1.0};

    // Declare an array to store the wavelet coefficients

    double h[N];

    // Call the daub function to generate the wavelet coefficients

    daub(seq,h);

    // Print the wavelet coefficients

    printf("The wavelet coefficients are:\n");

    for (int i = 0; i < N; i++)

    {

        printf("%f\n",h[i]);

    }

    return 0;

}
