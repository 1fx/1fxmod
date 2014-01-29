// Copyright (C) 2001-2002 Raven Software
//
// g_active.c --

#include "g_local.h"
#include "boe_local.h"

//RxCxW - 05-09-05 #SpawnPoint
gspawn_t	*G_SelectClientSpawnPoint ( gentity_t* ent,  qboolean plantsk );
//End
void P_SetTwitchInfo(gclient_t	*client)
{
	client->ps.painTime = level.time;
	client->ps.painDirection ^= 1;
}
/*
===============
G_DamageFeedback

Called just before a snapshot is sent to the given player.
Totals up all damage and generates both the player_state_t
damage values to that client for pain blends and kicks, and
global pain sound events for all clients.
===============
*/
void P_DamageFeedback( gentity_t *player ) 
{
	gclient_t	*client;
	float		count;
	vec3_t		angles;

	client = player->client;
	if ( client->ps.pm_type == PM_DEAD ) 
	{
		return;
	}

	// total points of damage shot at the player this frame
	count = client->damage_blood + client->damage_armor;
	if ( count == 0 ) 
	{
		// didn't take any damage
		return;		
	}

	if ( count > 255 ) 
	{
		count = 255;
	}

	// send the information to the client

	// world damage (falling, slime, etc) uses a special code
	// to make the blend blob centered instead of positional
	if ( client->damage_fromWorld ) 
	{
		client->ps.damagePitch = 255;
		client->ps.damageYaw = 255;

		client->damage_fromWorld = qfalse;
	} 
	else 
	{
		vectoangles( client->damage_from, angles );
		client->ps.damagePitch = angles[PITCH]/360.0 * 255;
		client->ps.damageYaw = angles[YAW]/360.0 * 255;
	}

	// play an apropriate pain sound
	if ( (level.time > player->pain_debounce_time)) 
	{
		// don't do more than two pain sounds a second
		if ( level.time - client->ps.painTime < 500 ) 
		{
			return;
		}
		

		P_SetTwitchInfo(client);
		player->pain_debounce_time = level.time + 700;
		G_AddEvent( player, EV_PAIN, player->health );
		client->ps.damageEvent++;
	}


	client->ps.damageCount = count;

	// clear totals
	client->damage_blood = 0;
	client->damage_armor = 0;
	client->damage_knockback = 0;
}

/*
=============
P_WorldEffects

Check for drowning
=============
*/
void P_WorldEffects( gentity_t *ent ) 
{
	int	 waterlevel;

	if ( ent->client->noclip ) 
	{
		// don't need air
		ent->client->airOutTime = level.time + 12000;	
		return;
	}

	waterlevel = ent->waterlevel;
	if(current_gametype.value == GT_HS){ // Henk 13/01/11 -> Slow when walking in the water
		if(waterlevel >= 1)
			ent->client->sess.slowtime = level.time+200;
	}

	// Disable Lower.
	if(level.noLRActive[0] && level.noLRMWEntFound[0]){ // if enabled -- Boe!Man 6/2/12: Also check for nolower2. This is qtrue when the entity was found.
		if(ent->r.currentOrigin[2] <= level.noLR[0][2] && !G_IsClientDead(ent->client)){
			//trap_SendServerCommand( ent->s.number, va("print \"nolower = %.2f.\n\"", level.nolower[2]) );
			trap_SendServerCommand(-1, va("print\"^3[Info] ^7%s ^7was killed for being lower.\n\"", ent->client->pers.cleanName));
			G_Damage(ent, NULL, NULL, NULL, NULL, 10000, 0, MOD_TRIGGER_HURT, 0);
		}
	}
	
	// Boe!Man 6/3/12: Check for roof. This is best done in a seperate function.
	Boe_checkRoof(ent);

	// check for drowning
	if ( waterlevel == 3 && (ent->watertype & CONTENTS_WATER)) 
	{
		// if out of air, start drowning
		if ( ent->client->airOutTime < level.time) 
		{
			// drown!
			ent->client->airOutTime += 1000;
			if ( ent->health > 0 ) 
			{
				// take more damage the longer underwater
				ent->damage += 2;
				if (ent->damage > 15)
				{
					ent->damage = 15;
				}

				// play a gurp sound instead of a normal pain sound
				if (ent->health <= ent->damage) 
				{
//					G_Sound(ent, CHAN_VOICE, G_SoundIndex("sound/pain_death/mullins/drown_dead.wav"));
				} 
				else
				{
					G_AddEvent ( ent, EV_PAIN_WATER, 0 );
				}

				// don't play a normal pain sound
				ent->pain_debounce_time = level.time + 200;

				G_Damage (ent, NULL, NULL, NULL, NULL, ent->damage, DAMAGE_NO_ARMOR, MOD_WATER, HL_NONE );
			}
		}
	} 
	else 
	{
		ent->client->airOutTime = level.time + 12000;
		ent->damage = 2;
	}
}

/*
===============
G_SetClientSound
===============
*/
void G_SetClientSound( gentity_t *ent ) 
{
	ent->client->ps.loopSound = 0;
}

/*
==============
ClientImpacts
==============
*/
void ClientImpacts( gentity_t *ent, pmove_t *pm ) {
	int		i, j;
	trace_t	trace;
	gentity_t	*other;

	memset( &trace, 0, sizeof( trace ) );
	for (i=0 ; i<pm->numtouch ; i++) {
		for (j=0 ; j<i ; j++) {
			if (pm->touchents[j] == pm->touchents[i] ) {
				break;
			}
		}
		if (j != i) {
			continue;	// duplicated
		}
		other = &g_entities[ pm->touchents[i] ];

		if ( ( ent->r.svFlags & SVF_BOT ) && ( ent->touch ) ) {
			ent->touch( ent, other, &trace );
		}

		if ( !other->touch ) {
			continue;
		}

		other->touch( other, ent, &trace );
	}

}

/*
============
G_IsClientSiameseTwin

Checks to see if the two clients should never have been separated at birth
============
*/
static qboolean G_IsClientSiameseTwin ( gentity_t* ent, gentity_t* ent2 )
{
	if ( G_IsClientSpectating ( ent->client ) || G_IsClientDead ( ent->client ) )
	{
		return qfalse;
	}

	if ( G_IsClientSpectating ( ent2->client ) || G_IsClientDead ( ent2->client ) )
	{
		return qfalse;
	}

	if (ent2->r.currentOrigin[0] + ent2->r.mins[0] > ent->r.currentOrigin[0] + ent->r.maxs[0])
	{
		return qfalse;
	}

	if (ent2->r.currentOrigin[1] + ent2->r.mins[1] > ent->r.currentOrigin[1] + ent->r.maxs[1])
	{
		return qfalse;
	}

	if (ent2->r.currentOrigin[2] + ent2->r.mins[2] > ent->r.currentOrigin[2] + ent->r.maxs[2])
	{
		return qfalse;
	}

	if (ent2->r.currentOrigin[0] + ent2->r.maxs[0] < ent->r.currentOrigin[0] + ent->r.mins[0])
	{
		return qfalse;
	}

	if (ent2->r.currentOrigin[1] + ent2->r.maxs[1] < ent->r.currentOrigin[1] + ent->r.mins[1])
	{
		return qfalse;
	}

	if (ent2->r.currentOrigin[2] + ent2->r.maxs[2] < ent->r.currentOrigin[2] + ent->r.mins[2])
	{
		return qfalse;
	}

	return qtrue;
}

