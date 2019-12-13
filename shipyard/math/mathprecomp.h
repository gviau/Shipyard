#pragma once

#include <system/systemprecomp.h>

#ifdef SHIPYARD_MATH_DLL
#define SHIPYARD_MATH_API SHIPYARD_API_EXPORT   
#else
#define SHIPYARD_MATH_API SHIPYARD_API_IMPORT
#endif // #ifdef SHIPYARD_MATH_DLL