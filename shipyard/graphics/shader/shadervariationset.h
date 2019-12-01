#pragma once

#include <graphics/shader/shaderfamilies.h>
#include <graphics/shader/shadervariationsetmanager.h>

namespace Shipyard
{
#define SHIP_DECLARE_SHADER_VARIATION_SET_BEGIN(shaderFamily) \
    class ShaderVariationSet_##shaderFamily \
    { \
        public: \
            ShaderVariationSet_##shaderFamily() \
            { \
                ShaderVariationSetManager& shaderVariationSetManager = GetShaderVariationSetManager(); \
                shaderVariationSetManager.StartShaderVariationSetForShaderFamily(ShaderFamily::##shaderFamily);

#define SHIP_DECLARE_SHADER_VARIATION_SET_END(shaderFamily) \
                shaderVariationSetManager.EndShaderVariationSetForShaderFamily(ShaderFamily::##shaderFamily); \
            } \
    }; static ShaderVariationSet_##shaderFamily gs_ShaderVariationSet_##shaderFamily;
}