#include "DataHandleProxy.hpp"

std::atomic<unsigned int> DataHandleProxy::s_counter{ 0 };

DataHandleProxy::DataHandleProxy() {
    s_counter++;
}

DataHandleProxy::~DataHandleProxy() {
    s_counter--;
}

HRESULT DataHandleProxy::GetData(/*out*/BYTE** buffer, /*out*/unsigned int* size) {
    if (!buffer || !size)
        return E_INVALIDARG;

    *buffer = m_data->ptr;
    *size = m_data->size;
    return S_OK;
}

/** IMarshal implementation. Called from server (stub). */
HRESULT DataHandleProxy::GetUnmarshalClass(const IID& iid, void* pv, DWORD destContext, void* reserved, DWORD mshlFlags, CLSID* clsid) {
    abort(); // should never be called
}

/** Indicate the total size of the marshaled object reference. Called from server (stub). */
HRESULT DataHandleProxy::GetMarshalSizeMax(const IID& iid, void* /*pv*/, DWORD /*destContext*/, void* /*reserved*/, DWORD mshlFlags, /*out*/ULONG* size) {
    abort(); // should never be called
}

/** Serialize object. Called from server (stub). */
HRESULT DataHandleProxy::MarshalInterface(IStream* strm, const IID& iid, void* pv, DWORD destContext, void* reserved, DWORD mshlFlags) {
    abort(); // should never be called
}

/** Deserialize object. Called from client (proxy). */
HRESULT DataHandleProxy::UnmarshalInterface(IStream* strm, const IID& iid, void** ppv) {
    // de-serialize shared-mem metadata
    bool writable = false;
    *strm >> writable;
    unsigned int obj_size = 0;
    *strm >> obj_size;

    // deserialize RefOwner reference to control server lifetime
    CHECK(CoUnmarshalInterface(strm, IID_PPV_ARGS(&m_server)));

    // map shared-mem
    m_data.reset(new SharedMem(SharedMem::CLIENT, "TestSharedMem", writable, obj_size));

    return QueryInterface(iid, ppv);
}

/** Destroys a marshaled data packet. Have never been observed called. */
HRESULT DataHandleProxy::ReleaseMarshalData(IStream* /*strm*/) {
    return S_OK;
}

/** Releases all connections to an object. Have never been observed called.  */
HRESULT DataHandleProxy::DisconnectObject(DWORD /*reserved*/) {
    return S_OK;
}
