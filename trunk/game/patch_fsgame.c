// Copyright (C) 2015 - Boe!Man, Henkie.
//
// patch_fsgame.c - Memory patch that changes the fs_game CVAR in the CS_SYSTEMINFO string
// so clients think the Mod is running under a different fs_game CVAR.

//==================================================================

#include "patch_local.h"

// Local function definitions.
#ifdef _WIN32
void Patch_fsgame0					(void);
#endif // _WIN32
static void Patch_fsgame			(char *s, const char *key, const char *value);

/*
==================
Patch_autoDownloadDetour

The actual code of where we check what file the client tries to download.
==================
*/

static void Patch_fsgame(char *s, const char *key, const char *value)
{
	if (strcmp(key, "fs_game") == 0){
		#ifdef _GOLD
		Info_SetValueForKey_Big(s, key, "rocmod");
		#else
		Info_SetValueForKey_Big(s, key, "RPM");
		#endif // FIXME
	}else{
		Info_SetValueForKey_Big(s, key, value);
	}
}

/*
==================
Patch_fsgame

The detour for both GCC and VC++ on Windows.
The Linux detour skips this step (call directly to fsgame).
==================
*/

#ifdef _WIN32
#ifdef __GNUC__
__asm__(".globl Patch_fsgame \n\t"
	"_Patch_fsgame: \n"
	"call _Patch_fsgame_frame \n"

	"push $0x0046EF20 \n\t"
	"ret\n"
	);
#elif _MSC_VER
#ifdef _GOLD
#define ADDRESS	 0x0044C7ED
#else
#define ADDRESS  0x0044B74D
#endif // _GOLD

__declspec(naked) void Patch_fsgame0()
{
	__asm {
		call	Patch_fsgame;
		push	ADDRESS;
		retn;
	}
}
#endif // __GNUC__

#undef ADDRESS
#endif // _WIN32

/*
==================
Patch_fsgameWorkaround

Writes the jump to the detour.
==================
*/

void Patch_fsgameWorkaround()
{
	#ifdef _WIN32
	#ifdef _GOLD
	Patch_detourAddress("client fs_game CVAR workaround", (long)&Patch_fsgame0, 0x0044C7ED);
	#else
	Patch_detourAddress("client fs_game CVAR workaround", (long)&Patch_fsgame0, 0x0044B74D);
	#endif // _GOLD
	#endif // _WIN32
	
	#ifdef __linux__
	#ifdef _GOLD
	Patch_detourAddress("client fs_game CVAR workaround", (long)&Patch_fsgame, 0x08084a07);
	#else
	Patch_detourAddress("client fs_game CVAR workaround", (long)&Patch_fsgame, 0x0808087b);
	#endif // _GOLD
	#endif // __linux__
}
