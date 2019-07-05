#pragma once

#include <system/array.h>
#include <system/platform.h>

namespace Shipyard
{
    enum class ShaderFamily : shipUint8;
    enum class ShaderOption : shipUint32;

    class ShaderCompiler;

    class SHIPYARD_API ShaderKey
    {
        friend class ShaderCompiler;

    public:
        typedef shipUint32 RawShaderKeyType;
        ShaderKey();

        void SetShaderFamily(ShaderFamily shaderFamily);
        void SetShaderOption(ShaderOption shaderOption, shipUint32 value);

        ShaderFamily GetShaderFamily() const;
        shipUint32 GetShaderOptionValue(ShaderOption shaderOption) const;

        RawShaderKeyType GetRawShaderKey() const;

        static void InitializeShaderKeyGroups();

        static void GetShaderKeyOptionsForShaderFamily(ShaderFamily shaderFamily, Array<ShaderOption>& shaderOptions);

        // Required to be used in a map (ShaderHandlerManager)
        shipBool operator< (const ShaderKey& rhs) const { return m_RawShaderKey < rhs.m_RawShaderKey; }
        shipBool operator== (const ShaderKey& rhs) const { return m_RawShaderKey == rhs.m_RawShaderKey; }
        shipBool operator!= (const ShaderKey& rhs) const { return m_RawShaderKey != rhs.m_RawShaderKey; }

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