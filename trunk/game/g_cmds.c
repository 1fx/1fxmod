// Copyright (C) 2001-2002 Raven Software.
//
#include "g_local.h"
#include "boe_local.h"

#include "../../ui/menudef.h"

// Henk 04/05/10 -> New command system(Yus this is very pro)
typedef struct 
{
	char	*shortCmd; // short admin command, ex: !uc, !p(with space) -> HENK FIX ME: Need more entries here for uppercase.
	char	*adminCmd; // full adm command for /adm and rcon
	int		*adminLevel; // pointer to cvar value because we can't store a non constant value, so we store a pointer :).
	void	(*Function)(); // store pointer to the given function so we can call it later
} admCmd_t;
extern  admCmd_t AdminCommands[] = 
{
	{"!uc ", "uppercut", &g_uppercut.integer, &Boe_Uppercut},
	{"!p ", "pop", &g_pop.integer, &Boe_pop}
};
// End
int AcceptBotCommand(char *cmd, gentity_t *pl);

void trap_SendServerCommand2( int clientNum, const char *text );
void trap_SendServerCommand( int clientNum, const char *text ) {
        if( strlen( text ) > 1022 ) {
			char a[100]; 
			Q_strncpyz (a, text, 100);
			trap_SendServerCommand2(clientNum, a);
			Boe_crashLog(va("overflow: name:[%s]",g_entities[clientNum].client->pers.netname)) ;
            return;
        }
		trap_SendServerCommand2(clientNum, text);
} 

int GetArgument(void){
	char	arg[16] = "\0"; // increase buffer so we can process more commands
	int		num;
	int i;
	trap_Argv( 1, arg, sizeof( arg ) );
		num = 0;
		for(i=0;i<16;i++){
			if(arg[i] == ' '){
			num = atoi(va("%c%c%c", arg[i+1], arg[i+2], arg[i+3]));
			break;
			}
		}
		if(num == 0){ // Get second argument because they use it from the console
			trap_Argv( 2, arg, sizeof( arg ) );
			num = atoi(arg);
		}
	return num;
}

int FormatDamage(int damage){ // lol fix me au3 script ftw
	if(damage >= 100 && damage < 200){
		damage = 100;
	}else if(damage >= 200 && damage < 300){
		damage = 200;
	}else if(damage >= 300 && damage < 400){
		damage = 300;
	}else if(damage >= 400 && damage < 500){
		damage = 400;
	}else if(damage >= 500 && damage < 600){
		damage = 500;
	}else if(damage >= 600 && damage < 700){
		damage = 600;
	}else if(damage >= 700 && damage < 800){
		damage = 700;
	}else if(damage >= 800 && damage < 900){
		damage = 800;
	}else if(damage >= 900 && damage < 1000){
		damage = 900;
	}else if(damage >= 1000 && damage < 1100){
		damage = 1000;
	}else if(damage >= 1100 && damage < 1200){
		damage = 1100;
	}else if(damage >= 1200 && damage < 1300){
		damage = 1200;
	}else if(damage >= 1300 && damage < 1400){
		damage = 1300;
	}else if(damage >= 1400 && damage < 1500){
		damage = 1400;
	}else if(damage >= 1500 && damage < 1600){
		damage = 1500;
	}else if(damage >= 1600 && damage < 1700){
		damage = 1600;
	}else if(damage >= 1700 && damage < 1800){
		damage = 1700;
	}else if(damage >= 1800 && damage < 1900){
		damage = 1800;
	}else if(damage >= 1900 && damage < 2000){
		damage = 1900;
	}else if(damage >= 2000 && damage < 2100){
	damage = 2000;
}
else if(damage >= 2100 && damage < 2200){
	damage = 2100;
}
else if(damage >= 2200 && damage < 2300){
	damage = 2200;
}
else if(damage >= 2300 && damage < 2400){
	damage = 2300;
}
else if(damage >= 2400 && damage < 2500){
	damage = 2400;
}
else if(damage >= 2500 && damage < 2600){
	damage = 2500;
}
else if(damage >= 2600 && damage < 2700){
	damage = 2600;
}
else if(damage >= 2700 && damage < 2800){
	damage = 2700;
}
else if(damage >= 2800 && damage < 2900){
	damage = 2800;
}
else if(damage >= 2900 && damage < 3000){
	damage = 2900;
}
else if(damage >= 3000 && damage < 3100){
	damage = 3000;
}
else if(damage >= 3100 && damage < 3200){
	damage = 3100;
}
else if(damage >= 3200 && damage < 3300){
	damage = 3200;
}
else if(damage >= 3300 && damage < 3400){
	damage = 3300;
}
else if(damage >= 3400 && damage < 3500){
	damage = 3400;
}
else if(damage >= 3500 && damage < 3600){
	damage = 3500;
}
else if(damage >= 3600 && damage < 3700){
	damage = 3600;
}
else if(damage >= 3700 && damage < 3800){
	damage = 3700;
}
else if(damage >= 3800 && damage < 3900){
	damage = 3800;
}
else if(damage >= 3900 && damage < 4000){
	damage = 3900;
}
else if(damage >= 4000 && damage < 4100){
	damage = 4000;
}
else if(damage >= 4100 && damage < 4200){
	damage = 4100;
}
else if(damage >= 4200 && damage < 4300){
	damage = 4200;
}
else if(damage >= 4300 && damage < 4400){
	damage = 4300;
}
else if(damage >= 4400 && damage < 4500){
	damage = 4400;
}
else if(damage >= 4500 && damage < 4600){
	damage = 4500;
}
else if(damage >= 4600 && damage < 4700){
	damage = 4600;
}
else if(damage >= 4700 && damage < 4800){
	damage = 4700;
}
else if(damage >= 4800 && damage < 4900){
	damage = 4800;
}
else if(damage >= 4900 && damage < 5000){
	damage = 4900;
}
else if(damage >= 5000 && damage < 5100){
	damage = 5000;
}
else if(damage >= 5100 && damage < 5200){
	damage = 5100;
}
else if(damage >= 5200 && damage < 5300){
	damage = 5200;
}
else if(damage >= 5300 && damage < 5400){
	damage = 5300;
}
else if(damage >= 5400 && damage < 5500){
	damage = 5400;
}
else if(damage >= 5500 && damage < 5600){
	damage = 5500;
}
else if(damage >= 5600 && damage < 5700){
	damage = 5600;
}
else if(damage >= 5700 && damage < 5800){
	damage = 5700;
}
else if(damage >= 5800 && damage < 5900){
	damage = 5800;
}
else if(damage >= 5900 && damage < 6000){
	damage = 5900;
}
else if(damage >= 6000 && damage < 6100){
	damage = 6000;
}
else if(damage >= 6100 && damage < 6200){
	damage = 6100;
}
else if(damage >= 6200 && damage < 6300){
	damage = 6200;
}
else if(damage >= 6300 && damage < 6400){
	damage = 6300;
}
else if(damage >= 6400 && damage < 6500){
	damage = 6400;
}
else if(damage >= 6500 && damage < 6600){
	damage = 6500;
}
else if(damage >= 6600 && damage < 6700){
	damage = 6600;
}
else if(damage >= 6700 && damage < 6800){
	damage = 6700;
}
else if(damage >= 6800 && damage < 6900){
	damage = 6800;
}
else if(damage >= 6900 && damage < 7000){
	damage = 6900;
}
else if(damage >= 7000 && damage < 7100){
	damage = 7000;
}
else if(damage >= 7100 && damage < 7200){
	damage = 7100;
}
else if(damage >= 7200 && damage < 7300){
	damage = 7200;
}
else if(damage >= 7300 && damage < 7400){
	damage = 7300;
}
else if(damage >= 7400 && damage < 7500){
	damage = 7400;
}
else if(damage >= 7500 && damage < 7600){
	damage = 7500;
}
else if(damage >= 7600 && damage < 7700){
	damage = 7600;
}
else if(damage >= 7700 && damage < 7800){
	damage = 7700;
}
else if(damage >= 7800 && damage < 7900){
	damage = 7800;
}
else if(damage >= 7900 && damage < 8000){
	damage = 7900;
}
else if(damage >= 8000 && damage < 8100){
	damage = 8000;
}
else if(damage >= 8100 && damage < 8200){
	damage = 8100;
}
else if(damage >= 8200 && damage < 8300){
	damage = 8200;
}
else if(damage >= 8300 && damage < 8400){
	damage = 8300;
}
else if(damage >= 8400 && damage < 8500){
	damage = 8400;
}
else if(damage >= 8500 && damage < 8600){
	damage = 8500;
}
else if(damage >= 8600 && damage < 8700){
	damage = 8600;
}
else if(damage >= 8700 && damage < 8800){
	damage = 8700;
}
else if(damage >= 8800 && damage < 8900){
	damage = 8800;
}
else if(damage >= 8900 && damage < 9000){
	damage = 8900;
}
else if(damage >= 9000 && damage < 9100){
	damage = 9000;
}
else if(damage >= 9100 && damage < 9200){
	damage = 9100;
}
else if(damage >= 9200 && damage < 9300){
	damage = 9200;
}
else if(damage >= 9300 && damage < 9400){
	damage = 9300;
}
else if(damage >= 9400 && damage < 9500){
	damage = 9400;
}
else if(damage >= 9500 && damage < 9600){
	damage = 9500;
}
else if(damage >= 9600 && damage < 9700){
	damage = 9600;
}
else if(damage >= 9700 && damage < 9800){
	damage = 9700;
}
else if(damage >= 9800 && damage < 9900){
	damage = 9800;
}
else if(damage >= 9900 && damage < 10000){
	damage = 9900;
}
else if(damage >= 10000 && damage < 10100){
	damage = 10000;
}
else{
	damage = 0;
}
return damage/10;
}

