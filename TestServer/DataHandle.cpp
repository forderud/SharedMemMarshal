#include "DataHandle.hpp"
#include "RefOwner.hpp"


DataHandle::DataHandle() {
}

DataHandle::~DataHandle() {
}

void DataHandle::Initialize(BOOL writable) {
    // create shared-mem segment
    m_data.reset(new SharedMem(SharedMem::OWNER, "TestSharedMem", writable, 1024));
}

HRESULT DataHandle::GetData(/*out*/BYTE** buffer, /*out*/unsigned int* size) {
    if (!buffer || !size)
        return E_INVALIDARG;

    *buffer = m_data->ptr;
    *size = m_data->size;
    return S_OK;
}

/** IMarshal implementation. Called from server (stub). */
HRESULT DataHandle::GetUnmarshalClass(const IID& iid, void* pv, DWORD destContext, void* reserved, DWORD mshlFlags, CLSID* clsid) {
    assert(iid == IID_IDataHandle);
    assert(mshlFlags == MSHLFLAGS_NORMAL); mshlFlags; // normal out-of-process marshaling

    *clsid = CLSID_DataHandleProxy; // use DataHandleProxy class for unmarshaling
    return S_OK;
}

/** Indicate the total size of the marshaled object reference. Called from server (stub). */
HRESULT DataHandle::GetMarshalSizeMax(const IID& iid, void* /*pv*/, DWORD /*destContext*/, void* /*reserved*/, DWORD mshlFlags, /*out*/ULONG* size) {
    assert(iid == IID_IDataHandle);
    assert(mshlFlags == MSHLFLAGS_NORMAL); mshlFlags; // normal out-of-process marshaling

    constexpr ULONG OBJREF_STANDARD_SIZE = 68; // sizeof(OBJREF) with flags=OBJREF_STANDARD and empty resolver address
    *size = sizeof(m_data->writable) + sizeof(m_data->size) + OBJREF_STANDARD_SIZE;
    return S_OK;
}

/** Serialize object. Called from server (stub). */
HRESULT DataHandle::MarshalInterface(IStream* strm, const IID& iid, void* pv, DWORD destContext, void* reserved, DWORD mshlFlags) {
    // verify that comm is between processes on same computer with shared-mem support 
    //if (destContext != MSHCTX_LOCAL)
    //    return E_FAIL;

    assert(iid == IID_IDataHandle);
    assert(pv == this); pv;             // class marshals itself
    assert(mshlFlags == MSHLFLAGS_NORMAL); mshlFlags; // normal out-of-process marshaling

    // serialize shared-mem metadata
    m_data->Serialize(*strm);

    // serialize reference to a RefOwner object to manage references to this object from the proxy
    auto ref_owner = CreateLocalInstance<RefOwner>();
    ref_owner->SetObject(static_cast<IDataHandle*>(this));
    CHECK(CoMarshalInterface(strm, IID_IUnknown, ref_owner, MSHCTX_LOCAL, NULL, mshlFlags));

    return S_OK;
}

/** Deserialize object. Called from client (proxy). */
HRESULT DataHandle::UnmarshalInterface(IStream* strm, const IID& iid, void** ppv) {
    abort(); // should never be called
}

/** Destroys a marshaled data packet. Have never been observed called. */
HRESULT DataHandle::ReleaseMarshalData(IStream* /*strm*/) {
    return S_OK;
}

/** Releases all connections to an object. Have never been observed called.  */
HRESULT DataHandle::DisconnectObject(DWORD /*reserved*/) {
    return E_UNEXPECTED;
}