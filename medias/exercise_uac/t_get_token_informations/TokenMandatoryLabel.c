#include <windows.h>
#include <sddl.h>
#include <stdio.h>
#include <stdlib.h>
#pragma comment(lib, "advapi32.lib")

int main(int argc, char *argv[])
{
    HANDLE hToken;
    DWORD dwSize = 0, dwResult;
    TOKEN_MANDATORY_LABEL ml;
    LPTSTR strSid = NULL;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        fprintf(stderr, "OpenProcessToken() : %u\n", GetLastError());
        return 1;
    }

    // get buffer size
    if (!GetTokenInformation(hToken, TokenIntegrityLevel, NULL, dwSize, &dwSize)) {
        dwResult = GetLastError();
        if (ERROR_INSUFFICIENT_BUFFER != dwResult) {
            fprintf(stderr, "GetTokenInformation(1) : %u\n", GetLastError());
            return 2;
        }
    }

    // get data
    if (!GetTokenInformation(
                hToken, TokenIntegrityLevel, &ml, dwSize, &dwSize)) {
        fprintf(stderr, "GetTokenInformation(2) : %u\n", GetLastError());
        return 3;
    }

    if (!ConvertSidToStringSid(ml.Label.Sid, &strSid)) {
        fprintf(stderr, "ConvertSidToStringSid() : %u\n", GetLastError());
        return 4;
    }

    printf("MandatoryLabel : SID = %s\n", strSid);
    printf("\tAttribute = %08X\n", ml.Label.Attributes);

    LocalFree(strSid);

    return 0;
}
