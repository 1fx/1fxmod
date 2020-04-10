/*
===========================================================================
Copyright (C) 2015 - 2020, Ane-Jouke Schat (Boe!Man)
Copyright (C) 2015 - 2020, Jordi Epema (Henkie)

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
            Info_SetValueForKey_Big(s, key, g_enforce1fxAdditions.integer ? "1fx.additions" : value);
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
    Patch_detourAddress("client system info CVAR workaround", (long)&Patch_systemInfo, 0x0044c7ed, qfalse);
    #elif _DEMO
    Patch_detourAddress("client system info CVAR workaround", (long)&Patch_systemInfo, 0x0044a1ed, qfalse);
    #else
    Patch_detourAddress("client system info CVAR workaround", (long)&Patch_systemInfo, 0x0044b74d, qfalse);
    #endif // _GOLD, _DEMO or Full
    #endif // _WIN32

    #ifdef __linux__
    #ifdef _GOLD
    Patch_detourAddress("client system info CVAR workaround", (long)&Patch_systemInfo, 0x08084a07, qfalse);
    #else
    Patch_detourAddress("client system info CVAR workaround", (long)&Patch_systemInfo, 0x0808087b, qfalse);
    #endif // _GOLD
    #endif // __linux__
}
