// Copyright (C) 2001-2002 Raven Software
//
// bg_pmove.c -- both games player movement code
// takes a playerstate and a usercmd as input and returns a modifed playerstate

#include "q_shared.h"
#include "bg_public.h"
#include "bg_local.h"
#include "g_local.h"

pmove_t		*pm;
pml_t		pml;

// Speed scales
float		pm_stopspeed			= 100.0f;
float		pm_duckScale			= 0.25f;
float		pm_swimScale			= 0.50f;
float		pm_wadeScale			= 0.70f;
const float	pm_ladderScale			= 0.5f;
				
// Accelerations
float		pm_accelerate			= 6.0f;
float		pm_airaccelerate		= 1.0f;
float		pm_wateraccelerate		= 4.0f;
float		pm_flyaccelerate		= 8.0f;
									
// Frictions							
float		pm_headfriction			= 0.0f;			// Friction when on someones head
float		pm_friction				= 6.0f;			// Friction when on the ground
float		pm_waterfriction		= 3.0f;			// Friction when in water
float		pm_ladderfriction		= 6.0f;			// Friction when on a ladder
float		pm_spectatorfriction	= 5.0f;			// Friction when flying aroudn as a spectator

int			c_pmove = 0;

ladder_t	pm_ladders[MAX_LADDERS];
int			pm_laddercount = 0;

static void PM_Weapon_AddInaccuracy	( attackType_t attack );
static void PM_Weapon_AddKickAngles	( vec3_t kickAngles );
static void PM_BeginZoomOut			( void );

/*
===============
PM_AddEvent
===============
*/
void PM_AddEvent( int newEvent ) 
{
	BG_AddPredictableEventToPlayerstate( newEvent, 0, pm->ps );
}

/*
===============
PM_AddEventWithParm
===============
*/
void PM_AddEventWithParm( int newEvent, int parm ) 
{
	BG_AddPredictableEventToPlayerstate( newEvent, parm, pm->ps );
}

/*
===============
PM_AddTouchEnt
===============
*/
void PM_AddTouchEnt ( int entityNum ) 
{
	int		i;

	// Cant add the world as a touch entity
	if ( entityNum == ENTITYNUM_WORLD ) 
	{
		return;
	}

	// Ensure the max touch limit has not been reached
	if ( pm->numtouch == MAXTOUCH ) 
	{
		return;
	}

	// see if it is already added
	for ( i = 0 ; i < pm->numtouch ; i++ ) 
	{
		if ( pm->touchents[ i ] == entityNum ) 
		{
			return;
		}
	}

	// add it
	pm->touchents[pm->numtouch] = entityNum;
	pm->numtouch++;
}


/*
==================
PM_ClipVelocity

Slide off of the impacting surface
==================
*/
void PM_ClipVelocity( vec3_t in, vec3_t normal, vec3_t out, float overbounce ) 
{
	float	backoff;
	float	change;
	int		i;
	
	backoff = DotProduct (in, normal);
	
	if ( backoff < 0 ) 
	{
		backoff *= overbounce;
	} 
	else 
	{
		backoff /= overbounce;
	}

	for ( i=0 ; i<3 ; i++ ) 
	{
		change = normal[i]*backoff;
		out[i] = in[i] - change;
	}
}


/*
==================
PM_Friction

Handles both ground friction and water friction
==================
*/
static void PM_Friction( void ) 
{
	vec3_t	vec;
	float	*vel;
	float	speed, newspeed, control;
	float	drop;
	
	vel = pm->ps->velocity;
	
	VectorCopy( vel, vec );
	if ( pml.walking ) 
	{
		vec[2] = 0;	// ignore slope movement
	}

	speed = VectorLength(vec);
	if (speed < 1) 
	{
		vel[0] = 0;
		vel[1] = 0;		// allow sinking underwater
		// FIXME: still have z friction underwater?
		return;
	}

	drop = 0;

	// apply ground friction
	if ( pm->waterlevel <= 1 ) 
	{
		if ( pml.walking && !(pml.groundTrace.surfaceFlags & SURF_SLICK) ) 
		{
			// if getting knocked back, no friction
			if ( ! (pm->ps->pm_flags & PMF_TIME_KNOCKBACK) ) 
			{
				control = speed < pm_stopspeed ? pm_stopspeed : speed;
				drop += control*pm_friction*pml.frametime;
			}
		}
	}

	// apply water friction even if just wading
	if ( pm->ps->pm_flags & PMF_LADDER )  
	{
		if ( !pml.groundPlane )
		{
			control = speed < pm_stopspeed ? pm_stopspeed : speed;
			drop += control*pm_ladderfriction*pml.frametime;
		}
	}
	else if ( pm->waterlevel ) 
	{
		drop += speed*pm_waterfriction*pm->waterlevel*pml.frametime;
	}
	// If on someones head then use special friction
	else if ( pm->ps->groundEntityNum < MAX_CLIENTS )
	{
		drop = speed*pm_headfriction*pml.frametime;
	}

	if ( pm->ps->pm_type == PM_SPECTATOR) 
	{
		drop += speed*pm_spectatorfriction*pml.frametime;
	}


	// scale the velocity
	newspeed = speed - drop;
	if (newspeed < 0) 
	{
		newspeed = 0;
	}
	newspeed /= speed;

	vel[0] = vel[0] * newspeed;
	vel[1] = vel[1] * newspeed;
	vel[2] = vel[2] * newspeed;
}


/*
==============
PM_Accelerate

Handles user intended acceleration
==============
*/
static void PM_Accelerate( vec3_t wishdir, float wishspeed, float accel ) 
{
#if 1
	// q2 style
	int			i;
	float		addspeed, accelspeed, currentspeed;

	currentspeed = DotProduct (pm->ps->velocity, wishdir);
	addspeed = wishspeed - currentspeed;
	if (addspeed <= 0) {
		return;
	}
	accelspeed = accel*pml.frametime*wishspeed;
	if (accelspeed > addspeed) {
		accelspeed = addspeed;
	}

	for (i=0 ; i<3 ; i++) {
		pm->ps->velocity[i] += accelspeed*wishdir[i];	
	}
#else
	// proper way (avoids strafe jump maxspeed bug), but feels bad
	vec3_t		wishVelocity;
	vec3_t		pushDir;
	float		pushLen;
	float		canPush;

	VectorScale( wishdir, wishspeed, wishVelocity );
	VectorSubtract( wishVelocity, pm->ps->velocity, pushDir );
	pushLen = VectorNormalize( pushDir );

	canPush = accel*pml.frametime*wishspeed;
	if (canPush > pushLen) {
		canPush = pushLen;
	}
	VectorMA( pm->ps->velocity, canPush, pushDir, pm->ps->velocity );
#endif
}

/*
============
PM_CmdScale

Returns the scale factor to apply to cmd movements
This allows the clients to use axial -127 to 127 values for all directions
without getting a sqrt(2) distortion in speed.
============
*/
static float PM_CmdScale( usercmd_t *cmd ) 
{
	int		max;
	float	total;
	float	scale;

	max = abs( cmd->forwardmove );
	if ( abs( cmd->rightmove ) > max ) 
	{
		max = abs( cmd->rightmove );
	}
	
	if ( abs( cmd->upmove ) > max ) 
	{
		max = abs( cmd->upmove );
	}

	if ( !max ) 
	{
		return 0;
	}

	total = sqrt( cmd->forwardmove * cmd->forwardmove
		+ cmd->rightmove * cmd->rightmove + cmd->upmove * cmd->upmove );
	scale = (float)pm->ps->speed * max / ( 127.0 * total );

	return scale;
}


/*
================
PM_SetMovementDir

Determine the rotation of the legs reletive
to the facing dir
================
*/
static void PM_SetMovementDir( void ) 
{
	if ( pm->cmd.forwardmove || pm->cmd.rightmove ) 
	{
		if ( pm->cmd.rightmove == 0 && pm->cmd.forwardmove > 0 ) 
		{
			pm->ps->movementDir = 0;
		} 
		else if ( pm->cmd.rightmove < 0 && pm->cmd.forwardmove > 0 ) 
		{
			pm->ps->movementDir = 1;
		} 
		else if ( pm->cmd.rightmove < 0 && pm->cmd.forwardmove == 0 ) 
		{
			pm->ps->movementDir = 2;
		} 
		else if ( pm->cmd.rightmove < 0 && pm->cmd.forwardmove < 0 ) 
		{
			pm->ps->movementDir = 3;
		} 
		else if ( pm->cmd.rightmove == 0 && pm->cmd.forwardmove < 0 ) 
		{
			pm->ps->movementDir = 4;
		}
		else if ( pm->cmd.rightmove > 0 && pm->cmd.forwardmove < 0 ) 
		{
			pm->ps->movementDir = 5;
		} 
		else if ( pm->cmd.rightmove > 0 && pm->cmd.forwardmove == 0 ) 
		{
			pm->ps->movementDir = 6;
		} 
		else if ( pm->cmd.rightmove > 0 && pm->cmd.forwardmove > 0 ) 
		{
			pm->ps->movementDir = 7;
		}
	} 
	else 
	{
		// if they aren't actively going directly sideways,
		// change the animation to the diagonal so they
		// don't stop too crooked
		if ( pm->ps->movementDir == 2 ) 
		{
			pm->ps->movementDir = 1;
		} 
		else if ( pm->ps->movementDir == 6 ) 
		{
			pm->ps->movementDir = 7;
		} 
	}
}

/*
=============
PM_CheckJump
=============
*/
static qboolean PM_CheckJump( void ) 
{
	if ( pm->ps->pm_time )
	{
		return qfalse;
	}

	// Cant jump when ducked
	if ( pm->ps->pm_flags & PMF_DUCKED ) 
	{
		return qfalse;
	}

	// don't allow jump until all buttons are up
	if ( pm->ps->pm_flags & PMF_RESPAWNED ) 
	{
		return qfalse;		
	}

	// not holding jump
	if ( pm->cmd.upmove < 10 ) 
	{
		return qfalse;
	}

	// must wait for jump to be released
	if ( pm->ps->pm_debounce & PMD_JUMP )
	{
		// clear upmove so cmdscale doesn't lower running speed
		pm->cmd.upmove = 0;
		return qfalse;
	}

	pml.groundPlane = qfalse;		// jumping away
	pml.walking = qfalse;
	pm->ps->pm_debounce |= PMD_JUMP;
	pm->ps->pm_flags |= PMF_JUMPING;

	pm->ps->groundEntityNum = ENTITYNUM_NONE;

	if ( pm->cmd.forwardmove >= 0 ) 
	{
		PM_ForceLegsAnim( pm->ps, LEGS_JUMP );
	} 
	else 
	{
		PM_ForceLegsAnim( pm->ps, LEGS_JUMP_BACK );
		pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
	}

	// Special case for ladders
	if ( pm->ps->ladder != -1 )
	{
		vec3_t forward;	
		VectorCopy ( pm_ladders[pm->ps->ladder].fwd, forward );
		forward[2] = 0;
		VectorNormalize ( forward );
		VectorMA ( pm->ps->velocity, -50, forward, pm->ps->velocity );
		pm->ps->pm_flags |= PMF_LADDER_JUMP;
		return qtrue;
	}

	pm->ps->velocity[2] = JUMP_VELOCITY;
	
	return qtrue;
}

/*
=============
PM_CheckWaterJump
=============
*/
static qboolean	PM_CheckWaterJump( void ) 
{
	vec3_t	spot;
	int		cont;
	vec3_t	flatforward;

	if (pm->ps->pm_time) 
	{
		return qfalse;
	}

	// check for water jump
	if ( pm->waterlevel != 2 ) 
	{
		return qfalse;
	}

	flatforward[0] = pml.forward[0];
	flatforward[1] = pml.forward[1];
	flatforward[2] = 0;
	VectorNormalize (flatforward);

	VectorMA (pm->ps->origin, 30, flatforward, spot);
	spot[2] += 4;
	cont = pm->pointcontents (spot, pm->ps->clientNum );
	if ( !(cont & CONTENTS_SOLID) ) 
	{
		return qfalse;
	}

	spot[2] += 16;
	cont = pm->pointcontents (spot, pm->ps->clientNum );
	if ( cont ) 
	{
		return qfalse;
	}

	// jump out of water
	VectorScale (pml.forward, 200, pm->ps->velocity);
	pm->ps->velocity[2] = 350;

	pm->ps->pm_flags |= PMF_TIME_WATERJUMP;
	pm->ps->pm_time = 2000;

	return qtrue;
}

/*
===================
PM_WaterJumpMove

Flying out of the water
===================
*/
static void PM_WaterJumpMove( void ) 
{
	// waterjump has no control, but falls
	PM_StepSlideMove( qtrue );

	pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
	if (pm->ps->velocity[2] < 0) 
	{
		// cancel as soon as we are falling down again
		pm->ps->pm_flags &= ~PMF_ALL_TIMES;
		pm->ps->pm_time = 0;
	}
}

/*
===================
PM_WaterMove
===================
*/
static void PM_WaterMove( void ) 
{
	int		i;
	vec3_t	wishvel;
	float	wishspeed;
	vec3_t	wishdir;
	float	scale;
	float	vel;

	if ( PM_CheckWaterJump() ) 
	{
		PM_WaterJumpMove();
		return;
	}

#if 0
	// jump = head for surface
	if ( pm->cmd.upmove >= 10 ) {
		if (pm->ps->velocity[2] > -300) {
			if ( pm->watertype == CONTENTS_WATER ) {
				pm->ps->velocity[2] = 100;
			} else {
				pm->ps->velocity[2] = 50;
			}
		}
	}
#endif
	PM_Friction ();

	scale = PM_CmdScale( &pm->cmd );
	//
	// user intentions
	//
	if ( !scale ) 
	{
		wishvel[0] = 0;
		wishvel[1] = 0;
		wishvel[2] = -60;		// sink towards bottom
	} 
	else 
	{
		for (i=0 ; i<3 ; i++)
		{
			wishvel[i] = scale * pml.forward[i]*pm->cmd.forwardmove + scale * pml.right[i]*pm->cmd.rightmove;
		}

		wishvel[2] += scale * pm->cmd.upmove;
	}

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);

	if ( wishspeed > pm->ps->speed * pm_swimScale ) 
	{
		wishspeed = pm->ps->speed * pm_swimScale;
	}

	PM_Accelerate (wishdir, wishspeed, pm_wateraccelerate);

	// make sure we can go up slopes easily under water
	if ( pml.groundPlane && DotProduct( pm->ps->velocity, pml.groundTrace.plane.normal ) < 0 ) 
	{
		vel = VectorLength(pm->ps->velocity);
		// slide along the ground plane
		PM_ClipVelocity (pm->ps->velocity, pml.groundTrace.plane.normal, 
			pm->ps->velocity, OVERCLIP );

		VectorNormalize(pm->ps->velocity);
		VectorScale(pm->ps->velocity, vel, pm->ps->velocity);
	}

	PM_SlideMove( qfalse );
}

