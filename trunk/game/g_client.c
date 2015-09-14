// Copyright (C) 2001-2002 Raven Software
//
#include "g_local.h"
#include "boe_local.h"

// g_client.c -- client functions that don't happen every frame

static vec3_t   playerMins = {-15, -15, -46};
static vec3_t   playerMaxs = {15, 15, 48};

#ifdef _SOF2_BOTS
// GRIM 15/04/2003 11:31PM
void G_RandomlyChooseOutfitting(gentity_t *ent, goutfitting_t *outfitting);
// GRIM
#endif

/*
================
G_AddClientSpawn

adds a spawnpoint to the spawnpoint array using the given entity for
origin and angles as well as the team for filtering teams.
================
*/
void G_AddClientSpawn ( gentity_t* ent, team_t team, qboolean monkey )
{
    static vec3_t   mins = {-15,-15,-45};
    static vec3_t   maxs = {15,15,46};
    vec3_t          end;
    vec3_t          newAngles;
    char            *newAngle;
    trace_t         tr;
    int             *spawnCount;
    gspawn_t        *spawns;

    // Drop it to the ground, and if it starts solid just throw it out
    VectorCopy ( ent->s.origin, end );
    end[2] -= 1024;

    tr.fraction = 0.0f;
    trap_Trace ( &tr, ent->s.origin, mins, maxs, end, ent->s.number, MASK_SOLID );

    // We are only looking for terrain collisions at this point
    if ( tr.contents & CONTENTS_TERRAIN )
    {
        // If its in the ground then throw it awway
        if ( tr.startsolid )
        {
            return;
        }
        // Drop it down to the ground now
        else if ( tr.fraction < 1.0f && tr.fraction > 0.0f )
        {
            VectorCopy ( tr.endpos, ent->s.origin );
            ent->s.origin[2] += 1.0f;
            tr.startsolid = qfalse;
        }
    }

    if ( tr.startsolid )
    {
        #ifdef _3DServer
        Com_Printf(S_COLOR_YELLOW "WARNING: %s starting in solid at %.2f,%.2f,%.2f\n", (monkey) ? "monkey_player" : "gametype_player", ent->s.origin[0], ent->s.origin[1], ent->s.origin[2]);
        #else
        Com_Printf(S_COLOR_YELLOW "WARNING: gametype_player starting in solid at %.2f,%.2f,%.2f\n", ent->s.origin[0], ent->s.origin[1], ent->s.origin[2]);
        #endif // _3DServer
    }

    #ifdef _3DServer
    if (monkey){
        spawns = &level.monkeySpawns[level.monkeySpawnCount];
        spawnCount = &level.monkeySpawnCount;
    }else{
        spawns = &level.spawns[level.spawnCount];
        spawnCount = &level.spawnCount;
    }
    #else
    spawns = &level.spawns[level.spawnCount];
    spawnCount = &level.spawnCount;
    #endif // _3DServer

    spawns->team = team;

    // Release the entity and store the spawn in its own array
    VectorCopy ( ent->s.origin, spawns->origin );

    // Boe!Man 7/22/13: We use this code for determining the angle, using a float isn't fool-proof here (as with NV_misc_bsp).
    if(G_SpawnVector("angles", "0 0 0", newAngles)){
        spawns->angles[0] = newAngles[0];
        spawns->angles[1] = newAngles[1];
        spawns->angles[2] = newAngles[2];
    }else if(G_SpawnString("angle", "0", &newAngle)){
        VectorClear(spawns->angles);
        sscanf(newAngle, "%f", &spawns->angles[1]);
    }else{
        VectorClear ( spawns->angles );
    }

    // Increase the spawn count
    (*spawnCount)++;
}

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -46) (16 16 48) initial
potential spawning position for deathmatch games.
The first time a player enters the game, they will be at an 'initial' spot.
Targets will be fired when someone spawns in on them.
*/
void SP_info_player_deathmatch( gentity_t *ent )
{
    // Cant take any more spawns!!
    if ( level.spawnCount >= MAX_SPAWNS )
    {
        G_FreeEntity ( ent );
        return;
    }

    G_AddClientSpawn ( ent, TEAM_FREE, qfalse );

    G_FreeEntity ( ent );
}

/*QUAKED info_player_intermission (1 0 1) (-16 -16 -46) (16 16 48)
The intermission will be viewed from this point.  Target an info_notnull for the view direction.
*/
void SP_info_player_intermission( gentity_t *ent )
{
}

/*
================
G_SpotWouldTelefrag
================
*/
qboolean G_SpotWouldTelefrag( gspawn_t* spawn )
{
    int         i, num;
    int         touch[MAX_GENTITIES];
    gentity_t   *hit;
    vec3_t      mins, maxs;

    VectorAdd( spawn->origin, playerMins, mins );
    VectorAdd( spawn->origin, playerMaxs, maxs );
    num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

    for (i=0 ; i<num ; i++)
    {
        hit = &g_entities[touch[i]];

        if ( hit->client)
        {
            if ( G_IsClientSpectating ( hit->client ) )
            {
                continue;
            }

            if ( G_IsClientDead ( hit->client ) )
            {
                continue;
            }

            return qtrue;
        }
    }

    return qfalse;
}

/*
================
G_SelectRandomSpawnPoint

go to a random point that doesn't telefrag
================
*/
gspawn_t* G_SelectRandomSpawnPoint ( team_t team, gclient_t *client )
{
    int         i;
    int         count, sCount;
    int         tfcount;
    gspawn_t    *spawns[MAX_SPAWNS];
    gspawn_t    *tfspawns[MAX_SPAWNS];

    count = 0;
    tfcount = 0;

#ifdef _3DServer
    if (client && client->sess.deadMonkey){
        sCount = level.monkeySpawnCount;
    }else{
        sCount = level.spawnCount;
    }
#else
    sCount = level.spawnCount;
#endif // _3DServer

    for ( i = 0; i < sCount; i ++ )
    {
        #ifdef _3DServer
        gspawn_t* spawn;
        if (client && client->sess.deadMonkey){
            spawn = &level.monkeySpawns[i];
        }else{
            spawn = &level.spawns[i];
        }
        #else
        gspawn_t* spawn = &level.spawns[i];
        #endif // _3DServer

        if ( team != -1 && team != spawn->team )
        {
            continue;
        }

        if ( G_SpotWouldTelefrag( spawn ) )
        {
            tfspawns[tfcount++] = spawn;
            continue;
        }

        spawns[ count++ ] = spawn;
    }

    // no spots that won't telefrag so just pick one that will
    if ( !count )
    {
        // No telefrag spots, just return NULL since there is no more to find
        if ( !tfcount )
        {
            return NULL;
        }

        // telefrag someone
        return tfspawns[ rand() % tfcount ];
    }
    ///RxCxW - 09.09.06 - 11:11pm #randomSpawnpoint
    ///return spawns[ rand() % count ];
    i = rand() % count;
    return spawns[ i ];
    ///End  - 09.09.06 - 11:11pm
}

/*
===========
G_SelectRandomSafeSpawnPoint

Select a random spawn point that is safe for the client to spawn at.  A safe spawn point
is one that is at least a certain distance from another client.
============
*/
gspawn_t* G_SelectRandomSafeSpawnPoint ( team_t team, float safeDistance, gclient_t *client )
{
    gspawn_t*   spawns[MAX_SPAWNS];
    float       safeDistanceSquared;
    int         count, sCount;
    int         i;

    // Square the distance for faster comparisons
    safeDistanceSquared = safeDistance * safeDistance;

    #ifdef _3DServer
    if (client && client->sess.deadMonkey){
        sCount = level.monkeySpawnCount;
    }else{
        sCount = level.spawnCount;
    }
    #else
    sCount = level.spawnCount;
    #endif // _3DServer

    // Build a list of spawns
    for ( i = 0, count = 0; i < sCount; i ++ )
    {
        int       j;

        #ifdef _3DServer
        gspawn_t* spawn;
        if (client && client->sess.deadMonkey){
            spawn = &level.monkeySpawns[i];
        }else{
            spawn = &level.spawns[i];
        }
        #else
        gspawn_t* spawn = &level.spawns[i];
        #endif // _3DServer

        // Ensure the team matches
        if ( team != -1 && team != spawn->team )
        {
            continue;
        }

        // Make sure this spot wont kill another player
        if ( G_SpotWouldTelefrag( spawn ) )
        {
            continue;
        }

        // Loop through connected clients
        for ( j = 0; j < level.numConnectedClients && count < MAX_SPAWNS; j ++ )
        {
            gentity_t* other = &g_entities[level.sortedClients[j]];
            vec3_t     diff;

            if ( other->client->pers.connected != CON_CONNECTED )
            {
                continue;
            }

            // Skip clients that are spectating or dead
            if ( G_IsClientSpectating ( other->client ) || G_IsClientDead ( other->client ) )
            {
                continue;
            }

            // on safe team, dont count this guy
            if ( level.gametypeData->teams && team == other->client->sess.team )
            {
                continue;
            }

            VectorSubtract ( other->r.currentOrigin, spawn->origin, diff );

            // Far enough away to qualify
            if ( VectorLengthSquared ( diff ) < safeDistanceSquared )
            {
                break;
            }
        }

        // If we didnt go through the whole list of clients then we must
        // have hit one that was too close.  But if we did go through teh whole
        // list then this spawn point is good to go
        if ( j >= level.numConnectedClients )
        {
            spawns[count++] = spawn;
        }
    }

    // Nothing found, try it at half the safe distance
    if ( !count )
    {
        // Gotta stop somewhere
        if ( safeDistance / 2 < 250 )
        {
            return G_SelectRandomSpawnPoint ( team, client );
        }
        else
        {
            return G_SelectRandomSafeSpawnPoint ( team, safeDistance / 2, client );
        }
    }

    // Spawn them at one of the spots
    return spawns[ rand() % count ];
}

/*
===========
G_SelectSpectatorSpawnPoint
============
*/
gspawn_t* G_SelectSpectatorSpawnPoint( void )
{
    static gspawn_t spawn;

    FindIntermissionPoint();

    VectorCopy( level.intermission_origin, spawn.origin );
    VectorCopy( level.intermission_angle, spawn.angles );

    return &spawn;
}

/*
===============
G_InitBodyQueue
===============
*/
void G_InitBodyQueue (void)
{
    gentity_t   *ent;
    int         max;

    if ( level.gametypeData->respawnType == RT_NONE )
    {
        level.bodySinkTime = 0;
        max = BODY_QUEUE_SIZE_MAX;
    }
    else
    {
        level.bodySinkTime = BODY_SINK_DELAY;
        max = BODY_QUEUE_SIZE;
    }

    level.bodyQueIndex = 0;
    for ( level.bodyQueSize = 0;
          level.bodyQueSize < max && level.bodyQueSize < level.maxclients;
          level.bodyQueSize++)
    {
        ent = G_Spawn();
        ent->classname = "bodyque";
        ent->neverFree = qtrue;
        level.bodyQue[level.bodyQueSize] = ent;
    }
}

/*
=============
BodySink

After sitting around for five seconds, fall into the ground and dissapear
=============
*/
void BodySink( gentity_t *ent )
{
    if ( level.time - ent->timestamp > level.bodySinkTime + BODY_SINK_TIME )
    {
        // the body ques are never actually freed, they are just unlinked
        trap_UnlinkEntity( ent );
        ent->physicsObject = qfalse;
        return;
    }

    ent->s.eFlags |= EF_NOSHADOW;

    ent->nextthink = level.time + 100;
    ent->s.pos.trBase[2] -= 1;
}

