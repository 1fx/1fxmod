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
// cg_weapons.c -- events and effects dealing with weapons

#include "cg_local.h"
#include "../game/bg_weapons.h"

// set up the appropriate ghoul2 info to a refent
void CG_SetGhoul2InfoRef( refEntity_t *ent, refEntity_t	*s1)
{
	ent->ghoul2 = s1->ghoul2;
	VectorCopy( s1->modelScale, ent->modelScale);
	ent->radius = s1->radius;
	VectorCopy( s1->angles, ent->angles);
}

/*
======================
CG_CalcMuzzlePoint
======================
*/
static qboolean	CG_CalcMuzzlePoint( int entityNum, vec3_t muzzlePoint) 
{
	weaponInfo_t	*weaponInfo;
	centity_t		*cent;
	refEntity_t		muzzle;
	int				muzzleBolt;
	attackInfo_t	*attackInfo;
	vec3_t			rootAngles;
	vec3_t			lowerTorsoAngles;
	vec3_t			upperTorsoAngles;
	vec3_t			headAngles;

	cent = CG_GetEntity ( entityNum );
	if ( !cent->currentValid ) 
	{
		return qfalse;
	}

	BG_PlayerAngles( cent->lerpAngles, 
				     NULL, 
						   
						   rootAngles,
						   lowerTorsoAngles,
						   upperTorsoAngles,
						   headAngles,
						   
						   cent->currentState.leanOffset - LEAN_OFFSET,

						   cent->pe.painTime, 
						   cent->pe.painDirection, 
						   cg.time,

						   &cent->pe.torso,
						   &cent->pe.legs,

						   cg.frametime, 
						   cent->lerpVelocity,
						   (cent->currentState.eFlags & EF_DEAD), 
						   cent->currentState.angles2[YAW],
						   cent->ghoul2 );

	memset( &muzzle, 0, sizeof( muzzle ) );

	if (!trap_G2_HaveWeGhoul2Models(cent->ghoul2))
	{	// no player model and/or no gun!
		return qfalse;
	}

	weaponInfo = &cg_weapons[cent->currentState.weapon];

	if ( cent->currentState.eFlags & EF_ALT_FIRING ) 
	{
		attackInfo = &weaponInfo->attack[ATTACK_ALTERNATE];
	}
	else
	{
		attackInfo = &weaponInfo->attack[ATTACK_NORMAL];
	}

	muzzleBolt = attackInfo->muzzleFlashBoltWorld;

	if ( muzzleBolt == -1 )
	{
		return qfalse;
	}

	if ( G2_PositionEntityOnBolt(&muzzle, 
							     cent->ghoul2, cent->pe.weaponModelSpot, muzzleBolt, 
							     cent->lerpOrigin, rootAngles, cent->modelScale))
	{	
		VectorCopy(muzzle.origin, muzzlePoint);
		return qtrue;
	}

	return qfalse;
}

/*
=============
CG_PlayerWeaponEffects

Add any weapon effects like muzzle flash or ejecting brass
=============
*/
void CG_PlayerWeaponEffects ( refEntity_t *parent, centity_t *cent, int team, vec3_t newAngles ) 
{
	weapon_t			weaponNum;
	const weaponInfo_t	*weaponInfo;
	const weaponData_t	*weaponDat;
	const attackInfo_t	*attackInfo;
	const attackData_t	*attackDat;
	refEntity_t			flash;
			
	cent->flashBoltInterface.isValid	= qfalse;
	cent->ejectBoltInterface.isValid	= qfalse;
	weaponNum = cent->currentState.weapon;	

	weaponInfo = &cg_weapons[weaponNum];
	weaponDat  = &weaponData[weaponNum];

	// Dont do this for the player unless they are in 3rd person
	if ( !cg.renderingThirdPerson )
	{
		if ( cent->currentState.number == cg.predictedPlayerState.clientNum )
		{
			return;
		}
	}

	// Make sure the player model is valid to attach to
	if ( !trap_G2_HaveWeGhoul2Models(cent->ghoul2) )
	{	
		return;
	}

	// The muzzle flash attach position needs to be updated for the duration of the effect
	if ( cg.time - cent->muzzleFlashTime > MUZZLE_FLASH_EFFECT_TIME  )
	{
		return;
	}

	// Alt fire or standard fire?
	attackInfo = &weaponInfo->attack[cent->muzzleFlashAttack];
	attackDat = &weaponDat->attack[cent->muzzleFlashAttack];

	// Position the flash entity on the muzzle flash bolt
	memset (&flash, 0, sizeof(flash));
	if ( !G2_PositionEntityOnBolt( &flash, 
								   cent->ghoul2, 
								   cent->pe.weaponModelSpot, 
								   attackInfo->muzzleFlashBoltWorld, 
								   cent->lerpOrigin, 
								   newAngles, 
								   cent->modelScale ) )
	{
		return;
	}

	// Keep the muzzle flash moving
	cent->flashBoltInterface.isValid	= qtrue;
	cent->flashBoltInterface.ghoul2		= cent->ghoul2;
	cent->flashBoltInterface.modelNum	= cent->pe.weaponModelSpot;
	cent->flashBoltInterface.boltNum	= attackInfo->muzzleFlashBoltWorld;
	
	VectorCopy ( flash.origin, cent->flashBoltInterface.origin );
	VectorCopy ( flash.axis[0], cent->flashBoltInterface.dir );
	VectorCopy ( flash.axis[0], cent->flashBoltInterface.forward );
	VectorCopy ( cent->modelScale, cent->flashBoltInterface.scale );

	// No more flash
	if ( cg.time - cent->muzzleFlashTime > MUZZLE_FLASH_TIME )
	{
		return;
	}

	// First frame of the flash?
	if ( cg.time == cent->muzzleFlashTime )
	{	
		// Ejecting brass
		if ( attackInfo->shellEject && cg_shellEjection.integer )
		{
			refEntity_t eject;

			memset ( &eject, 0, sizeof(eject) );

			// Now position the flash entity on the bolt itself.
			if ( G2_PositionEntityOnBolt( &eject, 
										  cent->ghoul2, 
										  cent->pe.weaponModelSpot, 
										  attackInfo->shellEjectBoltWorld, 
										  cent->lerpOrigin, 
										  newAngles, 
										  cent->modelScale ) )
			{
				vec3_t	angles;

				eject.axis[0][0] = -eject.axis[0][0];
				eject.axis[0][1] = -eject.axis[0][1];
				eject.axis[0][2] = -eject.axis[0][2];

				vectoangles ( eject.axis[0], angles );
				angles[YAW] += 90;
				AnglesToAxis ( angles, eject.axis );
				
				trap_FX_PlayEntityEffectID ( attackInfo->shellEject, eject.origin, eject.axis, -1, -1, -1, -1  );
			}
		}

		// Muzzle Flash
		if ( attackInfo->muzzleEffect )
		{

			if (attackDat->weaponFlags & UNLOCK_MUZZLEFLASH )
			{
				// Muzzle flash not locked to barrel.
				trap_FX_PlayEffectID(attackInfo->muzzleEffectInWorld, flash.origin, flash.axis[0], -1, -1 );
			}
			else
			{				
				trap_FX_PlayBoltedEffectID ( attackInfo->muzzleEffectInWorld, &cent->flashBoltInterface, -1, -1 );
			}
		}
	}

	// Add a dlight to the scene if it has a muzzle effect
	if ( attackInfo->muzzleEffect )
	{
		trap_R_AddLightToScene( flash.origin, 200, 0.6f, 0.4f, 0.2f );
	}
}

