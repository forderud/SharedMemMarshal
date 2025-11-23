#pragma once
#include "ComSupport.hpp"


/** Tiny COM oject for controlling the lifetime of another object. */
class ATL_NO_VTABLE RefOwner :
    public CComObjectRootEx<CComMultiThreadModel>, // also compatible with single-threaded apartment
    public CComCoClass<RefOwner>, // no registry entries
    public IUnknown {
public:
    RefOwner() {
    }

    /*NOT virtual*/ ~RefOwner() {
    }

    void SetObject(IUnknown* ptr) {
        m_object = ptr;
    }

    BEGIN_COM_MAP(RefOwner)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

private:
    CComPtr<IUnknown> m_object;  ///< object reference (for lifetime control)
};