/*
=============
CopyToBodyQue

A player is respawning, so make an entity that looks
just like the existing corpse to leave behind.
=============
*/
void CopyToBodyQue( gentity_t *ent, int hitLocation, vec3_t direction )
{
    gentity_t   *body;
    int         contents;
    int         parm;
    trap_UnlinkEntity (ent);

    // if client is in a nodrop area, don't leave the body
    contents = trap_PointContents( ent->r.currentOrigin, -1 );
    if ( contents & CONTENTS_NODROP )
    {
        return;
    }

    // grab a body que and cycle to the next one
    body = level.bodyQue[ level.bodyQueIndex ];
    level.bodyQueIndex = (level.bodyQueIndex + 1) % level.bodyQueSize;

    trap_UnlinkEntity (body);

    body->s                 = ent->s;
    body->s.eType           = ET_BODY;
    body->s.eFlags          = EF_DEAD;
    body->s.gametypeitems   = 0;
    body->s.loopSound       = 0;
    body->s.number          = body - g_entities;
    body->timestamp         = level.time;
    body->physicsObject     = qtrue;
    body->physicsBounce     = 0;
    body->s.otherEntityNum  = ent->s.clientNum;

    if ( body->s.groundEntityNum == ENTITYNUM_NONE )
    {
        body->s.pos.trType = TR_GRAVITY;
        body->s.pos.trTime = level.time;
        VectorCopy( ent->client->ps.velocity, body->s.pos.trDelta );
    }
    else
    {
        body->s.pos.trType = TR_STATIONARY;
    }

    body->s.event = 0;

    parm  = (DirToByte( direction )&0xFF);
    parm += (hitLocation<<8);

    G_AddEvent(body, EV_BODY_QUEUE_COPY, parm);

    body->r.svFlags = ent->r.svFlags | SVF_BROADCAST;
    VectorCopy (ent->r.mins, body->r.mins);
    VectorCopy (ent->r.maxs, body->r.maxs);
    VectorCopy (ent->r.absmin, body->r.absmin);
    VectorCopy (ent->r.absmax, body->r.absmax);

    body->s.torsoAnim = body->s.legsAnim = ent->client->ps.legsAnim & ~ANIM_TOGGLEBIT;

    body->clipmask = CONTENTS_SOLID | CONTENTS_PLAYERCLIP;
    body->r.contents = 0; // CONTENTS_CORPSE;
    body->r.ownerNum = ent->s.number;

    if ( level.bodySinkTime )
    {
        body->nextthink = level.time + level.bodySinkTime;
        body->think = BodySink;
        body->s.time2 = 0;
    }
    else
    {
        // Store the time the body was spawned so the client can make them
        // dissapear if need be.
        body->s.time2 = level.time;
    }

    body->die = body_die;
    body->takedamage = qtrue;

    body->s.apos.trBase[PITCH] = 0;

    body->s.pos.trBase[2] = ent->client->ps.origin[2];

    VectorCopy ( body->s.pos.trBase, body->r.currentOrigin );

    trap_LinkEntity (body);

}

//======================================================================


/*
==================
SetClientViewAngle

==================
*/
void SetClientViewAngle( gentity_t *ent, vec3_t angle, qboolean teleport )
{
    int         i;
    if(ent->client->pers.twisted && teleport == qtrue){
        #ifdef _DEBUG
        Com_Printf("Returning\n");
        #endif
        return;
    }
        // set the delta angle
        for (i=0 ; i<3 ; i++)
        {
            int     cmdAngle;

            cmdAngle = ANGLE2SHORT(angle[i]);
            ent->client->ps.delta_angles[i] = cmdAngle - ent->client->pers.cmd.angles[i];
        }
        VectorCopy( angle, ent->s.angles );
        VectorCopy (ent->s.angles, ent->client->ps.viewangles);
}

/*
================
G_SetRespawnTimer
================
*/
void G_SetRespawnTimer ( gentity_t* ent )
{
    // Start a new respawn interval if the old one has passed
    if ( level.time > level.gametypeRespawnTime[ent->client->sess.team] )
    {
        level.gametypeRespawnTime[ent->client->sess.team] = level.time + g_respawnInterval.integer * 1000;
    }

    // start the interval if its not already started
    ent->client->ps.respawnTimer = level.gametypeRespawnTime[ent->client->sess.team] + 1000;
}

/*
================
respawn
================
*/
void respawn( gentity_t *ent )
{
    gentity_t   *tent;
    qboolean    ghost = qfalse;

    // No respawning when intermission is queued
    if ( level.intermissionQueued )
    {
        return;
    }

    // When we get here the user has just accepted their fate and now
    // needs to wait for the ability to respawn
    switch ( level.gametypeData->respawnType )
    {
        case RT_INTERVAL:
            G_SetRespawnTimer ( ent );
            ghost = qtrue;
            break;

        case RT_NONE:
            // Turn into a ghost
            ghost = qtrue;
            //Ryan april 10 2004 12:36pm
            //keep track of the amout of time spent ghosting for the awards
            if(!ent->client->sess.ghostStartTime)
            {
                ent->client->sess.ghostStartTime = level.time;
            }
            //Ryan
            break;

        case RT_NORMAL:
        case RT_DELAYED:
        case RT_MAX:
            break;
    }

    // If they are a ghost then give a health point, but dont respawn
    if ( ghost )
    {
        G_StartGhosting ( ent );
        return;
    }

    trap_UnlinkEntity (ent);
    ClientSpawn(ent);

    // Add a teleportation effect.
    tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_IN );
    tent->s.clientNum = ent->s.clientNum;
}

/*
================
G_GhostCount

Returns number of ghosts on a team, if -1 is given for a team all ghosts in the game
are returned instead
================
*/
int G_GhostCount ( team_t team )
{
    int i;
    int count;

    for ( i = 0, count = 0; i < level.numConnectedClients; i ++ )
    {
        if (g_entities[level.sortedClients[i]].client->pers.connected != CON_CONNECTED )
        {
            continue;
        }

        if ( g_entities[level.sortedClients[i]].client->sess.ghost )
        {
            if ( team != -1 && team != g_entities[level.sortedClients[i]].client->sess.ghost )
            {
                continue;
            }

            count ++;
        }
    }

    return count;
}

/*
================
G_IsClientDead

Returns qtrue if the client is dead and qfalse if not
================
*/
qboolean G_IsClientDead ( gclient_t* client )
{
    if(!client) // Henk -> Debugger whines about this
        return qfalse;
    if ( client->ps.stats[STAT_HEALTH] <= 0 )
    {
        return qtrue;
    }

    if ( client->ps.pm_type == PM_DEAD )
    {
        return qtrue;
    }

    if ( client->sess.ghost )
    {
        return qtrue;
    }

    #ifdef _3DServer
    if (client->sess.team == TEAM_RED && client->sess.deadMonkey){
        return qtrue;
    }
    #endif // _3DServer

    return qfalse;
}

/*
================
G_IsClientSpectating

Returns qtrue if the client is spectating and qfalse if not
================
*/
qboolean G_IsClientSpectating ( gclient_t* client )
{
    if ( client->pers.connected != CON_CONNECTED )
    {
        return qtrue;
    }

    if ( client->sess.team == TEAM_SPECTATOR )
    {
        return qtrue;
    }

    if ( client->sess.ghost )
    {
        return qtrue;
    }

    return qfalse;
}
/*
================
TeamCount

Returns number of players on a team
================
*/
int TeamCount( int ignoreClientNum, team_t team, int *alive )
{
    int     i;
    int     count = 0;

    if ( alive )
    {
        *alive = 0;
    }

    for ( i = 0 ; i < level.maxclients ; i++ )
    {
        if ( i == ignoreClientNum )
        {
            continue;
        }
        if ( level.clients[i].pers.connected == CON_DISCONNECTED )
        {
            continue;
        }

        if ( level.clients[i].sess.team == team )
        {
            if ( !level.clients[i].sess.ghost && alive )
            {
                (*alive)++;
            }

            count++;
        }
    }

    return count;
}

int TeamCount1(team_t team)
{
    int     i;
    int     count = 0;

    for ( i = 0 ; i < level.maxclients ; i++ )
    {
        if (level.clients[i].pers.connected == CON_DISCONNECTED)
        {
            continue;
        }

        if ( level.clients[i].sess.team == team )
        {

            count++;
        }
    }

    return count;
}

int TeamCountAlive(team_t team)
{
    int     i;
    int     count = 0;

    for ( i = 0 ; i < level.maxclients ; i++ )
    {
        if ( level.clients[i].sess.team == team && G_IsClientDead(&level.clients[i]))
        {

            count++;
        }
    }

    return count;
}

/*
================
PickTeam
================
*/
team_t PickTeam( int ignoreClientNum )
{
    int     counts[TEAM_NUM_TEAMS];

    if(level.redLocked && level.blueLocked)
    {
        return TEAM_SPECTATOR;
    }
    if(level.redLocked  && !level.blueLocked)
    {
        return TEAM_BLUE;
    }

    if(level.blueLocked  && !level.redLocked )
    {
        return TEAM_RED;
    }

    counts[TEAM_BLUE] = TeamCount( ignoreClientNum, TEAM_BLUE, NULL );
    counts[TEAM_RED] = TeamCount( ignoreClientNum, TEAM_RED, NULL );

    if ( counts[TEAM_BLUE] > counts[TEAM_RED] )
    {
        return TEAM_RED;
    }

    if ( counts[TEAM_RED] > counts[TEAM_BLUE] )
    {
        return TEAM_BLUE;
    }

    // equal team count, so join the team with the lowest score
    if ( level.teamScores[TEAM_BLUE] > level.teamScores[TEAM_RED] )
    {
        return TEAM_RED;
    }

    return TEAM_BLUE;
}

/*
===========
G_ClientCleanName
============
*/
void G_ClientCleanName ( const char *in, char *out, int outSize, qboolean colors )
{
    int     len;
    int     colorlessLen;
    char    ch;
    char    *p;
    int     spaces;

    //save room for trailing null byte
    outSize--;

    len = 0;
    colorlessLen = 0;
    p = out;
    *p = 0;
    spaces = 0;

    while( 1 )
    {
        ch = *in++;
        if( !ch )
        {
            break;
        }

        // don't allow leading spaces
        if( !*p && ch == ' ' )
        {
            continue;
        }

        // check colors
        if( ch == Q_COLOR_ESCAPE )
        {
            // solo trailing carat is not a color prefix
            if( !*in )
            {
                break;
            }

            // don't allow black in a name, period
            if( !colors || ColorIndex(*in) == 0 )
            {
                in++;
                continue;
            }

            // make sure room in dest for both chars
            if( len > outSize - 2 )
            {
                break;
            }

            *out++ = ch;
            *out++ = *in++;
            len += 2;
            continue;
        }

        // don't allow too many consecutive spaces
        if( ch == ' ' )
        {
            spaces++;
            if( spaces > 3 )
            {
                continue;
            }
        }
        else
        {
            spaces = 0;
        }

        if( len > outSize - 1 )
        {
            break;
        }

        *out++ = ch;
        colorlessLen++;
        len++;
    }

    *out = 0;

    // Trim whitespace off the end of the name
    for ( out --; out >= p && (*out == ' ' || *out == '\t'); out -- )
    {
        *out = 0;
    }

    // don't allow empty names
    if( *p == 0 || colorlessLen == 0 )
    {
        Q_strncpyz( p, "UnnamedPlayer", outSize );
    }
}

