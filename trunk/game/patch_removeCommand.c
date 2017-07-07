// Copyright (C) 2015 - Boe!Man, Henkie.
//
// patch_removeCommand.c - Direct call to the engine, which
// removes a command from the engine command functions.

//==================================================================

#include "patch_local.h"

// Addresses hard-defined for the Cmd_RemoveCommand routine in the engine.
#ifdef _WIN32
#ifdef _GOLD
#define CMD_REMOVECOMMAND 0x00445b80
#elif _DEMO
#define CMD_REMOVECOMMAND 0x00443700
#else
#define CMD_REMOVECOMMAND 0x00444cb0
#endif // _GOLD, _DEMO or Full
#endif // _WIN32

#ifdef __linux__
#ifdef _GOLD
#define CMD_REMOVECOMMAND 0x0807f084
#else
#define CMD_REMOVECOMMAND "$0x0807b088"
#endif // _GOLD
#endif // __linux__

#if !(defined(__GNUC__) && __GNUC__ < 3)
typedef void *Cmd_RemoveCommand(const char *cmd_name);
#endif // not __GNUC__ < 3

/*
==================
Patch_removeCommand
11/15/15 - 12:15 PM
Removes a command from the
engine command functions.
==================
*/

void Patch_removeCommand(const char *cmd)
{
    Com_Printf("Applying %s removal patch... ", cmd);
    #if defined(__linux__) && (defined(__GNUC__) && __GNUC__ < 3)
    // Boe!Man 11/13/15: Of course, gcc 2.95 tries to optimize a direct call
    // resulting in a segmentation fault with the code for the other platforms.
    // Woody, :(..
    __asm__("push %0 \n\t"
            "movl " CMD_REMOVECOMMAND ", %%eax \n\t"
            "call *%%eax \n"
            "popl %%eax \n\t"
            :
            : "a" (cmd));
    #else
    (*(Cmd_RemoveCommand *)CMD_REMOVECOMMAND)(cmd);
    #endif // __linux __ && __GNUC__ < 3

    Com_Printf("done!\n");
}