void Preload(void){
	DB195 = trap_GP_ParseFile("country\\IPs.194", qtrue, qfalse);
	DB212 = trap_GP_ParseFile("country\\IPs.212", qtrue, qfalse);
	DB62 = trap_GP_ParseFile("country\\IPs.62", qtrue, qfalse);
	DB193 = trap_GP_ParseFile("country\\IPs.193", qtrue, qfalse);
	DB213 = trap_GP_ParseFile("country\\IPs.213", qtrue, qfalse);
	DB217 = trap_GP_ParseFile("country\\IPs.217", qtrue, qfalse);
	DB192 = trap_GP_ParseFile("country\\IPs.192", qtrue, qfalse);
	DB216 = trap_GP_ParseFile("country\\IPs.216", qtrue, qfalse);
	DB91 = trap_GP_ParseFile("country\\IPs.91", qtrue, qfalse);
	DB209 = trap_GP_ParseFile("country\\IPs.209", qtrue, qfalse);
	DB80 = trap_GP_ParseFile("country\\IPs.80", qtrue, qfalse);
	DB64 = trap_GP_ParseFile("country\\IPs.64", qtrue, qfalse);
	DB202 = trap_GP_ParseFile("country\\IPs.202", qtrue, qfalse);
	DB66 = trap_GP_ParseFile("country\\IPs.66", qtrue, qfalse);
	DB203 = trap_GP_ParseFile("country\\IPs.203", qtrue, qfalse);
}

void InitSpawn(int choice) // load bsp models before players loads a map(SOF2 clients cannot load a bsp model INGAME)
{
	AddSpawnField("classname", "misc_bsp"); // blocker
	if(choice == 1){
	AddSpawnField("bspmodel",	"instances/Generic/fence01");
	}else{
	AddSpawnField("bspmodel",	"instances/Kamchatka/wall01");
	}
	AddSpawnField("origin",		"-4841 -4396 1252");
	AddSpawnField("angles",		"0 90 0");
	AddSpawnField("model",		"trigger_hurt"); //blocked_trigger
	AddSpawnField("count",		 "1");

	G_SpawnGEntityFromSpawnVars(qfalse);

	level.numSpawnVars = 0;
	level.numSpawnVarChars = 0;
}	

void RemoveFence(void){
	int i;
	for (i = 0; i < MAX_GENTITIES; i++)
	{
		if(g_entities[i].classname != NULL)
		{
			if(!strcmp(g_entities[i].classname, "misc_bsp") )
				G_FreeEntity( &g_entities[i] );
		}
	}
}

void SpawnFence(int choice) // big cage
{
	AddSpawnField("classname", "misc_bsp"); // blocker
	if (choice == 1){
	AddSpawnField("bspmodel",	"instances/Generic/fence01");
	AddSpawnField("origin",		"-346 -309 -275");
	AddSpawnField("angles",		"0 270 0");
	}else if(choice == 2){
	AddSpawnField("bspmodel",	"instances/Generic/fence01");
	AddSpawnField("origin",		"-4073 -710 -275");
	AddSpawnField("angles",		"0 90 0");
	// Boe!Man 5/3/10: The solid walls.
	}else if (choice == 3){
	AddSpawnField("bspmodel",	"instances/Kamchatka/wall01");
	AddSpawnField("origin",		"-1813 -210 -10");
	AddSpawnField("angles",		"0 0 0");
	}else if(choice == 4){
	AddSpawnField("bspmodel",	"instances/Kamchatka/wall01");
	AddSpawnField("origin",		"-2607 -825 -10");
	AddSpawnField("angles",		"0 180 0");
	}
	AddSpawnField("model",		"trigger_hurt"); //blocked_trigger
	AddSpawnField("count",		 "1");

	G_SpawnGEntityFromSpawnVars(qfalse);

	level.numSpawnVars = 0;
	level.numSpawnVarChars = 0;
}

void RPM_ReadyCheck (gentity_t *ent)
{
#ifdef _SOF2_BOTS
	if (ent->r.svFlags & SVF_BOT)
	{
		if(!ent->client->pers.ready)
		{
			ent->client->pers.ready = 1;
		}
		return;
	}
#endif
	if(level.time < ent->client->pers.readyMessageTime + 3000)
	{
		return;
	}

	if(g_doWarmup.integer == 2 && !ent->client->pers.ready)
	{
		trap_SendServerCommand( ent-g_entities, va("cp \"@You are not ready for the match\nPlease type ^1/ready ^7in console\n\""));
		ent->client->pers.readyMessageTime = level.time;
	}
}

void RPM_ReadyUp (gentity_t *ent)
{
	if(ent->client->pers.ready)
	{
		return;
	}

	if(level.warmupTime > -1)
	{
		return;
	}

	ent->client->pers.ready = 1;
	trap_SendServerCommand( ent-g_entities, va("cp \"@ \n\"")); //send a blank message to clear the readyup message
}
/*
================
RPM_ReadyAll
================
*/
void RPM_ReadyAll (void)
{
	int i;
	gentity_t *ent;

	for ( i = 0; i < level.numPlayingClients; i ++ )
	{
		ent = &g_entities[level.sortedClients[i]];
		ent->client->pers.ready = 1;
	}
}

/*
=============
RPM_Pause
=============
*/
void RPM_Pause (gentity_t *adm)
{
	int			i;
	gentity_t	*ent;
//	gentity_t	*tent;

	if ( level.intermissiontime || level.pause || level.intermissionQueued)
	{
		return;
	}

	level.pause = 6;

	for (i=0 ; i< level.maxclients ; i++)
	{
		ent = g_entities + i;
		if (!ent->inuse)
		{
			continue;
		}
		ent->client->ps.pm_type = PM_INTERMISSION;
	}
	// send the current scoring to all clients
	SendScoreboardMessageToAllClients();

	Boe_GlobalSound(G_SoundIndex("sound/misc/events/buzz02.wav"));
/*	tent = G_TempEntity( vec3_origin, EV_GLOBAL_SOUND );
	tent->s.eventParm = G_SoundIndex("sound/misc/events/buzz02.wav");
	tent->r.svFlags = SVF_BROADCAST;
*/
}

/*
============
RPM_Unpause
============
*/
void RPM_Unpause (gentity_t *adm)
{
	int			i;
	gentity_t	*ent;
//	gentity_t	*tent;

	if(!level.pause)
	{
		trap_SendServerCommand( adm-g_entities, va("print \"The game is not currently paused.\n\"") );
		return;
	}

	if(level.time >= level.unpausetime + 1000)
	{
		level.unpausetime = level.time;
		level.pause--;

		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^1Resume ^7game in: %d ^4seconds", level.time + 2000, level.pause ) );

		Boe_GlobalSound(G_SoundIndex("sound/misc/events/buzz02.wav"));
/*		tent = G_TempEntity( vec3_origin, EV_GLOBAL_SOUND );
		tent->s.eventParm = G_SoundIndex("sound/misc/events/buzz02.wav");
		tent->r.svFlags = SVF_BROADCAST;
*/
		if(!level.pause)
		{
			level.unpausetime = 0;
			
			///RxCxW - 08.30.06 - 03:06pm #reset clients (scoreboard) display time
			trap_SetConfigstring( CS_LEVEL_START_TIME, va("%i", level.startTime ) );
			trap_SetConfigstring ( CS_GAMETYPE_TIMER, va("%i", level.gametypeRoundTime) );
			///End  - 08.30.06 - 03:06pm

			if( level.gametypeData->respawnType == RT_INTERVAL)
			{
				level.gametypeRespawnTime[TEAM_RED]  = level.time + g_respawnInterval.integer * 1000;
				level.gametypeRespawnTime[TEAM_BLUE] = level.time + g_respawnInterval.integer * 1000;
				level.gametypeRespawnTime[TEAM_FREE] = level.time + g_respawnInterval.integer * 1000;
			}

			for (i=0 ; i< level.maxclients ; i++)
			{
				ent = g_entities + i;
				if (!ent->inuse)
				{
					continue;
				}
				ent->client->ps.pm_type = PM_NORMAL;
			}
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@GO!", level.time + 2000) );
		}
	}
}

void RPM_Clan_Vs_All(gentity_t *adm)
{
	int		counts[TEAM_NUM_TEAMS] = {0};
	int		i, clanTeam, othersTeam;
	char	team[6];	

	clientSession_t	*sess;
	gentity_t	*ent;

	if(!level.gametypeData->teams){
		if(adm && adm->client){
			trap_SendServerCommand( adm - g_entities, va("print \"^3[Info] ^7Not playing a team game.\n\""));
		}
		else {
			Com_Printf("Not playing a team game.\n");
		}
		return;
	}
	if(adm && adm->client)	trap_Argv( 2, team, sizeof( team ) );
	else trap_Argv( 1, team, sizeof( team ) );
	
	/// which team has the most clan members on it?
	for (i = 0; i < level.numConnectedClients; i++) {
		sess = &g_entities[level.sortedClients[i]].client->sess;
		if(!sess->clanMember) continue;
		if(sess->team != TEAM_RED && sess->team != TEAM_BLUE) continue;
		counts[sess->team]++;
	}

	if (team[0] == 'r' || team[0] == 'R'){
		clanTeam = TEAM_RED;
		othersTeam = TEAM_BLUE;
	}
	else if (team[0] == 'b' || team[0] == 'B'){
		clanTeam = TEAM_BLUE;
		othersTeam = TEAM_RED;
	}
	else {
		if(counts[TEAM_RED] >= counts[TEAM_BLUE]){
			clanTeam = TEAM_RED;
			othersTeam = TEAM_BLUE;
		}
		else {
			clanTeam = TEAM_BLUE;
			othersTeam = TEAM_RED;
		}
	}

	for(i = 0; i < level.numConnectedClients; i++){
		ent = &g_entities[level.sortedClients[i]];
		sess = &ent->client->sess;

		if(sess->team != TEAM_RED && sess->team != TEAM_BLUE) continue;

		if(sess->clanMember){
			if(sess->team != clanTeam){
				ent->client->ps.stats[STAT_WEAPONS] = 0;
				TossClientItems( ent );
				G_StartGhosting( ent );
				sess->team = clanTeam;

			}
			else {
				continue;
			}
		}

		else {
			if(sess->team != othersTeam) {
				
				ent->client->ps.stats[STAT_WEAPONS] = 0;
				TossClientItems( ent );
				G_StartGhosting( ent );

				sess->team = othersTeam;

			}
			else continue;
		}
		
		if (ent->r.svFlags & SVF_BOT){
			char	userinfo[MAX_INFO_STRING];
			trap_GetUserinfo( ent->s.number, userinfo, sizeof( userinfo ) );
			Info_SetValueForKey( userinfo, "team", sess->team == TEAM_RED?"red":"blue");
			trap_SetUserinfo( ent->s.number, userinfo );
		}	

		ent->client->pers.identity = NULL;
		ClientUserinfoChanged( ent->s.number);		
		CalculateRanks();

		G_StopFollowing( ent );
		G_StopGhosting( ent );
		trap_UnlinkEntity ( ent );
		ClientSpawn( ent);

	}
	if (clanTeam == TEAM_BLUE){	
		level.blueLocked = 1;
		level.redLocked = 0;
		}
	else {
		level.redLocked = 1;
		level.blueLocked = 0;
	}

	/// tell everyone what just happened
	trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7Clan vs all", level.time + 5000));
	Boe_GlobalSound(G_SoundIndex("sound/misc/events/tut_lift02.mp3"));
			
	if(adm && adm->client) {
		Boe_adminLog (va("%s - ClanVsAll", adm->client->pers.cleanName)) ;
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Clan vs all by %s\n\"", adm->client->pers.netname));
	} else	{
		Boe_adminLog (va("RCON - ClanVsAll")) ;
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7Clan vs all.\n\""));
	}
	///End  - 02.26.05 - 02:59am
	///CalculateRanks();
}

