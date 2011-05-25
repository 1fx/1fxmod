// Copyright (C) 2001-2002 Raven Software
//
#include "g_local.h"
#include "boe_local.h"


void InitTrigger( gentity_t *self ) {
	if (!VectorCompare (self->s.angles, vec3_origin))
		G_SetMovedir (self->s.angles, self->movedir);
	if(!strstr(self->classname, "teleport") && !strstr(self->model, "NV_MODEL") && !strstr(self->model, "BLOCKED_TRIGGER"))
	trap_SetBrushModel( self, self->model ); // Henk -> This crashes teleports
	self->r.contents = CONTENTS_TRIGGER;		// replaces the -1 from trap_SetBrushModel
	self->r.svFlags = SVF_NOCLIENT;
}


// the wait time has passed, so set back up for another activation
void multi_wait( gentity_t *ent ) {
	ent->nextthink = 0;
}


// the trigger was just activated
// ent->activator should be set to the activator so it can be held through a delay
// so wait for the delay time before firing
void multi_trigger( gentity_t *ent, gentity_t *activator ) {

	ent->activator = activator;
	if ( ent->nextthink ) {
		return;		// can't retrigger until the wait is over
	}

	if ( activator->client ) {
		if ( ( ent->spawnflags & 1 ) && activator->client->sess.team != TEAM_RED ) 
		{
			return;
		}
		
		if ( ( ent->spawnflags & 2 ) && activator->client->sess.team != TEAM_BLUE ) 
		{
			return;
		}
	}

	G_UseTargets (ent, ent->activator);

	if ( ent->wait > 0 ) {
		ent->think = multi_wait;
		ent->nextthink = level.time + ( ent->wait + ent->random * crandom() ) * 1000;
	} else {
		// we can't just remove (self) here, because this is a touch function
		// called while looping through area links...
		ent->touch = 0;
		ent->nextthink = level.time + FRAMETIME;
		ent->think = G_FreeEntity;
	}
}

void Use_Multi( gentity_t *ent, gentity_t *other, gentity_t *activator ) {
	multi_trigger( ent, activator );
}

void Touch_Multi( gentity_t *self, gentity_t *other, trace_t *trace ) {
	if( !other->client ) {
		return;
	}
	multi_trigger( self, other );
}

/*QUAKED trigger_multiple (.5 .5 .5) ?
"wait" : Seconds between triggerings, 0.5 default, -1 = one time only.
"random"	wait variance, default is 0
Variable sized repeatable trigger.  Must be targeted at one or more entities.
so, the basic time between firing is a random time between
(wait - random) and (wait + random)
*/
void SP_trigger_multiple( gentity_t *ent ) 
{
	// This is a hack because the single player game has usable triggers
	// in shared bsp instances, since multiplayer doesnt have useable triggers
	// they end up just triggering when walked through.  So for now just toss them out
	if ( ent->spawnflags & 0x2 )
	{
		G_FreeEntity ( ent );
		return;
	}

	G_SpawnFloat( "wait", "0.5", &ent->wait );
	G_SpawnFloat( "random", "0", &ent->random );

	if ( ent->random >= ent->wait && ent->wait >= 0 ) 
	{
		ent->random = ent->wait - FRAMETIME;
		Com_Printf( "trigger_multiple has random >= wait\n" );
	}

	ent->touch = Touch_Multi;
	ent->use = Use_Multi;
	if(!Q_stricmp(ent->model, "BLOCKED_TRIGGER")){
		if (!VectorCompare (ent->s.angles, vec3_origin))
			G_SetMovedir (ent->s.angles, ent->movedir);
		ent->r.contents = CONTENTS_TRIGGER;
		ent->r.svFlags = SVF_NOCLIENT;
		trap_LinkEntity (ent);
		return;
	}
	InitTrigger( ent );
	trap_LinkEntity (ent);
}



/*
==============================================================================

trigger_always

==============================================================================
*/

