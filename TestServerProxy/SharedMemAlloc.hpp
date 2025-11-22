#pragma once
#include <cassert>
#include <string>
#include <stdexcept>
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


struct SharedMemAlloc {
    enum MODE {
        OWNER,
        CLIENT,
    };

    SharedMemAlloc(MODE mode, size_t segm_size);

    ~SharedMemAlloc();

private:
    const size_t       size   = 0;       ///< shared mem size
    HANDLE             handle = nullptr; ///< shared mem segment handle
public:
    unsigned char    * ptr    = nullptr; ///< pointer to start of shared mem segment
};
