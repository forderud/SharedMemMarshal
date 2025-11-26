#pragma once
#include <stdexcept>
#include <Windows.h>


/** "Magic" ring buffer that's mapped twice into virtual memory.
    This simplifies wrap-around handling when reading & writing to the buffer, since the buffer is repeated in the [size, 2*size) range.
    DOC: https://en.wikipedia.org/wiki/Circular_buffer#Optimization and https://gist.github.com/rygorous/3158316
    IMPROVEMENT OPPORTUNITY: https://andreleite.com/posts/2025/nstl/virtual-memory-ring-buffer/ */
class MagicRingBuffer {
public:
    MagicRingBuffer(size_t size) : m_size(size) {
        if (size & 0xFFFF)
            throw std::runtime_error("Buffer size not multiple of 64k");

        Allocate(size);
    }

    ~MagicRingBuffer() {
        Clear();
    }

    /** Get ring-buffer start address. */
    BYTE* Ptr() const {
        return m_ptr;
    }
    /** Get ring-buffer size. Buffer indices are valid up to 2x m_size since the buffer is mapped in twice. */
    size_t Size() const {
        return m_size;
    }

private:
    /** Allocate buffer that is mapped in twice. Retry-based implementation due to risk of data race. */
    void* Allocate(size_t size, unsigned int retries = 4) {
        // allocate buffer object
        m_handle = CreateFileMappingW(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, (size >> 32), (size & 0xFFFFFFFF), 0);
        if (!m_handle)
            throw std::bad_alloc();

        for (size_t i = 0; i < retries; i++) {
            void* target_addr = GetViableAddress(2 * size);
            void* ptr = TryMapAt(size, target_addr); // might sporadically fail
            if (ptr)
                return ptr;
        }

        throw std::bad_alloc();
    }

    /** Attempt to map a buffer of given size twice from the given start address. */
    void* TryMapAt(size_t size, void* start_addr = 0) {
        // try to map first instance of buffer
        // MIGHT SPORADICALLY FAIL if another thread have just allocated another buffer in same address range
        m_ptr = (BYTE*)MapViewOfFileEx(m_handle, FILE_MAP_ALL_ACCESS, 0, 0, size, start_addr);
        if (!m_ptr) {
            Clear();
            return nullptr;
        }
        // try to map second instance of buffer
        // MIGHT SPORADICALLY FAIL if another thread have just allocated another buffer in same address range
        void* ptr2 = MapViewOfFileEx(m_handle, FILE_MAP_ALL_ACCESS, 0, 0, size, (BYTE*)start_addr + size);
        if (!ptr2) {
            Clear();
            return nullptr;
        }

        return m_ptr;
    }

    void Clear() {
        if (m_ptr) {
            UnmapViewOfFile(m_ptr);
            UnmapViewOfFile(m_ptr + m_size);
            m_ptr = 0;
        }

        if (m_handle) {
            CloseHandle(m_handle);
            m_handle = 0;
        }
    }

    /** Determine a suitable address for a buffer of given sizeby first reserving a memory region and then immediately freeing it. */
    static void* GetViableAddress(size_t size) {
        void* ptr = VirtualAlloc(0, size, MEM_RESERVE, PAGE_NOACCESS);
        if (!ptr)
            throw std::bad_alloc();

        VirtualFree(ptr, 0, MEM_RELEASE);
        return ptr;
    }

    const size_t m_size = 0;
    HANDLE m_handle = 0;
    BYTE*  m_ptr = nullptr;
};
