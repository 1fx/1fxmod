/*
===========================================================================
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
Copyright (C) 2010 - 2020, Ane-Jouke Schat (Boe!Man)
Copyright (C) 2010 - 2020, Jordi Epema (Henkie)

This file is part of the 1fx. Mod source code.

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/
// cg_ents.c -- present snapshot entities, happens every single frame

#include "cg_local.h"
#include "..\game\q_shared.h"
#include "..\ghoul2\g2.h"

/*
======================
CG_SetEntitySoundPosition

Also called by event processing code
======================
*/
float *CG_SetEntitySoundPosition( centity_t *cent ) 
{
	static vec3_t v3Return;

	if ( cent->currentState.solid == SOLID_BMODEL ) 
	{
		vec3_t	origin;
		float	*v;

		v = cgs.inlineModelMidpoints[ cent->currentState.modelindex ];
		VectorAdd( cent->lerpOrigin, v, origin );
		trap_S_UpdateEntityPosition( cent->currentState.number, origin );
		VectorCopy(origin, v3Return);
	} 
	else 
	{
		trap_S_UpdateEntityPosition( cent->currentState.number, cent->lerpOrigin );
		VectorCopy(cent->lerpOrigin, v3Return);
	}

	return v3Return;
}

/*
==================
CG_EntityEffects

Add continuous entity effects, like local entity emission and lighting
==================
*/
static void CG_EntityEffects( centity_t *cent ) 
{
	sfxHandle_t		hSFX;
	float			*v3Origin;

	// update sound origins
	v3Origin = CG_SetEntitySoundPosition(cent);

	// add loop sound
	if ( cent->currentState.loopSound ) 
	{
		// ambient soundset string index valid?
		if (cent->currentState.eType == ET_MOVER &&	cent->currentState.mSoundSet)
		{
			hSFX = trap_AS_GetBModelSound( CG_ConfigString( CS_AMBIENT_SOUNDSETS + cent->currentState.mSoundSet), cent->currentState.loopSound);

			if (hSFX == -1)
			{
				Com_Printf("CG_EntityEffects(): Bad SFX handle -1, ambient soundset not loaded?\n");
			}
			else
			{
				trap_S_AddLoopingSound( cent->currentState.number, v3Origin/*cent->lerpOrigin*/, vec3_origin, hSFX, CHAN_AUTO );			
			}
		}
		else
		{
			if (cent->currentState.eType != ET_SPEAKER) 
			{
				trap_S_AddLoopingSound( 
					cent->currentState.number, 
					cent->lerpOrigin, 
					vec3_origin, 
					0,
					cgs.gameSounds[ cent->currentState.loopSound ] );
			} 
			else 
			{
				trap_S_AddRealLoopingSound( 
					cent->currentState.number, 
					cent->lerpOrigin, 
					vec3_origin, 
					((float)cent->currentState.time2) / 1000.0f,
					cgs.gameSounds[ cent->currentState.loopSound ] );
			}
		}
	}
}

/*
==================
CG_SetGhoul2Info
==================
*/
void CG_SetGhoul2Info( refEntity_t *ent, centity_t *cent)
{
	ent->ghoul2 = cent->ghoul2;
	VectorCopy( cent->modelScale, ent->modelScale);
	ent->radius = cent->radius + 256;
	VectorCopy (cent->lerpAngles, ent->angles);
}

