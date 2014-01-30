// Copyright (C) 2001-2002 Raven Software
//
// g_gametype.c -- dynamic gametype handling

#include "g_local.h"
#include "q_shared.h"
#include "g_team.h"
#include "inv.h"
#include "boe_local.h"

#define	MAX_GAMETYPE_SPAWN_POINTS	32
///RxCxW - 01.18.06 - 07:34pm #logging
///CJJ - 1.1.2005 - Logging Flag Captures
#define TRIGGER_REDFLAGCAPTURE      200
#define TRIGGER_BLUEFLAGCAPTURE      201
#define TRIGGER_CASECAPTURE      200
///CJJ - 1.1.2005 #END

int			g_gametypeItemCount = 0;
vec3_t		g_effectOrigin;

/*QUAKED gametype_player (0 1 0) (-16 -16 -46) (16 16 48) REDTEAM BLUETEAM
Potential spawning position for red or blue team in custom gametype games.
*/
void SP_gametype_player ( gentity_t *ent ) 
{
	team_t	team;

	// Cant take any more spawns!!
	if ( level.spawnCount >= MAX_SPAWNS )
	{
		G_FreeEntity ( ent );
		return;
	}
	
	// If a team filter is set then override any team settings for the spawns
	if ( level.mTeamFilter[0] )
	{
		if ( Q_stricmp ( level.mTeamFilter, "red") == 0 )
		{
			team = TEAM_RED;
		}
		else if ( Q_stricmp ( level.mTeamFilter, "blue") == 0 )
		{
			team = TEAM_BLUE;
		}
		else
		{
			G_FreeEntity ( ent );
			return;
		}
	}
	else
	{
		// Red team only
		if ( ent->spawnflags & 0x1 )
		{
			team = TEAM_RED;
		}
		else if ( ent->spawnflags & 0x2 )
		{
			team = TEAM_BLUE;
		}
	}

	G_AddClientSpawn ( ent, team );

	G_FreeEntity ( ent );
}

void SP_mission_player ( gentity_t* ent )
{
	ent->classname = "gametype_player";

	SP_gametype_player ( ent );
}

void gametype_trigger_use ( gentity_t *self, gentity_t *other, gentity_t *activator ) 
{
	if ( level.gametypeResetTime )
	{
		return;
	}
}

void gametype_trigger_touch ( gentity_t *self, gentity_t *other, trace_t *trace ) 
{
	if ( level.gametypeResetTime )
	{
		return;
	}
	
	if ( trap_GT_SendEvent ( GTEV_TRIGGER_TOUCHED, level.time, self->health, other->s.number, other->client->sess.team, 0, 0 ) )
	{
		G_UseTargets ( self, other );
	}
}
/*QUAKED gametype_trigger (0 0 .8) ? 
*/
void SP_gametype_trigger ( gentity_t* ent )
{
	// The target name is only used by the gametype system, so it shouldnt
	// be adjusted by the RMG
	if ( level.mTargetAdjust )
	{
		if ( ent->targetname )
			ent->targetname = strchr ( ent->targetname, '-' ) + 1;
	}

	/*
	///RxCxW - 09.30.06 - 08:17pm #spMaps
	if(!Q_stricmp(ent->model, "NV_MODEL")){
		if (!VectorCompare (ent->s.angles, vec3_origin))
			G_SetMovedir (ent->s.angles, ent->movedir);
		ent->r.contents = CONTENTS_TRIGGER;
		ent->r.svFlags = SVF_NOCLIENT;
		//ent->s.eType = ET_GAMETYPE_TRIGGER;
		return;
	}
	*/

	InitTrigger (ent);
}
static gentity_t* G_RealSpawnGametypeItem ( gentity_t* ent, qboolean dropped )
{
	gentity_t* it_ent;
	
	it_ent = G_Spawn();

	it_ent->flags |= FL_DROPPED_ITEM;
	it_ent->item = ent->item;

	VectorCopy( ent->r.currentOrigin, it_ent->s.origin );
	VectorCopy ( ent->s.angles, it_ent->s.apos.trBase );
	it_ent->classname = ent->item->classname;
	G_SpawnItem ( it_ent, it_ent->item );
	FinishSpawningItem(it_ent);	
	
	VectorSet( it_ent->r.mins, -ITEM_RADIUS * 4 / 3, -ITEM_RADIUS * 4 / 3, -ITEM_RADIUS );
	VectorSet( it_ent->r.maxs, ITEM_RADIUS * 4 / 3, ITEM_RADIUS * 4 / 3, ITEM_RADIUS );

	// Red team only
	if ( ent->s.eFlags & EF_REDTEAM )
	{
		it_ent->s.eFlags |= EF_REDTEAM;
	}
	
	if ( ent->s.eFlags & EF_BLUETEAM )
	{
		it_ent->s.eFlags |= EF_BLUETEAM;
	}

	return it_ent;
}

gentity_t* G_RealSpawnGametypeItem1 ( gitem_t* item, vec3_t origin, vec3_t angles, qboolean dropped ) // used to spawn briefcase at a player
{
	gentity_t* it_ent;

	it_ent = G_Spawn();

	it_ent->flags |= FL_DROPPED_ITEM;
	it_ent->item = item;

	VectorCopy( origin, it_ent->s.origin );
	VectorCopy ( angles, it_ent->s.apos.trBase );
	VectorCopy ( angles, it_ent->s.angles );
	it_ent->classname = item->classname;
	G_SpawnItem ( it_ent, it_ent->item );
	FinishSpawningItem(it_ent);	
	
	VectorSet( it_ent->r.mins, -ITEM_RADIUS * 4 / 3, -ITEM_RADIUS * 4 / 3, -ITEM_RADIUS );
	VectorSet( it_ent->r.maxs, ITEM_RADIUS * 4 / 3, ITEM_RADIUS * 4 / 3, ITEM_RADIUS );

	return it_ent;
}

