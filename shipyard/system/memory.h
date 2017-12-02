#pragma once

namespace Shipyard
{
#define MemAlloc(objType) new objType
#define MemFree(ptr) delete ptr
}