// Copyright (C) 2001-2002 Raven Software.
//
// g_misc.c

#include "g_local.h"


/*QUAKED func_group (0 0 0) ?
Used to group brushes together just for editor convenience.  They are turned into normal brushes by the utilities.
*/

/*QUAKED info_notnull (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for in-game calculation, like jumppad targets.
target_position does the same thing
*/
void SP_info_notnull( gentity_t *self )
{
	G_SetOrigin( self, self->s.origin );
}

/*
=================================================================================
TeleportPlayer
=================================================================================
*/
void TeleportPlayer ( gentity_t *player, vec3_t origin, vec3_t angles, qboolean nojump) 
{
	gentity_t	*tent;
	if(level.time < player->client->sess.lastTele)
		return;
	else
		player->client->sess.lastTele = level.time+500;

	// use temp events at source and destination to prevent the effect
	// from getting dropped by a second player event
	if ( !G_IsClientSpectating ( player->client ) ) 
	{
		tent = G_TempEntity( player->client->ps.origin, EV_PLAYER_TELEPORT_OUT );
		tent->s.clientNum = player->s.clientNum;

		tent = G_TempEntity( origin, EV_PLAYER_TELEPORT_IN );
		tent->s.clientNum = player->s.clientNum;
	}

	// unlink to make sure it can't possibly interfere with G_KillBox
	trap_UnlinkEntity (player);

	VectorCopy ( origin, player->client->ps.origin );
	player->client->ps.origin[2] += 1;

	// spit the player out
	AngleVectors( angles, player->client->ps.velocity, NULL, NULL );
	if(current_gametype.value == GT_HS){
		if(!nojump){
			VectorScale( player->client->ps.velocity, 500, player->client->ps.velocity ); // Henkie 22/02/10 -> Do not spit ( default 400)
			player->client->ps.pm_time = 160;		// another jump available after 160ms
		}
	}else{
		VectorScale( player->client->ps.velocity, 400, player->client->ps.velocity );
		player->client->ps.pm_time = 160;		// hold time
		player->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
	}

	// toggle the teleport bit so the client knows to not lerp
	player->client->ps.eFlags ^= EF_TELEPORT_BIT;

	// set angles
	SetClientViewAngle( player, angles );

	// kill anything at the destination
	if ( !G_IsClientSpectating ( player->client ) ) 
	{
		G_KillBox (player);
	}

	// save results of pmove
	BG_PlayerStateToEntityState( &player->client->ps, &player->s, qtrue );

	// use the precise origin for linking
	VectorCopy( player->client->ps.origin, player->r.currentOrigin );

	if ( !G_IsClientSpectating ( player->client ) ) 
	{
		trap_LinkEntity (player);
	}
	
	if(!nojump && player->client->sess.team != TEAM_SPECTATOR)
	G_PlayEffect ( G_EffectIndex("misc/electrical"),player->client->ps.origin, player->pos1);
}


/*QUAKED misc_teleporter_dest (1 0 0) (-32 -32 -24) (32 32 -16)
Point teleporters at these.
Now that we don't have teleport destination pads, this is just
an info_notnull
*/
void SP_misc_teleporter_dest( gentity_t *ent ) 
{
}

//===========================================================

/*QUAKED misc_model (1 0 0) (-16 -16 -16) (16 16 16) RMG
this model is inserted into the bsp file
"model"		arbitrary .md3 file to display
*/
void SP_misc_model( gentity_t *ent ) 
{
#if 0
	ent->s.modelindex = G_ModelIndex( ent->model );
	VectorSet (ent->mins, -16, -16, -16);
	VectorSet (ent->maxs, 16, 16, 16);
	trap_LinkEntity (ent);

	G_SetOrigin( ent, ent->s.origin );
	VectorCopy( ent->s.angles, ent->s.apos.trBase );
#else
	G_FreeEntity( ent );
#endif
}


