#include <iostream>
#include <thread>
#include "../TestComponent/TestComponent_h.h"
#include "../TestComponent/ComSupport.hpp"


/** RAII class for impersonating a different user. */
class ImpersonateUser {
public:
    ImpersonateUser() {
    }
    ~ImpersonateUser() {
        if (m_user_token) {
            if (!RevertToSelf()) {
                auto err = GetLastError();
                abort();
            }
        }
    }

    void Impersonate(std::wstring username, std::wstring password) {
        const wchar_t domain[] = L""; // default domain
        if (!LogonUser(username.c_str(), domain, password.c_str(), LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &m_user_token.m_h)) {
            auto err = GetLastError();
            abort();
        }
        if (!ImpersonateLoggedOnUser(m_user_token)) {
            auto err = GetLastError();
            abort();
        }
    }
private:
    CHandle m_user_token;
};

static BYTE GetValue(IDataHandle & h, unsigned int idx) {
    BYTE * buffer = nullptr;
    unsigned int size = 0;
    CHECK(h.GetData(&buffer, &size));
    assert(idx < size);

    return buffer[idx];
}

static void SetValue(IDataHandle & h, unsigned int idx, BYTE val) {
    BYTE * buffer = nullptr;
    unsigned int size = 0;
    CHECK(h.GetData(&buffer, &size));
    assert(idx < size);

    buffer[idx] = val;
}


void AccessOneHandle (ISharedMem * mgr, unsigned int idx) {
    CComPtr<IDataHandle> obj;
    CHECK(mgr->GetHandle(idx, true, &obj));
    std::cout << "Object retrieved" << std::endl;

    const unsigned int pos = 0;
    const unsigned char set_val = 42;
    SetValue(*obj, pos, set_val);
    std::cout << "SetData called" << std::endl;

    unsigned char get_val = GetValue(*obj, pos);
    std::cout << "GetData called" << std::endl;
    assert(get_val == set_val);
}

void AccessTwoHandles (ISharedMem * mgr, unsigned int idx) {
    CComPtr<IDataHandle> obj1;
    CHECK(mgr->GetHandle(idx, true, &obj1));
    std::cout << "Object #1 retrieved" << std::endl;

    CComPtr<IDataHandle> obj2; // read-only
    CHECK(mgr->GetHandle(idx, false, &obj2));
    std::cout << "Object #2 retrieved" << std::endl;

    const unsigned int pos = 0;
    const unsigned char set_val = 42;
    SetValue(*obj1, pos, set_val);
    std::cout << "SetData called" << std::endl;

    unsigned char get_val = GetValue(*obj2, pos);
    std::cout << "GetData called" << std::endl;
    assert(get_val == set_val);
}

int main() {
    ComInitialize com(COINIT_MULTITHREADED);
    ImpersonateUser impersonate;
    //impersonate.Impersonate();

    // create COM object (will live in a separate apartment)
    CComPtr<ISharedMem> mgr;
    CHECK(mgr.CoCreateInstance(L"TestComponent.DataCollection", nullptr, CLSCTX_LOCAL_SERVER | CLSCTX_ENABLE_CLOAKING));
    std::cout << "Collection created" << std::endl;

    {
        // multithreaded testing (triggers simultaneous calls to MarshalInterface)
        std::thread t1(AccessOneHandle, mgr, 0);
        std::thread t2(AccessOneHandle, mgr, 0);
        t1.join();
        t2.join();
    }

    // single-threaded testing
    for (size_t i = 0; i < 2; ++i) {
        AccessTwoHandles(mgr, 0);
    }
}
