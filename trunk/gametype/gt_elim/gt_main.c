// Copyright (C) 2001-2002 Raven Software.
//

#include "gt_local.h"

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

vmCvar_t	gt_flagReturnTime;;

static cvarTable_t gametypeCvarTable[] = 
{
	{ NULL, NULL, NULL, 0, 0.0f, 0.0f, 0, qfalse },
};

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
================
*/
void GT_Init ( void )
{
	memset ( &gametype, 0, sizeof(gametype) );

	// Boe!Man 4/2/11: Register the global sounds.
	gametype.captureSound = trap_Cmd_RegisterGlobalSound ( "sound/ctf_win.mp3" );

	// Register all cvars for this gametype
	GT_RegisterCvars ( );
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
	switch ( cmd )
	{
		case GTEV_TEAM_ELIMINATED:
			switch ( arg0 )
			{
				case TEAM_RED:
					trap_Cvar_VariableStringBuffer ( "server_redteamprefix", msg, 19 );
					trap_Cvar_VariableStringBuffer ( "server_color1", color1, 4 );
					trap_Cvar_VariableStringBuffer ( "server_color2", color2, 4 );
					trap_Cvar_VariableStringBuffer ( "server_color3", color3, 4 );
					trap_Cvar_VariableStringBuffer ( "server_color4", color4, 4 );
					trap_Cvar_VariableStringBuffer ( "server_color5", color5, 4 );
					trap_Cvar_VariableStringBuffer ( "server_color6", color6, 4 );
					trap_Cmd_TextMessage ( -1, va("@%s ^7team %se%sl%si%sm%si%snated!", msg, color1, color2, color3, color4, color5, color6 )); // red team eliminated
					trap_Cmd_TextMessage( -1, va("^3[ELIM] ^7Red team eliminated\n\""));
					trap_Cmd_AddTeamScore ( TEAM_BLUE, 1 );
					trap_Cmd_StartGlobalSound ( gametype.captureSound );
					trap_Cmd_Restart ( 5 );
					break;

				case TEAM_BLUE:
					trap_Cvar_VariableStringBuffer ( "server_blueteamprefix", msg, 19 );
					trap_Cvar_VariableStringBuffer ( "server_color1", color1, 4 );
					trap_Cvar_VariableStringBuffer ( "server_color2", color2, 4 );
					trap_Cvar_VariableStringBuffer ( "server_color3", color3, 4 );
					trap_Cvar_VariableStringBuffer ( "server_color4", color4, 4 );
					trap_Cvar_VariableStringBuffer ( "server_color5", color5, 4 );
					trap_Cvar_VariableStringBuffer ( "server_color6", color6, 4 );
					trap_Cmd_TextMessage ( -1, va("@%s ^7team %se%sl%si%sm%si%snated!", msg, color1, color2, color3, color4, color5, color6 )); // blue team eliminated
					trap_Cmd_TextMessage( -1, va("^3[ELIM] ^7Blue team eliminated\n\""));
					trap_Cmd_AddTeamScore ( TEAM_RED, 1 );
					trap_Cmd_StartGlobalSound ( gametype.captureSound );
					trap_Cmd_Restart ( 5 );
					break;
			}
			break;

		case GTEV_TIME_EXPIRED:
			trap_Cvar_VariableStringBuffer ( "server_color1", color1, 4 );
			trap_Cvar_VariableStringBuffer ( "server_color2", color2, 4 );
			trap_Cvar_VariableStringBuffer ( "server_color3", color3, 4 );
			trap_Cvar_VariableStringBuffer ( "server_color4", color4, 4 );
			trap_Cvar_VariableStringBuffer ( "server_color5", color5, 4 );
			trap_Cvar_VariableStringBuffer ( "server_color6", color6, 4 );
			trap_Cmd_TextMessage ( -1, va("@%sR%so%su%sn%sd %sD%sraw!", color1, color2, color3, color4, color5, color5, color6 )); // round draw
			trap_Cmd_TextMessage( -1, va("^3[ELIM] ^7Round Draw\n\""));
			trap_Cmd_Restart ( 5 );
			break;
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
