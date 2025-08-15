#define UNICODE
#define _UNICODE

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#pragma comment(lib, "advapi32.lib")

static BOOL GetLinkedToken(HANDLE *hLinkedToken)
{
    HANDLE hToken;
    TOKEN_LINKED_TOKEN hTokenLinkedToken;
    DWORD dwSize = sizeof(TOKEN_LINKED_TOKEN);
    int r;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY , &hToken)) {
        _ftprintf(stderr, _T("OpenProcessToken() : %u\n"), GetLastError());
        return FALSE;
    }

    if (!GetTokenInformation(
                hToken, TokenLinkedToken, &hTokenLinkedToken, dwSize, &dwSize)) {
        _ftprintf(stderr, _T("GetTokenInformation() : %u\n"), GetLastError());
        return FALSE;
    }

    *hLinkedToken = hTokenLinkedToken.LinkedToken;

    CloseHandle(hToken);

    return TRUE;
}

void _tmain( int argc, _TCHAR *argv[] )
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    DWORD dwLastError;
    HANDLE hToken = NULL;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    if( argc != 2 )
    {
        _tprintf(_T("Usage: %s [cmdline]\n"), argv[0]);
        return;
    }

    if (!GetLinkedToken(&hToken)) {
        return;
    }

    // Start the child process. 
    if( !CreateProcessWithTokenW(
                hToken,
                0,          // Logon Flags
                NULL,       // No module name (use command line)
                argv[1],    // Command line
                0,          // No creation flags
                NULL,       // Use parent's environment block
                NULL,       // Use parent's starting directory 
                &si,        // Pointer to STARTUPINFO structure
                &pi )       // Pointer to PROCESS_INFORMATION structure
      ) 
    {
        dwLastError = GetLastError();
        _tprintf(_T("CreateProcess failed (%d).\n"), dwLastError);
        if (ERROR_ELEVATION_REQUIRED == dwLastError) {
            _tprintf(_T("\trequires elevation.\n"));
        }
        return;
    }

    CloseHandle(hToken);

    // Wait until child process exits.
    WaitForSingleObject( pi.hProcess, INFINITE );

    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
}
