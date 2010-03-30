// Copyright (C) 2001-2002 Raven Software
//
// bg_slidemove.c -- part of bg_pmove functionality

#include "q_shared.h"
#include "bg_public.h"
#include "bg_local.h"

/*
==================
PM_SlideMove

Returns qtrue if the velocity was clipped in some way
==================
*/
#define	MAX_CLIP_PLANES	5

qboolean	PM_SlideMove( qboolean gravity ) 
{
	int			bumpcount, numbumps;
	vec3_t		dir;
	float		d;
	int			numplanes;
	vec3_t		planes[MAX_CLIP_PLANES];
	vec3_t		primal_velocity;
	vec3_t		clipVelocity;
	int			i, j, k;
	trace_t		trace;
	vec3_t		end;
	float		time_left;
	float		into;
	vec3_t		endVelocity;
	vec3_t		endClipVelocity;
	
	numbumps = 4;

	VectorCopy (pm->ps->velocity, primal_velocity);

	if ( gravity ) 
	{
		VectorCopy( pm->ps->velocity, endVelocity );
		endVelocity[2] -= pm->ps->gravity * pml.frametime;
		pm->ps->velocity[2] = ( pm->ps->velocity[2] + endVelocity[2] ) * 0.5;
		primal_velocity[2] = endVelocity[2];
		
		if ( pml.groundPlane ) 
		{
			// slide along the ground plane
			PM_ClipVelocity (pm->ps->velocity, pml.groundTrace.plane.normal, 
				pm->ps->velocity, OVERCLIP );
		}
	}

	time_left = pml.frametime;

	// never turn against the ground plane
	if ( pml.groundPlane ) 
	{
		numplanes = 1;
		VectorCopy( pml.groundTrace.plane.normal, planes[0] );
	} 
	else 
	{
		numplanes = 0;
	}

	// never turn against original velocity
	VectorNormalize2( pm->ps->velocity, planes[numplanes] );
	numplanes++;

	for ( bumpcount=0 ; bumpcount < numbumps ; bumpcount++ ) 
	{
		// calculate position we are trying to move to
		VectorMA( pm->ps->origin, time_left, pm->ps->velocity, end );

		// see if we can make it there
		pm->trace ( &trace, pm->ps->origin, pm->mins, pm->maxs, end, pm->ps->clientNum, pm->tracemask);

		if (trace.allsolid) 
		{
			// entity is completely trapped in another solid
			pm->ps->velocity[2] = 0;	// don't build up falling damage, but allow sideways acceleration
			return qtrue;
		}

		if (trace.fraction > 0) 
		{
			// actually covered some distance
			VectorCopy (trace.endpos, pm->ps->origin);
		}

		if (trace.fraction == 1) 
		{
			 break;		// moved the entire distance
		}

		// save entity for contact
		PM_AddTouchEnt( trace.entityNum );

		time_left -= time_left * trace.fraction;

		if (numplanes >= MAX_CLIP_PLANES) 
		{
			// this shouldn't really happen
			VectorClear( pm->ps->velocity );
			return qtrue;
		}

		//
		// if this is the same plane we hit before, nudge velocity
		// out along it, which fixes some epsilon issues with
		// non-axial planes
		//
		for ( i = 0 ; i < numplanes ; i++ ) 
		{
			if ( DotProduct( trace.plane.normal, planes[i] ) > 0.99 ) 
			{
				VectorAdd( trace.plane.normal, pm->ps->velocity, pm->ps->velocity );
				break;
			}
		}

		if ( i < numplanes ) 
		{
			continue;
		}

		VectorCopy (trace.plane.normal, planes[numplanes]);
		numplanes++;

		//
		// modify velocity so it parallels all of the clip planes
		//

		// find a plane that it enters
		for ( i = 0 ; i < numplanes ; i++ ) 
		{
			into = DotProduct( pm->ps->velocity, planes[i] );
			if ( into >= 0.1 ) 
			{
				continue;		// move doesn't interact with the plane
			}

			// see how hard we are hitting things
			if ( -into > pml.impactSpeed ) 
			{
				pml.impactSpeed = -into;
			}

			// slide along the plane
			PM_ClipVelocity (pm->ps->velocity, planes[i], clipVelocity, OVERCLIP );

			// slide along the plane
			PM_ClipVelocity (endVelocity, planes[i], endClipVelocity, OVERCLIP );

			// see if there is a second plane that the new move enters
			for ( j = 0 ; j < numplanes ; j++ ) 
			{
				if ( j == i ) 
				{
					continue;
				}
				if ( DotProduct( clipVelocity, planes[j] ) >= 0.1 ) 
				{
					continue;		// move doesn't interact with the plane
				}

				// try clipping the move to the plane
				PM_ClipVelocity( clipVelocity, planes[j], clipVelocity, OVERCLIP );
				PM_ClipVelocity( endClipVelocity, planes[j], endClipVelocity, OVERCLIP );

				// see if it goes back into the first clip plane
				if ( DotProduct( clipVelocity, planes[i] ) >= 0 ) 
				{
					continue;
				}

				// slide the original velocity along the crease
				CrossProduct (planes[i], planes[j], dir);
				VectorNormalize( dir );
				d = DotProduct( dir, pm->ps->velocity );
				VectorScale( dir, d, clipVelocity );

				CrossProduct (planes[i], planes[j], dir);
				VectorNormalize( dir );
				d = DotProduct( dir, endVelocity );
				VectorScale( dir, d, endClipVelocity );

				// see if there is a third plane the the new move enters
				for ( k = 0 ; k < numplanes ; k++ ) 
				{
					if ( k == i || k == j ) 
					{
						continue;
					}
					if ( DotProduct( clipVelocity, planes[k] ) >= 0.1 ) 
					{
						continue;		// move doesn't interact with the plane
					}

					// stop dead at a tripple plane interaction
					VectorClear( pm->ps->velocity );
					return qtrue;
				}
			}

			// if we have fixed all interactions, try another move
			VectorCopy( clipVelocity, pm->ps->velocity );
			VectorCopy( endClipVelocity, endVelocity );
			break;
		}
	}

	if ( gravity ) 
	{
		VectorCopy( endVelocity, pm->ps->velocity );
	}

	// don't change velocity if in a timer (FIXME: is this correct?)
	if ( pm->ps->pm_time ) 
	{
		VectorCopy( primal_velocity, pm->ps->velocity );
	}

	return (qboolean)( bumpcount != 0 );
}