/*
==================
CG_SetGhoul2Info

Position an entity based on the bolt point of a ghoul2 model
==================
*/
qboolean G2_PositionEntityOnBolt (
	refEntity_t *ent, 
	void		*ghoul2, 
	int			modelIndex, 
	int			boltIndex, 
	vec3_t		origin, 
	vec3_t		angles, 
	vec3_t		modelScale
	)
{
	qboolean boltMatrixOK = qfalse;
	mdxaBone_t boltMatrix;

	if (boltIndex < 0)
		return boltMatrixOK;

	boltMatrixOK = trap_G2API_GetBoltMatrix(ghoul2, modelIndex, boltIndex, &boltMatrix,
											angles, origin, cg.time, cgs.gameModels, modelScale);

	if (boltMatrixOK)
	{	
		// set up the axis and origin
		ent->origin[0] = boltMatrix.matrix[0][3];
		ent->origin[1] = boltMatrix.matrix[1][3];
		ent->origin[2] = boltMatrix.matrix[2][3];

		ent->axis[0][0] = boltMatrix.matrix[0][0];
		ent->axis[0][1] = boltMatrix.matrix[1][0];
		ent->axis[0][2] = boltMatrix.matrix[2][0];

		ent->axis[1][0] = boltMatrix.matrix[0][1];
		ent->axis[1][1] = boltMatrix.matrix[1][1];
		ent->axis[1][2] = boltMatrix.matrix[2][1];

		ent->axis[2][0] = boltMatrix.matrix[0][2];
		ent->axis[2][1] = boltMatrix.matrix[1][2];
		ent->axis[2][2] = boltMatrix.matrix[2][2];
	}

	return boltMatrixOK;
}

/*
==================
CG_ScaleModelAxis
==================
*/
void CG_ScaleModelAxis(refEntity_t	*ent)
{		
	// scale the model should we need to
	if (ent->modelScale[0] && ent->modelScale[0] != 1.0f)
	{
		VectorScale( ent->axis[0], ent->modelScale[0] , ent->axis[0] );
		ent->nonNormalizedAxes = qtrue;
	}
	if (ent->modelScale[1] && ent->modelScale[1] != 1.0f)
	{
		VectorScale( ent->axis[1], ent->modelScale[1] , ent->axis[1] );
		ent->nonNormalizedAxes = qtrue;
	}
	if (ent->modelScale[2] && ent->modelScale[2] != 1.0f)
	{
		VectorScale( ent->axis[2], ent->modelScale[2] , ent->axis[2] );
		ent->nonNormalizedAxes = qtrue;
	}
}

/*
==================
CG_General
==================
*/
static void CG_General( centity_t *cent ) 
{
	refEntity_t			ent;
	entityState_t		*s1;

	s1 = &cent->currentState;

	// Dont draw any player models when the round has ended
	if ( cg.snap->ps.pm_type == PM_INTERMISSION )
	{
		return;
	}

	// if set to invisible, skip
	if ((!s1->modelindex) && !(trap_G2_HaveWeGhoul2Models(cent->ghoul2))) 
	{
		return;
	}

	memset (&ent, 0, sizeof(ent));

	// set frame
	ent.frame = s1->frame;
	ent.oldframe = ent.frame;
	ent.backlerp = 0;

	CG_SetGhoul2Info(&ent, cent);

	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);

	ent.hModel = cgs.gameModels[s1->modelindex];

	// player model
	if (s1->number == cg.snap->ps.clientNum) 
	{
		ent.renderfx |= RF_THIRD_PERSON;	// only draw from mirrors
	}

	// Bodies need shadows under them
	if ( cent->currentState.eType == ET_BODY && !(cent->currentState.eFlags&EF_NOSHADOW))
	{
		CG_PlayerShadow( cent, &ent.shadowPlane );

		if ( cg_shadows.integer == 3 && ent.shadowPlane ) 
		{
			ent.renderfx |= RF_SHADOW_PLANE;
		}
	}

	ent.renderfx |= RF_MINLIGHT;

	// convert angles to axis
	AnglesToAxis( cent->lerpAngles, ent.axis );

	// add to refresh list
	trap_R_AddRefEntityToScene (&ent);
}

/*
==================
CG_Speaker

Speaker entities can automatically play sounds
==================
*/
static void CG_Speaker( centity_t *cent ) 
{
	if ( ! cent->currentState.clientNum ) 
	{	
		// not auto triggering
		return;		
	}

	if ( cg.time < cent->miscTime ) 
	{
		return;
	}

	trap_S_StartSound (NULL, cent->currentState.number, CHAN_ITEM, cgs.gameSounds[cent->currentState.eventParm], -1, -1 );

	cent->miscTime = cg.time + cent->currentState.frame * 100 + cent->currentState.clientNum * 100 * crandom();
}

