/*
===========================================================================
Copyright (C) 2011, ETpub team
Copyright (C) 2015 - 2020, Ane-Jouke Schat (Boe!Man)
Copyright (C) 2015 - 2020, Jordi Epema (Henkie)

This file is part of the 1fx. Mod source code.

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/
// g_crash.c - Crash log handler for development builds.

//==================================================================

#include "g_local.h"
#include "boe_local.h"

#ifdef _DEV

// Local variables for both Windows and Linux.
fileHandle_t    f               = 0;
int             numStackFrames  = 0;
int             segvloop        = 0; // Never written to from WIN32, but is used in determining the filename.

/*
==========================
crashLogger
10/11/15 - 6:27 PM
Logs a string to the crash log,
or log prints it if no file is
currently open.
==========================
*/

void crashLogger(char *s)
{
    if(f){
        trap_FS_Write(s, strlen(s), f);
    }else{
       G_LogPrintf(s);
    }
}

/*
==========================
openCrashLog
10/11/15 - 6:19 PM
Opens a new crash log for writing.
==========================
*/

qboolean openCrashLog()
{
    // Determine current time.
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    // Determine filename.
    char *filename = va("logs/crashdumps/%d%02d%02d%02d%02d%02d-%d.txt", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, segvloop);

    // Open the file.
    trap_FS_FOpenFile(filename, &f, FS_WRITE);
    G_LogPrintf("Writing crash log file: %s\n", filename);

    // Return success.
    return f;
}

/*
==========================
closeCrashLog
10/11/15 - 6:20 PM
Closes a previously opened crash log
if one was opened to begin with.
==========================
*/

void closeCrashLog()
{
    if(f){
        trap_FS_FCloseFile(f);
        G_LogPrintf("Closed crash log file.\n");
        G_LogPrintf("Please submit the created crash log to the 1fx. Mod developers\n");
        G_LogPrintf("by e-mailing the generated file to boe@1fxmod.org.\n\n");
    }
}

/*
==========================
logStart
10/13/15 - 6:42 PM
Inform the user the server
has crashed, log the header
to the crash log and write
the generics to file.
==========================
*/

void logStart()
{
    // Inform the user of what just happened.
    G_LogPrintf("\n-------------- FATAL ERROR ------------\n");
    G_LogPrintf("--        Server has crashed!        --\n");
    G_LogPrintf("---------------------------------------\n");
    G_LogPrintf("Now gathering information regarding the crash.\n");

    // Open the crash log.
    if(!openCrashLog()){
        G_LogPrintf("Unable to create crashdump (couldn't open file for writing).\n");
        G_LogPrintf("Writing to logfile or console instead.\n");
    }

    // Write header.
    crashLogger("------------- Crash Report ------------\n");
    crashLogger("--  Please e-mail to boe@1fxmod.org  --\n");
    crashLogger("---------------------------------------\n");
    crashLogger("1fx. Mod version: " MODFULL "\n");

    // Determine SoF2 version.
    #ifdef _GOLD
    crashLogger("SoF2 version: v1.03\n");
    #else
    crashLogger("SoF2 version: v1.00\n");
    #endif // _GOLD

    // Log Operating System.
    #ifdef _WIN32
    crashLogger("OS: WIN32\n");
    #else
    crashLogger("OS: Linux\n");
    #endif // _WIN32

    // See if we can log the map played.
    // During init this may be empty.
    crashLogger(va("Map: %s\n",level.mapname));

    // Try to log gametype if it is set.
    crashLogger("Gametype: ");
    if(current_gametype.value == GT_NONE){
        crashLogger("Unknown\n");
    }else if(current_gametype.value == GT_HS){
        crashLogger("Hide&Seek\n");
    }else if(current_gametype.value == GT_INF){
        crashLogger("Infiltration\n");
    }else if(current_gametype.value == GT_CTF){
        crashLogger("Capture the Flag\n");
    }else if(current_gametype.value == GT_DM){
        crashLogger("Deathmatch\n");
    }else if(current_gametype.value == GT_TDM){
        crashLogger("Team Deathmatch\n");
    }else if(current_gametype.value == GT_ELIM){
        crashLogger("Elimination\n");
    }else if(current_gametype.value == GT_HZ){
        crashLogger("Humans&Zombies\n");
    }

    // Is competition mode enabled?
    if(g_compMode.integer && cm_enabled.integer){
        crashLogger(va("Comp mode is enabled and in state: %d\n", cm_enabled.integer));
    }
}

