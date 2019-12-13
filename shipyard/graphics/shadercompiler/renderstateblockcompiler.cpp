#include <graphics/graphicsprecomp.h>

#include <graphics/shadercompiler/renderstateblockcompiler.h>

#include <graphics/shadercompiler/shadercompilerutilities.h>

#include <graphics/graphicscommon.h>
#include <graphics/shader/shaderkey.h>

#include <system/string.h>
#include <system/systemcommon.h>

#include <locale>

namespace Shipyard
{;

shipBool GetArrayIndexIfOptionIsAnArray(const StringA& value, shipUint32* outValue)
{
    size_t indexOfEndingBracket = value.FindIndexOfFirstReverse(']', value.Size() - 1);
    if (indexOfEndingBracket == StringA::InvalidIndex)
    {
        return false;
    }

    size_t indexOfStartingBracket = value.FindIndexOfFirstReverse('[', indexOfEndingBracket);
    if (indexOfStartingBracket == StringA::InvalidIndex)
    {
        return false;
    }

    StringA numberString = value.Substring(indexOfStartingBracket + 1, indexOfEndingBracket - indexOfStartingBracket - 1);

    *outValue = shipUint32(atoi(numberString.GetBuffer()));

    return true;
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

RenderStateBlockCompilationError InterpretBlendFactor(const StringA& value, BlendFactor* outValue)
{
    RenderStateBlockCompilationError renderStateBlockCompilationError = RenderStateBlockCompilationError::NoError;

    if (value.EqualCaseInsensitive("Zero"))
    {
        *outValue = BlendFactor::Zero;
    }
    else if (value.EqualCaseInsensitive("One"))
    {
        *outValue = BlendFactor::One;
    }
    else if (value.EqualCaseInsensitive("SrcColor"))
    {
        *outValue = BlendFactor::SrcColor;
    }
    else if (value.EqualCaseInsensitive("InvSrcColor"))
    {
        *outValue = BlendFactor::InvSrcColor;
    }
    else if (value.EqualCaseInsensitive("SrcAlpha"))
    {
        *outValue = BlendFactor::SrcAlpha;
    }
    else if (value.EqualCaseInsensitive("InvSrcAlpha"))
    {
        *outValue = BlendFactor::InvSrcAlpha;
    }
    else if (value.EqualCaseInsensitive("DestAlpha"))
    {
        *outValue = BlendFactor::DestAlpha;
    }
    else if (value.EqualCaseInsensitive("InvDestAlpha"))
    {
        *outValue = BlendFactor::InvDestAlpha;
    }
    else if (value.EqualCaseInsensitive("DestColor"))
    {
        *outValue = BlendFactor::DestColor;
    }
    else if (value.EqualCaseInsensitive("InvDestColor"))
    {
        *outValue = BlendFactor::InvDestColor;
    }
    else if (value.EqualCaseInsensitive("SrcAlphaSat"))
    {
        *outValue = BlendFactor::SrcAlphaSat;
    }
    else if (value.EqualCaseInsensitive("UserFactor"))
    {
        *outValue = BlendFactor::UserFactor;
    }
    else if (value.EqualCaseInsensitive("InvUserFactor"))
    {
        *outValue = BlendFactor::InvUserFactor;
    }
    else if (value.EqualCaseInsensitive("DualSrcColor"))
    {
        *outValue = BlendFactor::DualSrcColor;
    }
    else if (value.EqualCaseInsensitive("DualInvSrcColor"))
    {
        *outValue = BlendFactor::DualInvSrcColor;
    }
    else if (value.EqualCaseInsensitive("DualSrcAlpha"))
    {
        *outValue = BlendFactor::DualSrcAlpha;
    }
    else if (value.EqualCaseInsensitive("DualInvSrcAlpha"))
    {
        *outValue = BlendFactor::DualInvSrcAlpha;
    }
    else
    {
        renderStateBlockCompilationError = RenderStateBlockCompilationError::InvalidValueTypeForOption;
    }

    return renderStateBlockCompilationError;
}

RenderStateBlockCompilationError InterpretBlendOperator(const StringA& value, BlendOperator* outValue)
{
    RenderStateBlockCompilationError renderStateBlockCompilationError = RenderStateBlockCompilationError::NoError;

    if (value.EqualCaseInsensitive("Add"))
    {
        *outValue = BlendOperator::Add;
    }
    else if (value.EqualCaseInsensitive("Subtract"))
    {
        *outValue = BlendOperator::Subtract;
    }
    else if (value.EqualCaseInsensitive("ReverseSubstract"))
    {
        *outValue = BlendOperator::ReverseSubstract;
    }
    else if (value.EqualCaseInsensitive("Min"))
    {
        *outValue = BlendOperator::Min;
    }
    else if (value.EqualCaseInsensitive("Max"))
    {
        *outValue = BlendOperator::Max;
    }
    else
    {
        renderStateBlockCompilationError = RenderStateBlockCompilationError::InvalidValueTypeForOption;
    }

    return renderStateBlockCompilationError;
}

RenderStateBlockCompilationError InterpretRenderTargetWriteMask(const StringA& value, RenderTargetWriteMask* outValue)
{
    RenderStateBlockCompilationError renderStateBlockCompilationError = RenderStateBlockCompilationError::NoError;

    if (value.Size() > 4)
    {
        renderStateBlockCompilationError = RenderStateBlockCompilationError::InvalidValueTypeForOption;
    }
    else
    {
        RenderTargetWriteMask renderTargetWriteMask = RenderTargetWriteMask::RenderTargetWriteMask_None;

        for (shipUint32 i = 0; i < value.Size(); i++)
        {
            shipChar c = value[i];

            if (tolower(c) == tolower('R'))
            {
                renderTargetWriteMask = RenderTargetWriteMask(renderTargetWriteMask | RenderTargetWriteMask::RenderTargetWriteMask_R);
            }
            else if (tolower(c) == tolower('G'))
            {
                renderTargetWriteMask = RenderTargetWriteMask(renderTargetWriteMask | RenderTargetWriteMask::RenderTargetWriteMask_G);
            }
            else if (tolower(c) == tolower('B'))
            {
                renderTargetWriteMask = RenderTargetWriteMask(renderTargetWriteMask | RenderTargetWriteMask::RenderTargetWriteMask_B);
            }
            else if (tolower(c) == tolower('A'))
            {
                renderTargetWriteMask = RenderTargetWriteMask(renderTargetWriteMask | RenderTargetWriteMask::RenderTargetWriteMask_A);
            }
            else
            {
                return RenderStateBlockCompilationError::InvalidValueTypeForOption;
            }
        }

        *outValue = renderTargetWriteMask;
    }

    return renderStateBlockCompilationError;
}

shipBool InterpretStateBlockOption(
        const StringA& renderPipelineStateOption,
        const StringA& renderPipelineStateValue,
        RenderStateBlock& renderStateBlock,
        RenderStateBlockCompilationError& renderStateBlockCompilationError)
{
    if (renderPipelineStateOption == "")
    {
        renderStateBlockCompilationError = RenderStateBlockCompilationError::MissingOption;
        return false;
    }

    if (renderPipelineStateValue == "")
    {
        renderStateBlockCompilationError = RenderStateBlockCompilationError::MissingValueForOption;
        return false;
    }

    RasterizerState& rasterizerState = renderStateBlock.rasterizerState;
    DepthStencilState& depthStencilState = renderStateBlock.depthStencilState;
    BlendState& blendState = renderStateBlock.blendState;
    RenderTargetBlendState* pRenderTargetBlendState = blendState.renderTargetBlendStates;

    renderStateBlockCompilationError = RenderStateBlockCompilationError::NoError;

    shipUint32 arrayIndex = 0;
    shipBool validOptionValue = true;

    if (GetArrayIndexIfOptionIsAnArray(renderPipelineStateOption, &arrayIndex))
    {
        if (renderPipelineStateOption.FindIndexOfFirstCaseInsensitive("BlendEnable", 0) == 0)
        {
            validOptionValue = InterpretBooleanValue(renderPipelineStateValue, &pRenderTargetBlendState[arrayIndex].m_BlendEnable);
        }
        else if (renderPipelineStateOption.FindIndexOfFirstCaseInsensitive("SourceBlend", 0) == 0)
        {
            renderStateBlockCompilationError = InterpretBlendFactor(renderPipelineStateValue, &pRenderTargetBlendState[arrayIndex].m_SourceBlend);
        }
        else if (renderPipelineStateOption.FindIndexOfFirstCaseInsensitive("DestBlend", 0) == 0)
        {
            renderStateBlockCompilationError = InterpretBlendFactor(renderPipelineStateValue, &pRenderTargetBlendState[arrayIndex].m_DestBlend);
        }
        else if (renderPipelineStateOption.FindIndexOfFirstCaseInsensitive("BlendOperator", 0) == 0)
        {
            renderStateBlockCompilationError = InterpretBlendOperator(renderPipelineStateValue, &pRenderTargetBlendState[arrayIndex].m_BlendOperator);
        }
        else if (renderPipelineStateOption.FindIndexOfFirstCaseInsensitive("SourceAlphaBlend", 0) == 0)
        {
            renderStateBlockCompilationError = InterpretBlendFactor(renderPipelineStateValue, &pRenderTargetBlendState[arrayIndex].m_SourceAlphaBlend);
        }
        else if (renderPipelineStateOption.FindIndexOfFirstCaseInsensitive("DestAlphaBlend", 0) == 0)
        {
            renderStateBlockCompilationError = InterpretBlendFactor(renderPipelineStateValue, &pRenderTargetBlendState[arrayIndex].m_DestAlphaBlend);
        }
        else if (renderPipelineStateOption.FindIndexOfFirstCaseInsensitive("AlphaBlendOperator", 0) == 0)
        {
            renderStateBlockCompilationError = InterpretBlendOperator(renderPipelineStateValue, &pRenderTargetBlendState[arrayIndex].m_AlphaBlendOperator);
        }
        else if (renderPipelineStateOption.FindIndexOfFirstCaseInsensitive("RenderTargetWriteMask", 0) == 0)
        {
            renderStateBlockCompilationError = InterpretRenderTargetWriteMask(renderPipelineStateValue, &pRenderTargetBlendState[arrayIndex].m_RenderTargetWriteMask);
        }
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("IsFrontCounterClockwise"))
    {
        validOptionValue = InterpretBooleanValue(renderPipelineStateValue, &rasterizerState.m_IsFrontCounterClockwise);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("DepthClipEnable"))
    {
        validOptionValue = InterpretBooleanValue(renderPipelineStateValue, &rasterizerState.m_DepthClipEnable);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("ScissorEnable"))
    {
        validOptionValue = InterpretBooleanValue(renderPipelineStateValue, &rasterizerState.m_ScissorEnable);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("MultisampleEnable"))
    {
        validOptionValue = InterpretBooleanValue(renderPipelineStateValue, &rasterizerState.m_MultisampleEnable);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("AntialiasedLineEnable"))
    {
        validOptionValue = InterpretBooleanValue(renderPipelineStateValue, &rasterizerState.m_AntialiasedLineEnable);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("DepthEnable"))
    {
        validOptionValue = InterpretBooleanValue(renderPipelineStateValue, &depthStencilState.m_DepthEnable);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("EnableDepthWrite"))
    {
        validOptionValue = InterpretBooleanValue(renderPipelineStateValue, &depthStencilState.m_EnableDepthWrite);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("StencilEnable"))
    {
        validOptionValue = InterpretBooleanValue(renderPipelineStateValue, &depthStencilState.m_StencilEnable);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("DepthBias"))
    {
        validOptionValue = InterpretIntegerValue(renderPipelineStateValue, &rasterizerState.m_DepthBias);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("StencilReadMask"))
    {
        validOptionValue = InterpretIntegerValue(renderPipelineStateValue, &depthStencilState.m_StencilReadMask);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("StencilWriteMask"))
    {
        validOptionValue = InterpretIntegerValue(renderPipelineStateValue, &depthStencilState.m_StencilWriteMask);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("DepthBiasClamp"))
    {
        validOptionValue = InterpretFloatValue(renderPipelineStateValue, &rasterizerState.m_DepthBiasClamp);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("SlopeScaledDepthBias"))
    {
        validOptionValue = InterpretFloatValue(renderPipelineStateValue, &rasterizerState.m_SlopeScaledDepthBias);
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
        validOptionValue = InterpretComparisonFunc(renderPipelineStateValue, &depthStencilState.m_DepthComparisonFunc);
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
    validOptionValue = InterpretComparisonFunc(renderPipelineStateValue, &depthStencilState.m_FrontFaceStencilComparisonFunc);
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
    validOptionValue = InterpretComparisonFunc(renderPipelineStateValue, &depthStencilState.m_BackFaceStencilComparisonFunc);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("RedBlendUserFactor"))
    {
        validOptionValue = InterpretFloatValue(renderPipelineStateValue, &blendState.m_RedBlendUserFactor);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("GreenBlendUserFactor"))
    {
        validOptionValue = InterpretFloatValue(renderPipelineStateValue, &blendState.m_GreenBlendUserFactor);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("BlueBlendUserFactor"))
    {
        validOptionValue = InterpretFloatValue(renderPipelineStateValue, &blendState.m_BlueBlendUserFactor);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("AlphaBlendUserFactor"))
    {
        validOptionValue = InterpretFloatValue(renderPipelineStateValue, &blendState.m_AlphaBlendUserFactor);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("AlphaToCoverageEnable"))
    {
        validOptionValue = InterpretBooleanValue(renderPipelineStateValue, &blendState.m_AlphaToCoverageEnable);
    }
    else if (renderPipelineStateOption.EqualCaseInsensitive("IndependentBlendEnable"))
    {
        validOptionValue = InterpretBooleanValue(renderPipelineStateValue, &blendState.m_IndependentBlendEnable);
    }
    else
    {
        renderStateBlockCompilationError = RenderStateBlockCompilationError::UnrecognizedOption;
    }

    if (!validOptionValue)
    {
        renderStateBlockCompilationError = RenderStateBlockCompilationError::InvalidValueTypeForOption;
    }

    return (renderStateBlockCompilationError == RenderStateBlockCompilationError::NoError);
}

SHIPYARD_GRAPHICS_API RenderStateBlockCompilationError CompileRenderStateBlock(
        const ShaderKey& shaderKey,
        const Array<ShaderOption>& everyPossibleShaderOption,
        const StringA& renderPipelineBlockSource,
        RenderStateBlock& renderStateBlock)
{
    if (renderPipelineBlockSource.IsEmpty())
    {
        return RenderStateBlockCompilationError::NoError;
    }

    // We first have to remove parts of the render state block that aren't defined.
    StringA effectiveRenderPipelineBlockSource;

    GetEffectiveSourceForShaderKey(shaderKey, everyPossibleShaderOption, renderPipelineBlockSource, effectiveRenderPipelineBlockSource);

    RenderStateBlockCompilationError renderStateBlockCompilationError;
    CompileStateBlock(effectiveRenderPipelineBlockSource, renderStateBlock, renderStateBlockCompilationError, &InterpretStateBlockOption);

    return renderStateBlockCompilationError;
}

}