void RPM_UpdateLoadScreenMessage (void)
{
	char	*ammo, *damage;

	switch (g_weaponModFlags.integer)
	{
		case 1:
			ammo = "ON";
			damage = "OFF";
			break;
		case 2:
			ammo = "OFF";
			damage = "ON";
			break;
		case 3:
			ammo = "ON";
			damage = "ON";
			break;
		default:
			ammo = "OFF";
			damage = "OFF";
	}

	trap_SetConfigstring( CS_MESSAGE, va(" Modified Damage: %s  Modified Ammo: %s\n", damage, ammo));
}

/*
================
RPM_Refresh
================
*/
void RPM_Refresh(gentity_t *ent)
{
	if (!G_IsClientSpectating ( ent->client ) )
	{
		ent->flags &= ~FL_GODMODE;
		ent->client->ps.stats[STAT_HEALTH] = ent->health = -999;
		player_die (ent, ent, ent, 100000, MOD_REFRESH, HL_NONE, vec3_origin );
	}
	
	memset(&ent->client->pers.statinfo, 0, sizeof(ent->client->pers.statinfo));
	ent->client->pers.enterTime = level.time;
	ent->client->ps.persistant[PERS_SCORE] = 0;
	ent->client->sess.score = 0;
	ent->client->sess.deaths = 0;
	ent->client->sess.kills = 0;
	ent->client->sess.teamkillDamage      = 0;
	ent->client->sess.teamkillForgiveTime = 0;
}

void RPM_Obituary ( gentity_t *target, gentity_t *attacker, int mod, attackType_t attack, int hitLocation ) 
{
	int				targ, killer, gender, attackt, weapon;
	char			targetName[64];
	char			killerName[64];
	const char		*targetColor;
	const char		*killerColor;
	char			*message;
	char			*message2;
	char			*message3;
	qboolean		headShot = qfalse;
	qboolean		statOk = qfalse;
	statinfo_t		*atrstat = &attacker->client->pers.statinfo;
	gentity_t *tent;

	killerColor = S_COLOR_WHITE;
	targetColor   = S_COLOR_WHITE;
	message2 = "";
	message3 = "";
	///RxCxW - 01.08.06 - 09:14pm
	///attackt = attacker->client->pers.statinfo.attack;
	///weapon = attacker->client->pers.statinfo.weapon;
	G_LogPrintf("Starting Obituary..\n");
	if(!level.gametypeData->teams || (level.gametypeData->teams && !OnSameTeam ( target, attacker )))
	{
		statOk = qtrue;
	}

	//was the kill hit a HEADSHOT?
	hitLocation = hitLocation & (~HL_DISMEMBERBIT);
	if (hitLocation == HL_HEAD)// && statOk)
	{
		//trap_SendServerCommand( -1, va("print \"^3[Debug]^7Headshot\n\""));
		message3 = "{^3HeaDShoT^7}";
		//Boe_ClientSound(attacker, G_SoundIndex("sound/npc/col8/blakely/niceshot.mp3"));
		//trap_SendServerCommand( attacker->s.number, va("cp \"Headshot!\n\""));//g_headShotMessage.string));
		headShot = qtrue;
		//add to the total headshot count for this player
		atrstat->headShotKills++;
	}

	targ = target->s.number;

	if ( targ < 0 || targ >= MAX_CLIENTS ) 
	{
		G_LogPrintf("Out of range..\n");
		Com_Error( ERR_FATAL, "RPM_Obituary: target out of range" );
	}
	Q_strncpyz( targetName, target->client->pers.netname, sizeof(targetName));
	strcat( targetName, S_COLOR_WHITE );

	//find out who or what killed the client
	if ( attacker->client ) 
	{
		killer = attacker->s.number;
		Q_strncpyz( killerName, attacker->client->pers.netname, sizeof(killerName));
		strcat( killerName, S_COLOR_WHITE );
	}	
	else
	{
		killer = ENTITYNUM_WORLD;
	}
	
	// Play the death sound, water if they drowned
	if ( mod == MOD_WATER )
	{
		Boe_ClientSound(target, G_SoundIndex("sound/pain_death/mullins/drown_dead.mp3"));
	}
	else		//play a random death sound out of 3 possible sounds
	{
		switch(level.time % 3)
		{
		case 0:
			Boe_ClientSound(target, G_SoundIndex("sound/pain_death/male/die01.mp3"));
			break;

		case 1:
			Boe_ClientSound(target, G_SoundIndex("sound/pain_death/male/die04.mp3"));
			break;

		case 2:
			Boe_ClientSound(target, G_SoundIndex("sound/pain_death/male/die05.mp3"));
			break;

		default:
			Boe_ClientSound(target, G_SoundIndex("sound/pain_death/male/die01.mp3"));
			break;
		}
	}
	// Play the frag sound, and make sure its not played more than every 250ms
	if ( attacker->client)// && level.time - attacker->client->lastKillTime > 250 )
	{
		//If the attacker killed themselves play the selffrag sound
		if ( killer == targ )
		{
			Boe_ClientSound(target, G_SoundIndex("sound/self_frag.mp3"));
		}
		//Or if they killed a teammate
		/*else if(level.gametypeData->teams && OnSameTeam ( target, attacker ))
		{
			Boe_ClientSound(attacker, G_SoundIndex("sound/self_frag.mp3"));
			if(g_showTkMessage.integer)
			{
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,%s ^1killed ^7a ^3teamate!", level.time + 3000, killerName ) );
			}
		}*/
		else if(headShot)
		{
			//if they use rpm client-side the client will 
			//handle the sound etc...
			if(attacker->client->sess.rpmClient)
			{
				trap_SendServerCommand( attacker->s.number, va("headshot \"Headshot\n\""));
				Boe_ClientSound(attacker, G_SoundIndex("sound/npc/col8/blakely/niceshot.mp3"));
			}
			//if not we'll send them the sound etc..
			else// if(g_allowDeathMessages.integer)
			{
				trap_SendServerCommand( attacker->s.number, va("cp \"Headshot\n\""));//g_headShotMessage.string));
				Boe_ClientSound(attacker, G_SoundIndex("sound/npc/col8/blakely/niceshot.mp3"));
			}
			
			//if(g_allowDeathMessages.integer)
			//{
				//if we can show kills we'll display the 
				//heashot message with the "you killed" message
				if ( level.gametypeData->showKills )
				{	
					message2 = va("HEADSHOT\n");//g_headShotMessage.string );
				}

				//We'll tack this on to the end of the kill broadcast to all
				message3 = "{^3HeaDShoT^7}";
			//}
			
		}
		//if not headshot, suicide or tk just play the normal sound
		else
		{	
			Boe_ClientSound(attacker, G_SoundIndex("sound/frag.mp3"));
		}
		
		//set the time here now 
		attacker->client->lastKillTime = level.time;
	}	
	//set the teamcolor of the killed client
	switch ( target->client->sess.team )
	{
		case TEAM_RED:
			targetColor = S_COLOR_RED;
			break;

		case TEAM_BLUE:
			targetColor = S_COLOR_BLUE;
			break;
	}

	gender = GENDER_MALE;
	// HENKIE CRASH FIX
	//is the client female?, check the name of the model they're using
	if ( strstr ( target->client->pers.identity->mCharacter->mModel, "female" ) )
	{
		gender = GENDER_FEMALE;
	}
	switch( mod ) 
	{
		case MOD_SUICIDE:
			message = "suicides";
			break;
		case MOD_FALLING:
			if ( gender == GENDER_FEMALE )
				message = "fell to her death";
			else
				message = "fell to his death";
			break;
		case MOD_CRUSH:
			message = "was squished";
			break;
		case MOD_WATER:
			message = "sank like a rock";
			break;
		case MOD_TARGET_LASER:
			message = "saw the light";
			break;
		case MOD_TRIGGER_HURT:
			message = "was in the wrong place";
			break;
		case MOD_TEAMCHANGE:
			return;
		case MOD_CAR:
			message = "was killed in a terrible car accident";
			break;
		case MOD_POP:
			return;
//RxCxW - 1.13.2005	- Dugup (Unplant) #MOD
		case MOD_DUGUP:
			message = "Looks like someone Dug too deep!";
			break;
		case MOD_BURN:
			message = "was BURNT to a Crisp!";
			break;
//End
		default:
			message = NULL;
			break;
	}
	// Attacker killed themselves.  Ridicule them for it.
	if (killer == targ) 
	{
		switch (mod) 
		{
			case MOD_MM1_GRENADE_LAUNCHER:    
			case MOD_RPG7_LAUNCHER:           
			case MOD_M67_GRENADE:            
			case MOD_M84_GRENADE:
			case MOD_F1_GRENADE:
			case MOD_L2A2_GRENADE:
			case MOD_MDN11_GRENADE:
			case MOD_SMOHG92_GRENADE:
			case MOD_ANM14_GRENADE:
			case MOD_M15_GRENADE:
				if ( gender == GENDER_FEMALE )
					message = "blew herself up";
				else if ( gender == GENDER_NEUTER )
					message = "blew itself up";
				else
					message = "blew himself up";
				break;

			case MOD_REFRESH:
				message = "Refreshed";
				break;

			default:
				if ( gender == GENDER_FEMALE )
					message = "killed herself";
				else if ( gender == GENDER_NEUTER )
					message = "killed itself";
				else
					message = "killed himself";
				break;
		}
	}
	if (message) 
	{
		trap_SendServerCommand( -1, va("print \"%s%s %s.\n\"", targetColor, targetName, message));
		return;
	}
	// check for kill messages
	if ( level.gametypeData->showKills )
	{
		if ( attacker && attacker->client ) 
		{
			//if not a team game display the kill and the rank
			if ( !level.gametypeData->teams ) 
			{
				trap_SendServerCommand( attacker->s.number, va("cp \"%sYou killed %s%s\n place with %i\n\"", // %s after \n
				message2,
				targetColor,
				targetName, 
				//G_PlaceString( attacker->client->ps.persistant[PERS_RANK] + 1 ),
				attacker->client->ps.persistant[PERS_SCORE] ));
			} 
			else //if team just display the kill
			{
				trap_SendServerCommand( attacker->s.number, va("cp \"%sYou killed %s%s\n\"", message2, targetColor, targetName ));
			}
		}
	}
	// check for double client messages
	message2 = "";

	///RxCxW - 01.08.06 - 09:22pm - Used if/else if instead of switch which gave errors in dll game
		//switch ( attacker->client->sess.team )
		//{
		//	case TEAM_RED:
		//		killerColor = S_COLOR_RED;
		//		break;

		//	case TEAM_BLUE:
		//		killerColor = S_COLOR_BLUE;
		//		break;
		//}
	///End  - 01.08.06 - 09:31pm
			
	if ( killer != ENTITYNUM_WORLD ) 
	{
		///RxCxW - 01.08.06 - 09:31pm
		switch ( attacker->client->sess.team )
		{
			case TEAM_RED:
				killerColor = S_COLOR_RED;
				break;

			case TEAM_BLUE:
				killerColor = S_COLOR_BLUE;
				break;
		}
		///End  - 01.08.06 - 09:31pm

		switch (mod) 
		{
			case MOD_KNIFE:
				message = "was sliced by";
				if(statOk)
				{
					//atrstat->knifeKills++;
				}
				break;

			case MOD_USAS_12_SHOTGUN:
			case MOD_M590_SHOTGUN:
				if ( attack == ATTACK_ALTERNATE )
				{
					message = "was bludgeoned by";
					message2 = va("'s %s", weaponParseInfo[mod].mName );
				}
				else
				{
					message = "was pumped full of lead by";
					message2 = va("'s %s", weaponParseInfo[mod].mName );
				}
				break;

			case MOD_M1911A1_PISTOL:
			case MOD_USSOCOM_PISTOL: 
				if ( attack == ATTACK_ALTERNATE )
				{
					message = "was pistol whipped by";
					message2 = va("'s %s", weaponParseInfo[mod].mName );
				}
				else
				{
					message = "was shot by";
					message2 = va("'s %s", weaponParseInfo[mod].mName );
				}
				break;

			case MOD_AK74_ASSAULT_RIFLE:
				if ( attack == ATTACK_ALTERNATE )
				{
					message = "was stabbed by";
					message2 = va("'s %s", weaponParseInfo[mod].mName );
				}
				else
				{
					message = "was shot by";
					message2 = va("'s %s", weaponParseInfo[mod].mName );
				}
				break;

			case MOD_M4_ASSAULT_RIFLE:
				if ( attack == ATTACK_ALTERNATE )
				{
					message = "was detonated by";
					message2 = "'s M203";

					if(statOk)
					{
						//atrstat->explosiveKills++;
					}
				}
				else
				{
					message = "was shot by";
					message2 = va("'s %s", weaponParseInfo[mod].mName );
				}
				break;

			case MOD_M60_MACHINEGUN:
			case MOD_MICRO_UZI_SUBMACHINEGUN:
			case MOD_M3A1_SUBMACHINEGUN:
				message = "was shot by";
				message2 = va("'s %s", weaponParseInfo[mod].mName );
				break;

			case MOD_MSG90A1_SNIPER_RIFLE:    
				message = "was sniped by";
				message2 = va("'s %s", weaponParseInfo[mod].mName );
				break;

			case MOD_MM1_GRENADE_LAUNCHER:    
			case MOD_RPG7_LAUNCHER:           
			case MOD_M67_GRENADE:            
			case MOD_M84_GRENADE:
			case MOD_F1_GRENADE:
			case MOD_L2A2_GRENADE:
			case MOD_MDN11_GRENADE:
			case MOD_SMOHG92_GRENADE:
			case MOD_ANM14_GRENADE:
			case MOD_M15_GRENADE:
				message = "was detonated by";
				message2 = va("'s %s", weaponParseInfo[mod].mName );
				if(statOk)
				{
					if(mod == MOD_ANM14_GRENADE)
					{
						atrstat->hitcount++;
						atrstat->accuracy = (float)atrstat->hitcount / (float)atrstat->shotcount * 100;
						//atrstat->weapon_hits[ATTACK_NORMAL][mod]++;
					}

					//atrstat->explosiveKills++;
				}
				break;

			case MOD_TELEFRAG:
				message = "tried to invade";
				message2 = "'s personal space";
				break;

			default:
				message = "was killed by";
				break;
		}

		if (message) {
			trap_SendServerCommand( -1, va("print \"%s%s %s %s%s%s %s\n\"", targetColor, targetName, message, killerColor, killerName, message2, message3));
			return;
		}
	}

	// we don't know what it was
	G_LogPrintf("Done with RPM_Obituary..\n");
	trap_SendServerCommand( -1, va("print \"%s%s died.\n\"", targetColor, targetName ));
}

