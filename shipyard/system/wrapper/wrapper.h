#pragma once

#include <system/wrapper/wrapper_common.h>

#if PLATFORM == PLATFORM_WINDOWS
#include <system/wrapper/mswin/mswinfilehandler.h>
#include <system/wrapper/mswin/mswinfilehandlerstream.h>
#endif // #if PLATFORM == PLATFORM_WINDOWS