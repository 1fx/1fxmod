// Copyright (C) 2001-2002 Raven Software
//
// g_bot.c

#include "g_local.h"


static int		g_numBots;
static char		*g_botInfos[MAX_BOTS];


int				g_numArenas;
static char		*g_arenaInfos[MAX_ARENAS];


#define BOT_BEGIN_DELAY_BASE		2000
#define BOT_BEGIN_DELAY_INCREMENT	1500

#define BOT_SPAWN_QUEUE_DEPTH	16

typedef struct {
	int		clientNum;
	int		spawnTime;
} botSpawnQueue_t;

//static int			botBeginDelay = 0;  // bk001206 - unused, init
static botSpawnQueue_t	botSpawnQueue[BOT_SPAWN_QUEUE_DEPTH];

vmCvar_t bot_minplayers;

float trap_Cvar_VariableValue( const char *var_name ) {
	char buf[128];

	trap_Cvar_VariableStringBuffer(var_name, buf, sizeof(buf));
	return atof(buf);
}



/*
===============
G_ParseInfos
===============
*/
int G_ParseInfos( const char *buf, int max, char *infos[] ) {
	const char	*token;
	int			count;
	char		key[MAX_TOKEN_CHARS];
	char		info[MAX_INFO_STRING];

	count = 0;

	while ( 1 ) {
		token = COM_Parse( &buf );
		if ( !token[0] ) {
			break;
		}
		if ( strcmp( token, "{" ) ) {
			Com_Printf( "Missing { in info file\n" );
			break;
		}

		if ( count == max ) {
			Com_Printf( "Max infos exceeded\n" );
			break;
		}

		info[0] = '\0';
		while ( 1 ) {
			token = COM_ParseExt( &buf, qtrue );
			if ( !token[0] ) {
				Com_Printf( "Unexpected end of info file\n" );
				break;
			}
			if ( !strcmp( token, "}" ) ) {
				break;
			}
			Q_strncpyz( key, token, sizeof( key ) );

			token = COM_ParseExt( &buf, qfalse );
			if ( !token[0] ) 
			{
				Info_SetValueForKey( info, key, "<NULL>" );
			}
			else
			{
				Info_SetValueForKey( info, key, token );
			}
		}
		//NOTE: extra space for arena number
		infos[count] = trap_VM_LocalAlloc(strlen(info) + strlen("\\num\\") + strlen(va("%d", MAX_ARENAS)) + 1);
		if (infos[count]) {
			strcpy(infos[count], info);
			count++;
		}
	}
	return count;
}

/*
===============
G_LoadArenasFromFile
===============
*/
static void G_LoadArenasFromFile( char *filename ) {
	int				len;
	fileHandle_t	f;
	char			buf[MAX_ARENAS_TEXT];

	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( !f ) {
		trap_Printf( va( S_COLOR_RED "file not found: %s\n", filename ) );
		return;
	}
	if ( len >= MAX_ARENAS_TEXT ) {
		trap_Printf( va( S_COLOR_RED "file too large: %s is %i, max allowed is %i", filename, len, MAX_ARENAS_TEXT ) );
		trap_FS_FCloseFile( f );
		return;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );

	g_numArenas += G_ParseInfos( buf, MAX_ARENAS - g_numArenas, &g_arenaInfos[g_numArenas] );
}

/*
===============
G_LoadArenas
===============
*/
void G_LoadArenas( void ) 
{
	int			numdirs;
	char		filename[128];
	char		dirlist[1024];
	char*		dirptr;
	int			i, n;
	int			dirlen;

	g_numArenas = 0;

	// get all arenas from .arena files
	numdirs = trap_FS_GetFileList("scripts", ".arena", dirlist, 1024 );
	dirptr  = dirlist;
	for (i = 0; i < numdirs; i++, dirptr += dirlen+1) 
	{
		dirlen = strlen(dirptr);
		strcpy(filename, "scripts/");
		strcat(filename, dirptr);
		G_LoadArenasFromFile(filename);
	}

#ifdef _DEBUG
	Com_Printf ( "%i arenas parsed\n", g_numArenas );
#endif
	
	for( n = 0; n < g_numArenas; n++ ) 
	{
		Info_SetValueForKey( g_arenaInfos[n], "num", va( "%i", n ) );
	}
}

