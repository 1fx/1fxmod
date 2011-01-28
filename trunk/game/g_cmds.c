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
	{"!g ", "gametype", &g_mapswitch.integer, &Henk_Gametype},
	{"!gt ", "gametype", &g_mapswitch.integer, &Henk_Gametype},
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
		trap_SendServerCommand2(clientNum, text);
} 

qboolean henk_isdigit(char c){ // by henk
	if(c >= 48 && c <= 57){
		return qtrue;
	}else{
		return qfalse;
	}
}


qboolean henk_ischar(char c){
	if(c >= 97 && c <= 122){ // a-z lowercase
		return qtrue;
	}else if(c >= 65 && c <= 90){ // A-Z uppercase
		return qtrue;
	}else{
		return qfalse;
	}
}

unsigned int henk_atoi( const char *string ) {
	unsigned int		sign;
	unsigned int		value;
	unsigned int		c;


	// skip whitespace
	while ( *string <= ' ' ) {
		if ( !*string ) {
			return 0;
		}
		string++;
	}

	// check sign
	switch ( *string ) {
	case '+':
		string++;
		sign = 1;
		break;
	case '-':
		string++;
		sign = -1;
		break;
	default:
		sign = 1;
		break;
	}

	// read digits
	value = 0;
	do {
		c = *string++;
		if ( c < '0' || c > '9' ) {
			break;
		}
		c -= '0';
		value = value * 10 + c;
	} while ( 1 );

	// not handling 10e10 notation...

	return value * sign;
}

/*
================
G_AdjustClientBBox
================
*/
void G_AdjustClientBBox(gentity_t *other)
{
	other->client->minSave[0] = other->r.mins[0];
	other->client->minSave[1] = other->r.mins[1];
	other->client->minSave[2] = other->r.mins[2];

	other->client->maxSave[0] = other->r.maxs[0];
	other->client->maxSave[1] = other->r.maxs[1];
	other->client->maxSave[2] = other->r.maxs[2];

	// Adjust the hit box to account for hands and such 
	// that are sticking out of the normal bounding box
	if (other->client->ps.pm_flags & PMF_LEANING)
	{
		other->r.maxs[0] *= 3.0f;
		other->r.maxs[1] *= 3.0f;
		other->r.mins[0] *= 3.0f;
		other->r.mins[1] *= 3.0f;
		other->r.svFlags |= SVF_DOUBLED_BBOX;
	}

	// Relink the entity into the world
	trap_LinkEntity (other);
}


/*
================
G_AdjustClientBBoxs

Inflates every clients bbox to take leaning/jumping etc into account this frame.
This is all very nasty really...
================
*/
void G_AdjustClientBBoxs(void)
{
	int i;

	// Move all the clients back into the reference clients time frame.
	for (i = 0; i < level.numConnectedClients; i++)
	{
		gentity_t* other = &g_entities[level.sortedClients[i]];

		if (other->client->pers.connected != CON_CONNECTED)
		{
			continue;
		}

		// Skip entities not in use
		if (!other->inuse)
		{
			continue;
		}

		// Skip clients that are spectating
		if (G_IsClientSpectating(other->client) || G_IsClientDead(other->client))
		{
			continue;
		}

		G_AdjustClientBBox(other);
	}	
}



/*
================
G_UndoAdjustedClientBBox
================
*/
void G_UndoAdjustedClientBBox(gentity_t *other)
{
	// Put the hitbox back the way it was
	other->r.maxs[0] = other->client->maxSave[0];
	other->r.maxs[1] = other->client->maxSave[1];
	other->r.maxs[2] = other->client->maxSave[2];

	other->r.mins[0] = other->client->minSave[0];
	other->r.mins[1] = other->client->minSave[1];
	other->r.mins[2] = other->client->minSave[2];

	other->r.svFlags &= (~SVF_DOUBLED_BBOX);
}


/*
================
G_UndoAdjustedClientBBoxs
================
*/
void G_UndoAdjustedClientBBoxs(void)
{
	int i;

	for ( i = 0; i < level.numConnectedClients; i ++ )
	{
		gentity_t* other = &g_entities[level.sortedClients[i]];
		
		if ( other->client->pers.connected != CON_CONNECTED )
		{
			continue;
		}

		// Skip clients that are spectating
		if ( G_IsClientSpectating ( other->client ) || G_IsClientDead ( other->client ) )
		{
			continue;
		}

		G_UndoAdjustedClientBBox(other);

		// Relink the entity into the world
		trap_LinkEntity ( other );
	}
}

/*
================
G_SetClientPreLeaningBBox
================
*/
void G_SetClientPreLeaningBBox(gentity_t *ent)
{
	//Ryan
	//ent->r.maxs[2] += g_adjPLBMaxsZ.value;
	ent->r.maxs[2] += PLB_MAXZ;
	//Ryan
}

/*
================
G_SetClientLeaningBBox
================
*/

void G_SetClientLeaningBBox(gentity_t *ent)
{
	float	leanOffset;
	vec3_t	up;
	vec3_t	right;
	
	// adjust origin for leaning
	//BG_ApplyLeanOffset(&ent->client->ps, org);//ent->r.currentOrigin);
	leanOffset = (float)(ent->client->ps.leanTime - LEAN_TIME) / LEAN_TIME * LEAN_OFFSET;
	
	//Ryan
	//leanOffset *= g_adjLeanOffset.value;
	leanOffset *= BBOX_LEAN_OFFSET;
	//Ryan

	AngleVectors( ent->client->ps.viewangles, NULL, right, up);
	VectorMA(ent->r.currentOrigin, leanOffset, right, ent->r.currentOrigin);
	VectorMA(ent->r.currentOrigin, Q_fabs(leanOffset) * -0.20f, up, ent->r.currentOrigin);

	//Ryan
	//ent->r.maxs[2] += g_adjLBMaxsZ.value;
	ent->r.maxs[2] += LB_MAXZ;
	//Ryan
	
	if (ent->client->ps.pm_flags & PMF_DUCKED)
	{
		//Ryan
		//ent->r.mins[2] += g_adjDuckedLBMinsZ.value;
		ent->r.mins[2] += DUCKED_LB_MINZ;
		//Ryan
	}
	else
	{
		//Ryan
		//ent->r.mins[2] += g_adjLBMinsZ.value;
		ent->r.mins[2] += LB_MINZ;
		//Ryan
	}
}				