/*
==================
CG_Item
==================
*/
static void CG_Item( centity_t *cent ) 
{
	refEntity_t		ent;
	entityState_t	*es;
	gitem_t			*item;
	int				msec;
	float			frac;
	weaponInfo_t	*wi;

	es = &cent->currentState;
	if ( es->modelindex >= bg_numItems ) 
	{
		Com_Error( ERR_FATAL, "Bad item index %i on entity", es->modelindex );
	}

	// if set to invisible, skip
	if ( (!es->modelindex ) || ( es->eFlags & EF_NODRAW ) ) 
	{
		return;
	}	

	if ( !cg_items[es->modelindex].registered )
	{
		CG_RegisterItemVisuals ( es->modelindex);
	}

	item = &bg_itemlist[ es->modelindex ];
	
	// Track gametype items for the radar
	if ( es->modelindex >= MODELINDEX_GAMETYPE_ITEM && es->modelindex <= MODELINDEX_GAMETYPE_ITEM_2 )
	{
		cg.radarEntities[cg.radarEntityCount++] = cent;
	}

	// Simple items draws sprites only 
	if ( cg_simpleItems.integer && item->giType != IT_GAMETYPE && cg_items[es->modelindex].mSimpleIcon) 
	{
		memset( &ent, 0, sizeof( ent ) );
		ent.reType = RT_SPRITE;
		VectorCopy( cent->lerpOrigin, ent.origin );
		ent.radius = 8;
		ent.customShader = cg_items[es->modelindex].mSimpleIcon;
		ent.shaderRGBA[0] = 255;
		ent.shaderRGBA[1] = 255;
		ent.shaderRGBA[2] = 255;
		ent.shaderRGBA[3] = 255;
		trap_R_AddRefEntityToScene(&ent);
		return;
	}

	memset (&ent, 0, sizeof(ent));

	if (cent->currentState.eFlags & EF_ANGLE_OVERRIDE)
	{
		AnglesToAxis ( cent->currentState.angles, ent.axis );
	}
	else
	{
		vec3_t angles;

		VectorCopy ( cent->currentState.angles, angles );
		if ( item->giType == IT_WEAPON )
		{
			if ( item->giTag == WP_MM1_GRENADE_LAUNCHER )
			{
				angles[ROLL] += 135;
				cent->lerpOrigin[2] += 4;
			}
			else
			{
				angles[ROLL] += 90;
			}
		}

		AnglesToAxis ( angles, ent.axis );
	}

	wi = NULL;

	// the weapons have their origin where they attatch to player
	// models, so we need to offset them or they will rotate
	// eccentricly
	if (!(cent->currentState.eFlags & EF_ANGLE_OVERRIDE))
	{
		if ( item->giType == IT_WEAPON ) 
		{
			wi = &cg_weapons[item->giTag];
			cent->lerpOrigin[0] -= 
				wi->weaponMidpoint[0] * ent.axis[0][0] +
				wi->weaponMidpoint[1] * ent.axis[1][0] +
				wi->weaponMidpoint[2] * ent.axis[2][0];
			cent->lerpOrigin[1] -= 
				wi->weaponMidpoint[0] * ent.axis[0][1] +
				wi->weaponMidpoint[1] * ent.axis[1][1] +
				wi->weaponMidpoint[2] * ent.axis[2][1];
			cent->lerpOrigin[2] -= 
				wi->weaponMidpoint[0] * ent.axis[0][2] +
				wi->weaponMidpoint[1] * ent.axis[1][2] +
				wi->weaponMidpoint[2] * ent.axis[2][2];
		}

		// an extra height boost
		if ( item->giType == IT_WEAPON )
		{
			cent->lerpOrigin[2] -= 14;
		}
		else
		{
			cent->lerpOrigin[2] -= 15;
		}
	}

	ent.hModel = cg_items[es->modelindex].models[0];
	ent.ghoul2 = cg_items[es->modelindex].g2Models[0];
	ent.radius = cg_items[es->modelindex].radius[0];

	if ( ent.radius <= 0 )
	{
		ent.radius = 256;
	}

	VectorCopy (cent->lerpAngles, ent.angles);
	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);

	ent.nonNormalizedAxes = qfalse;

	// if just respawned, slowly scale up
	msec = cg.time - cent->miscTime;
	frac = 1.0;

	// items without glow textures need to keep a minimum light value
	// so they are always visible
