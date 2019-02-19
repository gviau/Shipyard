#include <common/common.h>

namespace Shipyard
{;

void RenderStateBlockStateOverride::ApplyOverridenValues(RenderStateBlock& renderStateBlock) const
{
#define SET_OVERRIDEN_RASTERIZER_STATE(state) if (m_OverridenState.IsBitSet(RenderStateBlockState::RenderStateBlockState_##state)) renderStateBlock.rasterizerState.m_##state = m_RenderStateBlockOverride.rasterizerState.m_##state;
#define SET_OVERRIDEN_DEPTHSTENCIL_STATE(state) if (m_OverridenState.IsBitSet(RenderStateBlockState::RenderStateBlockState_##state)) renderStateBlock.depthStencilState.m_##state = m_RenderStateBlockOverride.depthStencilState.m_##state;

    SET_OVERRIDEN_RASTERIZER_STATE(DepthBias);
    SET_OVERRIDEN_RASTERIZER_STATE(DepthBiasClamp);
    SET_OVERRIDEN_RASTERIZER_STATE(SlopeScaledDepthBias);
    SET_OVERRIDEN_RASTERIZER_STATE(FillMode);
    SET_OVERRIDEN_RASTERIZER_STATE(CullMode);
    SET_OVERRIDEN_RASTERIZER_STATE(IsFrontCounterClockwise);
    SET_OVERRIDEN_RASTERIZER_STATE(DepthClipEnable);
    SET_OVERRIDEN_RASTERIZER_STATE(ScissorEnable);
    SET_OVERRIDEN_RASTERIZER_STATE(MultisampleEnable);
    SET_OVERRIDEN_RASTERIZER_STATE(AntialiasedLineEnable);

    SET_OVERRIDEN_DEPTHSTENCIL_STATE(DepthComparisonFunc);
    SET_OVERRIDEN_DEPTHSTENCIL_STATE(StencilReadMask);
    SET_OVERRIDEN_DEPTHSTENCIL_STATE(StencilWriteMask);
    SET_OVERRIDEN_DEPTHSTENCIL_STATE(FrontFaceStencilFailOp);
    SET_OVERRIDEN_DEPTHSTENCIL_STATE(FrontFaceStencilDepthFailOp);
    SET_OVERRIDEN_DEPTHSTENCIL_STATE(FrontFaceStencilPassOp);
    SET_OVERRIDEN_DEPTHSTENCIL_STATE(FrontFaceStencilComparisonFunc);
    SET_OVERRIDEN_DEPTHSTENCIL_STATE(BackFaceStencilFailOp);
    SET_OVERRIDEN_DEPTHSTENCIL_STATE(BackFaceStencilDepthFailOp);
    SET_OVERRIDEN_DEPTHSTENCIL_STATE(BackFaceStencilPassOp);
    SET_OVERRIDEN_DEPTHSTENCIL_STATE(BackFaceStencilComparisonFunc);
    SET_OVERRIDEN_DEPTHSTENCIL_STATE(DepthEnable);
    SET_OVERRIDEN_DEPTHSTENCIL_STATE(EnableDepthWrite);
    SET_OVERRIDEN_DEPTHSTENCIL_STATE(StencilEnable);

#undef SET_OVERRIDEN_RASTERIZER_STATE
#undef SET_OVERRIDEN_DEPTHSTENCIL_STATE
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

}