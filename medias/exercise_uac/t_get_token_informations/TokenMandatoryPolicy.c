#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#pragma comment(lib, "advapi32.lib")

int main(int argc, char *argv[])
{
    HANDLE hToken;
    DWORD dwSize;
    TOKEN_MANDATORY_POLICY mp;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        fprintf(stderr, "OpenProcessToken() : %u\n", GetLastError());
        return 1;
    }

    dwSize = sizeof(TOKEN_MANDATORY_POLICY);

    if (!GetTokenInformation(
                hToken, TokenMandatoryPolicy, &mp, dwSize, &dwSize)) {
        fprintf(stderr, "GetTokenInformation() : %u\n", GetLastError());
        return 2;
    }

    printf("TOKEN_MANDATORY_POLICY = %d, ", mp.Policy);
    switch (mp.Policy) {
        case TOKEN_MANDATORY_POLICY_OFF:
            printf("TOKEN_MANDATORY_POLICY_OFF"); break;
        case TOKEN_MANDATORY_POLICY_NO_WRITE_UP:
            printf("TOKEN_MANDATORY_POLICY_NO_WRITE_UP"); break;
        case TOKEN_MANDATORY_POLICY_NEW_PROCESS_MIN:
            printf("TOKEN_MANDATORY_POLICY_NEW_PROCESS_MIN"); break;
        case TOKEN_MANDATORY_POLICY_VALID_MASK:
            printf("TOKEN_MANDATORY_POLICY_VALID_MASK"); break;
        default:
            printf("unknown");
    }
    printf("\n");

    return 0;
}
