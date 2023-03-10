

// QMF implementation using debauchies wavelet derived from LFSR as core

// Based on https://www.mathworks.com/help/wavelet/ref/dwt.html

#include <stdio.h>

#include <math.h>

// Define the LFSR parameters

#define POLY 0x80000057 // Polynomial for LFSR

#define INIT 0x1        // Initial state for LFSR

#define TAPS 32         // Number of taps for LFSR

// Define the QMF parameters

#define N 8             // Length of filter coefficients

#define M 4             // Decimation factor

// Define the input signal parameters

#define LEN 64          // Length of input signal

// Function to generate a pseudo-random sequence using LFSR

void lfsr(unsigned int *state, int *seq, int len)

{

    unsigned int lsb; // Least significant bit

    int i;

    for (i = 0; i < len; i++)

    {

        lsb = *state & 1;   // Get LSB (i.e., the output bit).

        seq[i] = lsb;       // Store the output bit in the sequence array.

        *state >>= 1;       // Shift register

        if (lsb)            // If the output bit is 1, apply toggle mask.

            *state ^= POLY;

    }

}

// Function to normalize a vector by its Euclidean norm

void normalize(double *vec, int len)

{

    double norm = 0.0; // Euclidean norm

    int i;

    for (i = 0; i < len; i++)

    {

        norm += vec[i] * vec[i]; // Sum of squares

    }

    norm = sqrt(norm);          // Square root of sum of squares

    for (i = 0; i < len; i++)

    {

        vec[i] /= norm;         // Divide each element by norm

    }

}

// Function to generate debauchies wavelet coefficients from a pseudo-random sequence using Daubechies' algorithm 

void daub(double *h, int n, int *seq)

{

    double beta[n];   // Beta coefficients 

    double alpha[n];  // Alpha coefficients 

    double gamma[n];  // Gamma coefficients 

    double delta[n];  // Delta coefficients 

    double zeta[n/2]; // Zeta coefficients 

    double eta[n/2];  // Eta coefficients 

   

   int i;

   for (i = 0; i < n; i++)

   {

       beta[i] = seq[i] ? -1.0 : 1.0;     // Map sequence to +/-1 values and store in beta array 

   }

   normalize(beta,n);                    // Normalize beta array 

   for (i = n-1; i >= n/2 ; i--)

   {

       alpha[i] = beta[i-n/2] - beta[i];     // Compute alpha array from beta array 

       gamma[i-n/2] = beta[i-n/2] + beta[i];     //

   }

   normalize(alpha,n);                   //

   

   

}
