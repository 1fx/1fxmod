// Copyright (C) 2001-2002 Raven Software.
//
#include "g_local.h"
#include "boe_local.h"

#include "../../ui/menudef.h"

int AcceptBotCommand(char *cmd, gentity_t *pl);

void EvenTeams (gentity_t *adm)
{
	int			team, i, j, a =0, autoEven;
	int			bCount = 0, rCount = 0;
	qboolean	dead = qfalse;

	if(autoEven == 0)
		return;

	bCount = TeamCount(-1, TEAM_BLUE, NULL );
	rCount = TeamCount(-1, TEAM_RED, NULL );

	if(rCount - bCount > 1){
		a = (rCount - bCount)/2;
		team = TEAM_BLUE;
	}
	else if(bCount - rCount > 1){
		a = (bCount - rCount)/2;
		team = TEAM_RED;
	}
	else {
		if(autoEven == 2) return;
		else if(adm != NULL)	trap_SendServerCommand(adm-g_entities, va("print \"^3[Info] ^7Teams are as even as possible.\n\""));
		else Com_Printf("Teams are as even as possible.\n");
		return;
	}
	
	for(i = 0; i < a; i++){
		gentity_t	*tent = NULL, *ent = NULL;
		for ( j = 0 ; j < level.numConnectedClients ; j++ )	{
			ent = &g_entities[level.sortedClients[j]];
			if ( ent->client->pers.connected != CON_CONNECTED && ent->client->pers.connected != CON_CONNECTING ) continue;
			if(ent->client->sess.team != TEAM_RED && ent->client->sess.team != TEAM_BLUE) continue;
			if(ent->client->sess.team == team)	continue;
			if(ent->s.gametypeitems > 0) continue;
			if(ent->client->sess.admin) continue;
			tent = ent;
		} 

		if((!tent || !tent->client) ){
			if(autoEven == 2) return;
			if(adm != NULL)	trap_SendServerCommand(adm-g_entities, va("print \"^3[Info] ^7Teams cannot be evened [all admin or item holders].\n\""));
			else Com_Printf("Teams cannot be evened [all admin or item holders]\n");
			return;
		}
		if(!dead){
			tent->client->ps.stats[STAT_WEAPONS] = 0;
			TossClientItems( tent );
			G_StartGhosting( tent );
		}		
		if (tent->r.svFlags & SVF_BOT)	{
			char	userinfo[MAX_INFO_STRING];
			trap_GetUserinfo( tent->s.number, userinfo, sizeof( userinfo ) );
			Info_SetValueForKey( userinfo, "team", tent->client->sess.team == TEAM_RED?"blue":"red");
			trap_SetUserinfo( tent->s.number, userinfo );
		}

		if(team == TEAM_RED) tent->client->sess.team = TEAM_RED;
		else				 tent->client->sess.team = TEAM_BLUE;

		tent->client->pers.identity = NULL;

		ClientUserinfoChanged( tent->s.number );

		if(!dead){
			G_StopFollowing(tent);
			G_StopGhosting( tent );
			trap_UnlinkEntity ( tent );
			ClientSpawn (tent);
		}
	}

	// Boe!Man 3/31/10: We tell 'em what happened.
	Boe_GlobalSound (G_SoundIndex("sound/misc/events/tut_lift02.mp3"));
			
	if(adm && adm->client) {
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Eventeams by %s.\n\"", adm->client->pers.netname));
	} else	{
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7Eventeams.\n\""));
	}
}

/*
==================
DeathmatchScoreboardMessage
==================
*/
void DeathmatchScoreboardMessage( gentity_t *ent ) 
{
	char		entry[1024];
	char		string[1400];
	int			stringlength;
	int			i, j;
	gclient_t	*cl;
	int			numSorted;

	// send the latest information on all clients
	string[0]    = 0;
	stringlength = 0;

	numSorted = level.numConnectedClients;
	
	for (i=0 ; i < numSorted ; i++) 
	{
		int	ping;

		cl = &level.clients[level.sortedClients[i]];

		if ( cl->pers.connected == CON_CONNECTING ) 
		{
			ping = -1;
		} 
		else 
		{
			ping = cl->ps.ping < 999 ? cl->ps.ping : 999;
			if(ping != 999){
			ping = ping*0.5;
			}
		}
	
		Com_sprintf (entry, sizeof(entry),
			" %i %i %i %i %i %i %i %i %i", 
			level.sortedClients[i],
			cl->sess.score, 
			cl->sess.kills, 
			cl->sess.deaths, 
			ping, 
			(level.time - cl->pers.enterTime)/60000,
			(cl->sess.ghost || cl->ps.pm_type == PM_DEAD) ? qtrue : qfalse,
			g_entities[level.sortedClients[i]].s.gametypeitems,
			g_teamkillDamageMax.integer ? 100 * cl->sess.teamkillDamage / g_teamkillDamageMax.integer : 0
			);

		j = strlen(entry);
		if (stringlength + j > 1022 )
		{
			break;
		}

		strcpy (string + stringlength, entry);
		stringlength += j;
	}

	trap_SendServerCommand( ent-g_entities, va("scores %i %i %i%s", i, 
							level.teamScores[TEAM_RED], 
							level.teamScores[TEAM_BLUE],
							string ) );
}


/*
==================
Cmd_Score_f

Request current scoreboard information
==================
*/
void Cmd_Score_f( gentity_t *ent ) 
{
	DeathmatchScoreboardMessage( ent );
}

/*
==================
CheatsOk
==================
*/
qboolean	CheatsOk( gentity_t *ent ) {
	if ( !g_cheats.integer ) {
		trap_SendServerCommand( ent-g_entities, va("print \"Cheats are not enabled on this server.\n\""));
		return qfalse;
	}
	if ( ent->health <= 0 ) {
		trap_SendServerCommand( ent-g_entities, va("print \"You must be alive to use this command.\n\""));
		return qfalse;
	}
	return qtrue;
}


/*
==================
ConcatArgs
==================
*/
char	*ConcatArgs( int start ) {
	int		i, c, tlen;
	static char	line[MAX_STRING_CHARS];
	int		len;
	char	arg[MAX_STRING_CHARS];

	len = 0;
	c = trap_Argc();
	for ( i = start ; i < c ; i++ ) {
		trap_Argv( i, arg, sizeof( arg ) );
		tlen = strlen( arg );
		if ( len + tlen >= MAX_STRING_CHARS - 1 ) {
			break;
		}
		memcpy( line + len, arg, tlen );
		len += tlen;
		if ( i != c - 1 ) {
			line[len] = ' ';
			len++;
		}
	}

	line[len] = 0;

	return line;
}

/*
==================
SanitizeString

Remove case and control characters
==================
*/
void SanitizeString( char *in, char *out ) {
	while ( *in ) {
		if ( *in == 27 ) {
			in += 2;		// skip color code
			continue;
		}
		if ( *in < 32 ) {
			in++;
			continue;
		}
		*out++ = tolower( *in++ );
	}

	*out = 0;
}

/*
==================
G_ClientNumberFromName

Finds the client number of the client with the given name
==================
*/
int G_ClientNumberFromName ( const char* name )
{
	char		s2[MAX_STRING_CHARS];
	char		n2[MAX_STRING_CHARS];
	int			i;
	gclient_t*	cl;

	// check for a name match
	SanitizeString( (char*)name, s2 );
	for ( i=0, cl=level.clients ; i < level.numConnectedClients ; i++, cl++ ) 
	{
		SanitizeString( cl->pers.netname, n2 );
		if ( !strcmp( n2, s2 ) ) 
		{
			return i;
		}
	}

	return -1;
}

/*
==================
ClientNumberFromString

Returns a player number for either a number or name string
Returns -1 if invalid
==================
*/
int ClientNumberFromString( gentity_t *to, char *s ) {
	gclient_t	*cl;
	int			idnum;
	char		s2[MAX_STRING_CHARS];
	char		n2[MAX_STRING_CHARS];

	// numeric values are just slot numbers
	if (s[0] >= '0' && s[0] <= '9') {
		idnum = atoi( s );
		if ( idnum < 0 || idnum >= level.maxclients ) {
			trap_SendServerCommand( to-g_entities, va("print \"Bad client slot: %i\n\"", idnum));
			return -1;
		}

		cl = &level.clients[idnum];
		if ( cl->pers.connected != CON_CONNECTED ) {
			trap_SendServerCommand( to-g_entities, va("print \"Client %i is not active\n\"", idnum));
			return -1;
		}
		return idnum;
	}

	// check for a name match
	SanitizeString( s, s2 );
	for ( idnum=0,cl=level.clients ; idnum < level.maxclients ; idnum++,cl++ ) {
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		SanitizeString( cl->pers.netname, n2 );
		if ( !strcmp( n2, s2 ) ) {
			return idnum;
		}
	}

	trap_SendServerCommand( to-g_entities, va("print \"User %s is not on the server\n\"", s));
	return -1;
}

/*
==================
Cmd_Drop_f

Drops the currenty selected weapon
==================
*/
void Cmd_Drop_f ( gentity_t* ent )
{
	gentity_t* dropped;

	// spectators cant drop anything since they dont have anything
	if ( ent->client->sess.team == TEAM_SPECTATOR )
	{
		return;
	}

	// Ghosts and followers cant drop stuff
	if ( ent->client->ps.pm_flags & (PMF_GHOST|PMF_FOLLOW) )
	{
		return;
	}

	// Drop the weapon the client wanted to drop
	dropped = G_DropWeapon ( ent, atoi(ConcatArgs( 1 )), 3000 );
	if ( !dropped )
	{
		return;
	}
}

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f (gentity_t *ent)
{
	char		*name;
	gitem_t		*it;
	int			i;
	qboolean	give_all;
	gentity_t		*it_ent;
	trace_t		trace;
	char		arg[MAX_QPATH];

	int start;
	int end;
	int l;

	trap_Argv( 1, arg, sizeof( arg ) );

	if ( !Q_stricmp ( arg, "me" ) )
	{
		start = ent->s.number;
		end = start + 1;
	}
	else if ( !Q_stricmp ( arg, "all" ) )
	{
		start = 0;
		end   = MAX_CLIENTS;
	}
	else
	{
		start = atoi ( arg );
		end = start + 1;
	}

	for ( l = start; l < end; l ++ )
	{
		ent = &g_entities[l];

		if ( !ent->inuse )
		{
			continue;
		}

		if ( G_IsClientDead ( ent->client ) )
		{
			continue;
		}

		if ( !CheatsOk( ent ) ) {
			return;
		}

	name = ConcatArgs( 2 );

	if (Q_stricmp(name, "all") == 0)
		give_all = qtrue;
	else
		give_all = qfalse;

	if (give_all || Q_stricmp( name, "health") == 0)
	{
		ent->health = MAX_HEALTH;
		if (!give_all)
			continue;
	}

	if (give_all || Q_stricmp(name, "weapons") == 0)
	{
		ent->client->ps.stats[STAT_WEAPONS] = (1 << WP_NUM_WEAPONS) - 1 - ( 1 << WP_NONE );
		if (!give_all)
			continue;
	}

	if (give_all || Q_stricmp(name, "ammo") == 0)
	{
		for ( i = WP_NONE + 1 ; i < WP_NUM_WEAPONS ; i++ ) 
		{
			attackType_t a;
	
			for ( a = ATTACK_NORMAL; a < ATTACK_MAX; a ++ )
			{
				ent->client->ps.clip[a][i] = weaponData[i].attack[a].clipSize;
				ent->client->ps.ammo[weaponData[i].attack[a].ammoIndex] = ammoData[weaponData[i].attack[a].ammoIndex].max;
			}
		}

		if (!give_all)
			continue;
	}

	if (give_all || Q_stricmp(name, "armor") == 0)
	{
		ent->client->ps.stats[STAT_ARMOR] = MAX_ARMOR;

		if (!give_all)
			continue;
	}

	// spawn a specific item right on the player
	if ( !give_all ) 
	{
		it = BG_FindItem (name);
		if (!it) 
		{
			continue;
		}

		if ( it->giType == IT_GAMETYPE )
		{
			continue;
		}

		it_ent = G_Spawn();
		VectorCopy( ent->r.currentOrigin, it_ent->s.origin );
		it_ent->classname = it->classname;
		G_SpawnItem (it_ent, it);
		FinishSpawningItem(it_ent );
		memset( &trace, 0, sizeof( trace ) );
		Touch_Item (it_ent, ent, &trace);
		if (it_ent->inuse) 
		{
			G_FreeEntity( it_ent );
		}
	}

	}
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f (gentity_t *ent)
{
	char	*msg;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	ent->flags ^= FL_GODMODE;
	if (!(ent->flags & FL_GODMODE) )
		msg = "godmode OFF\n";
	else
		msg = "godmode ON\n";

	trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f( gentity_t *ent ) {
	char	*msg;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET) )
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f( gentity_t *ent ) {
	char	*msg;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	if ( ent->client->noclip ) {
		msg = "noclip OFF\n";
	} else {
		msg = "noclip ON\n";
	}
	ent->client->noclip = !ent->client->noclip;

	trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_LevelShot_f

This is just to help generate the level pictures
for the menus.  It goes to the intermission immediately
and sends over a command to the client to resize the view,
hide the scoreboard, and take a special screenshot
==================
*/
void Cmd_LevelShot_f( gentity_t *ent ) 
{
	if ( !CheatsOk( ent ) ) 
	{
		return;
	}

	BeginIntermission();

	trap_SendServerCommand( ent-g_entities, "clientLevelShot" );
}

/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f( gentity_t *ent ) 
{
	// No killing yourself if your a spectator
	if ( G_IsClientSpectating ( ent->client ) )
	{
		return;
	}

	// No killing yourself if your dead
	if ( G_IsClientDead ( ent->client ) ) 
	{
		return;
	}

	ent->flags &= ~FL_GODMODE;
	ent->client->ps.stats[STAT_HEALTH] = ent->health = -999;
	player_die (ent, ent, ent, 100000, MOD_SUICIDE, HL_NONE, vec3_origin );
}

/*
=================
BroadCastTeamChange

Let everyone know about a team change
=================
*/
void BroadcastTeamChange( gclient_t *client, int oldTeam )
{
	switch ( client->sess.team )
	{
		case TEAM_RED:
			trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " joined the red team.\n\"", client->pers.netname) );
			break;

		case TEAM_BLUE:
			trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " joined the blue team.\n\"", client->pers.netname));
			break;

		case TEAM_SPECTATOR:
			if ( oldTeam != TEAM_SPECTATOR )
			{
				trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " joined the spectators.\n\"", client->pers.netname));
			}
			break;

		case TEAM_FREE:
			trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " joined the battle.\n\"", client->pers.netname));
			break;
	}
}