//	if ( ( item->giType == IT_WEAPON ) || ( item->giType == IT_ARMOR ) || (item->giType == IT_GAMETYPE ) ||
//		 ( item->giType == IT_HEALTH) ) 
	{
		ent.renderfx |= RF_MINLIGHT;
	}

	// add to refresh list
	trap_R_AddRefEntityToScene(&ent);
}

/*
===============
CG_Missile
===============
*/
static void CG_Missile( centity_t *cent ) 
{
	refEntity_t				ent;
	entityState_t			*s1;
	const weaponInfo_t		*weapon;
	const attackInfo_t		*attack;

	s1 = &cent->currentState;
	if ( s1->weapon > WP_NUM_WEAPONS ) 
	{
		s1->weapon = 0;
	}
	weapon = &cg_weapons[s1->weapon];

	// calculate the axis
	VectorCopy( s1->angles, cent->lerpAngles);

	if ( s1->weapon == WP_KNIFE )
	{
		cent->lerpAngles[ROLL] += (float)cg.time * 1.75f;
	}

	// Determine which attack to use
	if ( cent->currentState.eFlags & EF_ALT_FIRING )
	{
		attack = &weapon->attack[ATTACK_ALTERNATE];
	}
	else
	{
		attack = &weapon->attack[ATTACK_NORMAL];
	}

	// add trails
	if ( attack->missileTrailFunc )  
	{
		if ( cg.time > cent->miscTime  )
		{
			attack->missileTrailFunc( cent, s1->weapon );
			cent->miscTime = cg.time + 25;
		}
	}

	// add dynamic light
	if ( attack->missileDlight ) 
	{
		trap_R_AddLightToScene(cent->lerpOrigin, 
							   attack->missileDlight, 
							   attack->missileDlightColor[0], 
							   attack->missileDlightColor[1], 
							   attack->missileDlightColor[2] );
	}

	// add missile sound
	if ( attack->missileSound ) 
	{
		vec3_t	velocity;

		BG_EvaluateTrajectoryDelta( &cent->currentState.pos, cg.time, velocity );

		trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, velocity, 500, attack->missileSound );
	}

	//Don't draw something without a model
	if (!trap_G2_HaveWeGhoul2Models( attack->missileG2Model) )
	{
		return;
	}

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);
	if (0 == cent->modelScale[0] && 0 == cent->modelScale[1] && 0 == cent->modelScale[2])
	{
		VectorSet( cent->modelScale, 1, 1, 1);
	}

	CG_SetGhoul2Info(&ent, cent);  

	// flicker between two skins
	ent.skinNum = cg.clientFrame & 1;
	ent.renderfx = RF_NOSHADOW;
	ent.ghoul2 = attack->missileG2Model;
	
	// spin as it moves
	if ( !(s1->eFlags&EF_ANGLE_OVERRIDE) && s1->apos.trType != TR_INTERPOLATE )
	{
		// convert direction of travel into axis
		if ( VectorNormalize2( s1->pos.trDelta, ent.axis[0] ) == 0 ) 
		{
			ent.axis[0][2] = 1;
		}

		// spin as it moves
		if ( s1->pos.trType != TR_STATIONARY ) 
		{
			RotateAroundDirection( ent.axis, cg.time * 0.25f );
		} 
		else 
		{
			RotateAroundDirection( ent.axis, (float)s1->time );
		}
	}
	else
	{
		AnglesToAxis( cent->lerpAngles, ent.axis );
	}

	// add to refresh list
	trap_R_AddRefEntityToScene( &ent );
}

