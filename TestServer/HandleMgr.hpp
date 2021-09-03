#pragma once
#include "ComSupport.hpp"
#include "Resource.h"
#include "TestServer.h"
#include "DataHandle.hpp"


class ATL_NO_VTABLE HandleMgr :
    public CComObjectRootEx<CComMultiThreadModel>, // also compatible with single-threaded apartment
    public CComCoClass<HandleMgr, &CLSID_HandleMgr>,
    public IHandleMgr
{
public:
    HandleMgr(){
        s_counter++;

        // log object count to Visual Studio "Output" window
        auto msg = std::string("HandleMgr ctor. (") + std::to_string(s_counter) + " objects).\n";
        OutputDebugString(msg.c_str());
    }

    /*NOT virtual*/ ~HandleMgr() {
        s_counter--;

        // log object count to Visual Studio "Output" window
        auto msg = std::string("HandleMgr dtor. (") + std::to_string(s_counter) + " objects).\n";
        OutputDebugString(msg.c_str());
    }

    HRESULT GetHandle(BOOL writable, IDataHandle ** object) override {
        // create object
        auto obj1 = CreateLocalInstance<DataHandle>();
        obj1->Initialize(writable);

        // cast to IDataHandle and return to caller
        CComPtr<IDataHandle> obj2;
        CHECK(obj1.QueryInterface(&obj2));
        *object = obj2.Detach();
        return S_OK;
    }

    DECLARE_REGISTRY_RESOURCEID(IDR_HandleMgr)

    BEGIN_COM_MAP(HandleMgr)
        COM_INTERFACE_ENTRY(IHandleMgr)
    END_COM_MAP()

private:
    static std::atomic<unsigned int> s_counter; ///< object instance counter
};

OBJECT_ENTRY_AUTO(CLSID_HandleMgr, HandleMgr)
