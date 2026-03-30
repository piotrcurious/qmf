#include "qmf.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main() {
    double seq[N] = {1, 1, 1, 1, 1, 1, 1, 1};
    double h[N];
    daub(seq, h);

    // 1. Frequency Sweep Data
    FILE *fp_sweep = fopen("sweep.csv", "w");
    fprintf(fp_sweep, "freq,low_pass,high_pass\n");
    for (double f = 0; f <= 0.5; f += 0.005) {
        double real_l = 0, imag_l = 0;
        double real_h = 0, imag_h = 0;
        double g[N];
        for (int i = 0; i < N; i++) g[i] = ((i % 2) == 0 ? 1 : -1) * h[N - 1 - i];

        for (int n = 0; n < N; n++) {
            real_l += h[n] * cos(-2 * M_PI * f * n);
            imag_l += h[n] * sin(-2 * M_PI * f * n);
            real_h += g[n] * cos(-2 * M_PI * f * n);
            imag_h += g[n] * sin(-2 * M_PI * f * n);
        }
        double mag_l = sqrt(real_l * real_l + imag_l * imag_l);
        double mag_h = sqrt(real_h * real_h + imag_h * imag_h);
        fprintf(fp_sweep, "%f,%f,%f\n", f, mag_l, mag_h);
    }
    fclose(fp_sweep);

    // 2. Reconstruction Data
    // We use a complex signal for reconstruction test
    double x[LEN];
    for (int i = 0; i < LEN; i++) {
        x[i] = sin(2 * M_PI * i / 10.0) + 0.5 * sin(2 * M_PI * i / 3.0);
    }

    double yl[LEN / 2];
    double yh[LEN / 2];
    qmf(x, LEN, h, yl, yh);

    double xr[LEN];
    qmf_synth(yl, yh, LEN / 2, h, xr);

    FILE *fp_recon = fopen("recon.csv", "w");
    fprintf(fp_recon, "t,original,reconstructed\n");
    for (int i = 0; i < LEN; i++) {
        fprintf(fp_recon, "%d,%f,%f\n", i, x[i], xr[i]);
    }
    fclose(fp_recon);

    return 0;
}