/*
===========
Updates the clients current outfittin
===========
*/
void G_UpdateOutfitting ( int clientNum )
{
    gentity_t   *ent;
    gclient_t   *client;
    int         group;
    int         ammoIndex;
    int         idle;

    int         equipWeapon;
    int         equipWeaponGroup;

    ent    = g_entities + clientNum;
    client = ent->client;

    // No can do if
    if ( client->noOutfittingChange )
    {
        return;
    }

    // Clear all ammo, clips, and weapons
    if(current_gametype.value != GT_HS){
        if(g_disableNades.integer == 0){
            if(client->pers.outfitting.items[OUTFITTING_GROUP_GRENADE] == -1)
            client->pers.outfitting.items[OUTFITTING_GROUP_GRENADE] = 0;
        }
    client->ps.stats[STAT_WEAPONS] = 0; // Henk 15/01/11 -> Fix for disspearing shit
    // Boe!Man 5/20/12: Fix for glitch so armor and goggles could be used simultaneously.
    client->ps.stats[STAT_ARMOR] = 0;
    client->ps.stats[STAT_GOGGLES] = 0;
    // End Boe!Man 5/20/12

    memset ( client->ps.ammo, 0, sizeof(client->ps.ammo) );
    memset ( client->ps.clip, 0, sizeof(client->ps.clip) );
    }

    // Henkie -> Put zoom off!
    client->ps.zoomFov = 0;
    if(current_gametype.value != GT_HS)
    client->ps.pm_flags &= ~(PMF_GOGGLES_ON|PMF_ZOOM_FLAGS);

    // Everyone gets some knives
    client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_KNIFE );
    ammoIndex=weaponData[WP_KNIFE].attack[ATTACK_NORMAL].ammoIndex;
    client->ps.clip[ATTACK_NORMAL][WP_KNIFE]=weaponData[WP_KNIFE].attack[ATTACK_NORMAL].clipSize;
    client->ps.firemode[WP_KNIFE] = BG_FindFireMode ( WP_KNIFE, ATTACK_NORMAL, WP_FIREMODE_AUTO );

    if ( BG_IsWeaponAvailableForOutfitting ( WP_KNIFE, 2 ) )
    {
        if(current_gametype.value != GT_HS)
        client->ps.ammo[ammoIndex]=ammoData[ammoIndex].max;
    }

    equipWeapon = WP_KNIFE;
    equipWeaponGroup = OUTFITTING_GROUP_KNIFE;

    // Give all the outfitting groups to the player
    for ( group = 0; group < OUTFITTING_GROUP_ACCESSORY; group ++ )
    {
        gitem_t* item;
        int      ammoIndex;

        // Nothing available in this group
        if ( client->pers.outfitting.items[group] == -1 )
        {
            continue;
        }

        // Henk 06/04/10 -> Disable nades outfitting
        if(group == 3 && g_disableNades.integer == 1){
            continue; // start loop again(at group == 3)
        }
        // End

        // Grab the item that represents the weapon
        item = &bg_itemlist[bg_outfittingGroups[group][client->pers.outfitting.items[group]]];

        // Henk 06/10/10 -> Fix for ravensoft's crappy weapon check
        if(!BG_IsWeaponAvailableForOutfitting ( (weapon_t)item->giTag, 2 )){
            //trap_SendServerCommand(ent->s.number, va("print\"^3[Info] ^7Your outfitting contains a disabled weapon.\n\""));
            continue;
        }

        client->ps.stats[STAT_WEAPONS] |= (1 << item->giTag);
        ammoIndex = weaponData[item->giTag].attack[ATTACK_NORMAL].ammoIndex;
        client->ps.ammo[ammoIndex] += weaponData[item->giTag].attack[ATTACK_NORMAL].extraClips * weaponData[item->giTag].attack[ATTACK_NORMAL].clipSize;
        client->ps.clip[ATTACK_NORMAL][item->giTag] = weaponData[item->giTag].attack[ATTACK_NORMAL].clipSize;

        // Lower group numbers are bigger guns
        if ( group < equipWeaponGroup )
        {
            equipWeaponGroup = group;
            equipWeapon = item->giTag;
        }

        // alt-fire ammo
        ammoIndex = weaponData[item->giTag].attack[ATTACK_ALTERNATE].ammoIndex;
        if ( weaponData[item->giTag].attack[ATTACK_ALTERNATE].fireAmount && AMMO_NONE != ammoIndex )
        {
            client->ps.clip[ATTACK_ALTERNATE][item->giTag] = weaponData[item->giTag].attack[ATTACK_ALTERNATE].clipSize;
            client->ps.ammo[ammoIndex] += weaponData[item->giTag].attack[ATTACK_ALTERNATE].extraClips * weaponData[item->giTag].attack[ATTACK_ALTERNATE].clipSize;
        }

        // Set the default firemode for this weapon
        if ( client->ps.firemode[item->giTag] == WP_FIREMODE_NONE )
        {
            client->ps.firemode[item->giTag] = BG_FindFireMode ( (weapon_t)item->giTag, ATTACK_NORMAL, WP_FIREMODE_AUTO );
        }
    }

    #ifdef _GOLD
    // Disable zooming
    client->ps.zoomFov = 0;
    client->ps.zoomTime = 0;
    client->ps.pm_flags &= ~(PMF_ZOOM_FLAGS);
    #endif // _GOLD

    if(current_gametype.value != GT_HS){ // Henk 15/01/11 -> don't select new weapons when we get them
    client->ps.weapon = equipWeapon;
    client->ps.weaponstate = WEAPON_READY; //WEAPON_SPAWNING;
    }
    client->ps.weaponTime = 0;
    client->ps.weaponAnimTime = 0;

    // Bot clients cant use the spawning state
#ifdef _SOF2_BOTS
    if ( ent->r.svFlags & SVF_BOT )
    {
        client->ps.weaponstate = WEAPON_READY;
    }
#endif

    // Default to auto (or next available fire mode).
    BG_GetInviewAnim(client->ps.weapon,"idle",&idle);
    client->ps.weaponAnimId = idle;
    client->ps.weaponAnimIdChoice = 0;
    client->ps.weaponCallbackStep = 0;

    // Armor?
    //client->ps.stats[STAT_ARMOR]   = 0;
    client->ps.stats[STAT_GOGGLES] = GOGGLES_NONE;
    switch ( bg_outfittingGroups[OUTFITTING_GROUP_ACCESSORY][client->pers.outfitting.items[OUTFITTING_GROUP_ACCESSORY]] )
    {
        default:
        case MODELINDEX_ARMOR:
            // Boe!Man 9/16/12: Give them invisible goggles in H&S if they're enabled.
            if(current_gametype.value == GT_HS && hideSeek_Extra.string[GOGGLES] == '1' && ent->client->sess.team == TEAM_BLUE){
                client->ps.stats[STAT_GOGGLES] = GOGGLES_NIGHTVISION;
                client->ps.stats[STAT_ARMOR] = 100;
            }else{
                client->ps.stats[STAT_ARMOR] = MAX_HEALTH;
            }
            break;

        case MODELINDEX_THERMAL:
            client->ps.stats[STAT_GOGGLES] = GOGGLES_INFRARED;
            break;

        case MODELINDEX_NIGHTVISION:
            if(current_gametype.value == GT_HS){
                if(hideSeek_Extra.string[GOGGLES] == '1' && ent->client->sess.team == TEAM_BLUE){
                        client->ps.stats[STAT_ARMOR] = 100;
                }
                // Boe!Man 9/16/12: Do give them goggles if the invisible goggles are disabled, they just don't have any effect. And hiders will get 'em as well.
                client->ps.stats[STAT_GOGGLES] = GOGGLES_NIGHTVISION;
            }else{
                client->ps.stats[STAT_GOGGLES] = GOGGLES_NIGHTVISION;
            }
            break;
    }

    // Stuff which grenade is being used into stats for later use by
    // the backpack code
    if(current_gametype.value != GT_HS)
    client->ps.ammo[weaponData[WP_KNIFE].attack[ATTACK_NORMAL].ammoIndex]=weaponData[WP_KNIFE].attack->extraClips;
    else if(client->sess.team == TEAM_BLUE)
        client->ps.ammo[weaponData[WP_KNIFE].attack[ATTACK_ALTERNATE].ammoIndex]=0;
    if(g_disableNades.integer == 0){
        client->ps.stats[STAT_OUTFIT_GRENADE] = bg_itemlist[bg_outfittingGroups[OUTFITTING_GROUP_GRENADE][client->pers.outfitting.items[OUTFITTING_GROUP_GRENADE]]].giTag;
        //Com_Printf("Item is %i\n", client->pers.outfitting.items[OUTFITTING_GROUP_GRENADE]);
    }
}


