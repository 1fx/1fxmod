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
// 1fx_gt.c - All the functions, declarations and commands for the Gametype Modules go here.

//==================================================================

#include "g_local.h"
#include "boe_local.h"

#include "1fx_gt.h"

void    GT_Init     ( void );
void    GT_RunFrame ( int time );
int     GT_Event    ( int cmd, int time, int arg0, int arg1, int arg2, int arg3, int arg4 );

gametypeLocals_t    gametype;

typedef struct
{
    vmCvar_t    *vmCvar;
    char        *cvarName;
    char        *defaultString;
    int         cvarFlags;
    float       mMinValue, mMaxValue;
    int         modificationCount;  // for tracking changes
    qboolean    trackChange;        // track this variable, and announce if changed
    qboolean    teamShader;         // track and if changed, update shader state

} cvarTable_t;

static cvarTable_t gametypeCvarTable[] =
{
    { &gt_simpleScoring,    "gt_simpleScoring",     "0",  CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse }, // INF, CTF
    { &gt_flagReturnTime,   "gt_flagReturnTime",    "30", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse }, // CTF

    #ifdef _GOLD
    // DEM
    { &gt_bombFuseTime,		"gt_bombFuseTime",		"30", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &gt_bombDefuseTime,	"gt_bombDefuseTime",	"3",  CVAR_ARCHIVE | CVAR_LATCH, 0.0f, 0.0f, 0, qfalse },
	{ &gt_bombPlantTime,	"gt_bombPlantTime",		"3",  CVAR_ARCHIVE | CVAR_LATCH, 0.0f, 0.0f, 0, qfalse },
    #endif // _GOLD

    { NULL, NULL, NULL, 0, 0.0f, 0.0f, 0, qfalse }
};

/*
================
gtCall
by Boe!Man - 11/29/12

This is the only way control passes into the module.
Instead of recoding everything in the source, the trap_GT_* functions simply call this embedded function.
The function then takes control of whatever parameters are passed to the module.
================
*/

int gtCall( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11 )
{
    switch ( command )
    {
        case GAMETYPE_INIT:
            GT_Init ( );
            return 0;

        case GAMETYPE_START:
            if(current_gametype.value == GT_CTF){
                gametype.redFlagDropTime = 0;
                gametype.blueFlagDropTime = 0;

            #ifdef _GOLD
                G_SetHUDIcon (REDFLAG, gametype.iconRedFlag );
                G_SetHUDIcon (BLUEFLAG, gametype.iconBlueFlag );
            }else if(current_gametype.value == GT_DEM){
                gametype.firstFrame    = qtrue;
                gametype.bombPlantTime = 0;
                gametype.bombBeepTime  = 0;
                gametype.roundOver     = qfalse;
                G_SetHUDIcon ( 0, 0 );
            #endif // _GOLD
            }
            return 0;

        case GAMETYPE_RUN_FRAME:
            GT_RunFrame ( arg0 );
            return 0;

        case GAMETYPE_EVENT:
            return GT_Event ( arg0, arg1, arg2, arg3, arg4, arg5, arg6 );
    }

    return -1;
}

/*
=================
GT_RegisterCvars
=================
*/
void GT_RegisterCvars( void )
{
    cvarTable_t *cv;

    for ( cv = gametypeCvarTable ; cv->cvarName != NULL; cv++ )
    {
        trap_Cvar_Register( cv->vmCvar, cv->cvarName, cv->defaultString, cv->cvarFlags, cv->mMinValue, cv->mMaxValue );

        if ( cv->vmCvar )
        {
            cv->modificationCount = cv->vmCvar->modificationCount;
        }
    }
}

/*
=================
GT_UpdateCvars
=================
*/
void GT_UpdateCvars( void )
{
    cvarTable_t *cv;

    for ( cv = gametypeCvarTable ; cv->cvarName != NULL; cv++ )
    {
        if ( cv->vmCvar )
        {
            trap_Cvar_Update( cv->vmCvar );

            if ( cv->modificationCount != cv->vmCvar->modificationCount )
            {
                cv->modificationCount = cv->vmCvar->modificationCount;
            }
        }
    }
}