/*QUAKED misc_G2model (1 0 0) (-16 -16 -16) (16 16 16)
"model"		arbitrary .glm file to display
*/
void SP_misc_G2model( gentity_t *ent ) {

#if 0
	char name1[200] = "models/players/kyle/modelmp.glm";
	trap_G2API_InitGhoul2Model(&ent->s, name1, G_ModelIndex( name1 ), 0, 0, 0, 0);
	trap_G2API_SetBoneAnim(ent->s.ghoul2, 0, "model_root", 0, 12, BONE_ANIM_OVERRIDE_LOOP, 1.0f, level.time, -1, -1);
	ent->s.radius = 150;
//	VectorSet (ent->mins, -16, -16, -16);
//	VectorSet (ent->maxs, 16, 16, 16);
	trap_LinkEntity (ent);

	G_SetOrigin( ent, ent->s.origin );
	VectorCopy( ent->s.angles, ent->s.apos.trBase );
#else
	G_FreeEntity( ent );
#endif
}

//===========================================================

void locateCamera( gentity_t *ent ) 
{
	vec3_t		dir;
	gentity_t	*target;
	gentity_t	*owner;

	owner = G_PickTarget( ent->target );
	if ( !owner ) 
	{
		Com_Printf( "Couldn't find target for misc_partal_surface\n" );
		G_FreeEntity( ent );
		return;
	}
	ent->r.ownerNum = owner->s.number;

	// frame holds the rotate speed
	if ( owner->spawnflags & 1 ) 
	{
		ent->s.frame = 25;
	} 
	else if ( owner->spawnflags & 2 ) 
	{
		ent->s.frame = 75;
	}

	// swing camera ?
	if ( owner->spawnflags & 4 ) 
	{
		// set to 0 for no rotation at all
		ent->s.gametypeitems = 0;
	}
	else 
	{
		ent->s.gametypeitems = 1;
	}

	// clientNum holds the rotate offset
	ent->s.clientNum = owner->s.clientNum;

	VectorCopy( owner->s.origin, ent->s.origin2 );

	// see if the portal_camera has a target
	target = G_PickTarget( owner->target );
	if ( target ) {
		VectorSubtract( target->s.origin, owner->s.origin, dir );
		VectorNormalize( dir );
	} else {
		G_SetMovedir( owner->s.angles, dir );
	}

	ent->s.eventParm = DirToByte( dir );
}

/*QUAKED misc_portal_surface (0 0 1) (-8 -8 -8) (8 8 8)
The portal surface nearest this entity will show a view from the targeted misc_portal_camera, or a mirror view if untargeted.
This must be within 64 world units of the surface!
*/
void SP_misc_portal_surface(gentity_t *ent) 
{
	VectorClear( ent->r.mins );
	VectorClear( ent->r.maxs );
	trap_LinkEntity (ent);

	ent->r.svFlags = SVF_PORTAL;
	ent->s.eType = ET_PORTAL;

	if ( !ent->target ) 
	{
		VectorCopy( ent->s.origin, ent->s.origin2 );
	} 
	else 
	{
		ent->think = locateCamera;
		ent->nextthink = level.time + 100;
	}
}

/*QUAKED misc_portal_camera (0 0 1) (-8 -8 -8) (8 8 8) slowrotate fastrotate noswing
The target for a misc_portal_director.  You can set either angles or target another entity to determine the direction of view.
"roll" an angle modifier to orient the camera around the target vector;
*/
void SP_misc_portal_camera(gentity_t *ent) 
{
	float	roll;

	VectorClear( ent->r.mins );
	VectorClear( ent->r.maxs );
	trap_LinkEntity (ent);

	G_SpawnFloat( "roll", "0", &roll );

	ent->s.clientNum = roll/360.0 * 256;
}

