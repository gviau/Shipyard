#ifndef FULLSCREEN_HELPER_UTILS_HLSL
#define FULLSCREEN_HELPER_UTILS_HLSL

struct fullscreen_helper_vs_input
{
    float3 position : POSITION;
    float2 tex_coords : TEXCOORDS;
};

struct vs_output
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORDS;
};

vs_output VS_Main(fullscreen_helper_vs_input input)
{
    vs_output output;
    output.position = float4(input.position.xy, 0.0, 1.0);
    output.uv = input.tex_coords.xy;
 
    return output;
}

#endif // #ifndef FULLSCREEN_HELPER_UTILS_HLSL