void trigger_always_think( gentity_t *ent ) {
	G_UseTargets(ent, ent);
	G_FreeEntity( ent );
}

/*QUAKED trigger_always (.5 .5 .5) (-8 -8 -8) (8 8 8)
This trigger will always fire.  It is activated by the world.
*/
void SP_trigger_always (gentity_t *ent) {
	// we must have some delay to make sure our use targets are present
	ent->nextthink = level.time + 300;
	ent->think = trigger_always_think;
}


/*
==============================================================================

trigger_push

==============================================================================
*/

void trigger_push_touch (gentity_t *self, gentity_t *other, trace_t *trace ) 
{
}


/*
=================
AimAtTarget

Calculate origin2 so the target apogee will be hit
=================
*/
void AimAtTarget( gentity_t *self ) {
	gentity_t	*ent;
	vec3_t		origin;
	float		height, gravity, time, forward;
	float		dist;

	VectorAdd( self->r.absmin, self->r.absmax, origin );
	VectorScale ( origin, 0.5, origin );

	ent = G_PickTarget( self->target );
	if ( !ent ) {
		G_FreeEntity( self );
		return;
	}

	height = ent->s.origin[2] - origin[2];
	gravity = g_gravity.value;
	time = sqrt( height / ( .5 * gravity ) );
	if ( !time ) {
		G_FreeEntity( self );
		return;
	}

	// set s.origin2 to the push velocity
	VectorSubtract ( ent->s.origin, origin, self->s.origin2 );
	self->s.origin2[2] = 0;
	dist = VectorNormalize( self->s.origin2);

	forward = dist / time;
	VectorScale( self->s.origin2, forward, self->s.origin2 );

	self->s.origin2[2] = time * gravity;
}


/*QUAKED trigger_push (.5 .5 .5) ?
Must point at a target_position, which will be the apex of the leap.
This will be client side predicted, unlike target_push
*/
void SP_trigger_push( gentity_t *self ) {
	InitTrigger (self);

	// unlike other triggers, we need to send this one to the client
	self->r.svFlags &= ~SVF_NOCLIENT;

	// make sure the client precaches this sound
	G_SoundIndex("sound/world/jumppad.wav");

	self->s.eType = ET_PUSH_TRIGGER;
	self->touch = trigger_push_touch;
	self->think = AimAtTarget;
	self->nextthink = level.time + FRAMETIME;
	trap_LinkEntity (self);
}


void Use_target_push( gentity_t *self, gentity_t *other, gentity_t *activator ) {
	if ( !activator->client ) {
		return;
	}
	if ( activator->client->ps.pm_type != PM_NORMAL ) {
		return;
	}

	VectorCopy (self->s.origin2, activator->client->ps.velocity);

	// play fly sound every 1.5 seconds
	if ( activator->fly_sound_debounce_time < level.time ) {
		activator->fly_sound_debounce_time = level.time + 1500;
		G_Sound( activator, CHAN_AUTO, self->noise_index );
	}
}

/*QUAKED target_push (.5 .5 .5) (-8 -8 -8) (8 8 8) bouncepad
Pushes the activator in the direction.of angle, or towards a target apex.
"speed"		defaults to 1000
if "bouncepad", play bounce noise instead of windfly
*/
void SP_target_push( gentity_t *self ) {
	if (!self->speed) {
		self->speed = 1000;
	}
	G_SetMovedir (self->s.angles, self->s.origin2);
	VectorScale (self->s.origin2, self->speed, self->s.origin2);

	if ( self->spawnflags & 1 ) {
		self->noise_index = G_SoundIndex("sound/world/jumppad.wav");
	} else {
		self->noise_index = G_SoundIndex("sound/misc/windfly.wav");
	}
	if ( self->target ) {
		VectorCopy( self->s.origin, self->r.absmin );
		VectorCopy( self->s.origin, self->r.absmax );
		self->think = AimAtTarget;
		self->nextthink = level.time + FRAMETIME;
	}
	self->use = Use_target_push;
}

