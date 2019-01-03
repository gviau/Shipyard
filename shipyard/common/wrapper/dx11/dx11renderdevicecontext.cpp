#include <common/wrapper/dx11/dx11renderdevicecontext.h>

#include <common/wrapper/dx11/dx11_common.h>
#include <common/wrapper/dx11/dx11buffer.h>
#include <common/wrapper/dx11/dx11descriptorset.h>
#include <common/wrapper/dx11/dx11pipelinestateobject.h>
#include <common/wrapper/dx11/dx11renderdevice.h>
#include <common/wrapper/dx11/dx11rootsignature.h>
#include <common/wrapper/dx11/dx11shader.h>
#include <common/wrapper/dx11/dx11texture.h>

#include <common/vertexformat.h>

#include <system/array.h>

#pragma warning( disable : 4005 )

#include <d3d11.h>
#include <d3dcompiler.h>

#pragma warning( default : 4005 )

#include <cassert>

namespace Shipyard
{;

// Used to not have to constantly create new input layouts
ID3D11InputLayout* g_RegisteredInputLayouts[uint32_t(VertexFormatType::VertexFormatType_Count)];

ID3D11InputLayout* RegisterVertexFormatType(ID3D11Device* device, VertexFormatType vertexFormatType);

DX11RenderDeviceContext::DX11RenderDeviceContext(const GFXRenderDevice& renderDevice)
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

void DX11RenderDeviceContext::SetRenderTargetView(uint32_t renderTarget, ID3D11RenderTargetView* renderTargetView)
{
    assert(renderTarget < 8);

    m_RenderTargets[renderTarget] = renderTargetView;

    m_DeviceContext->OMSetRenderTargets(1, &renderTargetView, m_DepthStencilView);
}

void DX11RenderDeviceContext::SetDepthStencilView(ID3D11DepthStencilView* depthStencilView)
{
    m_DepthStencilView = depthStencilView;

    m_DeviceContext->OMSetRenderTargets(1, &m_RenderTargets[0], m_DepthStencilView);
}

void DX11RenderDeviceContext::ClearRenderTarget(float red, float green, float blue, float alpha, uint32_t renderTarget)
{
    assert(renderTarget < 8);

    ID3D11RenderTargetView* renderTargetView = m_RenderTargets[renderTarget];
    assert(renderTargetView != nullptr);

    float colorRGBA[] =
    {
        red,
        green,
        blue,
        alpha
    };

    m_DeviceContext->ClearRenderTargetView(renderTargetView, colorRGBA);
}

void DX11RenderDeviceContext::ClearDepthStencil(bool clearDepth, bool clearStencil)
{
    UINT clearFlag = (clearDepth ? D3D11_CLEAR_DEPTH : 0) | (clearStencil ? D3D11_CLEAR_STENCIL : 0);

    m_DeviceContext->ClearDepthStencilView(m_DepthStencilView, clearFlag, 1.0f, 0);
}

void DX11RenderDeviceContext::SetViewport(float topLeftX, float topLeftY, float width, float height)
{
    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = topLeftX;
    viewport.TopLeftY = topLeftY;
    viewport.Width = width;
    viewport.Height = height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    m_DeviceContext->RSSetViewports(1, &viewport);
}

void DX11RenderDeviceContext::PrepareNextDrawCalls(
        const GFXRootSignature& rootSignature,
        const GFXPipelineStateObject& pipelineStateObject,
        const GFXDescriptorSet& descriptorSet)
{
    m_RenderStateCache.BindRootSignature(rootSignature);
    m_RenderStateCache.BindPipelineStateObject(pipelineStateObject);
    m_RenderStateCache.BindDescriptorSet(descriptorSet, rootSignature);
}

void DX11RenderDeviceContext::Draw(PrimitiveTopology primitiveTopology, const GFXVertexBuffer& vertexBuffer, uint32_t startVertexLocation)
{
    m_RenderStateCache.CommitStateChangesForGraphics();

    D3D11_PRIMITIVE_TOPOLOGY topology = ConvertShipyardPrimitiveTopologyToDX11(primitiveTopology);
    m_DeviceContext->IASetPrimitiveTopology(topology);

    ID3D11Buffer* buffer = vertexBuffer.GetBuffer();

    VertexFormatType vertexFormatType = vertexBuffer.GetVertexFormatType();
    VertexFormat* vertexFormat = nullptr;

    GetVertexFormat(vertexFormatType, vertexFormat);

    uint32_t stride = vertexFormat->GetSize();
    uint32_t offset = 0;

    m_DeviceContext->IASetInputLayout(g_RegisteredInputLayouts[uint32_t(vertexFormatType)]);
    m_DeviceContext->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);

