#include <graphics/material/gfxmaterial.h>

namespace Shipyard
{;

SHIP_DECLARE_SHADER_INPUT_PROVIDER_BEGIN(GFXMaterialShaderInputProvider, PerInstance)
{
    SHIP_TEXTURE2D_SHADER_INPUT(ShaderInputScalarType::Float4, "AlbedoMap", AlbedoMapHandle);
    SHIP_TEXTURE2D_SHADER_INPUT(ShaderInputScalarType::Float4, "NormalMap", NormalMapHandle);
    SHIP_TEXTURE2D_SHADER_INPUT(ShaderInputScalarType::Float4, "MetalnessMap", MetalnessMapHandle);
    SHIP_TEXTURE2D_SHADER_INPUT(ShaderInputScalarType::Float4, "RoughnessMap", RoughnessMapHandle);
}
SHIP_DECLARE_SHADER_INPUT_PROVIDER_END(GFXMaterialShaderInputProvider)

GFXMaterial::GFXMaterial()
    : m_ShaderFamily(ShaderFamily::Error)
{

}

shipBool GFXMaterial::Create(ShaderFamily shaderFamily, GFXTexture2DHandle materialTextures[shipUint32(GfxMaterialTextureType::Count)])
{
    m_ShaderFamily = shaderFamily;

    m_GfxMaterialShaderInputProvider.AlbedoMapHandle = materialTextures[shipUint32(GfxMaterialTextureType::AlbedoMap)];
    m_GfxMaterialShaderInputProvider.NormalMapHandle = materialTextures[shipUint32(GfxMaterialTextureType::NormalMap)];
    m_GfxMaterialShaderInputProvider.MetalnessMapHandle = materialTextures[shipUint32(GfxMaterialTextureType::MetlanessMap)];
    m_GfxMaterialShaderInputProvider.RoughnessMapHandle = materialTextures[shipUint32(GfxMaterialTextureType::RoughnessMap)];

    return true;
}

ShaderFamily GFXMaterial::GetShaderFamily() const
{
    return m_ShaderFamily;
}

const GFXMaterialShaderInputProvider& GFXMaterial::GetGfxMaterialShaderInputProvider() const
{
    return m_GfxMaterialShaderInputProvider;
}

}