/*QUAKED misc_bsp (1 0 0) (-16 -16 -16) (16 16 16)
"bspmodel"		arbitrary .bsp file to display
*/
void SP_misc_bsp(gentity_t *ent) 
{
	char	temp[MAX_QPATH];
	char	*out;
	float	newAngle;
	int		tempint;

	G_SpawnFloat( "angles", "0", &newAngle );
	if (newAngle != 0.0)
	{
		ent->s.angles[0] = newAngle;
		ent->s.angles[1] = newAngle;
		ent->s.angles[2] = newAngle;
	}
	
	G_SpawnString("bspmodel", "", &out);

	//ent->s.eFlags = EF_PERMANENT; //EF_TELEPORT_BIT;

	// Mainly for debugging
	G_SpawnInt( "spacing", "0", &tempint);
	ent->s.time2 = tempint;
	G_SpawnInt( "flatten", "0", &tempint);
	ent->s.time = tempint;

	Com_sprintf(temp, MAX_QPATH, "#%s", out);
	trap_SetBrushModel( ent, temp );  // SV_SetBrushModel -- sets mins and maxs
	G_BSPIndex(temp);

	level.mNumBSPInstances++;
	Com_sprintf(temp, MAX_QPATH, "%d-", level.mNumBSPInstances);
	VectorCopy(ent->s.origin, level.mOriginAdjust);
	level.mRotationAdjust = ent->s.angles[1];
	level.mTargetAdjust = temp;
	level.hasBspInstances = qtrue;
	level.mBSPInstanceDepth++;
	G_SpawnString("filter", "", &out);
	strcpy(level.mFilter, out);
	G_SpawnString("teamfilter", "", &out);
	strcpy(level.mTeamFilter, out);

	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );
	VectorCopy( ent->s.angles, ent->s.apos.trBase );
	VectorCopy( ent->s.angles, ent->r.currentAngles );

	ent->s.eType = ET_MOVER;

	trap_LinkEntity (ent);

	trap_SetActiveSubBSP(ent->s.modelindex);
	G_SpawnEntitiesFromString(qtrue);
	trap_SetActiveSubBSP(-1);

	level.mBSPInstanceDepth--;
	level.mFilter[0] = level.mTeamFilter[0] = 0;

	if ( g_debugRMG.integer )
	{
		G_SpawnDebugCylinder ( ent->s.origin, ent->s.time2, &g_entities[0], 2000, COLOR_WHITE );

		if ( ent->s.time )
		{
			G_SpawnDebugCylinder ( ent->s.origin, ent->s.time, &g_entities[0], 2000, COLOR_RED );
		}
	}
}