/*
=================
SetTeam
=================
*/
void SetTeam( gentity_t *ent, char *s, const char* identity ) 
{
	int					team;
	int					oldTeam;
	gclient_t			*client;
	int					clientNum;
	spectatorState_t	specState;
	int					specClient;
	qboolean			ghost;
   	qboolean			noOutfittingChange = qfalse;

	// see what change is requested
	//
	client = ent->client;

	clientNum = client - level.clients;
	specClient = 0;
	specState = SPECTATOR_NOT;

	// If an identity was specified then inject it into
	// the clients userinfo
	if ( identity )
	{
		char userinfo[MAX_INFO_STRING];
		trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );

		if ( Q_stricmp ( identity, Info_ValueForKey ( userinfo, "identity" ) ) )
		{
			Info_SetValueForKey ( userinfo, "identity", identity );
			Info_SetValueForKey ( userinfo, "team_identity", identity );
			trap_SetUserinfo ( clientNum, userinfo );
		}
		else
		{
			identity = NULL;
		}
	}

	if ( !Q_stricmp( s, "follow1" ) ) 
	{
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FOLLOW;
		specClient = -1;
	} 
	else if ( !Q_stricmp( s, "follow2" ) ) 
	{
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FOLLOW;
		specClient = -2;
	} 
	else if ( !Q_stricmp( s, "spectator" ) || !Q_stricmp( s, "s" ) ) 
	{
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FREE;
	} 
	else if ( level.gametypeData->teams ) 
	{
		// if running a team game, assign player to one of the teams
		specState = SPECTATOR_NOT;
		if ( !Q_stricmp( s, "red" ) || !Q_stricmp( s, "r" ) ) 
		{
			team = TEAM_RED;
		} 
		else if ( !Q_stricmp( s, "blue" ) || !Q_stricmp( s, "b" ) ) 
		{
			team = TEAM_BLUE;
		} 
		else 
		{
			// pick the team with the least number of players
			team = PickTeam( clientNum );
		}

		if ( g_teamForceBalance.integer  ) 
		{
			int		counts[TEAM_NUM_TEAMS];

			counts[TEAM_BLUE] = TeamCount( ent->client->ps.clientNum, TEAM_BLUE, NULL );
			counts[TEAM_RED] = TeamCount( ent->client->ps.clientNum, TEAM_RED, NULL );

			// We allow a spread of two
			if ( team == TEAM_RED && counts[TEAM_RED] - counts[TEAM_BLUE] > 1 ) 
			{
				trap_SendServerCommand( ent->client->ps.clientNum, 
										"cp \"Red team has too many players.\n\"" );

				// ignore the request
				return; 
			}
			if ( team == TEAM_BLUE && counts[TEAM_BLUE] - counts[TEAM_RED] > 1 ) 
			{
				trap_SendServerCommand( ent->client->ps.clientNum, 
										"cp \"Blue team has too many players.\n\"" );

				// ignore the request
				return; 
			}

			// It's ok, the team we are switching to has less or same number of players
		}
	} 
	else 
	{
		// force them to spectators if there aren't any spots free
		team = TEAM_FREE;
	}

	// override decision if limiting the players
	if ( g_maxGameClients.integer > 0 && level.numNonSpectatorClients >= g_maxGameClients.integer ) 
	{
		team = TEAM_SPECTATOR;
	}

	// decide if we will allow the change
	oldTeam = client->sess.team;
	ghost   = client->sess.ghost;

	if ( team == oldTeam && team != TEAM_SPECTATOR )
	{
		if ( identity )
		{
			// get and distribute relevent paramters
			client->pers.identity = NULL;
			ClientUserinfoChanged( clientNum );
		}
		
		return;
	}

   	noOutfittingChange = ent->client->noOutfittingChange;

	// he starts at 'base'
	client->pers.teamState.state = TEAM_BEGIN;
	
	if ( oldTeam != TEAM_SPECTATOR ) 
	{
		if ( ghost )
		{
			G_StopGhosting ( ent );
		}
		else if ( !G_IsClientDead ( client ) )
		{
			// Kill him (makes sure he loses flags, etc)
			ent->flags &= ~FL_GODMODE;
			ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
			player_die (ent, ent, ent, 100000, MOD_TEAMCHANGE, HL_NONE, vec3_origin );

			ent->client->sess.ghost = qfalse;
		}
	}

	// If respawn interval start as a ghost
	if ( level.gametypeRespawnTime[ team ] )
	{
		ghost = qtrue;
	}

	// they go to the end of the line
	if ( team == TEAM_SPECTATOR ) 
	{
		client->sess.spectatorTime = level.time;
	}

	client->sess.team = team;
	client->sess.spectatorState = specState;
	client->sess.spectatorClient = specClient;

	// Always spawn into a ctf game using a respawn timer.
	if ( team != TEAM_SPECTATOR && level.gametypeData->respawnType == RT_INTERVAL )
	{
		G_SetRespawnTimer ( ent );
		ghost = qtrue;
	}

	BroadcastTeamChange( client, oldTeam );

	// See if we should spawn as a ghost
	if ( team != TEAM_SPECTATOR && level.gametypeData->respawnType == RT_NONE )
	{
		// If there are ghosts already then spawn as a ghost because
		// the game is already in progress.
		if ( (level.gametypeJoinTime && (level.time - level.gametypeJoinTime) > 20000) || noOutfittingChange || client->sess.noTeamChange )
		{
			ghost = qtrue;
		}

		// Spectator to a team doesnt count
		if ( oldTeam != TEAM_SPECTATOR )
		{
			client->sess.noTeamChange = qtrue;
		}
	}

	// If a ghost, enforce it
	if ( ghost )
	{
		// Make them a ghost again
		if ( team != TEAM_SPECTATOR )
		{
			G_StartGhosting ( ent );

			// get and distribute relevent paramters
			client->pers.identity = NULL;
			ClientUserinfoChanged( clientNum );

			CalculateRanks();

			return;
		}
	}	

	// get and distribute relevent paramters
	client->pers.identity = NULL;
	ClientUserinfoChanged( clientNum );

	CalculateRanks();

	// Begin the clients new life on the their new team
	ClientBegin( clientNum );
}

/*
=================
G_StartGhosting

Starts a client ghosting.  This essentially will kill a player which is alive
=================
*/
void G_StartGhosting ( gentity_t* ent )
{
	int i;

	// Dont start ghosting if already ghosting
	if ( ent->client->sess.ghost )
	{
		return;
	}

	ent->client->sess.ghost = qtrue;
	ent->client->sess.spectatorState = SPECTATOR_FREE;
	ent->client->sess.spectatorClient = -1;
	ent->client->ps.pm_flags |= PMF_GHOST;
	ent->client->ps.stats[STAT_HEALTH] = 100;
	ent->client->ps.pm_type = PM_SPECTATOR;
	ent->client->ps.pm_flags &= ~PMF_FOLLOW;

	trap_UnlinkEntity (ent);

	// stop any following clients
	for ( i = 0 ; i < level.maxclients ; i++ ) 
	{
		if ( G_IsClientSpectating ( &level.clients[i] )
			&& level.clients[i].sess.spectatorState == SPECTATOR_FOLLOW
			&& level.clients[i].sess.spectatorClient == ent->s.number ) 
		{
			G_StopFollowing( &g_entities[i] );
		}
	}
}

/*
=================
G_StopGhosting

Stops a client from ghosting.  The client will be dead after this
call
=================
*/
void G_StopGhosting ( gentity_t* ent )
{
	// Dont stop someone who isnt ghosting in the first place
	if ( !ent->client->sess.ghost )
	{
		return;
	}

	ent->client->sess.ghost = qfalse;
	ent->client->ps.pm_flags &= ~PMF_GHOST;
	ent->client->ps.pm_flags &= ~PMF_FOLLOW;

	if ( ent->client->sess.team == TEAM_SPECTATOR )
	{
		ent->client->ps.pm_type = PM_SPECTATOR;
	}
	else
	{
		ent->client->ps.pm_type = PM_DEAD;
		ent->health = ent->client->ps.stats[STAT_HEALTH] = 0;
	}
}

/*
=================
G_StopFollowing

If the client being followed leaves the game, or you just want to drop
to free floating spectator mode
=================
*/
void G_StopFollowing( gentity_t *ent ) 
{
	// Cant stop following if not following in the first place
	if ( !(ent->client->ps.pm_flags&PMF_FOLLOW) )
	{
		return;
	}

	// Clear the following variables
	ent->client->ps.pm_flags &= ~PMF_FOLLOW;
	ent->client->sess.spectatorState = SPECTATOR_FREE;
	ent->client->ps.clientNum = ent - g_entities;
	ent->client->ps.zoomFov = 0;
	ent->client->ps.pm_flags &= ~(PMF_GOGGLES_ON|PMF_ZOOM_FLAGS);
	ent->client->ps.persistant[PERS_TEAM] = ent->client->sess.team;
	ent->r.svFlags &= ~SVF_BOT;

	// Ghots dont really become spectators, just psuedo spectators
	if ( ent->client->sess.ghost )
	{
		// Do a start and stop to ensure the variables are all set properly
		G_StopGhosting ( ent );
		G_StartGhosting ( ent );
	}
	else
	{
		ent->client->sess.team = TEAM_SPECTATOR;	
		ent->client->ps.persistant[ PERS_TEAM ] = TEAM_SPECTATOR;	
	}

	// If we were in fact following someone, then make the angles and origin nice for
	// when we stop
   	if ( ent->client->sess.spectatorClient != -1 )
   	{
   		gclient_t* cl = &level.clients[ent->client->sess.spectatorClient];
    
   		int i;
   		for ( i = 0; i < 3; i ++ )
		{
   			ent->client->ps.delta_angles[i] = ANGLE2SHORT(cl->ps.viewangles[i] - SHORT2ANGLE(ent->client->pers.cmd.angles[i]));
		}
    
   		VectorCopy ( cl->ps.viewangles, ent->client->ps.viewangles );
   		VectorCopy ( cl->ps.origin, ent->client->ps.origin );
   		VectorClear ( ent->client->ps.velocity );
   		ent->client->ps.movementDir = 0;
    
   		BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qtrue );
   	}		

	ent->client->sess.spectatorClient = -1;
}

