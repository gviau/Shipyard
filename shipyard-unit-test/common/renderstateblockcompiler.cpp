#include <catch.hpp>

#include <common/shadercompiler/renderstateblockcompiler.h>

#include <common/common.h>
#include <common/shaderfamilies.h>
#include <common/shaderkey.h>

TEST_CASE("Test RenderStateBlockCompiler", "[ShaderCompiler]")
{
    Shipyard::ShaderKey::InitializeShaderKeyGroups();

    Shipyard::ShaderKey testShaderKey;
    testShaderKey.SetShaderFamily(Shipyard::ShaderFamily::Error);

    SECTION("Null RenderStateBlock")
    {
        Shipyard::StringA renderStateBlockSource = "";

        Shipyard::Array<Shipyard::ShaderOption> everyPossibleShaderOptionForShaderKey;
        Shipyard::ShaderKey::GetShaderKeyOptionsForShaderFamily(testShaderKey.GetShaderFamily(), everyPossibleShaderOptionForShaderKey);

        Shipyard::RasterizerState rasterizerState;
        Shipyard::DepthStencilState depthStencilState;

        Shipyard::RasterizerState oldRasterizerState = rasterizerState;
        Shipyard::DepthStencilState oldDepthStencilState = depthStencilState;

        Shipyard::RenderStateBlockCompilationError renderStateBlockCompilationError = Shipyard::CompileRenderStateBlock(
            testShaderKey,
            everyPossibleShaderOptionForShaderKey,
            renderStateBlockSource,
            rasterizerState,
            depthStencilState);

        REQUIRE(renderStateBlockCompilationError == Shipyard::RenderStateBlockCompilationError::NoError);

        REQUIRE(rasterizerState == oldRasterizerState);
        REQUIRE(depthStencilState == oldDepthStencilState);
    }

    SECTION("Simple RenderStateBlock")
    {
        Shipyard::StringA renderStateBlockSource = "FillMode = Wireframe; CullMode = CullFrontFace;";

        Shipyard::Array<Shipyard::ShaderOption> everyPossibleShaderOptionForShaderKey;
        Shipyard::ShaderKey::GetShaderKeyOptionsForShaderFamily(testShaderKey.GetShaderFamily(), everyPossibleShaderOptionForShaderKey);

        Shipyard::RasterizerState rasterizerState;
        Shipyard::DepthStencilState depthStencilState;

        Shipyard::RenderStateBlockCompilationError renderStateBlockCompilationError = Shipyard::CompileRenderStateBlock(
                testShaderKey,
                everyPossibleShaderOptionForShaderKey,
                renderStateBlockSource,
                rasterizerState,
                depthStencilState);

        REQUIRE(renderStateBlockCompilationError == Shipyard::RenderStateBlockCompilationError::NoError);

        REQUIRE(rasterizerState.m_FillMode == Shipyard::FillMode::Wireframe);
        REQUIRE(rasterizerState.m_CullMode == Shipyard::CullMode::CullFrontFace);
    }

    SECTION("RenderStateBlock With Inline Comment")
    {
        Shipyard::StringA renderStateBlockSource = "// FillMode = Wireframe;\n CullMode = CullFrontFace;";

        Shipyard::Array<Shipyard::ShaderOption> everyPossibleShaderOptionForShaderKey;
        Shipyard::ShaderKey::GetShaderKeyOptionsForShaderFamily(testShaderKey.GetShaderFamily(), everyPossibleShaderOptionForShaderKey);

        Shipyard::RasterizerState rasterizerState;
        Shipyard::DepthStencilState depthStencilState;

        Shipyard::RenderStateBlockCompilationError renderStateBlockCompilationError = Shipyard::CompileRenderStateBlock(
                testShaderKey,
                everyPossibleShaderOptionForShaderKey,
                renderStateBlockSource,
                rasterizerState,
                depthStencilState);

        REQUIRE(renderStateBlockCompilationError == Shipyard::RenderStateBlockCompilationError::NoError);

        REQUIRE(rasterizerState.m_FillMode == Shipyard::FillMode::Solid);
        REQUIRE(rasterizerState.m_CullMode == Shipyard::CullMode::CullFrontFace);
    }

    SECTION("RenderStateBlock With Block Comment")
    {
        Shipyard::StringA renderStateBlockSource = "/* FillMode = Wireframe; */ CullMode = CullFrontFace;";

        Shipyard::Array<Shipyard::ShaderOption> everyPossibleShaderOptionForShaderKey;
        Shipyard::ShaderKey::GetShaderKeyOptionsForShaderFamily(testShaderKey.GetShaderFamily(), everyPossibleShaderOptionForShaderKey);

        Shipyard::RasterizerState rasterizerState;
        Shipyard::DepthStencilState depthStencilState;

        Shipyard::RenderStateBlockCompilationError renderStateBlockCompilationError = Shipyard::CompileRenderStateBlock(
                testShaderKey,
                everyPossibleShaderOptionForShaderKey,
                renderStateBlockSource,
                rasterizerState,
                depthStencilState);

        REQUIRE(renderStateBlockCompilationError == Shipyard::RenderStateBlockCompilationError::NoError);

        REQUIRE(rasterizerState.m_FillMode == Shipyard::FillMode::Solid);
        REQUIRE(rasterizerState.m_CullMode == Shipyard::CullMode::CullFrontFace);
    }

    SECTION("RenderStateBlock With ifdef")
    {
        Shipyard::StringA renderStateBlockSource = "#if Test2Bits == 0 \n FillMode = Wireframe; \n #endif \n CullMode = CullFrontFace;";

        Shipyard::Array<Shipyard::ShaderOption> everyPossibleShaderOptionForShaderKey;
        Shipyard::ShaderKey::GetShaderKeyOptionsForShaderFamily(testShaderKey.GetShaderFamily(), everyPossibleShaderOptionForShaderKey);

        Shipyard::RasterizerState rasterizerState;
        Shipyard::DepthStencilState depthStencilState;

        Shipyard::RenderStateBlockCompilationError renderStateBlockCompilationError = Shipyard::CompileRenderStateBlock(
                testShaderKey,
                everyPossibleShaderOptionForShaderKey,
                renderStateBlockSource,
                rasterizerState,
                depthStencilState);

        REQUIRE(renderStateBlockCompilationError == Shipyard::RenderStateBlockCompilationError::NoError);

        REQUIRE(rasterizerState.m_FillMode == Shipyard::FillMode::Wireframe);
        REQUIRE(rasterizerState.m_CullMode == Shipyard::CullMode::CullFrontFace);

        rasterizerState = Shipyard::RasterizerState();
        depthStencilState = Shipyard::DepthStencilState();

        renderStateBlockSource = "#if Test2Bits == 1 \n FillMode = Wireframe; \n #endif \n CullMode = CullFrontFace;";

        renderStateBlockCompilationError = Shipyard::CompileRenderStateBlock(
                testShaderKey,
                everyPossibleShaderOptionForShaderKey,
                renderStateBlockSource,
                rasterizerState,
                depthStencilState);

        REQUIRE(renderStateBlockCompilationError == Shipyard::RenderStateBlockCompilationError::NoError);

        REQUIRE(rasterizerState.m_FillMode == Shipyard::FillMode::Solid);
        REQUIRE(rasterizerState.m_CullMode == Shipyard::CullMode::CullFrontFace);

        rasterizerState = Shipyard::RasterizerState();
        depthStencilState = Shipyard::DepthStencilState();

        SET_SHADER_OPTION(testShaderKey, Test2Bits, 1);

        renderStateBlockCompilationError = Shipyard::CompileRenderStateBlock(
            testShaderKey,
            everyPossibleShaderOptionForShaderKey,
            renderStateBlockSource,
            rasterizerState,
            depthStencilState);

        REQUIRE(renderStateBlockCompilationError == Shipyard::RenderStateBlockCompilationError::NoError);

        REQUIRE(rasterizerState.m_FillMode == Shipyard::FillMode::Wireframe);
        REQUIRE(rasterizerState.m_CullMode == Shipyard::CullMode::CullFrontFace);
    }

    SECTION("Full RenderStateBlock")
    {
        Shipyard::StringA renderStateBlockSource =
"DepthBias = -5;"
"DepthBiasClamp = 1.5;"
"SlopeScaledDepthBias = 2.25f;"
"FillMode = Wireframe;"
"CullMode = CullNone;"
"IsFrontCounterClockwise = true;"
"DepthClipEnable = True;"
"scissorenable = TRUE;"
"multisampleEnable = trUE;"
"AntialiasedLineEnable = true;"

"DepthComparisonFunc = Greater;"
"StencilReadMask = 0xff;"
"StencilWriteMask = 0b11001100;"
"FrontFaceStencilFailOp = Keep;"
"FrontFaceStencilDepthFailOp = Zero;"
"FrontFaceStencilPassOp = Replace;"
"FrontFaceStencilComparisonFunc = Never;"
"BackFaceStencilFailOp = IncrSaturate;"
"BackFaceStencilDepthFailOp = DecrSaturate;"
"BackFaceStencilPassOp = Invert;"
"BackFaceStencilComparisonFunc = Equal;"

"DepthEnable = false;"
"EnableDepthWrite = false;"
"StencilEnable = true;"
;

        Shipyard::Array<Shipyard::ShaderOption> everyPossibleShaderOptionForShaderKey;
        Shipyard::ShaderKey::GetShaderKeyOptionsForShaderFamily(testShaderKey.GetShaderFamily(), everyPossibleShaderOptionForShaderKey);

        Shipyard::RasterizerState rasterizerState;
        Shipyard::DepthStencilState depthStencilState;

        Shipyard::RenderStateBlockCompilationError renderStateBlockCompilationError = Shipyard::CompileRenderStateBlock(
                testShaderKey,
                everyPossibleShaderOptionForShaderKey,
                renderStateBlockSource,
                rasterizerState,
                depthStencilState);

        REQUIRE(renderStateBlockCompilationError == Shipyard::RenderStateBlockCompilationError::NoError);

        REQUIRE(rasterizerState.m_DepthBias == -5);
        REQUIRE(fabs(rasterizerState.m_DepthBiasClamp - 1.5f) < 0.0001f);
        REQUIRE(fabs(rasterizerState.m_SlopeScaledDepthBias - 2.25f) < 0.0001f);
        REQUIRE(rasterizerState.m_FillMode == Shipyard::FillMode::Wireframe);
        REQUIRE(rasterizerState.m_CullMode == Shipyard::CullMode::CullNone);
        REQUIRE(rasterizerState.m_IsFrontCounterClockwise == true);
        REQUIRE(rasterizerState.m_DepthClipEnable == true);
        REQUIRE(rasterizerState.m_ScissorEnable == true);
        REQUIRE(rasterizerState.m_MultisampleEnable == true);
        REQUIRE(rasterizerState.m_AntialiasedLineEnable == true);
        REQUIRE(depthStencilState.m_DepthComparisonFunc == Shipyard::ComparisonFunc::Greater);
        REQUIRE(depthStencilState.m_StencilReadMask == 0xff);
        REQUIRE(depthStencilState.m_StencilWriteMask == 0b11001100);
        REQUIRE(depthStencilState.m_FrontFaceStencilFailOp == Shipyard::StencilOperation::Keep);
        REQUIRE(depthStencilState.m_FrontFaceStencilDepthFailOp == Shipyard::StencilOperation::Zero);
        REQUIRE(depthStencilState.m_FrontFaceStencilPassOp == Shipyard::StencilOperation::Replace);
        REQUIRE(depthStencilState.m_FrontFaceStencilComparisonFunc == Shipyard::ComparisonFunc::Never);
        REQUIRE(depthStencilState.m_BackFaceStencilFailOp == Shipyard::StencilOperation::IncrSaturate);
        REQUIRE(depthStencilState.m_BackFaceStencilDepthFailOp == Shipyard::StencilOperation::DecrSaturate);
        REQUIRE(depthStencilState.m_BackFaceStencilPassOp == Shipyard::StencilOperation::Invert);
        REQUIRE(depthStencilState.m_BackFaceStencilComparisonFunc == Shipyard::ComparisonFunc::Equal);
        REQUIRE(depthStencilState.m_DepthEnable == false);
        REQUIRE(depthStencilState.m_EnableDepthWrite == false);
        REQUIRE(depthStencilState.m_StencilEnable == true);
    }

    SECTION("Complex RenderStateBlock")
    {
        Shipyard::StringA renderStateBlockSource =
"DepthBias = -5;\n"
"DepthBiasClamp = 1.5;\n"
"SlopeScaledDepthBias = 2.25f;\n"
"FillMode = /* Solid */ Wireframe;\n"
"CullMode = CullFrontFace;\n"
"IsFrontCounterClockwise = true;\n"
"// #if Test2Bits == 1\n"
"DepthClipEnable = True;\n"
"scissorenable = TRUE;\n"
"multisampleEnable = trUE;\n"
"AntialiasedLineEnable = true;\n"
"// #endif\n"

"DepthComparisonFunc = Greater;\n"
"// StencilReadMask = 0xff;\n"
"// StencilWriteMask = 0b11001100;\n"
"/*\n"
"FrontFaceStencilFailOp = Keep;\n"
"FrontFaceStencilDepthFailOp = Zero;\n"
"FrontFaceStencilPassOp = Replace;\n"
"FrontFaceStencilComparisonFunc = Never;\n"
"BackFaceStencilFailOp = IncrSaturate;\n"
"BackFaceStencilDepthFailOp = DecrSaturate;\n"
"BackFaceStencilPassOp = Invert;\n"
"BackFaceStencilComparisonFunc = Equal;\n"
"*/\n"

"#if Test2Bits == 1\n"
"DepthEnable = false;\n"
"EnableDepthWrite = false;\n"
"StencilEnable = true;\n"
"#elif Test2Bits == 2\n"
"DepthEnable = true;\n"
"EnableDepthWrite = true\n"
"#else\n"
"DepthEnable = false;\n"
"EnableDepthWrite = true;\n"
"#endif\n"
;

        Shipyard::Array<Shipyard::ShaderOption> everyPossibleShaderOptionForShaderKey;
        Shipyard::ShaderKey::GetShaderKeyOptionsForShaderFamily(testShaderKey.GetShaderFamily(), everyPossibleShaderOptionForShaderKey);

        Shipyard::RasterizerState rasterizerState;
        Shipyard::DepthStencilState depthStencilState;

        Shipyard::RenderStateBlockCompilationError renderStateBlockCompilationError = Shipyard::CompileRenderStateBlock(
            testShaderKey,
            everyPossibleShaderOptionForShaderKey,
            renderStateBlockSource,
            rasterizerState,
            depthStencilState);

        REQUIRE(renderStateBlockCompilationError == Shipyard::RenderStateBlockCompilationError::NoError);

        REQUIRE(rasterizerState.m_DepthBias == -5);
        REQUIRE(fabs(rasterizerState.m_DepthBiasClamp - 1.5f) < 0.0001f);
        REQUIRE(fabs(rasterizerState.m_SlopeScaledDepthBias - 2.25f) < 0.0001f);
        REQUIRE(rasterizerState.m_FillMode == Shipyard::FillMode::Wireframe);
        REQUIRE(rasterizerState.m_CullMode == Shipyard::CullMode::CullFrontFace);
        REQUIRE(rasterizerState.m_IsFrontCounterClockwise == true);
        REQUIRE(rasterizerState.m_DepthClipEnable == true);
        REQUIRE(rasterizerState.m_ScissorEnable == true);
        REQUIRE(rasterizerState.m_MultisampleEnable == true);
        REQUIRE(rasterizerState.m_AntialiasedLineEnable == true);
        REQUIRE(depthStencilState.m_DepthComparisonFunc == Shipyard::ComparisonFunc::Greater);
        REQUIRE(depthStencilState.m_StencilReadMask == 0);
        REQUIRE(depthStencilState.m_StencilWriteMask == 0);
        REQUIRE(depthStencilState.m_FrontFaceStencilFailOp == Shipyard::StencilOperation::Keep);
        REQUIRE(depthStencilState.m_FrontFaceStencilDepthFailOp == Shipyard::StencilOperation::Keep);
        REQUIRE(depthStencilState.m_FrontFaceStencilPassOp == Shipyard::StencilOperation::Keep);
        REQUIRE(depthStencilState.m_FrontFaceStencilComparisonFunc == Shipyard::ComparisonFunc::Always);
        REQUIRE(depthStencilState.m_BackFaceStencilFailOp == Shipyard::StencilOperation::Keep);
        REQUIRE(depthStencilState.m_BackFaceStencilDepthFailOp == Shipyard::StencilOperation::Keep);
        REQUIRE(depthStencilState.m_BackFaceStencilPassOp == Shipyard::StencilOperation::Keep);
        REQUIRE(depthStencilState.m_BackFaceStencilComparisonFunc == Shipyard::ComparisonFunc::Always);
        REQUIRE(depthStencilState.m_DepthEnable == false);
        REQUIRE(depthStencilState.m_EnableDepthWrite == true);
        REQUIRE(depthStencilState.m_StencilEnable == false);
    }
}