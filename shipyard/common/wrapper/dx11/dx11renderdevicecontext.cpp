#include <common/wrapper/dx11/dx11renderdevicecontext.h>

#include <common/wrapper/dx11/dx11_common.h>
#include <common/wrapper/dx11/dx11buffer.h>
#include <common/wrapper/dx11/dx11renderdevice.h>
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
    , m_RasterizerState(nullptr)
    , m_DepthStencilState(nullptr)
    , m_DepthStencilView(nullptr)
{
    m_Device = renderDevice.GetDevice();
    m_ImmediateDeviceContext = renderDevice.GetImmediateDeviceContext();

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

    if (m_RasterizerState != nullptr)
    {
        m_RasterizerState->Release();
    }
}

void DX11RenderDeviceContext::SetRenderTargetView(uint32_t renderTarget, ID3D11RenderTargetView* renderTargetView)
{
    assert(renderTarget < 8);

    m_RenderTargets[renderTarget] = renderTargetView;

    m_ImmediateDeviceContext->OMSetRenderTargets(1, &renderTargetView, m_DepthStencilView);
}

void DX11RenderDeviceContext::SetDepthStencilView(ID3D11DepthStencilView* depthStencilView)
{
    m_DepthStencilView = depthStencilView;

    m_ImmediateDeviceContext->OMSetRenderTargets(1, &m_RenderTargets[0], m_DepthStencilView);
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

    m_ImmediateDeviceContext->ClearRenderTargetView(renderTargetView, colorRGBA);
}

void DX11RenderDeviceContext::ClearDepthStencil(bool clearDepth, bool clearStencil)
{
    UINT clearFlag = (clearDepth ? D3D11_CLEAR_DEPTH : 0) | (clearStencil ? D3D11_CLEAR_STENCIL : 0);

    m_ImmediateDeviceContext->ClearDepthStencilView(m_DepthStencilView, clearFlag, 1.0f, 0);
}

void DX11RenderDeviceContext::SetRasterizerState(const RasterizerState& rasterizerState)
{
    D3D11_RASTERIZER_DESC rasterizerDesc;
    rasterizerDesc.FillMode = ConvertShipyardFillModeToDX11(rasterizerState.m_FillMode);
    rasterizerDesc.CullMode = ConvertShipyardCullModeToDX11(rasterizerState.m_CullMode);
    rasterizerDesc.FrontCounterClockwise = rasterizerState.m_IsFrontCounterClockWise;
    rasterizerDesc.DepthBias = rasterizerState.m_DepthBias;
    rasterizerDesc.DepthBiasClamp = rasterizerState.m_DepthBiasClamp;
    rasterizerDesc.SlopeScaledDepthBias = rasterizerState.m_SlopeScaledDepthBias;
    rasterizerDesc.DepthClipEnable = rasterizerState.m_DepthClipEnable;
    rasterizerDesc.ScissorEnable = rasterizerState.m_ScissorEnable;
    rasterizerDesc.MultisampleEnable = rasterizerState.m_MultisampleEnable;
    rasterizerDesc.AntialiasedLineEnable = rasterizerState.m_AntialiasedLineEnable;

    if (m_RasterizerState != nullptr)
    {
        m_RasterizerState->Release();
        m_RasterizerState = nullptr;
    }

    HRESULT hr = m_Device->CreateRasterizerState(&rasterizerDesc, &m_RasterizerState);
    if (FAILED(hr))
    {
        MessageBox(NULL, "CreateRasterizerState failed", "DX11 error", MB_OK);
        return;
    }

    m_ImmediateDeviceContext->RSSetState(m_RasterizerState);
}

void DX11RenderDeviceContext::SetDepthStencilState(const DepthStencilState& depthStencilState, uint8_t stencilRef)
{
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
    depthStencilDesc.DepthEnable = depthStencilState.m_DepthEnable;
    depthStencilDesc.DepthWriteMask = (depthStencilState.m_EnableDepthWrite ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO);
    depthStencilDesc.DepthFunc = ConvertShipyardComparisonFuncToDX11(depthStencilState.m_DepthComparisonFunc);
    depthStencilDesc.StencilEnable = depthStencilState.m_StencilEnable;
    depthStencilDesc.StencilReadMask = depthStencilState.m_StencilReadMask;
    depthStencilDesc.StencilWriteMask = depthStencilState.m_StencilWriteMask;
    depthStencilDesc.FrontFace.StencilFailOp = ConvertShipyardStencilOperationToDX11(depthStencilState.m_FrontFaceStencilFailOp);
    depthStencilDesc.FrontFace.StencilDepthFailOp = ConvertShipyardStencilOperationToDX11(depthStencilState.m_FrontFaceStencilDepthFailOp);
    depthStencilDesc.FrontFace.StencilPassOp = ConvertShipyardStencilOperationToDX11(depthStencilState.m_FrontFaceStencilPassOp);
    depthStencilDesc.FrontFace.StencilFunc = ConvertShipyardComparisonFuncToDX11(depthStencilState.m_FrontFaceStencilComparisonFunc);
    depthStencilDesc.BackFace.StencilFailOp = ConvertShipyardStencilOperationToDX11(depthStencilState.m_BackFaceStencilFailOp);
    depthStencilDesc.BackFace.StencilDepthFailOp = ConvertShipyardStencilOperationToDX11(depthStencilState.m_BackFaceStencilDepthFailOp);
    depthStencilDesc.BackFace.StencilPassOp = ConvertShipyardStencilOperationToDX11(depthStencilState.m_BackFaceStencilPassOp);
    depthStencilDesc.BackFace.StencilFunc = ConvertShipyardComparisonFuncToDX11(depthStencilState.m_BackFaceStencilComparisonFunc);

    if (m_DepthStencilState != nullptr)
    {
        m_DepthStencilState->Release();
        m_DepthStencilState = nullptr;
    }

    HRESULT hr = m_Device->CreateDepthStencilState(&depthStencilDesc, &m_DepthStencilState);
    if (FAILED(hr))
    {
        MessageBox(NULL, "CreateDepthStencilState failed", "DX11 error", MB_OK);
        return;
    }

    m_ImmediateDeviceContext->OMSetDepthStencilState(m_DepthStencilState, stencilRef);

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

    m_ImmediateDeviceContext->RSSetViewports(1, &viewport);
}