/*
=================
Cmd_Team_f
=================
*/
void Cmd_Team_f( gentity_t *ent ) 
{
	char team[MAX_TOKEN_CHARS];
	char identity[MAX_TOKEN_CHARS];

	// Need at least the team specified in the arguments
	if ( trap_Argc() < 2 ) 
	{
		int oldTeam = ent->client->sess.team;
		switch ( oldTeam ) 
		{
			case TEAM_BLUE:
				trap_SendServerCommand( ent-g_entities, "print \"Blue team\n\"" );
				break;
			
			case TEAM_RED:
				trap_SendServerCommand( ent-g_entities, "print \"Red team\n\"" );
				break;
			
			case TEAM_FREE:
				trap_SendServerCommand( ent-g_entities, "print \"Free team\n\"" );
				break;
		
			case TEAM_SPECTATOR:
				trap_SendServerCommand( ent-g_entities, "print \"Spectator team\n\"" );
				break;
		}
		
		return;
	}

	// Limit how often one can switch team
	if ( ent->client->switchTeamTime > level.time ) 
	{
		trap_SendServerCommand( ent-g_entities, "print \"May not switch teams more than once per 5 seconds.\n\"" );
		return;
	}

	trap_Argv( 1, team, sizeof( team ) );
	trap_Argv( 2, identity, sizeof( identity ) );

	SetTeam( ent, team, identity[0]?identity:NULL );

	// Remember the team switch time so they cant do it again really quick
	ent->client->switchTeamTime = level.time + 5000;
}


/*
=================
Cmd_Follow_f
=================
*/
void Cmd_Follow_f( gentity_t *ent ) 
{
	int		i;
	char	arg[MAX_TOKEN_CHARS];

	if ( trap_Argc() != 2 ) 
	{
		if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ) 
		{
			G_StopFollowing( ent );
		}
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	i = ClientNumberFromString( ent, arg );
	if ( i == -1 ) 
	{
		return;
	}

	// can't follow self
	if ( &level.clients[ i ] == ent->client ) 
	{
		return;
	}

	// cant cycle to dead people
	if ( level.clients[i].ps.pm_type == PM_DEAD )
	{
		return;
	}

	// can't follow another spectator
	if ( G_IsClientSpectating ( &level.clients[ i ] ) )
	{
		return;
	}

	// first set them to spectator as long as they arent a ghost
	if ( !ent->client->sess.ghost && ent->client->sess.team != TEAM_SPECTATOR ) 
	{
		SetTeam( ent, "spectator", NULL );
	}

	ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
	ent->client->sess.spectatorClient = i;
}

/*
=================
Cmd_FollowCycle_f
=================
*/
void Cmd_FollowCycle_f( gentity_t *ent, int dir ) 
{
	int		clientnum;
	int		deadclient;
	int		original;

	// first set them to spectator
	if ( !ent->client->sess.ghost && ent->client->sess.team != TEAM_SPECTATOR ) 
	{
		SetTeam( ent, "spectator", NULL );
	}

	if ( dir != 1 && dir != -1 ) 
	{
		Com_Error( ERR_FATAL, "Cmd_FollowCycle_f: bad dir %i", dir );
	}

	if ( ent->client->sess.spectatorClient == -1 )
	{
		clientnum = original = ent->s.number;
	}
	else
	{
		clientnum = original = ent->client->sess.spectatorClient;
	}

	deadclient = -1;
	do 
	{
		clientnum += dir;
		if ( clientnum >= level.maxclients ) 
		{
			clientnum = 0;
		}
		if ( clientnum < 0 ) 
		{
			clientnum = level.maxclients - 1;
		}

		// can only follow connected clients
		if ( level.clients[ clientnum ].pers.connected != CON_CONNECTED ) 
		{
			continue;
		}

		// can't follow another spectator
		if ( G_IsClientSpectating ( &level.clients[ clientnum ] ) ) 
		{
			continue;
		}

		// Cant switch to dead people unless there is nobody else to switch to
		if ( G_IsClientDead ( &level.clients[clientnum] ) )
		{		
			deadclient = clientnum;
			continue;
		}

		// Dissallow following of the enemy if the cvar is set
		if ( level.gametypeData->teams && !g_followEnemy.integer && ent->client->sess.team != TEAM_SPECTATOR )
		{
			// Are they on the same team?
			if ( level.clients[ clientnum ].sess.team != ent->client->sess.team )
			{
				continue;
			}
		}
			
		// this is good, we can use it
		ent->client->sess.spectatorClient = clientnum;
		ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
		return;

	} while ( clientnum != original );

	// If being forced to follow and there is a dead client to jump to, then jump to them now
	if ( deadclient != -1 && g_forceFollow.integer )
	{
		// this is good, we can use it
		ent->client->sess.spectatorClient = deadclient;
		ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
		return;
	}

   	G_StopFollowing( ent );

	// leave it where it was
}

/*
==================
G_SayTo
==================
*/
static void G_SayTo( gentity_t *ent, gentity_t *other, int mode, const char *name, const char *message ) 
{
	qboolean	 ghost = qfalse;
	qboolean	 spec  = qfalse;
	const char	*type;
	char		*admin;
	char 		*star;

	if (!other) 
	{
		return;
	}

	if (!other->inuse) 
	{
		return;
	}

	if (!other->client) 
	{
		return;
	}

	if ( other->client->pers.connected != CON_CONNECTED ) 
	{
		return;
	}

	if ( mode == SAY_TEAM  && !OnSameTeam(ent, other) ) 
	{
		return;
	}
	// Boe!Man 1/17/10: We do not wish to send the chat when it's Admin/Clan only..
	if ( mode == ADM_CHAT  && !other->client->sess.admin )
		return;
	if ( mode == CADM_CHAT && other->client->sess.admin < 2 && ent != other)
		return;

	if ( !level.intermissiontime && !level.intermissionQueued )
	{
		// Spectators cant talk to alive people
		if ( ent->client->sess.team == TEAM_SPECTATOR )
		{
			spec = qtrue;
		}

		if ( level.gametypeData->respawnType == RT_NONE )
		{
			// Dead people cant talk to alive people
			if ( !spec && G_IsClientDead ( ent->client ) )
			{
				ghost = qtrue;
			}
			
			// Boe!Man 1/17/10: Admins need to be heard, even if they're dead/spec.
			if(mode >= ADM_TALK && mode <= CADM_CHAT)
			{
				ghost = qfalse;
				spec = qfalse;
			}

			// If the client we are talking to is alive then a check
			// must be made to see if this talker is alowed to speak to this person
			if ( ent->s.number != other->s.number && !G_IsClientDead ( other->client ) && !G_IsClientSpectating( other->client) && (ghost || spec))
			{
				return;
			}
		}
	}

	admin = ""; // Boe!Man 1/18/10: Clean the Admin data.
	
	// Boe!Man 1/7/10: Team prefixes.
	if ( ghost )
	{
		type = "^7[^Cg^7]";
	}
	else if ( spec )
	{
		type = ("^7[^Cs^7]");
	}
	else if ( ent->client->sess.team == TEAM_RED )
	{
		type = "^7[^1h^7]";
	}
	else if ( ent->client->sess.team == TEAM_BLUE )
	{
		type = "^7[^ys^7]";
	}

	// Boe!Man 1/17/10: Admin Talk/Chat.
	if(mode == ADM_TALK || mode == ADM_CHAT || mode == CADM_CHAT){
	star = va("%s", server_starprefix.string);
	admin = "";
	}else{
	star = "";
	// Boe!Man 1/6/10: Admin prefixes. - Update 1/18/10: New CVARs for prefixes if the hoster wishes to change the values.
	if(ent->client->sess.admin == 2){
		admin = va("%s ", server_badminprefix.string);
	}else if(ent->client->sess.admin == 3){
		admin = va("%s ", server_adminprefix.string);
	}else if(ent->client->sess.admin == 4){
		admin = va("%s ", server_sadminprefix.string);
	}else{
		admin = "";
	}
	}
	
	// Boe!Man 1/17/10: Different kinds of Talking 'Modes'.
	switch(mode)
	{
	case ADM_CHAT:
		type = server_acprefix.string;
		Boe_ClientSound(other, G_SoundIndex("sound/misc/menus/invalid.wav"));
		break;
	case ADM_TALK:
		if(ent->client->sess.admin == 2){
		type = server_badminprefix.string;
		}else if(ent->client->sess.admin == 3){
		type = server_adminprefix.string;
		}else if(ent->client->sess.admin == 4){
		type = server_sadminprefix.string;
		}
		Boe_ClientSound(other, G_SoundIndex("sound/misc/menus/invalid.wav"));
		break;
	case CADM_CHAT:
		type = server_caprefix.string;
		Boe_ClientSound(other, G_SoundIndex("sound/misc/menus/invalid.wav"));
		break;
	default:
		break;
	}

	// Boe!Man 1/6/10
	trap_SendServerCommand( other-g_entities, va("%s %d \"%s %s %s%s%s %s\"", // Boe!Man 1/6/10: Adding prefixes. - Update 1/17/10: Adding Admin Talk/Chat prefixes.
							mode == SAY_TEAM ? "tchat" : "chat",
							ent->s.number,
							// Boe!Man 1/6/10: Adding the Admin prefix in front of the chat. - Update 1/17/10.
							star, type, admin, name, message, star)); // Boe!Man 1/17/10: Adding stars.
}

/*
==================
G_GetChatPrefix
==================
*/
void G_GetChatPrefix ( gentity_t* ent, gentity_t* target, int mode, char* name, int nameSize )
{
	const char* namecolor;
	char		location[64];
	qboolean	locationOk = qtrue;

	// Spectators and ghosts dont show locations
	if ( ent->client->ps.pm_type == PM_DEAD || G_IsClientSpectating ( ent->client ) )
	{
		locationOk = qfalse;
	}

	if ( !level.gametypeData->teams && mode == SAY_TEAM ) 
	{
		mode = SAY_ALL;
	}

	if ( level.gametypeData->teams )
	{
		switch ( ent->client->sess.team )
		{
			case TEAM_BLUE:
				namecolor = S_COLOR_BLUE;
				break;

			case TEAM_RED:
				namecolor = S_COLOR_RED;
				break;

			default:
				namecolor = S_COLOR_WHITE;
				break;
		}
	}
	else
	{
		namecolor = S_COLOR_WHITE;
	}

	switch ( mode ) 
	{
		default:
		case SAY_ALL:

			Com_sprintf (name, nameSize, "%s%s%s: ", namecolor, ent->client->pers.netname, S_COLOR_WHITE );

			break;

		case SAY_TEAM:

			if ( locationOk && Team_GetLocationMsg(ent, location, sizeof(location)))
			{
				Com_sprintf ( name, nameSize, "%s(%s%s) %s(%s): ", 
							  namecolor, 
							  ent->client->pers.netname, 
							  namecolor,
							  S_COLOR_WHITE, location );
			}
			else
			{
				Com_sprintf ( name, nameSize, "%s(%s%s)%s: ", 
							  namecolor, 
							  ent->client->pers.netname, 
							  namecolor,
							  S_COLOR_WHITE );
			}
			break;

		case SAY_TELL:

			if ( locationOk && target && level.gametypeData->teams   && 
				 target->client->sess.team == ent->client->sess.team  &&
				 Team_GetLocationMsg(ent, location, sizeof(location))    )
			{
				Com_sprintf ( name, nameSize, "%s[%s%s] %s(%s): ", 
							  namecolor,
							  ent->client->pers.netname, 
							  namecolor,
							  S_COLOR_WHITE, location );
			}
			else
			{
				Com_sprintf ( name, nameSize, "%s[%s%s]%s: ", 
							  namecolor,
							  ent->client->pers.netname, 
							  namecolor,
							  S_COLOR_WHITE );
			}
			break;
	}

	strcat ( name, S_COLOR_GREEN );
}

