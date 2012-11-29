// Copyright (C) 2001-2002 Raven Software.
//
#include "g_local.h"
#include "boe_local.h"

#include "../../ui/menudef.h"

// Henk 04/05/10 -> New command system(Yus this is very pro)
// Henk 04/05/10 -> New command system(Yus this is very pro)
typedef struct 
{
	char	*shortCmd; // short admin command, ex: !uc, !p(with space) -> HENK FIX ME: Need more entries here for uppercase.
	char	*adminCmd; // full adm command for /adm and rcon
	int		*adminLevel; // pointer to cvar value because we can't store a non constant value, so we store a pointer :).
	void	(*Function)(); // store pointer to the given function so we can call it later
} admCmd_t;

static admCmd_t AdminCommands[] = 
{
	// Boe!Man 1/22/11: Adding full synonyms.
	{"!uppercut","uppercut", &g_uppercut.integer, &Boe_Uppercut},
	{"!pop","pop", &g_pop.integer, &Boe_pop},
	{"!kick","kick", &g_kick.integer, &Boe_Kick},
	{"!addbadmin","addbadmin", &g_addbadmin.integer, &Boe_Add_bAdmin_f},
	{"!addadmin","addadmin", &g_addadmin.integer, &Boe_Add_Admin_f},
	{"!addsadmin","addsadmin", &g_addsadmin.integer, &Boe_Add_sAdmin_f},
	{"!twist","twist", &g_twist.integer, &Boe_Twist},
	{"!untwist","untwist", &g_twist.integer, &Boe_unTwist},
	{"!plant","plant", &g_plant.integer, &Boe_Plant},
	{"!unplant","unplant", &g_plant.integer, &Boe_unPlant},
	{"!runover","runover", &g_runover.integer, &Boe_Runover},
	{"!respawn","respawn", &g_respawn.integer, &Boe_Respawn},
	{"!maprestart","maprestart", &g_mapswitch.integer, &Boe_MapRestart},
	{"!burn","burn", &g_burn.integer, &Boe_Burn},
	{"!mute","mute", &g_mute.integer, &Boe_XMute},
	{"!unmute","unmute", &g_mute.integer, &Boe_UnMute},
	{"!strip","strip", &g_strip.integer, &Boe_Strip},
	{"!removeadmin","removeadmin", &g_removeadmin.integer, &Boe_Remove_Admin_f},
	{"!forceteam","forceteam", &g_forceteam.integer, &Adm_ForceTeam},
	{"!nolower","nolower", &g_nolower.integer, &Boe_NoLower},
	{"!noroof","noroof", &g_noroof.integer, &Boe_NoRoof},
	{"!shuffleteams","shuffleteams", &g_shuffleteams.integer, &Boe_ShuffleTeams},
	{"!shuffle","shuffle", &g_shuffleteams.integer, &Boe_ShuffleTeams},
	{"!nonades","nonades", &g_nades.integer, &Boe_NoNades},
	{"!scorelimit","scorelimit", &g_sl.integer, &Boe_ScoreLimit},
	{"!timelimit","timelimit", &g_tl.integer, &Boe_TimeLimit},
	{"!respawninterval","respawninterval", &g_ri.integer, &Boe_RespawnInterval},
	{"!realdamage","realdamage", &g_damage.integer, &Boe_RealDamage},
	{"!normaldamage","normaldamage", &g_damage.integer, &Boe_NormalDamage},
	{"!gametyperestart","gametyperestart", &g_gr.integer, &Boe_GametypeRestart},
	{"!addclan","addclan", &g_clan.integer, &Boe_Add_Clan_Member},
	{"!removeclan","removeclan", &g_clan.integer, &Boe_Remove_Clan_Member},
	{"!removeclanlist","removeclanlist", &g_clan.integer, &Henk_AdminRemove},
	{"!compmode","compmode", &g_cm.integer, &Boe_CompMode},
	{"!competitionmode","compmode", &g_cm.integer, &Boe_CompMode},
	{"!banlist","banlist", &g_ban.integer, &Henk_BanList},
	{"!ban","ban", &g_ban.integer, &Boe_Ban_f},
	{"!broadcast","broadcast", &g_broadcast.integer, &Boe_Broadcast},
	{"!subnetbanlist","subnetbanlist", &g_subnetban.integer, &Henk_SubnetBanList},
	{"!eventeams","eventeams", &g_eventeams.integer, &Henk_EvenTeams},
	{"!clanvsall","clanvsall", &g_clanvsall.integer, &Henk_CVA},
	{"!swapteams","swapteams", &g_swapteams.integer, &Henk_SwapTeams},
	{"!lock","lock", &g_lock.integer, &Henk_Lock},
	{"!unlock","unlock", &g_lock.integer, &Henk_Unlock},
	{"!flash","flash", &g_flash.integer, &Henk_Flash},
	{"!gametype","gametype", &g_mapswitch.integer, &Henk_Gametype},
	{"!unpause","unpause", &g_pause.integer, &Henk_Unpause},
	{"!pause","pause", &g_pause.integer, &Henk_Pause},
	{"!unban","unban", &g_ban.integer, &Henk_Unban},
	{"!subnetunban","subnetunban", &g_subnetban.integer, &Henk_SubnetUnban},
	{"!subnetban","subnetban", &g_subnetban.integer, &Boe_subnetBan},
	{"!pass","passvote", &g_forcevote.integer, &Boe_passVote},
	{"!cancel","cancelvote", &g_forcevote.integer, &Boe_cancelVote},
	{"!mapcycle","mapcycle", &g_mapswitch.integer, &Boe_Mapcycle},
	{"!adminlist","adminlist", &g_adminlist.integer, &Henk_Admlist},
	{"!clanlist","clanlist", &g_clan.integer, &Henk_Admlist},
	{"!adminremove","adminremove", &g_adminremove.integer, &Henk_AdminRemove},
	{"!friendlyfire","friendlyfire", &g_ff.integer, &Boe_friendlyFire},
	// Boe!Man 1/22/11: End full synonyms.

	{"!adr","adminremove", &g_adminremove.integer, &Henk_AdminRemove},
	{"!uc","uppercut", &g_uppercut.integer, &Boe_Uppercut},
	{"!ab","addbadmin", &g_addbadmin.integer, &Boe_Add_bAdmin_f},
	{"!aa","addadmin", &g_addadmin.integer, &Boe_Add_Admin_f},
	{"!as","addsadmin", &g_addsadmin.integer, &Boe_Add_sAdmin_f},
	{"!tw","twist", &g_twist.integer, &Boe_Twist},
	{"!utw","untwist", &g_twist.integer, &Boe_unTwist},
	{"!pl","plant", &g_plant.integer, &Boe_Plant},
	{"!upl","unplant", &g_plant.integer, &Boe_unPlant},
	{"!rounds","rounds", &g_cm.integer, &Boe_Rounds},
	{"!ro","runover", &g_runover.integer, &Boe_Runover},
	{"!rs","respawn", &g_respawn.integer, &Boe_Respawn}, // this is how we add synonyms
	{"!mr","maprestart", &g_mapswitch.integer, &Boe_MapRestart},
	{"!mr","map_restart", &g_mapswitch.integer, &Boe_MapRestart},
	{"!map","map", &g_mapswitch.integer, &Henk_Map},
	{"!altmap","altmap", &g_mapswitch.integer, &Henk_Map},
	{"!devmap","devmap", &g_mapswitch.integer, &Henk_Map},
	{"!um","unmute", &g_mute.integer, &Boe_UnMute},
	{"!st","strip", &g_strip.integer, &Boe_Strip},
	{"!ra","removeadmin", &g_removeadmin.integer, &Boe_Remove_Admin_f},
	{"!ft","forceteam", &g_forceteam.integer, &Adm_ForceTeam},
	{"!nl","nolower", &g_nolower.integer, &Boe_NoLower},
	{"!nr","noroof", &g_noroof.integer, &Boe_NoRoof},
	{"!sh","shuffleteams", &g_shuffleteams.integer, &Boe_ShuffleTeams},
	{"!nn","nonades", &g_nades.integer, &Boe_NoNades},
	{"!sl","scorelimit", &g_sl.integer, &Boe_ScoreLimit},
	{"!tl","timelimit", &g_tl.integer, &Boe_TimeLimit},
	{"!ri","respawninterval", &g_ri.integer, &Boe_RespawnInterval},
	{"!rd","realdamage", &g_damage.integer, &Boe_RealDamage},
	{"!nd","normaldamage", &g_damage.integer, &Boe_NormalDamage},
	{"!gr","gametyperestart", &g_gr.integer, &Boe_GametypeRestart},
	{"!acl","addclan", &g_clan.integer, &Boe_Add_Clan_Member},
	{"!rc","removeclan", &g_clan.integer, &Boe_Remove_Clan_Member},
	{"!rcl","removeclanlist", &g_clan.integer, &Henk_AdminRemove},
	{"!rcl","removeclan", &g_clan.integer, &Boe_Remove_Clan_Member},
	{"!3rd","3rd", &g_3rd.integer, &Boe_Third},
	{"!third","third", &g_3rd.integer, &Boe_Third},
	{"!cm","compmode", &g_cm.integer, &Boe_CompMode},
	{"!bl","banlist", &g_ban.integer, &Henk_BanList},
	{"!ba","ban", &g_ban.integer, &Boe_Ban_f},
	{"!br","broadcast", &g_broadcast.integer, &Boe_Broadcast},
	{"!sbl","subnetbanlist", &g_subnetban.integer, &Henk_SubnetBanList},
	{"!et","eventeams", &g_eventeams.integer, &Henk_EvenTeams},
	{"!cva","clanvsall", &g_clanvsall.integer, &Henk_CVA},
	{"!sw","swapteams", &g_swapteams.integer, &Henk_SwapTeams},
	{"!l","lock", &g_lock.integer, &Henk_Lock},
	{"!ul","unlock", &g_lock.integer, &Henk_Unlock},
	{"!fl","flash", &g_flash.integer, &Henk_Flash},
	{"!g","gametype", &g_mapswitch.integer, &Henk_Gametype},
	{"!gt","gametype", &g_mapswitch.integer, &Henk_Gametype},
	{"!pv","passvote", &g_forcevote.integer, &Boe_passVote},
	{"!cv","cancelvote", &g_forcevote.integer, &Boe_cancelVote},
	{"!upa","unpause", &g_pause.integer, &Henk_Unpause},
	{"!up","unpause", &g_pause.integer, &Henk_Unpause},
	{"!pa","pause", &g_pause.integer, &Henk_Pause},
	{"!uba","unban", &g_ban.integer, &Henk_Unban},
	{"!sbu","subnetunban", &g_subnetban.integer, &Henk_SubnetUnban},
	{"!ub","unban", &g_ban.integer, &Henk_Unban},
	{"!su","subnetunban", &g_subnetban.integer, &Henk_SubnetUnban},
	{"!sb","subnetban", &g_subnetban.integer, &Boe_subnetBan},
	{"!mc","mapcycle", &g_mapswitch.integer, &Boe_Mapcycle},
	{"!adl","adminlist", &g_adminlist.integer, &Henk_Admlist},
	{"!al","adminlist", &g_adminlist.integer, &Henk_Admlist},
	{"!cl","clanlist", &g_clan.integer, &Henk_Admlist},
	{"!r","respawn", &g_respawn.integer, &Boe_Respawn},
	{"!b","burn", &g_burn.integer, &Boe_Burn},
	{"!u","uppercut", &g_uppercut.integer, &Boe_Uppercut},
	{"!p","pop", &g_pop.integer, &Boe_pop},
	{"!k","kick", &g_kick.integer, &Boe_Kick},
	{"!m","mute", &g_mute.integer, &Boe_XMute},
	{"!s","strip", &g_strip.integer, &Boe_Strip},
	{"!ff","friendlyfire", &g_ff.integer, &Boe_friendlyFire},
	//{"!box","box", &g_strip.integer, &Henk_Box} // Boe!Man 4/8/11: Seems like a useless command?
};

static int AdminCommandsSize = sizeof( AdminCommands ) / sizeof( AdminCommands[0] );
// End

int AcceptBotCommand(char *cmd, gentity_t *pl);