/*
===================
PM_FlyMove
===================
*/
static void PM_FlyMove( void ) 
{
	int		i;
	vec3_t	wishvel;
	float	wishspeed;
	vec3_t	wishdir;
	float	scale;

	// normal slowdown
	PM_Friction ();

	scale = PM_CmdScale( &pm->cmd );
	//
	// user intentions
	//
	if ( !scale ) 
	{
		wishvel[0] = 0;
		wishvel[1] = 0;
		wishvel[2] = 0;
	} 
	else 
	{
		for (i=0 ; i<3 ; i++) 
		{
			wishvel[i] = scale * pml.forward[i]*pm->cmd.forwardmove + scale * pml.right[i]*pm->cmd.rightmove;
		}

		wishvel[2] += scale * pm->cmd.upmove;
	}

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);

	PM_Accelerate (wishdir, wishspeed, pm_flyaccelerate);

	PM_StepSlideMove( qfalse );
}


/*
===================
PM_AirMove

===================
*/
static void PM_AirMove( void ) 
{
	int			i;
	vec3_t		wishvel;
	float		fmove, smove;
	vec3_t		wishdir;
	float		wishspeed;
	float		scale;
	usercmd_t	cmd;

	PM_Friction();

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.rightmove;

	cmd = pm->cmd;
	scale = PM_CmdScale( &cmd );

	// set the movementDir so clients can rotate the legs for strafing
	PM_SetMovementDir();

	// project moves down to flat plane
	pml.forward[2] = 0;
	pml.right[2] = 0;
	VectorNormalize (pml.forward);
	VectorNormalize (pml.right);

	for ( i = 0 ; i < 2 ; i++ ) {
		wishvel[i] = pml.forward[i]*fmove + pml.right[i]*smove;
	}
	wishvel[2] = 0;

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);
	wishspeed *= scale;

	// not on ground, so little effect on velocity
	PM_Accelerate (wishdir, wishspeed, pm_airaccelerate);

	// we may have a ground plane that is very steep, even
	// though we don't have a groundentity
	// slide along the steep plane
	if ( pml.groundPlane ) {
		PM_ClipVelocity (pm->ps->velocity, pml.groundTrace.plane.normal, 
			pm->ps->velocity, OVERCLIP );
	}
	PM_StepSlideMove ( qtrue );
}

/*
===================
PM_WalkMove
===================
*/
static void PM_WalkMove( void ) {
	int			i;
	vec3_t		wishvel;
	float		fmove, smove;
	vec3_t		wishdir;
	float		wishspeed;
	float		scale;
	usercmd_t	cmd;
	float		accelerate;
	float		vel;

	if ( pm->waterlevel > 2 && DotProduct( pml.forward, pml.groundTrace.plane.normal ) > 0 ) 
	{
		// begin swimming
		PM_WaterMove();
		return;
	}
	//RxCxW - 05.22.06 - 04:17pm #Debug $FIX //RxCxW - 05.22.06 - 04:17pm 
	//Com_Printf("^1current origin: %.2f %.2f %.2f\n", pm->ps->origin[0], pm->ps->origin[1],pm->ps->origin[2]);
	//End  - 05.22.06 - 04:17pm


	if ( PM_CheckJump () )
	{
		PM_BeginZoomOut ( );

		// jumped away
		if ( pm->waterlevel > 1 ) 
		{
			PM_WaterMove();
		} 
		else 
		{
			PM_AirMove();
		}
		return;
	}

	PM_Friction ();

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.rightmove;

	cmd = pm->cmd;
	scale = PM_CmdScale( &cmd );

	// set the movementDir so clients can rotate the legs for strafing
	PM_SetMovementDir();

	// project moves down to flat plane
	pml.forward[2] = 0;
	pml.right[2] = 0;

	// project the forward and right directions onto the ground plane
	PM_ClipVelocity (pml.forward, pml.groundTrace.plane.normal, pml.forward, OVERCLIP );
	PM_ClipVelocity (pml.right, pml.groundTrace.plane.normal, pml.right, OVERCLIP );
	//
	VectorNormalize (pml.forward);
	VectorNormalize (pml.right);

	for ( i = 0 ; i < 3 ; i++ ) {
		wishvel[i] = pml.forward[i]*fmove + pml.right[i]*smove;
	}
	// when going up or down slopes the wish velocity should Not be zero
//	wishvel[2] = 0;

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);
	wishspeed *= scale;

	// clamp the speed lower if ducking
	if ( pm->ps->pm_flags & PMF_DUCKED ) {
		if ( wishspeed > pm->ps->speed * pm_duckScale ) {
			wishspeed = pm->ps->speed * pm_duckScale;
		}
	}

	// clamp the speed lower if wading or walking on the bottom
	if ( pm->waterlevel && !(pm->watertype & CONTENTS_LADDER) ) {
		float	waterScale;

		waterScale = pm->waterlevel / 3.0;
		waterScale = 1.0 - ( 1.0 - pm_swimScale ) * waterScale;
		if ( wishspeed > pm->ps->speed * waterScale ) {
			wishspeed = pm->ps->speed * waterScale;
		}
	}

	// when a player gets hit, they temporarily lose
	// full control, which allows them to be moved a bit
	if ( ( pml.groundTrace.surfaceFlags & SURF_SLICK ) || pm->ps->pm_flags & PMF_TIME_KNOCKBACK ) 
	{
		accelerate = pm_airaccelerate;
	} 
	else 
	{
		accelerate = pm_accelerate;

		// Accelerate faster when ducked
		if ( pm->ps->pm_flags & PMF_DUCKED ) 
		{
			accelerate *= 2;
		}
	}

	PM_Accelerate (wishdir, wishspeed, accelerate);

	if ( ( pml.groundTrace.surfaceFlags & SURF_SLICK ) || pm->ps->pm_flags & PMF_TIME_KNOCKBACK ) 
	{
		pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
	} 

	vel = VectorLength(pm->ps->velocity);

	// slide along the ground plane
	PM_ClipVelocity (pm->ps->velocity, pml.groundTrace.plane.normal, 
		pm->ps->velocity, OVERCLIP );

	// don't decrease velocity when going up or down a slope
	VectorNormalize(pm->ps->velocity);
	VectorScale(pm->ps->velocity, vel, pm->ps->velocity);

	// don't do anything if standing still
	if (!pm->ps->velocity[0] && !pm->ps->velocity[1]) 
	{
		return;
	}

	PM_StepSlideMove( qfalse );
}

/*
===================
PM_LadderMove
===================
*/
static void PM_LadderMove ( void ) 
{
	float	wishspeed;
	float	scale;
	vec3_t	wishdir;
	vec3_t	wishvel;
	float	accelerate;

	if ( PM_CheckJump () )
	{
		return;
	}

	PM_Friction ();

	scale = PM_CmdScale( &pm->cmd );

	accelerate = pm_accelerate;

	//
	// user intentions
	//
	if ( !scale ) 
	{
		wishvel[0] = 0;
		wishvel[1] = 0;
		wishvel[2] = 0;
	} 
	else 
	{
		int i;
		
		VectorNormalize ( pm_ladders[pm->ps->ladder].fwd );
		VectorNormalize ( pml.forward );

		if ( !pml.groundPlane )
		{
			vec3_t	mins;
			vec3_t	maxs;
			vec3_t	offset = {1, 1, 1};
			trace_t tr;

			VectorCopy ( pm->mins, mins );
			VectorSubtract ( mins, offset, mins );
			VectorCopy ( pm->maxs, maxs );
			VectorAdd ( maxs, offset, maxs );

			pm->trace ( &tr, pm->ps->origin, mins, maxs, pm->ps->origin, pm->ps->clientNum, pm->tracemask );
			if ( tr.fraction == 1.0f || !tr.startsolid )
			{
				if ( pm->cmd.forwardmove >= 0 )
				{
					VectorAdd ( pml.forward, pm_ladders[pm->ps->ladder].fwd, pml.forward );
				}
				else
				{
					VectorSubtract ( pml.forward, pm_ladders[pm->ps->ladder].fwd, pml.forward );
				}
			}
		}

		for ( i=0 ; i<3 ; i++ ) 
		{
			wishvel[i] = scale * pml.forward[i]*pm->cmd.forwardmove + scale * pml.right[i]*pm->cmd.rightmove * pm_ladderScale;
		}

		// Duck down ladders
		if ( pm->cmd.upmove < 0 )
		{
			wishvel[2] += scale * pm->cmd.upmove;
		}
	}

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);

	PM_Accelerate( wishdir, wishspeed, accelerate );

	PM_StepSlideMove( qfalse );
}

/*
==============
PM_DeadMove
==============
*/
static void PM_DeadMove( void ) {
	float	forward;

	if ( !pml.walking ) {
		return;
	}

	// extra friction

	forward = VectorLength (pm->ps->velocity);
	forward -= 20;
	if ( forward <= 0 ) {
		VectorClear (pm->ps->velocity);
	} else {
		VectorNormalize (pm->ps->velocity);
		VectorScale (pm->ps->velocity, forward, pm->ps->velocity);
	}
}


/*
===============
PM_NoclipMove
===============
*/
static void PM_NoclipMove( void ) {
	float	speed, drop, friction, control, newspeed;
	int			i;
	vec3_t		wishvel;
	float		fmove, smove;
	vec3_t		wishdir;
	float		wishspeed;
	float		scale;

	pm->ps->viewheight = DEFAULT_VIEWHEIGHT;

	// friction

	speed = VectorLength (pm->ps->velocity);
	if (speed < 1)
	{
		VectorCopy (vec3_origin, pm->ps->velocity);
	}
	else
	{
		drop = 0;

		friction = pm_friction*1.5;	// extra friction
		control = speed < pm_stopspeed ? pm_stopspeed : speed;
		drop += control*friction*pml.frametime;

		// scale the velocity
		newspeed = speed - drop;
		if (newspeed < 0)
			newspeed = 0;
		newspeed /= speed;

		VectorScale (pm->ps->velocity, newspeed, pm->ps->velocity);
	}

	// accelerate
	scale = PM_CmdScale( &pm->cmd );

	scale *= 1.5f;

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.rightmove;
	
	for (i=0 ; i<3 ; i++)
		wishvel[i] = pml.forward[i]*fmove + pml.right[i]*smove;
	wishvel[2] += pm->cmd.upmove;

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);
	wishspeed *= scale;

	PM_Accelerate( wishdir, wishspeed, pm_flyaccelerate );

	// move
	VectorMA (pm->ps->origin, pml.frametime, pm->ps->velocity, pm->ps->origin);
}

//============================================================================

/*
================
PM_FootstepForSurface

Returns an event number apropriate for the groundsurface
================
*/
static void PM_FootstepForSurface( void ) 
{
	if ( pml.groundTrace.surfaceFlags & SURF_NOSTEPS ) 
	{
		return;
	}

	PM_AddEventWithParm(EV_FOOTSTEP, pml.groundTrace.surfaceFlags & MATERIAL_MASK);
}


/*
=================
PM_CrashLand

Check for hard landings that generate sound events
=================
*/

int	minDeltaForDmg				= 97;
int minDeltaForSmallPainSound	= 30;
int minDeltaForBigPainSound		= 97;
int minDeltaForSlowDown			= 17;

static void PM_CrashLand( int impactMaterial, vec3_t impactNormal ) 
{
	float		delta;
	float		dist;
	float		vel, acc;
	float		t;
	float		a, b, c, den;
	float		f;
	int			scaleDelta;
	qboolean	jumped;

	static vec3_t up = {0,0,1};

	// were they juping?
	jumped = (pm->ps->pm_flags&PMF_JUMPING) ? qtrue : qfalse;

	pm->ps->pm_flags &= (~PMF_LADDER_JUMP);
	pm->ps->pm_flags &= (~PMF_JUMPING);

	// calculate the exact velocity on landing
	dist = pm->ps->origin[2] - pml.previous_origin[2];
	vel = pml.previous_velocity[2];
	acc = -pm->ps->gravity;

	a = acc / 2;
	b = vel;
	c = -dist;

	den =  b * b - 4 * a * c;
	if ( den < 0 ) 
	{
		return;
	}

	t = (-b - sqrt( den ) ) / ( 2 * a );

	delta = vel + t * acc;
	delta = delta * delta * 0.000275f;

	switch ( pm->waterlevel )
	{
		case 3:
			// never take falling damage if completely underwater
			return;

		// reduce falling damage if there is standing water
		case 2:
			delta *= 0.25;
			break;

		// reduce falling damage if there is standing water
		case 1:
			delta *= 0.5;
			break;
	}

	// Scale the delta based on the normal of the plane we hit
	f = DotProduct ( up, impactNormal );
	if ( f < .25 )
	{
		delta *= f;
	}

	// Just hit the ground, no more z velocity or we could bounce
	pm->ps->velocity[2] = 0;

	// start footstep cycle over if it wasnt a little jump
	if ( delta > minDeltaForSlowDown )
	{
		pm->ps->bobCycle = 0;
	}

	if ( delta < 1 ) 
	{
		return;
	}
	else if ( jumped && delta >= minDeltaForSlowDown )
	{
		// Cut their forward velocity, this pretty much eliminates strafe jumping
 		pm->ps->velocity[0] *= 0.25f;
		pm->ps->velocity[1] *= 0.25f;

		pm->ps->pm_time = 500;
	}

	// create a local entity event to play the sound

	// SURF_NODAMAGE is used for bounce pads where you don't ever
	// want to take damage or play a crunch sound
	scaleDelta = (int)delta;
	if (scaleDelta > 100 + minDeltaForDmg)
	{
		scaleDelta = 100 + minDeltaForDmg;
	}
	scaleDelta -= minDeltaForDmg;
	if ( !(pml.groundTrace.surfaceFlags & SURF_NODAMAGE) )  
	{
		if ( delta > minDeltaForBigPainSound ) 
		{
			PM_AddEventWithParm(EV_FALL_FAR, scaleDelta | ((impactMaterial & MATERIAL_MASK)<<8));
		} 
		else if ( delta > minDeltaForDmg ) 
		{
			// this is a pain grunt, so don't play it if dead
			if ( pm->ps->stats[STAT_HEALTH] > 0 ) 
			{
				PM_AddEventWithParm(EV_FALL_MEDIUM, scaleDelta | ((impactMaterial & MATERIAL_MASK)<<8));
			}
		} 
		else if ( delta > minDeltaForSlowDown ) 
		{
			PM_AddEventWithParm(EV_FALL_SHORT, impactMaterial & MATERIAL_MASK );
		} 
		else if ( delta > 10 && (pm->cmd.buttons & BUTTON_WALKING) )
		{
			PM_FootstepForSurface();
		}
	}
}

