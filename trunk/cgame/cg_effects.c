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
// cg_effects.c -- these functions generate localentities, usually as a result
// of event processing

#include "cg_local.h"

/*
==================
CG_BubbleTrail

Bullets shot underwater
==================
*/
void CG_BubbleTrail( vec3_t start, vec3_t end, float spacing ) {
	vec3_t		move;
	vec3_t		vec;
	float		len;
	int			i;

	if ( cg_noProjectileTrail.integer ) {
		return;
	}

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	// advance a random amount first
	i = rand() % (int)spacing;
	VectorMA( move, i, vec, move );

	VectorScale (vec, spacing, vec);

	for ( ; i < len; i += spacing ) {
		localEntity_t	*le;
		refEntity_t		*re;

		le = CG_AllocLocalEntity();
		le->leFlags = LEF_PUFF_DONT_SCALE;
		le->leType = LE_MOVE_SCALE_FADE;
		le->startTime = cg.time;
		le->endTime = cg.time + 1000 + random() * 250;
		le->lifeRate = 1.0 / ( le->endTime - le->startTime );

		re = &le->refEntity;
		re->shaderTime = cg.time / 1000.0f;

		re->reType = RT_SPRITE;
		re->rotation = 0;
		re->radius = 3;
		re->customShader = cgs.media.waterBubbleShader;
		re->shaderRGBA[0] = 0xff;
		re->shaderRGBA[1] = 0xff;
		re->shaderRGBA[2] = 0xff;
		re->shaderRGBA[3] = 0xff;

		le->color[3] = 1.0;

		le->pos.trType = TR_LINEAR;
		le->pos.trTime = cg.time;
		VectorCopy( move, le->pos.trBase );
		le->pos.trDelta[0] = crandom()*5;
		le->pos.trDelta[1] = crandom()*5;
		le->pos.trDelta[2] = crandom()*5 + 6;

		VectorAdd (move, vec, move);
	}
}


void CG_TestLine( vec3_t start, vec3_t end, int time, unsigned int color, int radius) 
{
	localEntity_t	*le;
	refEntity_t		*re;

	le = CG_AllocLocalEntity();
	le->leType = LE_LINE;
	le->startTime = cg.time;
	le->endTime = cg.time + time;
	le->lifeRate = 1.0 / ( le->endTime - le->startTime );

	re = &le->refEntity;
	VectorCopy( start, re->origin );
	VectorCopy( end, re->oldorigin);
	re->shaderTime = cg.time / 1000.0f;

	re->reType = RT_LINE;
	re->radius = 0.5*radius;
	re->customShader = cgs.media.whiteShader; //trap_R_RegisterShaderNoMip("textures/colombia/canvas_doublesided");

	re->shaderTexCoord[0] = re->shaderTexCoord[1] = 1.0f;

	if (color==0)
	{
		re->shaderRGBA[0] = re->shaderRGBA[1] = re->shaderRGBA[2] = re->shaderRGBA[3] = 0xff;
	}
	else
	{
		re->shaderRGBA[0] = color & 0xff;
		color >>= 8;
		re->shaderRGBA[1] = color & 0xff;
		color >>= 8;
		re->shaderRGBA[2] = color & 0xff;
		re->shaderRGBA[3] = 0xff;
	}

	le->color[3] = 1.0;
}

/*
==================
CG_ThrowShard
==================
*/
void CG_ThrowShard( vec3_t origin, vec3_t velocity, qhandle_t hModel ) {
	localEntity_t	*le;
	refEntity_t		*re;

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_FRAGMENT;
	le->startTime = cg.time;
	le->endTime = le->startTime + 5000 + random() * 3000;

	VectorCopy( origin, re->origin );
	AxisCopy( axisDefault, re->axis );
	re->hModel = hModel;

	le->pos.trType = TR_GRAVITY;
	le->angles.trType = TR_GRAVITY;
	VectorCopy( origin, le->pos.trBase );
	VectorCopy( velocity, le->pos.trDelta );
	VectorSet(le->angles.trBase, 20, 20, 20);
	VectorCopy( velocity, le->angles.trDelta );
	le->pos.trTime = cg.time;
	le->angles.trTime = cg.time;

	le->leFlags = LEF_TUMBLE;

	le->angles.trBase[YAW] = 180;

	le->bounceFactor = 0.3f;
}