void trap_SendServerCommand2( int clientNum, const char *text );
void trap_SendServerCommand( int clientNum, const char *text ) {
	
        if( strlen( text ) > 2044 ) { // Henk 15/09/10 -> Scores reach 1022 length easily when there are more then 32 players.
			char a[100]; 
			trap_SendServerCommand2(-1, "^3[Debug] ^7Report this at 1fx.uk.to");
			Boe_crashLog(va("overflow: name:[%s]",g_entities[clientNum].client->pers.netname)); // save this first incase of crash while copying memory
			Q_strncpyz (a, text, 100);
			Boe_crashLog(va("Data:[%s]", a)); // extra information
			trap_SendServerCommand2(clientNum, a);
            return;
        }
		trap_SendServerCommand2(clientNum, text);
} 

void EvenTeams (gentity_t *adm, qboolean aet)
{
	int		counts[TEAM_NUM_TEAMS];
	int		diff = 0;
	int		highTeam, i, j, lastConnectedTime;
	gentity_t *lastConnected, *ent;
	clientSession_t	*sess;
	qboolean canBeMoved = qfalse;

	if(current_gametype.value == GT_HS){
		EvenTeams_HS(adm, aet);
		return;
	}

	if(level.intermissiontime)
		return;

	if(!level.gametypeData->teams){
		if(adm && adm->client)
			trap_SendServerCommand( adm - g_entities, va("print \"^3[Info] ^7Not playing a team game.\n\"") );
		else if (aet == qfalse)
			Com_Printf("Not playing a team game.\n");
		return;
	}

	if(level.blueLocked || level.redLocked){
		if(adm && adm->client)
			trap_SendServerCommand( adm - g_entities, va("print \"^3[Info] ^7Teams are locked.\n\"") );
		else if (aet == qfalse)
			Com_Printf("Teams are locked.\n");
		return;

	}

	counts[TEAM_BLUE] = TeamCount(-1, TEAM_BLUE, NULL );
	counts[TEAM_RED] = TeamCount(-1, TEAM_RED, NULL );

	if(counts[TEAM_BLUE] > counts[TEAM_RED]){
		highTeam = TEAM_BLUE;
		diff = (counts[TEAM_BLUE] - counts[TEAM_RED]);
	}
	else if(counts[TEAM_BLUE] < counts[TEAM_RED]){
		highTeam = TEAM_RED;
		diff = (counts[TEAM_RED] - counts[TEAM_BLUE]);
	}
	else {
		if(adm && adm->client)
			trap_SendServerCommand( adm - g_entities, va("print \"^3[Info] ^7Teams are as even as possible.\n\"") );
		else if (aet == qfalse)
			Com_Printf("Teams are as even as possible.\n");
		return;
	}

	if(diff < 2){
		if(adm && adm->client)
			trap_SendServerCommand( adm - g_entities, va("print \"^3[Info] ^7Teams are as even as possible.\n\"") );
		else if (aet == qfalse)
			Com_Printf("Teams are as even as possible.\n");
		return;
	}
	diff /= 2;
	for(i = 0; i < diff; i++){
		lastConnectedTime = 0;
		for ( j = 0; j < level.numConnectedClients ; j++ )	{
			ent = &g_entities[level.sortedClients[j]];
			sess = &ent->client->sess;


			if ( ent->client->pers.connected != CON_CONNECTED 
				&& ent->client->pers.connected != CON_CONNECTING )
				continue;
			if(sess->team != TEAM_RED && sess->team != TEAM_BLUE)
				continue;
			if(sess->team != highTeam)
				continue;
			//if(sess->admin)
			//	continue;
			if(ent->s.gametypeitems)
				continue;

			if(ent->client->pers.enterTime > lastConnectedTime)	{
				lastConnectedTime = ent->client->pers.enterTime;
				lastConnected = ent;
			}
			canBeMoved = qtrue;
		} 

		if(!canBeMoved || !highTeam){
			if(adm != NULL)	trap_SendServerCommand( adm->s.number, va("print \"^3[Info] ^7Teams cannot be evened [all admin or item holders].\n\"") );
			else if (aet == qfalse) Com_Printf("Teams cannot be evened [all admin or item holders]\n");
			return;
		}
		lastConnected->client->ps.stats[STAT_WEAPONS] = 0;
		TossClientItems( lastConnected );
		G_StartGhosting( lastConnected );

		if(highTeam == TEAM_RED) lastConnected->client->sess.team = TEAM_BLUE;
		else lastConnected->client->sess.team = TEAM_RED;

		if (lastConnected->r.svFlags & SVF_BOT)	{
			char	userinfo[MAX_INFO_STRING];
			trap_GetUserinfo( lastConnected->s.number, userinfo, sizeof( userinfo ) );
			Info_SetValueForKey( userinfo, "team", lastConnected->client->sess.team == TEAM_RED?"red":"blue");
			trap_SetUserinfo( lastConnected->s.number, userinfo );
		}

		lastConnected->client->pers.identity = NULL;
		ClientUserinfoChanged( lastConnected->s.number );		
		CalculateRanks();

		G_StopFollowing( lastConnected );
		G_StopGhosting( lastConnected );
		trap_UnlinkEntity ( lastConnected );
		ClientSpawn( lastConnected);
	}


	// Boe!Man 3/31/10: We tell 'em what happened.
	Boe_GlobalSound (G_SoundIndex("sound/misc/events/tut_lift02.mp3"));
	
	if(adm && adm->client) {
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sE%sv%se%sn%si%sng teams!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Eventeams by %s.\n\"", adm->client->pers.netname));
		Boe_adminLog ("Eventeams", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), "none");
	}else if (aet == qfalse){
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sE%sv%se%sn%si%sng teams!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7Eventeams.\n\""));
		Boe_adminLog ("Eventeams", va("RCON"), "none");
	}else{
		trap_SendServerCommand(-1, va("print\"^3[Auto Action] ^7Eventeams.\n\""));
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
	//Ryan
	//char		string[1400];
	char		string[2048];
	//Ryan
	int			stringlength;
	int			i, j;
	gclient_t	*cl;
	int			numSorted;

	//Ryan may 15 2004
	//dont let the scores update duing the awards
	if(level.awardTime)
	{
		return;
	}
	//Ryan

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
		}
	
		//Ryan may 12 2004
		//Add some info for client-side users
		//RxCxW - 1.20.2005 - NOT compatible with 0.5. #Version
		if(ent->client->sess.rpmClient > 0.5)
		{
			Com_sprintf (entry, sizeof(entry),
				" %i %i %i %i %i %i %i %i %i %.2f %i",
				level.sortedClients[i],
				cl->sess.score,
				cl->sess.kills,
				cl->sess.deaths,
				ping,
				(level.time - cl->pers.enterTime)/60000,
				(cl->sess.ghost || cl->ps.pm_type == PM_DEAD) ? qtrue : qfalse,
				g_entities[level.sortedClients[i]].s.gametypeitems,
				g_teamkillDamageMax.integer ? 100 * cl->sess.teamkillDamage / g_teamkillDamageMax.integer : 0,
				cl->pers.statinfo.accuracy,
				cl->pers.statinfo.headShotKills
				//cl->pers.statinfo.damageDone
				);
		}else if(ent->client->sess.rpmClient == 1.1){
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
		}
		else
		{
			Com_sprintf (entry, sizeof(entry),
			" %i %i %i %i %i %i %i %i %i",
			level.sortedClients[i],
			cl->sess.score,
			cl->sess.kills,
			cl->sess.deaths,
			ping,
			(level.time - cl->pers.enterTime)/60000,
			(cl->sess.ghost ||cl->ps.pm_type == PM_DEAD) ? qtrue : qfalse, 
			g_entities[level.sortedClients[i]].s.gametypeitems,
			g_teamkillDamageMax.integer ? 100 * cl->sess.teamkillDamage / g_teamkillDamageMax.integer : 0
			);
		}

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

Dupe fix by henk
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
	if(current_gametype.value == GT_HS){
		if ( level.time > level.gametypeRoundTime ) // Henk  07/03/11 -> Don't let ppl drop stuff when the round has ended.
		{
			return;
		}
		if(!ent->client->ps.stats[STAT_WEAPONS] & ( 1 << atoi(ConcatArgs( 1 )) ))
		{
			//trap_SendServerCommand(ent->s.number, va("print\"^3[Info] ^7You don't have any item to drop\n\""));
			ent->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_KNIFE );
			ent->client->ps.ammo[weaponData[WP_KNIFE].attack[ATTACK_NORMAL].ammoIndex]=0;
			ent->client->ps.clip[ATTACK_NORMAL][WP_KNIFE]=weaponData[WP_KNIFE].attack[ATTACK_NORMAL].clipSize;
			ent->client->ps.firemode[WP_KNIFE] = BG_FindFireMode ( WP_KNIFE, ATTACK_NORMAL, WP_FIREMODE_AUTO );
			ent->client->ps.weapon = WP_KNIFE;
			ent->client->ps.weaponstate = WEAPON_READY;
			return;
		}
	}
	if(level.time < ent->client->sess.lastpickup && current_gametype.value == GT_HS){
		return;
	}else{

		// Drop the weapon the client wanted to drop
		dropped = G_DropWeapon ( ent, atoi(ConcatArgs( 1 )), 1000 );
		if ( !dropped )
		{
			//trap_SendServerCommand(ent->s.number, va("print\"^3[Info] ^7You don't have any item to drop\n\""));
			return;
		}
	}
}