/*
=============
PM_CorrectAllSolid
=============
*/
static int PM_CorrectAllSolid( trace_t *trace ) 
{
	int			i, j, k;
	vec3_t		point;

	if ( pm->debugLevel ) 
	{
		Com_Printf("%i:allsolid\n", c_pmove);
	}

	// jitter around
	for (i = -1; i <= 1; i++) {
		for (j = -1; j <= 1; j++) {
			for (k = -1; k <= 1; k++) {
				VectorCopy(pm->ps->origin, point);
				point[0] += (float) i;
				point[1] += (float) j;
				point[2] += (float) k;
				pm->trace (trace, point, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
				if ( !trace->allsolid ) {
					point[0] = pm->ps->origin[0];
					point[1] = pm->ps->origin[1];
					point[2] = pm->ps->origin[2] - 0.25;

					pm->trace (trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
					pml.groundTrace = *trace;
					return qtrue;
				}
			}
		}
	}

	pm->ps->groundEntityNum = ENTITYNUM_NONE;
	pml.groundPlane = qfalse;
	pml.walking = qfalse;

	return qfalse;
}


/*
=============
PM_GroundTraceMissed

The ground trace didn't hit a surface, so we are in freefall
=============
*/
static void PM_GroundTraceMissed( void ) 
{
	trace_t		trace;
	vec3_t		point;

	if ( pm->ps->groundEntityNum != ENTITYNUM_NONE ) 
	{
		// we just transitioned into freefall
		if ( pm->debugLevel ) 
		{
			Com_Printf("%i:lift\n", c_pmove);
		}

		// if they aren't in a jumping animation and the ground is a ways away, force into it
		// if we didn't do the trace, the player would be backflipping down staircases
		VectorCopy( pm->ps->origin, point );
		point[2] -= 64;

		pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
		if ( trace.fraction == 1.0 ) 
		{
			if ( pm->cmd.forwardmove >= 0 ) 
			{
				pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
			} 
			else 
			{
				pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
			}
		}
	}

	pm->ps->groundEntityNum = ENTITYNUM_NONE;
	pml.groundPlane = qfalse;
	pml.walking = qfalse;
}


/*
=============
PM_GroundTrace
=============
*/
static void PM_GroundTrace( void ) 
{
	vec3_t		point;
	trace_t		trace;
	float		minWalkNormal;

	point[0] = pm->ps->origin[0];
	point[1] = pm->ps->origin[1];
	point[2] = pm->ps->origin[2] - 0.25;

	pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
	pml.groundTrace = trace;

	// When stuck to antoher player set a flag to let the trigger code know so it can unstick the player
	if ( (trace.allsolid || trace.startsolid) && trace.entityNum < MAX_CLIENTS ) 
	{
		pm->ps->pm_flags |= PMF_SIAMESETWINS;
	}

	// if the trace didn't hit anything, we are in free fall
	if ( trace.fraction == 1.0 ) {
		PM_GroundTraceMissed();
		pml.groundPlane = qfalse;
		pml.walking = qfalse;
		return;
	}

	// check if getting thrown off the ground
	if ( pm->ps->velocity[2] > 0 && DotProduct( pm->ps->velocity, trace.plane.normal ) > 10 ) {
		if ( pm->debugLevel ) {
			Com_Printf("%i:kickoff\n", c_pmove);
		}
		// go into jump animation
		if ( pm->cmd.forwardmove >= 0 ) {
			PM_ForceLegsAnim( pm->ps, LEGS_JUMP );
			pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
		} else {
			PM_ForceLegsAnim( pm->ps, LEGS_JUMP_BACK );
			pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
		}

		pm->ps->groundEntityNum = ENTITYNUM_NONE;
		pml.groundPlane = qfalse;
		pml.walking = qfalse;
		return;
	}
	
	// slopes that are too steep will not be considered onground
	if ( trace.contents & CONTENTS_TERRAIN )
	{
		minWalkNormal = MIN_WALK_NORMAL_TERRAIN;
	}
	else
	{
		minWalkNormal = MIN_WALK_NORMAL;
	}

	if ( trace.plane.normal[2] < minWalkNormal ) 
	{
		if ( pm->debugLevel ) 
		{
			Com_Printf("%i:steep\n", c_pmove);
		}

		// FIXME: if they can't slide down the slope, let them
		// walk (sharp crevices)
		pm->ps->groundEntityNum = ENTITYNUM_NONE;
		pml.groundPlane = qtrue;
		pml.walking = qfalse;
		return;
	}

	pml.groundPlane = qtrue;
	pml.walking = qtrue;

	// hitting solid ground will end a waterjump
	if (pm->ps->pm_flags & PMF_TIME_WATERJUMP)
	{
		pm->ps->pm_flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND);
		pm->ps->pm_time = 0;
	}

	if ( pm->ps->groundEntityNum == ENTITYNUM_NONE) 
	{
		// just hit the ground
//		if ((pml.groundTrace.contents & CONTENTS_TERRAIN) && pml.previous_velocity[2] > -200)
//		{
//		}
//		else
		{
			if ( pm->debugLevel ) 
			{
				Com_Printf("%i:Land\n", c_pmove);
			}
			
			PM_CrashLand(trace.surfaceFlags & MATERIAL_MASK, trace.plane.normal );

			// don't do landing time if we were just going down a slope
			if ( pml.previous_velocity[2] < -200 ) 
			{
				// don't allow another jump for a little while
				pm->ps->pm_flags |= PMF_TIME_LAND;
				pm->ps->pm_time = 250;
			}
		}
	}

	pm->ps->groundEntityNum = trace.entityNum;

	// don't reset the z velocity for slopes
//	pm->ps->velocity[2] = 0;

	PM_AddTouchEnt( trace.entityNum );
}


/*
=============
PM_SetWaterLevel	FIXME: avoid this twice?  certainly if not moving
=============
*/
static void PM_SetWaterLevel( void ) 
{
	vec3_t		point;
	int			cont;
	int			sample1;
	int			sample2;

	//
	// get waterlevel, accounting for ducking
	//
	pm->waterlevel = 0;
	pm->watertype  = 0;

	point[0] = pm->ps->origin[0];
	point[1] = pm->ps->origin[1];
	point[2] = pm->ps->origin[2] + pm->mins[2];	
	cont = pm->pointcontents( point, pm->ps->clientNum );

	// See if we are on a ladder too
	if ( !(pm->ps->pm_flags&PMF_LADDER_JUMP) && (cont & CONTENTS_LADDER) )
	{
		if ( pm->ps->ladder == -1 )
		{
	 		pm->ps->ladder = BG_FindLadder ( pm->ps->origin );
		}

		pm->ps->pm_flags |= PMF_LADDER;
	}
	else
	{
		pm->ps->ladder = -1;
		pm->ps->pm_flags &= ~PMF_LADDER;
	}

	if ( cont & MASK_WATER ) 
	{
		sample2 = pm->ps->viewheight - MINS_Z;
		sample1 = sample2 / 2;

		pm->watertype = cont;
		pm->waterlevel = 1;
		point[2] = pm->ps->origin[2] + MINS_Z + sample1;
		cont = pm->pointcontents (point, pm->ps->clientNum );
		if ( cont & MASK_WATER ) 
		{
			pm->waterlevel = 2;
			point[2] = pm->ps->origin[2] + MINS_Z + sample2;
			cont = pm->pointcontents (point, pm->ps->clientNum );
			if ( cont & MASK_WATER )
			{
				pm->waterlevel = 3;
			}
		}
	}
}

/*
==============
PM_CheckCrouchJump

Handles crouch jumping
==============
*/
static void PM_CheckCrouchJump ( void )
{
	// Already crouch jumping so check to see if its over
	if ( pm->ps->pm_flags & PMF_CROUCH_JUMP )
	{
		// If they are on the ground the crouch jump is over
		if ( pml.groundPlane )
		{
			pm->ps->pm_flags &= ~PMF_CROUCH_JUMP;
		}
	}
	else
	{
		// If not on the ground and still heading up then crouch jump is possible.
		if ( !pml.groundPlane && (pm->ps->pm_flags & PMF_JUMPING) && pm->cmd.upmove < 0 )
		{
			pm->ps->pm_flags |= PMF_CROUCH_JUMP;
		}
	}

	// Check again if still crouch jumping and if so alter the view height
	// so the client doesnt look like they are ducking in mid air
	if ( (pm->ps->pm_flags & PMF_CROUCH_JUMP) && (pm->ps->pm_flags & PMF_JUMPING) )
	{
		// If still ducked look for windows
		if ( pm->ps->pm_flags & PMF_DUCKED )
		{
			trace_t trace;
			vec3_t  maxs;

			VectorCopy ( pm->maxs, maxs );
			maxs[2] = DEFAULT_PLAYER_Z_MAX;

			pm->trace (&trace, pm->ps->origin, pm->mins, maxs, pm->ps->origin, pm->ps->clientNum, pm->tracemask );
			if ( !(trace.allsolid || trace.startsolid) )
			{
				pm->ps->viewheight = DEFAULT_VIEWHEIGHT;
			}
		}
	}
}

/*
==============
PM_CheckDuck

Sets mins, maxs, and pm->ps->viewheight
==============
*/
static void PM_CheckDuck (void)
{
	trace_t	trace;

	if ( (pm == 0) || (pm->ps == 0) )
	{
		return;
	}

	pm->mins[0] = -15;
	pm->mins[1] = -15;

	pm->maxs[0] = 15;
	pm->maxs[1] = 15;

	pm->mins[2] = MINS_Z;
	
	if (pm->ps->pm_type == PM_DEAD)
	{
		pm->maxs[2] = DEAD_PLAYER_Z_MAX;
		pm->ps->viewheight = DEAD_VIEWHEIGHT;
		return;
	}

	// duck or prone
	if (pm->cmd.upmove < 0)
	{
		// assume ducked at first
		pm->ps->pm_flags |= PMF_DUCKED;		
	}
	else
	{	// stand up if possible
		if ( pm->ps->pm_flags & PMF_DUCKED )
		{
			pm->maxs[2] = DEFAULT_PLAYER_Z_MAX;
			pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, pm->ps->origin, pm->ps->clientNum, pm->tracemask );
			if (!trace.allsolid)
			{
				pm->ps->pm_flags &= ~PMF_DUCKED;
			}
		}
	}

	if ( (pm->ps->pm_flags & PMF_DUCKED) )
	{
		pm->maxs[2] = CROUCH_PLAYER_Z_MAX;
		pm->ps->viewheight = CROUCH_VIEWHEIGHT;
	}
	else
	{
		pm->maxs[2] = DEFAULT_PLAYER_Z_MAX;
		pm->ps->viewheight = DEFAULT_VIEWHEIGHT;
	}
}

