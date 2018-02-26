cbuffer constantBuffer : register(b0)
{
	float4x4 mat;
};

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

vs_output VS_Main(vs_input input)
{
	vs_output output;
	output.pos = mul(mat, float4(input.pos.xyz, 1.0));
	output.uv = input.uv;
	
	return output;
}

ps_output PS_Main(vs_output input)
{
	ps_output output;
	float4 color = tex.Sample(testSampler, input.uv);
	
#if Test1Bit
	output.color = color * 0.5;
#else
	output.color = color - float4(1.0, 1.0, 0.0, 0.0);
#endif
	
	return output;
}