/*
============
G_TouchTriggers

Find all trigger entities that ent's current position touches.
Spectators will only interact with teleporters.
============
*/
void G_TouchTriggers( gentity_t *ent ) 
{
	int				i;
	int				num;
	int				touch[MAX_GENTITIES];
	gentity_t		*hit;
	trace_t			trace;
	vec3_t			mins;
	vec3_t			maxs;
	static vec3_t	range = { 20, 20, 40 };

	if ( !ent->client ) 
	{
		return;
	}

	// dead clients don't activate triggers!
	if ( G_IsClientDead ( ent->client ) ) 
	{
		return;
	}

	VectorSubtract( ent->client->ps.origin, range, mins );
	VectorAdd( ent->client->ps.origin, range, maxs );

	num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	// can't use ent->r.absmin, because that has a one unit pad
	VectorAdd( ent->client->ps.origin, ent->r.mins, mins );
	VectorAdd( ent->client->ps.origin, ent->r.maxs, maxs );

	// Reset the players can use flag
	ent->client->ps.pm_flags &= ~(PMF_CAN_USE);
	ent->s.modelindex  = 0;

	for ( i=0 ; i<num ; i++ ) 
	{
		hit = &g_entities[touch[i]];

		// pmove would have to have detected siamese twins first
		if ( hit->client && hit != ent && !hit->client->siameseTwin && (ent->client->ps.pm_flags & PMF_SIAMESETWINS) )
		{
			// See if this client has a twin
			if ( !G_IsClientSiameseTwin ( ent, hit ) )
			{
				continue;
			}

			// About time these twins were separated!!
			ent->client->siameseTwin = hit;
			hit->client->siameseTwin = ent;
		}

		if ( !( hit->r.contents & CONTENTS_TRIGGER ) ) 
		{
			continue;
		}

		if ( !hit->touch && !ent->touch ) 
		{
			continue;
		}

		// ignore most entities if a spectator
		if ( G_IsClientSpectating ( ent->client ) ) 
		{
			if ( hit->s.eType != ET_TELEPORT_TRIGGER &&
				// this is ugly but adding a new ET_? type will
				// most likely cause network incompatibilities
				hit->touch != Touch_DoorTrigger) 
			{
				continue;
			}
		}

		// use seperate code for determining if an item is picked up
		// so you don't have to actually contact its bounding box
		if ( hit->s.eType == ET_ITEM ) 
		{
			if ( !BG_PlayerTouchesItem( &ent->client->ps, &hit->s, level.time ) ) 
			{
				continue;
			}
		} 
		else 
		{
			if ( !trap_EntityContact( mins, maxs, hit ) ) 
			{
				continue;
			}
		}

		memset( &trace, 0, sizeof(trace) );

		if ( hit->touch ) 
		{
			hit->touch (hit, ent, &trace);
		}

		if ( ( ent->r.svFlags & SVF_BOT ) && ( ent->touch ) ) 
		{
			ent->touch( ent, hit, &trace );
		}
	}

	// Dont bother looking for twins again unless pmove says so
	ent->client->ps.pm_flags &= (~PMF_SIAMESETWINS);
}


/*
============
G_MoverTouchTriggers

Find all trigger entities that ent's current position touches.
Spectators will only interact with teleporters.
============
*/
void G_MoverTouchPushTriggers( gentity_t *ent, vec3_t oldOrg ) 
{
	int			i, num;
	float		step, stepSize, dist;
	int			touch[MAX_GENTITIES];
	gentity_t	*hit;
	trace_t		trace;
	vec3_t		mins, maxs, dir, size, checkSpot;
	const vec3_t	range = { 40, 40, 52 };

	// non-moving movers don't hit triggers!
	if ( !VectorLengthSquared( ent->s.pos.trDelta ) ) 
	{
		return;
	}

	VectorSubtract( ent->r.mins, ent->r.maxs, size );
	stepSize = VectorLength( size );
	if ( stepSize < 1 )
	{
		stepSize = 1;
	}

	VectorSubtract( ent->r.currentOrigin, oldOrg, dir );
	dist = VectorNormalize( dir );
	for ( step = 0; step <= dist; step += stepSize )
	{
		VectorMA( ent->r.currentOrigin, step, dir, checkSpot );
		VectorSubtract( checkSpot, range, mins );
		VectorAdd( checkSpot, range, maxs );

		num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

		// can't use ent->r.absmin, because that has a one unit pad
		VectorAdd( checkSpot, ent->r.mins, mins );
		VectorAdd( checkSpot, ent->r.maxs, maxs );

		for ( i=0 ; i<num ; i++ ) 
		{
			hit = &g_entities[touch[i]];

			if ( hit->s.eType != ET_PUSH_TRIGGER )
			{
				continue;
			}

			if ( hit->touch == NULL ) 
			{
				continue;
			}

			if ( !( hit->r.contents & CONTENTS_TRIGGER ) ) 
			{
				continue;
			}


			if ( !trap_EntityContact( mins, maxs, hit ) ) 
			{
				continue;
			}

			memset( &trace, 0, sizeof(trace) );

			if ( hit->touch != NULL ) 
			{
				hit->touch(hit, ent, &trace);
			}
		}
	}
}

/*
=================
G_UpdatePlayerStateScores

Update the scores in the playerstate
=================
*/
void G_UpdatePlayerStateScores ( gentity_t* ent )
{
	// set the CS_SCORES1/2 configstrings, which will be visible to everyone
	if ( level.gametypeData->teams ) 
	{
		ent->client->ps.persistant[PERS_RED_SCORE] = level.teamScores[TEAM_RED];
		ent->client->ps.persistant[PERS_BLUE_SCORE] = level.teamScores[TEAM_BLUE];
		
		//Ryan & Dragon
		ent->client->ps.persistant[PERS_BLUE_ALIVE_COUNT] = level.teamAliveCount[TEAM_BLUE];
		ent->client->ps.persistant[PERS_RED_ALIVE_COUNT] = level.teamAliveCount[TEAM_RED];
		//Ryan & Dragon
	} 
	else 
	{
		if ( level.numConnectedClients == 0 ) 
		{
			ent->client->ps.persistant[PERS_RED_SCORE] = 0;
			ent->client->ps.persistant[PERS_BLUE_SCORE] = 0;
		} 
		else if ( level.numConnectedClients == 1 ) 
		{
			ent->client->ps.persistant[PERS_RED_SCORE] = level.clients[ level.sortedClients[0] ].ps.persistant[PERS_SCORE];
			ent->client->ps.persistant[PERS_BLUE_SCORE] = 0;
		} 
		else 
		{
			ent->client->ps.persistant[PERS_RED_SCORE] = level.clients[ level.sortedClients[0] ].ps.persistant[PERS_SCORE];
			ent->client->ps.persistant[PERS_BLUE_SCORE] = level.clients[ level.sortedClients[1] ].ps.persistant[PERS_SCORE];
		}
	}
}