/*
===============
PM_Footsteps
===============
*/
static void PM_Footsteps( void ) 
{
	float		bobmove;
	int			old;
	qboolean	footstep;

	//
	// calculate speed and cycle to be used for
	// all cyclic walking effects
	//
	pm->xyspeed = sqrt( pm->ps->velocity[0] * pm->ps->velocity[0]
		+  pm->ps->velocity[1] * pm->ps->velocity[1] );

	if ( pm->ps->groundEntityNum == ENTITYNUM_NONE ) 
	{

		if ( pm->ps->pm_flags & PMF_LADDER )
		{
		}
		else
		{
		// airborne leaves position in cycle intact, but doesn't advance
			if ( pm->waterlevel > 1 ) 
			{
				PM_ContinueLegsAnim( pm->ps, LEGS_SWIM );
			}
			return;
		}
	}

	// if not trying to move
	if ( !pm->cmd.forwardmove && !pm->cmd.rightmove ) 
	{
		if (  pm->xyspeed < 5 ) 
		{
			pm->ps->bobCycle = 0;	// start at beginning of cycle again
			
			if ( pm->ps->pm_flags & PMF_DUCKED ) 
			{
				if ( pm->ps->leanTime - LEAN_TIME < 0 )
				{
					PM_ContinueLegsAnim( pm->ps, LEGS_LEAN_CROUCH_LEFT );	
				}
				else if ( pm->ps->leanTime - LEAN_TIME > 0 )
				{
					PM_ContinueLegsAnim( pm->ps, LEGS_LEAN_CROUCH_RIGHT );	
				}
				else
				{
					PM_ContinueLegsAnim( pm->ps, LEGS_IDLE_CROUCH );
				}
			}		
			else 
			{
				if ( pm->ps->leanTime - LEAN_TIME < 0 )
				{
					PM_ContinueLegsAnim( pm->ps, LEGS_LEAN_LEFT );	
				}
				else if ( pm->ps->leanTime - LEAN_TIME > 0 )
				{
					PM_ContinueLegsAnim( pm->ps, LEGS_LEAN_RIGHT );	
				}
				else
				{
					PM_ContinueLegsAnim( pm->ps, TORSO_IDLE_PISTOL );
				}
			}
		}
		return;
	}
	

	footstep = qfalse;

	if ( (pm->ps->pm_flags & PMF_DUCKED) && (pm->ps->groundEntityNum != ENTITYNUM_NONE ) ) 
	{
		bobmove = 0.25;	// ducked characters bob much faster
		
		if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN ) 
		{
			PM_ContinueLegsAnim( pm->ps, LEGS_WALK_CROUCH_BACK );
		}
		else 
		{
			if ( pm->ps->leanTime - LEAN_TIME < 0 )
			{
				if ( pm->cmd.rightmove > 0 )
				{
					PM_ContinueLegsAnim( pm->ps, LEGS_LEANLEFT_CROUCH_WALKRIGHT );	
				}
				else
				{
					PM_ContinueLegsAnim( pm->ps, LEGS_LEANLEFT_CROUCH_WALKLEFT );	
				}
			}
			else if ( pm->ps->leanTime - LEAN_TIME > 0 )
			{
				if ( pm->cmd.rightmove > 0 )
				{
					PM_ContinueLegsAnim( pm->ps, LEGS_LEANRIGHT_CROUCH_WALKRIGHT );	
				}
				else
				{
					PM_ContinueLegsAnim( pm->ps, LEGS_LEANRIGHT_CROUCH_WALKLEFT );	
				}
			}
			else
			{
				PM_ContinueLegsAnim( pm->ps, LEGS_WALK_CROUCH );
			}
		}
	} 
	else 
	{
		if ( !( pm->cmd.buttons & BUTTON_WALKING ) )
		{
			PM_BeginZoomOut ( );

			bobmove = 0.4f;	// faster speeds bob faster
			if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN ) 
			{
				PM_ContinueLegsAnim( pm->ps, LEGS_RUN_BACK );
			}
			else 
			{
				PM_ContinueLegsAnim( pm->ps, LEGS_RUN );
			}
			footstep = qtrue;
		} 
		else 
		{
			bobmove = 0.3f;	// walking bobs slow
			if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN ) 
			{
				PM_ContinueLegsAnim( pm->ps, LEGS_WALK_BACK );
			}
			else 
			{
				if ( pm->ps->leanTime - LEAN_TIME < 0 )
				{
					if ( pm->cmd.rightmove > 0 )
					{
						PM_ContinueLegsAnim( pm->ps, LEGS_LEANLEFT_WALKRIGHT );	
					}
					else
					{
						PM_ContinueLegsAnim( pm->ps, LEGS_LEANLEFT_WALKLEFT );	
					}
				}
				else if ( pm->ps->leanTime - LEAN_TIME > 0 )
				{
					if ( pm->cmd.rightmove > 0 )
					{
						PM_ContinueLegsAnim( pm->ps, LEGS_LEANRIGHT_WALKRIGHT );	
					}
					else
					{
						PM_ContinueLegsAnim( pm->ps, LEGS_LEANRIGHT_WALKLEFT );	
					}
				}
				else
				{
					PM_ContinueLegsAnim( pm->ps, LEGS_WALK );
				}
			}
		}
	}

	// check for footstep / splash sounds
	old = pm->ps->bobCycle;
	pm->ps->bobCycle = (int)( old + bobmove * pml.msec ) & 255;

	// if we just crossed a cycle boundary, play an apropriate footstep event
	if ( ( ( old + 64 ) ^ ( pm->ps->bobCycle + 64 ) ) & 128 ) 
	{
		if ( pm->waterlevel == 0 ) 
		{
			// on ground will only play sounds if running
			if ( footstep && !pm->noFootsteps ) 
			{
				PM_FootstepForSurface();
			}
		} 
		else if ( pm->waterlevel == 1 ) 
		{
			// splashing
			PM_AddEvent( EV_WATER_FOOTSTEP );
		}
		else if ( pm->waterlevel == 2 ) 
		{
			// wading / swimming at surface
			PM_AddEvent( EV_SWIM );
		} 
		else if ( pm->waterlevel == 3 ) 
		{
			// no sound when completely underwater
		}
	}
}

/*
==============
PM_WaterEvents

Generate sound events for entering and leaving water
==============
*/
static void PM_WaterEvents( void ) 
{
	if ( !pml.previous_waterlevel && pm->waterlevel == 1 )
	{
		PM_AddEvent( EV_WATER_TOUCH );
	}
	else if ( pml.previous_waterlevel <= 1 && pm->waterlevel > 1 )
	{
		if ( pm->ps->velocity[2] < -100 )
		{
			PM_AddEvent( EV_WATER_LAND );
		}
	}

	//
	// check for head just coming out of water
	//
	if (pml.previous_waterlevel == 3 && pm->waterlevel != 3) {
		PM_AddEvent( EV_WATER_CLEAR );
	}
}

/*
===============
PM_SetWeaponTime
===============
*/
static void PM_SetWeaponTime ( TAnimWeapon *aW )
{	
	TAnimInfoWeapon *aIW;

	if(!aW)
	{
		assert(0);
	}

	// Weapon model info tells us how long the anim is
	aIW = aW->mWeaponModelInfo;
	if ( !aIW )
	{
		return;
	}
	if(current_gametype.value == GT_HS){
		if  (pm->ps->weapon == WP_KNIFE && (pm->ps->stats[STAT_GAMETYPE_ITEMS]) ) // Henk 14/01/11 -> Speedy briefcase knife -.-''
		{
			pm->ps->weaponTime = (500.0f / aIW->mFPS[0] * aIW->mNumFrames[0] / aIW->mSpeed); /// 1.3;
		}
		else
		{
			pm->ps->weaponTime = 1000.0f / aIW->mFPS[0] * aIW->mNumFrames[0] / aIW->mSpeed;
		}
	}else
	pm->ps->weaponTime = 1000.0f / aIW->mFPS[0] * aIW->mNumFrames[0] / aIW->mSpeed;
	pm->ps->weaponAnimTime = pm->ps->weaponTime;
}

/*
===============
BG_GetWeaponNote
===============
*/
TNoteTrack *BG_GetWeaponNote( playerState_t* ps, int weapon, int anim, int animChoice, int callbackStep )
{
	TAnimWeapon		*aW;
	TAnimInfoWeapon *aIW;
	TNoteTrack		*note;
	int				n=0;

	note = NULL;
	aW=BG_GetInviewAnimFromIndex( weapon, anim&~ANIM_TOGGLEBIT);
	if (!aW)
	{
		return 0;
	}

	aIW = aW->mWeaponModelInfo;
	if ( !aIW )
	{
		return 0;
	}

	// Find the callback for the given step
	for ( note = aIW->mNoteTracks[ps->weaponAnimIdChoice], n=0; note && n < callbackStep; note = note->mNext, n++ )
	{
		// Do nothing, loop does it all
	}

	return(note);
}

/*
==============
PM_CheckWeaponNotes
==============
*/
void PM_CheckWeaponNotes ( void )
{
	playerState_t	*ps;
	TAnimWeapon		*aW;
	TAnimInfoWeapon *aIW;
	TNoteTrack		*note;
	int				step;
	int				stepTime;

	pm->ps->weaponCallbackTime += pml.msec;	

	// 1st note step.
	step = 0;
	ps   = pm->ps;
	aW   = BG_GetInviewAnimFromIndex ( ps->weapon, (ps->weaponAnimId&~ANIM_TOGGLEBIT) );
	#ifdef Q3_VM
	assert ( aW );
	#endif
	if ( !aW )
	{
		return;
	}

	// Get the cached weapon model info
	aIW = aW->mWeaponModelInfo;
	if ( !aIW )
	{
		return;
	}

	stepTime = 1000.0f / aIW->mFPS[pm->ps->weaponAnimIdChoice] / aIW->mSpeed;;
	note     = aIW->mNoteTracks[pm->ps->weaponAnimIdChoice];

	while(note)
	{
		if( pm->ps->weaponCallbackTime >= note->mFrame * stepTime )
		{
			if(step > pm->ps->weaponCallbackStep)
			{			
				if( !Q_stricmp("fire",note->mNote) || !Q_stricmp("altfire",note->mNote) )
				{
					if(pm->ps->weaponstate==WEAPON_FIRING)
					{
						int seed;
						
						// Update the seed
						seed = pm->ps->stats[STAT_SEED];
						Q_rand ( &seed );
						seed = seed & 0xFFFF;
						pm->ps->stats[STAT_SEED] = seed;

						PM_AddEvent(EV_FIRE_WEAPON);
						PM_Weapon_AddInaccuracy(ATTACK_NORMAL);
						PM_Weapon_AddKickAngles(weaponData[pm->ps->weapon].attack[ATTACK_NORMAL].maxKickAngles);


					}
					else if(pm->ps->weaponstate==WEAPON_FIRING_ALT)
					{
						PM_AddEvent(EV_ALT_FIRE);
						PM_Weapon_AddKickAngles(weaponData[pm->ps->weapon].attack[ATTACK_ALTERNATE].maxKickAngles);
					}							
				}

				PM_AddEventWithParm ( EV_WEAPON_CALLBACK, 
									  ((step&0xFF) << 24) + 
									  ((pm->ps->weaponAnimIdChoice&0xFF)<<16) + 
									  (((ps->weaponAnimId&~ANIM_TOGGLEBIT)&0xFF)<<8) + 
									  pm->ps->weapon);	

				pm->ps->weaponCallbackStep=step;
			}
		}

		step++;					
		note=note->mNext;
	}
}

/*
===============
PM_SetWeaponAnimChoice
===============
*/
void PM_SetWeaponAnimChoice(TAnimWeapon *aW)
{
	TAnimInfoWeapon *aIW;

	if(!aW)
	{
	#ifdef Q3_VM
		assert(0);
	#endif
		return;
	}
	
	// Get the cached weapon model info
	aIW = aW->mWeaponModelInfo;
	if ( !aIW )
	{
		return;
	}

	pm->ps->weaponAnimIdChoice = rand()%aIW->mNumChoices;				
}

/*
===============
PM_GetAnimFromName
===============
*/
TAnimWeapon* PM_GetAnimFromName ( char *animName, playerState_t *ps, int *animIndex )
{
	TAnimWeapon		*aW=0;
	TAnimInfoWeapon *aIW=0;
	char			tempname[MAX_QPATH];

	switch(ps->weapon)
	{
		case WP_KNIFE:
			if(!strcmp(animName,"charge"))
			{
				// Get 'prefire' anim.
				aW=BG_GetInviewAnim(pm->ps->weapon,"prefire",animIndex);
				PM_SetWeaponAnimChoice(aW);
			}
			else if(!strcmp(animName,"fire"))
			{
				aW=BG_GetInviewAnimFromIndex(ps->weapon,ps->weaponAnimId&~ANIM_TOGGLEBIT);
				if((!strcmp(aW->mName,"prefire"))||strstr(aW->mName,"firetrans"))
				{
					// Get 'fire' anim.
					aW=BG_GetInviewAnim(ps->weapon,"fire",animIndex);
					PM_SetWeaponAnimChoice(aW);
				}
				else if(!strcmp(aW->mName,"fire"))
				{
					// Get 'firetrans' anim. We don't call PM_SetWeaponAnimChoice()
					// because the firetrans anims are matched to the fire anims.
					aIW=BG_GetInviewModelAnim(ps->weapon,"weaponmodel","fire");
					strcpy(tempname,aIW->mTransition[ps->weaponAnimIdChoice]);
					aW=BG_GetInviewAnim(ps->weapon,tempname,animIndex);
				}
				else
				{
					// Get 'prefire' anim.
					aW=BG_GetInviewAnim(pm->ps->weapon,"fire",animIndex);
					PM_SetWeaponAnimChoice(aW);
				}
			}
			else if(!strcmp(animName,"fireend"))
			{
				aW=BG_GetInviewAnim(ps->weapon,"fireend1",animIndex);
				PM_SetWeaponAnimChoice(aW);
			}
			else
			{
				// Nothing clever about the other sequences.
				aW=BG_GetInviewAnim(pm->ps->weapon,animName,animIndex);
				PM_SetWeaponAnimChoice(aW);
			}
			break;

		case WP_MM1_GRENADE_LAUNCHER:
		case WP_M590_SHOTGUN:
			if(!strcmp(animName,"reload"))
			{
				aW=BG_GetInviewAnimFromIndex(ps->weapon,ps->weaponAnimId&~ANIM_TOGGLEBIT);
				if(!strcmp(aW->mName,"reloadbegin")||!strcmp(aW->mName,"reloadshell"))
				{
					// Get 'reloadshell' anim.
					aW=BG_GetInviewAnim(ps->weapon,"reloadshell",animIndex);
				}
				else
				{
					// Get 'reloadbegin' anim.
					aW=BG_GetInviewAnim(pm->ps->weapon,"reloadbegin",animIndex);
				}
			}
			else if(!strcmp(animName,"reloadend"))
			{
				// Get 'reloadend' anim.
				aW=BG_GetInviewAnim(ps->weapon,"reloadend",animIndex);
			}
			else
			{
				// Nothing clever about the other sequences.
				aW=BG_GetInviewAnim(pm->ps->weapon,animName,animIndex);
			}
			PM_SetWeaponAnimChoice(aW);
			break;

		case WP_M67_GRENADE:
		case WP_M84_GRENADE:
		case WP_F1_GRENADE:
		case WP_L2A2_GRENADE:
		case WP_MDN11_GRENADE:
		case WP_SMOHG92_GRENADE:
		case WP_ANM14_GRENADE:
		case WP_M15_GRENADE:
			if(!strcmp(animName,"charge"))
			{
				// Get 'throwbegin' anim.
				aW=BG_GetInviewAnim(ps->weapon,"throwbegin",animIndex);
			}
			else if(!strcmp(animName,"altcharge"))
			{
				// Get 'throwbegin' anim.
				aW=BG_GetInviewAnim(ps->weapon,"altthrowbegin",animIndex);
			}
			else if(!strcmp(animName,"fire"))
			{
				// Get 'throwend' anim.
				aW=BG_GetInviewAnim(ps->weapon,"throwend",animIndex);
			}
			else if(!strcmp(animName,"altfire"))
			{
				// Get 'throwend' anim.
				aW=BG_GetInviewAnim(ps->weapon,"altthrowend",animIndex);
			}
			else
			{
				// Nothing clever about the other sequences.
				aW=BG_GetInviewAnim(pm->ps->weapon,animName,animIndex);
			}
			PM_SetWeaponAnimChoice(aW);
			break;

		default:
			// Other weapons don't do anything fancy.
			aW=BG_GetInviewAnim(ps->weapon,animName,animIndex);
			PM_SetWeaponAnimChoice(aW);
			break;
	}

	return(aW);
}

