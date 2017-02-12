// Copyright (C) 2014 - Boe!Man, Henkie.
//
// patch_local.h - Memory patch definitions are stored here.

//==================================================================

#ifndef _PATCH_LOCAL
#define _PATCH_LOCAL

#include "g_local.h"
#include "boe_local.h"
#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <sys/ptrace.h>
#include <unistd.h>

#if (defined(__GNUC__) && __GNUC__ < 3)
#include <linux/user.h>
#else
#ifndef __USE_MISC
#define __USE_MISC 1
#endif // not __USE_MISC

#include <sys/mman.h>
#endif // GNUC < 3

// Boe!Man 1/11/15: Required information for memory patching in another thread.
typedef struct patchArgs {
    pid_t       pid;        // PID of main thread/process.
    char        *buf;       // Bytedata of the modified call (variable length).
    long        address;    // Address to patch.
    int         length;     // Length of the byte data (buf).
    qboolean    success;    // True if the patch was successful.
}patchArgs_t;
#endif // _WIN32

// Function definitions.
char    *AlignDWORD                 ( long input );
void    Patch_detourAddress         ( char *genericName, long func, long offset, qboolean jmp );
void    Patch_writeBytes            ( char *genericName, long offset, unsigned char *buf );

void    Patch_autoDownloadExploit   ( void );
void    Patch_dosProtection         ( void );
void    Patch_rconLog               ( void );
void    Patch_sysinfoWorkaround     ( void );
void    Patch_q3infofix             ( void );
void    Patch_engineCVARs           ( void );
void    Patch_removeCommand         ( const char *cmd );

#endif // _PATCH_LOCAL
