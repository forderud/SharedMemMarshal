#include "Resource.h"
#include "TestComponent.h"
#include "TestComponent_i.c"
#include "ComSupport.hpp"
#include "DataHandle.hpp"
#include "DataHandleProxy.hpp"


bool _AtlLeakCheck = false;

class TestComponentModule : public ATL::CAtlDllModuleT<TestComponentModule> {
public:
    TestComponentModule() {
    }

    ~TestComponentModule() {
        if (_AtlLeakCheck) {
            DataHandle::LeakCheck();
            DataHandleProxy::LeakCheck();
        }
    }

    DECLARE_LIBID(LIBID_TestComponent)
    DECLARE_REGISTRY_APPID_RESOURCEID(IDR_AppID, "{CDD196FE-70ED-46F4-BED7-57615CB78F9B}")
};

TestComponentModule _AtlModule;



// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE /*hInstance*/, DWORD dwReason, LPVOID lpReserved) {
    if (dwReason == DLL_PROCESS_ATTACH) {
#if 0
        // break dllhost.exe until debugger is attached
        HANDLE process = GetCurrentProcess();
        TCHAR filename[MAX_PATH] = {};
        GetProcessImageFileName(process, filename, MAX_PATH);
        if (std::string(filename).find("dllhost.exe") != std::string::npos) {
            while (!IsDebuggerPresent())
                Sleep(200);
        }
#endif
    }

    return _AtlModule.DllMain(dwReason, lpReserved);
}

// Used to determine whether the DLL can be unloaded by OLE.
STDAPI DllCanUnloadNow() {
    return _AtlModule.DllCanUnloadNow();
}

// Returns a class factory to create an object of the requested type.
_Check_return_
STDAPI DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID* ppv) {
    return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}

// DllRegisterServer - Adds entries to the system registry.
STDAPI DllRegisterServer() {
    // registers object, typelib and all interfaces in typelib
    return _AtlModule.DllRegisterServer();
}

// DllUnregisterServer - Removes entries from the system registry.
STDAPI DllUnregisterServer() {
    return _AtlModule.DllUnregisterServer();
}

// DllInstall - Adds/Removes entries to the system registry per user per machine.
STDAPI DllInstall(BOOL bInstall, _In_opt_  LPCWSTR pszCmdLine) {
    static const wchar_t szUserSwitch[] = L"user";

    if (pszCmdLine != NULL) {
        if (_wcsnicmp(pszCmdLine, szUserSwitch, _countof(szUserSwitch)) == 0)
            ATL::AtlSetPerUserRegistration(true);
    }

    HRESULT hr = E_FAIL;
    if (bInstall) {
        hr = DllRegisterServer();
        if (FAILED(hr))
            DllUnregisterServer();
    } else {
        hr = DllUnregisterServer();
    }

    return hr;
}