// FIMXE: This is defined in a C++ header file. 
// We need to break it up or somethin so we don't have mutliple defs.
#define GHOUL2_NORENDER 2

/*
==============
CG_StartViewAnimation

Start a view animation for the given weapon and model index
==============
*/
void CG_StartViewWeaponAnimation ( int weapon, int modelindex, int choice, TAnimInfoWeapon* aIW )
{
	weaponInfo_t	*weaponInfo;	
	int				boneMode;
	
	weaponInfo = &cg_weapons[weapon];
	if ( weaponInfo->viewG2Indexes[modelindex] == -1 )
	{
		return;
	}

	boneMode = BONE_ANIM_OVERRIDE_DEFAULT;

/*
	if (cg_animBlend.integer)
	{
		boneMode |= BONE_ANIM_BLEND;
	}
*/

	trap_G2API_SetBoneAnim( weaponInfo->viewG2Model,
						    weaponInfo->viewG2Indexes[modelindex],
							"model_root",
							aIW->mStartFrame[choice],
							aIW->mStartFrame[choice] + aIW->mNumFrames[choice],
						    boneMode,
						    50.0f / (1000.0f / aIW->mFPS[choice]) * aIW->mSpeed,
						    cg.time,
						    aIW->mStartFrame[choice],
							150);
	
}

/*
==============
CG_AnimateViewWeapon

Animation the view weapon
==============
*/
void CG_AnimateViewWeapon ( playerState_t *ps )
{
	int				i;
	int				flags;
	weaponInfo_t	*weaponInfo;	

	if( ps->pm_type == PM_SPECTATOR )
	{
		return;
	}

	for ( i = 0; i < 5; i ++ )
	{
		if ( !cg.viewWeaponAnim[i] )
		{
			continue;
		}

		CG_StartViewWeaponAnimation ( ps->weapon, i, i==0?ps->weaponAnimIdChoice:0, cg.viewWeaponAnim[i] );
		cg.viewWeaponAnim[i] = NULL;
	}

	// Need to turn on/off anything?
	if(!cg.weaponHideModels)
	{
		return;
	}

	// Ok... turn on/off models.
	weaponInfo=&cg_weapons[ps->weapon];
	for ( i=0; i < 8; i++ )
	{
		if( weaponInfo->viewG2Indexes[i] == -1 )
		{
			continue;
		}

		flags=trap_G2API_GetGhoul2ModelFlagsByIndex(weaponInfo->viewG2Model,weaponInfo->viewG2Indexes[i]);
		if(cg.weaponHideModels&(1<<i))
		{
			trap_G2API_SetGhoul2ModelFlagsByIndex(weaponInfo->viewG2Model,weaponInfo->viewG2Indexes[i],
												  flags|GHOUL2_NORENDER);
		}
		else
		{
			trap_G2API_SetGhoul2ModelFlagsByIndex(weaponInfo->viewG2Model,weaponInfo->viewG2Indexes[i],
												  flags&=~GHOUL2_NORENDER);
		}
	}
}

/*
==============
CG_UpdateViewWeaponSurfaces

Update the on/off bullet suraces of the inview weapon
==============
*/
#define MAX_VIEWWEAPON_BULLETS	6
void CG_UpdateViewWeaponSurfaces ( playerState_t* ps )
{
	int					numBullets;
	int					i;
	const weaponInfo_t	*weaponInfo;

	weaponInfo = &cg_weapons[ps->weapon];

	numBullets = ps->clip[ATTACK_NORMAL][ps->weapon];

	// When reloading just show it at zero, its easier that way
	if ( ps->weaponstate == WEAPON_RELOADING || ps->weaponstate == WEAPON_RELOADING_ALT )
	{
		numBullets = 0;
	}
	// Dont have more than the max bullets
	else if (numBullets > MAX_VIEWWEAPON_BULLETS)
	{
		// toggle all of them
		numBullets = MAX_VIEWWEAPON_BULLETS;
	}

	// Make sure its registered
	if ( !weaponInfo->registered )
	{
		CG_RegisterWeapon ( ps->weapon );
	}

	// No model, nothing to do
	if ( !weaponInfo->viewG2Model )
	{
		return;
	}

	switch ( ps->weapon )
	{
		case WP_RPG7_LAUNCHER:
			trap_G2API_SetSurfaceOnOff(weaponInfo->viewG2Model,weaponInfo->viewG2Indexes[0],"rockethead",numBullets?0:G2SURFACEFLAG_OFF);
			trap_G2API_SetSurfaceOnOff(weaponInfo->viewG2Model,weaponInfo->viewG2Indexes[0],"rockettail",numBullets?0:G2SURFACEFLAG_OFF);
			break;

		case WP_M1911A1_PISTOL:
		case WP_USSOCOM_PISTOL:
		{
			int forward;
			int backward;

			if ( numBullets == 0 )
			{
				forward = G2SURFACEFLAG_OFF;
				backward = 0;
			}
			else
			{
				forward = 0;
				backward = G2SURFACEFLAG_OFF;
			}

			trap_G2API_SetSurfaceOnOff( weaponInfo->viewG2Model, weaponInfo->viewG2Indexes[0], "slide", forward );
			trap_G2API_SetSurfaceOnOff( weaponInfo->viewG2Model, weaponInfo->viewG2Indexes[0], "slidef", forward );
			trap_G2API_SetSurfaceOnOff( weaponInfo->viewG2Model, weaponInfo->viewG2Indexes[0], "slidel", forward );
			trap_G2API_SetSurfaceOnOff( weaponInfo->viewG2Model, weaponInfo->viewG2Indexes[0], "slider", forward );
			trap_G2API_SetSurfaceOnOff( weaponInfo->viewG2Model, weaponInfo->viewG2Indexes[0], "slideb", forward );
			trap_G2API_SetSurfaceOnOff( weaponInfo->viewG2Model, weaponInfo->viewG2Indexes[0], "slide_off", backward );
			trap_G2API_SetSurfaceOnOff( weaponInfo->viewG2Model, weaponInfo->viewG2Indexes[0], "slidef_off", backward );
			trap_G2API_SetSurfaceOnOff( weaponInfo->viewG2Model, weaponInfo->viewG2Indexes[0], "slidel_off", backward );
			trap_G2API_SetSurfaceOnOff( weaponInfo->viewG2Model, weaponInfo->viewG2Indexes[0], "slider_off", backward );
			trap_G2API_SetSurfaceOnOff( weaponInfo->viewG2Model, weaponInfo->viewG2Indexes[0], "slideb_off", backward );

			break;
		}

		case WP_M60_MACHINEGUN:
			// Turn on whats available
			for ( i = 0; i < numBullets; i++ )
			{
				trap_G2API_SetSurfaceOnOff( weaponInfo->viewG2Model, weaponInfo->viewG2Indexes[0], va("bullet%d", i + 1), 0 );
			}
			
			// Turn the rest off
			for (; i < MAX_VIEWWEAPON_BULLETS; i++)
			{
				trap_G2API_SetSurfaceOnOff( weaponInfo->viewG2Model, weaponInfo->viewG2Indexes[0], va("bullet%d", i + 1), G2SURFACEFLAG_OFF );
			}	
			break;
	}
}

