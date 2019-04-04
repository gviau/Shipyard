#pragma once

#include <system/platform.h>

#include <system/systemcommon.h>
#include <system/systemdebug.h>

namespace Shipyard
{
    template <class T>
    class Singleton
    {
    public:
        static T& GetInstance()
        {
            SHIP_ASSERT(ms_Instance != nullptr);
            return *ms_Instance;
        }

        static void CreateInstance()
        {
            SHIP_ASSERT(ms_Instance == nullptr);
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

        Singleton(const Singleton& src) = delete;
        Singleton& operator= (const Singleton& rhs) = delete;
        Singleton& operator= (const Singleton&& rhs) = delete;
    };

    template <class T> T* Singleton<T>::ms_Instance = nullptr;
}