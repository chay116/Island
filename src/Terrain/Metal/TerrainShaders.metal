//
//  TerrainShaders.metal
//  Island
//
//  Created by Chaeyoung Lim on 2022/09/28.
//


using namespace metal;
#import <metal_stdlib>
#import "../TerrainRenderer_shared.h"
#import "../../World/WorldRendererUtilities.metal"
#import "TerrainRendererUtilities.metal"

constant bool g_isDepthOnlyPass [[function_constant(0)]];
constant bool g_isNotDepthOnlyPass = !g_isDepthOnlyPass;

static inline bool quad_and(bool b)
{
    return b;
}

struct TerrainVertexOut {
    float4 position [[position]];
    float3 worldPosition [[function_constant(g_isNotDepthOnlyPass)]];
    float3 viewDir [[function_constant(g_isNotDepthOnlyPass)]];
    float  depth [[function_constant(g_isNotDepthOnlyPass)]];
    float2 uv [[function_constant(g_isNotDepthOnlyPass)]];
    float3 uvw [[function_constant(g_isNotDepthOnlyPass)]];
};

// Generates a bounce-step between 0...1
float bounceStep(float i)
{
    i = saturate(i*4.0 - 2.0);
    return i*i*(3.0-2.0*i);
}

[[patch(quad, 4)]]
vertex TerrainVertexOut terrain_vertex(uint pid [[patch_id]],
                                       float2 uv [[position_in_patch]],
                                       constant Uniforms& uniforms [[buffer(1)]],
                                       texture2d<float> height [[texture(0)]],
                                       constant float4x4& depthOnlyMatrix[[buffer(6), function_constant(g_isDepthOnlyPass)]])
{
    TerrainVertexOut out;

    uint patchY = pid / TERRAIN_PATCHES;
    uint patchX = pid % TERRAIN_PATCHES;

    float2 patchUV = float2(patchX, patchY) / (TERRAIN_PATCHES);

    float3 position = float3(patchUV.x + uv.x / TERRAIN_PATCHES, 0, patchUV.y + uv.y / TERRAIN_PATCHES);


    constexpr sampler sam(min_filter::linear, mag_filter::linear, mip_filter::none, address::mirrored_repeat);

    position.y = (height.sample(sam, position.xz).r);

    float3 worldPosition = float3((position.x - 0.5f) * TERRAIN_SCALE, position.y * TERRAIN_HEIGHT, (position.z - 0.5f) * TERRAIN_SCALE);
    
    if (g_isDepthOnlyPass)
    {
        out.position = depthOnlyMatrix * float4(worldPosition, 1.f);
    }
    else
    {
        out.position = uniforms.cameraUniforms.viewProjectionMatrix * float4(worldPosition, 1.0f);
    }

    // The rest of the parameters aren't necessary for non-shadow passes
    if (g_isNotDepthOnlyPass)
    {
        out.worldPosition = worldPosition;

        out.viewDir = worldPosition - uniforms.cameraUniforms.invViewMatrix[3].xyz;
        out.depth = length(out.viewDir);
        out.viewDir *= (1.f / out.depth);

        out.uv = position.xz;
        out.uvw = worldPosition;
    }

    return out;
}

float4 triplanar (sampler sam, float3 norm, float3 uvw, texture2d_array<float> tex, uint sliceIndex) {
    float3 blending = normalize(max(abs(norm), 0.00001f));
    float b = blending.x + blending.y + blending.z;
    blending /= b;

    float4 x = tex.sample(sam, uvw.yz, sliceIndex);
    float4 y = tex.sample(sam, uvw.xz, sliceIndex);
    float4 z = tex.sample(sam, uvw.xy, sliceIndex);

    return x * blending.x + y * blending.y + z * blending.z;
}

inline float sigmoid(float x, float strength, float threshold) {
    return saturate(1.0f / (1 + exp(-strength * (x - threshold))));
}

inline float materialMask(float slope, float elevation, constant TerrainHabitat& params)
{
    return sigmoid(slope, params.slopeStrength, params.slopeThreshold) * sigmoid(elevation, params.elevationStrength, params.elevationThreshold);
}

