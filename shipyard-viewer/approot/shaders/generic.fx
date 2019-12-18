#include "shaderinputproviders/SimpleConstantBufferProvider.hlsl"
#include "shaderinputproviders/GFXMaterialShaderInputProvider.hlsl"

#include "vertexformatutils.hlsl"

SamplerState testSampler
{
	MinificationFiltering = Linear;
	MagnificationFiltering = Linear;
	AddressModeU = Clamp;
	AddressModeV = Clamp;
};

struct vs_output
{
	float4 pos : SV_POSITION;
	float2 uv: TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 worldPos : WORLD_POS;
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
	output.pos = mul(WorldProjectionMatrix, float4(vertexData.position.xyz, 1.0));
	output.uv = vertexData.texCoords;
    output.normal = mul(WorldFromLocalMatrix, float4(vertexData.normal, 0.0)).xyz;
    output.tangent = mul(WorldFromLocalMatrix, float4(vertexData.tangent, 0.0)).xyz;
    output.worldPos = mul(WorldFromLocalMatrix, float4(vertexData.position.xyz, 1.0)).xyz;
	
	return output;
}

ps_output PS_Main(vs_output input)
{
	ps_output output;

    float3 normal = normalize(input.normal);
    float3 tangent = normalize(input.tangent);
    float3 bitangent = cross(normal, tangent);
    
    float3x3 tbn = float3x3(tangent.x, bitangent.x, normal.x,
                            tangent.y, bitangent.y, normal.y,
                            tangent.z, bitangent.z, normal.z);

    float4 albedoMap = AlbedoMap.Sample(testSampler, input.uv);
    float3 normalMap = NormalMap.Sample(testSampler, input.uv).rgb * 2.0 - 1.0;
    float roughnessMap = RoughnessMap.Sample(testSampler, input.uv).r;
    
    normal = mul(tbn, normalMap);
    
    output.color.rgb = dot(normal, normalize(float3(-1.0, 0.0, -1.0))) * albedoMap;
    
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