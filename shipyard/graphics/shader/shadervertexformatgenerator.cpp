#include <graphics/graphicsprecomp.h>

#include <graphics/shader/shadervertexformatgenerator.h>

#include <graphics/vertexformat.h>

#include <system/string.h>
#include <system/wrapper/wrapper.h>

namespace Shipyard
{;

void WriteVertexFormatShaderFile()
{
    FileHandler vertexFormatShaderFile;
    vertexFormatShaderFile.Open("shaders\\vertexformats\\vertexformats.hlsl", FileHandlerOpenFlag(FileHandlerOpenFlag::FileHandlerOpenFlag_Write | FileHandlerOpenFlag::FileHandlerOpenFlag_Create));

    if (!vertexFormatShaderFile.IsOpen())
    {
        return;
    }

    StringA content;
    content += "/**\n* AUTO-GENERATED FILE\n**/\n";
    content += "#ifndef SHADER_VERTEX_FORMATS_HLSL\n";
    content += "#define SHADER_VERTEX_FORMATS_HLSL\n\n";

    content += "#ifndef VERTEX_FORMAT_TYPE\n";
    content += "#define VERTEX_FORMAT_TYPE 0\n";
    content += "#endif // #ifndef VERTEX_FORMAT_TYPE\n\n";

    content += "#";

    for (shipUint16 i = 0; i < shipUint16(VertexFormatType::VertexFormatType_Count); i++)
    {
        VertexFormatType vertexFormatType = VertexFormatType(i);

        content += StringFormat("if VERTEX_FORMAT_TYPE == %d\n\n", i);

        content += GetShaderVertexInputForVertexFormat(vertexFormatType);

        if (VertexFormatTypeContainsColor(vertexFormatType))
        {
            content += "#define VERTEX_FORMAT_HAS_COLOR\n";
        }

        if (VertexFormatTypeContainsUV(vertexFormatType))
        {
            content += "#define VERTEX_FORMAT_HAS_TEXCOORDS\n";
        }

        if (VertexFormatTypeContainsNormals(vertexFormatType))
        {
            content += "#define VERTEX_FORMAT_HAS_NORMALS\n";
        }

        content += "\n";

        shipBool isLast = (i == shipUint16(VertexFormatType::VertexFormatType_Count) - 1);
        if (!isLast)
        {
            content += "#el";
        }
    }

    content += "#else\n\n";

    content += "struct vs_input\n";
    content += "{\n";
    content += "    float4 position : POSITION;\n";
    content += "};\n\n";

    content += "#endif\n\n";

    content += "#endif // #ifndef SHADER_VERTEX_FORMATS_HLSL";

    constexpr shipUint32 startingPosition = 0;
    constexpr shipBool flush = true;
    vertexFormatShaderFile.WriteChars(startingPosition, content.GetBuffer(), content.Size(), flush);
}

StringA GetShaderVertexInputForVertexFormat(VertexFormatType vertexFormatType)
{
    StringA shaderVertexInput;

    shaderVertexInput += "struct vs_input\n";
    shaderVertexInput += "{\n";

    VertexFormat* vertexFormat = nullptr;
    GetVertexFormat(vertexFormatType, vertexFormat);
    SHIP_ASSERT(vertexFormat != nullptr);

    shipUint32 numInputLayouts = vertexFormat->GetNumInputLayouts();
    const InputLayout* inputLayouts = vertexFormat->GetInputLayouts();

    for (shipUint32 j = 0; j < numInputLayouts; j++)
    {
        const InputLayout& inputLayout = inputLayouts[j];

        BaseFormatType shaderInputBaseFormatType = GetBaseFormatType(inputLayout.m_Format);
        shipUint32 shaderInputNumComponents = GetFormatNumComponents(inputLayout.m_Format);

        const shipChar* shaderInputBaseFormatTypeName = GetBaseFormatTypeName(shaderInputBaseFormatType);

        const shipChar* shaderInputType = StringFormat("%s%d", shaderInputBaseFormatTypeName, shaderInputNumComponents);

        const shipChar* shaderInputName = GetVertexShaderInputName(inputLayout.m_SemanticName);
        if (inputLayout.m_SemanticIndex > 0)
        {
            shaderInputName = StringFormat("%s%d", shaderInputName, inputLayout.m_SemanticIndex);
        }

        const shipChar* semanticName = GetVertexSemanticName(inputLayout.m_SemanticName);
        if (inputLayout.m_SemanticIndex > 0)
        {
            semanticName = StringFormat("%s%d", semanticName, inputLayout.m_SemanticIndex);
        }

        shaderVertexInput += "    ";
        shaderVertexInput += shaderInputType;
        shaderVertexInput += " ";
        shaderVertexInput += shaderInputName;
        shaderVertexInput += " : ";
        shaderVertexInput += semanticName;
        shaderVertexInput += ";\n";
    }

    shaderVertexInput += "};\n\n";

    return shaderVertexInput;
}

}