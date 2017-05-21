// Copyright (C) 2001-2002 Raven Software
//
#include "g_local.h"
#include "boe_local.h"

int G_MultipleDamageLocations(int hitLocation);

// KRIS 7/08/2003 11:03AM
static int last_showBBoxMissile = 0;
// KRIS

#define MISSILE_PRESTEP_TIME    50

/*
================
G_BounceMissile

================
*/
void G_BounceMissile( gentity_t *ent, trace_t *trace )
{
    vec3_t  velocity;
    float   dot;
    int     hitTime;

    // nothing to do if already stationary
    if ( ent->s.pos.trType == TR_STATIONARY )
    {
        return;
    }

    // reflect the velocity on the trace plane
    hitTime = level.previousTime + ( level.time - level.previousTime ) * trace->fraction;
    BG_EvaluateTrajectoryDelta( &ent->s.pos, hitTime, velocity );
    dot = DotProduct( velocity, trace->plane.normal );
    VectorMA( velocity, -2*dot, trace->plane.normal, ent->s.pos.trDelta );

    if ( ent->s.eFlags & EF_BOUNCE_HALF )
    {
        VectorScale( ent->s.pos.trDelta, 0.65, ent->s.pos.trDelta );
        // check for stop
        if ( trace->plane.normal[2] > 0.2 && VectorLength( ent->s.pos.trDelta ) < 40 )
        {
            G_SetOrigin( ent, trace->endpos );
            return;
        }
    }
    else if ( ent->s.eFlags & EF_BOUNCE_SCALE )
    {
        // IF it hit a client then barely bounce off of them since they are "soft"
        if ( trace->entityNum < MAX_CLIENTS )
        {
            VectorScale( ent->s.pos.trDelta, 0.04f, ent->s.pos.trDelta );

            // Make sure the grenade doesnt continuously collide with teh player it hit
            ent->target_ent = &g_entities[trace->entityNum];
        }
        else
        {
            VectorScale( ent->s.pos.trDelta, ent->bounceScale, ent->s.pos.trDelta );
        }

        // check for stop
        if ( trace->plane.normal[2] > 0.2 && VectorLength( ent->s.pos.trDelta ) < 40 )
        {
            G_SetOrigin( ent, trace->endpos );

            if ( ent->parent && ent->parent->client )
            {
                gentity_t* nearby;

                // Find someone on the opposite team near wher ethe grenade landed
                nearby = G_FindNearbyClient ( trace->endpos, ent->parent->client->sess.team==TEAM_RED?TEAM_BLUE:TEAM_RED, 800, NULL );

                if ( nearby )
                {
                    // Make sure there is someone around to hear them scream
                    nearby = G_FindNearbyClient ( trace->endpos, nearby->client->sess.team, 800, nearby );
                    G_VoiceGlobal ( nearby, "grenade", qtrue );
                }
            }

            return;
        }
    }

    VectorAdd( ent->r.currentOrigin, trace->plane.normal, ent->r.currentOrigin);
    VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
    ent->s.pos.trTime = level.time;

    G_AddEvent( ent, EV_GRENADE_BOUNCE, trace->surfaceFlags& MATERIAL_MASK );
}

/*
================
G_ExplodeMissile

Explode a missile without an impact
================
*/
void G_ExplodeMissile( gentity_t *ent ) {
    vec3_t      dir;
    vec3_t      origin;

    BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );
    SnapVector( origin );
    G_SetOrigin( ent, origin );

    // we don't have a valid direction, so just point straight up
    dir[0] = dir[1] = 0;
    dir[2] = 1;

    ent->s.eType = ET_GENERAL;
    if(current_gametype.value == GT_HS && ent->methodOfDeath == MOD_F1_GRENADE){
        if(ent->r.currentOrigin[2] > ent->parent->r.currentOrigin[2]){
            G_AddEvent( ent, EV_MISSILE_MISS, (DirToByte( dir ) << MATERIAL_BITS) | MATERIAL_NONE);
        }
    }else if(!(current_gametype.value == GT_HZ && (ent->methodOfDeath == MOD_M67_GRENADE || ent->methodOfDeath == MOD_L2A2_GRENADE))){
            G_AddEvent( ent, EV_MISSILE_MISS, (DirToByte( dir ) << MATERIAL_BITS) | MATERIAL_NONE);
    }

    ent->freeAfterEvent = qtrue;

    // All grenade explosions are now broadcast to ensure that fire and smoke is always seen
    ent->r.svFlags |= SVF_BROADCAST;

    // splash damage
    if ( ent->splashDamage )
    {
        if (ent->dflags & DAMAGE_AREA_DAMAGE)
        {
            // do damage over time rather than instantly
            G_CreateDamageArea ( ent->r.currentOrigin, ent->parent, ent->splashDamage*0.05f,ent->splashRadius, 8000,ent->methodOfDeath );

            // do some instant damage
            G_RadiusDamage( ent->r.currentOrigin, ent->parent, ent->damage, ent->splashRadius, ent,
                            1, ent->splashMethodOfDeath );
        }
        else
        {   // normal radius of effect damage
            G_RadiusDamage( ent->r.currentOrigin, ent->parent, ent->splashDamage, ent->splashRadius, ent,
                            1, ent->splashMethodOfDeath );
        }
    }

    trap_LinkEntity( ent );
}

