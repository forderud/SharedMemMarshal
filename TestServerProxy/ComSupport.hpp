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
