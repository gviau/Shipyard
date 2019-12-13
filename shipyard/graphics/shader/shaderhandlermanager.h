#pragma once

#include <graphics/shader/shaderkey.h>

#include <graphics/wrapper/wrapper_common.h>

#include <graphics/graphicssingleton.h>

#include <map>

namespace Shipyard
{
    class ShaderDatabase;
    class ShaderHandler;

    class SHIPYARD_GRAPHICS_API ShaderHandlerManager : public GraphicsSingleton<ShaderHandlerManager>
    {
        friend class GraphicsSingleton<ShaderHandlerManager>;

    public:
        ShaderHandlerManager();
        virtual ~ShaderHandlerManager();

        shipBool Initialize(GFXRenderDevice& gfxRenderDevice, ShaderDatabase& shaderDatabase);
        void Destroy();

        ShaderHandler* GetShaderHandlerForShaderKey(ShaderKey shaderKey);

    private:
        GFXRenderDevice* m_RenderDevice;
        std::map<ShaderKey, ShaderHandler*> m_ShaderHandlers;
        ShaderDatabase* m_ShaderDatabase;
    };

    SHIPYARD_GRAPHICS_API ShaderHandlerManager& GetShaderHandlerManager();
}