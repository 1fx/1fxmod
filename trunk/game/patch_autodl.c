// Copyright (C) 2014 - Boe!Man, Henkie.
//
// patch_autodl.c - "dirtrav" auto download exploit memory patch.

//==================================================================

#ifndef _GOLD
#include "patch_local.h"

// Local function definitions.
#ifdef _WIN32
void Patch_autoDownloadDetour0	(void);
#endif // _WIN32
void Patch_autoDownloadDetour	(char *message, int *clientNum, char *mapString);

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
Patch_autoDownloadDetour0

The detour for both GCC and VC++ on Windows.
The Linux detour skips this step (call directly to autoDownloadDetour).
==================
*/

#ifdef _WIN32
#ifdef __GNUC__
__asm__(".globl Patch_autoDownloadDetour0 \n\t"
	"_Patch_autoDownloadDetour0: \n"
	"call _Patch_autoDownloadDetour \n"

	"push $0x0046EF20 \n\t"
	"ret\n"
	);
#elif _MSC_VER
__declspec(naked) void Patch_autoDownloadDetour0()
{
	__asm {
		call	Patch_autoDownloadDetour;
		push	0x046EF20;
		retn
	}
}
#endif // __GNUC__
#endif // _WIN32

/*
==================
Patch_autoDownloadExploit

Writes the jump to the detour.
==================
*/

void Patch_autoDownloadExploit()
{
	#ifdef _WIN32
	Patch_detourAddress("Auto D/L exploit protection", (long)&Patch_autoDownloadDetour0, 0x46EF20);
	#elif __linux__
	Patch_detourAddress("Auto D/L exploit protection", (long)&Patch_autoDownloadDetour, 0x804b185);
	#endif // _WIN32
}

#endif // not _GOLD