/*
================
GT_Init

initializes the gametype by spawning the gametype items and
preparing them

-- Boe!Man 11/29/12: Since we spawn the GT items now from the game itself, this can be partly recoded.
================
*/
void GT_Init ( void )
{
    gtItemDef_t     itemDef;
    gtTriggerDef_t  triggerDef;
    gitem_t*        item;
    gentity_t*      find = NULL;
    gtitem_t        *gtitem;

    memset ( &gametype, 0, sizeof(gametype) );
    memset ( &triggerDef, 0, sizeof(triggerDef) );
    memset ( &itemDef, 0, sizeof(itemDef) );

    // Register all cvars for this gametype
    GT_RegisterCvars ();

    // Boe!Man 11/30/12: Register the global sounds per gametype, and/or effects.
    if(current_gametype.value == GT_INF){
        gametype.caseTakenSound   = G_SoundIndex ("sound/ctf_flag.mp3", qtrue);
        gametype.caseCaptureSound = G_SoundIndex ("sound/ctf_win.mp3", qtrue);
        gametype.caseReturnSound  = G_SoundIndex ("sound/ctf_return.mp3", qtrue);
    }else if(current_gametype.value == GT_CTF){
        gametype.flagTakenSound   = G_SoundIndex ("sound/ctf_flag.mp3", qtrue);
        gametype.flagCaptureSound = G_SoundIndex ("sound/ctf_win.mp3", qtrue);
        gametype.flagReturnSound  = G_SoundIndex ("sound/ctf_return.mp3", qtrue);
    }else if(current_gametype.value == GT_ELIM){
        gametype.captureSound     = G_SoundIndex ("sound/ctf_win.mp3", qtrue);
    }else if(current_gametype.value == GT_HS || current_gametype.value == GT_HZ){
        gametype.caseCaptureSound = G_SoundIndex ("sound/ctf_win.mp3", qtrue);
    }
    #ifdef _GOLD
    else if(current_gametype.value == GT_DEM){
        gametype.bombTakenSound    = G_SoundIndex ("sound/ctf_flag.mp3", qtrue);
        gametype.bombExplodedSound = G_SoundIndex ("sound/ctf_win.mp3", qtrue);
        gametype.bombPlantedSound  = G_SoundIndex ("sound/ctf_base.mp3", qtrue);
        gametype.bombBeepSound     = G_SoundIndex ("sound/misc/c4/beep", qtrue);
        gametype.bombExplodeEffect = G_EffectIndex ("explosions/mushroom_explosion.efx");
    }
    #endif // _GOLD

    #ifdef _GOLD
    // Register HUD icons.
    if(current_gametype.value == GT_CTF){
        gametype.iconRedFlag = G_IconIndex ( "gfx/menus/hud/ctf_red" );
        gametype.iconBlueFlag = G_IconIndex ( "gfx/menus/hud/ctf_blue" );

        gametype.iconRedFlagDropped = G_IconIndex ( "gfx/menus/hud/ctf_red_dropped" );
        gametype.iconBlueFlagDropped = G_IconIndex ( "gfx/menus/hud/ctf_blue_dropped" );

        gametype.iconRedFlagCarried = G_IconIndex ( "gfx/menus/hud/ctf_red_carried" );
        gametype.iconBlueFlagCarried = G_IconIndex ( "gfx/menus/hud/ctf_blue_carried" );
    }else if(current_gametype.value == GT_DEM){
        gametype.iconBombPlanted[0] = G_IconIndex ( "gfx/menus/hud/dem_planted" );
        gametype.iconBombPlanted[1] = G_IconIndex ( "gfx/menus/hud/dem_planted1" );
        gametype.iconBombPlanted[2] = G_IconIndex ( "gfx/menus/hud/dem_planted2" );
        gametype.iconBombPlanted[3] = G_IconIndex ( "gfx/menus/hud/dem_planted3" );
        gametype.iconBombPlanted[4] = G_IconIndex ( "gfx/menus/hud/dem_planted4" );
        gametype.iconBombPlanted[5] = G_IconIndex ( "gfx/menus/hud/dem_planted5" );
        gametype.iconBombPlanted[6] = G_IconIndex ( "gfx/menus/hud/dem_planted6" );
    }
    #endif // _GOLD

    // Register the items per gametype.
    if(current_gametype.value == GT_INF || current_gametype.value == GT_HS){
        item = BG_FindItem ("briefcase");
        if (item){
            gtitem = &level.gametypeItems[item->giTag];
            gtitem->id = ITEM_BRIEFCASE;

            item->quantity = ITEM_BRIEFCASE;
        }
    }else if(current_gametype.value == GT_CTF){
        item = BG_FindItem ("red_flag");
        if (item){
            gtitem = &level.gametypeItems[item->giTag];
            gtitem->id = ITEM_REDFLAG;

            item->quantity = ITEM_REDFLAG;
        }

        item = NULL;
        item = BG_FindItem ("blue_flag");
        if (item){
            gtitem = &level.gametypeItems[item->giTag];
            gtitem->id = ITEM_BLUEFLAG;

            item->quantity = ITEM_BLUEFLAG;
        }
    }
    #ifdef _GOLD
    else if(current_gametype.value == GT_DEM){
        // Register the trigger first, we need the sound from the trigger later.
        memset ( &triggerDef, 0, sizeof(triggerDef) );
        triggerDef.size		= sizeof(triggerDef);
        triggerDef.use		= qtrue;
        triggerDef.useTime	= gt_bombPlantTime.integer * 1000;
        triggerDef.useIcon	= G_IconIndex("gfx/menus/hud/tnt");
        triggerDef.useSound = G_SoundIndex("sound/misc/c4/c4_loop", qtrue);

        while ( NULL != (find = G_Find ( find, FOFS(classname), "gametype_trigger" ) ) )
        {
            if ( Q_stricmp ( find->targetname, "demolition_site_1") && Q_stricmp ( find->targetname, "demolition_site_2")){
                continue;
            }

            // Assign the id to it.
            if(Q_stricmp ( find->targetname, "demolition_site_1") == 0){
                find->health    = TRIGGER_DEMOSITE_1;
            }else{
                find->health    = TRIGGER_DEMOSITE_2;
            }
            find->touch     = gametype_trigger_touch;
            find->use		= gametype_trigger_use;
            find->delay		= triggerDef.useIcon;
            find->soundPos1 = triggerDef.useTime;
            find->soundLoop = triggerDef.useSound;

            trap_LinkEntity (find);
        }

        // Now register the items.
        itemDef.size = sizeof(itemDef);
        item = BG_FindItem ("c4");
        if (item){
            gtitem = &level.gametypeItems[item->giTag];
            gtitem->id = ITEM_BOMB;

            item->quantity = ITEM_BOMB;
        }

        itemDef.use = qtrue;
        itemDef.useTime = gt_bombDefuseTime.integer * 1000;
        itemDef.useSound = triggerDef.useSound;
        itemDef.useIcon = G_IconIndex ( "gfx/menus/hud/wire_cutters" );
        item = NULL;
        item = BG_FindItem ("armed_c4");
        if (item){
            gtitem = &level.gametypeItems[item->giTag];
            gtitem->id = ITEM_PLANTED_BOMB;

            item->quantity = ITEM_PLANTED_BOMB;

            gtitem->useIcon  = itemDef.useIcon;
            gtitem->useTime  = itemDef.useTime;
            gtitem->useSound = itemDef.useSound;
        }
    }
    #endif // _GOLD

    // Register triggers per gametype.
    if(current_gametype.value == GT_INF){
        while ( NULL != (find = G_Find ( find, FOFS(classname), "gametype_trigger" ) ) )
        {
            if ( Q_stricmp ( find->targetname, (const char*) "briefcase_destination")){
                continue;
            }

            // Assign the id to it.
            find->health = TRIGGER_EXTRACTION;
            find->touch  = gametype_trigger_touch;
            trap_LinkEntity (find);
        }
    }else if(current_gametype.value == GT_CTF){
        while ( NULL != (find = G_Find ( find, FOFS(classname), "gametype_trigger" ) ) )
        {
            if ( Q_stricmp ( find->targetname, (const char*) "red_capture_point")){
                continue;
            }

            // Assign the id to it.
            find->health = TRIGGER_REDCAPTURE;
            find->touch  = gametype_trigger_touch;
            trap_LinkEntity (find);
        }

        // Boe!Man 11/30/12: Two items, NULL find again.
        find = NULL;
        while ( NULL != (find = G_Find ( find, FOFS(classname), "gametype_trigger" ) ) )
        {
            if ( Q_stricmp ( find->targetname, (const char*) "blue_capture_point")){
                continue;
            }

            // Assign the id to it.
            find->health = TRIGGER_BLUECAPTURE;
            find->touch  = gametype_trigger_touch;
            trap_LinkEntity (find);
        }
    }

    // Boe!Man 11/29/12: Semi-debug, but we let the admin know the gt has been loaded.
    if(current_gametype.value == GT_HS){
        Com_Printf("Gametype initialized: h&s\n");
    }else if(current_gametype.value == GT_HZ){
        Com_Printf("Gametype initialized: h&z\n");
    }else{
        Com_Printf("Gametype initialized: %s\n", g_gametype.string);
    }
}

