//
//  TerrainRenderer.hpp
//  Island
//
//  Created by Chaeyoung Lim on 2022/09/28.
//

#ifndef TerrainRenderer_h
#define TerrainRenderer_h


#include "TerrainRenderer_shared.h"
#include "WorldRenderer_shared.h"
#include "Allocator.h"
#include <array>

class VegetableRenderer;

namespace CHA {
struct HabitatTextures
{
    MTL::Texture* diffSpecTextureArray;
    MTL::Texture* normalTextureArray;
};

class TerrainRenderer {
    friend class VegetableRenderer;
public:
    explicit TerrainRenderer( MTL::Device* pDevice, MTL::Library* pShaderLibrary );
    ~TerrainRenderer();
    
    void computeTesselationFactors(MTL::CommandBuffer* commandBuffer,
                                   const GpuBuffer& globalUniforms);
    
    void drawShadows(MTL::RenderCommandEncoder* renderEncoder,
                                const GpuBuffer& globalUniforms);
    
    void draw(MTL::RenderCommandEncoder* renderEncoder,
                         const GpuBuffer& globalUniforms);
    
    void computeUpdateHeightMap(MTL::CommandBuffer* commandBuffer,
                                const GpuBuffer& globalUniforms,
                                MTL::Buffer* mouseBuffer);
    
    
private:
    void    setComputePSO(MTL::Library* pShaderLibrar);
    void    CreateTerrainTextures();
    void    GenerateTerrainNormalMap(MTL::CommandBuffer* commandBuffer);
    void    GenerateTerrainPropertiesMap(MTL::CommandBuffer* commandBuffer);
    
    MTL::Device*                    m_pDevice;
    bool                            precomputationCompleted;
    
    // Terrain rendering data
    std::array<HabitatTextures, 4>  _terrainTextures;
    MTL::Buffer*                    _terrainParamsBuffer;
    MTL::Texture*                   _terrainHeight;
    MTL::Texture*                   _terrainNormalMap;
    MTL::Texture*                   _terrainPropertiesMap;
    MTL::Texture*                   _targetHeightmap;
    
    // Tesselation data
    MTL::Buffer*                    _visiblePatchesTessFactorBfr;
    MTL::Buffer*                    _visiblePatchIndicesBfr;
    float                           _tessellationScale;
    
    // Render pipelines
    MTL::RenderPipelineState*       _pplRnd_TerrainMainView;
    MTL::RenderPipelineState*       _pplRnd_TerrainShadow;
    const uint8_t                   _iabBufferIndex_PplTerrainMainView = 1;
    
    // Compute pipelines
    MTL::ComputePipelineState*      _pplCmp_FillInTesselationFactors;
    MTL::ComputePipelineState*      _pplCmp_BakeNormalsMips;
    MTL::ComputePipelineState*      _pplCmp_BakePropertiesMips;
    MTL::ComputePipelineState*      _pplCmp_ClearTexture;
    MTL::ComputePipelineState*      _pplCmp_UpdateHeightmap;
    MTL::ComputePipelineState*      _pplCmp_CopyChannel1Only;
};

inline int IabIndexForHabitatParam(TerrainHabitatType habType, TerrainHabitat_MemberIds memberId)
{
    return int (TerrainHabitat_MemberIds::COUNT) * habType + int (memberId);
}

template <typename T>
static void EncodeParam(MTL::ArgumentEncoder* paramsEncoder,
                        TerrainHabitatType habType,
                        TerrainHabitat_MemberIds memberId,
                        const T value)
{
    T* ptr = static_cast<T*>(paramsEncoder->constantData(IabIndexForHabitatParam(habType, memberId)));
    *ptr = value;
}

template <typename T>
static void EncodeParam(MTL::ArgumentEncoder* paramsEncoder,
                        TerrainParams_MemberIds memberId,
                        const T value)
{
    T* ptr = static_cast<T*>(paramsEncoder->constantData(int(memberId)));
    *ptr = value;
}

inline vector_float3 TerrainWorldBoundsMax()
{
    return (vector_float3) { TERRAIN_SCALE / -2.0f, 0, TERRAIN_SCALE / -2.0f };
}

inline vector_float3 TerrainWorldBoundsMin()
{
    return (vector_float3) { TERRAIN_SCALE / 2.0f, TERRAIN_HEIGHT, TERRAIN_SCALE / 2.0f};
}
};
#endif /* TerrainRenderer_h */
