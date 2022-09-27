//
//  WorldRenderer_shared.h
//  Island
//
//  Created by Chaeyoung Lim on 2022/09/24.
//

#ifndef WorldRenderer_shared_h
#define WorldRenderer_shared_h

#import <simd/simd.h>

// Matrices that are stored and generated internally within the camera object
struct CameraUniforms
{
    simd::float4x4      viewMatrix;
    simd::float4x4      projectionMatrix;
    simd::float4x4      viewProjectionMatrix;
    simd::float4x4      invOrientationProjectionMatrix;
    simd::float4x4      invViewProjectionMatrix;
    simd::float4x4      invProjectionMatrix;
    simd::float4x4      invViewMatrix;
};

struct Uniforms
{
    CameraUniforms    cameraUniforms;
    CameraUniforms    shadowCameraUniforms[3];

    // Mouse state: x,y = position in pixels; z = buttons
    simd::float3        mouseState;
    simd::float2        invScreenSize;
    float               projectionYScale;
    float               brushSize;
    float               ambientOcclusionContrast;
    float               ambientOcclusionScale;
    float               ambientLightScale;
    float               gameTime;
    float               frameTime;  // TODO. this doesn't appear to be initialized until UpdateCpuUniforms. OK?
};

struct DebugVertex
{
    simd::float4        position;
    simd::float4        color;
};

// Describes our standardized OBJ format geometry vertex format
struct ObjVertex
{
    simd::float3        position;
    simd::float3        normal;
    simd::float3        color;
#ifndef __METAL_VERSION__
    bool operator == (const ObjVertex& o) const
    {
        return simd::all (o.position == position) &&
        simd::all (o.normal == normal) &&
        simd::all (o.color == color);
    }
#endif
};

struct InstanceData
{
    simd::float4x4 instanceTransform;
    simd::float3x3 instanceNormalTransform;
    simd::float4 instanceColor;
};

#endif /* WorldRenderer_shared_h */