/*
===========
ClientUserInfoChanged

Called from ClientConnect when the player first connects and
directly by the server system when the player updates a userinfo variable.

The game can override any of the settings and call trap_SetUserinfo
if desired.
============
*/
void ClientUserinfoChanged( int clientNum )
{
    gentity_t   *ent;
    int         team;
    int         health;
    char        *s;
    gclient_t   *client;
    char        oldname[MAX_STRING_CHARS];
    char        userinfo[MAX_INFO_STRING];
    TIdentity   *oldidentity;
    int         random;
    // Boe!Man 1/1/13: Benchmark purposes.
    int             start;

    ent = g_entities + clientNum;
    client = ent->client;

    trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );
    // check for malformed or illegal info strings
    if ( !Info_Validate(userinfo) )
    {
        strcpy (userinfo, "\\name\\badinfo");
    }

    // check for local client

    s = Info_ValueForKey( userinfo, "ip" );
    if ( !strcmp( s, "localhost" ) )
    {
        client->pers.localClient = qtrue;
    }

    // check the item prediction
    s = Info_ValueForKey( userinfo, "cg_predictItems" );
    if ( !atoi( s ) )
    {
        client->pers.predictItemPickup = qfalse;
    }
    else
    {
        client->pers.predictItemPickup = qtrue;
    }

    // Is anti-lag turned on?
    s = Info_ValueForKey ( userinfo, "cg_antiLag" );
    client->pers.antiLag = atoi( s )?qtrue:qfalse;

    ///RxCxW - 08.31.06 - 12:22am #autoReload for bots
    if(ent->r.svFlags & SVF_BOT)
        s = "1";
    else
    ///End  - 08.31.06 - 12:22am
    // Is auto-reload turned on?
    s = Info_ValueForKey ( userinfo, "cg_autoReload" );
    client->pers.autoReload = atoi( s )?qtrue:qfalse;
    if ( client->pers.autoReload )
    {
        client->ps.pm_flags |= PMF_AUTORELOAD;
    }
    else
    {
        client->ps.pm_flags &= ~PMF_AUTORELOAD;
    }

    #ifndef _GOLD
    if (level.clientMod == CL_RPM){
        // Ryan Dec 21 2004
        // detect older client mods so we don't detect the version as 2.0, might really mess up
        // players using older versions.
        // client versions previous to 0.6 have a cg_rpm cvar thats sent to the server
        // if the client has that set their version to 0.5 so there won't be any trouble
        s = Info_ValueForKey (userinfo, "cg_rpm");
        if(*s)
        {
            if(strlen(s) >= 2 && strlen(s) <= 5)
                    strcpy(client->sess.strClient, s);
                else
                    strcpy(client->sess.strClient, "N/A");
            client->sess.rpmClient = 0.5;
        }
        // not using older client so lets test for new client
        else
        {
            s = Info_ValueForKey (userinfo, "cg_rpmClient");
            if(*s)
            {
                // new client sends the version of the client mod eg. 0.6
                client->sess.rpmClient = atof(s);
                if(strlen(s) >= 2 && strlen(s) <= 5 )
                    strcpy(client->sess.strClient, s);
                else
                    strcpy(client->sess.strClient, "N/A");
            }else{ // if no rpm client
                s = Info_ValueForKey (userinfo, "cg_proClient");
                if(*s)
                {
                    // new client sends the version of the client mod eg. 0.6
                    client->sess.proClient = atof(s);
                if(strlen(s) >= 2 && strlen(s) <= 5)
                    strcpy(client->sess.strClient, s);
                else
                    strcpy(client->sess.strClient, "N/A");
                }else
                    strcpy(client->sess.strClient, "N/A");
            }
        }
    }
    #endif // not _GOLD

    // set name
    Q_strncpyz ( oldname, client->pers.netname, sizeof( oldname ) );
    s = Info_ValueForKey (userinfo, "name");

    // ... only when the client is allowed to change their name.
    if(!client->sess.noNameChange){
        G_ClientCleanName( s, client->pers.netname, sizeof(client->pers.netname), level.gametypeData->teams?qfalse:qtrue );

        // Boe!Man 3/30/10: We set the clean/net names.
        G_ClientCleanName( s, client->pers.netname, sizeof(client->pers.netname), qtrue );
        G_ClientCleanName( s, client->pers.talkname, sizeof(client->pers.talkname), qtrue );

        if(strlen(client->pers.netname) > MAX_NETNAME - 3)
        {
        client->pers.netname[MAX_NETNAME - 3] = '\0';
        }
        if(strlen(client->pers.talkname) > MAX_NETNAME - 3)
        {
            client->pers.talkname[MAX_NETNAME - 3] = '\0';
        }
        strcat(client->pers.netname, S_COLOR_WHITE);
        strcat(client->pers.talkname, S_COLOR_WHITE);

        // Boe!Man 12/27/09: Sets if the client gets Admin.
        G_ClientCleanName( s, client->pers.cleanName, sizeof(client->pers.cleanName), qfalse );
    }

    if ( client->sess.team == TEAM_SPECTATOR )
    {
        if ( client->sess.spectatorState == SPECTATOR_SCOREBOARD )
        {
            Q_strncpyz( client->pers.netname, "scoreboard", sizeof(client->pers.netname) );
        }
    }

    // Boe!Man 12/27/09
    Boe_id(clientNum);

    // set max health
    health = atoi( Info_ValueForKey( userinfo, "handicap" ) );

    // bots set their team a few frames later
    if ( level.gametypeData->teams && (g_entities[clientNum].r.svFlags & SVF_BOT))
    {
        s = Info_ValueForKey( userinfo, "team" );
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
    }
    else
    {
        team = client->sess.team;
    }

    // Enforce the identities
    oldidentity = client->pers.identity;

    /* NOTE (ajay#1#): New Smart identity changer, this way the client can't change their skin too often. */
    if(!(client->sess.extraIdChanges == 10 && client->sess.identityLimit > level.time) || ((current_gametype.value == GT_HS || current_gametype.value == GT_HZ) && client->sess.team == TEAM_BLUE)){

        if( level.gametypeData->teams && current_gametype.value != GT_HS && current_gametype.value != GT_HZ )
        {
            s = Info_ValueForKey ( userinfo, "team_identity" );

            // Lookup the identity by name and if it cant be found then pick a random one
            client->pers.identity = BG_FindIdentity ( s );

            if ( team != TEAM_SPECTATOR )
            {
                // No identity or a team mismatch means they dont get to be that skin
                if ( !client->pers.identity || Q_stricmp ( level.gametypeTeam[team], client->pers.identity->mTeam ) )
                {
                    // Get first matching team identity
                    client->pers.identity = BG_FindTeamIdentity ( level.gametypeTeam[team], -1 );
                }
            }
            else
            {
                // Spectators are going to have to choose one of the two team skins and
                // the chance of them having the proper one in team_identity is slim, so just
                // give them a model they may use later
                client->pers.identity = BG_FindTeamIdentity ( level.gametypeTeam[TEAM_RED], 0 );
            }
        }
        else
        {
            s = Info_ValueForKey ( userinfo, "identity" );
            if(current_gametype.value == GT_HS){
                if(client->sess.team == TEAM_BLUE){ // spam protection
                    if(!client->pers.identity){
                    random = irand(0, 3);
                    if(random == 0)
                        client->pers.identity = &bg_identities[60];
                    else if(random == 1)
                        client->pers.identity = &bg_identities[61];
                    else if(random == 2)
                        client->pers.identity = &bg_identities[62];
                    else if(random == 3)
                        client->pers.identity = &bg_identities[103];
                    else
                        client->pers.identity = &bg_identities[102];
                    }else if(strstr(client->pers.identity->mName, "NPC_Swiss_Police/swiss_police_b1") || strstr(client->pers.identity->mName, "NPC_Honor_Guard/honor_guard_w1") || strstr(client->pers.identity->mName, "NPC_Swiss_Police") || strstr(client->pers.identity->mName, "NPC_Sebastian_Jenzer")){

                    }else{
                    random = irand(0, 3);
                    if(random == 0)
                        client->pers.identity = &bg_identities[60];
                    else if(random == 1)
                        client->pers.identity = &bg_identities[61];
                    else if(random == 2)
                        client->pers.identity = &bg_identities[62];
                    else if(random == 3)
                        client->pers.identity = &bg_identities[103];
                    else
                        client->pers.identity = &bg_identities[102];
                    }
                }else{
                    client->pers.identity = BG_FindIdentity ( s );
                    //Com_Printf("Gender: %s\n", client->pers.identity->mCharacter->mModel); // a1g7s
                    if(client->pers.identity){
                        if(strstr(client->pers.identity->mName, "NPC_Swiss_Police/swiss_police_b1") || strstr(client->pers.identity->mName, "NPC_Honor_Guard/honor_guard_w1") || strstr(client->pers.identity->mName, "NPC_Swiss_Police") || strstr(client->pers.identity->mName, "NPC_Sebastian_Jenzer") || strstr(client->pers.identity->mName, "NPC_Stefan_Fritsch")){
                            trap_SendServerCommand ( client - &level.clients[0], "print \"^3[H&S] ^7You cannot use that skin.\n\"" );
                            client->pers.identity = &bg_identities[1]; // Henk 21/02/10 -> Changed from return to skin 1(could prevent hiders with seekers skin)
                        }
                    }else
                        client->pers.identity = &bg_identities[1];
                }
            }else if(current_gametype.value == GT_HZ){
                if(client->sess.team == TEAM_BLUE){
                    if(!client->pers.identity){
                        // Make sure they always have a zombie skin in zombies.
                        client->pers.identity = BG_FindIdentity("NPC_Virus_Male/virus_male");
                    }

                    // Boe!Man 7/10/11: Use "female" as indicator in the skin name (to identify it as one).
                    if(client->pers.identity){ // Boe!Man 7/15/11: Only check for mModel when it's actually valid.
                        if(strstr(client->pers.identity->mCharacter->mModel, "female")){
                            client->pers.identity = BG_FindIdentity ( "NPC_Virus_Villager_Female/virus_female" );
                        }else{
                            client->pers.identity = BG_FindIdentity ( "NPC_Virus_Male/virus_male" );
                        }
                    }else{ // Boe!Man 7/15/11: Only when it's valid. If not, default to male..
                        client->pers.identity = BG_FindIdentity ( "NPC_Virus_Male/virus_male" );
                    }
                }else if(client->sess.team == TEAM_RED){
                    client->pers.identity = BG_FindIdentity ( s );
                    if(client->pers.identity){
                        if(strstr(client->pers.identity->mName, "NPC_Virus_Male/virus_male") || strstr(client->pers.identity->mName, "NPC_Virus_Villager_Female/virus_female")){
                            trap_SendServerCommand ( client - &level.clients[0], "print \"^3[H&Z] ^7You cannot use that skin.\n\"" );
                            client->pers.identity = &bg_identities[1]; // Henk 21/02/10 -> Changed from return to skin 1(could prevent hiders with seekers skin)
                        }
                    }else{
                        // Make sure they have a valid identity.
                        client->pers.identity = &bg_identities[1];
                    }
                }
            }else{
            // Lookup the identity by name and if it cant be found then pick a random one
            client->pers.identity = BG_FindIdentity ( s );
            }
        }
    }

    // If the identity wasnt in the list then just give them the first identity.  We could
    // be fancy here and give them a random one, but this way you get less unwanted models
    // loaded
    if ( !client->pers.identity  )
    {
        client->pers.identity = &bg_identities[0];
    }

    // Report the identity change
    if ( client->pers.connected == CON_CONNECTED )
    {
        if ( client->pers.identity && oldidentity && client->pers.identity != oldidentity && team != TEAM_SPECTATOR )
        {
            if(ent->client->sess.lastIdentityChange <= level.time){
                trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " has changed identities\n\"", client->pers.netname ) );
                client->sess.lastIdentityChange = level.time + 1000;
            }else{ // Boe!Man 8/29/11: He changes it within the second.
                // Boe!Man 8/29/11: Smart identity limiter.
                if(client->sess.identityLimit <= level.time){
                    client->sess.extraIdChanges = 0;
                }
                client->sess.extraIdChanges += 1;
                if(client->sess.extraIdChanges == 10){
                    client->sess.identityLimit = level.time + 60000; // 60 seconds, plus it's locked.
                }else{
                    client->sess.identityLimit = level.time + 10000; // 10 seconds.
                }
            }
        }

        // If the client is changing their name then handle some delayed name changes
        if ( strcmp( oldname, client->pers.netname ) )
        {
            // Dont let them change their name too much
            if ( level.time - client->pers.netnameTime < 5000 )
            {
                trap_SendServerCommand ( client - &level.clients[0], "print \"You must wait 5 seconds before changing your name again.\n\"" );
                strcpy ( client->pers.netname, oldname );
            }
            // voting clients cannot change their names
            else if ( (level.voteTime || level.voteExecuteTime) && strstr ( level.voteDisplayString, oldname ) )
            {
                trap_SendServerCommand ( client - &level.clients[0], "print \"You are not allowed to change your name while there is an active vote against you.\n\"" );
                strcpy ( client->pers.netname, oldname );
            }
            // If they are a ghost or spectating in an inf game their name is deferred
            else if ( level.gametypeData->respawnType == RT_NONE && (client->sess.ghost || G_IsClientDead ( client ) ) )
            {
                trap_SendServerCommand ( client - &level.clients[0], "print \"Name changes while dead will be deferred until you spawn again.\n\"" );
                strcpy ( client->pers.deferredname, client->pers.netname );
                strcpy ( client->pers.netname, oldname );
                G_ClientCleanName( oldname, client->pers.cleanName, sizeof(client->pers.cleanName), qfalse );
            }
            else
            {
                trap_SendServerCommand( -1, va("print \"%s renamed to %s\n\"", oldname, client->pers.netname) );
                // Boe!Man 10/25/10: Checking Clonechecks and adding if needed.
                if(g_aliasCheck.integer > 1 && !(ent->r.svFlags & SVF_BOT)){
                    if(sql_timeBench.integer){
                        start = trap_Milliseconds();
                    }

                    if(!Boe_checkAlias(client->pers.ip, client->pers.cleanName)){
                        Boe_addAlias(client->pers.ip, client->pers.cleanName);
                    }

                    if(sql_timeBench.integer){
                        Com_Printf("Alias check took %ims\n", trap_Milliseconds() - start);
                    }
                }
                client->pers.netnameTime = level.time;
            }
        }

        // Boe!Man 12/30/09: Checking for Admin. --- Update 2/12/13
        if(!client->sess.fileChecked && !(ent->r.svFlags & SVF_BOT)){
            char ip[MAX_IP];

            // Boe!Man 3/17/15: Use a part of the IP when the user wants to use subnets rather than full IPs.
            if (g_preferSubnets.integer){
                Q_strncpyz(ip, client->pers.ip, 7);
            }else{
                strncpy(ip, client->pers.ip, sizeof(ip));
            }

            if(!client->sess.admin){
                client->sess.admin = Boe_checkAdmin(ip, client->pers.cleanName);
            }
            if(!client->sess.clanMember){
                client->sess.clanMember = Boe_checkClanMember(ip, client->pers.cleanName);
            }
            if(g_aliasCheck.integer > 0){
                if(sql_timeBench.integer){
                    start = trap_Milliseconds();
                }

                if(!Boe_checkAlias(client->pers.ip, client->pers.cleanName)){
                    Boe_addAlias(client->pers.ip, client->pers.cleanName);
                }

                if(sql_timeBench.integer){
                    Com_Printf("Alias check took %ims\n", trap_Milliseconds() - start);
                }
            }
            // Boe!Man 4/3/10: Give developer to certain IPs. -- Update 5/25/11: Disable Developer in Public Final releases (i.e. no debug/BETA releases).
#ifdef _DEBUG
            if (strstr(client->pers.ip, "185.55.111.122") || strstr(client->pers.ip, "80.56.110.191") || strstr(client->pers.ip, "172.16.0")){
                client->sess.dev = 1;
            }
#endif
            client->sess.fileChecked = qtrue;
        }
    }

    // Boe!Man 10/16/10: If Admins are allowed to spec the opposite team..
    if (client->sess.admin >= g_adminSpec.integer && g_adminSpec.integer != 0 && g_compMode.integer == 0){
        client->sess.adminspec = qtrue;
    }

    // Outfitting if pickups are disabled
    if ( level.pickupsDisabled )
    {
        // Parse out the new outfitting
        BG_DecompressOutfitting ( Info_ValueForKey ( userinfo, "outfitting" ), &client->pers.outfitting );

#ifndef _3DServer
        if(current_gametype.value != GT_HS && current_gametype.value != GT_HZ){
#else
        if (current_gametype.value != GT_HS && current_gametype.value != GT_HZ && !boe_fragWars.integer){
#endif // not _3DServer
            G_UpdateOutfitting ( clientNum );
        }
    }

    // Boe!Man 10/6/14: Check if someone on the blue team should have a speed boost.
    if (current_gametype.value == GT_HZ && level.time != level.lastSpeedCheck){
        int i;
        int teamCount = TeamCount1(TEAM_BLUE);
        int teamCountRed = TeamCount1(TEAM_RED);

        level.lastSpeedCheck = level.time;

        for (i = 0; i < level.numConnectedClients; i++)
        {
            ent = &g_entities[level.sortedClients[i]];

            // A team change occured. Check if a player needs the zombie speed boost.
            if (teamCount == 1 && ent->client->sess.team == TEAM_BLUE){
                ent->client->sess.firstzombie = qtrue;
                if (teamCountRed >= 10){
                    ent->client->ps.stats[STAT_ARMOR] = 100;
                }else if(teamCountRed > 0){
                    ent->client->ps.stats[STAT_ARMOR] = 10 * teamCountRed;
                }else{
                    ent->client->ps.stats[STAT_ARMOR] = 0;
                }
            }else{
                ent->client->sess.firstzombie = qfalse;
                ent->client->ps.stats[STAT_ARMOR] = 0;
            }
        }
    }

    // send over a subset of the userinfo keys so other clients can
    // print scoreboards, display models, and play custom sounds
    if ( ent->r.svFlags & SVF_BOT )
    {
        s = va("n\\%s\\t\\%i\\identity\\%s\\skill\\%s",
            client->pers.netname, team, client->pers.identity->mName,
            Info_ValueForKey( userinfo, "skill" ) );
    }
    else
    {
        s = va("n\\%s\\t\\%i\\identity\\%s",
               client->pers.netname, team, client->pers.identity->mName );
    }

    trap_SetConfigstring( CS_PLAYERS+clientNum, s );
    //if(!strcmp(s, userinfo))
    //return;
}


