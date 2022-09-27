//
//  Island
//
//  Created by Chaeyoung Lim on 2022/09/24.
//

#include<sys/sysctl.h>
#include <simd/simd.h>
#include <stdlib.h>

#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTK_PRIVATE_IMPLEMENTATION
#include <MetalKit/MetalKit.hpp>
#undef MTK_PRIVATE_IMPLEMENTATION
#undef NS_PRIVATE_IMPLEMENTATION
#undef MTL_PRIVATE_IMPLEMENTATION
#undef CA_PRIVATE_IMPLEMENTATION



#include "DepthStencil.h"
#include "MathUtilities.h"
#include "Camera.h"
#import "WorldRenderer.h"
#import "ShaderTypes.h"
#include "Pipelines.h"
#include "Assert.h"
#include "Texture.h"

// Include header shared between C code here, which executes Metal API commands, and .metal files

namespace CHA {


WorldRenderer::WorldRenderer( MTL::Device* pDevice, const MTL::Size& size )
: m_pDevice( pDevice->retain() ), m_startTime( std::chrono::system_clock::now() )
{
    printf("Selected Device: %s\n", m_pDevice->name()->utf8String());
    m_pCommandQueue = m_pDevice->newCommandQueue();
    m_pShaderLibrary = m_pDevice->newDefaultLibrary();

    // We allow up to three command buffers to be in flight on GPU before we wait
    m_inFlightSemaphore  = dispatch_semaphore_create(kMaxBuffersInFlight);

    
    this->buildCamera(size);
    this->setVertexDescriptor();
    this->setPipelines();
    this->loadAssets();
    this->buildBuffers();
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
    /// set RenderPipelines
    m_pPipelineState = createRenderPSO(m_pDevice,
                                        m_pShaderLibrary,
                                        m_pDefaultVertexDescriptor,
                                        CHASTR("vertexShader"),
                                        CHASTR("fragmentShader"),
                                        CHASTR("My Pipeline"));
        
    /// set DepthStencil
    m_pDepthState = buildBasicDepthStencilState(m_pDevice);
    
    
    /// set Buffer
    uint16_t uniformBufferSize = kAlignedUniformsSize * kMaxBuffersInFlight;

    m_pDynamicUniformBuffer = m_pDevice->newBuffer(uniformBufferSize, MTL::ResourceStorageModeShared);
    m_pDynamicUniformBuffer->setLabel(CHASTR("UniformBuffer"));
}

void WorldRenderer::buildCamera(const MTL::Size& size)
{
    m_pCamera = new Camera((vector_float3) { 0.0f, 0.f, 10.f},
                           180.f,
                           0.f,
                           (float)size.width,
                           (float)size.height,
                           0.1f,
                           100.0f);

    m_pCameraController = new CameraController(m_pCamera);
}

void WorldRenderer::buildBuffers()
{
    
    const float s = 0.5f;
    
    ObjVertex verts[] = {
        //                                         Texture
        //   Positions           Normals         Coordinates
        { { -s, -s, +s }, {  0.f,  0.f,  1.f }, { 0.f, 1.f } },
        { { +s, -s, +s }, {  0.f,  0.f,  1.f }, { 1.f, 1.f } },
        { { +s, +s, +s }, {  0.f,  0.f,  1.f }, { 1.f, 0.f } },
        { { -s, +s, +s }, {  0.f,  0.f,  1.f }, { 0.f, 0.f } },

        { { +s, -s, +s }, {  1.f,  0.f,  0.f }, { 0.f, 1.f } },
        { { +s, -s, -s }, {  1.f,  0.f,  0.f }, { 1.f, 1.f } },
        { { +s, +s, -s }, {  1.f,  0.f,  0.f }, { 1.f, 0.f } },
        { { +s, +s, +s }, {  1.f,  0.f,  0.f }, { 0.f, 0.f } },

        { { +s, -s, -s }, {  0.f,  0.f, -1.f }, { 0.f, 1.f } },
        { { -s, -s, -s }, {  0.f,  0.f, -1.f }, { 1.f, 1.f } },
        { { -s, +s, -s }, {  0.f,  0.f, -1.f }, { 1.f, 0.f } },
        { { +s, +s, -s }, {  0.f,  0.f, -1.f }, { 0.f, 0.f } },

        { { -s, -s, -s }, { -1.f,  0.f,  0.f }, { 0.f, 1.f } },
        { { -s, -s, +s }, { -1.f,  0.f,  0.f }, { 1.f, 1.f } },
        { { -s, +s, +s }, { -1.f,  0.f,  0.f }, { 1.f, 0.f } },
        { { -s, +s, -s }, { -1.f,  0.f,  0.f }, { 0.f, 0.f } },

        { { -s, +s, +s }, {  0.f,  1.f,  0.f }, { 0.f, 1.f } },
        { { +s, +s, +s }, {  0.f,  1.f,  0.f }, { 1.f, 1.f } },
        { { +s, +s, -s }, {  0.f,  1.f,  0.f }, { 1.f, 0.f } },
        { { -s, +s, -s }, {  0.f,  1.f,  0.f }, { 0.f, 0.f } },

        { { -s, -s, -s }, {  0.f, -1.f,  0.f }, { 0.f, 1.f } },
        { { +s, -s, -s }, {  0.f, -1.f,  0.f }, { 1.f, 1.f } },
        { { +s, -s, +s }, {  0.f, -1.f,  0.f }, { 1.f, 0.f } },
        { { -s, -s, +s }, {  0.f, -1.f,  0.f }, { 0.f, 0.f } }
    };

    uint16_t indices[] = {
         0,  1,  2,  2,  3,  0, /* front */
         4,  5,  6,  6,  7,  4, /* right */
         8,  9, 10, 10, 11,  8, /* back */
        12, 13, 14, 14, 15, 12, /* left */
        16, 17, 18, 18, 19, 16, /* top */
        20, 21, 22, 22, 23, 20, /* bottom */
    };

    const size_t vertexDataSize = sizeof( verts );
    const size_t indexDataSize = sizeof( indices );
    const size_t cameraDataSize = sizeof( CameraUniforms );

    m_pVertexBuffer = m_pDevice->newBuffer( vertexDataSize, MTL::ResourceStorageModeManaged );
    m_pIndexBuffer = m_pDevice->newBuffer( indexDataSize, MTL::ResourceStorageModeManaged );
    m_pCameraBuffer = m_pDevice->newBuffer( cameraDataSize, MTL::ResourceStorageModeManaged );

    memcpy( m_pVertexBuffer->contents(), verts, vertexDataSize );
    memcpy( m_pIndexBuffer->contents(), indices, indexDataSize );

    m_pVertexBuffer->didModifyRange( NS::Range::Make( 0, m_pVertexBuffer->length() ) );
    m_pIndexBuffer->didModifyRange( NS::Range::Make( 0, m_pIndexBuffer->length() ) );
    m_pCameraBuffer->didModifyRange( NS::Range::Make( 0, m_pCameraBuffer->length() ) );
}

void WorldRenderer::loadAssets()
{
    /// Load assets into metal objects

    m_pColorMap = CreateTextureWithDevice(m_pDevice, CHASTR("perlinMap.png"), true, false, MTL::StorageModeManaged);
}


void WorldRenderer::updateState()
{
    /// Update any game state before encoding renderint commands to our drawable
    
}

void WorldRenderer::drawInView( void* view )
{
    /// Per frame updates here
    
    MTK::View* pView = static_cast<MTK::View*>(view);
    MTL::CommandBuffer* pCmd = m_pCommandQueue->commandBuffer();
    pCmd->setLabel(CHASTR("Frame CB"));
    __block dispatch_semaphore_t block_sema = m_inFlightSemaphore;
    
    dispatch_semaphore_wait( m_inFlightSemaphore, DISPATCH_TIME_FOREVER );
    pCmd->addCompletedHandler( ^void( MTL::CommandBuffer* pCmd ){
        dispatch_semaphore_signal( block_sema );
    });
    this->updateState();


    /// Delay getting the currentRenderPassDescriptor until we absolutely need it to avoid
    ///   holding onto the drawable and blocking the display pipeline any longer than necessar
    /// Final pass rendering code here

    CameraUniforms tmp = m_pCamera->getUniforms();
    memcpy( m_pCameraBuffer->contents(), &tmp, sizeof( CameraUniforms ) );
    MTL::RenderPassDescriptor* pRpd = pView->currentRenderPassDescriptor();
    MTL::RenderCommandEncoder* pEnc = pCmd->renderCommandEncoder(pRpd);
    pEnc->setLabel(CHASTR("MypEnc"));
    pEnc->pushDebugGroup(CHASTR("DrawBox"));
    pEnc->setFrontFacingWinding(MTL::WindingCounterClockwise);
    pEnc->setCullMode(MTL::CullModeBack);
    pEnc->setRenderPipelineState(m_pPipelineState);
    pEnc->setDepthStencilState(m_pDepthState);

    pEnc->setVertexBuffer( m_pVertexBuffer, /* offset */ 0, /* index */ 0 );
    pEnc->setVertexBuffer( m_pCameraBuffer, /* offset */ 0, /* index */ 1 );

    pEnc->setFragmentTexture(m_pColorMap, 0);
    
    pEnc->drawIndexedPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle,
                                6 * 6,
                                MTL::IndexType::IndexTypeUInt16,
                                m_pIndexBuffer,
                                0);
    
    pEnc->popDebugGroup();
    pEnc->endEncoding();
    pCmd->presentDrawable(pView->currentDrawable());
    
    pCmd->commit();
}

void WorldRenderer::drawableSizeWillChange(const MTL::Size& size, MTL::StorageMode GBufferStorageMode)
{
    // Update the camera
    m_pCamera->setAspectRatio( (float)size.width / (float)size.height );
};

}
