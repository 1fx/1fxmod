// Copyright (C) 2001-2002 Raven Software.
//
// g_misc.c

#include "g_local.h"
#include "boe_local.h"


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
    gentity_t   *tent;

    if(level.time < player->client->sess.lastTele)
        return;
    else
        player->client->sess.lastTele = level.time+500;
    //Com_Printf("%s is teleporting to %s\n", player->client->pers.netname, vtos(origin));
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
            VectorScale( player->client->ps.velocity, 600, player->client->ps.velocity ); // Henkie 22/02/10 -> Do not spit ( default 400)
            player->client->ps.pm_time = 0;     // another jump available after 160ms
        }
    }else{
        if(!nojump){
            VectorScale( player->client->ps.velocity, 600, player->client->ps.velocity ); // Henkie 22/02/10 -> Do not spit ( default 400)
            player->client->ps.pm_time = 0;     // another jump available after 160ms
        }
    }

    // toggle the teleport bit so the client knows to not lerp
    player->client->ps.eFlags ^= EF_TELEPORT_BIT;

    // set angles
    if(!nojump){
        SetClientViewAngle( player, angles );
    }

    // kill anything at the destination
    if ( !G_IsClientSpectating ( player->client ) )
    {
        G_KillBox (player, qtrue);
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
"model"     arbitrary .md3 file to display
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

/*
==============
SP_misc_weaponmodel
12/28/15 - 11:18 PM
Spawns a weapon that cannot be picked up
and has no other effects on the specified
origin.
==============
*/

void SP_misc_weaponmodel ( gentity_t *ent )
{
    int         i;
    gentity_t   *weapon = NULL;
    gitem_t     *weaponItem = NULL;

    // Check if there was a weapon specified.
    if(!ent->model || !ent->model[0]){
        Com_Printf ("misc_weaponmodel: no model specified, not spawning.\n");
        G_FreeEntity(ent);
        return;
    }

    // Get the weapon number.
    Q_strlwr(ent->model);
    for(i = WP_KNIFE; i < level.wpNumWeapons; i++){
        if(strstr(Q_strlwr(va("%s", bg_weaponNames[i])), ent->model)){
            weaponItem = BG_FindWeaponItem((weapon_t)i);
            break;
        }
    }

    // Invalid weapon specified, name not found.
    if(weaponItem == NULL){
        Com_Printf ("misc_weaponmodel: invalid weapon specified (%s), not spawning.\n", ent->model);
        G_FreeEntity(ent);
        return;
    }

    // Spawn the weapon and set the model.
    weapon = G_Spawn();
    weapon->classname = ent->classname;
    weapon->s.eType = ET_ITEM;
    weapon->s.eFlags = EF_PERMANENT;
    weapon->s.modelindex = weaponItem - bg_itemlist;

    // Update its origin and angle(s).
    G_SetOrigin(weapon, ent->s.origin);
    VectorCopy (ent->s.angles, weapon->s.angles);

    trap_LinkEntity (weapon);

    // Free the original entity, no need to keep this around.
    G_FreeEntity(ent);
}

/*QUAKED misc_G2model (1 0 0) (-16 -16 -16) (16 16 16)
"model"     arbitrary .glm file to display
*/
void SP_misc_G2model( gentity_t *ent ) {

#if 0
    char name1[200] = "models/players/kyle/modelmp.glm";
    trap_G2API_InitGhoul2Model(&ent->s, name1, G_ModelIndex( name1 ), 0, 0, 0, 0);
    trap_G2API_SetBoneAnim(ent->s.ghoul2, 0, "model_root", 0, 12, BONE_ANIM_OVERRIDE_LOOP, 1.0f, level.time, -1, -1);
    ent->s.radius = 150;
//  VectorSet (ent->mins, -16, -16, -16);
//  VectorSet (ent->maxs, 16, 16, 16);
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
    vec3_t      dir;
    gentity_t   *target;
    gentity_t   *owner;

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
    float   roll;

    VectorClear( ent->r.mins );
    VectorClear( ent->r.maxs );
    trap_LinkEntity (ent);

    G_SpawnFloat( "roll", "0", &roll );

    ent->s.clientNum = roll/360.0 * 256;
}

/*QUAKED misc_bsp (1 0 0) (-16 -16 -16) (16 16 16)
"bspmodel"      arbitrary .bsp file to display
*/
void SP_misc_bsp(gentity_t *ent)
{
    char    temp[MAX_QPATH];
    char    *out;
    vec3_t  newAngle;
    int     tempint;

    // Boe!Man 5/24/12: Using a float isn't foolproof, use a vector instead.
    if(G_SpawnVector("angles", "0 0 0", newAngle)){
        ent->s.angles[0] = newAngle[0];
        ent->s.angles[1] = newAngle[1];
        ent->s.angles[2] = newAngle[2];
    }

    G_SpawnString("bspmodel", "", &out);

    // Boe!Man 11/24/15: Make this entity permanent if spawned upon start of the map.
    if(level.time == level.startTime){
        // Boe!Man 12/16/15: Make sure they are not part of any section blocker.
        if(Q_stricmp(ent->target, "nolower")
        && Q_stricmp(ent->target, "noroof")
        && Q_stricmp(ent->target, "nomiddle")
        && Q_stricmp(ent->target, "nowhole")
        ){
            ent->s.eFlags = EF_PERMANENT;
        }
    }

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
    level.mSpawnFlags = ent->spawnflags;
    G_SpawnEntitiesFromString(qtrue);
    level.mSpawnFlags = 0;
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
"bspmodel"      arbitrary .bsp file to display
*/
/*void SP_misc_bsp(gentity_t *ent)
{
    char    temp[MAX_QPATH];
    char    *out;
    float   newAngle;
    int     tempint;

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
    char                temp[MAX_INFO_STRING];
    char                final[MAX_QPATH];
    char                seed[MAX_QPATH];
    char                missionType[MAX_QPATH];
    char                soundSet[MAX_QPATH];
    int                 shaderNum, i;
    char                *value;
    int                 terrainID;

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
//  shaderNum = gi.CM_GetShaderNum(s.modelindex);
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
        // Boe!Man 11/5/12.
        // Fixed h&s/h&z with separate mission files bugging other clients.
        // There's only one drawback, the server owner should keep these files in sync (inf, h&s, h&z) at ALL times to avoid random crashes etc.
        if(strstr(missionType, "h&s") || strstr(missionType, "h&z")){
            trap_Cvar_Set("RMG_mission", "inf");
            G_UpdateCvars();
            trap_Cvar_VariableStringBuffer("RMG_mission", missionType, MAX_QPATH);
        }

        trap_Cvar_VariableStringBuffer("RMG_soundset", soundSet, MAX_QPATH);
        trap_SetConfigstring(CS_AMBIENT_SOUNDSETS, soundSet );
    }

    // Arbitrary (but sane) limits to the number of terxels
//  if((mTerxels < MIN_TERXELS) || (mTerxels > MAX_TERXELS))
    {
//      Com_printf("G_Terrain: terxels out of range - defaulting to 4\n");
//      mTerxels = 4;
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
//  SetCommon(common);

    Info_SetValueForKey(temp, "terrainId", va("%d", terrainID));

    // Let the entity know if it is random generated or not
//  SetIsRandom(common->GetIsRandom());

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
    int     time;

    // play fx
    if(ent->disabled != qtrue)
    G_PlayEffect( ent->health, ent->s.origin, ent->pos1 );

    if( ent->count >= 0 )   // stops it from rolling over, yeah kinda lame...
    {
        ent->count--;
    }

    if( !ent->count )   // effect is suppose to play mCount times then dissapear
    {
        G_FreeEntity(ent);
        return;
    }

    // calc next play time
    time = (ent->wait + flrand(0.0f, ent->random)) * 1000;  // need it in milliseconds

    ent->think = fx_think;
    ent->nextthink = level.time + time;
}

/*
=================================================================================
nolower, noroof, nomiddle, nowhole.
=================================================================================
*/

typedef enum
{
    NOLOWER,
    NOROOF,
    NOMIDDLE,
    NOWHOLE
} section_t;

typedef enum
{
    INIT,
    OPENING,
    OPENED,
    CLOSING,
    CLOSED
} sectionState_t;

void g_sectionAddOrDelInstances(gentity_t *ent, qboolean add){
    gentity_t *ent2 = NULL;

    while (NULL != (ent2 = G_Find ( ent2, FOFS(target), ent->classname ))){
        if(ent2 != ent){ // Make sure we don't get the parent ent.
            if(!add){ // Upon removal, just make sure they are not drawed and clients can't interact with them.
                trap_UnlinkEntity(ent2);
            }else{ // Same as removal, but the other way around.
                trap_LinkEntity(ent2);
            }
        }
    }
}

void g_sectionAutoCheck(gentity_t *ent){
    if(level.autoLRMWActive[ent->section]){
        // Check what needs to be done depending on its current state.
        switch(ent->sectionState){
            case INIT:
                if(((ent->team2 == TEAM_FREE) ? (TeamCount(-1, TEAM_RED, NULL ) + TeamCount(-1, TEAM_BLUE, NULL )) : (TeamCount(-1, (team_t)ent->team2, NULL ))) >= ent->min_players){
                    // Section should be open, so hide the linking entities.
                    ent->sectionState = OPENED;
                    g_sectionAddOrDelInstances(ent, qfalse);
                }else{
                    ent->sectionState = CLOSED;
                }
                break;
            case CLOSED:
                if(((ent->team2 == TEAM_FREE) ? (TeamCount(-1, TEAM_RED, NULL ) + TeamCount(-1, TEAM_BLUE, NULL )) : (TeamCount(-1, (team_t)ent->team2, NULL ))) >= ent->min_players){
                    // Open the section.
                    ent->sectionState = OPENING;
                    G_Broadcast(va("%s ^7will be opened in %0.f seconds!", ent->message, ent->wait), BROADCAST_GAME, NULL);
                    G_printInfoMessageToAll("%s will be opened in %0.f seconds.", ent->message2 + 1, ent->wait);
                }
                break;
            case OPENED:
                if(((ent->team2 == TEAM_FREE) ? (TeamCount(-1, TEAM_RED, NULL ) + TeamCount(-1, TEAM_BLUE, NULL )) : (TeamCount(-1, (team_t)ent->team2, NULL ))) < ent->min_players){
                    // Close the section.
                    ent->sectionState = CLOSING;
                    G_Broadcast(va("%s ^7will be closed in %0.f seconds!", ent->message, ent->wait), BROADCAST_GAME, NULL);
                    G_printInfoMessageToAll("%s will be closed in %0.f seconds.", ent->message2 + 1, ent->wait);
                }
                break;
            case CLOSING:
                // Close it now, the wait has passed.
                ent->sectionState = CLOSED;
                g_sectionAddOrDelInstances(ent, qtrue);
                if(ent->section < NOMIDDLE)
                    level.noLROpened[ent->section] = qfalse;

                G_Broadcast(va("%s ^7closed!", ent->message), BROADCAST_GAME, NULL);
                G_printInfoMessageToAll("%s is now closed.", ent->message2 + 1);
                break;
            case OPENING:
                // Open it now, the wait has passed.
                ent->sectionState = OPENED;
                g_sectionAddOrDelInstances(ent, qfalse);
                if(ent->section < NOMIDDLE)
                    level.noLROpened[ent->section] = qtrue;

                G_Broadcast(va("%s ^7opened!", ent->message), BROADCAST_GAME, NULL);
                G_printInfoMessageToAll("%s is now opened.", ent->message2 + 1);
                break;
            default:
                break;
        }
    }

    // Boe!Man 11/22/13: When's our next check?
    if(ent->sectionState == CLOSING || ent->sectionState == OPENING){
        ent->nextthink = level.time + (int)ent->wait * 1000;
    }else{
        ent->nextthink = level.time + 10000;
    }
}

void g_blockSection(gentity_t *ent, int section){
    if(section < NOMIDDLE){
        G_SpawnVector("origin", "0", level.noLR[section]);

        // Boe!Man 11/26/13: Not depending on the original state, if this check fails, it's always disabled..
        if(level.noLR[section][2] == 0){
            level.noLRActive[section] = qfalse;
        }
    }

    // Boe!Man 11/21/13: The entity is found.
    level.noLRMWEntFound[section] = qtrue;

    // Boe!Man 11/21/13: Is auto nolower enabled?
    if(ent->autoSection && strstr(ent->autoSection, "yes") && ent->min_players > 0){
        if(!ent->wait){ // There should be a delay. Default is to wait 10 seconds.
            ent->wait = 10;
        }

        // Check if a team is defined.
        if(ent->team && strlen(ent->team) > 0){
            if(strstr(ent->team, "red")){
                ent->team2 = TEAM_RED;
            }else if(strstr(ent->team, "blue")){
                ent->team2 = TEAM_BLUE;
            }else if(strstr(ent->team, "r")){
                ent->team2 = TEAM_RED;
            }else if(strstr(ent->team, "b")){
                ent->team2 = TEAM_BLUE;
            }else{ // All.
                ent->team2 = TEAM_FREE;
            }
        }else{ // All.
            ent->team2 = TEAM_FREE;
        }

        // The think function needs to know what section is about to be closed/opened.
        ent->section = section;
        ent->sectionState = INIT;

        // Boe!Man 11/21/13: Create the event.
        ent->think = g_sectionAutoCheck;
        ent->nextthink = level.time + 1000; // Check every 10 seconds, except the first time (init).
    }else{ // No auto system.
        G_FreeEntity(ent);
    }
}

void g_checkSectionState(){
    // Nolower.
    if(g_useNoLower.integer){
        level.noLRActive[NOLOWER] = qtrue;
        level.autoLRMWActive[NOLOWER] = qtrue;
    }

    // Noroof.
    if(g_useNoRoof.integer){
        level.noLRActive[NOROOF] = qtrue;
        level.autoLRMWActive[NOROOF] = qtrue;
    }

    // Nomiddle.
    if(g_useNoMiddle.integer){
        level.autoLRMWActive[NOMIDDLE] = qtrue;
    }

    // Nowhole.
    if(g_useNoWhole.integer){
        level.autoLRMWActive[NOWHOLE] = qtrue;
    }
}

void nolower(gentity_t *ent){
    static char message[24];

    strncpy(message, G_ColorizeMessage("Lower"), sizeof(message));
    ent->message = message;
    ent->message2 = "\\Lower";
    g_blockSection(ent, NOLOWER);
}

void noroof(gentity_t *ent){
    static char message[24];

    strncpy(message, G_ColorizeMessage("Roof"), sizeof(message));
    ent->message = message;
    ent->message2 = "\\Roof";
    g_blockSection(ent, NOROOF);
}

void nomiddle(gentity_t *ent){
    static char message[24];

    strncpy(message, G_ColorizeMessage("Middle"), sizeof(message));
    ent->message = message;
    ent->message2 = "\\Middle";
    g_blockSection(ent, NOMIDDLE);
}

void nowhole(gentity_t *ent){
    static char message[24];

    strncpy(message, G_ColorizeMessage("Whole"), sizeof(message));
    ent->message = message;
    ent->message2 = "\\Whole";
    g_blockSection(ent, NOWHOLE);
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
    char        *fxName;

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

/*
=================================================================================
seekers

Entity that lets a user define the team balance.
=================================================================================
*/

void SP_seekers(gentity_t* ent)
{
    int i, tempInt;

    // We loop through the command, and check if we can find something useful.
    // The even teams command blindly checks whatever the user put here, it's up to the server owner to declare valid values.
    for(i = 1; i <= 16; i++){
        G_SpawnInt(va("%i", i), "-1", &tempInt);

        if(tempInt){
            level.customETHiderAmount[i-1] = tempInt;
        }
    }
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
    trap_LinkEntity(other);
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

    for (i = 0; i < level.numConnectedClients; i++)
    {
        gentity_t* other = &g_entities[level.sortedClients[i]];

        if (other->client->pers.connected != CON_CONNECTED)
        {
            continue;
        }

        // Skip clients that are spectating
        if (G_IsClientSpectating(other->client) || G_IsClientDead(other->client))
        {
            continue;
        }

        G_UndoAdjustedClientBBox(other);

        // Relink the entity into the world
        trap_LinkEntity(other);
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
    float   leanOffset;
    vec3_t  up;
    vec3_t  right;

    // adjust origin for leaning
    //BG_ApplyLeanOffset(&ent->client->ps, org);//ent->r.currentOrigin);
    leanOffset = (float)(ent->client->ps.leanTime - LEAN_TIME) / LEAN_TIME * LEAN_OFFSET;

    //Ryan
    //leanOffset *= g_adjLeanOffset.value;
    leanOffset *= BBOX_LEAN_OFFSET;
    //Ryan

    AngleVectors(ent->client->ps.viewangles, NULL, right, up);
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

/*
===================
G_PlaceString   //stole code from CG_Placestring
===================
*/
const char  *G_PlaceString(int rank) {
    static char str[64];
    char    *s, *t;

    if (rank & RANK_TIED_FLAG) {
        rank &= ~RANK_TIED_FLAG;
        t = "Tied for ";
    }
    else {
        t = "";
    }

    if (rank == 1) {
        s = S_COLOR_BLUE "1st" S_COLOR_WHITE;       // draw in blue
    }
    else if (rank == 2) {
        s = S_COLOR_RED "2nd" S_COLOR_WHITE;        // draw in red
    }
    else if (rank == 3) {
        s = S_COLOR_YELLOW "3rd" S_COLOR_WHITE;     // draw in yellow
    }
    else if (rank == 11) {
        s = "11th";
    }
    else if (rank == 12) {
        s = "12th";
    }
    else if (rank == 13) {
        s = "13th";
    }
    else if (rank % 10 == 1) {
        s = va("%ist", rank);
    }
    else if (rank % 10 == 2) {
        s = va("%ind", rank);
    }
    else if (rank % 10 == 3) {
        s = va("%ird", rank);
    }
    else {
        s = va("%ith", rank);
    }

    Com_sprintf(str, sizeof(str), "%s%s", t, s);
    return str;
}

/*
================
G_Refresh
================
*/

void G_Refresh(gentity_t *ent)
{
    if (level.time <= level.gametypeStartTime + 5000) {
        G_printInfoMessage(ent, "You shouldn't refresh at the start of a new round.");
        return;
    }
    else if (current_gametype.value == GT_HZ) {
        if (ent->client->sess.killtime > level.time) { // Boe!Man 7/15/11: He's not allowed to kill himself. Inform him and return.
            if ((ent->client->sess.killtime - level.time) > 1000) { // Boe!man 7/15/11: The counter should never show 0.. Just kill if it's actually under 1.
                trap_SendServerCommand(ent->client - &level.clients[0], va("print\"^3[H&Z] ^7You cannot refresh yourself for another %i second(s).\n\"", (ent->client->sess.killtime - level.time) / 1000));
                return;
            }
        }
    }

    if (!G_IsClientSpectating(ent->client))
    {
        ent->flags &= ~FL_GODMODE;
        ent->client->ps.stats[STAT_HEALTH] = ent->health = -999;
        player_die(ent, ent, ent, 100000, MOD_REFRESH, HL_NONE, vec3_origin);
    }

    // Empty the stats.
    G_EmptyStatsMemory(ent);

    ent->client->pers.enterTime = level.time;
    ent->client->ps.persistant[PERS_SCORE] = 0;
    ent->client->sess.score = 0;
    ent->client->sess.deaths = 0;
    ent->client->sess.kills = 0;
    ent->client->sess.killsAsZombie = 0;
    ent->client->sess.teamkillDamage = 0;
    ent->client->sess.teamkillForgiveTime = 0;
    ent->client->pers.statinfo.lastclient_hurt = -1;
    ent->client->pers.statinfo.lasthurtby = -1;
    ent->client->pers.statinfo.lastKillerHealth = -1;
    ent->client->pers.statinfo.lastKillerArmor = -1;
}

/*
================
G_Obituary
================
*/

void G_Obituary(gentity_t *target, gentity_t *attacker, int mod, attackType_t attack, int hitLocation)
{
    int             targ, killer, gender, attackt, weapon;
    char            targetName[64];
    char            killerName[64];
    const char      *targetColor;
    const char      *killerColor;
    char            message[256];
    char            broadcast[1024];
    char            *message2;
    char            *message3;
    char            *bodypartstr;
    qboolean        headShot = qfalse;
    qboolean        statOk = qfalse;
    statinfo_t      *atrstat = &attacker->client->pers.statinfo;

    if (!attacker)
        return;
    if (!target)
        return;
    if (!attacker->client)
        return;

    killerColor = S_COLOR_WHITE;
    targetColor = S_COLOR_WHITE;
    message2 = "";
    message3 = "";

    if (!level.gametypeData->teams || (level.gametypeData->teams && !OnSameTeam(target, attacker)))
    {
        statOk = qtrue;
    }

    //was the kill hit a HEADSHOT?
    hitLocation = hitLocation & (~HL_DISMEMBERBIT);
    if (hitLocation == HL_HEAD)// && statOk)
    {
        message3 = "{^3HeaDShoT^7}";
        attackt = attacker->client->pers.statinfo.attack;
        weapon = attacker->client->pers.statinfo.weapon;
        headShot = qtrue;
        //add to the total headshot count for this player
        atrstat->headShotKills++;
        atrstat->weapon_headshots[attackt * level.wpNumWeapons + weapon]++;
    }

    targ = target->s.number;

    if (targ < 0 || targ >= MAX_CLIENTS)
    {
        Com_Error(ERR_FATAL, "G_Obituary: target out of range");
    }
    Q_strncpyz(targetName, target->client->pers.netname, sizeof(targetName));
    strcat(targetName, S_COLOR_WHITE);

    //find out who or what killed the client
    if (attacker->client)
    {
        killer = attacker->s.number;
        Q_strncpyz(killerName, attacker->client->pers.netname, sizeof(killerName));
        strcat(killerName, S_COLOR_WHITE);
    }
    else
    {
        killer = ENTITYNUM_WORLD;
    }

    // Play the death sound, water if they drowned
    if (mod == MOD_WATER)
    {
        Boe_ClientSound(target, G_SoundIndex("sound/pain_death/mullins/drown_dead.mp3", qtrue));
    }
    else        //play a random death sound out of 3 possible sounds
    {
        switch (level.time % 3)
        {
        case 0:
            Boe_ClientSound(target, G_SoundIndex("sound/pain_death/male/die01.mp3", qtrue));
            break;

        case 1:
            Boe_ClientSound(target, G_SoundIndex("sound/pain_death/male/die04.mp3", qtrue));
            break;

        case 2:
            Boe_ClientSound(target, G_SoundIndex("sound/pain_death/male/die05.mp3", qtrue));
            break;

        default:
            Boe_ClientSound(target, G_SoundIndex("sound/pain_death/male/die01.mp3", qtrue));
            break;
        }
    }
    // Play the frag sound, and make sure its not played more than every 250ms
    if (attacker->client)
    {
        //If the attacker killed themselves play the selffrag sound
        if (killer == targ)
        {
            Boe_ClientSound(target, G_SoundIndex("sound/self_frag.mp3", qtrue));
        }
        else if (headShot) {
            //if they use rpm client-side the client will
            //handle the sound etc...
            #ifndef _GOLD
            if (attacker->client->sess.rpmClient)
            {
                trap_SendServerCommand(attacker->s.number, va("headshot \"Headshot\n\""));
                Boe_ClientSound(attacker, G_SoundIndex("sound/npc/col8/blakely/niceshot.mp3"));
            }
            //if not we'll send them the sound etc..
            else// if(g_allowDeathMessages.integer)
            {
            #else
                G_Broadcast("Headshot!", BROADCAST_GAME, attacker);
                Boe_ClientSound(attacker, G_SoundIndex("sound/npc/col8/blakely/niceshot.mp3", qtrue));
            #endif // not _GOLD

            #ifndef _GOLD
            }
            #endif // not _GOLD

            //if we can show kills we'll display the
            //heashot message with the "you killed" message
            if (level.gametypeData->showKills)
            {
                message2 = G_ColorizeMessage("Headshot!\n");
            }

            //We'll tack this on to the end of the kill broadcast to all
            message3 = "{^3HeaDShoT^7}";

        }
        //if not headshot, suicide or tk just play the normal sound
        else
        {
            Boe_ClientSound(attacker, G_SoundIndex("sound/frag.mp3", qtrue));
        }

        //set the time here now
        attacker->client->lastKillTime = level.time;
    }
    //set the teamcolor of the killed client
    switch (target->client->sess.team)
    {
    case TEAM_RED:
        targetColor = S_COLOR_RED;
        break;

    case TEAM_BLUE:
        targetColor = S_COLOR_BLUE;
        break;
    }

    gender = GENDER_MALE;

    if (target->client->pers.identity &&
        strstr(target->client->pers.identity->mCharacter->mModel, "female"))
    {
        gender = GENDER_FEMALE;
    }

    memset(message, 0, sizeof(message));

    switch (mod)
    {
        case MOD_SUICIDE:
            strncpy(message, "suicides", sizeof(message));
            break;
        case MOD_FALLING:
            if (gender == GENDER_FEMALE)
                strncpy(message, "fell to her death", sizeof(message));
            else
                strncpy(message, "fell to his death", sizeof(message));
            break;
        case MOD_CRUSH:
            strncpy(message, "was squished", sizeof(message));
            break;
        case MOD_WATER:
            strncpy(message, "sank like a rock", sizeof(message));
            break;
        case MOD_TARGET_LASER:
            strncpy(message, "saw the light", sizeof(message));
            break;
        case MOD_TRIGGER_HURT:
        case MOD_TRIGGER_HURT_NOSUICIDE:
            strncpy(message, "was in the wrong place", sizeof(message));
            break;
        case MOD_TEAMCHANGE:
            return;
        case MOD_CAR:
            strncpy(message, "was killed in a terrible car accident", sizeof(message));
            break;
        case MOD_POP:
            return;
            //RxCxW - 1.13.2005 - Dugup (Unplant) #MOD
        case MOD_DUGUP:
            strncpy(message, "Looks like someone Dug too deep!", sizeof(message));
            break;
        case MOD_BURN:
            strncpy(message, "was BURNT to a Crisp!", sizeof(message));
            break;
            //End
        default:
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
            if (gender == GENDER_FEMALE)
                strncpy(message, "blew herself up", sizeof(message));
            else if (gender == GENDER_NEUTER)
                strncpy(message, "blew itself up", sizeof(message));
            else
                strncpy(message, "blew himself up", sizeof(message));
            break;

        case MOD_REFRESH:
            strncpy(message, "Refreshed", sizeof(message));
            break;

        default:
            if (gender == GENDER_FEMALE)
                strncpy(message, "killed herself", sizeof(message));
            else if (gender == GENDER_NEUTER)
                strncpy(message, "killed itself", sizeof(message));
            else
                strncpy(message, "killed himself", sizeof(message));
            break;
        }
    }
    if (strlen(message) > 0)
    {
        trap_SendServerCommand(-1, va("print \"%s%s %s.\n\"", targetColor, targetName, message));
        return;
    }
    // check for kill messages
    if (level.gametypeData->showKills)
    {
        if (attacker && attacker->client)
        {
            //if not a team game display the kill and the rank
            if (!level.gametypeData->teams)
            {

                G_Broadcast(va("%sYou killed %s%s\n%s place with %i\n\"", // %s after \n
                    message2,
                    targetColor,
                    targetName,
                    G_PlaceString(attacker->client->ps.persistant[PERS_RANK] + 1),
                    attacker->client->ps.persistant[PERS_SCORE]
                    ), BROADCAST_GAME, attacker);
            }
            else //if team just display the kill
            {
                G_Broadcast(va("%sYou killed %s%s\n\"", message2, targetColor, targetName), BROADCAST_GAME, attacker);
            }
        }
    }
    // check for double client messages
    message2 = "";

    if (killer != ENTITYNUM_WORLD)
    {
        ///RxCxW - 01.08.06 - 09:31pm
        switch (attacker->client->sess.team)
        {
        case TEAM_RED:
            killerColor = S_COLOR_RED;
            break;

        case TEAM_BLUE:
            killerColor = S_COLOR_BLUE;
            break;
        }
        ///End  - 01.08.06 - 09:31pm

        // Get the shot location.
        switch (hitLocation)
        {
        case HL_FOOT_RT:
        case HL_FOOT_LT:
            bodypartstr = "in the foot ";
            break;
        case HL_LEG_UPPER_RT:
        case HL_LEG_UPPER_LT:
        case HL_LEG_LOWER_RT:
        case HL_LEG_LOWER_LT:
            bodypartstr = "in the leg ";
            break;
        case HL_HAND_RT:
        case HL_HAND_LT:
            bodypartstr = "in the hand ";
            break;
        case HL_ARM_RT:
        case HL_ARM_LT:
            bodypartstr = "in the arm ";
            break;
        case HL_HEAD:
            bodypartstr = "in the head ";
            break;
        case HL_WAIST:
            bodypartstr = "in the waist ";
            break;
        case HL_BACK_RT:
        case HL_BACK_LT:
        case HL_BACK:
            bodypartstr = "in the back ";
            break;
        case HL_CHEST_RT:
        case HL_CHEST_LT:
        case HL_CHEST:
            bodypartstr = "in the chest ";
            break;
        case HL_NECK:
            bodypartstr = "in the neck ";
            break;
        default:
            bodypartstr = "";
            break;
        }

        switch (mod)
        {
        case MOD_KNIFE:
            strncpy(message, va("was sliced %sby", bodypartstr), sizeof(message));
            if (statOk)
            {
                atrstat->knifeKills++;
            }
            break;

        case MOD_USAS_12_SHOTGUN:
        case MOD_M590_SHOTGUN:
            if (attack == ATTACK_ALTERNATE)
            {
                strncpy(message, va("was bludgeoned %sby", bodypartstr), sizeof(message));
                message2 = va("'s %s", weaponParseInfo[mod].mName);
            }
            else
            {
                strncpy(message, va("was pumped full of lead %sby", bodypartstr), sizeof(message));
                message2 = va("'s %s", weaponParseInfo[mod].mName);
            }
            break;

        case MOD_M1911A1_PISTOL:
        case MOD_USSOCOM_PISTOL:
        #ifdef _GOLD
        case MOD_SILVER_TALON:
        #endif // _GOLD
            if (attack == ATTACK_ALTERNATE)
            {
                strncpy(message, va("was pistol whipped %sby", bodypartstr), sizeof(message));
                message2 = va("'s %s", weaponParseInfo[mod].mName);
            }
            else
            {
                strncpy(message, ("was shot %sby", bodypartstr), sizeof(message));
                message2 = va("'s %s", weaponParseInfo[mod].mName);
            }
            break;

        case MOD_AK74_ASSAULT_RIFLE:
            if (attack == ATTACK_ALTERNATE)
            {
                strncpy(message, va("was stabbed %sby", bodypartstr), sizeof(message));
                message2 = va("'s %s", weaponParseInfo[mod].mName);
            }
            else
            {
                strncpy(message, va("was shot %sby", bodypartstr), sizeof(message));
                message2 = va("'s %s", weaponParseInfo[mod].mName);
            }
            break;

        case MOD_M4_ASSAULT_RIFLE:
            if (attack == ATTACK_ALTERNATE)
            {
                strncpy(message, "was detonated by", sizeof(message));
                message2 = "'s M203";

                if (statOk)
                {
                    atrstat->explosiveKills++;
                }
            }
            else
            {
                strncpy(message, va("was shot %sby", bodypartstr), sizeof(message));
                message2 = va("'s %s", weaponParseInfo[mod].mName);
            }
            break;

        case MOD_M60_MACHINEGUN:
        case MOD_MICRO_UZI_SUBMACHINEGUN:
        case MOD_M3A1_SUBMACHINEGUN:
        #ifdef _GOLD
        case MOD_SIG551:
        case MOD_MP5:
        #endif // _GOLD
            strncpy(message, va("was shot %sby", bodypartstr), sizeof(message));
            message2 = va("'s %s", weaponParseInfo[mod].mName);
            break;

        case MOD_MSG90A1_SNIPER_RIFLE:
            strncpy(message, va("was sniped %sby", bodypartstr), sizeof(message));
            message2 = va("'s %s", weaponParseInfo[mod].mName);
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
            strncpy(message, "was detonated by", sizeof(message));
            if (current_gametype.value != GT_HZ) {
                message2 = va("'s %s", weaponParseInfo[mod].mName);
            }
            else {
                if (mod == WP_L2A2_GRENADE || mod == altAttack(WP_L2A2_GRENADE)) {
                    message2 = "'s Claymore";
                }
                else {
                    message2 = va("'s %s", weaponParseInfo[mod].mName);
                }
            }
            if (statOk)
            {
                if (mod == MOD_ANM14_GRENADE)
                {
                    atrstat->hitcount++;
                    atrstat->accuracy = (float)atrstat->hitcount / (float)atrstat->shotcount * 100;
                    atrstat->weapon_hits[((mod > 256) ? ATTACK_ALTERNATE : ATTACK_NORMAL) * level.wpNumWeapons + normalAttackMod(mod)]++;
                }

                atrstat->explosiveKills++;
            }
            break;

        case MOD_TELEFRAG:
            strncpy(message, "was telefragged by", sizeof(message));
            break;

        default:
            strncpy(message, "was killed by", sizeof(message));
            break;
        }

        if (strlen(message) > 0){
            Com_sprintf(broadcast, sizeof(broadcast), "print \"%s%s %s %s%s%s %s\n\"\0", targetColor, targetName, message, killerColor, killerName, message2, message3);
            trap_SendServerCommand(-1, broadcast);
            return;
        }
    }

    // we don't know what it was
    trap_SendServerCommand(-1, va("print \"%s%s died.\n\"", targetColor, targetName));
}

/*
==================
G_unPause

Main logic for unpausing the game.
==================
*/

void G_unPause(gentity_t *adm)
{
    int         i;
    gentity_t   *ent;
    //  gentity_t   *tent;

    if (!level.pause)
    {
        G_printInfoMessage(adm, "The game is currently not paused.");
        return;
    }

    if (level.time >= level.unpausetime + 1000)
    {
        level.unpausetime = level.time;
        level.pause--;

        G_Broadcast(va("\\Resuming in: %d sec", level.pause), BROADCAST_GAME, NULL);

        Boe_GlobalSound(G_SoundIndex("sound/misc/events/buzz02.wav", qtrue));

        if (!level.pause)
        {
            level.unpausetime = 0;

            // Boe!Man 4/22/12: When in CTF, flags should be unfreezed as well. Let the gt know this by sending this command.
            if (current_gametype.value == GT_CTF) {
                trap_GT_SendEvent(GTEV_PAUSE, level.time, 0, 0, 0, 0, 0);
            }

            ///RxCxW - 08.30.06 - 03:06pm #reset clients (scoreboard) display time
            trap_SetConfigstring(CS_LEVEL_START_TIME, va("%i", level.startTime));
            trap_SetConfigstring(CS_GAMETYPE_TIMER, va("%i", level.gametypeRoundTime));
            ///End  - 08.30.06 - 03:06pm

            if (level.gametypeData->respawnType == RT_INTERVAL)
            {
                level.gametypeRespawnTime[TEAM_RED] = level.time + g_respawnInterval.integer * 1000;
                level.gametypeRespawnTime[TEAM_BLUE] = level.time + g_respawnInterval.integer * 1000;
                level.gametypeRespawnTime[TEAM_FREE] = level.time + g_respawnInterval.integer * 1000;
            }

            for (i = 0; i< level.maxclients; i++)
            {
                ent = g_entities + i;
                if (!ent->inuse)
                {
                    continue;
                }
                if (!ent->client)
                {
                    continue;
                }

                if (ent->client->sess.pausespawn == qtrue) {
                    ent->client->ps.RealSpawnTimer = level.time + ent->client->ps.oldTimer;
                }
                ent->client->ps.pm_type = PM_NORMAL;
            }
            G_Broadcast("Go!", BROADCAST_CMD, NULL);
            trap_SetConfigstring(CS_GAMETYPE_MESSAGE, va("%i,@Go!", level.time + 1000));
        }
    }
}

/*
==================
G_ReadyCheck

For checking who is ready during warmup.
==================
*/

void G_ReadyCheck(gentity_t *ent)
{
#ifdef _SOF2_BOTS
    if (ent->r.svFlags & SVF_BOT)
    {
        if (!ent->client->pers.ready)
        {
            ent->client->pers.ready = 1;
        }
        return;
    }
#endif
    if (level.time < ent->client->pers.readyMessageTime + 3000)
    {
        return;
    }

    if (g_doWarmup.integer == 2 && !ent->client->pers.ready)
    {
        G_Broadcast("You are not ready for the match\nPlease type ^ 1 / ready ^ 7in console", BROADCAST_GAME, ent);
        ent->client->pers.readyMessageTime = level.time;
    }
}

/*
==================
G_ReadyUp

Client informs server being ready.
==================
*/

void G_ReadyUp(gentity_t *ent)
{
    if (ent->client->pers.ready)
    {
        return;
    }

    if (level.warmupTime > -1)
    {
        return;
    }

    ent->client->pers.ready = 1;
    trap_SendServerCommand(ent - g_entities, va("cp \"@ \n\"")); //send a blank message to clear the readyup message
}

/*
==============
RemoveColorEscapeSequences
==============
*/

void RemoveColorEscapeSequences(char *text) {
    int i, l;

    l = 0;
    for (i = 0; text[i]; i++) {
        if (Q_IsColorString(&text[i])) {
            i++;
            continue;
        }
        if (text[i] > 0x7E)
            continue;
        text[l++] = text[i];
    }
    text[l] = '\0';
}

/*
==============
G_RemoveAdditionalCarets
10/17/15 - 7:22 PM
Removes additional carets from text.
==============
*/

void G_RemoveAdditionalCarets(char *text)
{
    int i, l;

    l = 0;
    for (i = 0; text[i]; i++) {
        if (text[i] == '^' &&
            (text[i + 1] == '^' || i + 1 == strlen(text))) {
            i++;
            continue;
        }

        text[l++] = text[i];
    }

    text[l] = '\0';
}

/*
==============
G_Hash
11/14/15 - 11:39 AM
Hash function reverse engineered from the engine,
so we can manually enable/disable it with a CVAR.
==============
*/

void G_Hash()
{
    int             i, hashCharVal, hashSignedIntVal;
    float           hashFloatVal;
    char            c;
    char            cvarName[MAX_TOKEN_CHARS];
    char            hashNewVal[MAX_TOKEN_CHARS];
    char            cvarNewVal[MAX_TOKEN_CHARS];
    unsigned int    hash = 0;

    // Check if the server started with hash enabled.
    if(!g_enableHash.integer){
        Com_Printf("Hash is disabled, the server owner can enable it with g_enableHash.\n");
        return;
    }

    // Check parameters specified.
    if ( trap_Argc() != 3 ){
        Com_Printf("usage: hash <cvar> <string>\n");
        return;
    }

    // Bitshift result while there is data.
    trap_Argv(2, hashNewVal, sizeof(hashNewVal));
    for(i = 0; i < strlen(hashNewVal); i++){
        c = hashNewVal[i];
        hashCharVal = 16 * hash + c - 97;
        hash ^= (hash >> 28) ^ hashCharVal;
    }

    // Cast to float and signed integer, and see if they are the same.
    hashFloatVal = hash;
    hashSignedIntVal = hashFloatVal;
    if ( hashSignedIntVal == hashFloatVal){
        // Safe to set it as a regular integer.
        Com_sprintf(cvarNewVal, sizeof(cvarNewVal), "%i", hashSignedIntVal);
    }else{
        // Must be set as float, different values.
        Com_sprintf(cvarNewVal, sizeof(cvarNewVal), "%f", hashFloatVal);
    }

    // Get CVAR name and set the new data.
    trap_Argv(1, cvarName, sizeof(cvarName));
    trap_Cvar_Set(cvarName, cvarNewVal);

    // Always print result, unlike the real function where it is actually a Com_DPrintf.
    Com_Printf("Cvar %s given hashed integer %d\n", cvarName, hash);
}

/*
==============
G_switchToNextMapInCycle
11/17/15 - 11:21 PM
Checks next map in the
map cycle played.

If it is found to be
invalid, it is skipped
and a will continue to switch
until a replacement is arranged.
==============
*/

void G_switchToNextMapInCycle(qboolean force)
{
    char            currentMap[12], nextMap[MAX_QPATH];
    char            command[MAX_QPATH];
    int             mapIndex, mapStart, mapOffset, i;
    TGenericParser2 GP2;
    TGPGroup        topGroup;
    TGPGroup        mcGroup;
    TGPGroup        mapSubGroup;
    fileHandle_t    mapFile;
    char            *mapStrStart, *mapStrEnd;
    qboolean        tryNextMap = qfalse;
    char            mapCommand[32];

    // Figure out the map command we should execute.
    if(level.mcKillServer){
        // Server expects a server kill to reset the level.time and other game specifics.
        Q_strncpyz(mapCommand, "killserver; map mp_shop\n", sizeof(mapCommand));
    }else{
        // Regular map switch: we don't need to kill the server.
        Q_strncpyz(mapCommand, "map mp_shop\n", sizeof(mapCommand));
    }

    // Check if we're running a mapcycle.
    if (!*g_mapcycle.string || !Q_stricmp (g_mapcycle.string, "none")){
        G_LogPrintf("Not actively playing a mapcycle, sv_mapcycle is empty or set to none!\n");

        if(force){
            G_LogPrintf("Server expects a switch - switching to mp_shop instead.\n");
            trap_SendConsoleCommand(EXEC_APPEND, mapCommand);
        }
        return;
    }

    // Check last map in the cycle.
    trap_Cvar_VariableStringBuffer("sv_lastmapcycle", currentMap, sizeof(currentMap));
    if(!strlen(currentMap)){
        // Want to switch to the first map in the cycle.
        mapStart = 0;
    }else if(strlen(currentMap) >= 4){
        // We've played a map previously,
        // check this value and add one to it.
        mapStart = atoi((char *)currentMap + 3) + 1;
    }

    // Parse the mapcycle file.
    GP2 = trap_GP_ParseFile(g_mapcycle.string, qtrue, qfalse);
    if(!GP2){
        Com_Printf("ERROR: trying to read the mapcycle file, but it failed. Has it been moved or altered in the meantime?\n");

        if(force){
            G_LogPrintf("Server expects a switch - switching to mp_shop instead.\n");
            trap_SendConsoleCommand(EXEC_APPEND, mapCommand);
        }
        return;
    }

    // Top group.
    topGroup = trap_GP_GetBaseParseGroup(GP2);
    if (!topGroup){
        Com_Printf("ERROR: trying to read the mapcycle file, but it failed. Has it been moved or altered in the meantime?\n");

        if(force){
            G_LogPrintf("Server expects a switch - switching to mp_shop instead.\n");
            trap_SendConsoleCommand(EXEC_APPEND, mapCommand);
        }
        return;
    }

    // Group in top group should only contain the "mapcycle" sub group.
    mcGroup = trap_GPG_FindSubGroup(topGroup, "mapcycle");
    if (!mcGroup){
        Com_Printf("ERROR: trying to read the mapcycle file, but it failed. Has it been moved or altered in the meantime?\n");

        if(force){
            G_LogPrintf("Server expects a switch - switching to mp_shop instead.\n");
            trap_SendConsoleCommand(EXEC_APPEND, mapCommand);
        }
        return;
    }

    // Loop through the available maps.
    mapIndex = mapStart;
    for(;;){
        // Grab the mapx sub group.
        // This could fail (ran out of maps), so start back at 0 if this fails.
        mapSubGroup = trap_GPG_FindSubGroup (mcGroup, va("map%d", mapIndex));
        if (!mapSubGroup){
            if(mapIndex == 0){
                if(mapStart > 1){
                    // map0 seems to be invalid, try map1 as backup.
                    G_LogPrintf("ERROR: map0 in the mapcycle is missing!\n");
                    tryNextMap = qtrue;
                    mapIndex++;
                    continue;
                }else{
                    // No recursive loops if we're not coming from a higher map.
                    level.mcSkipMaps = 0;
                    G_LogPrintf("ERROR: map0 in the mapcycle is missing!\n");
                    G_LogPrintf("Bailing out by switching to mp_shop for your sake...\n");
                    trap_SendConsoleCommand(EXEC_APPEND, mapCommand);
                    break;
                }
            }else if(mapIndex == 1 && tryNextMap){
                level.mcSkipMaps = 0;
                G_LogPrintf("ERROR: Tried map1, but it can also not be found!\n");
                G_LogPrintf("Bailing out by switching to mp_shop for your sake...\n");
                trap_SendConsoleCommand(EXEC_APPEND, mapCommand);
                break;
            }else{
                mapIndex = 0;
                continue;
            }
        }

        // Parse out the command.
        trap_GPG_FindPairValue (mapSubGroup, "command", "", command);
        if (!command[0]){
            // Empty command, continue to the next map in the cycle.
            G_LogPrintf("ERROR: empty command in the mapcycle for map %d\n", mapIndex);
            goto advanceNextMap;
        }

        // Figure out what map we're trying to play.
        mapStrStart = strstr(command, "devmap ");
        if(mapStrStart == NULL){
            mapStrStart = strstr(command, "altmap ");
            if(mapStrStart == NULL){
                mapStrStart = strstr(command, "map ");
                if(mapStrStart == NULL){
                    // No "map" in the command.
                    G_LogPrintf("ERROR: no map specified to switch to in the mapcycle for map %d\n", mapIndex);
                    goto advanceNextMap;
                }
                mapOffset = 4;
            }else{
                mapOffset = 7;
            }
        }else{
            mapOffset = 7;
        }

        // Determine end of the string and resulting map name.
        mapStrEnd = strstr(mapStrStart, ";");
        if(!mapStrEnd){
            mapStrEnd = strrchr(mapStrStart, 0);
        }

        mapStrStart += mapOffset;
        Q_strncpyz(nextMap, mapStrStart, (int)(mapStrEnd - mapStrStart) + 1);

        // Remove possible whitespace.
        for(i = strlen(nextMap); i >= 0; i--){
            if(!isspace(nextMap[i])){
                break;
            }else{
                nextMap[i] = '\0';
            }
        }

        // Now try to open the map we found.
        trap_FS_FOpenFile(va("maps/%s.bsp", nextMap), &mapFile, FS_READ);
        if(mapFile != 0){
            // Found a valid map!
            trap_FS_FCloseFile(mapFile);
            level.mcSkipMaps++;
            break;
        }else{
            G_LogPrintf("ERROR: invalid map in mapcycle for map entry %d: %s\n", mapIndex, nextMap);
        }

    advanceNextMap:
        if(mapIndex == mapStart && level.mcSkipMaps != 0){
            G_LogPrintf("ERROR: no maps available in the mapcycle to play, they are all invalid!\n");
            level.mcSkipMaps = 0;

            if(force){
                G_LogPrintf("Bailing out by switching to mp_shop for your sake...\n");
                trap_SendConsoleCommand(EXEC_APPEND, mapCommand);
            }
            break;
        }

        // Advance to the next map.
        level.mcSkipMaps++;
        mapIndex++;
    }
}

/*
==============
G_showItemLocations
5/28/17 - 7:05 AM
Determines whether the item locations
are to be known to the user.
==============
*/

qboolean G_showItemLocations()
{
    if(!g_objectiveLocations.integer){
        return qfalse;
    }

    if(cm_enabled.integer > 1){
        return qfalse;
    }

    if(current_gametype.value == GT_INF && g_caserun.integer){
        return qfalse;
    }

    return qtrue;
}

/*
==============
SortAlpha

Sorts char * array
alphabetically.
==============
*/

int QDECL SortAlpha(const void *a, const void *b)
{
    return strcmp(*(const char **)a, *(const char **)b);
}
