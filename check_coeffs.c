#include <stdio.h>
#include "qmf.h"

int main() {
    double seq[N] = {1, -1, 1, -1, 1, -1, 1, -1};
    double h[N];
    daub(seq, h);
    printf("Coefficients:\n");
    for(int i=0; i<N; i++) printf("%f ", h[i]);
    printf("\n");
    return 0;
}
