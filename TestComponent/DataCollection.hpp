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

    HRESULT STDMETHODCALLTYPE GetHandle(unsigned int idx, BOOL writable, IDataHandle ** object) override {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = m_cache.find(idx);

        if (it == m_cache.end()) {
            // create object, since it's not in cache
            auto obj = CreateLocalInstance<DataHandle>();
            obj->Initialize(idx, writable);
            m_cache[idx] = obj;
            it = m_cache.find(idx);
        }

        CComPtr<DataHandle> copy = it->second;
        *object = copy.Detach();
        return S_OK;
    }

    DECLARE_REGISTRY_RESOURCEID(IDR_DataCollection)

    BEGIN_COM_MAP(DataCollection)
        COM_INTERFACE_ENTRY(ISharedMem)
    END_COM_MAP()

private:
    std::map<unsigned int, CComPtr<DataHandle>> m_cache;
    std::mutex                                  m_mutex;
};

OBJECT_ENTRY_AUTO(CLSID_DataCollection, DataCollection)
