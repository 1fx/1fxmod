// Copyright (C) 2001-2002 Raven Software
//
#include "g_local.h"

//==========================================================

/*QUAKED target_give (1 0 0) (-8 -8 -8) (8 8 8)
Gives the activator all the items pointed to.
*/
void Use_Target_Give( gentity_t *ent, gentity_t *other, gentity_t *activator ) {
    gentity_t   *t;
    trace_t     trace;

    if ( !activator->client ) {
        return;
    }

    if ( !ent->target ) {
        return;
    }

    memset( &trace, 0, sizeof( trace ) );
    t = NULL;
    while ( (t = G_Find (t, FOFS(targetname), ent->target)) != NULL ) {
        if ( !t->item ) {
            continue;
        }
        Touch_Item( t, activator, &trace );

        // make sure it isn't going to respawn or show any events
        t->nextthink = 0;
        trap_UnlinkEntity( t );
    }
}

void SP_target_give( gentity_t *ent ) {
    ent->use = Use_Target_Give;
}


//==========================================================

/*QUAKED target_delay (1 0 0) (-8 -8 -8) (8 8 8)
"wait" seconds to pause before firing targets.
"random" delay variance, total delay = delay +/- random seconds
*/
void Think_Target_Delay( gentity_t *ent ) {
    G_UseTargets( ent, ent->activator );
}

void Use_Target_Delay( gentity_t *ent, gentity_t *other, gentity_t *activator ) {
    ent->nextthink = level.time + ( ent->wait + ent->random * crandom() ) * 1000;
    ent->think = Think_Target_Delay;
    ent->activator = activator;
}

void SP_target_delay( gentity_t *ent ) {
    // check delay for backwards compatability
    if ( !G_SpawnFloat( "delay", "0", &ent->wait ) ) {
        G_SpawnFloat( "wait", "1", &ent->wait );
    }

    if ( !ent->wait ) {
        ent->wait = 1;
    }
    ent->use = Use_Target_Delay;
}


//==========================================================

/*QUAKED target_score (1 0 0) (-8 -8 -8) (8 8 8)
"count" number of points to add, default 1

The activator is given this many points.
*/
void Use_Target_Score (gentity_t *ent, gentity_t *other, gentity_t *activator)
{
    G_AddScore( activator, ent->count );
}

void SP_target_score( gentity_t *ent )
{
    if ( !ent->count )
    {
        ent->count = 1;
    }

    ent->use = Use_Target_Score;
}


//==========================================================

/*QUAKED target_print (1 0 0) (-8 -8 -8) (8 8 8) redteam blueteam private
"message"   text to print
If "private", only the activator gets the message.  If no checks, all clients get the message.
*/
void Use_Target_Print (gentity_t *ent, gentity_t *other, gentity_t *activator) {
    if ( activator->client && ( ent->spawnflags & 4 ) ) {
        G_Broadcast(ent->message, BROADCAST_GAME, activator);
        return;
    }

    if ( ent->spawnflags & 3 ) {
        if ( ent->spawnflags & 1 ) {
            G_TeamCommand( TEAM_RED, ent->message);
        }
        if ( ent->spawnflags & 2 ) {
            G_TeamCommand( TEAM_BLUE, ent->message);
        }
        return;
    }

    G_Broadcast(ent->message, BROADCAST_GAME, NULL);
}

void SP_target_print( gentity_t *ent ) {
    ent->use = Use_Target_Print;
}


//==========================================================


/*QUAKED target_speaker (1 0 0) (-8 -8 -8) (8 8 8) looped-on looped-off global activator
"noise"     wav file to play
"soundSet"  soundset name to use (do not combine with 'noise', ignores all other flags)

A global sound will play full volume throughout the level.
Activator sounds will play on the player that activated the target.
Global and activator sounds can't be combined with looping.
Normal sounds play each time the target is used.
Looped sounds will be toggled by use functions.
Multiple identical looping sounds will just increase volume without any speed cost.
"wait" : Seconds between auto triggerings, 0 = don't auto trigger
"random"    wait variance, default is 0
"radius"    radius of attenuation
*/
void Use_Target_Speaker (gentity_t *ent, gentity_t *other, gentity_t *activator) {
    if (ent->spawnflags & 3) {  // looping sound toggles
        if (ent->s.loopSound)
            ent->s.loopSound = 0;   // turn it off
        else
            ent->s.loopSound = ent->noise_index;    // start it
    }else { // normal sound
        if ( ent->spawnflags & 8 ) {
            G_AddEvent( activator, EV_GENERAL_SOUND, ent->noise_index );
        } else if (ent->spawnflags & 4) {
            G_AddEvent( ent, EV_GLOBAL_SOUND, ent->noise_index );
        } else {
            G_AddEvent( ent, EV_GENERAL_SOUND, ent->noise_index );
        }
    }
}

