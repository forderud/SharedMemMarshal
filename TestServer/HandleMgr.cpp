#include "HandleMgr.hpp"
#include "DataHandle.hpp"


HandleMgr::HandleMgr() {
}

HandleMgr::~HandleMgr() {
}

HRESULT HandleMgr::GetHandle(IDataHandle** object) {
    // create object
    auto obj1 = CreateLocalInstance<DataHandle>();
    obj1->Initialize();

    // cast to IDataHandle and return to caller
    CComPtr<IDataHandle> obj2;
    CHECK(obj1.QueryInterface(&obj2));
    *object = obj2.Detach();
    return S_OK;
}