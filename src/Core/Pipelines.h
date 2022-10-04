//
//  Pipelines.hpp
//  Island
//
//  Created by Chaeyoung Lim on 2022/09/24.
//

#ifndef Pipelines_h
#define Pipelines_h

#include <Metal/Metal.hpp>

static constexpr MTL::PixelFormat g_albedo_specular_GBufferFormat    = MTL::PixelFormatRGBA8Unorm_sRGB;
static constexpr MTL::PixelFormat g_normal_shadow_GBufferFormat      = MTL::PixelFormatRGBA8Snorm;
static constexpr MTL::PixelFormat g_depth_GBufferFormat              = MTL::PixelFormatR32Float;
static constexpr MTL::PixelFormat g_depthStencilPixelFormat          = MTL::PixelFormatDepth32Float_Stencil8;
static constexpr MTL::PixelFormat g_colorPixelFormat                 = MTL::PixelFormatBGRA8Unorm_sRGB;
static constexpr MTL::PixelFormat g_shadowMapPixelFormat             = MTL::PixelFormatDepth32Float;
static constexpr MTL::PixelFormat gBuffer1Format                     = MTL::PixelFormatRGBA8Unorm;

static constexpr uint  sampleCount                        = 1;
static constexpr uint  _iabBufferIndex_PplTerrainMainView = 1;



namespace CHA {
MTL::RenderPipelineState* createTessRenderPSO(MTL::Device* pDevice,
                                              MTL::Library* pShaderLibrary,
                                              MTL::VertexDescriptor* pVertexDescriptor,
                                              const MTL::FunctionConstantValues* pconstValues,
                                              const NS::String* pVertexFunc,
                                              const NS::String* pFragmentFunc,
                                              const NS::String* pLabel);

MTL::RenderPipelineState* createVegRenderPSO(MTL::Device* pDevice,
                                             MTL::Library* pShaderLibrary,
                                             MTL::VertexDescriptor* pVertexDescriptor,
                                             const MTL::FunctionConstantValues* pconstValues,
                                             const NS::String* pVertexFunc,
                                             const NS::String* pFragmentFunc,
                                             const NS::String* pLabel);

MTL::RenderPipelineState* createRenderPSO(MTL::Device* pDevice,
                                          MTL::Library* pShaderLibrary,
                                          MTL::VertexDescriptor* pDefaultVertexDescriptor,
                                          const NS::String* pVertexFunc,
                                          const NS::String* pFragmentFunc,
                                          const NS::String* pLabel);
    
MTL::RenderPipelineState* createRenderPSOwithBlend(MTL::Device* pDevice,
                                                   MTL::Library* pShaderLibrary,
                                                   MTL::VertexDescriptor* pDefaultVertexDescriptor,
                                                   const NS::String* pVertexFunc,
                                                   const NS::String* pFragmentFunc,
                                                   const NS::String* pLabel);

MTL::RenderPipelineState* creatShadowPSO(MTL::Device* pDevice,
                                         MTL::Library* pShaderLibrary,
                                         MTL::VertexDescriptor* pDefaultVertexDescriptor,
                                         const MTL::FunctionConstantValues* pconstValues,
                                         const NS::String* pVertexFunc,
                                         const NS::String* pLabel);

MTL::ComputePipelineState* creatComputePSO(MTL::Device* pDevice,
                                           MTL::Library* pComputeLibrary,
                                           const NS::String* pComputeFunc,
                                           bool threadGroupSizeIsHwMultiple = true);

MTL::RenderPipelineState* createLightingRenderPSO(MTL::Device* pDevice,
                                                  MTL::Library* pShaderLibrary,
                                                  MTL::VertexDescriptor* pDefaultVertexDescriptor,
                                                  const NS::String* pVertexFunc,
                                                  const NS::String* pFragmentFunc,
                                                  const NS::String* pLabel);

}

#endif /* Pipelines_hpp */