void SP_target_speaker( gentity_t *ent )
{
    char    buffer[MAX_QPATH];
    char    *s;
    float   radius;

    if ( G_SpawnString ( "soundSet", "", &s ) )
    {   // this is a sound set
        ent->s.mSoundSet = G_AmbientSoundSetIndex(s);
        ent->s.eFlags = EF_PERMANENT;
        VectorCopy( ent->s.origin, ent->s.pos.trBase );
        trap_LinkEntity( ent );
        return;
    }

    if ( !G_SpawnString( "noise", "NOSOUND", &s ) )
    {
//      G_Error( "target_speaker without a noise key at %s", vtos( ent->s.origin ) );
        G_FreeEntity( ent );
        return;
    }

    G_SpawnFloat( "wait", "0", &ent->wait );
    G_SpawnFloat( "random", "0", &ent->random );
    G_SpawnFloat ( "radius", "0", &radius );

    // Simple conversion to an integer
    ent->s.time2 = (int) (radius * 1000.0f);

    // force all client reletive sounds to be "activator" speakers that
    // play on the entity that activates it
    if ( s[0] == '*' ) {
        ent->spawnflags |= 8;
    }

    if (!strstr( s, ".wav" )) {
        Com_sprintf (buffer, sizeof(buffer), "%s.wav", s );
    } else {
        Q_strncpyz( buffer, s, sizeof(buffer) );
    }
    ent->noise_index = G_SoundIndex(buffer);

    // a repeating speaker can be done completely client side
    ent->s.eType = ET_SPEAKER;
    ent->s.eventParm = ent->noise_index;
    ent->s.frame = ent->wait * 10;
    ent->s.clientNum = ent->random * 10;


    // check for prestarted looping sound
    if ( ent->spawnflags & 1 ) {
        ent->s.loopSound = ent->noise_index;
    }

    ent->use = Use_Target_Speaker;

    if (ent->spawnflags & 4) {
        ent->r.svFlags |= SVF_BROADCAST;
    }

    VectorCopy( ent->s.origin, ent->s.pos.trBase );

    // must link the entity so we get areas and clusters so
    // the server can determine who to send updates to
    trap_LinkEntity( ent );
}



//==========================================================

/*QUAKED target_laser (0 .5 .8) (-8 -8 -8) (8 8 8) START_ON
When triggered, fires a laser.  You can either set a target or a direction.
*/
void target_laser_think (gentity_t *self) {
    vec3_t  end;
    trace_t tr;
    vec3_t  point;

    // if pointed at another entity, set movedir to point at it
    if ( self->enemy ) {
        VectorMA (self->enemy->s.origin, 0.5, self->enemy->r.mins, point);
        VectorMA (point, 0.5, self->enemy->r.maxs, point);
        VectorSubtract (point, self->s.origin, self->movedir);
        VectorNormalize (self->movedir);
    }

    // fire forward and see what we hit
    VectorMA (self->s.origin, 2048, self->movedir, end);

    trap_Trace( &tr, self->s.origin, NULL, NULL, end, self->s.number, CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_CORPSE);

    if ( tr.entityNum ) {
        // hurt it if we can
        G_Damage ( &g_entities[tr.entityNum], self, self->activator, self->movedir,
            tr.endpos, self->damage, DAMAGE_NO_KNOCKBACK, MOD_TARGET_LASER, HL_NONE );
    }

    VectorCopy (tr.endpos, self->s.origin2);

    trap_LinkEntity( self );
    self->nextthink = level.time + FRAMETIME;
}

void target_laser_on (gentity_t *self)
{
    if (!self->activator)
        self->activator = self;
    target_laser_think (self);
}

void target_laser_off (gentity_t *self)
{
    trap_UnlinkEntity( self );
    self->nextthink = 0;
}

void target_laser_use (gentity_t *self, gentity_t *other, gentity_t *activator)
{
    self->activator = activator;
    if ( self->nextthink > 0 )
        target_laser_off (self);
    else
        target_laser_on (self);
}

void target_laser_start (gentity_t *self)
{
    gentity_t *ent;

    self->s.eType = ET_BEAM;

    if (self->target)
    {
        ent = G_Find (NULL, FOFS(targetname), self->target);
        if (!ent)
        {
            Com_Printf ("%s at %s: %s is a bad target\n", self->classname, vtos(self->s.origin), self->target);
        }
        self->enemy = ent;
    }
    else
    {
        G_SetMovedir (self->s.angles, self->movedir);
    }

    self->use = target_laser_use;
    self->think = target_laser_think;

    if ( !self->damage )
    {
        self->damage = 1;
    }

    if (self->spawnflags & 1)
    {
        target_laser_on (self);
    }
    else
    {
        target_laser_off (self);
    }
}

void SP_target_laser (gentity_t *self)
{
    // let everything else get spawned before we start firing
    self->think = target_laser_start;
    self->nextthink = level.time + FRAMETIME;
}


//==========================================================

