#include <graphics/shadercompiler/shadercompilerutilities.h>

#include <graphics/shader/shaderkey.h>

#include <graphics/graphicscommon.h>

#pragma warning( disable : 4005 )

#include <d3dcommon.h>
#include <d3dcompiler.h>

#pragma warning( default : 4005 )

namespace Shipyard
{;

extern const shipChar* g_ShaderOptionString[shipUint32(ShaderOption::Count)];

void GetEffectiveSourceForShaderKey(
        const ShaderKey& shaderKey,
        const Array<ShaderOption>& everyPossibleShaderOption,
        const StringA& source,
        StringA& effectiveSource)
{
    Array<D3D_SHADER_MACRO> shaderOptionDefines;
    shaderOptionDefines.Reserve(everyPossibleShaderOption.Size() + 1);

    for (ShaderOption shaderOption : everyPossibleShaderOption)
    {
        shipUint32 valueForShaderOption = shaderKey.GetShaderOptionValue(shaderOption);

        D3D_SHADER_MACRO shaderDefine;
        shaderDefine.Name = g_ShaderOptionString[shipUint32(shaderOption)];

        shipChar* buf = reinterpret_cast<shipChar*>(SHIP_ALLOC(8, 1));
        sprintf_s(buf, 8, "%u", valueForShaderOption);

        shaderDefine.Definition = buf;

        shaderOptionDefines.Add(shaderDefine);
    }

    D3D_SHADER_MACRO nullShaderDefine = { nullptr, nullptr };
    shaderOptionDefines.Add(nullShaderDefine);

    ID3DBlob* preprocessedOutput = nullptr;

    D3DPreprocess(
            &source[0],
            source.Size(),
            "dummyName",
            &shaderOptionDefines[0],
            nullptr,
            &preprocessedOutput,
            nullptr);

    for (D3D_SHADER_MACRO& shaderMacro : shaderOptionDefines)
    {
        SHIP_FREE(shaderMacro.Definition);
    }

    if (preprocessedOutput != nullptr)
    {
        // D3DPreprocess inserts a null character at the end of the preprocess string, and appends the initial string afterwards.
        // Let's remove the initial string.
        size_t preprocessedOutputSize = strlen((shipChar*)preprocessedOutput->GetBufferPointer());

        if (preprocessedOutputSize > 0)
        {
            effectiveSource.Resize(preprocessedOutputSize);

            memcpy(&effectiveSource[0], preprocessedOutput->GetBufferPointer(), preprocessedOutputSize);
        }

        preprocessedOutput->Release();

        if (preprocessedOutputSize > 0)
        {
            // D3DPreprocess also appends a bunch of #line directive, but we're not interested in those, so let's remove them.
            size_t lineDirectiveIndex = 0;
            while (true)
            {
                lineDirectiveIndex = effectiveSource.FindIndexOfFirst("#line", lineDirectiveIndex);

                if (lineDirectiveIndex == effectiveSource.InvalidIndex)
                {
                    break;
                }

                size_t newLineIndex = effectiveSource.FindIndexOfFirst('\n', lineDirectiveIndex);

                // +1 to also account for the newline
                size_t numCharactersToRemove = ((newLineIndex - lineDirectiveIndex) + 1);
                effectiveSource.Erase(lineDirectiveIndex, numCharactersToRemove);
            }
        }
    }
}

shipBool InterpretBooleanValue(const StringA& value, shipBool* outValue)
{
    if (value.EqualCaseInsensitive("true"))
    {
        *outValue = true;
    }
    else if (value.EqualCaseInsensitive("false"))
    {
        *outValue = false;
    }
    else
    {
        return false;
    }

    return true;
}

shipBool InterpretFloatValue(const StringA& value, shipFloat* outValue)
{
    for (size_t i = 0; i < value.Size(); i++)
    {
        int c = int(value[i]);

        shipBool isCharacterInvalidFloat = ((c < int('0') || c > int('9')) && c != int('-') && c != int('.') && c != int('f') && c != int('F'));
        if (isCharacterInvalidFloat)
        {
            return false;
        }
    }

    *outValue = shipFloat(atof(value.GetBuffer()));

    return true;
}


shipBool InterpretComparisonFunc(const StringA& value, ComparisonFunc* outValue)
{
    if (value.EqualCaseInsensitive("Never"))
    {
        *outValue = ComparisonFunc::Never;
    }
    else if (value.EqualCaseInsensitive("Less"))
    {
        *outValue = ComparisonFunc::Less;
    }
    else if (value.EqualCaseInsensitive("Equal"))
    {
        *outValue = ComparisonFunc::Equal;
    }
    else if (value.EqualCaseInsensitive("LessEqual"))
    {
        *outValue = ComparisonFunc::LessEqual;
    }
    else if (value.EqualCaseInsensitive("Greater"))
    {
        *outValue = ComparisonFunc::Greater;
    }
    else if (value.EqualCaseInsensitive("NotEqual"))
    {
        *outValue = ComparisonFunc::NotEqual;
    }
    else if (value.EqualCaseInsensitive("GreaterEqual"))
    {
        *outValue = ComparisonFunc::GreaterEqual;
    }
    else if (value.EqualCaseInsensitive("Always"))
    {
        *outValue = ComparisonFunc::Always;
    }
    else
    {
        return false;
    }

    return true;
}

}