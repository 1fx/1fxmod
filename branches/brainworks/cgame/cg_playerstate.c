// Copyright (C) 2001-2002 Raven Software.
//
// cg_playerstate.c -- this file acts on changes in a new playerState_t
// With normal play, this will be done after local prediction, but when
// following another player or playing back a demo, it will be checked
// when the snapshot transitions like all the other entities

#include "cg_local.h"

/*
==============
CG_DamageFeedback
==============
*/
void CG_DamageFeedback( int yawByte, int pitchByte, int damage ) 
{
	float		kick;
	int			health;
	float		scale;
	vec3_t		dir;
	vec3_t		angles;

	// show the attacking player's head and name in corner
	cg.attackerTime = cg.time;

	// the lower on health you are, the greater the view kick will be
	health = cg.snap->ps.stats[STAT_HEALTH];
	if ( health < 40 ) 
	{
		scale = 1;
	} 
	else 
	{
		scale = 40.0 / health;
	}

	kick = damage * scale;

	if (kick < 5)
		kick = 5;
	if (kick > 10)
		kick = 10;

	// if yaw and pitch are both 255, make the damage always centered (falling, etc)
	if ( yawByte == 255 && pitchByte == 255 ) 
	{
		cg.damageX = 0;
		cg.damageY = 0;
		cg.v_dmg_roll = 255;
		cg.v_dmg_pitch = -kick;
	} 
	else 
	{
		float front;
		float left;

		angles[PITCH] = (float)pitchByte / 255.0f * 360.0f;
		angles[YAW]   = (float)yawByte / 255.0f * 360.0f;
		angles[ROLL]  = 0;

		AngleVectors( angles, dir, NULL, NULL );
		VectorSubtract( vec3_origin, dir, dir );

		front = DotProduct (dir, cg.refdef.viewaxis[0] );
		left = DotProduct (dir, cg.refdef.viewaxis[1] );

		cg.v_dmg_roll = kick * left;		
		cg.v_dmg_pitch = -kick * front;

		cg.damageY = -AngleNormalize180(angles[PITCH]);
		cg.damageX = AngleNormalize180(angles[YAW] - cg.predictedPlayerState.viewangles[YAW] + 180);
	}

	// don't let the screen flashes vary as much
	if ( kick > 10 ) 
	{
		kick = 10;
	}

	cg.damageTime = cg.snap->serverTime;
}




/*
================
CG_Respawn

A respawn happened this snapshot
================
*/
void CG_Respawn( void ) 
{
	// no error decay on player movement
	cg.thisFrameTeleport = qtrue;

	// display weapons available
	cg.weaponSelectTime = cg.time;

	// select the weapon the server says we are using
	cg.weaponSelect = cg.snap->ps.weapon;

	// no more camera shake
	cg.shakeStart = 0;

	// Make sure the weapon selection menu isnt up
	cg.weaponMenuUp = qfalse;

	// clear any left over flash grenades
	cg.flashbangTime = 0;

	// Reset the animation
	CG_SetWeaponAnim( cg.snap->ps.weaponAnimId&(~ANIM_TOGGLEBIT), &cg.snap->ps );

	// Update the view weapon surfaces
	CG_UpdateViewWeaponSurfaces ( &cg.snap->ps );

	trap_ResetAutorun ( );
}

extern char *eventnames[];

/*
==============
CG_CheckPlayerstateEvents
==============
*/
void CG_CheckPlayerstateEvents( playerState_t *ps, playerState_t *ops ) 
{
	int			i;
	int			event;
	centity_t	*cent;

	if ( ps->externalEvent && ps->externalEvent != ops->externalEvent ) 
	{
		cent = CG_GetEntity ( ps->clientNum );
		cent->currentState.event = ps->externalEvent;
		cent->currentState.eventParm = ps->externalEventParm;
		CG_EntityEvent( cent, cent->lerpOrigin );
	}

	cent = &cg_entities[ ps->clientNum ];

	// go through the predictable events buffer
	for ( i = ps->eventSequence - MAX_PS_EVENTS ; i < ps->eventSequence ; i++ ) {
		// if we have a new predictable event
		if ( i >= ops->eventSequence
			// or the server told us to play another event instead of a predicted event we already issued
			// or something the server told us changed our prediction causing a different event
			|| (i > ops->eventSequence - MAX_PS_EVENTS && ps->events[i & (MAX_PS_EVENTS-1)] != ops->events[i & (MAX_PS_EVENTS-1)]) ) {

			event = ps->events[ i & (MAX_PS_EVENTS-1) ];
			cent->currentState.event = event;
			cent->currentState.eventParm = ps->eventParms[ i & (MAX_PS_EVENTS-1) ];
			CG_EntityEvent( cent, cent->lerpOrigin );

			cg.predictableEvents[ i & (MAX_PREDICTED_EVENTS-1) ] = event;

			cg.eventSequence++;
		}
	}
}