/*
=================
SpectatorThink
=================
*/
void SpectatorThink( gentity_t *ent, usercmd_t *ucmd ) 
{
	pmove_t		pm;
	gclient_t	*client;
	//Ryan
	int			counts[TEAM_NUM_TEAMS];
	//Ryan

	client = ent->client;

	if ( client->sess.spectatorState != SPECTATOR_FOLLOW ) 
	{
		client->ps.pm_type = PM_SPECTATOR;

		//Ryan
		//Check if we have to lock this spectator
		if(level.gametypeData->teams && level.specsLocked && client->sess.team == TEAM_SPECTATOR)
		{
			counts[TEAM_BLUE] = TeamCount( -1, TEAM_BLUE, NULL );
			counts[TEAM_RED] = TeamCount( -1, TEAM_RED, NULL );

			if(client->sess.invitedByRed && counts[TEAM_RED] < 1)
			{
				client->sess.invitedByRed = qfalse;
			}

			if(client->sess.invitedByBlue && counts[TEAM_BLUE] < 1)
			{
				client->sess.invitedByBlue = qfalse;
			}

			if((counts[TEAM_RED] > 0) || (counts[TEAM_BLUE] > 0) )
			{
				if(!client->sess.adminspec && !client->sess.referee && !client->sess.invitedByRed && !client->sess.invitedByBlue) 
				{
					vec3_t lookdown;
					VectorSet(lookdown, 90, 0, 0);
					SetClientViewAngle(ent, lookdown, qfalse);
					client->ps.pm_type = PM_FREEZE;
				}
				
			}
		}

		client->ps.speed = 400;	// faster than normal

		// set up for pmove
		memset (&pm, 0, sizeof(pm));
		pm.ps = &client->ps;
		pm.cmd = *ucmd;
		pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;	// spectators can fly through bodies
		pm.trace = trap_Trace;
		pm.pointcontents = trap_PointContents;

		pm.animations = NULL;

		// perform a pmove
		Pmove (&pm);

		G_UpdatePlayerStateScores ( ent );

		// save results of pmove
		VectorCopy( client->ps.origin, ent->s.origin );

		G_TouchTriggers( ent );
		trap_UnlinkEntity( ent );
	}
	
	//Ryan
	else if(level.gametypeData->teams && level.specsLocked && client->sess.team == TEAM_SPECTATOR) 
	{
		if(!client->sess.invitedByBlue && !client->sess.invitedByRed && !client->sess.adminspec && !client->sess.referee)
		{
			G_StopFollowing( ent );
			return;
		}
	}
	//Ryan

	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;

	// attack button cycles through spectators

	//Ryan june 7 2003		admins in free spec mode are not forced to follow
		//if ( client->sess.spectatorState != SPECTATOR_FOLLOW && g_forceFollow.integer )
	if ( client->ps.pm_type == PM_SPECTATOR && !client->sess.adminspec && client->sess.spectatorState != SPECTATOR_FOLLOW && g_forceFollow.integer )
	//Ryan
	{
		Cmd_FollowCycle_f( ent, 1 );
	}

	//Ryan
	else if ( client->ps.pm_type == PM_SPECTATOR && !client->sess.adminspec && client->sess.spectatorState != SPECTATOR_FOLLOW && g_compMode.integer )
	{
		//always force follow in competiton mode
		Cmd_FollowCycle_f( ent, 1 ); 
	}
	//Ryan

	if ( ( client->buttons & BUTTON_ATTACK ) && ! ( client->oldbuttons & BUTTON_ATTACK ) ) 
	{
		if(client->sess.team == TEAM_SPECTATOR && level.specsLocked){
			if(g_compMode.integer && (client->sess.invitedByBlue || client->sess.invitedByRed)){ // Fix for not able to switch
				Cmd_FollowCycle_f( ent, 1 );
			}
		}else{
		Cmd_FollowCycle_f( ent, 1 );
		}
	}
	else if ( ( client->buttons & BUTTON_ALT_ATTACK ) && ! ( client->oldbuttons & BUTTON_ALT_ATTACK ) ) 
	{
		Cmd_FollowCycle_f( ent, -1 );
	}
	//Ryan, again if in compmode - no free floating
	//else if ( !g_forceFollow.integer && ucmd->upmove > 0 && (client->ps.pm_flags & PMF_FOLLOW) )
	else if ( !g_compMode.integer && !g_forceFollow.integer && ucmd->upmove > 0 && (client->ps.pm_flags & PMF_FOLLOW) )
	//Ryan
	{
		G_StopFollowing( ent );
	}

	if (g_compMode.integer){
		if(client->sess.spectating >= 0 && client->sess.spectating <= 64){
			if(level.clients[client->sess.spectating].sess.thirdperson == 1){
				client->sess.spectatorFirstPerson = qfalse;
			}else if(level.clients[client->sess.spectating].sess.thirdperson == 0){
				client->sess.spectatorFirstPerson = qtrue;
			}else{
				client->sess.spectatorFirstPerson = qtrue;
			}
		}
	}

	//Ryan june 7 2003
	else if (client->sess.adminspec && ucmd->upmove > 0 && (client->ps.pm_flags & PMF_FOLLOW) )
	{
		G_StopFollowing( ent );
	}

	else if ( client->sess.rpmClient && (((client->buttons & BUTTON_ZOOMIN) && !( client->oldbuttons & BUTTON_ZOOMIN ))  || ((client->buttons & BUTTON_RELOAD) && !( client->oldbuttons & BUTTON_RELOAD )))) {
		// If not following then go to either third or first
		if ( client->sess.spectatorState != SPECTATOR_FOLLOW ) {
			client->sess.spectatorFirstPerson = qtrue;
			Cmd_FollowCycle_f( ent, -1 );
		}
		// If in first person then either go to free float or third person
		else if ( client->sess.spectatorFirstPerson ) {
			client->sess.spectatorFirstPerson = qfalse;	
		}
		// Must be in third person so just go to first
		else {
			client->sess.spectatorFirstPerson = qtrue;
		}
	}
	//End
}	

