#include "test.hlsl"
#include "test2.hlsl"

#include "shaderinputproviders/SimpleConstantBufferProvider.hlsl"

SamplerState testSampler
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Wrap;
	AddressV = Wrap;
};

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
	float4 color = TestTexture.Sample(testSampler, input.uv);
	
#if Test2Bits == 3
	output.color = (color - GetValue()) * GetMultiplier();
#elif Test2Bits == 2
	output.color = color * 0.25;
#elif Test2Bits == 1
	output.color = color - float4(1.0, 1.0, 0.0, 0.0);
#else
	output.color = color;
#endif
	
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
	
	RenderTargetWriteMask[0] = RGBA;
}