/*
===============
G_GetArenaInfoByNumber
===============
*/
const char *G_GetArenaInfoByMap( const char *map ) 
{
	int n;

	for( n = 0; n < g_numArenas; n++ ) 
	{
		if( Q_stricmp( Info_ValueForKey( g_arenaInfos[n], "map" ), map ) == 0 ) 
		{
			return g_arenaInfos[n];
		}
	}

	return NULL;
}

/*
===============
G_DoesMapSupportGametype

determines whether or not the current map supports the given gametype
===============
*/
qboolean G_DoesMapSupportGametype ( const char* gametype )
{
	char		mapname[MAX_QPATH];
	const char* info;
	const char*	type;
	char*		token;

	// Figure out the current map name first
	if ( RMG.integer )
	{
		Com_sprintf ( mapname, MAX_QPATH, "*random" );
	}
	else
	{
		trap_Cvar_VariableStringBuffer ( "mapname", mapname, MAX_QPATH );
	}

	// Get the arena info for the current map 
	info = G_GetArenaInfoByMap ( mapname );
	if ( !info )
	{
		// If they dont have an area file for this map then
		// just assume it supports all gametypes
		return qtrue;
	}

	// Get the supported gametypes
	type = Info_ValueForKey( info, "type" );

	while ( 1 )
	{
		token = COM_Parse ( &type );
		if ( !token || !*token )
		{
			break;
		}

		if ( Q_stricmp ( gametype, token ) == 0 )
		{
			return qtrue;
		}
	}

	return qfalse;
}

/*
=================
PlayerIntroSound
=================
*/
static void PlayerIntroSound( const char *modelAndSkin ) {
	char	model[MAX_QPATH];
	char	*skin;

	Q_strncpyz( model, modelAndSkin, sizeof(model) );
	skin = Q_strrchr( model, '/' );
	if ( skin ) {
		*skin++ = '\0';
	}
	else {
		skin = model;
	}

	if( Q_stricmp( skin, "default" ) == 0 ) {
		skin = model;
	}

	trap_SendConsoleCommand( EXEC_APPEND, va( "play sound/player/announce/%s.wav\n", skin ) );
}

/*
===============
G_AddRandomBot
===============
*/
void G_AddRandomBot( int team ) 
{
	int			i;
	int			n;
	int			num;
	float		skill;
	char		netname[36];
	char		*value;
	char		*teamstr;
	gclient_t	*cl;

	num = 0;
	
	for ( n = 0; n < g_numBots ; n++ ) 
	{
		value = Info_ValueForKey( g_botInfos[n], "name" );
	
		//
		for ( i=0 ; i< g_maxclients.integer ; i++ ) 
		{
			cl = level.clients + i;
			if ( cl->pers.connected != CON_CONNECTED ) 
			{
				continue;
			}
			
			if ( !(g_entities[cl->ps.clientNum].r.svFlags & SVF_BOT) ) 
			{
				continue;
			}
			
			if ( team >= 0 && cl->sess.team != team ) 
			{
				continue;
			}
			
			if ( !Q_stricmp( value, cl->pers.netname ) ) 
			{
				break;
			}
		}
		
		if (i >= g_maxclients.integer) 
		{
			num++;
		}
	}
	
	num = random() * num;
	
	for ( n = 0; n < g_numBots ; n++ ) 
	{
		value = Info_ValueForKey( g_botInfos[n], "name" );
		//
		for ( i=0 ; i< g_maxclients.integer ; i++ ) 
		{
			cl = level.clients + i;
			if ( cl->pers.connected != CON_CONNECTED ) 
			{
				continue;
			}
			
			if ( !(g_entities[cl->ps.clientNum].r.svFlags & SVF_BOT) ) 
			{
				continue;
			}
			
			if ( team >= 0 && cl->sess.team != team ) 
			{
				continue;
			}
			
			if ( !Q_stricmp( value, cl->pers.netname ) ) 
			{
				break;
			}
		}
		
		if (i >= g_maxclients.integer) 
		{
			num--;
			
			if (num <= 0) 
			{
				skill = trap_Cvar_VariableValue( "g_botSkill" );
				if (team == TEAM_RED) teamstr = "red";
				else if (team == TEAM_BLUE) teamstr = "blue";
				else teamstr = "";
				strncpy(netname, value, sizeof(netname)-1);
				netname[sizeof(netname)-1] = '\0';
				Q_CleanStr(netname);
				trap_SendConsoleCommand( EXEC_INSERT, va("addbot %s %f %s %i\n", netname, skill, teamstr, 0) );
				return;
			}
		}
	}
}