void trigger_booster_touch (gentity_t *self, gentity_t *other, trace_t *trace ) {
	vec3_t origin;

	if ( !other->client ) {
		return;
	}

	if ( other->client->ps.pm_type == PM_DEAD ) {
		return;
	}

	if (G_IsClientSpectating ( other->client ) ) 
	{
		return;
	}

	if(other->client->sess.lastjump >= level.time){
		return;
	}

	if(self->team){
		if(!strstr(self->team, "all")){
			if(other->client->sess.team == TEAM_RED && !strstr(self->team, "red") || other->client->sess.team == TEAM_BLUE && !strstr(self->team, "blue")){
				if(level.time >= other->client->sess.lastmsg){
					if(strstr(self->team, "red")){
						trap_SendServerCommand ( other->s.number, va("cp\"@^7Booster is for %s ^7team only!", server_redteamprefix.string));
					}else if(strstr(self->team, "blue")){
						trap_SendServerCommand ( other->s.number, va("cp\"@^7Booster is for %s ^7team only!", server_blueteamprefix.string));
					}
					//trap_SendServerCommand(other->s.number, va("print\"^3[Info] ^7Only the %s team can use this teleporter.\n\"", self->team));
					other->client->sess.lastmsg = level.time+5000;
				}
				return;
			}
	}
	}

	VectorCopy(self->r.currentOrigin, origin);
	origin[2] += 40;
	G_PlayEffect ( G_EffectIndex("levels/shop7_toxiic_explosion"),origin, self->pos1);
	//G_SpawnGEntityFromSpawnVars (qtrue);
	Henk_CloseSound(other->r.currentOrigin, G_SoundIndex("sound/movers/doors/airlock_door01/airlock_open.mp3"));

	VectorCopy (self->s.origin2, other->client->ps.velocity);
	other->client->ps.velocity[2] += self->up;
	other->client->sess.lastjump = level.time+500;
}

/*
-----------------------------
New easy teleporter by Henkie
13/02/2011
-----------------------------

origin_from: the origin of the teleporter
origin_to: the origin of the teleporter destination
angles_from: the angles of your view angles after teleporting this way
(only for both_sides teleporters)
angles_to: the angles of your view angles after teleporting this way
both_sides: if yes, you can teleporter back as well
team: team for which the teleporter is accessible
max_players: teleporter will disappear if this number of players is exceeded
min_players: teleporter will disappear if less than this number of players
*/

void trigger_NewTeleporter_touch (gentity_t *self, gentity_t *other, trace_t *trace ) {

	if ( !other->client ) {
		return;
	}
	if ( other->client->ps.pm_type == PM_DEAD ) {
		return;
	}
	// Spectators only?
	if ( ( self->spawnflags & 1 ) && !G_IsClientSpectating ( other->client ) ) 
	{
		return;
	}
	if(self->team){
		if(!strstr(self->team, "all")){
			if(other->client->sess.team == TEAM_RED && !strstr(self->team, "red") || other->client->sess.team == TEAM_BLUE && !strstr(self->team, "blue")){
				if(level.time >= other->client->sess.lastmsg){
					if(strstr(self->team, "red")){
						trap_SendServerCommand ( other->s.number, va("cp\"@^7Teleporter is for %s ^7team only!", server_redteamprefix.string));
					}else if(strstr(self->team, "blue")){
						trap_SendServerCommand ( other->s.number, va("cp\"@^7Teleporter is for %s ^7team only!", server_blueteamprefix.string));
					}
					//trap_SendServerCommand(other->s.number, va("print\"^3[Info] ^7Only the %s team can use this teleporter.\n\"", self->team));
					other->client->sess.lastmsg = level.time+5000;
				}
				return;
			}
		}
	}

	if(other->client->sess.team != TEAM_SPECTATOR && level.time > other->client->sess.lastTele){
		G_PlayEffect ( G_EffectIndex("misc/electrical"),other->client->ps.origin, other->pos1);
		Henk_CloseSound(self->origin_to, G_SoundIndex("sound/misc/menus/apply_changes.wav"));
		Henk_CloseSound(self->origin_from, G_SoundIndex("sound/misc/menus/apply_changes.wav"));
	}
	TeleportPlayer( other, self->origin_to, self->angles_to, qfalse );
}

