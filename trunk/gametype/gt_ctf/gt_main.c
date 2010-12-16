// Copyright (C) 2001-2002 Raven Software.
//

#include "gt_local.h"

#define	ITEM_REDFLAG			100
#define ITEM_BLUEFLAG			101
								
#define TRIGGER_REDCAPTURE		200
#define TRIGGER_BLUECAPTURE		201

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

vmCvar_t	gt_flagReturnTime;
vmCvar_t	gt_simpleScoring;

static cvarTable_t gametypeCvarTable[] = 
{
	// don't override the cheat state set by the system
	{ &gt_flagReturnTime,	"gt_flagReturnTime",	"30", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
	{ &gt_simpleScoring,	"gt_simpleScoring",		"0",  CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse },
};

static int gametypeCvarTableSize = sizeof( gametypeCvarTable ) / sizeof( gametypeCvarTable[0] );

/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .q3vm file
================
*/
int vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11 ) 
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
	int			i;
	cvarTable_t	*cv;

	for ( i = 0, cv = gametypeCvarTable ; i < gametypeCvarTableSize ; i++, cv++ ) 
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
	int			i;
	cvarTable_t	*cv;

	for ( i = 0, cv = gametypeCvarTable ; i < gametypeCvarTableSize ; i++, cv++ ) 
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
================
*/
void GT_Init ( void )
{
	gtItemDef_t		itemDef;
	gtTriggerDef_t	triggerDef;

	memset ( &gametype, 0, sizeof(gametype) );

	// Register the global sounds
	gametype.flagReturnSound  = trap_Cmd_RegisterGlobalSound ( "sound/ctf_return.mp3" );
	gametype.flagTakenSound   = trap_Cmd_RegisterGlobalSound ( "sound/ctf_flag.mp3" );
	gametype.flagCaptureSound = trap_Cmd_RegisterGlobalSound ( "sound/ctf_win.mp3" );

	// Register all cvars for this gametype
	GT_RegisterCvars ( );

	// Register the items
	memset ( &itemDef, 0, sizeof(itemDef) );
	trap_Cmd_RegisterItem ( ITEM_REDFLAG,  "red_flag", &itemDef );
	trap_Cmd_RegisterItem ( ITEM_BLUEFLAG, "blue_flag", &itemDef );

	// Register the triggers
	memset ( &triggerDef, 0, sizeof(triggerDef) );
	trap_Cmd_RegisterTrigger ( TRIGGER_REDCAPTURE, "red_capture_point", &triggerDef );
	trap_Cmd_RegisterTrigger ( TRIGGER_BLUECAPTURE, "blue_capture_point", &triggerDef );
}

/*
================
GT_RunFrame

Runs all thinking code for gametype
================
*/
void GT_RunFrame ( int time )
{
	char color1[5];
	char color2[5];
	char color3[5];
	char color4[5];
	char color5[5];
	char color6[5];
	char msg[20];
	gametype.time = time;

	// See if we need to return the red flag yet
	if ( gametype.redFlagDropTime && time - gametype.redFlagDropTime > gt_flagReturnTime.integer * 1000 )
	{
		trap_Cmd_ResetItem ( ITEM_REDFLAG );
		trap_Cvar_VariableStringBuffer ( "server_redteamprefix", msg, 19 );
		trap_Cvar_VariableStringBuffer ( "server_color1", color1, 4 );
		trap_Cvar_VariableStringBuffer ( "server_color2", color2, 4 );
		trap_Cvar_VariableStringBuffer ( "server_color3", color3, 4 );
		trap_Cvar_VariableStringBuffer ( "server_color4", color4, 4 );
		trap_Cvar_VariableStringBuffer ( "server_color5", color5, 4 );
		trap_Cvar_VariableStringBuffer ( "server_color6", color6, 4 );
		trap_Cmd_TextMessage ( -1, va("@The %s ^7Flag has %sr%se%st%su%sr%sned!", msg, color1, color2, color3, color4, color5, color6 ));
		trap_Cmd_TextMessage( -1, va("^3[CTF] ^7The Red Flag has returned\n\""));
		trap_Cmd_StartGlobalSound ( gametype.flagReturnSound );
		gametype.redFlagDropTime = 0;
		//trap_Cmd_ResetItem ( ITEM_REDFLAG );
		//trap_Cmd_TextMessage ( -1, "The Red Flag has returned!" );
		//trap_Cmd_StartGlobalSound ( gametype.flagReturnSound );
		//gametype.redFlagDropTime = 0;
	}

	// See if we need to return the blue flag yet
	if ( gametype.blueFlagDropTime && time - gametype.blueFlagDropTime > gt_flagReturnTime.integer * 1000 )
	{
		trap_Cmd_ResetItem ( ITEM_BLUEFLAG );
		trap_Cvar_VariableStringBuffer ( "server_blueteamprefix", msg, 19 );
		trap_Cvar_VariableStringBuffer ( "server_color1", color1, 4 );
		trap_Cvar_VariableStringBuffer ( "server_color2", color2, 4 );
		trap_Cvar_VariableStringBuffer ( "server_color3", color3, 4 );
		trap_Cvar_VariableStringBuffer ( "server_color4", color4, 4 );
		trap_Cvar_VariableStringBuffer ( "server_color5", color5, 4 );
		trap_Cvar_VariableStringBuffer ( "server_color6", color6, 4 );
		trap_Cmd_TextMessage ( -1, va("@The %s ^7Flag has %sr%se%st%su%sr%sned!", msg, color1, color2, color3, color4, color5, color6 ));
		trap_Cmd_TextMessage( -1, va("^3[CTF] ^7The Blue Flag has returned\n\""));
		trap_Cmd_StartGlobalSound ( gametype.flagReturnSound );
		gametype.blueFlagDropTime = 0;
		//trap_Cmd_ResetItem ( ITEM_BLUEFLAG );
		//trap_Cmd_TextMessage ( -1, "The Blue Flag has returned!" );
		//trap_Cmd_StartGlobalSound ( gametype.flagReturnSound );
		//gametype.blueFlagDropTime = 0;
	}

	GT_UpdateCvars ( );
}