/*
==================
G_Say
==================
*/
void G_Say ( gentity_t *ent, gentity_t *target, int mode, const char *chatText ) 
{
	int			j;
	gentity_t	*other;
	char		text[MAX_SAY_TEXT];
	char		name[64];

	// Logging stuff
	switch ( mode )
	{
		case SAY_ALL:
			G_LogPrintf( "say: %s: %s\n", ent->client->pers.netname, chatText );
			break;

		case SAY_TEAM:
			G_LogPrintf( "sayteam: %s: %s\n", ent->client->pers.netname, chatText );
			break;
	}

	// Generate the chat prefix
	G_GetChatPrefix ( ent, target, mode, name, sizeof(name) );

	// Save off the chat text
	Q_strncpyz( text, chatText, sizeof(text) );

	if ( target ) 
	{
		G_SayTo( ent, target, mode, name, text );
		return;
	}

	// echo the text to the console
	if ( g_dedicated.integer ) 
	{
		Com_Printf( "%s%s\n", name, text);
	}

	// send it to all the apropriate clients
	for (j = 0; j < level.numConnectedClients; j++) 
	{
		other = &g_entities[level.sortedClients[j]];
		G_SayTo( ent, other, mode, name, text );
	}
}


/*
==================
Cmd_Say_f
==================
*/
char *GetReason(gentity_t *ent, char *buffer, int id) {
int i;
char *reason;
int a = 0;
int start;
if(id >= 10){
	start = 6;
}else{
	start = 5;
}
for(i=start;i<=strlen(buffer);i++){
	if(buffer[i] == ' '){
		buffer[i] = '_';
	}
	reason[a] = buffer[i];
	a += 1;
}
if(strlen(buffer) <= 4){
	reason = "";
}
buffer = "";
ent = NULL;
return reason;
}

// Boe!Man 1/10/10
int CheckAdmin(gentity_t *ent, char *buffer, qboolean otheradmins) {
char		*numb;
int			id;
int			i;
for(i=0;i<=g_maxclients.integer;i++){
	numb = va("%i", i);
	if(strstr(buffer, numb)){
		id = atoi(numb);
		if(i>=10){
			break;
		}
	}
}

	if(id >= 0 && id <= g_maxclients.integer){
		if(g_entities[id].client->sess.admin && otheradmins == qfalse){
			trap_SendServerCommand(ent->s.number, va("print\"^3[Info] ^7You cannot use this command on other Admins.\n\""));
			return -1;
		}
			else{
				if ( g_entities[id].client->pers.connected != CON_DISCONNECTED )
				{
					if(ent && ent->client)
					{
						return id;
					}else{
						return -1;
					}
				}else{
					return -1;
				}
			}
	}else{
	return -1;
	}
}

