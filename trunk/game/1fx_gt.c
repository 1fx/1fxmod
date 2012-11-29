// Copyright (C) 2002-2012 - Raven Software, Boe!Man, Henkie.
//
// 1fx_gt.c - All the functions, declarations and commands for the Gametype Modules go here.

//==================================================================

#include "g_local.h"
#include "boe_local.h"

#include "1fx_gt.h"

#define	ITEM_BRIEFCASE			100				
#define TRIGGER_EXTRACTION		200

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

vmCvar_t	gt_simpleScoring;

static cvarTable_t gametypeCvarTable[] = 
{
	{ &gt_simpleScoring,	"gt_simpleScoring",		"0",  CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },

	{ NULL, NULL, NULL, 0, 0.0f, 0.0f, 0, qfalse },
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

	memset ( &gametype, 0, sizeof(gametype) );

	// Register all cvars for this gametype
	GT_RegisterCvars ();

	// Register the global sounds
	gametype.caseTakenSound   = G_SoundIndex ("sound/ctf_flag.mp3");
	gametype.caseCaptureSound = G_SoundIndex ("sound/ctf_win.mp3");
	gametype.caseReturnSound  = G_SoundIndex ("sound/ctf_return.mp3");

	// Register the items
	memset ( &itemDef, 0, sizeof(itemDef) );
	
	// Boe!Man 11/29/12: Register item.
	gitem_t* item;

	item = BG_FindItem ("briefcase");
	if (item){
		item->quantity = ITEM_BRIEFCASE;
	}

	// Register the triggers
	memset ( &triggerDef, 0, sizeof(triggerDef) );
	
	// Boe!Man 11/29/12: Register trigger.
	gentity_t* find;

	find = NULL;
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
	
	// Boe!Man 11/29/12: Semi-debug, but we let the admin know the gt has been loaded.
	Com_Printf("Gametype initialized (%s).\n", g_gametype.string);
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
			if ( !gt_simpleScoring.integer )
			{
				G_AddScore ( &g_entities[arg1], 5);
			}
			return 0;

		case GTEV_ITEM_STUCK:
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,%s", level.time + 5000, va("@The Briefcase has %sr%se%st%su%sr%sned!", server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string)));
			trap_SendServerCommand( -1, va("print \"^3[INF] ^7The briefcase has returned.\n\""));
			
			// Boe!Man 11/29/12: Reset item.
			gitem_t* item;
			
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

		case GTEV_TEAM_ELIMINATED:
			switch ( arg0 )
			{
				case TEAM_RED:
					trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,%s", level.time + 5000, va("@%s ^7team %se%sl%si%sm%si%snated!", server_redteamprefix.string, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string))); // Red team eliminated.
					trap_SendServerCommand( -1, va("print\"^3[INF] ^7Red team eliminated.\n\""));
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
					trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,%s", level.time + 5000, va("@%s ^7team %se%sl%si%sm%si%snated!", server_blueteamprefix.string, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string))); // Blue team eliminated.
					trap_SendServerCommand( -1, va("print\"^3[INF] ^7Blue team eliminated.\n\""));
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

		case GTEV_TIME_EXPIRED:
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,%s", level.time + 5000, va("@%s ^7team has %sd%se%sf%se%sn%sded the briefcase!", server_redteamprefix.string, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string))); // Red team defended the briefcase.
			trap_SendServerCommand( -1, va("print\"^3[INF] ^7Red team has defended the briefcase.\n\""));
			G_AddTeamScore ((team_t) TEAM_RED, 1);
			
			// Boe!Man 11/29/12: Reset gametype.
			level.gametypeResetTime = level.time + 5000;
			break;

		case GTEV_ITEM_DROPPED:
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,%s", level.time + 5000, va("@%s ^7has %sd%sr%so%sp%sp%sed the briefcase!", g_entities[arg1].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string))); // Dropped.
			break;

		case GTEV_ITEM_TOUCHED:

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

			return 0;

		case GTEV_TRIGGER_TOUCHED:
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

			return 0;
	}

	return 0;
}
