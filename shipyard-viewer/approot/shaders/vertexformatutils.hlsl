#ifndef VERTEX_FORMAT_UTILS_HLSL
#define VERTEX_FORMAT_UTILS_HLSL

#include "vertexformats/vertexformats.hlsl"

struct VertexData
{
    float3 position;
    float2 texCoords;
    float3 normal;
    float3 tangent;
    float4 color;
    float4 boneWeights;
    uint4 boneIndices;
};

void UnpackVertexInput(in vs_input vertexInput, out VertexData vertexData)
{
    vertexData = (VertexData)0;
    
#if VERTEX_FORMAT_TYPE == 6
    vertexData.position = float3(vertexInput.position.xy, 0.0);
#else
    vertexData.position = vertexInput.position.xyz;
#endif // #if VERTEX_FORMAT_TYPE == 6

#ifdef VERTEX_FORMAT_HAS_TEXCOORDS
    vertexData.texCoords.xy = vertexInput.tex_coords.xy;
#endif // #ifdef VERTEX_FORMAT_HAS_TEXCOORDS

#ifdef VERTEX_FORMAT_HAS_COLOR
    vertexData.color = vertexInput.color.rgba;
#endif // #ifdef VERTEX_FORMAT_HAS_COLOR

#ifdef VERTEX_FORMAT_HAS_NORMALS
    vertexData.normal = vertexInput.normal.xyz;
#endif // #ifdef VERTEX_FORMAT_HAS_NORMALS

#ifdef VERTEX_FORMAT_HAS_TANGENTS
	vertexData.tangent = vertexInput.tangent.xyz;
#endif // #ifdef VERTEX_FORMAT_HAS_TANGENTS
}

#endif // #ifndef VERTEX_FORMAT_UTILS_HLSL