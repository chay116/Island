//
//  Pipelines.cpp
//  Island
//
//  Created by Chaeyoung Lim on 2022/09/25.
//

#include "Pipelines.h"
#include "Assert.h"

namespace CHA {
MTL::RenderPipelineState* createTessRenderPSO(MTL::Device* pDevice,
                                              MTL::Library* pShaderLibrary,
                                              MTL::VertexDescriptor* pVertexDescriptor,
                                              const MTL::FunctionConstantValues* pconstValues,
                                              const NS::String* pVertexFunc,
                                              const NS::String* pFragmentFunc,
                                              const NS::String* pLabel)
{
    NS::Error* pError = nullptr;
    MTL::Function* pVertexFunction = nullptr;
    MTL::Function* pFragmentFunction = nullptr;
    
    if (pVertexFunc) {
        if (pconstValues)
            pVertexFunction = pShaderLibrary->newFunction( pVertexFunc, pconstValues, &pError );
        else
            pVertexFunction = pShaderLibrary->newFunction( pVertexFunc );
        CHA_ASSERT( (bool)pVertexFunction,
                   "Failed to load Vertex shader: ",
                   pVertexFunc );
    }
    if (pFragmentFunc) {
        if (pconstValues)
            pFragmentFunction = pShaderLibrary->newFunction( pFragmentFunc, pconstValues, &pError );
        else
            pFragmentFunction = pShaderLibrary->newFunction( pFragmentFunc );
        CHA_ASSERT( (bool)pFragmentFunction,
                   "Failed to load Fragment shader: ",
                   pFragmentFunction );
    }
    
    MTL::RenderPipelineDescriptor* pRenderPipeDesc = MTL::RenderPipelineDescriptor::alloc()->init();
    pRenderPipeDesc->setSampleCount( 1 );
    pRenderPipeDesc->setLabel( pLabel );
    pRenderPipeDesc->setVertexDescriptor( pVertexDescriptor );
    pRenderPipeDesc->setVertexFunction( pVertexFunction );
    pRenderPipeDesc->setFragmentFunction( pFragmentFunction );
    
    pRenderPipeDesc->colorAttachments()->object(0)->setPixelFormat( MTL::PixelFormatBGRA8Unorm_sRGB );
    pRenderPipeDesc->colorAttachments()->object(1)->setPixelFormat( MTL::PixelFormatRGBA8Unorm );
    pRenderPipeDesc->setDepthAttachmentPixelFormat( MTL::PixelFormatDepth32Float_Stencil8 );
    
    pRenderPipeDesc->setTessellationFactorFormat( MTL::TessellationFactorFormatHalf );
    pRenderPipeDesc->setTessellationPartitionMode( MTL::TessellationPartitionModeFractionalOdd );
    pRenderPipeDesc->setTessellationFactorStepFunction( MTL::TessellationFactorStepFunctionPerPatch );
    pRenderPipeDesc->setTessellationControlPointIndexType( MTL::TessellationControlPointIndexTypeNone );
    pRenderPipeDesc->setMaxTessellationFactor( 16 );
    
    
    // A good optimization that improves driver performance is to state when a pipeline will
    // modify an argument buffer or not. Doing this saves cache invalidations and memory fetches.
    // Because we know ahad of time that the terrain parameters Argument Buffer will never be modified, we
    // mark the slot immutable that it will be bound to.
    
    // Create the regular pipeline. This is used later on
    pRenderPipeDesc->fragmentBuffers()->object(1)->setMutability(MTL::MutabilityImmutable);
    
    MTL::RenderPipelineState* pPipelineState = pDevice->newRenderPipelineState(pRenderPipeDesc, &pError);
    CHA_ASSERT_NULL_ERROR( pError, "Failed to create Render Pipeline State: ", pLabel);
    pRenderPipeDesc->release();
    if (pVertexFunction) pVertexFunction->release();
    if (pFragmentFunction) pFragmentFunction->release();
    return pPipelineState;
}


MTL::RenderPipelineState* createVegRenderPSO(MTL::Device* pDevice,
                                             MTL::Library* pShaderLibrary,
                                             MTL::VertexDescriptor* pVertexDescriptor,
                                             const MTL::FunctionConstantValues* pconstValues,
                                             const NS::String* pVertexFunc,
                                             const NS::String* pFragmentFunc,
                                             const NS::String* pLabel)
{
    NS::Error* pError = nullptr;
    MTL::Function* pVertexFunction = nullptr;
    MTL::Function* pFragmentFunction = nullptr;
    
    if (pVertexFunc) {
        if (pconstValues)
            pVertexFunction = pShaderLibrary->newFunction( pVertexFunc, pconstValues, &pError );
        else
            pVertexFunction = pShaderLibrary->newFunction( pVertexFunc );
        CHA_ASSERT( (bool)pVertexFunction,
                   "Failed to load Vertex shader: ",
                   pVertexFunc );
    }
    if (pFragmentFunc) {
        if (pconstValues)
            pFragmentFunction = pShaderLibrary->newFunction( pFragmentFunc, pconstValues, &pError );
        else
            pFragmentFunction = pShaderLibrary->newFunction( pFragmentFunc );
        CHA_ASSERT( (bool)pFragmentFunction,
                   "Failed to load Fragment shader: ",
                   pFragmentFunction );
    }
    
    MTL::RenderPipelineDescriptor* pRenderPipeDesc = MTL::RenderPipelineDescriptor::alloc()->init();
    pRenderPipeDesc->setSampleCount( 1 );
    pRenderPipeDesc->setLabel( pLabel );
    pRenderPipeDesc->setVertexDescriptor( pVertexDescriptor );
    pRenderPipeDesc->setVertexFunction( pVertexFunction );
    pRenderPipeDesc->setFragmentFunction( pFragmentFunction );
    
    pRenderPipeDesc->colorAttachments()->object(0)->setPixelFormat( MTL::PixelFormatBGRA8Unorm_sRGB );
    pRenderPipeDesc->colorAttachments()->object(1)->setPixelFormat( MTL::PixelFormatRGBA8Unorm );
    pRenderPipeDesc->setDepthAttachmentPixelFormat( MTL::PixelFormatDepth32Float_Stencil8 );
    
    MTL::RenderPipelineState* pPipelineState = pDevice->newRenderPipelineState(pRenderPipeDesc, &pError);
    CHA_ASSERT_NULL_ERROR( pError, "Failed to create Render Pipeline State: ", pLabel);
    pRenderPipeDesc->release();
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

    MTL::RenderPipelineDescriptor* pRenderPipeDesc = MTL::RenderPipelineDescriptor::alloc()->init();
    pRenderPipeDesc->setLabel( pLabel );
    pRenderPipeDesc->setVertexDescriptor( pDefaultVertexDescriptor );
    pRenderPipeDesc->setVertexFunction( pVertexFunction );
    pRenderPipeDesc->setFragmentFunction( pFragmentFunction );
    pRenderPipeDesc->setSampleCount(1);
    pRenderPipeDesc->colorAttachments()->object(0)->setPixelFormat( g_colorPixelFormat );
    
    pRenderPipeDesc->setDepthAttachmentPixelFormat( g_depthStencilPixelFormat );
    pRenderPipeDesc->setStencilAttachmentPixelFormat( g_depthStencilPixelFormat );
    
    MTL::RenderPipelineState* pPipelineState = pDevice->newRenderPipelineState(pRenderPipeDesc, &pError);
    CHA_ASSERT_NULL_ERROR( pError, "Failed to create Render Pipeline State: ", pLabel);
    pRenderPipeDesc->release();
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
    
    MTL::RenderPipelineDescriptor* pRenderPipeDesc = MTL::RenderPipelineDescriptor::alloc()->init();
    pRenderPipeDesc->setLabel( pLabel );
    pRenderPipeDesc->setVertexDescriptor( pDefaultVertexDescriptor );
    pRenderPipeDesc->setVertexFunction( pVertexFunction );
    pRenderPipeDesc->setFragmentFunction( pFragmentFunction );
    
    pRenderPipeDesc->setDepthAttachmentPixelFormat( g_depthStencilPixelFormat );
    pRenderPipeDesc->setStencilAttachmentPixelFormat( g_depthStencilPixelFormat);
    
    pRenderPipeDesc->colorAttachments()->object(0)->setBlendingEnabled( true );
    pRenderPipeDesc->colorAttachments()->object(0)->setRgbBlendOperation( MTL::BlendOperationAdd );
    pRenderPipeDesc->colorAttachments()->object(0)->setAlphaBlendOperation( MTL::BlendOperationAdd );
    pRenderPipeDesc->colorAttachments()->object(0)->setSourceRGBBlendFactor( MTL::BlendFactorSourceAlpha );
    pRenderPipeDesc->colorAttachments()->object(0)->setSourceAlphaBlendFactor ( MTL::BlendFactorSourceAlpha );
    pRenderPipeDesc->colorAttachments()->object(0)->setDestinationRGBBlendFactor( MTL::BlendFactorOne );
    pRenderPipeDesc->colorAttachments()->object(0)->setDestinationAlphaBlendFactor( MTL::BlendFactorOne );
    
    MTL::RenderPipelineState* pPipelineState = pDevice->newRenderPipelineState(pRenderPipeDesc, &pError);
    CHA_ASSERT_NULL_ERROR( pError, "Failed to create Render Pipeline State: ", pLabel);
    pRenderPipeDesc->release();
    if (pVertexFunction) pVertexFunction->release();
    if (pFragmentFunction) pFragmentFunction->release();
    
    return pPipelineState;
}


MTL::RenderPipelineState* createLightingRenderPSO(MTL::Device* pDevice,
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

    MTL::RenderPipelineDescriptor* pRenderPipeDesc = MTL::RenderPipelineDescriptor::alloc()->init();
    pRenderPipeDesc->setLabel( pLabel );
    pRenderPipeDesc->setVertexDescriptor( pDefaultVertexDescriptor );
    pRenderPipeDesc->setVertexFunction( pVertexFunction );
    pRenderPipeDesc->setFragmentFunction( pFragmentFunction );
    pRenderPipeDesc->setSampleCount(1);
    pRenderPipeDesc->colorAttachments()->object(0)->setPixelFormat( MTL::PixelFormatBGRA8Unorm_sRGB );
    
    MTL::RenderPipelineState* pPipelineState = pDevice->newRenderPipelineState(pRenderPipeDesc, &pError);
    CHA_ASSERT_NULL_ERROR( pError, "Failed to create Render Pipeline State: ", pLabel);
    pRenderPipeDesc->release();
    if (pVertexFunction) pVertexFunction->release();
    if (pFragmentFunction) pFragmentFunction->release();
    
    return pPipelineState;
}


MTL::RenderPipelineState* creatShadowPSO(MTL::Device* pDevice,
                                         MTL::Library* pShaderLibrary,
                                         MTL::VertexDescriptor* pDefaultVertexDescriptor,
                                         const MTL::FunctionConstantValues* pconstValues,
                                         const NS::String* pVertexFunc,
                                         const NS::String* pLabel)
{
    NS::Error* pError = nullptr;
    MTL::Function* pVertexFunction = nullptr;
    
    if (pVertexFunc) {
        if (pconstValues)
            pVertexFunction = pShaderLibrary->newFunction( pVertexFunc, pconstValues, &pError );
        else
            pVertexFunction = pShaderLibrary->newFunction( pVertexFunc );
        CHA_ASSERT( (bool)pVertexFunction,
                   "Failed to load Vertex shader: ",
                   pVertexFunc );
    }
    MTL::RenderPipelineDescriptor* pRenderPipeDesc = MTL::RenderPipelineDescriptor::alloc()->init();
    pRenderPipeDesc->setLabel( pLabel );
    pRenderPipeDesc->setVertexDescriptor( pDefaultVertexDescriptor );
    pRenderPipeDesc->setVertexFunction( pVertexFunction );
    pRenderPipeDesc->setFragmentFunction( nullptr );
    
    pRenderPipeDesc->colorAttachments()->object(0)->setPixelFormat( MTL::PixelFormatInvalid );
    pRenderPipeDesc->colorAttachments()->object(1)->setPixelFormat( MTL::PixelFormatInvalid );
    pRenderPipeDesc->setDepthAttachmentPixelFormat( g_shadowMapPixelFormat );
    
    MTL::RenderPipelineState* pPipelineState = pDevice->newRenderPipelineState(pRenderPipeDesc, &pError);
    CHA_ASSERT_NULL_ERROR( pError, "Failed to create Render Pipeline State: ", pLabel);
    pRenderPipeDesc->release();
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



}
