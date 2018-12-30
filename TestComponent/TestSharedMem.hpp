#pragma once
#include <atomic>
#include "ComSupport.hpp"
#include "Resource.h"
#include "TestComponent_h.h"
#include "SharedMem.hpp"
#include "SignalHandler.hpp"


class ATL_NO_VTABLE TestSharedMem :
    public CComObjectRootEx<CComMultiThreadModel>, // also compatible with single-threaded apartment
    public CComCoClass<TestSharedMem, &CLSID_TestSharedMem>,
    public ISharedMem,
    public IMarshal {
public:
    TestSharedMem() : m_signal("TestSharedMem_") {
        s_counter++;

        // create shared-mem segment
        m_data.reset(new SharedMem(SharedMem::OWNER, "TestSharedMem", s_counter, 1024));
    }

    /*NOT virtual*/ ~TestSharedMem() {
    }

    typedef CComObjectRootEx<CComMultiThreadModel> PARENT;

    ULONG InternalAddRef() {
        return PARENT::InternalAddRef();
    }

    ULONG InternalRelease() {
        if (m_data && (m_data->mode == SharedMem::CLIENT) && (m_dwRef == 1)) {
            // last proxy reference released
            m_signal.Signal();
        }

        return PARENT::InternalRelease();
    }

    HRESULT STDMETHODCALLTYPE GetSize(/*out*/unsigned int* size) override {
        if (!size)
            return E_INVALIDARG;

        *size = static_cast<unsigned int>(m_data->size);
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE GetData(unsigned int idx, /*out*/unsigned char * val) override {
        if (!val)
            return E_INVALIDARG;
        if (idx >= m_data->size)
            return E_BOUNDS;

        *val = m_data->ptr[idx];
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE SetData (unsigned int idx, /*in*/unsigned char val) override {
        if (idx >= m_data->size)
            return E_BOUNDS;

        m_data->ptr[idx] = val;
        return S_OK;
    }


    /** IMarshal implementation. Called from server (stub). */
    HRESULT STDMETHODCALLTYPE GetUnmarshalClass(const IID& iid, void * pv, DWORD destContext, void * reserved, DWORD mshlFlags, CLSID* clsid) override {
        assert(iid == IID_ISharedMem);
        assert(mshlFlags == MSHLFLAGS_NORMAL); mshlFlags; // normal out-of-process marshaling

        *clsid = GetObjectCLSID(); // class will unmarshal itself
        return S_OK;
    }

    /** Indicate the total size of the marshaled object reference. Called from server (stub). */
    HRESULT STDMETHODCALLTYPE GetMarshalSizeMax(const IID& iid, void * /*pv*/, DWORD /*destContext*/, void * /*reserved*/, DWORD mshlFlags, /*out*/ULONG* size) override {
        assert(iid == IID_ISharedMem);
        assert(mshlFlags == MSHLFLAGS_NORMAL); mshlFlags; // normal out-of-process marshaling

        *size = 2*sizeof(unsigned int);
        return S_OK;
    }

    /** Serialize object. Called from server (stub). */
    HRESULT STDMETHODCALLTYPE MarshalInterface(IStream* strm, const IID& iid, void * pv, DWORD destContext, void * reserved, DWORD mshlFlags) override {
        // verify that comm is between processes on same computer with shared-mem support 
        //if (destContext != MSHCTX_LOCAL)
        //    return E_FAIL;

        assert(iid == IID_ISharedMem);
        assert(pv == this); pv;             // class marshals itself
        assert(mshlFlags == MSHLFLAGS_NORMAL); mshlFlags; // normal out-of-process marshaling

        // serialize shared-mem metadata
        *strm << m_data->segm_idx;
        *strm << m_data->size;

        // create signal to receive proxy destruction event (will increment ref-count to avoid premature destruction)
        m_signal.Create(m_data->segm_idx, GetUnknown());

        return S_OK;
    }

    /** Deserialize object. Called from client (proxy). */
    HRESULT STDMETHODCALLTYPE UnmarshalInterface(IStream* strm, const IID& iid, void ** ppv) override {
        // de-serialize shared-mem metadata
        unsigned int obj_idx = 0;
        *strm >> obj_idx;
        unsigned int obj_size = 0;
        *strm >> obj_size;

        // map shared-mem
        m_data.reset(new SharedMem(SharedMem::CLIENT, "TestSharedMem", obj_idx, obj_size));

        m_signal.Open(obj_idx);

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

    DECLARE_REGISTRY_RESOURCEID(IDR_TestSharedMem)

    BEGIN_COM_MAP(TestSharedMem)
        COM_INTERFACE_ENTRY(ISharedMem)
        COM_INTERFACE_ENTRY(IMarshal)
    END_COM_MAP()

private:
    std::unique_ptr<SharedMem>       m_data;
    SignalHandler                    m_signal;

    static std::atomic<unsigned int> s_counter; ///< object instance counter (non-decreasing)
};

OBJECT_ENTRY_AUTO(CLSID_TestSharedMem, TestSharedMem)
