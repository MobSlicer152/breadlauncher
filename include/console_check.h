#ifdef _WIN32
#include <conio.h>
#include <string.h>
#include <windows.h>
#include <TlHelp32.h>
#include <psapi.h>

#undef max
#define getch_ _getch
#define strcasecmp _stricmp
#endif

#ifndef getch_
#define getch_(...)
#endif

/* On Windows, this checks if the parent process is console based or not */
bool check_parent_is_console(void);
