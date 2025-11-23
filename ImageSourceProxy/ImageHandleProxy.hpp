#pragma once
#include <memory>
#include "ComSupport.hpp"
#include "Resource.h"
#include "ImageSourceProxy.h"
#include "../ImageSource/SharedMem.hpp"
#include "../ImageSource/MarshalImage.hpp"


/** Client-side proxy class for the ImageHandle class. */
class ATL_NO_VTABLE ImageHandleProxy :
    public CComObjectRootEx<CComMultiThreadModel>, // also compatible with single-threaded apartment
    public CComCoClass<ImageHandleProxy, &CLSID_ImageHandleProxy>,
    public IImageHandle,
    public IMarshal {
public:
    ImageHandleProxy();

    /*NOT virtual*/ ~ImageHandleProxy();

    HRESULT GetData(/*out*/Image2d* data) override;

    /** IMarshal implementation. Called from server (stub). */
    HRESULT GetUnmarshalClass(const IID& iid, void* pv, DWORD destContext, void* reserved, DWORD mshlFlags, CLSID* clsid) override;

    /** Indicate the total size of the marshaled object reference. Called from server (stub). */
    HRESULT GetMarshalSizeMax(const IID& iid, void* /*pv*/, DWORD /*destContext*/, void* /*reserved*/, DWORD mshlFlags, /*out*/ULONG* size) override;

    /** Serialize object. Called from server (stub). */
    HRESULT MarshalInterface(IStream* strm, const IID& iid, void* pv, DWORD destContext, void* reserved, DWORD mshlFlags) override;

    /** Deserialize object. Called from client (proxy). */
    HRESULT UnmarshalInterface(IStream* strm, const IID& iid, void** ppv) override;

    /** Destroys a marshaled data packet. Have never been observed called. */
    HRESULT ReleaseMarshalData(IStream* /*strm*/) override;

    /** Releases all connections to an object. Have never been observed called.  */
    HRESULT DisconnectObject(DWORD /*reserved*/) override;

    DECLARE_REGISTRY_RESOURCEID(IDR_ImageHandleProxy)

    BEGIN_COM_MAP(ImageHandleProxy)
        COM_INTERFACE_ENTRY(IImageHandle)
        COM_INTERFACE_ENTRY(IMarshal)
    END_COM_MAP()

private:
    std::unique_ptr<MarshalImage> m_image;
    CComPtr<IUnknown>             m_server;  ///< DataHandle server reference (controls lifetime)
};

OBJECT_ENTRY_AUTO(CLSID_ImageHandleProxy, ImageHandleProxy)
