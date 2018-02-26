#pragma once

#include <common/shaderkey.h>

#include <common/wrapper/wrapper_common.h>

#include <system/singleton.h>

#include <map>
using namespace std;

namespace Shipyard
{
    class ShaderHandler;

    class SHIPYARD_API ShaderHandlerManager : public Singleton<ShaderHandlerManager>
    {
        friend class Singleton<ShaderHandlerManager>;

    public:
        ShaderHandlerManager() {}
        virtual ~ShaderHandlerManager();

        ShaderHandler* GetShaderHandlerForShaderKey(ShaderKey shaderKey, GFXRenderDevice& gfxRenderDevice);

    private:
        map<ShaderKey, ShaderHandler*> m_ShaderHandlers;
    };
}