static float rand_bilinear (float2 worldCoord, float scale = 400.f)
{
    float2 scaledCoord = (worldCoord+15000)/scale - 0.5;

    const float uint_norm = 1.0 / ((float)uint(0xffffffff));

    uint randSeed = (uint) (floor(scaledCoord.x) + floor(scaledCoord.y)*49.0);
    float randY_0 = float(wang_hash(randSeed  )) * uint_norm;
    float randY_1 = float(wang_hash(randSeed+1)) * uint_norm;
    float a = mix(randY_0, randY_1, fract(scaledCoord.x));

    randSeed = (uint) (floor(scaledCoord.x) + floor(scaledCoord.y+1)*49.0);
    randY_0 = float(wang_hash(randSeed  )) * uint_norm;
    randY_1 = float(wang_hash(randSeed+1)) * uint_norm;
    float b = mix(randY_0, randY_1, fract(scaledCoord.x));

    return mix(a, b, fract(scaledCoord.y));
}

inline BrdfProperties sample_brdf(texture2d_array <float, access::sample>  diffSpecTextureArray,
                                  texture2d_array <float, access::sample>  normalTextureArray,
                                  int curSubLayerIdx,
                                  float textureScale,
                                  float specularPower,
                                  bool flipNormal,
                                  float3 worldPos,
                                  float3 normal,
                                  float3 tangent,
                                  float3 bitangent)
{
    constexpr sampler diffSampler(min_filter::linear, mag_filter::linear, mip_filter::linear, address::repeat);
    constexpr sampler normSampler(min_filter::linear, mag_filter::linear, mip_filter::linear, address::repeat);

    BrdfProperties ret;

    // Sample textures with frac(tex_coordinates) instead of just tex_coordinates.
    // This allows the same visual results even if samplers use different addressing modes (clamp, mirror...).
    // Some Indirect Argument Buffer tests will require sending a large array of samplers to the GPU, but if some are similar, the driver will
    // merge them. You can make them unique by changing their parameters, addressing mode for example, to avoid the merging if needed.
    float4 diffSpec = triplanar(diffSampler, normal, fract(worldPos * textureScale), diffSpecTextureArray, curSubLayerIdx);
    ret.albedo = diffSpec.xyz;

    float3 nmap = triplanar(normSampler, normal, fract(worldPos * textureScale), normalTextureArray, curSubLayerIdx).xyz;

    if (flipNormal)
        nmap.y = 1.0f - nmap.y;

    nmap = normalize(nmap * 2 - 1);
    ret.normal = normalize(nmap.x * tangent + nmap.y * bitangent + nmap.z * normal);

    ret.specIntensity = diffSpec.w;
    ret.specPower = specularPower;

    return ret;
}


