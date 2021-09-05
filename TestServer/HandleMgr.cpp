#include "HandleMgr.hpp"
#include "DataHandle.hpp"


std::atomic<unsigned int> HandleMgr::s_counter{ 0 };


HandleMgr::HandleMgr() {
    s_counter++;

    // log object count to Visual Studio "Output" window
    auto msg = std::string("HandleMgr ctor. (") + std::to_string(s_counter) + " objects).\n";
    OutputDebugString(msg.c_str());
}

HandleMgr::~HandleMgr() {
    s_counter--;

    // log object count to Visual Studio "Output" window
    auto msg = std::string("HandleMgr dtor. (") + std::to_string(s_counter) + " objects).\n";
    OutputDebugString(msg.c_str());
}

HRESULT HandleMgr::GetHandle(BOOL writable, IDataHandle** object) {
    // create object
    auto obj1 = CreateLocalInstance<DataHandle>();
    obj1->Initialize(writable);

    // cast to IDataHandle and return to caller
    CComPtr<IDataHandle> obj2;
    CHECK(obj1.QueryInterface(&obj2));
    *object = obj2.Detach();
    return S_OK;
}