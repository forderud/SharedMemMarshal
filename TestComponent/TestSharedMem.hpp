#pragma once
#include "ComSupport.hpp"
#include "Resource.h"
#include "TestComponent_h.h"
#include "SharedMem.hpp"


class ATL_NO_VTABLE TestSharedMem :
    public CComObjectRootEx<CComMultiThreadModel>, // also compatible with single-threaded apartment
    public CComCoClass<TestSharedMem, &CLSID_TestSharedMem>,
    public ISharedMem {
public:
    TestSharedMem() {
        m_data.reset(new SharedMem(SharedMem::OWNER, "TestSharedMem", 0, 1024));
    }

    /*NOT virtual*/ ~TestSharedMem() {
    }

    HRESULT STDMETHODCALLTYPE GetSize(/*out*/unsigned int* size) override {
        if (!size)
            return E_INVALIDARG;

        *size = static_cast<unsigned int>(m_data->size);
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE GetData(unsigned int idx, /*out*/unsigned char * val) override {
        if (!val)
            return E_INVALIDARG;
        if (idx >= m_data->size)
            return E_BOUNDS;

        *val = m_data->ptr[idx];
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE SetData (unsigned int idx, /*in*/unsigned char val) override {
        if (idx >= m_data->size)
            return E_BOUNDS;

        m_data->ptr[idx] = val;
        return S_OK;
    }

    DECLARE_REGISTRY_RESOURCEID(IDR_TestSharedMem)

    BEGIN_COM_MAP(TestSharedMem)
        COM_INTERFACE_ENTRY(ISharedMem)
    END_COM_MAP()

private:
    std::unique_ptr<SharedMem> m_data;
};

OBJECT_ENTRY_AUTO(CLSID_TestSharedMem, TestSharedMem)
