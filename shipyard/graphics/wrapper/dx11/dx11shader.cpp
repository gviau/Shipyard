#include <graphics/graphicsprecomp.h>

#include <graphics/wrapper/dx11/dx11shader.h>

#include <graphics/wrapper/dx11/dx11_common.h>

#include <system/logger.h>

#pragma warning( disable : 4005 )

#include <d3d11.h>
#include <d3dcompiler.h>

#pragma warning( default : 4005 )

namespace Shipyard
{;

DX11VertexShader::DX11VertexShader()
    : m_VertexShader(nullptr)
{

}

shipBool DX11VertexShader::Create(ID3D11Device& device, void* shaderData, shipUint64 shaderDataSize)
{
    HRESULT hr = device.CreateVertexShader(shaderData, SIZE_T(shaderDataSize), nullptr, &m_VertexShader);
    if (FAILED(hr))
    {
        SHIP_LOG_ERROR("DX11VertexShader::DX11VertexShader() --> Couldn't create vertex shader.");
        return false;
    }

    return true;
}

void DX11VertexShader::Destroy()
{
    SHIP_ASSERT_MSG(m_VertexShader != nullptr, "Can't call Destroy on invalid vertex shader 0x%p", this);

    m_VertexShader->Release();
    m_VertexShader = nullptr;
}

DX11PixelShader::DX11PixelShader()
    : m_PixelShader(nullptr)
{

}

shipBool DX11PixelShader::Create(ID3D11Device& device, void* shaderData, shipUint64 shaderDataSize)
{
    HRESULT hr = device.CreatePixelShader(shaderData, SIZE_T(shaderDataSize), nullptr, &m_PixelShader);
    if (FAILED(hr))
    {
        SHIP_LOG_ERROR("DX11PixelShader::DX11PixelShader() --> Couldn't create pixel shader.");
        return false;
    }

    return true;
}

void DX11PixelShader::Destroy()
{
    SHIP_ASSERT_MSG(m_PixelShader != nullptr, "Can't call Destroy on invalid pixel shader 0x%p", this);

    m_PixelShader->Release();
    m_PixelShader = nullptr;
}

}