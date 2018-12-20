#pragma once
#include <string>
#include <Windows.h>


static void CheckErrorAndThrow(const char * error_msg) {
    DWORD err = GetLastError();

    if (err == ERROR_NOT_ENOUGH_MEMORY)
        throw std::runtime_error("ERROR_NOT_ENOUGH_MEMORY");

    throw std::runtime_error(error_msg); // default message
}


struct SharedMem {
    enum MODE {
        OWNER,
        CLIENT,
    };

    SharedMem(MODE mode, std::string name, unsigned int segm_idx, size_t segm_size) : size(segm_size) {
        if (size != static_cast<unsigned int>(size))
            throw std::runtime_error("SharedMemAlloc: too large buffer");

        std::string segm_name = name + std::to_string(segm_idx);

        if (mode == MODE::OWNER) {
            // create shared mem segment
            handle = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, static_cast<unsigned int>(size), segm_name.c_str());
            if (!handle)
                CheckErrorAndThrow("CreateFileMapping failed");

            // let image pointer point to shared mem.
            ptr = static_cast<unsigned char*>(MapViewOfFile(handle, FILE_MAP_ALL_ACCESS, 0, 0, size));
            assert((reinterpret_cast<uintptr_t>(ptr) % 0x1000) == 0); // ptr is at least 4k aligned, since this is the mem. page size (HW limitation)
            if (!ptr)
                CheckErrorAndThrow("MapViewOfFile failed");
        } else {
            // open shared mem segment in read-only mode
            handle = OpenFileMapping(FILE_MAP_READ, FALSE, segm_name.c_str());
            if (!handle)
                CheckErrorAndThrow("CreateFileMapping failed");

            ptr = static_cast<unsigned char*>(MapViewOfFile(handle, FILE_MAP_READ, 0, 0, size));
            assert((reinterpret_cast<uintptr_t>(ptr) % 0x1000) == 0); // ptr is at least 4k aligned, since this is the mem. page size (HW limitation)
            if (!ptr)
                CheckErrorAndThrow("MapViewOfFile failed");
        }
    }

    ~SharedMem() {
        UnmapViewOfFile(ptr);
        ptr = nullptr;

        CloseHandle(handle);
        handle = nullptr;
    }

    HANDLE          handle = nullptr; ///< shared mem segment handle
    unsigned char * ptr    = nullptr; ///< pointer to start of shared mem segment
    const size_t    size   = 0;       ///< shared mem size
};
