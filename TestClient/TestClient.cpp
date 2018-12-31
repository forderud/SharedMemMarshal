#include <iostream>
#include "../TestComponent/TestComponent_h.h"
#include "../TestComponent/ComSupport.hpp"


int main() {
    ComInitialize com(COINIT_MULTITHREADED);

    // create COM object (will live in a separate apartment)
    CComPtr<ISharedMem> mgr;
    CHECK(mgr.CoCreateInstance(L"TestComponent.DataCollection", nullptr, CLSCTX_LOCAL_SERVER));
    std::cout << "Collection created" << std::endl;

    for (size_t i = 0; i < 2; ++i) {
        CComPtr<IDataHandle> obj;
        CHECK(mgr->GetHandle(0, &obj));
        std::cout << "Object retrieved" << std::endl;

        const unsigned int idx = 0;
        const unsigned char set_val = 42;
        CHECK(obj->SetData(idx, set_val));
        std::cout << "SetData called" << std::endl;

        unsigned char get_val = 0;
        CHECK(obj->GetData(idx, &get_val));
        std::cout << "GetData called" << std::endl;
        assert(get_val == set_val);
    }
}