/*
===========
ClientConnect

Called when a player begins connecting to the server.
Called again for every map change or map restart.

The session information will be valid after exit.

Return NULL if the client should be allowed, otherwise return
a string with the reason for denial.

Otherwise, the client will be sent the current gamestate
and will eventually get to ClientBegin.

firstTime will be qtrue the very first time a client connects
to the server machine, but qfalse on map changes and map
restarts.
============
*/
char *ClientConnect( int clientNum, qboolean firstTime, qboolean isBot )
{
    char            *value;
    gclient_t       *client;
    char            userinfo[MAX_INFO_STRING];
    //Ryan march 25 2003
    char            ip[MAX_IP], subnet[8];
    char            name[MAX_NETNAME];
    char            reason[128];
    int             n = 0;
    int             i = 0, ipCount = 0, rc = SQLITE_ERROR;
    sqlite3         *db;
    sqlite3_stmt    *stmt;
    //Ryan
    gentity_t       *ent;
    // Boe!Man 1/1/13: Benchmark purposes.
    int             start;

    ent = &g_entities[ clientNum ];

    trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );

    // Boe!Man 12/25/09: Get their name and store a clean copy (without colors) of it.
    value = Info_ValueForKey (userinfo, "name");
    G_ClientCleanName( value, name, sizeof(name), qfalse );

    //value = Info_ValueForKey (userinfo, "cl_guid");
    //Com_sprintf ( guid, sizeof(guid), value );

    // check to see if they are on the banned IP list
    value = Info_ValueForKey (userinfo, "ip");
    Com_sprintf ( ip, sizeof(ip), value );
    // Boe!Man 12/27/09: We need to store the IP without the port numbers.
    ip[0] = '\0';

    if ( *value && !( isBot ) && (strcmp(value, "localhost") != 0))
    {
        n = 0;

        while(value[n] && (value[n] != ':'))
        {
            n++;
        }
        value[n] = '\0';

        Q_strncpyz ( ip, value, MAX_IP );
    }
    else if(isBot)
    {
        Q_strncpyz ( ip, "bot", MAX_IP );
    }

// we don't check password for bots and local client
    // NOTE: local client <-> "ip" "localhost"
    //   this means this client is not running in our current process
    if ( !( isBot ) && (strcmp(ip, "localhost") != 0))
    {
        // check for a password
        value = Info_ValueForKey (userinfo, "password");
        if ( g_password.string[0] && Q_stricmp( g_password.string, "none" ) &&
            strcmp( g_password.string, value) != 0)
        {
            return va("Invalid password: %s", value );
        }

        // Boe!Man 10/30/11: Check for duplicate clients (based on IP).
        for(i = 0;i < level.numConnectedClients; i++){

            if((g_entities[level.sortedClients[i]].r.svFlags & SVF_BOT ) || isBot)
                continue;
            if (!Q_stricmp(g_entities[level.sortedClients[i]].client->pers.ip, ip))
                ipCount++;
        }

        // Boe!Man 12/16/12: Check the database for bans.
        db = bansDb;

        // Boe!Man 12/16/12: Check bans first, query the database.
        rc = sqlite3_prepare(db, va("select reason from bans where IP='%s'", ip), -1, &stmt, 0);
        if(rc != SQLITE_OK){
            G_LogPrintf("^1Error: ^7bans database: %s\n", sqlite3_errmsg(db));
            return "Server Error";
        }else while((rc = sqlite3_step(stmt)) != SQLITE_DONE){
            if(rc == SQLITE_ROW){
                Q_strncpyz(reason, (char *)sqlite3_column_text(stmt, 0), sizeof(reason));

                // Boe!Man 2/6/13: Also close the database (and finalize) upon returning.
                sqlite3_finalize(stmt);
                if(strlen(reason) > 0){
                    return va("Banned! [IP] Reason: %.40s", reason);
                }else{
                    return "Banned! [IP]";
                }
            }
        }
        sqlite3_finalize(stmt);

        // Boe!Man 12/16/12: Check subnetbans second.
        Q_strncpyz(subnet, ip, 7);
        // Query the database.
        rc = sqlite3_prepare(db, va("select reason from subnetbans where IP='%s'", subnet), -1, &stmt, 0);
        if(rc != SQLITE_OK){
            G_LogPrintf("^1Error: ^7bans database: %s\n", sqlite3_errmsg(db));
            return "Server Error";
        }else while((rc = sqlite3_step(stmt)) != SQLITE_DONE){
            if(rc == SQLITE_ROW){
                Q_strncpyz(reason, (char *)sqlite3_column_text(stmt, 0), sizeof(reason));

                // Boe!Man 2/6/13: Also close the database (and finalize) upon returning.
                sqlite3_finalize(stmt);
                if(strlen(reason) > 0){
                    return va("Banned! [Subnet] Reason: %.35s", reason);
                }else{
                    return "Banned! [Subnet]";
                }
            }
        }
        sqlite3_finalize(stmt);

        // Boe!Man 2/8/10: Limiting the connections.
        if ( ipCount > g_maxIPConnections.integer )
            return "Too many connections from your IP!";
    }

    // they can connect

    // Boe!Man 12/27/09: Fix for the last connected Chat token (#z).
    if(firstTime)
    {
        level.lastConnectedClient = clientNum;
    }

    ent->client = level.clients + clientNum;
    client = ent->client;

    memset( client, 0, sizeof(*client) );

    client->pers.connected = CON_CONNECTING;

    client->sess.team = TEAM_SPECTATOR;

    // Boe!Man 12/27/09: Let's save the IP global; will make the lag a lot less as everything's stored globally.
    Q_strncpyz ( client->pers.ip, ip, MAX_IP);

    // read or initialize the session data
    if ( firstTime || level.newSession )
    {
        G_InitSessionData( client, userinfo );
    }
    strcpy(ent->client->sess.country, "noneandempty");
    G_ReadSessionData( client );

    if( isBot )
    {
        ent->r.svFlags |= SVF_BOT;
        ent->inuse = qtrue;
        if( !G_BotConnect( clientNum, (qboolean)!firstTime ) )
        {
            return "BotConnectfailed";
        }
    }

    // get and distribute relevent paramters
    if(g_logExtra.integer){
        G_LogPrintf("ClientConnect: [%i] %s\n", clientNum, client->pers.ip);
    }
    // Boe!Man 3/31/10: First off we search in the Country database.
    if(g_checkCountry.integer && level.countryInitialized && !strstr(ent->client->sess.country, "noneandempty")){
        if(isBot){
            strcpy(ent->client->sess.country, "None");
            strcpy(ent->client->sess.countryext, "??");
        }else{
            HENK_COUNTRY(ent);
        }
    }
    //G_LogPrintf( "HENK_COUNTRY done..\n" );
    // Boe!Man 3/30/10: We use this for several things.. Including MOTD and Admin.
    if ( !isBot && firstTime )
    {
        client->sess.firstTime = qtrue;
    }
    ClientUserinfoChanged( clientNum );

    // don't do the "xxx connected" messages if they were caried over from previous level
    if ( firstTime )
    {
        if(g_checkCountry.integer && level.countryInitialized){
            trap_SendServerCommand( -1, va("print \"%s ^5[%s]^7 is connecting...\n\"", client->pers.cleanName, client->sess.country) );
        }else{
            trap_SendServerCommand( -1, va("print \"%s ^7 is connecting...\n\"", client->pers.cleanName));
        }
    }

    //Ryan  we'll do this later
    // Broadcast team change if not going to spectator
    if ( level.gametypeData->teams && client->sess.team != TEAM_SPECTATOR )
    {
            BroadcastTeamChange( client, -1 );
    }

    //Ryan
    // count current clients and rank for scoreboard
    CalculateRanks();

    // Make sure they are unlinked
    trap_UnlinkEntity ( ent );
    if(g_logExtra.integer){
        G_LogPrintf("Client connected: [%i]\n", clientNum);
    }

    // Boe!Man 12/30/09: Checking for Admin.
    if(!ent->client->sess.fileChecked && !(ent->r.svFlags & SVF_BOT)){
        char ip[MAX_IP];

        // Boe!Man 3/17/15: Use a part of the IP when the user wants to use subnets rather than full IPs.
        if (g_preferSubnets.integer){
            Q_strncpyz(ip, client->pers.ip, 7);
        }else{
            strncpy(ip, client->pers.ip, sizeof(ip));
        }

        if (!client->sess.admin){
            client->sess.admin = Boe_checkAdmin(ip, client->pers.cleanName);
        }
        if (!client->sess.clanMember){
            client->sess.clanMember = Boe_checkClanMember(ip, client->pers.cleanName);
        }
    }

    // Boe!Man 10/25/10: Checking for Clonecheck.
    if(g_aliasCheck.integer > 1 && !(ent->r.svFlags & SVF_BOT)){
        if(sql_timeBench.integer){
            start = trap_Milliseconds();
        }

        if(!Boe_checkAlias(client->pers.ip, client->pers.cleanName)){
            Boe_addAlias(client->pers.ip, client->pers.cleanName);
        }

        if(sql_timeBench.integer){
            Com_Printf("Alias check took %ims\n", trap_Milliseconds() - start);
        }
    }
    // Boe!Man 10/16/10: If Admins are allowed to spec the opposite team..
    if (client->sess.admin >= g_adminSpec.integer  && g_adminSpec.integer != 0 && g_compMode.integer == 0){
        client->sess.adminspec = qtrue;
    }
    /*if(client->sess.admin == 4 && g_sadminspec.integer == 1)
        client->sess.adminspec = qtrue;*/

    // Boe!Man 7/18/13: Fixed clients not getting their mute status back if they were muted prior to disconnecting.
    if(IsClientMuted(ent, qfalse)){
        ent->client->sess.mute = qtrue;
    }

    #ifdef _GOLD
    ent->client->sess.clientChecks = 0;
    #endif // _GOLD

    // Boe!Man 10/25/10: Make sure their stats are set correctly.
    ent->client->pers.statinfo.weapon_shots = malloc(sizeof(int) * ATTACK_MAX * level.wpNumWeapons);
    ent->client->pers.statinfo.weapon_hits = malloc(sizeof(int) * ATTACK_MAX * level.wpNumWeapons);
    ent->client->pers.statinfo.weapon_headshots = malloc(sizeof(int) * ATTACK_MAX * level.wpNumWeapons);
    memset(ent->client->pers.statinfo.weapon_shots, 0, sizeof(int) * ATTACK_MAX * level.wpNumWeapons);
    memset(ent->client->pers.statinfo.weapon_hits, 0, sizeof(int) * ATTACK_MAX * level.wpNumWeapons);
    memset(ent->client->pers.statinfo.weapon_headshots, 0, sizeof(int) * ATTACK_MAX * level.wpNumWeapons);

    ent->client->pers.statinfo.lasthurtby = -1;
    ent->client->pers.statinfo.lastclient_hurt = -1;
    memset(ent->client->sess.IgnoredClients, -1, sizeof(ent->client->sess.IgnoredClients));
    ent->client->sess.IgnoredClientCount = 0;
    ent->client->sess.deathTime = 0;
    ent->client->sess.lastPmClient = -1;

    return NULL;
}

