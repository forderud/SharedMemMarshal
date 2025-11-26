#include <cassert>
#include "MagicRingBuffer.hpp"


void RingBufferTests() {
    const size_t size = 0x10000;// 64k buffer
    MagicRingBuffer buffer(size);

    // initialize buffer with buffer[idx] = idx values
    for (size_t i = 0; i < size; i++)
        buffer.Ptr()[i] = (i % 0xFF);

    // verify that the same buffer pattern is repeated in [size, 2*size) range
    for (size_t i = 0; i < size; i++)
        assert(buffer.Ptr()[size + i] == (i % 0xFF));

}