/*
================
G_GrenadeThink

Marks the grenade ready to explode
================
*/
void G_GrenadeThink ( gentity_t* ent )
{
    ent->s.eFlags |= EF_EXPLODE;
}

void G_RunStuckMissile( gentity_t *ent )
{
    if ( ent->takedamage )
    {
        if ( ent->s.groundEntityNum >= 0 && ent->s.groundEntityNum < ENTITYNUM_WORLD )
        {
            gentity_t *other = &g_entities[ent->s.groundEntityNum];

            if ( (!VectorCompare( vec3_origin, other->s.pos.trDelta ) && other->s.pos.trType != TR_STATIONARY) ||
                (!VectorCompare( vec3_origin, other->s.apos.trDelta ) && other->s.apos.trType != TR_STATIONARY) )
            {//thing I stuck to is moving or rotating now, kill me
                G_Damage( ent, other, other, NULL, NULL, 99999, 0, MOD_CRUSH, HL_NONE );
                return;
            }
        }
    }
    // check think function
    G_RunThink( ent );
}

/*
================
G_BounceProjectile
================
*/
void G_BounceProjectile( vec3_t start, vec3_t impact, vec3_t dir, vec3_t endout ) {
    vec3_t v, newv;
    float dot;

    VectorSubtract( impact, start, v );
    dot = DotProduct( v, dir );
    VectorMA( v, -2*dot, dir, newv );

    VectorNormalize(newv);
    VectorMA(impact, 8192, newv, endout);
}


/*
================
G_CreateMissile
================
*/
gentity_t* G_CreateMissile( vec3_t org, vec3_t dir, float vel, int life, gentity_t *owner, attackType_t attack  )
{
    gentity_t   *missile;

    missile = G_Spawn();

    missile->nextthink = level.time + life;
    missile->think = G_FreeEntity;
    missile->s.eType = ET_MISSILE;
    missile->r.svFlags = SVF_USE_CURRENT_ORIGIN;
    missile->parent = owner;
    missile->r.ownerNum = owner->s.number;

    if ( attack == ATTACK_ALTERNATE )
    {
        missile->s.eFlags |= EF_ALT_FIRING;
    }

    missile->s.pos.trType = TR_LINEAR;
    missile->s.pos.trTime = level.time;// - MISSILE_PRESTEP_TIME;   // NOTENOTE This is a Quake 3 addition over JK2
    missile->target_ent = NULL;

    SnapVector(org);
    VectorCopy( org, missile->s.pos.trBase );
    VectorScale( dir, vel, missile->s.pos.trDelta );
    VectorCopy( org, missile->r.currentOrigin);
    VectorCopy( org, missile->origin_from); // Boe!Man 7/10/13: Use this for grenades like the F1 (in H&S), to determine where the seeker threw it from.
    SnapVector(missile->s.pos.trDelta);

    return missile;
}

/*
================
G_CauseAreaDamage
================
*/
void G_CauseAreaDamage( gentity_t *ent )
{
    G_RadiusDamage ( ent->r.currentOrigin, ent->parent, ent->splashDamage, ent->splashRadius, ent, 3, ent->methodOfDeath );

    ent->s.time2--;

    if ( ent->s.time2 <= 0 )
    {
        G_FreeEntity ( ent );
        return;
    }

    ent->nextthink = level.time + 350;
    trap_LinkEntity( ent );
}