/*
=================
ClientInactivityTimer

Returns qfalse if the client is dropped
=================
*/
qboolean ClientInactivityTimer( gclient_t *client ) {
	if ( ! g_inactivity.integer ) {
		// give everyone some time, so if the operator sets g_inactivity during
		// gameplay, everyone isn't kicked
		client->inactivityTime = level.time + 60 * 1000;
		client->inactivityWarning = qfalse;	
	}

	//Ryan, fixed so if we pause then unpause the incativity doesnt
	//drop everyone, also admins can't be dropped
	else if ( level.pause ||
		//client->sess.admin || // Boe!Man 4/26/12: No-one can think of a situation where Admins shouldn't be forceteamed, so I changed this.
		client->pers.cmd.forwardmove || 
		client->pers.cmd.rightmove || 
		client->pers.cmd.upmove ||
		(client->pers.cmd.buttons & (BUTTON_ATTACK|BUTTON_ALT_ATTACK)) ) {
		client->inactivityTime = level.time + g_inactivity.integer * 1000;
		client->inactivityWarning = qfalse;
	/*else if ( client->pers.cmd.forwardmove || 
		client->pers.cmd.rightmove || 
		client->pers.cmd.upmove ||
		(client->pers.cmd.buttons & (BUTTON_ATTACK|BUTTON_ALT_ATTACK)) ) {
		client->inactivityTime = level.time + g_inactivity.integer * 1000;
		client->inactivityWarning = qfalse;*/ 
	//Ryan

	} else if ( !client->pers.localClient) {
		#ifdef _SOF2_BOTS
		if (g_entities[client-level.clients].r.svFlags & SVF_BOT)// Henk 13/01/11 -> Fixed bots getting forceteamed(Issue #65)
			return qfalse;
		#endif
		if ( level.time > client->inactivityTime ) {
			SetTeam(&g_entities[client-level.clients], "s", NULL, qtrue); // Henk 08/04/10 -> Force ppl to spec instead of kicking them when afk
			trap_SendServerCommand(-1, va("print\"^3[Info] ^7%s ^7was forced to spectator for being AFK.\n\"", g_entities[client-level.clients].client->pers.netname));
			//trap_DropClient( client - level.clients, "Dropped due to inactivity" );
			return qfalse;
		}
		if ( level.time > client->inactivityTime - 10000 && !client->inactivityWarning ) {
			client->inactivityWarning = qtrue;
			trap_SendServerCommand( client - level.clients, "cp \"Ten seconds until inactivity drop!\n\"" );
		}
	}
	return qtrue;
}

/*
==================
ClientTimerActions

Actions that happen once a second
==================
*/
void ClientTimerActions( gentity_t *ent, int msec ) 
{
	gclient_t	*client;

	client = ent->client;
	// Check so see if the player has moved and if so dont let them change their outfitting
	if ( !client->noOutfittingChange && ((level.time - client->respawnTime) > 3000))
	{
		vec3_t vel;

		// Check the horizontal velocity for movement
		VectorCopy ( client->ps.velocity, vel );
		vel[2] = 0;
		if ( VectorLengthSquared ( vel ) > 100 )
		{
			client->noOutfittingChange = qtrue;
		}
	}

	// Forgive voice chats
	if ( g_voiceFloodCount.integer && ent->client->sess.voiceFloodCount )
	{
		int forgiveTime = 60000 / g_voiceFloodCount.integer;

		client->sess.voiceFloodTimer += msec;
		while ( client->sess.voiceFloodTimer >= forgiveTime ) 
		{
			// Forgive one voice chat
			client->sess.voiceFloodCount--;

			client->sess.voiceFloodTimer -= forgiveTime;
		}
	}
}

/*
====================
ClientIntermissionThink
====================
*/
void ClientIntermissionThink( gclient_t *client ) 
{
	client->ps.eFlags &= ~EF_TALK;
	client->ps.eFlags &= ~EF_FIRING;

	// the level will exit when everyone wants to or after timeouts

	// swap and latch button actions
	client->oldbuttons = client->buttons;
	client->buttons = client->pers.cmd.buttons;

	if ( (client->buttons & BUTTON_ATTACK) & ( client->oldbuttons ^ client->buttons ) ) 
	{
		// this used to be an ^1 but once a player says ready, it should stick
		client->readyToExit = qtrue;
	}
}

