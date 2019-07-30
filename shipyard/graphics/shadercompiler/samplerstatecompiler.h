#pragma once

#include <graphics/shader/shaderoptions.h>

#include <system/array.h>
#include <system/string.h>

namespace Shipyard
{
    struct SamplerState;
    class ShaderKey;

    enum class SamplerStateCompilerError
    {
        NoError,
        UnrecognizedOption,
        InvalidValueTypeForOption,
        MissingOption,
        MissingValueForOption,
    };

    SHIPYARD_API SamplerStateCompilerError CompileSamplerStateBlock(
            const ShaderKey& shaderKey,
            const Array<ShaderOption>& everyPossibleShaderOption,
            const StringA& samplerStateBlockSource,
            SamplerState& samplerState);
}