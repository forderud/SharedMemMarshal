#pragma once
#include <cassert>
#include <memory>
#include <mutex>
#include <string>
#include <stdexcept>
#include <vector>
#include <Windows.h>


/** Simple shared-memory allocator.
    Enables "zero copy" buffer exchange across process boundaries. */
struct SharedMem {
    /** Client-size pointer resolution. */
    static void* GetPointer(size_t offset);

    /** Owner-size allocation, free & offset calculation. */
    static void* Allocate(size_t size);

    static void Free(void* ptr);

    static size_t GetOffset(void* ptr) {
        std::lock_guard<std::mutex> lock(s_mutex);
        assert(s_segment);
        return (BYTE*)ptr - s_segment->m_ptr;
    }

private:
    struct Allocation {
        size_t offset = 0;
        size_t size = 0;
    };

    struct Segment {
        Segment(); // open existing segment
        Segment(size_t segm_size); // create new segment
        ~Segment();

        const size_t m_size = 0;       ///< shared mem size
        HANDLE       m_handle = nullptr; ///< shared mem segment handle
        BYTE*        m_ptr = nullptr; ///< pointer to start of shared mem segment
    };

    struct Inspector {
        ~Inspector();
    };

    static std::mutex               s_mutex; ///< protects s_segment & s_allocations
    static std::unique_ptr<Segment> s_segment;
    static std::vector<Allocation>  s_allocations;
    static Inspector                s_inspector;
};
