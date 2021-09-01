#include <iostream>
#include "../TestComponent/TestComponent.h"
#include "../TestComponent/TestComponent_i.c"
#include "../TestComponent/ComSupport.hpp"


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


void AccessTwoHandles (ISharedMem * mgr) {
    CComPtr<IDataHandle> obj1;
    CHECK(mgr->GetHandle(true, &obj1));
    std::cout << "Object #1 retrieved" << std::endl;

    CComPtr<IDataHandle> obj2; // read-only
    CHECK(mgr->GetHandle(false, &obj2));
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

    {
        // Activate fast stub rundown after COM server crashes. Reduces the cleanup delay from ~11min to <10sec
        CComPtr<IGlobalOptions> globalOptions;
        CHECK(globalOptions.CoCreateInstance(CLSID_GlobalOptions, NULL, CLSCTX_INPROC_SERVER));
        CHECK(globalOptions->Set(COMGLB_RO_SETTINGS, COMGLB_FAST_RUNDOWN));
    }

    // create COM object in a separate process
    CComPtr<ISharedMem> mgr;
    {
        DWORD class_context = CLSCTX_LOCAL_SERVER; // force-run in separate process
        CHECK(mgr.CoCreateInstance(L"TestComponent.DataCollection", nullptr, class_context));
        std::cout << "Collection created" << std::endl;
    }

    // test shared-mem access
    for (size_t i = 0; i < 2; ++i) {
        AccessTwoHandles(mgr);
    }
}
