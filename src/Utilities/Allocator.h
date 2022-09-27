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

class CHAAllocator;

template <typename T>
class CHAGpuBuffer
{
    friend class CHAAllocator;
private:
    // Only allowed to be constructed by friend CHAAllocator
    CHAGpuBuffer (CHAAllocator* inAllocator, size_t inOffset, size_t inSizeInBytes) :
    m_allocator (inAllocator),
    m_offsetWithinAllocator(inOffset),
    m_dataSizeInBytes(inSizeInBytes)
    {
        assert (inAllocator != NULL);
    }

public:
    CHAGpuBuffer () :
    m_allocator(NULL),
    m_offsetWithinAllocator(0),
    m_dataSizeInBytes(0)
    {}

    MTL::Buffer*  getBuffer() const;
    size_t         getOffset() const
    {
        assert (m_allocator != NULL);
        return m_offsetWithinAllocator;
    };

    void           fillInWith(const T* data, uint elementCount);

private:
    CHAAllocator*   m_allocator;
    size_t         m_offsetWithinAllocator;
    size_t         m_dataSizeInBytes;
};

class CHAAllocator
{
public:
    CHAAllocator (MTL::Device* device, size_t size, uint8_t ringSize);

    void                            switchToNextBufferInRing();
    void                            freezeNonRingBuffer();
    
    template<typename T>
    CHAGpuBuffer<T>                  allocBuffer(uint inElementCount);
    
    bool                            isWriteable() const;
    MTL::Buffer*                    getBuffer() const;

private:
    // ARC automatically makes these references strong
    std::vector<MTL::Buffer*>       m_buffers;
    uint8_t                         m_currentBufferIdx;
    size_t                          m_currentlyAllocated;
    bool                            m_isFrozen;
};



// Template inline implementations
template <typename T>
void CHAGpuBuffer<T>::fillInWith (const T* data, uint elementCount)
{
    assert (m_allocator != NULL);
    if (!m_allocator->isWriteable())
    {
        assert (false);
        return;
    }
    assert (m_offsetWithinAllocator + sizeof (T) * elementCount <= getBuffer()->length());
    memcpy ((uint8_t*)getBuffer()->contents() + m_offsetWithinAllocator, &(data[0]), sizeof (T) * elementCount);
}

template <typename T>
MTL::Buffer* CHAGpuBuffer <T>::getBuffer () const
{
    assert (m_allocator != NULL);
    return m_allocator->getBuffer();
}

template <typename T>
CHAGpuBuffer <T> CHAAllocator::allocBuffer (uint inElementCount)
{
    static const size_t alignment = 256;
    size_t offset = (m_currentlyAllocated + alignment - 1) & ~(alignment - 1);
    size_t size = sizeof(T) * inElementCount;
    
//    TF_ASSERT(offset + size > m_buffers[0]->length() , "Not enough space in the Metal buffer allocator to create a new Buffer.");
    m_currentlyAllocated = offset + size;
    return CHAGpuBuffer<T>(this, offset, size);
}


inline MTL::Buffer* CHAAllocator::getBuffer() const
{
    return m_buffers[m_currentBufferIdx];
}

inline void CHAAllocator::switchToNextBufferInRing()
{
//    TF_ASSERT(m_buffers.size() > 1 && !m_isFrozen, "Buffer is not allocated");
    m_currentBufferIdx = (m_currentBufferIdx + 1) % m_buffers.size();
};

inline void CHAAllocator::freezeNonRingBuffer()
{
//    TF_ASSERT(m_buffers.size() > 1 && m_currentlyAllocated > 0, "Buffer is not allocated");
    m_isFrozen = true;
};

inline bool CHAAllocator::isWriteable() const
{
    return !m_isFrozen;
};

#endif /* Allocator_h */