// ORIGINAL DATA BELOW 
/*QUAKED misc_bsp (1 0 0) (-16 -16 -16) (16 16 16)
"bspmodel"		arbitrary .bsp file to display
*/
/*void SP_misc_bsp(gentity_t *ent) 
{
	char	temp[MAX_QPATH];
	char	*out;
	float	newAngle;
	int		tempint;

	G_SpawnFloat( "angle", "0", &newAngle );
	if (newAngle != 0.0)
	{
		ent->s.angles[1] = newAngle;
	}
	// don't support rotation any other way
	ent->s.angles[0] = 0.0;
	ent->s.angles[2] = 0.0;
	
	G_SpawnString("bspmodel", "", &out);

	ent->s.eFlags = EF_PERMANENT;

	// Mainly for debugging
	G_SpawnInt( "spacing", "0", &tempint);
	ent->s.time2 = tempint;
	G_SpawnInt( "flatten", "0", &tempint);
	ent->s.time = tempint;

	Com_sprintf(temp, MAX_QPATH, "#%s", out);
	trap_SetBrushModel( ent, temp );  // SV_SetBrushModel -- sets mins and maxs
	G_BSPIndex(temp);

	level.mNumBSPInstances++;
	Com_sprintf(temp, MAX_QPATH, "%d-", level.mNumBSPInstances);
	VectorCopy(ent->s.origin, level.mOriginAdjust);
	level.mRotationAdjust = ent->s.angles[1];
	level.mTargetAdjust = temp;
	level.hasBspInstances = qtrue;
	level.mBSPInstanceDepth++;
	G_SpawnString("filter", "", &out);
	strcpy(level.mFilter, out);
	G_SpawnString("teamfilter", "", &out);
	strcpy(level.mTeamFilter, out);

	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );
	VectorCopy( ent->s.angles, ent->s.apos.trBase );
	VectorCopy( ent->s.angles, ent->r.currentAngles );

	ent->s.eType = ET_MOVER;

	trap_LinkEntity (ent);

	trap_SetActiveSubBSP(ent->s.modelindex);
	G_SpawnEntitiesFromString(qtrue);
	trap_SetActiveSubBSP(-1);

	level.mBSPInstanceDepth--;
	level.mFilter[0] = level.mTeamFilter[0] = 0;

	if ( g_debugRMG.integer )
	{
		G_SpawnDebugCylinder ( ent->s.origin, ent->s.time2, &g_entities[0], 2000, COLOR_WHITE );

		if ( ent->s.time )
		{
			G_SpawnDebugCylinder ( ent->s.origin, ent->s.time, &g_entities[0], 2000, COLOR_RED );
		}
	}
}

/*QUAKED terrain (1.0 1.0 1.0) ?
Terrain entity
It will stretch to the full height of the brush

numPatches - integer number of patches to split the terrain brush into (default 200)
terxels - integer number of terxels on a patch side (default 4) (2 <= count <= 8)
seed - integer seed for random terrain generation (default 0)
textureScale - float scale of texture (default 0.005)
heightMap - name of heightmap data image to use
terrainDef - defines how the game textures the terrain (file is base/ext_data/*.terrain - default is grassyhills)
instanceDef - defines which bsp instances appear
miscentDef - defines which client models spawn on the terrain (file is base/ext_data/*.miscents)
densityMap - how dense the client models are packed

*/
void SP_terrain(gentity_t *ent) 
{
	char				temp[MAX_INFO_STRING];
	char				final[MAX_QPATH];
	char				seed[MAX_QPATH];
	char				missionType[MAX_QPATH];
	char				soundSet[MAX_QPATH];
	int					shaderNum, i;
	char				*value;
	int					terrainID;

	VectorClear (ent->s.angles);
	if(!Q_stricmp(ent->model, "NV_MODEL")){
		ent->r.contents = CONTENTS_TERRAIN;
		//ent->r.svFlags = SVF_NOCLIENT;
		//ent->s.eFlags = EF_PERMANENT;
		ent->s.eType = ET_TERRAIN;
		trap_LinkEntity (ent);
	}
	else
	///End  - 04.07.07 - 11:20am
	trap_SetBrushModel( ent, ent->model );

	// Get the shader from the top of the brush
//	shaderNum = gi.CM_GetShaderNum(s.modelindex);
	shaderNum = 0;

	if (RMG.integer)
	{
		// Grab the default terrain file from the RMG cvar
		trap_Cvar_VariableStringBuffer("RMG_terrain", temp, MAX_QPATH);
		Com_sprintf(final, MAX_QPATH, "%s", temp);
		AddSpawnField("terrainDef", temp);
 
		trap_Cvar_VariableStringBuffer("RMG_instances", temp, MAX_QPATH);
		Com_sprintf(final, MAX_QPATH, "%s", temp);
		AddSpawnField("instanceDef", temp);

		trap_Cvar_VariableStringBuffer("RMG_miscents", temp, MAX_QPATH);
		Com_sprintf(final, MAX_QPATH, "%s", temp);
		AddSpawnField("miscentDef", temp);

		trap_Cvar_VariableStringBuffer("RMG_seed", seed, MAX_QPATH);
		trap_Cvar_VariableStringBuffer("RMG_mission", missionType, MAX_QPATH);

		trap_Cvar_VariableStringBuffer("RMG_soundset", soundSet, MAX_QPATH);
		trap_SetConfigstring(CS_AMBIENT_SOUNDSETS, soundSet );
	}

	// Arbitrary (but sane) limits to the number of terxels
//	if((mTerxels < MIN_TERXELS) || (mTerxels > MAX_TERXELS))
	{
//		Com_printf("G_Terrain: terxels out of range - defaulting to 4\n");
//		mTerxels = 4;
	}

	// Get info required for the common init
	temp[0] = 0;
	G_SpawnString("heightmap", "", &value);
	Info_SetValueForKey(temp, "heightMap", value);

	G_SpawnString("numpatches", "400", &value);
	Info_SetValueForKey(temp, "numPatches", va("%d", atoi(value)));

	G_SpawnString("terxels", "4", &value);
	Info_SetValueForKey(temp, "terxels", va("%d", atoi(value)));

	Info_SetValueForKey(temp, "seed", seed);
	Info_SetValueForKey(temp, "minx", va("%f", ent->r.mins[0]));
	Info_SetValueForKey(temp, "miny", va("%f", ent->r.mins[1]));
	Info_SetValueForKey(temp, "minz", va("%f", ent->r.mins[2]));
	Info_SetValueForKey(temp, "maxx", va("%f", ent->r.maxs[0]));
	Info_SetValueForKey(temp, "maxy", va("%f", ent->r.maxs[1]));
	Info_SetValueForKey(temp, "maxz", va("%f", ent->r.maxs[2]));

	Info_SetValueForKey(temp, "modelIndex", va("%d", ent->s.modelindex));

	G_SpawnString("terraindef", "grassyhills", &value);
	Info_SetValueForKey(temp, "terrainDef", value);

	G_SpawnString("instancedef", "", &value);
	Info_SetValueForKey(temp, "instanceDef", value);

	G_SpawnString("miscentdef", "", &value);
	Info_SetValueForKey(temp, "miscentDef", value);

	Info_SetValueForKey(temp, "missionType", missionType);
	
	for(i = 0; i < MAX_INSTANCE_TYPES; i++)
	{
		trap_Cvar_VariableStringBuffer(va("RMG_instance%d", i), final, MAX_QPATH);
		if(strlen(final))
		{
			Info_SetValueForKey(temp, va("inst%d", i), final);
		}
	}

	// Set additional data required on the client only
	G_SpawnString("densitymap", "", &value);
	Info_SetValueForKey(temp, "densityMap", value);

	Info_SetValueForKey(temp, "shader", va("%d", shaderNum));
	G_SpawnString("texturescale", "0.005", &value);
	Info_SetValueForKey(temp, "texturescale", va("%f", atof(value)));

	// Initialise the common aspects of the terrain
	terrainID = trap_CM_RegisterTerrain(temp);
//	SetCommon(common);

	Info_SetValueForKey(temp, "terrainId", va("%d", terrainID));

	// Let the entity know if it is random generated or not
//	SetIsRandom(common->GetIsRandom());

	// Let the game remember everything
	level.landScapes[terrainID] = ent;

	// Send all the data down to the client
	trap_SetConfigstring(CS_TERRAINS + terrainID, temp);

	// Make sure the contents are properly set
	ent->r.contents = CONTENTS_TERRAIN;
	ent->r.svFlags = SVF_NOCLIENT;
	ent->s.eFlags = EF_PERMANENT;
	ent->s.eType = ET_TERRAIN;

	// Hook into the world so physics will work
	trap_LinkEntity(ent);

	// If running RMG then initialize the terrain and handle team skins
	if ( RMG.integer ) 
	{
		trap_RMG_Init(terrainID);

		if ( level.gametypeData->teams )
		{
			char temp[MAX_QPATH];

			// Red team change from RMG ?
			trap_GetConfigstring ( CS_GAMETYPE_REDTEAM, temp, MAX_QPATH );
			if ( Q_stricmp ( temp, level.gametypeTeam[TEAM_RED] ) )
			{
				level.gametypeTeam[TEAM_RED] = trap_VM_LocalStringAlloc ( temp );
			}

			// Blue team change from RMG ?
			trap_GetConfigstring ( CS_GAMETYPE_BLUETEAM, temp, MAX_QPATH );
			if ( Q_stricmp ( temp, level.gametypeTeam[TEAM_BLUE] ) )
			{
				level.gametypeTeam[TEAM_BLUE] = trap_VM_LocalStringAlloc ( temp );
			}
		}
	}
}

