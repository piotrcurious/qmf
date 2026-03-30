#include "qmf.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main() {
    double seq[N] = {1, 1, 1, 1, 1, 1, 1, 1};
    double h[N];
    daub(seq, h);

    // 1. Full Frequency Response and Split Data (H(e^jw) and G(e^jw))
    FILE *fp_sweep = fopen("sweep.csv", "w");
    fprintf(fp_sweep, "freq,low_mag,low_phase,high_mag,high_phase,total_mag\n");
    for (double f = 0; f <= 0.5; f += 0.001) {
        double real_l = 0, imag_l = 0;
        double real_h = 0, imag_h = 0;
        double g[N];
        for (int i = 0; i < N; i++) g[i] = ((i % 2) == 0 ? 1 : -1) * h[N - 1 - i];

        for (int n = 0; n < N; n++) {
            double angle = -2 * M_PI * f * n;
            real_l += h[n] * cos(angle);
            imag_l += h[n] * sin(angle);
            real_h += g[n] * cos(angle);
            imag_h += g[n] * sin(angle);
        }
        double mag_l = sqrt(real_l * real_l + imag_l * imag_l);
        double phase_l = atan2(imag_l, real_l);
        double mag_h = sqrt(real_h * real_h + imag_h * imag_h);
        double phase_h = atan2(imag_h, real_h);

        // Sum of magnitudes squared should be 2 for PR QMF
        double total_mag = sqrt(mag_l * mag_l + mag_h * mag_h);

        fprintf(fp_sweep, "%f,%f,%f,%f,%f,%f\n", f, mag_l, phase_l, mag_h, phase_h, total_mag);
    }
    fclose(fp_sweep);

    // 2. Reconstruction Data (Middle part for MSE)
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
