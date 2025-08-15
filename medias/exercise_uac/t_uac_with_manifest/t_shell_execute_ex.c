#include <windows.h>
#include <shellapi.h>
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "shell32.lib")

int main(int argc, char *argv[])
{
    SHELLEXECUTEINFO sei;
    ZeroMemory(&sei, sizeof(sei));

    sei.cbSize = sizeof(SHELLEXECUTEINFO);
    sei.hwnd = GetDesktopWindow();
    sei.fMask = SEE_MASK_FLAG_DDEWAIT | SEE_MASK_FLAG_NO_UI;
    sei.lpVerb = "runas";
    sei.lpFile = "check_elevation.exe";
    sei.lpParameters = "";
    sei.nShow = SW_SHOWNORMAL;

    if (!ShellExecuteEx(&sei)) {
        printf("ShellExecuteEx() failed : %u\n", GetLastError());
    }

    return 0;
}
