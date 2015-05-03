// Copyright (C) 2015 - Boe!Man, Henkie.
//
// patch_fsgame.c - Memory patch that changes the fs_game CVAR in the CS_SYSTEMINFO string
// so clients think the Mod is running under a different fs_game CVAR.

//==================================================================

#include "patch_local.h"

// Local function definitions.
void Patch_fsgame	(char *s, const char *key, const char *value);

/*
==================
Patch_fsgame

The actual code of where we change the fs_game CVAR to the appropriate value.
==================
*/

void Patch_fsgame(char *s, const char *key, const char *value)
{
	if (strcmp(key, "fs_game") == 0 && level.clientMod != CL_NONE){
		#ifdef _GOLD
		if (level.clientMod == CL_ROCMOD){
			Info_SetValueForKey_Big(s, key, "rocmod");
		}
		#else
		if (level.clientMod == CL_RPM){
			Info_SetValueForKey_Big(s, key, "RPM");
		}
		#endif // _GOLD
	}else{
		Info_SetValueForKey_Big(s, key, value);
	}
}

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
	Patch_detourAddress("client fs_game CVAR workaround", (long)&Patch_fsgame, 0x0044C7ED, qfalse);
	#else
	Patch_detourAddress("client fs_game CVAR workaround", (long)&Patch_fsgame, 0x0044B74D, qfalse);
	#endif // _GOLD
	#endif // _WIN32
	
	#ifdef __linux__
	#ifdef _GOLD
	Patch_detourAddress("client fs_game CVAR workaround", (long)&Patch_fsgame, 0x08084a07, qfalse);
	#else
	Patch_detourAddress("client fs_game CVAR workaround", (long)&Patch_fsgame, 0x0808087b, qfalse);
	#endif // _GOLD
	#endif // __linux__
}
