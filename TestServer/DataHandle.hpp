#pragma once
#include <atomic>
#include <memory>
#include "ComSupport.hpp"
#include "Resource.h"
#include "TestServer.h"
#include "SharedMem.hpp"
#include "RefOwner.hpp"


class ATL_NO_VTABLE DataHandle :
    public CComObjectRootEx<CComMultiThreadModel>, // also compatible with single-threaded apartment
    public CComCoClass<DataHandle>, // no registry entries
    public IDataHandle,
    public IMarshal {
public:
    DataHandle() {
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

    HRESULT GetData(/*out*/BYTE ** buffer, /*out*/unsigned int* size) override {
        if (!buffer || !size)
            return E_INVALIDARG;

        *buffer = m_data->ptr;
        *size = m_data->size;
        return S_OK;
    }

    /** IMarshal implementation. Called from server (stub). */
    HRESULT GetUnmarshalClass(const IID& iid, void * pv, DWORD destContext, void * reserved, DWORD mshlFlags, CLSID* clsid) override {
        assert(iid == IID_IDataHandle);
        assert(mshlFlags == MSHLFLAGS_NORMAL); mshlFlags; // normal out-of-process marshaling

        *clsid = CLSID_DataHandleProxy; // use DataHandleProxy class for unmarshaling
        return S_OK;
    }

    /** Indicate the total size of the marshaled object reference. Called from server (stub). */
    HRESULT GetMarshalSizeMax(const IID& iid, void * /*pv*/, DWORD /*destContext*/, void * /*reserved*/, DWORD mshlFlags, /*out*/ULONG* size) override {
        assert(iid == IID_IDataHandle);
        assert(mshlFlags == MSHLFLAGS_NORMAL); mshlFlags; // normal out-of-process marshaling

        constexpr ULONG OBJREF_STANDARD_SIZE = 68; // sizeof(OBJREF) with flags=OBJREF_STANDARD and empty resolver address
        *size = sizeof(m_data->writable) + sizeof(m_data->size) + OBJREF_STANDARD_SIZE;
        return S_OK;
    }

    /** Serialize object. Called from server (stub). */
    HRESULT MarshalInterface(IStream* strm, const IID& iid, void * pv, DWORD destContext, void * reserved, DWORD mshlFlags) override {
        // verify that comm is between processes on same computer with shared-mem support 
        //if (destContext != MSHCTX_LOCAL)
        //    return E_FAIL;

        assert(iid == IID_IDataHandle);
        assert(pv == this); pv;             // class marshals itself
        assert(mshlFlags == MSHLFLAGS_NORMAL); mshlFlags; // normal out-of-process marshaling

        // serialize shared-mem metadata
        *strm << m_data->writable;
        *strm << m_data->size;

        // serialize reference to a RefOwner object to manage references to this object from the proxy
        auto ref_owner = CreateLocalInstance<RefOwner>();
        ref_owner->SetObject(static_cast<IDataHandle*>(this));
        CHECK(CoMarshalInterface(strm, IID_IUnknown, ref_owner, MSHCTX_LOCAL, NULL, MSHLFLAGS_NORMAL));

        return S_OK;
    }

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