/*
================
GT_RunFrame

Runs all thinking code for gametype
================
*/
void GT_RunFrame ( int time )
{
    gametype.time = time;

    #ifdef _GOLD
    if(current_gametype.value == GT_DEM){
        gitem_t* item;

        if (gametype.firstFrame)
        {
            int clients[MAX_CLIENTS];
            int i, count = 0;

            // In the first frame, set the initial location of the bomb.
            // (or, disabled if the server owner has this feature disabled)
            // After the bomb is given away, it will automatically update.
            if(G_showItemLocations()){
                Q_strncpyz(level.objectiveLoc, "Not given yet",
                    sizeof(level.objectiveLoc));
            }else{
                Q_strncpyz(level.objectiveLoc, "Disabled",
                    sizeof(level.objectiveLoc));
            }

            for(i = 0; i < level.numConnectedClients
                && count < MAX_CLIENTS; i++)
            {
                gclient_t* client = &level.clients[level.sortedClients[i]];

                if(client->pers.connected != CON_CONNECTED){
					continue;
				}

				if(client->sess.team == TEAM_BLUE){
					clients[count++] = level.sortedClients[i];
				}
            }

            if(count > 0){
                gametype.bombGiveClient = gametype.bombGiveClient % count;

                // Give the gametype item.
                item = BG_FindGametypeItemByID ( ITEM_BOMB );
                if(item){
                    level.clients[clients[gametype.bombGiveClient]].
                        ps.stats[STAT_GAMETYPE_ITEMS] |= (1<<item->giTag);

                    if(G_showItemLocations()){
                        Q_strncpyz(level.objectiveLoc,
                            level.clients[clients[gametype.bombGiveClient]].
                            pers.netname,
                            sizeof(level.objectiveLoc));
                    }
                }
                gametype.firstFrame = qfalse;

                // Next time use the next client in the list
                gametype.bombGiveClient = (gametype.bombGiveClient + 1 ) % count;
            }
        }

        if ( gametype.bombPlantTime )
        {
            static const int slowTime = 1000;
            static const int fastTime = 100;

            if ( !gametype.bombBeepTime || gametype.time > gametype.bombBeepTime )
            {
                float addTime;

                addTime = (float)(gametype.bombPlantTime - gametype.time) / (float)(gt_bombFuseTime.integer * 1000);
                addTime = fastTime + (addTime * (float)(slowTime - fastTime) );

                gametype.bombBeepTime = gametype.time + (int)addTime;

                G_SoundAtLoc (gametype.bombPlantOrigin, CHAN_AUTO, gametype.bombBeepSound);

                addTime = (float)(gametype.bombPlantTime - gametype.time) / (float)(gt_bombFuseTime.integer * 1000);
                addTime = 6.0f - 6.0f * addTime ;
                G_SetHUDIcon(0, gametype.iconBombPlanted[ Com_Clamp ( 0, 6, (int)addTime ) ]);
            }
        }

        if ( gametype.bombPlantTime && gametype.time > gametype.bombPlantTime )
        {
            static vec3_t up = {0,0,1};
            int clients[MAX_CLIENTS];
            int i, count = 0;

            G_PlayEffect(gametype.bombExplodeEffect, gametype.bombPlantOrigin,
                up);
            G_UseTargetsByName(gametype.bombPlantTarget, NULL, NULL);
            item = BG_FindGametypeItemByID(ITEM_PLANTED_BOMB);
			if(item){
				G_ResetGametypeItem (item);
			}

            if(!gametype.roundOver){
                G_AddTeamScore (TEAM_BLUE, 1);
                G_Broadcast(va("%s ^7team\nhas \\destroyed the target!",
                    server_blueteamprefix.string), BROADCAST_GAME, NULL);

                trap_SendServerCommand(-1,
                    "print\"^3[DEM] ^7Blue team has destroyed the target.\n\"");

                if(G_showItemLocations()){
                    Q_strncpyz(level.objectiveLoc, "Destroyed",
                        sizeof(level.objectiveLoc));
                }

                G_SetHUDIcon(0, gametype.iconBombPlanted[6]);

                // Start a global sound.
                if(!level.intermissionQueued
                    && !level.intermissiontime
                    && !level.awardTime)
                {
                    gentity_t* tent;
                    tent = G_TempEntity(vec3_origin, EV_GLOBAL_SOUND);
                    tent->s.eventParm = gametype.bombExplodedSound;
                    tent->r.svFlags = SVF_BROADCAST;
                }

                level.gametypeResetTime = level.time + 5000;

                // Give the guy who planted it some props
                if ( !gt_simpleScoring.integer )
                {
                    G_AddScore(&g_entities[gametype.bombPlantClient], 10);
                }

                // Is the timelimit hit already?
                if(level.timelimithit){
                    gentity_t*  tent;
                    tent = G_TempEntity( vec3_origin, EV_GAME_OVER );
                    tent->s.eventParm = GAME_OVER_TIMELIMIT;
                    tent->r.svFlags = SVF_BROADCAST;
                    level.timelimithit = qfalse;
                    LogExit("Timelimit hit.");
                }
            }

            gametype.bombPlantTime = 0;

            // Get the bomb client # so we can give the bomb to the same guy again
            for (i = 0; i < level.numConnectedClients && count < MAX_CLIENTS; i++){
                gclient_t* client = &level.clients[level.sortedClients[i]];

                if ( client->pers.connected != CON_CONNECTED )
				{
					continue;
				}

				if (client->sess.team == TEAM_BLUE){
					clients[count++] = level.sortedClients[i];
				}
            }

            if (count > 0)
            {
                for ( count--; count >= 0; count-- )
                {
                    if ( clients[count] == gametype.bombPlantClient )
                    {
                        gametype.bombGiveClient = count;
                        break;
                    }
                }
            }

            gametype.roundOver = qtrue;
        }
    }
    #endif // _GOLD

    GT_UpdateCvars ();

    // Boe!Man 4/22/12: Only check if flags need to be returned when the game is NOT paused.
    if(current_gametype.value == GT_CTF && !level.pause){
        // See if we need to return the red flag yet
        if ( gametype.redFlagDropTime && time - gametype.redFlagDropTime > gt_flagReturnTime.integer * 1000 )
        {
            // Boe!Man 11/29/12: Reset item.
            gitem_t* item;

            item = BG_FindGametypeItemByID ( ITEM_REDFLAG );
            if (item){
                G_ResetGametypeItem ( item );
            }

            G_Broadcast(va("The %s ^7flag has \\returned!", server_redteamprefix.string), BROADCAST_GAME, NULL);
            trap_SendServerCommand( -1, "print \"^3[CTF] ^7The Red flag has returned.\n\"");

            // Boe!Man 11/29/12: Global sound.
            if(!level.intermissionQueued && !level.intermissiontime && !level.awardTime){
                gentity_t* tent;
                tent = G_TempEntity( vec3_origin, EV_GLOBAL_SOUND );
                tent->s.eventParm = gametype.flagReturnSound;
                tent->r.svFlags = SVF_BROADCAST;
            }

            gametype.redFlagDropTime = 0;
            gametype.flagTaken[REDFLAG] = qfalse;

            #ifdef _GOLD
            G_SetHUDIcon (REDFLAG, gametype.iconRedFlag);
            #endif // _GOLD
        }
        // See if we need to return the blue flag yet
        if ( gametype.blueFlagDropTime && time - gametype.blueFlagDropTime > gt_flagReturnTime.integer * 1000 )
        {
            // Boe!Man 11/29/12: Reset item.
            gitem_t* item;

            item = BG_FindGametypeItemByID ( ITEM_BLUEFLAG );
            if (item){
                G_ResetGametypeItem ( item );
            }

            G_Broadcast(va("The %s ^7flag has \\returned!", server_blueteamprefix.string), BROADCAST_GAME, NULL);
            trap_SendServerCommand( -1, "print \"^3[CTF] ^7The Blue flag has returned.\n\"");

            // Boe!Man 11/29/12: Global sound.
            if(!level.intermissionQueued && !level.intermissiontime && !level.awardTime){
                gentity_t* tent;
                tent = G_TempEntity( vec3_origin, EV_GLOBAL_SOUND );
                tent->s.eventParm = gametype.flagReturnSound;
                tent->r.svFlags = SVF_BROADCAST;
            }

            gametype.blueFlagDropTime = 0;
            gametype.flagTaken[BLUEFLAG] = qfalse;

            #ifdef _GOLD
            G_SetHUDIcon (BLUEFLAG, gametype.iconBlueFlag);
            #endif // _GOLD
        }
    }
}

