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
	{"!uc ", "uppercut", &g_uppercut.integer, &Boe_Uppercut},
	{"!u ", "uppercut", &g_uppercut.integer, &Boe_Uppercut},
	{"!p ", "pop", &g_pop.integer, &Boe_pop},
	{"!k ", "kick", &g_kick.integer, &Boe_Kick},
	{"!ab ", "addbadmin", &g_addbadmin.integer, &Boe_Add_bAdmin_f},
	{"!aa ", "addadmin", &g_addadmin.integer, &Boe_Add_Admin_f},
	{"!as ", "addsadmin", &g_addsadmin.integer, &Boe_Add_sAdmin_f},
	{"!tw ", "twist", &g_twist.integer, &Boe_Twist},
	{"!utw ", "untwist", &g_twist.integer, &Boe_unTwist},
	{"!pl ", "plant", &g_plant.integer, &Boe_Plant},
	{"!upl ", "unplant", &g_plant.integer, &Boe_unPlant},
	{"!ro ", "runover", &g_runover.integer, &Boe_Runover},
	{"!r ", "respawn", &g_respawn.integer, &Boe_Respawn},
	{"!rs ", "respawn", &g_respawn.integer, &Boe_Respawn}, // this is how we add synonyms
	{"!mr", "maprestart", &g_mapswitch.integer, &Boe_MapRestart},
	{"!b ", "burn", &g_burn.integer, &Boe_Burn},
	{"!m ", "mute", &g_mute.integer, &Boe_XMute},
	{"!um ", "unmute", &g_mute.integer, &Boe_UnMute},
	{"!s ", "strip", &g_strip.integer, &Boe_Strip},
	{"!st ", "strip", &g_strip.integer, &Boe_Strip},
	{"!ra ", "removeadmin", &g_removeadmin.integer, &Boe_Remove_Admin_f},
	{"!ft ", "forceteam", &g_forceteam.integer, &Adm_ForceTeam},
	{"!nl", "nolower", &g_nolower.integer, &Boe_NoLower},
	{"!nn", "nonades", &g_nades.integer, &Boe_NoNades},
	{"!sl", "scorelimit", &g_sl.integer, &Boe_ScoreLimit},
	{"!tl", "timelimit", &g_tl.integer, &Boe_TimeLimit},
	{"!ri", "respawninterval", &g_ri.integer, &Boe_RespawnInterval},
	{"!rd", "realdamage", &g_damage.integer, &Boe_RealDamage},
	{"!nd", "normaldamage", &g_damage.integer, &Boe_NormalDamage},
	{"!gr", "gametyperestart", &g_gr.integer, &Boe_GametypeRestart},
	{"!acl ", "addclan", &g_clan.integer, &Boe_Add_Clan_Member},
	{"!rc ", "removeclan", &g_clan.integer, &Boe_Remove_Clan_Member},
	{"!rcl ", "removeclan", &g_clan.integer, &Boe_Remove_Clan_Member},
	{"!3rd", "3rd", &g_3rd.integer, &Boe_Third},
	{"!third", "third", &g_3rd.integer, &Boe_Third},
	{"!cm", "compmode", &g_cm.integer, &Boe_CompMode},
	{"!bl", "banlist", &g_ban.integer, &Henk_BanList},
	{"!ba ", "ban", &g_ban.integer, &Boe_Ban_f},
	{"!br ", "broadcast", &g_broadcast.integer, &Boe_Broadcast},
	{"!sbl", "subnetbanlist", &g_subnetban.integer, &Henk_SubnetBanList},
	{"!et", "eventeams", &g_eventeams.integer, &Henk_EvenTeams},
	{"!cva", "clanvsall", &g_clanvsall.integer, &Henk_CVA},
	{"!sw", "swapteams", &g_swapteams.integer, &Henk_SwapTeams},
	{"!l ", "lock", &g_lock.integer, &Henk_Lock},
	{"!map ", "map", &g_mapswitch.integer, &Henk_Map},
	{"!fl ", "flash", &g_flash.integer, &Henk_Flash},
	{"!g", "gametype", &g_mapswitch.integer, &Henk_Gametype},
	{"!gt", "gametype", &g_mapswitch.integer, &Henk_Gametype},
	{"!upa", "unpause", &g_mapswitch.integer, &Henk_Unpause},
	{"!up", "unpause", &g_mapswitch.integer, &Henk_Unpause},
	{"!pa", "pause", &g_mapswitch.integer, &Henk_Pause},
	{"!uba ", "unban", &g_ban.integer, &Henk_Unban},
	{"!sbu ", "subnetunban", &g_subnetban.integer, &Henk_SubnetUnban},
	{"!ub ", "unban", &g_ban.integer, &Henk_Unban},
	{"!su ", "subnetunban", &g_subnetban.integer, &Henk_SubnetUnban},
	{"!sb ", "subnetban", &g_subnetban.integer, &Boe_subnetBan},

		// Boe!Man 1/22/11: Adding full synonyms.
	{"!uppercut ", "uppercut", &g_uppercut.integer, &Boe_Uppercut},
	{"!pop ", "pop", &g_pop.integer, &Boe_pop},
	{"!kick ", "kick", &g_kick.integer, &Boe_Kick},
	{"!addbadmin ", "addbadmin", &g_addbadmin.integer, &Boe_Add_bAdmin_f},
	{"!addadmin ", "addadmin", &g_addadmin.integer, &Boe_Add_Admin_f},
	{"!addsadmin ", "addsadmin", &g_addsadmin.integer, &Boe_Add_sAdmin_f},
	{"!twist ", "twist", &g_twist.integer, &Boe_Twist},
	{"!untwist ", "untwist", &g_twist.integer, &Boe_unTwist},
	{"!plant ", "plant", &g_plant.integer, &Boe_Plant},
	{"!unplant ", "unplant", &g_plant.integer, &Boe_unPlant},
	{"!runover ", "runover", &g_runover.integer, &Boe_Runover},
	{"!respawn ", "respawn", &g_respawn.integer, &Boe_Respawn},
	{"!maprestart", "maprestart", &g_mapswitch.integer, &Boe_MapRestart},
	{"!burn ", "burn", &g_burn.integer, &Boe_Burn},
	{"!mute ", "mute", &g_mute.integer, &Boe_XMute},
	{"!unmute ", "unmute", &g_mute.integer, &Boe_UnMute},
	{"!strip ", "strip", &g_strip.integer, &Boe_Strip},
	{"!removeadmin ", "removeadmin", &g_removeadmin.integer, &Boe_Remove_Admin_f},
	{"!forceteam ", "forceteam", &g_forceteam.integer, &Adm_ForceTeam},
	{"!nolower", "nolower", &g_nolower.integer, &Boe_NoLower},
	{"!nonades", "nonades", &g_nades.integer, &Boe_NoNades},
	{"!scorelimit", "scorelimit", &g_sl.integer, &Boe_ScoreLimit},
	{"!timelimit", "timelimit", &g_tl.integer, &Boe_TimeLimit},
	{"!respawninterval", "respawninterval", &g_ri.integer, &Boe_RespawnInterval},
	{"!realdamage", "realdamage", &g_damage.integer, &Boe_RealDamage},
	{"!normaldamage", "normaldamage", &g_damage.integer, &Boe_NormalDamage},
	{"!gametyperestart", "gametyperestart", &g_gr.integer, &Boe_GametypeRestart},
	{"!addclan ", "addclan", &g_clan.integer, &Boe_Add_Clan_Member},
	{"!removeclan ", "removeclan", &g_clan.integer, &Boe_Remove_Clan_Member},
	{"!compmode", "compmode", &g_clan.integer, &Boe_CompMode},
	{"!competitionmode", "compmode", &g_clan.integer, &Boe_CompMode},
	{"!banlist", "banlist", &g_ban.integer, &Henk_BanList},
	{"!ban ", "ban", &g_ban.integer, &Boe_Ban_f},
	{"!broadcast ", "broadcast", &g_broadcast.integer, &Boe_Broadcast},
	{"!subnetbanlist", "subnetbanlist", &g_subnetban.integer, &Henk_SubnetBanList},
	{"!eventeams", "eventeams", &g_eventeams.integer, &Henk_EvenTeams},
	{"!clanvsall", "clanvsall", &g_clanvsall.integer, &Henk_CVA},
	{"!swapteams", "swapteams", &g_swapteams.integer, &Henk_SwapTeams},
	{"!lock ", "lock", &g_lock.integer, &Henk_Lock},
	{"!flash ", "flash", &g_flash.integer, &Henk_Flash},
	{"!gametype ", "gametype", &g_mapswitch.integer, &Henk_Gametype},
	{"!unpause", "unpause", &g_mapswitch.integer, &Henk_Unpause},
	{"!pause", "pause", &g_mapswitch.integer, &Henk_Pause},
	{"!unban ", "unban", &g_ban.integer, &Henk_Unban},
	{"!subnetunban ", "subnetunban", &g_subnetban.integer, &Henk_SubnetUnban},
	{"!subnetban ", "subnetban", &g_subnetban.integer, &Boe_subnetBan}
	// Boe!Man 1/22/11: End full synonyms.
};

