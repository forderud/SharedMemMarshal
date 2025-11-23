#include <atlbase.h>
#include "ImageSource.h"
#include "ImageSource_i.c"
#include "TestServerProxy_i.c"
#include "ComSupport.hpp"


class ImageSourceModule : public ATL::CAtlExeModuleT<ImageSourceModule> {
public:
    DECLARE_LIBID(LIBID_ImageSource)
};

ImageSourceModule _AtlModule;


// EXE Entry Point
int wWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, wchar_t* /*lpCmdLine*/, int nShowCmd/*=SW_SHOWDEFAULT*/) {
    // initialize COM early to enable IGlobalOptions config
    _AtlModule.InitializeCom();

    {
        // Activate fast stub rundown after COM client crashes. Reduces the cleanup delay from ~11min to <10sec
        CComPtr<IGlobalOptions> globalOptions;
        CHECK(globalOptions.CoCreateInstance(CLSID_GlobalOptions, NULL, CLSCTX_INPROC_SERVER));
        CHECK(globalOptions->Set(COMGLB_RO_SETTINGS, COMGLB_FAST_RUNDOWN));
    }

    return _AtlModule.WinMain(nShowCmd);
}
