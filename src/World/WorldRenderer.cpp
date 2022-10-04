//
//  Island
//
//  Created by Chaeyoung Lim on 2022/09/24.
//


#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTK_PRIVATE_IMPLEMENTATION
#include <MetalKit/MetalKit.hpp>
#undef MTK_PRIVATE_IMPLEMENTATION
#undef NS_PRIVATE_IMPLEMENTATION
#undef MTL_PRIVATE_IMPLEMENTATION
#undef CA_PRIVATE_IMPLEMENTATION



#include <simd/simd.h>
#include "DepthStencil.h"
#include "MathUtilities.h"
#include "Camera.h"
#import "WorldRenderer.h"
#import "ShaderTypes.h"
#include "Pipelines.h"
#include "Assert.h"
#include "Texture.h"

using namespace simd;

namespace CHA {

WorldRenderer::WorldRenderer( MTL::Device* pDevice, const MTL::Size& size )
: m_pDevice( pDevice ),
m_brushSize(1000.0f),
m_startTime( std::chrono::system_clock::now() )
{
    printf("Selected Device: %s\n", m_pDevice->name()->utf8String());
    m_pCommandQueue = m_pDevice->newCommandQueue();
    m_pShaderLibrary = m_pDevice->newDefaultLibrary();
    m_inFlightSemaphore  = dispatch_semaphore_create(kMaxBuffersInFlight);
    
    
    m_frameAllocator     = new Allocator (m_pDevice, 1024 * 1024 * 16, kMaxBuffersInFlight);
    m_uniforms_gpu       = m_frameAllocator->allocBuffer(1);
    m_onFrame            = 0;
    m_uniforms_cpu.gameTime = 0.f;
    m_terrainRenderer    = new TerrainRenderer(m_pDevice, m_pShaderLibrary);
    
    this->buildCamera(size);
    this->setDepthStencil(size);
    this->setVertexDescriptor();
    this->setPipelines();
    this->loadAssets();
}

WorldRenderer::~WorldRenderer()
{
    m_pDevice->release();
    m_pCommandQueue->release();
    m_pShadowDepthState->release();
    m_pShaderLibrary->release();
    m_pDynamicUniformBuffer->release();
    m_pDepthState->release();
    m_perlinMap->release();
    m_pDefaultVertexDescriptor->release();
    m_pVertexBuffer->release();
    m_pIndexBuffer->release();
    m_pCameraBuffer->release();
    m_skyCubeMap->release();
    
    delete m_pCamera;
    delete m_frameAllocator;
    delete m_terrainRenderer;
}

void WorldRenderer::setDepthStencil(const MTL::Size& size)
{
    m_shadowMap = buildShadowMapTexture(m_pDevice);

    MTL::DepthStencilDescriptor* depthStateDesc = MTL::DepthStencilDescriptor::alloc()->init();
    depthStateDesc->setDepthCompareFunction( MTL::CompareFunctionLess );
    depthStateDesc->setDepthWriteEnabled( true );
   
    m_shadowPassDesc = MTL::RenderPassDescriptor::alloc()->init();
    m_shadowPassDesc->depthAttachment()->setTexture( m_shadowMap );
    m_shadowPassDesc->depthAttachment()->setClearDepth( 1.f );
    m_shadowPassDesc->depthAttachment()->setLoadAction( MTL::LoadActionClear );
    m_shadowPassDesc->depthAttachment()->setStoreAction( MTL::StoreActionStore );
    m_pShadowDepthState = m_pDevice->newDepthStencilState(depthStateDesc);

    // GBuffer pass
    m_gBufferPassDesc = MTL::RenderPassDescriptor::alloc()->init();
    m_gBufferPassDesc->depthAttachment()->setClearDepth( 1.f );
    m_gBufferPassDesc->depthAttachment()->setLoadAction( MTL::LoadActionClear );
    m_gBufferPassDesc->depthAttachment()->setStoreAction( MTL::StoreActionStore );
    m_gBufferPassDesc->colorAttachments()->object(0)->setLoadAction( MTL::LoadActionDontCare );
    m_gBufferPassDesc->colorAttachments()->object(0)->setStoreAction( MTL::StoreActionStore );
    m_gBufferPassDesc->colorAttachments()->object(1)->setLoadAction( MTL::LoadActionDontCare );
    m_gBufferPassDesc->colorAttachments()->object(1)->setStoreAction( MTL::StoreActionStore );
    
    
    depthStateDesc->setDepthCompareFunction( MTL::CompareFunctionLess );
    depthStateDesc->setDepthWriteEnabled( true );
    
    m_gBufferDepthState = m_pDevice->newDepthStencilState(depthStateDesc);

    m_gBufferWithLoadPassDesc = m_gBufferPassDesc->copy();
    m_gBufferWithLoadPassDesc->depthAttachment()->setLoadAction( MTL::LoadActionLoad );
    m_gBufferWithLoadPassDesc->colorAttachments()->object(0)->setLoadAction( MTL::LoadActionLoad );
    m_gBufferWithLoadPassDesc->colorAttachments()->object(1)->setLoadAction( MTL::LoadActionLoad );
    
    m_lightingPassDesc = MTL::RenderPassDescriptor::alloc()->init();
    m_lightingPassDesc->colorAttachments()->object(0)->setLoadAction( MTL::LoadActionDontCare );
    m_lightingPassDesc->colorAttachments()->object(1)->setStoreAction( MTL::StoreActionStore );

    depthStateDesc->setDepthCompareFunction( MTL::CompareFunctionAlways );
    depthStateDesc->setDepthWriteEnabled( false );
    m_lightingDepthState = m_pDevice->newDepthStencilState(depthStateDesc);

    // Debug pass
    m_debugPassDesc = MTL::RenderPassDescriptor::alloc()->init();
    m_debugPassDesc->depthAttachment()->setLoadAction( MTL::LoadActionLoad );
    m_debugPassDesc->depthAttachment()->setStoreAction( MTL::StoreActionDontCare );
    m_debugPassDesc->colorAttachments()->object(0)->setLoadAction( MTL::LoadActionLoad );

    depthStateDesc->setDepthCompareFunction( MTL::CompareFunctionLess );
    depthStateDesc->setDepthWriteEnabled( true );
    m_debugDepthState = m_pDevice->newDepthStencilState(depthStateDesc);
    m_debugPassIsEnabled = false;
    
    this->drawableSizeWillChange(size, MTL::StorageModePrivate);
}

void WorldRenderer::setVertexDescriptor()
{
    // Positions.
    m_pDefaultVertexDescriptor = MTL::VertexDescriptor::alloc()->init();
    m_pDefaultVertexDescriptor->attributes()->object(VertexAttributePosition)->setFormat( MTL::VertexFormatFloat3 );
    m_pDefaultVertexDescriptor->attributes()->object(VertexAttributePosition)->setOffset( 0 );
    m_pDefaultVertexDescriptor->attributes()->object(VertexAttributePosition)->setBufferIndex( BufferIndexMeshPositions );

    // Texture coordinates.
    m_pDefaultVertexDescriptor->attributes()->object(VertexAttributeTexcoord)->setFormat( MTL::VertexFormatFloat2 );
    m_pDefaultVertexDescriptor->attributes()->object(VertexAttributeTexcoord)->setOffset( 0 );
    m_pDefaultVertexDescriptor->attributes()->object(VertexAttributeTexcoord)->setBufferIndex( BufferIndexMeshGenerics );

    // Position Buffer Layout
    m_pDefaultVertexDescriptor->layouts()->object(BufferIndexMeshPositions)->setStride( 12 );
    m_pDefaultVertexDescriptor->layouts()->object(BufferIndexMeshPositions)->setStepRate( 1 );
    m_pDefaultVertexDescriptor->layouts()->object(BufferIndexMeshPositions)->setStepFunction( MTL::VertexStepFunctionPerVertex );

    // Generic Attribute Buffer Layout
    m_pDefaultVertexDescriptor->layouts()->object(BufferIndexMeshGenerics)->setStride( 32 );
    m_pDefaultVertexDescriptor->layouts()->object(BufferIndexMeshGenerics)->setStepRate( 1 );
    m_pDefaultVertexDescriptor->layouts()->object(BufferIndexMeshGenerics)->setStepFunction( MTL::VertexStepFunctionPerVertex );
}

void WorldRenderer::setPipelines()
{
    m_pDepthState = buildBasicDepthStencilState(m_pDevice);
    uint16_t uniformBufferSize = kAlignedUniformsSize * kMaxBuffersInFlight;
    m_pDynamicUniformBuffer = m_pDevice->newBuffer(uniformBufferSize, MTL::ResourceStorageModeShared);
    m_pDynamicUniformBuffer->setLabel(CHASTR("UniformBuffer"));
    
    m_lightingPpl = createLightingRenderPSO(m_pDevice,
                                            m_pShaderLibrary,
                                            nullptr,
                                            CHASTR("LightingVs"),
                                            CHASTR("LightingPs"),
                                            CHASTR("Sky Rendering"));
    
    m_lightingPassDesc = MTL::RenderPassDescriptor::alloc()->init();
    m_lightingPassDesc->colorAttachments()->object(0)->setLoadAction( MTL::LoadActionDontCare );
    m_lightingPassDesc->colorAttachments()->object(1)->setStoreAction( MTL::StoreActionStore );
    m_lightingDepthState = buildLightingDepthStencilState(m_pDevice);
}

void WorldRenderer::buildCamera(const MTL::Size& size)
{
    m_pCamera = new Camera((vector_float3) { 50.0f, 1000.f, 50.f},
                           0.f,
                           0.f,
                           (float)size.width,
                           (float)size.height,
                           0.1f,
                           60000.0f);

    m_pCameraController = new CameraController(m_pCamera);
}

void WorldRenderer::loadAssets()
{
    /// Load assets into metal objects
    m_perlinMap  = CreateTextureWithDevice(m_pDevice, CHASTR("Assets/Data/Textures/perlinMap.png"), true, false, MTL::StorageModeManaged);
    m_skyCubeMap = CreateTextureWithDevice(m_pDevice, CHASTR("Assets/Data/Textures/skyCubeMap.ktx"), false, false)->newTextureView(MTL::PixelFormatRGBA8Unorm_sRGB);
}


void WorldRenderer::updateCPUUniforms()
{
    m_onFrame++;
    m_uniforms_cpu.cameraUniforms                = m_pCamera->getUniforms();
    float gameTime = m_onFrame * (1.0 / 60.f);
    m_uniforms_cpu.frameTime                     = MAX(0.001f, gameTime - 0);

    m_uniforms_cpu.mouseState                    = (vector_float3) { m_cursorPosition.x, m_cursorPosition.y, float(m_mouseButtonMask) };
    m_uniforms_cpu.invScreenSize                 = (vector_float2) {1.f / m_gBuffer0->width(), 1.f / m_gBuffer0->height() };
    m_uniforms_cpu.projectionYScale              = 1.73205066;
    m_uniforms_cpu.gameTime                      = (std::chrono::system_clock::now() - m_startTime).count();
    m_uniforms_cpu.ambientOcclusionContrast      = 3;
    m_uniforms_cpu.ambientOcclusionScale         = 0.800000011;
    m_uniforms_cpu.ambientLightScale             = 0.699999988;
    m_uniforms_cpu.brushSize                     = m_brushSize;

    {
        const float sun_yaw = -45.f;
        const float sun_pitch = -45.f;
        const float3 sunDirection = (normalize(matrix3x3_XRotation(radians_from_degrees(sun_pitch)) *
                                               matrix3x3_YRotation(radians_from_degrees(sun_yaw)) *
                                               (vector_float3){0.f, 0.f, 1.f}));

        float tan_half_angle = tanf(m_pCamera->getViewAngle() * .5f) * sqrtf(2.0);
        float half_angle = atanf(tan_half_angle);
        float sine_half_angle = sinf(half_angle);
        float cascade_sizes[NUM_CASCADES] = {400.0f, 1600.0f, 6400.0f };

        // Now the centers of the cone in distance to camera can be calulated
        float cascade_distances[NUM_CASCADES];
        cascade_distances[0] = 2 * cascade_sizes[0] * (1.0f - sine_half_angle * sine_half_angle);
        cascade_distances[1] = sqrtf(cascade_sizes[1]*cascade_sizes[1] - cascade_distances[0]*cascade_distances[0]*tan_half_angle*tan_half_angle) + cascade_distances[0];
        cascade_distances[2] = sqrtf(cascade_sizes[2]*cascade_sizes[2] - cascade_distances[1]*cascade_distances[1]*tan_half_angle*tan_half_angle) + cascade_distances[1];

        for (uint c = 0; c < NUM_CASCADES; c++)
        {
            float3 center = m_pCamera->getPosition() + m_pCamera->getDirection() * cascade_distances[c];
            float size = cascade_sizes[c];

            float stepsize = size/64.0f;
            Camera* shadow_cam  = new Camera(center - sunDirection*size, sun_yaw, sun_pitch, size*2.0f, size * 2.0f, 0.0f, size * 2);
            shadow_cam->setPosition(shadow_cam->getPosition() - fract(dot(center, shadow_cam->getUp()) /stepsize) * shadow_cam->getUp() * stepsize);
            shadow_cam->setPosition(shadow_cam->getPosition() - fract(dot(center, shadow_cam->getRight()) /stepsize) * shadow_cam->getRight() * stepsize);
            m_uniforms_cpu.shadowCameraUniforms[c] = shadow_cam->getUniforms();
        }
    }
}

void WorldRenderer::drawInView( void* view )
{
    MTK::View* pView = static_cast<MTK::View*>(view);
    MTL::CommandBuffer* pCmd = m_pCommandQueue->commandBuffer();
    pCmd->setLabel(CHASTR("Frame CB"));
    __block dispatch_semaphore_t block_sema = m_inFlightSemaphore;
    
    dispatch_semaphore_wait( m_inFlightSemaphore, DISPATCH_TIME_FOREVER );
    pCmd->addCompletedHandler( ^void( MTL::CommandBuffer* pCmd ){
        dispatch_semaphore_signal( block_sema );
    });
    
    
    this->updateCPUUniforms();
    m_uniforms_gpu.fillInWith(&m_uniforms_cpu, 1);
    m_terrainRenderer->computeTesselationFactors(pCmd, m_uniforms_gpu);
    
    for (uint32_t iCascade = 0; iCascade < NUM_CASCADES; iCascade++)
    {
        m_shadowPassDesc->depthAttachment()->setSlice(iCascade);
        MTL::RenderCommandEncoder* encoder = pCmd->renderCommandEncoder(m_shadowPassDesc);

        encoder->setCullMode(MTL::CullModeBack);
        encoder->setDepthClipMode(MTL::DepthClipModeClamp);
        encoder->setDepthStencilState(m_pShadowDepthState);
        encoder->setViewport(MTL::Viewport{0, 0, (double)m_shadowMap->width(), (double)m_shadowMap->height(), 0.f, 1.f});
        encoder->setScissorRect(MTL::ScissorRect{0, 0, m_shadowMap->width(), m_shadowMap->height()});
        encoder->setVertexBytes(&m_uniforms_cpu.shadowCameraUniforms[iCascade].viewProjectionMatrix, sizeof(matrix_float4x4), 6);

        m_terrainRenderer->drawShadows(encoder, m_uniforms_gpu);
        encoder->endEncoding();
    }
    
    // Geometry buffer pass
    {
        MTL::RenderCommandEncoder* pEnc = pCmd->renderCommandEncoder(m_gBufferPassDesc);

        pEnc->setCullMode(MTL::CullModeFront);
        pEnc->setDepthStencilState(m_gBufferDepthState);
        m_terrainRenderer->draw(pEnc, m_uniforms_gpu);
        pEnc->endEncoding();
        pEnc = pCmd->renderCommandEncoder(m_gBufferWithLoadPassDesc);
        pEnc->setDepthStencilState(m_gBufferDepthState);
        pEnc->endEncoding();
    }

    {
        MTL::RenderPassDescriptor* pRpd = pView->currentRenderPassDescriptor();
        m_lightingPassDesc->colorAttachments()->object(0)->setTexture( pRpd->colorAttachments()->object(0)->texture() );
        
        MTL::RenderCommandEncoder* encoder = pCmd->renderCommandEncoder(m_lightingPassDesc);
        encoder->setRenderPipelineState( m_lightingPpl );
        encoder->setDepthStencilState( m_lightingDepthState );
        encoder->setFragmentTexture( m_gBuffer0, 0 );
        encoder->setFragmentTexture( m_gBuffer1, 1 );
        encoder->setFragmentTexture( m_depth, 2 );
        encoder->setFragmentTexture( m_shadowMap, 3 );
        encoder->setFragmentTexture( m_skyCubeMap, 4 );
        encoder->setFragmentTexture( m_perlinMap, 5 );
        encoder->setFragmentBuffer( m_uniforms_gpu.getBuffer(), m_uniforms_gpu.getOffset(), 0 );
        encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, (NS::UInteger)0, (NS::UInteger)3);
        encoder->endEncoding();
    }

