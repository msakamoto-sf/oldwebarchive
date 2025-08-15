#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#pragma comment(lib, "advapi32.lib")

int main(int argc, char *argv[])
{
    HANDLE hToken;
    DWORD dwSize;
    TOKEN_ELEVATION elevation;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        fprintf(stderr, "OpenProcessToken() : %u\n", GetLastError());
        return 1;
    }

    dwSize = sizeof(TOKEN_ELEVATION);

    if (!GetTokenInformation(
                hToken, TokenElevation, &elevation, dwSize, &dwSize)) {
        fprintf(stderr, "GetTokenInformation() : %u\n", GetLastError());
        return 2;
    }

    printf("TOKEN_ELEVATION = %d\n", elevation.TokenIsElevated);

    printf("Hit Return:");
    getchar();

    return 0;
}
