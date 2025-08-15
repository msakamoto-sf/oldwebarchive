#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#pragma comment(lib, "advapi32.lib")

int main(int argc, char *argv[])
{
    HANDLE hToken;
    DWORD dwSize = 0, dwResult;
    PTOKEN_ELEVATION_TYPE pElevationType;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        fprintf(stderr, "OpenProcessToken() : %u\n", GetLastError());
        return 1;
    }

    // get buffer size
    if (!GetTokenInformation(hToken, TokenElevationType, NULL, dwSize, &dwSize)) {
        dwResult = GetLastError();
        if (ERROR_INSUFFICIENT_BUFFER != dwResult) {
            fprintf(stderr, "GetTokenInformation(1) : %u\n", GetLastError());
            return 2;
        }
    }

    pElevationType = (PTOKEN_ELEVATION_TYPE)malloc(dwSize);
    if (NULL == pElevationType) {
        fprintf(stderr, "malloc(pElevationType) failed.\n");
        return 3;
    }

    // get data
    if (!GetTokenInformation(
                hToken, TokenElevationType, pElevationType, dwSize, &dwSize)) {
        fprintf(stderr, "GetTokenInformation(2) : %u\n", GetLastError());
        return 4;
    }

    printf("TOKEN_ELEVATION_TYPE = %d, ", *pElevationType);
    switch (*pElevationType) {
        case TokenElevationTypeDefault:
            printf("TokenElevationTypeDefault");
            break;
        case TokenElevationTypeFull:
            printf("TokenElevationTypeFull");
            break;
        case TokenElevationTypeLimited:
            printf("TokenElevationTypeLimited");
            break;
        default:
            printf("unknown");
    }
    printf("\n");

    free(pElevationType);

    return 0;
}
