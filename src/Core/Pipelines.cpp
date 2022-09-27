//
//  Pipelines.cpp
//  Island
//
//  Created by Chaeyoung Lim on 2022/09/25.
//

#include "Pipelines.h"
#include "Assert.h"

namespace CHA {
MTL::RenderPipelineState* createTessRenderPSO(MTL::Device* pDevice, MTL::Library* pShaderLibrary, MTL::VertexDescriptor* pVertexDescriptor, const NS::String* pVertexFunc, const NS::String* pFragmentFunc, const NS::String* pLabel) {
    
    NS::Error* pError = nullptr;
    MTL::Function* pVertexFunction = nullptr;
    MTL::Function* pFragmentFunction = nullptr;
    
    if (pVertexFunc) {
        pVertexFunction = pShaderLibrary->newFunction( pVertexFunc );
        CHA_ASSERT( pVertexFunction, "Failed to load Vertex shader: ", pVertexFunc );
    }
    if (pFragmentFunc) {
        pFragmentFunction = pShaderLibrary->newFunction( pFragmentFunc );
        CHA_ASSERT( pFragmentFunction, "Failed to load Fragment shader: ", pFragmentFunc );
    }
    
    MTL::RenderPipelineDescriptor* pRenderPipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    pRenderPipelineDescriptor->setLabel( pLabel );
    pRenderPipelineDescriptor->setVertexDescriptor( pVertexDescriptor );
    pRenderPipelineDescriptor->setVertexFunction( pVertexFunction );
    pRenderPipelineDescriptor->setFragmentFunction( pFragmentFunction );
    pRenderPipelineDescriptor->colorAttachments()->object(0)->setPixelFormat( g_colorPixelFormat );
    pRenderPipelineDescriptor->setDepthAttachmentPixelFormat( g_depthStencilPixelFormat );
    
    pRenderPipelineDescriptor->setVertexDescriptor(pVertexDescriptor);
    pRenderPipelineDescriptor->setTessellationFactorStepFunction(MTL::TessellationFactorStepFunctionPerPatch);
    pRenderPipelineDescriptor->setMaxTessellationFactor(64);
    pRenderPipelineDescriptor->setTessellationPartitionMode(MTL::TessellationPartitionModePow2);
    
    MTL::RenderPipelineState* pPipelineState = pDevice->newRenderPipelineState(pRenderPipelineDescriptor, &pError);
    CHA_ASSERT_NULL_ERROR( pError, "Failed to create Render Pipeline State: ", pLabel);
    pRenderPipelineDescriptor->release();
    if (pVertexFunction) pVertexFunction->release();
    if (pFragmentFunction) pFragmentFunction->release();
    
    return pPipelineState;
}

MTL::RenderPipelineState* createRenderPSO(MTL::Device* pDevice,
                                          MTL::Library* pShaderLibrary,
                                          MTL::VertexDescriptor* pDefaultVertexDescriptor,
                                          const NS::String* pVertexFunc,
                                          const NS::String* pFragmentFunc,
                                          const NS::String* pLabel)
{
    
    NS::Error* pError = nullptr;
    MTL::Function* pVertexFunction = nullptr;
    MTL::Function* pFragmentFunction = nullptr;

    if (pVertexFunc) {
        pVertexFunction = pShaderLibrary->newFunction( pVertexFunc );
        CHA_ASSERT( pVertexFunction, "Failed to load Vertex shader: ", pVertexFunc );
    }
    if (pFragmentFunc) {
        pFragmentFunction = pShaderLibrary->newFunction( pFragmentFunc );
        CHA_ASSERT( pFragmentFunction, "Failed to load Fragment shader: ", pFragmentFunc );
    }

    MTL::RenderPipelineDescriptor* pRenderPipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    pRenderPipelineDescriptor->setLabel( pLabel );
    pRenderPipelineDescriptor->setVertexDescriptor( pDefaultVertexDescriptor );
    pRenderPipelineDescriptor->setVertexFunction( pVertexFunction );
    pRenderPipelineDescriptor->setFragmentFunction( pFragmentFunction );
    pRenderPipelineDescriptor->setSampleCount(1);
    pRenderPipelineDescriptor->colorAttachments()->object(0)->setPixelFormat( g_colorPixelFormat );
    
//    pRenderPipelineDescriptor->colorAttachments()->object(RenderTargetLighting)->setPixelFormat( g_colorPixelFormat );
//    pRenderPipelineDescriptor->colorAttachments()->object(RenderTargetAlbedo)->setPixelFormat( g_albedo_specular_GBufferFormat );
//    pRenderPipelineDescriptor->colorAttachments()->object(RenderTargetNormal)->setPixelFormat( g_normal_shadow_GBufferFormat );
//    pRenderPipelineDescriptor->colorAttachments()->object(RenderTargetDepth)->setPixelFormat( g_depth_GBufferFormat );
    pRenderPipelineDescriptor->setDepthAttachmentPixelFormat( g_depthStencilPixelFormat );
    pRenderPipelineDescriptor->setStencilAttachmentPixelFormat( g_depthStencilPixelFormat );
    
    MTL::RenderPipelineState* pPipelineState = pDevice->newRenderPipelineState(pRenderPipelineDescriptor, &pError);
    CHA_ASSERT_NULL_ERROR( pError, "Failed to create Render Pipeline State: ", pLabel);
    pRenderPipelineDescriptor->release();
    if (pVertexFunction) pVertexFunction->release();
    if (pFragmentFunction) pFragmentFunction->release();
    
    return pPipelineState;
}

MTL::RenderPipelineState* createRenderPSOwithBlend(MTL::Device* pDevice, MTL::Library* pShaderLibrary, MTL::VertexDescriptor* pDefaultVertexDescriptor, const NS::String* pVertexFunc, const NS::String* pFragmentFunc, const NS::String* pLabel) {
    
    NS::Error* pError = nullptr;
    MTL::Function* pVertexFunction = nullptr;
    MTL::Function* pFragmentFunction = nullptr;
    
    if (pVertexFunc) {
        pVertexFunction = pShaderLibrary->newFunction( pVertexFunc );
        CHA_ASSERT( pVertexFunction, "Failed to load Vertex shader: ", pVertexFunc );
    }
    if (pFragmentFunc) {
        pFragmentFunction = pShaderLibrary->newFunction( pFragmentFunc );
        CHA_ASSERT( pFragmentFunction, "Failed to load Fragment shader: ", pFragmentFunc );
    }
    
    MTL::RenderPipelineDescriptor* pRenderPipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    pRenderPipelineDescriptor->setLabel( pLabel );
    pRenderPipelineDescriptor->setVertexDescriptor( pDefaultVertexDescriptor );
    pRenderPipelineDescriptor->setVertexFunction( pVertexFunction );
    pRenderPipelineDescriptor->setFragmentFunction( pFragmentFunction );
    
//    pRenderPipelineDescriptor->colorAttachments()->object(RenderTargetLighting)->setPixelFormat( g_colorPixelFormat );
//    pRenderPipelineDescriptor->colorAttachments()->object(RenderTargetAlbedo)->setPixelFormat( g_albedo_specular_GBufferFormat );
//    pRenderPipelineDescriptor->colorAttachments()->object(RenderTargetNormal)->setPixelFormat( g_normal_shadow_GBufferFormat );
//    pRenderPipelineDescriptor->colorAttachments()->object(RenderTargetDepth)->setPixelFormat( g_depth_GBufferFormat );
    pRenderPipelineDescriptor->setDepthAttachmentPixelFormat( g_depthStencilPixelFormat );
    pRenderPipelineDescriptor->setStencilAttachmentPixelFormat( g_depthStencilPixelFormat);
    
    // Because iOS renderer can perform GBuffer pass in final pass, any pipeline rendering in
    // the final pass must take the GBuffers into account
    pRenderPipelineDescriptor->colorAttachments()->object(0)->setBlendingEnabled( true );
    pRenderPipelineDescriptor->colorAttachments()->object(0)->setRgbBlendOperation( MTL::BlendOperationAdd );
    pRenderPipelineDescriptor->colorAttachments()->object(0)->setAlphaBlendOperation( MTL::BlendOperationAdd );
    pRenderPipelineDescriptor->colorAttachments()->object(0)->setSourceRGBBlendFactor( MTL::BlendFactorSourceAlpha );
    pRenderPipelineDescriptor->colorAttachments()->object(0)->setSourceAlphaBlendFactor ( MTL::BlendFactorSourceAlpha );
    pRenderPipelineDescriptor->colorAttachments()->object(0)->setDestinationRGBBlendFactor( MTL::BlendFactorOne );
    pRenderPipelineDescriptor->colorAttachments()->object(0)->setDestinationAlphaBlendFactor( MTL::BlendFactorOne );
    
    MTL::RenderPipelineState* pPipelineState = pDevice->newRenderPipelineState(pRenderPipelineDescriptor, &pError);
    CHA_ASSERT_NULL_ERROR( pError, "Failed to create Render Pipeline State: ", pLabel);
    pRenderPipelineDescriptor->release();
    if (pVertexFunction) pVertexFunction->release();
    if (pFragmentFunction) pFragmentFunction->release();
    
    return pPipelineState;
}

MTL::RenderPipelineState* creatShadowPSO(MTL::Device* pDevice, MTL::Library* pShaderLibrary, MTL::VertexDescriptor* pDefaultVertexDescriptor, const NS::String* pVertexFunc, const NS::String* pLabel) {
    
    NS::Error* pError = nullptr;
    MTL::Function* pVertexFunction = nullptr;
    
    if (pVertexFunc) {
        pVertexFunction = pShaderLibrary->newFunction( pVertexFunc );
        CHA_ASSERT( pVertexFunction, "Failed to load Vertex shader: ", pVertexFunc );
    }
    
    MTL::RenderPipelineDescriptor* pRenderPipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    pRenderPipelineDescriptor->setLabel( pLabel );
    pRenderPipelineDescriptor->setVertexDescriptor( pDefaultVertexDescriptor );
    pRenderPipelineDescriptor->setVertexFunction( pVertexFunction );
    pRenderPipelineDescriptor->setFragmentFunction( nullptr );
    pRenderPipelineDescriptor->setDepthAttachmentPixelFormat( MTL::PixelFormatDepth16Unorm );
    
    MTL::RenderPipelineState* pPipelineState = pDevice->newRenderPipelineState(pRenderPipelineDescriptor, &pError);
    CHA_ASSERT_NULL_ERROR( pError, "Failed to create Render Pipeline State: ", pLabel);
    pRenderPipelineDescriptor->release();
    if (pVertexFunction) pVertexFunction->release();
    
    return pPipelineState;
}

MTL::ComputePipelineState* creatComputePSO(MTL::Device* pDevice, MTL::Library* pComputeLibrary, const NS::String* pComputeFunc, bool threadGroupSizeIsHwMultiple) {

    NS::Error* pError = nullptr;
    MTL::Function* pComputeFunction = pComputeLibrary->newFunction( pComputeFunc );
    CHA_ASSERT( pComputeFunction, "Failed to load Compute Function: ", pComputeFunc );
    
    MTL::ComputePipelineDescriptor* pComputePipelineDescriptor = MTL::ComputePipelineDescriptor::alloc()->init();
    pComputePipelineDescriptor->setThreadGroupSizeIsMultipleOfThreadExecutionWidth(threadGroupSizeIsHwMultiple);
    pComputePipelineDescriptor->setComputeFunction(pComputeFunction);
    
    MTL::ComputePipelineState* pComputePipelineState = pDevice->newComputePipelineState(pComputePipelineDescriptor, MTL::PipelineOptionNone, nullptr, &pError);
    CHA_ASSERT_NULL_ERROR( pError, "Failed to create Compute Pipeline State");
    
    return pComputePipelineState;
}

MTL::RenderPipelineState* creatTessPSO(MTL::Device* pDevice, MTL::Library* pShaderLibrary, MTL::VertexDescriptor* pDefaultVertexDescriptor, const NS::String* pVertexFunc, const NS::String* pLabel) {

//    NS::Error* pError = nullptr;
//    MTL::Function* pVertexFunction = pShaderLibrary->newFunction( pVertexFunc );
//    CHA_ASSERT( pVertexFunction, "Failed to load Vertex shader: ", pVertexFunc );
//
//    MTL::RenderPipelineDescriptor* pRenderPipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
//    pRenderPipelineDescriptor->setLabel( pLabel );
//    pRenderPipelineDescriptor->setVertexDescriptor( pDefaultVertexDescriptor );
//    pRenderPipelineDescriptor->setVertexFunction( pVertexFunction );
//    pRenderPipelineDescriptor->setFragmentFunction( nullptr );
//    pRenderPipelineDescriptor->setDepthAttachmentPixelFormat( MTL::PixelFormatDepth16Unorm );
//
    MTL::RenderPipelineState* pPipelineState = nullptr;
//    CHA_ASSERT_NULL_ERROR( pError, "Failed to create Render Pipeline State: ", pLabel);
//    pRenderPipelineDescriptor->release();
//    pVertexFunction->release();

//    MTL::CommandBuffer* pCommandBuffer = _pCommandQueue->commandBuffer();
//    assert(pCommandBuffer);
//
//    MTL::ComputeCommandEncoder* pComputeEncoder = pCommandBuffer->computeCommandEncoder();
//
//    pComputeEncoder->setComputteePipelineState( _pComputePSO );
//    pComputeEncoder->setTexture( _pTexture, 0 );
//
//    MTL::Size gridSize = MTL::Size( kTextureWidth, kTextureHeight, 1 );
//
//    NS::UInteger threadGroupSize = _pComputePSO->maxTotalThreadsPerThreadgroup();
//    MTL::Size threadgroupSize( threadGroupSize, 1, 1 );
//
//    pComputeEncoder->dispatchThreads( gridSize, threadgroupSize );
//
//    pComputeEncoder->endEncoding();
//
//    pCommandBuffer->commit();
    return pPipelineState;
}


}
