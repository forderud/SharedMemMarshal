#include <atlbase.h>
#include "TestServer.h"
#include "TestServer_i.c"
#include "TestComponent_i.c"
#include "DataHandle.hpp"


class TestServerModule : public ATL::CAtlExeModuleT<TestServerModule> {
public:
    DECLARE_LIBID(LIBID_TestServer)
};

TestServerModule _AtlModule;


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
