#pragma once
#include <atomic>
#include "ComSupport.hpp"
#include "Resource.h"
#include "TestComponent.h"
#include "SharedMem.hpp"
#include "SignalHandler.hpp"


class ATL_NO_VTABLE DataHandle :
    public CComObjectRootEx<CComMultiThreadModel>, // also compatible with single-threaded apartment
    public CComCoClass<DataHandle>, // no registry entries
    public IDataHandle,
    public IMarshal {
public:
    DataHandle() : m_signal("TestSharedMem_") {
        s_counter++;

        // log object count to Visual Studio "Output" window
        auto msg = std::string("DataHandle ctor. (") + std::to_string(s_counter) + " objects).\n";
        OutputDebugString(msg.c_str());
    }

    /*NOT virtual*/ ~DataHandle() {
        s_counter--;

        // log object count to Visual Studio "Output" window
        auto msg = std::string("DataHandle dtor. (") + std::to_string(s_counter) + " objects).\n";
        OutputDebugString(msg.c_str());
    }

    void Initialize(BOOL writable) {
        // create shared-mem segment
        m_data.reset(new SharedMem(SharedMem::OWNER, "TestSharedMem", writable, 1024));
    }

    HRESULT STDMETHODCALLTYPE GetData(/*out*/BYTE ** buffer, /*out*/unsigned int* size) override {
        if (!buffer || !size)
            return E_INVALIDARG;

        *buffer = m_data->ptr;
        *size = m_data->size;
        return S_OK;
    }

    /** IMarshal implementation. Called from server (stub). */
    HRESULT STDMETHODCALLTYPE GetUnmarshalClass(const IID& iid, void * pv, DWORD destContext, void * reserved, DWORD mshlFlags, CLSID* clsid) override {
        assert(iid == IID_IDataHandle);
        assert(mshlFlags == MSHLFLAGS_NORMAL); mshlFlags; // normal out-of-process marshaling

        *clsid = CLSID_DataHandleProxy; // use DataHandleProxy class for unmarshaling
        return S_OK;
    }

    /** Indicate the total size of the marshaled object reference. Called from server (stub). */
    HRESULT STDMETHODCALLTYPE GetMarshalSizeMax(const IID& iid, void * /*pv*/, DWORD /*destContext*/, void * /*reserved*/, DWORD mshlFlags, /*out*/ULONG* size) override {
        assert(iid == IID_IDataHandle);
        assert(mshlFlags == MSHLFLAGS_NORMAL); mshlFlags; // normal out-of-process marshaling

        *size = sizeof(m_data->writable) + sizeof(m_data->size);
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
        abort(); // should never be called
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

    BEGIN_COM_MAP(DataHandle)
        COM_INTERFACE_ENTRY(IDataHandle)
        COM_INTERFACE_ENTRY(IMarshal)
    END_COM_MAP()

private:
    std::unique_ptr<SharedMem>       m_data;
    SignalHandler                    m_signal;

    static std::atomic<unsigned int> s_counter; ///< object instance counter (non-decreasing)
};