gentity_t* G_SpawnGametypeItem ( const char* pickup_name, qboolean dropped )
{
	gentity_t* ent;

	// Look for the gametype item in the map
	ent = NULL;
	while ( NULL != (ent = G_Find ( ent, FOFS(classname), "gametype_item" ) ) )
	{
		// Match?
		if ( !Q_stricmp ( ent->item->pickup_name, pickup_name ) )
		{
			break;
		}
	}
	
	// If we couldnt find the item spawner then we have a problem	
	if ( !ent )
	{
		Com_Error ( ERR_FATAL, "Could not spawn gametype item '%s'\n", pickup_name );
		return NULL;
	}		

	return G_RealSpawnGametypeItem ( ent, dropped );
}

void G_GametypeItemThink ( gentity_t* ent )
{
	G_RealSpawnGametypeItem ( ent, qfalse );
}

/*QUAKED gametype_item (0 0 1) (-16 -16 -16) (16 16 16) 
"name"			name of the item to spawn (defined in gametype script)
*/
void SP_gametype_item ( gentity_t* ent )
{
	// TEMPORARY HACK
	if ( level.mTargetAdjust )
	{
		// Boe!Man 2/11/13: Safe target(name) checking.
		if(ent->targetname && ent->targetname[0]){
			if(strstr(ent->targetname, "-")){
				ent->targetname = strchr ( ent->targetname, '-' ) + 1;
			}else{
				Q_strncpyz(ent->targetname, ent->targetname, sizeof(ent->targetname));
			}
		}
		
		if(ent->target && ent->target[0]){
			if(strstr(ent->target, "-")){
				ent->target = strchr ( ent->target, '-' ) + 1;
			}else{
				Q_strncpyz(ent->target, ent->target, sizeof(ent->target));
			}
		}
	}
	
	G_SetOrigin( ent, ent->s.origin );
}

/*
===============
G_ResetGametypeItem
===============
*/
void G_ResetGametypeItem ( gitem_t* item )
{
	gentity_t *find;
	int		  i;
	// Convience check
	if ( !item )
	{
		return;
	}

	if(current_gametype.value == GT_HS || current_gametype.value == GT_HZ){
		Com_Printf("Briefcase has disappeared\n");
		//ASDKJASKJDKASDJKSJDSKJDSKDD
		return;
	}
	// Remove all spawned instances of the item on the map
	find = NULL;
	while ( NULL != (find = G_Find ( find, FOFS(classname), item->classname ) ) )
	{
		// Free this entity soon
		find->nextthink = level.time + 10;
		find->think = G_FreeEntity;
	}

	// Strip the item from all connected clients
	for ( i = 0; i < level.numConnectedClients; i ++ )
	{
		g_entities[level.sortedClients[i]].client->ps.stats[STAT_GAMETYPE_ITEMS] &= ~(1<<item->giTag);
	}

	// Respawn the item in all of its locations
	find = NULL;
	while ( NULL != (find = G_Find ( find, FOFS(classname), "gametype_item" ) ) )
	{
		if ( find->item != item )
		{
			continue;
		}

		G_RealSpawnGametypeItem ( find, qfalse );
	}
}

/*
===============
G_ResetGametypeEntities
===============
*/
void G_ResetGametypeEntities ( void )
{
	gentity_t* find;
	qboolean   initVisible;

	// Show total time
	initVisible = qtrue;
	trap_SetConfigstring ( CS_GAMETYPE_TIMER, "0" );

	// Reset all of the gametype items.  This must be done last because the spawn
	// function may alter enabled states of triggers or scripts
	find = NULL;
	while ( NULL != (find = G_Find ( find, FOFS(classname), "gametype_item" ) ) )
	{
		G_ResetGametypeItem ( find->item );
	}
}

/*
===============
G_RespawnClients
===============
*/
void G_RespawnClients ( qboolean force, team_t team, qboolean fullRestart )
{
	int i;

	// Respawn all clients back at a spawn pointer
	for ( i = 0; i < level.numConnectedClients; i ++ )
	{
		// Save the clients weapons
		playerState_t	ps;
		gentity_t*		ent;
		qboolean		ghost;

		ent = &g_entities[level.sortedClients[i]];

		// Make sure they are actually connected
		if ( ent->client->pers.connected != CON_CONNECTED )
		{
			continue;
		}

		// Save the entire player state so certain things
		// can be maintained across rounds
		ps = ent->client->ps;

		ghost = ent->client->sess.ghost;

		// Spawn the requested team
		if ( ent->client->sess.team != team )
		{
			continue;
		}

		// Dont force them to respawn if they arent a ghost
		if ( !ghost && !force )
		{
			continue;
		}

		// If the client is a ghost then revert them
		if ( ent->client->sess.ghost )
		{
			// Clean up any following monkey business
			G_StopFollowing ( ent );

			// Disable being a ghost
			ent->client->ps.pm_flags &= ~PMF_GHOST;
			ent->client->ps.pm_type = PM_NORMAL;
			ent->client->sess.ghost = qfalse;
		}
		
		if(current_gametype.value == GT_HS){
			ent->client->sess.mdnAttempts = 0;
			ent->client->sess.cageAttempts = 0;
		}

		ent->client->sess.noTeamChange = qfalse;
		ent->client->sess.roundkills = 0;
		//ent->client->sess.fpschecks = 0;
		trap_UnlinkEntity (ent);
		ClientSpawn ( ent );

		if ( fullRestart )
		{
			ent->client->sess.score = 0;
			ent->client->sess.kills = 0;
			ent->client->sess.deaths = 0;
			ent->client->sess.teamkillDamage = 0;
			ent->client->sess.teamkillForgiveTime = 0;
			ent->client->pers.enterTime = level.time;
			ent->client->sess.timeOfDeath = 0;
			ent->client->sess.cagescore = 0;
		}
	}
}

