#pragma once

#include <graphics/graphicscommon.h>

namespace Shipyard
{
    class SHIPYARD_API Sampler
    {
    public:
        const SamplerState& GetSamplerState() const;

    protected:
        SamplerState m_SamplerState;
    };
}