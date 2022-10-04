//
//  Allocator.cpp
//  Island
//
//  Created by Chaeyoung Lim on 2022/09/28.
//

#include "Allocator.h"

namespace CHA {

Allocator::Allocator(MTL::Device* device, size_t size, uint8_t ringSize) :
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


GpuBuffer::GpuBuffer () :
m_allocator(NULL),
m_offsetWithinAllocator(0),
m_dataSizeInBytes(0)
{};

GpuBuffer::GpuBuffer (Allocator* inAllocator, size_t inOffset, size_t inSizeInBytes) :
m_allocator (inAllocator),
m_offsetWithinAllocator(inOffset),
m_dataSizeInBytes(inSizeInBytes)
{
    assert (inAllocator != NULL);
}

void GpuBuffer::fillInWith(const Uniforms* data, uint elementCount)
{
    assert (m_allocator != NULL);
    assert (m_offsetWithinAllocator + sizeof (Uniforms) * elementCount <= getBuffer()->length());
    memcpy ((uint8_t*)getBuffer()->contents() + m_offsetWithinAllocator, &(data[0]), sizeof (Uniforms) * elementCount);
}

MTL::Buffer* GpuBuffer::getBuffer () const
{
    assert (m_allocator != NULL);
    return m_allocator->getBuffer();
}

GpuBuffer Allocator::allocBuffer (uint inElementCount)
{
    static const size_t alignment = 256;
    size_t offset = (m_currentlyAllocated + alignment - 1) & ~(alignment - 1);
    size_t size = sizeof(Uniforms) * inElementCount;
    
    CHA_ASSERT(offset + size <= m_buffers[0]->length() , "Not enough space in the Metal buffer allocator to create a new Buffer.");
    m_currentlyAllocated = offset + size;
    return GpuBuffer(this, offset, size);
}

size_t       GpuBuffer::getOffset() const
{
    assert (m_allocator != NULL);
    return m_offsetWithinAllocator;
};

MTL::Buffer* Allocator::getBuffer() const
{
    return m_buffers[m_currentBufferIdx];
}

}
