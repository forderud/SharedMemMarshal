/* "Plugin API" convenience functions.
Designed by Fredrik Orderud <fredrik.orderud@ge.com>.
Copyright (c) 2016, GE Healthcare, Ultrasound.           */
#pragma once

#include <vector>
#include <stdexcept>
#include <cassert>

#include <comdef.h> // for _com_error
#include <atlbase.h>
#include <atlsafe.h> // for CComSafeArray
#include <atlcom.h>  // for CComObject


// Alternative to the WIL macro impl.
// DOC: https://github.com/Microsoft/wil/wiki/Error-handling-helpers
// Please delete this impl. if WIL or other 3rd party impl. becomes available
#define RETURN_IF_FAILED(hr) \
    { \
        HRESULT hr_val = (hr); /* ensure that expression is only evaluated once */ \
        if (FAILED(hr_val)) \
            return hr_val; \
    }

/** Converts unicode string to ASCII */
static inline std::string ToAscii (const std::wstring& w_str) {
#pragma warning(push)
#pragma warning(disable: 4996) // function or variable may be unsafe
    size_t N = w_str.size();
    std::string s_str;
    s_str.resize(N);

    wcstombs((char*)s_str.c_str(), w_str.c_str(), N);

    return s_str;
#pragma warning(pop)
}


/** Translate COM HRESULT failure into exceptions. */
static void CHECK (HRESULT hr) {
    if (FAILED(hr)) {
        _com_error err(hr);
#ifdef _UNICODE
        const wchar_t * msg = err.ErrorMessage(); // weak ptr.
        throw std::runtime_error(ToAscii(msg));
#else
        const char * msg = err.ErrorMessage(); // weak ptr.
        throw std::runtime_error(msg);
#endif
    }
}

/** Convenience function to create a locally implemented COM instance without the overhead of CoCreateInstance.
The COM class does not need to be registred for construction to succeed. However, lack of registration can
cause problems if transporting the class out-of-process. */
template <class T>
static CComPtr<T> CreateLocalInstance() {
    // create an object (with ref. count zero)
    CComObject<T>* tmp = nullptr;
    CHECK(CComObject<T>::CreateInstance(&tmp));

    // move into smart-ptr (will incr. ref. count to one)
    return CComPtr<T>(tmp);
}
