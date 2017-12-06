#include <common/wrapper/dx11/dx11shader.h>

#pragma warning( disable : 4005 )

#include <d3d11.h>
#include <d3dcompiler.h>

#pragma warning( default : 4005 )

namespace Shipyard
{;

String GetD3DShaderVersion(D3D_FEATURE_LEVEL featureLevel);

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

DX11VertexShader::DX11VertexShader(ID3D11Device* device, const String& source)
    : VertexShader(source)
    , m_VertexShader(nullptr)
{
    ID3D10Blob* error = nullptr;

    D3D_FEATURE_LEVEL featureLevel = device->GetFeatureLevel();
    String shaderVersion = GetD3DShaderVersion(featureLevel);

    String version = ("vs_" + shaderVersion);
    unsigned int flags = 0;

#ifdef _DEBUG
    flags = D3DCOMPILE_DEBUG;
#endif

    HRESULT hr = D3DCompile(source.c_str(), source.size(), nullptr, nullptr, nullptr, "main", version.c_str(), flags, 0, &m_ShaderBlob, &error);
    if (FAILED(hr))
    {
        if (error != nullptr)
        {
            char* errorMsg = (char*)error->GetBufferPointer();
            MessageBox(NULL, errorMsg, "DX11 error", MB_OK);
        }

        return;
    }

    hr = device->CreateVertexShader(m_ShaderBlob->GetBufferPointer(), m_ShaderBlob->GetBufferSize(), nullptr, &m_VertexShader);
    if (FAILED(hr))
    {
        MessageBox(NULL, "CreateVertexShader failed", "DX11 error", MB_OK);
    }
}

DX11VertexShader::~DX11VertexShader()
{
    if (m_VertexShader != nullptr)
    {
        m_VertexShader->Release();
    }
}

DX11PixelShader::DX11PixelShader(ID3D11Device* device, const String& source)
    : PixelShader(source)
    , m_PixelShader(nullptr)
{
    ID3D10Blob* error = nullptr;

    D3D_FEATURE_LEVEL featureLevel = device->GetFeatureLevel();
    String shaderVersion = GetD3DShaderVersion(featureLevel);

    String version = ("ps_" + shaderVersion);
    unsigned int flags = 0;

#ifdef _DEBUG
    flags = D3DCOMPILE_DEBUG;
#endif

    HRESULT hr = D3DCompile(source.c_str(), source.size(), nullptr, nullptr, nullptr, "main", version.c_str(), flags, 0, &m_ShaderBlob, &error);
    if (FAILED(hr))
    {
        if (error != nullptr)
        {
            char* errorMsg = (char*)error->GetBufferPointer();
            MessageBox(NULL, errorMsg, "DX11 error", MB_OK);
        }

        return;
    }

    hr = device->CreatePixelShader(m_ShaderBlob->GetBufferPointer(), m_ShaderBlob->GetBufferSize(), nullptr, &m_PixelShader);
    if (FAILED(hr))
    {
        MessageBox(NULL, "CreateVertexShader failed", "DX11 error", MB_OK);
    }
}

DX11PixelShader::~DX11PixelShader()
{
    if (m_PixelShader != nullptr)
    {
        m_PixelShader->Release();
    }
}

String GetD3DShaderVersion(D3D_FEATURE_LEVEL featureLevel)
{
    String version = "";

    switch (featureLevel)
    {
    case D3D_FEATURE_LEVEL_11_0:
        version = "5_0";
        break;

    case D3D_FEATURE_LEVEL_10_1:
        version = "4_1";
        break;

    case D3D_FEATURE_LEVEL_10_0:
        version = "4_0";
        break;

    case D3D_FEATURE_LEVEL_9_3:
    case D3D_FEATURE_LEVEL_9_2:
    case D3D_FEATURE_LEVEL_9_1:
        version = "3_0";
        break;
    }

    return version;
}

}