/*
==============
CG_AddViewWeapon

Add the weapon, and flash for the player's view
==============
*/
void CG_AddViewWeapon(playerState_t *ps)
{
	refEntity_t			gun;
	refEntity_t			flash;
	centity_t			*cent;
	float				fovOffset;
	const weaponInfo_t	*weaponInfo;
	const weaponData_t	*weaponDat;
	const attackInfo_t	*attackInfo;
	const attackData_t	*attackDat;
	vec3_t				angles;
	int					delta;
	vec3_t				forward;
	vec3_t				vangles;
	int					speed;

	cg.flashBoltInterface.isValid = qfalse;

	if(ps->pm_type == PM_SPECTATOR || ps->persistant[PERS_TEAM] == TEAM_SPECTATOR )
	{
		return;
	}

	if(ps->pm_type == PM_INTERMISSION)
	{
		return;
	}

	// No gun if in third person view or a camera is active
	//if ( cg.renderingThirdPerson || cg.cameraMode) {
	if(cg.renderingThirdPerson)
	{
		return;
	}

	cent = &cg_entities[ps->clientNum]; 

	// allow the gun to be completely removed
	if( !cg_drawGun.integer || (ps->pm_flags & PMF_ZOOMED) )
	{
		return;
	}

	// drop gun lower at higher fov
	if ( cg_fov.integer > 90 )
	{
		fovOffset = -0.2 * ( cg_fov.integer - 90 );
	} 
	else 
	{
		fovOffset = 0;
	}

	CG_RegisterWeapon(ps->weapon);
	weaponInfo = &cg_weapons[ps->weapon];
	weaponDat  = &weaponData[ps->weapon];

	// Add the weapon and hands models.	
	memset(&gun,0,sizeof(gun));
	gun.ghoul2 = weaponInfo->viewG2Model;
	if(!trap_G2_HaveWeGhoul2Models(gun.ghoul2))
	{	
		// No weapon to draw!
		return;
	}

	VectorCopy ( cg.refdef.vieworg, gun.origin );

	// Add movement bobbing 
	gun.origin[2] += cg.xyspeed * cg.bobfracsin * 0.0015f;

	// Add landing offset
	delta = cg.time - cg.landTime;
	if ( delta < LAND_DEFLECT_TIME ) 
	{
		gun.origin[2] += cg.landChange * 0.25f * delta / LAND_DEFLECT_TIME;
	} 
	else if ( delta < LAND_DEFLECT_TIME + LAND_RETURN_TIME ) 
	{
		gun.origin[2] += cg.landChange * 0.25f * (LAND_DEFLECT_TIME + LAND_RETURN_TIME - delta) / LAND_RETURN_TIME;
	}

	// Add movement offset
	speed = sqrt( cg.predictedPlayerState.velocity[0] * cg.predictedPlayerState.velocity[0] +
				  cg.predictedPlayerState.velocity[1] * cg.predictedPlayerState.velocity[1] +
				  cg.predictedPlayerState.velocity[2] * cg.predictedPlayerState.velocity[2]);

	vectoangles ( cg.predictedPlayerState.velocity, vangles );
	vangles[1] += (360 - cg.predictedPlayerState.viewangles[1]);
	vangles[2] += (360 - cg.predictedPlayerState.viewangles[2]);
	AngleVectors ( vangles, forward, NULL, NULL);	

	VectorScale ( forward, speed, forward );

	VectorMA( gun.origin, forward[1] * 0.003f, cg.refdef.viewaxis[1], gun.origin );

	// Set model scale	
	VectorSet ( gun.modelScale,weaponParseInfo[ps->weapon].mForeshorten,1.0f,1.0f );

	vectoangles(cg.refdef.viewaxis[0],angles);
	AnglesToAxis(angles,gun.axis);
	CG_ScaleModelAxis(&gun);

	VectorCopy(gun.origin,gun.oldorigin);
	gun.renderfx=RF_DEPTHHACK|RF_FIRST_PERSON|RF_MINLIGHT|RF_NO_FOG;	
	gun.radius=64;

	trap_R_AddRefEntityToScene(&gun);

	// The muzzle flash attach position needs to be updated for the duration of the effect
	if ( cg.time - cent->muzzleFlashTime > MUZZLE_FLASH_EFFECT_TIME )
	{
		return;
	}

	// Grab the proper attack info
	if(cent->muzzleFlashAttack )
	{
		attackInfo = &weaponInfo->attack[ATTACK_ALTERNATE];
		attackDat  = &weaponDat->attack[ATTACK_ALTERNATE];
	}
	else
	{
		attackInfo = &weaponInfo->attack[ATTACK_NORMAL];
		attackDat  = &weaponDat->attack[ATTACK_NORMAL];
	}

	// Update the muzzle flashes origins
	memset(&flash,0,sizeof(flash));
	VectorCopy ( gun.modelScale, flash.modelScale );
	if(!G2_PositionEntityOnBolt( &flash,
								 gun.ghoul2, 
								 1,
								 attackInfo->muzzleFlashBoltView, 
								 gun.origin,
								 angles,
								 flash.modelScale ) )
	{
		return;
	}

	// Keep the muzzle flash origins updated
	cg.flashBoltInterface.isValid	= qtrue;
	cg.flashBoltInterface.ghoul2	= gun.ghoul2;
	cg.flashBoltInterface.modelNum	= 1;
	cg.flashBoltInterface.boltNum	= attackInfo->muzzleFlashBoltView;
	
	VectorCopy(flash.axis[0],cg.flashBoltInterface.dir );
	VectorCopy(flash.axis[0],cg.flashBoltInterface.forward );
	VectorCopy(flash.origin,cg.flashBoltInterface.origin);	
	VectorCopy(flash.modelScale,cg.flashBoltInterface.scale);

	// If no muzzle flash to handle then there is nothing left to do
	if( cg.time - cent->muzzleFlashTime > MUZZLE_FLASH_TIME )
	{
		return;
	}

	// First frame for the muzzle flash is when the effects are added
	if ( cg.time == cent->muzzleFlashTime )
	{
		// Add the muzzle flash
		if( attackInfo->muzzleEffect )
		{
			// Handle locked and unlocked muzzle flashes
			if(attackDat->weaponFlags & UNLOCK_MUZZLEFLASH )
			{
				trap_FX_PlayEffectID(attackInfo->muzzleEffect,flash.origin,flash.axis[0], -1, -1 );
			}
			else
			{
				trap_FX_PlayBoltedEffectID(attackInfo->muzzleEffect,&cg.flashBoltInterface, -1, -1 );
			}
		}

		// Add shell ejection
		if ( attackInfo->shellEject && cg_shellEjection.integer )
		{
			refEntity_t	shellEject;

			// Handle muzzle flashes
			memset(&shellEject,0,sizeof(shellEject));

			if(G2_PositionEntityOnBolt( &shellEject,
									    weaponInfo->viewG2Model,
									    1,
									    attackInfo->shellEjectBoltView,
									    cg.refdef.vieworg,
									    cg.refdef.viewangles,
									    gun.modelScale ) )
			{
				// Flip the forward axis so it goes backwards rather than forwards
				shellEject.axis[0][0] = -shellEject.axis[0][0];
				shellEject.axis[0][1] = -shellEject.axis[0][1];
				shellEject.axis[0][2] = -shellEject.axis[0][2];

				// Play the entity with a full axis
				trap_FX_PlayEntityEffectID(attackInfo->shellEject,shellEject.origin,shellEject.axis, -1, -1, -1, -1  );
			}
		}

		// Add tracers
		if ( attackInfo->tracerEffect && !(attackDat->projectileLifetime & PROJECTILE_FIRE))
		{
			// Lower the amount of tracers
			if ( rand()%100 < (cg_tracerChance.value * 100.0f) )
			{
				vec3_t origin;
				VectorMA ( flash.origin, 500, gun.axis[0], origin );
				trap_FX_PlayEffectID ( attackInfo->tracerEffect, origin, gun.axis[0], -1, -1  );
			}			
		}
	}

	// Add a dlight when there is a muzzle effect
	if ( attackInfo->muzzleEffect )
	{
		trap_R_AddLightToScene( flash.origin, 200, 0.6f, 0.4f, 0.2f );
	}
}

