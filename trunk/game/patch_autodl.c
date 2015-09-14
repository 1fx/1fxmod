// Copyright (C) 2014 - Boe!Man, Henkie.
//
// patch_autodl.c - "dirtrav" auto download exploit memory patch.

//==================================================================

#ifndef _GOLD
#include "patch_local.h"

// Local function definitions.
void Patch_autoDownloadDetour   (char *message, int *clientNum, char *mapString);

/*
==================
Patch_autoDownloadDetour

The actual code of where we check what file the client tries to download.
==================
*/

void Patch_autoDownloadDetour(char *message, int *clientNum, char *mapString)
{
    if (!strstr(mapString, ".pk3")){
        G_LogPrintf("Auto D/L protection: ^1HACK ATTEMPT: ^7client %d tries to download \"%s\"\n", clientNum, mapString);
        *mapString = '\0';
    }
    #ifdef _DEBUG
    else{
        Com_Printf("Auto D/L: Allowed client %d to download \"%s\"\n", clientNum, mapString);
    }
    #endif
}

/*
==================
Patch_autoDownloadExploit

Writes the jump to the detour.
==================
*/

void Patch_autoDownloadExploit()
{
    #ifdef _WIN32
    Patch_detourAddress("Auto D/L exploit protection", (long)&Patch_autoDownloadDetour, 0x0046EF20, qfalse);
    #elif __linux__
    Patch_detourAddress("Auto D/L exploit protection", (long)&Patch_autoDownloadDetour, 0x0804b185, qfalse);
    #endif // _WIN32
}

#endif // not _GOLD
