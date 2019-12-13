#pragma once

#include <graphics/graphicsprecomp.h>
#include <math/mathprecomp.h>
#include <system/systemprecomp.h>

#ifdef SHIPYARD_TOOLS_DLL
#define SHIPYARD_TOOLS_API SHIPYARD_API_EXPORT   
#else
#define SHIPYARD_TOOLS_API SHIPYARD_API_IMPORT
#endif // #ifdef SHIPYARD_TOOLS_DLL