/*
==============================================================================

WEAPON SELECTION

==============================================================================
*/

/*
===============
CG_WeaponSelectable
===============
*/
qboolean CG_WeaponSelectable( int i, qboolean allowEmpty ) 
{
	int ammo;

	if ( ! (cg.predictedPlayerState.stats[ STAT_WEAPONS ] & ( 1 << i ) ) ) 
	{
		return qfalse;
	}

	if ( allowEmpty && (cg.predictedPlayerState.pm_flags & PMF_LIMITED_INVENTORY) )
	{
		return qtrue;
	}

	if ( BG_WeaponHasAlternateAmmo ( i ) )
	{
		if ( cg.predictedPlayerState.ammo[ weaponData[i].attack[ATTACK_ALTERNATE].ammoIndex ] ||
			 cg.predictedPlayerState.clip[ATTACK_ALTERNATE][ i ] )
		{
			return qtrue;
		}
	}

	// Start with normal ammo
	ammo = 0;
	ammo += cg.predictedPlayerState.clip[ATTACK_NORMAL][i];
	ammo += cg.predictedPlayerState.ammo[weaponData[i].attack[ATTACK_NORMAL].ammoIndex];

	if ( !ammo )
	{
		return qfalse;
	}

	return qtrue;
}

/*
===============
CG_NextWeapon

selects the next weapon in the players inventory
===============
*/
void CG_NextWeapon ( qboolean allowEmpty, int exclude ) 
{
	int		i;
	int		original;
	int		selected;

	if ( !cg.snap ) 
	{
		return;
	}

	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) 
	{
		return;
	}

	if ( cg.predictedPlayerState.weaponstate == WEAPON_CHARGING     || 
	     cg.predictedPlayerState.weaponstate == WEAPON_CHARGING_ALT    ) 
	{
		return;
	}

	// When the weapon select menu is up the next and prev move through it
	if ( cg.weaponMenuUp )
	{
		selected = cg.weaponMenuSelect;
	}
	else
	{
		selected = cg.weaponSelect;
	}

	original = selected;

	for ( i = WP_NONE + 1 ; i < WP_NUM_WEAPONS; i++ ) 
	{
		selected++;
		if ( selected == WP_NUM_WEAPONS ) 
		{
			selected = WP_NONE + 1;
		}
		if ( selected != exclude && CG_WeaponSelectable( selected, allowEmpty ) ) 
		{
			break;
		}
	}

	if ( i == WP_NUM_WEAPONS ) 
	{
		selected = original;
	}

	// When the weapon select menu is up the next and prev move through it
	if ( cg.weaponMenuUp )
	{
		cg.weaponMenuSelect = selected;
	}
	else
	{
		cg.weaponSelect = selected;
	}
}

/*
===============
CG_PrevWeapon

Selects the previous weapon in the players inventory
===============
*/
void CG_PrevWeapon ( qboolean allowEmpty, int exclude ) 
{
	int		i;
	int		original;
	int		selected;

	if ( !cg.snap ) 
	{
		return;
	}

	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) 
	{
		return;
	}

	if ( cg.predictedPlayerState.weaponstate == WEAPON_CHARGING     || 
	     cg.predictedPlayerState.weaponstate == WEAPON_CHARGING_ALT    ) 
	{
		return;
	}

	// When the weapon select menu is up the next and prev move through it
	if ( cg.weaponMenuUp )
	{
		selected = cg.weaponMenuSelect;
	}
	else
	{
		selected = cg.weaponSelect;
	}

	original = selected;

	for ( i = WP_NONE + 1 ; i < WP_NUM_WEAPONS ; i++ ) 
	{
		selected--;
		if ( selected == WP_NONE ) 
		{
			selected = WP_NUM_WEAPONS-1;
		}
		if ( selected != exclude && CG_WeaponSelectable( selected, allowEmpty ) ) 
		{
			break;
		}
	}

	if ( i == WP_NUM_WEAPONS ) 
	{
		selected = original;
	}

	// When the weapon select menu is up the next and prev move through it
	if ( cg.weaponMenuUp )
	{
		cg.weaponMenuSelect = selected;
	}
	else
	{
		cg.weaponSelect = selected;
	}
}

