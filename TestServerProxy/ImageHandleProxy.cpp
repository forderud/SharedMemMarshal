#include "ImageHandleProxy.hpp"


ImageHandleProxy::ImageHandleProxy() {
}

ImageHandleProxy::~ImageHandleProxy() {
}

HRESULT ImageHandleProxy::GetData(/*out*/Image2d* data) {
    if (!data)
        return E_INVALIDARG;

    abort();
    //TODO: Add implementation
    return S_OK;
}

/** IMarshal implementation. Called from server (stub). */
HRESULT ImageHandleProxy::GetUnmarshalClass(const IID& iid, void* pv, DWORD destContext, void* reserved, DWORD mshlFlags, CLSID* clsid) {
    abort(); // should never be called
}

/** Indicate the total size of the marshaled object reference. Called from server (stub). */
HRESULT ImageHandleProxy::GetMarshalSizeMax(const IID& iid, void* /*pv*/, DWORD /*destContext*/, void* /*reserved*/, DWORD mshlFlags, /*out*/ULONG* size) {
    abort(); // should never be called
}

/** Serialize object. Called from server (stub). */
HRESULT ImageHandleProxy::MarshalInterface(IStream* strm, const IID& iid, void* pv, DWORD destContext, void* reserved, DWORD mshlFlags) {
    abort(); // should never be called
}

/** Deserialize object. Called from client (proxy). */
HRESULT ImageHandleProxy::UnmarshalInterface(IStream* strm, const IID& iid, void** ppv) {
    // de-serialize shared-mem metadata
    unsigned int obj_size = 0;
    RETURN_IF_FAILED(*strm >> obj_size);

    // map shared-mem
    m_data = std::make_unique<SharedMemAlloc>(SharedMemAlloc::CLIENT, obj_size);

    // deserialize RefOwner reference to control server lifetime
    RETURN_IF_FAILED(CoUnmarshalInterface(strm, IID_PPV_ARGS(&m_server)));

    return QueryInterface(iid, ppv);
}

/** Destroys a marshaled data packet. Have never been observed called. */
HRESULT ImageHandleProxy::ReleaseMarshalData(IStream* strm) {
    // skip over shared-mem metadata
    RETURN_IF_FAILED(strm->Seek({ SharedMemAlloc::MARSHAL_SIZE, 0 }, STREAM_SEEK_CUR, nullptr));

    // release RefOwner ref-count
    RETURN_IF_FAILED(CoReleaseMarshalData(strm));

    return S_OK;
}

/** Releases all connections to an object. Have never been observed called.  */
HRESULT ImageHandleProxy::DisconnectObject(DWORD /*reserved*/) {
    return E_UNEXPECTED;
}
