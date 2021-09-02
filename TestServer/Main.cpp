#include <atlbase.h>
#include "TestServer.h"
#include "TestServer_i.c"
#include "TestComponent_i.c"
#include "DataHandle.hpp"


class TestServerModule : public ATL::CAtlExeModuleT<TestServerModule> {
public:
    TestServerModule() {
    }

    ~TestServerModule() {
        DataHandle::LeakCheck();
    }


    DECLARE_LIBID(LIBID_TestServer)
};

TestServerModule _AtlModule;


// EXE Entry Point
int wWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, wchar_t* /*lpCmdLine*/, int nShowCmd/*=SW_SHOWDEFAULT*/) {
    return _AtlModule.WinMain(nShowCmd);
}