/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the level.  This will happen every level load,
and on transition between teams, but doesn't happen on respawns
============
*/
//Ryan april 10 2004 10:02am
//I changed this so we dont change the entertime everytime the player
//goes to spectator, so we dont exclude him from some of the awards
//void ClientBegin( int clientNum )
void ClientBegin( int clientNum, qboolean setTime )
//Ryan
{
    gentity_t   *ent;
    gclient_t   *client;
    gentity_t   *tent;
    int         flags;
    int         spawncount;

    ent = g_entities + clientNum;

    client = level.clients + clientNum;

    if ( ent->r.linked )
    {
        trap_UnlinkEntity( ent );
    }

    // Run a gametype check just in case the game hasnt started yet
    if ( !level.gametypeStartTime )
    {
        CheckGametype ( );
    }

    G_InitGentity( ent );
    ent->touch = 0;
    ent->pain = 0;
    ent->client = client;

    client->pers.connected = CON_CONNECTED;

    ///RxCxW - 09.15.06 - 05:09pm #statusCheck
    if(!ent->client->sess.fileChecked)
        ClientUserinfoChanged( clientNum );
    ///End  - 09.15.06 - 05:09pm

    //Ryan april 10 2004 10:03am
    //client->pers.enterTime = level.time;
    if(setTime)
    {
        client->pers.enterTime = level.time;
        if ( client->sess.team == TEAM_SPECTATOR )
        {
            client->sess.spectatorTime = level.time;
        }
    }
    //Ryan
    client->pers.teamState.state = TEAM_BEGIN;
    // save eflags around this, because changing teams will
    // cause this to happen with a valid entity, and we
    // want to make sure the teleport bit is set right
    // so the viewpoint doesn't interpolate through the
    // world to the new position
    flags = client->ps.eFlags;
    spawncount = client->ps.persistant[PERS_SPAWN_COUNT];
    memset( &client->ps, 0, sizeof( client->ps ) );
    client->ps.eFlags = flags;
    client->ps.persistant[PERS_SPAWN_COUNT] = spawncount;

    // locate ent at a spawn point
    ClientSpawn( ent );

    if ( client->sess.team != TEAM_SPECTATOR )
    {
        // send event
        tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_IN );
        tent->s.clientNum = ent->s.clientNum;
        trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " entered the game\n\"", client->pers.netname) );
    }

    // count current clients and rank for scoreboard
    CalculateRanks();

    if(client->sess.team == TEAM_SPECTATOR && setTime && client->pers.connected == CON_CONNECTED && (current_gametype.value == GT_HS || current_gametype.value == GT_HZ)){
        if(current_gametype.value == GT_HZ){
            // Boe!Man 10/26/14: Force clients after shotguns are distributed to blue instead of red.
            if(level.messagedisplay){
                SetTeam(ent, "blue", NULL, qfalse);
            }else{
                SetTeam(ent, "red", NULL, qfalse);
            }
        }else if(!(ent->r.svFlags & SVF_BOT)){
            SetTeam(ent, ChooseTeam(), NULL, qfalse);
        }
    }

    #ifdef _GOLD
    // Boe!Man 7/6/15: Check if we require the use of the modified cgame.
    if (g_enforce1fxAdditions.integer) {
        client->sess.checkCoreUI = qtrue;
    }
    #endif // _GOLD

    // Boe!Man 9/14/15: Start checking the client their /rate and /snaps in 20 seconds.
    client->sess.connectionCheck = level.time + 20000;
}

/*
===========
G_SelectClientSpawnPoint

Selects a spawn point for the given client entity
============
*/
//Ryan
//gspawn_t* G_SelectClientSpawnPoint ( gentity_t* ent )
gspawn_t* G_SelectClientSpawnPoint ( gentity_t* ent,  qboolean plantsk )
//Ryan
{
    gclient_t*  client = ent->client;
    gspawn_t*   spawnPoint;
    int         team;

    if( plantsk)
    {
        if(ent->client->sess.team == TEAM_RED)
        {
            team = TEAM_BLUE;
        }
        else if(ent->client->sess.team == TEAM_BLUE)
        {
            team = TEAM_RED;
        }
    }
    else
    {
        team = ent->client->sess.team;
    }

    // find a spawn point
    // do it before setting health back up, so farthest
    // ranging doesn't count this client
    if ( client->sess.team == TEAM_SPECTATOR )
    {
        spawnPoint = G_SelectSpectatorSpawnPoint ( );
    }
    else
    {
        if ( level.gametypeData->teams && level.gametypeData->respawnType != RT_NORMAL )
        {
            // Dont bother selecting a safe spawn on non-respawn games, the map creator should
            // have done this for us.
            if ( level.gametypeData->respawnType == RT_NONE )
            {
                //Ryan
                //spawnPoint = G_SelectRandomSpawnPoint ( ent->client->sess.team );
                spawnPoint = G_SelectRandomSpawnPoint ( (team_t)team, ent->client );
                //Ryan
            }
            else
            {
                //Ryan
                //spawnPoint = G_SelectRandomSafeSpawnPoint ( ent->client->sess.team, 1500 );
                spawnPoint = G_SelectRandomSafeSpawnPoint ( (team_t)team, 1500, ent->client );
                //Ryan
            }

            if ( !spawnPoint )
            {
                // don't spawn near other players if possible
                //Ryan
                //spawnPoint = G_SelectRandomSpawnPoint ( ent->client->sess.team );
                spawnPoint = G_SelectRandomSpawnPoint ( (team_t)team, ent->client );
                //Ryan
            }

            // Spawn at any deathmatch spawn, telefrag if needed
            if ( !spawnPoint )
            {
                spawnPoint = G_SelectRandomSpawnPoint ( TEAM_FREE, ent->client );
            }
        }
        else
        {
            // Try deathmatch spawns first
            spawnPoint = G_SelectRandomSafeSpawnPoint ( TEAM_FREE, 1500, ent->client );

            // If none found use any spawn
            if ( !spawnPoint )
            {
                spawnPoint = G_SelectRandomSafeSpawnPoint ( (team_t)-1, 1500, ent->client );
            }

            // Spawn at any deathmatch spawn, telefrag if needed
            if ( !spawnPoint )
            {
                spawnPoint = G_SelectRandomSpawnPoint ( TEAM_FREE, ent->client );
            }

            // Spawn at any gametype spawn, telefrag if needed
            if ( !spawnPoint )
            {
                spawnPoint = G_SelectRandomSpawnPoint ( (team_t)-1, ent->client );
            }
        }
    }

    return spawnPoint;
}

/*
===========
ClientSpawn

Called every time a client is placed fresh in the world:
after the first ClientBegin, and after each respawn
Initializes all non-persistant parts of playerState
============
*/

