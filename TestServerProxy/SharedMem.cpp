#include "SharedMem.hpp"


static void CheckErrorAndThrow(const char* error_msg) {
    DWORD err = GetLastError();

    if (err == ERROR_FILE_NOT_FOUND)
        throw std::runtime_error("ERROR_FILE_NOT_FOUND");
    else if (err == ERROR_NOT_ENOUGH_MEMORY)
        throw std::runtime_error("ERROR_NOT_ENOUGH_MEMORY");

    throw std::runtime_error(error_msg); // default message
}

std::mutex                          SharedMem::s_mutex;
std::unique_ptr<SharedMem::Segment> SharedMem::s_segment;
std::vector<SharedMem::Allocation>  SharedMem::s_allocations;

SharedMem::Inspector::~Inspector() {
    // memory leak check
    assert(SharedMem::s_allocations.empty());
};

SharedMem::Segment::Segment(MODE mode, size_t segm_size) : m_size(segm_size) {
    std::wstring segm_name = L"SharedMemMarshal.Segment";

    if (mode == MODE::OWNER) {
        const DWORD maxSizeHi = (m_size >> 32) & 0xFFFFFFFF;
        const DWORD maxSizeLo = m_size & 0xFFFFFFFF;

        // create shared mem segment
        m_handle = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr/*security*/, PAGE_READWRITE, maxSizeHi, maxSizeLo, segm_name.c_str());
        if (!m_handle)
            CheckErrorAndThrow("CreateFileMapping failed");

        // let image pointer point to shared mem.
        m_ptr = static_cast<unsigned char*>(MapViewOfFile(m_handle, FILE_MAP_ALL_ACCESS, 0, 0, m_size));
        assert((reinterpret_cast<uintptr_t>(m_ptr) % 0x1000) == 0); // ptr is at least 4k aligned, since this is the mem. page size (HW limitation)
        if (!m_ptr)
            CheckErrorAndThrow("MapViewOfFile failed");
    }
    else {
        // open existing shared mem segment
        bool writable = false;
        m_handle = OpenFileMappingW(writable ? FILE_MAP_ALL_ACCESS : FILE_MAP_READ, FALSE, segm_name.c_str());
        if (!m_handle)
            CheckErrorAndThrow("CreateFileMapping failed");

        m_ptr = static_cast<unsigned char*>(MapViewOfFile(m_handle, writable ? FILE_MAP_ALL_ACCESS : FILE_MAP_READ, 0, 0, m_size));
        assert((reinterpret_cast<uintptr_t>(m_ptr) % 0x1000) == 0); // ptr is at least 4k aligned, since this is the mem. page size (HW limitation)
        if (!m_ptr)
            CheckErrorAndThrow("MapViewOfFile failed");
    }
}

SharedMem::Segment::~Segment() {
    UnmapViewOfFile(m_ptr);
    m_ptr = nullptr;

    CloseHandle(m_handle);
    m_handle = nullptr;
}

void* SharedMem::Allocate(size_t size) {
    std::lock_guard<std::mutex> lock(s_mutex);

    if (!s_segment)
        s_segment.reset(new Segment(OWNER, 128*1024*1024)); // 128MB segment size

    // add new allocation after the last one
    size_t offset = 0;
    if (s_allocations.size() > 0) {
        auto& last_alloc = s_allocations.back();
        offset = last_alloc.offset + last_alloc.size;
    }

    Allocation new_alloc{ offset, size };
    s_allocations.push_back(new_alloc);
    return s_segment->m_ptr + new_alloc.offset;
}

void SharedMem::Free(void* ptr) {
    std::lock_guard<std::mutex> lock(s_mutex);

    size_t offset = (BYTE*)ptr - s_segment->m_ptr;

    for (auto it = s_allocations.begin(); it != s_allocations.end(); it++) {
        if (offset != it->offset)
            continue;

        s_allocations.erase(it);

        if (s_allocations.empty())
            s_segment.reset(); // close shared-mem segment
        
        return;
    }
   
    throw std::runtime_error("Unknonw allocation");
}

void* SharedMem::GetPointer(size_t offset) {
    std::lock_guard<std::mutex> lock(s_mutex);

    if (!s_segment)
        s_segment.reset(new Segment(CLIENT, /*map all*/0));

    return s_segment->m_ptr + offset;
}
