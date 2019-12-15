#pragma once

#include <graphics/wrapper/texture.h>

struct ID3D11Device;
struct ID3D11Resource;
struct ID3D11ShaderResourceView;
struct ID3D11Texture2D;
struct ID3D11Texture3D;
struct ID3D11UnorderedAccessView;

namespace Shipyard
{
    enum class TextureViewType
    {
        Tex1D,
        Tex2D,
        Tex2DArray,
        Tex3D,
        TexCube
    };

    class SHIPYARD_GRAPHICS_API DX11BaseTexture : public BaseTexture
    {
    public:
        DX11BaseTexture();

        ID3D11ShaderResourceView* GetShaderResourceView() const { return m_ShaderResourceView; }
        ID3D11UnorderedAccessView* GetUnorderedAccessView() const { return m_UnorderedAccessView; }

    protected:
        ID3D11ShaderResourceView* m_ShaderResourceView;
        ID3D11UnorderedAccessView* m_UnorderedAccessView;
    };

    class SHIPYARD_GRAPHICS_API DX11Texture2D : public Texture2D, public DX11BaseTexture
    {
    public:
        DX11Texture2D();
        DX11Texture2D(ID3D11Device& device, ID3D11Texture2D& texture, GfxFormat format);

        shipBool Create(
                ID3D11Device& device,
                shipUint32 width,
                shipUint32 height,
                GfxFormat pixelFormat,
                shipBool dynamic,
                void* initialData,
                shipBool generateMips,
                TextureUsage textureUsage = TextureUsage::TextureUsage_Default);
        void Destroy();

        ID3D11Texture2D* GetTexture() const { return m_Texture; }

    private:
        ID3D11Texture2D* m_Texture;
    };

    class SHIPYARD_GRAPHICS_API DX11Texture2DArray : public Texture2DArray, public DX11BaseTexture
    {
    public:
        DX11Texture2DArray();

        shipBool Create(
                ID3D11Device& device,
                shipUint32 width,
                shipUint32 height,
                shipUint32 numSlices,
                GfxFormat pixelFormat,
                shipBool dynamic,
                void* initialData,
                shipBool generateMips,
                TextureUsage textureUsage = TextureUsage::TextureUsage_Default);
        void Destroy();

        ID3D11Texture2D* GetTexture() const { return m_Texture; }

    private:
        ID3D11Texture2D* m_Texture;
    };

    class SHIPYARD_GRAPHICS_API DX11Texture3D : public Texture3D, public DX11BaseTexture
    {
    public:
        DX11Texture3D();

        shipBool Create(
                ID3D11Device& device,
                shipUint32 width,
                shipUint32 height,
                shipUint32 depth,
                GfxFormat pixelFormat,
                shipBool dynamic,
                void* initialData,
                shipBool generateMips,
                TextureUsage textureUsage = TextureUsage::TextureUsage_Default);
        void Destroy();

        ID3D11Texture3D* GetTexture() const { return m_Texture; }

    private:
        ID3D11Texture3D* m_Texture;
    };

    class SHIPYARD_GRAPHICS_API DX11TextureCube : public TextureCube, public DX11BaseTexture
    {
    public:
        DX11TextureCube();

        shipBool Create(
                ID3D11Device& device,
                shipUint32 width,
                shipUint32 height,
                GfxFormat pixelFormat,
                shipBool dynamic,
                void* initialData,
                shipBool generateMips,
                TextureUsage textureUsage = TextureUsage::TextureUsage_Default);
        void Destroy();

        ID3D11Texture2D* GetTexture() const { return m_Texture; }

    private:
        ID3D11Texture2D* m_Texture;
    };
}