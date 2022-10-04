//
//  Renderer.h
//  Island
//
//  Created by Chaeyoung Lim on 2022/09/24.
//

#ifndef WorldRenderer_h
#define WorldRenderer_h

#include "CameraController.h"
#include "TerrainRenderer.h"
#include <chrono>

namespace CHA {
static const size_t kAlignedUniformsSize = (sizeof(Uniforms) & ~0xFF) + 0x100;

class WorldRenderer {
public:
    static constexpr size_t kMaxBuffersInFlight = 3;
    WorldRenderer(MTL::Device* device, const MTL::Size& size);
    ~WorldRenderer();
    
    void setVertexDescriptor();
    void setDepthStencil(const MTL::Size& size);
    void setPipelines();
    void loadAssets();
    void buildBuffers();
    void buildCamera(const MTL::Size& size);
    void updateState();
    
    void drawableSizeWillChange(const MTL::Size& size, MTL::StorageMode GBufferStorageMode);
    void drawInView( void* view );
    
    MTL::Device*        device() const;
    MTL::PixelFormat    colorTargetPixelFormat() const;
    MTL::PixelFormat    depthStencilTargetPixelFormat() const;
    CameraController*   getCameraController() const;
    void                SwapIsDebugRendererEnabled();

    
private:
    void updateCPUUniforms();
    
    MTL::Device*                m_pDevice;
    MTL::CommandQueue*          m_pCommandQueue;
    MTL::Library*               m_pShaderLibrary;
    MTL::Buffer*                m_pDynamicUniformBuffer;
    MTL::DepthStencilState*     m_pDepthState;
    MTL::VertexDescriptor*      m_pDefaultVertexDescriptor;
    uint32_t                    m_uniformBufferOffset;
    uint8_t                     m_uniformBufferIndex;
    void*                       m_uniformBufferAddress;
    float                       m_rotation;
    
    MTL::Buffer*                m_pVertexBuffer;
    MTL::Buffer*                m_pIndexBuffer;
    MTL::Buffer*                m_pCameraBuffer;
    
    
    dispatch_semaphore_t                    m_inFlightSemaphore;
    uint8_t                                 m_onFrame;
    std::chrono::system_clock::time_point   m_startTime;
    Camera*                                 m_pCamera;
    CameraController*                       m_pCameraController;
    
    GpuBuffer                   m_uniforms_gpu;
    Uniforms                    m_uniforms_cpu;
    
    
    // Used for camera
    vector_float2                   m_cursorPosition;
    uint8_t                         m_mouseButtonMask;
    float                           m_brushSize;
    MTL::Buffer*                    m_mouseBuffer;
    
    // geometry buffer
    MTL::Texture*                   m_skyCubeMap;
    MTL::Texture*                   m_perlinMap;
    MTL::Texture*                   m_shadowMap;
    MTL::Texture*                   m_depth;
    MTL::Texture*                   m_gBuffer0;
    MTL::Texture*                   m_gBuffer1;
    
    // DepthStencilState
    MTL::DepthStencilState*     m_pShadowDepthState;
    MTL::DepthStencilState*     m_gBufferDepthState;
    MTL::DepthStencilState*     m_lightingDepthState;
    MTL::DepthStencilState*     m_debugDepthState;
    bool                        m_debugPassIsEnabled;
    
    MTL::RenderPassDescriptor*        m_shadowPassDesc;
    MTL::RenderPassDescriptor*        m_gBufferPassDesc;
    MTL::RenderPassDescriptor*        m_gBufferWithLoadPassDesc;
    MTL::RenderPassDescriptor*        m_lightingPassDesc;
    MTL::RenderPassDescriptor*        m_debugPassDesc;
    
    Allocator*            m_frameAllocator;
    MTL::RenderPipelineState*       m_lightingPpl;
    MTL::ComputePipelineState*      m_mousePositionComputeKnl;
    TerrainRenderer*                m_terrainRenderer;

    
};

// inline functions

inline MTL::Device* WorldRenderer::device() const
{
    return m_pDevice;
}

inline MTL::PixelFormat WorldRenderer::colorTargetPixelFormat() const
{
    return MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB;
}

inline MTL::PixelFormat WorldRenderer::depthStencilTargetPixelFormat() const
{
    return MTL::PixelFormat::PixelFormatDepth32Float_Stencil8;
}

inline CameraController* WorldRenderer::getCameraController() const
{
    return m_pCameraController;
}
};
#endif /* WorldRenderer_hpp */
