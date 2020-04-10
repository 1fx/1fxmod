/*
===========================================================================
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
Copyright (C) 2010 - 2020, Ane-Jouke Schat (Boe!Man)
Copyright (C) 2010 - 2020, Jordi Epema (Henkie)

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
// 1fx_gt.h - Local definitions from the gametypes are stored here.

//==================================================================

// Static declarations go here.
// INF
#define ITEM_BRIEFCASE          100             
#define TRIGGER_EXTRACTION      200

// CTF
#define REDFLAG                 0
#define BLUEFLAG                1

#define ITEM_REDFLAG            100
#define ITEM_BLUEFLAG           101
                                
#define TRIGGER_REDCAPTURE      200
#define TRIGGER_BLUECAPTURE     201

#ifdef _GOLD
// DEM
#define TRIGGER_DEMOSITE_1      200
#define TRIGGER_DEMOSITE_2	201

#define ITEM_BOMB		300
#define ITEM_PLANTED_BOMB	301
#endif // _GOLD

// CVAR internal declarations.
vmCvar_t    gt_simpleScoring;
vmCvar_t    gt_flagReturnTime;

#ifdef _GOLD
vmCvar_t    gt_bombFuseTime;
vmCvar_t    gt_bombDefuseTime;
vmCvar_t    gt_bombPlantTime;
#endif // _GOLD

typedef struct gametypeLocals_s
{
    int     time;

    // INF (and partly H&S/H&Z)
    int         caseTakenSound;
    int         caseCaptureSound; // Also used in H&S and H&Z.
    int         caseReturnSound;

    // CTF
    int         redFlagDropTime;
    int         blueFlagDropTime;
    
    int         flagReturnSound;
    int         flagTakenSound;
    int         flagCaptureSound;
    
    int         redCaptureEffect;
    int         blueCaptureEffect;
    
    // Boe!Man 4/22/12: We use this for pause stuff.
    int         pauseTime;      // Restore time.
    
    // Boe!Man 10/30/13: We use this to determine if the flag is taken.
    qboolean    flagTaken[2];
    
    #ifdef _GOLD
    // Gold CTF HUD icons, v1.00 doesn't have these.
    int         iconRedFlag;
    int         iconBlueFlag;
    int         iconRedFlagDropped;
    int         iconBlueFlagDropped;
    int         iconRedFlagCarried;
    int         iconBlueFlagCarried;
    #endif // _GOLD

    // ELIM
    int         captureSound;

    #ifdef _GOLD
    // DEM
    int         bombBeepTime;

    int         bombPlantTime;
    vec3_t      bombPlantOrigin;
    char        bombPlantTarget[MAX_QPATH];

    qboolean    firstFrame;

    int         bombExplodeEffect;
    int         bombBeepSound;
    int         bombTakenSound;
    int         bombExplodedSound;
    int         bombPlantedSound;

    int         iconBombPlanted[7];

    int         bombGiveClient;
    int         bombPlantClient;

    qboolean    roundOver;
    #endif // _GOLD
    
} gametypeLocals_t;

extern  gametypeLocals_t    gametype;
