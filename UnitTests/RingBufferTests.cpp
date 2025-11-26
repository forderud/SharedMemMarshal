#include <cassert>
#include "MagicRingBuffer.hpp"


void RingBufferTests() {
    const size_t size = 0x10000;// 64k buffer
    MagicRingBuffer buffer(size);

    // initialize buffer with buffer[idx] = idx values
    for (size_t i = 0; i < size; i++)
        buffer[i] = (i % 0xFF);

    // verify that the same buffer pattern is magically repeated in [size, 2*size) range
    for (size_t i = 0; i < size; i++)
        assert(buffer[size + i] == (i % 0xFF));
}
