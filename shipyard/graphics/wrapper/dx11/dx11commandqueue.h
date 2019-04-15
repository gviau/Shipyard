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

        bool Create();
        void Destroy();

        void ExecuteCommandLists(GFXRenderCommandList** ppRenderCommandLists, uint32_t numRenderCommandLists);

    private:
        struct DrawItem
        {
            DrawItem(
                    GFXRenderTargetHandle gfxRenderTargetHandle,
                    GFXDepthStencilRenderTargetHandle gfxDepthStencilRenderTargetHandle,
                    const GfxViewport& gfxViewport,
                    GFXRootSignatureHandle gfxRootSignatureHandle,
                    GFXDescriptorSetHandle gfxDescriptorSetHandle,
                    const ShaderHandler& shaderHandlerToUse,
                    PrimitiveTopology primitiveTopologyToUse,
                    const RenderStateBlockStateOverride* pRenderStateBlockStateOverrideToUse)
                : renderTargetHandle(gfxRenderTargetHandle)
                , depthStencilRenderTargetHandle(gfxDepthStencilRenderTargetHandle)
                , viewport(gfxViewport)
                , rootSignatureHandle(gfxRootSignatureHandle)
                , descriptorSetHandle(gfxDescriptorSetHandle)
                , shaderHandler(shaderHandlerToUse)
                , primitiveTopology(primitiveTopologyToUse)
                , pRenderStateBlockStateOverride(pRenderStateBlockStateOverrideToUse)
            {}

            GFXRenderTargetHandle renderTargetHandle;
            GFXDepthStencilRenderTargetHandle depthStencilRenderTargetHandle;

            const GfxViewport& viewport;

            GFXRootSignatureHandle rootSignatureHandle;
            GFXDescriptorSetHandle descriptorSetHandle;

            const ShaderHandler& shaderHandler;

            PrimitiveTopology primitiveTopology;

            const RenderStateBlockStateOverride* pRenderStateBlockStateOverride = nullptr;
        };

    private:
        size_t ClearFullRenderTarget(BaseRenderCommand* pCmd);
        size_t ClearSingleRenderTarget(BaseRenderCommand* pCmd);
        size_t ClearDepthStencilRenderTarget(BaseRenderCommand* pCmd);

        size_t Draw(BaseRenderCommand* pCmd);
        size_t DrawIndexed(BaseRenderCommand* pCmd);

        size_t MapBuffer(BaseRenderCommand* pCmd);

        void PrepareNextDrawCalls(const DrawItem& drawItem, VertexFormatType vertexFormatType);

        ID3D11Device* m_Device;
        ID3D11DeviceContext* m_DeviceContext;

        ID3D11RenderTargetView* m_RenderTargets[8];
        ID3D11DepthStencilView* m_DepthStencilView;

        DX11RenderStateCache m_RenderStateCache;
    };
}