#ifndef SPLINEX3_H
#define SPLINEX3_H

// Compute shape-preserving cubic spline interpolation
// Input:
//   x       - interpolation target x value
//   xx      - array of known x data points (must be sorted)
//   yy      - array of corresponding y values
//   nPoints - number of data points
// Output:
//   Interpolated y value at x
double SplineX3(double x, const double* xx, const double* yy, int nPoints);

#endif // SPLINEX3_H