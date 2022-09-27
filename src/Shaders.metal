//
//  Shaders.metal
//  Island
//
//  Created by Chaeyoung Lim on 2022/09/24.
//

// File for Metal kernel and shader functions

#include <metal_stdlib>
#include <simd/simd.h>

// Including header shared between this Metal shader code and Swift/C code executing Metal API commands
#import "ShaderTypes.h"
#import "./World/WorldRenderer_shared.h"

using namespace metal;

struct v2f
{
    float4 position [[position]];
    float3 normal;
    half3 color;
    float2 texcoord;
};
struct VertexData
{
    float3 position;
    float3 normal;
    float2 texcoord;
};

v2f vertex vertexShader( device const VertexData* vertexData [[buffer(0)]],
                       device const CameraUniforms& cameraData [[buffer(1)]],
                       uint vertexId [[vertex_id]],
                       uint instanceId [[instance_id]] )
{
    v2f o;
    const device VertexData& vd = vertexData[ vertexId ];
    o.position = cameraData.viewProjectionMatrix * float4( vd.position, 1.0 );
//    float3 normal = cameraData.invViewMatrix * vd.normal;
    o.normal = vd.normal;
    o.texcoord = vd.texcoord.xy;
    o.color = half3( 1, 1, 1 );
    return o;
}

half4 fragment fragmentShader( v2f in [[stage_in]],
                            texture2d< half, access::sample > tex [[texture(0)]] )
{
    constexpr sampler s( address::repeat, filter::linear );
    half3 texel = tex.sample( s, in.texcoord ).rgb;
    // assume light coming from (front-top-right)
    float3 l = normalize(float3( 1.0, 1.0, 0.8 ));
    float3 n = normalize( in.normal );
    half ndotl = half( saturate( dot( n, l ) ) );
    half3 illum = (in.color * texel * 0.1) + (in.color * texel * ndotl);
    return half4( illum, 1.0 );
}