///RxCxW - 01.10.06 - 03:09am #bbox
//Ryan
/*
================
G_ShowClientBBox

Ripped from unlagged code by Neil “haste” Toronto 
================

void G_ShowClientBBox(gentity_t *ent)
{
	vec3_t corners[8];
	float extx, exty, extz;
	gentity_t *tent;
	int i;

	// get the extents (size)
	extx = ent->r.maxs[0] - ent->r.mins[0];
	exty = ent->r.maxs[1] - ent->r.mins[1];
	extz = ent->r.maxs[2] - ent->r.mins[2];

	VectorAdd(ent->r.currentOrigin, ent->r.maxs, corners[3]);

	VectorCopy(corners[3], corners[2]);
	corners[2][0] -= extx;

	VectorCopy(corners[2], corners[1]);
	corners[1][1] -= exty;

	VectorCopy(corners[1], corners[0]);
	corners[0][0] += extx;

	for (i = 0; i < 4; i++)
	{
		VectorCopy(corners[i], corners[i + 4]);
		corners[i + 4][2] -= extz;
	}

/*	corners[0][2] = ent->r.currentOrigin[2] + g_adjPLBMaxsZ.value;
	corners[1][2] = ent->r.currentOrigin[2] + g_adjPLBMaxsZ.value;
	Com_Printf("^3corners0 Z: %.2f\n", corners[0][2]);
	Com_Printf("^3corners1 Z: %.2f\n", corners[1][2]);*/
/*
	// top
	tent = G_TempEntity(corners[0], EV_BOTWAYPOINT);
	VectorCopy(corners[1], tent->s.angles);
	tent->r.svFlags |= SVF_BROADCAST;

	tent = G_TempEntity(corners[2], EV_BOTWAYPOINT);
	VectorCopy(corners[3], tent->s.angles);
	tent->r.svFlags |= SVF_BROADCAST;

	tent = G_TempEntity(corners[2], EV_BOTWAYPOINT);
	VectorCopy(corners[1], tent->s.angles);
	tent->r.svFlags |= SVF_BROADCAST;

	tent = G_TempEntity(corners[0], EV_BOTWAYPOINT);
	VectorCopy(corners[3], tent->s.angles);
	tent->r.svFlags |= SVF_BROADCAST;

	// bottom
	tent = G_TempEntity(corners[7], EV_BOTWAYPOINT);
	VectorCopy(corners[6], tent->s.angles);
	tent->r.svFlags |= SVF_BROADCAST;

	tent = G_TempEntity(corners[5], EV_BOTWAYPOINT);
	VectorCopy(corners[4], tent->s.angles);
	tent->r.svFlags |= SVF_BROADCAST;

	tent = G_TempEntity(corners[5], EV_BOTWAYPOINT);
	VectorCopy(corners[6], tent->s.angles);
	tent->r.svFlags |= SVF_BROADCAST;

	tent = G_TempEntity(corners[7], EV_BOTWAYPOINT);
	VectorCopy(corners[4], tent->s.angles);
	tent->r.svFlags |= SVF_BROADCAST;
}
/*	
	gentity_t *tent;
	int i, j;
	vec3_t start, end;
	
	VectorAdd(ent->r.currentOrigin, other->r.maxs, start);

	for(i = 0; i <= 2; i++)
	{
		for(j = 0; j <= 2; j++)
		{
		}
	}
	tent = G_TempEntity(muzzlePoint, EV_BOTWAYPOINT);
	VectorCopy(tr.endpos, tent->s.angles);
	tent->r.svFlags |= SVF_BROADCAST;
*/


