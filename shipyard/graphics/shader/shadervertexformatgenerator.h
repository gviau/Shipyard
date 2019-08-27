#pragma once

#include <system/string.h>

#include <graphics/vertexformat.h>

namespace Shipyard
{
    void WriteVertexFormatShaderFile();
    StringA GetShaderVertexInputForVertexFormat(VertexFormatType vertexFormatType);
}