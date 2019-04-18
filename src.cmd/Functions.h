
#include <math.h>


// const double pi = 3.14159265358979323846;
// const double norm_factor = 1.0 / pow(2.0 * pi, 1.5);
// const double sigma = 1.0;

const double def_radius = 1.0;
const double def_scale = 1.0;  // default: gaussian amplitude scales down by 1/def_scale at radius, becoming narrower

double constantFunc(double x=0, double y=0, double z=0, double x0=0, double y0=0, double z0=0, double radius=0, double scale=0, bool neg=false) {
    return 0;
}

double gaussian(double x, double y, double z, double x0, double y0, double z0, double radius=def_radius, double scale=2.0, bool neg=false) {
    if (scale <= 1.0) { scale = 2.0; }
    double result = exp( -(pow(x-x0, 2.0) + pow(y-y0, 2.0) + pow(z-z0, 2.0))/(2.0*(radius*radius/(2*log(scale)))) );
    if (neg) return result; else return 1.0 - result;
}

double triangular(double x, double y, double z, double x0, double y0, double z0, double radius=def_radius, double scale=def_scale, bool neg=false) {
    if (scale == 0) { scale = 1.0; }
    double d = sqrt(pow(x-x0, 2.0) + pow(y-y0, 2.0) + pow(z-z0, 2.0)), scaledRadius = radius / scale;
    if (abs(d) < abs(scaledRadius)) {
        if (neg) return 1.0 - abs(d / scaledRadius); else return abs(d / scaledRadius);
    } else {
        if (neg) return 0; else return 1;
    }
}

double square(double x, double y, double z, double x0, double y0, double z0, double radius=def_radius, double scale=def_scale, bool neg=false) {
    double d = sqrt(pow(x-x0, 2.0) + pow(y-y0, 2.0) + pow(z-z0, 2.0));
    if (abs(d) <= radius) {
        if (abs(scale) > 1) {scale = scale/abs(scale);}
        if (neg) return scale; else return 1.0-scale;
    } else {
        if (neg) return 0; else return 1;
    }
}

double thresholdFactor(int iteration, double agressiveness, double (*func)(double, double, double, double, double, double, double, double), 
double x, double y, double z, double x0, double y0, double z0, double radius=def_radius, double scale=def_scale) {
    return 1.0-(*func)(x, y, z, x0, y0, z0, radius, scale);
}

/*
double constantFunc(const vec3f &p=vertices[t.v[0]].p, const vec3f &p0=vertices[t.v[0]].p, double radius=0, double scale=0) {
    return 0;
}

double gaussian3f(const vec3f &p, const vec3f &p0, double radius=def_radius, double scale=def_scale) {
    return exp( -pow((p-p0).length,2.0)/(2.0*(radius*radius/(2*log(scale))) ));
}

double triangular(const vec3f &p, const vec3f &p0, double radius=def_radius, double scale=def_scale) {
    if (scale == 0) { scale = 1.0; }
    double d = (p-p0).length, scaledRadius = scale * radius;
    if (abs(d) < abs(scaledRadius)) {
        return 1.0 - abs(d / scaledRadius);
    } else {
        return 0;
    }
}
*/