void Cmd_DropItem_f ( gentity_t* ent )
{
	// spectators cant drop anything since they dont have anything
	if ( ent->client->sess.team == TEAM_SPECTATOR )
		return;

	// Ghosts and followers cant drop stuff
	if ( ent->client->ps.pm_flags & (PMF_GHOST|PMF_FOLLOW) )
		return;

	// Nothing to drop
	if ( !ent->client->ps.stats[STAT_GAMETYPE_ITEMS] ){
		trap_SendServerCommand(ent->s.number, va("print\"^3[Info] ^7You don't have any gametype item to drop\n\""));
		return;
	}

	G_DropGametypeItems ( ent, 3000 );
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
				ent->client->ps.clip[a][i] = 250; //weaponData[i].attack[a].clipSize;
				ent->client->ps.ammo[weaponData[i].attack[a].ammoIndex] = 250;//ammoData[weaponData[i].attack[a].ammoIndex].max;
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

	if (current_gametype.value == GT_HZ){
		if(ent->client->sess.killtime > level.time){ // Boe!Man 7/15/11: He's not allowed to kill himself. Inform him and return.
			if(ent->client->sess.killtime - level.time > 1000){ // Boe!man 7/15/11: The counter should never show 0.. Just kill if it's actually under 1.
				trap_SendServerCommand ( ent->client - &level.clients[0], va("print\"^3[H&Z] ^7You cannot kill yourself for another %i second(s).\n\"", (ent->client->sess.killtime - level.time) / 1000 ));
				return;
			}
		}
	}

	if(/*current_gametype.value == GT_HS && */level.crossTheBridge && ent->client->sess.team == TEAM_BLUE)
	{
		trap_SendServerCommand ( ent->client - &level.clients[0], "print\"^3[Cross The Bridge] ^7Why would you want to kill yourself?\n\"" );
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
	char		message[512];
	char		admin[46];
	char		clan[46];
	qboolean	noAdmin = qfalse;

	if(current_gametype.value == GT_HS){
		switch ( client->sess.team )
		{
			case TEAM_RED:
				strcpy(message, va("%s" S_COLOR_WHITE "\njoined the %s\n\"", client->pers.netname, server_hiderteamprefix.string));
				trap_SendServerCommand( -1, va("print \"^3[Info] ^7%s ^7joined the hiders.\n\"", client->pers.cleanName));
				break;

			case TEAM_BLUE:
				strcpy(message, va("%s" S_COLOR_WHITE "\njoined the %s\n\"", client->pers.netname, server_seekerteamprefix.string));
				trap_SendServerCommand( -1, va("print \"^3[Info] ^7%s ^7joined the seekers.\n\"", client->pers.cleanName));
				break;

			case TEAM_SPECTATOR:
				if ( oldTeam != TEAM_SPECTATOR )
				{
					strcpy(message, va("%s" S_COLOR_WHITE "\njoined the %s\n\"", client->pers.netname, server_specteamprefix.string));
					trap_SendServerCommand( -1, va("print \"^3[Info] ^7%s ^7joined the spectators.\n\"", client->pers.cleanName));
				}
				else
				{
					return;
				}
				break;
		}
	}else if(current_gametype.value == GT_HZ){
		switch ( client->sess.team )
		{
			case TEAM_RED:
				strcpy(message, va("%s" S_COLOR_WHITE "\njoined the %s\n\"", client->pers.netname, server_humanteamprefix.string));
				trap_SendServerCommand( -1, va("print \"^3[Info] ^7%s ^7joined the humans.\n\"", client->pers.cleanName));
				break;

			case TEAM_BLUE:
				strcpy(message, va("%s" S_COLOR_WHITE "\njoined the %s\n\"", client->pers.netname, server_zombieteamprefix.string));
				trap_SendServerCommand( -1, va("print \"^3[Info] ^7%s ^7joined the zombies.\n\"", client->pers.cleanName));
				break;

			case TEAM_SPECTATOR:
				if ( oldTeam != TEAM_SPECTATOR )
				{
					strcpy(message, va("%s" S_COLOR_WHITE "\njoined the %s\n\"", client->pers.netname, server_specteamprefix.string));
					trap_SendServerCommand( -1, va("print \"^3[Info] ^7%s ^7joined the spectators.\n\"", client->pers.cleanName));
				}
				else
				{
					return;
				}
				break;
		}
	}else{
		switch ( client->sess.team )
		{
			case TEAM_RED:
				strcpy(message, va("%s\n^7joined the %s ^7team", client->pers.netname, server_redteamprefix.string));
				trap_SendServerCommand( -1, va("print \"^3[Info] ^7%s ^7joined the red team.\n\"", client->pers.cleanName));
				break;

			case TEAM_BLUE:
				strcpy(message, va("%s" S_COLOR_WHITE "\njoined the %s ^7team\n\"", client->pers.netname, server_blueteamprefix.string));
				trap_SendServerCommand( -1, va("print \"^3[Info] ^7%s ^7joined the blue team.\n\"", client->pers.cleanName));
				break;
			
			case TEAM_SPECTATOR:
				if ( oldTeam != TEAM_SPECTATOR )
				{
					strcpy(message, va("%s" S_COLOR_WHITE "\njoined the %s\n\"", client->pers.netname, server_specteamprefix.string));
					trap_SendServerCommand( -1, va("print \"^3[Info] ^7%s ^7joined the spectators.\n\"", client->pers.cleanName));
				}else
				{
					return;
				}
				break;

			case TEAM_FREE:
				strcpy(message, va("%s" S_COLOR_WHITE "\njoined the %sb%sa%st%st%sl%se\n\"", client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
				trap_SendServerCommand( -1, va("print \"^3[Info] ^7%s ^7joined the battle.\n\"", client->pers.cleanName));
				break;
		}
	}

	// Boe!Man 6/2/11: Only broadcast the message if the player specifies to by enabling events during compmode. Always broadcast it when compmode's disabled.
	if(cm_enabled.integer == 0 || cm_devents.integer == 0 || cm_enabled.integer == 1 && cm_devents.integer == 1){
		// Boe!Man 3/16/11: Append Clan and/or Admin status if needed.
		if(client->sess.firstTime == qtrue){
			if(client->sess.clanMember == qtrue){
				strncpy(clan, server_ctprefix.string, 45);
			}else{
				strncpy(clan, "", 2);
			}
			if(client->sess.admin > 1){
				if(client->sess.admin == 2){
					strncpy(admin, server_badminprefix.string, 45);
				}else if(client->sess.admin == 3){
					strncpy(admin, server_adminprefix.string, 45);
				}else if(client->sess.admin == 4){
					strncpy(admin, server_sadminprefix.string, 45);
				}
			}else{
				strncpy(admin, "", 2);
				noAdmin = qtrue; // Boe!Man 3/16/11: It doesn't matter if clan is empty, the lining won't fuck up. With an empty admin entry however, it will.. Hence this check.
			}

			// Boe!Man 3/16/11: Broadcast the teamchange.
			if(noAdmin == qfalse){
				trap_SendServerCommand( -1, va("cp \"@%s\n%s %s\n\"", clan, admin, message));
			}else{
				trap_SendServerCommand( -1, va("cp \"@%s\n%s\n\"", clan, message));
			}
		}else{ // Boe!Man 3/18/11: Else we just display the message itself.
			trap_SendServerCommand( -1, va("cp \"@%s\n\"", message));
		}
	}
	return;

}

/*
=================
SetTeam
=================
*/
void SetTeam( gentity_t *ent, char *s, const char* identity, qboolean forced ) 
{
	int					team;
	int					oldTeam;
	gclient_t			*client;
	int					clientNum;
	spectatorState_t	specState;
	int					specClient;
	qboolean			ghost;
   	qboolean			noOutfittingChange = qfalse;
	int					seekers;
	int					maxhiders;
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
		
		ent->client->sess.lastIdentityChange = level.time;

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
			if(current_gametype.value == GT_HS){
				// Henk 19/01/10 -> Join hiders
				team = TEAM_RED;
			}
		}

		if(!forced && current_gametype.value == GT_HZ){
			if(TeamCount1(TEAM_BLUE) < 3 && team == TEAM_BLUE){
			trap_SendServerCommand ( client - &level.clients[0], "print\"^3[H&Z] ^7Zombie team is locked.\n\"" );
			return;
			}
		}

		if ( g_teamForceBalance.integer && forced == qfalse /*|| current_gametype.value == GT_HS && forced == qfalse */) // Boe!Man 8/25/11: Don't force balance in H&S, the CVAR is defaulted to 1. In some cases users don't want it. 
		{
			int		counts[TEAM_NUM_TEAMS];

			if(current_gametype.value == GT_HS){
				counts[TEAM_BLUE] = TeamCount1(TEAM_BLUE);
				counts[TEAM_RED] = TeamCount1(TEAM_RED);
				// Henk 19/01/10 -> Team balance hiders/seekers
				// Boe!Man 8/12/11: Modified the code (lowered) so there's ALWAYS room for one extra seeker (else it will result in both teams being locked in specific team layouts).
				if(counts[TEAM_RED] >= 4 && counts[TEAM_RED] <= 7){
					seekers = 2;
				}else if(counts[TEAM_RED] >= 8 && counts[TEAM_RED] <= 12){
					seekers = 3;
				}else if(counts[TEAM_RED] >= 13 && counts[TEAM_RED] <= 17){
					seekers = 4;
				}else if(counts[TEAM_RED] >= 18){
					seekers = 5;
				}
				else{
					seekers = 1;
				}

				if(counts[TEAM_BLUE] == 2){
					maxhiders = 8;
				}else if(counts[TEAM_BLUE] == 3){
					maxhiders = 13;
				}else if(counts[TEAM_BLUE] == 4){
					maxhiders = 18;
				}else if(counts[TEAM_BLUE] == 5){
					maxhiders = 24;
				}else{
					maxhiders = 6; // Henkie 24/02/10 -> Was 4
				}
				if ( team == TEAM_BLUE && counts[TEAM_BLUE] >= seekers)	{
					trap_SendServerCommand ( client - &level.clients[0], "print\"^3[H&S] ^7Seekers have too many players.\n\"" );
					// ignore the request
					return;
				}else if(team == TEAM_RED && counts[TEAM_RED] >= maxhiders){
					trap_SendServerCommand ( client - &level.clients[0], "print\"^3[H&S] ^7Hiders have too many players.\n\"" );
					// ignore the request
					return;
				}
			}else{
				counts[TEAM_BLUE] = TeamCount( ent->client->ps.clientNum, TEAM_BLUE, NULL );
				counts[TEAM_RED] = TeamCount( ent->client->ps.clientNum, TEAM_RED, NULL );

				// We allow a spread of two
				if ( team == TEAM_RED && counts[TEAM_RED] - counts[TEAM_BLUE] > 1 ) 
				{
					trap_SendServerCommand( ent->s.number, 
											"print \"^3[Info] ^7Red team has too many players.\n\"" );

					// ignore the request
					return; 
				}
				if ( team == TEAM_BLUE && counts[TEAM_BLUE] - counts[TEAM_RED] > 1 ) 
				{
					trap_SendServerCommand( ent->s.number, 
											"print \"^3[Info] ^7Blue team has too many players.\n\"" );

					// ignore the request
					return; 
				}
			}

			// It's ok, the team we are switching to has less or same number of players
		}
	} 
	else 
	{
		// force them to spectators if there aren't any spots free
		team = TEAM_FREE;
	}

	// Henk 26/05/11 -> Add check to prevent switch team respawn
	// Henk 31/05/11 -> Removed due alot of possible faults
	/*if(level.time <= level.gametypeStartTime+20000){ // only check within the delay time
		if(strstr(level.deadClients, ent->client->pers.ip) && team == TEAM_RED || team == TEAM_BLUE){
			trap_SendServerCommand (ent->s.number, "print\"^3[Info] ^7You just died, please wait before switching teams.\n\"" );
			return;
		}
	}*/
	// End

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

	if (level.gametypeData->teams)
	{
		///RxCxW - 03.05.05 - 07:19am
		///if ((team == TEAM_RED) && level.redLocked)
		if ((team == TEAM_RED) && level.redLocked && client->sess.admin < 2) {
			// Boe!Man 2/15/11: H&S messages are different as they use another team prefix.
			if(current_gametype.value == GT_HS){
				trap_SendServerCommand(clientNum, va("cp \"@%s ^7are %sl%so%sc%sk%se%sd!\n\"", server_hiderteamprefix.string, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string ));
				trap_SendServerCommand(clientNum, va("print \"^3[Info] ^7Hiders are locked.\n\"") );
			}else{
				// Boe!Man 12/14/10
				trap_SendServerCommand(clientNum, va("cp \"@%s ^7team is %sl%so%sc%sk%se%sd!\n\"", server_redteamprefix.string, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string ));
				trap_SendServerCommand(clientNum, va("print \"^3[Info] ^7Red team is locked.\n\"") );
			}
			return;
		}
		///else if ((team == TEAM_BLUE) && level.blueLocked)
		else if ((team == TEAM_BLUE) && level.blueLocked && client->sess.admin < 2) {
			// Boe!Man 2/15/11: H&S messages are different as they use another team prefix.
			if(current_gametype.value == GT_HS){
				trap_SendServerCommand(clientNum, va("cp \"@%s ^7are %sl%so%sc%sk%se%sd!\n\"", server_seekerteamprefix.string, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string ));
				trap_SendServerCommand(clientNum, va("print \"^3[Info] ^7Seekers are locked.\n\"") );
			}else{
				// Boe!Man 12/14/10
				trap_SendServerCommand(clientNum, va("cp \"@%s ^7team is %sl%so%sc%sk%se%sd!\n\"", server_blueteamprefix.string, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string ));
				trap_SendServerCommand(clientNum, va("print \"^3[Info] ^7Blue team is locked.\n\"") );
			}
			return;
		}
	}

	if(oldTeam == TEAM_SPECTATOR)
	{
		//Ryan march 28 2004 3:24pm
		//keep track of how much time the player spent spectating
		//we'll subtract this later from the players time for awards
		client->sess.totalSpectatorTime += level.time - client->sess.spectatorTime;
		//Ryan

		//clear the invites if we are joining game play
		if(team == TEAM_RED || team == TEAM_BLUE)
		{
			client->sess.invitedByRed = 0;
			client->sess.invitedByBlue = 0;
		}
	}

   	noOutfittingChange = ent->client->noOutfittingChange;

	// he starts at 'base'
	client->pers.teamState.state = TEAM_BEGIN;
	
	if ( oldTeam != TEAM_SPECTATOR ) 
	{

		if(team == TEAM_RED || team == TEAM_BLUE)
		{
			client->sess.invitedByRed = 0;
			client->sess.invitedByBlue = 0;
		}

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

	// Kill any child entities of this client to protect against grenade team changers
	G_FreeEnitityChildren ( ent ); // Henk 14/01/11 -> Fix for specnade

	// Always spawn into a ctf game using a respawn timer.
	if(!level.pause)
	{
		if ( team != TEAM_SPECTATOR && level.gametypeData->respawnType == RT_INTERVAL )
		{
			G_SetRespawnTimer ( ent );
			ghost = qtrue;
		}
	}
	//Ryan
	//Ryan june 15 2003
	if(!level.pause && !client->sess.firstTime)
	{
		if(!forced)
		BroadcastTeamChange( client, oldTeam );
	}
	//Ryan
	//BroadcastTeamChange( client, oldTeam );

	// See if we should spawn as a ghost
	if ( team != TEAM_SPECTATOR && level.gametypeData->respawnType == RT_NONE )
	{
		// If there are ghosts already then spawn as a ghost because
		// the game is already in progress.
		if ( (level.gametypeJoinTime && (level.time - level.gametypeJoinTime) > 20000) || noOutfittingChange || client->sess.noTeamChange )
		{
			ghost = qtrue;
		}

		if(current_gametype.value == GT_HZ){
			if(level.gametypeJoinTime && (level.time - level.gametypeJoinTime) > 10000){
				ghost = qtrue;
			}
		}

		// Boe!Man 9/4/11: Don't allow people to spawn that are reconnecting and are being put in the team.
		if(current_gametype.value == GT_HS && level.cagefight && !client->sess.cageFighter){
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

			// Henk 02/02/10 -> Set score to 0 when switching team.
			// Boe!Man 2/8/11: Forcing this to be H&S only.
			if (current_gametype.value == GT_HS){
				ent->client->sess.score = 0;
				ent->client->sess.kills = 0;
				ent->client->sess.deaths = 0;
				ent->client->sess.timeOfDeath = 0; // Boe!Man 8/29/11: Also reset this when switching team (so seekers that won won't get RPG for example).
			}
			
			// Boe!Man 6/3/12: Reset noroof data when setting team.
			ent->client->sess.isOnRoof = qfalse;
			ent->client->sess.isOnRoofTime = 0;

			CalculateRanks();

			return;
		}
	}	

	// get and distribute relevent paramters
	client->pers.identity = NULL;
	ClientUserinfoChanged( clientNum );

	// Henk 02/02/10 -> Set score to 0 when switching team.
	if (current_gametype.value == GT_HS){
	ent->client->sess.score = 0;
	ent->client->sess.kills = 0;
	ent->client->sess.deaths = 0;
	ent->client->sess.timeOfDeath = 0; // Boe!Man 8/29/11: Also reset this when switching team (so seekers that won won't get RPG for example).
	}
	
	// Boe!Man 6/3/12: Reset noroof data when setting team.
	ent->client->sess.isOnRoof = qfalse;
	ent->client->sess.isOnRoofTime = 0;

	CalculateRanks();

	// Begin the clients new life on the their new team
	ClientBegin( clientNum, qfalse );
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
		trap_SendServerCommand( ent-g_entities, "print \"May not switch teams more than once per 3 seconds.\n\"" );
		return;
	}

	trap_Argv( 1, team, sizeof( team ) );
	trap_Argv( 2, identity, sizeof( identity ) );

	SetTeam( ent, team, identity[0]?identity:NULL, qfalse );

	// Remember the team switch time so they cant do it again really quick
	ent->client->switchTeamTime = level.time + 3000;
}


/*
=================
Cmd_Follow_f
=================
*/
void Cmd_Follow_f( gentity_t *ent ) 
{
	int		i, z;
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

	if(henk_ischar(arg[0])){
		for(z=0;z<=level.numConnectedClients;z++){
			//trap_SendServerCommand(-1, va("print\"^3[Debug] ^7%s comparing with %s.\n\"", g_entities[level.sortedClients[i]].client->pers.cleanName,numb));
			if(strstr(Q_strlwr(g_entities[level.sortedClients[z]].client->pers.cleanName), Q_strlwr(arg))){
				i = level.sortedClients[z];
				break;
			}
			i = -1;
		}
	}else{
		i = ClientNumberFromString( ent, arg );
	}
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

	if(ent->client->sess.team == TEAM_SPECTATOR && level.specsLocked)
	{
		if(level.clients[ i ].sess.team == TEAM_RED && !ent->client->sess.invitedByRed)
		{
			return;
		}
		if(level.clients[ i ].sess.team == TEAM_BLUE && !ent->client->sess.invitedByBlue)
		{
			return;
		}
	}

// Dissallow following of the enemy if the cvar is set
	//if ( level.gametypeData->teams && !g_followEnemy.integer && ent->client->sess.team != TEAM_SPECTATOR )
	if ( level.gametypeData->teams && (!g_followEnemy.integer || g_compMode.integer) &&
		ent->client->sess.team != TEAM_SPECTATOR && !ent->client->sess.adminspec)
	{
		// Are they on the same team?
		if ( level.clients[ i ].sess.team != ent->client->sess.team )
		{
			return;
		}
	}

	// first set them to spectator as long as they arent a ghost
	if ( !ent->client->sess.ghost && ent->client->sess.team != TEAM_SPECTATOR ) 
	{
		SetTeam( ent, "spectator", NULL, qfalse );
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
		SetTeam( ent, "spectator", NULL, qfalse );
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
		ent->client->sess.spectating = clientnum;
		//Ryan
		if(g_compMode.integer && !ent->client->sess.adminspec && !ent->client->sess.referee)
		{
			//No enemy following in compmode
			if(ent->client->sess.team != TEAM_SPECTATOR && level.clients[ clientnum ].sess.team != ent->client->sess.team)
			{
				continue;
			}

			if(ent->client->sess.team == TEAM_SPECTATOR)
			{
				if(level.clients[ clientnum ].sess.team == TEAM_RED)
				{
					if(level.specsLocked && !ent->client->sess.invitedByRed)
					{
						continue;
					}
				}
				else if(level.clients[ clientnum ].sess.team == TEAM_BLUE)
				{
					if(level.specsLocked && !ent->client->sess.invitedByBlue)
					{
						continue;
					}
				}
			}
		}
		//Ryan

		// Dissallow following of the enemy if the cvar is set
		//Ryan june 7 2003
			//if ( level.gametypeData->teams && !g_followEnemy.integer && ent->client->sess.team != TEAM_SPECTATOR )
		if ( level.gametypeData->teams && !g_followEnemy.integer && ent->client->sess.team != TEAM_SPECTATOR && !ent->client->sess.adminspec)
		//Ryan
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
	//Ryan
	//if ( deadclient != -1 && g_forceFollow.integer )
	if ( deadclient != -1 && (g_forceFollow.integer || g_compMode.integer))
	//Ryan
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
	qboolean	 beep  = qfalse;
	char	type[128];
	char	admin[128];
	char 	star[128];
	int i;

	//G_LogPrintf("Starting with chat\n");
	if (!other) 
	{
		return;
	}

	if (!other->inuse) 
	{
		return;
	}else if (!other->client) 
	{
		return;
	}else if ( other->client->pers.connected != CON_CONNECTED ) 
	{
		return;
	}else if ( mode == SAY_TEAM  && !OnSameTeam(ent, other) ) 
	{
		return;
	}

	// Boe!Man 1/17/10: We do not wish to send the chat when it's Admin/Clan only..
	if ( mode == ADM_CHAT  && !other->client->sess.admin )
		return;
	else if (mode == REF_CHAT && !other->client->sess.referee && !other->client->sess.admin)
		return;
	else if ( mode == SADM_CHAT  && other->client->sess.admin != 4 )
		return;
	else if (mode == CLAN_CHAT && !other->client->sess.clanMember )
		return;
	else if (mode == CLAN_TALK && !other->client->sess.clanMember )
		return;
	// Boe!Man 9/26/10: Hey Admin! chat needs to be visible for admins AND the one saying it.
	else if (mode == CADM_CHAT && ent->s.number != other->s.number && other->client->sess.admin < 2)
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

	for(i=0;i<ent->client->sess.IgnoredClientCount;i++){
		if(ent->client->sess.IgnoredClients[i] == other->s.number){
			return;
		}
	}

	strcpy(admin, ""); // Boe!Man 1/18/10: Clean the Admin data.
	
	// Boe!Man 1/7/10: Team prefixes.
	if ( ghost )
	{
		strcpy(type, "^7[^Cg^7] ");
	}
	else if ( spec )
	{
		strcpy(type, "^7[^Cs^7] ");
	}
	// Boe!Man 4/5/10: We delete the team prefixes.
	// Boe!Man 2/8/11: And we re-add them for Hide&Seek lol.
	
	else if ( ent->client->sess.team == TEAM_RED && current_gametype.value == GT_HS )
	{
		strcpy(type, "^7[^1h^7] ");
	}
	else if ( ent->client->sess.team == TEAM_BLUE && current_gametype.value == GT_HS )
	{
		strcpy(type, "^7[^ys^7] ");
	}
	else if ( ent->client->sess.team == TEAM_RED && current_gametype.value == GT_HZ )
	{
		strcpy(type, "^7[^1h^7] ");
	}
	else if ( ent->client->sess.team == TEAM_BLUE && current_gametype.value == GT_HZ )
	{
		strcpy(type, "^7[^yz^7] ");
	}
	
	// Boe!Man 4/6/10: And replace the type with something, well nothing.
	else{
		strcpy(type, "");
	}

	// Boe!Man 1/17/10: Admin Talk/Chat.
	if(mode >= ADM_TALK && mode <= CADM_CHAT){ // Henk 03/05/10 -> Optimized so cpu can calculate faster
	strcpy(star, server_starprefix.string);
	strcpy(admin, "");
	}else{
		strcpy(star, "");
		// Boe!Man 1/6/10: Admin prefixes. - Update 1/18/10: New CVARs for prefixes if the hoster wishes to change the values.
		if(ent->client->sess.admin == 2){
			strcpy(admin, server_badminprefix.string);
		}else if(ent->client->sess.admin == 3){
			strcpy(admin, server_adminprefix.string);
		}else if(ent->client->sess.admin == 4){
			strcpy(admin, server_sadminprefix.string);
		}else{
			strcpy(admin, "");
		}
		
		// Boe!Man 11/5/12: Fix for space in chat when *adminprefix CVAR was empty but the client was still an Admin.
		if(strlen(admin) > 0){
			Q_strcat(admin, sizeof(admin), " ");
		}
	}
	
	// Boe!Man 1/17/10: Different kinds of Talking 'Modes'.


	// Boe!Man 4/20/11: Should the beep be enabled?
	if (cm_devents.integer == 1 && cm_enabled.integer == 1 || cm_devents.integer == 0 && cm_enabled.integer != 1){
		beep = qtrue;
	}

	switch(mode)
	{
/*	case REF_CHAT:
		type = "^7Ref chat";
		Boe_ClientSound(other, G_SoundIndex("sound/misc/menus/invalid.wav"));
		break;
	case REF_TALK:
		type =  "^7Ref talk";
		Boe_ClientSound(other, G_SoundIndex("sound/misc/menus/invalid.wav"));
		break;
		*/
	case ADM_CHAT:
		strcpy(type, server_acprefix.string);
		if (beep == qtrue){
			Boe_ClientSound(other, G_SoundIndex("sound/misc/c4/beep.mp3"));
		}
		break;
	case SADM_CHAT:
		strcpy(type, server_scprefix.string);
		if (beep == qtrue){
			Boe_ClientSound(other, G_SoundIndex("sound/misc/c4/beep.mp3"));
		}
		break;
	case ADM_TALK:
		if(ent->client->sess.admin == 2){
		strcpy(type, server_badminprefix.string);
		}else if(ent->client->sess.admin == 3){
		strcpy(type, server_adminprefix.string);
		}else if(ent->client->sess.admin == 4){
		strcpy(type, server_sadminprefix.string);
		}
		if (beep == qtrue){
			Boe_ClientSound(other, G_SoundIndex("sound/misc/c4/beep.mp3"));
		}
		break;
	case CADM_CHAT:
		strcpy(type, server_caprefix.string);
		if (beep == qtrue){
			Boe_ClientSound(other, G_SoundIndex("sound/misc/c4/beep.mp3"));
		}
		break;
	case CLAN_CHAT:
		strcpy(type, server_ccprefix.string);
		if (beep == qtrue){
			Boe_ClientSound(other, G_SoundIndex("sound/misc/c4/beep.mp3"));
		}
		break;
	case CLAN_TALK:
		strcpy(type, server_ctprefix.string);
		if (beep == qtrue){
			Boe_ClientSound(other, G_SoundIndex("sound/misc/c4/beep.mp3"));
		}
		break;
	default:
		break;
	}

	// Boe!Man 1/6/10 - Update 2/8/11: Finally fixing these space issues. This would be the best way for it.
	if(ent->client->sess.admin > 0 && mode >= ADM_TALK && mode <= SADM_CHAT || mode == CLAN_CHAT || mode == CADM_CHAT || mode == CLAN_TALK){ // Henk 13/02/11 -> Fixed.
		trap_SendServerCommand( other-g_entities, va("%s %d \"%s %s %s%s %s\"", // Boe!Man 1/6/10: Adding prefixes. - Update 1/17/10: Adding Admin Talk/Chat prefixes. - Update 5/8/10: Solved message problem.
								mode == SAY_TEAM ? "tchat" : "chat",
								ent->s.number,
								// Boe!Man 1/6/10: Adding the Admin prefix in front of the chat. - Update 1/17/10.
								star, type, name, message, star)); // Boe!Man 1/17/10: Adding stars.
	}else if(ent->client->sess.admin > 0){
		trap_SendServerCommand( other-g_entities, va("%s %d \"%s%s%s%s\"", // Boe!Man 1/6/10: Adding prefixes. - Update 1/17/10: Adding Admin Talk/Chat prefixes. - Update 5/8/10: Solved message problem.
								mode == SAY_TEAM ? "tchat" : "chat",
								ent->s.number,
								// Boe!Man 1/6/10: Adding the Admin prefix in front of the chat. - Update 1/17/10.
								type, admin, name, message));
	}else{
		trap_SendServerCommand( other-g_entities, va("%s %d \"%s%s%s\"", // Boe!Man 1/6/10: Adding prefixes. - Update 1/17/10: Adding Admin Talk/Chat prefixes. - Update 5/8/10: Solved message problem.
							mode == SAY_TEAM ? "tchat" : "chat",
							ent->s.number,
							// Boe!Man 1/6/10: Adding the Admin prefix in front of the chat. - Update 1/17/10.
							type, name, message));
	}
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
	char		name[128]; // Henk 15/12/2010 -> Increased from 64 -> 128; Will fix the name dissapearing.
	int i;

	// Logging stuff
	// FIX ME: Protection for overflows/fails etc..
	if((strstr(chatText, "@fp") || strstr(chatText, "@FP") || strstr(chatText, "@Fp") || strstr(chatText, "@fP"))){
		for(i=0;i<=strlen(chatText);i++){
			if(chatText[i] == '@' && chatText[i+1] != 'f'){
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Sound blocked to prevent spam.\n\"") );
				return;
			}
		}
	}
	switch ( mode )
	{
		case SAY_ALL:
			G_LogPrintf( "say: %s: %s\n", ent->client->pers.netname, chatText );
			break;

		case SAY_TEAM:
			G_LogPrintf( "sayteam: %s: %s\n", ent->client->pers.netname, chatText );
			break;
	}

	// Boe!Man 8/25/11: Also log special chat when the CVAR is enabled.
	if(g_logSpecialChat.integer){
		switch ( mode )
		{
			case ADM_TALK:
				G_LogPrintf( "admtalk: %s: %s\n", ent->client->pers.netname, chatText );
				break;

			case ADM_CHAT:
				G_LogPrintf( "admchat: %s: %s\n", ent->client->pers.netname, chatText );
				break;

			case SADM_CHAT:
				G_LogPrintf( "sadmchat: %s: %s\n", ent->client->pers.netname, chatText );
				break;

			case CLAN_TALK:
				G_LogPrintf( "clantalk: %s: %s\n", ent->client->pers.netname, chatText );
				break;
			
			case CLAN_CHAT:
				G_LogPrintf( "clanchat: %s: %s\n", ent->client->pers.netname, chatText );
				break;

			case CADM_CHAT:
				G_LogPrintf( "heyadmin: %s: %s\n", ent->client->pers.netname, chatText );
				break;
		}
	}
	
	//G_LogPrintf("Getting prefix..\n");
	// Generate the chat prefix
	G_GetChatPrefix ( ent, target, mode, name, sizeof(name) );

	//G_LogPrintf("Copying..\n");
	// Save off the chat text
	Q_strncpyz( text, chatText, sizeof(text) );

	if(ent && ent->client){ // Henk 03/07/11 -> Reset inactivity timer when chatting
		ent->client->inactivityTime = level.time + g_inactivity.integer * 1000;
		ent->client->inactivityWarning = qfalse;
	}

	if ( target && target->inuse) 
	{
		//G_LogPrintf("Too target?..\n");
		G_SayTo( ent, target, mode, name, text );
		return;
	}
	//G_LogPrintf("Printing..\n");
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
	//G_LogPrintf("Done..\n");
}

