#pragma once
#include <mutex>
#include "ComSupport.hpp"
#include "Resource.h"
#include "TestComponent.h"
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
    public ISharedMem
{
public:
    DataCollection(){
    }

    /*NOT virtual*/ ~DataCollection() {
    }

    HRESULT STDMETHODCALLTYPE GetHandle(BOOL writable, IDataHandle ** object) override {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (!m_obj) {
            // create object on demand
            auto obj = CreateLocalInstance<DataHandle>();
            obj->Initialize(writable);
            CHECK(obj.QueryInterface(&m_obj));
        }

        CComPtr<IDataHandle> copy = m_obj;
        *object = copy.Detach();
        return S_OK;
    }

    DECLARE_REGISTRY_RESOURCEID(IDR_DataCollection)

    BEGIN_COM_MAP(DataCollection)
        COM_INTERFACE_ENTRY(ISharedMem)
    END_COM_MAP()

private:
    CComPtr<IDataHandle> m_obj;
    std::mutex           m_mutex;
};

OBJECT_ENTRY_AUTO(CLSID_DataCollection, DataCollection)