void Henk_PushArea( gentity_t *ent )
{
    vec3_t  dir, dir2;
    vec3_t  fireAngs;
    vec3_t  matrix[3], transpose[3];
    vec3_t  org, org2, move2;
    float   knockback = 200;
    gentity_t *tent;
    char *originstr;
    int i, e;
    int         entityList[MAX_GENTITIES];
    int         numListedEntities;
    vec3_t      mins, maxs;
    for ( i = 0 ; i < 3 ; i++ )
    {
        mins[i] =  ent->r.currentOrigin[i] - ent->splashRadius;
        maxs[i] =  ent->r.currentOrigin[i] + ent->splashRadius;
    }

    numListedEntities = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );

    for ( e = 0 ; e < numListedEntities ; e++ ) // henk note: Loop through all entities caught in the radius
    {
        tent = &g_entities[entityList[ e ]];
        if(tent && tent->client){
            if(level.time >= tent->client->sess.lastpush && tent->client->sess.team == TEAM_BLUE){
                // FIXME: This isn't really very efficient.
                // The only way to properly solve this is a matrix (just checking viewangles isn't enough).
                // We diff the two positions and rotate that point.
                VectorCopy(tent->r.currentAngles, fireAngs);
                G_CreateRotationMatrix(fireAngs, transpose);
                G_TransposeMatrix(transpose, matrix);
                G_RotatePoint(fireAngs, matrix);
                VectorSubtract(tent->r.currentOrigin, ent->r.currentOrigin, org);
                VectorCopy(org, org2);
                G_RotatePoint(org2, matrix);
                VectorSubtract(org2, org, move2);
                AngleVectors( move2, dir, dir2, NULL );

                VectorNormalize(dir);
                VectorNormalize(dir2);
                // Check which way to push him based on where the player is.
                if (ent->r.currentOrigin[0] > tent->r.currentOrigin[0]){
                    dir[0] = dir2[0] - dir[0];
                }
                if (ent->r.currentOrigin[1] > tent->r.currentOrigin[1]){
                    dir[1] = dir2[1] - dir[1];
                }

                // Knock the player back.
                G_ApplyKnockback ( tent, dir, knockback );

                // Also show the effect.
                originstr = va("%.0f %.0f %.0f", tent->r.currentOrigin[0], tent->r.currentOrigin[1], tent->r.currentOrigin[2]+45);
                G_PlayEffect ( G_EffectIndex("levels/kam_train_sparks"),tent->r.currentOrigin, vec3_origin);
                //AddSpawnField("classname", "1fx_play_effect");
                //AddSpawnField("effect",   "levels/kam_train_sparks");
                //AddSpawnField("origin", originstr);
                //AddSpawnField("wait", "1");
                //AddSpawnField("count", "1");
                //G_SpawnGEntityFromSpawnVars (qtrue);
                tent->client->sess.lastpush = level.time+500;
            }
        }
    }

    ent->s.time2--;

    if ( ent->s.time2 <= 0 )
    {
        G_FreeEntity ( ent );
        return;
    }
    //levels/kam_train_sparks.efx
    //levels/hk6_spark_shower
    originstr = va("%.0f %.0f %.0f", ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2]+45);
    AddSpawnField("classname", "1fx_play_effect");
    AddSpawnField("effect", "jon_sam_trail");
    AddSpawnField("origin", originstr);
    AddSpawnField("count", "1");
    AddSpawnField("wait", "3");
    G_SpawnGEntityFromSpawnVars (qtrue);

    ent->nextthink = level.time + 500;
    trap_LinkEntity( ent );
}

/*
================
G_CreateDamageArea
================
*/
gentity_t* G_CreateDamageArea ( vec3_t origin, gentity_t* attacker, float damage, float radius, int duration, int mod )
{
    gentity_t   *damageArea;

    damageArea = G_Spawn();
    if(current_gametype.value == GT_HZ && (mod == MOD_M67_GRENADE || mod == altAttack(MOD_M67_GRENADE))){
        damageArea->nextthink = level.time + 500;
        damageArea->think = Henk_PushArea;
    }else if (!(current_gametype.value == GT_HZ && (mod == MOD_L2A2_GRENADE || mod == altAttack(MOD_L2A2_GRENADE)))){
        damageArea->nextthink = level.time + 350;
        damageArea->think = G_CauseAreaDamage;
    }

    damageArea->s.eType = ET_DAMAGEAREA;
    damageArea->r.svFlags = SVF_USE_CURRENT_ORIGIN;
    damageArea->parent = attacker;
    damageArea->r.ownerNum = attacker->s.number;

    damageArea->s.pos.trType = TR_STATIONARY;
    damageArea->s.pos.trTime = level.time;
    damageArea->s.time2 = duration / 350;
    damageArea->target_ent = NULL;

    damageArea->classname = "DamageArea";

    VectorSet( damageArea->r.maxs, 1, 1, 1 );
    VectorScale( damageArea->r.maxs, -1, damageArea->r.mins );

    damageArea->splashDamage = damage;
    damageArea->splashRadius = radius;
    damageArea->methodOfDeath = mod;

    damageArea->dflags = DAMAGE_RADIUS;
    damageArea->clipmask = MASK_SHOT;

    VectorCopy( origin, damageArea->s.pos.trBase );
    VectorCopy( origin, damageArea->r.currentOrigin);
    SnapVector( damageArea->r.currentOrigin  );

    return damageArea;
}

