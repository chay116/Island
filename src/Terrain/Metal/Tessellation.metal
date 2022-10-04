//
//  Tessellation.metal
//  Island
//
//  Created by Chaeyoung Lim on 2022/09/29.
//

using namespace metal;
#import <metal_stdlib>
#import "../TerrainRenderer_shared.h"
#import "../../World/WorldRendererUtilities.metal"

static float tessFactor(float4x4 viewProjectionMatrix, float projectionYScale, float scale, float3 p0, float3 p1) {
    float3 center = (p0 + p1) * 0.5f;
    float diameter = distance(p0, p1);

    float4 clip = viewProjectionMatrix * float4(center, 1.0f);
    float projectedLength = abs(diameter * projectionYScale / clip.w);

    return max(scale * projectedLength, 1.0f);
}

kernel void TerrainKnl_FillInTesselationFactors(device MTLQuadTessellationFactorsHalf* outVisiblePatchesTessFactorBfr [[buffer(0)]],
                                                device uint32_t* outVisiblePatchIndices [[buffer(2)]],
                                                constant float& tesselationScale [[buffer(3)]],
                                                constant Uniforms& uniforms [[buffer(4)]],
                                                texture2d<float> heightMap [[texture(0)]],
                                                uint2 tid [[thread_position_in_grid]])
{
    float2 u00 = float2((tid.x + 0.f) / TERRAIN_PATCHES, (tid.y + 0.f) / TERRAIN_PATCHES);
    float2 u01 = float2((tid.x + 0.f) / TERRAIN_PATCHES, (tid.y + 1.f) / TERRAIN_PATCHES);
    float2 u10 = float2((tid.x + 1.f) / TERRAIN_PATCHES, (tid.y + 0.f) / TERRAIN_PATCHES);
    float2 u11 = float2((tid.x + 1.f) / TERRAIN_PATCHES, (tid.y + 1.f) / TERRAIN_PATCHES);

    float3 p00 = float3((u00.x - 0.5f) * TERRAIN_SCALE, 0, (u00.y - 0.5f) * TERRAIN_SCALE);
    float3 p01 = float3((u01.x - 0.5f) * TERRAIN_SCALE, 0, (u01.y - 0.5f) * TERRAIN_SCALE);
    float3 p10 = float3((u10.x - 0.5f) * TERRAIN_SCALE, 0, (u10.y - 0.5f) * TERRAIN_SCALE);
    float3 p11 = float3((u11.x - 0.5f) * TERRAIN_SCALE, 0, (u11.y - 0.5f) * TERRAIN_SCALE);

    {
        const uint patchID = tid.x + tid.y * TERRAIN_PATCHES;
        outVisiblePatchIndices [patchID] = patchID;

        constexpr sampler sam(min_filter::linear, mag_filter::linear, mip_filter::none, address::clamp_to_edge);

        p00.y = heightMap.sample(sam, u00).r * TERRAIN_HEIGHT;
        p01.y = heightMap.sample(sam, u01).r * TERRAIN_HEIGHT;
        p10.y = heightMap.sample(sam, u10).r * TERRAIN_HEIGHT;
        p11.y = heightMap.sample(sam, u11).r * TERRAIN_HEIGHT;

        float e0 = tessFactor(uniforms.cameraUniforms.viewProjectionMatrix, uniforms.projectionYScale, tesselationScale, p00, p01);
        float e1 = tessFactor(uniforms.cameraUniforms.viewProjectionMatrix, uniforms.projectionYScale, tesselationScale, p00, p10);
        float e2 = tessFactor(uniforms.cameraUniforms.viewProjectionMatrix, uniforms.projectionYScale, tesselationScale, p10, p11);
        float e3 = tessFactor(uniforms.cameraUniforms.viewProjectionMatrix, uniforms.projectionYScale, tesselationScale, p01, p11);

        outVisiblePatchesTessFactorBfr[patchID].edgeTessellationFactor[0] = e0;
        outVisiblePatchesTessFactorBfr[patchID].edgeTessellationFactor[1] = e1;
        outVisiblePatchesTessFactorBfr[patchID].edgeTessellationFactor[2] = e2;
        outVisiblePatchesTessFactorBfr[patchID].edgeTessellationFactor[3] = e3;

        outVisiblePatchesTessFactorBfr[patchID].insideTessellationFactor[0] = (e1 + e3) * 0.5f;
        outVisiblePatchesTessFactorBfr[patchID].insideTessellationFactor[1] = (e0 + e2) * 0.5f;
    }
}







