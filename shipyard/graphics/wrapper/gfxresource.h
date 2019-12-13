#pragma once

#include <graphics/wrapper/wrapper_common.h>

namespace Shipyard
{
    class SHIPYARD_GRAPHICS_API GfxResource
    {
    public:
        GfxResource(GfxResourceType resourceType);

        GfxResourceType GetResourceType() const;

    protected:
        GfxResourceType m_ResourceType;
    };
}