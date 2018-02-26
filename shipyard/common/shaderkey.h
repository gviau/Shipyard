#pragma once

#include <system/array.h>
#include <system/platform.h>

#include <cinttypes>

namespace Shipyard
{
    enum class ShaderFamily : uint8_t;
    enum class ShaderOption : uint32_t;

    class ShaderCompiler;

    class SHIPYARD_API ShaderKey
    {
        friend class ShaderCompiler;

    public:
        typedef uint32_t RawShaderKeyType;
        ShaderKey();

        void SetShaderFamily(ShaderFamily shaderFamily);
        void SetShaderOption(ShaderOption shaderOption, uint32_t value);

        ShaderFamily GetShaderFamily() const;
        uint32_t GetShaderOptionValue(ShaderOption shaderOption) const;

        RawShaderKeyType GetRawShaderKey() const;

        static void InitializeShaderKeyGroups();

        static void GetShaderKeyOptionsForShaderFamily(ShaderFamily shaderFamily, Array<ShaderOption>& shaderOptions);

        // Required to be used in a map (ShaderHandlerManager)
        bool operator< (const ShaderKey& rhs) const { return m_RawShaderKey < rhs.m_RawShaderKey; }

        static const RawShaderKeyType ms_ShaderFamilyMask = 0xFF;
        static const RawShaderKeyType ms_ShaderOptionMask = 0xFFFFFF;

        static const RawShaderKeyType ms_ShaderFamilyShift = 0x00;
        static const RawShaderKeyType ms_ShaderOptionShift = 0x08;

    private:
        RawShaderKeyType m_RawShaderKey;
    };
}

#define SET_SHADER_OPTION(shaderKey, shaderOption, value) shaderKey.SetShaderOption(Shipyard::ShaderOption::ShaderOption_##shaderOption, value)
#define GET_SHADER_OPTION_VALUE(shaderKey, shaderOption) shaderKey.GetShaderOptionValue(Shipyard::ShaderOption::ShaderOption_##shaderOption)