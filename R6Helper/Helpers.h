#include <windows.h>
#include <TlHelp32.h>

namespace Helpers {
    bool CheckProc(const wchar_t* szExe)
    {
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(PROCESSENTRY32);
        Process32First(hSnapshot, &pe);

        if (!_wcsicmp((wchar_t*)&pe.szExeFile, szExe))
        {
            return true;
        }

        while (Process32Next(hSnapshot, &pe))
        {
            if (!_wcsicmp((wchar_t*)&pe.szExeFile, szExe))
            {
                return true;
            }
        }

        return false;
    }
}