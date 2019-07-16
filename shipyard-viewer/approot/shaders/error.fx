#include "shaderinputproviders/SimpleConstantBufferProvider.hlsl"

SamplerState testSampler
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Wrap;
	AddressV = Wrap;
};

Texture2D tex : register(t0);

struct vs_input
{
	float3 pos : POSITION;
	float2 uv : TEXCOORD;
};

struct vs_output
{
	float4 pos : SV_POSITION;
	float2 uv: TEXCOORD;
};

struct ps_output
{
	float4 color : SV_TARGET;
};

vs_output VS_Main(vs_input input, uint instanceId : SV_InstanceID)
{
	LoadSimpleConstantBufferProviderConstantsForInstance(instanceId);
	
	vs_output output;
	output.pos = mul(Test, float4(input.pos.xyz, 1.0));
	output.uv = input.uv;
	
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