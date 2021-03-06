#pragma once
#include <string>
#include <Windows.h>
#include "Security.hpp"


/** Serialize/deserialize an arbitrary POD type. */
template <class T> IStream& operator<< (IStream& stream, const T& data) {
    unsigned long bytes_written = 0;
    CHECK(stream.Write(&data, sizeof(data), &bytes_written));
    if (bytes_written != sizeof(data))
        throw std::runtime_error("Truncated IStream write");
    return stream;
}
template <class T> IStream& operator>> (IStream& stream, T& data) {
    unsigned long bytes_read = 0;
    CHECK(stream.Read(&data, sizeof(data), &bytes_read));
    if (bytes_read != sizeof(data))
        throw std::runtime_error("Truncated IStream read");
    return stream;
}


struct SharedMem {
    enum MODE {
        OWNER,
        CLIENT,
    };

    SharedMem(MODE _mode, std::string name, bool _writable, unsigned int _segm_idx, unsigned int segm_size) : mode(_mode), writable(_writable), segm_idx(_segm_idx), size(segm_size) {
        if (size != static_cast<unsigned int>(size))
            throw std::runtime_error("SharedMemAlloc: too large buffer");

        std::string segm_name = name + std::to_string(segm_idx);

        if (mode == MODE::OWNER) {
#ifdef ENABLE_GLOBAL_COMMUNICATION
            SecurityEnableAllUsers acl;
            SECURITY_ATTRIBUTES * security = &acl;
#else
            SECURITY_ATTRIBUTES * security = nullptr;
#endif
            // create shared mem segment
            handle = CreateFileMapping(INVALID_HANDLE_VALUE, security, PAGE_READWRITE, 0, static_cast<unsigned int>(size), segm_name.c_str());
            if (!handle)
                CheckErrorAndThrow("CreateFileMapping failed");

            // let image pointer point to shared mem.
            ptr = static_cast<unsigned char*>(MapViewOfFile(handle, FILE_MAP_ALL_ACCESS, 0, 0, size));
            assert((reinterpret_cast<uintptr_t>(ptr) % 0x1000) == 0); // ptr is at least 4k aligned, since this is the mem. page size (HW limitation)
            if (!ptr)
                CheckErrorAndThrow("MapViewOfFile failed");
        } else {
            // open existing shared mem segment
            handle = OpenFileMapping(writable ? FILE_MAP_ALL_ACCESS : FILE_MAP_READ, FALSE, segm_name.c_str());
            if (!handle)
                CheckErrorAndThrow("CreateFileMapping failed");

            ptr = static_cast<unsigned char*>(MapViewOfFile(handle, writable ? FILE_MAP_ALL_ACCESS : FILE_MAP_READ, 0, 0, size));
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

    const MODE         mode;
    const bool         writable;
    const unsigned int segm_idx;
    const unsigned int size   = 0;       ///< shared mem size
    HANDLE             handle = nullptr; ///< shared mem segment handle
    unsigned char    * ptr    = nullptr; ///< pointer to start of shared mem segment
};
