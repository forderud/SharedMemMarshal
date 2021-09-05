#include "HandleMgr.hpp"
#include "DataHandle.hpp"


HandleMgr::HandleMgr() {
}

HandleMgr::~HandleMgr() {
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