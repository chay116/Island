//
//  Texture.cpp
//  Island
//
//  Created by Chaeyoung Lim on 2022/09/25.
//

#include "Texture.h"
#include "Assert.h"
#include <MetalKit/MetalKit.h>

MTL::Texture* CreateTextureWithDevice( MTL::Device* pDevice, const NS::String* filepath, bool sRGB, bool generateMips, MTL::StorageMode storageMode )
{

    MTKTextureLoader* textureLoader = [[MTKTextureLoader alloc] initWithDevice:(__bridge id<MTLDevice>)pDevice];
    NSDictionary<MTKTextureLoaderOption, id> *options =
    @{
        MTKTextureLoaderOptionSRGB:                @( (bool) sRGB ),
        MTKTextureLoaderOptionGenerateMipmaps:     @( (bool) generateMips ),
        MTKTextureLoaderOptionTextureUsage:        @(          (MTLTextureUsage)MTLTextureUsagePixelFormatView | (MTLTextureUsage)MTLTextureUsageShaderRead ),
        MTKTextureLoaderOptionTextureStorageMode : @( (MTLStorageMode)storageMode ),
    };

    
    NSError* __autoreleasing err = nil;
    NSURL* url;
    if ([[(__bridge NSString *)filepath substringToIndex:1] isEqualToString:@"/"])
        url = [NSURL fileURLWithPath: (__bridge NSString *)filepath];
    else
        url = [[NSBundle mainBundle] URLForResource:(__bridge NSString *)filepath withExtension:@""];
    
    id <MTLTexture> texture = [textureLoader newTextureWithContentsOfURL:url
                                                           options:options
                                                             error:&err];
    
    CHA_ASSERT( !err, "Error loading texture:", filepath );
    return (__bridge_retained MTL::Texture*)texture;
}
