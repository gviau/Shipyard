#pragma once

#include <graphics/wrapper/wrapper_common.h>

#ifdef DX11_RENDERER
#include <graphics/wrapper/dx11/dx11buffer.h>
#include <graphics/wrapper/dx11/dx11commandlist.h>
#include <graphics/wrapper/dx11/dx11commandlistallocator.h>
#include <graphics/wrapper/dx11/dx11commandqueue.h>
#include <graphics/wrapper/dx11/dx11descriptorset.h>
#include <graphics/wrapper/dx11/dx11pipelinestateobject.h>
#include <graphics/wrapper/dx11/dx11renderdevice.h>
#include <graphics/wrapper/dx11/dx11rendertarget.h>
#include <graphics/wrapper/dx11/dx11rootsignature.h>
#include <graphics/wrapper/dx11/dx11shader.h>
#include <graphics/wrapper/dx11/dx11texture.h>
#include <graphics/wrapper/dx11/dx11viewsurface.h>
#endif // #ifdef DX11_RENDERER