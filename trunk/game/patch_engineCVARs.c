// Copyright (C) 2015 - Boe!Man, Henkie.
//
// patch_engineCVARs.c - Modifies several important CVARs in the engine
// their properties.

//==================================================================

#include "patch_local.h"

// Addresses hard-defined for the CVARs to patch.
#ifdef _WIN32
#ifdef _GOLD
#define FLOODPROTECT_ADDRESS 0x00ba3d38
#define ALLOWDOWNLOAD_ADDRESS 0x00bfdd8c
#else
#define FLOODPROTECT_ADDRESS 0x00b7a974
#define ALLOWDOWNLOAD_ADDRESS 0x00bd39c4
#endif // _GOLD
#endif // _WIN32

#ifdef __linux__
#ifdef _GOLD
#define FLOODPROTECT_ADDRESS 0x082be93c
#define ALLOWDOWNLOAD_ADDRESS 0x0830d53c
#else
#define FLOODPROTECT_ADDRESS 0x08314d14
#define ALLOWDOWNLOAD_ADDRESS 0x08363900
#endif // _GOLD
#endif // __linux__

/*
==================
Patch_engineCVARs

Writes the modified CVAR attributes
to the specified memory address.
==================
*/

void Patch_engineCVARs()
{
    Com_Printf("Patching engine CVAR attributes...\n");

    // Modify sv_floodProtect with read only attribute.
    (*(cvar_t **)FLOODPROTECT_ADDRESS)->flags |= CVAR_ROM;
    Com_Printf("%-4ssv_floodProtect to read only.\n", " ");

    // Modify sv_allowDownload with read only attribute,
    // if 1fx. Client Additions are enforced.
    // Otherwise we reset it to read/write.
    if(g_enforce1fxAdditions.integer){
        (*(cvar_t **)ALLOWDOWNLOAD_ADDRESS)->flags |= CVAR_ROM;
        Com_Printf("%-4ssv_allowDownload to read only.\n", " ");
    }else{
        (*(cvar_t **)ALLOWDOWNLOAD_ADDRESS)->flags &= ~CVAR_ROM;
        Com_Printf("%-4ssv_allowDownload to read/write.\n", " ");
    }
}
