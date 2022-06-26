#include "DataHandleProxy.hpp"


DataHandleProxy::DataHandleProxy() {
}

DataHandleProxy::~DataHandleProxy() {
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
    // map shared-mem
    RETURN_IF_FAILED(SharedMem::DeSerialize("TestSharedMem", *strm, m_data));

    // deserialize RefOwner reference to control server lifetime
    RETURN_IF_FAILED(CoUnmarshalInterface(strm, IID_PPV_ARGS(&m_server)));

    return QueryInterface(iid, ppv);
}

/** Destroys a marshaled data packet. Have never been observed called. */
HRESULT DataHandleProxy::ReleaseMarshalData(IStream* strm) {
    // skip over shared-mem metadata
    RETURN_IF_FAILED(strm->Seek({ SharedMem::MARSHAL_SIZE, 0 }, STREAM_SEEK_CUR, nullptr));

    // release RefOwner ref-count
    RETURN_IF_FAILED(CoReleaseMarshalData(strm));

    return S_OK;
}

/** Releases all connections to an object. Have never been observed called.  */
HRESULT DataHandleProxy::DisconnectObject(DWORD /*reserved*/) {
    return E_UNEXPECTED;
}
