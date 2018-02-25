#pragma once

#include <system/array.h>
#include <system/platform.h>

#include <cinttypes>

namespace Shipyard
{
    enum class ShaderFamily : uint8_t;
    enum class ShaderOption : uint32_t;

    class SHIPYARD_API ShaderKey
    {
    public:
        ShaderKey();

        void SetShaderFamily(ShaderFamily shaderFamily);
        void SetShaderOption(ShaderOption shaderOption, uint32_t value);

        ShaderFamily GetShaderFamily() const;
        uint32_t GetShaderOptionValue(ShaderOption shaderOption) const;

        uint32_t GetRawShaderKey() const;

        static void InitializeShaderKeyGroups();

        static void GetShaderKeyOptionsForShaderFamily(ShaderFamily shaderFamily, Array<ShaderOption>& shaderOptions);

    private:
        static const uint32_t ms_ShaderFamilyMask = 0xFF;
        static const uint32_t ms_ShaderOptionMask = 0xFFFFFF;

        static const uint32_t ms_ShaderFamilyShift = 0x00;
        static const uint32_t ms_ShaderOptionShift = 0x08;
        uint32_t m_RawShaderKey;
    };
}