/*
=============
RPM_Awards
=============
*/
void RPM_Awards(void)
{
	static int overallScore = 0, headshots = 0, damage = 0, explosiveKills = 0, knifeKills = 0;
	static float  accuracy = 0, ratio = 0;
	unsigned int i, numPlayers = 0;
	//I changed this to an integer so players who connect
	//faster than the rest don't bump up the avg. time
	//float avgtime = 0.0, playerTime = 0.0;
	int avgtime = 0, playerTime = 0;
	statinfo_t     *stat;
	static gentity_t *bestOverall = NULL, *headshooter = NULL, *killer = NULL;
	static gentity_t *accurate = NULL, *bestRatio = NULL, *explosive = NULL, *knifer = NULL;
	gentity_t *ent;

	///RxCxW - 01.12.06 - 09:26pm #TEST - for award debug :p
	//char	*a, *b;
	///End  - 01.12.06 - 09:26pm

#ifdef _BOE_DBG
	if (strstr(boe_log.string, "1"))
		G_LogPrintf("1s\n");
#endif

	if(!level.awardTime)
	{
		//find the average time player by all connected clients
		for (i=0; i < level.maxclients ; i++)
		{
			ent = g_entities + i;
			if (!ent->inuse)
			{
				continue;
			}
			numPlayers++;
			avgtime += ((level.time - ent->client->pers.enterTime) - ent->client->sess.totalSpectatorTime) / 60000;
		}
		//incase timelimit runs out with nobody in the server
		//I'll use an "if" here
		if(numPlayers)
		{
			avgtime /= numPlayers;
			//Com_Printf("^3Averagetime %d\n", avgtime);
		}

		for (i = 0; i < level.maxclients ; i++)
		{
			ent = g_entities + i;
			if (!ent->inuse)
			{
				continue;
			}

			stat = &ent->client->pers.statinfo;

			playerTime = ((level.time - ent->client->pers.enterTime) - ent->client->sess.totalSpectatorTime) / 60000;
			//Com_Printf("%s playtime %d, Level time: %d, Enter Time: %d SPECTIME: %d\n", ent->client->pers.netname, playerTime, level.time, ent->client->pers.enterTime, ent->client->sess.totalSpectatorTime);

			//RxCxW - 1.20.2005 - #Version 0.5 compatible. Right?
			if(ent->client->sess.rpmClient >= 0.5)
//			if(ent->client->sess.rpmClient >= RPM_VERSION)
			{
				trap_SendServerCommand( i, va("stats %i %i %i %i %.2f %i %i %i %i",
				playerTime,
				(((level.time - ent->client->pers.enterTime) - ent->client->sess.totalSpectatorTime) % 60000) / 1000,
				stat->damageDone,
				stat->damageTaken,
				stat->ratio,
				stat->shotcount,
				stat->hitcount,
				stat->explosiveKills,
				stat->knifeKills));
			}

			//this will remove the SPECTATOR Press ESC etc.. from the specs screen
			if(ent->client->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR)
			{
				ent->client->ps.persistant[PERS_TEAM] = TEAM_FREE;
			}

			//Make sure they can't move
			ent->client->ps.pm_type = PM_FREEZE;

			//This will remove the HUD icons etc.. from the players screen
			ent->client->ps.stats[STAT_HEALTH] = -1;

			stat->overallScore = ent->client->sess.score + (int)(100 * (stat->accuracy + stat->ratio)) + (stat->damageDone - stat->damageTaken);

			if(stat->overallScore > overallScore)
			{
				overallScore = stat->overallScore;
				bestOverall = ent;
			}
			else if(stat->overallScore == overallScore && overallScore)
			{
				//if they got the same amout of points in less time
				//make them mvp
				if(ent->client->pers.enterTime > bestOverall->client->pers.enterTime)
				{
					overallScore = stat->overallScore;
					bestOverall = ent;
				}
			}

			if(stat->headShotKills > headshots)
			{
				headshots = stat->headShotKills;
				headshooter = ent;
			}
			else if(stat->headShotKills == headshots && headshots)
			{
				if(ent->client->pers.enterTime > headshooter->client->pers.enterTime)
				{
					headshots = stat->headShotKills;
					headshooter = ent;
				}
			}

			if(stat->damageDone > damage)
			{
				damage = stat->damageDone;
				killer = ent;
			}
			else if(stat->damageDone == damage && damage)
			{
				if(ent->client->pers.enterTime > killer->client->pers.enterTime)
				{
					damage = stat->damageDone;
					killer = ent;
				}
			}

			if(stat->explosiveKills > explosiveKills)
			{
				explosiveKills = stat->explosiveKills;
				explosive = ent;
			}
			else if(stat->explosiveKills == explosiveKills && explosiveKills)
			{
				if(ent->client->pers.enterTime > explosive->client->pers.enterTime)
				{
					explosiveKills = stat->explosiveKills;
					explosive = ent;
				}
			}

			if(stat->knifeKills > knifeKills )
			{
				knifeKills = stat->knifeKills;
				knifer = ent;
			}
			else if(stat->knifeKills == knifeKills && knifeKills)
			{
				if(ent->client->pers.enterTime > knifer->client->pers.enterTime)
				{
					knifeKills = stat->knifeKills;
					knifer = ent;
				}
			}

			//The awards below here are time sensitive so players
			//with 1/0 for ratio etc.. dont get the award
			if(playerTime < avgtime)
			{
				continue;
			}

			if(stat->accuracy > accuracy)
			{
				accuracy = stat->accuracy;
				accurate = ent;
			}
			else if(stat->accuracy == accuracy && accuracy)
			{
				//if this player held the accuracy longer give him the award
				if(ent->client->pers.enterTime < accurate->client->pers.enterTime)
				{
					accuracy = stat->accuracy;
					accurate = ent;
				}
			}

			if(stat->ratio > ratio)
			{
				ratio = stat->ratio;
				bestRatio = ent;
			}
			else if(stat->ratio == ratio && ratio)
			{
				if(ent->client->pers.enterTime < bestRatio->client->pers.enterTime)
				{
					ratio = stat->ratio;
					bestRatio = ent;
				}
			}
		}
	}

	for (i = 0; i < level.maxclients ; i++)
	{
		ent = g_entities + i;

		if (!ent->inuse)
		{
			continue;
		}

		///RxCxW - 01.12.06 - 03:03pm - dont send to bots
		if (ent->r.svFlags & SVF_BOT)
			continue;
		///End  - 01.12.06 - 03:03pm

		///RxCxW - #Version 0.5 Had awards too #Version
		///if(ent->client->sess.rpmClient < RPM_VERSION)
		if(ent->client->sess.rpmClient < 0.5)
		{
			///RxCxW - 01.12.06 - 09:21pm
#ifdef _DEBUG
			if(knifer == NULL)
				knifer = ent;
			if(explosive == NULL)
				explosive = ent;
			if(headshooter == NULL)
				explosive = ent;
#endif

			///CJJ - 2.5.2005 - Added a text based awards system for older RPM clients
			///We should spam them instead
			///trap_SendServerCommand( i, va("cp \"You are ^1NOT ^7using ^1R^4P^3M^7 %s.\nYou cannot see the awards without it.\nGet the ^$NEWEST ^7[SC] ^4VERSION ^7at\n^1http://clanforums.ath.cx/\n\nFor more information check out the\n^1R^4P^3M^7 OFFICIAL SITE: ^1www.rpm-mod.tk\n\"", RPM_VERSION_STRING));
			trap_SendServerCommand( i, va("cp \"^1Best Overall: ^7%s - %i\n^1Headshots: ^7%s - %i\n^1Killer: ^7%s - %i\n^1Accurate: ^7%s - %.2f percent\n^1Best Ratio: ^7%s - %.2f\n^1Nades: ^7%s - %i detonated\n^1Knifer: ^7%s - %i shanked",
				g_entities[bestOverall->s.number].client->pers.netname,
				overallScore,
				g_entities[headshooter->s.number].client->pers.netname,
				headshots,
				g_entities[killer->s.number].client->pers.netname,
				damage,
				g_entities[accurate->s.number].client->pers.netname,
				accuracy,
				g_entities[bestRatio->s.number].client->pers.netname,
				ratio,
				g_entities[explosive->s.number].client->pers.netname,
				explosiveKills,
				g_entities[knifer->s.number].client->pers.netname,
				knifeKills ));
			continue;
		}
		if(!level.awardTime)
		{
			///RxCxW - 01.12.06 - 09:21pm
			if(knifer == NULL)
				knifer = ent;
			if(explosive == NULL)
				explosive = ent;

			//a = va("awards %i %i %i %i %i %i %i %.2f %i %.2f %i %i\n", bestOverall->s.number, overallScore, headshooter->s.number, headshots, killer->s.number, damage, accurate->s.number, accuracy, bestRatio->s.number, ratio, explosive->s.number, explosiveKills);
			//b = va("%i %i", knifer->s.number, knifeKills );
			////b = va("%i %i", bestOverall->s.number, overallScore );
			/////trap_SendServerCommand( -1, va("awards %s %s", a, b));
			//Com_Printf("awards %s %s\n", a, b);

#ifdef _BOE_DBG
	if (strstr(boe_log.string, "1"))
		G_LogPrintf("1e\n");
#endif

#ifdef Q3_VM
			///End  - 01.12.06 - 09:23pm
			trap_SendServerCommand( -1, va("awards %i %i %i %i %i %i %i %.2f %i %.2f %i %i %i %i", 
				bestOverall->s.number,
				overallScore,
				headshooter->s.number, 
				headshots, 
				killer->s.number, 
				damage, 
				accurate->s.number, 
				accuracy, 
				bestRatio->s.number, 
				ratio, 
				explosive->s.number,
				explosiveKills,	
				knifer->s.number, 
				knifeKills 
				));
#endif
		}
	}
}

int FormatDamage(int damage){
	char test[10];
	char *test1;
	strcpy(test, va("%i", damage));
	if(damage >= 100 && damage < 1000){
	test1 = va("%c00", test[0]);
	damage = atoi(test1);
	}else if(damage >= 1000 && damage < 10000){
	test1 = va("%c%c00", test[0], test[1]);
	damage = atoi(test1);
	}else if(damage >= 10000 && damage < 100000){
	test1 = va("%c%c%c00", test[0], test[1], test[2]);
	damage = atoi(test1);
	}
return damage/10;
}