/*
===============
G_RemoveRandomBot
===============
*/
int G_RemoveRandomBot( int team ) 
{
	int			i;
	//char		netname[36];	///RxCxW - 09.09.06 - 11:55pm #REMOVED #kickBot
	gclient_t	*cl;

	for ( i=0 ; i< g_maxclients.integer ; i++ ) 
	{
		cl = level.clients + i;
		
		if ( cl->pers.connected != CON_CONNECTED ) 
		{
			continue;
		}
		
		if ( !(g_entities[cl->ps.clientNum].r.svFlags & SVF_BOT) ) 
		{
			continue;
		}
		
		if ( team >= 0 && cl->sess.team != team ) 
		{
			continue;
		}
	
		///RxCxW - 09.03.06 - 03:59pm #kickBot
		///strcpy(netname, cl->pers.netname);
		///Q_CleanStr(netname);
		///trap_SendConsoleCommand( EXEC_INSERT, va("kick \"%s\"\n", netname) );
		trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%i\"\n", cl->ps.clientNum) );
		///End  - 09.03.06 - 03:59pm
		return qtrue;
	}

	return qfalse;
}

/*
===============
G_CountHumanPlayers
===============
*/
int G_CountHumanPlayers( int team ) 
{
	int			i;
	int			num;
	gclient_t	*cl;

	num = 0;
	for ( i=0 ; i< g_maxclients.integer ; i++ ) 
	{
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) 
		{
			continue;
		}

		if ( g_entities[cl->ps.clientNum].r.svFlags & SVF_BOT ) 
		{
			continue;
		}

		if ( team >= 0 && cl->sess.team != team ) 
		{
			continue;
		}

		num++;
	}

	return num;
}

/*
===============
G_CountBotPlayers
===============
*/
int G_CountBotPlayers( int team ) 
{
	int			i;
	int			n;
	int			num;
	gclient_t	*cl;

	num = 0;
	
	for ( i=0 ; i< g_maxclients.integer ; i++ ) 
	{
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) 
		{
			continue;
		}

		if ( !(g_entities[cl->ps.clientNum].r.svFlags & SVF_BOT) ) 
		{
			continue;
		}

		if ( team >= 0 && cl->sess.team != team ) 
		{
			continue;
		}

		num++;
	}
	
	for( n = 0; n < BOT_SPAWN_QUEUE_DEPTH; n++ ) 
	{
		if( !botSpawnQueue[n].spawnTime ) 
		{
			continue;
		}
		
		if ( botSpawnQueue[n].spawnTime > level.time ) 
		{
			continue;
		}
		
		num++;
	}

	return num;
}

/*
===============
G_CheckMinimumPlayers
===============
*/
void G_CheckMinimumPlayers( void ) 
{
	int			minplayers;
	int			humanplayers;
	int			botplayers;
	static int	checkminimumplayers_time;

	if (level.intermissiontime) 
	{
		return;
	}

	// only check once each 10 seconds
	if (checkminimumplayers_time > level.time - 10000) 
	{
		return;
	}

	checkminimumplayers_time = level.time;
	trap_Cvar_Update(&bot_minplayers);
	minplayers = bot_minplayers.integer;

	if (minplayers <= 0) 
	{
		return;
	}

	if ( level.gametypeData->teams ) 
	{
		if (minplayers >= g_maxclients.integer / 2) 
		{
			minplayers = (g_maxclients.integer / 2) -1;
		}

		humanplayers = G_CountHumanPlayers( TEAM_RED );
		botplayers = G_CountBotPlayers(	TEAM_RED );
		
		//
		if (humanplayers + botplayers < minplayers) 
		{
			G_AddRandomBot( TEAM_RED );
		} 
		else if (humanplayers + botplayers > minplayers && botplayers) 
		{
			G_RemoveRandomBot( TEAM_RED );
		}
		
		//
		humanplayers = G_CountHumanPlayers( TEAM_BLUE );
		botplayers = G_CountBotPlayers( TEAM_BLUE );
		
		//
		if (humanplayers + botplayers < minplayers) 
		{
			G_AddRandomBot( TEAM_BLUE );
		} 
		else if (humanplayers + botplayers > minplayers && botplayers) 
		{
			G_RemoveRandomBot( TEAM_BLUE );
		}
	}
	else 
	{
		if (minplayers >= g_maxclients.integer) 
		{
			minplayers = g_maxclients.integer-1;
		}

		humanplayers = G_CountHumanPlayers( TEAM_FREE );
		botplayers = G_CountBotPlayers( TEAM_FREE );
		//
		
		if (humanplayers + botplayers < minplayers) 
		{
			G_AddRandomBot( TEAM_FREE );
		} 
		else if (humanplayers + botplayers > minplayers && botplayers) 
		{
			G_RemoveRandomBot( TEAM_FREE );
		}
	}
}