/*
==========================
logStop
10/13/15 - 6:44 PM
Write footer and try to
safely shutdown the server.
==========================
*/

void logStop()
{
    // Write footer and close the crash log.
    crashLogger("---------------------------------------\n");
    closeCrashLog();

    // Give a possible explanation if no stack frames could be retrieved.
    #ifdef __linux__
    if(!numStackFrames){
        G_LogPrintf("\nERROR: No stack frames present to log.\n");
        G_LogPrintf("If you're on a 64-bit Linux operating system, ensure 32 bit libc6 and gcc support library packages are installed.\n");
        G_LogPrintf("On Debian, those are 'lib32gcc1' and 'libc6-i386'. They may vary for other Linux distributions.\n\n");
    }
    #endif // __linux__

    G_LogPrintf("Trying to gracefully shut down the server...\n\n");

    #ifdef __linux__
    // Change stdin to non blocking, so this terminal keeps working properly.
    fcntl(0, F_SETFL, fcntl (0, F_GETFL, 0) & ~FNDELAY);
    #endif // __linux__

    // Shutdown the game module.
    G_ShutdownGame(0);
}

//==================================================================

#ifdef __linux__

#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <execinfo.h>
#include <link.h>
#include <sys/ucontext.h>
#include <features.h>

#if __GNUC__ < 3
#include <libunwind.h>
#endif // __GNUC__ < 3

extern char *strsignal (int __sig) __THROW;

// Function declarations.
void crashHandler           (int signal, siginfo_t *siginfo, ucontext_t *ctx);
void (*oldHandler)          (int signal);
struct sigaction oldact[32];

/*
==========================
enableCrashHandler
-
Main entry for this file,
enables the crash handler.

Original code from ETpub.
==========================
*/

void enableCrashHandler()
{

    struct sigaction act;

    memset(&act, 0, sizeof(act));
    memset(&oldact, 0, sizeof(oldact));
    act.sa_sigaction = (void *)crashHandler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;

    sigaction(SIGSEGV, &act, &oldact[SIGSEGV]);
    sigaction(SIGILL, &act, &oldact[SIGILL]);
    sigaction(SIGFPE, &act, &oldact[SIGFPE]);
    sigaction(SIGBUS, &act, &oldact[SIGBUS]);
    sigaction(SIGUSR2, &act, &oldact[SIGUSR2]);
}

/*
==========================
disableCrashHandler
-
Disables the crash handler.

Original code from ETpub.
==========================
*/

void disableCrashHandler()
{
    sigaction(SIGSEGV, &oldact[SIGSEGV], NULL);
}

/*
==========================
retrieveSigInfo
-
Retrieve and write signal and
additional signal info to file.

Original code from ETpub.
==========================
*/

void retrieveSigInfo(int signal, siginfo_t *siginfo)
{
    crashLogger(va("Signal: %s (%d)\n", strsignal(signal), signal));
    crashLogger(va("Siginfo: %p\n", siginfo));
    if(siginfo){
        crashLogger(va("Code: %d\n", siginfo->si_code));
        crashLogger(va("Faulting Memory Ref/Instruction: %p\n",
            siginfo->si_addr));
    }
    if(signal == SIGUSR2){
        char crashReason[1024];
        trap_Cvar_VariableStringBuffer("com_errorMessage",
            crashReason, sizeof(crashReason));

        crashLogger(va("Soft crash detected. Error message was: %s\n",
            crashReason));
    }
}

