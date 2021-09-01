#pragma once
#include <map>
#include "ComSupport.hpp"
#include "Resource.h"
#include "TestComponent_h.h"
#include "DataHandle.hpp"


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
            m_obj = obj;
        }

        CComPtr<DataHandle> copy = m_obj;
        *object = copy.Detach();
        return S_OK;
    }

    DECLARE_REGISTRY_RESOURCEID(IDR_DataCollection)

    BEGIN_COM_MAP(DataCollection)
        COM_INTERFACE_ENTRY(ISharedMem)
    END_COM_MAP()

private:
    CComPtr<DataHandle> m_obj;
    std::mutex          m_mutex;
};

OBJECT_ENTRY_AUTO(CLSID_DataCollection, DataCollection)
