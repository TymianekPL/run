#define _CRT_SECURE_NO_WARNINGS // wcstombs is "unsafe" lol
#include <Windows.h>

// Allocations
void* __cdecl operator new[](size_t size) { return HeapAlloc(GetProcessHeap(), 0, size); }
void __cdecl operator delete[](void* ptr) noexcept { HeapFree(GetProcessHeap(), 0, ptr); }

size_t __cdecl wcstombs(
     _Out_writes_opt_(_MaxCount) char* _Dest,
     _In_z_ wchar_t const* _Source,
     _In_ size_t _MaxCount
)
{
     int i = 0;
     while (i < _MaxCount - 1 && _Source[i] != L'\0')
     {
          _Dest[i] = static_cast <char>(_Source[i]);
          i++;
     }
     _Dest[i] = '\0';
     return i;
}

extern "C" void mainCRTStartup(void)
{
     int main(int argc, char* argv[]);

     int argc;
     wchar_t** argvW = CommandLineToArgvW(GetCommandLineW(), &argc);

     char** argv = new char*[argc];
     for (int i = 0; i < argc; ++i)
     {
          const size_t len = lstrlenW(argvW[i]) + 1;
          argv[i] = new char[len];
          wcstombs(argv[i], argvW[i], len);
     }

     const int result = main(argc, argv);

     LocalFree(argv);

     ExitProcess(result);
}

int main(const int argc, char* argv[])
{
     if (argc != 2) return -1;

     char* arg = argv[1];

     char* colonPos = arg;
     while (*colonPos && *colonPos != ':')
          ++colonPos;

     if (!*colonPos) return -1;

     char* uri = colonPos + 1;

     for (char* end = uri + lstrlenA(uri) - 1; end >= uri && *end == '/'; --end) *end = '\0';

     while (*uri == '/') uri++;

     STARTUPINFO si{};
     PROCESS_INFORMATION pi{};
     si.cb = sizeof(si);
     const BOOL success = CreateProcessA(
          nullptr, uri, nullptr, nullptr, FALSE,
          NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP,
          nullptr, nullptr, &si, &pi
     );

     if (!success) return static_cast <int>(GetLastError());

     WaitForSingleObject(pi.hProcess, INFINITE); // wait for process.


     DWORD code{};
     GetExitCodeProcess(pi.hProcess, &code);

     CloseHandle(pi.hProcess);
     CloseHandle(pi.hThread);

     return static_cast <int>(code);
}
