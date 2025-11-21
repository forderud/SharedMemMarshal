#include <array>
#include <cassert>
#include <atlsafe.h>


/** Creates a SafeArray object that points to existing data. */
template <unsigned int N>
SAFEARRAY* CreateWeakSafeArray(std::array<BYTE, N>& buffer, USHORT extra_flags) {
    assert(extra_flags & (FADF_AUTO | FADF_STATIC | FADF_EMBEDDED));
    SAFEARRAY* sa_obj = nullptr;
    HRESULT hr = SafeArrayAllocDescriptorEx(VT_UI1, 1, &sa_obj);
    assert(SUCCEEDED(hr));
    sa_obj->cbElements = 1; // element size
    sa_obj->fFeatures |= extra_flags;
    sa_obj->rgsabound[0] = { static_cast<ULONG>(buffer.size()), 0 };
    {
        hr = SafeArrayLock(sa_obj);
        assert(SUCCEEDED(hr));

        sa_obj->pvData = buffer.data();

        hr = SafeArrayUnlock(sa_obj);
        assert(SUCCEEDED(hr));
    }
    return sa_obj;
}

void VerifyThat_FADF_AUTO_NeitherClearsNorDeletesData() {
    std::array<BYTE, 8> buffer = { 0, 1, 2, 3, 4, 5, 6, 7 };

    SAFEARRAY* sa_old = nullptr;
    {
        // create an array
        CComSafeArray<BYTE> arr;
        arr.Attach(CreateWeakSafeArray(buffer, FADF_AUTO)); // tag data NOT to be cleared nor deleted at destruction

        sa_old = arr.m_psa;
        // SafeArrayDestroy in "arr" dtor does not delete data
    }

    // verify that array is NOT cleared and still accessible
    assert(sa_old->cDims == 1);
    assert(sa_old->cbElements == 1u);
    for (size_t i = 0; i < buffer.size(); ++i)
        assert(reinterpret_cast<BYTE*>(sa_old->pvData)[i] == buffer[i]);

    // manually delete SAFEARRAY descriptor
    HRESULT hr = SafeArrayDestroyDescriptor(sa_old);
    assert(SUCCEEDED(hr));
}


int main() {
    VerifyThat_FADF_AUTO_NeitherClearsNorDeletesData();
}