/*
===============
BG_GetWeaponAnim
===============
*/
TAnimWeapon *BG_GetWeaponAnim(int weaponAction,playerState_t *ps,int *animIndex)
{
	TAnimWeapon *aW=0;

	switch(weaponAction)
	{
		case WACT_READY:
			aW=PM_GetAnimFromName("ready",ps,animIndex);
			break;
		case WACT_IDLE:
			aW=PM_GetAnimFromName("idle",ps,animIndex);
			break;
		case WACT_FIRE:
			aW=PM_GetAnimFromName("fire",ps,animIndex);
			break;
		case WACT_FIRE_END:
			aW=PM_GetAnimFromName("fireend",ps,animIndex);
			break;
		case WACT_ALTFIRE:
			aW=PM_GetAnimFromName("altfire",ps,animIndex);
			break;
		case WACT_ALTFIRE_END:
			aW=PM_GetAnimFromName("altfireend",ps,animIndex);
			break;
		case WACT_RELOAD:
			aW=PM_GetAnimFromName("reload",ps,animIndex);
			break;
		case WACT_ALTRELOAD:
			aW=PM_GetAnimFromName("altreload",ps,animIndex);
			break;
		case WACT_RELOAD_END:
			aW=PM_GetAnimFromName("reloadend",ps,animIndex);
			break;
		case WACT_PUTAWAY:	
			aW=PM_GetAnimFromName("done",ps,animIndex);
			break;
		case WACT_ZOOMIN:
			aW=PM_GetAnimFromName("zoomin",ps,animIndex);
			break;
		case WACT_ZOOMOUT:	
			aW=PM_GetAnimFromName("zoomout",ps,animIndex);
			break;
		case WACT_CHARGE:
			aW=PM_GetAnimFromName("charge",ps,animIndex);
			break;
		case WACT_ALTCHARGE:
			aW=PM_GetAnimFromName("altcharge",ps,animIndex);
			break;
		default:
			Com_Printf("Anim unknown: %i\n",weaponAction);
			break;
	}

	return(aW);
}

/*
===============
PM_HandleWeaponAction
===============
*/
static void PM_HandleWeaponAction(int weaponAction)
{
	TAnimWeapon *aW;
	int			animIndex;

	aW = BG_GetWeaponAnim ( weaponAction, pm->ps, &animIndex );
	if(!aW)
	{
		return;
	}
	
	// Reset callback timer. We have to account for any remaining weapontime
	// or we could miss off callbacks during sequences that are short in duration.
	pm->ps->weaponCallbackTime = -pm->ps->weaponTime;
	pm->ps->weaponCallbackStep = -1;

	PM_SetWeaponTime(aW);
	if((pm->ps->weaponAnimId&~ANIM_TOGGLEBIT)==animIndex)
	{
		animIndex=pm->ps->weaponAnimId^ANIM_TOGGLEBIT;
	}

	pm->ps->weaponAnimId = animIndex;
}

/*
===============
PM_BeginZoomIn
===============
*/
static void PM_BeginZoomIn(void)
{
	// Reset the zom fov if not rezooming
	if ( !(pm->ps->pm_flags & PMF_ZOOM_REZOOM) )
	{
		pm->ps->zoomFov = 0;
	}

	pm->ps->weaponstate=WEAPON_ZOOMIN;
	PM_HandleWeaponAction(WACT_ZOOMIN);
}

/*
===============
PM_BeginZoomOut
===============
*/
static void PM_BeginZoomOut(void)
{
	if ( !(pm->ps->pm_flags & PMF_ZOOMED) )
	{
		return;
	}

	if ( !(pm->ps->pm_flags & PMF_ZOOM_DEFER_RELOAD ) )
	{
		pm->ps->zoomFov = 0;
	}

	pm->ps->weaponstate=WEAPON_ZOOMOUT;
	PM_HandleWeaponAction(WACT_ZOOMOUT);
	pm->ps->zoomTime=pm->ps->commandTime;
	pm->ps->pm_flags &= ~(PMF_ZOOM_LOCKED|PMF_ZOOM_REZOOM|PMF_ZOOMED);
}


/*
===============
PM_BeginWeaponChange
===============
*/
static void PM_BeginWeaponChange(int weapon)
{
	if ( weapon <= WP_NONE || weapon >= WP_NUM_WEAPONS )
	{
		return;
	}

	if ( !( pm->ps->stats[STAT_WEAPONS] & ( 1 << weapon ) ) )
	{
		return;
	}
	
	if ( pm->ps->weaponstate == WEAPON_DROPPING )
	{
		return;
	}

	// Dont allow switching to the weapon the client is already using
	if ( pm->ps->weapon == weapon )
	{
		PM_AddEvent(EV_CHANGE_WEAPON_CANCELLED );

		if ( pm->ps->weaponTime <= 0 )
		{
			// Add a little delay so it doenst fire because this was caused by
			// the menu selection
			pm->ps->weaponTime = 150;
		}

		return;
	}

	// turn off any kind of zooming when weapon switching.
	if( pm->ps->pm_flags & PMF_ZOOMED )
	{
		pm->ps->zoomFov	  = 0;
		pm->ps->zoomTime  = pm->ps->commandTime;
		pm->ps->pm_flags &= ~(PMF_ZOOM_FLAGS);
	}

	// Clear the weapon time
	pm->ps->weaponTime			 = 0;
	pm->ps->weaponFireBurstCount = 0;
	pm->ps->weaponAnimTime		 = 0;

	PM_AddEvent(EV_CHANGE_WEAPON);
	pm->ps->weaponstate = WEAPON_DROPPING;

	if( pm->ps->weapon >= WP_M67_GRENADE && pm->ps->weapon <= WP_M15_GRENADE && pm->ps->clip[ATTACK_NORMAL][pm->ps->weapon] <= 0 )
	{
		// We don't want to play the 'putaway' anim for the grenades if we are out of grenades!
		return;
	}

	PM_HandleWeaponAction(WACT_PUTAWAY);

	PM_StartTorsoAnim( pm->ps, weaponData[pm->ps->weapon].animDrop, pm->ps->weaponAnimTime );
}

/*
===============
PM_FinishWeaponChange
===============
*/
static void PM_FinishWeaponChange( void )
{
	int	weapon;

	weapon = pm->cmd.weapon & ~WP_DELAYED_CHANGE_BIT;

	if( weapon < WP_NONE || weapon >= WP_NUM_WEAPONS )
	{
		weapon = WP_KNIFE;
	}

	if(!( pm->ps->stats[STAT_WEAPONS] & ( 1 << weapon ) ) )
	{
		weapon = WP_KNIFE;
	}

	PM_AddEvent(EV_READY_WEAPON);
	
	pm->ps->weapon			= weapon;
	pm->ps->weaponstate		= WEAPON_RAISING;
	pm->ps->weaponTime		= 0;
	pm->ps->weaponAnimTime	= 0;

	// Default to auto (or next available fire mode).
	if ( pm->ps->firemode[pm->ps->weapon] == WP_FIREMODE_NONE )
	{
		pm->ps->firemode[pm->ps->weapon] = BG_FindFireMode ( pm->ps->weapon, ATTACK_NORMAL, WP_FIREMODE_AUTO );
	}

	// We don't want to play the 'takeout' anim for the grenades if we are about to reload anyway
	if( pm->ps->weapon >= WP_M67_GRENADE && pm->ps->weapon <= WP_M15_GRENADE && pm->ps->clip[ATTACK_NORMAL][pm->ps->weapon] <=0 )
	{
		return;
	}

	PM_HandleWeaponAction(WACT_READY);

	pm->ps->weaponTime = min(150,pm->ps->weaponTime);

	PM_StartTorsoAnim( pm->ps, weaponData[pm->ps->weapon].animRaise, pm->ps->weaponAnimTime ); // Henk 12/04/10 -> Faster weapon switch
}

/*
==============
PM_LoadShell

Only used for the M590 shotgun.
==============
*/
void PM_LoadShell(void)
{
	pm->ps->clip[ATTACK_NORMAL][pm->ps->weapon]++;
	pm->ps->ammo[weaponData[pm->ps->weapon].attack[ATTACK_NORMAL].ammoIndex]--;
}

/*
==============
PM_StartRefillClip
==============
*/
void PM_StartRefillClip ( attackType_t attack )
{
	int	extra;

	assert ( attack >= ATTACK_NORMAL && attack < ATTACK_MAX );

	// Sniper rifle should unzoom first before reloading.
	if( pm->ps->pm_flags & PMF_ZOOMED )
	{
		pm->ps->pm_flags |= PMF_ZOOM_DEFER_RELOAD;
		PM_BeginZoomOut();
		return;
	}

	pm->ps->weaponstate=(attack==ATTACK_ALTERNATE)?WEAPON_RELOADING_ALT:WEAPON_RELOADING;
	pm->ps->weaponFireBurstCount=0;

	if(pm->ps->weapon!=WP_KNIFE)
	{
		// We don't want to play the reload anim for the knife, as it is part of
		// the throw anim anyway.
		if(attack==ATTACK_ALTERNATE)
		{
			PM_HandleWeaponAction(WACT_ALTRELOAD);
		}
		else
		{
			PM_HandleWeaponAction(WACT_RELOAD);
		}
	}

	if( pm->ps->weapon==WP_M590_SHOTGUN || pm->ps->weapon == WP_MM1_GRENADE_LAUNCHER )
	{
		PM_StartTorsoAnim ( pm->ps, weaponData[pm->ps->weapon].animReloadStart, pm->ps->weaponTime );
		return;
	}
	
	extra  = weaponData[pm->ps->weapon].attack[attack].clipSize;
	extra -= pm->ps->clip[attack][pm->ps->weapon];

	if(pm->ps->ammo[weaponData[pm->ps->weapon].attack[attack].ammoIndex]<extra)
	{
		extra=pm->ps->ammo[weaponData[pm->ps->weapon].attack[attack].ammoIndex];
	}

	pm->ps->clip[attack][pm->ps->weapon]+=extra;
	pm->ps->ammo[weaponData[pm->ps->weapon].attack[attack].ammoIndex]-=extra;

	// Rezoom the sniper rifle, if it was zoomed before we started reloading.
	if(pm->ps->pm_flags & PMF_ZOOM_DEFER_RELOAD )
	{
		pm->ps->pm_flags |= PMF_ZOOM_REZOOM;
		pm->ps->pm_flags &= ~PMF_ZOOM_DEFER_RELOAD;
	}

	PM_StartTorsoAnim ( pm->ps, weaponData[pm->ps->weapon].animReload, pm->ps->weaponTime );	
}

/*
==============
PM_EndRefillClip
==============
*/
void PM_EndRefillClip(void)
{
	pm->ps->weaponstate=WEAPON_READY;
}

/*
===============
PM_GetAttackButtons
===============
*/
int PM_GetAttackButtons(void)
{
	int buttons=pm->cmd.buttons;

	// Debounce firemode select button.
	if ( buttons & BUTTON_FIREMODE )
	{
		if(!(pm->ps->pm_debounce & PMD_FIREMODE))
		{
			pm->ps->pm_debounce |= PMD_FIREMODE;
		}
		else
		{
			buttons &= ~BUTTON_FIREMODE;
		}
	}
	else
	{
		pm->ps->pm_debounce &= ~PMD_FIREMODE;
	}

	if(current_gametype.value == GT_HS){
		// As soon as the button is released you are ok to press attack again
		if ( pm->ps->pm_debounce & PMD_ATTACK ) 
		{
			if ( !(buttons & BUTTON_ATTACK) )
			{
				pm->ps->pm_debounce &= ~(PMD_ATTACK);
			}
			else if ( pm->ps->firemode[pm->ps->weapon] != WP_FIREMODE_AUTO )
			{
				buttons &= ~BUTTON_ATTACK;
			}
		}

		if ( pm->ps->stats[STAT_FROZEN] && !level.crossTheBridge) 
		{
			buttons &= ~BUTTON_ATTACK;
		}
	}

	// Handle firebutton in varous firemodes.
	switch( pm->ps->firemode[pm->ps->weapon] )
	{
		case WP_FIREMODE_AUTO:
			break;

		case WP_FIREMODE_BURST:
			// Debounce attack button and disable other buttons during burst fire.
			if(buttons&BUTTON_ATTACK)
			{
				if( !(pm->ps->pm_debounce & PMD_ATTACK))
				{
					pm->ps->pm_debounce |= PMD_ATTACK;
					if(!pm->ps->weaponFireBurstCount)
					{
						pm->ps->weaponFireBurstCount=3;
					}
				}
				else
				{
					buttons &= ~BUTTON_ATTACK;
				}
			}
			else
			{
				pm->ps->pm_debounce &= ~PMD_ATTACK;
			}
			if(pm->ps->weaponFireBurstCount)
			{
				buttons|=BUTTON_ATTACK;
				buttons&=~BUTTON_ALT_ATTACK;
				buttons&=~BUTTON_RELOAD;
				buttons&=~BUTTON_ZOOMIN;
				buttons&=~BUTTON_ZOOMOUT;
				buttons&=~BUTTON_FIREMODE;
			}
			break;

		case WP_FIREMODE_SINGLE:
			// Debounce attack button.
			if(buttons&BUTTON_ATTACK)
			{
				if(!(pm->ps->pm_debounce & PMD_ATTACK))
				{
					pm->ps->pm_debounce |= PMD_ATTACK;
				}
				else
				{
					buttons&=~BUTTON_ATTACK;
				}
			}
			else
			{
				pm->ps->pm_debounce &= ~PMD_ATTACK;
			}

			break;
	}

	// Handle single fire alt fire attacks or the sniper zoom
	if ( pm->ps->weapon == WP_MSG90A1 || (weaponData[pm->ps->weapon].attack[ATTACK_ALTERNATE].weaponFlags & (1<<WP_FIREMODE_SINGLE)) )
	{
		if ( buttons & BUTTON_ALT_ATTACK )
		{
			if( !(pm->ps->pm_debounce & PMD_ALTATTACK ) )
			{
				pm->ps->pm_debounce |= PMD_ALTATTACK;
			}
			else
			{
				buttons &= ~BUTTON_ALT_ATTACK;
			}
		}
		else
		{
			pm->ps->pm_debounce &= ~PMD_ALTATTACK;
		}				
	}

	return buttons;
}

/*
==============
PM_Weapon_AddInaccuracy
==============
*/
#define ACCURACY_FADERATE	0.2
#define RECOVER_TIME		800
#define RECOVER_TIME_SQ		200000.0 

