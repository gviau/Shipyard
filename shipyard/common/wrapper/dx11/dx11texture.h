#pragma once

#include <common/wrapper/texture.h>

struct ID3D11Device;
struct ID3D11Resource;
struct ID3D11ShaderResourceView;
struct ID3D11Texture2D;

namespace Shipyard
{
    enum class TextureViewType
    {
        Tex1D,
        Tex2D,
        Tex3D,
        Cube
    };

    class SHIPYARD_API DX11BaseTexture : public BaseTexture
    {
    public:
        DX11BaseTexture(ID3D11Device& device, uint32_t width, uint32_t height, GfxFormat pixelFormat, bool dynamic, void* initialData, bool generateMips);
        ~DX11BaseTexture();

        ID3D11ShaderResourceView* GetShaderResourceView() const { return m_ShaderResourceView; }

        static ID3D11ShaderResourceView* CreateShaderResourceView(ID3D11Device& device, ID3D11Resource& resource, GfxFormat pixelFormat, TextureViewType textureViewType);

    protected:
        ID3D11ShaderResourceView* m_ShaderResourceView;
    };

    class SHIPYARD_API DX11Texture2D : public Texture2D, public DX11BaseTexture
    {
    public:
        DX11Texture2D(ID3D11Device& device, uint32_t width, uint32_t height, GfxFormat pixelFormat, bool dynamic, void* initialData, bool generateMips);
        ~DX11Texture2D();

        ID3D11Texture2D* GetTexture() const { return m_Texture; }

    private:
        ID3D11Texture2D* m_Texture;
    };
}