/*
==============================================================================

trigger_teleport

==============================================================================
*/

void trigger_teleporter_touch (gentity_t *self, gentity_t *other, trace_t *trace ) {
	gentity_t	*dest;
	char	*origin2;
	origin2 = va("%.0f %.0f %.0f", self->r.currentOrigin[0], self->r.currentOrigin[1], self->r.currentOrigin[2]+30);
	if ( !other->client ) {
		return;
	}
	if ( other->client->ps.pm_type == PM_DEAD ) {
		return;
	}
	// Spectators only?
	if ( ( self->spawnflags & 1 ) && !G_IsClientSpectating ( other->client ) ) 
	{
		return;
	}

	if(TeamCount1(TEAM_RED) < self->minimumhiders){
		if(level.time >= other->client->sess.lastmsg){
		trap_SendServerCommand(-1, va("print\"^3[Info] ^7There has to be a minimum of %i hiders to use this teleport.\n\"", self->minimumhiders));
		}
		other->client->sess.lastmsg = level.time+10000;
		return;
	}
	G_PlayEffect ( G_EffectIndex("misc/electrical"),other->client->ps.origin, other->pos1);
	//G_SpawnGEntityFromSpawnVars (qtrue);
	dest = 	G_PickTarget( self->target );
	if (!dest) {
		Com_Printf ("Couldn't find teleporter destination\n");
		return;
	}

	TeleportPlayer( other, dest->s.origin, dest->s.angles, qfalse );
}


/*QUAKED trigger_teleport (.5 .5 .5) ? SPECTATOR
Allows client side prediction of teleportation events.
Must point at a target_position, which will be the teleport destination.

If spectator is set, only spectators can use this teleport
Spectator teleporters are not normally placed in the editor, but are created
automatically near doors to allow spectators to move through them
*/
void SP_trigger_teleport( gentity_t *self ) {
	char			*origin;
	origin = va("%.0f %.0f %.0f", self->r.currentOrigin[0], self->r.currentOrigin[1], self->r.currentOrigin[2]-30);
	AddSpawnField("classname", "fx_play_effect");
	AddSpawnField("effect", "fire/blue_target_flame");
	AddSpawnField("origin", origin);
	AddSpawnField("angles", "0 90 0");
	AddSpawnField("count", "-1");
	G_SpawnGEntityFromSpawnVars (qtrue);
	InitTrigger (self);
	//G_PlayEffect ( 3,self->client->ps.origin, self->pos1);
	// unlike other triggers, we need to send this one to the client
	// unless is a spectator trigger
	if ( self->spawnflags & 1 ) {
		self->r.svFlags |= SVF_NOCLIENT;
	} else {
		self->r.svFlags &= ~SVF_NOCLIENT;
	}

	// make sure the client precaches this sound
	G_SoundIndex("sound/world/jumppad.wav");

	self->s.eType = ET_TELEPORT_TRIGGER;
	self->touch = trigger_teleporter_touch;

	trap_LinkEntity (self);
}


/*
==============================================================================

trigger_hurt

==============================================================================
*/

