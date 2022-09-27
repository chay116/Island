//
//  DepthStencil.cpp
//  Island
//
//  Created by Chaeyoung Lim on 2022/09/25.
//

#include "DepthStencil.h"
#include "Assert.h"

namespace CHA {

MTL::DepthStencilState* buildBasicDepthStencilState(MTL::Device* pDevice) {
    MTL::DepthStencilDescriptor* pDepthStencilDesc = MTL::DepthStencilDescriptor::alloc()->init();
    pDepthStencilDesc->setLabel( CHASTR( "Basic DepthStencil" ) );
    pDepthStencilDesc->setDepthCompareFunction( MTL::CompareFunctionLess );
    pDepthStencilDesc->setDepthWriteEnabled( true );

    MTL::DepthStencilState* m_pTessDepthStencilState = pDevice->newDepthStencilState( pDepthStencilDesc );
    pDepthStencilDesc->release();
    return m_pTessDepthStencilState;
}

MTL::DepthStencilState* buildGBufferDepthStencilState(MTL::Device* pDevice) {
    MTL::StencilDescriptor* pStencilStateDesc = MTL::StencilDescriptor::alloc()->init();
#if LIGHT_STENCIL_CULLING
    pStencilStateDesc->setStencilCompareFunction( MTL::CompareFunctionAlways );
    pStencilStateDesc->setStencilFailureOperation( MTL::StencilOperationKeep );
    pStencilStateDesc->setDepthFailureOperation( MTL::StencilOperationKeep );
    pStencilStateDesc->setDepthStencilPassOperation( MTL::StencilOperationReplace );
    pStencilStateDesc->setReadMask( 0x0 );
    pStencilStateDesc->setWriteMask( 0xFF );
#endif
    MTL::DepthStencilDescriptor* pDepthStencilDesc = MTL::DepthStencilDescriptor::alloc()->init();
    pDepthStencilDesc->setLabel( CHASTR( "G-buffer Creation" ) );
    pDepthStencilDesc->setDepthCompareFunction( MTL::CompareFunctionLess );
    pDepthStencilDesc->setDepthWriteEnabled( true );
    pDepthStencilDesc->setFrontFaceStencil( pStencilStateDesc );
    pDepthStencilDesc->setBackFaceStencil( pStencilStateDesc );

    MTL::DepthStencilState* m_pGBufferDepthStencilState = pDevice->newDepthStencilState( pDepthStencilDesc );
    pDepthStencilDesc->release();
    pStencilStateDesc->release();
    return m_pGBufferDepthStencilState;
}

MTL::DepthStencilState* buildDirectionLightDepthStencilState(MTL::Device* pDevice) {
    MTL::StencilDescriptor* pStencilStateDesc = MTL::StencilDescriptor::alloc()->init();
#if LIGHT_STENCIL_CULLING
    // Stencil state setup so direction lighting fragment shader only executed on pixels
    // drawn in GBuffer stage (i.e. mask out the background/sky)
    pStencilStateDesc->setStencilCompareFunction( MTL::CompareFunctionEqual );
    pStencilStateDesc->setStencilFailureOperation( MTL::StencilOperationKeep );
    pStencilStateDesc->setDepthFailureOperation( MTL::StencilOperationKeep );
    pStencilStateDesc->setDepthStencilPassOperation( MTL::StencilOperationKeep );
    pStencilStateDesc->setReadMask( 0xFF );
    pStencilStateDesc->setWriteMask( 0x0 );
#endif
    MTL::DepthStencilDescriptor* pDepthStencilDesc = MTL::DepthStencilDescriptor::alloc()->init();
    pDepthStencilDesc->setLabel( CHASTR( "Deferred Directional Lighting" ) );
    pDepthStencilDesc->setDepthWriteEnabled( false );
    pDepthStencilDesc->setDepthCompareFunction( MTL::CompareFunctionAlways );
    pDepthStencilDesc->setFrontFaceStencil( pStencilStateDesc );
    pDepthStencilDesc->setBackFaceStencil( pStencilStateDesc );

    MTL::DepthStencilState* m_pDirectionLightDepthStencilState = pDevice->newDepthStencilState( pDepthStencilDesc );
    
    pDepthStencilDesc->release();
    pStencilStateDesc->release();

    return m_pDirectionLightDepthStencilState;
}

MTL::DepthStencilState* buildPointLightMaskDepthStencilState(MTL::Device* pDevice) {
    MTL::StencilDescriptor* pStencilStateDesc = MTL::StencilDescriptor::alloc()->init();
#if LIGHT_STENCIL_CULLING
    pStencilStateDesc->setStencilCompareFunction( MTL::CompareFunctionLess );
    pStencilStateDesc->setStencilFailureOperation( MTL::StencilOperationKeep );
    pStencilStateDesc->setDepthFailureOperation( MTL::StencilOperationKeep );
    pStencilStateDesc->setDepthStencilPassOperation( MTL::StencilOperationKeep );
    pStencilStateDesc->setReadMask( 0xFF );
    pStencilStateDesc->setWriteMask( 0x0 );
#endif // END NOT LIGHT_STENCIL_CULLING
    MTL::DepthStencilDescriptor* pDepthStencilDesc = MTL::DepthStencilDescriptor::alloc()->init();
    pDepthStencilDesc->setDepthWriteEnabled( false );
    pDepthStencilDesc->setDepthCompareFunction( MTL::CompareFunctionLessEqual );
    pDepthStencilDesc->setFrontFaceStencil( pStencilStateDesc );
    pDepthStencilDesc->setBackFaceStencil( pStencilStateDesc );
    pDepthStencilDesc->setLabel( CHASTR( "Point Light" ) );
    
    MTL::DepthStencilState* m_pLightDepthStencilState = pDevice->newDepthStencilState( pDepthStencilDesc );
    
    pDepthStencilDesc->release();
    pStencilStateDesc->release();

    return m_pLightDepthStencilState;
}

MTL::DepthStencilState* buildPointLightDepthStencilState(MTL::Device* pDevice) {
    MTL::StencilDescriptor* pStencilStateDesc = MTL::StencilDescriptor::alloc()->init();
    pStencilStateDesc->setStencilCompareFunction( MTL::CompareFunctionAlways );
    pStencilStateDesc->setStencilFailureOperation( MTL::StencilOperationKeep );
    pStencilStateDesc->setDepthFailureOperation( MTL::StencilOperationIncrementClamp );
    pStencilStateDesc->setDepthStencilPassOperation( MTL::StencilOperationKeep );
    pStencilStateDesc->setReadMask( 0x0 );
    pStencilStateDesc->setWriteMask( 0xFF );
    
    MTL::DepthStencilDescriptor* pDepthStencilDesc = MTL::DepthStencilDescriptor::alloc()->init();
    pDepthStencilDesc->setLabel( CHASTR( "Point Light Mask" ) );
    pDepthStencilDesc->setDepthWriteEnabled( false );
    pDepthStencilDesc->setDepthCompareFunction( MTL::CompareFunctionLessEqual );
    pDepthStencilDesc->setFrontFaceStencil( pStencilStateDesc );
    pDepthStencilDesc->setBackFaceStencil( pStencilStateDesc );
    
    MTL::DepthStencilState* m_pLightMaskDepthStencilState = pDevice->newDepthStencilState( pDepthStencilDesc );
    
    pDepthStencilDesc->release();
    pStencilStateDesc->release();

    return m_pLightMaskDepthStencilState;
}

//MTL::Texture* buildShadowMapTexture(MTL::Device* pDevice) {
//    MTL::TextureDescriptor* pShadowTextureDesc = MTL::TextureDescriptor::alloc()->init();
//
//    pShadowTextureDesc->setPixelFormat( BufferFormats::shadowDepthFormat );
//    pShadowTextureDesc->setWidth( 1024 );
//    pShadowTextureDesc->setHeight( 1024 );
//    pShadowTextureDesc->setMipmapLevelCount( 1 );
//    pShadowTextureDesc->setResourceOptions( MTL::ResourceStorageModePrivate );
//    pShadowTextureDesc->setTextureType(MTL::TextureType2DArray);
//    pShadowTextureDesc->setArrayLength(NUM_CASCADES);
//    pShadowTextureDesc->setUsage( MTL::TextureUsageRenderTarget | MTL::TextureUsageShaderRead );
//
//    MTL::Texture *m_pShadowMap = pDevice->newTexture( pShadowTextureDesc );
//    m_pShadowMap->setLabel( CHASTR( "Shadow Map" ) );
//    
//    
//    MTL::TextureDescriptor::texture2DDescriptor(BufferFormats::shadowDepthFormat, shadowWidth, shadowWidth, false);
//    
//    pShadowTextureDesc->release();
//    return m_pShadowMap;
//}
}
