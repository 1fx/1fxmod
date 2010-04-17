// Copyright (C) 2001-2002 Raven Software
//
// g_gametype.c -- dynamic gametype handling

#include "g_local.h"
#include "q_shared.h"
#include "g_team.h"
#include "inv.h"
#include "boe_local.h"

#define	MAX_GAMETYPE_SPAWN_POINTS	32

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
		if ( ent->targetname )
			ent->targetname = strchr ( ent->targetname, '-' ) + 1;
	
		if ( ent->target )
			ent->target = strchr ( ent->target, '-' ) + 1;
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
void G_RespawnClients ( qboolean force, team_t team )
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
		
		ent->client->sess.noTeamChange = qfalse;

		trap_UnlinkEntity (ent);
		ClientSpawn ( ent );
	}
}

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
		}
	}
}

/*
===============
G_ResetGametype
===============
*/
void G_ResetGametype ( void )
{
	char color1[64];
	char color2[64];
	char color3[64];
	char color4[64];
	char color5[64];
	char color6[64];

	gentity_t*	tent;

	// Reset the glass in the level
	G_ResetGlass ( );

	// Reset all pickups in the world
	G_ResetEntities ( );

	// Reset the gametype itself
	G_ResetGametypeEntities ( );

	// Initialize the respawn interval since this is a interval gametype
	switch ( level.gametypeData->respawnType )
	{
		case RT_INTERVAL:
			level.gametypeRespawnTime[TEAM_RED]  = 0;
			level.gametypeRespawnTime[TEAM_BLUE] = 0;
			level.gametypeRespawnTime[TEAM_FREE] = 0;
			break;

		case RT_NONE:
			level.gametypeDelayTime = level.time + g_roundstartdelay.integer * 1000;
			level.gametypeRoundTime = level.time + (g_roundtimelimit.integer * 60000);

			if ( level.gametypeDelayTime != level.time )
			{
				trap_Cvar_VariableStringBuffer ( "server_color1", color1, MAX_QPATH );
				trap_Cvar_VariableStringBuffer ( "server_color2", color2, MAX_QPATH );
				trap_Cvar_VariableStringBuffer ( "server_color3", color3, MAX_QPATH );
				trap_Cvar_VariableStringBuffer ( "server_color4", color4, MAX_QPATH );
				trap_Cvar_VariableStringBuffer ( "server_color5", color5, MAX_QPATH );
				trap_Cvar_VariableStringBuffer ( "server_color6", color6, MAX_QPATH );
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sG%se%st %sR%se%sady", level.gametypeDelayTime, color1, color2, color3, color4, color5, color6 ) );
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
	G_RespawnClients ( qtrue, TEAM_RED );
	G_RespawnClients ( qtrue, TEAM_BLUE );

	level.gametypeStartTime = level.time;
	level.gametypeResetTime = 0;

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
			item->pickup_name = (char *)trap_VM_LocalStringAlloc ( temp );
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
	G_ParseGametypeItems ( trap_GPG_FindSubGroup ( gametypeGroup, "items" ) );

	// Free up the parser
	trap_GP_Delete(&GP2);

	return qtrue;
}

/*G_EnableGametypeItemPickup

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
	// Boe!Man 4/4/10
	char buffer[64];
	char color1[64];
	char color2[64];
	char color3[64];
	char color4[64];
	char color5[64];
	char color6[64];

	// If the level is over then forget checking gametype stuff.
	if ( level.intermissiontime || level.pause )
	//End Ryan
	{
		return;
	}

	// Dont bother starting the gametype until
	// the first person joins
	if ( !level.gametypeStartTime )
	{
		int counts[TEAM_NUM_TEAMS];
		int i;

		memset ( counts, 0, sizeof(counts) );

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

			counts[other->client->sess.team]++;
		}

		if ( counts[TEAM_RED] || counts[TEAM_BLUE] )
		{
			G_ResetGametype ( );
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

			G_ResetGametype ( );
		}

		return;
	}

	// Handle respawn interval spawning
	if ( level.gametypeData->respawnType == RT_INTERVAL )
	{
		team_t team;
		for ( team = TEAM_RED; team < TEAM_SPECTATOR; team ++ )
		{
			if ( level.gametypeRespawnTime[team] && level.time > level.gametypeRespawnTime[team] )
			{
				// Respawn all dead clients
				G_RespawnClients ( qfalse, team );

				// Next interval
				level.gametypeRespawnTime[team] = 0;
			}
		}
	}

	// If we are in RT_NONE respawn mode then we need to look for everyone being dead
	if ( level.time > level.gametypeDelayTime && level.gametypeData->respawnType == RT_NONE && level.gametypeStartTime )
	{
		int i;
		int alive[TEAM_NUM_TEAMS];
		int dead[TEAM_NUM_TEAMS];

		memset ( &alive[0], 0, sizeof(alive) );
		memset ( &dead[0], 0, sizeof(dead) );
		for ( i = 0; i < level.numConnectedClients; i ++ )
		{
			gentity_t* ent = &g_entities[level.sortedClients[i]];

			if ( ent->client->pers.connected != CON_CONNECTED )
			{
				continue;
			}

			if ( G_IsClientDead ( ent->client ) )
			{
				dead[ent->client->sess.team] ++;
			}
			else
			{
				alive[ent->client->sess.team] ++;
			}
		}

		if(alive[TEAM_RED] == 1 && level.redMsgCount == 0){
			for ( i = 0; i < level.numConnectedClients; i ++ ){
				gentity_t* ent = &g_entities[level.sortedClients[i]];
				if ( ent->client->sess.team == TEAM_RED && alive[TEAM_RED] == 1 &&
					!G_IsClientDead ( ent->client ) && !level.redMsgCount){
						//Q_strncpyz(ent->client->sess.msgReason,"^7You are the\n^_LAST ALIVE ^7on the ^_Hiders!", sizeof(ent->client->sess.msgReason));
						trap_SendServerCommand( ent->s.number, va("cp \"@You are the last player alive!\n\"") );
						//trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@You are the last player alive!", level.time + 5000));
						trap_SendServerCommand(-1, va("print\"^3[Info] ^7%s is the last alive player in the red team.\n\"", ent->client->pers.netname));
						Boe_ClientSound(ent, G_SoundIndex("sound/misc/events/tut_door01.mp3"));
						level.redMsgCount++;
				}
			}
		}

		// If everyone is dead on a team then reset the gametype, but only if 
		// there was someone on that team to begin with.
		if ( !alive[TEAM_RED] && dead[TEAM_RED] )
		{			
			trap_Cvar_VariableStringBuffer ( "server_redteamprefix", buffer, MAX_QPATH );
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%s ^7team eliminated!", level.time + 5000, buffer));
			trap_SendServerCommand( -1, va("print \"^3[INF] ^7Red team eliminated\n\""));
			trap_GT_SendEvent ( GTEV_TEAM_ELIMINATED, level.time, TEAM_RED, 0, 0, 0, 0 );
		}
		else if ( !alive[TEAM_BLUE] && dead[TEAM_BLUE] )
		{		
			trap_Cvar_VariableStringBuffer ( "server_blueteamprefix", buffer, MAX_QPATH );
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%s ^7team eliminated!", level.time + 5000, buffer));
			trap_SendServerCommand( -1, va("print \"^3[INF] ^7Blue team eliminated\n\""));
			trap_GT_SendEvent ( GTEV_TEAM_ELIMINATED, level.time, TEAM_BLUE, 0, 0, 0, 0 );
		}
		// See if the time has expired
		if ( level.time > level.gametypeRoundTime )
		{
			trap_Cvar_VariableStringBuffer ( "server_redteamprefix", buffer, MAX_QPATH );
			trap_Cvar_VariableStringBuffer ( "server_color1", color1, MAX_QPATH );
			trap_Cvar_VariableStringBuffer ( "server_color2", color2, MAX_QPATH );
			trap_Cvar_VariableStringBuffer ( "server_color3", color3, MAX_QPATH );
			trap_Cvar_VariableStringBuffer ( "server_color4", color4, MAX_QPATH );
			trap_Cvar_VariableStringBuffer ( "server_color5", color5, MAX_QPATH );
			trap_Cvar_VariableStringBuffer ( "server_color6", color6, MAX_QPATH );
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%s ^7team has %sd%se%sf%se%sn%sded the briefcase!", level.time + 5000, buffer, color1, color2, color3, color4, color5, color6));
			trap_SendServerCommand( -1, va("print \"^3[INF] ^7Red team has defended the briefcase\n\""));
			trap_GT_SendEvent ( GTEV_TIME_EXPIRED, level.time, 0, 0, 0, 0, 0 );
		} 
	}
}

/*
=================
G_GametypeCommand

Handles a command coming from the current gametype VM
=================
*/
int G_GametypeCommand ( int cmd, int arg0, int arg1, int arg2, int arg3, int arg4 )
{
	switch ( cmd )
	{
		case GTCMD_RESTART:
			if ( arg0 <= 0 )
			{
				G_ResetGametype ( );
			}
			else
			{
				level.gametypeResetTime = level.time + arg0 * 1000;
			}
			break;

		case GTCMD_TEXTMESSAGE:
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,%s", level.time + 5000, (const char*)arg1 ) );
			break;		

		case GTCMD_RADIOMESSAGE:
			G_Voice ( &g_entities[arg0], NULL, SAY_TEAM, (const char*) arg1, qfalse );
			break;

		case GTCMD_REGISTERGLOBALSOUND:
			return G_SoundIndex ( (char*) arg0 );

		case GTCMD_STARTGLOBALSOUND:
		{
			gentity_t* tent;
			tent = G_TempEntity( vec3_origin, EV_GLOBAL_SOUND );
			tent->s.eventParm = arg0;
			tent->r.svFlags = SVF_BROADCAST;	
			break;
		}	

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
