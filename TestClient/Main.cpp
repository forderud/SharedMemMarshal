#include <iostream>
#include <thread>
#include "../TestComponent/TestComponent_h.h"
#include "../TestComponent/ComSupport.hpp"


void AccessOneHandle (ISharedMem * mgr, unsigned int idx) {
    CComPtr<IDataHandle> obj;
    CHECK(mgr->GetHandle(idx, &obj));
    std::cout << "Object retrieved" << std::endl;

    const unsigned int pos = 0;
    const unsigned char set_val = 42;
    CHECK(obj->SetData(pos, set_val));
    std::cout << "SetData called" << std::endl;

    unsigned char get_val = 0;
    CHECK(obj->GetData(pos, &get_val));
    std::cout << "GetData called" << std::endl;
    assert(get_val == set_val);
}

void AccessTwoHandles (ISharedMem * mgr, unsigned int idx) {
    CComPtr<IDataHandle> obj1;
    CHECK(mgr->GetHandle(idx, &obj1));
    std::cout << "Object #1 retrieved" << std::endl;

    CComPtr<IDataHandle> obj2;
    CHECK(mgr->GetHandle(idx, &obj2));
    std::cout << "Object #2 retrieved" << std::endl;

    const unsigned int pos = 0;
    const unsigned char set_val = 42;
    CHECK(obj1->SetData(pos, set_val));
    std::cout << "SetData called" << std::endl;

    unsigned char get_val = 0;
    CHECK(obj2->GetData(pos, &get_val));
    std::cout << "GetData called" << std::endl;
    assert(get_val == set_val);
}

int main() {
    ComInitialize com(COINIT_MULTITHREADED);

    // create COM object (will live in a separate apartment)
    CComPtr<ISharedMem> mgr;
    CHECK(mgr.CoCreateInstance(L"TestComponent.DataCollection", nullptr, CLSCTX_LOCAL_SERVER));
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