/*
================
G_MissileImpact
================
*/
extern gentity_t *CreateWeaponPickup(vec3_t pos,weapon_t weapon);
extern int G_GetHitLocation(gentity_t *target, vec3_t ppoint, vec3_t dir );
void G_MissileImpact( gentity_t *ent, trace_t *trace )
{
    gentity_t       *other;
    vec3_t  velocity;
    int d;
    static vec3_t   mins = {-15,-15,-45};
    static vec3_t   maxs = {15,15,46};
    other = &g_entities[trace->entityNum];
    d = 0;

    // check for bounce
#ifndef _3DServer
    if(current_gametype.value != GT_HS && current_gametype.value != GT_HZ){ // Henk 19/01/10 -> Grenades explode on impact
#else
    if (current_gametype.value != GT_HS && current_gametype.value != GT_HZ && !(current_gametype.value == GT_ELIM && boe_fragWars.integer)){ // Henk 19/01/10 -> Grenades explode on impact
#endif // not _3DServer
        if ( ( ent->s.eFlags & ( EF_BOUNCE | EF_BOUNCE_HALF | EF_BOUNCE_SCALE ) ) )
        {
            G_BounceMissile( ent, trace );
            return;
        }
    }
    // impact damage
    if (other->takedamage)
    {
        // FIXME: wrong damage direction?
        BG_EvaluateTrajectoryDelta( &ent->s.pos, level.time, velocity );
        if ( VectorLength( velocity ) == 0 )
        {
            velocity[2] = 1;    // stepped on a grenade
        }
        if ( ent->damage )
        {
            int location;

            location = HL_NONE;
            if ( other->client )
            {
                VectorNormalize ( velocity );
                // KRIS 12/08/2003 11:14AM
                if (ent->s.otherEntityNum2)
                {
                    location = ent->s.otherEntityNum2;
                }
                else
                // KRIS
                location = G_GetHitLocation ( other, ent->r.currentOrigin, velocity );
                if ( ent->splashDamage )
                {
                    location = G_MultipleDamageLocations(location);
                }
            }

            d = G_Damage(other, ent, &g_entities[ent->r.ownerNum], velocity,
                     ent->s.origin, ent->damage, ent->dflags,
                     ent->methodOfDeath, location );
            if(current_gametype.value == GT_HS && ent->methodOfDeath == 257)
                d = 1;
            if ( d && other->client)
            {
                gentity_t *tent;
                vec3_t hitdir;

                //Ryan may 16 2004
                //log the hit into our stats
                statinfo_t *stat = &g_entities[ent->r.ownerNum].client->pers.statinfo;

                if(!level.gametypeData->teams || (level.gametypeData->teams && !OnSameTeam ( &g_entities[ent->r.ownerNum], other )))
                {
                    stat->hitcount++;
                    stat->accuracy = (float)stat->hitcount / (float)stat->shotcount * 100;

                    if(ent->s.weapon == WP_M4_ASSAULT_RIFLE)
                    {
                        stat->weapon_hits[ATTACK_ALTERNATE * level.wpNumWeapons + WP_M4_ASSAULT_RIFLE]++;
                    }
                    else
                    {
                        stat->weapon_hits[ATTACK_NORMAL * level.wpNumWeapons + ent->s.weapon]++;
                    }
                }
                //Ryan
                // Put some procedural gore on the target.
                tent = G_TempEntity( ent->r.currentOrigin, EV_EXPLOSION_HIT_FLESH );

                // send entity and direction
                VectorSubtract(other->r.currentOrigin, ent->r.currentOrigin, hitdir);
                VectorNormalize(hitdir);
                tent->s.eventParm = DirToByte( hitdir );
                tent->s.otherEntityNum2 = other->s.number;          // Victim entity number

                // Pack the shot info into the temp end for gore
                tent->s.time  = ent->s.weapon + ((((int)other->s.apos.trBase[YAW]&0x7FFF) % 360) << 16);
                if ( ent->s.eFlags & EF_ALT_FIRING )
                {
                    tent->s.time += (ATTACK_ALTERNATE<<8);
                }

                VectorCopy ( other->r.currentOrigin, tent->s.angles );
                SnapVector ( tent->s.angles );
            }
        }
    }

    // is it cheaper in bandwidth to just remove this ent and create a new
    // one, rather than changing the missile into the explosion?

    if ( d && other->client)
    {
        if(current_gametype.value == GT_HZ && ent->methodOfDeath == MOD_M67_GRENADE){

        }else if(current_gametype.value == GT_HS && ent->methodOfDeath == MOD_F1_GRENADE){
            vec3_t          org1, org2;
            trace_t         tr;
            tr.fraction = 0.0f;
            VectorCopy(trace->endpos, org1);
            VectorCopy(trace->endpos, org2);
            org1[2] += 50;
            trap_Trace ( &tr, org1, mins, maxs, org2, ent->parent->s.number, MASK_ALL );
            if ( tr.startsolid || tr.allsolid )
            {
            G_AddEvent( ent, EV_MISSILE_HIT,
                    (DirToByte( trace->plane.normal ) << MATERIAL_BITS) | (trace->surfaceFlags & MATERIAL_MASK));
            }else if(trace->endpos[2] > ent->parent->r.currentOrigin[2]){
                G_AddEvent( ent, EV_MISSILE_MISS,
                    (DirToByte( trace->plane.normal ) << MATERIAL_BITS) | (trace->surfaceFlags & MATERIAL_MASK));
            }
        }else{
        G_AddEvent( ent, EV_MISSILE_HIT,
                    (DirToByte( trace->plane.normal ) << MATERIAL_BITS) | (trace->surfaceFlags & MATERIAL_MASK));
        }
        ent->s.otherEntityNum = other->s.number;
        // KRIS 12/08/2003 11:14AM
        //if( ent->damage )
        if (!ent->s.otherEntityNum2 && ent->damage)
        // KRIS
        {
            // FIXME: might be able to use the value from inside G_Damage to avoid recalc???
            ent->s.otherEntityNum2 = G_GetHitLocation ( other, g_entities[ent->r.ownerNum].r.currentOrigin, velocity );
        }
    }
    else
    {
        weapon_t forceCreate = WP_NONE;

        if (current_gametype.value == GT_HZ && (ent->methodOfDeath == MOD_M67_GRENADE || ent->methodOfDeath == altAttack(MOD_M67_GRENADE))){

        }else if(current_gametype.value == GT_HZ && (ent->methodOfDeath == MOD_L2A2_GRENADE || ent->methodOfDeath == altAttack(MOD_L2A2_GRENADE)) && ent->think != HZ_Claymore){
            forceCreate = WP_L2A2_GRENADE;
        }else if(current_gametype.value == GT_HS && ent->methodOfDeath == MOD_F1_GRENADE){
            vec3_t          org1, org2;
            trace_t         tr;
            tr.fraction = 0.0f;
            VectorCopy(trace->endpos, org1);
            VectorCopy(trace->endpos, org2);
            org1[2] += 50;
            trap_Trace ( &tr, org1, mins, maxs, org2, ent->parent->s.number, MASK_ALL );
            if ( tr.startsolid || tr.allsolid )
            {
            G_AddEvent( ent, EV_MISSILE_HIT,
                    (DirToByte( trace->plane.normal ) << MATERIAL_BITS) | (trace->surfaceFlags & MATERIAL_MASK));
            }else if(trace->endpos[2] > ent->parent->r.currentOrigin[2]){
                G_AddEvent( ent, EV_MISSILE_MISS,
                    (DirToByte( trace->plane.normal ) << MATERIAL_BITS) | (trace->surfaceFlags & MATERIAL_MASK));
            }
        }else{
        G_AddEvent( ent, EV_MISSILE_MISS,
                    (DirToByte( trace->plane.normal ) << MATERIAL_BITS) | (trace->surfaceFlags & MATERIAL_MASK));
        }

        // If missile should stick into impact point (e.g. a thrown knife).
        if((!Q_stricmp(ent->classname,"Knife") && current_gametype.value != GT_HS) || (current_gametype.value == GT_HZ && forceCreate == WP_L2A2_GRENADE)) // Henk 28/01/10 -> No weapon pickup creating for throw knifes..
        {
            // Create a pickup where we impacted.
            vec3_t      pickupPos;
            gentity_t   *pickupEnt;

            VectorMA(trace->endpos,1,trace->plane.normal,pickupPos);

            pickupEnt = CreateWeaponPickup(pickupPos, (current_gametype.value == GT_HZ && forceCreate == WP_L2A2_GRENADE) ? WP_L2A2_GRENADE : WP_KNIFE);
            if(pickupEnt)
            {
                vec3_t weapDir, weapAngles;

                BG_EvaluateTrajectoryDelta(&ent->s.pos, level.time, weapDir);

                //FIXME: needs work to set model angles!
                VectorNormalize(weapDir);
                vectoangles(weapDir, weapAngles);
                weapAngles[YAW] += 90;
                weapAngles[ROLL] = weapAngles[PITCH];
                weapAngles[PITCH] = 0;

                pickupEnt->s.angles[0] = weapAngles[0];
                pickupEnt->s.angles[1] = weapAngles[1];
                pickupEnt->s.angles[2] = weapAngles[2];

                if (!Q_stricmp(ent->classname, "Knife") && current_gametype.value != GT_HS){
                    pickupEnt->think = G_FreeEntity;
                    pickupEnt->nextthink = level.time + 30000;  // Stick around for 30 seconds
                }else{
                    int         i, num;
                    int         touch[MAX_GENTITIES];
                    gentity_t   *hit;
                    vec3_t      bboxMins, bboxMaxs;

                    // Boe!Man 12/13/14: Claymore specifics.
                    // Make sure it doesn't get picked up.
                    pickupEnt->touch = NULL;

                    // Its main think function.
                    pickupEnt->think = HZ_Claymore;
                    pickupEnt->nextthink = level.time + 500;

                    // We need to make sure the server knows this entity can take damage.
                    // With this, we can shoot at the entity.
                    pickupEnt->takedamage = qtrue;
                    pickupEnt->r.contents = -1;
                    pickupEnt->die = HZ_ClaymoreShoot;

                    // Copy over some essentials, so we can mimic a proper blast + damage area later.
                    pickupEnt->parent = ent->parent;
                    pickupEnt->splashDamage = ent->splashDamage;
                    pickupEnt->splashRadius = ent->splashRadius;
                    pickupEnt->splashMethodOfDeath = ent->splashMethodOfDeath;

                    // Check if a human was caught in the radius.
                    // If so, instantly explode.
                    VectorAdd(trace->endpos, mins, bboxMins);
                    VectorAdd(trace->endpos, maxs, bboxMaxs);

                    num = trap_EntitiesInBox(bboxMins, bboxMaxs, touch,
                        MAX_GENTITIES);

                    // Iterate through caught entities.
                    for(i = 0; i < num; i++){
                        hit = &g_entities[touch[i]];
                        if(hit->client && hit->client->sess.team == TEAM_RED){
                            // We hit a human.
                            G_printInfoMessage(ent->parent,
                                "Claymores cannot be thrown on humans!");

                            // Free entity and return.
                            G_FreeEntity(pickupEnt);
                            G_FreeEntity(ent);
                            return;
                        }
                    }
                }
                pickupEnt->count = 1;

                pickupEnt->s.eFlags |= EF_ANGLE_OVERRIDE;
                VectorCopy(pickupEnt->s.angles,pickupEnt->r.currentAngles);
                VectorCopy(pickupEnt->s.angles,pickupEnt->s.apos.trBase);
                pickupEnt->s.pos.trType=TR_STATIONARY;
                pickupEnt->s.apos.trTime=level.time;
                pickupEnt->clipmask = ent->clipmask;
                pickupEnt->s.groundEntityNum = trace->entityNum;
                trap_LinkEntity(pickupEnt);

                if (current_gametype.value == GT_HZ && forceCreate == WP_L2A2_GRENADE){
                    G_FreeEntity(ent);
                    return;
                }
            }
        }
    }

    ent->freeAfterEvent = qtrue;

    // All grenade explosions are now broadcast to ensure that fire and smoke is always seen
    ent->r.svFlags |= SVF_BROADCAST;

    // change over to a normal entity right at the point of impact
    ent->s.eType = ET_GENERAL;

    SnapVectorTowards( trace->endpos, ent->s.pos.trBase );  // save net bandwidth

    G_SetOrigin( ent, trace->endpos );

    // splash damage (doesn't apply to person directly hit)
    if ( ent->splashDamage )
    {
        if (ent->dflags & DAMAGE_AREA_DAMAGE)
        {
            // do damage over time rather than instantly
            G_CreateDamageArea ( trace->endpos, ent->parent, ent->splashDamage*0.10f,ent->splashRadius*2, 8000,ent->methodOfDeath );

            //do some instant damage
            G_RadiusDamage( trace->endpos, ent->parent, ent->damage, ent->splashRadius, other,
                            1, ent->splashMethodOfDeath );
        }
        else
        {   // normal radius of effect damage
            if(trace->endpos[2] > 2900 && ent->splashMethodOfDeath == WP_M15_GRENADE && level.crossTheBridge){
                if(ent->parent && ent->parent->client){
                    trap_SendServerCommand(ent->parent->s.number, va("print\"^3[Cross The Bridge] ^7You cannot blind the seekers by nading their choppers!\n\""));
                    G_FreeEntity(ent);
                }
            }else{
            G_RadiusDamage( trace->endpos, ent->parent, ent->splashDamage, ent->splashRadius,
                            other, 1, ent->splashMethodOfDeath );
            }
        }
    }

    trap_LinkEntity( ent );
}

/*
================
G_RunMissile
================
*/
void G_RunMissile( gentity_t *ent )
{
    vec3_t      origin;
    trace_t     tr;
    int         passent;

    // get current position
    BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );

    // if this missile bounced off an invulnerability sphere
    if ( ent->target_ent )
    {
        passent = ent->target_ent->s.number;
    }
    else
    {
        // ignore interactions with the missile owner
        passent = ent->r.ownerNum;
    }

    // Special case where the grenade has gone up into the sky
    if ( ent->s.eFlags & EF_INSKY )
    {
        // Check to see if its out of the world on the X,Y plane
        // or below it (above is a special case)
        if ( origin[0] < level.worldMins[0] ||
             origin[1] < level.worldMins[1] ||
             origin[0] > level.worldMaxs[0] ||
             origin[1] > level.worldMaxs[1] ||
             origin[2] < level.worldMins[2]    )
        {
            G_FreeEntity( ent );
            return;
        }

        // Above it only kills it if the item has no gravity
        if ( origin[2] > level.worldMaxs[2] && ent->s.pos.trType != TR_GRAVITY && ent->s.pos.trType != TR_LIGHTGRAVITY)
        {
            G_FreeEntity( ent );
            return;
        }

        trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, passent, ent->clipmask );

        // Hit another sky, must be reentering
        if ( tr.fraction == 1.0f )
        {
            ent->s.eFlags &= ~EF_INSKY;
            VectorCopy ( origin, ent->r.currentOrigin );
        }

        VectorCopy ( origin, ent->r.currentOrigin );
        trap_LinkEntity ( ent );
    }
    else
    {
        // Run the same test again because the condition may have changed as a result
        // of the greande falling below the sky again
        // Loop this trace so we can break windows
        // KRIS 7/08/2003 11:00AM
        int loopStopper;

        loopStopper = 0;

        // enlarge bboxs
        G_AdjustClientBBoxs();

        ent->s.otherEntityNum2 = 0;

        while ( 1 )
        {
            loopStopper++;
            if (loopStopper > 128)
            {
                Com_Printf(S_COLOR_YELLOW"G_RunMissile(): - loop fucked up\n");
                break;
            }

            // trace a line from the previous position to the current position
            trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, passent, ent->clipmask );

            // If its glass then redo the trace after breaking the glass
            if ( tr.fraction != 1 && !Q_stricmp ( g_entities[tr.entityNum].classname, "func_glass" ) )
            {
                g_entities[tr.entityNum].use ( &g_entities[tr.entityNum], ent, ent );
                continue;
            }

            if (tr.fraction != 1 && g_entities[ tr.entityNum ].client && (g_entities[ tr.entityNum ].client->ps.pm_flags & PMF_LEANING))
            {
                gentity_t *other;
                vec3_t rOrigin;

                // ok, we've hit someone who is leaning...
                //Com_Printf(S_COLOR_YELLOW"G_RunMissile(): - hit client %i who is leaning\n", tr.entityNum);

                other = &g_entities[tr.entityNum];

                // adjust the standard bbox to compensate for
                // different head position and trace against that
                G_UndoAdjustedClientBBox(other);

                // adjust bbox and origin while leaning
                G_SetClientPreLeaningBBox(other);

/*              if (g_debugAdjBBox.integer && (level.time - last_showBBoxMissile > 500))
                {
                    G_ShowClientBBox(other);
                }
*/
                trap_LinkEntity ( other );

                // trace against slightly adjust bbox (head gone)
                trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, passent, ent->clipmask );

                // we hit the adjusted bbox, stop here
                if (tr.entityNum == other->s.number)
                {
                    // KRIS 17/06/2003 2:33AM
/*                  if (g_debugAdjBBox.integer && (level.time - last_showBBoxMissile > 500))
                    {
                        last_showBBoxMissile = level.time;
                    }
                    // KRIS
*/
                    //Ryan april 6 2004
                    //these are all wrong r.currentOrigin always is at a certain height
                    //above th4e ground the player is on and if he's duckin it stays the
                    //same so these hit locations will be way off
                /*  if (tr.endpos[2] > other->r.currentOrigin[2] + 4)
                    {
                        ent->s.otherEntityNum2 = HL_WAIST;
                    }
                    else if (tr.endpos[2] > other->r.currentOrigin[2] - 24)
                    {
                        ent->s.otherEntityNum2 = HL_FOOT_LT;
                    }
                    else if (tr.endpos[2] > other->r.currentOrigin[2] - 10)
                    {
                        ent->s.otherEntityNum2 = HL_LEG_LOWER_LT;
                    }
                    else
                    {
                        ent->s.otherEntityNum2 = HL_LEG_UPPER_LT;
                    }*/

                    if (other->client->ps.pm_flags & PMF_DUCKED)
                    {
                        if (tr.endpos[2] > (other->r.currentOrigin[2] + other->r.maxs[2]) - 5)
                        {
                            ent->s.otherEntityNum2 = HL_WAIST;
                        }
                        else if (tr.endpos[2] > (other->r.currentOrigin[2] + other->r.maxs[2]) - 17)
                        {
                            ent->s.otherEntityNum2 = HL_LEG_UPPER_LT;
                        }
                        else if (tr.endpos[2] > (other->r.currentOrigin[2] + other->r.maxs[2]) - 23)
                        {
                            ent->s.otherEntityNum2 = HL_LEG_LOWER_LT;
                        }
                        else
                        {
                            ent->s.otherEntityNum2 = HL_FOOT_LT;
                        }
                    }
                    else
                    {
                        if (tr.endpos[2] > (other->r.currentOrigin[2] + other->r.maxs[2]) - 10)
                        {
                            ent->s.otherEntityNum2 = HL_WAIST;
                        }
                        else if (tr.endpos[2] > (other->r.currentOrigin[2] + other->r.maxs[2]) - 20)
                        {
                            ent->s.otherEntityNum2 = HL_LEG_UPPER_LT;
                        }
                        else if (tr.endpos[2] > (other->r.currentOrigin[2] + other->r.maxs[2]) - 38)
                        {
                            ent->s.otherEntityNum2 = HL_LEG_LOWER_LT;
                        }
                        else
                        {
                            ent->s.otherEntityNum2 = HL_FOOT_LT;
                        }
                    }
                    //Ryan

                    //Com_Printf(S_COLOR_YELLOW"G_RunMissile(): - we hit the adjusted bbox, stop here\n");
                    break;
                }

                // failing the first check, move the bbox origin in
                // the direction of the lean and make it smaller
                VectorCopy (other->r.currentOrigin, rOrigin);
                G_UndoAdjustedClientBBox(other);

                // adjust bbox and origin while leaning
                G_SetClientLeaningBBox(other);

/*              if (g_debugAdjBBox.integer && (level.time - last_showBBoxMissile > 500))
                {
                    G_ShowClientBBox(other);
                }
*/
                trap_LinkEntity ( other );

                // trace against smaller, offset bbox
                trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, passent, ent->clipmask );

                // reset origin
                VectorCopy (rOrigin, other->r.currentOrigin);

                // KRIS 17/06/2003 2:33AM
