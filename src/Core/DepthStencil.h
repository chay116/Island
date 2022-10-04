//
//  DepthStencil.hpp
//  Island
//
//  Created by Chaeyoung Lim on 2022/09/25.
//

#ifndef DepthStencil_hpp
#define DepthStencil_hpp

#include <Metal/Metal.hpp>
#include "GlobalParmeter.h"

namespace CHA {

MTL::DepthStencilState* buildBasicDepthStencilState(MTL::Device* pDevice);
MTL::DepthStencilState* buildGBufferDepthStencilState(MTL::Device* pDevice);
MTL::DepthStencilState* buildDirectionLightDepthStencilState(MTL::Device* pDevice);
MTL::DepthStencilState* buildPointLightMaskDepthStencilState(MTL::Device* pDevice);
MTL::DepthStencilState* buildPointLightDepthStencilState(MTL::Device* pDevice);
MTL::DepthStencilState* buildLightingDepthStencilState(MTL::Device* pDevice);
    MTL::Texture* buildShadowMapTexture(MTL::Device* pDevice);
}

#endif /* DepthStencil_hpp */
