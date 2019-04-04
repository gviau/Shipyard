#include <graphics/wrapper/dx11/dx11renderdevicecontext.h>

#include <graphics/wrapper/dx11/dx11_common.h>
#include <graphics/wrapper/dx11/dx11buffer.h>
#include <graphics/wrapper/dx11/dx11descriptorset.h>
#include <graphics/wrapper/dx11/dx11pipelinestateobject.h>
#include <graphics/wrapper/dx11/dx11renderdevice.h>
#include <graphics/wrapper/dx11/dx11rendertarget.h>
#include <graphics/wrapper/dx11/dx11rootsignature.h>
#include <graphics/wrapper/dx11/dx11shader.h>
#include <graphics/wrapper/dx11/dx11texture.h>

#include <graphics/shaderhandler.h>
#include <graphics/vertexformat.h>

#include <system/array.h>
#include <system/logger.h>
#include <system/systemcommon.h>

#pragma warning( disable : 4005 )

#include <d3d11.h>
#include <d3dcompiler.h>

#pragma warning( default : 4005 )

namespace Shipyard
{;

// Used to not have to constantly create new input layouts
ID3D11InputLayout* g_RegisteredInputLayouts[uint32_t(VertexFormatType::VertexFormatType_Count)];

ID3D11InputLayout* RegisterVertexFormatType(ID3D11Device* device, VertexFormatType vertexFormatType);

DX11RenderDeviceContext::DX11RenderDeviceContext(GFXRenderDevice& renderDevice)
    : RenderDeviceContext(renderDevice)
    , m_Device(renderDevice.GetDevice())
    , m_DeviceContext(renderDevice.GetImmediateDeviceContext())
    , m_DepthStencilView(nullptr)
    , m_RenderStateCache(m_Device, m_DeviceContext)
{
    for (int i = 0; i < 8; i++)
    {
        m_RenderTargets[i] = nullptr;
    }

    for (uint32_t i = 0; i < uint32_t(VertexFormatType::VertexFormatType_Count); i++)
    {
        g_RegisteredInputLayouts[i] = RegisterVertexFormatType(m_Device, VertexFormatType(i));
    }
}

DX11RenderDeviceContext::~DX11RenderDeviceContext()
{
    for (uint32_t i = 0; i < uint32_t(VertexFormatType::VertexFormatType_Count); i++)
    {
        if (g_RegisteredInputLayouts[i] != nullptr)
        {
            g_RegisteredInputLayouts[i]->Release();
        }
    }
}

void DX11RenderDeviceContext::ClearFullRenderTarget(GFXRenderTargetHandle gfxRenderTargetHandle, float red, float green, float blue, float alpha)
{
    SHIP_ASSERT(gfxRenderTargetHandle.handle != InvalidGfxHandle);

    const GFXRenderTarget& gfxRenderTarget = m_RenderDevice.GetRenderTarget(gfxRenderTargetHandle);

    ID3D11RenderTargetView* const * renderTargetViews = gfxRenderTarget.GetRenderTargetViews();

    float colorRGBA[] =
    {
        red,
        green,
        blue,
        alpha
    };

    for (uint32_t i = 0; i < GfxConstants::GfxConstants_MaxRenderTargetsBound; i++)
    {
        if (renderTargetViews[i] == nullptr)
        {
            continue;
        }

        m_DeviceContext->ClearRenderTargetView(renderTargetViews[i], colorRGBA);
    }
}

void DX11RenderDeviceContext::ClearSingleRenderTarget(GFXRenderTargetHandle gfxRenderTargetHandle, uint32_t renderTargetIndex, float red, float green, float blue, float alpha)
{
    SHIP_ASSERT(gfxRenderTargetHandle.handle != InvalidGfxHandle);
    SHIP_ASSERT(renderTargetIndex < GfxConstants::GfxConstants_MaxRenderTargetsBound);

    const GFXRenderTarget& gfxRenderTarget = m_RenderDevice.GetRenderTarget(gfxRenderTargetHandle);

    ID3D11RenderTargetView* const * renderTargetViews = gfxRenderTarget.GetRenderTargetViews();

    float colorRGBA[] =
    {
        red,
        green,
        blue,
        alpha
    };

    if (renderTargetViews[renderTargetIndex] == nullptr)
    {
        return;
    }

    m_DeviceContext->ClearRenderTargetView(renderTargetViews[renderTargetIndex], colorRGBA);
}

void DX11RenderDeviceContext::ClearDepthStencilRenderTarget(GFXDepthStencilRenderTargetHandle gfxDepthStencilRenderTargetHandle, DepthStencilClearFlag depthStencilClearFlag, float depthValue, uint8_t stencilValue)
{
    SHIP_ASSERT(gfxDepthStencilRenderTargetHandle.handle != InvalidGfxHandle);

    const GFXDepthStencilRenderTarget& gfxDepthStencilRenderTarget = m_RenderDevice.GetDepthStencilRenderTarget(gfxDepthStencilRenderTargetHandle);

    ID3D11DepthStencilView* const depthStencilView = gfxDepthStencilRenderTarget.GetDepthStencilView();

    uint clearFlag = ((depthStencilClearFlag == DepthStencilClearFlag::Depth) ? D3D11_CLEAR_DEPTH :
                      (depthStencilClearFlag == DepthStencilClearFlag::Stencil) ? D3D11_CLEAR_STENCIL :
                      (D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL));

    m_DeviceContext->ClearDepthStencilView(depthStencilView, clearFlag, depthValue, stencilValue);
}

void DX11RenderDeviceContext::PrepareNextDrawCalls(const DrawItem& drawItem, VertexFormatType vertexFormatType)
{
    const GFXRootSignature& gfxRootSignature = m_RenderDevice.GetRootSignature(drawItem.rootSignatureHandle);
    
    const GFXDescriptorSet& gfxDescriptorSet = m_RenderDevice.GetDescriptorSet(drawItem.descriptorSetHandle);

    PipelineStateObjectCreationParameters pipelineStateObjectCreationParameters;
    pipelineStateObjectCreationParameters.rootSignature = &gfxRootSignature;

    drawItem.shaderHandler.ApplyShader(pipelineStateObjectCreationParameters);

    pipelineStateObjectCreationParameters.primitiveTopology = drawItem.primitiveTopology;
    pipelineStateObjectCreationParameters.vertexFormatType = vertexFormatType;

    if (drawItem.renderTargetHandle.handle == InvalidGfxHandle)
    {
        pipelineStateObjectCreationParameters.numRenderTargets = 0;
    }
    else
    {
        const GFXRenderTarget& gfxRenderTarget = m_RenderDevice.GetRenderTarget(drawItem.renderTargetHandle);

        pipelineStateObjectCreationParameters.numRenderTargets = gfxRenderTarget.GetNumRenderTargetsAttached();

        GfxFormat const * renderTargetFormats = gfxRenderTarget.GetRenderTargetFormats();

        memcpy(&pipelineStateObjectCreationParameters.renderTargetsFormat[0], &renderTargetFormats[0], pipelineStateObjectCreationParameters.numRenderTargets);

        m_RenderStateCache.BindRenderTarget(gfxRenderTarget);
    }

    if (drawItem.depthStencilRenderTargetHandle.handle != InvalidGfxHandle)
    {
        const GFXDepthStencilRenderTarget& gfxDepthStencilRenderTarget = m_RenderDevice.GetDepthStencilRenderTarget(drawItem.depthStencilRenderTargetHandle);
        
        pipelineStateObjectCreationParameters.depthStencilFormat = gfxDepthStencilRenderTarget.GetDepthStencilFormat();

        m_RenderStateCache.BindDepthStencilRenderTarget(gfxDepthStencilRenderTarget);
    }

    // Apply override, if any.
    if (drawItem.pRenderStateBlockStateOverride != nullptr)
    {
        drawItem.pRenderStateBlockStateOverride->ApplyOverridenValues(pipelineStateObjectCreationParameters.renderStateBlock);
    }

    GFXPipelineStateObject gfxPipelineStateObject;
    gfxPipelineStateObject.Create(pipelineStateObjectCreationParameters);

    m_RenderStateCache.BindRootSignature(gfxRootSignature);
    m_RenderStateCache.BindPipelineStateObject(gfxPipelineStateObject);
    m_RenderStateCache.BindDescriptorSet(gfxDescriptorSet, gfxRootSignature);

    m_RenderStateCache.SetViewport(drawItem.viewport);
}

void DX11RenderDeviceContext::Draw(
        const DrawItem& drawItem,
        GFXVertexBufferHandle* gfxVertexBufferHandles,
        uint32_t startSlot,
        uint32_t numVertexBuffers,
        uint32_t startVertexLocation,
        uint32_t* vertexBufferOffsets)
{
    VertexFormatType vertexFormatType = ((numVertexBuffers > 0) ? m_RenderDevice.GetVertexBuffer(gfxVertexBufferHandles[startSlot]).GetVertexFormatType() : VertexFormatType::VertexFormatType_Count);
    PrepareNextDrawCalls(drawItem, vertexFormatType);

    SHIP_ASSERT(numVertexBuffers < GfxConstants_MaxVertexBuffers);
    GFXVertexBuffer* gfxVertexBuffers[GfxConstants_MaxVertexBuffers];
    for (uint32_t i = startSlot; i < (startSlot + numVertexBuffers); i++)
    {
        gfxVertexBuffers[i] = &m_RenderDevice.GetVertexBuffer(gfxVertexBufferHandles[i]);
    }

    m_RenderStateCache.SetVertexBuffers(gfxVertexBuffers, startSlot, numVertexBuffers, vertexBufferOffsets);

    m_RenderStateCache.CommitStateChangesForGraphics(m_RenderDevice);

    uint32_t numVertices = ((numVertexBuffers > 0) ? m_RenderDevice.GetVertexBuffer(gfxVertexBufferHandles[startSlot]).GetNumVertices() : 1);
    m_DeviceContext->Draw(numVertices, startVertexLocation);
}

void DX11RenderDeviceContext::DrawIndexed(
        const DrawItem& drawItem,
        GFXVertexBufferHandle* gfxVertexBufferHandles,
        uint32_t startSlot,
        uint32_t numVertexBuffers,
        uint32_t* vertexBufferOffsets,
        GFXIndexBufferHandle gfxIndexBufferHandle,
        uint32_t startVertexLocation,
        uint32_t startIndexLocation,
        uint32_t indexBufferOffset)
{
    SHIP_ASSERT(gfxIndexBufferHandle.handle != InvalidGfxHandle);
    const GFXIndexBuffer& gfxIndexBuffer = m_RenderDevice.GetIndexBuffer(gfxIndexBufferHandle);

    m_RenderStateCache.SetIndexBuffer(gfxIndexBuffer, indexBufferOffset);

    VertexFormatType vertexFormatType = ((numVertexBuffers > 0) ? m_RenderDevice.GetVertexBuffer(gfxVertexBufferHandles[startSlot]).GetVertexFormatType() : VertexFormatType::VertexFormatType_Count);
    PrepareNextDrawCalls(drawItem, vertexFormatType);

    SHIP_ASSERT(numVertexBuffers < GfxConstants_MaxVertexBuffers);
    GFXVertexBuffer* gfxVertexBuffers[GfxConstants_MaxVertexBuffers];
    for (uint32_t i = startSlot; i < (startSlot + numVertexBuffers); i++)
    {
        gfxVertexBuffers[i] = &m_RenderDevice.GetVertexBuffer(gfxVertexBufferHandles[i]);
    }

    m_RenderStateCache.SetVertexBuffers(gfxVertexBuffers, startSlot, numVertexBuffers, vertexBufferOffsets);

    m_RenderStateCache.CommitStateChangesForGraphics(m_RenderDevice);

    m_DeviceContext->DrawIndexed(gfxIndexBuffer.GetNumIndices(), startIndexLocation, startVertexLocation);
}

ID3D11InputLayout* RegisterVertexFormatType(ID3D11Device* device, VertexFormatType vertexFormatType)
{
    static_assert(uint32_t(VertexFormatType::VertexFormatType_Count) == 5, "Update the RegisterVertexFormatType function if you add or remove vertex formats");

    uint32_t idx = uint32_t(vertexFormatType);
    SHIP_ASSERT(g_RegisteredInputLayouts[idx] == nullptr);

    // Create a dummy shader just to validate the input layout
    StringA dummyShaderSource = "struct VS_INPUT {\nfloat3 position : POSITION;\n";

    if (VertexFormatTypeContainsUV(vertexFormatType))
    {
        dummyShaderSource += "float2 uv : TEXCOORD0;\n";
    }

    if (VertexFormatTypeContainsNormals(vertexFormatType))
    {
        dummyShaderSource += "float3 normal : NORMAL;\n";
    }

    if (VertexFormatTypeContainsColor(vertexFormatType))
    {
        dummyShaderSource += "float3 color : COLOR;\n";
    }

    dummyShaderSource += "};\n"

        "struct VS_OUTPUT {\n"
        "float4 position : SV_POSITION;\n"
        "};\n"

        "VS_OUTPUT main(VS_INPUT input) {\n"
        "VS_OUTPUT output = (VS_OUTPUT)0;\n"
        "output.position = float4(input.position, 1.0);\n"
        "return output;\n"
        "}\n";

    ID3D10Blob* shaderBlob = nullptr;
    ID3D10Blob* error = nullptr;

    D3D_FEATURE_LEVEL featureLevel = device->GetFeatureLevel();
    StringA shaderVersion = GetD3DShaderVersion(featureLevel);

    StringA version = ("vs_" + shaderVersion);
    uint32_t flags = 0;

#ifdef _DEBUG
    flags = D3DCOMPILE_DEBUG;
#endif

    HRESULT hr = D3DCompile(dummyShaderSource.GetBuffer(), dummyShaderSource.Size(), nullptr, nullptr, nullptr, "main", version.GetBuffer(), flags, 0, &shaderBlob, &error);
    if (FAILED(hr))
    {
        if (error != nullptr)
        {
            char* errorMsg = (char*)error->GetBufferPointer();
            OutputDebugString(errorMsg);

            error->Release();
        }

        SHIP_LOG_ERROR("RegisterVertexFormatType() --> Couldn't compile dummy shader for registering vertex layout.");

        return nullptr;
    }

    ID3D11VertexShader* shader = nullptr;
    bool success = true;
    hr = device->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &shader);
    if (FAILED(hr))
    {
        success = false;

        SHIP_LOG_ERROR("RegisterVertexFormatType() --> Couldn't create dummy vertex shader for registering vertex layout.");
    }

