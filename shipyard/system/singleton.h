#pragma once

#include <system/platform.h>

#include <cassert>

namespace Shipyard
{
    template <class T>
    class Singleton
    {
    public:
        static T& GetInstance()
        {
            assert(ms_Instance != nullptr);
            return *ms_Instance;
        }

        static void CreateInstance()
        {
            assert(ms_Instance == nullptr);
            ms_Instance = new T();
        }

        static void DestroyInstance()
        {
            delete ms_Instance;
            ms_Instance = nullptr;
        }

    private:
        static T* ms_Instance;

    protected:
        Singleton() {}
        virtual ~Singleton() {}
    };

    template <class T> T* Singleton<T>::ms_Instance = nullptr;
}