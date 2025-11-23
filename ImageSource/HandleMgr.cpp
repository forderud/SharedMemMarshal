#include "HandleMgr.hpp"
#include "ImageHandle.hpp"


HandleMgr::HandleMgr() {
}

HandleMgr::~HandleMgr() {
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