/*QUAKED trigger_hurt (.5 .5 .5) ? START_OFF - SILENT NO_PROTECTION SLOW
Any entity that touches this will be hurt.
It does dmg points of damage each server frame
Targeting the trigger will toggle its on / off state.

SILENT			supresses playing the sound
SLOW			changes the damage rate to once per second
NO_PROTECTION	*nothing* stops the damage

"dmg"			default 5 (whole numbers only)

*/
void hurt_use( gentity_t *self, gentity_t *other, gentity_t *activator ) {
	if ( self->r.linked ) {
		trap_UnlinkEntity( self );
	} else {
		trap_LinkEntity( self );
	}
}

void hurt_touch( gentity_t *self, gentity_t *other, trace_t *trace ) {
	int		dflags;

	if ( !other->takedamage ) {
		return;
	}

	if ( self->timestamp > level.time ) {
		return;
	}

	if ( self->spawnflags & 16 ) {
		self->timestamp = level.time + 1000;
	} else {
		self->timestamp = level.time + FRAMETIME;
	}

/*
	// play sound
	if ( !(self->spawnflags & 4) ) {
		G_Sound( other, CHAN_AUTO, self->noise_index );
	}
*/

	if (self->spawnflags & 8)
		dflags = DAMAGE_NO_PROTECTION;
	else
		dflags = 0;
	G_Damage (other, self, self, NULL, NULL, self->damage, dflags, MOD_TRIGGER_HURT, HL_NONE );
}

void SP_trigger_hurt( gentity_t *self ) {
	InitTrigger (self);

	self->noise_index = 0; // G_SoundIndex( "sound/world/electro.wav" );
	self->touch = hurt_touch;

	if ( !self->damage ) {
		self->damage = 5;
	}

	self->r.contents = CONTENTS_TRIGGER;

	if ( self->spawnflags & 2 ) {
		self->use = hurt_use;
	}

	// link in to the world if starting active
	if ( ! (self->spawnflags & 1) ) {
		trap_LinkEntity (self);
	}
}

/*QUAKED trigger_ladder (.5 .5 .5) ? 
Indicates a ladder and its normal

"angles"		angle ladder faces
*/
void SP_trigger_ladder ( gentity_t* self )
{
	vec3_t fwd;

	trap_LinkEntity ( self );

	trap_SetConfigstring( CS_LADDERS + level.ladderCount++, 
						  va("%i,%i,%i,%i,%i,%i,%i", 
							 (int)self->r.absmin[0],(int)self->r.absmin[1],(int)self->r.absmin[2],
							 (int)self->r.absmax[0],(int)self->r.absmax[1],(int)self->r.absmax[2],
							 (int)self->s.angles[YAW] ) );

	// Only need integer resolution
	self->r.absmin[0] = (int)self->r.absmin[0];
	self->r.absmin[1] = (int)self->r.absmin[1];
	self->r.absmin[2] = (int)self->r.absmin[2];
	self->r.absmax[0] = (int)self->r.absmax[0];
	self->r.absmax[1] = (int)self->r.absmax[1];
	self->r.absmax[2] = (int)self->r.absmax[2];
	
	self->s.angles[PITCH] = self->s.angles[ROLL] = 0;
	self->s.angles[YAW] = (int)self->s.angles[YAW];
	AngleVectors( self->s.angles, fwd, 0, 0);

	BG_AddLadder ( self->r.absmin, self->r.absmax, fwd );

	G_FreeEntity ( self );
}	

/*
==============================================================================

timer

==============================================================================
*/


/*QUAKED func_timer (0.3 0.1 0.6) (-8 -8 -8) (8 8 8) START_ON
This should be renamed trigger_timer...
Repeatedly fires its targets.
Can be turned on or off by using.

"wait"			base time between triggering all targets, default is 1
"random"		wait variance, default is 0
so, the basic time between firing is a random time between
(wait - random) and (wait + random)

*/
void func_timer_think( gentity_t *self ) {
	G_UseTargets (self, self->activator);
	// set time before next firing
	self->nextthink = level.time + 1000 * ( self->wait + crandom() * self->random );
}