static void PM_Weapon_AddInaccuracy( attackType_t attack )
{
	assert ( attack >= ATTACK_NORMAL && attack < ATTACK_MAX );

	// Zoomed sniper weapons don't add innacuracy if ont hte ground
	if( (pm->ps->pm_flags & PMF_ZOOMED) && pml.groundPlane )
	{
		return;
	}

	pm->ps->inaccuracy += weaponData[pm->ps->weapon].attack[attack].inaccuracy;

	pm->ps->inaccuracyTime = RECOVER_TIME;

	if ( pm->ps->inaccuracy > weaponData[pm->ps->weapon].attack[attack].maxInaccuracy )
		pm->ps->inaccuracy = weaponData[pm->ps->weapon].attack[attack].maxInaccuracy;
}

/*
==============
PM_Weapon_UpdateInaccuracy
==============
*/
static void PM_Weapon_UpdateInaccuracy(void)
{
	if( pm->ps->inaccuracy <= 0 )
	{
		pm->ps->inaccuracy = 0;
		return;
	}

	if ( pm->ps->weaponstate == WEAPON_FIRING || pm->ps->weaponstate == WEAPON_FIRING_ALT)
	{
		pm->ps->inaccuracyTime -= (pml.msec * 3 / 4);
	}
	else
	{
		pm->ps->inaccuracyTime -= pml.msec;
	}

	if ( pm->ps->inaccuracyTime <= 0 )
	{
		pm->ps->inaccuracy = 0;
	}
	else
	{
		//	decrement inaccuracy quadraticly to simulate the player recovering slowly at first, then rapidly
		int diff = RECOVER_TIME - pm->ps->inaccuracyTime;

		pm->ps->inaccuracy *= (1 - (diff*diff)/RECOVER_TIME_SQ);
	}
}

/*
==============
PM_Weapon_AddKickAngles
==============
*/
static void PM_Weapon_AddKickAngles(vec3_t kickAngles)
{
	// Throw the new kick angles into the integer versions
	pm->ps->kickPitch += (int)(kickAngles[PITCH] * 500.0f);

	if ( pm->ps->kickPitch > 180000 )
		pm->ps->kickPitch = 180000;
}

/*
==============
PM_Weapon_UpdateKickAngles
==============
*/
static void PM_Weapon_UpdateKickAngles(void)
{
	// bring our kickAngles back down to zero over time
	int			i;
	float		degreesCorrectedPerMSecond = 0.01f;
	qboolean	firing;
	float		degreesToCorrect = degreesCorrectedPerMSecond*pml.msec;

	vec3_t		kickAngles;

	// Extract the kick angles from their integer versions
	kickAngles[YAW]   = kickAngles[ROLL] = 0;
	kickAngles[PITCH] = (float)pm->ps->kickPitch / 1000.0f;

	firing = qfalse;

	// Determine if firing or not
	if ( pm->ps->weaponstate == WEAPON_FIRING || pm->ps->weaponstate == WEAPON_FIRING_ALT)
	{	
		firing = qtrue;
	}

	// If not firing then bring it down alot faster.
	if (!firing)
	{
		// return a whole lot faster if not firing
		VectorScale( kickAngles, 1.0f - (0.3f*((float)pml.msec/50.0f)), kickAngles );

		for (i = 0; i < 3; i++)
		{
			if (kickAngles[i] >= 0 && kickAngles[i] < 0.05f)
			{
				kickAngles[i] = 0.0f;
			}
			else if (kickAngles[i] <= 0 && kickAngles[i] > -0.05f)
			{
				kickAngles[i] = 0.0f;
			}
		}
	}
	else
	{
		for (i = 0; i < 3; i++)
		{
			if (kickAngles[i] > 0)
			{
				if (kickAngles[i] < degreesToCorrect)
				{
					kickAngles[i] = 0;
				}
				else
				{
					kickAngles[i] -= degreesToCorrect;
				}
			}
			else if (kickAngles[i] < 0)
			{
				if (kickAngles[i] > -degreesToCorrect)
				{
					kickAngles[i] = 0;
				}
				else
				{
					kickAngles[i] += degreesToCorrect;
				}
			}
		}
	}

	// Throw the new kick angles into the integer versions
	pm->ps->kickPitch = (int)(kickAngles[PITCH] * 1000.0f);
}

/*
==============
PM_Goggles

Handles turning goggles on and off
==============
*/
static void PM_Goggles ( void )
{
	// ignore if not a normal player or dead or a ghost
	if ( pm->ps->pm_type != PM_NORMAL || pm->ps->stats[STAT_HEALTH] <= 0 || (pm->ps->pm_flags & PMF_GHOST) ) 
	{
		return;
	}

	// See if they even have goggles
	if ( pm->ps->stats[STAT_GOGGLES] == GOGGLES_NONE )
	{
		return;
	}

	// If the thermal goggles are on and the user has zoomed then turn them off
	if ( pm->ps->stats[STAT_GOGGLES] == GOGGLES_INFRARED )
	{
		// If the player is zoomed then no goggles
		if ( pm->ps->pm_flags & PMF_ZOOMED )
		{
			pm->ps->pm_flags &= ~PMF_GOGGLES_ON;
			return;
		}
	}

	// When goggles button isnt down there is nothing to do
	if ( !(pm->cmd.buttons & BUTTON_GOGGLES ) )
	{
		pm->ps->pm_debounce &= ~PMD_GOGGLES;
		return;
	}

	// Dont do anything if the goggles button is being held down
	if ( pm->ps->pm_debounce & PMD_GOGGLES )
	{
		return;
	}	

	// toggle the goggles
	pm->ps->pm_debounce |= PMD_GOGGLES;	
	pm->ps->pm_flags ^= PMF_GOGGLES_ON;

	// Play some noise
	PM_AddEventWithParm(EV_GOGGLES, (pm->ps->pm_flags&PMF_GOGGLES_ON) ? qtrue : qfalse );
}

/*
==============
PM_WeaponIdle

Handles the progression of the looping idle animation
==============
*/
static void PM_WeaponIdle ( void )
{
	pm->ps->weaponAnimTime -= pml.msec;
	if ( pm->ps->weaponAnimTime <= 0 )
	{
		pm->ps->weaponAnimTime = 0;
		if ( pm->ps->weaponTime <= 0 )
		{
			PM_HandleWeaponAction(WACT_IDLE);
			pm->ps->weaponTime = 0;
		}
	}
}

