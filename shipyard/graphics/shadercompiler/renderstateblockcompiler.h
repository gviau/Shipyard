#pragma once

#include <graphics/shader/shaderoptions.h>

#include <system/array.h>
#include <system/string.h>

namespace Shipyard
{

struct RenderStateBlock;
class ShaderKey;

enum class RenderStateBlockCompilationError
{
    NoError,
    UnrecognizedOption,
    InvalidValueTypeForOption,
    MissingOption,
    MissingValueForOption,
};

SHIPYARD_API RenderStateBlockCompilationError CompileRenderStateBlock(
        const ShaderKey& shaderKey,
        const Array<ShaderOption>& everyPossibleShaderOption,
        const StringA& renderPipelineBlockSource,
        RenderStateBlock& renderStateBlock);

}