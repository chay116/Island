//
//  ComputeOcclusionAndSlope.metal
//  Island
//
//  Created by Chaeyoung Lim on 2022/10/01.
//

using namespace metal;
#import <metal_stdlib>
#import "../../World/WorldRenderer_shared.h"
#import "../TerrainRenderer_shared.h"



kernel void TerrainKnl_ComputeOcclusionAndSlopeFromHeightmap(texture2d<float> height [[texture(0)]],
                                                             texture2d<float, access::read_write> propTexture [[texture(1)]],
                                                             constant float2 *aoSamples [[buffer(0)]],
                                                             constant int & aoSampleCount [[buffer(1)]],
                                                             constant float2 &invSize [[buffer(2)]],
                                                             uint2 tid [[thread_position_in_grid]])
{
    constexpr sampler sam(min_filter::nearest, mag_filter::nearest, mip_filter::none,
                          address::clamp_to_edge);

    float2 uv_center = ((float2)tid + float2(0.5f, 0.5f)) * invSize;
    float aoVal;

    // Generate AO
    {
        float h_center = height.sample(sam, uv_center).r + 0.001f;

        int numVisible = 0;

        float2 uv = uv_center;

        for (int i = 0; i < aoSampleCount; i++) {
            float2 v = aoSamples[i];

            float h = height.sample(sam, uv + v / height.get_width()).r;

            if (h < h_center)
                numVisible++;
        }

        aoVal = (float)numVisible / (float)aoSampleCount;
    }

    float varianceVal;
    // Generate Variance
    {
        const float offset = 3.5f * invSize.x;
        float2 uv = uv_center;

        float center = height.sample(sam, uv).x;

        float total = 0;
        for (int j = -3; j <= 3; ++j)
        {
            for (int i = -3; i <= 3; ++i)
            {
                if (i == 0 && j == 0) continue;

                float sample = height.sample(sam, uv + float2(offset*i, offset*j)).x;

                total += sample - center;
            }
        }
        total = max(total, 0.f);
        total = total / ((7*7)-1) ;
        varianceVal = saturate(total*2);
    }

    float4 oldval = propTexture.read(tid);
    oldval.xy = float2(aoVal, varianceVal);

    propTexture.write(oldval, tid);
}

