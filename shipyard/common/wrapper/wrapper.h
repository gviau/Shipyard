#pragma once

#include <common/wrapper/wrapper_common.h>

#ifdef DX11_RENDERER
#include <common/wrapper/dx11/dx11buffer.h>
#include <common/wrapper/dx11/dx11descriptorset.h>
#include <common/wrapper/dx11/dx11pipelinestateobject.h>
#include <common/wrapper/dx11/dx11renderdevice.h>
#include <common/wrapper/dx11/dx11renderdevicecontext.h>
#include <common/wrapper/dx11/dx11rendertarget.h>
#include <common/wrapper/dx11/dx11rootsignature.h>
#include <common/wrapper/dx11/dx11shader.h>
#include <common/wrapper/dx11/dx11texture.h>
#include <common/wrapper/dx11/dx11viewsurface.h>
#endif // #ifdef DX11_RENDERER