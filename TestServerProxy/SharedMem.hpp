#pragma once
#include <string>
#include <Windows.h>


static void CheckErrorAndThrow(const char* error_msg) {
    DWORD err = GetLastError();

    if (err == ERROR_FILE_NOT_FOUND)
        throw std::runtime_error("ERROR_FILE_NOT_FOUND");
    else if (err == ERROR_NOT_ENOUGH_MEMORY)
        throw std::runtime_error("ERROR_NOT_ENOUGH_MEMORY");

    throw std::runtime_error(error_msg); // default message
}

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

    SharedMem(MODE mode, std::string name, bool _writable, unsigned int segm_size) : writable(_writable), size(segm_size) {
        if (size != static_cast<unsigned int>(size))
            throw std::runtime_error("SharedMemAlloc: too large buffer");

        std::string segm_name = name;

        if (mode == MODE::OWNER) {
            // create shared mem segment
            handle = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr/*security*/, PAGE_READWRITE, 0, static_cast<unsigned int>(size), segm_name.c_str());
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

    HRESULT Serialize(IStream& strm) {
        // serialize metadata
        RETURN_IF_FAILED(strm << writable);
        RETURN_IF_FAILED(strm << size);
        return S_OK;
    }

    static HRESULT DeSerialize(std::string name, IStream& strm, std::unique_ptr<SharedMem> & sm) {
        // de-serialize shared-mem metadata
        bool writable = false;
        RETURN_IF_FAILED(strm >> writable);
        unsigned int obj_size = 0;
        RETURN_IF_FAILED(strm >> obj_size);

        // map shared-mem
        sm = std::make_unique<SharedMem>(SharedMem::CLIENT, name, writable, obj_size);
        return S_OK;
    }

    const bool         writable;
    const unsigned int size   = 0;       ///< shared mem size
    HANDLE             handle = nullptr; ///< shared mem segment handle
    unsigned char    * ptr    = nullptr; ///< pointer to start of shared mem segment

    static const size_t MARSHAL_SIZE = sizeof(writable) + sizeof(size);
};