/*
==========================
doBacktrace
-
Do a backtrace and retrieve as
much information as possible.

Original code from ETpub.
==========================
*/

#if __GNUC__ > 2
// Regular backtrace function for SoF2 v1.03.
void doBacktrace(ucontext_t *ctx)
{

    // See <asm/sigcontext.h>

    // ctx.eip contains the actual value of eip
    // when the signal was generated.

    // ctx.cr2 contains the value of the cr2 register
    // when the signal was generated.

    // the cr2 register on i386 contains the address
    // that caused the page fault if there was one.

    int     i;
    char    **strings;
    void    *stack[32];

    // Globally save the number of stack frames retrieved.
    numStackFrames = backtrace(stack, 32);

    // Only start logging the backtrace if the previous call
    // actually retrieved stack frames to process.
    if(numStackFrames){
        // Set the actual calling address for accurate stack traces.
        // If we don't do this stack traces are less accurate.
        crashLogger(va("Stack frames: %d entries\n", numStackFrames-1));
        stack[1] = (void *)ctx->uc_mcontext.gregs[REG_EIP];

        crashLogger("Backtrace:\n");
        strings = backtrace_symbols(stack, numStackFrames); // Retrieve symbols.

        // Start at one and climb up.
        // The first entry points back to this function.
        for (i = 1; i < numStackFrames; i++){
            crashLogger(va("(%i) %s\n", i, strings[i]));
        }

        free(strings);
    }
}
#else
// libunwind method for SoF2 v1.00.
void doBacktrace(ucontext_t *ctx)
{
    unw_cursor_t    cursor;
    unw_context_t   context;

    unw_getcontext(&context);
    unw_init_local(&cursor, &context);

    // We can safely skip the first frame, so use this as a check.
    if(unw_step(&cursor) <= 0){
        return;
    }

    crashLogger("Backtrace:\n");
    // Loop through the rest.
    while (unw_step(&cursor) > 0)
    {
        unw_word_t  offset, pc;
        char        fname[64];

        unw_get_reg(&cursor, UNW_REG_IP, &pc);

        fname[0] = '\0';
        (void) unw_get_proc_name(&cursor, fname, sizeof(fname), &offset);

        crashLogger (va("(%d) %p: (%s+0x%x) [%p]\n", ++numStackFrames, pc, fname, offset, pc));
    }

    // Finish, also log the number of stack frames processed.
    crashLogger(va("Stack frames: %d entries\n", numStackFrames));
}
#endif // __GNUC__ > 2

/*
==========================
crashHandler
-
This function handles the
exception thrown.

Original code from ETpub.
==========================
*/

void crashHandler(int signal, siginfo_t *siginfo, ucontext_t *ctx)
{
    // We are real cautious here.
    disableCrashHandler();

    // Increment this counter to ensure we can bail out from recursive segfaults.
    if(signal == SIGSEGV){
        segvloop++;
    }

    if(segvloop < 2){
        // Open the crash log and log the generic information
        // such as header, operating system and such.
        logStart();

        // Retrieve what signal we got.
        retrieveSigInfo(signal, siginfo);

        // Attempt a backtrace.
        doBacktrace(ctx);

        // Log the footer and exit.
        logStop();

        // Pass control to the default handler.
        if(signal == SIGSEGV) {
            oldHandler = (void *)oldact[SIGSEGV].sa_sigaction;
            (*oldHandler)(signal);
        }else{
            exit(1);
        }
    }else{
        // Recursive segfault detected.
        Com_Error(ERR_FATAL_NOLOG, "Recursive segmentation fault. " \
            "Bailing out.");
        oldHandler = (void *)oldact[SIGSEGV].sa_sigaction;
        (*oldHandler)(signal);
    }
}

