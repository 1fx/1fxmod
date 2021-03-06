/*
===========================================================================
Copyright (C) 2014 - 2020, Ane-Jouke Schat (Boe!Man)
Copyright (C) 2014 - 2020, Jordi Epema (Henkie)

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
// patch_main.c - Functions used by more than one memory patch.

//==================================================================

#include "patch_local.h"

/*
==================
AlignDWORD

Aligns DWORD value from ulong to C string.
==================
*/

char *AlignDWORD(long input)
{
    static char output[9];
    char tempStr[11];
    int n;

    #ifdef __GNUC__
    n = snprintf(tempStr, sizeof(tempStr), "%p", input);
    #elif _MSC_VER
    n = _snprintf_s(tempStr, sizeof(tempStr), "%p", input);
    #endif // __GNUC__

    output[0] = tempStr[n - 2];
    output[1] = tempStr[n - 1];
    output[2] = tempStr[n - 4];
    output[3] = tempStr[n - 3];
    output[4] = tempStr[n - 6];
    output[5] = tempStr[n - 5];
    output[6] = tempStr[n - 8];
    output[7] = tempStr[n - 7];
    output[8] = '\0';

    return output;
}

/*
==================
Patch_linuxPatchAddress

Logic to patch an address in another thread using ptrace (Linux).
==================
*/

#ifdef __linux__
#if (defined (__GNUC__) && __GNUC__ > 2)
static int *exitStatus;
#endif // GNUC > 2