/*              if (g_debugAdjBBox.integer && (level.time - last_showBBoxMissile > 500))
                {
                    last_showBBoxMissile = level.time;
                }
                // KRIS
*/
                // good leaning hit, stop here
                if (tr.entityNum == other->s.number)
                {
                    //Com_Printf(S_COLOR_YELLOW"G_RunMissile(): - good leaning hit, stop here\n");
                    //Ryan this is off to for the same reason as above
                /*  if (tr.endpos[2] > other->r.currentOrigin[2] + 8)
                    {
                        ent->s.otherEntityNum2 = HL_HEAD;
                    }
                    else
                    {
                        ent->s.otherEntityNum2 = HL_WAIST;
                    }*/
                    if (tr.endpos[2] > (other->r.currentOrigin[2] + other->r.maxs[2]) - 10)
                    {
                        ent->s.otherEntityNum2 = HL_HEAD;
                    }
                    else
                    {
                        ent->s.otherEntityNum2 = HL_CHEST;
                    }
                    //Ryan
                    break;
                }

                // bad hit, ignore them and move on
                //Com_Printf(S_COLOR_YELLOW"G_RunMissile(): - bad hit, ignore them and move on\n");

                passent = other->s.number;
                continue;
            }
            break;
        }

        // return bboxs to normal
        G_UndoAdjustedClientBBoxs();

        // set final position
        VectorCopy( tr.endpos, ent->r.currentOrigin );
        // KRIS

        //Ryan Old code commented out for above code
