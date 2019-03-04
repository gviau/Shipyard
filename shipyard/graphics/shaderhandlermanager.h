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

        void SetShaderDatabase(ShaderDatabase& shaderDatabase);

        ShaderHandler* GetShaderHandlerForShaderKey(ShaderKey shaderKey, GFXRenderDevice& gfxRenderDevice);

    private:
        std::map<ShaderKey, ShaderHandler*> m_ShaderHandlers;
        ShaderDatabase* m_ShaderDatabase;
    };
}