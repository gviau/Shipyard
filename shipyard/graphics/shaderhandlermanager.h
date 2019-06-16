#pragma once

#include <graphics/shaderkey.h>

#include <graphics/wrapper/wrapper_common.h>

#include <system/singleton.h>

#include <map>

namespace Shipyard
{
    class ShaderDatabase;
    class ShaderHandler;

    class SHIPYARD_API ShaderHandlerManager : public Singleton<ShaderHandlerManager>
    {
        friend class Singleton<ShaderHandlerManager>;

    public:
        ShaderHandlerManager();
        virtual ~ShaderHandlerManager();

        bool Initialize(GFXRenderDevice& gfxRenderDevice, ShaderDatabase& shaderDatabase);
        void Destroy();

        ShaderHandler* GetShaderHandlerForShaderKey(ShaderKey shaderKey);

    private:
        GFXRenderDevice* m_RenderDevice;
        std::map<ShaderKey, ShaderHandler*> m_ShaderHandlers;
        ShaderDatabase* m_ShaderDatabase;
    };

    SHIPYARD_API ShaderHandlerManager& GetShaderHandlerManager();
}