    pCmd->presentDrawable(pView->currentDrawable());
    
    pCmd->commit();
}

void WorldRenderer::drawableSizeWillChange(const MTL::Size& size, MTL::StorageMode GBufferStorageMode)
{
    // Recreate textures
    MTL::TextureDescriptor* texDesc = MTL::TextureDescriptor::texture2DDescriptor( MTL::PixelFormatBGRA8Unorm_sRGB, size.width, size.height, false);
    
    texDesc->setUsage(MTL::TextureUsageRenderTarget | MTL::TextureUsageShaderRead);
    texDesc->setSampleCount( 1 );
    texDesc->setStorageMode( MTL::StorageModePrivate);
    m_gBuffer0 = m_pDevice->newTexture(texDesc);
    texDesc->setPixelFormat( MTL::PixelFormatRGBA8Unorm );
    m_gBuffer1 = m_pDevice->newTexture(texDesc);
    texDesc->setStorageMode( MTL::StorageModePrivate );
    texDesc->setPixelFormat( MTL::PixelFormatDepth32Float_Stencil8 );
    m_depth = m_pDevice->newTexture(texDesc);
    
    m_gBufferPassDesc->depthAttachment()->setTexture( m_depth );
    m_debugPassDesc->depthAttachment()->setTexture( m_depth );
    m_gBufferPassDesc->colorAttachments()->object(0)->setTexture(m_gBuffer0);
    m_gBufferPassDesc->colorAttachments()->object(1)->setTexture(m_gBuffer1);
    m_gBufferWithLoadPassDesc->depthAttachment()->setTexture(m_depth);
    m_gBufferWithLoadPassDesc->colorAttachments()->object(0)->setTexture(m_gBuffer0);
    m_gBufferWithLoadPassDesc->colorAttachments()->object(1)->setTexture(m_gBuffer1);

    m_pCamera->setAspectRatio( size.width / (float)size.height );
};

}