    m_DeviceContext->Draw(vertexBuffer.GetNumVertices(), startVertexLocation);
}

void DX11RenderDeviceContext::DrawIndexed(PrimitiveTopology primitiveTopology, const GFXVertexBuffer& vertexBuffer, const GFXIndexBuffer& indexBuffer, uint32_t startVertexLocation, uint32_t startIndexLocation)
{
    m_RenderStateCache.CommitStateChangesForGraphics();

    D3D11_PRIMITIVE_TOPOLOGY topology = ConvertShipyardPrimitiveTopologyToDX11(primitiveTopology);
    m_DeviceContext->IASetPrimitiveTopology(topology);

    ID3D11Buffer* d3dVertexBuffer = vertexBuffer.GetBuffer();
    ID3D11Buffer* d3dIndexBuffer = indexBuffer.GetBuffer();

    VertexFormatType vertexFormatType = vertexBuffer.GetVertexFormatType();
    VertexFormat* vertexFormat = nullptr;

    GetVertexFormat(vertexFormatType, vertexFormat);

    uint32_t stride = vertexFormat->GetSize();
    uint32_t offset = 0;

    m_DeviceContext->IASetInputLayout(g_RegisteredInputLayouts[uint32_t(vertexFormatType)]);
    m_DeviceContext->IASetVertexBuffers(0, 1, &d3dVertexBuffer, &stride, &offset);

    DXGI_FORMAT indexFormat = (indexBuffer.Uses2BytesPerIndex() ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT);
    m_DeviceContext->IASetIndexBuffer(d3dIndexBuffer, indexFormat, 0);

    m_DeviceContext->DrawIndexed(indexBuffer.GetNumIndices(), startIndexLocation, startVertexLocation);
}

ID3D11InputLayout* RegisterVertexFormatType(ID3D11Device* device, VertexFormatType vertexFormatType)
{
    static_assert(uint32_t(VertexFormatType::VertexFormatType_Count) == 5, "Update the RegisterVertexFormatType function if you add or remove vertex formats");

    uint32_t idx = uint32_t(vertexFormatType);
    assert(g_RegisteredInputLayouts[idx] == nullptr);

    // Create a dummy shader just to validate the input layout
    String dummyShaderSource = "struct VS_INPUT {\nfloat3 position : POSITION;\n";

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
    String shaderVersion = GetD3DShaderVersion(featureLevel);

    String version = ("vs_" + shaderVersion);
    uint32_t flags = 0;

#ifdef _DEBUG
    flags = D3DCOMPILE_DEBUG;
#endif

    HRESULT hr = D3DCompile(dummyShaderSource.c_str(), dummyShaderSource.size(), nullptr, nullptr, nullptr, "main", version.c_str(), flags, 0, &shaderBlob, &error);
    if (FAILED(hr))
    {
        if (error != nullptr)
        {
            char* errorMsg = (char*)error->GetBufferPointer();
            OutputDebugString(errorMsg);

            error->Release();
        }

        return nullptr;
    }

    ID3D11VertexShader* shader = nullptr;
    bool success = true;
    hr = device->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &shader);
    if (FAILED(hr))
    {
        success = false;
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

    const Array<InputLayout>& inputLayouts = vertexFormat->GetInputLayouts();

    Array<D3D11_INPUT_ELEMENT_DESC> inputElements;
    for (InputLayout inputLayout : inputLayouts)
    {
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
        shader->Release();
        return nullptr;
    }

    shader->Release();
    return inputLayout;
}

}