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
// patch_engineCVARs.c - Modifies several important CVARs in the engine
// their properties.

//==================================================================

#include "patch_local.h"

// Addresses hard-defined for the CVARs to patch.
#ifdef _WIN32
#ifdef _GOLD
#define FLOODPROTECT_ADDRESS 0x00ba3d38
#define ALLOWDOWNLOAD_ADDRESS 0x00bfdd8c
#elif _DEMO
#define FLOODPROTECT_ADDRESS 0x00b76168
#define ALLOWDOWNLOAD_ADDRESS 0x00bd0024
#else
#define FLOODPROTECT_ADDRESS 0x00b7a974
#define ALLOWDOWNLOAD_ADDRESS 0x00bd39c4
#endif // _GOLD, _DEMO, or Full
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
    Com_Printf("Applying engine CVAR attributes patch...\n");

    // Modify sv_floodProtect with read only attribute.
    (*(cvar_t **)FLOODPROTECT_ADDRESS)->flags |= CVAR_ROM;
    Com_Printf("%-4ssv_floodProtect to read only.\n", " ");

    #ifndef _DEMO
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
    #else
    // In demo, always disable sv_allowDownload.
    trap_Cvar_Set("sv_allowDownload", "0");

    (*(cvar_t **)ALLOWDOWNLOAD_ADDRESS)->flags |= CVAR_ROM;
    Com_Printf("%-4ssv_allowDownload to read only.\n", " ");
    #endif // not _DEMO
}
