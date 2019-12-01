#include <graphics/shader/shadervariationsetmanager.h>

#include <system/systemdebug.h>

namespace Shipyard
{;

ShaderVariationSetManager::ShaderVariationSetManager()
    : m_CurrentShaderVariationSetShaderFamily(ShaderFamily::Count)
{
    SHIP_STATIC_ASSERT(sizeof(ShaderKey::RawShaderKeyType) == 4);
    SHIP_STATIC_ASSERT_MSG(
            ms_MaxValidShaderOptionValueRanges == ((sizeof(ShaderKey::RawShaderKeyType) * 8) - ShaderKey::ms_ShaderOptionShift),
            "There isn't the same number of maximum valid shader option value ranges than number of option bits in a ShaderKey!");

    for (shipUint32 i = 0; i < shipUint32(ShaderFamily::Count); i++)
    {
        m_ValidShaderOptionValueRangeIndicesPerShaderFamily[i] = 0;
        m_InvalidPartialShaderKeyIndicesPerShaderFamily[i] = 0;
    }

    ShaderKey::InitializeShaderKeyGroups();
}

void ShaderVariationSetManager::StartShaderVariationSetForShaderFamily(ShaderFamily shaderFamily)
{
    SHIP_ASSERT(m_CurrentShaderVariationSetShaderFamily == ShaderFamily::Count);

    m_CurrentShaderVariationSetShaderFamily = shaderFamily;
}

void ShaderVariationSetManager::EndShaderVariationSetForShaderFamily(ShaderFamily shaderFamily)
{
    SHIP_ASSERT(m_CurrentShaderVariationSetShaderFamily == shaderFamily);

    m_CurrentShaderVariationSetShaderFamily = ShaderFamily::Count;
}

void ShaderVariationSetManager::SetShaderOptionValueRange(ShaderOption shaderOption, shipUint32 minRangeInclusive, shipUint32 maxRangeInclusive)
{
    SHIP_ASSERT(m_CurrentShaderVariationSetShaderFamily != ShaderFamily::Count);

    shipUint32 shaderFamilyIdx = shipUint32(m_CurrentShaderVariationSetShaderFamily);

    shipUint32 idx = m_ValidShaderOptionValueRangeIndicesPerShaderFamily[shaderFamilyIdx]++;

    ValidShaderOptionValueRange& validShaderOptionValueRangeForShaderFamily = m_ValidShaderOptionValueRangesPerShaderFamily[shaderFamilyIdx][idx];
    validShaderOptionValueRangeForShaderFamily.shaderOption = shaderOption;
    validShaderOptionValueRangeForShaderFamily.minRangeInclusive = minRangeInclusive;
    validShaderOptionValueRangeForShaderFamily.maxRangeInclusive = maxRangeInclusive;
}

void ShaderVariationSetManager::SetInvalidPartialShaderKey(const ShaderKey& invalidPartialShaderKey)
{
    SHIP_ASSERT(m_CurrentShaderVariationSetShaderFamily != ShaderFamily::Count);

    shipUint32 shaderFamilyIdx = shipUint32(m_CurrentShaderVariationSetShaderFamily);

    shipUint32 idx = m_InvalidPartialShaderKeyIndicesPerShaderFamily[shaderFamilyIdx]++;

    m_InvalidPartialShaderKeysPerShaderFamily[shaderFamilyIdx][idx] = invalidPartialShaderKey;
}

extern const shipChar* g_ShaderFamilyString[shipUint8(ShaderFamily::Count)];
extern const shipChar* g_ShaderOptionString[shipUint32(ShaderOption::Count)];

shipBool ShaderVariationSetManager::ValidateShaderKey(const ShaderKey& shaderKey, ShaderKeyValidationOption shaderKeyValidationOption) const
{
    ShaderFamily shaderFamily = shaderKey.GetShaderFamily();
    SHIP_ASSERT(shaderFamily != ShaderFamily::Count);

    shipUint32 shaderFamilyIdx = shipUint32(shaderFamily);

    shipUint32 numValidShaderOptionRangesForShaderFamily = m_ValidShaderOptionValueRangeIndicesPerShaderFamily[shaderFamilyIdx];
    for (shipUint32 i = 0; i < numValidShaderOptionRangesForShaderFamily; i++)
    {
        const ValidShaderOptionValueRange& validShaderOptionValueRange = m_ValidShaderOptionValueRangesPerShaderFamily[shaderFamilyIdx][i];

        shipUint32 shaderOptionValue = shaderKey.GetShaderOptionValue(validShaderOptionValueRange.shaderOption);

        shipBool shaderOptionValueValid = (shaderOptionValue >= validShaderOptionValueRange.minRangeInclusive && shaderOptionValue <= validShaderOptionValueRange.maxRangeInclusive);

        SHIP_ASSERT_MSG(
                shaderKeyValidationOption == ShaderKeyValidationOption::DontAssertOnError || shaderOptionValueValid,
                "ShaderOption %s with value %u for ShaderFamily %s is not in the valid [%u, %u] range.",
                g_ShaderOptionString[shipUint32(validShaderOptionValueRange.shaderOption)],
                shaderOptionValue,
                g_ShaderFamilyString[shaderFamilyIdx],
                validShaderOptionValueRange.minRangeInclusive,
                validShaderOptionValueRange.maxRangeInclusive);

        if (!shaderOptionValueValid)
        {
            return false;
        }
    }

    ShaderKey::RawShaderKeyType rawShaderKeyWithoutShaderFamilyToValidate = shaderKey.GetRawShaderKeyOptions();

    shipUint32 numInvalidPartialShaderKeyForShaderFamily = m_InvalidPartialShaderKeyIndicesPerShaderFamily[shaderFamilyIdx];
    for (shipUint32 i = 0; i < numInvalidPartialShaderKeyForShaderFamily; i++)
    {
        const ShaderKey& invalidPartialShaderKey = m_InvalidPartialShaderKeysPerShaderFamily[shaderFamilyIdx][i];

        ShaderKey::RawShaderKeyType rawInvalidPartialShaderKeyWithoutShaderFamily = invalidPartialShaderKey.GetRawShaderKeyOptions();

        shipBool shaderKeyOptionCombinationValid = ((rawShaderKeyWithoutShaderFamilyToValidate & rawInvalidPartialShaderKeyWithoutShaderFamily) != rawInvalidPartialShaderKeyWithoutShaderFamily);

        SHIP_ASSERT_MSG(
                shaderKeyValidationOption == ShaderKeyValidationOption::DontAssertOnError || shaderKeyOptionCombinationValid,
                "ShaderFamily %s for ShaderKey 0x%X has an invalid combination of shader options set: the following bits cannot be present together 0x%X",
                g_ShaderFamilyString[shaderFamilyIdx],
                shaderKey.GetRawShaderKey(),
                rawInvalidPartialShaderKeyWithoutShaderFamily);

        if (!shaderKeyOptionCombinationValid)
        {
            return false;
        }
    }

    return true;
}

ShaderVariationSetManager& GetShaderVariationSetManager()
{
    return ShaderVariationSetManager::GetInstance();
}

}