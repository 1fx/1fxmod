// Copyright (C) 2015 - Boe!Man, Henkie.
//
// patch_sysinfo.c - Memory patch that can change CVARs in the CS_SYSTEMINFO string
// so clients think the Mod is running under different circumstances.

//==================================================================

#include "patch_local.h"

// Local function definitions.
void Patch_systemInfo   (char *s, const char *key, const char *value);

/*
==================
Patch_systemInfo

The actual code of where we change CVARs to their appropriate value.
==================
*/

void Patch_systemInfo(char *s, const char *key, const char *value)
{
    if (strcmp(key, "fs_game") == 0){
        if(level.clientMod == CL_NONE){
            Info_SetValueForKey_Big(s, key, g_enforce1fxAdditions.integer ? "1fx.additions" : "1fx");
        }
        #ifdef _GOLD
        else if (level.clientMod == CL_ROCMOD){
            Info_SetValueForKey_Big(s, key, g_enforce1fxAdditions.integer ? "1fx.rocmod" : "rocmod");
        }
        #else
        else if (level.clientMod == CL_RPM){
            Info_SetValueForKey_Big(s, key, g_enforce1fxAdditions.integer ? "1fx.RPM" : "RPM");
        }
        #endif // _GOLD
    #ifdef _GOLD
    }else if(g_enforce1fxAdditions.integer && strcmp(key, "sv_referencedPakNames") == 0){
        if(level.clientMod == CL_NONE){
            Info_SetValueForKey_Big(s, key, COREUI_DEFAULT_PAKNAME);
        }else if(level.clientMod == CL_ROCMOD){
            Info_SetValueForKey_Big(s, key, COREUI_ROCMOD_PAKNAME);
        }
    }else if(g_enforce1fxAdditions.integer && strcmp(key, "sv_referencedPaks") == 0){
        if(level.clientMod == CL_NONE){
            Info_SetValueForKey_Big(s, key, va("%d ", COREUI_DEFAULT_PAKNUM));
        }else if(level.clientMod == CL_ROCMOD){
            Info_SetValueForKey_Big(s, key, va("%d ", COREUI_ROCMOD_PAKNUM));
        }
    #endif // _GOLD
    }else{
        Info_SetValueForKey_Big(s, key, value);
    }
}

/*
==================
Patch_sysinfoWorkaround

Writes the jump to the detour.
==================
*/

void Patch_sysinfoWorkaround()
{
    #ifdef _WIN32
    #ifdef _GOLD
    Patch_detourAddress("client system info CVAR workaround", (long)&Patch_systemInfo, 0x0044C7ED, qfalse);
    #else
    Patch_detourAddress("client system info CVAR workaround", (long)&Patch_systemInfo, 0x0044B74D, qfalse);
    #endif // _GOLD
    #endif // _WIN32

    #ifdef __linux__
    #ifdef _GOLD
    Patch_detourAddress("client system info CVAR workaround", (long)&Patch_systemInfo, 0x08084a07, qfalse);
    #else
    Patch_detourAddress("client system info CVAR workaround", (long)&Patch_systemInfo, 0x0808087b, qfalse);
    #endif // _GOLD
    #endif // __linux__
}
