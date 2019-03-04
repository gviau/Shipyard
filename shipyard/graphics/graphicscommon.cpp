#include <graphics/graphicscommon.h>

namespace Shipyard
{;

bool IsDepthStencilFormat(GfxFormat format)
{
    switch (format)
    {
    case GfxFormat::D16_UNORM:
    case GfxFormat::D24_UNORM_S8_UINT:
    case GfxFormat::D32_FLOAT:
    case GfxFormat::D32_FLOAT_S8X24_UINT:
        return true;

    default:
        return false;
    }
}

enum RenderStateBlockState : uint8_t
{
    RenderStateBlockState_DepthBias,
    RenderStateBlockState_DepthBiasClamp,
    RenderStateBlockState_SlopeScaledDepthBias,

    RenderStateBlockState_FillMode,
    RenderStateBlockState_CullMode,

    RenderStateBlockState_IsFrontCounterClockwise,
    RenderStateBlockState_DepthClipEnable,
    RenderStateBlockState_ScissorEnable,
    RenderStateBlockState_MultisampleEnable,
    RenderStateBlockState_AntialiasedLineEnable,

    RenderStateBlockState_DepthComparisonFunc,
    RenderStateBlockState_StencilReadMask,
    RenderStateBlockState_StencilWriteMask,

    RenderStateBlockState_FrontFaceStencilFailOp,
    RenderStateBlockState_FrontFaceStencilDepthFailOp,
    RenderStateBlockState_FrontFaceStencilPassOp,
    RenderStateBlockState_FrontFaceStencilComparisonFunc,

    RenderStateBlockState_BackFaceStencilFailOp,
    RenderStateBlockState_BackFaceStencilDepthFailOp,
    RenderStateBlockState_BackFaceStencilPassOp,
    RenderStateBlockState_BackFaceStencilComparisonFunc,

    RenderStateBlockState_DepthEnable,
    RenderStateBlockState_EnableDepthWrite,
    RenderStateBlockState_StencilEnable,

    RenderStateBlockState_0_BlendEnable,
    RenderStateBlockState_0_SourceBlend,
    RenderStateBlockState_0_DestBlend,
    RenderStateBlockState_0_BlendOperator,
    RenderStateBlockState_0_SourceAlphaBlend,
    RenderStateBlockState_0_DestAlphaBlend,
    RenderStateBlockState_0_AlphaBlendOperator,
    RenderStateBlockState_0_RenderTargetWriteMask,

    RenderStateBlockState_1_BlendEnable,
    RenderStateBlockState_1_SourceBlend,
    RenderStateBlockState_1_DestBlend,
    RenderStateBlockState_1_BlendOperator,
    RenderStateBlockState_1_SourceAlphaBlend,
    RenderStateBlockState_1_DestAlphaBlend,
    RenderStateBlockState_1_AlphaBlendOperator,
    RenderStateBlockState_1_RenderTargetWriteMask,

    RenderStateBlockState_2_BlendEnable,
    RenderStateBlockState_2_SourceBlend,
    RenderStateBlockState_2_DestBlend,
    RenderStateBlockState_2_BlendOperator,
    RenderStateBlockState_2_SourceAlphaBlend,
    RenderStateBlockState_2_DestAlphaBlend,
    RenderStateBlockState_2_AlphaBlendOperator,
    RenderStateBlockState_2_RenderTargetWriteMask,

    RenderStateBlockState_3_BlendEnable,
    RenderStateBlockState_3_SourceBlend,
    RenderStateBlockState_3_DestBlend,
    RenderStateBlockState_3_BlendOperator,
    RenderStateBlockState_3_SourceAlphaBlend,
    RenderStateBlockState_3_DestAlphaBlend,
    RenderStateBlockState_3_AlphaBlendOperator,
    RenderStateBlockState_3_RenderTargetWriteMask,

    RenderStateBlockState_4_BlendEnable,
    RenderStateBlockState_4_SourceBlend,
    RenderStateBlockState_4_DestBlend,
    RenderStateBlockState_4_BlendOperator,
    RenderStateBlockState_4_SourceAlphaBlend,
    RenderStateBlockState_4_DestAlphaBlend,
    RenderStateBlockState_4_AlphaBlendOperator,
    RenderStateBlockState_4_RenderTargetWriteMask,

