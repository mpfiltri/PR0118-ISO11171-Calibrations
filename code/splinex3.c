#include "splinex3.h"
#include <math.h>

static double dxx(double x1, double x0) {
    double dx = x1 - x0;
    return (dx == 0) ? 1e30 : dx;
}

double SplineX3(double x, const double* xx, const double* yy, int nPoints) {
    int i, j, Num = 0;
    int Nmax = nPoints - 1;
    double gxx[2], ggxx[2];
    double A, B, C, D;

    if (x < xx[0] || x > xx[Nmax]) {
        return 0.0;
    }

    for (i = 1; i <= Nmax; i++) {
        if (x <= xx[i]) {
            Num = i;
            break;
        }
    }

    for (j = 0; j <= 1; j++) {
        i = Num - 1 + j;

        if (i == 0 || i == Nmax) {
            gxx[j] = 1e30;
        } else if ((yy[i + 1] - yy[i] == 0) || (yy[i] - yy[i - 1] == 0)) {
            gxx[j] = 0.0;
        } else if (((xx[i + 1] - xx[i]) / (yy[i + 1] - yy[i]) +
                    (xx[i] - xx[i - 1]) / (yy[i] - yy[i - 1])) == 0) {
            gxx[j] = 0.0;
        } else if ((yy[i + 1] - yy[i]) * (yy[i] - yy[i - 1]) < 0) {
            gxx[j] = 0.0;
        } else {
            gxx[j] = 2.0 / (dxx(xx[i + 1], xx[i]) / (yy[i + 1] - yy[i]) +
                            dxx(xx[i], xx[i - 1]) / (yy[i] - yy[i - 1]));
        }
    }

    if (Num == 1) {
        gxx[0] = 1.5 * (yy[Num] - yy[Num - 1]) / dxx(xx[Num], xx[Num - 1]) - 0.5 * gxx[1];
    }
    if (Num == Nmax) {
        gxx[1] = 1.5 * (yy[Num] - yy[Num - 1]) / dxx(xx[Num], xx[Num - 1]) - 0.5 * gxx[0];
    }

    ggxx[0] = -2 * (gxx[1] + 2 * gxx[0]) / dxx(xx[Num], xx[Num - 1]) +
              6 * (yy[Num] - yy[Num - 1]) / pow(dxx(xx[Num], xx[Num - 1]), 2);
    ggxx[1] = 2 * (2 * gxx[1] + gxx[0]) / dxx(xx[Num], xx[Num - 1]) -
              6 * (yy[Num] - yy[Num - 1]) / pow(dxx(xx[Num], xx[Num - 1]), 2);

    D = (1.0 / 6.0) * (ggxx[1] - ggxx[0]) / dxx(xx[Num], xx[Num - 1]);
    C = 0.5 * (xx[Num] * ggxx[0] - xx[Num - 1] * ggxx[1]) / dxx(xx[Num], xx[Num - 1]);
    B = (yy[Num] - yy[Num - 1] - C * (pow(xx[Num], 2) - pow(xx[Num - 1], 2)) -
         D * (pow(xx[Num], 3) - pow(xx[Num - 1], 3))) / dxx(xx[Num], xx[Num - 1]);
    A = yy[Num - 1] - B * xx[Num - 1] - C * pow(xx[Num - 1], 2) - D * pow(xx[Num - 1], 3);

    return A + B * x + C * pow(x, 2) + D * pow(x, 3);
}