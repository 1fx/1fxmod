// Copyright (C) 2015 - Boe!Man, Henkie.
// Copyright (C) 2011 - ETpub team.
//
// g_crash.c - Crash log handler for development builds.

//==================================================================

#include "g_local.h"
#include "boe_local.h"

#ifdef _DEV
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

// Locals for this file.
int             segvloop        = 0;
int             numStackFrames  = 0;
fileHandle_t    f               = 0;

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
        crashLogger(va("Faulting Memory Ref/Instruction: %p\n",siginfo->si_addr));
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

#if __GNUC__ > 2 // Regular backtrace function for SoF2 v1.03.
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
#else // libunwind method for SoF2 v1.00.
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

    // Inform the user of what just happened.
    G_LogPrintf("\n-------------- FATAL ERROR ------------\n");
    G_LogPrintf("--        Server has crashed!        --\n");
    G_LogPrintf("---------------------------------------\n");
    G_LogPrintf("Now gathering information regarding the crash.\n");

    if(segvloop < 2){
        // Open the crash log.
        if(!openCrashLog()){
            G_LogPrintf("Unable to create crashdump (couldn't open file for writing).\n");
            G_LogPrintf("Writing to logfile or console instead.\n");
        }

        // Write header.
        crashLogger("------------- Crash Report ------------\n");
        crashLogger("--  Please e-mail to boe@1fxmod.org  --\n");
        crashLogger("---------------------------------------\n");
        crashLogger("1fx. Mod version: " INF_VERSION_STRING "\n");
        #ifdef _GOLD
        crashLogger("SoF2 version: v1.03\n");
        #else
        crashLogger("SoF2 version: v1.00\n");
        #endif // _GOLD
        crashLogger("OS: Linux\n");
        crashLogger(va("Map: %s\n",level.mapname)); // During init this may be empty.

        // Retrieve what signal we got.
        retrieveSigInfo(signal, siginfo);

        // Attempt a backtrace.
        doBacktrace(ctx);

        // Write footer and close the crash log.
        crashLogger("---------------------------------------\n");
        closeCrashLog();

        // Give a possible explanation if no stack frames could be retrieved.
        if(!numStackFrames){
            G_LogPrintf("\nERROR: No stack frames present to log.\n");
            G_LogPrintf("If you're on a 64-bit Linux operating system, ensure 32 bit libc6 and gcc support library packages are installed.\n");
            G_LogPrintf("On Debian, those are 'lib32gcc1' and 'libc6-i386'. They may vary for other Linux distributions.\n\n");
        }

        G_LogPrintf("Trying to gracefully shut down the server...\n\n");
        G_ShutdownGame(0);

        // Pass control to the default handler.
        if(signal == SIGSEGV) {
            oldHandler = (void *)oldact[SIGSEGV].sa_sigaction;
            (*oldHandler)(signal);
        }else{
            exit(1);
        }
    }else{
        // Recursive segfault detected.
        Com_Error(ERR_FATAL, "Recursive segmentation fault. Bailing out.");
        oldHandler = (void *)oldact[SIGSEGV].sa_sigaction;
        (*oldHandler)(signal);
    }
}
#endif // __linux__
#endif // _DEV
