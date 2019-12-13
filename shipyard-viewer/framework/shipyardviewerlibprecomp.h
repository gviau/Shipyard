#pragma once

#include <graphics/graphicsprecomp.h>
#include <math/mathprecomp.h>
#include <system/systemprecomp.h>
#include <tools/toolsprecomp.h>

#ifdef SHIPYARD_VIEWER_LIB_DLL
#define SHIPYARD_VIEWER_LIB_API SHIPYARD_API_EXPORT   
#else
#define SHIPYARD_VIEWER_LIB_API SHIPYARD_API_IMPORT
#endif // #ifdef SHIPYARD_VIEWER_LIB_DLL