/*
===============
G_CheckBotSpawn
===============
*/
void G_CheckBotSpawn( void ) 
{
	int		n;

	G_CheckMinimumPlayers();

	for( n = 0; n < BOT_SPAWN_QUEUE_DEPTH; n++ ) 
	{
		if( !botSpawnQueue[n].spawnTime ) 
		{
			continue;
		}
		if ( botSpawnQueue[n].spawnTime > level.time ) 
		{
			continue;
		}
		//Ryan april 10 2004 10:07am
		//ClientBegin( botSpawnQueue[n].clientNum );
		ClientBegin( botSpawnQueue[n].clientNum, qtrue );
		//Ryan
		botSpawnQueue[n].spawnTime = 0;
	}
}


/*
===============
AddBotToSpawnQueue
===============
*/
static void AddBotToSpawnQueue( int clientNum, int delay ) 
{
	int		n;

	for( n = 0; n < BOT_SPAWN_QUEUE_DEPTH; n++ ) 
	{
		if( !botSpawnQueue[n].spawnTime ) 
		{
			botSpawnQueue[n].spawnTime = level.time + delay;
			botSpawnQueue[n].clientNum = clientNum;
			return;
		}
	}

	Com_Printf( S_COLOR_YELLOW "Unable to delay spawn\n" );
	//Ryan april 10 2004 10:08am
	//ClientBegin( clientNum );
	ClientBegin( clientNum, qtrue );
	//Ryan
}

/*
===============
G_RemoveQueuedBotBegin

Called on client disconnect to make sure the delayed spawn
doesn't happen on a freed index
===============
*/
void G_RemoveQueuedBotBegin( int clientNum ) {
	int		n;

	for( n = 0; n < BOT_SPAWN_QUEUE_DEPTH; n++ ) {
		if( botSpawnQueue[n].clientNum == clientNum ) {
			botSpawnQueue[n].spawnTime = 0;
			return;
		}
	}
}


/*
===============
G_BotConnect
===============
*/
qboolean G_BotConnect( int clientNum, qboolean restart ) {
	bot_settings_t	settings;
	char			userinfo[MAX_INFO_STRING];

	trap_GetUserinfo( clientNum, userinfo, sizeof(userinfo) );

	Q_strncpyz( settings.personalityfile, Info_ValueForKey( userinfo, "personality" ), sizeof(settings.personalityfile) );
	settings.skill = atof( Info_ValueForKey( userinfo, "skill" ) );
	Q_strncpyz( settings.team, Info_ValueForKey( userinfo, "team" ), sizeof(settings.team) );

	if (!BotAISetupClient( clientNum, &settings, restart )) {
		trap_DropClient( clientNum, "BotAISetupClient failed" );
		return qfalse;
	}

	return qtrue;
}