void func_timer_use( gentity_t *self, gentity_t *other, gentity_t *activator ) {
	self->activator = activator;

	// if on, turn it off
	if ( self->nextthink ) {
		self->nextthink = 0;
		return;
	}

	// turn it on
	func_timer_think (self);
}

void SP_func_timer( gentity_t *self ) {
	G_SpawnFloat( "random", "1", &self->random);
	G_SpawnFloat( "wait", "1", &self->wait );

	self->use = func_timer_use;
	self->think = func_timer_think;

	if ( self->random >= self->wait ) {
		self->random = self->wait - FRAMETIME;
		Com_Printf( "func_timer at %s has random >= wait\n", vtos( self->s.origin ) );
	}

	if ( self->spawnflags & 1 ) {
		self->nextthink = level.time + FRAMETIME;
		self->activator = self;
	}

	self->r.svFlags = SVF_NOCLIENT;
}

/* 
================
NV_blocked_trigger
RxCxW - 10.01.06
================
*/
void NV_blocked_trigger	(gentity_t *ent)
{
	ent->r.contents = MASK_SHOT;
	ent->r.svFlags = SVF_NOCLIENT;
	ent->s.eType =	ET_TERRAIN;	
	
	if ( ent->damage ) {
		ent->noise_index = 0;
		ent->touch = hurt_touch;
		ent->use = hurt_use;
		ent->methodOfDeath = MOD_TRIGGER_HURT;
	}
	trap_LinkEntity (ent);
}
/* 
================
NV_blocked_Teleport
RxCxW - 10.01.06
================
*/
void NV_blocked_Teleport	(gentity_t *ent)
{
	ent->r.contents = CONTENTS_PLAYERCLIP;
	ent->r.svFlags = SVF_NOCLIENT;
	ent->s.eType = ET_TELEPORT_TRIGGER;

	ent->touch = trigger_teleporter_touch;
	trap_LinkEntity (ent);
}

void hideseek_cage(gentity_t *ent){
	VectorCopy(ent->s.origin, level.hideseek_cage);
	G_FreeEntity(ent);
	level.cagefightloaded = qtrue;
}

// Henk 13/02/11
void SP_teleporter(gentity_t* ent){
	char		*origin;
	trace_t		tr;
	vec3_t		dest;
	vec3_t		src;
	vec3_t		origin1;

	if(strstr(ent->both_sides, "yes")){
		AddSpawnField("classname", "teleporter");
		origin = va("%.0f %.0f %.0f", ent->origin_to[0], ent->origin_to[1], ent->origin_to[2]);
		AddSpawnField("origin_from", origin);
		origin = va("%.0f %.0f %.0f", ent->origin_from[0], ent->origin_from[1], ent->origin_from[2]);
		AddSpawnField("origin_to", origin);
		origin = va("%.0f %.0f %.0f", ent->angles_to[0], ent->angles_to[1], ent->angles_to[2]);
		AddSpawnField("angles_from", origin);
		origin = va("%.0f %.0f %.0f", ent->angles_from[0], ent->angles_from[1], ent->angles_from[2]);
		AddSpawnField("angles_to", origin);
		AddSpawnField("team", va("%s", ent->team));
		AddSpawnField("min_players", va("%i", ent->min_players));
		AddSpawnField("max_players",va("%i", ent->max_players));
		AddSpawnField("both_sides", "no");
		G_SpawnGEntityFromSpawnVars (qtrue);
		/*
		"classname" "teleporter"
		"origin_from" "1822 230 106"
		"origin_to" "-128 -90 108"
		"angles_from" "0 1 0"
		"angles_to" "0 1 0"
		"both_sides" "yes"
		"team" "red"
		"max_players" "0"
		"min_players" "0"
		*/
	}
	VectorSet( ent->r.mins, -ITEM_RADIUS, -ITEM_RADIUS, -ITEM_RADIUS );
	VectorSet( ent->r.maxs, ITEM_RADIUS, ITEM_RADIUS, ITEM_RADIUS );
	VectorSet( src, ent->origin_from[0], ent->origin_from[1], ent->origin_from[2] + 1 );
	VectorSet( dest, ent->origin_from[0], ent->origin_from[1], ent->origin_from[2] - 4096 );
	trap_Trace( &tr, src, ent->r.mins, ent->r.maxs, dest, ent->s.number, MASK_SOLID );

	if ( tr.startsolid ) 
	{
		Com_Printf ("Teleporter: %s startsolid at %s\n", ent->classname, vtos(ent->origin_from));
		G_FreeEntity( ent );
		return;
	}
	ent->s.groundEntityNum = tr.entityNum;
	VectorCopy(tr.endpos, ent->origin_from);
	VectorCopy(tr.endpos, origin1);
	origin1[2] += 30;
	G_SetOrigin( ent,origin1 );
	// origin_to until ground
	
	//origin = va("%.0f %.0f %.0f", ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2]-30);
	origin = va("%.0f %.0f %.0f", ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2]-30);
	AddSpawnField("classname", "fx_play_effect");
	AddSpawnField("effect", "fire/blue_target_flame");
	AddSpawnField("origin", origin);
	AddSpawnField("angles", "0 90 0");
	AddSpawnField("count", "-1");
	ent->effect_index = G_SpawnGEntityFromSpawnVars (qtrue);
	ent->r.contents = CONTENTS_TRIGGER;
	ent->r.svFlags &= ~SVF_NOCLIENT;

	// make sure the client precaches this sound
	G_SoundIndex("sound/world/jumppad.wav");

	ent->s.eType = ET_TELEPORT_TRIGGER;
	ent->touch = trigger_NewTeleporter_touch;

	trap_LinkEntity (ent);
}

