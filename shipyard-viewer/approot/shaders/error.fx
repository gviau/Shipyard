#include "shaderinputproviders/SimpleConstantBufferProvider.hlsl"

#include "vertexformatutils.hlsl"

struct vs_output
{
	float4 pos : SV_POSITION;
};

struct ps_output
{
	float4 color : SV_TARGET;
};

vs_output VS_Main(vs_input vertexInput, uint instanceId : SV_InstanceID)
{
	VertexData vertexData;
	UnpackVertexInput(vertexInput, vertexData);

	LoadSimpleConstantBufferProviderConstantsForInstance(instanceId);
	
	vs_output output;
	output.pos = mul(Test, float4(vertexData.position.xyz, 1.0));
	
	return output;
}

ps_output PS_Main(vs_output input)
{
	ps_output output;
	output.color = float4(0.95, 0.25, 0.94, 1.0);
	
	return output;
}

RenderState
{
    CullMode = CullBackFace;
    FillMode = Solid;
    IsFrontCounterClockwise = false;

    DepthEnable = true;
    EnableDepthWrite = true;
    DepthComparisonFunc = Less;
	
    StencilEnable = false;
    StencilReadMask = 0x00;
    StencilWriteMask = 0x00;
}