// Henk 07/04/10 -> Copied from RPM to make scoreboard from RPM client working
/*
=================	
RPM_UpdateTMI

//Used to only send teammate info hence "TMI" but I have decided to
//send all player infos so specs can see health etc.. of both teams
//and its also kinda cool in deathmatches to see the health of the enemy
=================
*/
void RPM_UpdateTMI(void)
{
	int			i, j;
	char		entry[1024];
	char		infoString[2048];
	int			infoStringLength;
	gclient_t	*cl;
	int			numAdded, location;
	gentity_t	*bestLoc;
	int			adm = 0;
	int			thirdperson;
	char		*s;
	char		userinfo[MAX_INFO_STRING];
	int			damage;
	char		*string;
/*	if (!level.gametypeData->teams)
	{	// only bother if a league game with teams
		return;
	}
*/
	// if we are waiting for the level to restart, do nothing, even if forced
	if (level.restarted){
		return;
	}

	if(level.pause) // Henk 06/04/10 -> No pause functionality yet
		return;

	if (level.time - level.lastTMIupdate < 1000) // Henk 06/04/10 -> Increase to reduce lagg
	{
		return;
	}

	level.lastTMIupdate = level.time;

	entry[0] = 0;
	infoString[0] = 0;
	infoStringLength = 0;
	numAdded = 0;
	
	for (i = 0; i < level.numConnectedClients; i++)
	{
		cl = &level.clients[level.sortedClients[i]];
		if (G_IsClientSpectating(cl))// || G_IsClientDead (cl)) 
		{
			continue;
		}

		bestLoc = Team_GetLocation(&g_entities[level.sortedClients[i]]);

		location = 0;
		if (bestLoc)
		{
			location = bestLoc->health;
		}
		// Henk 06/04/10 -> Check what view person is in
		trap_GetUserinfo( g_entities[level.sortedClients[i]].s.number, userinfo, sizeof( userinfo ) );
		if(g_entities[level.sortedClients[i]].client->sess.rpmClient >= 0.7){
		s = Info_ValueForKey( userinfo, "cg_thirdperson" );
		thirdperson = atoi(s);
		if(thirdperson >= 1){
			thirdperson = 1;
		}else{
			thirdperson = 0;
		}
		}else{
			thirdperson = 2;
		}
			adm = cl->sess.admin;
			damage = cl->pers.statinfo.damageDone;
			if(damage < 100){
				string = va("%i", cl->ps.weapon);
			}else{
				damage = FormatDamage(damage);
				if(cl->ps.weapon >= 10){ // fix for large weapon string = damage *10 on scoreboard
					damage = damage/10;
				}
				string = va("%i%i", damage, cl->ps.weapon);	
			}
			
			Com_sprintf (entry, sizeof(entry),
				" %i %i %i %i %i %i %s %i %i",
				level.sortedClients[i],
				cl->ps.stats[STAT_HEALTH],
				cl->ps.stats[STAT_ARMOR],
				location,
				thirdperson, // 1 = third | 0 = first | 2 = n/a, aka no client
				adm,
				string,
				//cl->ps.weapon, //cl->pers.statinfo.damageDone
				cl->sess.mute,		// Confirmed
				cl->sess.clanMember	// Confirmed
				);

		j = strlen(entry);

		if (infoStringLength + j > 2046)
		{
			break;
		}
		strcpy (infoString + infoStringLength, entry);
		infoStringLength += j;
		numAdded++;
	}
	for (i = 0 ; i < level.numConnectedClients; i++)
	{
		cl = &level.clients[level.sortedClients[i]];
		

//RxCxW - 1.20.2005 - #scoreboard #Version compatiblity
		if(cl->sess.rpmClient > 0.6)	
			trap_SendServerCommand(level.sortedClients[i], va("tmi %i%s", numAdded, infoString));
	}
}

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
		Boe_adminLog (va("%s - EVENTEAMS", adm->client->pers.cleanName)) ;
	} else	{
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7Eventeams.\n\""));
		Boe_adminLog (va("RCON - EVENTEAMS")) ;
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
			ping = ping*0.4;
			}
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
			trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE "\njoined the %s ^7team\n\"", client->pers.netname, server_redteamprefix.string) );
			break;

		case TEAM_BLUE:
			trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE "\njoined the %s ^7team\n\"", client->pers.netname, server_blueteamprefix.string));
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

	if (level.gametypeData->teams)
	{
		///RxCxW - 03.05.05 - 07:19am
		///if ((team == TEAM_RED) && level.redLocked)
		if ((team == TEAM_RED) && level.redLocked && client->sess.admin < 2) {
			trap_SendServerCommand(clientNum, "cp \"^1Red ^7team has been ^3locked.\n\"" );
			return;
		}
		///else if ((team == TEAM_BLUE) && level.blueLocked)
		else if ((team == TEAM_BLUE) && level.blueLocked && client->sess.admin < 2) {
			trap_SendServerCommand(clientNum, "cp \"^4Blue ^7team has been ^3locked.\n\"" );
			return;
		}
	}

	if(oldTeam == TEAM_SPECTATOR)
	{
		//Ryan march 28 2004 3:24pm
		//keep track of how much time the player spent spectating
		//we'll subtract this later from the players time for awards
		//client->sess.totalSpectatorTime += level.time - client->sess.spectatorTime;
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
		BroadcastTeamChange( client, oldTeam );
	}
	//Ryan
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
		ent->client->sess.team != TEAM_SPECTATOR && !ent->client->adminspec)
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
		if ( level.gametypeData->teams && !g_followEnemy.integer && ent->client->sess.team != TEAM_SPECTATOR && !ent->client->adminspec )
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
	if ( deadclient != -1 && (g_forceFollow.integer || g_compMode.integer))
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
	char	type[10];
	char	admin[36];
	char 	star[10];
	
	G_LogPrintf("Starting with chat\n");
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
	else if (mode == CADM_CHAT && other->client->sess.admin < 2 )
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
	/*
	else if ( ent->client->sess.team == TEAM_RED )
	{
		type = "^7[^1h^7]";
	}
	else if ( ent->client->sess.team == TEAM_BLUE )
	{
		type = "^7[^ys^7]";
	}
	*/
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
		Boe_ClientSound(other, G_SoundIndex("sound/misc/menus/invalid.wav"));
		break;
	case ADM_TALK:
		if(ent->client->sess.admin == 2){
		strcpy(type, server_badminprefix.string);
		}else if(ent->client->sess.admin == 3){
		strcpy(type, server_adminprefix.string);
		}else if(ent->client->sess.admin == 4){
		strcpy(type, server_sadminprefix.string);
		}
		Boe_ClientSound(ent, G_SoundIndex("sound/misc/menus/invalid.wav"));
		break;
	case CADM_CHAT:
		strcpy(type, server_caprefix.string);
		Boe_ClientSound(other, G_SoundIndex("sound/misc/menus/invalid.wav"));
		break;
	case CLAN_CHAT:
		strcpy(type, server_ccprefix.string);
		Boe_ClientSound(other, G_SoundIndex("sound/misc/menus/invalid.wav"));
		break;
	default:
		break;
	}

	// Boe!Man 1/6/10
	trap_SendServerCommand( other-g_entities, va("%s %d \"%s %s%s%s%s %s\"", // Boe!Man 1/6/10: Adding prefixes. - Update 1/17/10: Adding Admin Talk/Chat prefixes.
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
	if((strstr(chatText, "@fp") || strstr(chatText, "@FP") || strstr(chatText, "@Fp") || strstr(chatText, "@fP")) && strstr(chatText, " @")){
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Sound blocked to prevent spam.\n\"") );
		return;
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
	
	G_LogPrintf("Getting prefix..\n");
	// Generate the chat prefix
	G_GetChatPrefix ( ent, target, mode, name, sizeof(name) );

	G_LogPrintf("Copying..\n");
	// Save off the chat text
	Q_strncpyz( text, chatText, sizeof(text) );

	if ( target && target->inuse) 
	{
		G_LogPrintf("Too target?..\n");
		G_SayTo( ent, target, mode, name, text );
		return;
	}
	G_LogPrintf("Printing..\n");
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
	G_LogPrintf("Done..\n");
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
// This function is called alot, better optimize this -.-''
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
	memset(&numb, 0, sizeof(numb)); // clean buffer
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
	int			id, uclevel;
	gentity_t	*targ;
	char		id2[64];
	int			i;
	int			a = 0;
	// Boe!Man 1/17/10
	vec3_t		lookdown;
	vec3_t		fireAngs;
	int			anim = 0;
	float		knockback = 400.0;
	vec3_t		dir;
	// Boe!Man 1/30/10
	gentity_t	*tent;
	gclient_t	*client;
	int			idle;
	// Boe!Man 3/20/10
	int			it, nadeDir, weapon;
	float		x, y;
	gentity_t	*missile;
	char team[4];
	// Boe!Man 5/3/10: Temp(?) fix.
	qboolean	acmd = qfalse;
	int test1, test2, test3;

	if ( trap_Argc () < 2 && !arg0 )
		return;
	if (arg0)
		p = ConcatArgs( 0 );
	else if(mode >= ADM_TALK && mode <= CADM_CHAT)
		p = ConcatArgs( 2 );
	else
		p = ConcatArgs( 1 );

	// Boe!Man 1/10/10: Chat Admin command tokens.
	if ((strstr(p, "!k ")) || (strstr(p, "!kick "))) {	
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
	else if ((strstr(p, "!ab ")) || (strstr(p, "!addbadmin "))) {
		if (ent->client->sess.admin >= g_addbadmin.integer){
			id = CheckAdmin(ent, p, qfalse);
			targ = g_entities+id;
			if(targ->client->sess.admin >= 1){
				id = -1;
			}if(id != -1){;
			g_entities[id].client->sess.admin = 2;
			Q_strncpyz (id2, targ->client->pers.boe_id, 64);
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
	else if ((strstr(p, "!aa ")) || (strstr(p, "!addadmin "))) {
		if (ent->client->sess.admin >= g_addadmin.integer){
			id = CheckAdmin(ent, p, qfalse);
			targ = g_entities+id;
			if(targ->client->sess.admin >= 1){
				id = -1;
			}if(id != -1){;
			// Boe!Man 1/21/10: In the session the client also has to be an Admin.
			g_entities[id].client->sess.admin = 3;
			Q_strncpyz (id2, targ->client->pers.boe_id, 64);
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
	else if ((strstr(p, "!as ")) || (strstr(p, "!addsadmin "))) {
		if (ent->client->sess.admin >= g_addsadmin.integer){
			id = CheckAdmin(ent, p, qfalse);
			targ = g_entities+id;
			if(targ->client->sess.admin >= 1){
				id = -1;
			}if(id != -1){;
			// Boe!Man 1/17/10: Fix for getting B-Admin when adding S-Admin.
			g_entities[id].client->sess.admin = 4;
			Q_strncpyz (id2, targ->client->pers.boe_id, 64);
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
	else if ((strstr(p, "!tw ")) || (strstr(p, "!twist "))){
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
	else if ((strstr(p, "!utw ")) || (strstr(p, "!untwist "))){
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
	else if ((strstr(p, "!pl ")) || (strstr(p, "!plant "))){
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
	else if ((strstr(p, "!upl ")) || (strstr(p, "!unplant "))){
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
	else if ((strstr(p, "!u ")) || (strstr(p, "!uc ")) || (strstr(p, "!uppercut "))) {
		if (ent->client->sess.admin >= g_uppercut.integer){
			Boe_Uppercut(1, ent, qtrue);
		}
	}
	else if ((strstr(p, "!ro ")) || (strstr(p, "!runover "))){
		if (ent->client->sess.admin >= g_runover.integer){
			Boe_Runover(1, ent, qtrue);
		}
	}
	else if ((strstr(p, "!r ")) || (strstr(p, "!rs ")) || (strstr(p, "!respawn "))) {
		if (ent->client->sess.admin >= g_respawn.integer){
			Boe_Respawn(1, ent, qtrue);
		}
	}
	else if ((strstr(p, "!mr")) || (strstr(p, "!maprestart"))) { // Boe!Man 1/21/10: No need for a space when we're restarting the map.
		if (ent->client->sess.admin >= g_maprestart.integer){
			trap_SendConsoleCommand( EXEC_APPEND, va("map_restart 5\n"));
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%sM%sa%sp %sr%se%sstart!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
			Boe_GlobalSound (G_SoundIndex("sound/misc/menus/invalid.wav"));
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Map restarted by %s.\n\"", ent->client->pers.netname));
			Boe_adminLog (va("%s - MAP RESTART", ent->client->pers.cleanName)) ;
		}
		else if ( ent->client->sess.admin < g_maprestart.integer){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
			id = -1;
		}
		G_Say( ent, NULL, mode, p );
		return;
	}
	else if ((strstr(p, "!p ")) || (strstr(p, "!pop "))) {
		if (ent->client->sess.admin >= g_pop.integer){
			Boe_pop(1, ent, qtrue);
		}
	}
	else if ((strstr(p, "!b ")) || (strstr(p, "!burn "))) {
		if (ent->client->sess.admin >= g_burn.integer){
			Boe_Burn(1, ent, qtrue);
		}
	}
	else if ((strstr(p, "!m ")) || (strstr(p, "!mute "))) {
		if (ent->client->sess.admin >= g_mute.integer){
			Boe_Mute(1, ent, qtrue, qtrue);
		}
	}
	else if ((strstr(p, "!um ")) || (strstr(p, "!unmute "))) {
		if (ent->client->sess.admin >= g_mute.integer){
			Boe_Mute(1, ent, qfalse, qtrue);
		}
	}
	else if ((strstr(p, "!s ")) || (strstr(p, "!st "))) {
		if (ent->client->sess.admin >= g_strip.integer){
		Boe_Strip(1, ent, qtrue);
		}
	}
	else if ((strstr(p, "!ra ")) || (strstr(p, "!removeadmin "))) {
		if (ent->client->sess.admin >= g_removeadmin.integer){
			Boe_Remove_Admin_f(1, ent, qtrue);
		}
	}
	else if ((strstr(p, "!et")) || (strstr(p, "!eventeams"))) {
		if (ent->client->sess.admin >= g_eventeams.integer){
			EvenTeams(ent);
		}
		G_Say( ent, NULL, mode, p);
		return;
	}
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
	else if ((strstr(p, "!ft ")) || (strstr(p, "!forceteam "))){
		Adm_ForceTeam(ent, qtrue);
	}
	else if ((strstr(p, "!nl")) || (strstr(p, "!nolower"))) {
		if (ent->client->sess.admin >= g_nolower.integer){
			if(level.nolower1 == qtrue){
				level.nolower1 = qfalse;
				trap_Cvar_Set("g_disablelower", "0");
				if (strstr(level.mapname, "mp_kam2")){
				RemoveFence();
				}
				trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7No lower has been disabled by %s.\n\"", ent->client->pers.netname));
				Boe_adminLog (va("%s - NOLOWER DISABLED", ent->client->pers.cleanName)) ;
			}else{
				level.nolower1 = qtrue;
				trap_Cvar_Set("g_disablelower", "1");
				if (strstr(level.mapname, "mp_kam2")){
				SpawnFence(1);
				SpawnFence(2);
				SpawnFence(3);
				SpawnFence(4);
				}
				trap_SendServerCommand(-1, va("print \"^3[Admin Action] ^7No lower has been enabled by %s.\n\"", ent->client->pers.netname));
				Boe_adminLog (va("%s - NOLOWER ENABLED", ent->client->pers.cleanName)) ;
			}
		}
		trap_Cvar_Update(&g_disablelower);
		else if (ent->client->sess.admin < g_nolower.integer){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
		}
		G_Say( ent, NULL, mode, p);
		return;
	}
	else if(strstr(p, "!nn")){
		if(ent->client->sess.admin >= g_nades.integer){
			if(g_disablenades.integer == 1){
				g_disablenades.integer = 0;
				trap_Cvar_Set("g_disablenades", "0");
				trap_Cvar_Set("g_availableweapons", "200200002200000000200");
				BG_SetAvailableOutfitting("200200002200000000200");
				for(i=0;i<=level.numConnectedClients;i++){
				level.clients[level.sortedClients[i]].noOutfittingChange = qfalse;
				G_UpdateOutfitting(g_entities[level.sortedClients[i]].s.number);
				level.clients[level.sortedClients[i]].ps.ammo[weaponData[WP_SMOHG92_GRENADE].attack[ATTACK_NORMAL].ammoIndex]=1;
				level.clients[level.sortedClients[i]].ps.stats[STAT_WEAPONS] |= ( 1 << WP_SMOHG92_GRENADE );
				level.clients[level.sortedClients[i]].ps.clip[ATTACK_NORMAL][WP_SMOHG92_GRENADE]=1;
				}
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sN%sa%sd%se%ss %senabled!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Admin Action] ^7Nades enabled by %s.\n\"", ent->client->pers.netname));
				Boe_adminLog (va("%s - NADES ENABLED", ent->client->pers.cleanName)) ;
			}else{
				g_disablenades.integer = 1;
				trap_Cvar_Set("g_disablenades", "1");
				// change g_available
				trap_Cvar_Set("g_availableweapons", "200200002200000000000");
				BG_SetAvailableOutfitting("200200002200000000000");
				for(i=0;i<=level.numConnectedClients;i++){
				level.clients[level.sortedClients[i]].noOutfittingChange = qfalse;
				G_UpdateOutfitting(g_entities[level.sortedClients[i]].s.number);
				}
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sN%sa%sd%se%ss %sdisabled!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Admin Action] ^7Nades disabled by %s.\n\"", ent->client->pers.netname));
				Boe_adminLog (va("%s - NADES DISABLED", ent->client->pers.cleanName)) ;
			}
		}else if (ent->client->sess.admin < g_nades.integer){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
		}
		G_Say( ent, NULL, mode, p);
		return;
	}
	else if(strstr(p, "!sl ")){
		if (ent->client->sess.admin >= g_sl.integer){
			char *numb;
			int number;
			if(strlen(p) >= 5){
				//numb = va("%c%c%c", p[4], p[5], p[6]);
				//number = atoi(numb);
				number = GetArgument();
				trap_SendConsoleCommand( EXEC_APPEND, va("scorelimit %i\n", number));
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Admin Action] ^7Scorelimit changed to %i by %s.\n\"", number, ent->client->pers.netname));
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sS%sc%so%sr%se%slimit %i!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, number));
				Boe_adminLog (va("%s - SCORELIMIT %i", ent->client->pers.cleanName, number)) ;
			}
		}else if (ent->client->sess.admin < g_sl.integer){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
		}
		G_Say( ent, NULL, mode, p);
		return;
	}
	else if(strstr(p, "!tl ")){
		if (ent->client->sess.admin >= g_tl.integer){
			char *numb;
			int number;
			if(strlen(p) >= 5){
				//numb = va("%c%c%c", p[4], p[5], p[6]);
				//number = atoi(numb);
				number = GetArgument();
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sT%si%sm%se%sl%simit %i!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, number));
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Admin Action] ^7Timelimit changed to %i by %s.\n\"", number, ent->client->pers.netname));
				trap_SendConsoleCommand( EXEC_APPEND, va("timelimit %i\n", number));
				Boe_adminLog (va("%s - TIMELIMIT %i", ent->client->pers.cleanName, number)) ;
			}
		}else if (ent->client->sess.admin < g_tl.integer){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
		}
		G_Say( ent, NULL, mode, p);
		return;
	}
	else if(strstr(p, "!ri ")){
		if (ent->client->sess.admin >= g_ri.integer){
			char *numb;
			int number;
			if(strlen(p) >= 5){
				//numb = va("%c%c%c", p[4], p[5], p[6]);
				//number = atoi(numb);
				number = GetArgument();
				trap_SendConsoleCommand( EXEC_APPEND, va("g_respawninterval %i\n", number));
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sR%se%ss%sp%sa%swn interval %i!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, number));
				Boe_adminLog (va("%s - RESPAWN INTERVAL %i", ent->client->pers.cleanName, number)) ;
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Admin Action] ^7Respawn interval changed to %i by %s.\n\"", number, ent->client->pers.netname));
			}
		}else if (ent->client->sess.admin < g_ri.integer){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
		}
		G_Say( ent, NULL, mode, p);
		return;
	}
	else if(strstr(p, "!rd")){
		if (ent->client->sess.admin >= g_damage.integer){
			g_instagib.integer = 1;
			BG_InitWeaponStats();
			for(i=0;i<=level.numConnectedClients;i++){
			level.clients[level.sortedClients[i]].noOutfittingChange = qfalse;
			G_UpdateOutfitting(g_entities[level.sortedClients[i]].s.number);
			}
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sR%se%sa%sl %sd%samage!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Admin Action] ^7Real damage by %s.\n\"", ent->client->pers.netname));
			
		}else if (ent->client->sess.admin < g_damage.integer){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
		}
		G_Say( ent, NULL, mode, p);
		return;
	}
	else if(strstr(p, "!nd")){
		if (ent->client->sess.admin >= g_damage.integer){
			g_instagib.integer = 0;
			BG_InitWeaponStats();
			for(i=0;i<=level.numConnectedClients;i++){
			level.clients[level.sortedClients[i]].noOutfittingChange = qfalse;
			G_UpdateOutfitting(g_entities[level.sortedClients[i]].s.number);
			}
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sN%so%sr%sm%sa%sl damage!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Admin Action] ^7Normal damage by %s.\n\"", ent->client->pers.netname));
		}else if (ent->client->sess.admin < g_damage.integer){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
		}
		G_Say( ent, NULL, mode, p);
		return;
	}
	else if(strstr(p, "!gr")){
		if (ent->client->sess.admin >= g_gr.integer){
			Boe_GlobalSound (G_SoundIndex("sound/misc/menus/invalid.wav"));
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Gametype restart by %s.\n\"", ent->client->pers.netname));
			Boe_adminLog (va("%s - GAMETYPE RESTART", ent->client->pers.cleanName)) ;
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sG%sa%sm%se%st%sype restart!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
			trap_SendConsoleCommand( EXEC_APPEND, va("gametype_restart\n"));

		}else if (ent->client->sess.admin < g_gr.integer){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
		}
		G_Say( ent, NULL, mode, p);
		return;
	}else if(strstr(p, "!cva")){
		if (ent->client->sess.admin >= g_clanvsall.integer){
			RPM_Clan_Vs_All(ent);
		}else if (ent->client->sess.admin < g_clanvsall.integer){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
		}
		G_Say( ent, NULL, mode, p);
		return;
	}
	else if(strstr(p, "!sw") || strstr(p, "!swapteams")){
		if (ent->client->sess.admin >= g_swapteams.integer){
			Boe_SwapTeams(ent);
		}else if (ent->client->sess.admin < g_swapteams.integer){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
		}
		G_Say( ent, NULL, mode, p);
		return;
	}
	else if(strstr(p, "!l ") || strstr(p, "!lock ")){
		//if (ent->client->sess.admin >= g_lock.integer){
		if (ent->client->sess.admin >= g_lock.integer || ent->client->sess.referee == 1){
			if(strstr(p, "b") || strstr(p, "blue")){
				RPM_lockTeam(ent, qtrue, "blue");
			}else if(strstr(p, "r") || strstr(p, "red")){
				RPM_lockTeam(ent, qtrue, "red");
			}else if(strstr(p, "s") || strstr(p, "spec")){
				RPM_lockTeam(ent, qtrue, "spec");
			}else{
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Unknown team entered.\n\""));
				return;}
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		}else if (ent->client->sess.admin < g_lock.integer){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
		}
		G_Say( ent, NULL, mode, p);
		return;
	}
	else if(strstr(p, "!addclan ") || strstr(p, "!acl ")){
		Boe_Add_Clan_Member(1, ent, qtrue);
	}
	else if(strstr(p, "!rc ") || strstr(p, "!rcl ") || strstr(p, "!removeclan ")){
		Boe_Remove_Clan_Member(1, ent, qtrue);
	}
	else if ((strstr(p, "!fl ")) || (strstr(p, "!flash "))){
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
			}
			if(strstr(p, "all")){
				for(it=0;it<level.numConnectedClients;it++){
				missile = NV_projectile( &g_entities[level.sortedClients[it]], g_entities[level.sortedClients[it]].r.currentOrigin, dir, weapon, 0 );
				missile->nextthink = level.time + 250;
				}
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7Everyone has been %sf%sl%sa%ss%sh%sed by %s", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, ent->client->pers.netname));
				trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Everyone has been flashed by %s.\n\"", ent->client->pers.netname));
				Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
				Boe_adminLog (va("%s - FLASH ALL", ent->client->pers.cleanName)) ;
			}else{
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
	else if(strstr(p, "!fp")){
		if (ent->client->sess.admin >= 4){
			HENK_CHECKFP(ent);
		}else if (ent->client->sess.admin < 4){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
		}
		G_Say( ent, NULL, mode, p);
		return;
	}else if(strstr(p, "!map ")){
		if (ent->client->sess.admin >= 4){
			char *numb;
			int i;
			//fileHandle_t	f;
			if(strlen(p) >= 5){
				for(i=0;i<=20;i++){
					if(p[i] == ' '){
						numb = va("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", p[i+1], p[i+2], p[i+3], p[i+4], p[i+5], p[i+6], p[i+7], p[i+8], p[i+9], p[i+10], p[i+11], p[i+12], p[i+13], p[i+14], p[i+15]);
						break;
					}
				}//trap_FS_FOpenFile( va("maps\\%s.bsp", numb), &f, FS_READ );
				//if ( !f ){
				//	trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Map not found.\n\""));
				//	return;
				//}	
				//trap_FS_FCloseFile(f);
				
				//trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sT%si%sm%se%sl%simit %i!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, number));
				//trap_SendServerCommand( ent-g_entities, va("print \"^3[Admin Action] ^7Timelimit changed to %i by %s.\n\"", number, ent->client->pers.netname));
				trap_SendConsoleCommand( EXEC_APPEND, va("map %s\n", numb));
				G_Say( ent, NULL, mode, p);
				//Boe_adminLog (va("%s - TIMELIMIT %i", ent->client->pers.cleanName, number)) ;
			}
		}else if (ent->client->sess.admin < 4){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
		}
		return;
	}else if(strstr(p, "!gt ")){
		if (ent->client->sess.admin >= 4){
			char *numb;
			int number;
			char gametype[8];
			if(strstr(p, "ctf")){
				trap_SendConsoleCommand( EXEC_APPEND, va("g_gametype ctf\n"));
				strcpy(gametype, "ctf");
			}else if(strstr(p, "inf")){
				trap_SendConsoleCommand( EXEC_APPEND, va("g_gametype inf\n"));
				strcpy(gametype, "inf");
			}else if(strstr(p, "dm")){
				trap_SendConsoleCommand( EXEC_APPEND, va("g_gametype dm\n"));
				strcpy(gametype, "dm");
			}else if(strstr(p, "tdm")){
				trap_SendConsoleCommand( EXEC_APPEND, va("g_gametype tdm\n"));
				strcpy(gametype, "tdm");
			}else if(strstr(p, "elim")){
				trap_SendConsoleCommand( EXEC_APPEND, va("g_gametype tdm\n"));
				strcpy(gametype, "elim");
			}else{
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Unkown gametype.\n\""));
				G_Say( ent, NULL, mode, p);
				return;
			}
				//trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sT%si%sm%se%sl%simit %i!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, number));
				trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Gametype changed to %s by %s.\n\"", gametype, ent->client->pers.netname));
				//Boe_adminLog (va("%s - TIMELIMIT %i", ent->client->pers.cleanName, number)) ;
			}
		else if (ent->client->sess.admin < 4){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
		}
		G_Say( ent, NULL, mode, p);
		return;
	}else if(strstr(p, "!cm")){
		if (ent->client->sess.admin >= 4){
			if(g_compMode.integer == 0){
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sC%so%sm%sp%se%stition mode enabled!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
				trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Competition mode enabled by %s.\n\"", ent->client->pers.netname));
				Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
				trap_Cvar_Set("g_compMode", "1");
				//Boe_adminLog (va("%s - TIMELIMIT %i", ent->client->pers.cleanName, number)) ;
			}else{
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sC%so%sm%sp%se%stition mode disabled!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
				trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Competition mode disabled by %s.\n\"", ent->client->pers.netname));
				Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
				trap_Cvar_Set("g_compMode", "0");
			}
			trap_Cvar_Update(&g_compMode);
		}else if (ent->client->sess.admin < 4){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
		}
		G_Say( ent, NULL, mode, p);
		return;
	}else if(strstr(p, "!3rd")){
		if (ent->client->sess.admin >= 4){
			if(g_allowthirdperson.integer == 0){
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sT%sh%si%sr%sd%sperson enabled!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
				trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Thirdperson enabled by %s.\n\"", ent->client->pers.netname));
				Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
				trap_Cvar_Set("g_allowthirdperson", "1");
				//Boe_adminLog (va("%s - TIMELIMIT %i", ent->client->pers.cleanName, number)) ;
			}else{
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sT%sh%si%sr%sd%sperson disabled!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
				trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Thirdperson disabled by %s.\n\"", ent->client->pers.netname));
				Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
				trap_Cvar_Set("g_allowthirdperson", "0");
			}
		}else if (ent->client->sess.admin < 4){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
		}
		G_Say( ent, NULL, mode, p);
		return;
	}else if(strstr(p, "!pa")){
		if (ent->client->sess.admin >= 4){
				//trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sT%sh%si%sr%sd%sperson enabled!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
				trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Paused by %s.\n\"", ent->client->pers.netname));
				RPM_Pause(ent);
		}else if (ent->client->sess.admin < 4){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
		}
		G_Say( ent, NULL, mode, p);
		return;
	}else if(strstr(p, "!up")){
		if (ent->client->sess.admin >= 4){
				//trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sT%sh%si%sr%sd%sperson enabled!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
				trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Unpaused by %s.\n\"", ent->client->pers.netname));
				RPM_Unpause(ent);
		}else if (ent->client->sess.admin < 4){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
		}
		G_Say( ent, NULL, mode, p);
		return;
	}else if(strstr(p, "!mo ")){
		if (ent->client->sess.admin >= 4){
			id = CheckAdmin(ent, p, qtrue);
			targ = g_entities+id;
			if(id < 0) return;
				//trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sT%sh%si%sr%sd%sperson enabled!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
			if(targ->client->sess.monkey == qtrue){
				targ->client->sess.monkey = qfalse;
				targ->client->noOutfittingChange = qfalse;
				G_UpdateOutfitting(targ->s.number);
				trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Monkey disabled on %s.\n\"", targ->client->pers.netname));
				Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			}else{
				targ->client->sess.monkey = qtrue;
				SetTeam( targ, "s", NULL );
				SetTeam( targ, "r", NULL );
				trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Monkey enabled on %s.\n\"", targ->client->pers.netname));
				Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			}
		}else if (ent->client->sess.admin < 4){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
		}
		G_Say( ent, NULL, mode, p);
		return;
	}
	// Boe!Man 5/2/10: The Referee tokens. I think we're actually the first with this... (:
	else if(strstr(p, "!i ")){
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
	}else if(strstr(p, "!test")){
		AdminCommands[1].Function(1, ent, qtrue);
		trap_SendServerCommand( -1, va("print \"^3[Debug] ^7%s level is %i.\n\"", AdminCommands[1].adminCmd, *AdminCommands[1].adminLevel));

	}else if ((strstr(p, "!xu ")) || (strstr(p, "!xuc ")) || (strstr(p, "!xuppercut "))) {
		char *numb;
		int id = -1, i;
		if(ent->client->sess.admin >= 4){
		for(i=0;i<=20;i++){
			if(p[i] == ' '){
				numb = va("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", p[i+1], p[i+2], p[i+3], p[i+4], p[i+5], p[i+6], p[i+7], p[i+8], p[i+9], p[i+10], p[i+11], p[i+12], p[i+13], p[i+14], p[i+15]);
				break;
			}
		}
		for(i=0;i<=level.numConnectedClients;i++){
			//trap_SendServerCommand(-1, va("print\"^3[Debug] ^7%s comparing with %s.\n\"", g_entities[level.sortedClients[i]].client->pers.cleanName,numb));
			if(strstr(Q_strlwr(g_entities[level.sortedClients[i]].client->pers.cleanName), Q_strlwr(numb))){
				id = level.sortedClients[i];
				break;
			}
		}
		if(id != -1){
		g_entities[id].client->ps.pm_flags |= PMF_JUMPING;
		g_entities[id].client->ps.groundEntityNum = ENTITYNUM_NONE;
		// Boe!Man 5/3/10: We higher the uppercut.
		g_entities[id].client->ps.velocity[2] = 1400;
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		
		if(g_entities[id].client->sess.lastIdentityChange)	{
			g_entities[id].client->sess.lastIdentityChange = qfalse;
		}
		else {
			g_entities[id].client->sess.lastIdentityChange = qtrue;
		}
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,^7%s was %su%sp%sp%se%sr%scut by %s", level.time + 5000, g_entities[id].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, ent->client->pers.netname));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was uppercut by %s.\n\"", g_entities[id].client->pers.netname,ent->client->pers.netname));
		}
		}	else if (ent->client->sess.admin < 4){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\""));
		}
		}
	// Boe!Man 1/24/10: Different kinds of Talk during Gameplay.
	if ((strstr(p, "!at ")) || (strstr(p, "!admintalk ")) || (strstr(p, "!AT"))) {
		if (ent->client->sess.admin){
			p = ConcatArgs(1);
			for(i=4;i<=strlen(p);i++){
			p[a] = p[i];
			a += 1;
			}
			mode = ADM_TALK;
			acmd = qtrue;
		}else{
			/*
			p = ConcatArgs(1);
			G_Say( ent, NULL, mode, p );
			return;
			*/
			p = ConcatArgs(1);
			for(i=4;i<=strlen(p);i++){
			p[a] = p[i];
			a += 1;
			}
			mode = CADM_CHAT;
			acmd = qtrue;
		}
	}
	else if ((strstr(p, "!ac ")) || (strstr(p, "!AC "))) {
		if (ent->client->sess.admin){
			p = ConcatArgs(1);
			for(i=4;i<=strlen(p);i++){
			p[a] = p[i];
			a += 1;
			}
			mode = ADM_CHAT;
			acmd = qtrue;
		}else{
			p = ConcatArgs(1);
			G_Say( ent, NULL, mode, p );
			return;
		}
	}
	// Boe!Man 4/17/10: Clan chat.
	else if ((strstr(p, "!cc ")) || (strstr(p, "!CC "))) {
		if (ent->client->sess.clanMember == 1){
			p = ConcatArgs(1);
			for(i=4;i<=strlen(p);i++){
			p[a] = p[i];
			a += 1;
			}
			mode = CLAN_CHAT;
			acmd = qtrue;
		}else{
			p = ConcatArgs(1);
			G_Say( ent, NULL, mode, p );
			return;
		}
	}
	else if ((strstr(p, "!ca "))) {
			p = ConcatArgs(1);
			for(i=4;i<=strlen(p);i++){
			p[a] = p[i];
			a += 1;
			}
			mode = CADM_CHAT;
			acmd = qtrue;
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
	
	Boe_Tokens(ent, p, SAY_TELL);

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
		trap_SendServerCommand( ent-g_entities, va("print \"You have a very small penis.\n\"" ) );
		Boe_crashLog(va("%s - %s - vsay_team: %s", ent->client->pers.cleanName, ent->client->pers.ip, text)) ;
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
	char			string[1024];
	Com_Printf("Adding ip to list...\n");
	len = trap_FS_FOpenFile( va("country\\IP.known"), &f, FS_APPEND_TEXT );
	if (!f)
	{
		Com_Printf("^1Error opening File\n");
		return;
	}
	strcpy(string, va("1\n{\nip \"%s\"\ncountry \"%s\"\next \"%s\"\n}\n", ip, country, ext));

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
		G_LogPrintf("Checking ip..\n");
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

void HENK_CHECKFP(gentity_t *ent){
	void	*GP2, *group;
	char	*filePtr, *file, Files[1024];
	int		fileCount;
	struct fp{
		char ip[24];
		char guid[10];
	} Players[64];
	int xt = 0, xd = 0, xz = 0;
	fileCount = trap_FS_GetFileList( "fairplay", ".db", Files, 1024 );
	filePtr = Files;
	file = va("fairplay\\%s", filePtr);
	GP2 = trap_GP_ParseFile(file, qtrue, qfalse);
	if (!GP2)
	{
		G_LogPrintf("Error in file: \"%s\" or file not found.\n", file);
		return;
	}
		group = trap_GPG_GetSubGroups(GP2);

		while(group)
		{
			trap_GPG_FindPairValue(group, "IP", "0", Players[xt].ip);
			trap_GPG_FindPairValue(group, "guid", "", Players[xt].guid);
			xt += 1;
			group = trap_GPG_GetNext(group);
		}
		trap_GP_Delete(&GP2);
		for(xz=0;xz<level.numConnectedClients;xz++){
			level.clients[level.sortedClients[xz]].sess.fairplay = qfalse;
			for(xd=0;xd<xt;xd++){
				if(strstr(level.clients[level.sortedClients[xz]].pers.ip, Players[xd].ip)){
					level.clients[level.sortedClients[xz]].sess.fairplay = qtrue;
					strcpy(level.clients[level.sortedClients[xz]].sess.fpGuid, Players[xd].guid);
				}
			}
		}

		trap_SendServerCommand( -1, va("print \"^3[Info] ^7Everyone without fairplay has been forced to Spectator.\n\"") );
		for(xz=0;xz<level.numConnectedClients;xz++){
			if(level.clients[level.sortedClients[xz]].sess.fairplay == qfalse){
				SetTeam(&g_entities[level.sortedClients[xz]], "spec", NULL);
			}
		}
		return;
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
	G_LogPrintf( "HENK_COUNTRY starting...\n" );
	//Com_sprintf(IP, 24, ent->client->pers.ip);
	IP = va("%s", ent->client->pers.ip);
	//Com_Printf("IP is: %s\n", ent->client->pers.ip);
	// Set countx to zero, when you do not set the variable you get weird ass results.
	countx[0] = 0;
	countx[1] = 0;
	countx[2] = 0;
	countx[3] = 0;
	// End
	G_LogPrintf( "HENK_COUNTRY: 1\n" );
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
				*++IP; // ignore this error, compiler says it does not have an effect
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
	G_LogPrintf( "HENK_COUNTRY: 2\n" );
	for(i=0;i<=3;i++){ // 4 octets
		for(z=0;z<countx[i];z++){
		octetx[i][z] = octet[i][countx[i]-(z+1)];
		}
		octetx[i][countx[i]] = '\0';
	}
	G_LogPrintf( "HENK_COUNTRY: 3(Octet: %s)\n", octetx[0]);
	// End
	
	// Handle the preloaded country files
	/*if(strstr(va("%s", octetx[0]), "195")){
		group = trap_GPG_GetSubGroups(DB195);
	}else if(strstr(va("%s", octetx[0]), "212")){
		group = trap_GPG_GetSubGroups(DB212);
	}else if(strstr(va("%s", octetx[0]), "62")){
		group = trap_GPG_GetSubGroups(DB62);
	}else if(strstr(va("%s", octetx[0]), "193")){
		group = trap_GPG_GetSubGroups(DB193);
	}else if(strstr(va("%s", octetx[0]), "213")){
		group = trap_GPG_GetSubGroups(DB213);
	}else if(strstr(va("%s", octetx[0]), "217")){
		group = trap_GPG_GetSubGroups(DB217);
	}else if(strstr(va("%s", octetx[0]), "192")){
		group = trap_GPG_GetSubGroups(DB192);
	}else if(strstr(va("%s", octetx[0]), "216")){
		group = trap_GPG_GetSubGroups(DB216);
	}else if(strstr(va("%s", octetx[0]), "91")){
		group = trap_GPG_GetSubGroups(DB91);
	}else if(strstr(va("%s", octetx[0]), "209")){
		group = trap_GPG_GetSubGroups(DB209);
	}else if(strstr(va("%s", octetx[0]), "80")){
		group = trap_GPG_GetSubGroups(DB80);
	}else if(strstr(va("%s", octetx[0]), "64")){
		group = trap_GPG_GetSubGroups(DB64);
	}else if(strstr(va("%s", octetx[0]), "202")){
		group = trap_GPG_GetSubGroups(DB202);
	}else if(strstr(va("%s", octetx[0]), "66")){
		group = trap_GPG_GetSubGroups(DB66);
	}else if(strstr(va("%s", octetx[0]), "203")){
		group = trap_GPG_GetSubGroups(DB203);
	}else{*/
	// End
	G_LogPrintf( "HENK_COUNTRY: else check\n" );

		fileCount = trap_FS_GetFileList( "country", va(".%s", octetx[0]), Files, 1024 );
		filePtr = Files;
		file = va("country\\%s", filePtr);
		GP2 = trap_GP_ParseFile(file, qtrue, qfalse);
		if (!GP2)
		{
			G_LogPrintf("Error in file: \"%s\" or file not found.\n", file);
		}
		G_LogPrintf( "HENK_COUNTRY: 123\n" );
		group = trap_GPG_GetSubGroups(GP2);
		G_LogPrintf( "HENK_COUNTRY: 1234\n" );
	//}
	G_LogPrintf( "HENK_COUNTRY: 4\n" );
	RealOctet[0] = atoi(octetx[0]);
	RealOctet[1] = atoi(octetx[1]);
	RealOctet[2] = atoi(octetx[2]);
	RealOctet[3] = atoi(octetx[3]);

	IPnum = (RealOctet[0] * 16777216) + (RealOctet[1] * 65536) + (RealOctet[2] * 256) + (RealOctet[3]);
	G_LogPrintf( "HENK_COUNTRY: 5\n" );
	if(GP2){ // henk check if group is correct
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
				G_LogPrintf( "HENK_COUNTRY: 7\n" );
				trap_GP_Delete(&GP2);
				strcpy(ent->client->sess.country, country);
				strcpy(ent->client->sess.countryext, ext);
				AddIPList(ent->client->pers.ip, country, ext);
				G_LogPrintf( "HENK_COUNTRY: 8\n" );
				return;
				break; // stop searching
			}
			group = trap_GPG_GetNext(group);
		}
	}
		G_LogPrintf( "HENK_COUNTRY: 9\n" );
		// Start checking other file
		fileCount = trap_FS_GetFileList( "country", ".overig", Files, 1024 );
		filePtr = Files;
		file = va("country\\%s", filePtr);
		GP2 = trap_GP_ParseFile(file, qtrue, qfalse);
		G_LogPrintf( "HENK_COUNTRY: 10\n" );
		if (!GP2)
		{
			G_LogPrintf("Error in file: \"%s\" or file not found.\n", file);
		}
		group = trap_GPG_GetSubGroups(GP2);
	G_LogPrintf( "HENK_COUNTRY: 11\n" );
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
				G_LogPrintf( "HENK_COUNTRY: 12\n" );
				trap_GP_Delete(&GP2);
				strcpy(ent->client->sess.country, country);
				strcpy(ent->client->sess.countryext, ext);
				AddIPList(ent->client->pers.ip, country, ext);
				G_LogPrintf( "HENK_COUNTRY: 13\n" );
				return;
				break; // stop searching
			}
			group = trap_GPG_GetNext(group);
			G_LogPrintf( "HENK_COUNTRY: 14\n" );
		}
		// End other file
		strcpy(ent->client->sess.countryext, "??");
		trap_GP_Delete(&GP2);
		G_LogPrintf( "HENK_COUNTRY: Done\n" );
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
#ifdef _SOF2_BOTS
	else if (Q_stricmp (cmd, "addbot") == 0)
		trap_SendServerCommand( clientNum, va("print \"ADDBOT command can only be used via RCON\n\"" ) );
#endif

	else
		trap_SendServerCommand( clientNum, va("print \"unknown cmd %s\n\"", cmd ) );
}
