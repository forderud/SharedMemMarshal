#include <iostream>
#include "..\TestServer\TestServer.h"
#include "TestComponent_i.c"
#include "ComSupport.hpp"


/** RAII class for COM initialization. */
class ComInitialize {
public:
    ComInitialize(COINIT apartment /*= COINIT_MULTITHREADED*/) : m_initialized(false) {
        // REF: https://msdn.microsoft.com/en-us/library/windows/desktop/ms695279.aspx
        HRESULT hr = CoInitializeEx(NULL, apartment);
        if (SUCCEEDED(hr))
            m_initialized = true;
    }

    ~ComInitialize() {
        if (m_initialized)
            CoUninitialize();
    }

private:
    bool m_initialized; ///< must uninitialize in dtor
};

void AccessTwoHandles (IHandleMgr* mgr, const unsigned char set_val) {
    CComPtr<IDataHandle> obj1;
    CHECK(mgr->GetHandle(true, &obj1)); // writable

    CComPtr<IDataHandle> obj2;
    CHECK(mgr->GetHandle(false, &obj2)); // read-only

    const unsigned int idx = 0;
    {
        // set mem value in first shared-mem segment
        BYTE* buffer = nullptr;
        unsigned int size = 0;
        CHECK(obj1->GetData(&buffer, &size));
        assert(idx < size);

        buffer[idx] = set_val;

        std::cout << "Value " << (int)set_val << " stored to first IDataHandle.\n";
    }
    {
        // verify that the same value also appear in the second shared-mem segment at a different mem. address
        BYTE* buffer = nullptr;
        unsigned int size = 0;
        CHECK(obj2->GetData(&buffer, &size));
        assert(idx < size);
        assert(buffer[idx] == set_val);

        std::cout << "Value " << (int)buffer[idx] << " read from second IDataHandle.\n";
    }
}

int main() {
    ComInitialize com(COINIT_MULTITHREADED);

    // create COM object in a separate process
    CComPtr<IHandleMgr> mgr;
    {
        CHECK(mgr.CoCreateInstance(L"TestServer.DataCollection")); // will run in separate TestServer.exe
        std::cout << "TestServer.DataCollection created." << std::endl;
    }

    // test shared-mem access
    for (size_t i = 0; i < 2; ++i) {
        AccessTwoHandles(mgr, (BYTE)(41 + 13*i));
    }
}