void Cmd_Say_f( gentity_t *ent, int mode, qboolean arg0 ) {
	char		*p;
	char        newp[512] = "";
	// Boe!Man 1/10/10
	int			i;
	int			a = 0;
	// Boe!Man 1/17/10
	int			anim = 0;
	float		knockback = 400.0;
	// Boe!Man 5/3/10: Temp(?) fix.
	qboolean	acmd = qfalse;
	char		test[512];
	int			ignore = -1;
	qboolean	command = qfalse;

	void	*GP2, *group;
	char txtlevel[2];
	char name[10];
	char action[512];
	char message[512];
	char broadcast[512];

	if(!ent || !ent->client)
		return;

	if ( trap_Argc () < 2 && !arg0 )
		return;
	if (arg0)
		p = ConcatArgs( 0 );
	else if(mode >= ADM_TALK && mode <= CADM_CHAT)
		p = ConcatArgs( 2 );
	else
		p = ConcatArgs( 1 );

	strcpy(test, p); // Henk 08/09/10 -> Copy p to static buffer to prevent unwanted changes by other functions
	// Boe!Man 1/24/10: Different kinds of Talk during Gameplay. -- Update 3/21/11: No need to be admin in order to use this..
	if ((strstr(p, "!at")) || (strstr(p, "!AT")) || (strstr(p, "!aT")) || (strstr(p, "!At"))) {
		if (ent->client->sess.admin){
			p = ConcatArgs(1);
			for(i=0;i<=strlen(p);i++){
				if(p[i] == '!' && (p[i+1] == 'a' || p[i+1] == 'A') && (p[i+2] == 't' || p[i+2] == 'T')){
					ignore = i;
				}
				if(ignore == -1){
				newp[a] = p[i];
				a += 1;
				}else if(i == ignore || i == ignore+1 || i == ignore+2){
				if(a != 0)
					i+=1; // Fix for spaces
				}else{
				if(a == 0 && p[i] == ' ') // Fix for spaces
					continue;
				newp[a] = p[i];
				a += 1;
				}
			}
			mode = ADM_TALK;
			acmd = qtrue;
			strcpy(p, newp);
		}else{
			p = ConcatArgs(1);
			for(i=0;i<=strlen(p);i++){
				if(p[i] == '!' && (p[i+1] == 'a' || p[i+1] == 'A') && (p[i+2] == 't' || p[i+2] == 'T')){
					ignore = i;
				}
				if(ignore == -1){
				newp[a] = p[i];
				a += 1;
				}else if(i == ignore || i == ignore+1 || i == ignore+2){
				if(a != 0)
					i+=1; // Fix for spaces
				}else{
				if(a == 0 && p[i] == ' ') // Fix for spaces
					continue;
				newp[a] = p[i];
				a += 1;
				}
			}
			mode = CADM_CHAT;
			acmd = qtrue;
			strcpy(p, newp);
		}
	}
	else if ((strstr(p, "!ac")) || (strstr(p, "!AC")) || (strstr(p, "!aC")) || (strstr(p, "!Ac"))) {
		if (/*ent->client->sess.admin && */!strstr(Q_strlwr(test), "!acl")){ // Boe!Man 3/23/11: Check if they didn't mean to add a clan member..
			p = ConcatArgs(1);
			for(i=0;i<=strlen(p);i++){
				if(p[i] == '!' && (p[i+1] == 'a' || p[i+1] == 'A') && (p[i+2] == 'c' || p[i+2] == 'C')){
					ignore = i;
				}
				if(ignore == -1){
				newp[a] = p[i];
				a += 1;
				}else if(i == ignore || i == ignore+1 || i == ignore+2){
				if(a != 0)
					i+=1; // Fix for spaces
				}else{
				if(a == 0 && p[i] == ' ') // Fix for spaces
					continue;
				newp[a] = p[i];
				a += 1;
				}
			}

			// Boe!Man 8/25/11: If a non-admin is using Admin Chat, force it to Hey Admin instead.
			if(ent->client->sess.admin){
				mode = ADM_CHAT;
			}else{
				mode = CADM_CHAT;
			}

			acmd = qtrue;
			strcpy(p, newp);
		}else if(!strstr(Q_strlwr(test), "!acl")){
			p = ConcatArgs(1);
			G_Say( ent, NULL, mode, p );
			return;
		}
	}
	else if ((strstr(p, "!sc")) || (strstr(p, "!SC")) || (strstr(p, "!sC")) || (strstr(p, "!Sc"))) {
		if (ent->client->sess.admin == 4){
			p = ConcatArgs(1);
			for(i=0;i<=strlen(p);i++){
				if(p[i] == '!' && (p[i+1] == 's' || p[i+1] == 'S') && (p[i+2] == 'c' || p[i+2] == 'C')){
					ignore = i;
				}
				if(ignore == -1){
				newp[a] = p[i];
				a += 1;
				}else if(i == ignore || i == ignore+1 || i == ignore+2){
				if(a != 0)
					i+=1; // Fix for spaces
				}else{
				if(a == 0 && p[i] == ' ') // Fix for spaces
					continue;
				newp[a] = p[i];
				a += 1;
				}
			}
			mode = SADM_CHAT;
			acmd = qtrue;
			strcpy(p, newp);
		}else{
			p = ConcatArgs(1);
			G_Say( ent, NULL, mode, p );
			return;
		}
	}
	// Boe!Man 4/17/10: Clan chat.
	else if ((strstr(p, "!cc")) || (strstr(p, "!CC")) || (strstr(p, "!Cc")) || (strstr(p, "!cC"))) {
		if (ent->client->sess.clanMember){
			p = ConcatArgs(1);
			for(i=0;i<=strlen(p);i++){
				if(p[i] == '!' && (p[i+1] == 'c' || p[i+1] == 'C') && (p[i+2] == 'c' || p[i+2] == 'C')){
					ignore = i;
				}
				if(ignore == -1){
				newp[a] = p[i];
				a += 1;
				}else if(i == ignore || i == ignore+1 || i == ignore+2){
				if(a != 0)
					i+=1; // Fix for spaces
				}else{
				if(a == 0 && p[i] == ' ') // Fix for spaces
					continue;
				newp[a] = p[i];
				a += 1;
				}
			}
			mode = CLAN_CHAT;
			acmd = qtrue;
			strcpy(p, newp);
		}else{
			p = ConcatArgs(1);
			G_Say( ent, NULL, mode, p );
			return;
		}
	}
	else if ((strstr(p, "!ct")) || (strstr(p, "!CT")) || (strstr(p, "!Ct")) || (strstr(p, "!cT"))) {
		if (ent->client->sess.clanMember){
			p = ConcatArgs(1);
			for(i=0;i<=strlen(p);i++){
				if(p[i] == '!' && (p[i+1] == 'c' || p[i+1] == 'C') && (p[i+2] == 't' || p[i+2] == 'T')){
					ignore = i;
				}
				if(ignore == -1){
				newp[a] = p[i];
				a += 1;
				}else if(i == ignore || i == ignore+1 || i == ignore+2){
				if(a != 0)
					i+=1; // Fix for spaces
				}else{
				if(a == 0 && p[i] == ' ') // Fix for spaces
					continue;
				newp[a] = p[i];
				a += 1;
				}
			}
			mode = CLAN_TALK;
			acmd = qtrue;
			strcpy(p, newp);
		}else{
			p = ConcatArgs(1);
			G_Say( ent, NULL, mode, p );
			return;
		}
	}
	else if ((strstr(p, "!ca")) || (strstr(p, "!Ca")) || (strstr(p, "!cA")) || (strstr(p, "!CA"))) {
			p = ConcatArgs(1);
			for(i=0;i<=strlen(p);i++){
				if(p[i] == '!' && (p[i+1] == 'c' || p[i+1] == 'C') && (p[i+2] == 'a' || p[i+2] == 'A')){
					ignore = i;
				}
				if(ignore == -1){
				newp[a] = p[i];
				a += 1;
				}else if(i == ignore || i == ignore+1 || i == ignore+2){
				if(a != 0)
					i+=1; // Fix for spaces
				}else{
				if(a == 0 && p[i] == ' ') // Fix for spaces
					continue;
				newp[a] = p[i];
				a += 1;
				}
			}

			// Boe!Man 8/25/11: If an admin is using Hey Admin, force it to Admin Chat instead.
			if(ent->client->sess.admin){
				mode = ADM_CHAT;
			}else{
				mode = CADM_CHAT;
			}
			acmd = qtrue;
			strcpy(p, newp);
	}
	// Henk loop through my admin command array
	// Boe!Man 1/8/11: Only go through this cycle if the client indeed has admin powers. If not, save on resources.
	if(ent->client->sess.admin > 0){
	if(acmd != qtrue){
		for(i=0;i<AdminCommandsSize;i++){
			if(strstr(Q_strlwr(test), Q_strlwr(AdminCommands[i].shortCmd)) && IsValidCommand(AdminCommands[i].shortCmd, test)){
				command = qtrue;
				if(ent->client->sess.admin >= *AdminCommands[i].adminLevel){
					AdminCommands[i].Function(1, ent, qtrue);
					break;
				}else{
					if(ent->client->sess.referee == 1 && strstr(Q_strlwr(test), "!l")){ // exception for referee lock
						Henk_Lock(1, ent, qtrue);
					}
					trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
				}
			}
		}
		// check custom commands
	if(g_enableCustomCommands.integer == 1){
		if(level.custom == qtrue){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7There's already a custom command being executed.\n\""));
			return;
		}
		GP2 = trap_GP_ParseFile(g_customCommandsFile.string, qtrue, qfalse);
		if(GP2){
			group = trap_GPG_GetSubGroups(GP2);
			while(group){
				trap_GPG_FindPairValue(group, "ShortCommand", "none", name);
				if(!strstr(name, "none")){
					if(strstr(Q_strlwr(test), Q_strlwr(name))){
						command = qtrue;
						trap_GPG_FindPairValue(group, "AdminLevel", "5", txtlevel);
						if(ent->client->sess.admin >= atoi(txtlevel)){
							trap_GPG_FindPairValue(group, "Action", "say \"No custom action defined\"", action);
							trap_GPG_FindPairValue(group, "Broadcast", "Custom action applied", broadcast);
							trap_GPG_FindPairValue(group, "Message", "Custom action has been applied.", message);
							trap_SendServerCommand( -1, va("print \"^3[Custom Admin Action] ^7%s.\n\"", message));
							trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%s", level.time + 5000, broadcast));
							memset(level.action, 0, sizeof(level.action));
							Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
							strcpy(level.action, action);
							level.customtime = level.time+2000;
							level.custom = qtrue;
						}else{
							trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your admin level is too low to use this command.\n\""));
						}
					}
				}
				group = trap_GPG_GetNext ( group );
			}
			trap_GP_Delete(&GP2);
		}

	}
		//

	}
	// End

	/*	
	// Boe!Man 5/2/10: The Referee tokens. I think we're actually the first with this... (:
	// Boe!Man 6/3/11: Not needed, no one uses them.
	if(strstr(p, "!i ")){
		if(ent->client->sess.admin > 1){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Use /adm suspend or !su to become a Referee!\n\""));
		}else if(ent->client->sess.referee == 1){
			if (!level.gametypeData->teams){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Not playing a team game!\n\""));
			return;}
			if (strstr(p, "b")){
			strcpy(team, "b");
			RPM_TeamInfo(ent, va("%s", team));}
			else if(strstr(p, "r")){
			strcpy(team, "b");
			RPM_TeamInfo(ent, va("%s", team));}
			else{
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Unknown team specified!\n\""));
			return;}
		}
		G_Say( ent, NULL, mode, p);
		return;
	}
	else if(strstr(p, "!rt")){
		if(ent->client->sess.admin > 1){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Use /adm suspend or !su to become a Referee!\n\""));
		}else if(ent->client->sess.referee == 1){
			if(level.warmupTime == 0 || g_doWarmup.integer != 2){
			trap_SendServerCommand(ent - g_entities, va("print \"^3[Info] ^7Server is currently not in Ready-up mode.\n\""));
			return;}
			if (!level.gametypeData->teams){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Not playing a team game!\n\""));
			return;}
			if(strlen(p) == 3){
			//RPM_ReadyAll();
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@Teams %sr%se%sa%sd%si%sed by %s", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, ent->client->pers.netname));
			trap_SendServerCommand( -1, va("print \"^3[Referee Action] ^7Teams readied by %s.\n\"", ent->client->pers.netname));
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			}
		}
		//else
		//{
			//RPM_ReadyTeam(ent, qtrue, arg2);
		//}
		G_Say( ent, NULL, mode, p);
		return;
	//}else if(strstr(p, "!test")){
	//	AdminCommands[1].Function(1, ent, qtrue);
	//	trap_SendServerCommand( -1, va("print \"^3[Debug] ^7%s level is %i.\n\"", AdminCommands[1].adminCmd, *AdminCommands[1].adminLevel));

	}
	*/
	}

	// Boe!Man 12/20/09
	Boe_Tokens(ent, p, mode, qtrue);
	Boe_Tokens(ent, p, mode, qfalse);
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
	
	Boe_Tokens(ent, p, SAY_TELL, qtrue);
	Boe_Tokens(ent, p, SAY_TELL, qfalse);

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

	if(strlen(id) > 64){
		char	text[32] = "";
		Q_strncpyz(text, id, sizeof(text));
		//trap_SendServerCommand( ent-g_entities, va("print \"You have a very small penis.\n\"" ) ); // boe - lol'd
		//Boe_crashLog(va("%s - %s - vsay_team: %s", ent->client->pers.cleanName, ent->client->pers.ip, text)) ;
		return;
	}

	// Spectators and ghosts dont talk
	if ( ent->client->ps.pm_type == PM_DEAD || G_IsClientSpectating ( ent->client ) )
	{
		return;
	}

	// Voice flooding protection on?
	if ( g_voiceFloodCount.integer )
	{
		// If this client has been penalized for voice chatting to much then dont allow the voice chat
		if ( ent->client->sess.voiceFloodPenalty )
		{
			if ( ent->client->sess.voiceFloodPenalty > level.time )
			{
				return;
			}

			// No longer penalized
			ent->client->sess.voiceFloodPenalty = 0;
		}

		// See if this client flooded with voice chats
		ent->client->sess.voiceFloodCount++;
		if ( ent->client->sess.voiceFloodCount >= g_voiceFloodCount.integer )
		{
			ent->client->sess.voiceFloodCount = 0;
			ent->client->sess.voiceFloodTimer = 0;
			ent->client->sess.voiceFloodPenalty = level.time + g_voiceFloodPenalty.integer * 1000;

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

	TeleportPlayer( ent, origin, angles, qfalse );
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
		if(IsClientMuted(ent, qtrue)){
			return;
		}
		Cmd_Say_f (ent, SAY_ALL, qfalse);
		return;
	}
	if (Q_stricmp (cmd, "say_team") == 0) {
		// Boe!Man 1/30/10: We need to make sure the clients aren't muted.. And otherwise prevent them talking.
		/*if(ent->client->sess.mute){
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You are currently muted by an Admin.\n\"") );
		return;
		}*/
		if(IsClientMuted(ent, qtrue)){
			return;
		}
		Cmd_Say_f (ent, SAY_TEAM, qfalse);
		return;
	}
	
	if (Q_stricmp (cmd, "tell") == 0) 
	{
		// Boe!Man 1/30/10: We need to make sure the clients aren't muted.. And otherwise prevent them talking.
		/*if(ent->client->sess.mute){
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You are currently muted by an Admin.\n\"") );
		return;
		}*/
		if(IsClientMuted(ent, qtrue)){
			return;
		}
		Cmd_Tell_f ( ent );
		return;
	}

	if (Q_stricmp (cmd, "vsay_team") == 0) 
	{
		// Boe!Man 1/30/10: We need to make sure the clients aren't muted.. And otherwise prevent them talking.
		/*if(ent->client->sess.mute){
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You are currently muted by an Admin.\n\"") );
		return;
		}*/
		if(IsClientMuted(ent, qtrue)){
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

	if (Q_stricmp (cmd, "ignore") == 0)
	{
		Henk_Ignore (ent);
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
	else if (Q_stricmp (cmd, "dropitem") == 0)
		Cmd_DropItem_f (ent);
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
	else if (Q_stricmp (cmd, "info") == 0)
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
	else if (Q_stricmp (cmd, "tokens") == 0)
		Boe_displayTokens ( ent );

	// Boe!Man 4/3/10
#ifndef PUB_RELEASE
	else if (Q_stricmp (cmd, "dev") == 0)
		Boe_dev_f( ent );
#endif

	// Henk 07/04/10 -> Send info to all players(for RPM scoreboard)
	else if (Q_stricmp (cmd, "tmi") == 0){
		RPM_UpdateTMI();
	}else if (Q_stricmp (cmd, "refresh") == 0)
		RPM_Refresh( ent );
	else if (Q_stricmp (cmd, "ready") == 0)
		RPM_ReadyUp( ent );
	else if (Q_stricmp (cmd, "tcmd") == 0)
		RPM_Tcmd( ent );
	else if (Q_stricmp (cmd, "ref") == 0)
		RPM_ref_cmd( ent );
	// Boe!Man 10/5/10: New how-to command for CTB.
	else if (Q_stricmp (cmd, "howto") == 0)
		Boe_Howto( ent );
	
	
#ifndef PUB_RELEASE
	/*else if (Q_stricmp (cmd, "henk_test") == 0){
		ent->client->sess.zombie = qfalse;
		ent->client->sess.zombiebody = -1;
		ent->client->sess.firstzombie = qfalse;
		level.zombie = 0;
		
	}*/
	//else if (Q_stricmp (cmd, "henk_test") == 0){
	//	trap_Argv( 1, cmd, sizeof( cmd ) );
	//	DropRandom(ent, atoi(cmd));
	//}
	// F1 effect: effects/explosions/col9_boat_explosion.efx
	// or effects/levels/osprey_death_explosion.efx
	// Boe!Man 4/8/11: DEBUG COMMANDS BELOW.
	// ======================================
	/*
	else if (Q_stricmp (cmd, "henk_score") == 0){
		UpdateScores();
		ShowScores();
	}
	else if (Q_stricmp (cmd, "henk1") == 0){
		ent->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_F1_GRENADE );
		ent->client->ps.clip[ATTACK_NORMAL][WP_F1_GRENADE]=1;
		ent->client->ps.ammo[weaponData[WP_F1_GRENADE].attack[ATTACK_NORMAL].ammoIndex]=10;
	}
	else if (Q_stricmp (cmd, "henk2") == 0){
		ent->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_L2A2_GRENADE );
		ent->client->ps.clip[ATTACK_NORMAL][WP_L2A2_GRENADE]=1;
		ent->client->ps.ammo[weaponData[WP_L2A2_GRENADE].attack[ATTACK_NORMAL].ammoIndex]=10;
	}
	else if (Q_stricmp (cmd, "boeboe") == 0){
		InitCagefight();
	}
	else if (Q_stricmp (cmd, "boeboe1") == 0){
		// Build a list of spawns
		for ( test = 0; test < level.spawnCount; test ++ ){
			trap_SendServerCommand( clientNum, va("print\"[%i] origin: %.2f %.2f %.2f\n\"", test, level.spawns[test].origin[0], level.spawns[test].origin[1], level.spawns[test].origin[2]) );
		}
	}
	else if(Q_stricmp(cmd, "listskins") == 0){
		int i;
		TIdentity	*oldidentity;

		for (i = 0; i < 105; i++){
			oldidentity = &bg_identities[i];
			if(oldidentity){
				trap_SendServerCommand( clientNum, va("print\"[%i] %s\n\"", i, oldidentity->mName) );
			}
		}
	}
	else if (Q_stricmp (cmd, "boeboe2") == 0){
		// Show tied players.
		trap_SendServerCommand( clientNum, va("print\"^3[Debug] ^7%i winners that are tied and should go into cagefight.\n\"", TiedPlayers() ));
		// Start cagefight.
		InitCagefight();
	}
	else if (Q_stricmp (cmd, "boeboe3") == 0){
		// Ban a player on specific arguments.
		char ip[MAX_TOKEN_CHARS];
		char name[MAX_TOKEN_CHARS];
		char by[MAX_TOKEN_CHARS];
		char reason[MAX_TOKEN_CHARS];
		char banid[1024]; // The fdsfdsfds
		
		trap_Argv( 1, ip, sizeof( ip ) );
		trap_Argv( 2, name, sizeof( name ) );
		trap_Argv( 3, by, sizeof( by ) );
		trap_Argv( 4, reason, sizeof( reason ) );
		
		trap_SendServerCommand(-1, va("print\"^3[Debug] ^7I'm going to ban the following player IP: %s\n\"", ip));
		trap_SendServerCommand(-1, va("print\"^3[Debug] ^7Whose name is: %s\n\"", name));
		trap_SendServerCommand(-1, va("print\"^3[Debug] ^7By this fella: %s\n\"", by));
		trap_SendServerCommand(-1, va("print\"^3[Debug] ^7For the reason: %s\n\n\"", reason));
		
		Com_sprintf (banid, sizeof(banid), "%s\\%s//%s||%s",
		ip,
		name,
		by,
		reason);
		
		trap_SendServerCommand(-1, va("print\"^3[Debug] ^7This is the sprintf string I just created (banid): %s\n\n\"", banid));
		if(Boe_AddToList(banid, g_banfile.string, "Ban", NULL)){
			trap_SendServerCommand(-1, va("print\"^3[Debug] ^7Succesfully written to the file: %s\n\"", g_banfile.string));
		}else{
			trap_SendServerCommand(-1, va("print\"^3[Debug] ^7Write fail to: %s\n\"", g_banfile.string));
		}
	}else if(Q_stricmp (cmd, "boeboe4") == 0){
		// Set enterTime A LOT higher so we can fool et that we entered last.
		ent->client->pers.enterTime = level.time;
	}else if(Q_stricmp (cmd, "boeboe5") == 0){
		trap_SetConfigstring( CS_MUSIC, "music/hongkong/hk1" );
	}
	*/
#endif
	
#ifdef _SOF2_BOTS
	else if (Q_stricmp (cmd, "addbot") == 0)
		trap_SendServerCommand( clientNum, va("print \"ADDBOT command can only be used via RCON\n\"" ) );
#endif

	else
		trap_SendServerCommand( clientNum, va("print \"unknown cmd %s\n\"", cmd ) );
}

/*
==========
Boe_Adm_f
==========
*/

void Boe_adm_f ( gentity_t *ent )
{
	int		i, x, adm, levelx, to, oldcolour, sendsize;
	char	arg1[MAX_STRING_TOKENS];
	char	arg2[MAX_STRING_TOKENS];
	gclient_t	*client;

	void	*GP2, *group;
	char txtlevel[2];
	char name[10];
	char action[512];
	char *action2;
	char message[512];
	char broadcast[512];
	
	// Boe!Man 7/9/12: strcat everything to a 'big buffer', and afterwards send big packets to the client (to keep the packet size as small as possible).
	// As of rev 804 (using default admin level values) big buffer sizes:
	// /adm as S-Admin: 3912
	// /adm list: 4207
	// /adm as S-Admin w/ cm enabled: 4014
	// /adm list w/ cm enabled: 4309
	char bigbuf[4500] = "\0";
	char sendbuf[1024] = "\0";
	char *point;

	client = ent->client;
	trap_Argv( 1, arg1, sizeof( arg1 ) );
	trap_Argv( 2, arg2, sizeof( arg2 ) );
	
	adm = ent->client->sess.admin;
	if(!Q_stricmp(arg1, "login")){ // Boe!Man 4/3/11: Very small optimize.
		if(g_passwordAdmins.integer && !adm){ // only check if its enabled -- Boe!Man 6/28/11: And check if they're admin-less still.
			if(!Q_stricmp(arg2, "none")){ // no password set so deny access.
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Access denied: Default password can't be used!\n\""));
				return;
			}else if(!Q_stricmp(arg2, g_adminPass.string) || !Q_stricmp(arg2, g_sadminPass.string) || !Q_stricmp(arg2, g_badminPass.string)){ // good password, now check if he's in admin list
				if(CheckPasswordList(ent, arg2)){
					// got admin message
					return;
				}else{
					trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Access denied: Invalid password!\n\""));
					return;
				}
			}else{ // wrong password
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Access denied: Invalid password!\n\""));
				return;
			}
		}else if(!g_passwordAdmins.integer){ // Boe!Man 3/31/11: Else display an alternate message, instead of the "You don't have Admin powers!" message (confusing).
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Access denied: No password logins allowed by the server!\n\""));
			return;
		}else if(adm){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Access denied: You already got Admin powers!\n\""));
			return;
		}
	}

	if(!adm){
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Access denied: You don't have Admin powers!\n\""));
		return;
	}
	if (!Q_stricmp ( arg1, "?" )||!Q_stricmp ( arg1, "" )||!Q_stricmp(arg1, "list"))
	{
	if (adm > 1){
		trap_SendServerCommand( ent-g_entities, va("print \" \n^3Lvl   Commands         Arguments      Explanation\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" ----------------------------------------------------------\n\""));
		// Boe!Man 6/17/11: If they want to print the whole list of admin commands, be sure to do so.
		if(!Q_stricmp(arg1, "list")){
			to = 5; // Include RCON commands.
		}else{
			to = adm;
		}

		// Boe!Man 9/21/10: We loop the print process and make sure they get in proper order.
		for(levelx=2;levelx<=to;levelx++){
		if (g_kick.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   k   kick         <i/n> <reason> ^7[^3Kick a player^7]\n", g_kick.integer));
			}
		if (g_addbadmin.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   ab  addbadmin    <i/n>          ^7[^3Add a Basic Admin^7]\n", g_addbadmin.integer));
			}
		if (g_addadmin.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   aa  addadmin     <i/n>          ^7[^3Add an Admin^7]\n", g_addadmin.integer));
			}
		// Boe!Man 1/4/10: Fix with using Tab in the Admin list.
		if (g_addsadmin.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   as  addsadmin    <i/n>          ^7[^3Add a Server Admin^7]\n", g_addsadmin.integer));
			}
		if (g_ban.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   ba  ban          <i/n> <reason> ^7[^3Ban a player^7]\n", g_ban.integer));
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   uba unban        <ip/line #>    ^7[^3Unban a banned IP^7]\n", g_ban.integer));
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   bl  banlist                     ^7[^3Shows the current banlist^7]\n", g_ban.integer));
			}
		if (g_subnetban.integer == levelx){
			// Boe!Man 1/6/10: Reason added to the Subnetban command.
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   sb  subnetban    <i/n> <reason> ^7[^3Ban a players' subnet^7]\n", g_subnetban.integer));
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   sbu subnetunban  <ip/line #>    ^7[^3Unban a banned subnet^7]\n", g_subnetban.integer));
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   sbl subnetbanlist               ^7[^3Shows the current subnetbanlist^7]\n", g_subnetban.integer));
			}
		if (g_uppercut.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   uc  uppercut     <i/n>          ^7[^3Launch a player upwards^7]\n", g_uppercut.integer));
			}
		if (g_twist.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   tw  twist        <i/n>          ^7[^3Twist a player^7]\n", g_twist.integer));
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   utw untwist      <i/n>          ^7[^3Untwist a twisted player^7]\n", g_twist.integer));
			}
		if (g_runover.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   ro  runover      <i/n>          ^7[^3Boost a player backwards^7]\n", g_runover.integer));
			}
		if (g_mapswitch.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   mr  maprestart                  ^7[^3Restart the current map^7]\n", g_mapswitch.integer));
			}
		if (g_flash.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   fl  flash        <i/n>          ^7[^3Flash a player^7]\n", g_flash.integer));
			}
		if (g_pop.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   p   pop          <i/n>          ^7[^3Pop a player^7]\n", g_pop.integer));
			}
		if (g_strip.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   s   strip        <i/n>          ^7[^3Remove weapons from a player^7]\n", g_strip.integer));
			}
		if (g_mute.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   m   mute         <i/n> <time>   ^7[^3Mute a player^7]\n", g_mute.integer));
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   um  unmute       <i/n>          ^7[^3Unmute a player^7]\n", g_mute.integer));
			}
		if (g_plant.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   pl  plant        <i/n>          ^7[^3Plant a player in the ground^7]\n", g_plant.integer));
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   upl unplant      <i/n>          ^7[^3Unplant a planted player^7]\n", g_plant.integer));
			}
		if (g_burn.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   b   burn         <i/n>          ^7[^3Burn a player^7]\n", g_burn.integer));
			}
		if (g_eventeams.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   et  eventeams                   ^7[^3Make the teams even^7]\n", g_eventeams.integer));
			}
		if (g_sl.integer == levelx && cm_enabled.integer != 1){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   sl  scorelimit   <time>         ^7[^3Change the scorelimit^7]\n", g_sl.integer));
			}
		if (g_tl.integer == levelx && cm_enabled.integer != 1){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   tl  timelimit    <time>         ^7[^3Change the timelimit^7]\n", g_tl.integer));
			}
		if (g_nolower.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   nl  nolower                     ^7[^3Enable/Disable Nolower^7]\n", g_nolower.integer));
			}
		if (g_noroof.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   nr  noroof                      ^7[^3Enable/Disable Noroof^7]\n", g_noroof.integer));
			}
		if (g_damage.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   nd  normaldamage                ^7[^3Toggle Normal damage^7]\n", g_damage.integer));
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   rd  realdamage                  ^7[^3Toggle Real damage^7]\n", g_damage.integer));
			}
		if (g_ri.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   ri  ri           <time>         ^7[^3Change the respawn interval^7]\n", g_ri.integer));
			}
		if (g_gr.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   gr  gametyperestart             ^7[^3Restart the current gametype^7]\n", g_gr.integer));
			}
		if (g_clanvsall.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   cva clanvsall                   ^7[^3Clan versus other players^7]\n", g_clanvsall.integer));
			}
		if (g_swapteams.integer == levelx && cm_enabled.integer != 1){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   sw  swapteams                   ^7[^3Swap the players from both teams^7]\n", g_swapteams.integer));
			}
		if (g_shuffleteams.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   sh  shuffleteams                ^7[^3Mix the teams at random^7]\n", g_shuffleteams.integer));
			}
		if (g_lock.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   l   lock         <team>         ^7[^3Lock a team^7]\n", g_lock.integer));
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   ul  unlock       <team>         ^7[^3Unlock a team^7]\n", g_lock.integer));
			}
		if (g_clan.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   acl addclan      <i/n>          ^7[^3Add a clan member^7]\n", g_clan.integer));
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   rcl removeclan   <i/n>          ^7[^3Remove a clan member^7]\n", g_clan.integer));
			}
		if (g_broadcast.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   br  broadcast    <message>      ^7[^3Broadcast a message^7]\n", g_broadcast.integer));
			}
		if (g_forceteam.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   ft  forceteam    <i/n> <team>   ^7[^3Force a player to join a team^7]\n", g_forceteam.integer));
			}
		if (g_nades.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   nn  nonades                     ^7[^3Enable or disable nades^7]\n", g_nades.integer));
			}
		if (g_respawn.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   rs  respawn                     ^7[^3Respawn a player^7]\n", g_respawn.integer));
			}
		if (g_removeadmin.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   ra  removeadmin  <i/n>          ^7[^3Remove an Admin^7]\n", g_removeadmin.integer));
			}
		if (g_cm.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   cm  compmode                    ^7[^3Toggles Competition Mode^7]\n", g_cm.integer));
			}
		if (g_mapswitch.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   g   gametype     <gametype>     ^7[^3Switch to the specified gametype^7]\n", g_mapswitch.integer));
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   map              <map name>     ^7[^3Switch to the specified map^7]\n", g_mapswitch.integer));
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   altmap           <map name>     ^7[^3Switch to the specified altmap^7]\n", g_mapswitch.integer));
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   devmap           <map name>     ^7[^3Switch to the specified devmap^7]\n", g_mapswitch.integer));
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   mapcycle                        ^7[^3Switch to the next map in the cycle^7]\n", g_mapswitch.integer));
			}
		if (g_pause.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   pa  pause                       ^7[^3Pause the game^7]\n", g_pause.integer));
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   upa unpause                     ^7[^3Resume the game^7]\n", g_pause.integer));
			}
		if (g_forcevote.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   pv  passvote                    ^7[^3Pass the running vote^7]\n", g_forcevote.integer));
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   cv  cancelvote                  ^7[^3Cancel the running vote^7]\n", g_forcevote.integer));
			}
		if (g_adminlist.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   al  adminlist                   ^7[^3Show the Adminlist^7]\n", g_adminlist.integer));
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   al  adminlist    'pass'         ^7[^3Show the passworded Adminlist^7]\n", g_adminlist.integer));
			}
		if (g_adminremove.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   adr adminremove  <line #>       ^7[^3Remove an Admin from the list^7]\n", g_adminremove.integer));
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   adr adminremove  <line #>'pass' ^7[^3Remove a passworded Admin from list^7]\n", g_adminremove.integer));
			}
		if (g_3rd.integer == levelx){
			Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   3rd third                       ^7[^3Toggles Thirdperson on or off^7]\n", g_3rd.integer));
			}

		// Boe!Man 6/17/11: Display competition mode Admin commands when it's enabled in the starting stage.
		if(cm_enabled.integer == 1){
			if(to != 5 && g_cm.integer == levelx || to == 5 && levelx == 5){
				Q_strcat(bigbuf, sizeof(bigbuf), va("\n^7[^3Competition Mode Commands^7]\n"));
				Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   rounds           'one/two'      ^7[^3Set number of rounds^7]\n", g_cm.integer));
				Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   sw  swapteams                   ^7[^3Toggle auto swap^7]\n", g_cm.integer));
				Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   sl  scorelimit   <time>         ^7[^3Change the match scorelimit^7]\n", g_cm.integer));
				Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7]   tl  timelimit    <time>         ^7[^3Change the match timelimit^7]\n", g_cm.integer));
			}
		}
		
		// Boe!Man 9/21/10: End of Loop.
		}
		
		// Boe!Man 7/9/12: Now process that loop. We put everything in a big buffer, split in packets of 1000 bytes ea (max is 1024, header should still be sent along with the packet).
		//Com_Printf("The /adm buffer size is: %i\n", strlen(bigbuf));
		if(strlen(bigbuf) <= 1000){ // Buffer is still small (could be used for limited B-Admins or so), just send the packet as a whole.
			trap_SendServerCommand( ent-g_entities, va("print \"%s\"", bigbuf));
		}else{
			point = bigbuf; // pointer to bigbuf.
			levelx = Boe_firstDigitOfInt(strlen(bigbuf));
			
			for(i=0;i<=levelx;i++){
				if(i != levelx){
					if(point[999] == '^'){ // Boe!Man 7/17/12: Don't allow to copy in the middle of the color..
						strncpy(sendbuf, point, 1001);
						point += 1001;
					}else{
						strncpy(sendbuf, point, 1000);
						point += 1000;
					}
				}else{
					strncpy(sendbuf, point, strlen(point));
				}
				
				if(oldcolour == 1){
					trap_SendServerCommand( ent-g_entities, va("print \"^3%s\"", sendbuf));
				}else{
					trap_SendServerCommand( ent-g_entities, va("print \"%s\"", sendbuf));
				}
				
				x = 0;
				oldcolour = -1;
				sendsize = strlen(sendbuf);
				while (oldcolour == -1){ // Check for the last colour, if we split the packet in a "yellow" colour, that won't get restored. Hence, search for it now.
					if(sendbuf[sendsize-x] == '3' && sendbuf[sendsize-x-1] == '^'){
						oldcolour = 1;
						//Com_Printf("Old colour was yellow, supposeably. i = %i\n", i);
					}else if(sendbuf[sendsize-x] == '7' && sendbuf[sendsize-x-1] == '^'){
						oldcolour = 0;
					}else if(x == 50){ // avoid endless loops. 50 should be sufficient.
						oldcolour = 0;
					}
					x += 1;
				}
				
				memset(sendbuf, 0, sizeof(sendbuf));
			}
		}
					
					
		memset(bigbuf, 0, sizeof(bigbuf)); // Boe!Man 7/9/12: Clear memory (reset buffer).

		if(g_enableCustomCommands.integer == 1){
			trap_SendServerCommand( ent-g_entities, va("print \"\n^7[^3Custom Commands^7] \n\"")); // Boe!Man 3/6/11: Spaces fix (so layout doesn't mess up).
			PrintCustom(ent-g_entities);
			// Add wrapper for CustomCommands file
		}

		// Boe!Man 6/17/11: Add a note for RCON if they display the whole list.
		if(to == 5){
			trap_SendServerCommand( ent-g_entities, va("print \"  [^32^7] B-Admin        ^7[^33^7] Admin        ^7[^34^7] S-Admin        ^7[^35^7] RCON\n\""));
		}else{
			trap_SendServerCommand( ent-g_entities, va("print \"    [^32^7] B-Admin          ^7[^33^7] Admin          ^7[^34^7] S-Admin\n\""));
		}
		trap_SendServerCommand( ent-g_entities, va("print \"\n^7Use ^3[Page Up]^7 and ^3[Page Down]^7 keys to scroll.\n\""));
	}
	return;
	}
	
	// Henk loop through my admin command array
	
	if (!Q_stricmp ( arg1, "chat" ) || !Q_stricmp ( arg1, "adminchat" )){
		/*
		if(ent->client->sess.mute){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You are currently muted by admin.\n\"") );
			return;
		}*/
		if(IsClientMuted(ent, qtrue)){
			return;
		}
		Cmd_Say_f (ent, ADM_CHAT, qfalse);
		return;
	}
	if (!Q_stricmp ( arg1, "talk" ) || !Q_stricmp ( arg1, "admintalk" )){
		/*if(ent->client->sess.mute){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You are currently muted by admin.\n\"") );
			return;
		}*/
		if(IsClientMuted(ent, qtrue)){
			return;
		}
		Cmd_Say_f (ent, ADM_TALK, qfalse);
		return;
	}

	if (!Q_stricmp ( arg1, "clanchat" )){
		/*if(ent->client->sess.mute){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You are currently muted by admin.\n\"") );
			return;
		}*/
		if(IsClientMuted(ent, qtrue)){
			return;
		}else if(!ent->client->sess.clanMember){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You are no clan member.\n\"") );
			return;
		}
		Cmd_Say_f (ent, CLAN_CHAT, qfalse);
		return;
	}

	for(i=0;i<AdminCommandsSize;i++){ // Henk 15/09/10 -> Fixed loop going outside array(causing crashes)
		//Com_Printf("Checking %s with %s\n", arg1, AdminCommands[i].adminCmd);
		if(!Q_stricmp(arg1, AdminCommands[i].adminCmd)){
			if(ent->client->sess.admin >= *AdminCommands[i].adminLevel){
				AdminCommands[i].Function(2, ent, qfalse);
				return;
			}else{
			// Boe!Man 12/30/09: Putting two Info messages together.
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your admin level is too low to use this command.\n\""));
			return;
			}
		}
	}
	if(g_enableCustomCommands.integer == 1){
		if(level.custom == qtrue){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7There's already a custom command being executed.\n\""));
			return;
		}
		GP2 = trap_GP_ParseFile(g_customCommandsFile.string, qtrue, qfalse);
		if(!GP2){
			Com_Printf("Error while loading %s\n", g_customCommandsFile.string);
			return;
		}
		group = trap_GPG_GetSubGroups(GP2);
		while(group){
			trap_GPG_FindPairValue(group, "Command", "none", name);
			if(!Q_stricmp(arg1, name)){
				trap_GPG_FindPairValue(group, "AdminLevel", "5", txtlevel);
				if(ent->client->sess.admin >= atoi(txtlevel)){
					trap_GPG_FindPairValue(group, "Action", "say \"No custom action defined\"", action);
					if(strstr(action, "%arg")){ // Boe!Man 7/27/12: Doesn't matter what argument, as long as there's an argument to be replaced, do it.
						action2 = Boe_parseCustomCommandArgs(action);
						memset(action, 0, sizeof(action));
						strncpy(action, action2, strlen(action2));
					}
					trap_GPG_FindPairValue(group, "Broadcast", "Custom action applied", broadcast);
					trap_GPG_FindPairValue(group, "Message", "Custom action has been applied.", message);
					trap_SendServerCommand( -1, va("print \"^3[Custom Admin action] ^7%s.\n\"", message));
					trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%s", level.time + 5000, broadcast));
					memset(level.action, 0, sizeof(level.action));
					strcpy(level.action, action);
					Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
					level.customtime = level.time+2000;
					level.custom = qtrue;
					return;
				}else{
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your admin level is too low to use this command.\n\""));
				return;
				}
			}
			group = trap_GPG_GetNext ( group );
		}
		trap_GP_Delete(&GP2);
	}
}

