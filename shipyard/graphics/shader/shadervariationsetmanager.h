#pragma once

#include <system/platform.h>

#include <graphics/shader/shaderfamilies.h>
#include <graphics/shader/shaderkey.h>
#include <graphics/shader/shaderoptions.h>

namespace Shipyard
{
    class SHIPYARD_API ShaderVariationSetManager
    {
    public:
        static ShaderVariationSetManager& GetInstance()
        {
            static ShaderVariationSetManager s_ShaderVariationSetManager;
            return s_ShaderVariationSetManager;
        }

        ShaderVariationSetManager();

        void StartShaderVariationSetForShaderFamily(ShaderFamily shaderFamily);
        void EndShaderVariationSetForShaderFamily(ShaderFamily shaderFamily);

        // Sets the valid range for a ShaderOption for the current ShaderFamily. Values outside of that range will trigger
        // an assert when trying to use them during rendering.
        void SetShaderOptionValueRange(ShaderOption shaderOption, shipUint32 minRangeInclusive, shipUint32 maxRangeInclusive);

        // Sets an invalid combination of ShaderOptions, in the form of a partial ShaderKey. Combination of those ShaderOptions
        // will trigger an assert when trying to use a ShaderKey that contains them during rendering.
        void SetInvalidPartialShaderKey(const ShaderKey& invalidPartialShaderKey);

        enum class ShaderKeyValidationOption { AssertOnError, DontAssertOnError };
        shipBool ValidateShaderKey(const ShaderKey& shaderKey, ShaderKeyValidationOption shaderKeyValidationOption) const;

    private:
        struct ValidShaderOptionValueRange
        {
            ShaderOption shaderOption = ShaderOption::Count;
            shipUint32 minRangeInclusive = 0;
            shipUint32 maxRangeInclusive = 0xffffffff;
        };

        static const shipUint32 ms_MaxValidShaderOptionValueRanges = 24;
        static const shipUint32 ms_MaxInvalidPartialShaderKeys = 1024;

    private:
        ShaderFamily m_CurrentShaderVariationSetShaderFamily;

        shipUint32 m_ValidShaderOptionValueRangeIndicesPerShaderFamily[shipUint32(ShaderFamily::Count)];
        ValidShaderOptionValueRange m_ValidShaderOptionValueRangesPerShaderFamily[shipUint32(ShaderFamily::Count)][ms_MaxValidShaderOptionValueRanges];

        shipUint32 m_InvalidPartialShaderKeyIndicesPerShaderFamily[shipUint32(ShaderFamily::Count)];
        ShaderKey m_InvalidPartialShaderKeysPerShaderFamily[shipUint32(ShaderFamily::Count)][ms_MaxInvalidPartialShaderKeys];
    };

    SHIPYARD_API ShaderVariationSetManager& GetShaderVariationSetManager();
}