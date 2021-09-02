#pragma once
#include "ComSupport.hpp"
#include "Resource.h"
#include "TestServer.h"
#include "DataHandle.hpp"


/** Convenience function to create a locally implemented COM instance without the overhead of CoCreateInstance.
The COM class does not need to be registred for construction to succeed. However, lack of registration can
cause problems if transporting the class out-of-process. */
template <class T>
static CComPtr<T> CreateLocalInstance() {
    // create an object (with ref. count zero)
    CComObject<T>* tmp = nullptr;
    CHECK(CComObject<T>::CreateInstance(&tmp));

    // move into smart-ptr (will incr. ref. count to one)
    return CComPtr<T>(tmp);
}


class ATL_NO_VTABLE DataCollection :
    public CComObjectRootEx<CComMultiThreadModel>, // also compatible with single-threaded apartment
    public CComCoClass<DataCollection, &CLSID_DataCollection>,
    public IHandleMgr
{
public:
    DataCollection(){
        s_counter++;

        // log object count to Visual Studio "Output" window
        auto msg = std::string("DataCollection ctor. (") + std::to_string(s_counter) + " objects).\n";
        OutputDebugString(msg.c_str());
    }

    /*NOT virtual*/ ~DataCollection() {
        s_counter--;

        // log object count to Visual Studio "Output" window
        auto msg = std::string("DataCollection dtor. (") + std::to_string(s_counter) + " objects).\n";
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

    DECLARE_REGISTRY_RESOURCEID(IDR_DataCollection)

    BEGIN_COM_MAP(DataCollection)
        COM_INTERFACE_ENTRY(IHandleMgr)
    END_COM_MAP()

private:
    static std::atomic<unsigned int> s_counter; ///< object instance counter
};

OBJECT_ENTRY_AUTO(CLSID_DataCollection, DataCollection)