/*
===============
CG_Weapon_f
===============
*/
void CG_Weapon_f( void ) 
{
	int		num;
	int		category;
	int		last;

	if ( !cg.snap ) 
	{
		return;
	}

	if ( (cg.snap->ps.pm_flags & PMF_FOLLOW) ) 
	{
		return;
	}

	if ( cg.predictedPlayerState.weaponstate == WEAPON_CHARGING     || 
	     cg.predictedPlayerState.weaponstate == WEAPON_CHARGING_ALT    ) 
	{
		return;
	}

	category = atoi( CG_Argv( 1 ) );

	if (category < 0 || category > CAT_MAX) 
	{
		return;
	}

	if ( !cg_weaponMenuFast.integer )
	{
		if ( !cg.weaponMenuUp )
		{
			cg.weaponMenuSelect = cg.predictedPlayerState.weapon;
			cg.weaponMenuUp = qtrue; // show weapon menu
		}
	}
	else
	{
		cg.weaponMenuSelect = cg.predictedPlayerState.weapon;
	}

	last = -1;
	if (category == weaponData[cg.weaponMenuSelect].category)
	{
		last = cg.weaponMenuSelect;	
	}

	// Find next weapon in currently active category.
	for (num = WP_KNIFE; num < WP_NUM_WEAPONS; num++)
	{
		if(num>last && category==weaponData[num].category && (cg.predictedPlayerState.stats[STAT_WEAPONS]&(1<<num)))
		{
			if ( (cg.predictedPlayerState.stats[ STAT_WEAPONS ] & ( 1 << num ) ) ) 
			{
				break;
			}
		}
	}
	
	// Ok... wrap around.. and try again.
	if (WP_NUM_WEAPONS == num)
	{
		for (num = WP_KNIFE; num < WP_NUM_WEAPONS; num++)
		{
			if(num!=last && category == weaponData[num].category && (cg.snap->ps.stats[STAT_WEAPONS]&(1<<num)))
			{
				if ( (cg.predictedPlayerState.stats[ STAT_WEAPONS ] & ( 1 << num ) ) ) 
				{
					break;
				}
			}
		}
	}

	if (WP_NUM_WEAPONS == num)
	{	
		// Couldn't find one!
		return;
	}

	if ( !cg_weaponMenuFast.integer )
	{
		cg.weaponMenuSelect = num;
	}
	else
	{
		cg.weaponSelect = num;
	}
}

/*
===================
CG_OutOfAmmoChange

The current weapon has just run out of ammo
===================
*/
void CG_OutOfAmmoChange( int weapon ) 
{
	// Get the best weapon thats not a grenade
	cg.weaponSelect = WP_M67_GRENADE;
	CG_PrevWeapon ( qfalse, weapon );
}

/*
================
CG_PredictedBullet

Fires a bullet client side and produces all the effects that would be produced
by the server if it had done it
================
*/
void CG_PredictedBullet ( centity_t* cent, attackType_t attack )
{
	vec3_t			fireAngs;
	float			inaccuracy;
	qboolean		detailed;
	vec3_t			end;
	vec3_t			start;
	trace_t			tr;
	int				i;
	int				pellets;
	entityState_t	*ent;
	int				seed;

	ent = &cent->currentState;

	// Dont bother if antilag is turned off or its a projectile weapon
	if ( cg_antiLag.integer < 1 || !cg_impactPrediction.integer || (weaponData[ent->weapon].attack[attack].weaponFlags & PROJECTILE_FIRE))
	{
		return;
	}

	// Calculate the muzzle point
	VectorCopy( cg.predictedPlayerState.origin, start );
	start[2] += cg.predictedPlayerState.viewheight;

	CG_UpdateViewWeaponSurfaces ( &cg.predictedPlayerState );

	detailed = qtrue;
	pellets = (weaponData[ent->weapon].attack[attack].pellets / 2) + 1;
	pellets = weaponData[ent->weapon].attack[attack].pellets;

	// Handle leaning
	VectorCopy(cg.predictedPlayerState.viewangles, fireAngs);
	if ( cg.predictedPlayerState.pm_flags & PMF_LEANING )
	{
		vec3_t  right;
		float	leanOffset;

		leanOffset = (float)(cg.predictedPlayerState.leanTime - LEAN_TIME) / LEAN_TIME * LEAN_OFFSET;
		AngleVectors( fireAngs, NULL, right, NULL );
		VectorMA( start, leanOffset, right, start );
	}

	seed = cg.predictedPlayerState.stats[STAT_SEED];

	// Current inaccuracy
	inaccuracy = (float)cg.predictedPlayerState.inaccuracy / 1000.0f;
	if ( detailed )
	{
		if ( cg.predictedPlayerState.pm_flags & PMF_DUCKED )
		{
			inaccuracy *= DUCK_ACCURACY_MODIFIER;
		}
		else if ( cg.predictedPlayerState.pm_flags & PMF_JUMPING )
		{
			inaccuracy *= JUMP_ACCURACY_MODIFIER;
		}
	}

	for ( i = 0; i < pellets; i ++ )
	{
		BG_CalculateBulletEndpoint ( start, fireAngs, inaccuracy, weaponData[ent->weapon].attack[attack].rV.range, end, &seed );

		// See if it hit a wall
		CG_Trace ( &tr, start, NULL, NULL, end, cent->currentState.number, MASK_SHOT&~CONTENTS_BODY );

		if ( tr.fraction >= 0.0f && tr.fraction <= 1.0f )
		{
			VectorCopy ( tr.endpos, end );
		}

		CG_PlayerTrace ( &tr, start, end, cent->currentState.number );			

		if ( tr.entityNum >= cgs.maxclients && tr.entityNum != ENTITYNUM_NONE )
		{		
			CG_Bullet( tr.endpos, ent->number, ent->weapon, tr.plane.normal, ENTITYNUM_WORLD, 
					   tr.surfaceFlags & MATERIAL_MASK,
					   attack );
		}			
#ifdef _SOF2_FLESHIMPACTPREDICTION
		else if ( tr.entityNum < cgs.maxclients )
		{
			qboolean blood = qtrue;
			vec3_t	 dir;

			if ( cg_impactPrediction.integer < 2 )
			{
				return;
			}
			// If invulerable then no blood
			if ( cg_entities[tr.entityNum].currentState.eFlags & EF_INVULNERABLE )
			{
				blood = qfalse;
			}
			// If on the same team in friendly fire then no blood
			else if ( cgs.gametypeData->teams && !cgs.friendlyFire && cgs.clientinfo[tr.entityNum].team == cgs.clientinfo[cg.snap->ps.clientNum].team )
			{
				blood = qfalse;
			}

			VectorSubtract ( end, start, dir );
			VectorNormalize ( dir );

			// If no blood then just make a little white puff
			if ( !blood || cg_lockBlood.integer )
			{
				CG_Bullet( tr.endpos, ent->number, ent->weapon, dir, ENTITYNUM_WORLD, 0, attack );
			}
			else
			{
				CG_PredictedProcGore ( ent->weapon, attack, start, end, &cg_entities[tr.entityNum] );

				CG_MissileHitPlayer( ent->weapon, tr.endpos, dir, tr.entityNum, qfalse );
			}
		}
#endif
	}
}

