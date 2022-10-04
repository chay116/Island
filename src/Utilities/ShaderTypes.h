//
//  ShaderTypes.h
//  Island
//
//  Created by Chaeyoung Lim on 2022/09/24.
//

//
//  Header containing types and enum constants shared between Metal shaders and Swift/ObjC source
//
#ifndef ShaderTypes_h
#define ShaderTypes_h

#include <simd/simd.h>

#ifndef __METAL_VERSION__
typedef struct __attribute__ ((packed)) packed_float3 {
    float x;
    float y;
    float z;
} packed_float3;
#endif

typedef enum BufferIndex
{
    BufferIndexMeshPositions     = 0,
    BufferIndexMeshGenerics      = 1,
    BufferIndexFrameData         = 2,
    BufferIndexLightsData        = 3,
    BufferIndexLightsPosition    = 4,


} BufferIndex;

typedef enum VertexAttributes
{
    VertexAttributePosition  = 0,
    VertexAttributeTexcoord  = 1,
    VertexAttributeNormal    = 2,
    VertexAttributeTangent   = 3,
    VertexAttributeBitangent = 4
} VertexAttributes;

typedef enum TextureIndex
{
    TextureIndexBaseColor = 0,
    TextureIndexSpecular  = 1,
    TextureIndexNormal    = 2,
    TextureIndexShadow    = 3,
    TextureIndexAlpha     = 4,

    NumMeshTextures = TextureIndexNormal + 1

} TextureIndex;

typedef enum RenderTargetIndex
{
    RenderTargetLighting  = 0,
    RenderTargetAlbedo    = 1,
    RenderTargetNormal    = 2,
    RenderTargetDepth     = 3
} RenderTargetIndex;

struct FrameData
{
    // Per Frame Constants
    simd::float4x4 projection_matrix;
    simd::float4x4 projection_matrix_inverse;
    simd::float4x4 view_matrix;
    uint8_t framebuffer_width;
    uint8_t framebuffer_height;

    // Per Mesh Constants
    simd::float4x4 temple_modelview_matrix;
    simd::float4x4 temple_model_matrix;
    simd::float3x3 temple_normal_matrix;
    float shininess_factor;

    float fairy_size;
    float fairy_specular_intensity;

    simd::float4x4 sky_modelview_matrix;
    simd::float4x4 shadow_mvp_matrix;
    simd::float4x4 shadow_mvp_xform_matrix;

    simd::float4 sun_eye_direction;
    simd::float4 sun_color;
    float sun_specular_intensity;
};

struct PointLight
{
    simd::float3 light_color;
    float light_radius;
    float light_speed;
};

struct SimpleVertex
{
    simd::float2 position;
};

struct ShadowVertex
{
    packed_float3 position;
};



#endif /* ShaderTypes_h */

