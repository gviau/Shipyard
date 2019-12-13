#include <shipyardunittestprecomp.h>

#include <extern/catch/catch.hpp>

#include <system/array.h>

#include <utils/unittestutils.h>

#include <algorithm>

TEST_CASE("Test array", "[Array]")
{
    Shipyard::ScoppedGlobalAllocator scoppedGlobalAllocator;

    Shipyard::Array<int> arr;

    SECTION("push_back")
    {
        constexpr int numElements = 3;

        for (int i = 0; i < numElements; i++)
        {
            arr.Add(i);
        }

        REQUIRE(arr.Size() == numElements);
        REQUIRE(arr.Capacity() >= numElements);

        for (int i = 0; i < numElements; i++)
        {
            REQUIRE(arr[i] == i);
        }
    }

    SECTION("pop_back")
    {
        constexpr int numElements = 3;

        for (int i = 0; i < numElements; i++)
        {
            arr.Add(i);
        }

        REQUIRE(arr.Size() == numElements);
        REQUIRE(arr.Capacity() >= numElements);

        for (int i = 0; i < numElements; i++)
        {
            arr.Pop();
        }

        REQUIRE(arr.Size() == 0);
        REQUIRE(arr.Capacity() >= numElements);

        arr.Add(1);
        arr.Add(2);

        arr.Pop();

        REQUIRE(arr[0] == 1);
    }

    SECTION("insert")
    {
        constexpr int numElements = 3;

        for (int i = 0; i < numElements; i++)
        {
            arr.Add(i);
        }

        arr.InsertAt(0, numElements);

        REQUIRE(arr.Size() == (numElements + 1));
        REQUIRE(arr.Capacity() >= (numElements + 1));
        REQUIRE(arr[0] == numElements);

        for (int i = 0; i < numElements; i++)
        {
            int idx = i + 1;

            REQUIRE(arr[idx] == i);
        }
    }

    SECTION("resize")
    {
        arr.Resize(10);

        REQUIRE(arr.Size() == 10);
        REQUIRE(arr.Capacity() >= 10);

        arr.Resize(5);

        REQUIRE(arr.Size() == 5);
        REQUIRE(arr.Capacity() >= 10);

        arr.Resize(15);

        REQUIRE(arr.Size() == 15);
        REQUIRE(arr.Capacity() >= 15);
    }

    SECTION("reserve")
    {
        arr.Reserve(10);

        REQUIRE(arr.Size() == 0);
        REQUIRE(arr.Capacity() == 10);

        arr.Reserve(5);

        REQUIRE(arr.Capacity() == 10);

        arr.Resize(5);

        REQUIRE(arr.Size() == 5);
        REQUIRE(arr.Capacity() == 10);
    }

    SECTION("clear")
    {
        constexpr int numElements = 3;

        for (int i = 0; i < numElements; i++)
        {
            arr.Add(i);
        }

        arr.Clear();

        REQUIRE(arr.Size() == 0);

        for (int i = 0; i < numElements; i++)
        {
            arr.Add(i);
        }

        REQUIRE(arr.Size() == numElements);
    }

    SECTION("empty")
    {
        REQUIRE(arr.Empty());

        arr.Add(1);

        REQUIRE(!arr.Empty());

        arr.Pop();

        REQUIRE(arr.Empty());
    }

    SECTION("forward iterator")
    {
        constexpr int numElements = 3;
        for (int i = 0; i < numElements; i++)
        {
            arr.Add(i);
        }

        int idx = 0;

        Shipyard::Array<int>::Iterator it = arr.begin();
        for (; it != arr.end(); ++it)
        {
            REQUIRE((*it) == idx);

            idx += 1;
        }
    }

    SECTION("Inplace array")
    {
        constexpr int numElements = 3;

        Shipyard::InplaceArray<int, numElements> inplaceArray;

        REQUIRE(inplaceArray.Capacity() == numElements);
        REQUIRE(inplaceArray.Size() == 0);

        for (int i = 0; i < numElements; i++)
        {
            inplaceArray.Add(i);
        }

        REQUIRE(inplaceArray.Capacity() == numElements);
        REQUIRE(inplaceArray.Size() == numElements);

        for (int i = 0; i < numElements; i++)
        {
            inplaceArray.Add(i);
        }

        REQUIRE(inplaceArray.Capacity() >= numElements * 2);
        REQUIRE(inplaceArray.Size() == numElements * 2);

        inplaceArray.Clear();

        REQUIRE(inplaceArray.Capacity() == 0);
        REQUIRE(inplaceArray.Size() == 0);

        inplaceArray.Reserve(numElements);

        for (int i = 0; i < numElements; i++)
        {
            inplaceArray.Add(i);
        }

        REQUIRE(inplaceArray.Capacity() == numElements);
        REQUIRE(inplaceArray.Size() == numElements);
    }

    SECTION("Change allocator")
    {
        constexpr int numElements = 3;

        for (int i = 0; i < numElements; i++)
        {
            arr.Add(i);
        }

        const size_t heapSize = 1024;
        Shipyard::ScoppedBuffer scoppedBuffer(heapSize);

        Shipyard::FixedHeapAllocator fixedHeapAllocator;
        fixedHeapAllocator.Create(scoppedBuffer.pBuffer, heapSize);

        arr.SetAllocator(&fixedHeapAllocator);

        REQUIRE(arr.Size() == numElements);
        REQUIRE(arr.Capacity() >= numElements);

        for (int i = 0; i < numElements; i++)
        {
            REQUIRE(arr[i] == i);
        }

        // Required since we destroy the allocator before the destructor.
        arr.Clear();

        fixedHeapAllocator.Destroy();
    }
}