/*
==============
PM_Weapon

Generates weapon events and modifes the weapon counter
==============
*/
static void PM_Weapon( void )
{
	int				*ammoSource;
	int				attackButtons;
	attackData_t	*attackData;
	qboolean		altFire;
	
	// Get modifed attack buttons.
	attackButtons = PM_GetAttackButtons();

	// don't allow attack until all buttons are up
	if ( pm->ps->pm_flags & PMF_RESPAWNED ) 
	{
		return;
	}

	// ignore if not a normal player
	if ( pm->ps->pm_type != PM_NORMAL ) 
	{
		return;
	}

	// check for dead player
	if ( pm->ps->stats[STAT_HEALTH] <= 0 ) 
	{
		pm->ps->weapon = WP_NONE;
		return;
	}

	// Update the weapon inaccuracies and recoil
	PM_Weapon_UpdateInaccuracy();
	PM_Weapon_UpdateKickAngles();

	if( pm->ps->weaponTime > 0 )
	{
		pm->ps->weaponTime-=pml.msec;
	}

	// See if we've hit a note?
	PM_CheckWeaponNotes();

	// Check for weapon change.
	if( pm->ps->weaponstate == WEAPON_CHARGING || pm->ps->weaponstate == WEAPON_CHARGING_ALT )
	{
		// Can't change if weapon is charging.
		// Update the grenade timer
		if ( pm->ps->grenadeTimer > 0 )
		{
			pm->ps->grenadeTimer -= pml.msec;

			// Force it to go off if the timer has run out
			if ( pm->ps->grenadeTimer <= 0 )
			{
				pm->ps->grenadeTimer = 1;
				pm->ps->weaponTime = 0;
				attackButtons &= ~(BUTTON_ATTACK|BUTTON_ALT_ATTACK);
			}
		}
	}
	else if ( pm->ps->weaponstate == WEAPON_SPAWNING )
	{
		if ( pm->cmd.weapon != pm->ps->weapon )
		{
			return;
		}

		pm->ps->weaponstate = WEAPON_READY;
		pm->ps->weaponTime = 0;
	}
	else if( pm->ps->weaponTime <= 0 || pm->ps->weaponstate < WEAPON_RELOADING )
	{
		// Dont change weapons if this is a weapon selection
		if ( (pm->cmd.weapon & WP_DELAYED_CHANGE_BIT) && ((pm->cmd.buttons&BUTTON_ATTACK)||(pm->cmd.buttons&BUTTON_ALT_ATTACK)) )
		{
			PM_BeginWeaponChange( pm->cmd.weapon & ~WP_DELAYED_CHANGE_BIT );
		}
		else if ( !(pm->cmd.weapon & WP_DELAYED_CHANGE_BIT) && pm->ps->weapon != pm->cmd.weapon )
		{
			PM_BeginWeaponChange( pm->cmd.weapon );
		}
	}

	if ( pm->ps->weaponTime > 0 )
	{
		// Handle the weapons idle animation
		PM_WeaponIdle ( );

		return;
	}

	// Reload the alt clip immediately
	if( !pm->ps->clip[ATTACK_ALTERNATE][pm->ps->weapon] && pm->ps->ammo[weaponData[pm->ps->weapon].attack[ATTACK_ALTERNATE].ammoIndex] > 0) 
	{
		switch(weaponData[pm->ps->weapon].attack[ATTACK_ALTERNATE].fireFromClip)
		{
			case 2:
				// Reload altclip.
				PM_StartRefillClip( ATTACK_ALTERNATE );					
				return;
		}
	}

	// Select firemode.
	if( attackButtons & BUTTON_FIREMODE )
	{
		pm->ps->firemode[pm->ps->weapon] = BG_FindFireMode( pm->ps->weapon, ATTACK_NORMAL, pm->ps->firemode[pm->ps->weapon] + 1 );
	}

	// Decrement burst fire counter if running.
	if(pm->ps->weaponFireBurstCount)
	{
		pm->ps->weaponFireBurstCount--;
	}

	// change weapon if time
	if ( pm->ps->weaponstate == WEAPON_DROPPING )
	{
		PM_FinishWeaponChange();
		return;
	}

	// Zoom in animation complete... now set zoom parms.
	if( pm->ps->weaponstate == WEAPON_ZOOMIN )
	{
		// The zoomfov may still be remembered from a reload while zooming
		if ( !pm->ps->zoomFov )
		{
			pm->ps->zoomFov = 20;
		}

		pm->ps->pm_flags |= PMF_ZOOMED;
		pm->ps->pm_flags |= PMF_ZOOM_LOCKED;
		pm->ps->pm_flags &= ~PMF_ZOOM_REZOOM;
		pm->ps->weaponstate=WEAPON_READY;
		return;
	}

	if( pm->ps->weaponstate==WEAPON_CHARGING || pm->ps->weaponstate==WEAPON_CHARGING_ALT )
	{
		switch(pm->ps->weapon)
		{		
			case WP_M67_GRENADE:
			case WP_M84_GRENADE:
			case WP_F1_GRENADE:
			case WP_L2A2_GRENADE:
			case WP_MDN11_GRENADE:
			case WP_SMOHG92_GRENADE:
			case WP_ANM14_GRENADE:
			case WP_M15_GRENADE:
				if(!(attackButtons&(BUTTON_ATTACK|BUTTON_ALT_ATTACK)) ) 
				{
					if(pm->ps->weaponstate==WEAPON_CHARGING)
					{
						if ( pm->ps->grenadeTimer <= 1 )
						{
							PM_AddEvent(EV_FIRE_WEAPON);
							pm->ps->weaponstate=WEAPON_FIRING;
							pm->ps->weaponTime = 250;
						}
						else
						{
							pm->ps->weaponstate=WEAPON_FIRING;
							PM_HandleWeaponAction(WACT_FIRE);
							PM_StartTorsoAnim( pm->ps, TORSO_ATTACK_GRENADE_END, pm->ps->weaponTime);
						}
					}
					else
					{
						if ( pm->ps->grenadeTimer <= 1 )
						{
							PM_AddEvent(EV_ALT_FIRE);
							pm->ps->weaponstate=WEAPON_FIRING;
							pm->ps->weaponTime = 250;
						}
						else
						{
							pm->ps->weaponstate=WEAPON_FIRING_ALT;
							PM_HandleWeaponAction(WACT_ALTFIRE);
							PM_StartTorsoAnim( pm->ps, TORSO_ATTACK_GRENADE_END, pm->ps->weaponTime);
						}
					}
					return;
				}
				else
				{
					return;
				}
				break;

			default:
				break;
		}
	}

	// Special end of fire animation for some weapons. Knife currently the only one.
	if( pm->ps->weaponstate == WEAPON_FIRING )
	{
		switch(pm->ps->weapon)
		{
			case WP_KNIFE:
				if(!(pm->cmd.buttons&BUTTON_ATTACK))
				{
					PM_HandleWeaponAction(WACT_FIRE_END);
					pm->ps->weaponTime=0;
					pm->ps->weaponstate=WEAPON_READY;
					return;
				}
			default:
				break;
		}
	}

	// ************************************************************************
	// Special reload behavior for warious weapons.
	// ************************************************************************
	if((pm->ps->weaponstate==WEAPON_RELOADING)||(pm->ps->weaponstate==WEAPON_RELOADING_ALT))
	{
		switch(pm->ps->weapon)
		{
			case WP_MM1_GRENADE_LAUNCHER:
			case WP_M590_SHOTGUN:
				// The M590 shotgun has a unique behavior in that when it is reloading,
				// the reload can be interrupted by firing. This cancels the reload of
				// course.
				if(attackButtons&(BUTTON_ATTACK|BUTTON_ALT_ATTACK)&&(pm->ps->clip[ATTACK_NORMAL][pm->ps->weapon]>0))
				{
					// Allow normal fire operation and cancel reload. Note: in
					// the interests of gameplay, I allow us to go right to the
					// fire anim, although it doesn't look as smooth as going to
					// reload end and then fire.
					PM_HandleWeaponAction(WACT_RELOAD_END);					
					PM_EndRefillClip();

					PM_StartTorsoAnim ( pm->ps, weaponData[pm->ps->weapon].animReloadEnd, pm->ps->weaponTime );	

					return;
				}
				else if( ( pm->ps->clip[ATTACK_NORMAL][pm->ps->weapon] < weaponData[pm->ps->weapon].attack[ATTACK_NORMAL].clipSize) &&
						 ( pm->ps->ammo[weaponData[pm->ps->weapon].attack[ATTACK_NORMAL].ammoIndex]>0))
				{
					// Load 1 more shell.
					pm->ps->clip[ATTACK_NORMAL][pm->ps->weapon]++;
					pm->ps->ammo[weaponData[pm->ps->weapon].attack[ATTACK_NORMAL].ammoIndex]--;
					PM_HandleWeaponAction(WACT_RELOAD);

					PM_StartTorsoAnim ( pm->ps, weaponData[pm->ps->weapon].animReload, pm->ps->weaponTime );
					return;
				}
				else
				{
					// Weapon fully loaded so play end reload sequence.
					PM_HandleWeaponAction(WACT_RELOAD_END);
					PM_EndRefillClip();

					PM_StartTorsoAnim ( pm->ps, weaponData[pm->ps->weapon].animReloadEnd, pm->ps->weaponTime );	

					return;
				}
				break;

			default:
				PM_EndRefillClip();
				break;
		}
	}
	else if(pm->ps->pm_flags & PMF_ZOOM_DEFER_RELOAD )
	{
		PM_StartRefillClip( ATTACK_NORMAL );
		return;
	}
	else if( pm->ps->weapon==WP_KNIFE || (pm->ps->weapon>=WP_RPG7_LAUNCHER && pm->ps->weapon<=WP_M15_GRENADE) )
	{
		if(pm->ps->clip[ATTACK_NORMAL][pm->ps->weapon]<1)
		{
			// Clip is now empty so see if we have enough ammo to reload this weapon?
			if (pm->ps->ammo[weaponData[pm->ps->weapon].attack[ATTACK_NORMAL].ammoIndex] > 0) 
			{
				// Yes, so reload it.
				PM_StartRefillClip( ATTACK_NORMAL );
				return;
			}
			else
			{
				if(pm->ps->weapon!=WP_RPG7_LAUNCHER)
				{
					// Clear grenade type from inventory.
					pm->ps->stats[STAT_WEAPONS]&=~(1<<pm->ps->weapon); 

					// Out of ammo so switch weapons.
					PM_AddEventWithParm(EV_NOAMMO, pm->ps->weapon);
					return;
				}
			}
		}
	}
	
	// Handle zooming in/out for sniper rifle.
	if(pm->ps->weapon==WP_MSG90A1)
	{
		if( (attackButtons&BUTTON_ALT_ATTACK) || (pm->ps->pm_flags & PMF_ZOOM_REZOOM) )
		{
			if( pm->ps->pm_flags & PMF_ZOOMED )
			{
				PM_BeginZoomOut();
			}
			else
			{
				PM_BeginZoomIn();
			}			
			return;
		}
		else if( pm->ps->pm_flags & PMF_ZOOMED )
		{
			if(pm->cmd.buttons&BUTTON_ZOOMIN)
			{
				pm->ps->zoomFov = pm->ps->zoomFov >> 1;
				if ( pm->ps->zoomFov < 5)
				{
					pm->ps->zoomFov = 5;
				}
				pm->ps->weaponTime=175;
				return;
			}
			else if(pm->cmd.buttons&BUTTON_ZOOMOUT)
			{
				pm->ps->zoomFov = pm->ps->zoomFov << 1;
				if(pm->ps->zoomFov > 20 )
				{
					pm->ps->zoomFov = 20;
				}
				pm->ps->weaponTime=175;
				return;
			}
		}
	}

	// Reload weapon?
	if ( attackButtons & BUTTON_RELOAD ) 
	{
		if(pm->ps->clip[ATTACK_NORMAL][pm->ps->weapon] < weaponData[pm->ps->weapon].attack[ATTACK_NORMAL].clipSize)
		{
			// No, so see if we have enough ammo to reload this weapon?
			if (pm->ps->ammo[weaponData[pm->ps->weapon].attack[ATTACK_NORMAL].ammoIndex] > 0) 
			{
				// Yes, so reload it.
				PM_StartRefillClip( ATTACK_NORMAL );
				return;
			}
		}
	}

	// Start weapon when either frozen or not shooting
	if( pm->ps->stats[STAT_FROZEN] && !level.crossTheBridge || !(attackButtons&(BUTTON_ATTACK|BUTTON_ALT_ATTACK)) )
	{
		// Handle the weapons idle animation
		PM_WeaponIdle ( );

		pm->ps->weaponstate = WEAPON_READY;
		return;
	}

	// Determine whether to use the alternate or normal attack info	
	if ( attackButtons & BUTTON_ATTACK )
	{
		altFire    = qfalse;
		attackData = &weaponData[pm->ps->weapon].attack[ATTACK_NORMAL];
	}
	else 
	{
		altFire    = qtrue;
		attackData = &weaponData[pm->ps->weapon].attack[ATTACK_ALTERNATE];

		// Cant throw last knife
		if( pm->ps->weapon==WP_KNIFE && pm->ps->ammo[attackData->ammoIndex] < 1 )
		{
			return;
		}
	}

	// Ammo taken from pool, clip or altclip?
	switch(attackData->fireFromClip)
	{
		case 0:
			ammoSource = &pm->ps->ammo[attackData->ammoIndex];
			break;

		default:
		case 1:
			ammoSource = &pm->ps->clip[ATTACK_NORMAL][pm->ps->weapon];
			break;

		case 2:
			ammoSource = &pm->ps->clip[ATTACK_ALTERNATE][pm->ps->weapon];
			break;
	}

	// Is there enough ammo to fire?
	if ( (*ammoSource) - attackData->fireAmount < 0 )
	{
		// No, so reload if there is more ammo
		if( pm->ps->ammo[ attackData->ammoIndex ] > 0 ) 
		{
			// If auto reloading is enabled then reload the gun
			if ( pm->ps->pm_flags & PMF_AUTORELOAD )
			{
				switch ( attackData->fireFromClip)
				{
					case 1:
						// Reload clip.
						PM_StartRefillClip( ATTACK_NORMAL );
						return;

					case 2:
						// Reload altclip.
						PM_StartRefillClip( ATTACK_ALTERNATE );					
						return;
				}
			}
		}
		// Out of ammo, switch weapons if not an alt-attack
		else if ( !altFire )
		{
			PM_AddEventWithParm (EV_NOAMMO, pm->ps->weapon);
		}

		// Handle the weapons idle animation
		PM_WeaponIdle ( );

		pm->ps->weaponstate = WEAPON_READY;

		return;
	}

	// This attack doesnt exist
	if ( !attackData->damage )
	{
		// Handle the weapons idle animation
		PM_WeaponIdle ( );

		pm->ps->weaponstate = WEAPON_READY;

		return;
	}

	// Decrease the ammo
	(*ammoSource) -= attackData->fireAmount;

	// Handle charging cases
	switch(pm->ps->weapon)
	{
		case WP_KNIFE:
			
			if ( altFire )
			{
				if( pm->ps->weaponstate != WEAPON_FIRING && pm->ps->weaponstate != WEAPON_FIRING_ALT )
				{
					PM_HandleWeaponAction(WACT_ALTFIRE);
					pm->ps->weaponstate=WEAPON_FIRING_ALT;
				}
			}
			else
			{
				PM_HandleWeaponAction ( WACT_FIRE );
				pm->ps->weaponstate = WEAPON_FIRING;
			}

			// Play the torso animation associated with the attack
			PM_StartTorsoAnim ( pm->ps, attackData->animFire, pm->ps->weaponAnimTime );	

			break;

		case WP_M67_GRENADE:
		case WP_M84_GRENADE:
		case WP_F1_GRENADE:
		case WP_L2A2_GRENADE:
		case WP_MDN11_GRENADE:
		case WP_SMOHG92_GRENADE:
		case WP_ANM14_GRENADE:
		case WP_M15_GRENADE:

			// Start the detonation timer on the grenade going.
			pm->ps->grenadeTimer = attackData->projectileLifetime;

			if ( altFire )
			{
				PM_HandleWeaponAction ( WACT_ALTCHARGE );
				pm->ps->weaponstate = WEAPON_CHARGING_ALT;
			}
			else
			{
				PM_HandleWeaponAction ( WACT_CHARGE );
				pm->ps->weaponstate = WEAPON_CHARGING;
			}

			PM_StartTorsoAnim( pm->ps, TORSO_ATTACK_GRENADE_START, pm->ps->weaponTime);

			break;

		default:

			if ( altFire )
			{
				PM_HandleWeaponAction(WACT_ALTFIRE);
				pm->ps->weaponstate=WEAPON_FIRING_ALT;
			}
			else
			{
				PM_HandleWeaponAction(WACT_FIRE);
				pm->ps->weaponstate=WEAPON_FIRING;
			}

			pm->ps->weaponTime += attackData->fireDelay;

			// Play the torso animation associated with the attack
			if ( pm->ps->pm_flags & PMF_ZOOMED )
			{
				PM_StartTorsoAnim ( pm->ps, attackData->animFireZoomed, pm->ps->weaponTime );	
			}
			else
			{
				PM_StartTorsoAnim ( pm->ps, attackData->animFire, pm->ps->weaponTime );	
			}

			break;
	}
}

/*
================
PM_DropTimers
================
*/
static void PM_DropTimers( void )
 {
	// drop misc timing counter
	if ( pm->ps->pm_time ) 
	{
		if ( pml.msec >= pm->ps->pm_time ) 
		{
			pm->ps->pm_flags &= ~PMF_ALL_TIMES;
			pm->ps->pm_time = 0;
		} 
		else 
		{
			pm->ps->pm_time -= pml.msec;
		}
	}
}

/*
==============
PM_CheckLean
==============
*/
static void PM_CheckLean( void )
{
	trace_t		trace;
	qboolean	canlean;
	float		leanTime;

	if ( !pm || !pm->ps )
	{		
		return;
	}
	
	// No leaning as a spectator or a ghost
	if ( (pm->ps->pm_flags & PMF_GHOST) || pm->ps->pm_type == PM_SPECTATOR )
	{
		pm->ps->leanTime = LEAN_TIME;
		pm->ps->pm_flags &= ~PMF_LEANING;
		return;
	}

	leanTime = (float)pm->ps->leanTime - LEAN_TIME;
	canlean  = qfalse;

	// If their lean button is being pressed and they are on the ground then perform the lean
	if( (pm->cmd.buttons & (BUTTON_LEAN_RIGHT|BUTTON_LEAN_LEFT)) && (pm->ps->groundEntityNum != ENTITYNUM_NONE) )
	{
 		vec3_t	start, end, right, mins, maxs;
		int		leanDir;
		
		if( pm->cmd.buttons & BUTTON_LEAN_RIGHT )
		{
			leanDir = 1;
		}
		else
		{
			leanDir = -1;
		}

		// check for collision
		VectorCopy( pm->ps->origin, start );
		start[2] += pm->ps->viewheight;
		AngleVectors( pm->ps->viewangles, NULL, right, NULL );
		VectorSet( mins, -6, -6, -8 ); 
		VectorSet( maxs, 6, 6, 8 ); 
		
		// since we're moving the camera over
		// check that move
		VectorMA( start, leanDir * LEAN_OFFSET * 1.25f, right, end );
		if(pm->trace)
		pm->trace(&trace, start, mins, maxs, end, pm->ps->clientNum, pm->tracemask );

		if ( trace.fraction < 0 || trace.fraction >= 1.0f )
		{
			leanTime += (leanDir * pml.msec);
			if( leanTime > LEAN_TIME )
			{
				leanTime = LEAN_TIME;
			}
			else if( leanTime < -LEAN_TIME )
			{
				leanTime = -LEAN_TIME;
			}

			canlean = qtrue;
		}
		else if ( (pm->ps->pm_flags&PMF_LEANING) && trace.fraction < 1.0f )
		{
			int templeanTime = (float)leanDir * (float)LEAN_TIME * trace.fraction;

			if ( fabs(templeanTime) < fabs(leanTime) )
			{
				leanTime = templeanTime;
			}
		}
	}

	if ( !canlean )
	{
		if( leanTime > 0 )
		{
			leanTime -= pml.msec;
			if( leanTime < 0 )
			{
				leanTime = 0;
			}
		}
		else if ( leanTime < 0 )
		{
			leanTime += pml.msec;
			if( leanTime > 0 )
			{
				leanTime = 0;
			}
		}
	}	

	// Set a pm flag for leaning for convienience
	if ( leanTime != 0 )
	{
		pm->ps->pm_flags |= PMF_LEANING;
	}
	else
	{
		pm->ps->pm_flags &= ~PMF_LEANING;
	}

	// The lean time is kept positive by adding in the base lean time
	pm->ps->leanTime = (int) (leanTime + LEAN_TIME);
}

/*
================
PM_UpdateViewAngles

This can be used as another entry point when only the viewangles
are being updated isntead of a full move
================
*/
void PM_UpdateViewAngles( playerState_t *ps, const usercmd_t *cmd ) 
{
	short	temp;
	int		i;
	vec3_t	kickAngles;

	if ( ps->pm_type == PM_INTERMISSION) 
	{
		return;		// no view changes at all
	}

	if ( ps->pm_type != PM_SPECTATOR && ps->stats[STAT_HEALTH] <= 0 ) 
	{
		return;		// no view changes at all
	}

	// Extract the kcik angles 
	kickAngles[PITCH] = ((float)ps->kickPitch / 1000.0f);
	kickAngles[YAW]   = kickAngles[ROLL] = 0;

	// circularly clamp the angles with deltas
	for (i=0 ; i<3 ; i++) 
	{
		temp = cmd->angles[i] + ps->delta_angles[i] - ANGLE2SHORT(kickAngles[i]);
		if ( i == PITCH ) 
		{
			// don't let the player look up or down more than 90 degrees
			if ( temp > 16000 ) 
			{
				ps->delta_angles[i] = 16000 - (cmd->angles[i]- ANGLE2SHORT(kickAngles[i]));
				temp = 16000;
			} 
			else if ( temp < -16000 ) 
			{
				ps->delta_angles[i] = -16000 - (cmd->angles[i] - ANGLE2SHORT(kickAngles[i]));
				temp = -16000;
			}
		}
		ps->viewangles[i] = SHORT2ANGLE(temp);
	}

	PM_CheckLean ( );
}

