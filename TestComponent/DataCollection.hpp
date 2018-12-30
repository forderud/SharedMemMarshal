#pragma once
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

    HRESULT GetHandle(unsigned int idx, IDataHandle ** object) {
        auto obj = CreateLocalInstance<DataHandle>();
        obj->Initialize(idx);
        *object = obj.Detach();
        return S_OK;
    }

    DECLARE_REGISTRY_RESOURCEID(IDR_DataCollection)

    BEGIN_COM_MAP(DataCollection)
        COM_INTERFACE_ENTRY(ISharedMem)
    END_COM_MAP()

private:
};

OBJECT_ENTRY_AUTO(CLSID_DataCollection, DataCollection)