/*
===============
CG_Mover
===============
*/
static void CG_Mover( centity_t *cent ) 
{
	refEntity_t			ent;
	entityState_t		*s1;

	s1 = &cent->currentState;

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);
	AnglesToAxis( cent->lerpAngles, ent.axis );

	ent.renderfx = RF_NOSHADOW;

	CG_SetGhoul2Info(&ent, cent);  

	// flicker between two skins (FIXME?)
	ent.skinNum = ( cg.time >> 6 ) & 1;

	// get the model, either as a bmodel or a modelindex
	if ( s1->solid == SOLID_BMODEL ) 
	{
		ent.hModel = cgs.inlineDrawModel[s1->modelindex];
	} 
	else 
	{
		ent.hModel = cgs.gameModels[s1->modelindex];
	}

	// add to refresh list
	trap_R_AddRefEntityToScene(&ent);

	// add the secondary model
	if ( s1->modelindex2 ) 
	{
		ent.skinNum = 0;
		ent.hModel = cgs.gameModels[s1->modelindex2];
		trap_R_AddRefEntityToScene(&ent);
	}
}

/*
===============
CG_Beam

Also called as an event
===============
*/
void CG_Beam( centity_t *cent ) 
{
	refEntity_t			ent;
	entityState_t		*s1;

	s1 = &cent->currentState;

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( s1->pos.trBase, ent.origin );
	VectorCopy( s1->origin2, ent.oldorigin );
	AxisClear( ent.axis );
	ent.reType = RT_BEAM;

	ent.renderfx = RF_NOSHADOW;

	CG_SetGhoul2Info(&ent, cent);  

	// add to refresh list
	trap_R_AddRefEntityToScene(&ent);
}


/*
===============
CG_Portal
===============
*/
static void CG_Portal( centity_t *cent ) 
{
	refEntity_t			ent;
	entityState_t		*s1;

	s1 = &cent->currentState;

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( cent->lerpOrigin, ent.origin );
	VectorCopy( s1->origin2, ent.oldorigin );
	ByteToDir( s1->eventParm, ent.axis[0] );
	PerpendicularVector( ent.axis[1], ent.axis[0] );

	// negating this tends to get the directions like they want
	// we really should have a camera roll value
	VectorSubtract( vec3_origin, ent.axis[1], ent.axis[1] );

	CrossProduct( ent.axis[0], ent.axis[1], ent.axis[2] );
	ent.reType = RT_PORTALSURFACE;
	ent.oldframe = s1->gametypeitems;
	ent.frame = s1->frame;		// rotation speed
	ent.skinNum = s1->clientNum/256.0 * 360;	// roll offset

	CG_SetGhoul2Info(&ent, cent);  

	// add to refresh list
	trap_R_AddRefEntityToScene(&ent);
}


/*
=========================
CG_AdjustPositionForMover

Also called by client movement prediction code
=========================
*/
void CG_AdjustPositionForMover( 
	const vec3_t	in, 
	int				moverNum, 
	int				fromTime, 
	int				toTime, 
	vec3_t			out 
	) 
{
	centity_t	*cent;
	vec3_t	oldOrigin, origin, deltaOrigin;
	vec3_t	oldAngles, angles, deltaAngles;

	if ( moverNum <= 0 || moverNum >= ENTITYNUM_MAX_NORMAL ) 
	{
		VectorCopy( in, out );
		return;
	}

	cent = CG_GetEntity ( moverNum );
	if ( cent->currentState.eType != ET_MOVER ) 
	{
		VectorCopy( in, out );
		return;
	}

	BG_EvaluateTrajectory( &cent->currentState.pos, fromTime, oldOrigin );
	BG_EvaluateTrajectory( &cent->currentState.apos, fromTime, oldAngles );

	BG_EvaluateTrajectory( &cent->currentState.pos, toTime, origin );
	BG_EvaluateTrajectory( &cent->currentState.apos, toTime, angles );

	VectorSubtract( origin, oldOrigin, deltaOrigin );
	VectorSubtract( angles, oldAngles, deltaAngles );

	VectorAdd( in, deltaOrigin, out );

	// FIXME: origin change when on a rotating object
}