void G_WallUse(gentity_t *self, gentity_t *other, gentity_t *activator);
/*
===============
G_ResetPickups
===============
*/
void G_ResetEntities ( void )
{
	int i;

	// Run through all the entities in the level and reset those which
	// need to be reset
	for ( i = 0; i < level.num_entities; i ++ )
	{
		gentity_t* ent;

		ent = &g_entities[i];

		if(current_gametype.value == GT_HS){
			if(ent->classname){
				if(strstr(ent->classname, "1fx_play_effect")){ // Henk 19/02/10 -> Remove effects every new round
					G_FreeEntity(ent);
					continue;
				}
			}
		}

		// Skip entities not in use
		if ( !ent->inuse )
		{
			continue;
		}

		// If this is a player then unlink it so then clients
		// spawned in dont telefrag
		if ( ent->s.eType == ET_PLAYER || ent->s.eType == ET_BODY )
		{
			trap_UnlinkEntity ( ent );
		}
		// If this is a missile
		else if ( ent->s.eType == ET_MISSILE )
		{
			G_FreeEntity ( ent );
		}
		// If the dropped flag is set then free it
		else if ( ent->flags & FL_DROPPED_ITEM )
		{
			G_FreeEntity ( ent );
		}
		// If this item is waiting to be respawned, then respawn it
		else if ( ent->think == RespawnItem )
		{
			RespawnItem ( ent );
		}
		else if ( ent->s.eType == ET_DAMAGEAREA )
		{
			G_FreeEntity ( ent );
		}else if ( ent->s.eType == ET_MOVER && ent->use != NULL && ent->use == G_WallUse )
		{
			if ( ent->spawnflags & 1 )
			{
				trap_UnlinkEntity ( ent );
			}
			else
			{
				trap_LinkEntity ( ent );
			}
		}
	}
}

void ResetCages(){
	int i;
	for ( i = 0; i < level.num_entities; i ++ )
	{
		gentity_t* ent;

		ent = &g_entities[i];

		if(ent->hideseek == 1){
			G_FreeEntity(ent);
		}

	}
}

