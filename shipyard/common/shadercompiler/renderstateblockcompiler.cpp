#include <common/shadercompiler/renderstateblockcompiler.h>

#include <common/common.h>
#include <common/shaderkey.h>

#include <system/string.h>

#include <locale>

#pragma warning( disable : 4005 )

#include <d3dcommon.h>
#include <d3dcompiler.h>

#pragma warning( default : 4005 )

namespace Shipyard
{;

RenderStateBlockCompilationError InterpretBooleanValue(const StringA& value, bool* outValue)
{
    RenderStateBlockCompilationError renderStateBlockCompilationError = RenderStateBlockCompilationError::NoError;

    if (value.EqualCaseInsensitive("true"))
    {
        *outValue = true;
    }
    else if (value.EqualCaseInsensitive("false"))
    {
        *outValue = false;
    }
    else
    {
        renderStateBlockCompilationError = RenderStateBlockCompilationError::InvalidValueTypeForOption;
    }

    return renderStateBlockCompilationError;
}

template <typename IntegerType>
RenderStateBlockCompilationError InterpretIntegerValue(const StringA& value, IntegerType* outValue)
{
    RenderStateBlockCompilationError renderStateBlockCompilationError = RenderStateBlockCompilationError::NoError;

    size_t startingIndex = 0;
    bool isNegative = false;
    if (value[0] == '-')
    {
        startingIndex = 1;
        isNegative = true;
    }

    if (value.Substring(startingIndex, 2 + startingIndex).EqualCaseInsensitive("0x"))
    {
        StringA hexadecimalValue = value.Substring(2 + startingIndex, value.InvalidIndex);

        for (size_t i = 0; i < hexadecimalValue.Size(); i++)
        {
            int c = int(hexadecimalValue[i]);

            bool isCharacterInvalidHexadecimal = ((c < int('0') || c > int('9')) && (c < int('a') || c > int('f')) && (c < int('A') || c > int('F')));
            if (isCharacterInvalidHexadecimal)
            {
                renderStateBlockCompilationError = RenderStateBlockCompilationError::InvalidValueTypeForOption;
                break;
            }
        }

        *outValue = IntegerType(strtol(hexadecimalValue.GetBuffer(), nullptr, 16));

        if (isNegative)
        {
            *outValue = - (*outValue);
        }
    }
    else if (value.Substring(startingIndex, 2 + startingIndex).EqualCaseInsensitive("0b"))
    {
        StringA binaryValue = value.Substring(2 + startingIndex, value.InvalidIndex);

        for (size_t i = 0; i < binaryValue.Size(); i++)
        {
            char c = binaryValue[i];

            bool isCharacterInvalidBinary = (c != '0' && c != '1');
            if (isCharacterInvalidBinary)
            {
                renderStateBlockCompilationError = RenderStateBlockCompilationError::InvalidValueTypeForOption;
                break;
            }
        }

        *outValue = IntegerType(strtol(binaryValue.GetBuffer(), nullptr, 2));

        if (isNegative)
        {
            *outValue = -(*outValue);
        }
    }
    else
    {
        for (size_t i = startingIndex; i < value.Size(); i++)
        {
            int c = int(value[i]);

            bool isCharacterInvalidDecimal = (c < int('0') || c > int('9'));
            if (isCharacterInvalidDecimal)
            {
                renderStateBlockCompilationError = RenderStateBlockCompilationError::InvalidValueTypeForOption;
                break;
            }
        }

        // Negative values are properly handlded here since we hand over the whole string.
        *outValue = IntegerType(strtol(value.GetBuffer(), nullptr, 10));
    }

    return renderStateBlockCompilationError;
}

RenderStateBlockCompilationError InterpretFloatValue(const StringA& value, float* outValue)
{
    RenderStateBlockCompilationError renderStateBlockCompilationError = RenderStateBlockCompilationError::NoError;

    for (size_t i = 0; i < value.Size(); i++)
    {
        int c = int(value[i]);

        bool isCharacterInvalidFloat = ((c < int('0') || c > int('9')) && c != int('-') && c != int('.') && c != int('f') && c != int('F'));
        if (isCharacterInvalidFloat)
        {
            renderStateBlockCompilationError = RenderStateBlockCompilationError::InvalidValueTypeForOption;
            break;
        }
    }

    *outValue = float(atof(value.GetBuffer()));

    return renderStateBlockCompilationError;
}

RenderStateBlockCompilationError InterpretFillMode(const StringA& value, FillMode* outValue)
{
    RenderStateBlockCompilationError renderStateBlockCompilationError = RenderStateBlockCompilationError::NoError;

    if (value.EqualCaseInsensitive("Solid"))
    {
        *outValue = FillMode::Solid;
    }
    else if (value.EqualCaseInsensitive("Wireframe"))
    {
        *outValue = FillMode::Wireframe;
    }
    else
    {
        renderStateBlockCompilationError = RenderStateBlockCompilationError::InvalidValueTypeForOption;
    }

    return renderStateBlockCompilationError;
}

RenderStateBlockCompilationError InterpretCullMode(const StringA& value, CullMode* outValue)
{
    RenderStateBlockCompilationError renderStateBlockCompilationError = RenderStateBlockCompilationError::NoError;

    if (value.EqualCaseInsensitive("CullNone"))
    {
        *outValue = CullMode::CullNone;
    }
    else if (value.EqualCaseInsensitive("CullBackFace"))
    {
        *outValue = CullMode::CullBackFace;
    }
    else if (value.EqualCaseInsensitive("CullFrontFace"))
    {
        *outValue = CullMode::CullFrontFace;
    }
    else
    {
        renderStateBlockCompilationError = RenderStateBlockCompilationError::InvalidValueTypeForOption;
    }

    return renderStateBlockCompilationError;
}

RenderStateBlockCompilationError InterpretComparisonFunc(const StringA& value, ComparisonFunc* outValue)
{
    RenderStateBlockCompilationError renderStateBlockCompilationError = RenderStateBlockCompilationError::NoError;

    if (value.EqualCaseInsensitive("Never"))
    {
        *outValue = ComparisonFunc::Never;
    }
    else if (value.EqualCaseInsensitive("Less"))
    {
        *outValue = ComparisonFunc::Less;
    }
    else if (value.EqualCaseInsensitive("Equal"))
    {
        *outValue = ComparisonFunc::Equal;
    }
    else if (value.EqualCaseInsensitive("LessEqual"))
    {
        *outValue = ComparisonFunc::LessEqual;
    }
    else if (value.EqualCaseInsensitive("Greater"))
    {
        *outValue = ComparisonFunc::Greater;
    }
    else if (value.EqualCaseInsensitive("NotEqual"))
    {
        *outValue = ComparisonFunc::NotEqual;
    }
    else if (value.EqualCaseInsensitive("GreaterEqual"))
    {
        *outValue = ComparisonFunc::GreaterEqual;
    }
    else if (value.EqualCaseInsensitive("Always"))
    {
        *outValue = ComparisonFunc::Always;
    }
    else
    {
        renderStateBlockCompilationError = RenderStateBlockCompilationError::InvalidValueTypeForOption;
    }

    return renderStateBlockCompilationError;
}

RenderStateBlockCompilationError InterpretStencilOperation(const StringA& value, StencilOperation* outValue)
{
    RenderStateBlockCompilationError renderStateBlockCompilationError = RenderStateBlockCompilationError::NoError;

    if (value.EqualCaseInsensitive("Keep"))
    {
        *outValue = StencilOperation::Keep;
    }
    else if (value.EqualCaseInsensitive("Zero"))
    {
        *outValue = StencilOperation::Zero;
    }
    else if (value.EqualCaseInsensitive("Replace"))
    {
        *outValue = StencilOperation::Replace;
    }
    else if (value.EqualCaseInsensitive("IncrSaturate"))
    {
        *outValue = StencilOperation::IncrSaturate;
    }
    else if (value.EqualCaseInsensitive("DecrSaturate"))
    {
        *outValue = StencilOperation::DecrSaturate;
    }
    else if (value.EqualCaseInsensitive("Invert"))
    {
        *outValue = StencilOperation::Invert;
    }
    else if (value.EqualCaseInsensitive("Incr"))
    {
        *outValue = StencilOperation::Incr;
    }
    else if (value.EqualCaseInsensitive("Decr"))
    {
        *outValue = StencilOperation::Decr;
    }
    else
    {
        renderStateBlockCompilationError = RenderStateBlockCompilationError::InvalidValueTypeForOption;
    }

    return renderStateBlockCompilationError;
}

RenderStateBlockCompilationError InterpretRenderPipelineStateOption(
        const StringA& renderPipelineStateOption,
        const StringA& renderPipelineStateValue,
        RasterizerState& rasterizerState,
        DepthStencilState& depthStencilState)
{
    if (renderPipelineStateOption == "")
    {
        return RenderStateBlockCompilationError::MissingOption;
    }

    if (renderPipelineStateValue == "")
    {
        return RenderStateBlockCompilationError::MissingValueForOption;
    }

    RenderStateBlockCompilationError renderStateBlockCompilationError = RenderStateBlockCompilationError::UnrecognizedOption;

    if (renderPipelineStateOption.EqualCaseInsensitive("IsFrontCounterClockwise"))
    {
        renderStateBlockCompilationError = InterpretBooleanValue(renderPipelineStateValue, &rasterizerState.m_IsFrontCounterClockwise);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("DepthClipEnable"))
    {
        renderStateBlockCompilationError = InterpretBooleanValue(renderPipelineStateValue, &rasterizerState.m_DepthClipEnable);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("ScissorEnable"))
    {
        renderStateBlockCompilationError = InterpretBooleanValue(renderPipelineStateValue, &rasterizerState.m_ScissorEnable);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("MultisampleEnable"))
    {
        renderStateBlockCompilationError = InterpretBooleanValue(renderPipelineStateValue, &rasterizerState.m_MultisampleEnable);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("AntialiasedLineEnable"))
    {
        renderStateBlockCompilationError = InterpretBooleanValue(renderPipelineStateValue, &rasterizerState.m_AntialiasedLineEnable);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("DepthEnable"))
    {
        renderStateBlockCompilationError = InterpretBooleanValue(renderPipelineStateValue, &depthStencilState.m_DepthEnable);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("EnableDepthWrite"))
    {
        renderStateBlockCompilationError = InterpretBooleanValue(renderPipelineStateValue, &depthStencilState.m_EnableDepthWrite);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("StencilEnable"))
    {
        renderStateBlockCompilationError = InterpretBooleanValue(renderPipelineStateValue, &depthStencilState.m_StencilEnable);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("DepthBias"))
    {
        renderStateBlockCompilationError = InterpretIntegerValue(renderPipelineStateValue, &rasterizerState.m_DepthBias);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("StencilReadMask"))
    {
        renderStateBlockCompilationError = InterpretIntegerValue(renderPipelineStateValue, &depthStencilState.m_StencilReadMask);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("StencilWriteMask"))
    {
        renderStateBlockCompilationError = InterpretIntegerValue(renderPipelineStateValue, &depthStencilState.m_StencilWriteMask);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("DepthBiasClamp"))
    {
        renderStateBlockCompilationError = InterpretFloatValue(renderPipelineStateValue, &rasterizerState.m_DepthBiasClamp);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("SlopeScaledDepthBias"))
    {
        renderStateBlockCompilationError = InterpretFloatValue(renderPipelineStateValue, &rasterizerState.m_SlopeScaledDepthBias);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("FillMode"))
    {
        renderStateBlockCompilationError = InterpretFillMode(renderPipelineStateValue, &rasterizerState.m_FillMode);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("CullMode"))
    {
        renderStateBlockCompilationError = InterpretCullMode(renderPipelineStateValue, &rasterizerState.m_CullMode);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("DepthComparisonFunc"))
    {
        renderStateBlockCompilationError = InterpretComparisonFunc(renderPipelineStateValue, &depthStencilState.m_DepthComparisonFunc);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("FrontFaceStencilFailOp"))
    {
        renderStateBlockCompilationError = InterpretStencilOperation(renderPipelineStateValue, &depthStencilState.m_FrontFaceStencilFailOp);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("FrontFaceStencilDepthFailOp"))
    {
        renderStateBlockCompilationError = InterpretStencilOperation(renderPipelineStateValue, &depthStencilState.m_FrontFaceStencilDepthFailOp);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("FrontFaceStencilPassOp"))
    {
        renderStateBlockCompilationError = InterpretStencilOperation(renderPipelineStateValue, &depthStencilState.m_FrontFaceStencilPassOp);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("FrontFaceStencilComparisonFunc"))
    {
        renderStateBlockCompilationError = InterpretComparisonFunc(renderPipelineStateValue, &depthStencilState.m_FrontFaceStencilComparisonFunc);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("BackFaceStencilFailOp"))
    {
        renderStateBlockCompilationError = InterpretStencilOperation(renderPipelineStateValue, &depthStencilState.m_BackFaceStencilFailOp);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("BackFaceStencilDepthFailOp"))
    {
        renderStateBlockCompilationError = InterpretStencilOperation(renderPipelineStateValue, &depthStencilState.m_BackFaceStencilDepthFailOp);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("BackFaceStencilPassOp"))
    {
        renderStateBlockCompilationError = InterpretStencilOperation(renderPipelineStateValue, &depthStencilState.m_BackFaceStencilPassOp);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("BackFaceStencilComparisonFunc"))
    {
        renderStateBlockCompilationError = InterpretComparisonFunc(renderPipelineStateValue, &depthStencilState.m_BackFaceStencilComparisonFunc);
    }

    return renderStateBlockCompilationError;
}

extern StringA g_ShaderOptionString[uint32_t(ShaderOption::Count)];

void GetEffectiveRenderPipelineBlockForShaderKey(
        const ShaderKey& shaderKey,
        const Array<ShaderOption>& everyPossibleShaderOption,
        const StringA& renderPipelineBlockSource,
        StringA& effectiveRenderPipelineBlockSource)
{
    Array<D3D_SHADER_MACRO> shaderOptionDefines;
    shaderOptionDefines.Reserve(everyPossibleShaderOption.Size());

    for (ShaderOption shaderOption : everyPossibleShaderOption)
    {
        uint32_t valueForShaderOption = shaderKey.GetShaderOptionValue(shaderOption);

        D3D_SHADER_MACRO shaderDefine;
        shaderDefine.Name = g_ShaderOptionString[uint32_t(shaderOption)].GetBuffer();

        char* buf = new char[8];
        sprintf_s(buf, 8, "%u", valueForShaderOption);

        shaderDefine.Definition = buf;

        shaderOptionDefines.Add(shaderDefine);
    }

    D3D_SHADER_MACRO nullShaderDefine = { nullptr, nullptr };
    shaderOptionDefines.Add(nullShaderDefine);

    ID3DBlob* preprocessedOutput = nullptr;

    D3DPreprocess(
            &renderPipelineBlockSource[0],
            renderPipelineBlockSource.Size(),
            "dummyName",
            &shaderOptionDefines[0],
            nullptr,
            &preprocessedOutput,
            nullptr);

    uint32_t idx = 0;
    while (shaderOptionDefines[idx].Name != nullptr)
    {
        delete[] shaderOptionDefines[idx++].Definition;
    }

    if (preprocessedOutput != nullptr)
    {
        // D3DPreprocess inserts a null character at the end of the preprocess string, and appends the initial string afterwards.
        // Let's remove the initial string.
        size_t preprocessedOutputSize = strlen((char*)preprocessedOutput->GetBufferPointer());

        if (preprocessedOutputSize > 0)
        {
            effectiveRenderPipelineBlockSource.Resize(preprocessedOutputSize);

            memcpy(&effectiveRenderPipelineBlockSource[0], preprocessedOutput->GetBufferPointer(), preprocessedOutputSize);
        }

        preprocessedOutput->Release();

        if (preprocessedOutputSize > 0)
        {
            // D3DPreprocess also appends a bunch of #line directive, but we're not interested in those, so let's remove them.
            size_t lineDirectiveIndex = 0;
            while (true)
            {
                lineDirectiveIndex = effectiveRenderPipelineBlockSource.FindIndexOfFirst("#line", lineDirectiveIndex);

                if (lineDirectiveIndex == effectiveRenderPipelineBlockSource.InvalidIndex)
                {
                    break;
                }

                size_t newLineIndex = effectiveRenderPipelineBlockSource.FindIndexOfFirst('\n', lineDirectiveIndex);

                // +1 to also account for the newline
                size_t numCharactersToRemove = ((newLineIndex - lineDirectiveIndex) + 1);
                effectiveRenderPipelineBlockSource.Erase(lineDirectiveIndex, numCharactersToRemove);
            }
        }
    }
}

SHIPYARD_API RenderStateBlockCompilationError CompileRenderStateBlock(
        const ShaderKey& shaderKey,
        const Array<ShaderOption>& everyPossibleShaderOption,
        const StringA& renderPipelineBlockSource,
        RasterizerState& rasterizerState,
        DepthStencilState& depthhStencilState)
{
    if (renderPipelineBlockSource.IsEmpty())
    {
        return RenderStateBlockCompilationError::NoError;
    }

    // We first have to remove parts of the render state block that aren't defined.
    StringA effectiveRenderPipelineBlockSource;

    GetEffectiveRenderPipelineBlockForShaderKey(shaderKey, everyPossibleShaderOption, renderPipelineBlockSource, effectiveRenderPipelineBlockSource);

    StringA renderPipelineStateOption;
    StringA renderPipelineStateValue;
    renderPipelineStateOption.Reserve(256);
    renderPipelineStateValue.Reserve(256);

    bool processOption = true;

    for (size_t i = 0; i < effectiveRenderPipelineBlockSource.Size(); i++)
    {
        char c = effectiveRenderPipelineBlockSource[i];

        if (c == ';')
        {
            RenderStateBlockCompilationError renderStateBlockCompilationError = InterpretRenderPipelineStateOption(
                    renderPipelineStateOption,
                    renderPipelineStateValue,
                    rasterizerState,
                    depthhStencilState);

            if (renderStateBlockCompilationError != RenderStateBlockCompilationError::NoError)
            {
                return renderStateBlockCompilationError;
            }

            renderPipelineStateOption.Resize(0);
            renderPipelineStateValue.Resize(0);
            processOption = true;
        }
        else if (c == '=')
        {
            processOption = false;
        }
        else
        {
            if (!isalnum(c) && c != '-' && c != '.')
            {
                continue;
            }

            if (processOption)
            {
                renderPipelineStateOption += c;
            }
            else
            {
                renderPipelineStateValue += c;
            }
        }
    }

    return RenderStateBlockCompilationError::NoError;
}

}