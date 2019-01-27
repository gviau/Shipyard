#pragma once

#include <system/platform.h>

namespace Shipyard
{

#if PLATFORM == PLATFORM_WINDOWS

class MswinFileHandler;

typedef MswinFileHandler FileHandler;

#endif // #if PLATFORM == PLATFORM_WINDOWS
}