/*
=================
ConsoleCommand
=================
*/
qboolean ConsoleCommand( void )
{
	char cmd[MAX_TOKEN_CHARS];
	int i;
	char arg1[64];
	char arg2[64];

	trap_Argv( 0, cmd, sizeof( cmd ) );
	trap_Argv( 1, arg1, sizeof( arg1 ) );
	trap_Argv( 2, arg2, sizeof( arg2 ) );

	// Boe!Man 4/6/11: Check the altmap command prior to the admin loop.
	if ( Q_stricmp (cmd, "altmap") == 0 )
	{
		if(strstr(arg2, "ctf") || strstr(arg2, "inf") || strstr(arg2, "tdm") || strstr(arg2, "dm") || strstr(arg2, "elim") || strstr(arg2, "h&s") || strstr(arg2, "h&z")){
			trap_Cvar_Set( "g_gametype", arg2);
			trap_Cvar_Update ( &g_gametype );
		}
		trap_Cvar_Set( "g_alternateMap", "1");
		trap_Cvar_Update ( &g_alternateMap );
		trap_SendConsoleCommand( EXEC_APPEND, va("map %s\n", arg1));
		return qtrue;
	}

	for(i=0;i<AdminCommandsSize;i++){ // Henk 15/09/10 -> Fixed loop going outside array(causing crashes)
		//Com_Printf("Checking %s with %s\n", arg1, AdminCommands[i].adminCmd);
		if(!Q_stricmp(cmd, AdminCommands[i].adminCmd)){
				AdminCommands[i].Function(1, NULL, qfalse);
				return qtrue;
		}
	}

	if ( Q_stricmp (cmd, "entitylist") == 0 )
	{
		Svcmd_EntityList_f();
		return qtrue;
	}

	/*if ( Q_stricmp (cmd, "forceteam") == 0 )
	{
		Svcmd_ForceTeam_f();
		return qtrue;
	}

	if ( Q_stricmp ( cmd, "cancelvote" ) == 0 )
	{
		Svcmd_CancelVote_f();
		return qtrue;
	}*/

#ifdef _SOF2_BOTS

	if (Q_stricmp (cmd, "addbot") == 0)
	{
		//trap_Printf("^3[Info] ^7This command has been temporary disabled by the Mod Developers.\n");
		Svcmd_AddBot_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "botlist") == 0) 
	{
		//trap_Printf("^3[Info] ^7This command has been temporary disabled by the Mod Developers.\n");
		Svcmd_BotList_f();
		return qtrue;
	}

