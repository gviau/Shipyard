#include "shaderinputproviders/ImGuiShaderInputProvider.hlsl"

#define VERTEX_FORMAT_TYPE 5

#include "vertexformatutils.hlsl"

SamplerState ImGuiTextureSampler
{
	MinificationFiltering = Linear;
	MagnificationFiltering = Linear;
	AddressModeU = Wrap;
	AddressModeV = Wrap;
};

struct vs_output
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

struct ps_output
{
	float4 color : SV_TARGET;
};

vs_output VS_Main(vs_input vertexInput)
{
	VertexData vertexData;
	UnpackVertexInput(vertexInput, vertexData);

	vs_output output;
	output.pos = mul(ImGuiOrthographicProjectionMatrix, float4(vertexData.position.xy, 0.0, 1.0));
	output.color = vertexData.color;
	output.uv = vertexData.texCoords;
	
	return output;
}

ps_output PS_Main(vs_output input)
{
	ps_output output;
	output.color = input.color * ImGuiTexture.Sample(ImGuiTextureSampler, input.uv);

	return output;
}

RenderState
{
    CullMode = CullNone;
    FillMode = Solid;
    IsFrontCounterClockwise = false;
    ScissorEnable = true;
    DepthClipEnable = true;

    DepthEnable = false;
    EnableDepthWrite = true;
    DepthComparisonFunc = Always;
	
    StencilEnable = false;
    StencilReadMask = 0x00;
    StencilWriteMask = 0x00;
	
	BlendEnable[0] = true;
    SourceBlend[0] = SrcAlpha;
    DestBlend[0] = InvSrcAlpha;
    BlendOperator[0] = Add;
    SourceAlphaBlend[0] = InvSrcAlpha;
    DestAlphaBlend[0] = Zero;
    AlphaBlendOperator[0] = Add;
	RenderTargetWriteMask[0] = RGBA;
}