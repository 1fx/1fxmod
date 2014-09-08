// Copyright (C) 2014 - Boe!Man, Henkie.
//
// patch_autodl.c - "dirtrav" auto download exploit memory patch (WIN32 only).

//==================================================================

#ifdef _WIN32
#include "patch_local.h"

// Local function definitions.
void Patch_autoDownloadDetour0(void);
void Patch_autoDownloadDetour(char *message, int *clientNum, char *mapString);

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

The detour for both GCC and VC++.
==================
*/

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

/*
==================
Patch_autoDownloadExploit

Writes the jump to the detour.
==================
*/

void Patch_autoDownloadExploit()
{
	Patch_detourAddress("Auto D/L exploit protection", (unsigned long)&Patch_autoDownloadDetour0, 0x46EF20);
}

#endif // _WIN32
