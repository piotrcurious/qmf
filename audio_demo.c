#include "qmf.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

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

    double g[N];
    for (int i = 0; i < N; i++) g[i] = ((i % 2) == 0 ? 1 : -1) * h[N - 1 - i];

    // State for continuous filtering (buffer for N-1 previous samples)
    double input_state[N - 1];
    memset(input_state, 0, sizeof(input_state));

    // For stereo split visualization
    // We use synthesis filter impulse response overlap-add to simulate continuous synthesis
    double xl_state[N - 1];
    double xh_state[N - 1];
    memset(xl_state, 0, sizeof(xl_state));
    memset(xh_state, 0, sizeof(xh_state));

    double input_chunk[CHUNK_SIZE];
    double extended_input[CHUNK_SIZE + N - 1];
    double yl[CHUNK_SIZE / 2];
    double yh[CHUNK_SIZE / 2];

    printf("Processing real-time filter demo with shift %f and state persistence...\n", shift);
    while (fread(input_chunk, sizeof(double), CHUNK_SIZE, in) == CHUNK_SIZE) {
        // Construct extended input with history for continuous filtering
        memcpy(extended_input, input_state, (N - 1) * sizeof(double));
        memcpy(extended_input + N - 1, input_chunk, CHUNK_SIZE * sizeof(double));

        // Save current chunk end for next chunk history
        memcpy(input_state, input_chunk + CHUNK_SIZE - (N - 1), (N - 1) * sizeof(double));

        // Block-based Analysis
        int len_y = CHUNK_SIZE / 2;
        for (int i = 0; i < len_y; i++) {
            yl[i] = 0; yh[i] = 0;
            for (int j = 0; j < N; j++) {
                int idx = 2 * i + 1 - j + (N - 1); // Adjust for history
                if (idx >= 0 && idx < (CHUNK_SIZE + N - 1)) {
                    yl[i] += extended_input[idx] * h[j];
                    yh[i] += extended_input[idx] * g[j];
                }
            }
        }

        // Block-based Synthesis (with state persistence)
        double xl_recon[CHUNK_SIZE];
        double xh_recon[CHUNK_SIZE];
        memset(xl_recon, 0, sizeof(xl_recon));
        memset(xh_recon, 0, sizeof(xh_recon));

        // Add history (tails from previous chunk)
        memcpy(xl_recon, xl_state, (N - 1) * sizeof(double));
        memcpy(xh_recon, xh_state, (N - 1) * sizeof(double));
        memset(xl_state, 0, sizeof(xl_state));
        memset(xh_state, 0, sizeof(xh_state));

        for (int i = 0; i < len_y; i++) {
            for (int j = 0; j < N; j++) {
                int idx = 2 * i + j - (N - 2);
                if (idx >= 0 && idx < CHUNK_SIZE) {
                    xl_recon[idx] += yl[i] * h[j];
                    xh_recon[idx] += yh[i] * g[j];
                } else if (idx >= CHUNK_SIZE) {
                    // Save tails for next chunk
                    xl_state[idx - CHUNK_SIZE] += yl[i] * h[j];
                    xh_state[idx - CHUNK_SIZE] += yh[i] * g[j];
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