/*
=================================================================================
G_DebugCylinderThink
=================================================================================
*/
void G_DebugCylinderThink ( gentity_t* ent )
{
	vec3_t vec1;
	vec3_t vec2;

	ent->nextthink = level.time + 1000;

	VectorCopy ( ent->parent->client->ps.origin, vec1 );
	VectorCopy ( ent->r.currentOrigin, vec2 );
	
	vec1[2] = 0;
	vec2[2] = 0;

	// IF we are too far away then kill it 
	if ( Distance ( vec1, vec2 ) > ent->speed )
	{
		trap_UnlinkEntity ( ent );
		return;
	}

	trap_LinkEntity ( ent );
}

/*
=================================================================================
G_SpawnDebugCylinder
=================================================================================
*/
void G_SpawnDebugCylinder ( vec3_t origin, float radius, gentity_t* clientent, float viewRadius, int colorIndex )
{
	gentity_t* ent;

	if ( !g_cheats.integer )
	{
		return;
	}

	ent = G_Spawn ( );
	ent->s.eType = ET_DEBUG_CYLINDER;
	ent->s.time = colorIndex;
	ent->s.time2 = (int) radius;
	ent->parent = clientent;
	ent->speed = viewRadius;

	VectorCopy ( origin, ent->s.origin );
	VectorCopy ( origin, ent->r.currentOrigin );

	ent->nextthink = level.time + 1000;
	ent->think = G_DebugCylinderThink;
	
	trap_LinkEntity ( ent );
}


