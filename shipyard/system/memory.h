#pragma once

namespace Shipyard
{
#define MemAlloc(objType) new objType
#define MemArrayAlloc(objType, numElements) new objType[numElements]
#define MemFree(ptr) delete ptr
#define MemArrayFree(ptr) delete[] ptr
}