#include <graphics/shadercompiler/samplerstatecompiler.h>

#include <graphics/shadercompiler/shadercompilerutilities.h>

#include <graphics/shader/shaderkey.h>

#include <graphics/graphicscommon.h>

#include <math/mathutilities.h>

#pragma warning( disable : 4005 )

#include <d3dcommon.h>
#include <d3dcompiler.h>

#pragma warning( default : 4005 )

namespace Shipyard
{;

SamplerStateCompilerError InterpretSamplingFilterValue(const StringA& value, SamplingFilter* outValue)
{
    if (value.EqualCaseInsensitive("Nearest"))
    {
        *outValue = SamplingFilter::Nearest;
    }
    else if (value.EqualCaseInsensitive("Linear"))
    {
        *outValue = SamplingFilter::Linear;
    }
    else
    {
        return SamplerStateCompilerError::InvalidValueTypeForOption;
    }

    return SamplerStateCompilerError::NoError;
}

SamplerStateCompilerError InterpretAddressModeValue(const StringA& value, TextureAddressMode* outValue)
{
    if (value.EqualCaseInsensitive("Clamp"))
    {
        *outValue = TextureAddressMode::Clamp;
    }
    else if (value.EqualCaseInsensitive("Wrap"))
    {
        *outValue = TextureAddressMode::Wrap;
    }
    else if (value.EqualCaseInsensitive("Border"))
    {
        *outValue = TextureAddressMode::Border;
    }
    else if (value.EqualCaseInsensitive("Mirror"))
    {
        *outValue = TextureAddressMode::Mirror;
    }
    else
    {
        return SamplerStateCompilerError::InvalidValueTypeForOption;
    }

    return SamplerStateCompilerError::NoError;
}

SamplerStateCompilerError InterpretBorderColor(const StringA& value, shipFloat outValue[4])
{
    shipUint32 colorRGBA8888 = 0;
    shipBool validOption = InterpretIntegerValue(value, &colorRGBA8888);

    if (!validOption)
    {
        return SamplerStateCompilerError::InvalidValueTypeForOption;
    }

    outValue[0] = MIN(MAX(shipFloat((colorRGBA8888 >> 24) & 0xff) / 255.0f, 0.0f), 1.0f);
    outValue[1] = MIN(MAX(shipFloat((colorRGBA8888 >> 16) & 0xff) / 255.0f, 0.0f), 1.0f);
    outValue[2] = MIN(MAX(shipFloat((colorRGBA8888 >> 8)  & 0xff) / 255.0f, 0.0f), 1.0f);
    outValue[3] = MIN(MAX(shipFloat( colorRGBA8888        & 0xff) / 255.0f, 0.0f), 1.0f);

    return SamplerStateCompilerError::NoError;
}

shipBool InterpretStateBlockOption(
        const StringA& samplerStateOption,
        const StringA& samplerStateValue,
        SamplerState& samplerStateBlock,
        SamplerStateCompilerError& samplerStateBlockCompilationError)
{
    if (samplerStateOption == "")
    {
        samplerStateBlockCompilationError = SamplerStateCompilerError::MissingOption;
        return false;
    }

    if (samplerStateValue == "")
    {
        samplerStateBlockCompilationError = SamplerStateCompilerError::MissingValueForOption;
        return false;
    }

    samplerStateBlockCompilationError = SamplerStateCompilerError::NoError;

    shipBool validOptionValue = true;

    if (samplerStateOption.EqualCaseInsensitive("MinificationFiltering"))
    {
        samplerStateBlockCompilationError = InterpretSamplingFilterValue(samplerStateValue, &samplerStateBlock.MinificationFiltering);
    }
    else if (samplerStateOption.EqualCaseInsensitive("MagnificationFiltering"))
    {
        samplerStateBlockCompilationError = InterpretSamplingFilterValue(samplerStateValue, &samplerStateBlock.MagnificationFiltering);
    }
    else if (samplerStateOption.EqualCaseInsensitive("MipmapFiltering"))
    {
        samplerStateBlockCompilationError = InterpretSamplingFilterValue(samplerStateValue, &samplerStateBlock.MipmapFiltering);
    }
    else if (samplerStateOption.EqualCaseInsensitive("AddressModeU"))
    {
        samplerStateBlockCompilationError = InterpretAddressModeValue(samplerStateValue, &samplerStateBlock.AddressModeU);
    }
    else if (samplerStateOption.EqualCaseInsensitive("AddressModeV"))
    {
        samplerStateBlockCompilationError = InterpretAddressModeValue(samplerStateValue, &samplerStateBlock.AddressModeV);
    }
    else if (samplerStateOption.EqualCaseInsensitive("AddressModeW"))
    {
        samplerStateBlockCompilationError = InterpretAddressModeValue(samplerStateValue, &samplerStateBlock.AddressModeW);
    }
    else if (samplerStateOption.EqualCaseInsensitive("ComparisonFunction"))
    {
        validOptionValue = InterpretComparisonFunc(samplerStateValue, &samplerStateBlock.ComparisonFunction);
    }
    else if (samplerStateOption.EqualCaseInsensitive("MipLodBias"))
    {
        validOptionValue = InterpretFloatValue(samplerStateValue, &samplerStateBlock.MipLodBias);
    }
    else if (samplerStateOption.EqualCaseInsensitive("MaxAnisotropy"))
    {
        validOptionValue = InterpretIntegerValue(samplerStateValue, &samplerStateBlock.MaxAnisotropy);
    }
    else if (samplerStateOption.EqualCaseInsensitive("BorderRGBA"))
    {
        samplerStateBlockCompilationError = InterpretBorderColor(samplerStateValue, samplerStateBlock.BorderRGBA);
    }
    else if (samplerStateOption.EqualCaseInsensitive("MinLod"))
    {
        validOptionValue = InterpretFloatValue(samplerStateValue, &samplerStateBlock.MinLod);
    }
    else if (samplerStateOption.EqualCaseInsensitive("MaxLod"))
    {
        validOptionValue = InterpretFloatValue(samplerStateValue, &samplerStateBlock.MaxLod);
    }
    else if (samplerStateOption.EqualCaseInsensitive("UseAnisotropicFiltering"))
    {
        validOptionValue = InterpretBooleanValue(samplerStateValue, &samplerStateBlock.UseAnisotropicFiltering);
    }
    else
    {
        samplerStateBlockCompilationError = SamplerStateCompilerError::UnrecognizedOption;
    }

    if (!validOptionValue)
    {
        samplerStateBlockCompilationError = SamplerStateCompilerError::InvalidValueTypeForOption;
    }
        
    return (samplerStateBlockCompilationError == SamplerStateCompilerError::NoError);
}

SamplerStateCompilerError CompileSamplerStateBlock(
        const ShaderKey& shaderKey,
        const Array<ShaderOption>& everyPossibleShaderOption,
        const StringA& samplerStateBlockSource,
        SamplerState& samplerState)
{
    if (samplerStateBlockSource.IsEmpty())
    {
        return SamplerStateCompilerError::NoError;
    }

    // We first have to remove parts of the render state block that aren't defined.
    StringA effectiveSamplerStateBlockSource;

    GetEffectiveSourceForShaderKey(shaderKey, everyPossibleShaderOption, samplerStateBlockSource, effectiveSamplerStateBlockSource);

    SamplerStateCompilerError samplerStateCompilationError;
    CompileStateBlock(effectiveSamplerStateBlockSource, samplerState, samplerStateCompilationError, &InterpretStateBlockOption);

    return samplerStateCompilationError;
}

}