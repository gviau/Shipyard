#pragma once

#include <graphics/graphicscommon.h>

#include <graphics/wrapper/gfxresource.h>

namespace Shipyard
{
    class SHIPYARD_API Sampler : public GfxResource
    {
    public:
        Sampler();

        const SamplerState& GetSamplerState() const;

    protected:
        SamplerState m_SamplerState;
    };
}