#include <stdio.h>
#include <stdlib.h>
#include "functions.h"

int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        printf("error - not given enough arguments\n");
        exit(1);
    }
    
    double a, b, c;
    a = atof(argv[1]);
    b = atof(argv[2]);
    c = atof(argv[3]);

    int is_pythagorean = isPythagorean(a, b, c);
    if (is_pythagorean)
    {
        double angle_a = radian_angle(a, b, c);
        double angle_b = radian_angle(b, c, a);
        double angle_c = radian_angle(c, a, b);
        printf("%f %f %f\n", angle_a, angle_b, angle_c);
    }
    else
    {
        printf("Error\n");
    }
    return 0;
}