void Cmd_Say_f( gentity_t *ent, int mode, qboolean arg0 ) {
	char		*p;
	// Boe!Man 1/10/10
	char		*status;
	int			id;
	gentity_t	*targ;
	char		id2[64];
	char		*id1;
	int			i;
	int			a = 0;
	// Boe!Man 1/17/10
	vec3_t		lookdown;
	vec3_t		fireAngs;
	int			anim = 0;
	float		knockback = 400.0;
	vec3_t		dir;
	// Boe!Man 1/30/10
	gentity_t	*tent, *other;
	gclient_t	*client;
	int			idle;
	char		*lwrP;
	char		rcon[64];
	// Boe!Man 3/20/10
	int			it, nadeDir, weapon;
	float		x, y;
	gentity_t	*missile;

	if ( trap_Argc () < 2 && !arg0 ) {
		return;
	}

	if (arg0)
	{
		p = ConcatArgs( 0 );
	}
	else if(mode >= ADM_TALK && mode <= CADM_CHAT)
		p = ConcatArgs( 2 );
	else
	{
		p = ConcatArgs( 1 );
	}
	lwrP = Q_strlwr(p);
	// Boe!Man 1/10/10: Chat Admin command tokens.
	if ((strstr(lwrP, "!k ")) || (strstr(lwrP, "!kick "))) {	
		if ( ent->client->sess.admin >= g_kick.integer){
			id = CheckAdmin(ent, p, qfalse);
			if(id < 0) return;
			trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"%s\"\n", id, GetReason(ent, p, id)));
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s was %sk%si%sc%sk%se%sd ^7by %s", level.time + 5000, g_entities[id].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, ent->client->pers.netname));
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was kicked by %s.\n\"", g_entities[id].client->pers.netname,ent->client->pers.netname));
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			Boe_adminLog (va("%s - KICK: %s", ent->client->pers.cleanName, g_entities[id].client->pers.cleanName  )) ;
		}
		else if ( ent->client->sess.admin < g_kick.integer){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
			id = -1;
		}
	}
	// Boe!Man 1/15/10: More Chat Admin command tokens.
	else if ((strstr(lwrP, "!ab ")) || (strstr(lwrP, "!addbadmin "))) {
		if (ent->client->sess.admin >= g_addbadmin.integer){
			id = CheckAdmin(ent, p, qfalse);
			targ = g_entities+id;
			id1 = targ->client->pers.boe_id;
			if(targ->client->sess.admin >= 1){
				id = -1;
			}if(id != -1){;
			g_entities[id].client->sess.admin = 2;
			Q_strncpyz (id2, id1, 64);
			strcat ( id2, ":2" );
			if(Boe_AddToList(id2, g_adminfile.string, "Admin", NULL)){
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s is now a %sB^7-%sA%sd%sm%si%sn", level.time + 5000, g_entities[id].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was made B-Admin by %s.\n\"", g_entities[id].client->pers.netname,ent->client->pers.netname));
			Boe_adminLog (va("%s - ADD B-ADMIN: %s", ent->client->pers.cleanName, g_entities[id].client->pers.cleanName  )) ;
			}
			}
			p = ConcatArgs(1);
		}
		else if ( ent->client->sess.admin < g_addbadmin.integer){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
			id = -1;
		}
		G_Say( ent, NULL, mode, p );
		return;
		}
	else if ((strstr(lwrP, "!aa ")) || (strstr(lwrP, "!addadmin "))) {
		if (ent->client->sess.admin >= g_addadmin.integer){
			id = CheckAdmin(ent, p, qfalse);
			targ = g_entities+id;
			id1 = targ->client->pers.boe_id;
			if(targ->client->sess.admin >= 1){
				id = -1;
			}if(id != -1){;
			// Boe!Man 1/21/10: In the session the client also has to be an Admin.
			g_entities[id].client->sess.admin = 3;
			Q_strncpyz (id2, id1, 64);
			strcat ( id2, ":3" );
			if(Boe_AddToList(id2, g_adminfile.string, "Admin", NULL)){
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s is now an %sA%sd%sm%si%sn", level.time + 5000, g_entities[id].client->pers.netname, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s is now an Admin.\n\"", g_entities[id].client->pers.netname));
			Boe_adminLog (va("%s - ADD ADMIN: %s", ent->client->pers.cleanName, g_entities[id].client->pers.cleanName  )) ;
			}
			}
			p = ConcatArgs(1);
		}
		else if ( ent->client->sess.admin < g_addadmin.integer){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
			id = -1;
		}
		G_Say( ent, NULL, mode, p );
		return;
		}
	else if ((strstr(lwrP, "!as ")) || (strstr(lwrP, "!addsadmin "))) {
		if (ent->client->sess.admin >= g_addsadmin.integer){
			id = CheckAdmin(ent, p, qfalse);
			targ = g_entities+id;
			id1 = targ->client->pers.boe_id;
			if(targ->client->sess.admin >= 1){
				id = -1;
			}if(id != -1){;
			// Boe!Man 1/17/10: Fix for getting B-Admin when adding S-Admin.
			g_entities[id].client->sess.admin = 4;
			Q_strncpyz (id2, id1, 64);
			// Boe!Man 1/17/10: Fix for getting B-Admin when adding S-Admin.
			strcat ( id2, ":4" );
			if(Boe_AddToList(id2, g_adminfile.string, "Admin", NULL)){
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s is now a %sS^7-%sA%sd%sm%si%sn", level.time + 5000, g_entities[id].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s is now a S-Admin.\n\"", g_entities[id].client->pers.netname));
			Boe_adminLog (va("%s - ADD S-ADMIN: %s", ent->client->pers.cleanName, g_entities[id].client->pers.cleanName  )) ;
			}
			}
			p = ConcatArgs(1);
		}
		else if ( ent->client->sess.admin < g_addsadmin.integer){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
			id = -1;
		}
		G_Say( ent, NULL, mode, p );
		return;
	}
	else if ((strstr(lwrP, "!tw ")) || (strstr(lwrP, "!twist "))){
		if (ent->client->sess.admin >= g_twist.integer){
			id = CheckAdmin(ent, p, qtrue);
			targ = g_entities+id;
			if ( targ->client->sess.ghost ){ // Boe!Man 1/24/10: We cannot do this to a Ghost.
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You cannot twist a Ghost.\n\""));
				id = -1;
				return;}
			if (G_IsClientSpectating(targ->client)){ // Boe!Man 1/24/10: We cannot do this to a Spectator.
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You cannot twist a Spectator.\n\""));
				id = -1;
				return;}
			if(id != -1){;
			VectorSet(lookdown, 100, 0, 130);
			SetClientViewAngle(targ, lookdown);
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s was %st%sw%si%ss%st%sed by %s", level.time + 5000, g_entities[id].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, ent->client->pers.netname));
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was twisted by %s.\n\"", g_entities[id].client->pers.netname,ent->client->pers.netname));
			Boe_adminLog (va("%s - TWIST: %s", ent->client->pers.cleanName, g_entities[id].client->pers.cleanName  )) ;
			}
			p = ConcatArgs(1);
		}
		else if ( ent->client->sess.admin < g_twist.integer){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
			id = -1;
		}
		G_Say( ent, NULL, mode ,p);
		return;
	}
	else if ((strstr(lwrP, "!utw ")) || (strstr(lwrP, "!untwist "))){
	if (ent->client->sess.admin >= g_twist.integer){
			id = CheckAdmin(ent, p, qtrue);
			targ = g_entities+id;
			if ( targ->client->sess.ghost ){ // Boe!Man 1/24/10: We cannot do this to a Ghost.
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You cannot untwist a Ghost.\n\""));
				id = -1;
				return;}
			if (G_IsClientSpectating(targ->client)){ // Boe!Man 1/24/10: We cannot do this to a Spectator.
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You cannot untwist a Spectator.\n\""));
				id = -1;
				return;}
			if(id != -1){;
			VectorSet(lookdown, 0, 0, 0);	
			SetClientViewAngle(targ, lookdown);
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s was %su%sn%st%sw%si%ssted by %s", level.time + 5000, g_entities[id].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, ent->client->pers.netname));
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was untwisted by %s.\n\"", g_entities[id].client->pers.netname,ent->client->pers.netname));
			Boe_adminLog (va("%s - UNTWIST: %s", ent->client->pers.cleanName, g_entities[id].client->pers.cleanName  )) ;
			}
			p = ConcatArgs(1);
		}
		else if ( ent->client->sess.admin < g_twist.integer){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
			id = -1;
		}
		G_Say( ent, NULL, mode ,p);
		return;
	}
	else if ((strstr(lwrP, "!pl ")) || (strstr(lwrP, "!plant "))){
		if (ent->client->sess.admin >= g_plant.integer){
			id = CheckAdmin(ent, p, qtrue);
			targ = g_entities+id;
			if ( targ->client->sess.ghost ){ // Boe!Man 1/24/10: We cannot do this to a Ghost.
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You cannot plant a Ghost.\n\""));
				id = -1;
				return;}
			if (G_IsClientSpectating(targ->client)){ // Boe!Man 1/24/10: We cannot do this to a Spectator.
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You cannot plant a Spectator.\n\""));
				id = -1;
				return;}
			if(targ->client->pers.planted){
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7%s ^7is already planted.\n\"", targ->client->pers.netname));
				id = -1;
			}
			if (id != -1){
			if ( targ->client->ps.pm_flags & PMF_DUCKED )
				targ->client->ps.origin[2] -=40;
			else
				targ->client->ps.origin[2] -= 65;
			VectorCopy( targ->client->ps.origin, targ->s.origin );
			targ->client->pers.planted = qtrue;
			Boe_ClientSound(targ, G_SoundIndex("sound/misc/confused/wood_break.mp3"));
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sp%sl%sa%sn%st%sed by %s", level.time + 5000, g_entities[id].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, ent->client->pers.netname));
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was planted by %s.\n\"", g_entities[id].client->pers.netname,ent->client->pers.netname));
			Boe_adminLog (va("%s - PLANT: %s", ent->client->pers.cleanName, g_entities[id].client->pers.cleanName  )) ;
			}
		}
		else if ( ent->client->sess.admin < g_plant.integer){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
			id = -1;
		}
		G_Say( ent, NULL, mode ,p);
		return;
	}
	else if ((strstr(lwrP, "!upl ")) || (strstr(lwrP, "!unplant "))){
		if (ent->client->sess.admin >= g_plant.integer){
			id = CheckAdmin(ent, p, qtrue);
			targ = g_entities+id;
			if ( targ->client->sess.ghost ){ // Boe!Man 1/24/10: We cannot do this to a Ghost.
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You cannot unplant a Ghost.\n\""));
				id = -1;
				return;}
			if (G_IsClientSpectating(targ->client)){ // Boe!Man 1/24/10: We cannot do this to a Spectator.
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You cannot unplant a Spectator.\n\""));
				id = -1;
				return;}
			if(!targ->client->pers.planted){
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7%s ^7is not planted.\n\"", targ->client->pers.netname));
				id = -1;
			}
			if (id != -1){
			if(targ->client->pers.planted){
			targ->client->ps.origin[2] += 65;
			VectorCopy( targ->client->ps.origin, targ->s.origin );
			targ->client->pers.planted = qfalse;
			Boe_ClientSound(targ, G_SoundIndex("sound/misc/confused/wood_break.mp3"));
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %su%sn%sp%sl%sa%snted by %s", level.time + 5000, g_entities[id].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, ent->client->pers.netname));
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was unplanted by %s.\n\"", g_entities[id].client->pers.netname,ent->client->pers.netname));
			Boe_adminLog (va("%s - UNPLANT: %s", ent->client->pers.cleanName, g_entities[id].client->pers.cleanName  )) ;
			}
			}
		}
		else if ( ent->client->sess.admin < g_plant.integer){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
			id = -1;
		}
		G_Say( ent, NULL, mode ,p);
		return;
	}
	else if ((strstr(lwrP, "!u ")) || (strstr(lwrP, "!uc ")) || (strstr(lwrP, "!uppercut "))) {
		if (ent->client->sess.admin >= g_uppercut.integer){
			id = CheckAdmin(ent, p, qtrue);
			targ = g_entities+id;
			if ( targ->client->sess.ghost ){ // Boe!Man 1/24/10: We cannot do this to a Ghost.
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You cannot uppercut a Ghost.\n\""));
				id = -1;
				return;}
			if (G_IsClientSpectating(targ->client)){ // Boe!Man 1/24/10: We cannot do this to a Spectator.
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You cannot uppercut a Spectator.\n\""));
				id = -1;
				return;}
			if(id != -1){
			targ->client->ps.pm_flags |= PMF_JUMPING;
			targ->client->ps.groundEntityNum = ENTITYNUM_NONE;
			targ->client->ps.velocity[2] = 1000;
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s was %su%sp%sp%se%sr%scut by %s", level.time + 5000, g_entities[id].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, ent->client->pers.netname));
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was uppercut by %s.\n\"", g_entities[id].client->pers.netname,ent->client->pers.netname));
			Boe_adminLog (va("%s - UPPERCUT: %s", ent->client->pers.cleanName, g_entities[id].client->pers.cleanName  )) ;
			}
			p = ConcatArgs(1);
		}
		else if ( ent->client->sess.admin < g_uppercut.integer){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
			id = -1;
		}
		G_Say( ent, NULL, mode, p );
		return;
	}
	else if ((strstr(lwrP, "!ro ")) || (strstr(lwrP, "!runover "))){
		if (ent->client->sess.admin >= g_runover.integer && (strstr(level.mapname, "col9"))){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Cross the bridge] ^7You cannot runover anyone in this mini game.\n\""));
			id = -1;
		}
		// Boe!Man 3/20/10: Fix for runover working for S-Admin only.
		else if (ent->client->sess.admin >= g_runover.integer){
			id = CheckAdmin(ent, p, qtrue);
			targ = g_entities+id;
			if ( targ->client->sess.ghost ){ // Boe!Man 1/17/10: We cannot runover a Ghost.
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You cannot runover a Ghost.\n\""));
				id = -1;
				return;}
			if (G_IsClientSpectating(targ->client)){ // Boe!Man 1/24/10: We cannot runover a Spectator.
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You cannot runover a Spectator.\n\""));
				id = -1;
				return;}
			if(id != -1){
			VectorCopy(targ->client->ps.viewangles, fireAngs);
			AngleVectors( fireAngs, dir, NULL, NULL );	
			dir[0] *= -1.0;
			dir[1] *= -1.0;
			dir[2] = 0.0;
			VectorNormalize ( dir );
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			Boe_ClientSound(ent, G_SoundIndex("sound/ambience/vehicles/hit_scrape.mp3"));
			targ->client->ps.weaponstate = WEAPON_FIRING;
			//targ->client->ps.firemode[targ->s.weapon] = WP_FIREMODE_SINGLE;
			targ->client->ps.velocity[2] = 20;
			targ->client->ps.weaponTime = 3000;
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sr%su%sn%so%sv%ser by %s", level.time + 5000, g_entities[id].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, ent->client->pers.netname));
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was runover by %s.\n\"", g_entities[id].client->pers.netname,ent->client->pers.netname));
			Boe_adminLog (va("%s - RUNOVER: %s", ent->client->pers.cleanName, g_entities[id].client->pers.cleanName  )) ;
			G_Damage (targ, NULL, NULL, NULL, NULL, 15, 0, MOD_CAR, HL_NONE );
			G_ApplyKnockback ( targ, dir, knockback );
			}
		}
		else if ( ent->client->sess.admin < g_runover.integer){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
			id = -1;
		}
		G_Say( ent, NULL, mode, p );
		return;
	}
	else if ((strstr(lwrP, "!r ")) || (strstr(lwrP, "!rs ")) || (strstr(lwrP, "!respawn "))) {
		if (ent->client->sess.admin >= g_respawn.integer){
			id = CheckAdmin(ent, p, qtrue);
			targ = g_entities+id;
			if(id != -1){
			if ( targ->client->sess.ghost ){
			G_StopFollowing ( targ );
			targ->client->ps.pm_flags &= ~PMF_GHOST;
			targ->client->ps.pm_type = PM_NORMAL;
			targ->client->sess.ghost = qfalse;
			}else{
				TossClientItems(targ);
			}
			targ->client->sess.noTeamChange = qfalse;
			trap_UnlinkEntity (targ);
			ClientSpawn (targ);
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			Boe_ClientSound(targ, G_SoundIndex("sound/ambience/vehicles/telephone_pole.mp3"));
			// Boe!Man 1/21/10: Not displaying message fix.. Updated 2/1/10.
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s was %sr%se%ss%sp%sa%swned by %s", level.time + 5000, targ->client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, ent->client->pers.netname));
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was respawned by %s.\n\"", targ->client->pers.netname,ent->client->pers.netname));
			Boe_adminLog (va("%s - RESPAWN: %s", ent->client->pers.cleanName, g_entities[id].client->pers.cleanName  )) ;}
		}
		else if ( ent->client->sess.admin < g_respawn.integer){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
			id = -1;
		}
		G_Say( ent, NULL, mode, p );
		return;
	}
	else if ((strstr(lwrP, "!mr")) || (strstr(lwrP, "!maprestart"))) { // Boe!Man 1/21/10: No need for a space when we're restarting the map.
		if (ent->client->sess.admin >= g_maprestart.integer){
			trap_SendConsoleCommand( EXEC_APPEND, va("map_restart 5\n"));
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%sM%sa%sp %sr%se%sstart!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
			Boe_GlobalSound (G_SoundIndex("sound/misc/menus/invalid.wav"));
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Map restarted by %s.\n\"", ent->client->pers.netname));
			Boe_adminLog (va("%s - MAP RESTART: %s", ent->client->pers.cleanName, g_entities[id].client->pers.cleanName  )) ;
		}
		else if ( ent->client->sess.admin < g_maprestart.integer){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
			id = -1;
		}
		G_Say( ent, NULL, mode, p );
		return;
	}
	else if ((strstr(lwrP, "!p ")) || (strstr(lwrP, "!pop "))) {
		if (ent->client->sess.admin >= g_pop.integer){
			id = CheckAdmin(ent, p, qtrue);
			targ = g_entities+id;
			if ( targ->client->sess.ghost ){ // Boe!Man 1/24/10: We cannot do this to a Ghost.
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You cannot pop a Ghost.\n\""));
				id = -1;
				return;}
			if (G_IsClientSpectating(targ->client)){ // Boe!Man 1/24/10: We cannot do this to a Spectator.
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You cannot pop a Spectator.\n\""));
				id = -1;
				return;}
			if(id != -1){
			Boe_ClientSound(targ, G_SoundIndex("sound/npc/air1/guard02/laughs.mp3"));
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sp%so%sp%sp%se%sd by %s", level.time + 5000, g_entities[id].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, ent->client->pers.netname));
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was popped by %s.\n\"", g_entities[id].client->pers.netname,ent->client->pers.netname));
			Boe_adminLog (va("%s - POP: %s", ent->client->pers.cleanName, g_entities[id].client->pers.cleanName  )) ;
			G_Damage (targ, NULL, NULL, NULL, NULL, 10000, 0, MOD_POP, HL_HEAD|HL_FOOT_RT|HL_FOOT_LT|HL_LEG_UPPER_RT|HL_LEG_UPPER_LT|HL_HAND_RT|HL_HAND_LT|HL_WAIST|HL_CHEST|HL_NECK);
			}
		}
		else if ( ent->client->sess.admin < g_pop.integer){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
			id = -1;
		}
		G_Say( ent, NULL, mode, p);
		return;
	}
	else if ((strstr(lwrP, "!b ")) || (strstr(lwrP, "!burn "))) {
		if (ent->client->sess.admin >= g_burn.integer){
			id = CheckAdmin(ent, p, qtrue);
			targ = g_entities+id;
			if ( targ->client->sess.ghost ){ // Boe!Man 1/24/10: We cannot do this to a Ghost.
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You cannot burn a Ghost.\n\""));
				id = -1;
				return;}
			if (G_IsClientSpectating(targ->client)){ // Boe!Man 1/24/10: We cannot do this to a Spectator.
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You cannot burn a Spectator.\n\""));
				id = -1;
				return;}
			if(id != -1){
			targ->client->sess.burnSeconds = 6;
			tent = G_TempEntity( g_entities[targ->s.number].r.currentOrigin, EV_EXPLOSION_HIT_FLESH ); 
			tent->s.eventParm = 0; 
			tent->s.otherEntityNum2 = g_entities[targ->s.number].s.number; 
			tent->s.time = WP_ANM14_GRENADE + ((((int)g_entities[targ->s.number].s.apos.trBase[YAW]&0x7FFF) % 360) << 16); 
			VectorCopy ( g_entities[targ->s.number].r.currentOrigin, tent->s.angles ); 
			SnapVector ( tent->s.angles ); 
			Boe_ClientSound(targ, G_SoundIndex("/sound/weapons/incendiary_grenade/incen01.mp3"));
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sb%su%sr%sn%se%sd by %s", level.time + 5000, g_entities[id].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, ent->client->pers.netname));
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was burned by %s.\n\"", g_entities[id].client->pers.netname,ent->client->pers.netname));
			Boe_adminLog (va("%s - BURN: %s", ent->client->pers.cleanName, g_entities[id].client->pers.cleanName  )) ;
			}
		}
		else if ( ent->client->sess.admin < g_burn.integer){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
			id = -1;
		}
		G_Say( ent, NULL, mode, p);
		return;
	}
	else if ((strstr(lwrP, "!m ")) || (strstr(lwrP, "!mute "))) {
		if (ent->client->sess.admin >= g_mute.integer){
			id = CheckAdmin(ent, p, qfalse);
			targ = g_entities+id;
			if(g_entities[id].client->sess.mute == qtrue){
			trap_SendServerCommand(ent-g_entities, va("print \"^3[Info] ^7This client is already muted!\n\""));
			id = -1;}
			if(id != -1){
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			g_entities[id].client->sess.mute = qtrue;
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sm%su%st%se%sd by %s", level.time + 5000, g_entities[id].client->pers.netname, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, ent->client->pers.netname));
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was muted by %s.\n\"", g_entities[id].client->pers.netname, ent->client->pers.netname));
			Boe_adminLog (va("%s - MUTE: %s", ent->client->pers.cleanName, g_entities[id].client->pers.cleanName  )) ;
			}
		}
		else if ( ent->client->sess.admin < g_mute.integer){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
			id = -1;
		}
		G_Say( ent, NULL, mode, p);
		return;
	}
	else if ((strstr(lwrP, "!um ")) || (strstr(lwrP, "!unmute "))) {
		if (ent->client->sess.admin >= g_mute.integer){
			id = CheckAdmin(ent, p, qfalse);
			targ = g_entities+id;
			if(g_entities[id].client->sess.mute == qfalse){
			trap_SendServerCommand(ent-g_entities, va("print \"^3[Info] ^7This client is not muted!\n\""));
			id = -1;}
			if(id != -1){
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			g_entities[id].client->sess.mute = qfalse;
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %su%sn%sm%su%st%sed by %s", level.time + 5000, g_entities[id].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, ent->client->pers.netname));
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was unmuted by %s.\n\"", g_entities[id].client->pers.netname, ent->client->pers.netname));
			Boe_adminLog (va("%s - UNMUTE: %s", ent->client->pers.cleanName, g_entities[id].client->pers.cleanName  )) ;
			}
		}
		else if (ent->client->sess.admin < g_mute.integer){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
			id = -1;
		}
		G_Say( ent, NULL, mode, p);
		return;
	}
	else if ((strstr(lwrP, "!st ")) || (strstr(lwrP, "!strip "))) {
		if (ent->client->sess.admin >= g_strip.integer){
			id = CheckAdmin(ent, p, qfalse);
			targ = g_entities+id;
			client = targ->client;
			if ( targ->client->sess.ghost ){ // Boe!Man 1/24/10: We cannot do this to a Ghost.
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You cannot strip a Ghost.\n\""));
				id = -1;
				return;}
			if (G_IsClientSpectating(targ->client)){ // Boe!Man 1/24/10: We cannot do this to a Spectator.
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You cannot strip a Spectator.\n\""));
				id = -1;
				return;}
			if(id != -1){
			targ->client->ps.zoomFov = 0;
			targ->client->ps.pm_flags &= ~(PMF_GOGGLES_ON|PMF_ZOOM_FLAGS);
			client->ps.stats[STAT_WEAPONS] = 0;
			client->ps.stats[STAT_GOGGLES] = GOGGLES_NONE;  
			memset ( client->ps.ammo, 0, sizeof(client->ps.ammo) );
			memset ( client->ps.clip, 0, sizeof(client->ps.clip) );
			client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_KNIFE );
			client->ps.clip[ATTACK_NORMAL][WP_KNIFE]=weaponData[WP_KNIFE].attack[ATTACK_NORMAL].clipSize;
			client->ps.firemode[WP_KNIFE] = BG_FindFireMode ( WP_KNIFE, ATTACK_NORMAL, WP_FIREMODE_AUTO );
			client->ps.weapon = WP_KNIFE;
			BG_GetInviewAnim(client->ps.weapon,"idle",&idle);
			client->ps.weaponAnimId = idle;
			client->ps.weaponstate = WEAPON_READY;
			client->ps.weaponTime = 0;
			client->ps.weaponAnimTime = 0;
			client->ps.stats[STAT_OUTFIT_GRENADE] = bg_itemlist[bg_outfittingGroups[-1][client->pers.outfitting.items[-1]]].giTag;
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %ss%st%sr%si%sp%sped by %s", level.time + 5000, g_entities[id].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, ent->client->pers.netname));
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was stripped by %s.\n\"", g_entities[id].client->pers.netname, ent->client->pers.netname));
			Boe_adminLog (va("%s - STRIP: %s", ent->client->pers.cleanName, g_entities[id].client->pers.cleanName  )) ;
			}
		}
		else if (ent->client->sess.admin < g_strip.integer){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
			id = -1;
		}
		G_Say( ent, NULL, mode, p);
		return;
	}
	else if ((strstr(lwrP, "!ra ")) || (strstr(lwrP, "!removeadmin "))) {
		if (ent->client->sess.admin >= g_removeadmin.integer){
			id = CheckAdmin(ent, p, qtrue);
			targ = g_entities+id;
			id1 = targ->client->pers.boe_id;
			if(targ->client->sess.admin < 2){
				id = -1;
			}
			if(targ->client->sess.admin == 0){
				id = -1;
			}
			if(id != -1){
			Boe_Remove_from_list(id1, g_adminfile.string, "admin", NULL, qfalse, qtrue, qfalse);
			targ->client->sess.admin = 0;
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7is no longer an %sA%sd%sm%si%sn", level.time + 5000, g_entities[id].client->pers.netname, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was removed as Admin by %s.\n\"", g_entities[id].client->pers.netname,ent->client->pers.netname));
			Boe_adminLog (va("%s - REMOVE ADMIN: %s", ent->client->pers.cleanName, g_entities[id].client->pers.cleanName  )) ;
			p = ConcatArgs(1);
			}
		}
		else if (ent->client->sess.admin < g_removeadmin.integer){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
			id = -1;
		}
		G_Say( ent, NULL, mode, p);
		return;
	}
	else if ((strstr(lwrP, "!et")) || (strstr(lwrP, "!eventeams"))) {
		if (ent->client->sess.admin >= g_eventeams.integer){
			EvenTeams(ent);
			Boe_adminLog (va("%s - EVENTEAMS", ent->client->pers.cleanName)) ;
		}
		else if (ent->client->sess.admin < g_eventeams.integer){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
		}
		G_Say( ent, NULL, mode, p);
		return;
	}
	else if ((strstr(lwrP, "!333"))) {
		if (ent->client->sess.admin >= g_333.integer){
			// disable 333
			if(pmove_fixed.value == 0){
				trap_Cvar_Set ( "pmove_fixed", "1");
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7333 FPS jumps %sd%si%ss%sa%sbled by %s", level.time + 5000, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, ent->client->pers.netname));
				trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7333 FPS jumps disabled by %s.\n\"", ent->client->pers.netname));
				Boe_adminLog (va("%s - 333 DISABLED", ent->client->pers.cleanName)) ;
			}else{
				trap_Cvar_Set( "pmove_fixed", "0");
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7333 FPS jumps %se%sn%sa%sb%sled ^7by %s", level.time + 5000, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, ent->client->pers.netname));
				trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7333 FPS jumps enabled by %s.\n\"", ent->client->pers.netname));
				Boe_adminLog (va("%s - 333 ENABLED", ent->client->pers.cleanName)) ;
			}
			trap_Cvar_Update ( &pmove_fixed );
		}
		else if (ent->client->sess.admin < g_eventeams.integer){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
		}
		G_Say( ent, NULL, mode, p);
		return;
	}
	else if ((strstr(lwrP, "!ft ")) || (strstr(lwrP, "!forceteam "))){
		if (ent->client->sess.admin >= g_forceteam.integer){
			id = CheckAdmin(ent, p, qtrue);
			if(id < 0) return;
			Boe_adminLog (va("%s - FORCETEAM: %s", ent->client->pers.cleanName, g_entities[id].client->pers.cleanName  )) ;
			if(strstr(p, "b")){
				SetTeam( &g_entities[id], "b", NULL );
			}else if(strstr(p, "r")){
				SetTeam( &g_entities[id], "r", NULL );
			}
		}
		else if (ent->client->sess.admin < g_forceteam.integer){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
		}
		G_Say ( ent, NULL, mode, p);
		return;
	}
	else if ((strstr(lwrP, "!nl")) || (strstr(lwrP, "!nolower"))) {
		if (ent->client->sess.admin >= g_forceteam.integer){
			if(level.nolower1 == qtrue){
				level.nolower1 = qfalse;
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7No lower has been disabled by %s.\n\"", ent->client->pers.netname));
				Boe_adminLog (va("%s - NOLOWER DISABLED", ent->client->pers.cleanName)) ;
			}else{
				level.nolower1 = qtrue;
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7No lower has been enabled by %s.\n\"", ent->client->pers.netname));
				Boe_adminLog (va("%s - NOLOWER ENABLED", ent->client->pers.cleanName)) ;
			}
		}
		else if (ent->client->sess.admin < g_forceteam.integer){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
		}
		G_Say( ent, NULL, mode, p);
		return;
	}
	else if ((strstr(lwrP, "!fl ")) || (strstr(lwrP, "!flash "))){
		if (ent->client->sess.admin >= g_flash.integer){
			id = CheckAdmin(ent, p, qtrue);
			targ = g_entities+id;
			if(id < 0) return;
			targ = g_entities + id;
			weapon = WP_M84_GRENADE;
			nadeDir = 1;
			for( it = 0; it < 1; it++ ) {
			x = 100 * cos( DEG2RAD(nadeDir * it));  
			y = 100 * sin( DEG2RAD(nadeDir * it));
			VectorSet( dir, x, y, 100 );
			dir[2] = 300;	
			missile = NV_projectile( targ, targ->r.currentOrigin, dir, weapon, 0 );
			missile->nextthink = level.time + 250;
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sf%sl%sa%ss%sh%sed by %s", level.time + 5000, g_entities[id].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, ent->client->pers.netname));
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was flashed by %s.\n\"", g_entities[id].client->pers.netname,ent->client->pers.netname));
			Boe_adminLog (va("%s - FLASH: %s", ent->client->pers.cleanName, g_entities[id].client->pers.cleanName  )) ;
			}
		}
		else if (ent->client->sess.admin < g_flash.integer){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
		}
		G_Say( ent, NULL, mode, p);
		return;
	}
	/*else if(strstr(lwrP, "!rcon")){
	trap_Cvar_VariableStringBuffer ( "rconpassword", rcon, MAX_QPATH );
	trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Rconpassword is: %s.\n\"", rcon));
	}*/

	// Henk 08/02/10 -> Fix for not showing uppercase
	if (arg0)
	{
		p = ConcatArgs( 0 );
	}
	else if(mode >= ADM_TALK && mode <= CADM_CHAT)
		p = ConcatArgs( 2 );
	else
	{
		p = ConcatArgs( 1 );
	}

	// Boe!Man 1/24/10: Different kinds of Talk during Gameplay.
	if ((strstr(lwrP, "!at ")) || (strstr(lwrP, "!admintalk "))) {
		if (ent->client->sess.admin){
			p = ConcatArgs(1);
			for(i=4;i<=strlen(p);i++){
			p[a] = p[i];
			a += 1;
			}
			mode = ADM_TALK;
		}else{
			p = ConcatArgs(1);
			G_Say( ent, NULL, mode, p );
			return;
		}
	}
	else if ((strstr(lwrP, "!ac ")) || (strstr(lwrP, "!adminchat "))) {
		if (ent->client->sess.admin){
			p = ConcatArgs(1);
			for(i=4;i<=strlen(p);i++){
			p[a] = p[i];
			a += 1;
			}
			mode = ADM_CHAT;
		}else{
			p = ConcatArgs(1);
			G_Say( ent, NULL, mode, p );
			return;
		}
	}
	else if ((strstr(lwrP, "!ca "))) {
			p = ConcatArgs(1);
			for(i=4;i<=strlen(p);i++){
			p[a] = p[i];
			a += 1;
			}
			mode = CADM_CHAT;
	}


	// Boe!Man 12/20/09
	Boe_Tokens(ent, p, mode);

	// End
	G_Say( ent, NULL, mode, p );
}