/*
==================
CG_CheckChangedPredictableEvents
==================
*/
void CG_CheckChangedPredictableEvents( playerState_t *ps ) 
{
	int			i;
	int			event;
	centity_t	*cent;

	cent = &cg_entities[ps->clientNum];
	for ( i = ps->eventSequence - MAX_PS_EVENTS ; i < ps->eventSequence ; i++ ) 
	{
		//
		if (i >= cg.eventSequence) 
		{
			continue;
		}
		// if this event is not further back in than the maximum predictable events we remember
		if (i > cg.eventSequence - MAX_PREDICTED_EVENTS) 
		{
			// if the new playerstate event is different from a previously predicted one
			if ( ps->events[i & (MAX_PS_EVENTS-1)] != cg.predictableEvents[i & (MAX_PREDICTED_EVENTS-1) ] ) 
			{
				event = ps->events[ i & (MAX_PS_EVENTS-1) ];
				cent->currentState.event = event;
				cent->currentState.eventParm = ps->eventParms[ i & (MAX_PS_EVENTS-1) ];
				CG_EntityEvent( cent, cent->lerpOrigin );

				cg.predictableEvents[ i & (MAX_PREDICTED_EVENTS-1) ] = event;

				if ( cg_showmiss.integer ) 
				{
					Com_Printf("WARNING: changed predicted event\n");
				}
			}
		}
	}
}

/*
==================
CG_CheckLocalSounds
==================
*/
void CG_CheckLocalSounds( playerState_t *ps, playerState_t *ops ) 
{
	// don't play the sounds if the player just changed teams
	if ( ps->persistant[PERS_TEAM] != ops->persistant[PERS_TEAM] ) 
	{
		return;
	}

	// health changes of more than -1 should make pain sounds
	if ( ps->stats[STAT_HEALTH] < ops->stats[STAT_HEALTH] - 1 ) 
	{
		if ( ps->stats[STAT_HEALTH] > 0 ) 
		{
			CG_PainEvent( &cg_entities[ps->clientNum], ps->stats[STAT_HEALTH] );
		}
	}

	// Look for a zoom transition that isnt the first zoom in
	if ( ops->zoomFov && (ps->zoomFov != ops->zoomFov) )
	{
		trap_S_StartLocalSound ( cgs.media.zoomSound, CHAN_AUTO );
	}
}

/*
===============
CG_TransitionPlayerState
===============
*/
void CG_TransitionPlayerState( playerState_t *ps, playerState_t *ops ) 
{
	// respawning.  This is done before the follow mode check because spawn count is 
	// maintained into the following client
	if ( ps->persistant[PERS_SPAWN_COUNT] != ops->persistant[PERS_SPAWN_COUNT] ) 
	{
		CG_Respawn();
	}

	// check for changing follow mode
	if ( ps->clientNum != ops->clientNum ) 
	{
		cg.thisFrameTeleport = qtrue;
		// make sure we don't get any unwanted transition effects
		*ops = *ps;
	}

	// damage events (player is getting wounded)
	if ( ps->damageEvent != ops->damageEvent && ps->damageCount ) 
	{
		CG_DamageFeedback( ps->damageYaw, ps->damagePitch, ps->damageCount );
	}

	// Make sure we clear the weapon menu when we die
	if ( ps->stats[STAT_HEALTH] != ops->stats[STAT_HEALTH] )
	{
		if ( ps->stats[STAT_HEALTH] <= 0 )
		{
			cg.weaponMenuUp = qfalse;
			cg.deathTime = cg.time;
			trap_ResetAutorun ( );
		}
	}

	if ( cg.mapRestart ) 
	{
		CG_Respawn();
		cg.mapRestart = qfalse;
	}

	if ( cg.snap->ps.pm_type != PM_INTERMISSION && ps->persistant[PERS_TEAM] != TEAM_SPECTATOR ) 
	{
		CG_CheckLocalSounds( ps, ops );
	}

	// Always use the weapon from the player state when following
	if( (ps->pm_flags & PMF_FOLLOW) || (ps->weapon != ops->weapon) )
	{
		cg.weaponSelect = ps->weapon;
	}

	// Check for weapon animation change.
	if(ps->weaponAnimId!=ops->weaponAnimId )
	{
		CG_SetWeaponAnim(ps->weaponAnimId&(~ANIM_TOGGLEBIT),ps);
	}

	// run events
	CG_CheckPlayerstateEvents( ps, ops );

	// smooth the ducking viewheight change and not crouch jumping
	if ( ps->viewheight != ops->viewheight && !(ps->pm_flags & PMF_CROUCH_JUMP) ) 
	{
		cg.duckChange = ps->viewheight - ops->viewheight;
		cg.duckTime = cg.time;
	}

	// Need to update the view weapon surfaces when weapon states change
	if ( ps->weaponstate != ops->weaponstate )
	{
		CG_UpdateViewWeaponSurfaces ( ps );
	}
}