    RenderStateBlockState_5_BlendEnable,
    RenderStateBlockState_5_SourceBlend,
    RenderStateBlockState_5_DestBlend,
    RenderStateBlockState_5_BlendOperator,
    RenderStateBlockState_5_SourceAlphaBlend,
    RenderStateBlockState_5_DestAlphaBlend,
    RenderStateBlockState_5_AlphaBlendOperator,
    RenderStateBlockState_5_RenderTargetWriteMask,

    RenderStateBlockState_6_BlendEnable,
    RenderStateBlockState_6_SourceBlend,
    RenderStateBlockState_6_DestBlend,
    RenderStateBlockState_6_BlendOperator,
    RenderStateBlockState_6_SourceAlphaBlend,
    RenderStateBlockState_6_DestAlphaBlend,
    RenderStateBlockState_6_AlphaBlendOperator,
    RenderStateBlockState_6_RenderTargetWriteMask,

    RenderStateBlockState_7_BlendEnable,
    RenderStateBlockState_7_SourceBlend,
    RenderStateBlockState_7_DestBlend,
    RenderStateBlockState_7_BlendOperator,
    RenderStateBlockState_7_SourceAlphaBlend,
    RenderStateBlockState_7_DestAlphaBlend,
    RenderStateBlockState_7_AlphaBlendOperator,
    RenderStateBlockState_7_RenderTargetWriteMask,

    RenderStateBlockState_RedBlendUserFactor,
    RenderStateBlockState_GreenBlendUserFactor,
    RenderStateBlockState_BlueBlendUserFactor,
    RenderStateBlockState_AlphaBlendUserFactor,

    RenderStateBlockState_AlphaToCoverageEnable,
    RenderStateBlockState_IndependentBlendEnable,