void InitSpawn(int choice) // load bsp models before players loads a map(SOF2 clients cannot load a bsp model INGAME)
{
	AddSpawnField("classname", "misc_bsp"); // blocker
	if(choice == 1){
	AddSpawnField("bspmodel",	"instances/Generic/fence01");
	}else if(choice == 2){
	AddSpawnField("bspmodel",	"instances/Colombia/npc_jump1");
	}else{
	AddSpawnField("bspmodel",	"instances/Kamchatka/wall01");
	}
	AddSpawnField("origin",		"-4841 -4396 5000");
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
		if(adm && adm->client)
			trap_SendServerCommand( adm-g_entities, va("print \"The game is not currently paused.\n\"") );
		else
			Com_Printf("The game is not currently paused.\n");
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
	trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%sC%sl%sa%sn vs all!", level.time + 5000, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
	Boe_GlobalSound(G_SoundIndex("sound/misc/events/tut_lift02.mp3"));
			
	if(adm && adm->client) {
		Boe_adminLog (va("%s - ClanVsAll", adm->client->pers.cleanName)) ;
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Clan vs all by %s.\n\"", adm->client->pers.netname));
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
	ent->client->pers.statinfo.lastclient_hurt = -1;
	ent->client->pers.statinfo.lasthurtby = -1;
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
//	gentity_t *tent;

	killerColor = S_COLOR_WHITE;
	targetColor   = S_COLOR_WHITE;
	message2 = "";
	message3 = "";
	///RxCxW - 01.08.06 - 09:14pm
	///attackt = attacker->client->pers.statinfo.attack;
	///weapon = attacker->client->pers.statinfo.weapon;
	//G_LogPrintf("Starting Obituary..\n");
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
		attackt = attacker->client->pers.statinfo.attack;
		weapon = attacker->client->pers.statinfo.weapon;
		//Boe_ClientSound(attacker, G_SoundIndex("sound/npc/col8/blakely/niceshot.mp3"));
		//trap_SendServerCommand( attacker->s.number, va("cp \"Headshot!\n\""));//g_headShotMessage.string));
		headShot = qtrue;
		//add to the total headshot count for this player
		atrstat->headShotKills++;
		atrstat->weapon_headshots[attackt][weapon]++;
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
					atrstat->knifeKills++;
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
						atrstat->explosiveKills++;
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
						atrstat->weapon_hits[ATTACK_NORMAL][mod]++; // MIGHT BE BUG
					}

					atrstat->explosiveKills++;
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
	//G_LogPrintf("Done with RPM_Obituary..\n");
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
	int			UpdateTime;
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

	if(level.numConnectedClients >= 0 && level.numConnectedClients <= 10)
		UpdateTime = 2000;
	else if(level.numConnectedClients > 11 && level.numConnectedClients <= 20)
		UpdateTime = 3000;
	else if(level.numConnectedClients > 20)
		UpdateTime = 4000;

	if (level.time - level.lastTMIupdate < UpdateTime) // Henk 06/04/10 -> Increase to reduce lagg
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
		IsClientMuted(&g_entities[cl->ps.clientNum], qfalse);
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
			
			//if(cl->sess.rpmClient > 0.6){
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
			/*}else if(cl->sess.proClient > 4.0){
			Com_sprintf (entry, sizeof(entry),
				" %i %i %i %i %i %i %s %i %i",
				level.sortedClients[i],
				cl->ps.stats[STAT_HEALTH],
				cl->ps.stats[STAT_ARMOR],
				cl->ps.weapon,
				location,
				adm
				);
			}*/

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
		else if(cl->sess.proClient > 4.0)
			trap_SendServerCommand(level.sortedClients[i], va("tmi %i%s", numAdded, infoString));
	}
}

