#include <shipyardunittestprecomp.h>

#include <extern/catch/catch.hpp>

#include <graphics/shadercompiler/samplerstatecompiler.h>

#include <graphics/graphicscommon.h>
#include <graphics/shader/shaderfamilies.h>
#include <graphics/shader/shaderkey.h>

#include <utils/unittestutils.h>

TEST_CASE("Test SamplerStateCompiler", "[ShaderCompiler]")
{
    Shipyard::ScoppedGlobalAllocator scoppedGlobalAllocator;

    Shipyard::ShaderKey::InitializeShaderKeyGroups();

    Shipyard::ShaderKey testShaderKey;
    testShaderKey.SetShaderFamily(Shipyard::ShaderFamily::Error);

    SECTION("Null SamplerState")
    {
        Shipyard::StringA samplerStateBlockSource = "";

        Shipyard::Array<Shipyard::ShaderOption> everyPossibleShaderOptionForShaderKey;
        Shipyard::ShaderKey::GetShaderKeyOptionsForShaderFamily(testShaderKey.GetShaderFamily(), everyPossibleShaderOptionForShaderKey);

        Shipyard::SamplerState samplerState;

        Shipyard::SamplerState oldSamplerState = samplerState;

        Shipyard::SamplerStateCompilerError samplerStateCompilationError = Shipyard::CompileSamplerStateBlock(
                testShaderKey,
                everyPossibleShaderOptionForShaderKey,
                samplerStateBlockSource,
                samplerState);

        REQUIRE(samplerStateCompilationError == Shipyard::SamplerStateCompilerError::NoError);

        REQUIRE(samplerState == oldSamplerState);
    }

    SECTION("Simple SampleState")
    {
        Shipyard::StringA samplerStateSource = "MinificationFiltering = Linear; AddressModeU = Mirror;";

        Shipyard::Array<Shipyard::ShaderOption> everyPossibleShaderOptionForShaderKey;
        Shipyard::ShaderKey::GetShaderKeyOptionsForShaderFamily(testShaderKey.GetShaderFamily(), everyPossibleShaderOptionForShaderKey);

        Shipyard::SamplerState samplerState;

        Shipyard::SamplerStateCompilerError samplerStateCompilationError = Shipyard::CompileSamplerStateBlock(
                testShaderKey,
                everyPossibleShaderOptionForShaderKey,
                samplerStateSource,
                samplerState);

        REQUIRE(samplerStateCompilationError == Shipyard::SamplerStateCompilerError::NoError);

        REQUIRE(samplerState.MinificationFiltering == Shipyard::SamplingFilter::Linear);
        REQUIRE(samplerState.AddressModeU == Shipyard::TextureAddressMode::Mirror);
    }

    SECTION("Full SamplerState")
    {
        Shipyard::StringA samplerStateSource =
"MinificationFiltering = Linear;"
"MagnificationFiltering = NEAREST;"
"MipmapFiltering = LinEar;"
"AddressModeU = Clamp;"
"AddressModeV = WRAP;"
"AddressModeW = border;"
"ComparisonFunction = Greater;"

"MipLodBias = 1.50f;"
"MaxAnisotropy = 5;"
"BorderRGBA = 0xaabbccdd;"
"MinLod = -1;"
"MaxLod = 5.5;"

"UseAnisotropicFiltering = true;"
;

        Shipyard::Array<Shipyard::ShaderOption> everyPossibleShaderOptionForShaderKey;
        Shipyard::ShaderKey::GetShaderKeyOptionsForShaderFamily(testShaderKey.GetShaderFamily(), everyPossibleShaderOptionForShaderKey);

        Shipyard::SamplerState samplerState;

        Shipyard::SamplerStateCompilerError samplerStateCompilationError = Shipyard::CompileSamplerStateBlock(
                testShaderKey,
                everyPossibleShaderOptionForShaderKey,
                samplerStateSource,
                samplerState);

        REQUIRE(samplerStateCompilationError == Shipyard::SamplerStateCompilerError::NoError);

        REQUIRE(samplerState.MinificationFiltering == Shipyard::SamplingFilter::Linear);
        REQUIRE(samplerState.MagnificationFiltering == Shipyard::SamplingFilter::Nearest);
        REQUIRE(samplerState.MipmapFiltering == Shipyard::SamplingFilter::Linear);
        REQUIRE(samplerState.AddressModeU == Shipyard::TextureAddressMode::Clamp);
        REQUIRE(samplerState.AddressModeV == Shipyard::TextureAddressMode::Wrap);
        REQUIRE(samplerState.AddressModeW == Shipyard::TextureAddressMode::Border);
        REQUIRE(samplerState.ComparisonFunction == Shipyard::ComparisonFunc::Greater);
        REQUIRE(fabs(samplerState.MipLodBias - 1.5f) < 0.0001f);
        REQUIRE(samplerState.MaxAnisotropy == 5);
        REQUIRE(fabs(samplerState.BorderRGBA[0] - 0.66667f) < 0.00001f);
        REQUIRE(fabs(samplerState.BorderRGBA[1] - 0.73333f) < 0.00001f);
        REQUIRE(fabs(samplerState.BorderRGBA[2] - 0.8f) < 0.00001f);
        REQUIRE(fabs(samplerState.BorderRGBA[3] - 0.86667f) < 0.00001f);
        REQUIRE(fabs(samplerState.MinLod + 1.0f) < 0.00001f);
        REQUIRE(fabs(samplerState.MaxLod - 5.5f) < 0.00001f);
        REQUIRE(samplerState.UseAnisotropicFiltering == true);
    }
}