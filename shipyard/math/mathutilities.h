#pragma once

#include <cmath>

#include <system/platform.h>

namespace Shipyard
{
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

    SHIP_INLINE bool IsAlmostEqual(float a, float b, float epsilon = 0.00001f)
    {
        return ((fabs(a - b)) < epsilon);
    }

    SHIP_INLINE bool IsAlmostEqual(double a, double b, double epsilon = 0.00001)
    {
        return ((fabs(a - b)) < epsilon);
    }
}