/*
==================
Cmd_Tell_f
==================
*/
static void Cmd_Tell_f( gentity_t *ent ) {
	int			targetNum;
	gentity_t	*target;
	char		*p;
	char		arg[MAX_TOKEN_CHARS];

	if ( trap_Argc () < 2 ) {
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	targetNum = atoi( arg );
	if ( targetNum < 0 || targetNum >= level.maxclients ) {
		return;
	}

	target = &g_entities[targetNum];
	if ( !target || !target->inuse || !target->client ) {
		return;
	}

	p = ConcatArgs( 2 );

	G_LogPrintf( "tell: %s to %s: %s\n", ent->client->pers.netname, target->client->pers.netname, p );
	G_Say( ent, target, SAY_TELL, p );
	// don't tell to the player self if it was already directed to this player
	// also don't send the chat back to a bot
	if ( ent != target && !(ent->r.svFlags & SVF_BOT)) {
		G_Say( ent, ent, SAY_TELL, p );
	}
}


static void G_VoiceTo ( gentity_t *ent, gentity_t *other, int mode, const char* name, const char *id, qboolean voiceonly ) 
{
	// Only team say is supported right now for voice chatting
	if (mode != SAY_TEAM) 
	{
		return;
	}

	if (!other || !other->inuse || !other->client) 
	{
		return;
	}

	if ( !OnSameTeam(ent, other) ) 
	{
		return;
	}

	trap_SendServerCommand( other-g_entities, va("%s %d %d \"%s\" \"%s\"", "vtchat", voiceonly, ent->s.number, name, id));
}

/*
==================
G_CanVoiceGlobal

Can we globaly speak right now
==================
*/
qboolean G_CanVoiceGlobal ( void )
{
	if ( level.gametypeData->teams && level.time - level.globalVoiceTime > 5000 )
	{
		return qtrue;
	}

	return qfalse;
}

/*
==================
G_VoiceGlobal

says something out loud that everyone in the radius can hear
==================
*/
void G_VoiceGlobal ( gentity_t* ent, const char* id, qboolean force )
{
	if ( !ent )
	{
		return;
	}

	if ( !level.gametypeData->teams )
	{
		return;
	}

	if ( !force && level.time - level.globalVoiceTime < 5000 )
	{
		return;
	}
		
	level.globalVoiceTime = level.time;

	trap_SendServerCommand( -1, va("vglobal %d \"%s\"", ent->s.number, id));
}

/*
==================
G_Voice
==================
*/
void G_Voice( gentity_t *ent, gentity_t *target, int mode, const char *id, qboolean voiceonly ) 
{
	int			j;
	gentity_t	*other;
	char		name[MAX_SAY_TEXT];

	// Spectators and ghosts dont talk
	if ( ent->client->ps.pm_type == PM_DEAD || G_IsClientSpectating ( ent->client ) )
	{
		return;
	}

	// Voice flooding protection on?
	if ( g_voiceFloodCount.integer )
	{
		// If this client has been penalized for voice chatting to much then dont allow the voice chat
		if ( ent->client->voiceFloodPenalty )
		{
			if ( ent->client->voiceFloodPenalty > level.time )
			{
				return;
			}

			// No longer penalized
			ent->client->voiceFloodPenalty = 0;
		}

		// See if this client flooded with voice chats
		ent->client->voiceFloodCount++;
		if ( ent->client->voiceFloodCount >= g_voiceFloodCount.integer )
		{
			ent->client->voiceFloodCount = 0;
			ent->client->voiceFloodTimer = 0;
			ent->client->voiceFloodPenalty = level.time + g_voiceFloodPenalty.integer * 1000;

			trap_SendServerCommand( ent-g_entities, va("print \"Voice chat flooded, you will be able use voice chats again in (%d) seconds\n\"", g_voiceFloodPenalty.integer ) );

			return;
		}
	}

	G_GetChatPrefix ( ent, target, mode, name, sizeof(name) );

	if ( target ) 
	{
		G_VoiceTo( ent, target, mode, name, id, voiceonly );
		return;
	}

	// send it to all the apropriate clients
	for (j = 0; j < level.maxclients; j++) 
	{
		other = &g_entities[j];
		G_VoiceTo( ent, other, mode, name, id, voiceonly );
	}
}

/*
==================
Cmd_Voice_f
==================
*/
static void Cmd_Voice_f( gentity_t *ent, int mode, qboolean arg0, qboolean voiceonly ) 
{
	char		*p;

	if ( trap_Argc () < 2 && !arg0 ) {
		return;
	}

	if (arg0)
	{
		p = ConcatArgs( 0 );
	}
	else
	{
		p = ConcatArgs( 1 );
	}

	G_Voice( ent, NULL, mode, p, voiceonly );
}

/*
==================
Cmd_Where_f
==================
*/
void Cmd_Where_f( gentity_t *ent ) 
{
	trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", vtos( ent->s.origin ) ) );
}

/*
============
G_VoteDisabled

determins if the given vote is disabled
============
*/
int G_VoteDisabled ( const char* callvote ) 
{
	return trap_Cvar_VariableIntegerValue( va("novote_%s", callvote) );
}

/*
==================
Cmd_CallVote_f
==================
*/
void Cmd_CallVote_f( gentity_t *ent ) 
{
	int		i;
	char	arg1[MAX_STRING_TOKENS];
	char	arg2[MAX_STRING_TOKENS];

	if ( !g_allowVote.integer ) 
	{
		trap_SendServerCommand( ent-g_entities, "print \"Voting not allowed here.\n\"" );
		return;
	}

	if ( level.intermissiontime || level.intermissionQueued )
	{
		trap_SendServerCommand( ent-g_entities, "print \"Voting not allowed during intermission.\n\"" );
		return;
	}

	// No voting within the minute of a map change
	if ( level.time - level.startTime < 1000 * 60 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"Cannot vote within the first minute of a map change.\n\"" );
		return;
	}

	if ( level.numConnectedClients > 1 && level.numVotingClients == 1 ) 
	{
		trap_SendServerCommand( ent-g_entities, "print \"You need at least 2 clients to call a vote.\n\"" );
		return;
	}

	if ( level.voteTime ) 
	{
		trap_SendServerCommand( ent-g_entities, "print \"A vote is already in progress.\n\"" );
		return;
	}

	if ( ent->client->pers.voteCount >= MAX_VOTE_COUNT ) 
	{
		trap_SendServerCommand( ent-g_entities, "print \"You have called the maximum number of votes.\n\"" );
		return;
	}
	
	if ( ent->client->sess.team == TEAM_SPECTATOR ) 
	{
		trap_SendServerCommand( ent-g_entities, "print \"Not allowed to call a vote as spectator.\n\"" );
		return;
	}

	if ( ent->client->voteDelayTime > level.time )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You are not allowed to vote within %d minute of a failed vote.\n\"", g_failedVoteDelay.integer ) );
		return;
	}
		
	// Save the voting client id
	level.voteClient = ent->s.number;

	// make sure it is a valid command to vote on
	trap_Argv( 1, arg1, sizeof( arg1 ) );
	trap_Argv( 2, arg2, sizeof( arg2 ) );

	if( strchr( arg1, ';' ) || strchr( arg2, ';' ) ) 
	{
		trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
		return;
	}

	if ( !Q_stricmp( arg1, "map_restart" ) ) {
	} else if ( !Q_stricmp( arg1, "mapcycle" ) ) {
	} else if ( !Q_stricmp( arg1, "map" ) ) {
	} else if ( !Q_stricmp( arg1, "rmgmap" ) ) {
	} else if ( !Q_stricmp( arg1, "g_gametype" ) ) {
	} else if ( !Q_stricmp( arg1, "kick" ) ) {
	} else if ( !Q_stricmp( arg1, "clientkick" ) ) {
	} else if ( !Q_stricmp( arg1, "g_doWarmup" ) ) {
	} else if ( !Q_stricmp( arg1, "timelimit" ) ) {
	} else if ( !Q_stricmp( arg1, "timeextension" ) ) {
	} else if ( !Q_stricmp( arg1, "scorelimit" ) ) {
	} else 
	{
		trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
		trap_SendServerCommand( ent-g_entities, "print \"Vote commands are: map_restart, nextmap, map <mapname>, g_gametype <n>, kick <player>, clientkick <clientnum>, g_doWarmup, timelimit <time>, scorelimit <score>.\n\"" );
		return;
	}

	// see if this particular vote is disabled
	if ( G_VoteDisabled ( arg1 ) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"The '%s' vote has been disabled on this server.\n\"", arg1) );
		return;
	}	

	// if there is still a vote to be executed
	if ( level.voteExecuteTime ) 
	{
		level.voteExecuteTime = 0;
		trap_SendConsoleCommand( EXEC_APPEND, va("%s\n", level.voteString ) );
	}

	// special case for g_gametype, check for bad values
	if ( !Q_stricmp( arg1, "g_gametype" ) ) 
	{
		// Verify the gametype
		i = BG_FindGametype ( arg2 );
		if ( i < 0 )
		{
			trap_SendServerCommand( ent-g_entities, "print \"Invalid gametype.\n\"" );
			return;
		}	

		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %s", arg1, arg2 );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s %s", arg1, bg_gametypeData[i].name );
	} 
	else if ( !Q_stricmp( arg1, "map" ) ) 
	{
		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %s", arg1, arg2 );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
	} 
	else if ( !Q_stricmp( arg1, "rmgmap" ) ) 
	{
		char	arg3[MAX_STRING_TOKENS];
		char	arg4[MAX_STRING_TOKENS];

		trap_Argv( 3, arg3, sizeof( arg3 ) );
		trap_Argv( 4, arg4, sizeof( arg4 ) );

		Com_sprintf( level.voteString, sizeof( level.voteString ), "rmgmap 1 %s 2 %s 3 %s 4 \"%s\" 0", arg2, arg3, arg4, ConcatArgs ( 5 ) );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
	}
	else if ( !Q_stricmp( arg1, "mapcycle" ) ) 
	{
		if (!*g_mapcycle.string || !Q_stricmp ( g_mapcycle.string, "none" ) ) 
		{
			trap_SendServerCommand( ent-g_entities, "print \"there is no map cycle ccurently set up.\n\"" );
			return;
		}

		Com_sprintf( level.voteString, sizeof( level.voteString ), "mapcycle");
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "nextmap" );
	} 
	else if ( !Q_stricmp ( arg1, "clientkick" ) )
	{
		int n = atoi ( arg2 );

		if ( n < 0 || n >= MAX_CLIENTS )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"invalid client number %d.\n\"", n ) );
			return;
		}

		if ( g_entities[n].client->pers.connected == CON_DISCONNECTED )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"there is no client with the client number %d.\n\"", n ) );
			return;
		}
			
		Com_sprintf ( level.voteString, sizeof(level.voteString ), "%s %s", arg1, arg2 );
		Com_sprintf ( level.voteDisplayString, sizeof(level.voteDisplayString), "kick %s", g_entities[n].client->pers.netname );
	}
	else if ( !Q_stricmp ( arg1, "kick" ) )
	{
		int clientid = G_ClientNumberFromName ( arg2 );

		if ( clientid == -1 )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"there is no client named '%s' currently on the server.\n\"", arg2 ) );
			return;
		}

		Com_sprintf ( level.voteString, sizeof(level.voteString ), "clientkick %d", clientid );
		Com_sprintf ( level.voteDisplayString, sizeof(level.voteDisplayString), "kick %s", g_entities[clientid].client->pers.netname );
	}
	else if ( !Q_stricmp ( arg1, "timeextension" ) )
	{
		if ( !g_timelimit.integer )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"There is no timelimit to extend.\n\"") );
			return;
		}

		if ( !g_timeextension.integer )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"This server does not allow time extensions.\n\"") );
			return;
		}
		Com_sprintf ( level.voteString, sizeof(level.voteString ), "extendtime %d", g_timeextension.integer );
		Com_sprintf ( level.voteDisplayString, sizeof(level.voteDisplayString), "extend timelimit by %d minutes", g_timeextension.integer );
	}
	else 
	{
		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s \"%s\"", arg1, arg2 );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
	}

	trap_SendServerCommand( -1, va("print \"%s called a vote.\n\"", ent->client->pers.netname ) );

	// start the voting, the caller autoamtically votes yes
	level.voteTime = level.time;
	level.voteYes = 1;
	level.voteNo = 0;

	for ( i = 0 ; i < level.maxclients ; i++ ) 
	{
		level.clients[i].ps.eFlags &= ~EF_VOTED;
	}
	ent->client->ps.eFlags |= EF_VOTED;

	trap_SetConfigstring( CS_VOTE_TIME, va("%i,%i", level.voteTime, g_voteDuration.integer*1000 ) );
	trap_SetConfigstring( CS_VOTE_STRING, level.voteDisplayString );	
	trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
	trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );	
	trap_SetConfigstring( CS_VOTE_NEEDED, va("%i", level.numVotingClients / 2 ) );
}

