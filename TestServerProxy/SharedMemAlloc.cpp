#include "SharedMemAlloc.hpp"


static void CheckErrorAndThrow(const char* error_msg) {
    DWORD err = GetLastError();

    if (err == ERROR_FILE_NOT_FOUND)
        throw std::runtime_error("ERROR_FILE_NOT_FOUND");
    else if (err == ERROR_NOT_ENOUGH_MEMORY)
        throw std::runtime_error("ERROR_NOT_ENOUGH_MEMORY");

    throw std::runtime_error(error_msg); // default message
}

SharedMem::SharedMem(MODE mode, size_t segm_size) : m_size(segm_size) {
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

SharedMem::~SharedMem() {
    UnmapViewOfFile(m_ptr);
    m_ptr = nullptr;

    CloseHandle(m_handle);
    m_handle = nullptr;
}
