#include "buffer.h"

Buffer::Buffer(int initBufferSize):
                m_buffer(initBufferSize),
                m_readPos(0),m_writePos(0){}

size_t Buffer::ReadableBytes() const
{
    return m_writePos - m_readPos;
}

