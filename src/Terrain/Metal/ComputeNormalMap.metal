//
//  ComputeNormalMap.metal
//  Island
//
//  Created by Chaeyoung Lim on 2022/10/01.
//

using namespace metal;
#import <metal_stdlib>
#import "../TerrainRenderer_shared.h"

kernel void TerrainKnl_ComputeNormalsFromHeightmap(texture2d<float> height [[texture(0)]],
                                                   texture2d<float, access::write> normal [[texture(1)]],
                                                   uint2 tid [[thread_position_in_grid]])
{
    constexpr sampler sam(min_filter::nearest, mag_filter::nearest, mip_filter::none,
                          address::clamp_to_edge, coord::pixel);

    float xz_scale = TERRAIN_SCALE / height.get_width();
    float y_scale = TERRAIN_HEIGHT;

    if (tid.x < height.get_width() && tid.y < height.get_height()) {
        float h_up     = height.sample(sam, (float2)(tid + uint2(0, 1))).r;
        float h_down   = height.sample(sam, (float2)(tid - uint2(0, 1))).r;
        float h_right  = height.sample(sam, (float2)(tid + uint2(1, 0))).r;
        float h_left   = height.sample(sam, (float2)(tid - uint2(1, 0))).r;
        float h_center = height.sample(sam, (float2)(tid + uint2(0, 0))).r;

        float3 v_up    = float3( 0,        (h_up    - h_center) * y_scale,  xz_scale);
        float3 v_down  = float3( 0,        (h_down  - h_center) * y_scale, -xz_scale);
        float3 v_right = float3( xz_scale, (h_right - h_center) * y_scale,  0);
        float3 v_left  = float3(-xz_scale, (h_left  - h_center) * y_scale,  0);

        float3 n0 = cross(v_up, v_right);
        float3 n1 = cross(v_left, v_up);
        float3 n2 = cross(v_down, v_left);
        float3 n3 = cross(v_right, v_down);

        float3 n = normalize(n0 + n1 + n2 + n3) * 0.5f + 0.5f;

        normal.write(float4(n.xzy, 1), tid);
    }
}