/*
==================
Cmd_Vote_f
==================
*/
void Cmd_Vote_f( gentity_t *ent ) 
{
	char msg[64];

	if ( !level.voteTime ) 
	{
		trap_SendServerCommand( ent-g_entities, "print \"No vote in progress.\n\"" );
		return;
	}

	if ( ent->client->ps.eFlags & EF_VOTED ) 
	{
		trap_SendServerCommand( ent-g_entities, "print \"Vote already cast.\n\"" );
		return;
	}

	if ( ent->client->sess.team == TEAM_SPECTATOR ) 
	{
		trap_SendServerCommand( ent-g_entities, "print \"Not allowed to vote as spectator.\n\"" );
		return;
	}

	trap_SendServerCommand( ent-g_entities, "print \"Vote cast.\n\"" );

	ent->client->ps.eFlags |= EF_VOTED;

	trap_Argv( 1, msg, sizeof( msg ) );

	if ( msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1' ) 
	{
		level.voteYes++;
		trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
	} 
	else 
	{
		level.voteNo++;
		trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );	
	}

	// a majority will be determined in CheckVote, which will also account
	// for players entering or leaving
}


/*
=================
Cmd_SetViewpos_f
=================
*/
void Cmd_SetViewpos_f( gentity_t *ent ) 
{
	vec3_t		origin, angles;
	char		buffer[MAX_TOKEN_CHARS];
	int			i;

	if ( !g_cheats.integer ) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Cheats are not enabled on this server.\n\""));
		return;
	}
	
	if ( trap_Argc() != 5 ) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"usage: setviewpos x y z yaw\n\""));
		return;
	}

	VectorClear( angles );
	for ( i = 0 ; i < 3 ; i++ ) 
	{
		trap_Argv( i + 1, buffer, sizeof( buffer ) );
		origin[i] = atof( buffer );
	}

	trap_Argv( 4, buffer, sizeof( buffer ) );
	angles[YAW] = atof( buffer );

	TeleportPlayer( ent, origin, angles );
}

