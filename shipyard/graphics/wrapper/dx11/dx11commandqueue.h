#pragma once

#include <graphics/wrapper/commandqueue.h>

#include <graphics/wrapper/dx11/dx11renderstatecache.h>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11DepthStencilState;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilView;
struct ID3D11RenderTargetView;

namespace Shipyard
{
    struct BaseRenderCommand;

    class SHIPYARD_API DX11CommandQueue : public CommandQueue
    {
    public:
        DX11CommandQueue(GFXRenderDevice& gfxRenderDevice, CommandQueueType commandQueueType);
        ~DX11CommandQueue();

        shipBool Create();
        void Destroy();

        void ExecuteCommandLists(GFXRenderCommandList** ppRenderCommandLists, shipUint32 numRenderCommandLists);

    private:
        struct DrawItem
        {
            DrawItem(
                    GFXRenderTargetHandle gfxRenderTargetHandle,
                    GFXDepthStencilRenderTargetHandle gfxDepthStencilRenderTargetHandle,
                    const GfxViewport& gfxViewport,
                    const GfxRect& gfxScissor,
                    GFXPipelineStateObjectHandle gfxPipelineStateObjectHandle,
                    GFXRootSignatureHandle gfxRootSignatureHandle,
                    GFXDescriptorSetHandle gfxDescriptorSetHandle)
                : renderTargetHandle(gfxRenderTargetHandle)
                , depthStencilRenderTargetHandle(gfxDepthStencilRenderTargetHandle)
                , viewport(gfxViewport)
                , scissor(gfxScissor)
                , pipelineStateObjetHandle(gfxPipelineStateObjectHandle)
                , rootSignatureHandle(gfxRootSignatureHandle)
                , descriptorSetHandle(gfxDescriptorSetHandle)
            {}

            GFXRenderTargetHandle renderTargetHandle;
            GFXDepthStencilRenderTargetHandle depthStencilRenderTargetHandle;

            const GfxViewport& viewport;
            const GfxRect& scissor;

            GFXPipelineStateObjectHandle pipelineStateObjetHandle;
            GFXRootSignatureHandle rootSignatureHandle;
            GFXDescriptorSetHandle descriptorSetHandle;
        };

    private:
        size_t ClearFullRenderTarget(BaseRenderCommand* pCmd);
        size_t ClearSingleRenderTarget(BaseRenderCommand* pCmd);
        size_t ClearDepthStencilRenderTarget(BaseRenderCommand* pCmd);

        size_t Draw(BaseRenderCommand* pCmd);
        size_t DrawSeveralVertexBuffers(BaseRenderCommand* pCmd);
        size_t DrawIndexed(BaseRenderCommand* pCmd);
        size_t DrawIndexedSeveralVertexBuffers(BaseRenderCommand* pCmd);

        size_t MapBuffer(BaseRenderCommand* pCmd);

        void PrepareNextDrawCalls(const DrawItem& drawItem);

        ID3D11Device* m_Device;
        ID3D11DeviceContext* m_DeviceContext;

        ID3D11RenderTargetView* m_RenderTargets[8];
        ID3D11DepthStencilView* m_DepthStencilView;

        DX11RenderStateCache m_RenderStateCache;
    };
}