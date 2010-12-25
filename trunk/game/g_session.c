#include "g_local.h"

/*
=======================================================================
  SESSION DATA
Session data is the only data that stays persistant across level loads
and map restarts.
=======================================================================
*/

/*
================
G_WriteClientSessionData

Called on game shutdown
================
*/
void G_WriteClientSessionData( gclient_t *client ) 
{
	const char	*s;
	const char	*var;
	//Ryan june 7 2003
//RxCxW - #ClanList - Modded From RPM GOLD - 1.04.2005
// We want to save our special status so we dont have to check again when the map starts
//RM	s = va("%i %i %i", client->sess.team, client->sess.admin, client->sess.referee );
	s = va("%i %i %i %i", client->sess.team, client->sess.admin, client->sess.referee, client->sess.clanMember);
	//s = va("%i", client->sess.team );

	//Ryan

	var = va( "session%i", client - level.clients );

	trap_Cvar_Set( var, s );
}

/*
================
G_ReadSessionData

Called on a reconnect
================
*/
void G_ReadSessionData( gclient_t *client ) 
{
	char		s[MAX_STRING_CHARS];
	const char	*var;
	int			sessionTeam;
	///Ryan june 7 2003
	int			adminSess;
	int			refSess;
	///RxCxW - #ClanList - 1.04.2005
	int			clanSess;
	///End
	///Ryan

	var = va( "session%i", client - level.clients );
	trap_Cvar_VariableStringBuffer( var, s, sizeof(s) );

	///Ryan june 7 2003
	///sscanf( s, "%i %i %i", &sessionTeam, &adminSess, &refSess );
	sscanf( s, "%i %i %i %i", &sessionTeam, &adminSess, &refSess, &clanSess); //RxCxW - 04.2005 #Clan
	///Ryan

	/// bk001205 - format issues
	client->sess.team = (team_t)sessionTeam;
	///Ryan june 7 2003
	client->sess.admin = adminSess;
	client->sess.referee = refSess;
	client->sess.clanMember = clanSess;		//RxCxW - 1.04.2005 - #ClanList
}


/*
================
G_InitSessionData

Called on a first-time connect
================
*/
void G_InitSessionData( gclient_t *client, char *userinfo ) 
{
	clientSession_t	*sess;
	const char		*value;
	

	sess = &client->sess;

	// initial team determination
	if ( level.gametypeData->teams ) 
	{
		if ( g_teamAutoJoin.integer ) 
		{
			sess->team = PickTeam( -1 );
			//Ryan We'll do this later 
			//BroadcastTeamChange( client, -1 );
			//Ryan
		} 
		else 
		{
			// always spawn as spectator in team games
			sess->team = TEAM_SPECTATOR;	
		}
	} 
	else 
	{
		value = Info_ValueForKey( userinfo, "team" );
		if ( value[0] == 's' ) 
		{
			// a willing spectator, not a waiting-in-line
			sess->team = TEAM_SPECTATOR;
		} 
		else 
		{
			if ( g_maxGameClients.integer > 0 && level.numNonSpectatorClients >= g_maxGameClients.integer ) 
			{
				sess->team = TEAM_SPECTATOR;
			} 
			else 
			{
				sess->team = TEAM_FREE;
			}
		}
	}

	sess->spectatorState = SPECTATOR_FREE;
	sess->spectatorTime = level.time;

	G_WriteClientSessionData( client );
}


/*
==================
G_InitWorldSession

==================
*/
void G_InitWorldSession( void ) 
{
	char	s[MAX_STRING_CHARS];
	int		gt;

	trap_Cvar_VariableStringBuffer( "session", s, sizeof(s) );
	
	gt = BG_FindGametype ( s );
	
	// if the gametype changed since the last session, don't use any
	// client sessions
	if ( level.gametype != gt ) 
	{
		level.newSession = qtrue;
		Com_Printf( "Gametype changed, clearing session data.\n" );
	}
}

/*
==================
G_WriteSessionData
==================
*/
void G_WriteSessionData( void ) 
{
	int		i;

	if(level.gametypeData)
	trap_Cvar_Set( "session", level.gametypeData->name );

	for ( i = 0 ; i < level.maxclients ; i++ ) 
	{
		if ( level.clients[i].pers.connected == CON_CONNECTED ) 
		{
			G_WriteClientSessionData( &level.clients[i] );
		}
	}
}
