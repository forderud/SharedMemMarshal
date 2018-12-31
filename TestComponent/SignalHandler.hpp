#pragma once
#include <cassert>
#include <string>
#include <Windows.h>


/** Support class for signaling destruction events from "proxy" objects to the associated server. */
class SignalHandler {
    static const unsigned int HEX_ENC_LEN = 8; ///< number of characters required to encode a uint32_t in hexadecimal representation
public:
    SignalHandler (std::string name) : m_name(name) {
        // reserve space for hex-encoded uint32_t
        m_name.resize(m_name.size() + HEX_ENC_LEN);
    }

    ~SignalHandler () {
        if (m_wait) {
            // unregister callback
            if (!UnregisterWaitEx(m_wait, NULL)) {
                DWORD err = GetLastError();
                // disregard ERROR_IO_PENDING errors
                if (err != ERROR_IO_PENDING)
                    abort();
            }
            m_wait = 0;
        }

        if (m_event) {
            CloseHandle(m_event);
            m_event = 0;
        }
    }

#ifdef _WIN32
    /** Create event object that is named based on "val" to keep it unique. Called in server. */
    void Create (uint32_t val, IUnknown * ptr) {
        if (!m_event) {
            m_event = CreateEventEx(NULL/*security*/, EventName(val), 0/*flags*/, SYNCHRONIZE);
            assert(m_event);

            assert(!m_wait);
            if (!RegisterWaitForSingleObject(&m_wait, m_event, SignalHandler::SignalCB, this, INFINITE, WT_EXECUTEDEFAULT)) {
                DWORD err = GetLastError();
                err;
                abort();
            }
            assert(m_wait);
        }

        // call AddRef to keep "ptr" alive while the proxy lives
        if (!m_ref) {
            m_ref = ptr;
        } else {
            assert(m_ref == ptr);
            m_ref.p->AddRef();
            m_extra_refs++;
        }
    }

    /** Open event object associated with "val". Called in proxy. */
    void Open (uint32_t val) {
        assert(!m_event);
        m_event = OpenEvent(EVENT_MODIFY_STATE, FALSE, EventName(val));
        assert(m_event);
    }

    /** Send "release" event back to server, now that the object is deserialized.
        Based loosely on https://thrysoee.dk/InsideCOM+/ch14c.htm */
    void Signal () {
        assert(m_event);

        if (!SetEvent(m_event)) {
            DWORD err = GetLastError();
            err;
            abort();
        }
    }
#endif

private:
#ifdef _WIN32
    /** Called in server after proxy have called SetEvent. */
    static void CALLBACK SignalCB (void* pv, BOOLEAN /*timedOut*/) {
        SignalHandler * obj = reinterpret_cast<SignalHandler*>(pv);
        assert(obj);
        assert(obj->m_ref);

        // call Release on "ptr"
        // do this last, since it might trigger deletion of "obj"
        if (obj->m_extra_refs > 0) {
            obj->m_extra_refs--;
            obj->m_ref.p->Release();
        } else {
            obj->m_ref = nullptr;
        }
    }

    /** Generate a unique name based on "val". */
    const char* EventName (uint32_t val) {
        // using sprintf instead of std::to_string to avoid dynamic memory mgmt.
#pragma warning(push)
#pragma warning(disable: 4996) // function or variable may be unsafe
        sprintf(const_cast<char*>(m_name.data()) + m_name.size()-HEX_ENC_LEN, "%I32X", val); // append hex-encoded uint32_t and null-termination
#pragma warning(pop)
        return m_name.c_str();
    }

    HANDLE            m_event = 0;
    HANDLE            m_wait  = 0; ///< wait callback handle (used by server)
    CComPtr<IUnknown> m_ref;       ///< extra ref-count (used by server to indicate ref-count held by proxy)
    int               m_extra_refs = 0;
#endif
    std::string m_name;
};
