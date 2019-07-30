#pragma once

namespace Shipyard
{
    template <typename ErrorType, typename StateBlockType>
    void CompileStateBlock(
            const StringA& stateBlockSource,
            StateBlockType& compiledStateBlock,
            ErrorType& stateBlockCompilationError,
            InterpretStateBlockOptionPtr<ErrorType, StateBlockType> userCallback)
    {
        StringA stateOption;
        StringA stateValue;
        stateOption.Reserve(256);
        stateValue.Reserve(256);

        shipBool processOption = true;

        for (size_t i = 0; i < stateBlockSource.Size(); i++)
        {
            shipChar c = stateBlockSource[i];

            if (c == ';')
            {
                shipBool couldCompileOption = userCallback(
                        stateOption,
                        stateValue,
                        compiledStateBlock,
                        stateBlockCompilationError);

                if (!couldCompileOption)
                {
                    return;
                }

                stateOption.Resize(0);
                stateValue.Resize(0);
                processOption = true;
            }
            else if (c == '=')
            {
                processOption = false;
            }
            else
            {
                if (!isalnum(c) && c != '-' && c != '.' && c != '[' && c != ']')
                {
                    continue;
                }

                if (processOption)
                {
                    stateOption += c;
                }
                else
                {
                    stateValue += c;
                }
            }
        }
    }

    template <typename IntegerType>
    shipBool InterpretIntegerValue(const StringA& value, IntegerType* outValue)
    {
        size_t startingIndex = 0;
        shipBool isNegative = false;
        if (value[0] == '-')
        {
            startingIndex = 1;
            isNegative = true;
        }

        if (value.Substring(startingIndex, 2 + startingIndex).EqualCaseInsensitive("0x"))
        {
            StringA hexadecimalValue = value.Substring(2 + startingIndex, value.InvalidIndex);

            for (size_t i = 0; i < hexadecimalValue.Size(); i++)
            {
                int c = int(hexadecimalValue[i]);

                shipBool isCharacterInvalidHexadecimal = ((c < int('0') || c > int('9')) && (c < int('a') || c > int('f')) && (c < int('A') || c > int('F')));
                if (isCharacterInvalidHexadecimal)
                {
                    return false;
                }
            }

            *outValue = IntegerType(strtol(hexadecimalValue.GetBuffer(), nullptr, 16));

            if (isNegative)
            {
                *outValue = static_cast<IntegerType>(-std::make_signed<IntegerType>::type(*outValue));
            }
        }
        else if (value.Substring(startingIndex, 2 + startingIndex).EqualCaseInsensitive("0b"))
        {
            StringA binaryValue = value.Substring(2 + startingIndex, value.InvalidIndex);

            for (size_t i = 0; i < binaryValue.Size(); i++)
            {
                shipChar c = binaryValue[i];

                shipBool isCharacterInvalidBinary = (c != '0' && c != '1');
                if (isCharacterInvalidBinary)
                {
                    return false;
                }
            }

            *outValue = IntegerType(strtol(binaryValue.GetBuffer(), nullptr, 2));

            if (isNegative)
            {
                *outValue = static_cast<IntegerType>(-std::make_signed<IntegerType>::type(*outValue));
            }
        }
        else
        {
            for (size_t i = startingIndex; i < value.Size(); i++)
            {
                int c = int(value[i]);

                shipBool isCharacterInvalidDecimal = (c < int('0') || c > int('9'));
                if (isCharacterInvalidDecimal)
                {
                    return false;
                }
            }

            // Negative values are properly handled here since we hand over the whole string.
            *outValue = IntegerType(strtol(value.GetBuffer(), nullptr, 10));
        }

        return true;
    }
}