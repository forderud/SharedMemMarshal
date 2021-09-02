#pragma once
#include <atomic>
#include "ComSupport.hpp"
#include "Resource.h"
#include "TestServerProxy.h"
#include "SharedMem.hpp"
#include "SignalHandler.hpp"


/** Client-side proxy class for the DataHandle class. */
class ATL_NO_VTABLE DataHandleProxy :
    public CComObjectRootEx<CComMultiThreadModel>, // also compatible with single-threaded apartment
    public CComCoClass<DataHandleProxy, &CLSID_DataHandleProxy>,
    public IDataHandle,
    public IMarshal {
public:
    DataHandleProxy() : m_signal("TestServer_SharedMem") {
        s_counter++;
    }

    /*NOT virtual*/ ~DataHandleProxy() {
        s_counter--;
    }

    typedef CComObjectRootEx<CComMultiThreadModel> PARENT;

    ULONG InternalAddRef() {
        return PARENT::InternalAddRef();
    }

    ULONG InternalRelease() {
        ULONG ref_cnt = PARENT::InternalRelease();

        if (ref_cnt == 0) {
            // no more outstanding references
            if (m_data)
                m_signal.Signal(); // signal to server that proxy is deleted
        }

        return ref_cnt;
    }

    HRESULT GetData(/*out*/BYTE** buffer, /*out*/unsigned int* size) override {
        if (!buffer || !size)
            return E_INVALIDARG;

        *buffer = m_data->ptr;
        *size = m_data->size;
        return S_OK;
    }

    /** IMarshal implementation. Called from server (stub). */
    HRESULT GetUnmarshalClass(const IID& iid, void* pv, DWORD destContext, void* reserved, DWORD mshlFlags, CLSID* clsid) override {
        abort(); // should never be called
    }

    /** Indicate the total size of the marshaled object reference. Called from server (stub). */
    HRESULT GetMarshalSizeMax(const IID& iid, void* /*pv*/, DWORD /*destContext*/, void* /*reserved*/, DWORD mshlFlags, /*out*/ULONG* size) override {
        abort(); // should never be called
    }

    /** Serialize object. Called from server (stub). */
    HRESULT MarshalInterface(IStream* strm, const IID& iid, void* pv, DWORD destContext, void* reserved, DWORD mshlFlags) override {
        abort(); // should never be called
    }

    /** Deserialize object. Called from client (proxy). */
    HRESULT UnmarshalInterface(IStream* strm, const IID& iid, void** ppv) override {
        // de-serialize shared-mem metadata
        bool writable = false;
        *strm >> writable;
        unsigned int obj_size = 0;
        *strm >> obj_size;

        // map shared-mem
        m_data.reset(new SharedMem(SharedMem::CLIENT, "TestSharedMem", writable, obj_size));

        m_signal.Open();

        return QueryInterface(iid, ppv);
    }

    /** Destroys a marshaled data packet. Have never been observed called. */
    HRESULT ReleaseMarshalData(IStream* /*strm*/) override {
        return S_OK;
    }

    /** Releases all connections to an object. Have never been observed called.  */
    HRESULT DisconnectObject(DWORD /*reserved*/) override {
        return S_OK;
    }

    DECLARE_REGISTRY_RESOURCEID(IDR_DataHandleProxy)

    BEGIN_COM_MAP(DataHandleProxy)
        COM_INTERFACE_ENTRY(IDataHandle)
        COM_INTERFACE_ENTRY(IMarshal)
    END_COM_MAP()

private:
    std::unique_ptr<SharedMem>       m_data;
    SignalHandler                    m_signal;

    static std::atomic<unsigned int> s_counter; ///< object instance counter (non-decreasing)
};

OBJECT_ENTRY_AUTO(CLSID_DataHandleProxy, DataHandleProxy)