#endif

	// Boe!Man 10/31/11: Removing the original addip/removeip system since it doesn't get used. Updating it to use the current ban system.

	if (Q_stricmp (cmd, "addip") == 0)
	{
		Svcmd_AddIP_f();
		return qtrue;
	}
	
	// Boe!Man 11/04/11: This is obsolete and doesn't get used (they can use unban/subnetunban instead). Removed.
	/*
	if (Q_stricmp (cmd, "removeip") == 0)
	{
		Svcmd_RemoveIP_f();
		return qtrue;
	}
	*/

	if (Q_stricmp (cmd, "listip") == 0)
	{
		// Boe!Man 10/31/11: This gets replaced with the 1fx. Mod-style banlist.
		trap_SendConsoleCommand( EXEC_NOW, "banlist\n" );
		return qtrue;
	}
	
	// End Boe!Man 10/31/11

	if (Q_stricmp (cmd, "gametype_restart" ) == 0 )
	{
		trap_Argv( 1, cmd, sizeof( cmd ) );
		G_ResetGametype ( Q_stricmp ( cmd, "full" ) == 0, qfalse );
		return qtrue;
	}

	if (Q_stricmp (cmd, "extendtime" ) == 0 )
	{
		Svcmd_ExtendTime_f();
		return qtrue;
	}

	if (g_dedicated.integer) 
	{
		if (Q_stricmp (cmd, "say") == 0) 
		{
			trap_SendServerCommand( -1, va("chat -1 \"%s: %s\n\"", server_rconprefix.string, ConcatArgs(1) ) );
			return qtrue;
		}

		// everything else will also be printed as a say command
		trap_SendServerCommand( -1, va("chat -1 \"%s: %s\n\"", server_rconprefix.string, ConcatArgs(0) ) );
		return qtrue;
	}

	return qfalse;
}
