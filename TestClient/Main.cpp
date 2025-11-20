#include <iostream>
#include "..\TestServer\TestServer.h"
#include "..\TestServer\TestServer_i.c"
#include "ComSupport.hpp"


void AccessTwoHandles (IHandleMgr& mgr, const unsigned char set_val) {
    CComPtr<IDataHandle> obj1;
    CHECK(mgr.GetHandle(true, &obj1)); // writable

    CComPtr<IDataHandle> obj2;
    CHECK(mgr.GetHandle(false, &obj2)); // read-only

    const unsigned int idx = 0;
    {
        // set mem value in first shared-mem segment
        BYTE* buffer = nullptr;
        unsigned int size = 0;
        CHECK(obj1->GetRawData(&buffer, &size));
        assert(idx < size);

        buffer[idx] = set_val;

        std::cout << "Value " << (int)set_val << " stored to first IDataHandle.\n";
    }
    {
        // verify that the same value also appear in the second shared-mem segment at a different mem. address
        BYTE* buffer = nullptr;
        unsigned int size = 0;
        CHECK(obj2->GetRawData(&buffer, &size));
        assert(idx < size);
        assert(buffer[idx] == set_val);

        std::cout << "Value " << (int)buffer[idx] << " read from second IDataHandle.\n";
    }
}

int main() {
    // Initialize COM.
    CoInitializeEx(NULL, COINIT_MULTITHREADED);

    {
        // create COM object in a separate TestServer.exe process
        CComPtr<IHandleMgr> mgr;
        CHECK(mgr.CoCreateInstance(CLSID_HandleMgr));
        std::cout << "TestServer.HandleMgr created." << std::endl;

        // Test shared-mem access.
        for (size_t i = 0; i < 2; ++i) {
            AccessTwoHandles(*mgr, (BYTE)(41 + 13 * i));
        }
    }

    // Unload COM.
    // Triggers automatic garbage collection of leaking COM object references in TestServer,
    // except for classes implementing IMarshal.
    CoUninitialize();
}