/*
=============================
CG_InterpolateEntityPosition
=============================
*/
static void CG_InterpolateEntityPosition( centity_t *cent ) 
{
	vec3_t		current;
	vec3_t		next;
	float		f;

	// it would be an internal error to find an entity that interpolates without
	// a snapshot ahead of the current one
	if ( cg.nextSnap == NULL ) 
	{
		Com_Error( ERR_FATAL, "CG_InterpoateEntityPosition: cg.nextSnap == NULL" );
	}

	f = cg.frameInterpolation;	

	// this will linearize a sine or parabolic curve, but it is important
	// to not extrapolate player positions if more recent data is available
	BG_EvaluateTrajectory( &cent->currentState.pos, cg.snap->serverTime, current );
	BG_EvaluateTrajectory( &cent->nextState.pos, cg.nextSnap->serverTime, next );

	cent->lerpOrigin[0] = current[0] + f * ( next[0] - current[0] );
	cent->lerpOrigin[1] = current[1] + f * ( next[1] - current[1] );
	cent->lerpOrigin[2] = current[2] + f * ( next[2] - current[2] );

	BG_EvaluateTrajectory( &cent->currentState.apos, cg.snap->serverTime, current );
	BG_EvaluateTrajectory( &cent->nextState.apos, cg.nextSnap->serverTime, next );

	cent->lerpAngles[0] = LerpAngle( current[0], next[0], f );
	cent->lerpAngles[1] = LerpAngle( current[1], next[1], f );
	cent->lerpAngles[2] = LerpAngle( current[2], next[2], f );

	BG_EvaluateTrajectoryDelta( &cent->currentState.pos, cg.snap->serverTime, current );
	BG_EvaluateTrajectoryDelta( &cent->nextState.pos, cg.nextSnap->serverTime, next );
	cent->lerpVelocity[0] = current[0] + f * ( next[0] - current[0] );
	cent->lerpVelocity[1] = current[1] + f * ( next[1] - current[1] );
	cent->lerpVelocity[2] = current[2] + f * ( next[2] - current[2] );

	cent->lerpLeanOffset = cent->currentState.leanOffset + (int)(f * (float)(cent->nextState.leanOffset - cent->currentState.leanOffset)) - LEAN_OFFSET;
}

/*
===============
CG_CalcEntityLerpPositions
===============
*/
void CG_CalcEntityLerpPositions( centity_t *cent ) 
{
	// if this player does not want to see extrapolated players
	if ( !cg_smoothClients.integer ) 
	{
		// make sure the clients use TR_INTERPOLATE
		if ( cent->currentState.number < MAX_CLIENTS ) 
		{
			cent->currentState.pos.trType = TR_INTERPOLATE;
			cent->nextState.pos.trType = TR_INTERPOLATE;
		}
	}

	if ( cent->interpolate && cent->currentState.pos.trType == TR_INTERPOLATE ) 
	{
		CG_InterpolateEntityPosition( cent );
		return;
	}

	// first see if we can interpolate between two snaps for
	// linear extrapolated clients
	if ( cent->interpolate								 && 
		 cent->currentState.pos.trType == TR_LINEAR_STOP &&
		 cent->currentState.number < MAX_CLIENTS            ) 
	{
		CG_InterpolateEntityPosition( cent );
		return;
	}

	// just use the current frame and evaluate as best we can
	BG_EvaluateTrajectory( &cent->currentState.pos, cg.time, cent->lerpOrigin );
	BG_EvaluateTrajectory( &cent->currentState.apos, cg.time, cent->lerpAngles );
	VectorCopy ( cent->currentState.pos.trDelta, cent->lerpVelocity );
	cent->lerpLeanOffset = cent->currentState.leanOffset - LEAN_OFFSET;

	// adjust for riding a mover if it wasn't rolled into the predicted
	// player state
	if ( cent->currentState.number != cg.predictedPlayerState.clientNum )
	{
		CG_AdjustPositionForMover( cent->lerpOrigin, cent->currentState.groundEntityNum, 
								   cg.snap->serverTime, cg.time, cent->lerpOrigin );
	}
}

