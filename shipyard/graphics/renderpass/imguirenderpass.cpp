#include <graphics/renderpass/imguirenderpass.h>

#include <graphics/defaulttextures.h>
#include <graphics/rendercontext.h>

#include <graphics/shader/shaderfamilies.h>
#include <graphics/shader/shaderhandler.h>
#include <graphics/shader/shaderhandlermanager.h>
#include <graphics/shader/shaderkey.h>

#include <graphics/wrapper/wrapper.h>

#include <extern/imgui/imgui.h>

#include <extern/glm/gtc/matrix_transform.hpp>

namespace Shipyard
{;

SHIP_DECLARE_SHADER_INPUT_PROVIDER_BEGIN(ImGuiShaderInputProvider, Default)
{
    SHIP_SCALAR_SHADER_INPUT("ImGuiOrthographicProjectionMatrix", OrthographicProjectionMatrix);
    SHIP_TEXTURE2D_SHADER_INPUT(ShaderInputScalarType::Float4, "ImGuiTexture", ImGuiTexture);
}
SHIP_DECLARE_SHADER_INPUT_PROVIDER_END(ImGuiShaderInputProvider)

void ImGuiRenderPass::Execute(RenderContext& renderContext)
{
    static GFXVertexBufferHandle gfxVertexBufferHandle;
    static GFXIndexBufferHandle gfxIndexBufferHandle;
    static shipInt32 vertexBufferSize = 0;
    static shipInt32 indexBufferSize = 0;

    static bool show_demo_window = true;
    ImGui::ShowDemoWindow(&show_demo_window);

    ImGui::Render();

    GFXRenderDevice& gfxRenderDevice = *renderContext.GetRenderDevice();
    GFXDirectRenderCommandList& gfxRenderCommandList = *renderContext.GetRenderCommandList();
    const GFXViewSurface& gfxViewSurface = *renderContext.GetViewSurface();

    ImDrawData* imguiDrawData = ImGui::GetDrawData();

    // Create and grow vertex/index buffers if needed
    if (!gfxVertexBufferHandle.IsValid() || vertexBufferSize < imguiDrawData->TotalVtxCount)
    {
        if (gfxVertexBufferHandle.IsValid())
        {
            gfxRenderDevice.DestroyVertexBuffer(gfxVertexBufferHandle);
        }

        vertexBufferSize = imguiDrawData->TotalVtxCount + 5000;

        constexpr shipBool dynamic = true;
        constexpr void* initialData = nullptr;
        gfxVertexBufferHandle = gfxRenderDevice.CreateVertexBuffer(shipUint32(vertexBufferSize), VertexFormatType::ImGui, dynamic, initialData);
    }

    if (!gfxIndexBufferHandle.IsValid() || indexBufferSize < imguiDrawData->TotalIdxCount)
    {
        if (gfxIndexBufferHandle.IsValid())
        {
            gfxRenderDevice.DestroyIndexBuffer(gfxIndexBufferHandle);
        }

        indexBufferSize = imguiDrawData->TotalIdxCount + 10000;

        constexpr shipBool use2BytesPerIndex = (sizeof(ImDrawIdx) == 2);
        constexpr shipBool dynamic = true;
        constexpr void* initialData = nullptr;
        gfxIndexBufferHandle = gfxRenderDevice.CreateIndexBuffer(shipUint32(indexBufferSize), use2BytesPerIndex, dynamic, initialData);
    }

    // Upload vertex/index data into a single contiguous GPU buffer
    constexpr shipUint32 bufferOffset = 0;
    ImDrawVert* vertexDest = reinterpret_cast<ImDrawVert*>(gfxRenderCommandList.MapVertexBuffer(gfxVertexBufferHandle, MapFlag::Write_Discard, bufferOffset));
    ImDrawIdx* indexDest = reinterpret_cast<ImDrawIdx*>(gfxRenderCommandList.MapIndexBuffer(gfxIndexBufferHandle, MapFlag::Write_Discard, bufferOffset));

    for (int n = 0; n < imguiDrawData->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = imguiDrawData->CmdLists[n];
        memcpy(vertexDest, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
        memcpy(indexDest, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
        vertexDest += cmd_list->VtxBuffer.Size;
        indexDest += cmd_list->IdxBuffer.Size;
    }

    // Setup orthographic projection matrix into our constant buffer
    // Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
    m_ImGuiShaderInputProvider.OrthographicProjectionMatrix = glm::orthoLH(
            imguiDrawData->DisplayPos.x,
            imguiDrawData->DisplayPos.x + imguiDrawData->DisplaySize.x,
            imguiDrawData->DisplayPos.y + imguiDrawData->DisplaySize.y,
            imguiDrawData->DisplayPos.y,
            0.0f, 1.0f);

    m_ImGuiShaderInputProvider.OrthographicProjectionMatrix[2][2] = 0.5f;
    m_ImGuiShaderInputProvider.OrthographicProjectionMatrix[3][2] = 0.5f;

    InplaceArray<ShaderInputProvider*, 1> imguiShaderInputProviders;
    imguiShaderInputProviders.Add(&m_ImGuiShaderInputProvider);

    ShaderKey imguiShaderKey;
    imguiShaderKey.SetShaderFamily(ShaderFamily::ImGui);

    ShaderHandler* imguiShaderHandler = ShaderHandlerManager::GetInstance().GetShaderHandlerForShaderKey(imguiShaderKey);

    ShaderHandler::RenderState imguiRenderState;
    imguiRenderState.GfxRenderTargetHandle = gfxViewSurface.GetBackBufferRenderTargetHandle();
    imguiRenderState.GfxDepthStencilRenderTargetHandle = { InvalidGfxHandle };
    imguiRenderState.PrimitiveTopologyToRender = PrimitiveTopology::TriangleList;
    imguiRenderState.VertexFormatTypeToRender = VertexFormatType::ImGui;
    imguiRenderState.OptionalRenderStateBlockStateOverride = nullptr;

    ShaderHandler::ShaderRenderElements imguiShaderRenderElements = imguiShaderHandler->GetShaderRenderElements(gfxRenderDevice, imguiRenderState);

    // Render command lists
    // (Because we merged all buffers into a single one, we maintain our own offset into them)
    int global_idx_offset = 0;
    int global_vtx_offset = 0;
    ImVec2 clip_off = imguiDrawData->DisplayPos;
    for (int n = 0; n < imguiDrawData->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = imguiDrawData->CmdLists[n];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback != NULL)
            {
                // User callback, registered via ImDrawList::AddCallback()
                // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                if (pcmd->UserCallback != ImDrawCallback_ResetRenderState)
                {
                    pcmd->UserCallback(cmd_list, pcmd);
                }
            }
            else
            {
                GfxViewport gfxViewport;
                gfxViewport.topLeftX = 0.0f;
                gfxViewport.topLeftY = 0.0f;
                gfxViewport.width = imguiDrawData->DisplaySize.x;
                gfxViewport.height = imguiDrawData->DisplaySize.y;
                gfxViewport.minDepth = 0.0f;
                gfxViewport.maxDepth = 1.0f;

                // Apply scissor/clipping rectangle
                GfxRect scissorRect =
                {
                    (shipInt32)(pcmd->ClipRect.x - clip_off.x),
                    (shipInt32)(pcmd->ClipRect.y - clip_off.y),
                    (shipInt32)(pcmd->ClipRect.z - clip_off.x),
                    (shipInt32)(pcmd->ClipRect.w - clip_off.y)
                };

                // Bind texture, Draw
                GFXTexture2DHandle* textureToBind = (GFXTexture2DHandle*)pcmd->TextureId;
                m_ImGuiShaderInputProvider.ImGuiTexture = ((textureToBind != nullptr && textureToBind->IsValid()) ? *textureToBind : DefaultTextures::WhiteTexture);

                imguiShaderHandler->ApplyShaderInputProviders(gfxRenderDevice, gfxRenderCommandList, imguiShaderInputProviders);

                DrawIndexedCommand* drawIndexedCommand = gfxRenderCommandList.DrawIndexed();
                drawIndexedCommand->gfxRootSignatureHandle = imguiShaderRenderElements.GfxRootSignatureHandle;
                drawIndexedCommand->gfxPipelineStateObjectHandle = imguiShaderRenderElements.GfxPipelineStateObjectHandle;
                drawIndexedCommand->gfxDescriptorSetHandle = imguiShaderRenderElements.GfxDescriptorSetHandle;
                drawIndexedCommand->gfxRenderTargetHandle = gfxViewSurface.GetBackBufferRenderTargetHandle();
                drawIndexedCommand->gfxDepthStencilRenderTargetHandle = { InvalidGfxHandle };
                drawIndexedCommand->gfxViewport = gfxViewport;
                drawIndexedCommand->gfxScissorRect = scissorRect;

                drawIndexedCommand->numVertexBuffers = 1;
                drawIndexedCommand->pGfxVertexBufferHandles = &gfxVertexBufferHandle;

                shipUint32 vertexBufferOffset = 0;
                drawIndexedCommand->pVertexBufferOffsets = &vertexBufferOffset;

                drawIndexedCommand->gfxIndexBufferHandle = gfxIndexBufferHandle;
                drawIndexedCommand->indexCount = pcmd->ElemCount;
                drawIndexedCommand->startIndexLocation = pcmd->IdxOffset + global_idx_offset;
                drawIndexedCommand->baseVertexLocation = pcmd->VtxOffset + global_vtx_offset;
                drawIndexedCommand->indexBufferOffset = 0;
            }
        }
        global_idx_offset += cmd_list->IdxBuffer.Size;
        global_vtx_offset += cmd_list->VtxBuffer.Size;
    }
}

}