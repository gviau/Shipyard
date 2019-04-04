#include <extern/catch/catch.hpp>

#include <graphics/shadercompiler/renderstateblockcompiler.h>

#include <graphics/graphicscommon.h>
#include <graphics/shaderfamilies.h>
#include <graphics/shaderkey.h>

#include <utils/unittestutils.h>

TEST_CASE("Test RenderStateBlockCompiler", "[ShaderCompiler]")
{
    Shipyard::ScoppedGlobalAllocator scoppedGlobalAllocator;

    Shipyard::ShaderKey::InitializeShaderKeyGroups();

    Shipyard::ShaderKey testShaderKey;
    testShaderKey.SetShaderFamily(Shipyard::ShaderFamily::Error);

    SECTION("Null RenderStateBlock")
    {
        Shipyard::StringA renderStateBlockSource = "";

        Shipyard::Array<Shipyard::ShaderOption> everyPossibleShaderOptionForShaderKey;
        Shipyard::ShaderKey::GetShaderKeyOptionsForShaderFamily(testShaderKey.GetShaderFamily(), everyPossibleShaderOptionForShaderKey);

        Shipyard::RenderStateBlock renderStateBlock;

        Shipyard::RenderStateBlock oldRenderStateBlock = renderStateBlock;
        
        Shipyard::RenderStateBlockCompilationError renderStateBlockCompilationError = Shipyard::CompileRenderStateBlock(
            testShaderKey,
            everyPossibleShaderOptionForShaderKey,
            renderStateBlockSource,
            renderStateBlock);

        REQUIRE(renderStateBlockCompilationError == Shipyard::RenderStateBlockCompilationError::NoError);

        REQUIRE(renderStateBlock.rasterizerState == oldRenderStateBlock.rasterizerState);
        REQUIRE(renderStateBlock.depthStencilState == oldRenderStateBlock.depthStencilState);
    }

    SECTION("Simple RenderStateBlock")
    {
        Shipyard::StringA renderStateBlockSource = "FillMode = Wireframe; CullMode = CullFrontFace;";

        Shipyard::Array<Shipyard::ShaderOption> everyPossibleShaderOptionForShaderKey;
        Shipyard::ShaderKey::GetShaderKeyOptionsForShaderFamily(testShaderKey.GetShaderFamily(), everyPossibleShaderOptionForShaderKey);

        Shipyard::RenderStateBlock renderStateBlock;

        Shipyard::RenderStateBlockCompilationError renderStateBlockCompilationError = Shipyard::CompileRenderStateBlock(
                testShaderKey,
                everyPossibleShaderOptionForShaderKey,
                renderStateBlockSource,
                renderStateBlock);

        REQUIRE(renderStateBlockCompilationError == Shipyard::RenderStateBlockCompilationError::NoError);

        REQUIRE(renderStateBlock.rasterizerState.m_FillMode == Shipyard::FillMode::Wireframe);
        REQUIRE(renderStateBlock.rasterizerState.m_CullMode == Shipyard::CullMode::CullFrontFace);
    }

    SECTION("RenderStateBlock With Inline Comment")
    {
        Shipyard::StringA renderStateBlockSource = "// FillMode = Wireframe;\n CullMode = CullFrontFace;";

        Shipyard::Array<Shipyard::ShaderOption> everyPossibleShaderOptionForShaderKey;
        Shipyard::ShaderKey::GetShaderKeyOptionsForShaderFamily(testShaderKey.GetShaderFamily(), everyPossibleShaderOptionForShaderKey);

        Shipyard::RenderStateBlock renderStateBlock;

        Shipyard::RenderStateBlockCompilationError renderStateBlockCompilationError = Shipyard::CompileRenderStateBlock(
                testShaderKey,
                everyPossibleShaderOptionForShaderKey,
                renderStateBlockSource,
                renderStateBlock);

        REQUIRE(renderStateBlockCompilationError == Shipyard::RenderStateBlockCompilationError::NoError);

        REQUIRE(renderStateBlock.rasterizerState.m_FillMode == Shipyard::FillMode::Solid);
        REQUIRE(renderStateBlock.rasterizerState.m_CullMode == Shipyard::CullMode::CullFrontFace);
    }

    SECTION("RenderStateBlock With Block Comment")
    {
        Shipyard::StringA renderStateBlockSource = "/* FillMode = Wireframe; */ CullMode = CullFrontFace;";

        Shipyard::Array<Shipyard::ShaderOption> everyPossibleShaderOptionForShaderKey;
        Shipyard::ShaderKey::GetShaderKeyOptionsForShaderFamily(testShaderKey.GetShaderFamily(), everyPossibleShaderOptionForShaderKey);

        Shipyard::RenderStateBlock renderStateBlock;

        Shipyard::RenderStateBlockCompilationError renderStateBlockCompilationError = Shipyard::CompileRenderStateBlock(
                testShaderKey,
                everyPossibleShaderOptionForShaderKey,
                renderStateBlockSource,
                renderStateBlock);

        REQUIRE(renderStateBlockCompilationError == Shipyard::RenderStateBlockCompilationError::NoError);

        REQUIRE(renderStateBlock.rasterizerState.m_FillMode == Shipyard::FillMode::Solid);
        REQUIRE(renderStateBlock.rasterizerState.m_CullMode == Shipyard::CullMode::CullFrontFace);
    }

    SECTION("RenderStateBlock With ifdef")
    {
        Shipyard::StringA renderStateBlockSource = "#if Test2Bits == 0 \n FillMode = Wireframe; \n #endif \n CullMode = CullFrontFace;";

        Shipyard::Array<Shipyard::ShaderOption> everyPossibleShaderOptionForShaderKey;
        Shipyard::ShaderKey::GetShaderKeyOptionsForShaderFamily(testShaderKey.GetShaderFamily(), everyPossibleShaderOptionForShaderKey);

        Shipyard::RenderStateBlock renderStateBlock;

        Shipyard::RenderStateBlockCompilationError renderStateBlockCompilationError = Shipyard::CompileRenderStateBlock(
                testShaderKey,
                everyPossibleShaderOptionForShaderKey,
                renderStateBlockSource,
                renderStateBlock);

        REQUIRE(renderStateBlockCompilationError == Shipyard::RenderStateBlockCompilationError::NoError);

        REQUIRE(renderStateBlock.rasterizerState.m_FillMode == Shipyard::FillMode::Wireframe);
        REQUIRE(renderStateBlock.rasterizerState.m_CullMode == Shipyard::CullMode::CullFrontFace);

        renderStateBlock = Shipyard::RenderStateBlock();
        
        renderStateBlockSource = "#if Test2Bits == 1 \n FillMode = Wireframe; \n #endif \n CullMode = CullFrontFace;";

        renderStateBlockCompilationError = Shipyard::CompileRenderStateBlock(
                testShaderKey,
                everyPossibleShaderOptionForShaderKey,
                renderStateBlockSource,
                renderStateBlock);

        REQUIRE(renderStateBlockCompilationError == Shipyard::RenderStateBlockCompilationError::NoError);

        REQUIRE(renderStateBlock.rasterizerState.m_FillMode == Shipyard::FillMode::Solid);
        REQUIRE(renderStateBlock.rasterizerState.m_CullMode == Shipyard::CullMode::CullFrontFace);

        renderStateBlock = Shipyard::RenderStateBlock();

        SET_SHADER_OPTION(testShaderKey, Test2Bits, 1);

        renderStateBlockCompilationError = Shipyard::CompileRenderStateBlock(
                testShaderKey,
                everyPossibleShaderOptionForShaderKey,
                renderStateBlockSource,
                renderStateBlock);

        REQUIRE(renderStateBlockCompilationError == Shipyard::RenderStateBlockCompilationError::NoError);

        REQUIRE(renderStateBlock.rasterizerState.m_FillMode == Shipyard::FillMode::Wireframe);
        REQUIRE(renderStateBlock.rasterizerState.m_CullMode == Shipyard::CullMode::CullFrontFace);
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

"BlendEnable[0] = true;"
"SourceBlend[ 0 ] = SrcColor;"
"DestBlend[0 ] = DestColor;"
"BlendOperator[ 0] = subtract;"
"sourceAlphaBlend[1] = InvDestAlpha;"
"destAlphaBlend[3] = UserFactor;"
"AlphaBlendOperator[7] = MAX;"
"redblenduserfactor = 0.5;"
"RenderTargetWriteMask[0] = Gr;"

"BlendEnable[2] = true;"

"AlphaToCoverageEnable = true;"
"IndependentBlendEnable = true;"
;

        Shipyard::Array<Shipyard::ShaderOption> everyPossibleShaderOptionForShaderKey;
        Shipyard::ShaderKey::GetShaderKeyOptionsForShaderFamily(testShaderKey.GetShaderFamily(), everyPossibleShaderOptionForShaderKey);

        Shipyard::RenderStateBlock renderStateBlock;

        Shipyard::RenderStateBlockCompilationError renderStateBlockCompilationError = Shipyard::CompileRenderStateBlock(
                testShaderKey,
                everyPossibleShaderOptionForShaderKey,
                renderStateBlockSource,
                renderStateBlock);

        REQUIRE(renderStateBlockCompilationError == Shipyard::RenderStateBlockCompilationError::NoError);

        REQUIRE(renderStateBlock.rasterizerState.m_DepthBias == -5);
        REQUIRE(fabs(renderStateBlock.rasterizerState.m_DepthBiasClamp - 1.5f) < 0.0001f);
        REQUIRE(fabs(renderStateBlock.rasterizerState.m_SlopeScaledDepthBias - 2.25f) < 0.0001f);
        REQUIRE(renderStateBlock.rasterizerState.m_FillMode == Shipyard::FillMode::Wireframe);
        REQUIRE(renderStateBlock.rasterizerState.m_CullMode == Shipyard::CullMode::CullNone);
        REQUIRE(renderStateBlock.rasterizerState.m_IsFrontCounterClockwise == true);
        REQUIRE(renderStateBlock.rasterizerState.m_DepthClipEnable == true);
        REQUIRE(renderStateBlock.rasterizerState.m_ScissorEnable == true);
        REQUIRE(renderStateBlock.rasterizerState.m_MultisampleEnable == true);
        REQUIRE(renderStateBlock.rasterizerState.m_AntialiasedLineEnable == true);
        REQUIRE(renderStateBlock.depthStencilState.m_DepthComparisonFunc == Shipyard::ComparisonFunc::Greater);
        REQUIRE(renderStateBlock.depthStencilState.m_StencilReadMask == 0xff);
        REQUIRE(renderStateBlock.depthStencilState.m_StencilWriteMask == 0b11001100);
        REQUIRE(renderStateBlock.depthStencilState.m_FrontFaceStencilFailOp == Shipyard::StencilOperation::Keep);
        REQUIRE(renderStateBlock.depthStencilState.m_FrontFaceStencilDepthFailOp == Shipyard::StencilOperation::Zero);
        REQUIRE(renderStateBlock.depthStencilState.m_FrontFaceStencilPassOp == Shipyard::StencilOperation::Replace);
        REQUIRE(renderStateBlock.depthStencilState.m_FrontFaceStencilComparisonFunc == Shipyard::ComparisonFunc::Never);
        REQUIRE(renderStateBlock.depthStencilState.m_BackFaceStencilFailOp == Shipyard::StencilOperation::IncrSaturate);
        REQUIRE(renderStateBlock.depthStencilState.m_BackFaceStencilDepthFailOp == Shipyard::StencilOperation::DecrSaturate);
        REQUIRE(renderStateBlock.depthStencilState.m_BackFaceStencilPassOp == Shipyard::StencilOperation::Invert);
        REQUIRE(renderStateBlock.depthStencilState.m_BackFaceStencilComparisonFunc == Shipyard::ComparisonFunc::Equal);
        REQUIRE(renderStateBlock.depthStencilState.m_DepthEnable == false);
        REQUIRE(renderStateBlock.depthStencilState.m_EnableDepthWrite == false);
        REQUIRE(renderStateBlock.depthStencilState.m_StencilEnable == true);
        REQUIRE(renderStateBlock.blendState.renderTargetBlendStates[0].m_BlendEnable == true);
        REQUIRE(renderStateBlock.blendState.renderTargetBlendStates[0].m_SourceBlend == Shipyard::BlendFactor::SrcColor);
        REQUIRE(renderStateBlock.blendState.renderTargetBlendStates[0].m_DestBlend == Shipyard::BlendFactor::DestColor);
        REQUIRE(renderStateBlock.blendState.renderTargetBlendStates[0].m_BlendOperator == Shipyard::BlendOperator::Subtract);
        REQUIRE(renderStateBlock.blendState.renderTargetBlendStates[0].m_RenderTargetWriteMask == Shipyard::RenderTargetWriteMask(Shipyard::RenderTargetWriteMask::RenderTargetWriteMask_R | Shipyard::RenderTargetWriteMask::RenderTargetWriteMask_G));
        REQUIRE(renderStateBlock.blendState.renderTargetBlendStates[2].m_BlendEnable == true);
        REQUIRE(renderStateBlock.blendState.renderTargetBlendStates[1].m_SourceAlphaBlend == Shipyard::BlendFactor::InvDestAlpha);
        REQUIRE(renderStateBlock.blendState.renderTargetBlendStates[3].m_DestAlphaBlend == Shipyard::BlendFactor::UserFactor);
        REQUIRE(renderStateBlock.blendState.renderTargetBlendStates[7].m_AlphaBlendOperator == Shipyard::BlendOperator::Max);
        REQUIRE(renderStateBlock.blendState.m_AlphaToCoverageEnable == true);
        REQUIRE(renderStateBlock.blendState.m_IndependentBlendEnable == true);
        REQUIRE(Shipyard::IsAlmostEqual(renderStateBlock.blendState.m_RedBlendUserFactor, 0.5f));
        REQUIRE(Shipyard::IsAlmostEqual(renderStateBlock.blendState.m_AlphaBlendUserFactor, 1.0f));
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

        Shipyard::RenderStateBlock renderStateBlock;

        Shipyard::RenderStateBlockCompilationError renderStateBlockCompilationError = Shipyard::CompileRenderStateBlock(
                testShaderKey,
                everyPossibleShaderOptionForShaderKey,
                renderStateBlockSource,
                renderStateBlock);

        REQUIRE(renderStateBlockCompilationError == Shipyard::RenderStateBlockCompilationError::NoError);

        REQUIRE(renderStateBlock.rasterizerState.m_DepthBias == -5);
        REQUIRE(fabs(renderStateBlock.rasterizerState.m_DepthBiasClamp - 1.5f) < 0.0001f);
        REQUIRE(fabs(renderStateBlock.rasterizerState.m_SlopeScaledDepthBias - 2.25f) < 0.0001f);
        REQUIRE(renderStateBlock.rasterizerState.m_FillMode == Shipyard::FillMode::Wireframe);
        REQUIRE(renderStateBlock.rasterizerState.m_CullMode == Shipyard::CullMode::CullFrontFace);
        REQUIRE(renderStateBlock.rasterizerState.m_IsFrontCounterClockwise == true);
        REQUIRE(renderStateBlock.rasterizerState.m_DepthClipEnable == true);
        REQUIRE(renderStateBlock.rasterizerState.m_ScissorEnable == true);
        REQUIRE(renderStateBlock.rasterizerState.m_MultisampleEnable == true);
        REQUIRE(renderStateBlock.rasterizerState.m_AntialiasedLineEnable == true);
        REQUIRE(renderStateBlock.depthStencilState.m_DepthComparisonFunc == Shipyard::ComparisonFunc::Greater);
        REQUIRE(renderStateBlock.depthStencilState.m_StencilReadMask == 0);
        REQUIRE(renderStateBlock.depthStencilState.m_StencilWriteMask == 0);
        REQUIRE(renderStateBlock.depthStencilState.m_FrontFaceStencilFailOp == Shipyard::StencilOperation::Keep);
        REQUIRE(renderStateBlock.depthStencilState.m_FrontFaceStencilDepthFailOp == Shipyard::StencilOperation::Keep);
        REQUIRE(renderStateBlock.depthStencilState.m_FrontFaceStencilPassOp == Shipyard::StencilOperation::Keep);
        REQUIRE(renderStateBlock.depthStencilState.m_FrontFaceStencilComparisonFunc == Shipyard::ComparisonFunc::Always);
        REQUIRE(renderStateBlock.depthStencilState.m_BackFaceStencilFailOp == Shipyard::StencilOperation::Keep);
        REQUIRE(renderStateBlock.depthStencilState.m_BackFaceStencilDepthFailOp == Shipyard::StencilOperation::Keep);
        REQUIRE(renderStateBlock.depthStencilState.m_BackFaceStencilPassOp == Shipyard::StencilOperation::Keep);
        REQUIRE(renderStateBlock.depthStencilState.m_BackFaceStencilComparisonFunc == Shipyard::ComparisonFunc::Always);
        REQUIRE(renderStateBlock.depthStencilState.m_DepthEnable == false);
        REQUIRE(renderStateBlock.depthStencilState.m_EnableDepthWrite == true);
        REQUIRE(renderStateBlock.depthStencilState.m_StencilEnable == false);
    }
}