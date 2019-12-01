#include <graphics/shader/shaderkey.h>

#include <graphics/shader/shaderfamilies.h>
#include <graphics/shader/shaderoptions.h>

#include <system/systemcommon.h>

namespace Shipyard
{;

extern shipUint8 g_NumBitsForShaderOption[shipUint32(ShaderOption::Count)];

struct ShaderKeyGroupBase
{
    virtual shipUint32 GetShiftForShaderOption(ShaderOption shaderOption) = 0;
    virtual void GetShaderOptions(Array<ShaderOption>& shaderOptions) = 0;
};

#define START_SHADER_KEY(shaderFamily) \
    struct ShaderKeyGroup_##shaderFamily : public ShaderKeyGroupBase \
    { \
        static constexpr shipUint32 ms_ShaderOptionGroup[] = {

#define ADD_SHADER_OPTION(shaderOption) \
    shipUint32(ShaderOption::ShaderOption_##shaderOption),

#define END_SHADER_KEY(shaderFamily) \
        0xFFFFFFFF \
        }; \
        virtual shipUint32 GetShiftForShaderOption(ShaderOption shaderOption) override \
        { \
            shipUint32 idx = 0; \
            shipUint32 currentShaderOption = 0; \
            shipUint32 rightShift = 0; \
            while (true) \
            { \
                currentShaderOption = ms_ShaderOptionGroup[idx]; \
                if (currentShaderOption == 0xFFFFFFFF || currentShaderOption == shipUint32(shaderOption)) break; \
                rightShift += g_NumBitsForShaderOption[currentShaderOption]; \
                idx += 1; \
            } \
            return rightShift; \
        } \
        virtual void GetShaderOptions(Array<ShaderOption>& shaderOptions) override \
        { \
            shaderOptions.Clear(); \
            shipUint32 idx = 0; \
            shipUint32 currentShaderOption = 0; \
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

#include <graphics/shader/shaderkeydefinitions.h>

// 256 is the maximum amount of shader family we can have, since we allocate only 8 bits for it in a ShaderKey
ShaderKeyGroupBase* g_ShaderKeyGroups[256];

void ShaderKey::InitializeShaderKeyGroups()
{
    for (shipUint32 i = 0; i < 256; i++)
    {
        g_ShaderKeyGroups[i] = nullptr;
    }

    for (shipUint32 i = 0; i < shipUint32(ShaderOption::Count); i++)
    {
        // Maximum of 24 bits per shader option (32 bits shader key with 8 bits for the shader family)
        SHIP_ASSERT(g_NumBitsForShaderOption[i] > 0 && g_NumBitsForShaderOption[i] <= 24);
    }
    
#define START_SHADER_KEY(shaderFamily) \
    g_ShaderKeyGroups[shipUint32(ShaderFamily:: ##shaderFamily)] = &g_ShaderKeyGroup_##shaderFamily;

#include <graphics/shader/shaderkeydefinitions.h>
}

void ShaderKey::GetShaderKeyOptionsForShaderFamily(ShaderFamily shaderFamily, Array<ShaderOption>& shaderOptions)
{
    ShaderKeyGroupBase* shaderKeyGroup = g_ShaderKeyGroups[shipUint32(shaderFamily)];
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
    m_RawShaderKey |= (shipUint32(shaderFamily) << ms_ShaderFamilyShift);
}

void ShaderKey::SetShaderOption(ShaderOption shaderOption, shipUint32 value)
{
    shipUint32 shaderFamily = ((m_RawShaderKey >> ms_ShaderFamilyShift) & ms_ShaderFamilyMask);
    shipUint32 options = ((m_RawShaderKey >> ms_ShaderOptionShift) & ms_ShaderOptionMask);

    shipUint32 numBitsForShaderOption = g_NumBitsForShaderOption[shipUint32(shaderOption)];
    shipUint32 shaderOptionBitMask = (1 << numBitsForShaderOption) - 1;

    SHIP_ASSERT(value <= shaderOptionBitMask);

    shipUint32 shiftForShaderOption = g_ShaderKeyGroups[shipUint32(GetShaderFamily())]->GetShiftForShaderOption(shaderOption);

    options &= ~(shaderOptionBitMask << shiftForShaderOption);
    options |= (value << shiftForShaderOption);

    m_RawShaderKey = ((options << ms_ShaderOptionShift) | shaderFamily);
}

ShaderFamily ShaderKey::GetShaderFamily() const
{
    return ShaderFamily((m_RawShaderKey >> ms_ShaderFamilyShift) & ms_ShaderFamilyMask);
}

shipUint32 ShaderKey::GetShaderOptionValue(ShaderOption shaderOption) const
{
    shipUint32 options = ((m_RawShaderKey >> ms_ShaderOptionShift) & ms_ShaderOptionMask);

    shipUint32 numBitsForShaderOption = g_NumBitsForShaderOption[shipUint32(shaderOption)];
    shipUint32 shaderOptionBitMask = (1 << numBitsForShaderOption) - 1;

    shipUint32 shiftForShaderOption = g_ShaderKeyGroups[shipUint32(GetShaderFamily())]->GetShiftForShaderOption(shaderOption);

    return ((options >> shiftForShaderOption) & shaderOptionBitMask);
}

ShaderKey::RawShaderKeyType ShaderKey::GetRawShaderKey() const
{
    return m_RawShaderKey;
}

void ShaderKey::GetEveryShaderKeyForShaderFamily(ShaderFamily shaderFamily, BigArray<ShaderKey>& everyShaderKeyForShaderFamily)
{
    Array<ShaderOption> everyPossibleShaderOption;
    ShaderKey::GetShaderKeyOptionsForShaderFamily(shaderFamily, everyPossibleShaderOption);

    shipUint32 numBitsInShaderKey = 0;
    for (ShaderOption shaderOption : everyPossibleShaderOption)
    {
        numBitsInShaderKey += shipUint32(g_NumBitsForShaderOption[shipUint32(shaderOption)]);
    }

    shipUint32 everyShaderOptionSet = ((1 << numBitsInShaderKey) - 1);

    shipUint32 possibleNumberOfPermutations = everyShaderOptionSet + 1;

    ShaderKey shaderKey;
    shaderKey.SetShaderFamily(shaderFamily);

    ShaderKey::RawShaderKeyType baseRawShaderKey = shaderKey.GetRawShaderKey();

    // Go through every permutation of shader options
    shipUint32 shaderOptionAsInt = everyShaderOptionSet;

    everyShaderKeyForShaderFamily.Reserve(possibleNumberOfPermutations);

    for (shipUint32 i = 0; i < possibleNumberOfPermutations; i++)
    {
        ShaderKey& currentShaderKeyPermutation = everyShaderKeyForShaderFamily.Grow();
        currentShaderKeyPermutation.m_RawShaderKey = (baseRawShaderKey | (shaderOptionAsInt << ShaderKey::ms_ShaderOptionShift));

        shaderOptionAsInt -= 1;
    }
}

}