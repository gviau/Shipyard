#include <graphics/shaderkey.h>

#include <graphics/shaderfamilies.h>
#include <graphics/shaderoptions.h>

#include <system/systemcommon.h>

namespace Shipyard
{;

extern uint8_t g_NumBitsForShaderOption[uint32_t(ShaderOption::Count)];

struct ShaderKeyGroupBase
{
    virtual uint32_t GetShiftForShaderOption(ShaderOption shaderOption) = 0;
    virtual void GetShaderOptions(Array<ShaderOption>& shaderOptions) = 0;
};

#define START_SHADER_KEY(shaderFamily) \
    struct ShaderKeyGroup_##shaderFamily : public ShaderKeyGroupBase \
    { \
        static constexpr uint32_t ms_ShaderOptionGroup[] = {

#define ADD_SHADER_OPTION(shaderOption) \
    uint32_t(ShaderOption::ShaderOption_##shaderOption),

#define END_SHADER_KEY(shaderFamily) \
        0xFFFFFFFF \
        }; \
        virtual uint32_t GetShiftForShaderOption(ShaderOption shaderOption) override \
        { \
            uint32_t idx = 0; \
            uint32_t currentShaderOption = 0; \
            uint32_t rightShift = 0; \
            while (true) \
            { \
                currentShaderOption = ms_ShaderOptionGroup[idx]; \
                if (currentShaderOption == 0xFFFFFFFF || currentShaderOption == uint32_t(shaderOption)) break; \
                rightShift += g_NumBitsForShaderOption[currentShaderOption]; \
                idx += 1; \
            } \
            return rightShift; \
        } \
        virtual void GetShaderOptions(Array<ShaderOption>& shaderOptions) override \
        { \
            shaderOptions.Clear(); \
            uint32_t idx = 0; \
            uint32_t currentShaderOption = 0; \
            while (true) \
            { \
                currentShaderOption = ms_ShaderOptionGroup[idx]; \
                if (currentShaderOption == 0xFFFFFFFF) break; \
                shaderOptions.Add(ShaderOption(currentShaderOption)); \
                idx += 1; \
            } \
        } \
    }; \
    ShaderKeyGroup_##shaderFamily g_ShaderKeyGroup_##shaderFamily;

#include <graphics/shaderkeydefinitions.h>

// 256 is the maximum amount of shader family we can have, since we allocate only 8 bits for it in a ShaderKey
ShaderKeyGroupBase* g_ShaderKeyGroups[256];

void ShaderKey::InitializeShaderKeyGroups()
{
    for (uint32_t i = 0; i < 256; i++)
    {
        g_ShaderKeyGroups[i] = nullptr;
    }

    for (uint32_t i = 0; i < uint32_t(ShaderOption::Count); i++)
    {
        // Maximum of 24 bits per shader option (32 bits shader key with 8 bits for the shader family)
        SHIP_ASSERT(g_NumBitsForShaderOption[i] > 0 && g_NumBitsForShaderOption[i] <= 24);
    }
    
#define START_SHADER_KEY(shaderFamily) \
    g_ShaderKeyGroups[uint32_t(ShaderFamily:: ##shaderFamily)] = &g_ShaderKeyGroup_##shaderFamily;

#include <graphics/shaderkeydefinitions.h>
}

void ShaderKey::GetShaderKeyOptionsForShaderFamily(ShaderFamily shaderFamily, Array<ShaderOption>& shaderOptions)
{
    ShaderKeyGroupBase* shaderKeyGroup = g_ShaderKeyGroups[uint32_t(shaderFamily)];
    SHIP_ASSERT(shaderKeyGroup != nullptr);

    shaderKeyGroup->GetShaderOptions(shaderOptions);
}

ShaderKey::ShaderKey()
    : m_RawShaderKey(0)
{
}

void ShaderKey::SetShaderFamily(ShaderFamily shaderFamily)
{
    m_RawShaderKey &= ~(ms_ShaderFamilyMask << ms_ShaderFamilyShift);
    m_RawShaderKey |= (uint32_t(shaderFamily) << ms_ShaderFamilyShift);
}

void ShaderKey::SetShaderOption(ShaderOption shaderOption, uint32_t value)
{
    uint32_t shaderFamily = ((m_RawShaderKey >> ms_ShaderFamilyShift) & ms_ShaderFamilyMask);
    uint32_t options = ((m_RawShaderKey >> ms_ShaderOptionShift) & ms_ShaderOptionMask);

    uint32_t numBitsForShaderOption = g_NumBitsForShaderOption[uint32_t(shaderOption)];
    uint32_t shaderOptionBitMask = (1 << numBitsForShaderOption) - 1;

    SHIP_ASSERT(value <= shaderOptionBitMask);

    uint32_t shiftForShaderOption = g_ShaderKeyGroups[uint32_t(GetShaderFamily())]->GetShiftForShaderOption(shaderOption);

    options &= ~(shaderOptionBitMask << shiftForShaderOption);
    options |= (value << shiftForShaderOption);

    m_RawShaderKey = ((options << ms_ShaderOptionShift) | shaderFamily);
}

ShaderFamily ShaderKey::GetShaderFamily() const
{
    return ShaderFamily((m_RawShaderKey >> ms_ShaderFamilyShift) & ms_ShaderFamilyMask);
}

uint32_t ShaderKey::GetShaderOptionValue(ShaderOption shaderOption) const
{
    uint32_t options = ((m_RawShaderKey >> ms_ShaderOptionShift) & ms_ShaderOptionMask);

    uint32_t numBitsForShaderOption = g_NumBitsForShaderOption[uint32_t(shaderOption)];
    uint32_t shaderOptionBitMask = (1 << numBitsForShaderOption) - 1;

    uint32_t shiftForShaderOption = g_ShaderKeyGroups[uint32_t(GetShaderFamily())]->GetShiftForShaderOption(shaderOption);

    return ((options >> shiftForShaderOption) & shaderOptionBitMask);
}

ShaderKey::RawShaderKeyType ShaderKey::GetRawShaderKey() const
{
    return m_RawShaderKey;
}

}