#pragma once
#include <memory>
#include "ComSupport.hpp"
#include "Resource.h"
#include "TestServer.h"


class ATL_NO_VTABLE HandleMgr :
    public CComObjectRootEx<CComMultiThreadModel>, // also compatible with single-threaded apartment
    public CComCoClass<HandleMgr, &CLSID_HandleMgr>,
    public IHandleMgr {
public:
    HandleMgr();

    /*NOT virtual*/ ~HandleMgr();

    HRESULT GetImageHandle(IImageHandle** object) override;

    DECLARE_REGISTRY_RESOURCEID(IDR_HandleMgr)

    BEGIN_COM_MAP(HandleMgr)
        COM_INTERFACE_ENTRY(IHandleMgr)
    END_COM_MAP()
};

OBJECT_ENTRY_AUTO(CLSID_HandleMgr, HandleMgr)
