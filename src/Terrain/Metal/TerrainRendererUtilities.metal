//
//  TerrainRendererUtilities.metal
//  Island
//
//  Created by Chaeyoung Lim on 2022/09/28.
//

using namespace metal;
#import <metal_stdlib>
#import "../TerrainRenderer_shared.h"
#import "../../World/WorldRendererUtilities.metal"

inline float2 WorldPosToNormPos(float3 worldPos)
{
    return worldPos.xz / TERRAIN_SCALE + 0.5;
}

inline float2 NormalizedPosTo2dWorldPos(float2 uv)
{
    return float2((uv.x - 0.5f) * TERRAIN_SCALE, (uv.y - 0.5f) * TERRAIN_SCALE);
}

struct TerrainEvaluationParams
{
    float heightVariance;
    float slope;
    float elevation;
};

inline float fade_in_out(float2 range, float2 fadeRange, float val)
{
    float fade_in = smoothstep(range.x, range.x + (range.y-range.x) * fadeRange.x, val);
    float fade_out = 1 - smoothstep(range.y - (range.y-range.x) * fadeRange.y, range.y, val);
    return fade_in * fade_out;
}

inline float trim(float thld, float value)
{
    float res = max(value, step(1.0-thld, value));
    res = min(value, step(thld, value));
    return res;
}

inline float evaluate_snow(thread TerrainEvaluationParams &params)
{
    const float fadeOut = 0.2;
    const float valleyFadeOut = 0.1;

    float heightVarianceFade = smoothstep(valleyFadeOut, 1, (params.elevation+0.09)*0.9174);

    float elevationFade = smoothstep(fadeOut, 1, params.elevation);
    return saturate(elevationFade + smoothstep(0.0, 1.0, heightVarianceFade*params.heightVariance*250.0));
}

inline float evaluate_grass(thread TerrainEvaluationParams &params)
{
    // Fade out grass at a certain elevation and at very steep angles
    const float heightFadeIn = 0.04;
    const float heightFadeOut = 0.25;

    float heightFade = fade_in_out(float2(heightFadeIn, heightFadeOut), float2(0.2, 0.5), params.elevation);
    float slopeFade = smoothstep(0.0, 1.0, saturate((params.slope)*2.2-1.1));

    return (heightFade * slopeFade);
}

inline float evaluate_sand(thread TerrainEvaluationParams &params)
{
    float heightFade = 1.0 - smoothstep(0.0, 1.0, saturate((params.elevation-0.009)*(1.0/0.05)));
    return (heightFade);
}

static inline void EvaluateTerrainAtLocation(float2 uv,
                                      float3 worldPosition,
                                      texture2d<float> heightMap,
                                      texture2d<float> normalMap,
                                      texture2d<float> propertiesMap,
                                      constant const TerrainParams& terrainParams,
                                      thread float outHabitat[TerrainHabitatTypeCOUNT],
                                      thread float3 &outNormal)
{
    constexpr sampler sam(min_filter::linear, mag_filter::linear, mip_filter::nearest);

    outNormal = normalize(normalMap.sample(sam, uv).xzy * 2.0f - 1.0f);

    float heightVariance = propertiesMap.sample(sam, uv).y;
    heightVariance = max(heightVariance, propertiesMap.sample(sam, uv, level(1)).y);

    TerrainEvaluationParams params;
    params.heightVariance = heightVariance;
    params.elevation = saturate(worldPosition.y / TERRAIN_HEIGHT);
    params.slope = saturate(outNormal.y);

    outHabitat[TerrainHabitatTypeSnow] = trim(0.002, evaluate_snow(params));

    float remainder = 1.0 - outHabitat[TerrainHabitatTypeSnow];

    outHabitat[TerrainHabitatTypeSand] = trim(0.002, evaluate_sand(params));
    outHabitat[TerrainHabitatTypeGrass] = trim(0.002, evaluate_grass(params));
    const float grassPlusSand = outHabitat[TerrainHabitatTypeSand]+outHabitat[TerrainHabitatTypeGrass];
    if (grassPlusSand > 1)
    {
        outHabitat[TerrainHabitatTypeSand] /= grassPlusSand;
        outHabitat[TerrainHabitatTypeGrass] /= grassPlusSand;
    }
    outHabitat[TerrainHabitatTypeSand] *= remainder;
    outHabitat[TerrainHabitatTypeGrass] *= remainder;

    remainder -= outHabitat[TerrainHabitatTypeSand];
    remainder -= outHabitat[TerrainHabitatTypeGrass];

    outHabitat[TerrainHabitatTypeRock] = remainder;
}

