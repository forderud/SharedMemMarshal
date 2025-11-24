#include <iostream>
#include "../ImageSource/ImageSource.h"
#include "../ImageSource/ImageSource_i.c"
#include "../ImageSource/ComSupport.hpp"


void AccessImageData(IHandleMgr& mgr, bool verbose) {
    CComPtr<IImageHandle> img; // controls image-data lifetime
    CHECK(mgr.GetImageHandle(&img));

    Image2d frame;
    CHECK(img->GetData(&frame));

    if (verbose) {
        printf("Frame time=%f\n", frame.time);
        printf("Frame format=%u\n", frame.format);
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
}

int main() {
    // Initialize COM.
    CoInitializeEx(NULL, COINIT_MULTITHREADED);

    {
        // create COM object in a separate ImageSource.exe process
        CComPtr<IHandleMgr> mgr;
        CHECK(mgr.CoCreateInstance(CLSID_HandleMgr));
        std::cout << "ImageSource.HandleMgr created." << std::endl;

        // access image-data 20k times with console logging every 1k iteration
        for (unsigned int it = 0; it < 20000; it++) {
            bool verbose = (it % 1000) == 0;
            if (verbose)
                printf("Iteration %u...\n", it);
            AccessImageData(*mgr, verbose);
            if (verbose)
                printf("\n");
        }
    }

    // Unload COM.
    // Triggers automatic garbage collection of leaking COM object references in ImageSource,
    // except for classes implementing IMarshal.
    CoUninitialize();
}
