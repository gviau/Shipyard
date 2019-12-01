#pragma once

#include <system/platform.h>

namespace Shipyard
{

#if PLATFORM == PLATFORM_WINDOWS

class MswinFileHandler;
class MswinFileHandlerStream;

typedef MswinFileHandler FileHandler;
typedef MswinFileHandlerStream FileHandlerStream;

#endif // #if PLATFORM == PLATFORM_WINDOWS
}