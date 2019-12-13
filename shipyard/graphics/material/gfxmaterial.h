#pragma once

#include <system/platform.h>

#include <graphics/shader/shaderfamilies.h>
#include <graphics/shader/shaderinputprovider.h>
#include <graphics/shader/shaderkey.h>

#include <graphics/graphicstypes.h>

namespace Shipyard
{
    enum class GfxMaterialTextureType
    {
        AlbedoMap,
        NormalMap,
        MetlanessMap,
        RoughnessMap,

        Count
    };

    struct GFXMaterialShaderInputProvider : public BaseShaderInputProvider<GFXMaterialShaderInputProvider>
    {
        GFXTexture2DHandle AlbedoMapHandle;
        GFXTexture2DHandle NormalMapHandle;
        GFXTexture2DHandle MetalnessMapHandle;
        GFXTexture2DHandle RoughnessMapHandle;
    };

    class SHIPYARD_GRAPHICS_API GFXMaterial
    {
    public:
        GFXMaterial();
        ~GFXMaterial();

        shipBool Create(ShaderFamily shaderFamily, GFXTexture2DHandle materialTextures[shipUint32(GfxMaterialTextureType::Count)]);

        ShaderFamily GetShaderFamily() const;
        const GFXMaterialShaderInputProvider& GetGfxMaterialShaderInputProvider() const;

    private:
        ShaderFamily m_ShaderFamily;
        GFXMaterialShaderInputProvider m_GfxMaterialShaderInputProvider;
    };
}