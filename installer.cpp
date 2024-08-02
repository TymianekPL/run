#include <Windows.h>
#include <ShlObj.h>
#include <ShellAPI.h>

void FormatLastError(const DWORD error, WCHAR* buffer, const size_t size)
{
     if (error == 0)
     {
          lstrcpyW(buffer, L"No error");
          return;
     }

     FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    nullptr, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, static_cast <DWORD>(size),
                    nullptr);
}

void ReportError(const WCHAR* message)
{
     WCHAR fullMessage[1024];
     WCHAR errorText[512];

     FormatLastError(GetLastError(), errorText, sizeof(errorText) / sizeof(errorText[0]));
     wsprintfW(fullMessage, L"%s\nError: %s", message, errorText);

     MessageBoxW(nullptr, fullMessage, L"Error", MB_OK | MB_ICONERROR);
     ExitProcess(1);
}

extern "C" void mainCRTStartup(void)
{
     WCHAR szDir[MAX_PATH]{}; // prompt (directory select)
     BROWSEINFO bi{};
     bi.lpszTitle = "Select a folder to save the executable:";
     LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
     if (pidl != nullptr)
     {
          if (SHGetPathFromIDListW(pidl, szDir))
          {
               const size_t len = lstrlenW(szDir);
               if (len > 0 && szDir[len - 1] == L'\\') szDir[len - 1] = L'\0'; // remove trailing slashes
          }
          CoTaskMemFree(pidl);
     }
     else
     {
          ReportError(L"Failed to select a directory.");
          ExitProcess(1);
     }

     // Copy run.exe to the selected location
     WCHAR szDest[MAX_PATH] = {0};
     wsprintfW(szDest, L"%s\\run.exe", szDir);
     if (!CopyFileW(L"run.exe", szDest, FALSE))
     {
          ReportError(L"Failed to copy the executable.");
          ExitProcess(1);
     }

     // -- registry part
     HKEY hKey;

     // HKCU\run
     LONG lRes = RegCreateKeyExW(HKEY_CLASSES_ROOT, L"run", 0, nullptr, REG_OPTION_NON_VOLATILE,
                                 KEY_SET_VALUE,
                                 nullptr, &hKey, nullptr);
     if (lRes != ERROR_SUCCESS)
     {
          ReportError(L"Failed to create registry key HKCR\\run");
          ExitProcess(1);
     }

     // HKCU\run\URL Protocol
     WCHAR szURLProtocolValue[] = L"";
     lRes = RegSetValueExW(hKey, L"URL Protocol", 0, REG_SZ, reinterpret_cast <const BYTE*>(szURLProtocolValue),
                           sizeof(szURLProtocolValue));
     if (lRes != ERROR_SUCCESS)
     {
          SetLastError(lRes);
          ReportError(L"Failed to set registry value HKCR\\run\\URL Protocol");
          ExitProcess(GetLastError());
     }
     RegCloseKey(hKey);

     // HKCU\run\Shell
     lRes = RegCreateKeyExW(HKEY_CLASSES_ROOT, L"run\\Shell", 0, nullptr, REG_OPTION_NON_VOLATILE,
                            KEY_SET_VALUE, nullptr, &hKey, nullptr);
     if (lRes != ERROR_SUCCESS)
     {
          SetLastError(lRes);
          ReportError(LR"(Failed to create registry key HKCR\run\Shell)");
          ExitProcess(1);
     }
     RegCloseKey(hKey);

     // HKCU\run\Shell\Open
     lRes = RegCreateKeyExW(HKEY_CLASSES_ROOT, L"run\\Shell\\Open", 0, nullptr, REG_OPTION_NON_VOLATILE,
                            KEY_SET_VALUE, nullptr, &hKey, nullptr);
     if (lRes != ERROR_SUCCESS)
     {
          SetLastError(lRes);
          ReportError(LR"(Failed to create registry key HKCR\run\Shell\Open)");
          ExitProcess(1);
     }
     RegCloseKey(hKey);

     // HKCU\run\Shell\Open\command
     lRes = RegCreateKeyExW(HKEY_CLASSES_ROOT, L"run\\Shell\\Open\\command", 0, nullptr,
                            REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, nullptr, &hKey, nullptr);
     if (lRes != ERROR_SUCCESS)
     {
          SetLastError(lRes);
          ReportError(LR"(Failed to create registry key HKCR\run\Shell\Open\command)");
          ExitProcess(1);
     }

     // Set the default value for HKCU\run\Shell\Open\command
     WCHAR szCommand[MAX_PATH + 20]{};
     wsprintfW(szCommand, LR"("%s" "%%1")", szDest);
     lRes = RegSetValueExW(hKey, nullptr, 0, REG_SZ, reinterpret_cast <const BYTE*>(szCommand),
                           (lstrlenW(szCommand) + 1) * sizeof(WCHAR));
     if (lRes != ERROR_SUCCESS)
     {
          SetLastError(lRes);
          ReportError(LR"(Failed to set registry value HKCR\run\Shell\Open\command)");
          ExitProcess(1);
     }
     RegCloseKey(hKey);
     // -- end registry part

     MessageBoxW(nullptr, L"Successfully installed.", L"Success", MB_OK);
     ExitProcess(0);
}
