#pragma once
#include <cassert>
#include <memory>
#include <mutex>
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


/** Simple shared-memory allocator.
    Enables "zero copy" buffer exchange across process boundaries. */
struct SharedMem {
    /** Client-size pointer resolution. */
    static BYTE* GetPointer(size_t offset);

    /** Owner-size allocation, free & offset calculation. */
    static BYTE* Allocate(size_t size);

    static void Free(BYTE* ptr);

    static size_t GetOffset(BYTE* ptr) {
        std::lock_guard<std::mutex> lock(s_mutex);
        assert(s_segment);
        return ptr - s_segment->m_ptr;
    }

private:
    enum MODE {
        OWNER,
        CLIENT,
    };

    struct Allocation {
        size_t offset = 0;
        size_t size = 0;
    };

    struct Segment {
        Segment(MODE mode, size_t segm_size);
        ~Segment();

        const size_t   m_size = 0;       ///< shared mem size
        HANDLE         m_handle = nullptr; ///< shared mem segment handle
        unsigned char* m_ptr = nullptr; ///< pointer to start of shared mem segment
    };

    struct Inspector {
        ~Inspector();
    };

    static std::mutex               s_mutex; ///< protects s_segment & s_allocations
    static std::unique_ptr<Segment> s_segment;
    static std::vector<Allocation>  s_allocations;
    static Inspector                s_inspector;
};
