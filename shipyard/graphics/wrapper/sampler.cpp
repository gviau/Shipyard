#include <graphics/graphicsprecomp.h>

#include <graphics/wrapper/sampler.h>

namespace Shipyard
{;

Sampler::Sampler()
    : GfxResource(GfxResourceType::Sampler)
{

}

const SamplerState& Sampler::GetSamplerState() const
{
    return m_SamplerState;
}

}