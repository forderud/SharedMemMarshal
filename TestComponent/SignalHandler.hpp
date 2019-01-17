#pragma once
#include <cassert>
#include <mutex>
#include <string>
#include <Windows.h>
#include "Security.hpp"



/** Support class for signaling destruction events from "proxy" objects to the associated server. */
class SignalHandler {
    static const unsigned int HEX_ENC_LEN = 16; ///< number of characters required to encode a uint64_t in hexadecimal representation
public:
    SignalHandler (std::string name) : m_name(name) {
        // reserve space for hex-encoded uint64_t
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
    /** Create event object that is named based on "val" to keep it unique.
        Called in server for each MarshalInterface, which might occur concurrent. Therefore, must be thread-safe. */
    void Create (uint64_t val, IUnknown * ptr) {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (!m_event) {
#ifdef ENABLE_GLOBAL_COMMUNICATION
            SecurityEnableAllUsers acl;
            SECURITY_ATTRIBUTES * security = &acl;
#else
            SECURITY_ATTRIBUTES * security = nullptr;
#endif
            m_event = CreateEventEx(security, EventName(val), 0/*flags*/, SYNCHRONIZE);
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
    void Open (uint64_t val) {
        assert(!m_event);
        m_event = OpenEvent(EVENT_MODIFY_STATE, FALSE, EventName(val));
        assert(m_event);
    }

    /** Send "Release" event back to server. Called in client when proxy object is destructed.
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
    /** Called in server by OS thread-pool after proxy have called SetEvent.
        Must be thread-safe, since multiple proxies might signal concurrently. */
    static void CALLBACK SignalCB (void* pv, BOOLEAN /*timedOut*/) {
        SignalHandler * obj = reinterpret_cast<SignalHandler*>(pv);
        assert(obj);

        // call Release on "ptr"
        obj->ReleaseReference();
    }

    /** Thread-safe proxy ref-count decrement. */
    void ReleaseReference () {
        assert(m_ref);
        bool last_release = false;

        {
            std::lock_guard<std::mutex> lock(m_mutex);

            if (m_extra_refs > 0) {
                // release one reference
                m_extra_refs--;
                m_ref.p->Release();
            } else {
                last_release = true;
            }
        }

        if (last_release) {
            // release last reference
            // might trigger deletion, so it cannot be done while holding a lock
            m_ref = nullptr;
        }
    }

    /** Generate a unique name based on "val". */
    const char* EventName (uint64_t val) {
        // using sprintf instead of std::to_string to avoid dynamic memory mgmt.
#pragma warning(push)
#pragma warning(disable: 4996) // function or variable may be unsafe
        sprintf(const_cast<char*>(m_name.data()) + m_name.size()-HEX_ENC_LEN, "%I64X", val); // append hex-encoded uint64_t and null-termination
#pragma warning(pop)
        return m_name.c_str();
    }

    HANDLE            m_event = 0;
    HANDLE            m_wait  = 0; ///< wait callback handle (used by server)

    CComPtr<IUnknown> m_ref;       ///< extra ref-count (used by server to indicate ref-count held by proxy)
    int               m_extra_refs = 0;
    std::mutex        m_mutex;  ///< protect m_ref & m_extra_refs
#endif
    std::string m_name;
};