/*
================
PM_AdjustAttackStates
================
*/

void PM_AdjustAttackStates( pmove_t *pm )
{
	int ammoOk;

	// Check ammo after usage...
	if(pm->cmd.buttons & BUTTON_ALT_ATTACK)
	{
		ammoOk = pm->ps->ammo[weaponData[pm->ps->weapon].attack[ATTACK_ALTERNATE].ammoIndex] - weaponData[pm->ps->weapon].attack[ATTACK_ALTERNATE].fireAmount;
	}
	else
	{
		ammoOk = pm->ps->clip[ATTACK_NORMAL][pm->ps->weapon ] - weaponData[pm->ps->weapon].attack[ATTACK_NORMAL].fireAmount;
	}
	
	// Set the firing flag.
	if(!(pm->ps->pm_flags & PMF_RESPAWNED) && (pm->ps->pm_type!=PM_INTERMISSION) && (ammoOk>=0))
	{
		if((pm->cmd.buttons & BUTTON_ALT_ATTACK)||(pm->ps->weaponstate==WEAPON_FIRING_ALT))
		{
			pm->ps->eFlags |= EF_ALT_FIRING;
		}
		else if((pm->cmd.buttons & BUTTON_ATTACK)||(pm->ps->weaponstate==WEAPON_FIRING))
		{
			pm->ps->eFlags &= ~EF_ALT_FIRING;
		}

		// This flag should always get set, even when alt-firing
		pm->ps->eFlags |= EF_FIRING;
	} 
	else 
	{
		// Clear 'em out
		pm->ps->eFlags &= ~(EF_FIRING|EF_ALT_FIRING);
	}
}

/*
================
PmoveSingle
================
*/
void trap_SnapVector( float *v );

void PmoveSingle (pmove_t *pmove) {
	char	mapname[64];
	
	if(current_gametype.value == GT_HS)
	trap_Cvar_VariableStringBuffer ( "mapname", mapname, MAX_QPATH ); // Henk 26/02/10 -> Cross the bridge

	pm = pmove;

	// this counter lets us debug movement problems with a journal
	// by setting a conditional breakpoint fot the previous frame
	c_pmove++;

	// clear results
	pm->numtouch = 0;
	pm->watertype = 0;
	pm->waterlevel = 0;

	if ( pm->ps->stats[STAT_HEALTH] <= 0 ) {
		pm->tracemask &= ~CONTENTS_BODY;	// corpses can fly through bodies
	}

	// make sure walking button is clear if they are running, to avoid
	// proxy no-footsteps cheats
	if ( abs( pm->cmd.forwardmove ) > 64 || abs( pm->cmd.rightmove ) > 64 ) {
		pm->cmd.buttons &= ~BUTTON_WALKING;
	}

	// When the lean modifier button is held the strafe left and right keys
	// will act as lean left and right
	if ( pm->cmd.buttons & BUTTON_LEAN )
	{
		pm->cmd.buttons &= ~(BUTTON_LEAN_LEFT|BUTTON_LEAN_RIGHT);

		// Strafe left = lean left
		if ( pm->cmd.rightmove < 0 )
		{
			pm->cmd.buttons |= BUTTON_LEAN_LEFT;
		}
		// Strafe right = lean right
		else if ( pm->cmd.rightmove > 0 )
		{
			pm->cmd.buttons |= BUTTON_LEAN_RIGHT;
		}

		// NO strafing with lean button down
		pm->cmd.rightmove = 0;
	}

	// Cant move when leaning
	if ( (pm->cmd.buttons & (BUTTON_LEAN_LEFT|BUTTON_LEAN_RIGHT)))
	{
//		pm->cmd.rightmove = 0;
		pm->cmd.forwardmove = 0;

		// Cant jump when leaning
		if ( pm->cmd.upmove > 0 )
		{
			pm->cmd.upmove = 0;
		}
	}
	
	// Cant run when zoomed
	if ( (pm->ps->pm_flags&PMF_ZOOMED) || pm->ps->weaponstate == WEAPON_ZOOMIN || (pm->cmd.buttons & (BUTTON_LEAN_LEFT|BUTTON_LEAN_RIGHT)) )
	{
		if ( pm->cmd.forwardmove > 64 )
		{
			pm->cmd.forwardmove = 64;
		}
		else if ( pm->cmd.forwardmove < -64 )
		{
			pm->cmd.forwardmove = -64;
		}


		if ( pm->cmd.rightmove > 64 )
		{
			pm->cmd.rightmove = 64;
		}
		else if ( pm->cmd.rightmove < -64 )
		{
			pm->cmd.rightmove = -64;
		}

		pm->cmd.buttons |= BUTTON_WALKING;
	}

	// set the talk balloon flag
	if ( pm->cmd.buttons & BUTTON_TALK ) 
	{
		pm->ps->eFlags |= EF_TALK;
	} 
	else 
	{
		pm->ps->eFlags &= ~EF_TALK;
	}

	// In certain situations, we may want to control which attack buttons are pressed and what kind of functionality
	//	is attached to them
	PM_AdjustAttackStates( pm );

	// clear the respawned flag if attack and use are cleared
	if ( pm->ps->stats[STAT_HEALTH] > 0 && !( pm->cmd.buttons & BUTTON_ATTACK) )
	{
		pm->ps->pm_flags &= ~PMF_RESPAWNED;
	}

	// if talk button is down, dissallow all other input
	// this is to prevent any possible intercept proxy from
	// adding fake talk balloons
	if ( pmove->cmd.buttons & BUTTON_TALK ) 
	{
		// keep the talk button set tho for when the cmd.serverTime > 66 msec
		// and the same cmd is used multiple times in Pmove
		pmove->cmd.buttons = BUTTON_TALK;
		pmove->cmd.forwardmove = 0;
		pmove->cmd.rightmove = 0;
		pmove->cmd.upmove = 0;
	}

	// clear all pmove local vars
	memset (&pml, 0, sizeof(pml));

	// determine the time
	pml.msec = pmove->cmd.serverTime - pm->ps->commandTime;

	if ( pml.msec < 1 ) {
		pml.msec = 1;
	} else if ( pml.msec > 200 ) {
		pml.msec = 200;
	}
	pm->ps->commandTime = pmove->cmd.serverTime;

	// Frozen?
	if ( pm->ps->stats[STAT_FROZEN])// && !strstr(mapname, "col9") )
	{
		pm->ps->stats[STAT_FROZEN] -= pml.msec;
		if ( pm->ps->stats[STAT_FROZEN] < 0 )
		{
			pm->ps->stats[STAT_FROZEN] = 0;
		}
		else
		{
//			pm->cmd.buttons = pm->cmd.buttons & (BUTTON_RELOAD|BUTTON_ATTACK|BUTTON_);	
			pm->cmd.forwardmove = 0;
			pm->cmd.rightmove = 0;
			pm->cmd.upmove = 0;
		}
	}

	// save old org in case we get stuck
	VectorCopy (pm->ps->origin, pml.previous_origin);

	// save old velocity for crashlanding
	VectorCopy (pm->ps->velocity, pml.previous_velocity);

	pml.frametime = pml.msec * 0.001;

	// update the viewangles
	PM_UpdateViewAngles( pm->ps, &pm->cmd );

	AngleVectors (pm->ps->viewangles, pml.forward, pml.right, pml.up);

	if ( pm->cmd.upmove < 10 ) 
	{
		// not holding jump
		pm->ps->pm_debounce &= ~PMD_JUMP;
	}

	// decide if backpedaling animations should be used
	if ( pm->cmd.forwardmove < 0 ) {
		pm->ps->pm_flags |= PMF_BACKWARDS_RUN;
	} else if ( pm->cmd.forwardmove > 0 || ( pm->cmd.forwardmove == 0 && pm->cmd.rightmove ) ) {
		pm->ps->pm_flags &= ~PMF_BACKWARDS_RUN;
	}

	if ( pm->ps->pm_type >= PM_DEAD ) {
		pm->cmd.forwardmove = 0;
		pm->cmd.rightmove = 0;
		pm->cmd.upmove = 0;
	}

	if ( pm->ps->pm_type == PM_SPECTATOR ) 
	{
		pm->mins[0] = -15;
		pm->mins[1] = -15;
		pm->maxs[0] = 15;
		pm->maxs[1] = 15;
		pm->mins[2] = MINS_Z;
		pm->maxs[2] = DEFAULT_PLAYER_Z_MAX;
		pm->ps->viewheight = DEFAULT_VIEWHEIGHT;

//		PM_FlyMove ();
		PM_NoclipMove ( );
		PM_DropTimers ();
		return;
	}

	if ( pm->ps->pm_type == PM_NOCLIP ) {
		PM_NoclipMove ();
		PM_DropTimers ();
		return;
	}

	if (pm->ps->pm_type == PM_FREEZE) {
		return;		// no movement at all
	}

	if ( pm->ps->pm_type == PM_INTERMISSION ) 
	{
		return;		// no movement at all
	}

	// set mins, maxs, and viewheight
	PM_CheckDuck ();

	// set groundentity
	PM_GroundTrace();

	// set watertype, and waterlevel
	PM_SetWaterLevel();
	pml.previous_waterlevel = pmove->waterlevel;

	PM_CheckCrouchJump ( );

	if ( pm->ps->pm_type == PM_DEAD ) {
		PM_DeadMove ();
	}

	PM_DropTimers();

	if ( pm->ps->pm_flags & PMF_LADDER )
	{
		PM_LadderMove ( );
	}
	else if (pm->ps->pm_flags & PMF_TIME_WATERJUMP) 
	{
		PM_WaterJumpMove();
	} 
	else if ( pm->waterlevel > 1 ) 
	{
		// swimming
		PM_WaterMove();
	} 
	else if ( pml.walking ) 
	{
		// walking on ground
		PM_WalkMove();
	} 
	else 
	{
		// airborne
		PM_AirMove();
	}

	// set groundentity, watertype, and waterlevel
	if(!(VectorCompare(pm->ps->origin,pml.previous_origin)))
	{
		// If we didn't move at all, then why bother doing this again -MW.
		PM_GroundTrace();
	}

	PM_SetWaterLevel();

	// turn goggles on/off
	PM_Goggles ( );

	// weapons
	PM_Weapon();

	// torso animation
	PM_TorsoAnimation( pm->ps );

	// footstep events / legs animations
	PM_Footsteps();

	// entering / leaving water splashes
	PM_WaterEvents();

	// snap some parts of playerstate to save network bandwidth
	trap_SnapVector( pm->ps->velocity );
}

/*
================
PM_UpdatePVSOrigin

The pvs of the client is calculated using its own origin and this function
ensures that the origin is set correctly.  The main reason for having a PVS
origin is that when leaning your can poke around corners which will in turn
change what you can see.
================
*/
void PM_UpdatePVSOrigin ( pmove_t *pmove )
{
	pm = pmove;

	// Set a pm flag for leaning and calculate the view origin for the lean
	if ( pm->ps->leanTime - LEAN_TIME != 0 )
	{
		vec3_t	right;
		float	leanOffset;

		leanOffset = (float)(pm->ps->leanTime - LEAN_TIME) / LEAN_TIME * LEAN_OFFSET;

		AngleVectors( pm->ps->viewangles, NULL, right, NULL );
		VectorMA( pm->ps->origin, leanOffset, right, pm->ps->pvsOrigin );
	}
	else
	{
		VectorCopy ( pm->ps->origin, pm->ps->pvsOrigin );
	}
}

/*
================
Pmove

Can be called by either the server or the client
================
*/
void Pmove (pmove_t *pmove) {
	int			finalTime;

	finalTime = pmove->cmd.serverTime;

	if ( finalTime < pmove->ps->commandTime ) {
		return;	// should not happen
	}

	if ( finalTime > pmove->ps->commandTime + 1000 ) {
		pmove->ps->commandTime = finalTime - 1000;
	}

	pmove->ps->pmove_framecount = (pmove->ps->pmove_framecount+1) & ((1<<PS_PMOVEFRAMECOUNTBITS)-1);
	//Com_Printf("%i\n", pmove->ps->pmove_framecount);
	// chop the move up if it is too long, to prevent framerate
	// dependent behavior
	while ( pmove->ps->commandTime != finalTime ) {
		int		msec;

		msec = finalTime - pmove->ps->commandTime;

		if ( pmove->pmove_fixed ) {
			if ( msec > pmove->pmove_msec ) {
				msec = pmove->pmove_msec;
			}
		}
		else {
			if ( msec > 66 ) {
				msec = 66;
			}
		}

		pmove->cmd.serverTime = pmove->ps->commandTime + msec;

		PmoveSingle( pmove );

		PM_UpdatePVSOrigin ( pmove );	

		if ( pmove->ps->pm_debounce & PMD_JUMP ) 
		{
			pmove->cmd.upmove = 20;
		}
	}
}

/*
================
BG_AddLadder

Adds a ladder to the ladder list
================
*/
void BG_AddLadder ( vec3_t absmin, vec3_t absmax, vec3_t fwd )
{
	pm_ladders[pm_laddercount].origin[0] = (absmax[0] + absmin[0]) / 2;
	pm_ladders[pm_laddercount].origin[1] = (absmax[1] + absmin[1]) / 2;
	pm_ladders[pm_laddercount].origin[2] = (absmax[2] + absmin[2]) / 2;
	VectorCopy ( fwd, pm_ladders[pm_laddercount].fwd );
	pm_laddercount++;
}

/*
================
BG_FindLadder

Searches through the ladder list and finds the closes to the given origin
================
*/
int BG_FindLadder ( vec3_t pos )
{
	int		ladder;
	int		result;
	float	dist;

	dist   = 999999.0f;
	result = -1;

	for ( ladder = 0; ladder < pm_laddercount; ladder ++ )
	{
		float dist2 = DistanceSquared( pos, pm_ladders[ladder].origin );

		if ( dist2 < dist )
		{
			dist = dist2;
			result = ladder;
		}
	}

	return result;
}

