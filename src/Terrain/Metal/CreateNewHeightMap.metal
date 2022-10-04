//
//  CreateNewHeightMap.metal
//  Island
//
//  Created by Chaeyoung Lim on 2022/10/01.
//

using namespace metal;
#import <metal_stdlib>
#import "../TerrainRenderer_shared.h"
#import "../../World/WorldRenderer_shared.h"


inline float evaluateModificationBrush(simd::float2 worldPosition,
                                       simd::float4 mousePosition,
                                       float        size)
{
    float dist = simd::length(worldPosition - mousePosition.xz);
    dist /= size;
    return saturate(min(2-dist, 1.0 / (1.0 + pow(dist*2.0, 4))));
}

kernel void TerrainKnl_ClearTexture(texture2d<float, access::write> tex,
                                    uint2 tid [[thread_position_in_grid]])
{
    tex.write(0, tid);
}

kernel void TerrainKnl_UpdateHeightmap(texture2d<float, access::read_write> heightMap   [[texture(0)]],
                                       uint2 tid                                        [[thread_position_in_grid]],
                                       constant float4 &mousePosition                   [[buffer(0)]],
                                       constant Uniforms& globalUniforms            [[buffer(1)]])
{
    float2 world_xz = (float2(tid) / float2(heightMap.get_width(), heightMap.get_width()) - .5f) * TERRAIN_SCALE;
    float displacement = evaluateModificationBrush(world_xz, mousePosition, globalUniforms.brushSize) * 0.008f;
    if (globalUniforms.mouseState.z == 2) displacement *= -1.0;
    float h = heightMap.read(tid).r;
    heightMap.write(h+displacement, tid);
}

kernel void TerrainKnl_CopyChannel1Only(texture2d<float> src [[texture(0)]],
                                        texture2d<float, access::write> dst [[texture(1)]],
                                        uint2 tid [[thread_position_in_grid]])
{
    float r = src.read(tid).r;
    dst.write(r, tid);
}

