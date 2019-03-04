#include <graphics/wrapper/dx11/dx11shader.h>

#include <graphics/wrapper/dx11/dx11_common.h>

#include <system/logger.h>

#pragma warning( disable : 4005 )

#include <d3d11.h>
#include <d3dcompiler.h>

#pragma warning( default : 4005 )

namespace Shipyard
{;

DX11BaseShader::DX11BaseShader()
    : m_ShaderBlob(nullptr)
{
}

DX11BaseShader::~DX11BaseShader()
{
    if (m_ShaderBlob != nullptr)
    {
        m_ShaderBlob->Release();
    }
}

DX11VertexShader::DX11VertexShader(ID3D11Device& device, void* shaderData, uint64_t shaderDataSize)
    : VertexShader(shaderData, shaderDataSize)
    , m_VertexShader(nullptr)
{
    HRESULT hr = device.CreateVertexShader(shaderData, SIZE_T(shaderDataSize), nullptr, &m_VertexShader);
    if (FAILED(hr))
    {
        SHIP_LOG_ERROR("DX11VertexShader::DX11VertexShader() --> Couldn't create vertex shader.");
    }
}

DX11VertexShader::~DX11VertexShader()
{
    if (m_VertexShader != nullptr)
    {
        m_VertexShader->Release();
    }
}

DX11PixelShader::DX11PixelShader(ID3D11Device& device, void* shaderData, uint64_t shaderDataSize)
    : PixelShader(shaderData, shaderDataSize)
    , m_PixelShader(nullptr)
{
    HRESULT hr = device.CreatePixelShader(shaderData, SIZE_T(shaderDataSize), nullptr, &m_PixelShader);
    if (FAILED(hr))
    {
        SHIP_LOG_ERROR("DX11PixelShader::DX11PixelShader() --> Couldn't create vertex shader.");
    }
}

DX11PixelShader::~DX11PixelShader()
{
    if (m_PixelShader != nullptr)
    {
        m_PixelShader->Release();
    }
}

}