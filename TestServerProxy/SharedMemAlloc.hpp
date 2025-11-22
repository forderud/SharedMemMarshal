#pragma once
#include <cassert>
#include <memory>
#include <string>
#include <stdexcept>
#include <vector>
#include <Windows.h>


/** Serialize/deserialize an arbitrary POD type. */
template <class T> HRESULT operator<< (IStream& stream, const T& data) {
    unsigned long bytes_written = 0;
    RETURN_IF_FAILED(stream.Write(&data, sizeof(data), &bytes_written));
    if (bytes_written != sizeof(data))
        return E_FAIL;
    return S_OK;
}
template <class T> HRESULT operator>> (IStream& stream, T& data) {
    unsigned long bytes_read = 0;
    RETURN_IF_FAILED(stream.Read(&data, sizeof(data), &bytes_read));
    if (bytes_read != sizeof(data))
        return E_FAIL;
    return S_OK;
}


struct SharedMem {
    enum MODE {
        OWNER,
        CLIENT,
    };

    struct Allocation {
        size_t offset = 0;
        size_t size = 0;
    };

    /** Client-size pointer resolution. */
    static BYTE* GetPointer(size_t offset);

    /** Owner-size allocation & free. */
    static BYTE* Allocate(size_t size);
    static void Free(BYTE* ptr);

    static size_t GetOffset(BYTE* ptr) {
        assert(m_segment);
        return ptr - m_segment->m_ptr;
    }

private:
    struct Segment {
        Segment(MODE mode, size_t segm_size);
        ~Segment();

        const size_t   m_size = 0;       ///< shared mem size
        HANDLE         m_handle = nullptr; ///< shared mem segment handle
        unsigned char* m_ptr = nullptr; ///< pointer to start of shared mem segment
    };

    static std::unique_ptr<Segment> m_segment;
    static std::vector<Allocation>  m_allocations;
};
