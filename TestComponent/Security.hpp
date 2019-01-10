#pragma once
#include <sddl.h>


static void CheckErrorAndThrow(const char * error_msg) {
    DWORD err = GetLastError();

    if (err == ERROR_FILE_NOT_FOUND)
        throw std::runtime_error("ERROR_FILE_NOT_FOUND");
    else if (err == ERROR_NOT_ENOUGH_MEMORY)
        throw std::runtime_error("ERROR_NOT_ENOUGH_MEMORY");

    throw std::runtime_error(error_msg); // default message
}


/** Security attribute for granting all authenticated users read/write/execute access.
    Enables opening of an object created by a non-admin user in a different session. */
class SecurityEnableAllUsers {
public:
    SecurityEnableAllUsers() {
        // REF: https://docs.microsoft.com/nb-no/windows/desktop/SecAuthZ/security-descriptor-string-format
        const wchar_t sdd[] = L"D:"//DACL type
            L"(D;OICI;GA;;;BG)"    //Deny guests
            L"(D;OICI;GA;;;AN)"    //Deny anonymous
            L"(A;OICI;GRGWGX;;;AU)"//Allow read, write and execute for Users
            L"(A;OICI;GA;;;BA)";   //Allow all for Administrators

        if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(sdd, SDDL_REVISION_1, (PSECURITY_DESCRIPTOR*)&sa.lpSecurityDescriptor, NULL))
            CheckErrorAndThrow("ConvertStringSecurityDescriptorToSecurityDescriptor");
    }
    ~SecurityEnableAllUsers() {
        LocalFree(sa.lpSecurityDescriptor);
        sa.lpSecurityDescriptor = nullptr;
    }

    SECURITY_ATTRIBUTES* operator & () {
        return &sa;
    }

private:
    SECURITY_ATTRIBUTES sa = {};
};