/*
===============
G_AddBot
===============
*/
static void G_AddBot( const char *name, float skill, const char *team, int delay, char *altname) 
{
	int				clientNum;
	char			*botinfo;
	gentity_t		*bot;
	char			*key;
	char			*s;
	char			*botname;
	char			*identity;
	char			userinfo[MAX_INFO_STRING];

	// get the botinfo from bots.txt
	botinfo = G_GetBotInfoByName( name );
	if ( !botinfo ) {
		Com_Printf( S_COLOR_RED "Error: Bot '%s' not defined\n", name );
		return;
	}

	// create the bot's userinfo
	userinfo[0] = '\0';

	botname = Info_ValueForKey( botinfo, "funname" );
	if( !botname[0] ) {
		botname = Info_ValueForKey( botinfo, "name" );
	}
	// check for an alternative name
	if (altname && altname[0]) {
		botname = altname;
	}
	Info_SetValueForKey( userinfo, "name", botname );
	Info_SetValueForKey( userinfo, "rate", "25000" );
	Info_SetValueForKey( userinfo, "snaps", "20" );
	Info_SetValueForKey( userinfo, "skill", va("%1.2f", skill) );

	if ( skill >= 1 && skill < 2 ) {
		Info_SetValueForKey( userinfo, "handicap", "50" );
	}
	else if ( skill >= 2 && skill < 3 ) {
		Info_SetValueForKey( userinfo, "handicap", "70" );
	}
	else if ( skill >= 3 && skill < 4 ) {
		Info_SetValueForKey( userinfo, "handicap", "90" );
	}

	key = "identity";
	identity = Info_ValueForKey( botinfo, key );
	if ( !*identity ) 
	{
		identity = "mullinsjungle";
	}
	Info_SetValueForKey( userinfo, key, identity );

	s = Info_ValueForKey(botinfo, "personality");
	if (!*s )
	{
		Info_SetValueForKey( userinfo, "personality", "botfiles/default.jkb" );
	}
	else
	{
		Info_SetValueForKey( userinfo, "personality", s );
	}

	// have the server allocate a client slot
	clientNum = trap_BotAllocateClient();
	if ( clientNum == -1 ) {
		Com_Printf( S_COLOR_RED "Unable to add bot.  All player slots are in use.\n" );
		Com_Printf( S_COLOR_RED "Start server with more 'open' slots (or check setting of sv_maxclients cvar).\n" );
		return;
	}

	// initialize the bot settings
	if( !team || !*team || !Q_stricmp ( team, "auto" )) 
	{
		if( level.gametypeData->teams ) 
		{
			if( PickTeam(clientNum) == TEAM_RED) 
			{
				team = "red";
			}
			else 
			{
				team = "blue";
			}
		}
		else 
		{
			team = "red";
		}
	}
//	Info_SetValueForKey( userinfo, "characterfile", Info_ValueForKey( botinfo, "aifile" ) );
	Info_SetValueForKey( userinfo, "skill", va( "%5.2f", skill ) );
	Info_SetValueForKey( userinfo, "team", team );

	bot = &g_entities[ clientNum ];
	bot->r.svFlags |= SVF_BOT;
	bot->inuse = qtrue;

	// register the userinfo
	trap_SetUserinfo( clientNum, userinfo );

	// have it connect to the game as a normal client
	if ( ClientConnect( clientNum, qtrue, qtrue ) ) {
		return;
	}

	if ( level.gametypeData->teams )
	{
		if (team && Q_stricmp(team, "red") == 0)
		{
			bot->client->sess.team = TEAM_RED;
		}
		else if (team && Q_stricmp(team, "blue") == 0)
		{
			bot->client->sess.team = TEAM_BLUE;
		}
		else
		{
			bot->client->sess.team = PickTeam( -1 );
		}
	}

	if( delay == 0 ) {
		//Ryan april 10 2004 10:09am
		//ClientBegin( clientNum );
		ClientBegin( clientNum, qtrue );
		//Ryan
		return;
	}

	AddBotToSpawnQueue( clientNum, delay );
}


/*
===============
Svcmd_AddBot_f
===============
*/
void Svcmd_AddBot_f( void ) {
	float			skill;
	int				delay;
	char			name[MAX_TOKEN_CHARS];
	char			altname[MAX_TOKEN_CHARS];
	char			string[MAX_TOKEN_CHARS];
	char			team[MAX_TOKEN_CHARS];

	// are bots enabled?
	if ( !trap_Cvar_VariableIntegerValue( "bot_enable" ) ) {
		return;
	}

	// name
	trap_Argv( 1, name, sizeof( name ) );
	if ( !name[0] ) {
		trap_Printf( "Usage: Addbot <botname> [skill 1-5] [team] [msec delay] [altname]\n" );
		return;
	}

	// skill
	trap_Argv( 2, string, sizeof( string ) );
	if ( !string[0] ) {
		skill = 4;
	}
	else {
		skill = atof( string );
	}

	// team
	trap_Argv( 3, team, sizeof( team ) );

	// delay
	trap_Argv( 4, string, sizeof( string ) );
	if ( !string[0] ) {
		delay = 0;
	}
	else {
		delay = atoi( string );
	}

	// alternative name
	trap_Argv( 5, altname, sizeof( altname ) );

	G_AddBot( name, skill, team, delay, altname );

	// if this was issued during gameplay and we are playing locally,
	// go ahead and load the bot's media immediately
	if ( level.time - level.startTime > 1000 && trap_Cvar_VariableIntegerValue( "cl_running" ) ) 
	{
		trap_SendServerCommand( -1, "loaddeferred\n" );
	}
}