// Henk 10/02/11
void SP_booster(gentity_t* ent){
	char			*origin;
	trace_t		tr;
	vec3_t		dest;
	vec3_t		src;

	VectorSet( ent->r.mins, -ITEM_RADIUS, -ITEM_RADIUS, -ITEM_RADIUS );
	VectorSet( ent->r.maxs, ITEM_RADIUS, ITEM_RADIUS, ITEM_RADIUS );
	VectorSet( src, ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] + 1 );
	VectorSet( dest, ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] - 4096 );
	trap_Trace( &tr, src, ent->r.mins, ent->r.maxs, dest, ent->s.number, MASK_SOLID );
	if ( tr.startsolid ) 
	{
		Com_Printf ("Booster: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
		G_FreeEntity( ent );
		return;
	}
	ent->s.groundEntityNum = tr.entityNum;
	G_SetOrigin( ent, tr.endpos );
	origin = va("%.0f %.0f %.0f", ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2]-15);
	AddSpawnField("classname", "func_rotating");
	AddSpawnField("model", "none");
	AddSpawnField("model2", "models/objects/Armory/virus.md3");
	AddSpawnField("speed", "100");
	AddSpawnField("dmg", "0");
	AddSpawnField("origin", origin);
	AddSpawnField("angles", "0 90 0");
	AddSpawnField("count", "-1");
	G_SpawnGEntityFromSpawnVars (qtrue);
	origin = va("%.0f %.0f %.0f", ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2]+15);
	AddSpawnField("classname", "fx_play_effect");
	AddSpawnField("effect", "gen_tendril1");
	AddSpawnField("origin", origin);
	AddSpawnField("count", "-1");

	G_SpawnGEntityFromSpawnVars (qtrue);
	G_SetMovedir (ent->s.angles, ent->s.origin2);
	VectorScale (ent->s.origin2, ent->forward, ent->s.origin2);
	ent->r.contents = CONTENTS_TRIGGER;
	ent->r.svFlags &= ~SVF_NOCLIENT;
	ent->s.eType = ET_PUSH_TRIGGER;
	ent->touch = trigger_booster_touch;
	trap_LinkEntity (ent);
}