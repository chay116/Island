//
//  Texture.hpp
//  Island
//
//  Created by Chaeyoung Lim on 2022/09/25.
//

#ifndef Texture_hpp
#define Texture_hpp

#include <Metal/Metal.hpp>

MTL::Texture* CreateTextureWithDevice( MTL::Device* pDevice, const NS::String* filepath, bool sRGB, bool generateMips, MTL::StorageMode storageMode = MTL::StorageModePrivate );

#endif /* Texture_hpp */