void *Patch_linuxPatchAddress(void *arguments)
{
    char *modifiedData;
    int i, curSize;
    union u{
        long val;
        char chars[4];
    }data;
    patchArgs_t *args = (patchArgs_t *)arguments;

    #if (defined(__GNUC__) && __GNUC__ < 3)
    usleep(15000); // Boe!Man 3/8/15: Sleep for 15msec, this fixes a very strange problem of LinuxThreads sending a SIGSTOP while attaching.
    #else
    pid_t pID;

    // Create the shared memory map, so we can read the proper exit status of the patch.
    exitStatus = mmap(NULL, sizeof *exitStatus, PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *exitStatus = 1;

    // Fork the process.
    pID = fork();

    if(pID < 0){
        perror("fail!\nForking process failed");
    }else if(pID != 0){
        // Wait for the child.
        wait(NULL);

        // Take note of the proper success value, and unmap the shared memory.
        if(*exitStatus == 0){
            args->success = qtrue;
        }
        munmap(exitStatus, sizeof *exitStatus);

        return NULL;
    }
    #endif // GNUC < 3

    if (ptrace(PTRACE_ATTACH, args->pid, NULL, NULL) == -1){
        perror("fail!\nAttach failed");
        return NULL;
    }

    if (waitpid(args->pid, NULL, WUNTRACED) != args->pid){
        perror("fail!\nCouldn't wait for PID");
        return NULL;
    }

    modifiedData = args->buf;

    // Write in chunks of 4 bytes (WORD on i386).
    i = 0;
    while(i < args->length){
        curSize = (args->length - i > 4) ? 4 : args->length - i;

        // If the current size to write is not 4, we need to read existing data to make sure the buffer is entirely filled.
        if(curSize != 4){
            data.val = ptrace(PTRACE_PEEKDATA, args->pid, args->address + i, NULL);
            if(data.val == -1){
                perror("fail!\nReading original data failed");
                return NULL;
            }

            // Copy only the modified data and keep the rest intact.
            memcpy(data.chars, modifiedData, curSize);
        }else{
            memcpy(data.chars, modifiedData, 4);
        }

        if(ptrace(PTRACE_POKEDATA, args->pid, args->address + i, data.val) == -1){
            perror("fail!\nWriting modified data failed");
            return NULL;
        }

        i += 4;
    }

    if (ptrace(PTRACE_DETACH, args->pid, NULL, NULL) == -1){
        perror("fail!\nDetach failed");
        return NULL;
    }

    printf("done!\n");
    #if (defined (__GNUC__) && __GNUC__ > 2)
    *exitStatus = 0;
    _Exit(EXIT_SUCCESS);
    #else
    args->success = qtrue;
    #endif // GNUC > 2
}
#endif // __linux__

/*
==================
Patch_detourAddress

Main logic of detouring
to an address.
==================
*/

void Patch_detourAddress(char *genericName, long func, long offset, qboolean jmp)
{
#ifndef MACOS_X // FIXME!
    char patch[9];
    #ifdef _WIN32
    unsigned char buf[5];
    #elif defined(__linux__) || defined(MACOS_X)
    pthread_t thread;
    patchArgs_t args;

    unsigned char buf[4];
    #endif // _WIN32
    int i;
    int currentByte = 0;

    Com_Printf("Applying %s... ", genericName);
    #ifdef _WIN32
    if (jmp)
        sprintf(patch, va("E9%s", AlignDWORD(func - offset))); // JMP instead of CALL.
    else
        sprintf(patch, va("E8%s", AlignDWORD(func - offset))); // Regular CALL.
    #elif __linux__
    strncpy(patch, AlignDWORD(func - offset), sizeof(patch));
    #endif // _WIN32

    // Convert it to bytes.
    for(i = 0; i < strlen(patch); i += 2){
        char tempStr[3];
        tempStr[0] = patch[i];
        tempStr[1] = patch[i + 1];
        tempStr[2] = '\0';
        buf[currentByte] = (unsigned char)strtol(tempStr, NULL, 16);
        currentByte++;
    }

    #ifdef _WIN32
    // Patch and check if the patch was successful, if not, throw a fatal error.
    if(WriteProcessMemory(GetCurrentProcess(), (void *)(offset - 5), buf, 5, NULL) == 0){ // Write the new call.
        Com_Printf("fail!\n");
        #ifdef NDEBUG
        Com_Error(ERR_FATAL_NOLOG, "Patching the executable failed! Aborting the game.");
        #endif // NDEBUG
    }else{
        Com_Printf("done!\n");
    }
    #elif __linux__
    #if (defined(__GNUC__) && __GNUC__ > 2)
    // We don't need to get the thread return status on modern Linux systems, so ignore the signal so they don't turn defunct.
    signal(SIGCHLD, SIG_IGN);
    #endif // GNUC > 2

    // Fill args with essential stuff, like PID and addresses.
    args.pid = getpid();
    args.buf = (char *)&buf;
    args.address = offset - 4;
    args.length = 4;
    args.success = qfalse;

    // Create the thread that patches the address with the modified data.
    if (pthread_create(&thread, NULL, &Patch_linuxPatchAddress, (void *)&args) != 0){
        perror("fail!\nCreate thread failed");
    }else{
        // Wait for the thread to finish.
        pthread_join(thread, NULL);
    }

    #ifdef NDEBUG
    // Check if the patch was successful, and if not, throw a fatal error.
    if(!args.success){
        Com_Error(ERR_FATAL_NOLOG, "Patching the executable failed! Aborting the game.");
    }
    #endif // NDEBUG
    #endif // _WIN32
#endif // TEMP not MACOS_X
}

/*
==================
Patch_writeBytes

Main logic of writing
modified bytes to
an address.
==================
*/

void Patch_writeBytes(char *genericName, long offset, unsigned char *buf)
{
#ifndef MACOS_X // FIXME!
    #ifdef __linux__
    pthread_t thread;
    patchArgs_t args;
    #endif // __linux__

    Com_Printf("Applying %s... ", genericName);
    #ifdef _WIN32
    // Patch and check if the patch was successful, if not, throw a fatal error.
    if(WriteProcessMemory(GetCurrentProcess(), (void *)(offset - strlen(buf)), buf, strlen(buf), NULL) == 0){ // Write the modified data.
        Com_Printf("fail!\n");
        #ifdef NDEBUG
        Com_Error(ERR_FATAL_NOLOG, "Patching the executable failed! Aborting the game.");
        #endif // NDEBUG
    }else{
        Com_Printf("done!\n");
    }
    #elif __linux__
    #if (defined(__GNUC__) && __GNUC__ > 2)
    // We don't need to get the thread return status on modern Linux systems, so ignore the signal so they don't turn defunct.
    signal(SIGCHLD, SIG_IGN);
    #endif // GNUC > 2

    // Fill args with essential stuff, like PID and addresses.
    args.pid = getpid();
    args.buf = (char *)buf;
    args.length = strlen(buf);
    args.address = offset - args.length;
    args.success = qfalse;

    // Create the thread that patches the address with the modified data.
    if (pthread_create(&thread, NULL, &Patch_linuxPatchAddress, (void *)&args) != 0){
        perror("fail!\nCreate thread failed");
    }else{
        // Wait for the thread to finish.
        pthread_join(thread, NULL);
    }

    #ifdef NDEBUG
    // Check if the patch was successful, and if not, throw a fatal error.
    if(!args.success){
        Com_Error(ERR_FATAL_NOLOG, "Patching the executable failed! Aborting the game.");
    }
    #endif // NDEBUG
    #endif // _WIN32
#endif // TEMP not MACOS_X
}

/*
==================
Patch_Main

Main function called from SOF2, in order to apply all memory patches.
==================
*/

void Patch_Main()
{
    int i = trap_Milliseconds();

    Com_Printf("------------------------------------------\n");
    Com_Printf("Applying memory runtime modifications...\n");

    // Get rid of common exploits like q3infoboom.
    Patch_q3infofix();
    Patch_autoDownloadExploit();

    if(g_dosPatch.integer){
        Patch_dosProtection();
    }
    #ifdef _WIN32
    else{
        Patch_updateExploitProtection();
    }
    #endif // _WIN32

    #ifndef _GOLD
    // Remove the hash command from the v1.00 and v1.02t command list.
    Patch_removeCommand("hash");
    #else
    // Remove the banlist function from the v1.03 command list.
    Patch_removeCommand("banlist");
    #endif // not _GOLD

    Patch_rconLog();
    Patch_sysinfoWorkaround();
    Patch_engineCVARs();

    Com_Printf("All runtime memory modifications took %d ms.\n", trap_Milliseconds() - i);
}
