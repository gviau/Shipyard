#include <graphics/graphicsprecomp.h>

#include <graphics/wrapper/gfxresource.h>

namespace Shipyard
{;

GfxResource::GfxResource(GfxResourceType gfxResourceType)
    : m_ResourceType(gfxResourceType)
{

}

GfxResourceType GfxResource::GetResourceType() const
{
    return m_ResourceType;
}

}