/*
====================
IP2Country by Henkie
4/1/10 - 12:04 AM
====================
*/

void AddIPList(char ip[24], char country[128], char ext[6])
{
	int				len;
	fileHandle_t	f;
	char			*file;
	char			*string;
	Com_Printf("Adding ip to list...\n");
	file = va("country\\IP.known");
	len = trap_FS_FOpenFile( file, &f, FS_APPEND_TEXT );
	if (!f)
	{
		Com_Printf("^1Error opening File\n");
		return;
	}
	string = va("1\n{\nip \"%s\"\ncountry \"%s\"\next \"%s\"\n}\n", ip, country, ext);

	trap_FS_Write(string, strlen(string), f);
	trap_FS_Write("\n", 1, f);
	trap_FS_FCloseFile(f);

}

qboolean CheckIP(gentity_t *ent){
	void	*GP2, *group;
	char	*filePtr, *file, Files[1024];
	char	ip[24], country[128], ext[6];
	int		fileCount;
	//Com_Printf("Checking in known list\n");
	fileCount = trap_FS_GetFileList( "country", ".known", Files, 1024 );
	filePtr = Files;
	file = va("country\\%s", filePtr);
	GP2 = trap_GP_ParseFile(file, qtrue, qfalse);
	if (!GP2)
	{
		G_LogPrintf("Error in file: \"%s\" or file not found.\n", file);
		return qfalse;
	}
		group = trap_GPG_GetSubGroups(GP2);

		while(group)
		{
			trap_GPG_FindPairValue(group, "ip", "0", ip);
			if(strstr(ip, ent->client->pers.ip)){
				trap_GPG_FindPairValue(group, "country", "", country);
				trap_GPG_FindPairValue(group, "ext", "", ext);
				trap_GP_Delete(&GP2);
				strcpy(ent->client->sess.country, country);
				strcpy(ent->client->sess.countryext, ext);
				//Com_Printf("Found in known list\n");
				return qtrue;
				break; // stop searching
			}
			group = trap_GPG_GetNext(group);
		}
		strcpy(ent->client->sess.countryext, "??");
		trap_GP_Delete(&GP2);
		//Com_Printf("Not found in known list\n");
		return qfalse;
}

void HENK_COUNTRY(gentity_t *ent){
	void	*GP2, *group;
	char	*filePtr, *file, Files[1024];
	int		fileCount;
	char	*IP;
	int		count = 0;
	int		iIP, i, z, countx[4], loops = 0;
	char	begin_ip[24], end_ip[24], country[128], ext[6];
	int		begin_ipi, end_ipi;

	char	octet[4][4], octetx[4][4];
	int		RealOctet[4];
	int		IPnum;

	//Com_sprintf(IP, 24, ent->client->pers.ip);
	IP = va("%s", ent->client->pers.ip);
	//Com_Printf("IP is: %s\n", ent->client->pers.ip);
	// Set countx to zero, when you do not set the variable you get weird ass results.
	countx[0] = 0;
	countx[1] = 0;
	countx[2] = 0;
	countx[3] = 0;
	// End
	while(*IP){
		if(*IP == '.')
		{
			for(i=0;i<count;i++){
				if(loops == 0){
				octet[0][i] = *--IP;
				countx[0] += 1;
				}else if(loops == 1){
				octet[1][i] = *--IP;
				countx[1] += 1;
				}else if(loops == 2){
				octet[2][i] = *--IP;
				countx[2] += 1;
				}
			}
			for(i=0;i<count;i++){
				*++IP;
			}
			loops += 1;
			count = 0;
		}else{
		count += 1;
		}
		IP++;
		if(*IP == '\0'){
			for(i=0;i<3;i++){
				if(*--IP != '.'){
				octet[3][i] = *IP;
				countx[3] += 1;
				}
			}
			break;
		}
	}
	for(i=0;i<=3;i++){ // 4 octets
		for(z=0;z<countx[i];z++){
		//Com_Printf("octet[%i][%i] -> trying\n", i, countx[i]-(z+1));
		octetx[i][z] = octet[i][countx[i]-(z+1)];
		}
		octetx[i][countx[i]] = '\0';
	}
	// End

	fileCount = trap_FS_GetFileList( "country", va(".%s", octetx[0]), Files, 1024 );
	filePtr = Files;
	file = va("country\\%s", filePtr);
	GP2 = trap_GP_ParseFile(file, qtrue, qfalse);
	if (!GP2)
	{
		G_LogPrintf("Error in file: \"%s\" or file not found.\n", file);
	}

	RealOctet[0] = atoi(octetx[0]);
	RealOctet[1] = atoi(octetx[1]);
	RealOctet[2] = atoi(octetx[2]);
	RealOctet[3] = atoi(octetx[3]);
	//Com_Printf("Octet1: %i\n", RealOctet[0]);
	//Com_Printf("Octet2: %i\n", RealOctet[1]);
	//Com_Printf("Octet3: %i\n", RealOctet[2]);
	//Com_Printf("Octet4: %i\n", RealOctet[3]);
	IPnum = (RealOctet[0] * 16777216) + (RealOctet[1] * 65536) + (RealOctet[2] * 256) + (RealOctet[3]);
	//Com_Printf("IPnum: %i\n", IPnum);

		group = trap_GPG_GetSubGroups(GP2);

		while(group)
		{
			trap_GPG_FindPairValue(group, "begin_ip", "0", begin_ip);
			begin_ipi = atoi(begin_ip);
			trap_GPG_FindPairValue(group, "end_ip", "", end_ip);
			end_ipi = atoi(end_ip);
			trap_GPG_FindPairValue(group, "country", "", country);
			trap_GPG_FindPairValue(group, "ext", "", ext);
			if(IPnum > begin_ipi && IPnum < end_ipi){
				// found entry
				trap_GP_Delete(&GP2);
				strcpy(ent->client->sess.country, country);
				strcpy(ent->client->sess.countryext, ext);
				AddIPList(ent->client->pers.ip, country, ext);
				return;
				break; // stop searching
			}
			group = trap_GPG_GetNext(group);
		}
		strcpy(ent->client->sess.countryext, "??");
		trap_GP_Delete(&GP2);
		return;
}

/*
=================
ClientCommand
=================
*/
void ClientCommand( int clientNum ) {
	gentity_t *ent;
	char	cmd[MAX_TOKEN_CHARS];

	ent = g_entities + clientNum;
	if ( !ent->client ) {
		return;		// not fully in game yet
	}


	trap_Argv( 0, cmd, sizeof( cmd ) );

	//rww - redirect bot commands
	if (strstr(cmd, "bot_") && AcceptBotCommand(cmd, ent))
	{
		return;
	}
	//end rww
	if (Q_stricmp (cmd, "say") == 0) {
		// Boe!Man 1/30/10: We need to make sure the clients aren't muted.. And otherwise prevent them talking.
		if(ent->client->sess.mute){
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You are currently muted by an Admin.\n\"") );
		return;
		}
		Cmd_Say_f (ent, SAY_ALL, qfalse);
		return;
	}
	if (Q_stricmp (cmd, "say_team") == 0) {
		// Boe!Man 1/30/10: We need to make sure the clients aren't muted.. And otherwise prevent them talking.
		if(ent->client->sess.mute){
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You are currently muted by an Admin.\n\"") );
		return;
		}
		Cmd_Say_f (ent, SAY_TEAM, qfalse);
		return;
	}
	
	if (Q_stricmp (cmd, "tell") == 0) 
	{
		// Boe!Man 1/30/10: We need to make sure the clients aren't muted.. And otherwise prevent them talking.
		if(ent->client->sess.mute){
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You are currently muted by an Admin.\n\"") );
		return;
		}
		Cmd_Tell_f ( ent );
		return;
	}

	if (Q_stricmp (cmd, "vsay_team") == 0) 
	{
		// Boe!Man 1/30/10: We need to make sure the clients aren't muted.. And otherwise prevent them talking.
		if(ent->client->sess.mute){
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You are currently muted by an Admin.\n\"") );
		return;
		}
		Cmd_Voice_f (ent, SAY_TEAM, qfalse, qfalse);
		return;
	}

	if (Q_stricmp (cmd, "score") == 0) {
		Cmd_Score_f (ent);
		return;
	}

	if (Q_stricmp (cmd, "team") == 0)
	{
		Cmd_Team_f (ent);
		return;
	}

	// ignore all other commands when at intermission
	if (level.intermissiontime) 
	{
//		Cmd_Say_f (ent, qfalse, qtrue);
		return;
	}

	if ( Q_stricmp ( cmd, "drop" ) == 0 )
		Cmd_Drop_f ( ent );
	else if (Q_stricmp (cmd, "give") == 0)
		Cmd_Give_f (ent);
	else if (Q_stricmp (cmd, "god") == 0)
		Cmd_God_f (ent);
	else if (Q_stricmp (cmd, "notarget") == 0)
		Cmd_Notarget_f (ent);
	else if (Q_stricmp (cmd, "noclip") == 0)
		Cmd_Noclip_f (ent);
	else if (Q_stricmp (cmd, "kill") == 0)
		Cmd_Kill_f (ent);
	else if (Q_stricmp (cmd, "levelshot") == 0)
		Cmd_LevelShot_f (ent);
	else if (Q_stricmp (cmd, "follow") == 0)
		Cmd_Follow_f (ent);
	else if (Q_stricmp (cmd, "follownext") == 0)
		Cmd_FollowCycle_f (ent, 1);
	else if (Q_stricmp (cmd, "followprev") == 0)
		Cmd_FollowCycle_f (ent, -1);
	else if (Q_stricmp (cmd, "where") == 0)
		Cmd_Where_f (ent);
	else if (Q_stricmp (cmd, "callvote") == 0)
		Cmd_CallVote_f (ent);
	else if (Q_stricmp (cmd, "vote") == 0)
		Cmd_Vote_f (ent);
	else if (Q_stricmp (cmd, "setviewpos") == 0)
		Cmd_SetViewpos_f( ent );
	// Boe!Man 3/30/10
	else if (Q_stricmp ( cmd, "motd" ) == 0 && !level.intermissionQueued){
		ent->client->sess.motdStartTime = level.time;
		ent->client->sess.motdStopTime = level.time + 7000;
		Boe_Motd( ent );}
	else if (Q_stricmp (cmd, "adm") == 0)
		Boe_adm_f( ent );
	else if (Q_stricmp (cmd, "about") == 0)
		Boe_About ( ent );
	// Boe!Man 3/31/10
	else if (Q_stricmp (cmd, "players") == 0)
		Boe_Players ( ent );
	else if (Q_stricmp (cmd, "stats") == 0)
		Boe_Stats ( ent );
	else if (Q_stricmp (cmd, "sounds") == 0)
		Boe_Display_Sounds( ent );
	else if (Q_stricmp (cmd, "sounds2") == 0)
		Boe_Display_Sounds2( ent );
	else if (Q_stricmp (cmd, "extrasounds") == 0)
		Boe_Display_ExtraSounds( ent );

#ifdef _SOF2_BOTS
	else if (Q_stricmp (cmd, "addbot") == 0)
		trap_SendServerCommand( clientNum, va("print \"ADDBOT command can only be used via RCON\n\"" ) );
#endif

	else
		trap_SendServerCommand( clientNum, va("print \"unknown cmd %s\n\"", cmd ) );
}
