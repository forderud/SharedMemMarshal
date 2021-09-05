#pragma once
#include <atomic>
#include <memory>
#include "ComSupport.hpp"
#include "TestServer.h"
#include "SharedMem.hpp"


class ATL_NO_VTABLE DataHandle :
    public CComObjectRootEx<CComMultiThreadModel>, // also compatible with single-threaded apartment
    public CComCoClass<DataHandle>, // no registry entries
    public IDataHandle,
    public IMarshal {
public:
    DataHandle();

    /*NOT virtual*/ ~DataHandle();

    void Initialize(BOOL writable);

    HRESULT GetData(/*out*/BYTE** buffer, /*out*/unsigned int* size) override;

    /** IMarshal implementation. Called from server (stub). */
    HRESULT GetUnmarshalClass(const IID& iid, void* pv, DWORD destContext, void* reserved, DWORD mshlFlags, CLSID* clsid) override;

    /** Indicate the total size of the marshaled object reference. Called from server (stub). */
    HRESULT GetMarshalSizeMax(const IID& iid, void* /*pv*/, DWORD /*destContext*/, void* /*reserved*/, DWORD mshlFlags, /*out*/ULONG* size) override;

    /** Serialize object. Called from server (stub). */
    HRESULT MarshalInterface(IStream* strm, const IID& iid, void* pv, DWORD destContext, void* reserved, DWORD mshlFlags) override;

    /** Deserialize object. Called from client (proxy). */
    HRESULT UnmarshalInterface(IStream* strm, const IID& iid, void ** ppv) override {
        abort(); // should never be called
    }

    /** Destroys a marshaled data packet. Have never been observed called. */
    HRESULT ReleaseMarshalData(IStream * /*strm*/) override {
        return S_OK;
    }

    /** Releases all connections to an object. Have never been observed called.  */
    HRESULT DisconnectObject(DWORD /*reserved*/) override {
        return S_OK;
    }

    BEGIN_COM_MAP(DataHandle)
        COM_INTERFACE_ENTRY(IDataHandle)
        COM_INTERFACE_ENTRY(IMarshal)
    END_COM_MAP()

private:
    std::unique_ptr<SharedMem>       m_data;
    static std::atomic<unsigned int> s_counter; ///< object instance counter
};
