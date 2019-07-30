#pragma once

#include <graphics/shader/shaderoptions.h>

#include <system/array.h>
#include <system/string.h>

namespace Shipyard
{
    enum class ComparisonFunc : shipUint8;
    class ShaderKey;

    void GetEffectiveSourceForShaderKey(
            const ShaderKey& shaderKey,
            const Array<ShaderOption>& everyPossibleShaderOption,
            const StringA& source,
            StringA& effectiveSource);

    // Assumes that the state block is composed of statements of type:
    // option = value;
    //
    // Will call user callback when the parser accumulated one option with one value. The callback must return false if an error occured during the parsing
    // of the current option & value pair, true if everything was parsed properly.
    //
    // Returns an error code.
    template <typename ErrorType, typename StateBlockType>
    using InterpretStateBlockOptionPtr = shipBool (*) (
            const StringA& stateOption,
            const StringA& stateValue,
            StateBlockType& stateBlock,
            ErrorType& compilationError);

    template <typename ErrorType, typename StateBlockType>
    void CompileStateBlock(
            const StringA& stateBlockSource,
            StateBlockType& compiledStateBlock,
            ErrorType& stateBlockCompilationError,
            InterpretStateBlockOptionPtr<ErrorType, StateBlockType> userCallback);

    shipBool InterpretBooleanValue(const StringA& value, shipBool* outValue);

    template <typename IntegerType>
    shipBool InterpretIntegerValue(const StringA& value, IntegerType* outValue);

    shipBool InterpretFloatValue(const StringA& value, shipFloat* outValue);
    shipBool InterpretComparisonFunc(const StringA& value, ComparisonFunc* outValue);
}

#include <graphics/shadercompiler/shadercompilerutilities.inl>