/*
===============
CG_DebugCylinder
===============
*/
void CG_DebugCylinder ( centity_t* cent )
{
	refEntity_t ref;
	vec3_t		normal;

	memset( &ref, 0, sizeof(ref) );
	VectorCopy ( cent->currentState.origin, ref.origin );
	VectorCopy ( cent->currentState.origin, ref.oldorigin );
	ref.origin[2] += 100;
	ref.radius = cent->currentState.time2;
	ref.rotation = cent->currentState.time2 + 10;
	ref.customShader = trap_R_RegisterShader( "line" );
	ref.reType = RT_CYLINDER;
	
	ref.shaderRGBA[0] = (unsigned char) (255.0f * g_color_table[ColorIndex(cent->currentState.time)][0]);
	ref.shaderRGBA[1] = (unsigned char) (255.0f * g_color_table[ColorIndex(cent->currentState.time)][1]);
	ref.shaderRGBA[2] = (unsigned char) (255.0f * g_color_table[ColorIndex(cent->currentState.time)][2]);
	ref.shaderRGBA[3] = 255;

	VectorSubtract(ref.oldorigin, ref.origin, normal);
	VectorNormalize(normal);
	VectorCopy( normal, ref.axis[0] );
	RotateAroundDirection( ref.axis, 0);

	trap_R_AddRefEntityToScene ( &ref ); 
}

static void CG_AddLocalSet( centity_t *cent )
{
	cent->ambientSetTime = trap_AS_AddLocalSet( CG_ConfigString( CS_AMBIENT_SOUNDSETS + cent->currentState.mSoundSet/*localSoundSet*/), cg.refdef.vieworg, cent->lerpOrigin, cent->currentState.number, cent->ambientSetTime );
}

/*
===============
CG_AddCEntity
===============
*/
static void CG_AddCEntity( centity_t *cent ) 
{
	// event-only entities will have been dealt with already
	if ( cent->currentState.eType >= ET_EVENTS ) 
	{
		return;
	}

	// calculate the current origin
	CG_CalcEntityLerpPositions( cent );

	// add automatic effects
	CG_EntityEffects( cent );

	// add local sound set if any
	if ( cent->currentState.mSoundSet && cent->currentState.eType != ET_MOVER )
	{
		CG_AddLocalSet( cent );		
	}

	// do this before we copy the data to refEnts
	if (trap_G2_HaveWeGhoul2Models(cent->ghoul2))
	{
		trap_G2_SetGhoul2ModelIndexes(cent->ghoul2, cgs.gameModels, cgs.skins);
	}

	switch ( cent->currentState.eType ) 
	{
		default:
			Com_Error( ERR_FATAL, "Bad entity type: %i\n", cent->currentState.eType );
			break;

		case ET_DAMAGEAREA:
		case ET_INVISIBLE:
		case ET_PUSH_TRIGGER:
		case ET_TELEPORT_TRIGGER:
		case ET_TERRAIN:
			break;

		case ET_GENERAL:
			CG_General( cent );
			break;

		case ET_PLAYER:
			CG_Player( cent );
			break;

		case ET_ITEM:
			CG_Item( cent );
			break;

		case ET_MISSILE:
			CG_Missile( cent );
			break;

		case ET_MOVER:
			CG_Mover( cent );
			break;

		case ET_BEAM:
			CG_Beam( cent );
			break;

		case ET_PORTAL:
			CG_Portal( cent );
			break;

		case ET_SPEAKER:
			CG_Speaker( cent );
			break;

		case ET_BODY:

			// If the bodies are to stay around forever and there
			// is a body time set then see if we should hide it
			if ( cg_bodyTime.integer && cent->currentState.time2 )
			{
				int time = cg_bodyTime.integer * 1000;

				if ( time < BODY_SINK_DELAY )
				{
					time = BODY_SINK_DELAY;
				}

				time += cent->currentState.time2;

				// Body is long gone
				if ( cg.time > time + BODY_SINK_TIME )
				{
					return;
				}

				// Sink the body
				if ( cg.time > time )
				{
					cent->lerpOrigin[2] -= ((float)(cg.time - time) / (float)BODY_SINK_TIME) * (BODY_SINK_TIME/100.0f);

					// Hack to stop shadows from showing up
					cent->currentState.eFlags |= EF_NOSHADOW;
				}
			}

			// Dont show your own dead body unless in third person
			if ( !cg.renderingThirdPerson )
			{
				if ( cg.snap->ps.clientNum == cent->currentState.otherEntityNum )
				{
					if ( cg_entities[cg.snap->ps.clientNum].currentState.eFlags & EF_DEAD )
					{
						return;
					}
				}
			}

			CG_General( cent );
			break;

		case ET_DEBUG_CYLINDER:
			CG_DebugCylinder ( cent );
			break;
	}
}