#elif defined _WIN32

#include <windows.h>
#include <process.h>
#include <imagehlp.h>

// Assign types.
typedef BOOL (WINAPI *PFNSYMINITIALIZE)(HANDLE, LPSTR, BOOL);
typedef BOOL (WINAPI *PFNSYMCLEANUP)(HANDLE);
typedef PGET_MODULE_BASE_ROUTINE PFNSYMGETMODULEBASE;
typedef BOOL (WINAPI *PFNSTACKWALK)(DWORD, HANDLE, HANDLE, LPSTACKFRAME, LPVOID, PREAD_PROCESS_MEMORY_ROUTINE, PFUNCTION_TABLE_ACCESS_ROUTINE, PGET_MODULE_BASE_ROUTINE, PTRANSLATE_ADDRESS_ROUTINE);
typedef BOOL (WINAPI *PFNSYMGETSYMFROMADDR)(HANDLE, DWORD, LPDWORD, PIMAGEHLP_SYMBOL);
typedef BOOL (WINAPI *PFNSYMENUMERATEMODULES)(HANDLE, PSYM_ENUMMODULES_CALLBACK, PVOID);
typedef PFUNCTION_TABLE_ACCESS_ROUTINE PFNSYMFUNCTIONTABLEACCESS;

// Function declarations.
LONG CALLBACK   crashHandler    (LPEXCEPTION_POINTERS e);

// Local variables.
HMODULE                     imagehlp                    = NULL;
PFNSYMINITIALIZE            pfnSymInitialize            = NULL;
PFNSYMCLEANUP               pfnSymCleanup               = NULL;
PFNSYMGETMODULEBASE         pfnSymGetModuleBase         = NULL;
PFNSTACKWALK                pfnStackWalk                = NULL;
PFNSYMGETSYMFROMADDR        pfnSymGetSymFromAddr        = NULL;
PFNSYMENUMERATEMODULES      pfnSymEnumerateModules      = NULL;
PFNSYMFUNCTIONTABLEACCESS   pfnSymFunctionTableAccess   = NULL;

/*
==========================
enableCrashHandler
-
Main entry for this file,
enables the crash handler.

Original code from ETpub.
==========================
*/

void enableCrashHandler()
{
    // Load the imagehlp library which we need for crash logging.
    imagehlp = LoadLibrary("IMAGEHLP.DLL");
    if(!imagehlp){
        G_LogPrintf("WARNING: imagehlp.dll unavailable for crash logging.\n");
        return;
    }

    // Check symbols.
    pfnSymInitialize = (PFNSYMINITIALIZE) GetProcAddress(imagehlp, "SymInitialize");
    pfnSymCleanup = (PFNSYMCLEANUP) GetProcAddress(imagehlp, "SymCleanup");
    pfnSymGetModuleBase = (PFNSYMGETMODULEBASE) GetProcAddress(imagehlp, "SymGetModuleBase");
    pfnStackWalk = (PFNSTACKWALK) GetProcAddress(imagehlp, "StackWalk");
    pfnSymGetSymFromAddr = (PFNSYMGETSYMFROMADDR) GetProcAddress(imagehlp, "SymGetSymFromAddr");
    pfnSymEnumerateModules = (PFNSYMENUMERATEMODULES) GetProcAddress(imagehlp, "SymEnumerateModules");
    pfnSymFunctionTableAccess = (PFNSYMFUNCTIONTABLEACCESS) GetProcAddress(imagehlp, "SymFunctionTableAccess");

    // Verify if the DLL is correctly loaded.
    if(
        !pfnSymInitialize ||
        !pfnSymCleanup ||
        !pfnSymGetModuleBase ||
        !pfnStackWalk ||
        !pfnSymGetSymFromAddr ||
        !pfnSymEnumerateModules ||
        !pfnSymFunctionTableAccess
    ){
        FreeLibrary(imagehlp);
        G_LogPrintf("WARNING: imagehlp.dll is missing exports, and is thus unavailble for crash logging.\n");
        imagehlp = NULL;
        return;
    }

    // It is now safe to install the exception handler.
    SetUnhandledExceptionFilter(crashHandler);
}