/*
================
CG_FireWeapon

Caused by an EV_FIRE_WEAPON event
================
*/
void CG_FireWeapon( centity_t *cent, attackType_t attack ) 
{
	entityState_t	*ent;
	int				c;
	weaponInfo_t	*weaponInfo;
	attackInfo_t	*attackInfo;

	ent = &cent->currentState;
	
	if ( ent->weapon == WP_NONE ) 
	{
		return;
	}

	if ( ent->weapon >= WP_NUM_WEAPONS ) 
	{
		Com_Error( ERR_FATAL, "CG_FireWeapon: ent->weapon >= WP_NUM_WEAPONS" );
		return;
	}

	weaponInfo = &cg_weapons[ ent->weapon ];
	attackInfo = &weaponInfo->attack[attack];

	// mark the entity as muzzle flashing, so when it is added it will
	// append the flash to the weapon model
	cent->muzzleFlashTime   = cg.time;
	cent->muzzleFlashAttack = attack;

	// play a sound
	for ( c = 0 ; c < 4 ; c++ )
	{
		if ( !attackInfo->flashSound[c] )
		{
			break;
		}
	}

	if ( c > 0 )
	{
		c = rand() % c;
		if ( attackInfo->flashSound[c] )
		{
			trap_S_StartSound( NULL, ent->number, CHAN_WEAPON, attackInfo->flashSound[c], -1, 2000 );
		}
	}

	// Handle dissapearing bullets if this is the main guy firing
	if ( cent->currentState.number == cg.snap->ps.clientNum )
	{
		CG_PredictedBullet ( cent, attack );
	}
}

/*
================
CG_ProjectileThink

Create trail fx for projectiles
================
*/
void CG_ProjectileThink( centity_t *cent, int weaponNum )
{
	const weaponInfo_t	*weaponInfo;
	const weaponData_t	*weaponDat;
	vec3_t				forward;
	int					trailEffectId;
	
	weaponInfo = &cg_weapons[weaponNum];
	weaponDat = &weaponData[weaponNum];

	if ( cent->currentState.eFlags & EF_ALT_FIRING )
	{
		trailEffectId = weaponInfo->attack[ATTACK_ALTERNATE].tracerEffect;
	}
	else
	{
		trailEffectId = weaponInfo->attack[ATTACK_NORMAL].tracerEffect;
	}

	// use tracer effect for projectile trails
	if (trailEffectId)
	{	
		// only calc normalize if there IS an effect
		if ( VectorNormalize2( cent->currentState.pos.trDelta, forward ) == 0.0f )
		{
			forward[2] = 1.0f;
		}

		trap_FX_PlayEffectID(trailEffectId, cent->lerpOrigin, forward, -1, -1  );
	}
}

/*
=================
CG_FlashBang

Blinds a player and makes them loose sound for a bit
=================
*/

#define	MAX_FLASHBANG_AFFECT_DISTANCE	1750
#define MAX_FLASHBANG_DISTANCE			3000
#define MAX_FLASHBANG_TIME				11000

void CG_FlashBang ( vec3_t origin, vec3_t dir )
{
	trace_t trace;
	vec3_t	start;
	vec3_t	end;
	vec3_t	delta;
	float	distance;
	vec3_t	fromangles;
	int		angle;
	int		time;

	// Dont flash grenade dead people
	if ( (cg.predictedPlayerState.pm_flags & (PMF_GHOST|PMF_FOLLOW)) || cg.predictedPlayerState.pm_type != PM_NORMAL )
	{
		return;
	}

	VectorCopy ( origin, start );
	start[2] += cg.snap->ps.viewheight;				

	VectorCopy ( cg.refdef.vieworg, end );
	end[2] += cg.snap->ps.viewheight;				

	VectorSubtract( cg.refdef.vieworg, origin, delta );
					
	// distance to the flash bang
	distance = VectorLength( delta );

	// Make sure its not too far away
	if(distance > MAX_FLASHBANG_DISTANCE )
	{
		return;
	}

	// Can the player see the flashbang?
	CG_Trace ( &trace, start, NULL, NULL, end, ENTITYNUM_NONE, MASK_SHOT ); 
	if ( trace.contents & (CONTENTS_SOLID|CONTENTS_TERRAIN) ) 
	{
		return;
	}

	VectorNormalize ( delta );		
	vectoangles ( delta, fromangles );

	angle =  cg.snap->ps.viewangles[1] - fromangles[1];
	angle += 180;

	if(angle < 0)
	{
		angle += 360;
	}

	// add distance if not facing the grenade.. 
	if(!(angle > 300 || angle < 60 )) 
	{
		angle = 120 - abs(angle-180);
		distance += (MAX_FLASHBANG_AFFECT_DISTANCE * angle / 240);
	}

	if(distance > MAX_FLASHBANG_AFFECT_DISTANCE - 50 )
	{
		distance = MAX_FLASHBANG_AFFECT_DISTANCE - 50;				
	}

	distance = MAX_FLASHBANG_AFFECT_DISTANCE - distance;
	time	 = MAX_FLASHBANG_TIME;

	cg.flashbangTime     = cg.time;
	cg.flashbangFadeTime = (MAX_FLASHBANG_TIME * distance / MAX_FLASHBANG_AFFECT_DISTANCE);
	cg.flashbangAlpha    = 1.0f * distance / MAX_FLASHBANG_AFFECT_DISTANCE;

	// If NVG or thermals were on then blast them a bit more
	if ( (cg.predictedPlayerState.pm_flags & PMF_GOGGLES_ON) )
	{
		cg.flashbangFadeTime += (cg.flashbangFadeTime / 4);
	}
}

/*
=================
CG_MissileHitWall

Caused by an EV_MISSILE_MISS event, or directly by local bullet tracing
=================
*/
void CG_MissileHitWall (
 	int			 weapon, 
	vec3_t		 origin, 
	vec3_t		 dir, 
	int			 material, 
	attackType_t attack
	)	 
{
	qhandle_t	impactEffect;
	int			ammoIndex;

	// Flash grenades are handled very differnetly
	if ( weapon == WP_M84_GRENADE )
	{
		CG_FlashBang ( origin, dir );
	}

	// Melee is handled specialol, the standard bullet routines are used
	// to handle hit detection and what not, but the effects are handled 
	// separately.
	if ( weaponData[weapon].attack[attack].melee )
	{
		impactEffect = trap_MAT_GetEffect( (char*)weaponData[weapon].attack[attack].melee, material&MATERIAL_MASK);
		if (impactEffect)
		{
			trap_FX_PlayEffectID( impactEffect, origin, dir, -1, -1  );
		}

		return;
	}

	ammoIndex = weaponData[weapon].attack[attack].ammoIndex;

	switch( ammoIndex )
	{
		default:
		case AMMO_KNIFE:
		case AMMO_045:
		case AMMO_556:
		case AMMO_9  :
		case AMMO_12 :
		case AMMO_762:
			impactEffect = trap_MAT_GetEffect(ammoData[ammoIndex].name, material&MATERIAL_MASK);
			if (impactEffect)
			{
				trap_FX_PlayEffectID( impactEffect, origin, dir, -1, -1  );
			}
			break;

		case AMMO_40:
		case AMMO_M15:
		case AMMO_M67:
		case AMMO_M84:
		case AMMO_F1:
		case AMMO_RPG7:
		case AMMO_L2A2:
		case AMMO_MDN11:
		case AMMO_SMOHG92:
		case AMMO_ANM14:

			impactEffect = trap_MAT_GetEffect(ammoData[ammoIndex].name, material&MATERIAL_MASK);
			if (impactEffect)
			{			
				trap_FX_PlayEffectID( impactEffect, origin, dir, -1, -1  );
			}
		
			// grenade!
			trap_FX_PlayEffectID( cg_weapons[weapon].attack[attack].explosionEffect, origin, dir, -1, 3000  );

			break;
	}
}