/*
===============
CG_AddPacketEntities
===============
*/
void CG_AddPacketEntities( void ) 
{
	int			num;
	centity_t	*cent;

	// set cg.frameInterpolation
	if ( cg.nextSnap ) 
	{
		int		delta;

		delta = (cg.nextSnap->serverTime - cg.snap->serverTime);
		if ( delta == 0 ) 
		{
			cg.frameInterpolation = 0;
		} 
		else 
		{
			cg.frameInterpolation = (float)( cg.time - cg.snap->serverTime ) / delta;
		}
	} 
	else 
	{
		// actually, it should never be used, because 
		// no entities should be marked as interpolating
		cg.frameInterpolation = 0;									
	}

	// the auto-rotating items will all have the same axis
	cg.autoAngles[0] = 0;
	cg.autoAngles[1] = ( cg.time & 2047 ) * 360 / 2048.0;
	cg.autoAngles[2] = 0;

	cg.autoAnglesFast[0] = 0;
	cg.autoAnglesFast[1] = ( cg.time & 1023 ) * 360 / 1024.0f;
	cg.autoAnglesFast[2] = 0;

	AnglesToAxis( cg.autoAngles, cg.autoAxis );
	AnglesToAxis( cg.autoAnglesFast, cg.autoAxisFast );

	// generate and add the entity from the playerstate

	// add in the Ghoul2 stuff.
//	VectorCopy( cg_entities[ cg.snap->ps.clientNum].modelScale, cg.predictedPlayerEntity.modelScale);
//	cg.predictedPlayerEntity.radius = cg_entities[ cg.snap->ps.clientNum].radius;

//	num = cg_entitiescg.predictedPlayerEntity.currentState.number;
	BG_PlayerStateToEntityState( &cg.predictedPlayerState, 
								 &cg_entities[cg.predictedPlayerState.clientNum].currentState, 
								 qfalse );

/*
	// If the client number changes then we have to blow away our current model
	if ( num != cg.predictedPlayerEntity.currentState.number )
	{
		trap_G2API_CleanGhoul2Models ( &cg.predictedPlayerEntity.ghoul2 );
		cg.predictedPlayerEntity.pe.weaponModelSpot = 0;
		cg.predictedPlayerEntity.pe.weapon = 0;
		cg.predictedPlayerEntity.ghoul2 = NULL;
	}
*/

	cg_entities[cg.predictedPlayerState.clientNum].interpolate = qfalse;
	CG_AddCEntity( &cg_entities[cg.predictedPlayerState.clientNum] );

	// lerp the non-predicted value for lightning gun origins
//	CG_CalcEntityLerpPositions( &cg_entities[ cg.snap->ps.clientNum ] );

	// Reset radar entities
	cg.radarEntityCount = 0;

	// add each entity sent over by the server
	for ( num = 0 ; num < cg.snap->numEntities ; num++ ) 
	{
		// Don't re-add ents that have been predicted.
		if (cg.snap->entities[ num ].number != cg.snap->ps.clientNum)
		{
			cent = CG_GetEntity ( cg.snap->entities[ num ].number ); 
			CG_AddCEntity( cent );
		}
	}

	for(num=0;num<cg_numpermanents;num++)
	{
		cent = cg_permanents[num];
		if (cent->currentValid)
		{
			CG_AddCEntity( cent );
		}
	}
}

