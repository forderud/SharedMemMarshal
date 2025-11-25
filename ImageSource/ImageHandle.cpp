#include "ImageHandle.hpp"
#include "RefOwner.hpp"
#include "../ImageSourceProxy/ImageSourceProxy.h"


ImageHandle::ImageHandle() {
}

ImageHandle::~ImageHandle() {
}

void ImageHandle::Initialize() {
    // allocate image in shared-mem segment
    double time = 3.14;
    unsigned int format = 42;
    USHORT dims[] = { 64, 32 };
    m_image.reset(new MarshalImage(time, format, dims));

    //initialize data
    auto* ptr = (BYTE*)m_image->data->pvData;
    for (size_t i = 0; i < m_image->size(); i++)
        ptr[i] = (i & 0xFF);
}

HRESULT ImageHandle::GetData(/*out*/Image2d* data) {
    if (!data)
        return E_INVALIDARG;

    *data = Image2d(*m_image, m_image->data); // shallow copy
    return S_OK;
}

/** IMarshal implementation. Called from server (stub). */
HRESULT ImageHandle::GetUnmarshalClass(const IID& iid, void* /*pv*/, DWORD /*destContext*/, void* /*reserved*/, DWORD mshlFlags, CLSID* clsid) {
    assert(iid == __uuidof(IImageHandle)); iid;
    assert(mshlFlags == MSHLFLAGS_NORMAL); mshlFlags; // normal out-of-process marshaling

    *clsid = __uuidof(ImageHandleProxy); // use ImageHandleProxy class for unmarshaling
    return S_OK;
}

/** Indicate the total size of the marshaled object reference. Called from server (stub). */
HRESULT ImageHandle::GetMarshalSizeMax(const IID& iid, void* /*pv*/, DWORD /*destContext*/, void* /*reserved*/, DWORD mshlFlags, /*out*/ULONG* size) {
    assert(iid == __uuidof(IImageHandle)); iid;
    assert(mshlFlags == MSHLFLAGS_NORMAL); mshlFlags; // normal out-of-process marshaling

    constexpr ULONG OBJREF_STANDARD_SIZE = 68; // sizeof(OBJREF) with flags=OBJREF_STANDARD and empty resolver address
    *size = MarshalImage::MarshalSize() + OBJREF_STANDARD_SIZE;
    return S_OK;
}

/** Serialize object. Called from server (stub). */
HRESULT ImageHandle::MarshalInterface(IStream* strm, const IID& iid, void* pv, DWORD /*destContext*/, void* /*reserved*/, DWORD mshlFlags) {
    // verify that comm is between processes on same computer with shared-mem support 
    //if (destContext != MSHCTX_LOCAL)
    //    return E_FAIL;

    assert(iid == __uuidof(IImageHandle)); iid;
    assert(pv == this); pv;             // class marshals itself
    assert(mshlFlags == MSHLFLAGS_NORMAL); mshlFlags; // normal out-of-process marshaling

    // serialize shared-mem metadata
    m_image->Serialize(strm);

    // serialize reference to a RefOwner object to manage references to this object from the proxy
    auto ref_owner = CreateLocalInstance<RefOwner>();
    ref_owner->SetObject(static_cast<IImageHandle*>(this));
    RETURN_IF_FAILED(CoMarshalInterface(strm, IID_IUnknown, ref_owner, MSHCTX_LOCAL, NULL, mshlFlags));

    return S_OK;
}

/** Deserialize object. Called from client (proxy). */
HRESULT ImageHandle::UnmarshalInterface(IStream* /*strm*/, const IID& /*iid*/, void** /*ppv*/) {
    abort(); // should never be called
}

/** Destroys a marshaled data packet. Have never been observed called. */
HRESULT ImageHandle::ReleaseMarshalData(IStream* strm) {
    // skip over shared-mem metadata
    RETURN_IF_FAILED(strm->Seek({ MarshalImage::MarshalSize(), 0 }, STREAM_SEEK_CUR, nullptr));

    // release RefOwner ref-count
    RETURN_IF_FAILED(CoReleaseMarshalData(strm));

    return S_OK;
}

/** Releases all connections to an object. Have never been observed called.  */
HRESULT ImageHandle::DisconnectObject(DWORD /*reserved*/) {
    return E_UNEXPECTED;
}
