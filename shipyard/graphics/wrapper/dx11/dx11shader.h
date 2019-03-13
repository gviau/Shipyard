#pragma once

#include <graphics/wrapper/shader.h>

struct ID3D10Blob;
struct ID3D11Device;
struct ID3D11PixelShader;
struct ID3D11VertexShader;

namespace Shipyard
{
    class SHIPYARD_API DX11BaseShader : public BaseShader
    {

    };

    class SHIPYARD_API DX11VertexShader : public VertexShader, public DX11BaseShader
    {
    public:
        DX11VertexShader();

        bool Create(ID3D11Device& device, void* shaderData, uint64_t shaderDataSize);
        void Destroy();

        ID3D11VertexShader* GetShader() const { return m_VertexShader; }

    private:
        ID3D11VertexShader* m_VertexShader;
    };

    class SHIPYARD_API DX11PixelShader : public PixelShader, public DX11BaseShader
    {
    public:
        DX11PixelShader();

        bool Create(ID3D11Device& device, void* shaderData, uint64_t shaderDataSize);
        void Destroy();

        ID3D11PixelShader* GetShader() const { return m_PixelShader; }

    private:
        ID3D11PixelShader* m_PixelShader;
    };
}