/*
===============
Svcmd_BotList_f
===============
*/
void Svcmd_BotList_f( void ) {
	int i;
	char name[MAX_TOKEN_CHARS];
	char funname[MAX_TOKEN_CHARS];
	char model[MAX_TOKEN_CHARS];
	char personality[MAX_TOKEN_CHARS];

	trap_Printf("^1name             model            personality              funname\n");
	for (i = 0; i < g_numBots; i++) {
		strcpy(name, Info_ValueForKey( g_botInfos[i], "name" ));
		if ( !*name ) {
			strcpy(name, "UnnamedPlayer");
		}
		strcpy(funname, Info_ValueForKey( g_botInfos[i], "funname" ));
		if ( !*funname ) {
			strcpy(funname, "");
		}
		strcpy(model, Info_ValueForKey( g_botInfos[i], "model" ));
		if ( !*model ) {
			strcpy(model, "visor/default");
		}
		strcpy(personality, Info_ValueForKey( g_botInfos[i], "personality"));
		if (!*personality ) {
			strcpy(personality, "botfiles/default.jkb");
		}
		trap_Printf(va("%-16s %-16s %-20s %-20s\n", name, model, personality, funname));
	}
}


/*
===============
G_SpawnBots
===============
*/
static void G_SpawnBots( char *botList, int baseDelay ) {
	char		*bot;
	char		*p;
	float		skill;
	int			delay;
	char		bots[MAX_INFO_VALUE];

	skill = trap_Cvar_VariableValue( "g_botSkill" );
	if( skill < 2 ) 
	{
		trap_Cvar_Set( "g_botSkill", "2" );
		skill = 2;
	}
	else if ( skill > 5 ) 
	{
		trap_Cvar_Set( "g_botSkill", "5" );
		skill = 5;
	}

	Q_strncpyz( bots, botList, sizeof(bots) );
	p = &bots[0];
	delay = baseDelay;
	while( *p ) {
		//skip spaces
		while( *p && *p == ' ' ) {
			p++;
		}
		if( !p ) {
			break;
		}

		// mark start of bot name
		bot = p;

		// skip until space of null
		while( *p && *p != ' ' ) {
			p++;
		}
		if( *p ) {
			*p++ = 0;
		}

		// we must add the bot this way, calling G_AddBot directly at this stage
		// does "Bad Things"
		trap_SendConsoleCommand( EXEC_INSERT, va("addbot %s %f free %i\n", bot, skill, delay) );

		delay += BOT_BEGIN_DELAY_INCREMENT;
	}
}


/*
===============
G_LoadBotsFromFile
===============
*/
static void G_LoadBotsFromFile( char *filename ) {
	int				len;
	fileHandle_t	f;
	char			buf[MAX_BOTS_TEXT];

	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( !f ) {
		trap_Printf( va( S_COLOR_RED "file not found: %s\n", filename ) );
		return;
	}
	if ( len >= MAX_BOTS_TEXT ) {
		trap_Printf( va( S_COLOR_RED "file too large: %s is %i, max allowed is %i", filename, len, MAX_BOTS_TEXT ) );
		trap_FS_FCloseFile( f );
		return;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );

	g_numBots += G_ParseInfos( buf, MAX_BOTS - g_numBots, &g_botInfos[g_numBots] );
}