/*
=================
CG_MissileHitPlayer
=================
*/
void CG_MissileHitPlayer (
	int			 weapon, 
	vec3_t		 origin, 
	vec3_t		 dir, 
	int			 entityNum,
	attackType_t attack
	) 
{
	qhandle_t	impactEffect = 0;
	int			ammoIndex;
	
	ammoIndex = weaponData[weapon].attack[attack].ammoIndex;

	switch( ammoIndex )
	{
		default:
		case AMMO_KNIFE:
		case AMMO_045:
		case AMMO_556:
		case AMMO_9  :
		case AMMO_12 :
		case AMMO_762:

			if ( entityNum == cg.clientNum )
			{	// The person hit is the local player.
				if (!cg_lockBlood.integer)
				{	// Only play the exit wound effect
					trap_FX_PlayEffectID( cgs.media.playerFleshImpactEffect, origin, dir, -1, -1  );
				}
			}
			else if (cg_lockBlood.integer)
			{	// Only play the generic hit effect.
				trap_FX_PlayEffectID( trap_MAT_GetEffect(ammoData[ammoIndex].name, MATERIAL_NONE), 
									origin, dir, -1, -1  );
			}
			else
			{
				trap_FX_PlayEffectID( cgs.media.mBloodSmall, origin, dir, -1, -1  );
				trap_FX_PlayEffectID( cgs.media.playerFleshImpactEffect, origin, dir, -1, -1  );	// Exit wound too.
			}

			break;

		case AMMO_RPG7:
		case AMMO_40:
		case AMMO_M15:
		case AMMO_M67:
		case AMMO_M84:
		case AMMO_F1:
		case AMMO_L2A2:
		case AMMO_MDN11:
		case AMMO_SMOHG92:
		case AMMO_ANM14:
			if (cg_lockBlood.integer)
			{	// Play the generic hit effect.
				impactEffect = trap_MAT_GetEffect(ammoData[ammoIndex].name, MATERIAL_NONE);
			}
			else
			{
				impactEffect = cgs.media.mBloodSmall; // trap_MAT_GetEffect(ammoData[ammoIndex].name, MATERIAL_FLESH);
			}

			if (impactEffect)
			{
				trap_FX_PlayEffectID( impactEffect, origin, dir, -1, 3000 );
			}

			trap_FX_PlayEffectID( cg_weapons[weapon].attack[attack].explosionEffect, origin, dir, -1, 3000 );
/*
			trap_S_StartSound(origin, ENTITYNUM_WORLD, CHAN_WEAPON, cg_weapons[weapon].attack[attack].explosionSound, -1, -1);
*/

			break;
	}
}

void CG_HandleStickyMissile(centity_t *cent,entityState_t *es,vec3_t dir,int targetEnt)
{
	int				hitLoc;
	qboolean		altFire;
	int				ammoIndex;
	weaponInfo_t	*weaponInfo;
	qboolean		addBoltedMiss;
	int				missileIndex;
	int				boltIndex;

	hitLoc=es->otherEntityNum2;
	altFire=(cent->currentState.eFlags & EF_ALT_FIRING)!=0;

	if ( cent->currentState.eFlags & EF_ALT_FIRING )
	{
		ammoIndex = weaponData[es->weapon].attack[ATTACK_ALTERNATE].ammoIndex;
	}
	else
	{
		ammoIndex = weaponData[es->weapon].attack[ATTACK_NORMAL].ammoIndex;
	}
	
	// Currently the only type of sticky missile is the thrown knife. Others might be added in future though.
	addBoltedMiss=qfalse;
	switch(es->weapon)
	{
		case AMMO_KNIFE:
			if(altFire)
			{
				addBoltedMiss=qtrue;
			}
			break;

		default:
			break;
	}

	if(addBoltedMiss==qtrue)
	{
		centity_t* centTarget;

		weaponInfo=&cg_weapons[es->weapon];
		if(!trap_G2_HaveWeGhoul2Models(weaponInfo->weaponG2Model))
		{
			return;
		}

		centTarget = CG_GetEntity ( targetEnt );

		missileIndex=trap_G2API_CopySpecificGhoul2Model(weaponInfo->weaponG2Model,0,centTarget->ghoul2,-1);
		if(missileIndex!=-1)
		{
//			Com_Printf("Missile index=%i\n",missileIndex);
			boltIndex=trap_G2API_FindBoltIndex(centTarget->ghoul2,0,"rhand");
			if(boltIndex==-1)
			{
				boltIndex=trap_G2API_AddBolt(centTarget->ghoul2,0,"rhand");
			}
			if(boltIndex!=-1)
			{
//				Com_Printf("Bolt index=%i\n",boltIndex);
				if(!trap_G2API_AttachG2Model(centTarget->ghoul2,missileIndex,centTarget->ghoul2,boltIndex,0))
				{
//					Com_Printf("Couldn't attach!\n");
				}
			}
		}
	}
}

/*
============================================================================

BULLETS

============================================================================
*/


/*
===============
CG_Tracer
===============
*/
void CG_Tracer(int tracerEffectID, vec3_t source, vec3_t dest ) 
{
	vec3_t		forward;
	float		len;

	// Lower the amount of tracers
	float chance = cg_tracerChance.value * 100.0f;
	if ( rand()%100 > chance )
	{
		return;
	}

	VectorSubtract( dest, source, forward );
	len = VectorNormalize( forward );
	if (len > 100)
	{
		VectorMA( source, 100, forward, source );
		trap_FX_PlayEffectID( tracerEffectID, source, forward, -1, -1  );
	}
}

