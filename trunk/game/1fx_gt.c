// Copyright (C) 2002-2012 - Raven Software, Boe!Man, Henkie.
//
// 1fx_gt.c - All the functions, declarations and commands for the Gametype Modules go here.

//==================================================================

#include "g_local.h"
#include "boe_local.h"

#include "1fx_gt.h"

void	GT_Init		( void );
void	GT_RunFrame	( int time );
int		GT_Event	( int cmd, int time, int arg0, int arg1, int arg2, int arg3, int arg4 );

gametypeLocals_t	gametype;

typedef struct 
{
	vmCvar_t	*vmCvar;
	char		*cvarName;
	char		*defaultString;
	int			cvarFlags;
	float		mMinValue, mMaxValue;
	int			modificationCount;  // for tracking changes
	qboolean	trackChange;	    // track this variable, and announce if changed
	qboolean	teamShader;			// track and if changed, update shader state

} cvarTable_t;

static cvarTable_t gametypeCvarTable[] = 
{
	{ &gt_simpleScoring,	"gt_simpleScoring",		"0",  CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse }, // INF, CTF
	{ &gt_flagReturnTime,	"gt_flagReturnTime",	"30", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse }, // CTF

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
	cvarTable_t	*cv;

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
	cvarTable_t	*cv;

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
	gtItemDef_t		itemDef;
	gtTriggerDef_t	triggerDef;
	gitem_t*		item;
	gentity_t*		find;

	memset ( &gametype, 0, sizeof(gametype) );

	// Register all cvars for this gametype
	GT_RegisterCvars ();

	// Boe!Man 11/30/12: Register the global sounds per gametype.
	if(current_gametype.value == GT_INF){
		gametype.caseTakenSound   = G_SoundIndex ("sound/ctf_flag.mp3");
		gametype.caseCaptureSound = G_SoundIndex ("sound/ctf_win.mp3");
		gametype.caseReturnSound  = G_SoundIndex ("sound/ctf_return.mp3");
	}else if(current_gametype.value == GT_CTF){
		gametype.flagTakenSound   = G_SoundIndex ("sound/ctf_flag.mp3");
		gametype.flagCaptureSound = G_SoundIndex ("sound/ctf_win.mp3");
		gametype.flagReturnSound  = G_SoundIndex ("sound/ctf_return.mp3");
	}else if(current_gametype.value == GT_ELIM){
		gametype.captureSound	  = G_SoundIndex ( "sound/ctf_win.mp3" );
	}else if(current_gametype.value == GT_HS || current_gametype.value == GT_HZ){
		gametype.caseCaptureSound = G_SoundIndex ("sound/ctf_win.mp3");
	}

	// Register the items
	memset ( &itemDef, 0, sizeof(itemDef) );
	
	// Boe!Man 11/29/12: Register items per gametype.
	if(current_gametype.value == GT_INF || current_gametype.value == GT_HS){
		item = BG_FindItem ("briefcase");
		if (item){
			item->quantity = ITEM_BRIEFCASE;
		}
	}else if(current_gametype.value == GT_CTF){
		item = BG_FindItem ("red_flag");
		if (item){
			item->quantity = ITEM_REDFLAG;
		}
		
		item = NULL;
		item = BG_FindItem ("blue_flag");
		if (item){
			item->quantity = ITEM_BLUEFLAG;
		}
	}

	// Register the triggers
	memset ( &triggerDef, 0, sizeof(triggerDef) );
	
	// Boe!Man 11/29/12: Register triggers per gametype.
	find = NULL;
	if(current_gametype.value == GT_INF){
		while ( NULL != (find = G_Find ( find, FOFS(classname), "gametype_trigger" ) ) )
		{
			if ( Q_stricmp ( find->targetname, (const char*) "briefcase_destination"))
			{
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
			if ( Q_stricmp ( find->targetname, (const char*) "red_capture_point"))
			{
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
			if ( Q_stricmp ( find->targetname, (const char*) "blue_capture_point"))
			{
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
			if(current_gametype.value == GT_INF || current_gametype.value == GT_CTF){
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
			break;

		case GTEV_TIME_EXPIRED:
			if(current_gametype.value == GT_INF){
				G_Broadcast(va("%s ^7team\n\\defended the briefcase!", server_redteamprefix.string), BROADCAST_GAME, NULL);
				trap_SendServerCommand( -1, va("print\"^3[INF] ^7Red team defended the briefcase.\n\""));
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
						break;
					case ITEM_REDFLAG:
						G_Broadcast(va("%s\n\\dropped the %s ^7flag!", g_entities[arg1].client->pers.netname, server_redteamprefix.string), BROADCAST_GAME, NULL);
						gametype.redFlagDropTime = time;
						break;
					default:
						break;
				}
			}

			break;

		case GTEV_ITEM_TOUCHED:
			if(current_gametype.value == GT_INF){
				switch ( arg0 )
				{
					case ITEM_BRIEFCASE:
						if ( arg2 == TEAM_BLUE || g_caserun.integer )
						{
							G_Broadcast(va("%s\nhas \\taken the briefcase!", g_entities[arg1].client->pers.netname), BROADCAST_GAME, NULL);
							trap_SendServerCommand(-1, va("print\"^3[%s] %s ^7has taken the briefcase.\n\"", (g_caserun.integer) ? "CR" : "INF", g_entities[arg1].client->pers.netname));

							// Boe!Man 11/29/12: Global sound.
							if(!level.intermissionQueued && !level.intermissiontime && !level.awardTime){
								gentity_t* tent;
								tent = G_TempEntity( vec3_origin, EV_GLOBAL_SOUND );
								tent->s.eventParm = gametype.caseTakenSound;
								tent->r.svFlags = SVF_BROADCAST;
							}

							if (g_caserun.integer){
								g_entities[arg1].client->caserunHoldTime = level.time + 5000;
							}
							
							// Boe!Man 11/29/12: Radio message.
							G_Voice ( &g_entities[arg1], NULL, SAY_TEAM, "got_it", qfalse );
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
							G_Broadcast(va("%s\nhas \\taken the %s ^7flag!", g_entities[arg1].client->pers.netname, server_blueteamprefix.string), BROADCAST_GAME, NULL);
							trap_SendServerCommand(-1, va("print\"^3[CTF] %s ^7has taken the Blue flag.\n\"", g_entities[arg1].client->pers.netname));
							
							// Boe!Man 11/29/12: Global sound.
							if(!level.intermissionQueued && !level.intermissiontime && !level.awardTime){
							gentity_t* tent;
							tent = G_TempEntity( vec3_origin, EV_GLOBAL_SOUND );
							tent->s.eventParm = gametype.flagTakenSound;
							tent->r.svFlags = SVF_BROADCAST;
							}
							
							// Boe!Man 11/29/12: Radio message.
							G_Voice ( &g_entities[arg1], NULL, SAY_TEAM, "got_it", qfalse );
							gametype.blueFlagDropTime = 0;
							gametype.flagTaken[BLUEFLAG] = qtrue;
							return 1;
						}else if(arg2 == TEAM_BLUE && g_ctfClassic.integer && gametype.blueFlagDropTime){ // Boe!Man 2/1/13: Include touch-flag (classic) CTF mode.
							G_Broadcast(va("%s\nhas \\returned the %s ^7flag!", g_entities[arg1].client->pers.netname, server_blueteamprefix.string), BROADCAST_GAME, NULL);
							trap_SendServerCommand(-1, va("print\"^3[CTF] %s ^7has returned the Blue flag.\n\"", g_entities[arg1].client->pers.netname));
							
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
							return 0;
						}
						break;
					
					case ITEM_REDFLAG:
						if(arg2 == TEAM_BLUE)
						{
							G_Broadcast(va("%s\nhas \\taken the %s ^7flag!", g_entities[arg1].client->pers.netname, server_redteamprefix.string), BROADCAST_GAME, NULL);
							trap_SendServerCommand(-1, va("print\"^3[CTF] %s ^7has taken the Red flag.\n\"", g_entities[arg1].client->pers.netname));
							
							// Boe!Man 11/29/12: Global sound.
							if(!level.intermissionQueued && !level.intermissiontime && !level.awardTime){
							gentity_t* tent;
							tent = G_TempEntity( vec3_origin, EV_GLOBAL_SOUND );
							tent->s.eventParm = gametype.flagTakenSound;
							tent->r.svFlags = SVF_BROADCAST;
							}
							
							// Boe!Man 11/29/12: Radio message.
							G_Voice ( &g_entities[arg1], NULL, SAY_TEAM, "got_it", qfalse );
							gametype.redFlagDropTime = 0;
							gametype.flagTaken[REDFLAG] = qtrue;
							return 1;
						}else if(arg2 == TEAM_RED && g_ctfClassic.integer && gametype.redFlagDropTime){ // Boe!Man 2/1/13: Include touch-flag (classic) CTF mode.
							G_Broadcast(va("%s\nhas \\returned the %s ^7flag!", g_entities[arg1].client->pers.netname, server_redteamprefix.string), BROADCAST_GAME, NULL);
							trap_SendServerCommand(-1, va("print\"^3[CTF] %s ^7has returned the Red flag.\n\"", g_entities[arg1].client->pers.netname));
							
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
							return 0;
						}
						break;
				}
			}else if(current_gametype.value == GT_HS || current_gametype.value == GT_HZ){
				if(arg0 == ITEM_BRIEFCASE && arg2 == TEAM_BLUE){
					return 1;
				}
			}
			return 0;

		case GTEV_TRIGGER_TOUCHED:
			if(current_gametype.value == GT_INF){
				switch ( arg0 )
				{
					case TRIGGER_EXTRACTION:
					{
						gitem_t*	item;
						gentity_t*	ent;

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
								trap_SendServerCommand( -1, va("print\"^3[INF] %s ^7has captured the briefcase.\n\"", g_entities[arg1].client->pers.cleanName));
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
							}
						}
						break;
					}
				}
			}else if(current_gametype.value == GT_CTF){
				switch ( arg0 )
				{
					gitem_t*	item;
					gentity_t*	ent;
					
					case TRIGGER_BLUECAPTURE:
						ent  = &g_entities[arg1];
						item = BG_FindGametypeItemByID ( ITEM_REDFLAG );

						if ( item )
						{
							if ( ent->client->ps.stats[STAT_GAMETYPE_ITEMS] & (1<<item->giTag) && ((g_ctfClassic.integer == 2 && !gametype.flagTaken[BLUEFLAG]) || g_ctfClassic.integer != 2))
							{
								G_Broadcast(va("%s\nhas \\captured the %s ^7flag!", g_entities[arg1].client->pers.netname, server_redteamprefix.string), BROADCAST_GAME, NULL);
								trap_SendServerCommand( -1, va("print\"^3[CTF] %s ^7has captured the Red flag.\n\"", g_entities[arg1].client->pers.cleanName));
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
								trap_SendServerCommand( -1, va("print\"^3[CTF] %s ^7has captured the Blue flag.\n\"", g_entities[arg1].client->pers.cleanName));
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
								return 1;
							}
						}
						break;
				}
			}
			
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
		
		default:
			break;
			
		return 0;
	}

	return 0;
}