/*
===============
G_LoadBots
===============
*/
static void G_LoadBots( void ) {
	vmCvar_t	botsFile;
	int			numdirs;
	char		filename[128];
	char		dirlist[1024];
	char*		dirptr;
	int			i;
	int			dirlen;

	if ( !trap_Cvar_VariableIntegerValue( "bot_enable" ) ) {
		return;
	}

	g_numBots = 0;

	trap_Cvar_Register( &botsFile, "g_botsFile", "", CVAR_INIT|CVAR_ROM, 0.0, 0.0 );
	if( *botsFile.string ) {
		G_LoadBotsFromFile(botsFile.string);
	}
	else {
		//G_LoadBotsFromFile("scripts/bots.txt");
		G_LoadBotsFromFile("botfiles/bots.txt");
	}

	// get all bots from .bot files
	numdirs = trap_FS_GetFileList("scripts", ".bot", dirlist, 1024 );
	dirptr  = dirlist;
	for (i = 0; i < numdirs; i++, dirptr += dirlen+1) {
		dirlen = strlen(dirptr);
		strcpy(filename, "scripts/");
		strcat(filename, dirptr);
		G_LoadBotsFromFile(filename);
	}
	trap_Printf( va( "%i bots parsed\n", g_numBots ) );
}



/*
===============
G_GetBotInfoByNumber
===============
*/
char *G_GetBotInfoByNumber( int num ) {
	if( num < 0 || num >= g_numBots ) {
		trap_Printf( va( S_COLOR_RED "Invalid bot number: %i\n", num ) );
		return NULL;
	}
	return g_botInfos[num];
}


/*
===============
G_GetBotInfoByName
===============
*/
char *G_GetBotInfoByName( const char *name ) {
	int		n;
	char	*value;

	for ( n = 0; n < g_numBots ; n++ ) {
		value = Info_ValueForKey( g_botInfos[n], "name" );
		if ( !Q_stricmp( value, name ) ) {
			return g_botInfos[n];
		}
	}

	return NULL;
}

//rww - pd
void LoadPath_ThisLevel(void);
//end rww

/*
===============
G_InitBots
===============
*/
void G_InitBots( qboolean restart ) 
{
	G_LoadBots();

	trap_Cvar_Register( &bot_minplayers, "bot_minplayers", "0", CVAR_SERVERINFO, 0.0, 0.0 );

	//rww - new bot route stuff
	LoadPath_ThisLevel();
}

//Ryan march 8 2004 7:24
//Got this from v1.01 source code
/*
===============
G_DoesMapExist

determines whether or not the given map exists on the server
===============
*/
qboolean G_DoesMapExist ( const char* mapname )
{
	if ( G_GetArenaInfoByMap ( mapname ) )
	{
		return qtrue;
	}

	return qfalse;
}

// GRIM 15/04/2003 11:33PM
/* Taken from mandown, just to make the compiler stfu for now
==============
G_RandomlyChooseOutfitting
==============
*/
void G_RandomlyChooseOutfitting(gentity_t *ent, goutfitting_t *outfitting)
{
#ifdef _SOF2_BOTS
	int count;
	int i, j, k;

	if (ent->client->sess.team == TEAM_SPECTATOR)
	{
		return;
	}

	for(i = OUTFITTING_GROUP_PRIMARY; i < OUTFITTING_GROUP_MAX; i++)
	{
		outfitting->items[i] = 0;
		count = 0;
		
		// only ever choose armor - the rest is worthless
		if (i == OUTFITTING_GROUP_ACCESSORY)
		{
			outfitting->items[i] = 0;
			continue;
		}
		
		for(j = 0; j < MAX_OUTFITTING_GROUPITEM; j++)
		{
			// selecting nothing = 0, which doesn't count as a selection
			if (bg_outfittingGroups[i][j] == -1)
				break;

			count++;
		}

		// make sure we actually have a weapon in this group
		// (no no, I never devided by zero!! I swear!!!)
		if (count > 0)
		{
			// I hate loops...
			for(k = 0; k < 128; k++)
			{
				j = rand() % count;
			/*	// HACK - temp fix for shitty g_availableWeaponsString
				// just don't use MM1 or RPG
				if ((bg_itemlist[bg_outfittingGroups[i][j]].giTag == WP_MM1_GRENADE_LAUNCHER)
				 || (bg_itemlist[bg_outfittingGroups[i][j]].giTag == WP_RPG7_LAUNCHER))
				{
					continue;
				}
*/
				// FIX ME - g_availableWeaponsString doesn't work for server!!!
				if (BG_IsWeaponAvailableForOutfitting ( bg_itemlist[bg_outfittingGroups[i][j]].giTag, 2 ))
				{
					break;
				}
			}
			outfitting->items[i] = j;
		}
	}
#else
	return;
#endif
}
// GRIM

