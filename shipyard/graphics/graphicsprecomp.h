#pragma once

#include <math/mathprecomp.h>
#include <system/systemprecomp.h>

#ifdef SHIPYARD_GRAPHICS_DLL
#define SHIPYARD_GRAPHICS_API SHIPYARD_API_EXPORT   
#else
#define SHIPYARD_GRAPHICS_API SHIPYARD_API_IMPORT
#endif // #ifdef SHIPYARD_GRAPHICS_DLL