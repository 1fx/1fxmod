// Copyright (C) 2015 - Boe!Man, Henkie.
//
// patch_addFSGame.c - Direct call to the engine, so the fake fs_game
// is being exposed to the client's in-game downloader.

// Only used when g_enforce1fxAdditions is off and the new fs_game
// is not the same as the real fs_game.

//==================================================================

#include "patch_local.h"

// Addresses hard-defined for the FS_AddGameDirectory routine in the engine.
#ifdef _WIN32
#ifdef _GOLD
#define FS_ADDGAMEDIR 0x00451630
#else
#define FS_ADDGAMEDIR 0x00450420
#endif // _GOLD
#endif // _WIN32

#ifdef __linux__
#ifdef _GOLD
#define FS_ADDGAMEDIR 0x080889b4
#else
#define FS_ADDGAMEDIR "$0x08084808"
#endif // _GOLD
#endif // __linux__

#if !(defined(__GNUC__) && __GNUC__ < 3)
typedef void *FS_AddGameDir_Engine(const char *path, const char *dir);
#endif // not __GNUC__ < 3

/*
==================
FS_AddGameDirectory
11/13/15 - 11:34 AM
Calls the real FS_AddGameDirectory
in the engine.
==================
*/

static void FS_AddGameDirectory(const char *path, const char *dir)
{
    #if defined(__linux__) && (defined(__GNUC__) && __GNUC__ < 3)
    // Boe!Man 11/13/15: Of course, gcc 2.95 tries to optimize a direct call
    // resulting in a segmentation fault with the code for the other platforms.
    // Woody, :(..
    __asm__("push %0 \n\t"
            "push %1 \n\t"
            "movl " FS_ADDGAMEDIR ", %%eax \n\t"
            "call *%%eax \n"
            "popl %%eax \n\t"
            :
            : "a" (dir), "c" (path));
    #else
    (*(FS_AddGameDir_Engine *)FS_ADDGAMEDIR)(path, dir);
    #endif // __linux __ && __GNUC__ < 3
}

/*
==================
Patch_addAdditionalFSGame
11/13/15 - 9:29 AM
Adds another search path to the game directories.
This is used for the in-game downloader.
==================
*/

void Patch_addAdditionalFSGame(char *searchpath)
{
    char tempCvarValue[MAX_CVAR_VALUE_STRING];

    trap_Cvar_VariableStringBuffer("fs_cdpath", tempCvarValue, sizeof(tempCvarValue));
    if(tempCvarValue[0]){
        FS_AddGameDirectory(tempCvarValue, searchpath);
    }
    trap_Cvar_VariableStringBuffer("fs_basepath", tempCvarValue, sizeof(tempCvarValue));
    if(tempCvarValue[0]){
        FS_AddGameDirectory(tempCvarValue, searchpath);
    }
    trap_Cvar_VariableStringBuffer("fs_homepath", tempCvarValue, sizeof(tempCvarValue));
    if(tempCvarValue[0]){
        FS_AddGameDirectory(tempCvarValue, searchpath);
    }
}
