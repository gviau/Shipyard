#pragma once

#include <system/platform.h>

#include <system/memory.h>
#include <system/systemcommon.h>
#include <system/systemdebug.h>

namespace Shipyard
{
    template <class T>
    class SHIPYARD_GRAPHICS_API GraphicsSingleton
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
            ms_Instance = SHIP_NEW(T, 1);
        }

        static void DestroyInstance()
        {
            SHIP_DELETE(ms_Instance);
            ms_Instance = nullptr;
        }

    protected:
        static T* ms_Instance;

    protected:
        GraphicsSingleton() {}
        virtual ~GraphicsSingleton() {}

        GraphicsSingleton(const GraphicsSingleton& src) = delete;
        GraphicsSingleton& operator= (const GraphicsSingleton& rhs) = delete;
        GraphicsSingleton& operator= (const GraphicsSingleton&& rhs) = delete;
    };

    template <class T> T* GraphicsSingleton<T>::ms_Instance = nullptr;
}