void DX11RenderDeviceContext::SetVertexShader(GFXVertexShader* vertexShader)
{
    ID3D11VertexShader* shader = (vertexShader != nullptr) ? vertexShader->GetShader() : nullptr;
    m_ImmediateDeviceContext->VSSetShader(shader, nullptr, 0);
}

void DX11RenderDeviceContext::SetPixelShader(GFXPixelShader* pixelShader)
{
    ID3D11PixelShader* shader = (pixelShader != nullptr) ? pixelShader->GetShader() : nullptr;
    m_ImmediateDeviceContext->PSSetShader(shader, nullptr, 0);
}

void DX11RenderDeviceContext::SetVertexShaderConstantBuffer(GFXConstantBuffer* constantBuffer, uint32_t slot)
{
    ID3D11Buffer* buffer = (constantBuffer != nullptr) ? constantBuffer->GetBuffer() : nullptr;
    m_ImmediateDeviceContext->VSSetConstantBuffers(slot, 1, &buffer);
}

void DX11RenderDeviceContext::SetPixelShaderConstantBuffer(GFXConstantBuffer* constantBuffer, uint32_t slot)
{
    ID3D11Buffer* buffer = (constantBuffer != nullptr) ? constantBuffer->GetBuffer() : nullptr;
    m_ImmediateDeviceContext->PSSetConstantBuffers(slot, 1, &buffer);
}

void DX11RenderDeviceContext::SetPixelShaderTexture(GFXTexture2D* texture, uint32_t slot)
{
    ID3D11ShaderResourceView* shaderResourceView = (texture != nullptr) ? texture->GetShaderResourceView() : nullptr;
    m_ImmediateDeviceContext->PSSetShaderResources(slot, 1, &shaderResourceView);
}

void DX11RenderDeviceContext::Draw(PrimitiveTopology primitiveTopology, const GFXVertexBuffer& vertexBuffer, uint32_t startVertexLocation)
{
    D3D11_PRIMITIVE_TOPOLOGY topology = ConvertShipyardPrimitiveTopologyToDX11(primitiveTopology);
    m_ImmediateDeviceContext->IASetPrimitiveTopology(topology);

    ID3D11Buffer* buffer = vertexBuffer.GetBuffer();

    VertexFormatType vertexFormatType = vertexBuffer.GetVertexFormatType();
    VertexFormat* vertexFormat = nullptr;

    GetVertexFormat(vertexFormatType, vertexFormat);

    uint32_t stride = vertexFormat->GetSize();
    uint32_t offset = 0;

    m_ImmediateDeviceContext->IASetInputLayout(g_RegisteredInputLayouts[uint32_t(vertexFormatType)]);
    m_ImmediateDeviceContext->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);

    m_ImmediateDeviceContext->Draw(vertexBuffer.GetNumVertices(), startVertexLocation);
}

void DX11RenderDeviceContext::DrawIndexed(PrimitiveTopology primitiveTopology, const GFXVertexBuffer& vertexBuffer, const GFXIndexBuffer& indexBuffer, uint32_t startVertexLocation, uint32_t startIndexLocation)
{
    D3D11_PRIMITIVE_TOPOLOGY topology = ConvertShipyardPrimitiveTopologyToDX11(primitiveTopology);
    m_ImmediateDeviceContext->IASetPrimitiveTopology(topology);

    ID3D11Buffer* d3dVertexBuffer = vertexBuffer.GetBuffer();
    ID3D11Buffer* d3dIndexBuffer = indexBuffer.GetBuffer();

    VertexFormatType vertexFormatType = vertexBuffer.GetVertexFormatType();
    VertexFormat* vertexFormat = nullptr;

    GetVertexFormat(vertexFormatType, vertexFormat);

    uint32_t stride = vertexFormat->GetSize();
    uint32_t offset = 0;

    m_ImmediateDeviceContext->IASetInputLayout(g_RegisteredInputLayouts[uint32_t(vertexFormatType)]);
    m_ImmediateDeviceContext->IASetVertexBuffers(0, 1, &d3dVertexBuffer, &stride, &offset);

    DXGI_FORMAT indexFormat = (indexBuffer.Uses2BytesPerIndex() ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT);
    m_ImmediateDeviceContext->IASetIndexBuffer(d3dIndexBuffer, indexFormat, 0);

    m_ImmediateDeviceContext->DrawIndexed(indexBuffer.GetNumIndices(), startIndexLocation, startVertexLocation);
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