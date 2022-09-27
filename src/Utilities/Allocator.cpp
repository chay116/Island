//
//  Allocator.cpp
//  Island
//
//  Created by Chaeyoung Lim on 2022/09/28.
//

#include "Allocator.h"

CHAAllocator::CHAAllocator(MTL::Device* device, size_t size, uint8_t ringSize) :
m_currentBufferIdx (0),
m_currentlyAllocated (0),
m_isFrozen (false)
{
    assert (ringSize > 0);
    for (uint8_t i = 0; i < ringSize; i++)
    {
        m_buffers.push_back(device->newBuffer(size, MTL::CPUCacheModeDefaultCache));
    }
};