fragment GBufferFragOut terrain_fragment(const TerrainVertexOut in [[stage_in]],
                                         constant TerrainParams & mat [[buffer(1)]],
                                         constant Uniforms& globalUniforms [[buffer(2)]],
                                         texture2d<float> heightMap [[texture(0)]],
                                         texture2d<float> normalMap [[texture(1)]],
                                         texture2d<float> propertiesMap [[texture(2)]])
{
    constexpr sampler sam(min_filter::linear, mag_filter::linear, mip_filter::nearest);

    float masks [TerrainHabitatTypeCOUNT];
    float3 normal;

    float noise [4];
    float noise_smooth [4];
    {
        float scale0 = smoothstep(0, 1, rand_bilinear(in.worldPosition.xz, 400.f));
        float scale1 = smoothstep(0, 1, rand_bilinear(in.worldPosition.xz, 100.f)) * 0.75 + 0.25;
        float scale2 = smoothstep(0, 1, rand_bilinear(in.worldPosition.xz, 25.f)) * 0.5 + 0.5;
        float randX = scale0*scale1*scale2;
        noise[0] = smoothstep(0.8, 1.0, 1.0-randX);
        noise_smooth[2]=randX;

        scale0 = smoothstep(0, 1, rand_bilinear(in.worldPosition.xz+4000, 300.f));
        scale1 = smoothstep(0, 1, rand_bilinear(in.worldPosition.xz+4000, 750.f)) * 0.8 + 0.2;
        scale2 = smoothstep(0, 1, rand_bilinear(in.worldPosition.xz+4000, 10.f)) * 0.2 + 0.8;
        randX = scale0*scale1*scale2;
        noise[1] = smoothstep(0.7, 0.95, 1.0-randX);
        noise_smooth[3]=randX;

        scale0 = smoothstep(0, 1, rand_bilinear(in.worldPosition.xz+8000, 200.f));
        scale1 = smoothstep(0, 1, rand_bilinear(in.worldPosition.xz+8000, 66.f)) * 0.75 + 0.25;
        scale2 = smoothstep(0, 1, rand_bilinear(in.worldPosition.xz+8000, 12.5f)) * 0.5 + 0.5;
        randX = scale0*scale1*scale2;
        noise[2] = smoothstep(0.6, 0.9, 1.0-randX);
        noise_smooth[0]=randX;

        noise[3] = 0.f;

        float remainder = 1.f;

        // For noise [0, 1, 2]
        for (int i = 0; i < 3; i++)
        {
            noise [i] = trim(0.005, noise[i]);
            remainder -= noise [i];

            // For noise [1, 2] [2] []
            for (int j = i+1; j < 3; j++)
            {
                noise [j] *= remainder;
            }
        }
        noise [3] = remainder;
        noise_smooth[1]=randX;
    }

    EvaluateTerrainAtLocation(in.uv, in.worldPosition.xyz, heightMap,
                              normalMap, propertiesMap, mat,
                              masks,
                              normal);

    float3 bitangent = float3(0.0034, 0.0072, 1);
    float3 tangent = normalize(cross(normal, bitangent));
    bitangent = cross(tangent, normal);

    BrdfProperties finalBrdf = {};

    for (int curLayerIdx = 0; curLayerIdx < TerrainHabitatTypeCOUNT; curLayerIdx++)
    {
        const float curLayerWeight = masks [curLayerIdx];
        if (quad_and(curLayerWeight == 0.f)) { continue; }

        BrdfProperties curLayerBrdf {};
        for (int curSubLayerIdx = 0; curSubLayerIdx < VARIATION_COUNT_PER_HABITAT; curSubLayerIdx++)
        {
            const float curSubLayerWeight = noise [curSubLayerIdx];
            if (quad_and(curSubLayerWeight == 0.f)) { continue; }

            BrdfProperties curSubLayerBrdf = sample_brdf(
                                                         mat.habitats[curLayerIdx].diffSpecTextureArray,
                                                         mat.habitats[curLayerIdx].normalTextureArray,
                                                         curSubLayerIdx,
                                                         mat.habitats [curLayerIdx].textureScale,
                                                         mat.habitats [curLayerIdx].specularPower,
                                                         mat.habitats [curLayerIdx].flipNormal,
                                                         in.worldPosition,
                                                         normal,
                                                         tangent,
                                                         bitangent);

            curLayerBrdf.albedo        += curSubLayerBrdf.albedo    * curSubLayerWeight * (noise_smooth[curSubLayerIdx]*0.2+0.8);
            curLayerBrdf.normal        += curSubLayerBrdf.normal    * curSubLayerWeight;
            curLayerBrdf.specIntensity += curSubLayerBrdf.specIntensity * curSubLayerWeight;
            curLayerBrdf.specPower     += curSubLayerBrdf.specPower * curSubLayerWeight;
        }

        finalBrdf.albedo        += curLayerBrdf.albedo    * curLayerWeight;
        finalBrdf.normal        += curLayerBrdf.normal    * curLayerWeight;
        finalBrdf.specIntensity += curLayerBrdf.specIntensity * curLayerWeight;
        finalBrdf.specPower     += curLayerBrdf.specPower * curLayerWeight;
    }

    finalBrdf.normal = normalize(finalBrdf.normal);

    float ambientOcclusion = propertiesMap.sample(sam, in.uv).r;
    finalBrdf.ao = ambientOcclusion;

    GBufferFragOut output = PackBrdfProperties(finalBrdf);
    return output;
}

