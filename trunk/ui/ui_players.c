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
// ui_players.c

#include "ui_local.h"
#include "..\ghoul2\g2.h"


#define UI_TIMER_GESTURE		2300
#define UI_TIMER_JUMP			1000
#define UI_TIMER_LAND			130
#define UI_TIMER_WEAPON_SWITCH	300
#define UI_TIMER_ATTACK			500
#define	UI_TIMER_MUZZLE_FLASH	20
#define	UI_TIMER_WEAPON_DELAY	250

#define JUMP_HEIGHT				56

#define SWINGSPEED				0.3f

#define SPIN_SPEED				0.9f
#define COAST_TIME				1000


static int			dp_realtime;
static float		jumpHeight;


/*
===============
UI_ForceLegsAnim
===============
*/
static void UI_ForceLegsAnim( playerInfo_t *pi, int anim ) 
{
	pi->legsAnim = ( ( pi->legsAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;

	if ( anim == LEGS_JUMP ) 
	{
		pi->legsAnimationTimer = UI_TIMER_JUMP;
	}
}


/*
===============
UI_SetLegsAnim
===============
*/
static void UI_SetLegsAnim( playerInfo_t *pi, int anim ) {
	if ( pi->pendingLegsAnim ) {
		anim = pi->pendingLegsAnim;
		pi->pendingLegsAnim = 0;
	}
	UI_ForceLegsAnim( pi, anim );
}


/*
===============
UI_ForceTorsoAnim
===============
*/
static void UI_ForceTorsoAnim( playerInfo_t *pi, int anim ) 
{
	pi->torsoAnim = ( ( pi->torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;
}


/*
===============
UI_SetTorsoAnim
===============
*/
static void UI_SetTorsoAnim( playerInfo_t *pi, int anim ) {
	if ( pi->pendingTorsoAnim ) {
		anim = pi->pendingTorsoAnim;
		pi->pendingTorsoAnim = 0;
	}

	UI_ForceTorsoAnim( pi, anim );
}


/*
===============
UI_TorsoSequencing
===============
*/
static void UI_TorsoSequencing( playerInfo_t *pi ) 
{
	int		currentAnim;

	currentAnim = pi->torsoAnim & ~ANIM_TOGGLEBIT;

	if ( pi->torsoAnimationTimer > 0 ) 
	{
		return;
	}
}


/*
===============
UI_LegsSequencing
===============
*/
static void UI_LegsSequencing( playerInfo_t *pi ) 
{
	int		currentAnim;

	currentAnim = pi->legsAnim & ~ANIM_TOGGLEBIT;

	if ( pi->legsAnimationTimer > 0 ) 
	{
		if ( currentAnim == LEGS_JUMP ) 
		{
			jumpHeight = JUMP_HEIGHT * sin( M_PI * ( UI_TIMER_JUMP - pi->legsAnimationTimer ) / UI_TIMER_JUMP );
		}
		return;
	}
}

/*
===============
UI_SetLerpFrameAnimation
===============
*/
static void UI_SetLerpFrameAnimation( playerInfo_t *ci, lerpFrame_t *lf, int newAnimation ) {
	animation_t	*anim;

	lf->animationNumber = newAnimation;
	newAnimation &= ~ANIM_TOGGLEBIT;

	if ( newAnimation < 0 || newAnimation >= MAX_ANIMATIONS ) {
		trap_Error( va("Bad animation number: %i", newAnimation) );
	}

	anim = &ci->animations[ newAnimation ];

	lf->animation = anim;
	lf->animationTime = lf->frameTime + anim->initialLerp;
}


/*
===============
UI_RunLerpFrame
===============
*/
static void UI_RunLerpFrame( playerInfo_t *ci, lerpFrame_t *lf, int newAnimation ) {
	int			f;
	animation_t	*anim;

	// see if the animation sequence is switching
	if ( newAnimation != lf->animationNumber || !lf->animation ) {
		UI_SetLerpFrameAnimation( ci, lf, newAnimation );
	}

	f = 0;

	// if we have passed the current frame, move it to
	// oldFrame and calculate a new frame
	if ( dp_realtime >= lf->frameTime ) {
		lf->oldFrame = lf->frame;
		lf->oldFrameTime = lf->frameTime;

		// get the next frame based on the animation
		anim = lf->animation;
		if (lf->animation->numFrames)
		{
			if ( dp_realtime < lf->animationTime ) {
				lf->frameTime = lf->animationTime;		// initial lerp
			} else {
				lf->frameTime = lf->oldFrameTime + anim->frameLerp;
			}
			f = ( lf->frameTime - lf->animationTime ) / anim->frameLerp;

			if ( f >= anim->numFrames ) {
				f -= anim->numFrames;
				if ( anim->loopFrames != -1 ) //Before 0 meant no loop
				{
					if(anim->numFrames - anim->loopFrames == 0)
					{
						f %= anim->numFrames;
					}
					else
					{
						f %= (anim->numFrames - anim->loopFrames);
					}
					f += anim->loopFrames;
				} 
				else 
				{
					f = anim->numFrames - 1;
					// the animation is stuck at the end, so it
					// can immediately transition to another sequence
					lf->frameTime = dp_realtime;
				}
			}
		}

		lf->frame = anim->firstFrame + f;
		if ( dp_realtime > lf->frameTime ) {
			lf->frameTime = dp_realtime;
		}
	}

	if ( lf->frameTime > dp_realtime + 200 ) {
		lf->frameTime = dp_realtime;
	}

	if ( lf->oldFrameTime > dp_realtime ) {
		lf->oldFrameTime = dp_realtime;
	}
	// calculate current lerp value
	if ( lf->frameTime == lf->oldFrameTime ) {
		lf->backlerp = 0;
	} else {
		lf->backlerp = 1.0 - (float)( dp_realtime - lf->oldFrameTime ) / ( lf->frameTime - lf->oldFrameTime );
	}
}


/*
===============
UI_PlayerAnimation
===============
*/
static void UI_PlayerAnimation( playerInfo_t *pi, int *legsOld, int *legs, float *legsBackLerp,
						int *torsoOld, int *torso, float *torsoBackLerp ) 
{

	// legs animation
	pi->legsAnimationTimer -= uiInfo.uiDC.frameTime;
	if ( pi->legsAnimationTimer < 0 ) {
		pi->legsAnimationTimer = 0;
	}

	UI_LegsSequencing( pi );

	if ( pi->legs.yawing && ( pi->legsAnim & ~ANIM_TOGGLEBIT ) == TORSO_IDLE_PISTOL ) {
		UI_RunLerpFrame( pi, &pi->legs, TORSO_IDLE_PISTOL );
	} else {
		UI_RunLerpFrame( pi, &pi->legs, pi->legsAnim );
	}
	*legsOld = pi->legs.oldFrame;
	*legs = pi->legs.frame;
	*legsBackLerp = pi->legs.backlerp;

	// torso animation
	pi->torsoAnimationTimer -= uiInfo.uiDC.frameTime;
	if ( pi->torsoAnimationTimer < 0 ) {
		pi->torsoAnimationTimer = 0;
	}

	UI_TorsoSequencing( pi );

	UI_RunLerpFrame( pi, &pi->torso, pi->torsoAnim );
	*torsoOld = pi->torso.oldFrame;
	*torso = pi->torso.frame;
	*torsoBackLerp = pi->torso.backlerp;
}


/*
==================
UI_SwingAngles
==================
*/
static void UI_SwingAngles( float destination, float swingTolerance, float clampTolerance,
					float speed, float *angle, qboolean *swinging ) {
	float	swing;
	float	move;
	float	scale;

	if ( !*swinging ) {
		// see if a swing should be started
		swing = AngleSubtract( *angle, destination );
		if ( swing > swingTolerance || swing < -swingTolerance ) {
			*swinging = qtrue;
		}
	}

	if ( !*swinging ) {
		return;
	}
	
	// modify the speed depending on the delta
	// so it doesn't seem so linear
	swing = AngleSubtract( destination, *angle );
	scale = fabs( swing );
	if ( scale < swingTolerance * 0.5 ) {
		scale = 0.5;
	} else if ( scale < swingTolerance ) {
		scale = 1.0;
	} else {
		scale = 2.0;
	}

	// swing towards the destination angle
	if ( swing >= 0 ) {
		move = uiInfo.uiDC.frameTime * scale * speed;
		if ( move >= swing ) {
			move = swing;
			*swinging = qfalse;
		}
		*angle = AngleMod( *angle + move );
	} else if ( swing < 0 ) {
		move = uiInfo.uiDC.frameTime * scale * -speed;
		if ( move <= swing ) {
			move = swing;
			*swinging = qfalse;
		}
		*angle = AngleMod( *angle + move );
	}

	// clamp to no more than tolerance
	swing = AngleSubtract( destination, *angle );
	if ( swing > clampTolerance ) {
		*angle = AngleMod( destination - (clampTolerance - 1) );
	} else if ( swing < -clampTolerance ) {
		*angle = AngleMod( destination + (clampTolerance - 1) );
	}
}


/*
======================
UI_MovedirAdjustment
======================
*/
static float UI_MovedirAdjustment( playerInfo_t *pi ) {
	vec3_t		relativeAngles;
	vec3_t		moveVector;

	VectorSubtract( pi->viewAngles, pi->moveAngles, relativeAngles );
	AngleVectors( relativeAngles, moveVector, NULL, NULL );
	if ( Q_fabs( moveVector[0] ) < 0.01 ) {
		moveVector[0] = 0.0;
	}
	if ( Q_fabs( moveVector[1] ) < 0.01 ) {
		moveVector[1] = 0.0;
	}

	if ( moveVector[1] == 0 && moveVector[0] > 0 ) {
		return 0;
	}
	if ( moveVector[1] < 0 && moveVector[0] > 0 ) {
		return 22;
	}
	if ( moveVector[1] < 0 && moveVector[0] == 0 ) {
		return 45;
	}
	if ( moveVector[1] < 0 && moveVector[0] < 0 ) {
		return -22;
	}
	if ( moveVector[1] == 0 && moveVector[0] < 0 ) {
		return 0;
	}
	if ( moveVector[1] > 0 && moveVector[0] < 0 ) {
		return 22;
	}
	if ( moveVector[1] > 0 && moveVector[0] == 0 ) {
		return  -45;
	}

	return -22;
}


/*
===============
UI_PlayerAngles
===============
*/
static void UI_PlayerAngles( playerInfo_t *pi, vec3_t legs[3], vec3_t torso[3], vec3_t head[3] ) {
	vec3_t		legsAngles, torsoAngles, headAngles;
	float		dest;
	float		adjust;

	VectorCopy( pi->viewAngles, headAngles );
	headAngles[YAW] = AngleMod( headAngles[YAW] );
	VectorClear( legsAngles );
	VectorClear( torsoAngles );

	// --------- yaw -------------

	// allow yaw to drift a bit
	if ( ( pi->legsAnim & ~ANIM_TOGGLEBIT ) != LEGS_WALK ) 
	{
		// if not standing still, always point all in the same direction
		pi->torso.yawing = qtrue;	// always center
		pi->torso.pitching = qtrue;	// always center
		pi->legs.yawing = qtrue;	// always center
	}

	// adjust legs for movement dir
	adjust = UI_MovedirAdjustment( pi );
	legsAngles[YAW] = headAngles[YAW] + adjust;
	torsoAngles[YAW] = headAngles[YAW] + 0.25 * adjust;


	// torso
	UI_SwingAngles( torsoAngles[YAW], 25, 90, SWINGSPEED, &pi->torso.yawAngle, &pi->torso.yawing );
	UI_SwingAngles( legsAngles[YAW], 40, 90, SWINGSPEED, &pi->legs.yawAngle, &pi->legs.yawing );

	torsoAngles[YAW] = pi->torso.yawAngle;
	legsAngles[YAW] = pi->legs.yawAngle;

	// --------- pitch -------------

	// only show a fraction of the pitch angle in the torso
	if ( headAngles[PITCH] > 180 ) {
		dest = (-360 + headAngles[PITCH]) * 0.75;
	} else {
		dest = headAngles[PITCH] * 0.75;
	}
	UI_SwingAngles( dest, 15, 30, 0.1f, &pi->torso.pitchAngle, &pi->torso.pitching );
	torsoAngles[PITCH] = pi->torso.pitchAngle;

	// pull the angles back out of the hierarchial chain
	AnglesSubtract( headAngles, torsoAngles, headAngles );
	AnglesSubtract( torsoAngles, legsAngles, torsoAngles );
	AnglesToAxis( legsAngles, legs );
	AnglesToAxis( torsoAngles, torso );
	AnglesToAxis( headAngles, head );
}


/*
===============
UI_PlayerFloatSprite
===============
*/
static void UI_PlayerFloatSprite( playerInfo_t *pi, vec3_t origin, qhandle_t shader ) {
	refEntity_t		ent;

	memset( &ent, 0, sizeof( ent ) );
	VectorCopy( origin, ent.origin );
	ent.origin[2] += 48;
	ent.reType = RT_SPRITE;
	ent.customShader = shader;
	ent.radius = 10;
	ent.renderfx = 0;
	trap_R_AddRefEntityToScene( &ent );
}


/*
======================
UI_MachinegunSpinAngle
======================
*/
float	UI_MachinegunSpinAngle( playerInfo_t *pi ) {
	int		delta;
	float	angle;
	float	speed;
	int		torsoAnim;

	delta = dp_realtime - pi->barrelTime;
	if ( pi->barrelSpinning ) {
		angle = pi->barrelAngle + delta * SPIN_SPEED;
	} else {
		if ( delta > COAST_TIME ) {
			delta = COAST_TIME;
		}

		speed = 0.5 * ( SPIN_SPEED + (float)( COAST_TIME - delta ) / COAST_TIME );
		angle = pi->barrelAngle + delta * speed;
	}

	torsoAnim = pi->torsoAnim  & ~ANIM_TOGGLEBIT;

	return angle;
}


/*
===============
UI_DrawPlayer
===============
*/
void UI_DrawPlayer( float x, float y, float w, float h, playerInfo_t *pi, int time ) 
{
	refdef_t		refdef;
	refEntity_t		legs;
	refEntity_t		torso;
	refEntity_t		head;
	refEntity_t		gun;
	vec3_t			origin;
	int				renderfx;
	vec3_t			mins = {-16, -16, -24};
	vec3_t			maxs = {16, 16, 32};
	float			len;
	float			xx;
	animation_t		*anim;
	float			animSpeed;
	int				flags=BONE_ANIM_OVERRIDE_FREEZE;

	if ( !pi->playerG2Model ) // || !pi->animations[0].numFrames ) 
	{
		return;
	}

	// this allows the ui to cache the player model on the main menu
	if (w == 0 || h == 0) {
		return;
	}

	dp_realtime = time;

	if ( pi->pendingWeapon != -1 && dp_realtime > pi->weaponTimer ) {
		pi->weapon = pi->pendingWeapon;
		pi->lastWeapon = pi->pendingWeapon;
		pi->pendingWeapon = -1;
		pi->weaponTimer = 0;
	}

	UI_AdjustFrom640( &x, &y, &w, &h );

	y -= jumpHeight;

	memset( &refdef, 0, sizeof( refdef ) );
	memset( &legs, 0, sizeof(legs) );
	memset( &torso, 0, sizeof(torso) );
	memset( &head, 0, sizeof(head) );

	refdef.rdflags = RDF_NOWORLDMODEL;

	AxisClear( refdef.viewaxis );

	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	refdef.fov_x = (int)((float)refdef.width / 640.0f * 90.0f);
	xx = refdef.width / tan( refdef.fov_x / 360 * M_PI );
	refdef.fov_y = atan2( refdef.height, xx );
	refdef.fov_y *= ( 360 / (float)M_PI );

	// calculate distance so the player nearly fills the box
	len = 0.8f * ( maxs[2] - mins[2] );		
	origin[0] = len / tan( DEG2RAD(refdef.fov_x) * 0.5 );
	origin[1] = 0.5 * ( mins[1] + maxs[1] );
	origin[2] = -0.5 * ( mins[2] + maxs[2] );

	refdef.time = dp_realtime;

	trap_R_ClearScene();

	// get the rotation information
	UI_PlayerAngles( pi, legs.axis, torso.axis, head.axis );
	
	// get the animation state (after rotation, to allow feet shuffle)
	UI_PlayerAnimation( pi, &legs.oldframe, &legs.frame, &legs.backlerp,
		 &torso.oldframe, &torso.frame, &torso.backlerp );

	renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW | RF_MINLIGHT;

	//
	// add the legs
	//
//	legs.hModel = pi->playerModel;
//	legs.customSkin = pi->legsSkin;
	legs.ghoul2 = pi->playerG2Model;

//	lf->animation = anim;
//	lf->animationTime = lf->frameTime + anim->initialLerp;

	anim = pi->legs.animation;
	animSpeed = 50.0f / anim->frameLerp;
	if (anim->loopFrames != -1)
	{
		flags = BONE_ANIM_OVERRIDE_LOOP;
	}

	// Temp
//		flags |= BONE_ANIM_BLEND;

/*	if (torsoOnly)
	{
		trap_G2API_SetBoneAnim(cent->ghoul2, 0, "upper_lumbar", anim->firstFrame, anim->firstFrame + anim->numFrames, flags, animSpeed, lf->frameTime, -1, 150);
	}*/
//	else
	if ( pi->legsOldAnim != pi->legs.animationNumber )
	{
		trap_G2API_SetBoneAnim( legs.ghoul2, 0, "model_root", 
								anim->firstFrame, 
								anim->firstFrame + anim->numFrames, 
								flags, animSpeed, pi->legs.frameTime / 10, -1, 150);
		pi->legsOldAnim = pi->legs.animationNumber;
	}

	VectorCopy( origin, legs.origin );

	VectorCopy( origin, legs.lightingOrigin );
	legs.renderfx = renderfx;
	VectorCopy (legs.origin, legs.oldorigin);

	trap_R_AddRefEntityToScene( &legs );

//	if (!legs.hModel) {
//		return;
//	}

	//
	// add the gun
	//
	if ( pi->currentWeapon != WP_NONE ) {
		memset( &gun, 0, sizeof(gun) );
//		gun.hModel = pi->weaponModel;
//		VectorCopy( origin, gun.lightingOrigin );
		gun.ghoul2 = pi->weaponG2Model;

		// NOTENOTE Need to change this to a bolt position on player model
//rjr		UI_PositionEntityOnTag( &gun, &torso, pi->torsoModel, "tag_weapon");
		gun.renderfx = renderfx;
		trap_R_AddRefEntityToScene( &gun );
	}

	//
	// add the chat icon
	//
	if ( pi->chat ) {
		UI_PlayerFloatSprite( pi, origin, trap_R_RegisterShaderNoMip( "sprites/balloon3" ) );
	}

	//
	// add an accent light
	//
	origin[0] -= 100;	// + = behind, - = in front
	origin[1] += 100;	// + = left, - = right
	origin[2] += 100;	// + = above, - = below
	trap_R_AddLightToScene( origin, 500, 0.5, 0.5, 0.5 );

	origin[0] -= 100;
	origin[1] -= 100;
	origin[2] -= 100;
	trap_R_AddLightToScene( origin, 500, 0.5, 0.5, 0.5 );

	trap_R_RenderScene( &refdef );
}

/*
==========================
UI_FileExists
==========================
*/
static qboolean	UI_FileExists(const char *filename) {
	int len;
	fileHandle_t	f;

	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if (len>0) {
		trap_FS_FCloseFile(f);
		return qtrue;
	}
	return qfalse;
}

/*
==========================
UI_RegisterClientIdentity
==========================
*/
qboolean UI_RegisterClientIdentity ( playerInfo_t *pi, const char* identityName ) 
{
	char filename[MAX_QPATH];

	if (pi->playerG2Model)
	{
		trap_G2API_CleanGhoul2Models(&pi->playerG2Model);
		pi->playerG2Model = NULL;
	}

	pi->playerG2Model = UI_RegisterIdentity ( identityName, filename );
	if ( !pi->playerG2Model )
	{
		Com_Printf( "Failed to read Ghoul2 Skin%s\n", filename );
		return qfalse;
	}

	// load the animations
	if ( !BG_ParseAnimationFile( filename, pi->animations ) ) 
	{
		Com_Printf( "Failed to load animation file %s\n", filename );
		return qfalse;
	}

	pi->legsOldAnim = 0;

	return qtrue;
}


/*
===============
UI_PlayerInfo_SetIdentity
===============
*/
void UI_PlayerInfo_SetIdentity ( playerInfo_t *pi, const char *identity ) 
{
	memset( pi, 0, sizeof(*pi) );
	UI_RegisterClientIdentity ( pi, identity );
	pi->weapon = WP_FIRST_RANGED_WEAPON;
	pi->currentWeapon = pi->weapon;
	pi->lastWeapon = pi->weapon;
	pi->pendingWeapon = -1;
	pi->weaponTimer = 0;
	pi->chat = qfalse;
	pi->newModel = qtrue;
}


/*
===============
UI_PlayerInfo_SetInfo
===============
*/
void UI_PlayerInfo_SetInfo( playerInfo_t *pi, int legsAnim, int torsoAnim, vec3_t viewAngles, vec3_t moveAngles, weapon_t weaponNumber, qboolean chat ) {
	int			currentAnim;
	weapon_t	weaponNum;

	pi->chat = chat;

	// view angles
	VectorCopy( viewAngles, pi->viewAngles );

	// move angles
	VectorCopy( moveAngles, pi->moveAngles );

	if ( pi->newModel ) {
		pi->newModel = qfalse;

		jumpHeight = 0;
		pi->pendingLegsAnim = 0;
		UI_ForceLegsAnim( pi, legsAnim );
		pi->legs.yawAngle = viewAngles[YAW];
		pi->legs.yawing = qfalse;

		pi->pendingTorsoAnim = 0;
		UI_ForceTorsoAnim( pi, torsoAnim );
		pi->torso.yawAngle = viewAngles[YAW];
		pi->torso.yawing = qfalse;

		if ( weaponNumber != -1 ) {
			pi->weapon = weaponNumber;
			pi->currentWeapon = weaponNumber;
			pi->lastWeapon = weaponNumber;
			pi->pendingWeapon = -1;
			pi->weaponTimer = 0;
		}

		return;
	}

	// weapon
	if ( weaponNumber == -1 ) {
		pi->pendingWeapon = -1;
		pi->weaponTimer = 0;
	}
	else if ( weaponNumber != WP_NONE ) {
		pi->pendingWeapon = weaponNumber;
		pi->weaponTimer = dp_realtime + UI_TIMER_WEAPON_DELAY;
	}
	weaponNum = pi->lastWeapon;
	pi->weapon = weaponNum;

	if ( torsoAnim == BOTH_DEATH_CHEST_1 || legsAnim == BOTH_DEATH_CHEST_1 ) 
	{
		torsoAnim = legsAnim = BOTH_DEATH_CHEST_1;
		pi->weapon = pi->currentWeapon = WP_NONE;

		jumpHeight = 0;
		pi->pendingLegsAnim = 0;
		UI_ForceLegsAnim( pi, legsAnim );

		pi->pendingTorsoAnim = 0;
		UI_ForceTorsoAnim( pi, torsoAnim );

		return;
	}

	// leg animation
	currentAnim = pi->legsAnim & ~ANIM_TOGGLEBIT;
	if ( legsAnim != LEGS_JUMP && ( currentAnim == LEGS_JUMP ) ) 
	{
		pi->pendingLegsAnim = legsAnim;
	}
	else if ( legsAnim != currentAnim ) {
		jumpHeight = 0;
		pi->pendingLegsAnim = 0;
		UI_ForceLegsAnim( pi, legsAnim );
	}

	// torso animation
	if ( torsoAnim == TORSO_IDLE_PISTOL ) 
	{
		if ( weaponNum == WP_NONE || weaponNum == WP_KNIFE ) 
		{
			torsoAnim = TORSO_IDLE_PISTOL;
		}
		else 
		{
			torsoAnim = TORSO_IDLE_PISTOL;
		}
	}
 
	currentAnim = pi->torsoAnim & ~ANIM_TOGGLEBIT;

	if ( torsoAnim != currentAnim ) 
	{
		pi->pendingTorsoAnim = 0;
		UI_ForceTorsoAnim( pi, torsoAnim );
	}
}

/*
===============
UI_LoadIdentityIcons
===============
*/
void UI_LoadIdentityIcons ( void )
{
	int		i;
	char	fileName[MAX_OSPATH];
	
#ifdef SPECIAL_PRE_CACHE
	return;
#endif

	for ( i = 0; i < bg_identityCount; i ++ )
	{	
		Com_sprintf(fileName, sizeof(fileName), "gfx/playericons/%s ( %s ).JPG", bg_identities[i].mCharacter->mName, bg_identities[i].mSkin->mSkin);
		bg_identities[i].mIcon = trap_R_RegisterShaderNoMip(fileName);
	}
}


/*
=================
UI_ProcessIdentityItems

Attaches all the items for a character skin
=================
*/
static void UI_ProcessIdentityItems ( TGhoul2 ghoul2, TInventoryTemplate *items )
{
	int				modelIndex;
	int				boltIndex;
	TSurfaceList	*surf;

	while ( items )
	{
		if ( items->mItem )
		{
			if ( items->mItem->mModel && items->mBolt )
			{
				modelIndex = trap_G2API_InitGhoul2Model(&ghoul2, items->mItem->mModel, 0, 0, 0, 0, 0);
				if (modelIndex != -1)
				{
					boltIndex = trap_G2API_AddBolt ( ghoul2, 0, items->mBolt);
					if (boltIndex != -1)
					{
						trap_G2API_AttachG2Model( ghoul2, modelIndex, ghoul2, boltIndex, 0);
					}
				}
			}

			surf = items->mItem->mOffList;
			while(surf)
			{
				trap_G2API_SetSurfaceOnOff( ghoul2, 0, surf->mName, G2SURFACEFLAG_OFF);

				surf = surf->mNext;
			}

			surf = items->mItem->mOnList;
			while(surf)
			{
				trap_G2API_SetSurfaceOnOff( ghoul2, 0, surf->mName, 0);

				surf = surf->mNext;
			}
		}

		items = items->mNext;
	}
}

/*
=================
UI_RegisterIdentity

Registers an identity
=================
*/
TGhoul2 UI_RegisterIdentity ( const char *identityName, char *animationFile )
{
	char				name[MAX_QPATH];
	TGenericParser2		skinFile;
	TGPGroup			*basegroup, *group, *sub;
	char				temp[20480], *end;
	int					numPairs;
	TIdentity			*identity;
	TGhoul2				ghoul2Ptr;

	numPairs   = 0;
	end		   = temp;
	*end	   = 0;
	ghoul2Ptr  = NULL;

	*animationFile = 0;

	// Find the identity in question
	identity = BG_FindIdentity( identityName );
	if (!identity )
	{
		return NULL;
	}

	if (trap_G2API_InitGhoul2Model( &ghoul2Ptr, identity->mCharacter->mModel, 0, 0, 0, (1<<4), 0) == -1)
	{
		return NULL;
	}

#ifdef SPECIAL_PRE_CACHE
	Com_sprintf(temp, sizeof(temp), "gfx/playericons/%s ( %s ).JPG", identity->mCharacter->mName, identity->mSkin->mSkin);
	identity->mIcon = trap_R_RegisterShaderNoMip(temp);
#endif
	
	trap_G2API_GetAnimFileNameIndex( ghoul2Ptr, 0, name );
	Com_sprintf(animationFile, MAX_QPATH, "%s_mp.cfg", name );

	if ( identity->mCharacter->mParent)
	{
		UI_ProcessIdentityItems( ghoul2Ptr, identity->mCharacter->mParent->mInventory );
	}

	UI_ProcessIdentityItems( ghoul2Ptr, identity->mCharacter->mInventory);
	UI_ProcessIdentityItems( ghoul2Ptr, identity->mSkin->mInventory);

	// don't need the mouth
	trap_G2API_SetSurfaceOnOff( ghoul2Ptr, 0, "mouth_r", G2SURFACEFLAG_OFF|G2SURFACEFLAG_NODESCENDANTS);
	trap_G2API_SetSurfaceOnOff( ghoul2Ptr, 0, "mouth_l", G2SURFACEFLAG_OFF|G2SURFACEFLAG_NODESCENDANTS);

	// Parse the g2skin file
	Com_sprintf( name, sizeof(name), "models/characters/skins/%s.g2skin", identity->mSkin->mSkin );
	skinFile = trap_GP_ParseFile( name, qtrue, qfalse );
	if ( !skinFile )
	{
		trap_G2API_CleanGhoul2Models( &ghoul2Ptr);
		return NULL;
	}

	basegroup = trap_GP_GetBaseParseGroup ( skinFile );
	group = trap_GPG_GetSubGroups ( basegroup );

	while(group)
	{
		trap_GPG_GetName ( group, name );

		// Parse the material
		if ( Q_stricmp ( name, "material") == 0)
		{
			char	matName[MAX_QPATH];
			char	shaderName[MAX_QPATH];

			trap_GPG_FindPairValue ( group, "name", "", matName );

			sub = trap_GPG_FindSubGroup ( group, "group");
			if (sub)
			{
				trap_GPG_FindPairValue ( sub, "shader1", "", shaderName );
				if (!shaderName[0])
				{
					trap_GPG_FindPairValue ( sub, "texture1", "", shaderName );
				}
			}

			if (matName[0] && shaderName[0])
			{
				end += Com_sprintf(end, sizeof(temp) - (end-temp+1), "%s %s ", matName, shaderName);
				numPairs++;
			}
		}

		group = trap_GPG_GetNext ( group );
	}

	trap_GP_Delete(&skinFile);

	if (numPairs)
	{
		qhandle_t	handle;

		handle = trap_G2API_RegisterSkin( identity->mName, numPairs, temp);
		trap_G2API_SetSkin( ghoul2Ptr, 0, handle);
	}

	return ghoul2Ptr;
}