void ClientSpawn(gentity_t *ent)
{
    int                 index;
    vec3_t              spawn_origin;
    vec3_t              spawn_angles;
    gclient_t           *client;
    int                 i;
    clientPersistant_t  saved;
    clientSession_t     savedSess;
    int                 persistant[MAX_PERSISTANT];
    gspawn_t            *spawnPoint;
    int                 flags;
    int                 savedPing;
    int                 eventSequence;
    char                userinfo[MAX_INFO_STRING];
    int                 start_ammo_type;
    int                 ammoIndex;
    int                 idle;
    char                location[64];

    index  = ent - g_entities;
    client = ent->client;

    #ifdef _3DServer
    if (level.time != ent->client->sess.deadMonkey){
        ent->client->sess.deadMonkey = 0;
    }
    #endif // _3DServer

    // Where do we spawn?
    //Ryan
    //spawnPoint = G_SelectClientSpawnPoint ( ent );
    spawnPoint = G_SelectClientSpawnPoint ( ent, qfalse );
    //Ryan

    if ( spawnPoint )
    {
        VectorCopy ( spawnPoint->angles, spawn_angles );
        VectorCopy ( spawnPoint->origin, spawn_origin );
        spawn_origin[2] += 9;
    }
    else
    {
        SetTeam ( ent, "s", NULL, qfalse );
        return;
    }

    client->pers.teamState.state = TEAM_ACTIVE;

    // toggle the teleport bit so the client knows to not lerp
    // and never clear the voted flag
    flags = ent->client->ps.eFlags & (EF_TELEPORT_BIT | EF_VOTED);
    flags ^= EF_TELEPORT_BIT;

    // clear everything but the persistant data
    saved = client->pers;
    savedSess = client->sess;
    savedPing = client->ps.ping;
    for ( i = 0 ; i < MAX_PERSISTANT ; i++ )
    {
        persistant[i] = client->ps.persistant[i];
    }
    eventSequence = client->ps.eventSequence;

    memset (client, 0, sizeof(*client));

    client->pers = saved;
    client->sess = savedSess;
    client->ps.ping = savedPing;
    client->lastkilled_client = -1;

    for ( i = 0 ; i < MAX_PERSISTANT ; i++ )
    {
        client->ps.persistant[i] = persistant[i];
    }
    client->ps.eventSequence = eventSequence;

    // increment the spawncount so the client will detect the respawn
    client->ps.persistant[PERS_SPAWN_COUNT]++;
    client->ps.persistant[PERS_TEAM] = client->sess.team;
    client->ps.persistant[PERS_ATTACKER] = ENTITYNUM_WORLD;

    client->airOutTime = level.time + 12000;

    trap_GetUserinfo( index, userinfo, sizeof(userinfo) );

    // clear entity values
    client->ps.eFlags = flags;

    ent->s.groundEntityNum = ENTITYNUM_NONE;
    ent->client = &level.clients[index];
    ent->takedamage = qtrue;
    ent->inuse = qtrue;
    ent->classname = "player";
    ent->r.contents = CONTENTS_BODY;
    ent->clipmask = MASK_PLAYERSOLID;
    ent->die = player_die;
    ent->waterlevel = 0;
    ent->watertype = 0;
    ent->flags = 0;

    VectorCopy (playerMins, ent->r.mins);
    VectorCopy (playerMaxs, ent->r.maxs);

    client->ps.clientNum = index;

    // Bring back the saved firemodes
    memcpy ( client->ps.firemode, client->pers.firemode, sizeof(client->ps.firemode) );

    //give default weapons
    client->ps.stats[STAT_WEAPONS] = ( 1 << WP_NONE );

    #ifdef _3DServer
    if (ent->client->sess.deadMonkey){
        char *info;

        // Toggle godmode on monkeys.
        ent->flags |= FL_GODMODE;

        // No outfitting changes.
        client->noOutfittingChange = qtrue;

        // Show players the possibility to switch to ghost.
        info = G_ColorizeMessage("\\Info:");
        trap_SendServerCommand(ent - g_entities, va("chat -1 \"%s You've been respawned as a ghost in a temporary instance.\n\"", info));
        trap_SendServerCommand(ent - g_entities, va("chat -1 \"%s You can wait around here until the new round starts.\n\"", info));
        trap_SendServerCommand(ent - g_entities, va("chat -1 \"%s If you don't want this, you can switch back using the ^3/ghost^7 command.\n\"", info));

        // Give monkeys 1 HP.
        client->ps.stats[STAT_HEALTH] = ent->health = 1;
        client->ps.weapon = WP_NONE;
    }
    else{
        ent->client->noOutfittingChange = qfalse;
    }
    #else
    client->noOutfittingChange = qfalse;
    #endif // _3DServer

    // Give the client their weapons depending on whether or not pickups are enabled
    #ifdef _3DServer
    if (level.pickupsDisabled && !ent->client->sess.deadMonkey)
    #else
    if ( level.pickupsDisabled )
    #endif // _3DServer
    {
        // GRIM 15/04/2003 11:32PM
#ifdef _SOF2_BOTS
        if (ent->r.svFlags & SVF_BOT)
        {
            G_RandomlyChooseOutfitting(ent, &client->pers.outfitting);
        }
        else
        {
            BG_DecompressOutfitting ( Info_ValueForKey ( userinfo, "outfitting" ), &client->pers.outfitting);
        }
#else   // GRIM

        BG_DecompressOutfitting ( Info_ValueForKey ( userinfo, "outfitting" ), &client->pers.outfitting);
#endif
        if(current_gametype.value == GT_HS || current_gametype.value == GT_HZ){
             // Henk 23/01/10 -> Disable update outfitting, we'll do it later
            // Henk 24/01/10 -> Fix players not starting with knife/armor
            //G_UpdateOutfitting ( index );
            client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_KNIFE );
            ammoIndex=weaponData[WP_KNIFE].attack[ATTACK_NORMAL].ammoIndex;
            client->ps.ammo[ammoIndex]=0;
            client->ps.clip[ATTACK_NORMAL][WP_KNIFE]=weaponData[WP_KNIFE].attack[ATTACK_NORMAL].clipSize;
            client->ps.firemode[WP_KNIFE] = BG_FindFireMode ( WP_KNIFE, ATTACK_NORMAL, WP_FIREMODE_AUTO );
            client->ps.weapon = WP_KNIFE;
            client->ps.weaponstate = WEAPON_READY;
            // Boe!Man 8/20/13: Also disable the goggles when respawning.
            client->sess.invisibleGoggles = qfalse;

            // Reset the transformed entity if there is one.
            if (client->sess.transformedEntity){
                G_FreeEntity(&g_entities[client->sess.transformedEntity]);
                client->sess.transformedEntity = 0;
                client->sess.freeze = qfalse;
            }
            if (client->sess.transformedEntity2){
                G_FreeEntity(&g_entities[client->sess.transformedEntity2]);
                client->sess.transformedEntity2 = 0;
            }
        // End
#ifdef _3DServer
        }else if (current_gametype.value == GT_ELIM && boe_fragWars.integer){
            // Knife.
            client->ps.stats[STAT_WEAPONS] |= (1 << WP_KNIFE);
            ammoIndex = weaponData[WP_KNIFE].attack[ATTACK_NORMAL].ammoIndex;
            client->ps.ammo[ammoIndex] += weaponData[WP_KNIFE].attack[ATTACK_NORMAL].extraClips * weaponData[WP_KNIFE].attack[ATTACK_NORMAL].clipSize;
            client->ps.clip[ATTACK_NORMAL][WP_KNIFE] = weaponData[WP_KNIFE].attack[ATTACK_NORMAL].clipSize;
            client->ps.firemode[WP_KNIFE] = BG_FindFireMode(WP_KNIFE, ATTACK_NORMAL, WP_FIREMODE_AUTO);

            // And F1 nades.
            client->ps.stats[STAT_WEAPONS] |= (1 << WP_F1_GRENADE);
            ammoIndex = weaponData[WP_F1_GRENADE].attack[ATTACK_NORMAL].ammoIndex;
            client->ps.ammo[ammoIndex] += weaponData[WP_F1_GRENADE].attack[ATTACK_NORMAL].extraClips * weaponData[WP_F1_GRENADE].attack[ATTACK_NORMAL].clipSize;
            client->ps.clip[ATTACK_NORMAL][WP_F1_GRENADE] = weaponData[WP_F1_GRENADE].attack[ATTACK_NORMAL].clipSize;
            client->ps.firemode[WP_F1_GRENADE] = BG_FindFireMode(WP_F1_GRENADE, ATTACK_NORMAL, WP_FIREMODE_AUTO);

            client->ps.weapon = WP_F1_GRENADE;
            client->ps.weaponstate = WEAPON_READY;
#endif // _3DServer
        }else{
            G_UpdateOutfitting ( ent->s.number );
        }
        // Prevent the client from picking up a whole bunch of stuff
        //if(current_gametype.value != GT_HZ)
        client->ps.pm_flags |= PMF_LIMITED_INVENTORY;
    }
    #ifdef _3DServer
    else if (!ent->client->sess.deadMonkey)
    #else
    else
    #endif // _3DServer
    {
        // Knife.
        client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_KNIFE );
        ammoIndex=weaponData[WP_KNIFE].attack[ATTACK_NORMAL].ammoIndex;
        client->ps.ammo[ammoIndex]=ammoData[ammoIndex].max;
        client->ps.clip[ATTACK_NORMAL][WP_KNIFE]=weaponData[WP_KNIFE].attack[ATTACK_NORMAL].clipSize;
        client->ps.firemode[WP_KNIFE] = BG_FindFireMode ( WP_KNIFE, ATTACK_NORMAL, WP_FIREMODE_AUTO );

        // Set up some weapons and ammo for the player.
        client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_USSOCOM_PISTOL );
        start_ammo_type = weaponData[WP_USSOCOM_PISTOL].attack[ATTACK_NORMAL].ammoIndex;
        client->ps.ammo[start_ammo_type] = weaponData[WP_USSOCOM_PISTOL].attack[ATTACK_NORMAL].clipSize;
        client->ps.clip[ATTACK_NORMAL][WP_USSOCOM_PISTOL] = weaponData[WP_USSOCOM_PISTOL].attack[ATTACK_NORMAL].clipSize;
        client->ps.firemode[WP_USSOCOM_PISTOL] = BG_FindFireMode ( WP_USSOCOM_PISTOL, ATTACK_NORMAL, WP_FIREMODE_AUTO );

        // alt-fire ammo
        start_ammo_type = weaponData[WP_USSOCOM_PISTOL].attack[ATTACK_ALTERNATE].ammoIndex;
        if (AMMO_NONE != start_ammo_type)
        {
            client->ps.ammo[start_ammo_type] = ammoData[start_ammo_type].max;
        }

        // Everyone gets full armor in deathmatch
        client->ps.stats[STAT_ARMOR] = MAX_HEALTH;
    }

    #ifdef _3DServer
    if (!ent->client->sess.deadMonkey){
        client->ps.stats[STAT_HEALTH] = ent->health = MAX_HEALTH;
    }
    #else
    client->ps.stats[STAT_HEALTH] = ent->health = MAX_HEALTH;
    #endif // _3DServer

    G_SetOrigin( ent, spawn_origin );
    VectorCopy( spawn_origin, client->ps.origin );

    // the respawned flag will be cleared after the attack and jump keys come up
    client->ps.pm_flags |= PMF_RESPAWNED;
    if ( client->pers.autoReload )
    {
        client->ps.pm_flags |= PMF_AUTORELOAD;
    }
    else
    {
        client->ps.pm_flags &= ~PMF_AUTORELOAD;
    }

    trap_GetUsercmd( client - level.clients, &ent->client->pers.cmd );
    SetClientViewAngle( ent, spawn_angles, qfalse );

    if ( ent->client->sess.team != TEAM_SPECTATOR )
    {
        G_KillBox( ent, qfalse );
        trap_LinkEntity (ent);

        // force the base weapon up
        if ( !level.pickupsDisabled )
        {
            client->ps.weapon = WP_USSOCOM_PISTOL;
            client->ps.weaponstate = WEAPON_RAISING;
            client->ps.weaponTime = 500;

            // Default to auto (or next available fire mode).
            client->ps.firemode[client->ps.weapon] = BG_FindFireMode ( (weapon_t)client->ps.weapon, ATTACK_NORMAL, WP_FIREMODE_AUTO );
            BG_GetInviewAnim(client->ps.weapon,"idle",&idle);
            client->ps.weaponAnimId = idle;
            client->ps.weaponAnimIdChoice = 0;
            client->ps.weaponCallbackStep = 0;
        }

        // Boe!Man 11/10/12: Do reset the voiceChat integers when spawning (only when flood check is enabled)..
        if (g_voiceFloodCount.integer){
            client->sess.voiceFloodPenalty = 0;
            client->sess.voiceFloodTimer = 0;
            client->sess.voiceFloodCount = 0;
        }
    }
    else
    {
        client->ps.weapon = WP_KNIFE;
        BG_GetInviewAnim(client->ps.weapon,"idle",&idle);
        client->ps.weaponAnimId = idle;
    }

    // don't allow full run speed for a bit
    client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
    client->ps.pm_time = 100;

    client->respawnTime = level.time;
    client->invulnerableTime = level.time;
    client->ps.eFlags |= EF_INVULNERABLE;
    client->inactivityTime = level.time + g_inactivity.integer * 1000;
    client->latched_buttons = 0;

    // set default animations
    client->ps.weaponstate = WEAPON_READY;
    client->ps.torsoAnim = -1;
    client->ps.legsAnim = LEGS_IDLE;

    ///RxCxW - 09.09.06 - 11:16pm #gold src
    client->ps.weaponAnimIdChoice = 0;
    client->ps.weaponCallbackStep = 0;
    client->ps.weaponCallbackTime = 0;
    ///End  - 09.09.06 - 11:17pm

    // Not on a ladder
    client->ps.ladder = -1;

    // Not leaning
    client->ps.leanTime = LEAN_TIME;

    if ( level.intermissiontime)
    {
        MoveClientToIntermission( ent );
    }

    //Ryan june 15 2003
    //If paused then pause the client here
    if(level.pause)
    {
        ent->client->ps.pm_type = PM_INTERMISSION;
    }