/*
====================
G_Use

use key pressed
====================
*/
void G_Use ( gentity_t* ent )
{
	int				i;
	int				num;
	int				touch[MAX_GENTITIES];
	gentity_t		*hit;
	vec3_t			mins;
	vec3_t			maxs;
	static vec3_t	range = { 20, 20, 40 };

	if ( !ent->client ) 
	{
		return;
	}

	// dead clients don't activate triggers!
	if ( ent->client->ps.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	VectorSubtract( ent->client->ps.origin, range, mins );
	VectorAdd( ent->client->ps.origin, range, maxs );

	num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	// can't use ent->r.absmin, because that has a one unit pad
	VectorAdd( ent->client->ps.origin, ent->r.mins, mins );
	VectorAdd( ent->client->ps.origin, ent->r.maxs, maxs );

	for ( i=0 ; i<num ; i++ ) 
	{
		hit = &g_entities[touch[i]];

		if ( !hit->use ) 
		{
			continue;
		}

		// Misstion triggers can be used
		if ( !Q_stricmp ( hit->classname, "gametype_trigger" ) )
		{
			hit->use ( hit, ent, ent );
		}
	}
}

/*
================
ClientEvents

Events will be passed on to the clients for presentation,
but any server game effects are handled here
================
*/
void ClientEvents( gentity_t *ent, int oldEventSequence ) 
{
	int			i;
	int			event;
	gclient_t	*client;
	vec3_t		dir;

	client = ent->client;

	if ( oldEventSequence < client->ps.eventSequence - MAX_PS_EVENTS ) 
	{
		oldEventSequence = client->ps.eventSequence - MAX_PS_EVENTS;
	}

	for ( i = oldEventSequence ; i < client->ps.eventSequence ; i++ ) 
	{
		event = client->ps.events[ i & (MAX_PS_EVENTS-1) ];

		switch ( event ) 
		{
			case EV_FALL_MEDIUM:
			case EV_FALL_FAR:
			{
				int damage;
				
				damage  = client->ps.eventParms[ i & (MAX_PS_EVENTS-1) ];
				damage &= 0x000000ff;

				client->ps.eventParms[ i & (MAX_PS_EVENTS-1) ] = damage;
							
				if ( ent->s.eType != ET_PLAYER ) 
				{
					break;		// not in the player model
				}
				
				if ( (g_dmflags.integer & DF_NO_FALLING)) 
				{
					break;
				}		
			
				VectorSet (dir, 0, 0, 1);
				ent->pain_debounce_time = level.time + 200;	// no normal pain sound
				G_Damage (ent, NULL, NULL, NULL, NULL, damage, DAMAGE_NO_ARMOR, MOD_FALLING, HL_NONE );
				break;
			}

			case EV_FIRE_WEAPON:
				ent->client->noOutfittingChange = qtrue;
				ent->client->invulnerableTime = 0;
				G_FireWeapon( ent, ATTACK_NORMAL );
				break;

			case EV_ALT_FIRE:
				ent->client->noOutfittingChange = qtrue;
				ent->client->invulnerableTime = 0;
				G_FireWeapon( ent, ATTACK_ALTERNATE );
				break;

			case EV_USE:
				G_Use ( ent );
				break;

			default:
				break;
		}
	}

}

/*
==============
StuckInOtherClient
==============
*/
static int StuckInOtherClient(gentity_t *ent) 
{
	int i;
	gentity_t	*ent2;

	ent2 = &g_entities[0];
	for ( i = 0; i < MAX_CLIENTS; i++, ent2++ ) 
	{
		if ( ent2 == ent ) 
		{
			continue;
		}

		if ( !ent2->inuse ) 
		{
			continue;
		}
		
		if ( !ent2->client ) 
		{
			continue;
		}
		
		if ( ent2->health <= 0 ) 
		{
			continue;
		}
		
		//
		if (ent2->r.absmin[0] > ent->r.absmax[0])
			continue;	
		if (ent2->r.absmin[1] > ent->r.absmax[1])
			continue;
		if (ent2->r.absmin[2] > ent->r.absmax[2])
			continue;
		if (ent2->r.absmax[0] < ent->r.absmin[0])
			continue;
		if (ent2->r.absmax[1] < ent->r.absmin[1])
			continue;
		if (ent2->r.absmax[2] < ent->r.absmin[2])
			continue;
		return qtrue;
	}
	return qfalse;
}

void BotTestSolid(vec3_t origin);

/*
==============
SendPendingPredictableEvents
==============
*/
void SendPendingPredictableEvents( playerState_t *ps ) {
	gentity_t *t;

	int event, seq;
	int extEvent, number;

	// if there are still events pending
	if ( ps->entityEventSequence < ps->eventSequence ) {
		// create a temporary entity for this event which is sent to everyone
		// except the client who generated the event
		seq = ps->entityEventSequence & (MAX_PS_EVENTS-1);
		event = ps->events[ seq ] | ( ( ps->entityEventSequence & 3 ) << 8 );
		// set external event to zero before calling BG_PlayerStateToEntityState
		extEvent = ps->externalEvent;
		ps->externalEvent = 0;
		// create temporary entity for event
		t = G_TempEntity( ps->origin, event );
		number = t->s.number;
		BG_PlayerStateToEntityState( ps, &t->s, qtrue );
		t->s.number = number;
		t->s.eType = ET_EVENTS + event;
		t->s.eFlags |= EF_PLAYER_EVENT;
		t->s.otherEntityNum = ps->clientNum;
		// send to everyone except the client who generated the event
		t->r.svFlags |= SVF_NOTSINGLECLIENT;
		t->r.singleClient = ps->clientNum;
		// set back external event
		ps->externalEvent = extEvent;
	}
}

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame on fast clients.

If "g_synchronousClients 1" is set, this will be called exactly
once for each server frame, which makes for smooth demo recording.
==============
*/
void ClientThink_real( gentity_t *ent ) 
{
	gclient_t	*client;
	pmove_t		pm;
	int			oldEventSequence;
	int			msec;
	usercmd_t	*ucmd;

	///RxCxW - 2.04.05 - 05:04am
	gspawn_t	*G_SelectClientSpawnPoint ( gentity_t* ent,  qboolean plantsk );

	vec3_t	dir, fireAngs;
	
	client = ent->client;

	// don't think if the client is not yet connected (and thus not yet spawned in)
	if (client->pers.connected != CON_CONNECTED) 
	{
		return;
	}

	// mark the time, so the connection sprite can be removed
	ucmd = &ent->client->pers.cmd;

	// sanity check the command time to prevent speedup cheating
	if ( ucmd->serverTime > level.time + 200 ) 
	{
		ucmd->serverTime = level.time + 200;
	}
	
	if ( ucmd->serverTime < level.time - 1000 ) 
	{
		ucmd->serverTime = level.time - 1000;
	} 
	
	msec = ucmd->serverTime - client->ps.commandTime;

/*	if(msec < 4 && ent->client->sess.team != TEAM_SPECTATOR && g_popHighFps.integer == 1){
		if(level.time >= level.debugtime){
			if(!G_IsClientDead(ent->client)){
				ent->client->sess.fpschecks += 1;
				if(ent->client->sess.fpschecks >= 3){
					trap_SendServerCommand (-1, va("print\"^3[Info] ^7%s has been popped due to a higher fps then allowed(125).\n\"", ent->client->pers.cleanName ));
					G_Damage (ent, NULL, NULL, NULL, NULL, 10000, 0, MOD_POP, HL_HEAD|HL_FOOT_RT|HL_FOOT_LT|HL_LEG_UPPER_RT|HL_LEG_UPPER_LT|HL_HAND_RT|HL_HAND_LT|HL_WAIST|HL_CHEST|HL_NECK);
					ent->client->sess.fpschecks = 0;
				}
			}
			level.debugtime = level.time+1000;
			return;
		}
	}
	*/
		
	// following others may result in bad times, but we still want
	// to check for follow toggles
	if ( msec < 1 && client->sess.spectatorState != SPECTATOR_FOLLOW ) 
	{
		return;
	}

	if ( msec > 200 ) 
	{
		msec = 200;
	}

	if ( pmove_msec.integer < 8 ) 
	{
		trap_Cvar_Set("pmove_msec", "8");
	}
	else if (pmove_msec.integer > 33) 
	{
		trap_Cvar_Set("pmove_msec", "33");
	}

	if ( pmove_fixed.integer || client->pers.pmoveFixed ) 
	{
		ucmd->serverTime = ((ucmd->serverTime + pmove_msec.integer-1) / pmove_msec.integer) * pmove_msec.integer;
	}

	//Ryan march 21 9:10am
	if(level.awardTime)
	{
		ent->client->ps.pm_type = PM_FREEZE;
		memset (&pm, 0, sizeof(pm));
		pm.ps = &client->ps;
		pm.cmd = *ucmd;
		Pmove (&pm);
		return;
	}
	//Ryan

	//
	// check for exiting intermission
	//
	if( level.intermissiontime )
	{
		ClientIntermissionThink( client );
		return;
	}

	//Ryan june 15 2003
	if(level.pause)		//if paused stop here
	{
		///RxCxW - 08.28.06 - 03:51pm - #paused - reset inactivity counter so we dont get kicked
		if ( g_inactivity.integer )
			client->inactivityTime = level.time + g_inactivity.integer * 1000;
		else 
			client->inactivityTime = level.time + 60 * 1000;
		client->inactivityWarning = qfalse;
		///End  - 08.28.06 - 03:52pm
		return;
	}
	//Ryan

	// Boe!Man 3/30/10: We wait for the motd.
	if(client->sess.firstTime && !client->sess.motdStartTime && !level.intermissionQueued)
	{
		if ( ucmd->buttons & BUTTON_ANY )
		{
			client->sess.motdStartTime = level.time;
			client->sess.motdStopTime = level.time + 7000;
			trap_SendServerCommand( ent - g_entities, va("chat -1 \"%sI%sn%sf%so%s: ^7This server is running %s ^7%s\n\"", server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, INF_VERSION_STRING_COLORED, INF_VERSION_STRING));
			trap_SendServerCommand( ent - g_entities, va("chat -1 \"%sI%sn%sf%so%s: ^7Please report any bugs on 1fx.uk.to\n\"", server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string));
			Boe_Motd(ent);
		}
	}
	//Ryan

	//Ryan july 1 2003
	if(client->sess.motdStartTime)
	{	
		if (level.time >= client->sess.motdStartTime + 3000 
		&& level.time <= client->sess.motdStopTime)
		{
			client->sess.motdStartTime += 3000;
			Boe_Motd(ent);
		}
		// Boe!Man 3/16/11: Better to reset the values and actually put firstTime to qfalse so it doesn't mess up when we want to broadcast a teamchange.
		else if(level.time >= client->sess.motdStopTime)
		{
			client->sess.motdStartTime = 0;
			client->sess.motdStopTime = 0;
			if(client->sess.firstTime)
			{
				BroadcastTeamChange( client, -1 );
				client->sess.firstTime = qfalse;
			}
		}

	}
	if(level.time > client->sess.oneSecChecks){
		client->sess.oneSecChecks = level.time + 1000;
		if ( client->sess.burnSeconds ){
			client->sess.burnSeconds--;
			if (ent->client->ps.stats[STAT_HEALTH] >= 35)
				G_Damage (ent, NULL, NULL, NULL, NULL, 12, 0, MOD_BURN, HL_NONE );

			VectorCopy(ent->client->ps.viewangles, fireAngs);
			AngleVectors( fireAngs, dir, NULL, NULL );	
			dir[0] *= -1.0;
			dir[1] *= -1.0;
			dir[2] = 0.0;
			VectorNormalize ( dir );
			G_ApplyKnockback ( ent, dir, 10 );	//knock them back
		}
	}
	// spectators don't do much
	if ( G_IsClientSpectating ( client ) ) 
	{
		//Ryan march 7 2004 12:13AM  
		//Prevent the 999 ping on respawn
#ifdef _SOF2_BOTS
		if (ent->r.svFlags & SVF_BOT)
		{
			return;
		}	
#endif
		//Ryan

		if ( client->sess.spectatorState == SPECTATOR_SCOREBOARD ) 
		{
			return;
		}
		
		SpectatorThink( ent, ucmd );
		return;
	}

	// check for inactivity timer, but never drop the local client of a non-dedicated server
	if ( !ClientInactivityTimer( client ) ) 
	{
		return;
	}

	if ( client->noclip ) 
	{
		client->ps.pm_type = PM_NOCLIP;
	} 
	else if ( client->ps.stats[STAT_HEALTH] <= 0 ) 
	{
		client->ps.pm_type = PM_DEAD;
	}
	else 
	{
		client->ps.pm_type = PM_NORMAL;
	}

	if(current_gametype.value == GT_HS){
		// Boe!Man 7/21/12: Fix for MM1 gravity not maintaining balance with actual g_gravity CVAR (MM1 should always provide a gravity boost, no matter the actual g_gravity CVAR value).
		if(client->ps.weapon == WP_MM1_GRENADE_LAUNCHER){
			if(client->ps.gravity > 350){
				client->ps.gravity = g_gravity.value-300;
			}else{
				client->ps.gravity = g_gravity.value;
			}
		}else{
			client->ps.gravity = g_gravity.value;
		}
		if(client->ps.weapon == WP_RPG7_LAUNCHER && level.time > client->sess.slowtime){
			client->ps.speed = g_speed.value+70;
			if(level.time >= client->sess.SpeedAnimation){
				if(ent->r.currentOrigin[1] != client->sess.oldvelocity[1] || ent->r.currentOrigin[2] != client->sess.oldvelocity[2] ){
					G_PlayEffect ( G_EffectIndex("arm2smallsmoke"), client->ps.origin, ent->pos1);
					client->sess.SpeedAnimation = level.time+10;
					VectorCopy(ent->r.currentOrigin,client->sess.oldvelocity);
				}
			}
		}else if(level.time <= client->sess.speedtime){
			client->ps.speed = g_speed.value+70;
			if(level.time >= client->sess.SpeedAnimation){
				if(ent->r.currentOrigin[1] != client->sess.oldvelocity[1] || ent->r.currentOrigin[2] != client->sess.oldvelocity[2] ){
					G_PlayEffect ( G_EffectIndex("arm2smallsmoke"), client->ps.origin, ent->pos1);
					client->sess.SpeedAnimation = level.time+30;
					VectorCopy(ent->r.currentOrigin,client->sess.oldvelocity);
				}
			}
		}else if(level.time <= client->sess.slowtime){
			client->ps.speed = g_speed.value-110;
		}else{
			client->ps.speed = g_speed.value;
		}
			
		// Boe!Man 11/11/12: /howto messages for minigames, just CTB for now.
		if(level.time > client->sess.spamTime && client->sess.spamTime != -1){
			client->sess.spamTime = -1; // Boe!Man 10/5/10: Just display these at map restart/client begin.
			if(level.crossTheBridge){
				trap_SendServerCommand( ent-g_entities, va("chat -1 \"%sI%sn%sf%so%s: You're playing a mini-game instead of regular gameplay.\n\"", server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
				trap_SendServerCommand( ent-g_entities, va("chat -1 \"%sI%sn%sf%so%s: Don't know how this works? Type '/howto' to get a short tutorial.\n\"", server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
				//trap_SendServerCommand( ent-g_entities, va("chat -1 \"%sI%sn%sf%so%s: Brilliant ideas for another mini-game? Let us know!\n\"", server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
				//trap_SendServerCommand( ent-g_entities, va("chat -1 \"%sI%sn%sf%so%s: Development forums at 1fx.uk.to!\n\"", server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
			}
		}
		
		// Boe!Man 1/28/14: Also check the inactivity stuff for seekers.
		if(level.messagedisplay && client->sess.team == TEAM_BLUE){
			vec3_t newOrigin;
			
			// The seeker moved.
			if(client->pers.cmd.forwardmove || client->pers.cmd.rightmove || client->pers.cmd.upmove || (client->pers.cmd.buttons & (BUTTON_ATTACK|BUTTON_ALT_ATTACK))){
				if(!client->seekerAway){
					client->seekerAwayTime = level.time + 10000;
				}else{ // He was away, now he's not anymore.
					trap_SendServerCommand(-1, va("print\"^3[H&S] ^7%s ^7is not away anymore.\n\"", client->pers.cleanName));
					client->seekerAway = qfalse;
					client->seekerAwayTime = level.time + 10000;
				}
			}else if(!client->seekerAway && level.time > client->seekerAwayTime){
				trap_SendServerCommand(-1, va("print\"^3[H&S] ^7%s ^7is now away.\n\"", client->pers.cleanName));
				client->seekerAway = level.time + 500;
				
				VectorCopy(client->ps.origin, newOrigin);
				newOrigin[0] += 5;
				newOrigin[2] += 75;
				G_PlayEffect ( G_EffectIndex("misc/exclaimation"), newOrigin, ent->pos1);
			}else if(client->seekerAway && level.time > client->seekerAway){
				client->seekerAway = level.time + 500;
				
				VectorCopy(client->ps.origin, newOrigin);
				newOrigin[0] += 5;
				newOrigin[2] += 75;
				G_PlayEffect ( G_EffectIndex("misc/exclaimation"), newOrigin, ent->pos1);
			}
		}
	}else if(current_gametype.value == GT_HZ){
		if(client->sess.firstzombie == qtrue){
			client->ps.speed = g_speed.value+70;
			if(level.time >= client->sess.SpeedAnimation){
				if(ent->r.currentOrigin[1] != client->sess.oldvelocity[1] || ent->r.currentOrigin[2] != client->sess.oldvelocity[2] ){
					G_PlayEffect ( G_EffectIndex("arm2smallsmoke"), client->ps.origin, ent->pos1);
					client->sess.SpeedAnimation = level.time+10;
					VectorCopy(ent->r.currentOrigin,client->sess.oldvelocity);
				}
			}
		}else{
			client->ps.speed = g_speed.value;
		}
		client->ps.gravity = g_gravity.value;
	}else{
		client->ps.gravity = g_gravity.value;
		client->ps.speed = g_speed.value;
	}
	
	// Boe!Man 4/16/13: The accelerator can be used in all gametypes, though they are probably used only in H&S/H&Z...
	if(client->sess.acceleratorCooldown){
		if(client->sess.acceleratorCooldown > level.time){
			client->ps.speed += (g_speed.value / 5000) * (client->sess.acceleratorCooldown - level.time);
		}else{
			client->sess.acceleratorCooldown = 0;
		}
	}
	
	// Boe!Man 7/21/12: New FPS fix (for all gametypes). Should be light on resources.
	if(g_noHighFps.integer){
		if(msec < 4){
			if(msec <= 2){ // Boe!Man 8/3/12: msec 3 = 333 FPS, anything below that is 500/1000, fix that by upping the gravity even more.
				client->ps.gravity += 500;
			}else{
				client->ps.gravity += 111; // 333fps fix.
			}
		}
	}

	// set up for pmove
	oldEventSequence = client->ps.eventSequence;

	memset (&pm, 0, sizeof(pm));

	pm.ps = &client->ps;
	pm.cmd = *ucmd;
	
	if ( pm.ps->pm_type == PM_DEAD ) 
	{
		pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;
	}
	else if ( client->siameseTwin ) 
	{
		// Make sure we are still stuck, if so, clip through players.
		if ( G_IsClientSiameseTwin ( ent, client->siameseTwin ) )
		{
			pm.tracemask = MASK_PLAYERSOLID & ~(CONTENTS_BODY);
		}
		else
		{
			// Ok, we arent stuck anymore so we can clear the stuck flag.
			client->siameseTwin->client->siameseTwin = NULL;
			client->siameseTwin = NULL;

			pm.tracemask = MASK_PLAYERSOLID;
		}
	}
	else if ( ent->r.svFlags & SVF_BOT ) 
	{
		pm.tracemask = MASK_PLAYERSOLID | CONTENTS_BOTCLIP;
	}
	else 
	{
		pm.tracemask = MASK_PLAYERSOLID;
	}
	pm.trace = trap_Trace;
	pm.pointcontents = trap_PointContents;
	pm.debugLevel = g_debugMove.integer;
	pm.noFootsteps = (qboolean)(( g_dmflags.integer & DF_NO_FOOTSTEPS ) > 0);

	pm.pmove_fixed = pmove_fixed.integer | client->pers.pmoveFixed;
	pm.pmove_msec = pmove_msec.integer;

	pm.animations = NULL;

#ifdef _DEBUG
	pm.isClient=0;
#endif

	VectorCopy( client->ps.origin, client->oldOrigin );

	Pmove (&pm);

	G_UpdatePlayerStateScores ( ent );

	// save results of pmove
	if ( ent->client->ps.eventSequence != oldEventSequence ) 
	{
		ent->eventTime = level.time;
	}
	
	// See if the invulnerable flag should be removed for this client
	if ( ent->client->ps.eFlags & EF_INVULNERABLE) 
	{
		if ( level.time - ent->client->invulnerableTime >= g_respawnInvulnerability.integer * 1000 )
		{
			ent->client->ps.eFlags &= ~EF_INVULNERABLE;
		}
	}

	if (g_smoothClients.integer)
	{
		BG_PlayerStateToEntityStateExtraPolate( &ent->client->ps, &ent->s, ent->client->ps.commandTime, qtrue );
	}
	else 
	{
		BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qtrue );
	}

	SendPendingPredictableEvents( &ent->client->ps );

	if ( !( ent->client->ps.eFlags & EF_FIRING ) ) 
	{
		client->fireHeld = qfalse;		// for grapple
	}

	// use the snapped origin for linking so it matches client predicted versions
	VectorCopy( ent->s.pos.trBase, ent->r.currentOrigin );

	VectorCopy (pm.mins, ent->r.mins);
	VectorCopy (pm.maxs, ent->r.maxs);

	ent->waterlevel = pm.waterlevel;
	ent->watertype = pm.watertype;

	// Need to cache off the firemodes to the persitant data segment so they
	// are maintained across spectating and respawning
	memcpy ( ent->client->pers.firemode, ent->client->ps.firemode, sizeof(ent->client->ps.firemode ) );

	// execute client events
	ClientEvents( ent, oldEventSequence );

	// Update the client animation info
	G_UpdateClientAnimations ( ent );

	// link entity now, after any personal teleporters have been used
	trap_LinkEntity (ent);
	if ( !ent->client->noclip ) 
	{
		G_TouchTriggers( ent );
	}

	// NOTE: now copy the exact origin over otherwise clients can be snapped into solid
	VectorCopy( ent->client->ps.origin, ent->r.currentOrigin );

	// Update the clients anti-lag history
	G_UpdateClientAntiLag ( ent );

	// touch other objects
	ClientImpacts( ent, &pm );

	// save results of triggers and client events
	if (ent->client->ps.eventSequence != oldEventSequence) 
	{
		ent->eventTime = level.time;
	}

	// swap and latch button actions
	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	client->latched_buttons |= client->buttons & ~client->oldbuttons;

	// check for respawning
	if ( client->ps.stats[STAT_HEALTH] <= 0 ) 
	{
		// wait for the attack button to be pressed
		if ( level.time > client->respawnTime ) 
		{
			//Ryan june 7 2003							//admins are not forced to respawn
				//if ( g_forcerespawn.integer > 0 && 
				//( level.time - client->respawnTime ) > g_forcerespawn.integer * 1000 )
//RxCxW - 1.12.2005 - Added Support for other #AdminLevels
		/*
			if ( g_forcerespawn.integer > 0 && 
				( level.time - client->respawnTime ) > g_forcerespawn.integer * 1000 && 
				client->sess.admin != 1) 
		*/
			////04.03.05 - 08.05pm - #Fixed admin being forced to respawn
			if ( g_forcerespawn.integer > 0 && 
				( level.time - client->respawnTime ) > g_forcerespawn.integer * 1000) 
//End
			//Ryan
			{
				respawn( ent );
				return;
			}

			// pressing attack or use is the normal respawn method
			if ( ucmd->buttons & BUTTON_ATTACK ) 
			{
				respawn( ent );
			}
		}

		return;
	}
	//Ryan
	if(g_doWarmup.integer == 2) 
	{
		if(!client->sess.firstTime && level.warmupTime < 0)
		{
			RPM_ReadyCheck(ent);
		}
	}
	//Ryan

	// perform once-a-second actions
	ClientTimerActions( ent, msec );
}

/*
==================
G_CheckClientTeamkill

Checks to see whether or not this client should be booted from the server
because they killed too many teammates
==================
*/
void G_CheckClientTeamkill ( gentity_t* ent )
{
	if ( !g_teamkillDamageMax.integer || !level.gametypeData->teams || !ent->client->sess.teamkillDamage ) 
	{
		return;
	}
	//#REMOVED
	//Ryan june 7 2003
	//RxCxW - 1.12.2005 - Support for #AdminLevels - Cant be kicked
	//admins & clan members can't be kicked for teamkilling
	/*
	//RM	if (ent->client->sess.admin == 1)
	if (ent->client->sess.admin >= 2){
		return;
	}
	*/
	//End
				
	// See if they crossed the max team kill damage
	else if ( ent->client->sess.teamkillDamage < g_teamkillDamageMax.integer )
	{	
		// Does the client need forgiving?
		if ( ent->client->sess.teamkillForgiveTime )
		{
			// Are we in a forgiving mood yet?
			if ( level.time > ent->client->sess.teamkillForgiveTime + 60000 )
			{				
				ent->client->sess.teamkillForgiveTime += 60000;
				ent->client->sess.teamkillDamage -= g_teamkillDamageForgive.integer;
			}
		}

		// All forgivin now?
		if ( ent->client->sess.teamkillDamage <= 0 )
		{
			ent->client->sess.teamkillDamage = 0;
			ent->client->sess.teamkillForgiveTime = 0;
		}

		return;
	}

	ent->client->sess.teamkillDamage      = 0;
	ent->client->sess.teamkillForgiveTime = 0;


	// Buh bye
	//trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"team killing\"\n", ent->s.number ) );
	//End - 03.01.05 - 09:45pm 
}

/*
==================
G_CheckClientTimeouts

Checks whether a client has exceded any timeouts and act accordingly
==================
*/
void G_CheckClientTimeouts ( gentity_t *ent )
{
	// Only timeout supported right now is the timeout to spectator mode
	if ( !g_timeouttospec.integer )
	{
		return;
	}

	// Can only do spect timeouts on dedicated servers
	if ( !g_dedicated.integer )
	{
		return;
	}

	// Already a spectator, no need to boot them to spectator
	if ( ent->client->sess.team == TEAM_SPECTATOR )
	{
		return;
	}

	// Need to be connected
	if ( ent->client->pers.connected != CON_CONNECTED )
	{
		return;
	}

	// See how long its been since a command was received by the client and if its 
	// longer than the timeout to spectator then force this client into spectator mode
	if ( level.time - ent->client->pers.cmd.serverTime > g_timeouttospec.integer * 1000 )
	{
		SetTeam ( ent, "spectator", NULL, qfalse );
	}
}

/*
==================
ClientThink

A new command has arrived from the client
==================
*/
void ClientThink( int clientNum ) 
{
	gentity_t *ent;

	ent = g_entities + clientNum;
	trap_GetUsercmd( clientNum, &ent->client->pers.cmd );

	// mark the time we got info, so we can display the
	// phone jack if they don't get any for a while
	ent->client->lastCmdTime = level.time;

	if ( !(ent->r.svFlags & SVF_BOT) && !g_synchronousClients.integer ) 
	{
		ClientThink_real( ent );
	}
}
/*
==================
G_RunClient
==================
*/
void G_RunClient( gentity_t *ent ) 
{
	if ( !(ent->r.svFlags & SVF_BOT) && !g_synchronousClients.integer ) 
	{		
		return;
	}

	ent->client->pers.cmd.serverTime = level.time;
	ClientThink_real( ent );
}


/*
==================
SpectatorClientEndFrame
==================
*/
void SpectatorClientEndFrame( gentity_t *ent ) 
{
	gclient_t	*cl;

	// if we are doing a chase cam or a remote view, grab the latest info
	if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ) 
	{
		int		clientNum, flags;

		clientNum = ent->client->sess.spectatorClient;

		// team follow1 and team follow2 go to whatever clients are playing
		if ( clientNum == -1 ) 
		{
			clientNum = level.follow1;
		} 
		else if ( clientNum == -2 ) 
		{
			clientNum = level.follow2;
		}
		
		if ( clientNum >= 0 ) 
		{
			cl = &level.clients[ clientNum ];
		
			if ( cl->pers.connected == CON_CONNECTED && !G_IsClientSpectating ( cl ) ) 
			{
				int count;
				int ping;
				int respawnTimer;

				count = ent->client->ps.persistant[PERS_SPAWN_COUNT];
				ping  = ent->client->ps.ping;
				flags = (cl->ps.eFlags & ~(EF_VOTED)) | (ent->client->ps.eFlags & (EF_VOTED));
				respawnTimer = ent->client->ps.respawnTimer;

				ent->client->ps = cl->ps;
				ent->client->ps.pm_flags |= PMF_FOLLOW;
				if ( ent->client->sess.spectatorFirstPerson ) 
				{
					ent->client->ps.pm_flags |= PMF_FIRSTPERSONSPEC;
				}
				ent->client->ps.eFlags = flags;
				ent->client->ps.persistant[PERS_SPAWN_COUNT] = count;
				ent->client->ps.ping = ping;
				ent->client->ps.respawnTimer = respawnTimer;

				return;
			} 
			else 
			{
				// drop them to free spectators unless they are dedicated camera followers
				if ( ent->client->sess.spectatorClient >= 0 ) 
				{
					Cmd_FollowCycle_f (ent, 1);
				}
			}
		}
	}

	if ( ent->client->sess.spectatorState == SPECTATOR_SCOREBOARD ) 
	{
		ent->client->ps.pm_flags |= PMF_SCOREBOARD;
	} 
	else 
	{
		ent->client->ps.pm_flags &= ~PMF_SCOREBOARD;
	}
}

/*
==============
ClientEndFrame

Called at the end of each server frame for each connected client
A fast client will have multiple ClientThink for each ClientEdFrame,
while a slow client may have multiple ClientEndFrame between ClientThink.
==============
*/
void ClientEndFrame( gentity_t *ent ) 
{
	clientPersistant_t	*pers;

	if ( G_IsClientSpectating ( ent->client ) ) 
	{
		SpectatorClientEndFrame( ent );
		return;
	}

	pers = &ent->client->pers;

	// save network bandwidth
#if 0
	if ( !g_synchronousClients.integer && ent->client->ps.pm_type == PM_NORMAL ) 
	{
		// FIXME: this must change eventually for non-sync demo recording
		VectorClear( ent->client->ps.viewangles );
	}
#endif

	//
	// If the end of unit layout is displayed, don't give
	// the player any normal movement attributes
	//

	//Ryan june 15 2003
		//if ( level.intermissiontime ) 
	if ( level.intermissiontime || level.pause )
	//Ryan
	{
		return;
	}

	// burn from lava, etc
	P_WorldEffects (ent);

	// apply all the damage taken this frame
	P_DamageFeedback (ent);

	// add the EF_CONNECTION flag if we haven't gotten commands recently
	if ( level.time - ent->client->lastCmdTime > 1000 ) 
	{
		ent->s.eFlags |= EF_CONNECTION;
	} 
	else 
	{
		ent->s.eFlags &= ~EF_CONNECTION;
	}

	// FIXME: get rid of ent->health...
	ent->client->ps.stats[STAT_HEALTH] = ent->health;	

	G_SetClientSound (ent);

	// set the latest infor
	if (g_smoothClients.integer) 
	{
		BG_PlayerStateToEntityStateExtraPolate( &ent->client->ps, &ent->s, ent->client->ps.commandTime, qtrue );
	}
	else 
	{
		BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qtrue );
	}
	
	SendPendingPredictableEvents( &ent->client->ps );

	// set the bit for the reachability area the client is currently in
//	i = trap_AAS_PointReachabilityAreaIndex( ent->client->ps.origin );
//	ent->client->areabits[i >> 3] |= 1 << (i & 7);
}


