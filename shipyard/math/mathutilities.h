#pragma once

#include <cmath>

namespace Shipyard
{
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

    inline bool IsAlmostEqual(float a, float b, float epsilon = 0.00001f)
    {
        return ((fabs(a) - fabs(b)) < epsilon);
    }

    inline bool IsAlmostEqual(double a, double b, double epsilon = 0.00001)
    {
        return ((fabs(a) - fabs(b)) < epsilon);
    }
}