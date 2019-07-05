#pragma once

#include <cmath>

#include <system/platform.h>

namespace Shipyard
{
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

    SHIP_INLINE shipBool IsAlmostEqual(shipFloat a, shipFloat b, shipFloat epsilon = 0.00001f)
    {
        return ((fabs(a - b)) < epsilon);
    }

    SHIP_INLINE shipBool IsAlmostEqual(shipDouble a, shipDouble b, shipDouble epsilon = 0.00001)
    {
        return ((fabs(a - b)) < epsilon);
    }
}