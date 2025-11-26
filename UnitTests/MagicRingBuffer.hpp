#pragma once
#include <stdexcept>
#include <Windows.h>

#pragma comment(lib, "onecore.lib") // VirtualAlloc2, MapViewOfFile3


/** "Magic" ring buffer that's mapped twice into virtual memory.
    This simplifies wrap-around handling when reading & writing to the buffer, since the buffer is repeated in the [size, 2*size) range.
    DOC: https://en.wikipedia.org/wiki/Circular_buffer#Optimization and https://andreleite.com/posts/2025/nstl/virtual-memory-ring-buffer/ */
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
        return m_ptr1;
    }
    /** Get ring-buffer size. Buffer indices are valid up to 2x m_size since the buffer is mapped in twice. */
    size_t Size() const {
        return m_size;
    }

private:
    /** Allocate buffer that is mapped in twice. */
    void Allocate(size_t size) {
        // create page-file-backed memory section
        m_handle = CreateFileMappingW(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, (size >> 32), (size & 0xFFFFFFFF), 0);
        if (!m_handle)
            throw std::bad_alloc();

        // reserve virtual address range that is twice as large
        void* placeholder = VirtualAlloc2(GetCurrentProcess(), NULL, 2 * size, MEM_RESERVE | MEM_RESERVE_PLACEHOLDER, PAGE_NOACCESS, NULL, 0);
        if (!placeholder)
            throw std::bad_alloc();

        // split second half of placeholder range
        VirtualFree((BYTE*)placeholder + size, size, MEM_RELEASE | MEM_PRESERVE_PLACEHOLDER);

        // map first buffer instance
        m_ptr1 = (BYTE*)MapViewOfFile3(m_handle, GetCurrentProcess(), placeholder, 0, size, MEM_REPLACE_PLACEHOLDER, PAGE_READWRITE, NULL, 0);
        if (!m_ptr1) {
            Clear();
            throw std::bad_alloc();
        }

        // map second buffer instance
        m_ptr2 = (BYTE*)MapViewOfFile3(m_handle, GetCurrentProcess(), (BYTE*)placeholder + size, 0, size, MEM_REPLACE_PLACEHOLDER, PAGE_READWRITE, NULL, 0);
        if (!m_ptr2) {
            Clear();
            throw std::bad_alloc();
        }

        assert(m_ptr2 == m_ptr1 + size);
    }

    void Clear() {
        if (m_ptr2) {
            UnmapViewOfFile(m_ptr2);
            m_ptr2 = nullptr;
        }

        if (m_ptr1) {
            UnmapViewOfFile(m_ptr1);
            m_ptr1 = nullptr;
        }

        if (m_handle) {
            CloseHandle(m_handle);
            m_handle = 0;
        }
    }

    const size_t m_size = 0;
    HANDLE m_handle = 0;
    BYTE*  m_ptr1 = nullptr;
    BYTE*  m_ptr2 = nullptr;
};
