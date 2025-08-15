#define UNICODE
#define _UNICODE

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "advapi32.lib")

BOOL RunAsDesktopUser(
        __in    const _TCHAR *szApp,
        __in    _TCHAR *szCmdLine,
        __in    const _TCHAR *szCurrDir,
        __in    STARTUPINFOW *si,
        __inout PROCESS_INFORMATION *pi)
{
    HANDLE hShellProcess = NULL;
    HANDLE hShellProcessToken = NULL;
    HANDLE hPrimaryToken = NULL;
    HWND hwnd = NULL;
    DWORD dwPID = 0;
    BOOL ret;
    DWORD dwLastErr;
    BOOL retval = FALSE;
    HANDLE hProcessToken = NULL;
    TOKEN_PRIVILEGES tkp;
    DWORD dwTokenRights = TOKEN_QUERY | TOKEN_ASSIGN_PRIMARY | TOKEN_DUPLICATE | TOKEN_ADJUST_DEFAULT | TOKEN_ADJUST_SESSIONID;

    // Enable SeIncreaseQuotaPrivilege in this process.
    // This won't work if current process is not elevated.)
    if (!OpenProcessToken(
                GetCurrentProcess(), 
                TOKEN_ADJUST_PRIVILEGES, 
                &hProcessToken))
    {
        dwLastErr = GetLastError();
        _ftprintf(stderr, _T("OpenProcessToken() failed: %u"), dwLastErr);
        return FALSE;
    }

    tkp.PrivilegeCount = 1;
    LookupPrivilegeValueW(NULL, SE_INCREASE_QUOTA_NAME, &tkp.Privileges[0].Luid);
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    AdjustTokenPrivileges(hProcessToken, FALSE, &tkp, 0, NULL, NULL);
    dwLastErr = GetLastError();
    CloseHandle(hProcessToken);
    if (ERROR_SUCCESS != dwLastErr) {
        _ftprintf(stderr, _T("AdjustTokenPrivileges() failed: %u"), dwLastErr);
        return FALSE;
    }

    // Get an HWND representing the desktop shell.
    // CAVEATS:  This will fail if the shell is not running 
    // (crashed or terminated), or the default shell has been
    // replaced with a custom shell.  
    // This also won't return what you probably want 
    // if Explorer has been terminated and restarted elevated.
    hwnd = GetShellWindow();
    if (NULL == hwnd) {
        _ftprintf(stderr, _T("No desktop shell is present"));
        return FALSE;
    }

    // Get the PID of the desktop shell process.
    GetWindowThreadProcessId(hwnd, &dwPID);
    if (0 == dwPID) {
        _ftprintf(stderr, _T("Unable to get PID of desktop shell."));
        return FALSE;
    }

    // Open the desktop shell process in order to query it (get the token)
    hShellProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwPID);
    if (!hShellProcess) {
        dwLastErr = GetLastError();
        _ftprintf(stderr, _T("Can't open desktop shell process: %u"), dwLastErr);
        return FALSE;
    }

    // From this point down, we have handles to close, so make sure to clean up.

    // Get the process token of the desktop shell.
    ret = OpenProcessToken(hShellProcess, TOKEN_DUPLICATE, &hShellProcessToken);
    if (!ret) {
        dwLastErr = GetLastError();
        _ftprintf(stderr, _T("Can't get process token of desktop shell: %u"), dwLastErr);
        return FALSE;
    }

    // Duplicate the shell's process token to get a primary token.
    // Based on experimentation, this is the minimal set of rights required 
    // for CreateProcessWithTokenW (contrary to current documentation).
    ret = DuplicateTokenEx(
            hShellProcessToken, dwTokenRights, NULL, 
            SecurityImpersonation, TokenPrimary, &hPrimaryToken);
    if (!ret) {
        dwLastErr = GetLastError();
        _ftprintf(stderr, _T("Can't get primary token: %u"), dwLastErr);
        goto cleanup;
    }

    // Start the target process with the new token.
    ret = CreateProcessWithTokenW(
            hPrimaryToken,
            0,
            szApp,
            szCmdLine,
            0,
            NULL,
            szCurrDir,
            si,
            pi);
    if (!ret)
    {
        dwLastErr = GetLastError();
        _ftprintf(stderr, _T("CreateProcessWithTokenW() failed: %u"), dwLastErr);
        goto cleanup;
    }

    retval = TRUE;

cleanup:
    // Clean up resources
    CloseHandle(hShellProcessToken);
    CloseHandle(hPrimaryToken);
    CloseHandle(hShellProcess);
    return retval;
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

    if (!RunAsDesktopUser(
                NULL,
                argv[1],
                NULL,
                &si,
                &pi)) {
        _ftprintf(stderr, _T("RunAsDesktopUser() failed!\n"));
        return;
    }

    // Wait until child process exits.
    WaitForSingleObject( pi.hProcess, INFINITE );

    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
}