void fx_think( gentity_t *ent ) 
{
	int		time;

	// play fx
	if(ent->disabled != qtrue)
	G_PlayEffect( ent->health, ent->s.origin, ent->pos1 );

	if( ent->count >= 0 )	// stops it from rolling over, yeah kinda lame...
	{
		ent->count--;
	}

	if( !ent->count )	// effect is suppose to play mCount times then dissapear
	{
		G_FreeEntity(ent);
		return;
	}

	// calc next play time
	time = (ent->wait + flrand(0.0f, ent->random)) * 1000;	// need it in milliseconds 

	ent->think = fx_think;
	ent->nextthink = level.time + time;
}


void nolower(gentity_t *ent)
{
	G_SpawnVector("origin", "0", level.nolower);
	// Boe!Man 3/1/11: Add the minimum players (if found).
	if (ent->min_players > 0){
		level.autoNoLower = ent->min_players;
		if(ent->wait == 0){
			level.autoNoLowerDelay = 10;
		}else{
			level.autoNoLowerDelay = ent->wait;
		}
		level.autoNoLowerUpdateTime = level.time + 10000;
	}
	level.nolower1 = qtrue;
}

/*QUAKED fx_play_effect (.2 .5 .8) (-8 -8 -8) (8 8 8) START_OFF
Plays specified effect file

"effect"    name of .efx file
"wait"      seconds between triggerings, default 0.3
"random"    wait variance in seconds, default 0
"target"    direction of effect, default up
"count"     plays effect this many times then deletes itself, default -1 = infinite 


START_OFF    fx starts off


*/

void SP_fx_play_effect(gentity_t *ent)
{
	char		*fxName;

	G_SetOrigin( ent, ent->s.origin );

	G_SpawnString("effect", "", &fxName);
	ent->health = G_EffectIndex(fxName);

	if (ent->wait == 0.0)
	{
		ent->wait = 0.3;
	}
	
	/*
	target = G_Find(0, FOFS(targetname), ent->target);
	if (target)
	{
		VectorSubtract( target->s.origin, ent->s.origin, ent->pos1 );
		VectorNormalize( ent->pos1 );
		// find angles
		vectoangles( ent->pos1, ent->r.currentAngles );
		// copy over to other angles
		VectorCopy( ent->r.currentAngles, ent->s.angles );	
		VectorCopy( ent->r.currentAngles, ent->s.apos.trBase );
	}
	else
	{*/
		ent->pos1[0] = ent->pos1[1] = 0.0; // Henkie -> If you remove this effects will not be targeted up to the sky
		ent->pos1[2] = 1.0;
	//}

	ent->think = fx_think;
	ent->nextthink = level.time + 100;
}


