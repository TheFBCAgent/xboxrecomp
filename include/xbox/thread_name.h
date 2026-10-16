/*
 * xbox_name_thread - give a host thread a readable name (SetThreadDescription).
 *
 * Profilers and debuggers otherwise only see a thread's start address, and
 * every guest thread starts in the same bridge trampoline. The name shows in
 * Visual Studio, WinDbg, ETW/WPA, Process Explorer, and GetThreadDescription.
 *
 * Header-only so any runtime library (and the title's host code) can use it
 * without a link dependency. SetThreadDescription is looked up at run time:
 * it needs Windows 10 1607 (Wine has it too); without it this does nothing.
 */
#ifndef XBOX_THREAD_NAME_H
#define XBOX_THREAD_NAME_H

#ifdef _WIN32
#include <windows.h>
#include <stdarg.h>
#include <stdio.h>

typedef HRESULT(WINAPI *xbox_set_thread_desc_fn)(HANDLE, PCWSTR);

/* Names `thread` (NULL = the calling thread) with a printf-style format. */
static inline void xbox_name_thread(HANDLE thread, const char *fmt, ...)
{
    static xbox_set_thread_desc_fn set_desc;
    static volatile LONG looked_up;
    char name[64];
    WCHAR wname[64];
    va_list ap;

    if (!looked_up) {
        set_desc = (xbox_set_thread_desc_fn)(void (*)(void))GetProcAddress(
            GetModuleHandleA("kernel32.dll"), "SetThreadDescription");
        InterlockedExchange(&looked_up, 1);
    }
    if (!set_desc)
        return;
    va_start(ap, fmt);
    vsnprintf(name, sizeof(name), fmt, ap);
    va_end(ap);
    if (!MultiByteToWideChar(CP_UTF8, 0, name, -1, wname, 64))
        return;
    set_desc(thread ? thread : GetCurrentThread(), wname);
}
#else
/* POSIX hosts: not named yet (pthread_setname_np caps names at 16 bytes). */
static inline void xbox_name_thread(void *thread, const char *fmt, ...)
{
    (void)thread;
    (void)fmt;
}
#endif /* _WIN32 */

#endif /* XBOX_THREAD_NAME_H */