/*
==========================
disableCrashHandler
-
Disables the crash handler.

Original code from ETpub.
==========================
*/

void disableCrashHandler()
{
    SetUnhandledExceptionFilter(NULL);
    pfnSymInitialize = NULL;
    pfnSymCleanup = NULL;
    pfnSymGetModuleBase = NULL;
    pfnStackWalk = NULL;
    pfnSymGetSymFromAddr = NULL;
    pfnSymEnumerateModules = NULL;
    pfnSymFunctionTableAccess = NULL;
    FreeLibrary(imagehlp);
}

/*
==========================
ExceptionName
-
Returns exception name.

Original code from ETpub.
==========================
*/

char *ExceptionName(DWORD exceptioncode)
{
    switch (exceptioncode){
        case EXCEPTION_ACCESS_VIOLATION: return "Access violation"; break;
        case EXCEPTION_DATATYPE_MISALIGNMENT: return "Datatype misalignment"; break;
        case EXCEPTION_BREAKPOINT: return "Breakpoint"; break;
        case EXCEPTION_SINGLE_STEP: return "Single step"; break;
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return "Array bounds exceeded"; break;
        case EXCEPTION_FLT_DENORMAL_OPERAND: return "Float denormal operand"; break;
        case EXCEPTION_FLT_DIVIDE_BY_ZERO: return "Float divide by zero"; break;
        case EXCEPTION_FLT_INEXACT_RESULT: return "Float inexact result"; break;
        case EXCEPTION_FLT_INVALID_OPERATION: return "Float invalid operation"; break;
        case EXCEPTION_FLT_OVERFLOW: return "Float overflow"; break;
        case EXCEPTION_FLT_STACK_CHECK: return "Float stack check"; break;
        case EXCEPTION_FLT_UNDERFLOW: return "Float underflow"; break;
        case EXCEPTION_INT_DIVIDE_BY_ZERO: return "Integer divide by zero"; break;
        case EXCEPTION_INT_OVERFLOW: return "Integer overflow"; break;
        case EXCEPTION_PRIV_INSTRUCTION: return "Privileged instruction"; break;
        case EXCEPTION_IN_PAGE_ERROR: return "In page error"; break;
        case EXCEPTION_ILLEGAL_INSTRUCTION: return "Illegal instruction"; break;
        case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "Noncontinuable exception"; break;
        case EXCEPTION_STACK_OVERFLOW: return "Stack overflow"; break;
        case EXCEPTION_INVALID_DISPOSITION: return "Invalid disposition"; break;
        case EXCEPTION_GUARD_PAGE: return "Guard page"; break;
        case EXCEPTION_INVALID_HANDLE: return "Invalid handle"; break;
        default: break;
    }

    return "Unknown exception";
}

/*
==========================
retrieveExceptionInfo
-
Retrieve and write exception and
additional exception address to file.

Original code from ETpub.
==========================
*/

void retrieveExceptionInfo(LPEXCEPTION_POINTERS e)
{
    char crashReason[1024];

    // Log the exception thrown.
    crashLogger(va("Exception: %s (0x%08x)\n", ExceptionName(e->ExceptionRecord->ExceptionCode), e->ExceptionRecord->ExceptionCode));
    crashLogger(va("Exception address: 0x%08x\n", e->ExceptionRecord->ExceptionAddress));

    // Is it a soft crash? If yes, print the error.
    trap_Cvar_VariableStringBuffer("com_errorMessage",
        crashReason, sizeof(crashReason));

    if(strlen(crashReason)){
        crashLogger(va("Soft crash detected. Error message was: %s\n",
        crashReason));
    }
}

