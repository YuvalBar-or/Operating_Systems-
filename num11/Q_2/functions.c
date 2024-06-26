#include <math.h>

const double EPSILON = 0.0001;

//assume that this order is correect -> check a^2+b^2=c^2
int isPythagorean(double a, double b, double c){
    if ((a < 0) || (b < 0) || (c < 0))
    {
        return 0;
    }
    if ((c < a) || (c < b))
    {
        return 0;
    }
    else
    {
        double sum = a*a + b*b;
        double diff = c*c - sum;
        if (diff*diff < EPSILON)
        {
            return 1;
        }
    }
    return 0;
}

double radian_angle(double a, double b, double c)
{
    double cosA = (b*b + c*c - a*a) / (2*b*c);
    return acos(cosA);
}