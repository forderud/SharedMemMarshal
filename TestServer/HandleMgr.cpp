#include "HandleMgr.hpp"
#include "DataHandle.hpp"
#include "ImageHandle.hpp"


HandleMgr::HandleMgr() {
}

HandleMgr::~HandleMgr() {
}

HRESULT HandleMgr::GetDataHandle(IDataHandle** object) {
    // create object
    auto obj1 = CreateLocalInstance<DataHandle>();
    obj1->Initialize();

    // cast to IDataHandle and return to caller
    CComPtr<IDataHandle> obj2;
    CHECK(obj1.QueryInterface(&obj2));
    *object = obj2.Detach();
    return S_OK;
}

HRESULT HandleMgr::GetImageHandle(IImageHandle** object) {
    // create object
    auto obj1 = CreateLocalInstance<ImageHandle>();
    obj1->Initialize();

    // cast to IImageHandle and return to caller
    CComPtr<IImageHandle> obj2;
    CHECK(obj1.QueryInterface(&obj2));
    *object = obj2.Detach();
    return S_OK;
}
