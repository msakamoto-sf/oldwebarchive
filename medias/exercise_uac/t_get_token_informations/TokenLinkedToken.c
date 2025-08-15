#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#pragma comment(lib, "advapi32.lib")

int print_privileges(HANDLE hToken)
{
    DWORD dwSize = 0, dwResult;
    PTOKEN_PRIVILEGES pPrivilegeInfo;
    int i;
    LUID luid;
    DWORD attr;
    LPTSTR lpName;

    // get buffer size
    if (!GetTokenInformation(hToken, TokenPrivileges, NULL, dwSize, &dwSize)) {
        dwResult = GetLastError();
        if (ERROR_INSUFFICIENT_BUFFER != dwResult) {
            fprintf(stderr, "GetTokenInformation(1) : %u\n", GetLastError());
            return 2;
        }
    }

    pPrivilegeInfo = (PTOKEN_PRIVILEGES)malloc(dwSize);
    if (NULL == pPrivilegeInfo) {
        fprintf(stderr, "malloc(pPrivilegeInfo) failed.\n");
        return 3;
    }

    // get data
    if (!GetTokenInformation(
                hToken, TokenPrivileges, pPrivilegeInfo, dwSize, &dwSize)) {
        fprintf(stderr, "GetTokenInformation(2) : %u\n", GetLastError());
        return 4;
    }

    for (i = 0; i < pPrivilegeInfo->PrivilegeCount; i++) {
        luid = pPrivilegeInfo->Privileges[i].Luid;
        attr = pPrivilegeInfo->Privileges[i].Attributes;
        printf("Privileges[%d]:\n", i);

        /* get name buffer size */
        dwSize = 0;
        if (!LookupPrivilegeName(
                    NULL, /* local system privilege name */
                    &luid, NULL, &dwSize)) {
            dwResult = GetLastError();
            if (ERROR_INSUFFICIENT_BUFFER != dwResult) {
                fprintf(stderr, "LookupPrivilegeName(1) : %u\n", GetLastError());
                return 5;
            }
        }
        lpName = (LPTSTR)malloc(dwSize + 1); /* plus NULL terminator */
        if (NULL == lpName) {
            fprintf(stderr, "malloc(lpName) failed\n");
            return 6;
        }
        /* get name */
        if (!LookupPrivilegeName(
                    NULL, /* local system privilege name */
                    &luid, lpName, &dwSize)) {
            dwResult = GetLastError();
            if (ERROR_INSUFFICIENT_BUFFER != dwResult) {
                fprintf(stderr, "LookupPrivilegeName(2) : %u\n", GetLastError());
                return 7;
            }
        }
        lpName[dwSize + 1] = '\0';
        printf("\tPrivilegeName = [%s]\n", lpName);
        free(lpName);

        printf("\tPrivilegeAttributes:\n");

        if (SE_PRIVILEGE_ENABLED && attr) {
            printf("\t\tSE_PRIVILEGE_ENABLED\n");
        }
        if (SE_PRIVILEGE_ENABLED_BY_DEFAULT && attr) {
            printf("\t\tSE_PRIVILEGE_ENABLED_BY_DEFAULT\n");
        }
        if (SE_PRIVILEGE_REMOVED && attr) {
            printf("\t\tSE_PRIVILEGE_REMOVED\n");
        }
        if (SE_PRIVILEGE_USED_FOR_ACCESS && attr) {
            printf("\t\tSE_PRIVILEGE_USED_FOR_ACCESS\n");
        }
    }

    free(pPrivilegeInfo);

    return 0;
}

int main(int argc, char *argv[])
{
    HANDLE hToken;
    TOKEN_LINKED_TOKEN hLinkedToken;
    DWORD dwSize = sizeof(TOKEN_LINKED_TOKEN);
    int r;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        fprintf(stderr, "OpenProcessToken() : %u\n", GetLastError());
        return 1;
    }

    printf("--------------- CURRENT TOKEN --------------------\n");
    if (0 != (r = print_privileges(hToken))) {
        return r;
    }

    if (!GetTokenInformation(
                hToken, TokenLinkedToken, &hLinkedToken, dwSize, &dwSize)) {
        fprintf(stderr, "GetTokenInformation() : %u\n", GetLastError());
        return -1;
    }

    printf("--------------- LINKED TOKEN --------------------\n");
    if (0 != (r = print_privileges(hLinkedToken.LinkedToken))) {
        return r;
    }

    CloseHandle(hLinkedToken.LinkedToken);

    CloseHandle(hToken);

    return 0;
}
