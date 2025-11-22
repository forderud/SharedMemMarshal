#include <iostream>
#include "..\TestServer\TestServer.h"
#include "..\TestServer\TestServer_i.c"
#include "ComSupport.hpp"


void AccessSharedMem (IHandleMgr& mgr) {
    {
        CComPtr<IDataHandle> obj;
        CHECK(mgr.GetDataHandle(&obj));

        // Get pointer to shared-mem segment.
        // The segment is mapped read-only. Write attempts will therefore trigger "Access violation writing location".
        BYTE* buffer = nullptr;
        size_t size = 0;
        CHECK(obj->GetRawData(&buffer, &size));


        std::cout << "Checking shared-mem buffer content..." << std::endl;
        for (size_t i = 0; i < size; i++) {
            if (buffer[i] != (i & 0xFF))
                throw std::runtime_error("incorrect buffer content");
        }
    }
    {
        CComPtr<IImageHandle> img;
        CHECK(mgr.GetImageHandle(&img));

        Image2d frame;
        CHECK(img->GetData(&frame));

        printf("Frame time=%f\n", frame.time);
        printf("Frame dims={%u, %u}\n", frame.dims[0], frame.dims[1]);
        printf("Frame size=%u\n", frame.size());
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

        AccessSharedMem(*mgr);
    }

    // Unload COM.
    // Triggers automatic garbage collection of leaking COM object references in TestServer,
    // except for classes implementing IMarshal.
    CoUninitialize();
}