/*
======================
CG_Bullet

Renders bullet effects.
======================
*/
void CG_Bullet( 
	vec3_t		 end, 
	int			 sourceEntityNum, 
	int			 weapon, 
	vec3_t		 normal, 
	int			 fleshEntityNum, 
	int			 material,
	attackType_t attack
	)
{
	trace_t trace;
	int		sourceContentType;
	int		destContentType;
	vec3_t	start;

	// if the shooter is currently valid, calc a source point and possibly
	// do trail effects
	if ( sourceEntityNum >= 0 )
	{
		if ( CG_CalcMuzzlePoint( sourceEntityNum, start ) )
		{
			int	tracerEffectID = cg_weapons[weapon].attack[attack].tracerEffect;;
		
			if (0 != tracerEffectID ) 
			{
				sourceContentType = trap_CM_PointContents( start, 0 );
				destContentType = trap_CM_PointContents( end, 0 );

				// do a complete bubble trail if necessary
				if ( ( sourceContentType == destContentType ) && ( sourceContentType & CONTENTS_WATER ) ) {
					CG_BubbleTrail( start, end, 32 );
				}
				// bubble trail from water into air
				else if ( ( sourceContentType & CONTENTS_WATER ) ) {
					trap_CM_BoxTrace( &trace, end, start, NULL, NULL, 0, CONTENTS_WATER );
					CG_BubbleTrail( start, trace.endpos, 32 );
				}
				// bubble trail from air into water
				else if ( ( destContentType & CONTENTS_WATER ) ) {
					trap_CM_BoxTrace( &trace, start, end, NULL, NULL, 0, CONTENTS_WATER );
					CG_BubbleTrail( trace.endpos, end, 32 );
				}

				// Tracers on enemies only
				if ( sourceEntityNum != cg.snap->ps.clientNum || cg.renderingThirdPerson )
				{
					CG_Tracer(tracerEffectID, start, end );
				}
			}
		}

		if ( sourceEntityNum != cg.snap->ps.clientNum )
		{
			// Melee attacks should not cause the bullet fly by sound
			if ( !weaponData[weapon].attack[attack].melee )
			{
				CG_BulletFlyBySound (start, end);
			}
		}
	}

	// impact splash and mark
	if ( fleshEntityNum != ENTITYNUM_WORLD ) 
	{
		CG_MissileHitPlayer(weapon, end, normal, fleshEntityNum, qfalse); 
	}
	else 
	{
		CG_MissileHitWall( weapon, end, normal, material, attack );
	}
}

/*
===============
CG_StartModelAnims
===============
*/
static void CG_StartModelAnims ( TAnimWeapon *aW, playerState_t *ps )
{
	TAnimInfoWeapon *aIW;

	if(!aW)
	{
		assert(0);
	}

	cg.weaponHideModels=-1;

	// Loop for all models affected by this anim.
	aIW=aW->mInfos;
	while(aIW)
	{
		// Ignore if no anims available.
		if ( !aIW->mNumChoices )
		{
			aIW=aIW->mNext;
			continue;
		}

		if(!Q_stricmp(aIW->mType,"hands"))
		{
			// Hands... now which one?
			if(!Q_stricmp(aIW->mName,"right"))
			{
				// Right.
				cg.viewWeaponAnim[2] = aIW;	
				cg.weaponHideModels&=~(1<<2);
			}
			else if(!Q_stricmp(aIW->mName,"left"))
			{
				// Left.
				cg.viewWeaponAnim[3] = aIW;	
				cg.weaponHideModels&=~(1<<3);
			}
		}
		else if(!Q_stricmp(aIW->mType,"weaponmodel"))
		{
			cg.viewWeaponAnim[0] = aIW;
			cg.weaponHideModels&=~(1<<0);
		}
		else if(!Q_stricmp(aIW->mType,"bolton"))
		{
			// Weapon model.
			cg.viewWeaponAnim[4] = aIW;
			cg.weaponHideModels&=~(1<<4);
		}
		else
		{
			Com_Printf("Anim unknown for model %s\n",aIW->mType);
		}
		
		// Follow link to next model.
		aIW=aIW->mNext;
	}
}

/*
===============
CG_SetWeaponAnim
===============
*/
void CG_SetWeaponAnim(int weaponAction,playerState_t *ps)
{
	TAnimWeapon	*aW;
	aW=BG_GetInviewAnimFromIndex(ps->weapon,weaponAction);
	if(aW)
	{
		CG_StartModelAnims(aW,ps);
	}
}

/*
===============
CG_WeaponCallback
===============
*/
void CG_WeaponCallback ( playerState_t* ps, centity_t* cent, int weapon, int anim, int animChoice, int step )
{
	int				i,j;
	TNoteTrack		*note;
	entityState_t	*ent;	
	weaponInfo_t	*weaponInfo;
	char			*surfName;
	int				surfFlags;

	ent  = &cent->currentState;

	// Make sure the weapon number is valid
	if( weapon <= WP_NONE || weapon >= WP_NUM_WEAPONS )
	{
		Com_Error( ERR_FATAL, "CG_WeaponCallabck: ps->weapon >= WP_NUM_WEAPONS");
		return;
	}

	weaponInfo = &cg_weapons[ weapon ];

	note = BG_GetWeaponNote ( ps, weapon, anim, animChoice, step );
	if ( !note )
	{
		return;
	}

	if(!strcmp(note->mNote,"fire"))
	{
		// Callback is type fire = muzzle flash + sound + shell eject.
		// TODO.
	}
	else if(!strcmp(note->mNote,"altfire"))
	{
		// Callback is type altfire = muzzle flash + sound + shell eject.
		// TODO.
	}
	else
	{
		// Search the sound list... is it a sound callback?
		for(i=0;i<MAX_WEAPON_SOUNDS;i++)
		{
			if(!weaponParseInfo[weapon].mSoundNames[i][0])
			{
				break;
			}

			if(!strcmp(weaponParseInfo[weapon].mSoundNames[i],note->mNote))
			{
				// Play sound.
				// FIXME: randomly select sound??
				trap_S_StartSound(NULL,ent->number,CHAN_WEAPON,weaponInfo->otherWeaponSounds[i][0], -1, -1);
				break;
			}
		}

		// Search the surfaces list... is it a surfaces callback?
		for(i=0;i<MAX_SURFACE_CALLBACKS;i++)
		{
			if(!weaponParseInfo[weapon].mSurfaceCallbacks[i].mName[0])
			{
				break;
			}
			if(!strcmp(weaponParseInfo[weapon].mSurfaceCallbacks[i].mName,note->mNote))
			{
				for(j=0;j<MAX_CALLBACK_SURFACES;j++)
				{
					surfName=weaponParseInfo[weapon].mSurfaceCallbacks[i].mOnOffSurfaces[j].mName;
					if(!surfName[0])
					{
						break;
					}
					// Turn Ghoul2 surface on/off.
					surfFlags=weaponParseInfo[weapon].mSurfaceCallbacks[i].mOnOffSurfaces[j].mStatus?0:GHOUL2_NORENDER;
					trap_G2API_SetSurfaceOnOff(weaponInfo->viewG2Model,weaponInfo->viewG2Indexes[0],surfName,surfFlags);
				}
				break;
			}
		}
	}
}