    RenderStateBlockState_Count
};

void RenderStateBlockStateOverride::ApplyOverridenValues(RenderStateBlock& renderStateBlock) const
{
    if (m_OverridenState.IsClear())
    {
        return;
    }

#define SET_OVERRIDDEN_RASTERIZER_STATE(state) if (m_OverridenState.IsBitSet(RenderStateBlockState::RenderStateBlockState_##state)) renderStateBlock.rasterizerState.m_##state = m_RenderStateBlockOverride.rasterizerState.m_##state;
#define SET_OVERRIDDEN_DEPTHSTENCIL_STATE(state) if (m_OverridenState.IsBitSet(RenderStateBlockState::RenderStateBlockState_##state)) renderStateBlock.depthStencilState.m_##state = m_RenderStateBlockOverride.depthStencilState.m_##state;
#define SET_OVERRIDDEN_BLENDSTATE_STATE(state) if (m_OverridenState.IsBitSet(RenderStateBlockState::RenderStateBlockState_##state)) renderStateBlock.blendState.m_##state = m_RenderStateBlockOverride.blendState.m_##state;
#define SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_0(state) if (m_OverridenState.IsBitSet(RenderStateBlockState::RenderStateBlockState_0_##state)) renderStateBlock.blendState.renderTargetBlendStates[0].m_##state = m_RenderStateBlockOverride.blendState.renderTargetBlendStates[0].m_##state;
#define SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_1(state) if (m_OverridenState.IsBitSet(RenderStateBlockState::RenderStateBlockState_1_##state)) renderStateBlock.blendState.renderTargetBlendStates[1].m_##state = m_RenderStateBlockOverride.blendState.renderTargetBlendStates[1].m_##state;
#define SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_2(state) if (m_OverridenState.IsBitSet(RenderStateBlockState::RenderStateBlockState_2_##state)) renderStateBlock.blendState.renderTargetBlendStates[2].m_##state = m_RenderStateBlockOverride.blendState.renderTargetBlendStates[2].m_##state;
#define SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_3(state) if (m_OverridenState.IsBitSet(RenderStateBlockState::RenderStateBlockState_3_##state)) renderStateBlock.blendState.renderTargetBlendStates[3].m_##state = m_RenderStateBlockOverride.blendState.renderTargetBlendStates[3].m_##state;
#define SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_4(state) if (m_OverridenState.IsBitSet(RenderStateBlockState::RenderStateBlockState_4_##state)) renderStateBlock.blendState.renderTargetBlendStates[4].m_##state = m_RenderStateBlockOverride.blendState.renderTargetBlendStates[4].m_##state;
#define SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_5(state) if (m_OverridenState.IsBitSet(RenderStateBlockState::RenderStateBlockState_5_##state)) renderStateBlock.blendState.renderTargetBlendStates[5].m_##state = m_RenderStateBlockOverride.blendState.renderTargetBlendStates[5].m_##state;
#define SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_6(state) if (m_OverridenState.IsBitSet(RenderStateBlockState::RenderStateBlockState_6_##state)) renderStateBlock.blendState.renderTargetBlendStates[6].m_##state = m_RenderStateBlockOverride.blendState.renderTargetBlendStates[6].m_##state;
#define SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_7(state) if (m_OverridenState.IsBitSet(RenderStateBlockState::RenderStateBlockState_7_##state)) renderStateBlock.blendState.renderTargetBlendStates[7].m_##state = m_RenderStateBlockOverride.blendState.renderTargetBlendStates[7].m_##state;

    SET_OVERRIDDEN_RASTERIZER_STATE(DepthBias);
    SET_OVERRIDDEN_RASTERIZER_STATE(DepthBiasClamp);
    SET_OVERRIDDEN_RASTERIZER_STATE(SlopeScaledDepthBias);
    SET_OVERRIDDEN_RASTERIZER_STATE(FillMode);
    SET_OVERRIDDEN_RASTERIZER_STATE(CullMode);
    SET_OVERRIDDEN_RASTERIZER_STATE(IsFrontCounterClockwise);
    SET_OVERRIDDEN_RASTERIZER_STATE(DepthClipEnable);
    SET_OVERRIDDEN_RASTERIZER_STATE(ScissorEnable);
    SET_OVERRIDDEN_RASTERIZER_STATE(MultisampleEnable);
    SET_OVERRIDDEN_RASTERIZER_STATE(AntialiasedLineEnable);

    SET_OVERRIDDEN_DEPTHSTENCIL_STATE(DepthComparisonFunc);
    SET_OVERRIDDEN_DEPTHSTENCIL_STATE(StencilReadMask);
    SET_OVERRIDDEN_DEPTHSTENCIL_STATE(StencilWriteMask);
    SET_OVERRIDDEN_DEPTHSTENCIL_STATE(FrontFaceStencilFailOp);
    SET_OVERRIDDEN_DEPTHSTENCIL_STATE(FrontFaceStencilDepthFailOp);
    SET_OVERRIDDEN_DEPTHSTENCIL_STATE(FrontFaceStencilPassOp);
    SET_OVERRIDDEN_DEPTHSTENCIL_STATE(FrontFaceStencilComparisonFunc);
    SET_OVERRIDDEN_DEPTHSTENCIL_STATE(BackFaceStencilFailOp);
    SET_OVERRIDDEN_DEPTHSTENCIL_STATE(BackFaceStencilDepthFailOp);
    SET_OVERRIDDEN_DEPTHSTENCIL_STATE(BackFaceStencilPassOp);
    SET_OVERRIDDEN_DEPTHSTENCIL_STATE(BackFaceStencilComparisonFunc);
    SET_OVERRIDDEN_DEPTHSTENCIL_STATE(DepthEnable);
    SET_OVERRIDDEN_DEPTHSTENCIL_STATE(EnableDepthWrite);
    SET_OVERRIDDEN_DEPTHSTENCIL_STATE(StencilEnable);

    SET_OVERRIDDEN_BLENDSTATE_STATE(RedBlendUserFactor);
    SET_OVERRIDDEN_BLENDSTATE_STATE(GreenBlendUserFactor);
    SET_OVERRIDDEN_BLENDSTATE_STATE(BlueBlendUserFactor);
    SET_OVERRIDDEN_BLENDSTATE_STATE(AlphaBlendUserFactor);
    SET_OVERRIDDEN_BLENDSTATE_STATE(AlphaToCoverageEnable);
    SET_OVERRIDDEN_BLENDSTATE_STATE(IndependentBlendEnable);

    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_0(BlendEnable);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_0(SourceBlend);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_0(DestBlend);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_0(BlendOperator);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_0(SourceAlphaBlend);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_0(DestAlphaBlend);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_0(AlphaBlendOperator);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_0(RenderTargetWriteMask);

    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_1(BlendEnable);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_1(SourceBlend);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_1(DestBlend);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_1(BlendOperator);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_1(SourceAlphaBlend);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_1(DestAlphaBlend);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_1(AlphaBlendOperator);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_1(RenderTargetWriteMask);

    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_2(BlendEnable);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_2(SourceBlend);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_2(DestBlend);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_2(BlendOperator);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_2(SourceAlphaBlend);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_2(DestAlphaBlend);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_2(AlphaBlendOperator);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_2(RenderTargetWriteMask);

    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_3(BlendEnable);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_3(SourceBlend);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_3(DestBlend);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_3(BlendOperator);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_3(SourceAlphaBlend);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_3(DestAlphaBlend);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_3(AlphaBlendOperator);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_3(RenderTargetWriteMask);

    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_4(BlendEnable);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_4(SourceBlend);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_4(DestBlend);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_4(BlendOperator);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_4(SourceAlphaBlend);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_4(DestAlphaBlend);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_4(AlphaBlendOperator);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_4(RenderTargetWriteMask);

    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_5(BlendEnable);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_5(SourceBlend);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_5(DestBlend);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_5(BlendOperator);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_5(SourceAlphaBlend);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_5(DestAlphaBlend);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_5(AlphaBlendOperator);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_5(RenderTargetWriteMask);

    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_6(BlendEnable);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_6(SourceBlend);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_6(DestBlend);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_6(BlendOperator);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_6(SourceAlphaBlend);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_6(DestAlphaBlend);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_6(AlphaBlendOperator);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_6(RenderTargetWriteMask);

    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_7(BlendEnable);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_7(SourceBlend);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_7(DestBlend);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_7(BlendOperator);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_7(SourceAlphaBlend);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_7(DestAlphaBlend);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_7(AlphaBlendOperator);
    SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_7(RenderTargetWriteMask);

#undef SET_OVERRIDDEN_RASTERIZER_STATE
#undef SET_OVERRIDDEN_DEPTHSTENCIL_STATE
#undef SET_OVERRIDDEN_BLENDSTATE_STATE
#undef SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_0
#undef SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_1
#undef SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_2
#undef SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_3
#undef SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_4
#undef SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_5
#undef SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_6
#undef SET_OVERRIDDEN_RENDERTARGET_BLENDSTATE_STATE_7
}

void RenderStateBlockStateOverride::OverrideDepthBiasState(int overrideValue)
{
    m_RenderStateBlockOverride.rasterizerState.m_DepthBias = overrideValue;
    m_OverridenState.SetBit(RenderStateBlockState::RenderStateBlockState_DepthBias);
}

void RenderStateBlockStateOverride::OverrideDepthBiasClampState(float overrideValue)
{
    m_RenderStateBlockOverride.rasterizerState.m_DepthBiasClamp = overrideValue;
    m_OverridenState.SetBit(RenderStateBlockState::RenderStateBlockState_DepthBiasClamp);
}

void RenderStateBlockStateOverride::OverrideSlopeScaledDepthBiasState(float overrideValue)
{
    m_RenderStateBlockOverride.rasterizerState.m_SlopeScaledDepthBias = overrideValue;
    m_OverridenState.SetBit(RenderStateBlockState::RenderStateBlockState_SlopeScaledDepthBias);
}

void RenderStateBlockStateOverride::OverrideFillModeState(FillMode overrideValue)
{
    m_RenderStateBlockOverride.rasterizerState.m_FillMode = overrideValue;
    m_OverridenState.SetBit(RenderStateBlockState::RenderStateBlockState_FillMode);
}

void RenderStateBlockStateOverride::OverrideCullModeState(CullMode overrideValue)
{
    m_RenderStateBlockOverride.rasterizerState.m_CullMode = overrideValue;
    m_OverridenState.SetBit(RenderStateBlockState::RenderStateBlockState_CullMode);
}

void RenderStateBlockStateOverride::OverrideIsFrontCounterClockwiseState(bool overrideValue)
{
    m_RenderStateBlockOverride.rasterizerState.m_IsFrontCounterClockwise = overrideValue;
    m_OverridenState.SetBit(RenderStateBlockState::RenderStateBlockState_IsFrontCounterClockwise);
}

void RenderStateBlockStateOverride::OverrideDepthClipEnableState(bool overrideValue)
{
    m_RenderStateBlockOverride.rasterizerState.m_DepthClipEnable = overrideValue;
    m_OverridenState.SetBit(RenderStateBlockState::RenderStateBlockState_DepthClipEnable);
}

void RenderStateBlockStateOverride::OverrideScissorEnableState(bool overrideValue)
{
    m_RenderStateBlockOverride.rasterizerState.m_ScissorEnable = overrideValue;
    m_OverridenState.SetBit(RenderStateBlockState::RenderStateBlockState_ScissorEnable);
}

void RenderStateBlockStateOverride::OverrideMultisampleEnableState(bool overrideValue)
{
    m_RenderStateBlockOverride.rasterizerState.m_MultisampleEnable = overrideValue;
    m_OverridenState.SetBit(RenderStateBlockState::RenderStateBlockState_MultisampleEnable);
}

void RenderStateBlockStateOverride::OverrideAntialiasedLineEnableState(bool overrideValue)
{
    m_RenderStateBlockOverride.rasterizerState.m_AntialiasedLineEnable = overrideValue;
    m_OverridenState.SetBit(RenderStateBlockState::RenderStateBlockState_AntialiasedLineEnable);
}

void RenderStateBlockStateOverride::OverrideDepthComparisonFuncState(ComparisonFunc overrideValue)
{
    m_RenderStateBlockOverride.depthStencilState.m_DepthComparisonFunc = overrideValue;
    m_OverridenState.SetBit(RenderStateBlockState::RenderStateBlockState_DepthComparisonFunc);
}

void RenderStateBlockStateOverride::OverrideStencilReadMaskState(uint8_t overrideValue)
{
    m_RenderStateBlockOverride.depthStencilState.m_StencilReadMask = overrideValue;
    m_OverridenState.SetBit(RenderStateBlockState::RenderStateBlockState_StencilReadMask);
}

void RenderStateBlockStateOverride::OverrideStencilWriteMaskState(uint8_t overrideValue)
{
    m_RenderStateBlockOverride.depthStencilState.m_StencilWriteMask = overrideValue;
    m_OverridenState.SetBit(RenderStateBlockState::RenderStateBlockState_StencilWriteMask);
}

void RenderStateBlockStateOverride::OverrideFrontFaceStencilFailOpState(StencilOperation overrideValue)
{
    m_RenderStateBlockOverride.depthStencilState.m_FrontFaceStencilFailOp = overrideValue;
    m_OverridenState.SetBit(RenderStateBlockState::RenderStateBlockState_FrontFaceStencilFailOp);
}

void RenderStateBlockStateOverride::OverrideFrontFaceStencilDepthFailOpState(StencilOperation overrideValue)
{
    m_RenderStateBlockOverride.depthStencilState.m_FrontFaceStencilDepthFailOp = overrideValue;
    m_OverridenState.SetBit(RenderStateBlockState::RenderStateBlockState_FrontFaceStencilDepthFailOp);
}

void RenderStateBlockStateOverride::OverrideFrontFaceStencilPassOpState(StencilOperation overrideValue)
{
    m_RenderStateBlockOverride.depthStencilState.m_FrontFaceStencilPassOp = overrideValue;
    m_OverridenState.SetBit(RenderStateBlockState::RenderStateBlockState_FrontFaceStencilPassOp);
}

void RenderStateBlockStateOverride::OverrideFrontFaceStencilComparisonFuncState(ComparisonFunc overrideValue)
{
    m_RenderStateBlockOverride.depthStencilState.m_FrontFaceStencilComparisonFunc = overrideValue;
    m_OverridenState.SetBit(RenderStateBlockState::RenderStateBlockState_FrontFaceStencilComparisonFunc);
}

void RenderStateBlockStateOverride::OverrideBackFaceStencilFailOpState(StencilOperation overrideValue)
{
    m_RenderStateBlockOverride.depthStencilState.m_BackFaceStencilFailOp = overrideValue;
    m_OverridenState.SetBit(RenderStateBlockState::RenderStateBlockState_BackFaceStencilFailOp);
}

void RenderStateBlockStateOverride::OverrideBackFaceStencilDepthFailOpState(StencilOperation overrideValue)
{
    m_RenderStateBlockOverride.depthStencilState.m_BackFaceStencilDepthFailOp = overrideValue;
    m_OverridenState.SetBit(RenderStateBlockState::RenderStateBlockState_BackFaceStencilDepthFailOp);
}

void RenderStateBlockStateOverride::OverrideBackFaceStencilPassOpState(StencilOperation overrideValue)
{
    m_RenderStateBlockOverride.depthStencilState.m_BackFaceStencilPassOp = overrideValue;
    m_OverridenState.SetBit(RenderStateBlockState::RenderStateBlockState_BackFaceStencilPassOp);
}

void RenderStateBlockStateOverride::OverrideBackFaceStencilComparisonFuncState(ComparisonFunc overrideValue)
{
    m_RenderStateBlockOverride.depthStencilState.m_BackFaceStencilComparisonFunc = overrideValue;
    m_OverridenState.SetBit(RenderStateBlockState::RenderStateBlockState_BackFaceStencilComparisonFunc);
}

void RenderStateBlockStateOverride::OverrideDepthEnableState(bool overrideValue)
{
    m_RenderStateBlockOverride.depthStencilState.m_DepthEnable = overrideValue;
    m_OverridenState.SetBit(RenderStateBlockState::RenderStateBlockState_DepthEnable);
}

void RenderStateBlockStateOverride::OverrideEnableDepthWriteState(bool overrideValue)
{
    m_RenderStateBlockOverride.depthStencilState.m_EnableDepthWrite = overrideValue;
    m_OverridenState.SetBit(RenderStateBlockState::RenderStateBlockState_EnableDepthWrite);
}

void RenderStateBlockStateOverride::OverrideStencilEnableState(bool overrideValue)
{
    m_RenderStateBlockOverride.depthStencilState.m_StencilEnable = overrideValue;
    m_OverridenState.SetBit(RenderStateBlockState::RenderStateBlockState_StencilEnable);
}

void RenderStateBlockStateOverride::OverrideRedBlendUserFactor(float overrideValue)
{
    m_RenderStateBlockOverride.blendState.m_RedBlendUserFactor = overrideValue;
    m_OverridenState.SetBit(RenderStateBlockState::RenderStateBlockState_RedBlendUserFactor);
}

void RenderStateBlockStateOverride::OverrideGreenBlendUserFactor(float overrideValue)
{
    m_RenderStateBlockOverride.blendState.m_GreenBlendUserFactor = overrideValue;
    m_OverridenState.SetBit(RenderStateBlockState::RenderStateBlockState_GreenBlendUserFactor);
}

void RenderStateBlockStateOverride::OverrideBlueBlendUserFactor(float overrideValue)
{
    m_RenderStateBlockOverride.blendState.m_BlueBlendUserFactor = overrideValue;
    m_OverridenState.SetBit(RenderStateBlockState::RenderStateBlockState_BlueBlendUserFactor);
}

void RenderStateBlockStateOverride::OverrideAlphaBlendUserFactor(float overrideValue)
{
    m_RenderStateBlockOverride.blendState.m_AlphaBlendUserFactor = overrideValue;
    m_OverridenState.SetBit(RenderStateBlockState::RenderStateBlockState_AlphaBlendUserFactor);
}

void RenderStateBlockStateOverride::OverrideAlphaToCoverageEnable(bool overrideValue)
{
    m_RenderStateBlockOverride.blendState.m_AlphaToCoverageEnable = overrideValue;
    m_OverridenState.SetBit(RenderStateBlockState::RenderStateBlockState_AlphaToCoverageEnable);
}

void RenderStateBlockStateOverride::OverrideIndependentBlendEnable(bool overrideValue)
{
    m_RenderStateBlockOverride.blendState.m_IndependentBlendEnable = overrideValue;
    m_OverridenState.SetBit(RenderStateBlockState::RenderStateBlockState_IndependentBlendEnable);
}

void RenderStateBlockStateOverride::OverrideBlendEnable(bool overrideValue, uint32_t renderTargetIndex)
{
    assert(renderTargetIndex < GfxConstants::GfxConstants_MaxRenderTargetsBound);

    m_RenderStateBlockOverride.blendState.renderTargetBlendStates[renderTargetIndex].m_BlendEnable = overrideValue;
    m_OverridenState.SetBit((RenderStateBlockState_1_BlendEnable - RenderStateBlockState_0_BlendEnable) * renderTargetIndex + RenderStateBlockState_0_BlendEnable);
}

void RenderStateBlockStateOverride::OverrideSourceBlend(BlendFactor overrideValue, uint32_t renderTargetIndex)
{
    assert(renderTargetIndex < GfxConstants::GfxConstants_MaxRenderTargetsBound);

    m_RenderStateBlockOverride.blendState.renderTargetBlendStates[renderTargetIndex].m_SourceBlend = overrideValue;
    m_OverridenState.SetBit((RenderStateBlockState_1_SourceBlend - RenderStateBlockState_0_SourceBlend) * renderTargetIndex + RenderStateBlockState_0_SourceBlend);
}

void RenderStateBlockStateOverride::OverrideDestBlend(BlendFactor overrideValue, uint32_t renderTargetIndex)
{
    assert(renderTargetIndex < GfxConstants::GfxConstants_MaxRenderTargetsBound);

    m_RenderStateBlockOverride.blendState.renderTargetBlendStates[renderTargetIndex].m_DestBlend = overrideValue;
    m_OverridenState.SetBit((RenderStateBlockState_1_DestBlend - RenderStateBlockState_0_DestBlend) * renderTargetIndex + RenderStateBlockState_0_DestBlend);
}

void RenderStateBlockStateOverride::OverrideBlendOperator(BlendOperator overrideValue, uint32_t renderTargetIndex)
{
    assert(renderTargetIndex < GfxConstants::GfxConstants_MaxRenderTargetsBound);

    m_RenderStateBlockOverride.blendState.renderTargetBlendStates[renderTargetIndex].m_BlendOperator = overrideValue;
    m_OverridenState.SetBit((RenderStateBlockState_1_BlendOperator - RenderStateBlockState_0_BlendOperator) * renderTargetIndex + RenderStateBlockState_0_BlendOperator);
}

void RenderStateBlockStateOverride::OverrideSourceAlphaBlend(BlendFactor overrideValue, uint32_t renderTargetIndex)
{
    assert(renderTargetIndex < GfxConstants::GfxConstants_MaxRenderTargetsBound);

    m_RenderStateBlockOverride.blendState.renderTargetBlendStates[renderTargetIndex].m_SourceAlphaBlend = overrideValue;
    m_OverridenState.SetBit((RenderStateBlockState_1_SourceAlphaBlend - RenderStateBlockState_0_SourceAlphaBlend) * renderTargetIndex + RenderStateBlockState_0_SourceAlphaBlend);
}

void RenderStateBlockStateOverride::OverrideDestAlphaBlend(BlendFactor overrideValue, uint32_t renderTargetIndex)
{
    assert(renderTargetIndex < GfxConstants::GfxConstants_MaxRenderTargetsBound);

    m_RenderStateBlockOverride.blendState.renderTargetBlendStates[renderTargetIndex].m_DestAlphaBlend = overrideValue;
    m_OverridenState.SetBit((RenderStateBlockState_1_DestAlphaBlend - RenderStateBlockState_0_DestAlphaBlend) * renderTargetIndex + RenderStateBlockState_0_DestAlphaBlend);
}

void RenderStateBlockStateOverride::OverrideAlphaBlendOperator(BlendOperator overrideValue, uint32_t renderTargetIndex)
{
    assert(renderTargetIndex < GfxConstants::GfxConstants_MaxRenderTargetsBound);

    m_RenderStateBlockOverride.blendState.renderTargetBlendStates[renderTargetIndex].m_AlphaBlendOperator = overrideValue;
    m_OverridenState.SetBit((RenderStateBlockState_1_AlphaBlendOperator - RenderStateBlockState_0_AlphaBlendOperator) * renderTargetIndex + RenderStateBlockState_0_AlphaBlendOperator);
}

void RenderStateBlockStateOverride::OverrideRenderTargetWriteMask(RenderTargetWriteMask overrideValue, uint32_t renderTargetIndex)
{
    assert(renderTargetIndex < GfxConstants::GfxConstants_MaxRenderTargetsBound);

    m_RenderStateBlockOverride.blendState.renderTargetBlendStates[renderTargetIndex].m_RenderTargetWriteMask = overrideValue;
    m_OverridenState.SetBit((RenderStateBlockState_1_RenderTargetWriteMask - RenderStateBlockState_0_RenderTargetWriteMask) * renderTargetIndex + RenderStateBlockState_0_RenderTargetWriteMask);
}

}