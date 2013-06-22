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
			
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,%s", level.time + 5000, va("@The %s ^7Flag has %sr%se%st%su%sr%sned!", server_redteamprefix.string, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string)));
			trap_SendServerCommand( -1, va("print \"^3[CTF] ^7The Red Flag has returned.\n\""));
			
			// Boe!Man 11/29/12: Global sound.
			if(!level.intermissionQueued && !level.intermissiontime && !level.awardTime){
				gentity_t* tent;
				tent = G_TempEntity( vec3_origin, EV_GLOBAL_SOUND );
				tent->s.eventParm = gametype.flagReturnSound;
				tent->r.svFlags = SVF_BROADCAST;
			}
			
			gametype.redFlagDropTime = 0;
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
			
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,%s", level.time + 5000, va("@The %s ^7Flag has %sr%se%st%su%sr%sned!", server_blueteamprefix.string, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string)));
			trap_SendServerCommand( -1, va("print \"^3[CTF] ^7The Blue Flag has returned.\n\""));
			
			// Boe!Man 11/29/12: Global sound.
			if(!level.intermissionQueued && !level.intermissiontime && !level.awardTime){
				gentity_t* tent;
				tent = G_TempEntity( vec3_origin, EV_GLOBAL_SOUND );
				tent->s.eventParm = gametype.flagReturnSound;
				tent->r.svFlags = SVF_BROADCAST;
			}
			
			gametype.blueFlagDropTime = 0;
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
				gitem_t* item;
				
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,%s", level.time + 5000, va("@The Briefcase has %sr%se%st%su%sr%sned!", server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string)));
				trap_SendServerCommand( -1, va("print \"^3[INF] ^7The briefcase has returned.\n\""));
				
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
						trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,%s", level.time + 5000, va("@The %s ^7Flag has %sr%se%st%su%sr%sned!", server_redteamprefix.string, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string)));
						trap_SendServerCommand( -1, va("print \"^3[CTF] ^7The Red Flag has returned.\n\""));
						
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
						return 1;
						
					case ITEM_BLUEFLAG:
						trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,%s", level.time + 5000, va("@The %s ^7Flag has %sr%se%st%su%sr%sned!", server_blueteamprefix.string, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string)));
						trap_SendServerCommand( -1, va("print \"^3[CTF] ^7The Blue Flag has returned.\n\""));
						
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
						return 1;
				}
			}else if(current_gametype.value == GT_HS){
				trap_SendServerCommand( -1, va("print \"^3[H&S] ^7The briefcase has disappeared.\n\""));
				return 1;
			}
			
			break;

		case GTEV_TEAM_ELIMINATED:
			if(current_gametype.value == GT_INF || current_gametype.value == GT_ELIM){
				switch ( arg0 )
				{
					case TEAM_RED:
						trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,%s", level.time + 5000, va("@%s ^7team %se%sl%si%sm%si%snated!", server_redteamprefix.string, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string))); // Red team eliminated.
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
						trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,%s", level.time + 5000, va("@%s ^7team %se%sl%si%sm%si%snated!", server_blueteamprefix.string, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string))); // Blue team eliminated.
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
						trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,%s", level.time + 5000, va("@%s ^7won!", server_seekerteamprefix.string))); // Seekers won.
						trap_SendServerCommand( -1, va("print\"^3[H&S] ^7Seekers won the match.\n\""));
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
						trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,%s", level.time + 5000, va("@%s ^7won!", server_zombieteamprefix.string))); // Zombies won.
						trap_SendServerCommand( -1, va("print\"^3[H&Z] ^7Zombies won the match.\n\""));
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
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,%s", level.time + 5000, va("@%s ^7team has %sd%se%sf%se%sn%sded the briefcase!", server_redteamprefix.string, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string))); // Red team defended the briefcase.
				trap_SendServerCommand( -1, va("print\"^3[INF] ^7Red team has defended the briefcase.\n\""));
				G_AddTeamScore ((team_t) TEAM_RED, 1);
				
				// Boe!Man 11/29/12: Reset gametype.
				level.gametypeResetTime = level.time + 5000;
			}else if(current_gametype.value == GT_ELIM){
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,%s", level.time + 5000, va("@%sR%so%su%sn%sd %sDraw!", server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string))); // Round Draw.
				trap_SendServerCommand( -1, va("print\"^3[ELIM] ^7Round Draw.\n\""));
				// Boe!Man 11/29/12: Reset gametype.
				level.gametypeResetTime = level.time + 5000;
			}else if(current_gametype.value == GT_HS){
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,%s", level.time + 5000, va("@%s ^7won!", server_hiderteamprefix.string))); // Hiders won.
				trap_SendServerCommand( -1, va("print\"^3[H&S] ^7Hiders won the match.\n\""));
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
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,%s", level.time + 5000, va("@%s ^7won!", server_humanteamprefix.string))); // Humans won.
				trap_SendServerCommand( -1, va("print\"^3[H&Z] ^7Humans won the match.\n\""));
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
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,%s", level.time + 5000, va("@%s ^7has %sd%sr%so%sp%sp%sed the briefcase!", g_entities[arg1].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string))); // Dropped.
				break;
			}else if(current_gametype.value == GT_CTF){
				switch (arg0)
				{
					case ITEM_BLUEFLAG:
						trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,%s", level.time + 5000, va("@%s has %sd%sr%so%sp%sp%sed the %s ^7Flag!", g_entities[arg1].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, server_blueteamprefix.string))); // Dropped.
						gametype.blueFlagDropTime = time;
						break;
						
					case ITEM_REDFLAG:
						trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,%s", level.time + 5000, va("@%s has %sd%sr%so%sp%sp%sed the %s ^7Flag!", g_entities[arg1].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, server_redteamprefix.string))); // Dropped.
						gametype.redFlagDropTime = time;
						break;
				}
			}
			break;

		case GTEV_ITEM_TOUCHED:
			if(current_gametype.value == GT_INF){
				switch ( arg0 )
				{
					case ITEM_BRIEFCASE:
						if ( arg2 == TEAM_BLUE )
						{
							trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,%s", level.time + 5000, va("@%s ^7has %st%sa%sk%se%sn the briefcase!", g_entities[arg1].client->pers.netname, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string))); // Taken.
							trap_SendServerCommand(-1, va("print\"^3[INF] %s ^7has taken the briefcase.\n\"", g_entities[arg1].client->pers.netname));
							// Boe!Man 11/29/12: Global sound.
							if(!level.intermissionQueued && !level.intermissiontime && !level.awardTime){
							gentity_t* tent;
							tent = G_TempEntity( vec3_origin, EV_GLOBAL_SOUND );
							tent->s.eventParm = gametype.caseTakenSound;
							tent->r.svFlags = SVF_BROADCAST;
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
							trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,%s", level.time + 5000, va("@%s has %st%sa%sk%se%sn the %s ^7Flag!", g_entities[arg1].client->pers.netname, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, server_blueteamprefix.string))); // Taken.
							trap_SendServerCommand(-1, va("print\"^3[CTF] %s ^7has taken the Blue Flag.\n\"", g_entities[arg1].client->pers.netname));
							
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
							return 1;
						}else if(arg2 == TEAM_BLUE && g_ctfClassic.integer && gametype.blueFlagDropTime){ // Boe!Man 2/1/13: Include touch-flag (classic) CTF mode.
							trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,%s", level.time + 5000, va("@%s has %sr%se%st%su%sr%sned the %s ^7Flag!", g_entities[arg1].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, server_blueteamprefix.string))); // Return.
							trap_SendServerCommand(-1, va("print\"^3[CTF] %s ^7has returned the Blue Flag.\n\"", g_entities[arg1].client->pers.netname));
							
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
							return 0;
						}
						break;
					
					case ITEM_REDFLAG:
						if(arg2 == TEAM_BLUE)
						{
							trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,%s", level.time + 5000, va("@%s has %st%sa%sk%se%sn the %s ^7Flag!", g_entities[arg1].client->pers.netname, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, server_redteamprefix.string))); // Taken.
							trap_SendServerCommand(-1, va("print\"^3[CTF] %s ^7has taken the Red Flag.\n\"", g_entities[arg1].client->pers.netname));
							
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
							return 1;
						}else if(arg2 == TEAM_RED && g_ctfClassic.integer && gametype.redFlagDropTime){ // Boe!Man 2/1/13: Include touch-flag (classic) CTF mode.
							trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,%s", level.time + 5000, va("@%s has %sr%se%st%su%sr%sned the %s ^7Flag!", g_entities[arg1].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, server_redteamprefix.string))); // Return.
							trap_SendServerCommand(-1, va("print\"^3[CTF] %s ^7has returned the Red Flag.\n\"", g_entities[arg1].client->pers.netname));
							
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

						ent  = &g_entities[arg1];
						item = BG_FindGametypeItemByID ( ITEM_BRIEFCASE );

						if ( item )
						{
							if ( ent->client->ps.stats[STAT_GAMETYPE_ITEMS] & (1<<item->giTag) )
							{
								trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,%s", level.time + 5000, va("@%s ^7has %sc%sa%sp%st%su%sred the briefcase!", g_entities[arg1].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string)));
								trap_SendServerCommand( -1, va("print\"^3[INF] %s ^7has captured the briefcase.\n\"", g_entities[arg1].client->pers.netname));
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
							if ( ent->client->ps.stats[STAT_GAMETYPE_ITEMS] & (1<<item->giTag) )
							{
								trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,%s", level.time + 5000, va("@%s has %sc%sa%sp%st%su%sred the %s ^7Flag!", g_entities[arg1].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, server_redteamprefix.string)));
								trap_SendServerCommand( -1, va("print\"^3[CTF] %s ^7has captured the Red Flag.\n\"", g_entities[arg1].client->pers.netname));
								
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
								return 1;
							}
						}
						break;
						
					case TRIGGER_REDCAPTURE:
						ent  = &g_entities[arg1];
						item = BG_FindGametypeItemByID ( ITEM_BLUEFLAG );

						if ( item )
						{
							if ( ent->client->ps.stats[STAT_GAMETYPE_ITEMS] & (1<<item->giTag) )
							{
								trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,%s", level.time + 5000, va("@%s has %sc%sa%sp%st%su%sred the %s ^7Flag!", g_entities[arg1].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, server_blueteamprefix.string)));
								trap_SendServerCommand( -1, va("print\"^3[CTF] %s ^7has captured the Blue Flag.\n\"", g_entities[arg1].client->pers.netname));
								
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
