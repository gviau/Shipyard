#pragma once

#include <system/platform.h>

#ifdef SHIPYARD_SYSTEM_DLL
#define SHIPYARD_SYSTEM_API SHIPYARD_API_EXPORT   
#else
#define SHIPYARD_SYSTEM_API SHIPYARD_API_IMPORT
#endif // #ifdef SHIPYARD_SYSTEM_DLL