/*
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
*/

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
			if(sess->admin)
				continue;
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

	// Boe!Man 1/6/10
	trap_SendServerCommand( other-g_entities, va("%s %d \"%s %s%s %s%s %s\"", // Boe!Man 1/6/10: Adding prefixes. - Update 1/17/10: Adding Admin Talk/Chat prefixes. - Update 5/8/10: Solved message problem.
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


char *GetReason(void) {
	char	arg[256] = "\0"; // increase buffer so we can process more commands
	char	*reason = "";
	int i, z;
	qboolean Do = qfalse;
	trap_Argv( 1, arg, sizeof( arg ) );
	for(i=0;i<256;i++){
		if(arg[i] == ' '){
			if(Do == qtrue){ // second space so its the reason
				for(z=i+1;z<256-i;z++){
					if(arg[z] == ' '){
						arg[z] = '_';
					}
					reason = va("%s%c", reason, arg[z]);
				}
				if(strlen(reason) < 1){
					trap_Argv( 3, arg, sizeof( arg ) );
					return arg;
				}
				return reason;
			}
		Do = qtrue;
		}
	}
	if(strlen(reason) < 1){
		trap_Argv( 3, arg, sizeof( arg ) );
		return arg;
	}
	return "";
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

int StartAfterCommand(char *param){
	int i;
	for(i=0;i<=strlen(param);i++){
		if(param[i] == ' '){
			return i+1;
		}
	}
	return 0;
}

void Cmd_Say_f( gentity_t *ent, int mode, qboolean arg0 ) {
	char		*p;
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
	if ((strstr(p, "!at ")) || (strstr(p, "!admintalk ")) || (strstr(p, "!AT")) || (strstr(p, "!aT ")) || (strstr(p, "!At "))) {
		if (ent->client->sess.admin){
			p = ConcatArgs(1);
			for(i=StartAfterCommand(va("%s", p));i<=strlen(p);i++){
			p[a] = p[i];
			a += 1;
			}
			mode = ADM_TALK;
			acmd = qtrue;
		}else{
			p = ConcatArgs(1);
			for(i=StartAfterCommand(va("%s", p));i<=strlen(p);i++){
			p[a] = p[i];
			a += 1;
			}
			mode = CADM_CHAT;
			acmd = qtrue;
		}
	}
	else if ((strstr(p, "!ac ")) || (strstr(p, "!AC ")) || (strstr(p, "!aC ")) || (strstr(p, "!Ac "))) {
		if (ent->client->sess.admin){
			p = ConcatArgs(1);
			for(i=StartAfterCommand(va("%s", p));i<=strlen(p);i++){
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
	else if ((strstr(p, "!cc ")) || (strstr(p, "!CC ")) || (strstr(p, "!Cc ")) || (strstr(p, "!cC "))) {
		if (ent->client->sess.clanMember == 1){
			p = ConcatArgs(1);
			for(i=StartAfterCommand(va("%s", p));i<=strlen(p);i++){
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
	else if ((strstr(p, "!ca ")) || (strstr(p, "!Ca ")) || (strstr(p, "!cA ")) || (strstr(p, "!CA "))) {
			p = ConcatArgs(1);
			for(i=StartAfterCommand(va("%s", p));i<=strlen(p);i++){
			p[a] = p[i];
			a += 1;
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
====================
IP2Country by Henkie
4/1/10 - 12:04 AM
====================
*/

void AddIPList(char ip1[24], char country[128], char ext[6])
{
	int				len;
	fileHandle_t	f;
	char			string[1024];
	char	octet[4][4], octetx[4][4];
	int		i, z, countx[4], loops = 0, count = 0;
	char *IP;
	//G_LogPrintf("Checking ip..\n");
	IP = va("%s", ip1);
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
	for(i=0;i<=3;i++){ // 4 octets
		for(z=0;z<countx[i];z++){
		octetx[i][z] = octet[i][countx[i]-(z+1)];
		}
		octetx[i][countx[i]] = '\0';
	}
	Com_Printf("Adding ip to list...\n");
	len = trap_FS_FOpenFile( va("country\\known\\IP.%s", octetx[0]), &f, FS_APPEND_TEXT );
	if (!f)
	{
		Com_Printf("^1Error opening File\n");
		return;
	}
	strcpy(string, va("1\n{\nip \"%s\"\ncountry \"%s\"\next \"%s\"\n}\n", ip1, country, ext));

	trap_FS_Write(string, strlen(string), f);
	trap_FS_Write("\n", 1, f);
	trap_FS_FCloseFile(f);

}

qboolean CheckIP(gentity_t *ent){ // Henk Update 12/05/10 -> Lag spike when file reaches 200kb, fix by splitting files
	void	*GP2, *group;
	char	*filePtr, *file, Files[1024];
	char	ip1[24], country[128], ext[6];
	int		fileCount;
	char	octet[4][4], octetx[4][4];
	int		i, z, countx[4], loops = 0, count = 0;
	char *IP;

	IP = va("%s", ent->client->pers.ip);
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
	for(i=0;i<=3;i++){ // 4 octets
		for(z=0;z<countx[i];z++){
		octetx[i][z] = octet[i][countx[i]-(z+1)];
		}
		octetx[i][countx[i]] = '\0';
	}
	fileCount = trap_FS_GetFileList( "country\\", va("IP.%s.%s", octetx[0], octetx[1]), Files, 1024 );
	filePtr = Files;
	file = va("country\\%s", filePtr);
	GP2 = trap_GP_ParseFile(file, qtrue, qfalse);
	if (!GP2)
	{
		// Boe!Man 11/12/10: Ignore file warnings!
		//G_LogPrintf("Error in file: \"%s\" or file not found.\n", file);
		return qfalse;
	}
		group = trap_GPG_GetSubGroups(GP2);

		while(group)
		{
			trap_GPG_FindPairValue(group, "ip", "0", ip1);
			if(strstr(ip1, ent->client->pers.ip)){
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
	int		i, z, countx[4], loops = 0;
	char	begin_ip[24], end_ip[24], country[128], ext[6];
	unsigned int		begin_ipi, end_ipi;

	char	octet[4][4], octetx[4][4];
	int		RealOctet[4];
	unsigned int		IPnum, part;
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
	for(i=0;i<=3;i++){ // 4 octets
		for(z=0;z<countx[i];z++){
		octetx[i][z] = octet[i][countx[i]-(z+1)];
		}
		octetx[i][countx[i]] = '\0';
	}

	RealOctet[0] = atoi(octetx[0]);
	RealOctet[1] = atoi(octetx[1]);
	RealOctet[2] = atoi(octetx[2]);
	RealOctet[3] = atoi(octetx[3]);

	IPnum = (RealOctet[0] * 16777216) + (RealOctet[1] * 65536) + (RealOctet[2] * 256) + (RealOctet[3]);
	if(IPnum >= 16777216 && IPnum <= 461234175){
	part = 0;
	}else if(IPnum >= 461234176 && IPnum <= 773699583){
	part = 1;
	}else if(IPnum >= 773699584 && IPnum <= 1029439487){
	part = 2;
	}else if(IPnum >= 1029439488 && IPnum <= 1041746799){
	part = 3;
	}else if(IPnum >= 1041746800 && IPnum <= 1044913967){
	part = 4;
	}else if(IPnum >= 1044913968 && IPnum <= 1046879847){
	part = 5;
	}else if(IPnum >= 1046879848 && IPnum <= 1048973567){
	part = 6;
	}else if(IPnum >= 1048973568 && IPnum <= 1052290559){
	part = 7;
	}else if(IPnum >= 1052290560 && IPnum <= 1053401087){
	part = 8;
	}else if(IPnum >= 1053401088 && IPnum <= 1072928263){
	part = 9;
	}else if(IPnum >= 1072928264 && IPnum <= 1076300015){
	part = 10;
	}else if(IPnum >= 1076300016 && IPnum <= 1078285215){
	part = 11;
	}else if(IPnum >= 1078285216 && IPnum <= 1081803895){
	part = 12;
	}else if(IPnum >= 1081803896 && IPnum <= 1093127551){
	part = 13;
	}else if(IPnum >= 1093127552 && IPnum <= 1104232751){
	part = 14;
	}else if(IPnum >= 1104232752 && IPnum <= 1117413375){
	part = 15;
	}else if(IPnum >= 1117413376 && IPnum <= 1120383327){
	part = 16;
	}else if(IPnum >= 1120383328 && IPnum <= 1125120847){
	part = 17;
	}else if(IPnum >= 1125120848 && IPnum <= 1136718895){
	part = 18;
	}else if(IPnum >= 1136718896 && IPnum <= 1158481426){
	part = 19;
	}else if(IPnum >= 1158481427 && IPnum <= 1160702847){
	part = 20;
	}else if(IPnum >= 1160702848 && IPnum <= 1167326335){
	part = 21;
	}else if(IPnum >= 1167326336 && IPnum <= 1208590335){
	part = 22;
	}else if(IPnum >= 1208590336 && IPnum <= 1210119119){
	part = 23;
	}else if(IPnum >= 1210119120 && IPnum <= 1254971023){
	part = 24;
	}else if(IPnum >= 1254971024 && IPnum <= 1295048191){
	part = 25;
	}else if(IPnum >= 1295048192 && IPnum <= 1310226431){
	part = 26;
	}else if(IPnum >= 1310226432 && IPnum <= 1317674543){
	part = 27;
	}else if(IPnum >= 1317674544 && IPnum <= 1346416639){
	part = 28;
	}else if(IPnum >= 1346416640 && IPnum <= 1348354047){
	part = 29;
	}else if(IPnum >= 1348354048 && IPnum <= 1358290943){
	part = 30;
	}else if(IPnum >= 1358290944 && IPnum <= 1361035767){
	part = 31;
	}else if(IPnum >= 1361035768 && IPnum <= 1372143615){
	part = 32;
	}else if(IPnum >= 1372143616 && IPnum <= 1388729280){
	part = 33;
	}else if(IPnum >= 1388729281 && IPnum <= 1401923583){
	part = 34;
	}else if(IPnum >= 1401923584 && IPnum <= 1407537607){
	part = 35;
	}else if(IPnum >= 1407537608 && IPnum <= 1422393343){
	part = 36;
	}else if(IPnum >= 1422393344 && IPnum <= 1431832846){
	part = 37;
	}else if(IPnum >= 1431832847 && IPnum <= 1446905695){
	part = 38;
	}else if(IPnum >= 1446905696 && IPnum <= 1467347935){
	part = 39;
	}else if(IPnum >= 1467347936 && IPnum <= 1495236607){
	part = 40;
	}else if(IPnum >= 1495236608 && IPnum <= 1508573259){
	part = 41;
	}else if(IPnum >= 1508573260 && IPnum <= 1535377407){
	part = 42;
	}else if(IPnum >= 1535377408 && IPnum <= 1539759871){
	part = 43;
	}else if(IPnum >= 1539759872 && IPnum <= 1540412415){
	part = 44;
	}else if(IPnum >= 1540412416 && IPnum <= 1540902911){
	part = 45;
	}else if(IPnum >= 1540902912 && IPnum <= 1550057471){
	part = 46;
	}else if(IPnum >= 1550057472 && IPnum <= 1578591055){
	part = 47;
	}else if(IPnum >= 1578591056 && IPnum <= 1592074239){
	part = 48;
	}else if(IPnum >= 1592074240 && IPnum <= 1652631535){
	part = 49;
	}else if(IPnum >= 1652631536 && IPnum <= 1835966463){
	part = 50;
	}else if(IPnum >= 1835966464 && IPnum <= 1908539391){
	part = 51;
	}else if(IPnum >= 1908539392 && IPnum <= 2022244351){
	part = 52;
	}else if(IPnum >= 2022244352 && IPnum <= 2185428991){
	part = 53;
	}else if(IPnum >= 2185428992 && IPnum <= 2343501823){
	part = 54;
	}else if(IPnum >= 2343501824 && IPnum <= 2569601023){
	part = 55;
	}else if(IPnum >= 2569601024 && IPnum <= 2769682431){
	part = 56;
	}else if(IPnum >= 2769682432 && IPnum <= 2918289407){
	part = 57;
	}else if(IPnum >= 2918289408 && IPnum <= 2943315967){
	part = 58;
	}else if(IPnum >= 2943315968 && IPnum <= 2988509363){
	part = 59;
	}else if(IPnum >= 2988509364 && IPnum <= 2988559015){
	part = 60;
	}else if(IPnum >= 2988559016 && IPnum <= 3003076607){
	part = 61;
	}else if(IPnum >= 3003076608 && IPnum <= 3162046463){
	part = 62;
	}else if(IPnum >= 3162046464 && IPnum <= 3188187135){
	part = 63;
	}else if(IPnum >= 3188187136 && IPnum <= 3225618943){
	part = 64;
	}else if(IPnum >= 3225619200 && IPnum <= 3228526591){
	part = 65;
	}else if(IPnum >= 3228526592 && IPnum <= 3231728127){
	part = 66;
	}else if(IPnum >= 3231728384 && IPnum <= 3238608895){
	part = 67;
	}else if(IPnum >= 3238608896 && IPnum <= 3240463103){
	part = 68;
	}else if(IPnum >= 3240463104 && IPnum <= 3244898047){
	part = 69;
	}else if(IPnum >= 3244898048 && IPnum <= 3247266559){
	part = 70;
	}else if(IPnum >= 3247266560 && IPnum <= 3251147519){
	part = 71;
	}else if(IPnum >= 3251147520 && IPnum <= 3252912639){
	part = 72;
	}else if(IPnum >= 3252912640 && IPnum <= 3255320575){
	part = 73;
	}else if(IPnum >= 3255320576 && IPnum <= 3257764647){
	part = 74;
	}else if(IPnum >= 3257764648 && IPnum <= 3259760639){
	part = 75;
	}else if(IPnum >= 3259760640 && IPnum <= 3262474259){
	part = 76;
	}else if(IPnum >= 3262474260 && IPnum <= 3262478703){
	part = 77;
	}else if(IPnum >= 3262478704 && IPnum <= 3262480181){
	part = 78;
	}else if(IPnum >= 3262480182 && IPnum <= 3264907639){
	part = 79;
	}else if(IPnum >= 3264907640 && IPnum <= 3267887103){
	part = 80;
	}else if(IPnum >= 3267887104 && IPnum <= 3271589887){
	part = 81;
	}else if(IPnum >= 3271589888 && IPnum <= 3273015295){
	part = 82;
	}else if(IPnum >= 3273015296 && IPnum <= 3274936159){
	part = 83;
	}else if(IPnum >= 3274936160 && IPnum <= 3276032303){
	part = 84;
	}else if(IPnum >= 3276032304 && IPnum <= 3276363967){
	part = 85;
	}else if(IPnum >= 3276363968 && IPnum <= 3276396207){
	part = 86;
	}else if(IPnum >= 3276396208 && IPnum <= 3276751175){
	part = 87;
	}else if(IPnum >= 3276751176 && IPnum <= 3278938347){
	part = 88;
	}else if(IPnum >= 3278938348 && IPnum <= 3278943038){
	part = 89;
	}else if(IPnum >= 3278943039 && IPnum <= 3278944412){
	part = 90;
	}else if(IPnum >= 3278944413 && IPnum <= 3280732159){
	part = 91;
	}else if(IPnum >= 3280732160 && IPnum <= 3280980847){
	part = 92;
	}else if(IPnum >= 3280980848 && IPnum <= 3283533823){
	part = 93;
	}else if(IPnum >= 3283533824 && IPnum <= 3285437439){
	part = 94;
	}else if(IPnum >= 3285437440 && IPnum <= 3285552383){
	part = 95;
	}else if(IPnum >= 3285552384 && IPnum <= 3287449087){
	part = 96;
	}else if(IPnum >= 3287449088 && IPnum <= 3324656127){
	part = 97;
	}else if(IPnum >= 3324656128 && IPnum <= 3351411967){
	part = 98;
	}else if(IPnum >= 3351411968 && IPnum <= 3359479383){
	part = 99;
	}else if(IPnum >= 3359479384 && IPnum <= 3391720959){
	part = 100;
	}else if(IPnum >= 3391720960 && IPnum <= 3397330943){
	part = 101;
	}else if(IPnum >= 3397330944 && IPnum <= 3400548351){
	part = 102;
	}else if(IPnum >= 3400548352 && IPnum <= 3414667263){
	part = 103;
	}else if(IPnum >= 3414667264 && IPnum <= 3419900159){
	part = 104;
	}else if(IPnum >= 3419900160 && IPnum <= 3428599295){
	part = 105;
	}else if(IPnum >= 3428599296 && IPnum <= 3449212671){
	part = 106;
	}else if(IPnum >= 3449212672 && IPnum <= 3460892271){
	part = 107;
	}else if(IPnum >= 3460892272 && IPnum <= 3468094303){
	part = 108;
	}else if(IPnum >= 3468094304 && IPnum <= 3474193663){
	part = 109;
	}else if(IPnum >= 3474193664 && IPnum <= 3486635007){
	part = 110;
	}else if(IPnum >= 3486635008 && IPnum <= 3495583743){
	part = 111;
	}else if(IPnum >= 3495583744 && IPnum <= 3508303455){
	part = 112;
	}else if(IPnum >= 3508303456 && IPnum <= 3511335423){
	part = 113;
	}else if(IPnum >= 3511335424 && IPnum <= 3517427711){
	part = 114;
	}else if(IPnum >= 3517427712 && IPnum <= 3523411967){
	part = 115;
	}else if(IPnum >= 3523411968 && IPnum <= 3558399999){
	part = 116;
	}else if(IPnum >= 3558400000 && IPnum <= 3560940043){
	part = 117;
	}else if(IPnum >= 3560940044 && IPnum <= 3560943590){
	part = 118;
	}else if(IPnum >= 3560943591 && IPnum <= 3561275391){
	part = 119;
	}else if(IPnum >= 3561275392 && IPnum <= 3564343807){
	part = 120;
	}else if(IPnum >= 3564343808 && IPnum <= 3565753983){
	part = 121;
	}else if(IPnum >= 3565753984 && IPnum <= 3569942527){
	part = 122;
	}else if(IPnum >= 3569942528 && IPnum <= 3575633119){
	part = 123;
	}else if(IPnum >= 3575633120 && IPnum <= 3576263535){
	part = 124;
	}else if(IPnum >= 3576263536 && IPnum <= 3582531783){
	part = 125;
	}else if(IPnum >= 3582531784 && IPnum <= 3585190527){
	part = 126;
	}else if(IPnum >= 3585190528 && IPnum <= 3588594751){
	part = 127;
	}else if(IPnum >= 3588594752 && IPnum <= 3624375871){
	part = 128;
	}else if(IPnum >= 3624375872 && IPnum <= 3626385393){
	part = 129;
	}else if(IPnum >= 3626385394 && IPnum <= 3631015903){
	part = 130;
	}else if(IPnum >= 3631015904 && IPnum <= 3636160927){
	part = 131;
	}else if(IPnum >= 3636160928 && IPnum <= 3641762607){
	part = 132;
	}else if(IPnum >= 3641762608 && IPnum <= 3645722623){
	part = 133;
	}else if(IPnum >= 3645722624 && IPnum <= 3647965295){
	part = 134;
	}else if(IPnum >= 3647965296 && IPnum <= 3652050619){
	part = 135;
	}else{
	part = 136;
	}
	fileCount = trap_FS_GetFileList( "country", va(".%i", part), Files, 1024 );
	filePtr = Files;
	file = va("country\\%s", filePtr);
	GP2 = trap_GP_ParseFile(file, qtrue, qfalse);
	if(GP2){ // henk check if group is correct
		group = trap_GPG_GetSubGroups(GP2);
		while(group)
		{
			trap_GPG_FindPairValue(group, "lownum", "0", begin_ip);
			begin_ipi = henk_atoi(begin_ip);
			trap_GPG_FindPairValue(group, "highnum", "", end_ip);
			end_ipi = henk_atoi(end_ip);
			//trap_SendServerCommand( -1, va("print \"^3[Debug]^7 %i with %i\n\"", IPnum, begin_ipi) );
			if(IPnum >= begin_ipi && IPnum <= end_ipi){
				trap_GPG_FindPairValue(group, "country", "", country);
				trap_GPG_FindPairValue(group, "ext", "??", ext); // add ?? as default variable
				trap_GP_Delete(&GP2);
				strcpy(ent->client->sess.country, country);
				strcpy(ent->client->sess.countryext, ext);
				return;
				break; // stop searching
			}
			group = trap_GPG_GetNext(group);
		}
	}else{
		//trap_SendServerCommand( -1, va("print \"^3[Debug]^7 File not found\n\"") );
		G_LogPrintf("IP2Country Debug: File not found\n"); // Boe 12/23/10: Append to console & log to file rather than sending debug information to all clients.
		
	}
		// End other file
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
		if(IsClientMuted(ent, qtrue)){
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
		//char arg1[10];
		//trap_Argv( 1, arg1, sizeof( arg1 ) );
		//Henk_RemoveLineFromFile(ent, atoi(arg1), "users/bans.txt");
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
		trap_SendServerCommand( ent-g_entities, va("print \" \n ^3Lvl   Commands         Arguments     Explanation\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" ----------------------------------------------------------\n\""));
	// Boe!Man 9/21/10: We loop the print process and make sure they get in proper order.
	for(level=2;level<=adm;level++){
	if (adm >= g_kick.integer && g_kick.integer != 5 && g_kick.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   k   kick         <id> <reason> ^7[^3Kick a player^7]\n\"", g_kick.integer));
		}
	if (adm >= g_addbadmin.integer && g_addbadmin.integer != 5 && g_addbadmin.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   ab  addbadmin    <id>          ^7[^3Add a Basic Admin^7]\n\"", g_addbadmin.integer));
		}
	if (adm >= g_addadmin.integer && g_addadmin.integer != 5 && g_addadmin.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   aa  addadmin     <id>          ^7[^3Add an Admin^7]\n\"", g_addadmin.integer));
		}
	// Boe!Man 1/4/10: Fix with using Tab in the Admin list.
	if (adm >= g_addsadmin.integer && g_addsadmin.integer != 5 && g_addsadmin.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   as  addsadmin    <id>          ^7[^3Add a Server Admin^7]\n\"", g_addsadmin.integer));
		}
	if (adm >= g_ban.integer && g_ban.integer != 5 && g_ban.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   ba  ban          <id> <reason> ^7[^3Ban a player^7]\n\"", g_ban.integer));
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   uba unban        <ip>          ^7[^3Unban a banned IP^7]\n\"", g_ban.integer));
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   bl  banlist                    ^7[^3Shows the current banlist^7]\n\"", g_ban.integer));
		}
	if (adm >= g_subnetban.integer && g_subnetban.integer != 5 && g_subnetban.integer == level){
		// Boe!Man 1/6/10: Reason added to the Subnetban command.
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   sb  subnetban    <id> <reason> ^7[^3Ban a players' subnet^7]\n\"", g_subnetban.integer));
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   sub subnetunban  <ip>          ^7[^3Unban a banned subnet^7]\n\"", g_subnetban.integer));
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   sbl subnetbanlist              ^7[^3Shows the current subnetbanlist^7]\n\"", g_subnetban.integer));
		}
	if (adm >= g_uppercut.integer && g_uppercut.integer != 5 && g_uppercut.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   uc  uppercut     <id>          ^7[^3Launch a player upwards^7]\n\"", g_uppercut.integer));
		}
	if (adm >= g_twist.integer && g_twist.integer != 5 && g_twist.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   tw  twist        <id>          ^7[^3Twist a player^7]\n\"", g_twist.integer));
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   utw untwist      <id>          ^7[^3Untwist a twisted player^7]\n\"", g_twist.integer));
		}
	if (adm >= g_runover.integer && g_runover.integer != 5 && g_runover.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   ro  runover      <id>          ^7[^3Boost a player backwards^7]\n\"", g_runover.integer));
		}
	if (adm >= g_mapswitch.integer && g_mapswitch.integer != 5 && g_mapswitch.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   mr  maprestart                 ^7[^3Restart the current map^7]\n\"", g_mapswitch.integer));
		}
	if (adm >= g_flash.integer && g_flash.integer != 5 && g_flash.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   fl  flash        <id>          ^7[^3Flash a player^7]\n\"", g_flash.integer));
		}
	if (adm >= g_pop.integer && g_pop.integer != 5 && g_pop.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   p   pop          <id>          ^7[^3Pop a player^7]\n\"", g_pop.integer));
		}
	if (adm >= g_strip.integer && g_strip.integer != 5 && g_strip.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   s  strip         <id>          ^7[^3Remove weapons from a player^7]\n\"", g_strip.integer));
		}
	if (adm >= g_mute.integer && g_mute.integer != 5 && g_mute.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   m   mute         <id>          ^7[^3Mute a player^7]\n\"", g_mute.integer));
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   um  unmute       <id>          ^7[^3Unmute a player^7]\n\"", g_mute.integer));
		}
	if (adm >= g_plant.integer && g_plant.integer != 5 && g_plant.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   pl  plant        <id>          ^7[^3Plant a player in the ground^7]\n\"", g_plant.integer));
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   upl unplant      <id>          ^7[^3Unplant a planted player^7]\n\"", g_plant.integer));
		}
	if (adm >= g_burn.integer && g_burn.integer != 5 && g_burn.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   b   burn         <id>          ^7[^3Burn a player^7]\n\"", g_burn.integer));
		}
	if (adm >= g_eventeams.integer && g_eventeams.integer != 5 && g_eventeams.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   et  eventeams                  ^7[^3Make the teams even^7]\n\"", g_eventeams.integer));
		}
	/*if (adm >= g_333.integer && g_333.integer != 5){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   333 333                        ^7[^3Enable/Disable 333 FPS jumps^7]\n\"", g_333.integer));
		}*/
	if (adm >= g_sl.integer && g_sl.integer != 5 && g_sl.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   sl  scorelimit   <time>        ^7[^3Change the scorelimit^7]\n\"", g_sl.integer));
		}
	if (adm >= g_tl.integer && g_tl.integer != 5 && g_tl.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   tl  timelimit    <time>        ^7[^3Change the timelimit^7]\n\"", g_tl.integer));
		}
	if (adm >= g_nolower.integer && g_nolower.integer != 5 && g_nolower.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   nl  nolower                    ^7[^3Enable/Disable Nolower^7]\n\"", g_nolower.integer));
		}
	if (adm >= g_nades.integer && g_nades.integer != 5 && g_nades.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   nd  normaldamage               ^7[^3Toggle Normal damage^7]\n\"", g_nades.integer));
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   rd  realdamage                 ^7[^3Toggle Real damage^7]\n\"", g_nades.integer));
		}
	if (adm >= g_ri.integer && g_ri.integer != 5 && g_ri.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   ri  ri           <time>        ^7[^3Change the respawn interval^7]\n\"", g_ri.integer));
		}
	if (adm >= g_gr.integer && g_gr.integer != 5 && g_gr.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   gr  gametyperestart            ^7[^3Restart the current gametype^7]\n\"", g_gr.integer));
		}
	if (adm >= g_clanvsall.integer && g_clanvsall.integer != 5 && g_clanvsall.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   cva clanvsall                  ^7[^3Clan versus other players^7]\n\"", g_clanvsall.integer));
		}
	if (adm >= g_swapteams.integer && g_swapteams.integer != 5 && g_swapteams.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   sw  swapteams                  ^7[^3Swap the players from both teams^7]\n\"", g_swapteams.integer));
		}
	if (adm >= g_lock.integer && g_lock.integer != 5 && g_lock.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   l   lock         <team>        ^7[^3Lock a team^7]\n\"", g_lock.integer));
		}
	if (adm >= g_clan.integer && g_clan.integer != 5 && g_clan.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   acl addclan      <id>          ^7[^3Add a clan member^7]\n\"", g_clan.integer));
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   rcl removeclan   <id>          ^7[^3Remove a clan member^7]\n\"", g_clan.integer));
		}
	if (adm >= g_broadcast.integer && g_broadcast.integer != 5 && g_broadcast.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   br  broadcast    <message>     ^7[^3Broadcast a message^7]\n\"", g_broadcast.integer));
		}
	if (adm >= g_forceteam.integer && g_forceteam.integer != 5 && g_forceteam.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   ft  forceteam    <team>        ^7[^3Force a player to join a team^7]\n\"", g_forceteam.integer));
		}
	if (adm >= g_nades.integer && g_nades.integer != 5 && g_nades.integer == level){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   nn  nonades                    ^7[^3Enable or disable nades^7]\n\"", g_forceteam.integer));
		}
	// temp entry
	/*if (adm >= 4 && level == 4){
		trap_SendServerCommand( ent-g_entities, va("print \" [^34^7]       adminspec                  ^7[^3Allows you to spec an enemy player^7]\n\""));
	}*/
	// Boe!Man 9/21/10: End of Loop.
	}
	trap_SendServerCommand( ent-g_entities, va("print \"    [^32^7] B-Admin          ^7[^33^7] Admin          ^7[^34^7] S-Admin\n\""));
	trap_SendServerCommand( ent-g_entities, va("print \" \n^7Use ^3[Page Up]^7 and ^3[Page Down]^7 keys to scroll.\n\""));
	}
	return;
	}
	
	// Henk loop through my admin command array
	
	for(i=0;i<AdminCommandsSize;i++){ // Henk 15/09/10 -> Fixed loop going outside array(causing crashes)
		//Com_Printf("Checking %s with %s\n", arg1, AdminCommands[i].adminCmd);
		if(!Q_stricmp(arg1, AdminCommands[i].adminCmd)){
			if(ent->client->sess.admin >= *AdminCommands[i].adminLevel){
				AdminCommands[i].Function(2, ent, qfalse);
				return;
			}else{
			// Boe!Man 12/30/09: Putting two Info messages together.
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7`Your admin level is too low to use this command.\n\""));
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
	//Ryan
//	char arg1[64];
	//Ryan

	trap_Argv( 0, cmd, sizeof( cmd ) );

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