static int AdminCommandsSize = sizeof( AdminCommands ) / sizeof( AdminCommands[0] );
// End

int AcceptBotCommand(char *cmd, gentity_t *pl);

void trap_SendServerCommand2( int clientNum, const char *text );
void trap_SendServerCommand( int clientNum, const char *text ) {
	
        if( strlen( text ) > 2044 ) { // Henk 15/09/10 -> Scores reach 1022 length easily when there are more then 32 players.
			char a[100]; 
			Boe_crashLog(va("overflow: name:[%s]",g_entities[clientNum].client->pers.netname)); // save this first incase of crash while copying memory
			Q_strncpyz (a, text, 100);
			Boe_crashLog(va("Data:[%s]", a)); // extra information
			trap_SendServerCommand2(clientNum, a);
            return;
        }
		if(clientNum != -1){
			if(&g_entities[clientNum]){
				if(g_entities[clientNum].client){
					if(g_entities[clientNum].client->ps.ping != 999)
						trap_SendServerCommand2(clientNum, text);
				}
			}
		}else
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
		Boe_adminLog (va("%s - EVENTEAMS", adm->client->pers.cleanName)) ;
	}else if (aet == qfalse){
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sE%sv%se%sn%si%sng teams!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7Eventeams.\n\""));
		Boe_adminLog (va("RCON - EVENTEAMS")) ;
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

	if(current_gametype.value == GT_HS && strstr(level.mapname, "col9") && ent->client->sess.team == TEAM_BLUE)
	{
		trap_SendServerCommand ( ent->client - &level.clients[0], "print\"^3[Cross the bridge] ^7Why would you want to kill yourself?\n\"" );
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
	if(current_gametype.value == GT_HS){
		switch ( client->sess.team )
		{
			case TEAM_RED:
				trap_SendServerCommand( -1, va("cp \"@%s" S_COLOR_WHITE "\njoined the %s\n\"", client->pers.netname, server_hiderteamprefix.string) );
				trap_SendServerCommand( -1, va("print \"^3[Info] ^7%s ^7joined the hiders.\n\"", client->pers.cleanName));
				break;

			case TEAM_BLUE:
				trap_SendServerCommand( -1, va("cp \"@%s" S_COLOR_WHITE "\njoined the %s\n\"", client->pers.netname, server_seekerteamprefix.string));
				trap_SendServerCommand( -1, va("print \"^3[Info] ^7%s ^7joined the seekers.\n\"", client->pers.cleanName));
				break;

			case TEAM_SPECTATOR:
				if ( oldTeam != TEAM_SPECTATOR )
				{
					trap_SendServerCommand( -1, va("cp \"@%s" S_COLOR_WHITE "\njoined the spectators\n\"", client->pers.netname));
						trap_SendServerCommand( -1, va("print \"^3[Info] ^7%s ^7joined the spectators.\n\"", client->pers.cleanName));
				}
				break;

			case TEAM_FREE:
				trap_SendServerCommand( -1, va("cp \"@%s" S_COLOR_WHITE " joined the battle.\n\"", client->pers.netname));
					trap_SendServerCommand( -1, va("print \"^3[Info] ^7%s ^7joined the battle.\n\"", client->pers.cleanName));
				break;
		}
	}else{
		switch ( client->sess.team )
		{
			case TEAM_RED:
				trap_SendServerCommand( -1, va("cp \"@%s" S_COLOR_WHITE "\njoined the %s ^7team\n\"", client->pers.netname, server_redteamprefix.string) );
				trap_SendServerCommand( -1, va("print \"^3[Info] ^7%s ^7joined the red team.\n\"", client->pers.cleanName));
				break;

			case TEAM_BLUE:
				trap_SendServerCommand( -1, va("cp \"@%s" S_COLOR_WHITE "\njoined the %s ^7team\n\"", client->pers.netname, server_blueteamprefix.string));
				trap_SendServerCommand( -1, va("print \"^3[Info] ^7%s ^7joined the blue team.\n\"", client->pers.cleanName));
				break;
			
			case TEAM_SPECTATOR:
				if ( oldTeam != TEAM_SPECTATOR )
				{
					trap_SendServerCommand( -1, va("cp \"@%s" S_COLOR_WHITE "\njoined the spectators\n\"", client->pers.netname));
						trap_SendServerCommand( -1, va("print \"^3[Info] ^7%s ^7joined the spectators.\n\"", client->pers.cleanName));
				}
				break;

			case TEAM_FREE:
				trap_SendServerCommand( -1, va("cp \"@%s" S_COLOR_WHITE " joined the battle.\n\"", client->pers.netname));
					trap_SendServerCommand( -1, va("print \"^3[Info] ^7%s ^7joined the battle.\n\"", client->pers.cleanName));
				break;
		}
	}

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

		if ( g_teamForceBalance.integer && forced == qfalse  ) 
		{
			int		counts[TEAM_NUM_TEAMS];

			if(current_gametype.value == GT_HS){
				counts[TEAM_BLUE] = TeamCount1(TEAM_BLUE);
				counts[TEAM_RED] = TeamCount1(TEAM_RED);
				// Henk 19/01/10 -> Team balance hiders/seekers
				if(counts[TEAM_RED] >= 4 && counts[TEAM_RED] <= 8){
					seekers = 2;
				}else if(counts[TEAM_RED] >= 9 && counts[TEAM_RED] <= 13){
					seekers = 3;
				}else if(counts[TEAM_RED] >= 14 && counts[TEAM_RED] <= 18){
					seekers = 4;
				}else if(counts[TEAM_RED] >= 19){
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
											"print \"Red team has too many players.\n\"" );

					// ignore the request
					return; 
				}
				if ( team == TEAM_BLUE && counts[TEAM_BLUE] - counts[TEAM_RED] > 1 ) 
				{
					trap_SendServerCommand( ent->s.number, 
											"print \"Blue team has too many players.\n\"" );

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
			// Boe!Man 12/14/10
			trap_SendServerCommand(clientNum, va("cp \"@%s ^7team is currently %sl%so%sc%sk%se%sd!\n\"", server_redteamprefix.string, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string ));
			trap_SendServerCommand(clientNum, va("print \"^3[Info] ^7Red team is currently locked.\n\"") );
			return;
		}
		///else if ((team == TEAM_BLUE) && level.blueLocked)
		else if ((team == TEAM_BLUE) && level.blueLocked && client->sess.admin < 2) {
			// Boe!Man 12/14/10
			trap_SendServerCommand(clientNum, va("cp \"@%s ^7team is currently %sl%so%sc%sk%se%sd!\n\"", server_blueteamprefix.string, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string ));
			trap_SendServerCommand(clientNum, va("print \"^3[Info] ^7Blue team is currently locked.\n\"") );
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
			ent->client->sess.score = 0;
			ent->client->sess.kills = 0;
			ent->client->sess.deaths = 0;

			CalculateRanks();

			return;
		}
	}	

	// get and distribute relevent paramters
	client->pers.identity = NULL;
	ClientUserinfoChanged( clientNum );

	// Henk 02/02/10 -> Set score to 0 when switching team.
	ent->client->sess.score = 0;
	ent->client->sess.kills = 0;
	ent->client->sess.deaths = 0;

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
	char	type[128];
	char	admin[128];
	char 	star[128];
	
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
	}
	
	// Boe!Man 1/17/10: Different kinds of Talking 'Modes'.

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
		Boe_ClientSound(other, G_SoundIndex("sound/misc/c4/beep.mp3"));
		break;
	case SADM_CHAT:
		strcpy(type, server_scprefix.string);
		Boe_ClientSound(other, G_SoundIndex("sound/misc/c4/beep.mp3"));
		break;
	case ADM_TALK:
		if(ent->client->sess.admin == 2){
		strcpy(type, server_badminprefix.string);
		}else if(ent->client->sess.admin == 3){
		strcpy(type, server_adminprefix.string);
		}else if(ent->client->sess.admin == 4){
		strcpy(type, server_sadminprefix.string);
		}
		Boe_ClientSound(other, G_SoundIndex("sound/misc/c4/beep.mp3"));
		break;
	case CADM_CHAT:
		strcpy(type, server_caprefix.string);
		Boe_ClientSound(other, G_SoundIndex("sound/misc/c4/beep.mp3"));
		break;
	case CLAN_CHAT:
		strcpy(type, server_ccprefix.string);
		Boe_ClientSound(other, G_SoundIndex("sound/misc/c4/beep.mp3"));
		break;
	default:
		break;
	}

	// Boe!Man 1/6/10 - Update 2/8/11: Finally fixing these space issues. This would be the best way for it.
	if(ent->client->sess.admin > 0 && mode >= ADM_TALK && mode <= CADM_CHAT){
		trap_SendServerCommand( other-g_entities, va("%s %d \"%s %s %s%s %s\"", // Boe!Man 1/6/10: Adding prefixes. - Update 1/17/10: Adding Admin Talk/Chat prefixes. - Update 5/8/10: Solved message problem.
								mode == SAY_TEAM ? "tchat" : "chat",
								ent->s.number,
								// Boe!Man 1/6/10: Adding the Admin prefix in front of the chat. - Update 1/17/10.
								star, type, name, message, star)); // Boe!Man 1/17/10: Adding stars.
	}else if(ent->client->sess.admin > 0){
		trap_SendServerCommand( other-g_entities, va("%s %d \"%s%s %s%s\"", // Boe!Man 1/6/10: Adding prefixes. - Update 1/17/10: Adding Admin Talk/Chat prefixes. - Update 5/8/10: Solved message problem.
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
	
	//G_LogPrintf("Getting prefix..\n");
	// Generate the chat prefix
	G_GetChatPrefix ( ent, target, mode, name, sizeof(name) );

	//G_LogPrintf("Copying..\n");
	// Save off the chat text
	Q_strncpyz( text, chatText, sizeof(text) );

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
	char        newp[128] = "";
	// Boe!Man 1/10/10
	int			i;
	int			a = 0;
	// Boe!Man 1/17/10
	int			anim = 0;
	float		knockback = 400.0;
	char team[4];
	// Boe!Man 5/3/10: Temp(?) fix.
	qboolean	acmd = qfalse;
	char		test[128];
	int			ignore = -1;
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
	// Henk loop through my admin command array
	// Boe!Man 1/8/11: Only go through this cycle if the client indeed has admin powers. If not, save on resources.
	if(ent->client->sess.admin > 0){
	for(i=0;i<AdminCommandsSize;i++){
		if(strstr(Q_strlwr(test), Q_strlwr(AdminCommands[i].shortCmd))){
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
	// End
	
		/*else if ((strstr(p, "!333"))) {
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
	}*/
	// Boe!Man 5/2/10: The Referee tokens. I think we're actually the first with this... (:
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
	}
	// Boe!Man 1/24/10: Different kinds of Talk during Gameplay.
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
		}
	}
	else if ((strstr(p, "!ac")) || (strstr(p, "!AC")) || (strstr(p, "!aC")) || (strstr(p, "!Ac"))) {
		if (ent->client->sess.admin){
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
			mode = ADM_CHAT;
			acmd = qtrue;
			strcpy(p, newp);
		}else{
			p = ConcatArgs(1);
			G_Say( ent, NULL, mode, p );
			return;
		}
	}
	else if ((strstr(p, "!sc")) || (strstr(p, "!SC")) || (strstr(p, "!sC")) || (strstr(p, "!Sc"))) {
		if (ent->client->sess.admin){
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
		if (ent->client->sess.admin){
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
			mode = CADM_CHAT;
			acmd = qtrue;
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
	// Boe!Man 4/3/10
	else if (Q_stricmp (cmd, "dev") == 0)
		Boe_dev_f( ent );
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
	else if (Q_stricmp (cmd, "henk") == 0){
		trap_SendServerCommand( clientNum, va("print \"Timer: %i\n\"", ent->client->ps.respawnTimer ) );
	}
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
	int		i, adm, level;
	char	arg1[MAX_STRING_TOKENS];
	char	arg2[MAX_STRING_TOKENS];
	gclient_t	*client;
	client = ent->client;
	trap_Argv( 1, arg1, sizeof( arg1 ) );
	trap_Argv( 2, arg2, sizeof( arg2 ) );
	
	adm = ent->client->sess.admin;
	if(!adm){
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Access denied: You don't have Admin powers!\n\""));
		return;
	}
	if (!Q_stricmp ( arg1, "?" )||!Q_stricmp ( arg1, "" ))
	{
	if (adm > 1){
		trap_SendServerCommand( ent-g_entities, va("print \" \n^3Lvl   Commands         Arguments     Explanation\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" ----------------------------------------------------------\n\""));
	// Boe!Man 9/21/10: We loop the print process and make sure they get in proper order.
	for(level=2;level<=adm;level++){
	if (adm >= g_kick.integer && g_kick.integer != 5 && g_kick.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   k   kick         <id> <reason> ^7[^3Kick a player^7]\n\"", g_kick.integer));
		}
	if (adm >= g_addbadmin.integer && g_addbadmin.integer != 5 && g_addbadmin.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   ab  addbadmin    <id>          ^7[^3Add a Basic Admin^7]\n\"", g_addbadmin.integer));
		}
	if (adm >= g_addadmin.integer && g_addadmin.integer != 5 && g_addadmin.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   aa  addadmin     <id>          ^7[^3Add an Admin^7]\n\"", g_addadmin.integer));
		}
	// Boe!Man 1/4/10: Fix with using Tab in the Admin list.
	if (adm >= g_addsadmin.integer && g_addsadmin.integer != 5 && g_addsadmin.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   as  addsadmin    <id>          ^7[^3Add a Server Admin^7]\n\"", g_addsadmin.integer));
		}
	if (adm >= g_ban.integer && g_ban.integer != 5 && g_ban.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   ba  ban          <id> <reason> ^7[^3Ban a player^7]\n\"", g_ban.integer));
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   uba unban        <ip>          ^7[^3Unban a banned IP^7]\n\"", g_ban.integer));
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   bl  banlist                    ^7[^3Shows the current banlist^7]\n\"", g_ban.integer));
		}
	if (adm >= g_subnetban.integer && g_subnetban.integer != 5 && g_subnetban.integer == level){
		// Boe!Man 1/6/10: Reason added to the Subnetban command.
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   sb  subnetban    <id> <reason> ^7[^3Ban a players' subnet^7]\n\"", g_subnetban.integer));
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   sub subnetunban  <ip>          ^7[^3Unban a banned subnet^7]\n\"", g_subnetban.integer));
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   sbl subnetbanlist              ^7[^3Shows the current subnetbanlist^7]\n\"", g_subnetban.integer));
		}
	if (adm >= g_uppercut.integer && g_uppercut.integer != 5 && g_uppercut.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   uc  uppercut     <id>          ^7[^3Launch a player upwards^7]\n\"", g_uppercut.integer));
		}
	if (adm >= g_twist.integer && g_twist.integer != 5 && g_twist.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   tw  twist        <id>          ^7[^3Twist a player^7]\n\"", g_twist.integer));
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   utw untwist      <id>          ^7[^3Untwist a twisted player^7]\n\"", g_twist.integer));
		}
	if (adm >= g_runover.integer && g_runover.integer != 5 && g_runover.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   ro  runover      <id>          ^7[^3Boost a player backwards^7]\n\"", g_runover.integer));
		}
	if (adm >= g_mapswitch.integer && g_mapswitch.integer != 5 && g_mapswitch.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   mr  maprestart                 ^7[^3Restart the current map^7]\n\"", g_mapswitch.integer));
		}
	if (adm >= g_flash.integer && g_flash.integer != 5 && g_flash.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   fl  flash        <id>          ^7[^3Flash a player^7]\n\"", g_flash.integer));
		}
	if (adm >= g_pop.integer && g_pop.integer != 5 && g_pop.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   p   pop          <id>          ^7[^3Pop a player^7]\n\"", g_pop.integer));
		}
	if (adm >= g_strip.integer && g_strip.integer != 5 && g_strip.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   s  strip         <id>          ^7[^3Remove weapons from a player^7]\n\"", g_strip.integer));
		}
	if (adm >= g_mute.integer && g_mute.integer != 5 && g_mute.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   m   mute         <id>          ^7[^3Mute a player^7]\n\"", g_mute.integer));
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   um  unmute       <id>          ^7[^3Unmute a player^7]\n\"", g_mute.integer));
		}
	if (adm >= g_plant.integer && g_plant.integer != 5 && g_plant.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   pl  plant        <id>          ^7[^3Plant a player in the ground^7]\n\"", g_plant.integer));
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   upl unplant      <id>          ^7[^3Unplant a planted player^7]\n\"", g_plant.integer));
		}
	if (adm >= g_burn.integer && g_burn.integer != 5 && g_burn.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   b   burn         <id>          ^7[^3Burn a player^7]\n\"", g_burn.integer));
		}
	if (adm >= g_eventeams.integer && g_eventeams.integer != 5 && g_eventeams.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   et  eventeams                  ^7[^3Make the teams even^7]\n\"", g_eventeams.integer));
		}
	/*if (adm >= g_333.integer && g_333.integer != 5){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   333 333                        ^7[^3Enable/Disable 333 FPS jumps^7]\n\"", g_333.integer));
		}*/
	if (adm >= g_sl.integer && g_sl.integer != 5 && g_sl.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   sl  scorelimit   <time>        ^7[^3Change the scorelimit^7]\n\"", g_sl.integer));
		}
	if (adm >= g_tl.integer && g_tl.integer != 5 && g_tl.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   tl  timelimit    <time>        ^7[^3Change the timelimit^7]\n\"", g_tl.integer));
		}
	if (adm >= g_nolower.integer && g_nolower.integer != 5 && g_nolower.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   nl  nolower                    ^7[^3Enable/Disable Nolower^7]\n\"", g_nolower.integer));
		}
	if (adm >= g_nades.integer && g_nades.integer != 5 && g_nades.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   nd  normaldamage               ^7[^3Toggle Normal damage^7]\n\"", g_nades.integer));
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   rd  realdamage                 ^7[^3Toggle Real damage^7]\n\"", g_nades.integer));
		}
	if (adm >= g_ri.integer && g_ri.integer != 5 && g_ri.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   ri  ri           <time>        ^7[^3Change the respawn interval^7]\n\"", g_ri.integer));
		}
	if (adm >= g_gr.integer && g_gr.integer != 5 && g_gr.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   gr  gametyperestart            ^7[^3Restart the current gametype^7]\n\"", g_gr.integer));
		}
	if (adm >= g_clanvsall.integer && g_clanvsall.integer != 5 && g_clanvsall.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   cva clanvsall                  ^7[^3Clan versus other players^7]\n\"", g_clanvsall.integer));
		}
	if (adm >= g_swapteams.integer && g_swapteams.integer != 5 && g_swapteams.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   sw  swapteams                  ^7[^3Swap the players from both teams^7]\n\"", g_swapteams.integer));
		}
	if (adm >= g_lock.integer && g_lock.integer != 5 && g_lock.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   l   lock         <team>        ^7[^3Lock a team^7]\n\"", g_lock.integer));
		}
	if (adm >= g_clan.integer && g_clan.integer != 5 && g_clan.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   acl addclan      <id>          ^7[^3Add a clan member^7]\n\"", g_clan.integer));
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   rcl removeclan   <id>          ^7[^3Remove a clan member^7]\n\"", g_clan.integer));
		}
	if (adm >= g_broadcast.integer && g_broadcast.integer != 5 && g_broadcast.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   br  broadcast    <message>     ^7[^3Broadcast a message^7]\n\"", g_broadcast.integer));
		}
	if (adm >= g_forceteam.integer && g_forceteam.integer != 5 && g_forceteam.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   ft  forceteam    <team>        ^7[^3Force a player to join a team^7]\n\"", g_forceteam.integer));
		}
	if (adm >= g_nades.integer && g_nades.integer != 5 && g_nades.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%i^7]   nn  nonades                    ^7[^3Enable or disable nades^7]\n\"", g_forceteam.integer));
		}
	// temp entry
	/*if (adm >= 4 && level == 4){
		trap_SendServerCommand( ent-g_entities, va("print \" [^34^7]       adminspec                  ^7[^3Allows you to spec an enemy player^7]\n\""));
	}*/
	// Boe!Man 9/21/10: End of Loop.
	}
	if(g_enableCustomCommands.integer == 1){
		trap_SendServerCommand( ent-g_entities, va("print \"\n ^7[^3Custom Commands^7] \n\""));
		// Add wrapper for CustomCommands.txt
	}
	trap_SendServerCommand( ent-g_entities, va("print \"    [^32^7] B-Admin          ^7[^33^7] Admin          ^7[^34^7] S-Admin\n\""));
	trap_SendServerCommand( ent-g_entities, va("print \"\n^7Use ^3[Page Up]^7 and ^3[Page Down]^7 keys to scroll.\n\""));
	}
	return;
	}
	
	// Henk loop through my admin command array
	
	if (!Q_stricmp ( arg1, "chat" ) || !Q_stricmp ( arg1, "adminchat" )){
		if(ent->client->sess.mute){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You are currently muted by admin.\n\"") );
			return;
		}
		Cmd_Say_f (ent, ADM_CHAT, qfalse);
		return;
	}
	if (!Q_stricmp ( arg1, "talk" ) || !Q_stricmp ( arg1, "admintalk" )){
		if(ent->client->sess.mute){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You are currently muted by admin.\n\"") );
			return;
		}
		Cmd_Say_f (ent, ADM_TALK, qfalse);
		return;
	}

	if (!Q_stricmp ( arg1, "clanchat" )){
		if(ent->client->sess.mute){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You are currently muted by admin.\n\"") );
			return;
		}else if(!ent->client->sess.clanMember){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You are no clan member\n\"") );
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

	trap_Argv( 0, cmd, sizeof( cmd ) );
	trap_Argv( 1, arg1, sizeof( arg1 ) );
	for(i=0;i<AdminCommandsSize;i++){ // Henk 15/09/10 -> Fixed loop going outside array(causing crashes)
		//Com_Printf("Checking %s with %s\n", arg1, AdminCommands[i].adminCmd);
		if(!Q_stricmp(cmd, AdminCommands[i].adminCmd)){
				AdminCommands[i].Function(1, NULL, qfalse);
				return qtrue;
		}
	}

	if ( Q_stricmp (cmd, "altmap") == 0 )
	{
		trap_Cvar_Set( "g_alternateMap", "1");
		trap_Cvar_Update ( &g_alternateMap );
		trap_SendConsoleCommand( EXEC_APPEND, va("map %s\n", arg1));
		return qtrue;
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
	}*/

	if ( Q_stricmp ( cmd, "cancelvote" ) == 0 )
	{
		Svcmd_CancelVote_f();
		return qtrue;
	}

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

	if (Q_stricmp (cmd, "addip") == 0)
	{
		Svcmd_AddIP_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "removeip") == 0)
	{
		Svcmd_RemoveIP_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "listip") == 0)
	{
		trap_SendConsoleCommand( EXEC_NOW, "g_banIPs\n" );
		return qtrue;
	}

	if (Q_stricmp (cmd, "gametype_restart" ) == 0 )
	{
		trap_Argv( 1, cmd, sizeof( cmd ) );
		G_ResetGametype ( Q_stricmp ( cmd, "full" ) == 0 );
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
			trap_SendServerCommand( -1, va("chat -1 \"server: %s\n\"", ConcatArgs(1) ) );
			return qtrue;
		}

		// everything else will also be printed as a say command
		trap_SendServerCommand( -1, va("chat -1 \"server: %s\n\"", ConcatArgs(0) ) );
		return qtrue;
	}

	return qfalse;
}