/*
==================
PM_StepSlideMove

==================
*/
void PM_StepSlideMove( qboolean gravity ) 
{
	trace_t		trace;
	vec3_t		down;
	vec3_t		up = {0, 0, 1};
	qboolean	result = qtrue;
	vec3_t		start_o;
	vec3_t		start_v;
	vec3_t		save_o;
	vec3_t		save_v;
	int			stepsize;
	float		delta;

	VectorCopy (pm->ps->origin, start_o);
	VectorCopy (pm->ps->velocity, start_v);

	if ( PM_SlideMove( gravity ) == 0 ) 
	{
		// we got exactly where we wanted to go on the first try	
		return;		
	}

	// Set the standard stepsize
	stepsize = STEPSIZE;

	// Add to the step size if we are crouched when jumping
	if ( pm->ps->pm_flags & PMF_CROUCH_JUMP )
	{
		stepsize += (DEFAULT_VIEWHEIGHT-CROUCH_VIEWHEIGHT);
	}

	// Save the origin and velocity in case we have to undo
	VectorCopy ( pm->ps->origin, save_o );
	VectorCopy ( pm->ps->velocity, save_v );

	// First lets see if there is any hope of steping up
	pm->maxs[2] -= stepsize;

	VectorCopy ( start_o, pm->ps->origin );
	VectorCopy ( start_v, pm->ps->velocity );

	pm->ps->origin[2] += stepsize;

	// try the move with the altered hit box
	if ( PM_SlideMove( gravity ))
	{
	}

	// See how far down now
	VectorCopy ( pm->ps->origin, down );
	down[2] -= stepsize;

	// Trace it
	pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, down, pm->ps->clientNum, pm->tracemask);

	// Return the players hitbox to normal
	pm->maxs[2] += stepsize;

	// No stepping up if you have upward velocity
	if ( pm->ps->velocity[2] > 0 && (trace.fraction == 1.0 || DotProduct(trace.plane.normal, up) < 0.7)) 
	{
		VectorCopy ( save_o, pm->ps->origin );
		VectorCopy ( save_v, pm->ps->velocity );
		return;
	}

	if ( trace.allsolid || trace.startsolid ) 
	{
		result = qfalse;
	}
	else
	{
		if ( trace.fraction < 1.0 )
			PM_ClipVelocity( pm->ps->velocity, trace.plane.normal, pm->ps->velocity, OVERCLIP );

		// Now double check not stuck
		VectorCopy ( trace.endpos, pm->ps->origin );
		pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, pm->ps->origin, pm->ps->clientNum, pm->tracemask);

		if ( trace.allsolid || trace.startsolid )
			result = qfalse;
	}
	
	if ( !result )
	{
		VectorCopy ( save_o, pm->ps->origin );
		VectorCopy ( save_v, pm->ps->velocity );
	}

	// use the step move
	delta = pm->ps->origin[2] - start_o[2];

	// Shoould we send a step event?
	if ( delta > 2 ) 
	{
		if ( delta < 7 ) 
		{
			PM_AddEvent( EV_STEP_4 );
		} 
		else if ( delta < 11 ) 
		{
			PM_AddEvent( EV_STEP_8 );
		} 
		else if ( delta < 15 ) 
		{
			PM_AddEvent( EV_STEP_12 );
		} 
		else 
		{
			PM_AddEvent( EV_STEP_16 );
		}
	}
	
	if ( pm->debugLevel ) 
	{
		Com_Printf("%i:stepped\n", c_pmove);
	}
}