/*
================
GT_Event

Handles all events sent to the gametype
Boe!Man 11/29/12: Instead of sending events back to the game to process, everything happens here. This is obviously a massive speed boost,
                  since normally the code would send the syscall to the engine, engine -> gametype, gametype -> engine, engine -> game.
                  Now everything happens here, in the game module.
================
*/
int GT_Event ( int cmd, int time, int arg0, int arg1, int arg2, int arg3, int arg4 )
{
    switch ( cmd )
    {
        case GTEV_ITEM_DEFEND:
            #ifdef _GOLD
            if(current_gametype.value == GT_INF || current_gametype.value == GT_CTF || current_gametype.value == GT_DEM)
            #else
            if(current_gametype.value == GT_INF || current_gametype.value == GT_CTF)
            #endif // _GOLD
            {
                if ( !gt_simpleScoring.integer )
                {
                    G_AddScore ( &g_entities[arg1], 5);
                }

                g_entities[arg1].client->pers.statinfo.itemDefends++;
            }
            return 0;

        // Boe!Man 4/22/12: Pause code in CTF.
        case GTEV_PAUSE:
            if(current_gametype.value == GT_CTF){
                if(arg0){ // 1 so pause the gametype as well.
                    gametype.pauseTime = time;
                }else{
                    // Check if there are any flags dropped.
                    if(gametype.blueFlagDropTime){ // Blue flag was dropped before the pause.
                        gametype.blueFlagDropTime += (time - gametype.pauseTime);
                    }
                    if(gametype.redFlagDropTime){ // NOT else if because BOTH flags can be dropped..
                        gametype.redFlagDropTime += (time - gametype.pauseTime);
                    }

                    // Reset timer.
                    gametype.pauseTime = 0;
                }
            }
            return 0;
        // End Boe!Man 4/22/12

        case GTEV_ITEM_STUCK:
            if(current_gametype.value == GT_INF){
                int i;
                gitem_t* item;

                if (!g_caserun.integer){
                    G_Broadcast("The briefcase has \\returned!", BROADCAST_GAME, NULL);
                    trap_SendServerCommand( -1, "print \"^3[INF] ^7The briefcase has returned.\n\"");
                }else{
                    G_Broadcast("\\Briefcase respawned!", BROADCAST_GAME, NULL);
                    trap_SendServerCommand(-1, "print \"^3[CR] ^7The briefcase has respawned.\n\"");

                    // Boe!Man 12/2/14: FIXME: Bodies disappear here.
                    for (i = 0; i < level.num_entities; i++)
                    {
                        gentity_t* ent;

                        ent = &g_entities[i];

                        if (ent->s.eType == ET_BODY){
                            trap_UnlinkEntity(ent);
                        }
                    }
                }

                // Boe!Man 11/29/12: Reset item.
                item = BG_FindGametypeItemByID ( ITEM_BRIEFCASE );
                if (item){
                    G_ResetGametypeItem ( item );
                }

                // Boe!Man 11/29/12: Global sound.
                if(!level.intermissionQueued && !level.intermissiontime && !level.awardTime){
                    gentity_t* tent;
                    tent = G_TempEntity( vec3_origin, EV_GLOBAL_SOUND );
                    tent->s.eventParm = gametype.caseReturnSound;
                    tent->r.svFlags = SVF_BROADCAST;
                }
                return 1;
            }else if(current_gametype.value == GT_CTF){
                switch (arg0)
                {
                    gitem_t* item;

                    case ITEM_REDFLAG:
                        G_Broadcast(va("The %s ^7flag has \\returned!", server_redteamprefix.string), BROADCAST_GAME, NULL);
                        trap_SendServerCommand( -1, "print \"^3[CTF] ^7The Red flag has returned.\n\"");

                        // Boe!Man 11/29/12: Reset item.
                        item = BG_FindGametypeItemByID ( ITEM_REDFLAG );
                        if (item){
                            G_ResetGametypeItem ( item );
                        }

                        // Boe!Man 11/29/12: Global sound.
                        if(!level.intermissionQueued && !level.intermissiontime && !level.awardTime){
                            gentity_t* tent;
                            tent = G_TempEntity( vec3_origin, EV_GLOBAL_SOUND );
                            tent->s.eventParm = gametype.flagReturnSound;
                            tent->r.svFlags = SVF_BROADCAST;
                        }

                        gametype.redFlagDropTime = 0;
                        gametype.flagTaken[REDFLAG] = qfalse;

                        #ifdef _GOLD
                        G_SetHUDIcon (REDFLAG, gametype.iconRedFlag);
                        #endif // _GOLD
                        return 1;

                    case ITEM_BLUEFLAG:
                        G_Broadcast(va("The %s ^7flag has \\returned!", server_blueteamprefix.string), BROADCAST_GAME, NULL);
                        trap_SendServerCommand( -1, "print \"^3[CTF] ^7The Blue flag has returned.\n\"");

                        // Boe!Man 11/29/12: Reset item.
                        item = BG_FindGametypeItemByID ( ITEM_BLUEFLAG );
                        if (item){
                            G_ResetGametypeItem ( item );
                        }


                        // Boe!Man 11/29/12: Global sound.
                        if(!level.intermissionQueued && !level.intermissiontime && !level.awardTime){
                            gentity_t* tent;
                            tent = G_TempEntity( vec3_origin, EV_GLOBAL_SOUND );
                            tent->s.eventParm = gametype.flagReturnSound;
                            tent->r.svFlags = SVF_BROADCAST;
                        }

                        gametype.blueFlagDropTime = 0;
                        gametype.flagTaken[BLUEFLAG] = qfalse;

                        #ifdef _GOLD
                        G_SetHUDIcon (BLUEFLAG, gametype.iconBlueFlag);
                        #endif // _GOLD
                        return 1;
                }
            }else if(current_gametype.value == GT_HS){
                trap_SendServerCommand( -1, "print \"^3[H&S] ^7The briefcase has disappeared.\n\"");
                return 1;
            }

            break;

        case GTEV_TEAM_ELIMINATED:
            if(current_gametype.value == GT_INF || current_gametype.value == GT_ELIM){
                switch ( arg0 )
                {
                    case TEAM_RED:
                        G_Broadcast(va("%s ^7team \\eliminated!", server_redteamprefix.string), BROADCAST_GAME, NULL);
                        trap_SendServerCommand( -1, va("print\"^3[%s] ^7Red team eliminated.\n\"", Q_strupr(g_gametype.string)));
                        G_AddTeamScore ((team_t) TEAM_BLUE, 1);
                        // Boe!Man 11/29/12: Global sound.
                        if(!level.intermissionQueued && !level.intermissiontime && !level.awardTime){
                        gentity_t* tent;
                        tent = G_TempEntity( vec3_origin, EV_GLOBAL_SOUND );
                        if(current_gametype.value == GT_INF){
                            tent->s.eventParm = gametype.caseCaptureSound;
                        }else{ // ELIM
                            tent->s.eventParm = gametype.captureSound;
                        }
                        tent->r.svFlags = SVF_BROADCAST;
                        }

                        // Boe!Man 11/29/12: Reset gametype.
                        level.gametypeResetTime = level.time + 5000;
                        break;

                    case TEAM_BLUE:
                        G_Broadcast(va("%s ^7team \\eliminated!", server_blueteamprefix.string), BROADCAST_GAME, NULL);
                        trap_SendServerCommand( -1, va("print\"^3[%s] ^7Blue team eliminated.\n\"", Q_strupr(g_gametype.string)));
                        G_AddTeamScore ((team_t) TEAM_RED, 1);
                        // Boe!Man 11/29/12: Global sound.
                        if(!level.intermissionQueued && !level.intermissiontime && !level.awardTime){
                        gentity_t* tent;
                        tent = G_TempEntity( vec3_origin, EV_GLOBAL_SOUND );
                        if(current_gametype.value == GT_INF){
                            tent->s.eventParm = gametype.caseCaptureSound;
                        }else{ // ELIM
                            tent->s.eventParm = gametype.captureSound;
                        }
                        tent->r.svFlags = SVF_BROADCAST;
                        }

                        // Boe!Man 11/29/12: Reset gametype.
                        level.gametypeResetTime = level.time + 5000;
                        break;
                }
            }else if(current_gametype.value == GT_HS){
                switch ( arg0 )
                {
                    case TEAM_RED:
                        G_Broadcast(va("%s ^7won!", server_seekerteamprefix.string), BROADCAST_GAME, NULL);
                        trap_SendServerCommand( -1, "print\"^3[H&S] ^7Seekers won the match.\n\"");
                        G_AddTeamScore ((team_t) TEAM_BLUE, 1);
                        // Boe!Man 11/29/12: Global sound.
                        if(!level.intermissionQueued && !level.intermissiontime && !level.awardTime){
                        gentity_t* tent;
                        tent = G_TempEntity( vec3_origin, EV_GLOBAL_SOUND );
                        tent->s.eventParm = gametype.caseCaptureSound;
                        tent->r.svFlags = SVF_BROADCAST;
                        }

                        // Boe!Man 11/29/12: Reset gametype.
                        level.gametypeResetTime = level.time + 5000;
                        break;

                    case TEAM_BLUE:
                        break;
                }
            }else if(current_gametype.value == GT_HZ){
                switch ( arg0 )
                {
                    case TEAM_RED:
                        G_Broadcast(va("%s ^7won!", server_zombieteamprefix.string), BROADCAST_GAME, NULL);
                        trap_SendServerCommand( -1, "print\"^3[H&Z] ^7Zombies won the match.\n\"");
                        G_AddTeamScore ((team_t) TEAM_BLUE, 1);
                        // Boe!Man 11/29/12: Global sound.
                        if(!level.intermissionQueued && !level.intermissiontime && !level.awardTime){
                        gentity_t* tent;
                        tent = G_TempEntity( vec3_origin, EV_GLOBAL_SOUND );
                        tent->s.eventParm = gametype.caseCaptureSound;
                        tent->r.svFlags = SVF_BROADCAST;
                        }

                        // Boe!Man 11/29/12: Reset gametype.
                        level.gametypeResetTime = level.time + 5000;
                        break;

                    case TEAM_BLUE:
                        break;
                }
            }
            #ifdef _GOLD
            else if(current_gametype.value == GT_DEM){
                switch ( arg0 )
                {
                    case TEAM_RED:
                        G_Broadcast(va("%s ^7team \\eliminated!", server_redteamprefix.string), BROADCAST_GAME, NULL);
                        trap_SendServerCommand( -1, "print\"^3[DEM] ^7Red team eliminated.\n\"");
                        G_AddTeamScore ((team_t) TEAM_BLUE, 1);
                        // Boe!Man 11/29/12: Global sound.
                        if(!level.intermissionQueued && !level.intermissiontime && !level.awardTime){
                            gentity_t* tent;
                            tent = G_TempEntity( vec3_origin, EV_GLOBAL_SOUND );
                            tent->s.eventParm = gametype.bombExplodedSound;
                            tent->r.svFlags = SVF_BROADCAST;
                        }

                        // Boe!Man 11/29/12: Reset gametype.
                        level.gametypeResetTime = level.time + 5000;
                        gametype.roundOver = qtrue;
                        break;

                    case TEAM_BLUE:
                        // If the bomb is planted the defending team MUST defuse it.
                        if ( !gametype.bombPlantTime )
                        {
                            G_Broadcast(va("%s ^7team \\eliminated!", server_blueteamprefix.string), BROADCAST_GAME, NULL);
                            trap_SendServerCommand( -1, "print\"^3[DEM] ^7Blue team eliminated.\n\"");
                            G_AddTeamScore ((team_t) TEAM_RED, 1);
                            // Boe!Man 11/29/12: Global sound.
                            if(!level.intermissionQueued && !level.intermissiontime && !level.awardTime){
                                gentity_t* tent;
                                tent = G_TempEntity( vec3_origin, EV_GLOBAL_SOUND );
                                tent->s.eventParm = gametype.bombExplodedSound;
                                tent->r.svFlags = SVF_BROADCAST;
                            }

                            // Boe!Man 11/29/12: Reset gametype.
                            level.gametypeResetTime = level.time + 5000;
                            gametype.roundOver = qtrue;
                        }

                        break;
                }

                break;
            }
            #endif // _GOLD

            break;

        case GTEV_TIME_EXPIRED:
            if(current_gametype.value == GT_INF){
                G_Broadcast(va("%s ^7team\n\\defended the briefcase!", server_redteamprefix.string), BROADCAST_GAME, NULL);
                trap_SendServerCommand( -1, "print\"^3[INF] ^7Red team defended the briefcase.\n\"");
                G_AddTeamScore ((team_t) TEAM_RED, 1);

                // Boe!Man 11/29/12: Reset gametype.
                level.gametypeResetTime = level.time + 5000;
            }else if(current_gametype.value == GT_ELIM){
                G_Broadcast("\\Round draw!", BROADCAST_GAME, NULL);
                trap_SendServerCommand( -1, "print\"^3[ELIM] ^7Round Draw.\n\"");
                // Boe!Man 11/29/12: Reset gametype.
                level.gametypeResetTime = level.time + 5000;
            }else if(current_gametype.value == GT_HS){
                G_Broadcast(va("%s ^7won!", server_hiderteamprefix.string), BROADCAST_GAME, NULL);
                trap_SendServerCommand( -1, "print\"^3[H&S] ^7Hiders won the match.\n\"");
                G_AddTeamScore ((team_t) TEAM_RED, 1);
                // Boe!Man 11/29/12: Global sound.
                if(!level.intermissionQueued && !level.intermissiontime && !level.awardTime){
                gentity_t* tent;
                tent = G_TempEntity( vec3_origin, EV_GLOBAL_SOUND );
                tent->s.eventParm = gametype.caseCaptureSound;
                tent->r.svFlags = SVF_BROADCAST;
                }

                // Boe!Man 11/29/12: Reset gametype.
                level.gametypeResetTime = level.time + 5000;
                break;
            }else if(current_gametype.value == GT_HZ){
                G_Broadcast(va("%s ^7won!", server_humanteamprefix.string), BROADCAST_GAME, NULL);
                trap_SendServerCommand( -1, "print\"^3[H&Z] ^7Humans won the match.\n\"");
                G_AddTeamScore ((team_t) TEAM_RED, 1);
                // Boe!Man 11/29/12: Global sound.
                if(!level.intermissionQueued && !level.intermissiontime && !level.awardTime){
                gentity_t* tent;
                tent = G_TempEntity( vec3_origin, EV_GLOBAL_SOUND );
                tent->s.eventParm = gametype.caseCaptureSound;
                tent->r.svFlags = SVF_BROADCAST;
                }

                // Boe!Man 11/29/12: Reset gametype.
                level.gametypeResetTime = level.time + 5000;
                break;
            }
            #ifdef _GOLD
            else if(current_gametype.value == GT_DEM){
                G_Broadcast(va("%s ^7team\nhas \\defended the bomb site!", server_redteamprefix.string), BROADCAST_GAME, NULL);
                trap_SendServerCommand( -1, "print\"^3[DEM] ^7Red team has defended the bomb site.\n\"");
                G_AddTeamScore ((team_t) TEAM_RED, 1);

                // Boe!Man 11/29/12: Reset gametype.
                level.gametypeResetTime = level.time + 5000;
                gametype.roundOver = qtrue;
            }
            #endif // _GOLD

            break;

        case GTEV_ITEM_DROPPED:
            if(current_gametype.value == GT_INF){
                G_Broadcast(va("%s\n\\dropped the briefcase!", g_entities[arg1].client->pers.netname), BROADCAST_GAME, NULL);
            }else if(current_gametype.value == GT_CTF){
                switch (arg0)
                {
                    case ITEM_BLUEFLAG:
                        G_Broadcast(va("%s\n\\dropped the %s ^7flag!", g_entities[arg1].client->pers.netname, server_blueteamprefix.string), BROADCAST_GAME, NULL);
                        gametype.blueFlagDropTime = time;

                        #ifdef _GOLD
                        G_SetHUDIcon (BLUEFLAG, gametype.iconBlueFlagDropped);
                        #endif // _GOLD
                        break;
                    case ITEM_REDFLAG:
                        G_Broadcast(va("%s\n\\dropped the %s ^7flag!", g_entities[arg1].client->pers.netname, server_redteamprefix.string), BROADCAST_GAME, NULL);
                        gametype.redFlagDropTime = time;

                        #ifdef _GOLD
                        G_SetHUDIcon (REDFLAG, gametype.iconRedFlagDropped);
                        #endif // _GOLD
                        break;
                    default:
                        break;
                }
            }
            #ifdef _GOLD
            else if(current_gametype.value == GT_DEM){
                G_Broadcast(va("%s\nhas \\dropped the bomb!", g_entities[arg1].client->pers.netname), BROADCAST_GAME, NULL);
            }
            #endif // _GOLD

            break;

        case GTEV_ITEM_TOUCHED:
            if(current_gametype.value == GT_INF){
                switch (arg0){
                    case ITEM_BRIEFCASE:
                        if (arg2 == TEAM_BLUE || g_caserun.integer){
                            G_Broadcast(va("%s\nhas \\taken the briefcase!",
                                g_entities[arg1].client->pers.netname),
                                BROADCAST_GAME, NULL);

                            trap_SendServerCommand(-1,
                                va("print\"^3[%s] ^7%s has taken the " \
                                "briefcase.\n\"",
                                (g_caserun.integer) ? "CR" : "INF",
                                g_entities[arg1].client->pers.cleanName));

                            if(G_showItemLocations()){
                                Q_strncpyz(level.objectiveLoc,
                                    g_entities[arg1].client->pers.netname,
                                    sizeof(level.objectiveLoc));
                            }

                            // Boe!Man 11/29/12: Global sound.
                            if(!level.intermissionQueued
                                && !level.intermissiontime
                                && !level.awardTime)
                            {
                                gentity_t* tent;
                                tent = G_TempEntity(vec3_origin,
                                    EV_GLOBAL_SOUND);
                                tent->s.eventParm = gametype.caseTakenSound;
                                tent->r.svFlags = SVF_BROADCAST;
                            }

                            if (g_caserun.integer){
                                g_entities[arg1].client->caserunHoldTime
                                    = level.time + 5000;
                            }

                            // Boe!Man 11/29/12: Radio message.
                            G_Voice(&g_entities[arg1], NULL, SAY_TEAM,
                                "got_it", qfalse);
                            return 1;
                        }
                        break;
                }
            }else if(current_gametype.value == GT_CTF){
                switch (arg0)
                {
                    gitem_t* item;

                    case ITEM_BLUEFLAG:
                        if(arg2 == TEAM_RED)
                        {
                            G_Broadcast(va("%s\nhas \\taken the %s ^7flag!",
                                g_entities[arg1].client->pers.netname,
                                server_blueteamprefix.string),
                                BROADCAST_GAME, NULL);

                            trap_SendServerCommand(-1,
                                va("print\"^3[CTF] ^7%s has taken the Blue " \
                                "flag.\n\"",
                                g_entities[arg1].client->pers.cleanName));

                            if(G_showItemLocations()){
                                Q_strncpyz(level.objectiveLoc,
                                    g_entities[arg1].client->pers.netname,
                                    sizeof(level.objectiveLoc));
                            }

                            // Boe!Man 11/29/12: Global sound.
                            if(!level.intermissionQueued
                                && !level.intermissiontime
                                && !level.awardTime)
                            {
                                gentity_t* tent;
                                tent = G_TempEntity(vec3_origin,
                                    EV_GLOBAL_SOUND);
                                tent->s.eventParm = gametype.flagTakenSound;
                                tent->r.svFlags = SVF_BROADCAST;
                            }

                            // Boe!Man 11/29/12: Radio message.
                            G_Voice(&g_entities[arg1], NULL, SAY_TEAM,
                                "got_it", qfalse);
                            gametype.blueFlagDropTime = 0;
                            gametype.flagTaken[BLUEFLAG] = qtrue;
                            #ifdef _GOLD
                            G_SetHUDIcon (BLUEFLAG,
                                gametype.iconBlueFlagCarried);
                            #endif // _GOLD
                            return 1;
                        }else if(arg2 == TEAM_BLUE && g_ctfClassic.integer
                            && gametype.blueFlagDropTime)
                        {
                            // Boe!Man 2/1/13: Include touch-flag
                            // (classic) CTF mode.
                            G_Broadcast(va("%s\nhas \\returned the %s ^7flag!",
                                g_entities[arg1].client->pers.netname,
                                server_blueteamprefix.string),
                                BROADCAST_GAME, NULL);

                            trap_SendServerCommand(-1,
                                va("print\"^3[CTF] ^7%s has returned the " \
                                "Blue flag.\n\"",
                                g_entities[arg1].client->pers.cleanName));

                            item = BG_FindGametypeItemByID (ITEM_BLUEFLAG);
                            if(item){
                                G_ResetGametypeItem (item);
                            }

                            // Boe!Man 11/29/12: Global sound.
                            if(!level.intermissionQueued
                                && !level.intermissiontime
                                && !level.awardTime)
                            {
                                gentity_t* tent;
                                tent = G_TempEntity(vec3_origin,
                                    EV_GLOBAL_SOUND);
                                tent->s.eventParm = gametype.flagReturnSound;
                                tent->r.svFlags = SVF_BROADCAST;
                            }
                            gametype.blueFlagDropTime = 0;
                            gametype.flagTaken[BLUEFLAG] = qfalse;
                            #ifdef _GOLD
                            G_SetHUDIcon (BLUEFLAG, gametype.iconBlueFlag);
                            #endif // _GOLD
                            return 0;
                        }
                        break;

                    case ITEM_REDFLAG:
                        if(arg2 == TEAM_BLUE)
                        {
                            G_Broadcast(va("%s\nhas \\taken the %s ^7flag!",
                                g_entities[arg1].client->pers.netname,
                                server_redteamprefix.string),
                                BROADCAST_GAME, NULL);

                            trap_SendServerCommand(-1,
                                va("print\"^3[CTF] ^7%s has taken the " \
                                "Red flag.\n\"",
                                g_entities[arg1].client->pers.cleanName));

                            if(G_showItemLocations()){
                                Q_strncpyz(level.objective2Loc,
                                    g_entities[arg1].client->pers.netname,
                                    sizeof(level.objective2Loc));
                            }

                            // Boe!Man 11/29/12: Global sound.
                            if(!level.intermissionQueued
                                && !level.intermissiontime
                                && !level.awardTime)
                            {
                            gentity_t* tent;
                                tent = G_TempEntity(vec3_origin,
                                    EV_GLOBAL_SOUND);
                                tent->s.eventParm = gametype.flagTakenSound;
                                tent->r.svFlags = SVF_BROADCAST;
                            }

                            // Boe!Man 11/29/12: Radio message.
                            G_Voice (&g_entities[arg1], NULL, SAY_TEAM,
                                "got_it", qfalse);
                            gametype.redFlagDropTime = 0;
                            gametype.flagTaken[REDFLAG] = qtrue;

                            #ifdef _GOLD
                            G_SetHUDIcon (REDFLAG, gametype.iconRedFlagCarried);
                            #endif // _GOLD
                            return 1;
                        }else if(arg2 == TEAM_RED && g_ctfClassic.integer
                            && gametype.redFlagDropTime)
                        {
                            // Boe!Man 2/1/13: Include touch-flag
                            // (classic) CTF mode.
                            G_Broadcast(va("%s\nhas \\returned the %s ^7flag!",
                                g_entities[arg1].client->pers.netname,
                                server_redteamprefix.string),
                                BROADCAST_GAME, NULL);

                            trap_SendServerCommand(-1,
                                va("print\"^3[CTF] ^7%s has returned the " \
                                "Red flag.\n\"",
                                g_entities[arg1].client->pers.cleanName));

                            item = BG_FindGametypeItemByID(ITEM_REDFLAG);
                            if(item){
                                G_ResetGametypeItem(item);
                            }

                            // Boe!Man 11/29/12: Global sound.
                            if(!level.intermissionQueued
                                && !level.intermissiontime
                                && !level.awardTime)
                            {
                                gentity_t* tent;
                                tent = G_TempEntity(vec3_origin,
                                    EV_GLOBAL_SOUND);
                                tent->s.eventParm = gametype.flagReturnSound;
                                tent->r.svFlags = SVF_BROADCAST;
                            }
                            gametype.redFlagDropTime = 0;
                            gametype.flagTaken[REDFLAG] = qfalse;

                            #ifdef _GOLD
                            G_SetHUDIcon(REDFLAG, gametype.iconRedFlag);
                            #endif // _GOLD
                            return 0;
                        }
                        break;
                }
            }else if(current_gametype.value == GT_HS || current_gametype.value == GT_HZ){
                if(arg0 == ITEM_BRIEFCASE && arg2 == TEAM_BLUE){
                    return 1;
                }
            }
            #ifdef _GOLD
            else if(current_gametype.value == GT_DEM){
                if(arg0 == ITEM_BOMB && arg2 == TEAM_BLUE){
                    G_Broadcast(va("%s\nhas \\taken the bomb!",
                        g_entities[arg1].client->pers.netname),
                        BROADCAST_GAME, NULL);

                    trap_SendServerCommand(-1, va("print\"^3[DEM] ^7%s has " \
                        "taken the bomb.\n\"",
                        g_entities[arg1].client->pers.cleanName));

                    if(G_showItemLocations()){
                        Q_strncpyz(level.objectiveLoc,
                            g_entities[arg1].client->pers.netname,
                            sizeof(level.objectiveLoc));
                    }

                    // Boe!Man 11/29/12: Global sound.
                    if(!level.intermissionQueued
                        && !level.intermissiontime && !level.awardTime)
                    {
                        gentity_t* tent;
                        tent = G_TempEntity(vec3_origin, EV_GLOBAL_SOUND);
                        tent->s.eventParm = gametype.bombTakenSound;
                        tent->r.svFlags = SVF_BROADCAST;
                    }

                    // Boe!Man 11/29/12: Radio message.
                    G_Voice(&g_entities[arg1], NULL, SAY_TEAM,
                        "got_it", qfalse);

                    return 1;
                }

                return 0;
            }
            #endif // _GOLD

            break;

        case GTEV_TRIGGER_TOUCHED:
            if(current_gametype.value == GT_INF){
                switch ( arg0 )
                {
                    case TRIGGER_EXTRACTION:
                    {
                        gitem_t*    item;
                        gentity_t*  ent;

                        // Boe!Man 12/2/14: No capture on caserun.
                        if (g_caserun.integer){
                            return 1;
                        }

                        ent  = &g_entities[arg1];
                        item = BG_FindGametypeItemByID ( ITEM_BRIEFCASE );

                        if ( item )
                        {
                            if ( ent->client->ps.stats[STAT_GAMETYPE_ITEMS] & (1<<item->giTag) )
                            {
                                G_Broadcast(va("%s\nhas \\captured the briefcase!", g_entities[arg1].client->pers.netname), BROADCAST_GAME, NULL);
                                trap_SendServerCommand( -1, va("print\"^3[INF] ^7%s has captured the briefcase.\n\"", g_entities[arg1].client->pers.cleanName));
                                g_entities[arg1].client->pers.statinfo.itemCaptures++;

                                // Boe!Man 11/29/12: Global sound.
                                if(!level.intermissionQueued && !level.intermissiontime && !level.awardTime){
                                gentity_t* tent;
                                tent = G_TempEntity( vec3_origin, EV_GLOBAL_SOUND );
                                tent->s.eventParm = gametype.caseCaptureSound;
                                tent->r.svFlags = SVF_BROADCAST;
                                }

                                G_AddTeamScore ((team_t) arg2, 1);
                                if ( !gt_simpleScoring.integer )
                                {
                                    G_AddScore ( &g_entities[arg1], 10 );
                                }

                                // Boe!Man 11/29/12: Reset gametype.
                                level.gametypeResetTime = level.time + 5000;

                                // Is the timelimit hit already?
                                if(level.timelimithit){
                                    gentity_t*  tent;
                                    tent = G_TempEntity( vec3_origin, EV_GAME_OVER );
                                    tent->s.eventParm = GAME_OVER_TIMELIMIT;
                                    tent->r.svFlags = SVF_BROADCAST;
                                    level.timelimithit = qfalse;
                                    LogExit("Timelimit hit.");
                                }
                            }
                        }
                        break;
                    }
                }
            }else if(current_gametype.value == GT_CTF){
                switch ( arg0 )
                {
                    gitem_t*    item;
                    gentity_t*  ent;

                    case TRIGGER_BLUECAPTURE:
                        ent  = &g_entities[arg1];
                        item = BG_FindGametypeItemByID ( ITEM_REDFLAG );

                        if ( item )
                        {
                            if ( ent->client->ps.stats[STAT_GAMETYPE_ITEMS] & (1<<item->giTag) && ((g_ctfClassic.integer == 2 && !gametype.flagTaken[BLUEFLAG]) || g_ctfClassic.integer != 2))
                            {
                                G_Broadcast(va("%s\nhas \\captured the %s ^7flag!", g_entities[arg1].client->pers.netname, server_redteamprefix.string), BROADCAST_GAME, NULL);
                                trap_SendServerCommand( -1, va("print\"^3[CTF] ^7%s has captured the Red flag.\n\"", g_entities[arg1].client->pers.cleanName));
                                g_entities[arg1].client->pers.statinfo.itemCaptures++;

                                // Boe!Man 11/29/12: Reset item.
                                G_ResetGametypeItem ( item );

                                // Boe!Man 11/29/12: Global sound.
                                if(!level.intermissionQueued && !level.intermissiontime && !level.awardTime){
                                gentity_t* tent;
                                tent = G_TempEntity( vec3_origin, EV_GLOBAL_SOUND );
                                tent->s.eventParm = gametype.flagCaptureSound;
                                tent->r.svFlags = SVF_BROADCAST;
                                }

                                G_AddTeamScore ((team_t) arg2, 1);
                                if ( !gt_simpleScoring.integer )
                                {
                                    G_AddScore ( &g_entities[arg1], 10 );
                                }
                                gametype.redFlagDropTime = 0;
                                gametype.flagTaken[REDFLAG] = qfalse;

                                #ifdef _GOLD
                                G_SetHUDIcon (REDFLAG, gametype.iconRedFlag);
                                #endif // _GOLD
                                return 1;
                            }
                        }
                        break;

                    case TRIGGER_REDCAPTURE:
                        ent  = &g_entities[arg1];
                        item = BG_FindGametypeItemByID ( ITEM_BLUEFLAG );

                        if ( item )
                        {
                            if ( ent->client->ps.stats[STAT_GAMETYPE_ITEMS] & (1<<item->giTag) && ((g_ctfClassic.integer == 2 && !gametype.flagTaken[REDFLAG]) || g_ctfClassic.integer != 2))
                            {
                                G_Broadcast(va("%s\nhas \\captured the %s ^7flag!", g_entities[arg1].client->pers.netname, server_blueteamprefix.string), BROADCAST_GAME, NULL);
                                trap_SendServerCommand( -1, va("print\"^3[CTF] ^7%s has captured the Blue flag.\n\"", g_entities[arg1].client->pers.cleanName));
                                g_entities[arg1].client->pers.statinfo.itemCaptures++;

                                // Boe!Man 11/29/12: Reset item.
                                G_ResetGametypeItem ( item );

                                // Boe!Man 11/29/12: Global sound.
                                if(!level.intermissionQueued && !level.intermissiontime && !level.awardTime){
                                gentity_t* tent;
                                tent = G_TempEntity( vec3_origin, EV_GLOBAL_SOUND );
                                tent->s.eventParm = gametype.flagCaptureSound;
                                tent->r.svFlags = SVF_BROADCAST;
                                }

                                G_AddTeamScore ((team_t) arg2, 1);
                                if ( !gt_simpleScoring.integer )
                                {
                                    G_AddScore ( &g_entities[arg1], 10 );
                                }
                                gametype.blueFlagDropTime = 0;
                                gametype.flagTaken[BLUEFLAG] = qfalse;

                                #ifdef _GOLD
                                G_SetHUDIcon (BLUEFLAG, gametype.iconBlueFlag);
                                #endif // _GOLD
                                return 1;
                            }
                        }
                        break;
                }
            }

            break;

        case GTEV_CLIENT_DEATH:
            if(current_gametype.value == GT_TDM){
                // arg0 = clientID;
                // arg1 = clientTeam;
                // arg2 = killerID
                // arg3 = killerTeam

                // If another client killed this guy and wasnt on the same team, then
                // augment the team score for the killer
                if ( arg2 != -1 && arg3 != arg1 )
                {
                    G_AddTeamScore ((team_t) arg3, 1);
                }
            }
            break;

        #ifdef _GOLD
        case GTEV_ITEM_CANBEUSED:
            if (current_gametype.value == GT_DEM && arg0 == ITEM_PLANTED_BOMB && arg2 == TEAM_RED){
				return 1;
			}

			return 0;

        case GTEV_TRIGGER_CANBEUSED:
            if(current_gametype.value == GT_DEM){
                gitem_t*	item;
                gentity_t*	ent;

                ent  = &g_entities[arg1];
                item = BG_FindGametypeItemByID ( ITEM_BOMB );

                if (item && ent->client->ps.stats[STAT_GAMETYPE_ITEMS] & (1<<item->giTag)){
                    return 1;
                }
            }

            break;

        case GTEV_ITEM_USED:
            if(current_gametype.value == GT_DEM){
                gitem_t *item;

                // Reset gametype item.
                item = BG_FindGametypeItemByID ( ITEM_PLANTED_BOMB );
                if ( item )
                {
                    G_ResetGametypeItem ( item );
                }

                gametype.bombPlantTime = 0;
                gametype.bombBeepTime = 0;

                G_AddTeamScore (TEAM_RED, 1);

                G_Broadcast(
                    va("%s\nhas \\defused the bomb!",
                    g_entities[arg1].client->pers.netname),
                    BROADCAST_GAME, NULL);

                trap_SendServerCommand(-1,
                    va("print\"^3[DEM] ^7%s has defused the bomb.\n\"",
                    g_entities[arg1].client->pers.cleanName));

                if(G_showItemLocations()){
                    Q_strncpyz(level.objectiveLoc, "Defused",
                        sizeof(level.objectiveLoc));
                }

                // Boe!Man 11/29/12: Global sound.
                if(!level.intermissionQueued
                    && !level.intermissiontime
                    && !level.awardTime)
                {
                    gentity_t* tent;
                    tent = G_TempEntity(vec3_origin, EV_GLOBAL_SOUND);
                    tent->s.eventParm = gametype.bombExplodedSound;
                    tent->r.svFlags = SVF_BROADCAST;
                }

                level.gametypeResetTime = level.time + 5000;
                gametype.roundOver = qtrue;

                // Give the guy who defused it some props
                if(!gt_simpleScoring.integer){
                    G_AddScore(&g_entities[arg1], 10);
                }

                // Is the timelimit hit already?
                if(level.timelimithit){
                    gentity_t*  tent;
                    tent = G_TempEntity( vec3_origin, EV_GAME_OVER );
                    tent->s.eventParm = GAME_OVER_TIMELIMIT;
                    tent->r.svFlags = SVF_BROADCAST;
                    level.timelimithit = qfalse;
                    LogExit("Timelimit hit.");
                }

                return 1;
            }

            break;

        case GTEV_TRIGGER_USED:
            if(current_gametype.value == GT_DEM){
                gitem_t     *item;
                gentity_t	*find;

                gametype.bombPlantTime = time + gt_bombFuseTime.integer * 1000;
                gametype.bombPlantClient = arg1;

                // Set the bomb plant origin.
                VectorCopy (g_entities[arg1].client->ps.origin, gametype.bombPlantOrigin);

                // Take the bomb away from the client.
                item = BG_FindGametypeItemByID ( ITEM_BOMB );
                if ( item ){
                    level.clients[arg1].ps.stats[STAT_GAMETYPE_ITEMS] &= ~(1<<item->giTag);
                }

                // Spawn the bomb.
                item = BG_FindGametypeItemByID ( ITEM_PLANTED_BOMB );
                if ( item )
                {
                    gentity_t* ent = LaunchItem ( item, gametype.bombPlantOrigin, vec3_origin);
                    if ( ent )
                    {
                        VectorCopy (vec3_origin, ent->s.angles );
                    }
                }

                // Broadcast the change.
                G_Broadcast(va("%s\nhas \\planted the bomb!",
                    g_entities[arg1].client->pers.netname),
                    BROADCAST_GAME, NULL);

                trap_SendServerCommand(-1,
                    va("print\"^3[DEM] ^7%s has planted the bomb.\n\"",
                    g_entities[arg1].client->pers.cleanName));

                if(G_showItemLocations()){
                    Q_strncpyz(level.objectiveLoc, "Planted",
                        sizeof(level.objectiveLoc));
                }

                // Get the trigger target.
                Com_sprintf(gametype.bombPlantTarget,
                    sizeof(gametype.bombPlantTarget), "");
                find = NULL;
                while(NULL != (find = G_Find (find, FOFS(classname),
                    "gametype_trigger")))
                {
                    if(find->health == arg0){
                        Com_sprintf(gametype.bombPlantTarget,
                            sizeof(gametype.bombPlantTarget), "%s",
                            find->target);
                        break;
                    }
                }

                // Set the HUD icon.
                G_SetHUDIcon(0, gametype.iconBombPlanted[0]);

                // Start global sound.
                if(!level.intermissionQueued
                    && !level.intermissiontime && !level.awardTime)
                {
                    gentity_t* tent;
                    tent = G_TempEntity( vec3_origin, EV_GLOBAL_SOUND );
                    tent->s.eventParm = gametype.bombPlantedSound;
                    tent->r.svFlags = SVF_BROADCAST;
                }

                return 0;
            }

            break;
        #endif // _GOLD

        default:
            break;

        return 0;
    }

    return 0;
}