/*
        while ( 1 )
        {
            // trace a line from the previous position to the current position
            trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, passent, ent->clipmask );

            // If its glass then redo the trace after breaking the glass
            if ( tr.fraction != 1 && !Q_stricmp ( g_entities[tr.entityNum].classname, "func_glass" ) )
            {
                g_entities[tr.entityNum].use ( &g_entities[tr.entityNum], ent, ent );
                continue;
            }

            break;
        }

        if ( tr.startsolid || tr.allsolid )
        {
            // make sure the tr.entityNum is set to the entity we're stuck in
            trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, ent->r.currentOrigin, passent, ent->clipmask );
            tr.fraction = 0;
        }
        else
        {
            VectorCopy( tr.endpos, ent->r.currentOrigin );
        }
*/
        //Ryan
        trap_LinkEntity( ent );

        if ( tr.fraction != 1 )
        {
            // Hit the sky or moving through something
            if ( tr.surfaceFlags & SURF_NOIMPACT )
            {
                // Dont kill a missle that hits the sky and has gravity
                if ( tr.surfaceFlags & SURF_SKY )
                {
                    ent->s.eFlags |= EF_INSKY;
                    ent->r.svFlags |= SVF_BROADCAST;
                    VectorCopy ( origin, ent->r.currentOrigin );
                    trap_LinkEntity( ent );
                }
                else
                {
                    G_FreeEntity( ent );
                    return;
                }
            }
            else
            {
                G_MissileImpact( ent, &tr );

                // Is it time to explode
                if ( ent->s.eFlags & EF_EXPLODE )
                {
                    ent->s.eFlags &= (~EF_EXPLODE);
                    G_ExplodeMissile ( ent );
                    return;
                }

                // Exploded
                if ( ent->s.eType != ET_MISSILE )
                {
                    return;
                }
            }
        }
        // Is it time to explode
        else if ( ent->s.pos.trType == TR_STATIONARY && (ent->s.eFlags & EF_EXPLODE) )
        {
            ent->s.eFlags &= (~EF_EXPLODE);
            G_ExplodeMissile ( ent );
            return;
        }
    }

    // If this is a knife then reorient its angles
    if ( ent->s.weapon == WP_KNIFE )
    {
        vec3_t vel;

        BG_EvaluateTrajectoryDelta( &ent->s.pos, level.time, vel );
        vectoangles( vel, ent->s.angles );

        ent->s.angles[YAW] += 90;
//      ent->s.angles[ROLL] = ent->s.angles[PITCH];
        ent->s.angles[ROLL] = 0;
        ent->s.angles[PITCH] = 0;
    }

    // check think function after bouncing
    G_RunThink( ent );
}


//=============================================================================