void target_teleporter_use( gentity_t *self, gentity_t *other, gentity_t *activator )
{
    gentity_t   *dest;
    //Com_Printf("Teleporting %s to origin\n", activator->client->pers.cleanName);
    if (!activator->client)
    {
        return;
    }

    dest =  G_PickTarget( self->target );
    if (!dest)
    {
        Com_Printf ("Couldn't find teleporter destination\n");
        return;
    }
    //Com_Printf("Teleporting %s to origin\n", activator->client->pers.cleanName);
    TeleportPlayer( activator, dest->s.origin, dest->s.angles, qfalse );
}

/*QUAKED target_teleporter (1 0 0) (-8 -8 -8) (8 8 8)
The activator will be teleported away.
*/
void SP_target_teleporter( gentity_t *self ) {
    if (!self->targetname)
        Com_Printf("untargeted %s at %s\n", self->classname, vtos(self->s.origin));

    self->use = target_teleporter_use;
}

//==========================================================


/*QUAKED target_relay (.5 .5 .5) (-8 -8 -8) (8 8 8) RED_ONLY BLUE_ONLY RANDOM
This doesn't perform any actions except fire its targets.
The activator can be forced to be from a certain team.
if RANDOM is checked, only one of the targets will be fired, not all of them
*/
void target_relay_use (gentity_t *self, gentity_t *other, gentity_t *activator)
{
    if ( ( self->spawnflags & 1 ) && activator->client
        && activator->client->sess.team != TEAM_RED )
    {
        return;
    }
    if ( ( self->spawnflags & 2 ) && activator->client
        && activator->client->sess.team != TEAM_BLUE ) {
        return;
    }
    if ( self->spawnflags & 4 ) {
        gentity_t   *ent;

        ent = G_PickTarget( self->target );
        if ( ent && ent->use ) {
            ent->use( ent, self, activator );
        }
        return;
    }
    G_UseTargets (self, activator);
}

void SP_target_relay (gentity_t *self) {
    self->use = target_relay_use;
}


//==========================================================

/*QUAKED target_kill (.5 .5 .5) (-8 -8 -8) (8 8 8)
Kills the activator.
*/
void target_kill_use( gentity_t *self, gentity_t *other, gentity_t *activator )
{
    G_Damage ( activator, NULL, NULL, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_TELEFRAG, HL_NONE );
}

void SP_target_kill( gentity_t *self ) {
    self->use = target_kill_use;
}

/*QUAKED target_position (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for in-game calculation, like jumppad targets.
*/
void SP_target_position( gentity_t *self ){
    G_SetOrigin( self, self->s.origin );
}

static void target_location_linkup(gentity_t *ent)
{
    int i;
    int n;

    if (level.locationLinked)
        return;

    level.locationLinked = qtrue;

    level.locationHead = NULL;

    trap_SetConfigstring( CS_LOCATIONS, "unknown" );

    for ( i = 0, ent = g_entities, n = 1;
          i < level.num_entities;
          i++, ent++)
    {
        if (ent->classname && !Q_stricmp(ent->classname, "target_location"))
        {
            // lets overload some variables!
            ent->health = n; // use for location marking

            trap_SetConfigstring( CS_LOCATIONS + n, ent->message );
            n++;
            ent->nextTrain = level.locationHead;
            level.locationHead = ent;
        }
    }

    // All linked together now
}

/*QUAKED target_location (0 0.5 0) (-8 -8 -8) (8 8 8)
Set "message" to the name of this location.

Closest target_location in sight used for the location, if none
in site, closest in distance
*/
void SP_target_location( gentity_t *self )
{
    self->think = target_location_linkup;
    self->nextthink = level.time + 200;  // Let them all spawn first
    if(self->message == NULL){
        self->message = "an unknown location";
    }

    G_SetOrigin( self, self->s.origin );
}

/*QUAKED target_effect (0 0.5 0) (-8 -8 -8) (8 8 8)
Plays an effect each time its targetted

"effect"  effect to play
"delay"   delay in milliseconds before the effect plays
*/

void target_effect_delayed_use ( gentity_t* self )
{
    G_PlayEffect( self->health, self->s.origin, self->pos1 );

    self->nextthink = 0;
}

void target_effect_use (gentity_t *self, gentity_t *other, gentity_t *activator)
{
    if ( self->count )
    {
        self->think = target_effect_delayed_use;
        self->nextthink = level.time + self->count;
    }
    else
    {
        target_effect_delayed_use ( self );
    }
}

void SP_target_effect ( gentity_t *ent )
{
    char        *effectName;
    gentity_t   *target;

    G_SetOrigin( ent, ent->s.origin );

    G_SpawnString ( "effect", "", &effectName );
    ent->health = G_EffectIndex(effectName);

    // See if they want it delayed a bit
    G_SpawnInt ( "delay", "0", &ent->count );

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
    {
        ent->pos1[0] = ent->pos1[1] = 0.0;
        ent->pos1[2] = 1.0;
    }

    ent->use = target_effect_use;
}
