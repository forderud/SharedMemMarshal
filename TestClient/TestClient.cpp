#include <iostream>
#include "../TestComponent/TestComponent_h.h"
#include "../TestComponent/ComSupport.hpp"


int main() {
    ComInitialize com(COINIT_MULTITHREADED);

    // create COM object (will live in a separate apartment)
    CComPtr<ISharedMem> obj;
    CHECK(obj.CoCreateInstance(L"TestComponent.TestSharedMem"));
    std::cout << "Object created" << std::endl;

    const unsigned int idx = 0;
    const unsigned char set_val = 42;
    CHECK(obj->SetData(idx, set_val));
    std::cout << "SetData called" << std::endl;

    unsigned char get_val = 0;
    CHECK(obj->GetData(idx, &get_val));
    std::cout << "GetData called" << std::endl;
    assert(get_val == set_val);
}
