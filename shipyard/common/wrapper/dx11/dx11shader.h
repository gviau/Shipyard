#pragma once

#include <common/wrapper/shader.h>

struct ID3D10Blob;
struct ID3D11Device;
struct ID3D11PixelShader;
struct ID3D11VertexShader;

namespace Shipyard
{
    class SHIPYARD_API DX11BaseShader : public BaseShader
    {
    public:
        DX11BaseShader();
        ~DX11BaseShader();

        ID3D10Blob* GetShaderBlob() const { return m_ShaderBlob; }

    protected:
        ID3D10Blob* m_ShaderBlob;
    };

    class SHIPYARD_API DX11VertexShader : public VertexShader, public DX11BaseShader
    {
    public:
        DX11VertexShader(ID3D11Device& device, void* shaderData, uint64_t shaderDataSize);
        ~DX11VertexShader();

        ID3D11VertexShader* GetShader() const { return m_VertexShader; }

    private:
        ID3D11VertexShader* m_VertexShader;
    };

    class SHIPYARD_API DX11PixelShader : public PixelShader, public DX11BaseShader
    {
    public:
        DX11PixelShader(ID3D11Device& device, void* shaderData, uint64_t shaderDataSize);
        ~DX11PixelShader();

        ID3D11PixelShader* GetShader() const { return m_PixelShader; }

    private:
        ID3D11PixelShader* m_PixelShader;
    };
}