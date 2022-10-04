//
//  Allocator.hpp
//  Island
//
//  Created by Chaeyoung Lim on 2022/09/28.
//

#ifndef Allocator_h
#define Allocator_h

#import <vector>
#import <simd/simd.h>
#include <Metal/Metal.hpp>
#include "Assert.h"
#include "WorldRenderer_shared.h"


namespace CHA {

class Allocator;

class GpuBuffer
{
    friend class Allocator;
private:
    // Only allowed to be constructed by friend CHAAllocator
    GpuBuffer (Allocator* inAllocator, size_t inOffset, size_t inSizeInBytes);

public:
    GpuBuffer();
    MTL::Buffer*   getBuffer() const;
    size_t         getOffset() const;
    void           fillInWith(const Uniforms* data, uint elementCount);

private:
    Allocator*     m_allocator;
    size_t         m_offsetWithinAllocator;
    size_t         m_dataSizeInBytes;
};

class Allocator
{
public:
    Allocator (MTL::Device* device, size_t size, uint8_t ringSize);
    GpuBuffer                       allocBuffer(uint inElementCount);
    MTL::Buffer*                    getBuffer() const;

private:
    // ARC automatically makes these references strong
    std::vector<MTL::Buffer*>       m_buffers;
    uint8_t                         m_currentBufferIdx;
    size_t                          m_currentlyAllocated;
    bool                            m_isFrozen;
};



}


#endif /* Allocator_h */
