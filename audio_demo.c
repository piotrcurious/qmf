#include "qmf.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

#define CHUNK_SIZE 64

// Helper to write raw 16-bit PCM stereo (Little Endian)
void write_stereo(FILE *out, double l, double r) {
    int16_t out_l = (int16_t)(l * 32767.0);
    int16_t out_r = (int16_t)(r * 32767.0);
    fwrite(&out_l, 2, 1, out);
    fwrite(&out_r, 2, 1, out);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input_mono_raw_double> <output_stereo_raw_16bit> [freq_shift]\n", argv[0]);
        fprintf(stderr, "Processes raw double PCM (mono) to raw 16-bit stereo (low-L, high-R)\n");
        fprintf(stderr, "freq_shift: Spectral shift for split point (-0.5 to 0.5, default 0)\n");
        return 1;
    }

    FILE *in = fopen(argv[1], "rb");
    FILE *out = fopen(argv[2], "wb");
    if (!in || !out) {
        perror("fopen");
        return 1;
    }

    double shift = (argc > 3) ? atof(argv[3]) : 0.0;

    // Generate shifted coefficients
    double seq[N] = {1, 1, 1, 1, 1, 1, 1, 1};
    double h[N];
    daub_shift(seq, h, shift);

    double input_buffer[CHUNK_SIZE];
    double yl[CHUNK_SIZE / 2];
    double yh[CHUNK_SIZE / 2];

    // For high-quality reconstruction visualization (stereo split)
    // We use the synthesis filters but without summing to maintain separation
    double xl_recon[CHUNK_SIZE];
    double xh_recon[CHUNK_SIZE];

    // We need synthesis filters g as well
    double g[N];
    for (int i = 0; i < N; i++) g[i] = ((i % 2) == 0 ? 1 : -1) * h[N - 1 - i];

    printf("Processing real-time filter demo with shift %f...\n", shift);
    while (fread(input_buffer, sizeof(double), CHUNK_SIZE, in) == CHUNK_SIZE) {
        // Analysis split
        qmf(input_buffer, CHUNK_SIZE, h, yl, yh);

        // Synthesis upsampling (high quality)
        for (int i = 0; i < CHUNK_SIZE; i++) {
            xl_recon[i] = 0;
            xh_recon[i] = 0;
        }

        for (int i = 0; i < CHUNK_SIZE / 2; i++) {
            for (int j = 0; j < N; j++) {
                int idx = 2 * i + j - (N - 2);
                if (idx >= 0 && idx < CHUNK_SIZE) {
                    xl_recon[idx] += yl[i] * h[N - 1 - j];
                    xh_recon[idx] += yh[i] * g[N - 1 - j];
                }
            }
        }

        for (int i = 0; i < CHUNK_SIZE; i++) {
            write_stereo(out, xl_recon[i], xh_recon[i]);
        }
    }

    fclose(in);
    fclose(out);
    printf("Done. Output saved to %s\n", argv[2]);
    return 0;
}