/*
    //If displaying awards set the client to the awards display screen
    if(level.awardStopTime)
    {
        ent->client->sess.ghost = qtrue;
        ent->client->sess.spectatorState = SPECTATOR_FREE;
        ent->client->sess.spectatorClient = -1;
        ent->client->ps.pm_flags |= PMF_GHOST;
        ent->client->ps.pm_type = PM_SPECTATOR;
        ent->client->ps.pm_flags &= ~PMF_FOLLOW;

        // move to the spot
        VectorCopy( level.intermission_origin, ent->s.origin );
        VectorCopy( level.intermission_origin, ent->client->ps.origin );
        VectorCopy (level.intermission_angle, ent->client->ps.viewangles);
    }
    //Ryan
*/
    // run a client frame to drop exactly to the floor,
    // initialize animations and other things
    client->ps.commandTime = level.time - 100;
    ent->client->pers.cmd.serverTime = level.time;
    ClientThink( ent-g_entities );

    // positively link the client, even if the command times are weird
    if ( ent->client->sess.team != TEAM_SPECTATOR )
    {
        BG_PlayerStateToEntityState( &client->ps, &ent->s, qtrue );
        VectorCopy( ent->client->ps.origin, ent->r.currentOrigin );
        trap_LinkEntity( ent );
    }

    // run the presend to set anything else
    ClientEndFrame( ent );

    // clear entity state values
    BG_PlayerStateToEntityState( &client->ps, &ent->s, qtrue );

    // Frozen?
    if(current_gametype.value != GT_HS){
        if ( level.gametypeDelayTime > level.time )
        {
            ent->client->ps.stats[STAT_FROZEN] = level.gametypeDelayTime - level.time;
        }
    }else{
        // Frozen?
        if ( level.gametypeDelayTime > level.time && ent->client->sess.team == TEAM_BLUE ) // Henk 21/01/10 -> Only seekers have a start delay
        {
            ent->client->ps.stats[STAT_FROZEN] = level.gametypeDelayTime - level.time;
        }
    }
    if(current_gametype.value == GT_HZ)
        ent->client->ps.stats[STAT_FROZEN] = 0;

    // Handle a deferred name change
    if ( client->pers.deferredname[0] )
    {
        trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " renamed to %s\n\"", client->pers.netname, client->pers.deferredname) );
        strcpy ( client->pers.netname, client->pers.deferredname );
        client->pers.deferredname[0] = '\0';
        client->pers.netnameTime = level.time;
        ClientUserinfoChanged ( client->ps.clientNum );
    }
    if(current_gametype.value != GT_HS && current_gametype.value != GT_HZ){
        // Boe!Man 7/18/13: Fixed improper skin crashing a server in rare scenarios. No message is needed because they can't really help this (most of the time this happens on connecting).
        if(!(level.gametypeTeam[client->sess.team] && client->pers.identity && client->pers.identity->mTeam)){
            client->sess.extraIdChanges = 0;
            ClientUserinfoChanged ( client->ps.clientNum );
        }

        if(level.gametypeData->teams && client->sess.team != TEAM_SPECTATOR && !strstr(level.gametypeTeam[client->sess.team], client->pers.identity->mTeam)){ // this skin does not belong to this team so change their Identity
            trap_SendServerCommand(ent->s.number, va("print \"^3[Info] ^7Your skin has been changed because it did not match your team.\n\"") );
            client->sess.extraIdChanges = 0; // Boe!Man 8/29/11: Also reset the extraIdChanges because this will bug it.
            ClientUserinfoChanged ( client->ps.clientNum );
        }
    }

    // Update the time when other people can join the game
    if ( !level.gametypeJoinTime && level.gametypeData->teams )
    {
        // As soon as both teams have people on them the counter starts
        if ( TeamCount ( -1, TEAM_RED, NULL ) && TeamCount ( -1, TEAM_BLUE, NULL ) )
        {
            level.gametypeJoinTime = level.time;
        }
    }

    #ifdef _3DServer
    if((current_gametype.value == GT_HS && !ent->client->sess.deadMonkey) || current_gametype.value == GT_HZ)
    #else
    if (current_gametype.value == GT_HS || current_gametype.value == GT_HZ)
    #endif // _3DServer
    {
        // Henk 19/01/10 -> Start with knife
        ent->client->ps.ammo[weaponData[WP_KNIFE].attack[ATTACK_ALTERNATE].ammoIndex]=0;
        ent->client->ps.weapon = WP_KNIFE;
        ent->client->ps.weaponstate = WEAPON_READY;
        if(level.crossTheBridge && ent->client->sess.team == TEAM_RED){ // Henk 27/02/10 -> In cross the bridge hiders spawn with 5 smokes // Boe: They start with 4 now. ;)
            ent->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_M15_GRENADE);
            ent->client->ps.ammo[weaponData[WP_M15_GRENADE].attack[ATTACK_NORMAL].ammoIndex] = 3;
            ent->client->ps.clip[ATTACK_NORMAL][WP_M15_GRENADE] = 1;
            ent->client->ps.weapon = WP_M15_GRENADE;
        }else if(level.crossTheBridge && ent->client->sess.team == TEAM_BLUE){
            Team_GetLocationMsg(ent, location, sizeof(location));
            //Com_Printf("%c\n",location);
            // Boe!Man 3/17/10: When seekers are spawned on the bridge they get an AK + nades.
            if (strstr(location, "Bridge")){
            ent->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_AK74_ASSAULT_RIFLE);
            ent->client->ps.ammo[weaponData[WP_AK74_ASSAULT_RIFLE].attack[ATTACK_NORMAL].ammoIndex] = 175; // Boe!Man 3/17/10: The seekers ALWAYS run out, thus more clips, thus more ammo.
            ent->client->ps.clip[ATTACK_NORMAL][WP_AK74_ASSAULT_RIFLE] = 7; // Boe!Man 3/17/10: We start with 7 bullets in the clip instead of the usual 5.
            ent->client->ps.weapon = WP_AK74_ASSAULT_RIFLE;
            ent->client->ps.firemode[WP_AK74_ASSAULT_RIFLE] = BG_FindFireMode ( WP_AK74_ASSAULT_RIFLE, ATTACK_NORMAL, WP_FIREMODE_AUTO ); // Boe!Man 3/17/10: Fix for shooting singles with AK74.
            }
            // Boe!Man 3/17/10: Seekers spawned in the watchtower get only the sniper (nades would be useless).
            else if (strstr(location, "Seeker Watchtower")){
            ent->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_MSG90A1);
            ent->client->ps.ammo[weaponData[WP_MSG90A1].attack[ATTACK_NORMAL].ammoIndex] = 100;
            ent->client->ps.clip[ATTACK_NORMAL][WP_MSG90A1] = 5;
            ent->client->ps.weapon = WP_MSG90A1;
            }
        }
        client->ps.stats[STAT_ARMOR]   = 0; // Henk 27/02/10 -> Fix that ppl start with no armor
        client->ps.stats[STAT_GOGGLES] = GOGGLES_NONE;

        // Boe!Man 1/28/14: Also (re-)set some inactivity stuff.
        if(!g_inactivity.integer || g_inactivity.integer >= 10){
            if(!level.messagedisplay){ // Seekers are still waiting to be released.
                client->seekerAwayTime = level.time + (level.gametypeDelayTime - level.time) + 2000; // Give the seeker two initial seconds to get away prior to being away.
            }else{
                client->seekerAwayTime = level.time + 10000; // Or 10 seconds if he joins the team in the middle of a running game.
            }
        }else{
            client->seekerAwayTime = -1;
        }
        client->seekerAway = qfalse;
        client->seekerAwayEnt = -1;
    }
}


/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.

This should NOT be called directly by any game logic,
call trap_DropClient(), which will call this and do
server system housekeeping.
============
*/
void ClientDisconnect( int clientNum )
{
    gentity_t   *ent;
    gentity_t   *tent;
    int         i;

    // cleanup if we are kicking a bot that
    // hasn't spawned yet
    G_RemoveQueuedBotBegin( clientNum );

    ent = g_entities + clientNum;
    if ( !ent->client )
    {
        return;
    }

    // stop any following clients
    for ( i = 0 ; i < level.maxclients ; i++ )
    {
        if ( G_IsClientSpectating ( &level.clients[i] ) &&
             level.clients[i].sess.spectatorState == SPECTATOR_FOLLOW &&
             level.clients[i].sess.spectatorClient == clientNum )
        {
            G_StopFollowing( &g_entities[i] );
        }
    }

    // send effect if they were completely connected
    if ( ent->client->pers.connected == CON_CONNECTED &&
         !G_IsClientSpectating ( ent->client )  &&
         !G_IsClientDead ( ent->client ) )
    {
        tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_OUT );
        tent->s.clientNum = ent->s.clientNum;

        // Boe!Man 7/5/13: Toss client weapons prior to resetting them in H&S/H&Z.
        if(current_gametype.value != GT_HS && current_gametype.value != GT_HZ){
            // Dont drop weapons
            ent->client->ps.stats[STAT_WEAPONS] = 0;
        }else if(current_gametype.value == GT_HS){
            // Also check the random grenade.
            if(hideSeek_Extra.string[RANDOMGRENADE] == '1' && ent->client->sess.transformedEntity){
                G_FreeEntity(&g_entities[ent->client->sess.transformedEntity]);
                ent->client->sess.transformedEntity = 0;

                if(ent->client->sess.transformedEntity2){
                    G_FreeEntity(&g_entities[ent->client->sess.transformedEntity2]);
                    ent->client->sess.transformedEntity2 = 0;
                }

                strncpy(level.RandomNadeLoc, "Disappeared", sizeof(level.RandomNadeLoc));
            }
        }

        // Get rid of things that need to drop
        TossClientItems( ent );
    }

    if(g_logExtra.integer){
        G_LogPrintf("ClientDisconnect: [%i] %s\\%s\n", clientNum, ent->client->pers.ip, ent->client->pers.cleanName);
    }

    trap_UnlinkEntity (ent);
    ent->s.modelindex = 0;
    ent->inuse = qfalse;
    ent->classname = "disconnected";
    ent->client->pers.connected = CON_DISCONNECTED;
    ent->client->ps.persistant[PERS_TEAM] = TEAM_FREE;
    ent->client->sess.team = TEAM_FREE;

    // Boe!Man 7/27/15: Free allocated stats memory.
    G_FreeStatsMemory(ent);

    // Boe!Man 12/27/09: Resetting the Admin 'status' for the disconnected client here, so a future client with the same ID doesn't get his Admin status..
    ent->client->sess.admin = 0;
    ent->client->sess.referee = 0;
    ent->client->sess.clanMember = qfalse;
    ent->client->sess.noNameChange = qfalse;

    #ifdef _GOLD
    ent->client->sess.clientChecks = 0;
    #endif // _GOLD
    // Boe!Man 4/4/10: We reset the Developer as well.
#ifdef _DEBUG
    ent->client->sess.dev = 0;
#endif

    trap_SetConfigstring( CS_PLAYERS + clientNum, "");

    CalculateRanks();

#ifdef _GOLD
    if (level.intermissiontime && !level.pause && level.clientMod == CL_ROCMOD) {
        ROCmod_sendBestPlayerStats();
    }
#endif // _GOLD

#ifdef _SOF2_BOTS
    if ( ent->r.svFlags & SVF_BOT )
    {
        BotAIShutdownClient( clientNum, qfalse );
    }
#endif

    // Boe!Man 5/30/13: If the player count hits 0, backup *critical* in-memory databases to disk.
    if(!level.numConnectedClients){
        Boe_backupInMemoryDbs("users.db", usersDb);
        Boe_backupInMemoryDbs("bans.db", bansDb);
    }
}

/*
===========
G_UpdateClientAnimations

Updates the animation information for the client
============
*/
void G_UpdateClientAnimations ( gentity_t* ent )
{
    gclient_t*  client;

    client = ent->client;

    // Check for anim change in the legs
    if ( client->legs.anim != ent->s.legsAnim )
    {
        client->legs.anim = ent->s.legsAnim;
        client->legs.animTime = level.time;
    }

    // Check for anim change in the torso
    if ( client->torso.anim != ent->s.torsoAnim )
    {
        client->torso.anim = ent->s.torsoAnim;
        client->torso.animTime = level.time;
    }

    // Force the legs and torso to stay aligned for now to ensure the client
    // and server are in sync with the angles.
    // TODO: Come up with a way to keep these in sync on both client and server
    client->torso.yawing   = qtrue;
    client->torso.pitching = qtrue;
    client->legs.yawing    = qtrue;

    // Calculate the real torso and leg angles
    BG_PlayerAngles ( ent->client->ps.viewangles,
                      NULL,

                      ent->client->ghoulLegsAngles,
                      ent->client->ghoulLowerTorsoAngles,
                      ent->client->ghoulUpperTorsoAngles,
                      ent->client->ghoulHeadAngles,

                      (float)(ent->client->ps.leanTime - LEAN_TIME) / LEAN_TIME * LEAN_OFFSET,

                      0,
                      0,
                      level.time,

                      &client->torso,
                      &client->legs,

                      level.time - level.previousTime,

                      client->ps.velocity,

                      qfalse,
                      ent->s.angles2[YAW],
                      NULL );
}

/*
===========
G_FindNearbyClient

Locates a client thats near the given origin
============
*/
gentity_t* G_FindNearbyClient ( vec3_t origin, team_t team, float radius, gentity_t* ignore )
{
    int i;

    for ( i = 0; i < level.numConnectedClients; i ++ )
    {
        gentity_t* other = &g_entities[ level.sortedClients[i] ];
        vec3_t     diff;

        if ( other->client->pers.connected != CON_CONNECTED )
        {
            continue;
        }

        if ( other == ignore )
        {
            continue;
        }

        if ( G_IsClientSpectating ( other->client ) || G_IsClientDead ( other->client ) )
        {
            continue;
        }

        if ( other->client->sess.team != team )
        {
            continue;
        }

        // See if this client is close enough to yell sniper
        VectorSubtract ( other->r.currentOrigin, origin, diff );
        if ( VectorLengthSquared ( diff ) < radius * radius )
        {
            return other;
        }
    }

    return NULL;
}

/*
===========
G_FreeStatsMemory

Frees stats memory either of a client or all.
============
*/
void G_FreeStatsMemory(gentity_t *ent)
{
    int i;

    if (ent == NULL){
        gclient_t *tent;

        for (i = 0; i < level.maxclients; i++)
        {
            tent = &level.clients[i];

            if (tent == NULL)
                continue;
            if (tent->pers.connected != CON_CONNECTED)
                continue;

            if (tent->pers.statinfo.weapon_shots != NULL)
                free(tent->pers.statinfo.weapon_shots);
            if (tent->pers.statinfo.weapon_hits != NULL)
                free(tent->pers.statinfo.weapon_hits);
            if (tent->pers.statinfo.weapon_headshots != NULL)
                free(tent->pers.statinfo.weapon_headshots);

            tent->pers.statinfo.weapon_shots = NULL;
            tent->pers.statinfo.weapon_hits = NULL;
            tent->pers.statinfo.weapon_headshots = NULL;
        }
    }else{
        if (ent->client->pers.statinfo.weapon_shots != NULL)
            free(ent->client->pers.statinfo.weapon_shots);
        if (ent->client->pers.statinfo.weapon_hits != NULL)
            free(ent->client->pers.statinfo.weapon_hits);
        if (ent->client->pers.statinfo.weapon_headshots != NULL)
            free(ent->client->pers.statinfo.weapon_headshots);

        ent->client->pers.statinfo.weapon_shots = NULL;
        ent->client->pers.statinfo.weapon_hits = NULL;
        ent->client->pers.statinfo.weapon_headshots = NULL;
    }
}