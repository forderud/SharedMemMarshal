#pragma once
#include <atomic>
#include "ComSupport.hpp"
#include "Resource.h"
#include "TestComponent_h.h"
#include "SharedMem.hpp"
#include "SignalHandler.hpp"


class ATL_NO_VTABLE DataHandle :
    public CComObjectRootEx<CComMultiThreadModel>, // also compatible with single-threaded apartment
    public CComCoClass<DataHandle, &CLSID_DataHandle>,
    public IDataHandle,
    public IMarshal {
public:
    DataHandle() : m_signal("TestSharedMem_") {
        s_counter++;
    }

    /*NOT virtual*/ ~DataHandle() {
        s_counter--;
    }

    void Initialize(bool writable) {
        // create shared-mem segment
        m_data.reset(new SharedMem(SharedMem::OWNER, "TestSharedMem", writable, 1024));
    }

    typedef CComObjectRootEx<CComMultiThreadModel> PARENT;

    ULONG InternalAddRef() {
        return PARENT::InternalAddRef();
    }

    ULONG InternalRelease() {
        ULONG ref_cnt = PARENT::InternalRelease();

        if (ref_cnt == 0) {
            // no more outstanding references
            if (m_data && (m_data->mode == SharedMem::CLIENT))
                m_signal.Signal(); // signal to server that proxy is deleted
        }

        return ref_cnt;
    }

    HRESULT STDMETHODCALLTYPE GetData(/*out*/BYTE ** buffer, /*out*/unsigned int* size) override {
        if (!buffer || !size)
            return E_INVALIDARG;

        *buffer = m_data->ptr;
        *size = static_cast<unsigned int>(m_data->size);
        return S_OK;
    }

    /** IMarshal implementation. Called from server (stub). */
    HRESULT STDMETHODCALLTYPE GetUnmarshalClass(const IID& iid, void * pv, DWORD destContext, void * reserved, DWORD mshlFlags, CLSID* clsid) override {
        assert(iid == IID_IDataHandle);
        assert(mshlFlags == MSHLFLAGS_NORMAL); mshlFlags; // normal out-of-process marshaling

        *clsid = GetObjectCLSID(); // class will unmarshal itself
        return S_OK;
    }

    /** Indicate the total size of the marshaled object reference. Called from server (stub). */
    HRESULT STDMETHODCALLTYPE GetMarshalSizeMax(const IID& iid, void * /*pv*/, DWORD /*destContext*/, void * /*reserved*/, DWORD mshlFlags, /*out*/ULONG* size) override {
        assert(iid == IID_IDataHandle);
        assert(mshlFlags == MSHLFLAGS_NORMAL); mshlFlags; // normal out-of-process marshaling

        *size = 2*sizeof(unsigned int) + sizeof(bool);
        return S_OK;
    }

    /** Serialize object. Called from server (stub). */
    HRESULT STDMETHODCALLTYPE MarshalInterface(IStream* strm, const IID& iid, void * pv, DWORD destContext, void * reserved, DWORD mshlFlags) override {
        // verify that comm is between processes on same computer with shared-mem support 
        //if (destContext != MSHCTX_LOCAL)
        //    return E_FAIL;

        assert(iid == IID_IDataHandle);
        assert(pv == this); pv;             // class marshals itself
        assert(mshlFlags == MSHLFLAGS_NORMAL); mshlFlags; // normal out-of-process marshaling

        // serialize shared-mem metadata
        *strm << m_data->writable;
        *strm << m_data->size;

        // create signal to receive proxy destruction event (will increment ref-count to avoid premature destruction)
        m_signal.Create(GetUnknown());

        return S_OK;
    }

    /** Deserialize object. Called from client (proxy). */
    HRESULT STDMETHODCALLTYPE UnmarshalInterface(IStream* strm, const IID& iid, void ** ppv) override {
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
    HRESULT STDMETHODCALLTYPE ReleaseMarshalData(IStream * /*strm*/) override {
        return S_OK;
    }

    /** Releases all connections to an object. Have never been observed called.  */
    HRESULT STDMETHODCALLTYPE DisconnectObject(DWORD /*reserved*/) override {
        return S_OK;
    }

    static void LeakCheck() {
        assert((s_counter == 0) && "DataHandle leak detected.");
    }

    DECLARE_REGISTRY_RESOURCEID(IDR_DataHandle)

    BEGIN_COM_MAP(DataHandle)
        COM_INTERFACE_ENTRY(IDataHandle)
        COM_INTERFACE_ENTRY(IMarshal)
    END_COM_MAP()

private:
    std::unique_ptr<SharedMem>       m_data;
    SignalHandler                    m_signal;

    static std::atomic<unsigned int> s_counter; ///< object instance counter (non-decreasing)
};

OBJECT_ENTRY_AUTO(CLSID_DataHandle, DataHandle)