    if (error != nullptr)
    {
        error->Release();
    }

    if (!success)
    {
        shader->Release();
        return nullptr;
    }

    VertexFormat* vertexFormat = nullptr;
    GetVertexFormat(vertexFormatType, vertexFormat);

    const InputLayout* inputLayouts = vertexFormat->GetInputLayouts();
    uint32_t numInputLayouts = vertexFormat->GetNumInputLayouts();

    Array<D3D11_INPUT_ELEMENT_DESC> inputElements;
    for (uint32_t i = 0; i < numInputLayouts; i++)
    {
        const InputLayout& inputLayout = inputLayouts[i];

        D3D11_INPUT_ELEMENT_DESC inputElement;
        inputElement.SemanticName = ConvertShipyardSemanticNameToDX11(inputLayout.m_SemanticName);
        inputElement.SemanticIndex = inputLayout.m_SemanticIndex;
        inputElement.Format = ConvertShipyardFormatToDX11(inputLayout.m_Format);
        inputElement.InputSlot = inputLayout.m_InputSlot;
        inputElement.AlignedByteOffset = inputLayout.m_ByteOffset;
        inputElement.InputSlotClass = (inputLayout.m_IsDataPerInstance) ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
        inputElement.InstanceDataStepRate = inputLayout.m_InstanceDataStepRate;

        inputElements.Add(inputElement);
    }

    ID3D11InputLayout* inputLayout = nullptr;
    hr = device->CreateInputLayout(&inputElements[0], inputElements.Size(), shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), &inputLayout);
    if (FAILED(hr))
    {
        SHIP_LOG_ERROR("RegisterVertexFormatType() --> Couldn't create input layout.");

        shader->Release();
        return nullptr;
    }

    shader->Release();
    return inputLayout;
}

}