/*
================
GT_Event

Handles all events sent to the gametype
================
*/
int GT_Event ( int cmd, int time, int arg0, int arg1, int arg2, int arg3, int arg4 )
{
	char color1[5];
	char color2[5];
	char color3[5];
	char color4[5];
	char color5[5];
	char color6[5];
	char msg[20];
	char clientname[MAX_QPATH];
	switch ( cmd )
	{
		case GTEV_ITEM_DEFEND:
			if ( !gt_simpleScoring.integer )
			{
				trap_Cmd_AddClientScore ( arg1, 5 );
			}
			return 0;

		case GTEV_ITEM_STUCK:
			switch ( arg0 )
			{
				case ITEM_REDFLAG:
					trap_Cmd_ResetItem ( ITEM_REDFLAG );
					trap_Cvar_VariableStringBuffer ( "server_redteamprefix", msg, 19 );
					trap_Cvar_VariableStringBuffer ( "server_color1", color1, 4 );
					trap_Cvar_VariableStringBuffer ( "server_color2", color2, 4 );
					trap_Cvar_VariableStringBuffer ( "server_color3", color3, 4 );
					trap_Cvar_VariableStringBuffer ( "server_color4", color4, 4 );
					trap_Cvar_VariableStringBuffer ( "server_color5", color5, 4 );
					trap_Cvar_VariableStringBuffer ( "server_color6", color6, 4 );
					trap_Cmd_TextMessage ( -1, va("@The %s ^7Flag has %sr%se%st%su%sr%sned!", msg, color1, color2, color3, color4, color5, color6 ));
					trap_Cmd_TextMessage( -1, va("^3[CTF] ^7The Red Flag has returned\n\""));
					trap_Cmd_StartGlobalSound ( gametype.flagReturnSound );
					gametype.redFlagDropTime = 0;
					return 1;

				case ITEM_BLUEFLAG:
					trap_Cmd_ResetItem ( ITEM_BLUEFLAG );
					trap_Cvar_VariableStringBuffer ( "server_blueteamprefix", msg, 19 );
					trap_Cvar_VariableStringBuffer ( "server_color1", color1, 4 );
					trap_Cvar_VariableStringBuffer ( "server_color2", color2, 4 );
					trap_Cvar_VariableStringBuffer ( "server_color3", color3, 4 );
					trap_Cvar_VariableStringBuffer ( "server_color4", color4, 4 );
					trap_Cvar_VariableStringBuffer ( "server_color5", color5, 4 );
					trap_Cvar_VariableStringBuffer ( "server_color6", color6, 4 );
					trap_Cmd_TextMessage ( -1, va("@The %s ^7Flag has %sr%se%st%su%sr%sned!", msg, color1, color2, color3, color4, color5, color6 ));
					trap_Cmd_TextMessage( -1, va("^3[CTF] ^7The Blue Flag has returned\n\""));
					trap_Cmd_StartGlobalSound ( gametype.flagReturnSound );
					gametype.blueFlagDropTime = 0;
					return 1;
			}
			
			break;

		case GTEV_ITEM_DROPPED:
		{
			char clientname[MAX_QPATH];

			trap_Cmd_GetClientName ( arg1, clientname, MAX_QPATH );

			switch ( arg0 )
			{
				case ITEM_BLUEFLAG:
					trap_Cvar_VariableStringBuffer ( "server_blueteamprefix", msg, 19 );
					trap_Cvar_VariableStringBuffer ( "server_color1", color1, 4 );
					trap_Cvar_VariableStringBuffer ( "server_color2", color2, 4 );
					trap_Cvar_VariableStringBuffer ( "server_color3", color3, 4 );
					trap_Cvar_VariableStringBuffer ( "server_color4", color4, 4 );
					trap_Cvar_VariableStringBuffer ( "server_color5", color5, 4 );
					trap_Cvar_VariableStringBuffer ( "server_color6", color6, 4 );
					trap_Cmd_TextMessage ( -1, va("@%s has %sd%sr%so%sp%sp%sed the %s ^7Flag!", clientname, color1, color2, color3, color4, color5, color6, msg ) );
					//trap_Cmd_TextMessage( -1, va("^3[CTF] %s ^7has dropped the Blue Flag\n\"", clientname));
					gametype.blueFlagDropTime = time;
					break;
				
				case ITEM_REDFLAG:
					trap_Cvar_VariableStringBuffer ( "server_redteamprefix", msg, 19 );
					trap_Cvar_VariableStringBuffer ( "server_color1", color1, 4 );
					trap_Cvar_VariableStringBuffer ( "server_color2", color2, 4 );
					trap_Cvar_VariableStringBuffer ( "server_color3", color3, 4 );
					trap_Cvar_VariableStringBuffer ( "server_color4", color4, 4 );
					trap_Cvar_VariableStringBuffer ( "server_color5", color5, 4 );
					trap_Cvar_VariableStringBuffer ( "server_color6", color6, 4 );
					trap_Cmd_TextMessage ( -1, va("@%s has %sd%sr%so%sp%sp%sed the %s ^7Flag!", clientname, color1, color2, color3, color4, color5, color6, msg ) );
					//trap_Cmd_TextMessage( -1, va("^3[CTF] %s ^7has dropped the Red Flag\n\"", clientname));
					gametype.redFlagDropTime = time;
					break;
			}
			break;
		}

		case GTEV_ITEM_TOUCHED:

			switch ( arg0 )
			{
				case ITEM_BLUEFLAG:
					if ( arg2 == TEAM_RED )
					{
						char clientname[MAX_QPATH];
						trap_Cmd_GetClientName ( arg1, clientname, MAX_QPATH );
						trap_Cvar_VariableStringBuffer ( "server_blueteamprefix", msg, 19 );
						trap_Cvar_VariableStringBuffer ( "server_color2", color2, 4 );
						trap_Cvar_VariableStringBuffer ( "server_color3", color3, 4 );
						trap_Cvar_VariableStringBuffer ( "server_color4", color4, 4 );
						trap_Cvar_VariableStringBuffer ( "server_color5", color5, 4 );
						trap_Cvar_VariableStringBuffer ( "server_color6", color6, 4 );
						trap_Cmd_TextMessage ( -1, va("@%s has %st%sa%sk%se%sn the %s ^7Flag!", clientname, color2, color3, color4, color5, color6, msg ) );
						trap_Cmd_TextMessage( -1, va("^3[CTF] %s ^7has taken the Blue Flag\n\"", clientname));
						trap_Cmd_StartGlobalSound ( gametype.flagTakenSound );
						trap_Cmd_RadioMessage ( arg1, "got_it" );
						gametype.blueFlagDropTime = 0;

						return 1;
					}
					break;

				case ITEM_REDFLAG:
					if ( arg2 == TEAM_BLUE )
					{
						char clientname[MAX_QPATH];
						trap_Cmd_GetClientName ( arg1, clientname, MAX_QPATH );
						trap_Cvar_VariableStringBuffer ( "server_redteamprefix", msg, 19 );
						trap_Cvar_VariableStringBuffer ( "server_color2", color2, 4 );
						trap_Cvar_VariableStringBuffer ( "server_color3", color3, 4 );
						trap_Cvar_VariableStringBuffer ( "server_color4", color4, 4 );
						trap_Cvar_VariableStringBuffer ( "server_color5", color5, 4 );
						trap_Cvar_VariableStringBuffer ( "server_color6", color6, 4 );
						trap_Cmd_TextMessage ( -1, va("@%s has %st%sa%sk%se%sn the %s ^7Flag!", clientname, color2, color3, color4, color5, color6, msg ) );
						trap_Cmd_TextMessage( -1, va("^3[CTF] %s ^7has taken the Red Flag\n\"", clientname));
						trap_Cmd_StartGlobalSound ( gametype.flagTakenSound );
						trap_Cmd_RadioMessage ( arg1, "got_it" );
						gametype.redFlagDropTime = 0;

						return 1;
					}
					break;
			}

			return 0;

		case GTEV_TRIGGER_TOUCHED:
			switch ( arg0 )
			{
				case TRIGGER_BLUECAPTURE:
					if ( trap_Cmd_DoesClientHaveItem ( arg1, ITEM_REDFLAG ) )
					{
						char clientname[MAX_QPATH];
						trap_Cmd_GetClientName ( arg1, clientname, MAX_QPATH );
						trap_Cvar_VariableStringBuffer ( "server_redteamprefix", msg, 19 );
						trap_Cvar_VariableStringBuffer ( "server_color1", color2, 4 );
						trap_Cvar_VariableStringBuffer ( "server_color2", color2, 4 );
						trap_Cvar_VariableStringBuffer ( "server_color3", color3, 4 );
						trap_Cvar_VariableStringBuffer ( "server_color4", color4, 4 );
						trap_Cvar_VariableStringBuffer ( "server_color5", color5, 4 );
						trap_Cvar_VariableStringBuffer ( "server_color6", color6, 4 );
						trap_Cmd_TextMessage ( -1, va("@%s has %sc%sa%sp%st%su%sred the %s ^7Flag!", clientname , color1, color2, color3, color4, color5, color6, msg) );
						trap_Cmd_TextMessage( -1, va("^3[CTF] %s ^7has captured the Red Flag\n\"", clientname));
						trap_Cmd_ResetItem ( ITEM_REDFLAG );
						trap_Cmd_StartGlobalSound ( gametype.flagCaptureSound );
						trap_Cmd_AddTeamScore ( arg2, 1 );

						if ( !gt_simpleScoring.integer )
						{
							trap_Cmd_AddClientScore ( arg1, 10 );
						}
						gametype.redFlagDropTime = 0;
						return 1;
					}
					break;

				case TRIGGER_REDCAPTURE:
					if ( trap_Cmd_DoesClientHaveItem ( arg1, ITEM_BLUEFLAG ) )
					{
						char clientname[MAX_QPATH];
						trap_Cmd_GetClientName ( arg1, clientname, MAX_QPATH );
						trap_Cvar_VariableStringBuffer ( "server_blueteamprefix", msg, 19 );
						trap_Cvar_VariableStringBuffer ( "server_color1", color2, 4 );
						trap_Cvar_VariableStringBuffer ( "server_color2", color2, 4 );
						trap_Cvar_VariableStringBuffer ( "server_color3", color3, 4 );
						trap_Cvar_VariableStringBuffer ( "server_color4", color4, 4 );
						trap_Cvar_VariableStringBuffer ( "server_color5", color5, 4 );
						trap_Cvar_VariableStringBuffer ( "server_color6", color6, 4 );
						trap_Cmd_TextMessage ( -1, va("@%s has %sc%sa%sp%st%su%sred the %s ^7Flag!", clientname , color1, color2, color3, color4, color5, color6, msg) );
						trap_Cmd_TextMessage( -1, va("^3[CTF] %s ^7has captured the Blue Flag\n\"", clientname));
						trap_Cmd_ResetItem ( ITEM_BLUEFLAG );
						trap_Cmd_StartGlobalSound ( gametype.flagCaptureSound );
						trap_Cmd_AddTeamScore ( arg2, 1 );

						if ( !gt_simpleScoring.integer )
						{
							trap_Cmd_AddClientScore ( arg1, 10 );
						}

						gametype.blueFlagDropTime = 0;
						return 1;
					}
					break;
			}

			return 0;
	}

	return 0;
}

#ifndef GAMETYPE_HARD_LINKED
// this is only here so the functions in q_shared.c and bg_*.c can link (FIXME)

void QDECL Com_Error( int level, const char *msg, ... ) 
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	trap_Error( text );
}

void QDECL Com_Printf( const char *msg, ... ) 
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	trap_Print( text );
}

#endif