/*
==========================
doBacktrace
-
Do a backtrace and retrieve as
much information as possible.

Original code from ETpub.
==========================
*/

void doBacktrace(LPEXCEPTION_POINTERS e)
{
    PIMAGEHLP_SYMBOL    pSym;
    STACKFRAME          sf;
    HANDLE              process, thread;
    DWORD               dwModBase, Disp;
    BOOL                more = FALSE;
    char                moduleName[MAX_PATH] = "";

    // Allocate some memory for the symbols.
    pSym = (PIMAGEHLP_SYMBOL)GlobalAlloc(GMEM_FIXED, 16384);

    // Initialize memory and relevant variables.
    ZeroMemory(&sf, sizeof(sf));
    sf.AddrPC.Offset = e->ContextRecord->Eip;
    sf.AddrStack.Offset = e->ContextRecord->Esp;
    sf.AddrFrame.Offset = e->ContextRecord->Ebp;
    sf.AddrPC.Mode = AddrModeFlat;
    sf.AddrStack.Mode = AddrModeFlat;
    sf.AddrFrame.Mode = AddrModeFlat;

    process = GetCurrentProcess();
    thread = GetCurrentThread();

    crashLogger("Backtrace:\n");

    // Loop through the stack trace.
    while(1){
        more = pfnStackWalk(
            IMAGE_FILE_MACHINE_I386,
            process,
            thread,
            &sf,
            e->ContextRecord,
            NULL,
            pfnSymFunctionTableAccess,
            pfnSymGetModuleBase,
            NULL
        );

        // No more frames available.
        if(!more || sf.AddrFrame.Offset == 0){
            break;
        }

        dwModBase = pfnSymGetModuleBase(process, sf.AddrPC.Offset);

        if(dwModBase){
            GetModuleFileName((HINSTANCE)dwModBase, moduleName, MAX_PATH);
        }else{
            wsprintf(moduleName, "Unknown");
        }

        pSym->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
        pSym->MaxNameLength = MAX_PATH;

        // Log as much information as possible.
        if(pfnSymGetSymFromAddr(process, sf.AddrPC.Offset, &Disp, pSym)){
            crashLogger(va("(%d) %s(%s+%#0x) [0x%08x]\n", ++numStackFrames, moduleName, pSym->Name, Disp, sf.AddrPC.Offset));
        }else{
            crashLogger(va("(%d) %s [0x%08x]\n", ++numStackFrames, moduleName, sf.AddrPC.Offset));
        }
    }

    // Finish, also log the number of stack frames processed.
    crashLogger(va("Stack frames: %d entries\n", numStackFrames));

    // Free allocated memory.
    GlobalFree(pSym);
}

/*
==========================
crashHandler
-
This function handles the
exception thrown.

Original code from ETpub.
==========================
*/

LONG CALLBACK crashHandler(LPEXCEPTION_POINTERS e)
{
    char basepath[MAX_PATH];
    char gamepath[MAX_PATH];

    // Try to load the symbols.
    trap_Cvar_VariableStringBuffer("fs_basepath", basepath, sizeof(basepath));
    trap_Cvar_VariableStringBuffer("fs_game", gamepath, sizeof(gamepath));
    pfnSymInitialize(GetCurrentProcess(), va("%s\\%s", basepath, gamepath), TRUE);

    // Open the crash log and log the generic information
    // such as header, operating system and such.
    logStart();

    // Log if imagehlp is unavailable on the host system.
    if(imagehlp == NULL){
        crashLogger("imagehlp.dll is missing exports or is unavailable on system.\n");
    }

    // Retrieve what exception we got.
    retrieveExceptionInfo(e);

    // Attempt a backtrace.
    doBacktrace(e);

    // Log the footer and exit.
    logStop();

    // Cleanup symbols.
    pfnSymCleanup(GetCurrentProcess());

    return 1;
}

#endif // __linux__
#endif // _DEV
