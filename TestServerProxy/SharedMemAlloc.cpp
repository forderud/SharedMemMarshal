#include "SharedMemAlloc.hpp"

SharedMem::SharedMem(MODE mode, std::wstring name, bool _writable, unsigned int segm_size) : writable(_writable), size(segm_size) {
    if (size != static_cast<unsigned int>(size))
        throw std::runtime_error("SharedMemAlloc: too large buffer");

    std::wstring segm_name = name;

    if (mode == MODE::OWNER) {
        // create shared mem segment
        handle = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr/*security*/, PAGE_READWRITE, 0, static_cast<unsigned int>(size), segm_name.c_str());
        if (!handle)
            CheckErrorAndThrow("CreateFileMapping failed");

        // let image pointer point to shared mem.
        ptr = static_cast<unsigned char*>(MapViewOfFile(handle, FILE_MAP_ALL_ACCESS, 0, 0, size));
        assert((reinterpret_cast<uintptr_t>(ptr) % 0x1000) == 0); // ptr is at least 4k aligned, since this is the mem. page size (HW limitation)
        if (!ptr)
            CheckErrorAndThrow("MapViewOfFile failed");
    }
    else {
        // open existing shared mem segment
        handle = OpenFileMappingW(writable ? FILE_MAP_ALL_ACCESS : FILE_MAP_READ, FALSE, segm_name.c_str());
        if (!handle)
            CheckErrorAndThrow("CreateFileMapping failed");

        ptr = static_cast<unsigned char*>(MapViewOfFile(handle, writable ? FILE_MAP_ALL_ACCESS : FILE_MAP_READ, 0, 0, size));
        assert((reinterpret_cast<uintptr_t>(ptr) % 0x1000) == 0); // ptr is at least 4k aligned, since this is the mem. page size (HW limitation)
        if (!ptr)
            CheckErrorAndThrow("MapViewOfFile failed");
    }
}

SharedMem::~SharedMem() {
    UnmapViewOfFile(ptr);
    ptr = nullptr;

    CloseHandle(handle);
    handle = nullptr;
}