/*
==================
CG_GlassShatter
Throws glass shards from within a given bounding box in the world
==================
*/
void CG_GlassShatter(int entnum, vec3_t org, vec3_t mins, vec3_t maxs)
{
	vec3_t velocity, a, shardorg, dif, difx;
	float windowmass;
	float shardsthrow = 0;

	trap_S_StartSound(org, entnum, CHAN_BODY, cgs.media.glassBreakSound, -1, -1 );

	VectorSubtract(maxs, mins, a);

	windowmass = VectorLength(a); //should give us some idea of how big the chunk of glass is

	while (shardsthrow < windowmass)
	{
		velocity[0] = crandom()*150;
		velocity[1] = crandom()*150;
		velocity[2] = 150 + crandom()*75;

		VectorCopy(org, shardorg);
	
		dif[0] = (maxs[0]-mins[0])/2;
		dif[1] = (maxs[1]-mins[1])/2;
		dif[2] = (maxs[2]-mins[2])/2;

		if (dif[0] < 2)
		{
			dif[0] = 2;
		}
		if (dif[1] < 2)
		{
			dif[1] = 2;
		}
		if (dif[2] < 2)
		{
			dif[2] = 2;
		}

		difx[0] = (float) Q_irand(1, (int)((dif[0]*0.9)*2));
		difx[1] = (float) Q_irand(1, (int)((dif[1]*0.9)*2));
		difx[2] = (float) Q_irand(1, (int)((dif[2]*0.9)*2));

		if (difx[0] > dif[0])
		{
			shardorg[0] += difx[0]-(dif[0]);
		}
		else
		{
			shardorg[0] -= difx[0];
		}
		if (difx[1] > dif[1])
		{
			shardorg[1] += difx[1]-(dif[1]);
		}
		else
		{
			shardorg[1] -= difx[1];
		}
		if (difx[2] > dif[2])
		{
			shardorg[2] += difx[2]-(dif[2]);
		}
		else
		{
			shardorg[2] -= difx[2];
		}

		trap_FX_PlayEffectID( cgs.media.glassChunkEffect, shardorg, velocity, -1, -1 );

		shardsthrow += 20;
	}
}

qboolean CG_PointLineIntersect ( vec3_t start, vec3_t end, vec3_t point, float rad, vec3_t intersection )
{
	vec3_t dir;
	vec3_t distance;
	float  len;
	float  lineSize;

	VectorSubtract ( end, start, dir );
	lineSize = VectorNormalize ( dir );

	// Calculate the distnace from the shooter to the target
	VectorSubtract ( point, start, distance );

	// Use that distnace to determine the point of tangent in relation to
	// the center of the player entity
	VectorMA ( start, DotProduct ( dir, distance ), dir, intersection );

	VectorSubtract ( intersection, point, distance );
	len = VectorLengthSquared ( distance );

	// Is the intersection point within the given radius requirements?
	if ( len < rad * rad )
	{
		// Make sure its not past the end of the given line
		VectorSubtract ( intersection, start, distance );
		len = VectorLengthSquared ( distance );

		// If the len
		if ( len < lineSize * lineSize )
		{
			return qtrue;
		}
	}

	return qfalse;
}

void CG_BulletFlyBySound ( vec3_t start, vec3_t end )
{
	// make the incidental sounds - all of these should already be precached on the server
	vec3_t	soundPoint;
	if( CG_PointLineIntersect(start, end, cg.refdef.vieworg, 100, soundPoint ) )
	{
		if (irand(1, 10) < 5)
		{
			int which = irand(0, NUMFLYBYS - 1);
			
			trap_S_StartSound (soundPoint, cg.predictedPlayerState.clientNum, CHAN_AUTO, cgs.media.flybySounds[which], -1, -1 );
		}
	}
}