/*
===============
G_ResetGametype
===============
*/
void G_ResetGametype ( qboolean fullRestart, qboolean cagefight )
{
	gentity_t*	tent;
	int i;
	// Reset the glass in the level
	G_ResetGlass ( );

	// Reset all pickups in the world
	G_ResetEntities ( );

	ResetCages();

	if(current_gametype.value == GT_HZ){
		for ( i = 0; i < level.numConnectedClients; i ++ )
		{
			gentity_t* other = &g_entities[level.sortedClients[i]];

			if ( other->client->pers.connected != CON_CONNECTED )
			{
				continue;
			}
			if ( other->client->sess.team == TEAM_SPECTATOR )
			{
				continue;
			}
			other->client->sess.zombie = qfalse;
			other->client->sess.zombiebody = -1;
			other->client->sess.firstzombie = qfalse;
			SetTeam(other, "red", NULL, qtrue);
		}
	}

	// Reset the gametype itself
	G_ResetGametypeEntities ( );
	//memset(level.deadClients, 0, sizeof(level.deadClients)); // reset
	level.cagefight = qfalse;
	level.aetdone = qfalse;
	level.zombie = -1;
	level.cagefighttimer = 0;
	trap_Cvar_VariableStringBuffer ( "mapname", level.mapname, MAX_QPATH );
	// Cant have a 0 roundtimelimit
	if ( g_roundtimelimit.integer < 1 )
	{
		trap_Cvar_Set ( "g_roundtimelimit", "1" );
		trap_Cvar_Update ( &g_roundtimelimit );
	}
	///End  - 09.09.06 - 11:24pm

	// Boe!Man 11/19/10: Did the round just start?
	if (g_compMode.integer > 0 && cm_enabled.integer > 1){
		if (level.teamScores[TEAM_RED] == level.teamScores[TEAM_BLUE] && level.teamScores[TEAM_RED] == 0){
		// Boe!Man 11/17/10: Lock the teams again because we just performed a map restart.	
			trap_SendServerCommand(-1, va("print \"^3[Auto Action] ^7Teams have been locked.\n\""));
			if (cm_slock.integer == 1){
				level.specsLocked = 1;
			}
			level.blueLocked = 1;
			level.redLocked = 1;
		}
	}

	// Initialize the respawn interval since this is a interval gametype
	switch ( level.gametypeData->respawnType )
	{
		case RT_INTERVAL:
			level.gametypeRespawnTime[TEAM_RED]  = 0;
			level.gametypeRespawnTime[TEAM_BLUE] = 0;
			level.gametypeRespawnTime[TEAM_FREE] = 0;
			break;

		case RT_NONE:
			if(current_gametype.value == GT_HS){
				level.gametypeDelayTime = level.time + hideSeek_roundstartdelay.integer * 1000;
			}else{
				level.gametypeDelayTime = level.time + g_roundstartdelay.integer * 1000;
			}
			
			if(current_gametype.value == GT_HS){
				if(cagefight == qtrue){
					level.gametypeRoundTime = level.time + (1*60000);
				}else{
					level.gametypeRoundTime = level.time + (g_roundtimelimit.integer * 60000); // Henk 22/01/10 -> Round time without startup delay // + g_roundstartdelay.integer * 1000;
				}
				
				if(level.crossTheBridge){
					level.gametypeDelayTime = level.gametypeRoundTime;
				}
			}else{
				level.gametypeRoundTime = level.time + (g_roundtimelimit.integer * 60000);
			}

			if ( level.gametypeDelayTime != level.time )
			{
				// Boe!Man 11/19/10: Messages and actions for the two rounds.
				if (g_compMode.integer > 0 && cm_enabled.integer > 1){
					// Boe!Man 11/19/10: Messages for round one.
					if (cm_enabled.integer <= 3){
						if (level.teamScores[TEAM_RED] == level.teamScores[TEAM_BLUE]){
							trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sS%sc%so%sr%se tied with %i - %i!", level.time + (g_roundstartdelay.integer * 1000), server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE]));
						}
						else if (level.teamScores[TEAM_RED] > level.teamScores[TEAM_BLUE]){
							trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%s ^7team leads with %i - %i!", level.time + (g_roundstartdelay.integer * 1000), server_redteamprefix.string, level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE]));}
						else if (level.teamScores[TEAM_BLUE] > level.teamScores[TEAM_RED]){
							trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%s ^7team leads with %i - %i!", level.time + (g_roundstartdelay.integer * 1000), server_blueteamprefix.string, level.teamScores[TEAM_BLUE], level.teamScores[TEAM_RED]));}
					}
					// Boe!Man 11/19/10: Messages for round two.
					else if(cm_enabled.integer > 3){
						if (level.teamScores[TEAM_RED]+cm_sr.integer == level.teamScores[TEAM_BLUE]+cm_sb.integer){
							trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sS%sc%so%sr%se tied with %i - %i!", level.time + (g_roundstartdelay.integer * 1000), server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, level.teamScores[TEAM_RED]+cm_sr.integer, level.teamScores[TEAM_BLUE]+cm_sb.integer));
						}
						else if (level.teamScores[TEAM_RED]+cm_sr.integer > level.teamScores[TEAM_BLUE]+cm_sb.integer){
							trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%s ^7team leads with %i - %i!", level.time + (g_roundstartdelay.integer * 1000), server_redteamprefix.string, level.teamScores[TEAM_RED]+cm_sr.integer, level.teamScores[TEAM_BLUE]+cm_sb.integer));}
						else if (level.teamScores[TEAM_BLUE]+cm_sb.integer > level.teamScores[TEAM_RED]+cm_sr.integer){
							trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%s ^7team leads with %i - %i!", level.time + (g_roundstartdelay.integer * 1000), server_blueteamprefix.string, level.teamScores[TEAM_BLUE]+cm_sb.integer, level.teamScores[TEAM_RED]+cm_sr.integer));}
					}
				}
				// Boe!Man 11/19/10: Not in compMode. Just switch to the regular Get Ready msg.
				else{
					// Get Ready
					if(current_gametype.value != GT_HS && current_gametype.value != GT_HZ)
					trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sG%se%st %sr%se%sa%sdy!", level.time + (g_roundstartdelay.integer * 1000), server_color1.string, server_color2.string, server_color3.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
				}
			}

			if (g_autoEvenTeams.integer == 1 && current_gametype.value != GT_HS){
				EvenTeams(NULL, qtrue);
			}
			
			// Boe!Man 1/30/14: We're in a gametype with rounds and the user wishes to have backups of the databases enabled, do this every start of another round.
			if(sql_automaticBackup.integer){
				Boe_backupInMemoryDbs("users.db", usersDb);
				Boe_backupInMemoryDbs("aliases.db", aliasesDb);
				Boe_backupInMemoryDbs("bans.db", bansDb);
			}
			
			trap_SetConfigstring ( CS_GAMETYPE_TIMER, va("%i", level.gametypeRoundTime) );
			break;
	}

	level.gametypeJoinTime  = 0;

	// Allow people to join 20 seconds after both teams have people on them
	if ( !level.gametypeData->teams )
	{
		level.gametypeJoinTime = level.time;
	}

	// Respawn all clients
	G_RespawnClients ( qtrue, TEAM_RED, fullRestart );
	G_RespawnClients ( qtrue, TEAM_BLUE, fullRestart );

	level.gametypeStartTime = level.time;
	level.gametypeResetTime = 0;

	if(current_gametype.value == GT_HZ){
		level.messagedisplay = qfalse;
		level.messagedisplay1 = qfalse;
		level.messagedisplay2 = qfalse;
	}

	if(current_gametype.value == GT_HS){
		// Henk 19/01/10 -> Reset level variables
		level.cagefightdone = qfalse;
		level.lastaliveCheck[0] = qfalse;
		level.lastaliveCheck[1] = qfalse;
		level.messagedisplay = qfalse;
		level.messagedisplay1 = qfalse;
		level.MM1given = qfalse;
		level.rememberSeekKills = level.SeekKills;
		level.SeekKills = 0;
		level.MM1Time = 0;
		level.RPGTime = 0;
		level.M4Time = 0;
		strcpy(level.cagewinner, "none");
		level.timelimithit = qfalse; // allow timelimit hit message
		// Henkie 23/02/10 -> Cache the sounds' index
		level.clicksound = G_SoundIndex("sound/misc/menus/click.wav");

		// Boe!Man 6/29/11: Also set the appropriate message if the weapon's been disabled.
		if(hideSeek_Weapons.string[1] == '0'){ // M4
			Com_sprintf(level.M4loc, sizeof(level.M4loc), "%s", "Disabled");
		}else{
			Com_sprintf(level.M4loc, sizeof(level.M4loc), "%s", "Not given yet");
		}
		if(hideSeek_Weapons.string[0] == '0'){ // RPG
			Com_sprintf(level.RPGloc, sizeof(level.RPGloc), "%s", "Disabled");
		}else{
			Com_sprintf(level.RPGloc, sizeof(level.RPGloc), "%s", "Not given yet");
		}
		if(hideSeek_Weapons.string[2] == '0'){ // MM1
			Com_sprintf(level.MM1loc, sizeof(level.MM1loc), "%s", "Disabled");
		}else{
			Com_sprintf(level.MM1loc, sizeof(level.MM1loc), "%s", "Not given yet");
		}
		
		if(TeamCount1(TEAM_RED) == 0){
			level.lastalive[0] = -1;
			level.lastalive[1] = -1;
		}else if(TeamCount1(TEAM_RED) <= 2){
			level.lastalive[1] = -1;
		}

		if ( fullRestart )
		{
			level.lastalive[0] = -1;
			level.lastalive[1] = -1;
			level.lastseek = -1;
			level.warmupTime = 0;
			level.startTime = level.time;
			memset ( level.teamScores, 0, sizeof(level.teamScores) );
			trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );
			trap_SetConfigstring( CS_LEVEL_START_TIME, va("%i", level.startTime ) );	
		}
	}
	
	// Reset the clients local effects
	tent = G_TempEntity( vec3_origin, EV_GAMETYPE_RESTART );
	tent->r.svFlags |= SVF_BROADCAST;

	// Start the gametype
	trap_GT_Start ( level.gametypeStartTime );

}

/*
===============
G_ParseGametypeItems
===============
*/
qboolean G_ParseGametypeItems ( TGPGroup* itemsGroup )
{
	TGPGroup	itemGroup;
	int			itemCount;
	char		temp[MAX_QPATH];
	gentity_t	*ent;

	// Handle NULL for convienience
	if ( !itemsGroup )
	{
		return qfalse;
	}

	// Loop over all the items and add each 
	itemGroup = trap_GPG_GetSubGroups ( itemsGroup );
	itemCount = 0;

	while ( itemGroup )
	{	
		gitem_t*   item;
		
		// Parse out the pickup name
		trap_GPG_GetName ( itemGroup, temp );

		item = BG_FindItem ( temp );
		if ( !item )
		{
			item = &bg_itemlist[ MODELINDEX_GAMETYPE_ITEM + itemCount ];
#ifdef _TRUEMALLOC
			trap_TrueMalloc((void **)&item->pickup_name, sizeof(temp));
			if(item->pickup_name){
				strcpy((char *)item->pickup_name, temp);
			}
#else
			item->pickup_name = (char *)trap_VM_LocalStringAlloc ( temp );
#endif
			itemCount++;
		}

		// Handle the entity specific stuff by finding all matching items that 
		// were spawned.
		ent = NULL;
		while ( NULL != (ent = G_Find ( ent, FOFS(targetname), item->pickup_name ) ) )
		{
			// If not a gametype item then skip it
			if ( Q_stricmp ( ent->classname, "gametype_item" ) )
			{
				continue;
			}
			// Setup the gametype data
			ent->item	   = item;
			ent->nextthink = level.time + 200;
			ent->think     = G_GametypeItemThink;
		}

		// Next sub group
		itemGroup = trap_GPG_GetNext(itemGroup);
	}

#ifdef _SOF2_BOTS
	// GRIM 10/06/2002 6:18PM
	level.gtItemCount = itemCount;
	// GRIM
#endif

	return qtrue;
}

/*
===============
G_ParseGametypeFile
===============
*/
qboolean G_ParseGametypeFile ( void )
{
	TGenericParser2 GP2;
	TGPGroup		topGroup;
	TGPGroup		gametypeGroup;
	char			value[4096];

	// Default the gametype config strings
	trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, "0,0" );

	// Open the given gametype file
	GP2 = trap_GP_ParseFile ( (char*)level.gametypeData->script, qtrue, qfalse );
	if (!GP2)
	{
		return qfalse;
	}

	// Grab the top group and the list of sub groups
	topGroup = trap_GP_GetBaseParseGroup(GP2);
	gametypeGroup = trap_GPG_FindSubGroup(topGroup, "gametype" );
	if ( !gametypeGroup )
	{
		trap_GP_Delete(&GP2);
		return qfalse;
	}

	// Look for the respawn type
	trap_GPG_FindPairValue ( gametypeGroup, "respawn", "normal", value );
	if ( !Q_stricmp ( value, "none" ) )
	{
		level.gametypeData->respawnType = RT_NONE;
	}
	else if ( !Q_stricmp ( value, "interval" ) )
	{
		level.gametypeData->respawnType = RT_INTERVAL;
	}
	else
	{
		level.gametypeData->respawnType = RT_NORMAL;
	}

	// Grab the defined items
	G_ParseGametypeItems ( (TGPGroup *)trap_GPG_FindSubGroup ( gametypeGroup, "items" ) );

	// Free up the parser
	trap_GP_Delete(&GP2);

	return qtrue;
}

//RxCxW - 02.03.05 - 11:30am - #DropGametypeItems #GOLD
/*
=================
G_EnableGametypeItemPickup

Drops all of the gametype items held by the player
=================
*/
void G_EnableGametypeItemPickup ( gentity_t* ent )
{
	ent->s.eFlags &= ~EF_NOPICKUP;
}
/*
=================
G_DropGametypeItems

Drops all of the gametype items held by the player
=================
*/
void G_DropGametypeItems ( gentity_t* self, int delayPickup )
{
	float		angle;
	int			i;
	gentity_t	*drop;
	gitem_t		*item;

	// drop all custom gametype items
	angle = 0;
	for ( i = 0 ; i < MAX_GAMETYPE_ITEMS ; i++ ) 
	{
		// skip this gametype item if the client doenst have it
		if ( !(self->client->ps.stats[STAT_GAMETYPE_ITEMS] & (1<<i)) ) 
		{
			continue;
		}

		item = BG_FindGametypeItem ( i );
		if ( !item ) 
		{
			continue;
		}

		drop = G_DropItem( self, item, angle );
		drop->count = 1;
		angle += 45;

		if ( delayPickup )
		{
			drop->nextthink = level.time + delayPickup;	
			drop->s.eFlags |= EF_NOPICKUP;
			drop->think = G_EnableGametypeItemPickup;
		}
		
		// TAke it away from the client just in case
		self->client->ps.stats[STAT_GAMETYPE_ITEMS] &= ~(1<<i);

		if ( self->enemy && self->enemy->client && !OnSameTeam ( self->enemy, self ) )
		{	//05.07.05 - 07:15pm
			//trap_GT_SendEvent ( GTEV_ITEM_DEFEND, level.time, level.gametypeItems[item->giTag].id, self->enemy->s.clientNum, self->enemy->client->sess.team, 0, 0  );
			trap_GT_SendEvent ( GTEV_ITEM_DEFEND, level.time, item->quantity, self->enemy->s.clientNum, self->enemy->client->sess.team, 0, 0  );
			//End  - 05.07.05 - 07:15pm
		}
	}

	self->client->ps.stats[STAT_GAMETYPE_ITEMS] = 0;
}
//End - 02.03.05 - 11:31am

/*
=================
CheckGametype
=================
*/
void CheckGametype ( void )
{

	// If the level is over then forget checking gametype stuff.
	//Ryan june 15 2003
	//if ( level.intermissiontime )
	if(current_gametype.value == GT_HS){
		if(level.startcage == qtrue){
			InitCagefight();
		}
	}
	if ( level.intermissiontime || level.pause || cm_enabled.integer == 3)
	//End Ryan
	{
		return;
	}

	// Dont bother starting the gametype until
	// the first person joins
	if ( !level.gametypeStartTime )
	{
		int i;

		memset ( level.teamAliveCount, 0, sizeof(level.teamAliveCount) );

		for ( i = 0; i < level.numConnectedClients; i ++ )
		{
			gentity_t* other = &g_entities[level.sortedClients[i]];

			if ( other->client->pers.connected != CON_CONNECTED )
			{
				continue;
			}
				
			if ( other->client->pers.connected != CON_CONNECTED )
			{
				continue;
			}

			level.teamAliveCount[other->client->sess.team]++;
		}

		if ( level.teamAliveCount[TEAM_RED] || level.teamAliveCount[TEAM_BLUE] || level.teamAliveCount[TEAM_FREE] )
		{
			G_ResetGametype ( qfalse, qfalse );
			return;
		}
	}
 
	// Check for delayed gametype reset
	if ( level.gametypeResetTime )
	{
		if ( level.time > level.gametypeResetTime )
		{
			// Dont do this again
			level.gametypeResetTime = 0;

			G_ResetGametype ( qfalse, qfalse );
		}

		return;
	}

	// Handle respawn interval spawning
	if(level.gametypeData){
		if ( level.gametypeData->respawnType == RT_INTERVAL )
		{
			int team;
			qboolean autoETDone = qfalse;

			for ( team = TEAM_RED; team < TEAM_SPECTATOR; team ++ )
			{
				if ( level.gametypeRespawnTime[team] && level.time > level.gametypeRespawnTime[team] )
				{
					if(!autoETDone && g_autoEvenTeams.integer){
						EvenTeams(NULL, qtrue);
					}
					autoETDone = qtrue;
					
					// Respawn all dead clients
					G_RespawnClients ( qfalse, (team_t)team, qfalse );

					// Next interval
					level.gametypeRespawnTime[team] = 0;
				}
			}
		}
	}
	
	if(level.gametypeData){
	// If we are in RT_NONE respawn mode then we need to look for everyone being dead
	if ( level.gametypeData->respawnType == RT_NONE && level.gametypeStartTime ){
		int i;
		int alive[TEAM_NUM_TEAMS];
		int dead[TEAM_NUM_TEAMS];
		int players[TEAM_NUM_TEAMS];

		//Ryan & Dragon
		memset ( &level.teamAliveCount[0], 0, sizeof(level.teamAliveCount) );
		//Ryan & Dragon

		memset ( &alive[0], 0, sizeof(alive) );
		memset ( &dead[0], 0, sizeof(dead) );
		memset ( &players[0], 0, sizeof(players) );
		for ( i = 0; i < level.numConnectedClients; i ++ )
		{
			gentity_t* ent = &g_entities[level.sortedClients[i]];

			if ( ent->client->pers.connected != CON_CONNECTED )
			{
				continue;
			}

			if(ent->client->ps.weapon == WP_NONE && !G_IsClientDead(ent->client) && current_gametype.value != GT_HS && ent->client->sunRespawnTimer == 0 && ent->client->sess.team != TEAM_SPECTATOR){ // Boe!Man 6/14/11: Don't respawn them if they're waiting to be respawned with the sun condition.
				trap_UnlinkEntity (ent);
				ClientSpawn(ent);
				//trap_SendServerCommand( -1, va("print \"^3[Info] ^7%s has been respawned due to being spawned with no weapons.\n\"", ent->client->pers.netname));
			}

			players[ent->client->sess.team] ++;

			if ( G_IsClientDead ( ent->client ) )
			{
				dead[ent->client->sess.team] ++;
			}
			else
			{
				alive[ent->client->sess.team] ++;
				level.teamAliveCount[ent->client->sess.team] ++;
			}
		}

		if(alive[TEAM_RED] == 1 && level.redMsgCount == 0 && players[TEAM_RED] >= 2 && current_gametype.value != GT_HS){
			for ( i = 0; i < level.numConnectedClients; i ++ ){
				gentity_t* ent = &g_entities[level.sortedClients[i]];
				if ( ent->client->sess.team == TEAM_RED && alive[TEAM_RED] == 1 &&
					!G_IsClientDead ( ent->client ) && !level.redMsgCount){
						trap_SendServerCommand ( ent->s.number, va("cp\"@You are the %sl%sa%ss%st %sp%sl%sayer alive!", server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color4.string, server_color5.string, server_color6.string));
						trap_SendServerCommand(-1, va("print\"^3[Info] ^7%s is the last alive player in the red team.\n\"", ent->client->pers.netname));
						Boe_ClientSound(ent, G_SoundIndex("sound/misc/events/tut_door01.mp3"));
						level.redMsgCount++;
						if(current_gametype.value == GT_HZ){
							trap_SendServerCommand(-1, va("print\"^3[Debug] ^7%s should get the M67 at the next round...\n\"", ent->client->pers.netname));
						}
				}
			}
		}

		if(alive[TEAM_BLUE] == 1 && level.blueMsgCount == 0 && players[TEAM_BLUE] >= 2 && current_gametype.value != GT_HS && current_gametype.value != GT_HZ){
			for ( i = 0; i < level.numConnectedClients; i ++ ){
				gentity_t* ent = &g_entities[level.sortedClients[i]];
				if ( ent->client->sess.team == TEAM_BLUE && alive[TEAM_BLUE] == 1 &&
					!G_IsClientDead ( ent->client ) && !level.blueMsgCount){
						trap_SendServerCommand ( ent->s.number, va("cp\"@You are the %sl%sa%ss%st %sp%sl%sayer alive!", server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color4.string, server_color5.string, server_color6.string));
						trap_SendServerCommand(-1, va("print\"^3[Info] ^7%s is the last alive player in the blue team.\n\"", ent->client->pers.netname));
						Boe_ClientSound(ent, G_SoundIndex("sound/misc/events/tut_door01.mp3"));
						level.blueMsgCount++;
				}
			}
		}

		if(current_gametype.value == GT_HZ && alive[TEAM_RED] == 0 && TeamCount1(TEAM_RED)+TeamCount1(TEAM_BLUE) >= 2){
			trap_GT_SendEvent ( GTEV_TEAM_ELIMINATED, level.time, TEAM_RED, 0, 0, 0, 0 );
		}

		// If everyone is dead on a team then reset the gametype, but only if 
		// there was someone on that team to begin with.
		if ( !alive[TEAM_RED] && dead[TEAM_RED] && current_gametype.value != GT_HZ )
		{	
			if(level.timelimithit == qtrue && level.cagefight != qtrue && (strstr(g_gametype.string, "inf") || strstr(g_gametype.string, "elim"))){
				gentity_t*	tent;
				int			tiedplayers;

				if(current_gametype.value == GT_HS){
					tiedplayers = TiedPlayers();
					if(tiedplayers < 2 || !level.cagefightloaded){
						tent = G_TempEntity( vec3_origin, EV_GAME_OVER );
						tent->s.eventParm = GAME_OVER_TIMELIMIT;
						tent->r.svFlags = SVF_BROADCAST;
						level.timelimithit = qfalse;
						#ifdef _DEBUG
						Com_Printf("Updating scores..\n");
						#endif
						UpdateScores();
						trap_GT_SendEvent ( GTEV_TEAM_ELIMINATED, level.time, TEAM_RED, 0, 0, 0, 0 ); // Boe!Man 9/6/11: Add this here to prevent the gametype not being properly ended (when timelimit's hit).
						LogExit( "Seekers have won the match" );
					}else{
						trap_SendServerCommand(-1, va("print \"^3[H&S] ^7%i hiders found with top score, starting cage round.\n\"", tiedplayers));
						level.cagefighttimer = level.time+3000;
						level.startcage = qtrue;
					}
				}else{
					tent = G_TempEntity( vec3_origin, EV_GAME_OVER );
					if(cm_enabled.integer > 0){ // Boe!Man 3/18/11: Only change the entry if competition mode's enabled.
						tent->s.eventParm = LEEG;
					}else{
						tent->s.eventParm = GAME_OVER_TIMELIMIT;
					}
					tent->r.svFlags = SVF_BROADCAST;

					if (g_compMode.integer > 0 && cm_enabled.integer > 1){
						Boe_compTimeLimitCheck();
					}else{	
						LogExit( "Timelimit Hit." );
					}
				}
			}
			trap_GT_SendEvent ( GTEV_TEAM_ELIMINATED, level.time, TEAM_RED, 0, 0, 0, 0 );
		}
		else if ( !alive[TEAM_BLUE] && dead[TEAM_BLUE] )
		{		
			if(level.timelimithit == qtrue && (strstr(g_gametype.string, "inf") || strstr(g_gametype.string, "elim"))){
				gentity_t*	tent;
				tent = G_TempEntity( vec3_origin, EV_GAME_OVER );
				tent->s.eventParm = GAME_OVER_TIMELIMIT;
				tent->r.svFlags = SVF_BROADCAST;
				level.timelimithit = qfalse;
				LogExit( "Blue team has been eliminated" );
			}
			trap_GT_SendEvent ( GTEV_TEAM_ELIMINATED, level.time, TEAM_BLUE, 0, 0, 0, 0 );
		}if(level.cagefight == qtrue && level.teamAliveCount[TEAM_RED] == 1){
			for ( i = 0; i < level.numConnectedClients; i ++ ){
				if(!G_IsClientDead(g_entities[level.sortedClients[i]].client) && g_entities[level.sortedClients[i]].client->sess.team == TEAM_RED){
					G_AddScore(&g_entities[level.sortedClients[i]], 100);
					strncpy(level.cagewinner, g_entities[level.sortedClients[i]].client->pers.netname, sizeof(level.cagewinner));
				}
			}
			trap_SendServerCommand(-1, va("print \"^3[H&S] ^7Fight ended.\n\""));
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,^7%sF%si%sg%sh%st %sended", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
			#ifdef _DEBUG
			Com_Printf("Updating scores..\n");
			#endif
			UpdateScores();
			level.timelimithit = qfalse;
			level.cagefight = qfalse;
			level.cagefightdone = qtrue;
			LogExit( "Someone has won the cagefight" );
			return;
		}
		// See if the time has expired
		if ( level.time > level.gametypeRoundTime )
		{
			if(level.cagefight == qtrue && current_gametype.value == GT_HS){
				for ( i = 0; i < level.numConnectedClients; i ++ ){
					if(!G_IsClientDead(g_entities[level.sortedClients[i]].client) && g_entities[level.sortedClients[i]].client->sess.team == TEAM_RED){
						G_AddScore(&g_entities[level.sortedClients[i]], 100);
						g_entities[level.sortedClients[i]].client->sess.kills += 1; // round winner should get 1 point more.
						strcpy(level.cagewinner, g_entities[level.sortedClients[i]].client->pers.netname);
					}
				}
				trap_SendServerCommand(-1, va("print \"^3[H&S] ^7Fight ended.\n\""));
				Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,^7%sF%si%sg%sh%st %sended", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));

				#ifdef _DEBUG
				Com_Printf("Updating scores..\n");
				#endif
				UpdateScores();
				level.timelimithit = qfalse;
				level.cagefight = qfalse;
				LogExit( "Timelimit hit" );
				return;
			}
			if(current_gametype.value == GT_HS){
				StripHiders(); 
			}
			trap_GT_SendEvent ( GTEV_TIME_EXPIRED, level.time, 0, 0, 0, 0, 0 );
			if(level.timelimithit == qtrue && (strstr(g_gametype.string, "inf") || strstr(g_gametype.string, "elim"))){
				gentity_t*	tent;
				int			tiedplayers;

				tent = G_TempEntity( vec3_origin, EV_GAME_OVER );

				if(cm_enabled.integer > 0){ // Boe!Man 3/18/11: Only change the entry if competition mode's enabled.
					tent->s.eventParm = LEEG;
				}else{
					tent->s.eventParm = GAME_OVER_TIMELIMIT;
				}
				tent->r.svFlags = SVF_BROADCAST;
				if(current_gametype.value == GT_HS){
					tiedplayers = TiedPlayers();
					if(tiedplayers < 2 || !level.cagefightloaded){
						#ifdef _DEBUG
						Com_Printf("Updating scores..\n");
						#endif
						UpdateScores();
						level.timelimithit = qfalse;
						LogExit( "Timelimit hit." );
					}else{
						trap_SendServerCommand(-1, va("print \"^3[H&S] ^7%i hiders found with top score, starting cage round.\n\"", tiedplayers));
						level.cagefighttimer = level.time+3000;
						level.startcage = qtrue;
					}
				}else{
					if (g_compMode.integer > 0 && cm_enabled.integer > 1){
						Boe_compTimeLimitCheck();
					}else{	
						LogExit( "Timelimit Hit." );
					}
				}
			}
		} 
	}
	}
}

/*
=================
G_GametypeCommand

Handles a command coming from the current gametype VM
=================

int G_GametypeCommand ( int cmd, int arg0, int arg1, int arg2, int arg3, int arg4 )
{
	switch ( cmd )
	{
		case GTCMD_RESTART:
			if ( arg0 <= 0 )
			{
				G_ResetGametype ( qfalse, qfalse );
			}
			else
			{
				level.gametypeResetTime = level.time + arg0 * 1000;
			}
			break;

		case GTCMD_TEXTMESSAGE:
			if(!level.intermissionQueued && !level.intermissiontime && !level.awardTime)
			{
				if(strstr((const char*)arg1, "[INF]")){
					trap_SendServerCommand( -1, va("print \"%s\n\"", (const char*)arg1));
				}else if(strstr((const char*)arg1, "[CTF]")){
					trap_SendServerCommand( -1, va("print \"%s\n\"", (const char*)arg1));
				}else if(strstr((const char*)arg1, "[ELIM]")){
					trap_SendServerCommand( -1, va("print \"%s\n\"", (const char*)arg1));
				}else if(strstr((const char*)arg1, "[DM]")){
					trap_SendServerCommand( -1, va("print \"%s\n\"", (const char*)arg1));
				}else if(strstr((const char*)arg1, "[TDM]")){
					trap_SendServerCommand( -1, va("print \"%s\n\"", (const char*)arg1));
				}else if(strstr((const char*)arg1, "[H&S]")){
					trap_SendServerCommand( -1, va("print \"%s\n\"", (const char*)arg1));
				}else if(strstr((const char*)arg1, "[H&Z]")){
					trap_SendServerCommand( -1, va("print \"%s\n\"", (const char*)arg1));
				}else{
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,%s", level.time + 5000, (const char*)arg1 ) );
				}
			}
			
			break;		

		case GTCMD_RADIOMESSAGE:
				G_Voice ( &g_entities[arg0], NULL, SAY_TEAM, (const char*) arg1, qfalse );
			break;

		case GTCMD_REGISTERGLOBALSOUND:
			return G_SoundIndex ( (char*) arg0 );

		case GTCMD_STARTGLOBALSOUND:
		//Ryan
		//Dont play any sounds for the gametype stuff if we are ending the round
		if(!level.intermissionQueued && !level.intermissiontime && !level.awardTime)
		{
				gentity_t* tent;
				tent = G_TempEntity( vec3_origin, EV_GLOBAL_SOUND );
				tent->s.eventParm = arg0;
				tent->r.svFlags = SVF_BROADCAST;	
		}
		// Boe!Man 8/7/12: This has been horribly wrong, fix for round ending in H&S messing up if timelimit was already hit (and possibly other gt's).
		break;
		//Ryan

		case GTCMD_REGISTEREFFECT:
			return G_EffectIndex ( (char*) arg0 );

		case GTCMD_PLAYEFFECT:
			G_PlayEffect ( arg0, (float*) arg1, (float*) arg2 );
			break;

		case GTCMD_ADDCLIENTSCORE:
			G_AddScore ( &g_entities[arg0], arg1 );
			break;

		case GTCMD_ADDTEAMSCORE:
			G_AddTeamScore ( (team_t) arg0, arg1 );
			break;

		case GTCMD_RESETITEM:
		{
			gitem_t* item;

			item = BG_FindGametypeItemByID ( arg0 );
			if ( item )
			{
				G_ResetGametypeItem ( item );
				return qtrue;
			}
			break;
		}

		case GTCMD_GETCLIENTNAME:
			Com_sprintf ( (char*) arg1, arg2, "%s", g_entities[arg0].client->pers.netname );
			break;

		case GTCMD_DOESCLIENTHAVEITEM:
		{
			gitem_t*	item;
			gentity_t*	ent;

			ent  = &g_entities[arg0];
			item = BG_FindGametypeItemByID ( arg1 );

			if ( item )
			{
				if ( ent->client->ps.stats[STAT_GAMETYPE_ITEMS] & (1<<item->giTag) )
				{
					return 1;
				}
			}

			return 0;
		}

		case GTCMD_REGISTERITEM:
		{
			gitem_t* item;

			item = BG_FindItem ( (const char*) arg1 );
			if ( item )
			{
				item->quantity = arg0;
				return qtrue;
			}

			return qfalse;
		}

		case GTCMD_REGISTERTRIGGER:
		{
			gentity_t* find;

			find = NULL;
			while ( NULL != (find = G_Find ( find, FOFS(classname), "gametype_trigger" ) ) )
			{
				if ( Q_stricmp ( find->targetname, (const char*) arg1 ) )
				{
					continue;
				}

				// Assign the id to it.
				find->health = arg0;
				find->touch  = gametype_trigger_touch;
				trap_LinkEntity (find);
			}

			return 0;
		}

		default:
			break;
	}

	return -1;
}
*/
