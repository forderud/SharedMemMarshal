#include <iostream>
#include "..\TestServer\TestServer.h"
#include "..\TestServer\TestServer_i.c"
#include "ComSupport.hpp"


void AccessSharedMem (IHandleMgr& mgr) {
    CComPtr<IImageHandle> img;
    CHECK(mgr.GetImageHandle(&img));

    Image2d frame;
    CHECK(img->GetData(&frame));

    printf("Frame time=%f\n", frame.time);
    printf("Frame dims={%u, %u}\n", frame.dims[0], frame.dims[1]);
    printf("Frame size=%u\n", frame.size());
    {
        // access image-data in shared-mem segment.
        // The segment is mapped read-only. Write attempts will therefore trigger "Access violation writing location".
        auto* data = (BYTE*)frame.data->pvData;
        printf("Frame data (first 128bytes): ");
        for (size_t i = 0; (i < frame.size()) && (i < 128); i++)
            printf("%u, ", data[i]);
        printf("\n");
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
