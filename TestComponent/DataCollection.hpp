#pragma once
#include "ComSupport.hpp"
#include "Resource.h"
#include "TestComponent.h"
#include "DataHandle.hpp"

extern bool _AtlLeakCheck;

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
    public ISharedMem
{
public:
    DataCollection(){
    }

    /*NOT virtual*/ ~DataCollection() {
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

    HRESULT EnableLeakCheck(BOOL enable) override {
        _AtlLeakCheck = enable;
        return S_OK;
    }

    DECLARE_REGISTRY_RESOURCEID(IDR_DataCollection)

    BEGIN_COM_MAP(DataCollection)
        COM_INTERFACE_ENTRY(ISharedMem)
    END_COM_MAP()
};

OBJECT_ENTRY_AUTO(CLSID_DataCollection, DataCollection)
