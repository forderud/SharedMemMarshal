#pragma once
#include <memory>
#include "ComSupport.hpp"
#include "TestServer.h"
#include "SharedMemAlloc.hpp"
#include "MarshalImage.hpp"


class ATL_NO_VTABLE ImageHandle :
    public CComObjectRootEx<CComMultiThreadModel>, // also compatible with single-threaded apartment
    public CComCoClass<ImageHandle>, // no registry entries
    public IImageHandle,
    public IMarshal {
public:
    ImageHandle();

    /*NOT virtual*/ ~ImageHandle();

    void Initialize();

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

    BEGIN_COM_MAP(ImageHandle)
        COM_INTERFACE_ENTRY(IImageHandle)
        COM_INTERFACE_ENTRY(IMarshal)
    END_COM_MAP()

private:
    std::unique_ptr<MarshalImage> m_image;
};
