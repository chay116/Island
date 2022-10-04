//
//  TerrainRenderer.cpp
//  Island
//
//  Created by Chaeyoung Lim on 2022/09/28.
//


#include <MetalKit/MetalKit.hpp>


#include "TerrainRenderer.h"
#include "MathUtilities.h"
#include "Pipelines.h"
#include "Texture.h"
#include "Assert.h"
#include <vector>

using namespace simd;

namespace CHA {
void EncodeParamsFromData(MTL::ArgumentEncoder* encoder, TerrainHabitatType curHabitat, const std::array <const TerrainHabitat::ParticleProperties*, 4>& particleProperties, const std::array <HabitatTextures, 4>& terrainTextures)
{
    EncodeParam (encoder, curHabitat, TerrainHabitat_MemberIds::particle_keyTimePoints,        particleProperties[curHabitat]->keyTimePoints);
    EncodeParam (encoder, curHabitat, TerrainHabitat_MemberIds::particle_scaleFactors,         particleProperties[curHabitat]->scaleFactors);
    EncodeParam (encoder, curHabitat, TerrainHabitat_MemberIds::particle_alphaFactors,         particleProperties[curHabitat]->alphaFactors);
    EncodeParam (encoder, curHabitat, TerrainHabitat_MemberIds::particle_gravity,              particleProperties[curHabitat]->gravity);
    EncodeParam (encoder, curHabitat, TerrainHabitat_MemberIds::particle_lightingCoefficients, particleProperties[curHabitat]->lightingCoefficients);
    EncodeParam (encoder, curHabitat, TerrainHabitat_MemberIds::particle_doesCollide,          particleProperties[curHabitat]->doesCollide);
    EncodeParam (encoder, curHabitat, TerrainHabitat_MemberIds::particle_doesRotate,           particleProperties[curHabitat]->doesRotate);
    EncodeParam (encoder, curHabitat, TerrainHabitat_MemberIds::particle_castShadows,          particleProperties[curHabitat]->castShadows);
    EncodeParam (encoder, curHabitat, TerrainHabitat_MemberIds::particle_distanceDependent,    particleProperties[curHabitat]->distanceDependent);
    encoder->setTexture(terrainTextures[curHabitat].diffSpecTextureArray, IabIndexForHabitatParam(curHabitat, TerrainHabitat_MemberIds::diffSpecTextureArray));
    encoder->setTexture(terrainTextures[curHabitat].normalTextureArray, IabIndexForHabitatParam(curHabitat, TerrainHabitat_MemberIds::normalTextureArray));
};

void TerrainRenderer::computeTesselationFactors(MTL::CommandBuffer* commandBuffer,
                               const GpuBuffer& globalUniforms)
{
    MTL::ComputeCommandEncoder* computeEncoder = commandBuffer->computeCommandEncoder();

    computeEncoder->setComputePipelineState(_pplCmp_FillInTesselationFactors);
    computeEncoder->setBuffer(_visiblePatchesTessFactorBfr, 0, 0);
    computeEncoder->setBuffer(_visiblePatchIndicesBfr, 0, 2);
    computeEncoder->setBytes(&_tessellationScale, sizeof(float), 3);
    computeEncoder->setBuffer(globalUniforms.getBuffer(), globalUniforms.getOffset(), 4);
    computeEncoder->setTexture(_terrainHeight, 0);
    computeEncoder->dispatchThreadgroups(MTL::Size::Make(2, 2, 1), MTL::Size::Make(16, 16, 1));
    computeEncoder->endEncoding();
}

TerrainRenderer::TerrainRenderer( MTL::Device* pDevice, MTL::Library* pShaderLibrary )
{
    m_pDevice = pDevice;

    precomputationCompleted = false;
    MTL::CommandQueue* queue = m_pDevice->newCommandQueue();
    MTL::CommandBuffer* commandBuffer = queue->commandBuffer();
    commandBuffer->addCompletedHandler(^(MTL::CommandBuffer* commandBuffer) {
        this->precomputationCompleted = true;
    });

    // Loading the textures used by the terrain
    this->CreateTerrainTextures();
    this->setComputePSO(pShaderLibrary);


    MTL::Function* terrainShadingFunc = pShaderLibrary->newFunction(CHASTR("terrain_fragment"));
    MTL::ArgumentEncoder* paramsEncoder = terrainShadingFunc->newArgumentEncoder(1);
    CHA_ASSERT(paramsEncoder, "ParamEncoder Error");

    _terrainParamsBuffer = m_pDevice->newBuffer(paramsEncoder->encodedLength(), MTL::ResourceStorageModeManaged);

    paramsEncoder->setArgumentBuffer(_terrainParamsBuffer, 0);

    TerrainHabitat::ParticleProperties chunkyParticle;
    chunkyParticle.keyTimePoints = (float4){0.0,4.0,5.0,6.0};
    chunkyParticle.scaleFactors = (float4){1.0,1.0,1.0,0.4};
    chunkyParticle.alphaFactors = (float4){1.0,1.0,1.0,0.0};
    chunkyParticle.gravity = (float4){0.0f,-400.0f,0.0f,0.0f}; // includes weight
    chunkyParticle.lightingCoefficients = (float4){1.0f,0.0f,0.0f,0.0f};
    chunkyParticle.doesCollide = 1;
    chunkyParticle.doesRotate = 1;
    chunkyParticle.castShadows = 1;
    chunkyParticle.distanceDependent = 0;

    TerrainHabitat::ParticleProperties puffyParticle;
    puffyParticle.keyTimePoints = (float4){0.0,0.3,0.8,1.2};
    puffyParticle.scaleFactors = ((float4){0.0,0.8,2.0,2.9})*0.3f;
    puffyParticle.alphaFactors = (float4){1.0,1.0,1.0,0.0};
    puffyParticle.gravity = (float4){0.0f,-50.0f,0.0f,0.0}; // includes weight
    puffyParticle.lightingCoefficients = (float4){0.0f,1.0f,0.0f,0.0f};
    puffyParticle.doesCollide = 0;
    puffyParticle.doesRotate = 0;
    puffyParticle.castShadows = 0;
    puffyParticle.distanceDependent = 1;

    std::array <const TerrainHabitat::ParticleProperties*, 4> particleProperties =
    {
        &puffyParticle,
        &puffyParticle,
        &chunkyParticle,
        &chunkyParticle,
    };

    // Configure the various terrain "habitats."
    // - these are the look-and-feel of visually distinct areas that differ by elevation
    TerrainHabitatType curHabitat;

    curHabitat = TerrainHabitatTypeSand;
    EncodeParam (paramsEncoder, curHabitat, TerrainHabitat_MemberIds::slopeStrength, 100.f);
    EncodeParam (paramsEncoder, curHabitat, TerrainHabitat_MemberIds::slopeThreshold, 0.f);
    EncodeParam (paramsEncoder, curHabitat, TerrainHabitat_MemberIds::elevationStrength, 100.f);
    EncodeParam (paramsEncoder, curHabitat, TerrainHabitat_MemberIds::elevationThreshold, 0.f);
    EncodeParam (paramsEncoder, curHabitat, TerrainHabitat_MemberIds::specularPower, 32.f);
    EncodeParam (paramsEncoder, curHabitat, TerrainHabitat_MemberIds::textureScale, 0.001f);
    EncodeParam (paramsEncoder, curHabitat, TerrainHabitat_MemberIds::flipNormal, false);
    EncodeParamsFromData(paramsEncoder, curHabitat, particleProperties, _terrainTextures);

    curHabitat = TerrainHabitatTypeGrass;
    EncodeParam (paramsEncoder, curHabitat, TerrainHabitat_MemberIds::slopeStrength, 100.f);
    EncodeParam (paramsEncoder, curHabitat, TerrainHabitat_MemberIds::slopeThreshold, 0.f);
    EncodeParam (paramsEncoder, curHabitat, TerrainHabitat_MemberIds::elevationStrength, 40.f);
    EncodeParam (paramsEncoder, curHabitat, TerrainHabitat_MemberIds::elevationThreshold, 0.146f);
    EncodeParam (paramsEncoder, curHabitat, TerrainHabitat_MemberIds::specularPower, 32.f);
    EncodeParam (paramsEncoder, curHabitat, TerrainHabitat_MemberIds::textureScale, 0.001f);
    EncodeParam (paramsEncoder, curHabitat, TerrainHabitat_MemberIds::flipNormal, false);
    EncodeParamsFromData(paramsEncoder, curHabitat, particleProperties, _terrainTextures);

    curHabitat = TerrainHabitatTypeRock;
    EncodeParam (paramsEncoder, curHabitat, TerrainHabitat_MemberIds::slopeStrength, 100.f);
    EncodeParam (paramsEncoder, curHabitat, TerrainHabitat_MemberIds::slopeThreshold, 0.f);
    EncodeParam (paramsEncoder, curHabitat, TerrainHabitat_MemberIds::elevationStrength, 40.f);
    EncodeParam (paramsEncoder, curHabitat, TerrainHabitat_MemberIds::elevationThreshold, 0.28f);
    EncodeParam (paramsEncoder, curHabitat, TerrainHabitat_MemberIds::specularPower, 32.f);
    EncodeParam (paramsEncoder, curHabitat, TerrainHabitat_MemberIds::textureScale, 0.002f);
    EncodeParam (paramsEncoder, curHabitat, TerrainHabitat_MemberIds::flipNormal, false);
    EncodeParamsFromData (paramsEncoder, curHabitat, particleProperties, _terrainTextures);

    curHabitat = TerrainHabitatTypeSnow;
    EncodeParam (paramsEncoder, curHabitat, TerrainHabitat_MemberIds::slopeStrength, 43.f);
    EncodeParam (paramsEncoder, curHabitat, TerrainHabitat_MemberIds::slopeThreshold, 0.612f);
    EncodeParam (paramsEncoder, curHabitat, TerrainHabitat_MemberIds::elevationStrength, 100.f);
    EncodeParam (paramsEncoder, curHabitat, TerrainHabitat_MemberIds::elevationThreshold, 0.39f);
    EncodeParam (paramsEncoder, curHabitat, TerrainHabitat_MemberIds::specularPower, 32.f);
    EncodeParam (paramsEncoder, curHabitat, TerrainHabitat_MemberIds::textureScale, 0.002f);
    EncodeParam (paramsEncoder, curHabitat, TerrainHabitat_MemberIds::flipNormal, false);
    EncodeParamsFromData (paramsEncoder, curHabitat, particleProperties, _terrainTextures);

    EncodeParam (paramsEncoder, TerrainParams_MemberIds::ambientOcclusionScale, 0.f);
    EncodeParam (paramsEncoder, TerrainParams_MemberIds::ambientOcclusionContrast, 0.f);
    EncodeParam (paramsEncoder, TerrainParams_MemberIds::ambientLightScale, 0.f);
    EncodeParam (paramsEncoder, TerrainParams_MemberIds::atmosphereScale, 0.f);

    _terrainParamsBuffer->didModifyRange(NS::Range::Make(0, _terrainParamsBuffer->length()));


    // Use a height map to define the initial terrain topography
    _targetHeightmap = CreateTextureWithDevice(m_pDevice,CHASTR("Assets/Data/Textures/TerrainHeightMap.png"), false, false);

    const uint16_t heightMapWidth = _targetHeightmap->width();
    const uint16_t heightMapHeight = _targetHeightmap->height();

    // Use TerrainKnl_CopyChannel1Only to ignore possible alpha channel and create _terrainHeight texture.
    {
        MTL::Texture* _srcTex = _targetHeightmap;
        _srcTex->setLabel(CHASTR("SourceTerrain") );

        MTL::TextureDescriptor *texDesc = MTL::TextureDescriptor::texture2DDescriptor(MTL::PixelFormatR32Float,
                                                                                      heightMapWidth,
                                                                                      heightMapHeight,
                                                                                      false);
        texDesc->setUsage( MTL::TextureUsageShaderRead | MTL::TextureUsageShaderWrite );
        texDesc->setStorageMode( MTL::StorageModePrivate );
        MTL::Texture* _dstTex = m_pDevice->newTexture(texDesc);
        _dstTex->setLabel(CHASTR("CopiedTerrain"));

        assert(((heightMapWidth  / 16)*16) == heightMapWidth);
        assert(((heightMapHeight / 16)*16) == heightMapHeight);

        MTL::ComputeCommandEncoder* computeEncoder = commandBuffer->computeCommandEncoder();
        computeEncoder->setLabel(CHASTR("CopyRG16ToR16"));
        computeEncoder->setComputePipelineState(_pplCmp_CopyChannel1Only);
        computeEncoder->setTexture(_srcTex, 0);
        computeEncoder->setTexture(_dstTex, 1);
        computeEncoder->dispatchThreads(MTL::Size(heightMapWidth, heightMapHeight, 1), MTL::Size(16, 16, 1));
        computeEncoder->endEncoding();

        _terrainHeight = _dstTex;
    }

    // Create normals and props textures
    {
        MTL::TextureDescriptor* texDesc = MTL::TextureDescriptor::alloc()->init();
        texDesc->setWidth(heightMapWidth);
        texDesc->setHeight(heightMapHeight);
        texDesc->setPixelFormat( MTL::PixelFormatRG11B10Float );
        texDesc->setUsage( MTL::TextureUsageShaderRead | MTL::TextureUsageShaderWrite );
        texDesc->setMipmapLevelCount( std::log2(MAX(heightMapWidth, heightMapHeight)) + 1 );
        texDesc->setStorageMode( MTL::StorageModePrivate );
        _terrainNormalMap = m_pDevice->newTexture( texDesc );
        this->GenerateTerrainNormalMap( commandBuffer );

        texDesc->setPixelFormat( MTL::PixelFormatRGBA8Unorm );
        _terrainPropertiesMap = m_pDevice->newTexture(texDesc);

        // We need to clear the properties map as 'GenerateTerrainPropertiesMap' will only fill in specific color channels
        {
            MTL::ComputeCommandEncoder* encoder = commandBuffer->computeCommandEncoder();
            encoder->setComputePipelineState(_pplCmp_ClearTexture);
            encoder->setTexture(_terrainPropertiesMap, 0);
            encoder->dispatchThreads(MTL::Size(heightMapWidth, heightMapHeight, 1), MTL::Size(8, 8, 1));
            encoder->endEncoding();
        }
        this->GenerateTerrainPropertiesMap(commandBuffer);

        {
            MTL::BlitCommandEncoder* blit = commandBuffer->blitCommandEncoder();
            blit->generateMipmaps(_terrainNormalMap);
            blit->generateMipmaps(_terrainPropertiesMap);
            blit->endEncoding();
        }
    }

    // Loading rendering pipelines
    {
        MTL::FunctionConstantValues *constants = MTL::FunctionConstantValues::alloc()->init();
        constexpr bool no = false;
        constants->setConstantValue(&no, MTL::DataTypeBool, (NS::UInteger)0);

        _pplRnd_TerrainMainView = createTessRenderPSO(m_pDevice,
                                                      pShaderLibrary,
                                                      nullptr,
                                                      constants,
                                                      CHASTR("terrain_vertex"),
                                                      CHASTR("terrain_fragment"),
                                                      CHASTR("Terrain"));


        // Create the depth only pipeline for the shadow view
        constexpr bool yes = true;
        constants->setConstantValue(&yes, MTL::DataTypeBool, (NS::UInteger)0);

        _pplRnd_TerrainShadow = creatShadowPSO(m_pDevice,
                                               pShaderLibrary,
                                               nullptr,
                                               constants,
                                               CHASTR("terrain_vertex"),
                                               CHASTR("Terrain Shadow"));
    }

    _tessellationScale = 25.0f;
    _visiblePatchIndicesBfr = m_pDevice->newBuffer(sizeof(uint32_t) * TERRAIN_PATCHES * TERRAIN_PATCHES,
                                                   MTL::ResourceStorageModePrivate);
    _visiblePatchesTessFactorBfr = m_pDevice->newBuffer(sizeof(MTL::QuadTessellationFactorsHalf)
                                                        * TERRAIN_PATCHES * TERRAIN_PATCHES,
                                                        MTL::ResourceStorageModePrivate);

    commandBuffer->commit();
}


TerrainRenderer::~TerrainRenderer()
{
    for (int i=0; i < 4; i++) {
        _terrainTextures[i].diffSpecTextureArray->release();
        _terrainTextures[i].normalTextureArray->release();
    }

    _terrainParamsBuffer->release();
    _terrainHeight->release();
    _terrainNormalMap->release();
    _terrainPropertiesMap->release();
    _targetHeightmap->release();
    _visiblePatchesTessFactorBfr->release();
    _visiblePatchIndicesBfr->release();
    _pplRnd_TerrainMainView->release();
    _pplRnd_TerrainShadow->release();
    _pplCmp_FillInTesselationFactors->release();
    _pplCmp_BakeNormalsMips->release();
    _pplCmp_BakePropertiesMips->release();
    _pplCmp_ClearTexture->release();
    _pplCmp_UpdateHeightmap->release();
    _pplCmp_CopyChannel1Only->release();
}

void TerrainRenderer::setComputePSO(MTL::Library* pShaderLibrary)
{
    // Create the compute pipelines
    //  - this is needed further along in data initialization
    _pplCmp_FillInTesselationFactors = creatComputePSO(m_pDevice, pShaderLibrary, CHASTR("TerrainKnl_FillInTesselationFactors"));
    _pplCmp_BakePropertiesMips       = creatComputePSO(m_pDevice, pShaderLibrary, CHASTR("TerrainKnl_ComputeOcclusionAndSlopeFromHeightmap"));
    _pplCmp_BakeNormalsMips          = creatComputePSO(m_pDevice, pShaderLibrary, CHASTR("TerrainKnl_ComputeNormalsFromHeightmap"));
    _pplCmp_ClearTexture             = creatComputePSO(m_pDevice, pShaderLibrary, CHASTR("TerrainKnl_ClearTexture"));
    _pplCmp_UpdateHeightmap          = creatComputePSO(m_pDevice, pShaderLibrary, CHASTR("TerrainKnl_UpdateHeightmap"));
    _pplCmp_CopyChannel1Only         = creatComputePSO(m_pDevice, pShaderLibrary, CHASTR("TerrainKnl_CopyChannel1Only"));
}

/// Set Textures
void TerrainRenderer::CreateTerrainTextures()
{
    //sand
    _terrainTextures[0].diffSpecTextureArray = CreateTextureWithDevice(m_pDevice,CHASTR("Assets/Data/Textures/terrain_sand_diffspec_array.ktx"), false, false)->newTextureView(MTL::PixelFormatRGBA8Unorm_sRGB);
    _terrainTextures[0].normalTextureArray = CreateTextureWithDevice(m_pDevice,CHASTR("Assets/Data/Textures/terrain_sand_normal_array.ktx"), false, false);

    //grass
    _terrainTextures[1].diffSpecTextureArray = CreateTextureWithDevice(m_pDevice,CHASTR("Assets/Data/Textures/terrain_grass_diffspec_array.ktx"), false, false)->newTextureView(MTL::PixelFormatRGBA8Unorm_sRGB);
    _terrainTextures[1].normalTextureArray = CreateTextureWithDevice(m_pDevice,CHASTR("Assets/Data/Textures/terrain_grass_normal_array.ktx"), false, false);

    //rock
    _terrainTextures[2].diffSpecTextureArray = CreateTextureWithDevice(m_pDevice,CHASTR("Assets/Data/Textures/terrain_rock_diffspec_array.ktx"), false, false)->newTextureView(MTL::PixelFormatRGBA8Unorm_sRGB);
    _terrainTextures[2].normalTextureArray = CreateTextureWithDevice(m_pDevice,CHASTR("Assets/Data/Textures/terrain_rock_normal_array.ktx"), false, false);

    //snow
    _terrainTextures[3].diffSpecTextureArray = CreateTextureWithDevice(m_pDevice,CHASTR("Assets/Data/Textures/terrain_snow_diffspec_array.ktx"), false, false)->newTextureView(MTL::PixelFormatRGBA8Unorm_sRGB);
    _terrainTextures[3].normalTextureArray = CreateTextureWithDevice(m_pDevice,CHASTR("Assets/Data/Textures/terrain_snow_normal_array.ktx"), false, false);
}


void TerrainRenderer::GenerateTerrainNormalMap(MTL::CommandBuffer* commandBuffer)
{
    MTL::ComputeCommandEncoder* computeEncoder = commandBuffer->computeCommandEncoder();

    assert (((_terrainHeight->width()  / 16)*16) == _terrainHeight->width());
    assert (((_terrainHeight->height() / 16)*16) == _terrainHeight->height());

    computeEncoder->setComputePipelineState(_pplCmp_BakeNormalsMips);
    computeEncoder->setTexture(_terrainHeight, 0);
    computeEncoder->setTexture(_terrainNormalMap, 1);
    computeEncoder->dispatchThreadgroups(MTL::Size(_terrainHeight->width(), _terrainHeight->height(), 1), MTL::Size(8, 8, 1));
    computeEncoder->endEncoding();
}

void TerrainRenderer::GenerateTerrainPropertiesMap(MTL::CommandBuffer* commandBuffer)
{
    auto GenerateSamplesBuffer = [] (MTL::Device* device, int numSamples)
    {
        std::vector<vector_float2> res;

        srandom(12345);
        const float sampleRadius = 32.0f;

        for (int i = 0; i < numSamples; i++)
        {
            float u = (float)random() / (float)RAND_MAX;
            float v = (float)random() / (float)RAND_MAX;

            float r = sqrtf(u);
            float theta = 2.0f * (float)M_PI * v;
            res.push_back((vector_float2) {cosf(theta), sinf(theta)} * r * sampleRadius);
        }

        MTL::Buffer* buffer = device->newBuffer(res.data(),
                                                   res.size()*sizeof(float),
                                                   MTL::ResourceStorageModeManaged);
        return buffer;
    };

    static constexpr int numSamples = 256;
    static MTL::Buffer* sampleBuffer = GenerateSamplesBuffer(m_pDevice, numSamples);

    MTL::ComputeCommandEncoder* computeEncoder = commandBuffer->computeCommandEncoder();

    computeEncoder->setComputePipelineState(_pplCmp_BakePropertiesMips);
    computeEncoder->setTexture(_terrainHeight, 0);
    computeEncoder->setTexture(_terrainPropertiesMap, 1);
    computeEncoder->setBuffer(sampleBuffer, 0, 0);
    computeEncoder->setBytes(&numSamples, sizeof(numSamples), 1);

    packed_float2 invSize = {1.f / _terrainHeight->width(), 1.f / _terrainHeight->height()};
    computeEncoder->setBytes(&invSize, sizeof(invSize), 2);
    computeEncoder->dispatchThreads(MTL::Size::Make(_terrainHeight->width(), _terrainHeight->height(), 1),
                                    MTL::Size::Make(16, 16, 1));
    computeEncoder->endEncoding();
}


 


void TerrainRenderer::drawShadows(MTL::RenderCommandEncoder* renderEncoder,
                            const GpuBuffer& globalUniforms)
{
    renderEncoder->setRenderPipelineState(_pplRnd_TerrainShadow);
    renderEncoder->setDepthBias(0.001, 2, 1);
    renderEncoder->setTessellationFactorBuffer(_visiblePatchesTessFactorBfr, 0, 0);
    renderEncoder->setCullMode(MTL::CullModeBack);
    renderEncoder->setVertexBuffer(globalUniforms.getBuffer(), globalUniforms.getOffset(), 1);
    renderEncoder->setVertexTexture(_terrainHeight, 0);
    renderEncoder->drawPatches(4, 0, TERRAIN_PATCHES * TERRAIN_PATCHES, _visiblePatchIndicesBfr, 0, 1, 0);
}

// The terrain main rendering pass
void TerrainRenderer::draw(MTL::RenderCommandEncoder* renderEncoder,
                     const GpuBuffer& globalUniforms)
{
    renderEncoder->setRenderPipelineState(_pplRnd_TerrainMainView);

    // - Note: depth stencil state is already set by the main renderer

    // Indicate to Metal that these resources will be accessed by the GPU and therefore
    //   must be mapped to the GPU's address space
    for (int i = 0; i < _terrainTextures.size(); i++)
    {
        renderEncoder->useResource(_terrainTextures[i].diffSpecTextureArray, MTL::ResourceUsageSample | MTL::ResourceUsageRead);
        renderEncoder->useResource(_terrainTextures[i].normalTextureArray, MTL::ResourceUsageSample | MTL::ResourceUsageRead);
    }

    renderEncoder->setTessellationFactorBuffer(_visiblePatchesTessFactorBfr, 0, 0);
    renderEncoder->setVertexBuffer(globalUniforms.getBuffer(), globalUniforms.getOffset(), 1);
    renderEncoder->setVertexTexture(_terrainHeight, 0);

    // Set the argument buffer
    renderEncoder->setFragmentBuffer(_terrainParamsBuffer, 0, _iabBufferIndex_PplTerrainMainView);
    renderEncoder->setFragmentBuffer(globalUniforms.getBuffer(), globalUniforms.getOffset(), 2);
    renderEncoder->setFragmentTexture(_targetHeightmap, 0);
    renderEncoder->setFragmentTexture(_terrainNormalMap, 1);
//    renderEncoder->setFragmentTexture(_terrainPropertiesMap, 2);
    renderEncoder->drawPatches(4, 0, TERRAIN_PATCHES*TERRAIN_PATCHES, _visiblePatchIndicesBfr, 0, 1, 0);
}

void TerrainRenderer::computeUpdateHeightMap(MTL::CommandBuffer* commandBuffer,
                            const GpuBuffer& globalUniforms,
                            MTL::Buffer* mouseBuffer)
{
    MTL::ComputeCommandEncoder* computeEncoder = commandBuffer->computeCommandEncoder();

    computeEncoder->setComputePipelineState(_pplCmp_UpdateHeightmap);
    computeEncoder->setTexture(_terrainHeight, 0);
    computeEncoder->setBuffer(mouseBuffer, 0, 0);
    computeEncoder->setBuffer(globalUniforms.getBuffer(), globalUniforms.getOffset(), 1);
    computeEncoder->dispatchThreadgroups(MTL::Size(_terrainHeight->width()/8, _terrainHeight->height()/8, 1), MTL::Size(8, 8, 1));
    computeEncoder->endEncoding();
    this->GenerateTerrainNormalMap(commandBuffer);
    this->GenerateTerrainPropertiesMap(commandBuffer);

    MTL::BlitCommandEncoder* blit = commandBuffer->blitCommandEncoder();
    blit->generateMipmaps(_terrainNormalMap);
    blit->generateMipmaps(_terrainPropertiesMap);
    blit->endEncoding();
}
};
