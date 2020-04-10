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
// cg_weaponinit.c -- weapon initialization

#include "cg_local.h"
#include "../game/bg_weapons.h"

// FIMXE: This is defined in a C++ header file. 
// We need to break it up or somethin so we don't have mutliple defs.
#define GHOUL2_NORENDER 2

/*
=================
CG_RegisterItemVisuals

The server says this item is used on this level
=================
*/
void CG_RegisterItemVisuals( int itemNum ) 
{
	itemInfo_t		*itemInfo;
	gitem_t			*item;
	char			simpleName[MAX_QPATH];

	if ( itemNum < 0 || itemNum >= bg_numItems ) 
	{
		Com_Error( ERR_FATAL, "CG_RegisterItemVisuals: itemNum %d out of range [0-%d]", itemNum, bg_numItems-1 );
	}

	itemInfo = &cg_items[ itemNum ];
	if ( itemInfo->registered ) 
	{
		return;
	}

	item = &bg_itemlist[ itemNum ];

	memset( itemInfo, 0, sizeof( &itemInfo ) );
	itemInfo->registered = qtrue;

	itemInfo->models[0] = trap_R_RegisterModel( item->world_model[0] );

	// Check to see if its a ghoul model we loaded
	if (!Q_stricmp(&item->world_model[0][strlen(item->world_model[0]) - 4], ".glm"))
	{
		trap_G2API_InitGhoul2Model(&itemInfo->g2Models[0], item->world_model[0], 0 , 0, 0, 0, 0);
		itemInfo->radius[0] = 60;

		// Special case to make sure some sufaces are show that should be
		switch ( item->giTag )
		{
			case WP_AK74_ASSAULT_RIFLE:
				trap_G2API_SetSurfaceOnOff( itemInfo->g2Models[0], 0, "bayonet_off", 0 );
				break;

			case WP_M4_ASSAULT_RIFLE:
				trap_G2API_SetSurfaceOnOff( itemInfo->g2Models[0], 0, "m203_off", 0 );
				break;
		}
	}

	itemInfo->icon = trap_R_RegisterShaderNoMip( item->icon );

	if (item->icon[0])
	{
		Com_sprintf(simpleName, sizeof(simpleName), "%s_simple", item->icon);
		itemInfo->mSimpleIcon = trap_R_RegisterShader(simpleName);
	}
	else
	{
		itemInfo->mSimpleIcon = 0;
	}


	if ( item->giType == IT_WEAPON ) 
	{
		CG_RegisterWeapon( item->giTag );
	}
}

/*
=================
CG_RegisterWeapon

The server says this item is used on this level
=================
*/
void CG_RegisterWeapon(int weaponNum)
{
	weaponInfo_t		*weaponInfo;
	weaponData_t		*weaponDat;
	TWeaponModel		*weaponModel;
	TOptionalWeapon		*optionalPart;
	gitem_t				*item,*ammo;
	TWeaponParseInfo	*wPI;
	int					weaponIndex;	// model index for view weapon models
	int					bufferIndex;	// weapon + buffer + rhand [+ lhand ]
	int					rHandIndex;
	int					lHandIndex;
	int					boltonIndex;
	int					boltIndex;
	int					soundSet, i;
	int					*indexes;

	if(weaponNum==0)
	{
		return;
	}
	
	boltonIndex = -1;

	assert(weaponNum < WP_NUM_WEAPONS);

	weaponInfo=&cg_weapons[weaponNum];
	if(weaponInfo->registered)
	{
		return;
	}

	memset(weaponInfo,0,sizeof(*weaponInfo));

	// Register the item visuals (icons etc).
	weaponInfo->item = item = BG_FindWeaponItem ( weaponNum );

	// Must be initial loading
	if ( !weaponInfo->registered )
	{
		CG_LoadingItem( item - bg_itemlist );
	}

	// Ok, successfully registered the entire weapon.
	weaponInfo->registered=qtrue;

	weaponDat = &weaponData[weaponNum];

	if(item->classname)
	{
		// Hmmm... this needs some work... some of it looks if now.
		CG_RegisterItemVisuals( item - bg_itemlist );	
	}
	else
	{	
		Com_Error( ERR_FATAL, "CG_RegisterWeapon: Couldn't find weapon item %i", weaponNum);
	}

	// Register the weapon's world ammo model.
	for ( ammo = bg_itemlist + 1 ; ammo->classname ; ammo++ )
	{
		if ( ammo->giType == IT_AMMO && ammo->giTag == weaponNum )
		{
			break;
		}
	}

	if( ammo->classname && ammo->world_model[0] )
	{
		trap_G2API_InitGhoul2Model(&weaponInfo->ammoG2Model,ammo->world_model[0],0,0,0,0,0);
	}

	// Create the world model.
	weaponIndex = trap_G2API_InitGhoul2Model(&weaponInfo->weaponG2Model,weaponDat->worldModel,0,0,0,0,0);
	if(!trap_G2_HaveWeGhoul2Models(weaponInfo->weaponG2Model))
	{
		Com_Printf("CG_RegisterWeapon: Unable to load weapon world model: %s\n", weaponDat->worldModel);
	}

	// Register the weapon model...
	indexes=weaponInfo->viewG2Indexes;
	for(i=0;i<8;i++)
	{
		indexes[i]=-1;
	}

	wPI=&weaponParseInfo[weaponNum];
	
	// Create the view weapon model in slot 0.
	indexes[0] = weaponIndex = trap_G2API_InitGhoul2Model(&weaponInfo->viewG2Model,
													      wPI->mWeaponModel.mModel,0,0,0,0,0);

	// Now build and assemble all the other bits.
	if(trap_G2_HaveWeGhoul2Models(weaponInfo->viewG2Model))
	{		
		// Add a bolt to the weapon so buffer can be bolted on.
		boltIndex=trap_G2API_AddBolt(weaponInfo->viewG2Model,weaponIndex,wPI->mWeaponModel.mBufferBoltToBone);

		// Every view weapon has a buffer, create this in slot 1.
		indexes[1]=bufferIndex=trap_G2API_InitGhoul2Model(&weaponInfo->viewG2Model,
														  wPI->mWeaponModel.mBufferModel,
														  weaponIndex+1,0,0,0,0);
		// Bolt the buffer to the weapon.
		trap_G2API_AttachG2Model(weaponInfo->viewG2Model,bufferIndex, 
								 weaponInfo->viewG2Model,boltIndex,weaponIndex);

		// Right hand??
		if(wPI->mWeaponModel.mRightHandsBoltToBone[0])
		{
			// Add a right hand bolt to the buffer.
			boltIndex=trap_G2API_AddBolt(weaponInfo->viewG2Model,bufferIndex,
										 wPI->mWeaponModel.mRightHandsBoltToBone);

			// Right hand.. create this now in slot 2.
			indexes[2]=rHandIndex=trap_G2API_InitGhoul2Model(&weaponInfo->viewG2Model,
								 							 "models/weapons/rhand/rhand.glm",
															 weaponIndex+2,0,0,0,0);
			
			// Bolt the right hand to the buffer.
			trap_G2API_AttachG2Model(weaponInfo->viewG2Model,rHandIndex,weaponInfo->viewG2Model,
									 boltIndex,bufferIndex);
		}

		// Left hand??
		if(wPI->mWeaponModel.mLeftHandsBoltToBone[0])
		{
			// Add a left hand bolt to the buffer.
			boltIndex=trap_G2API_AddBolt(weaponInfo->viewG2Model,bufferIndex,
										 wPI->mWeaponModel.mLeftHandsBoltToBone);

			// Left hand.. create this now in slot 3.
			indexes[3]=lHandIndex=trap_G2API_InitGhoul2Model(&weaponInfo->viewG2Model,
														    "models/weapons/lhand/lhand.glm",
															weaponIndex+3,0,0,0,0);

			// Bolt the left hand to the buffer.
			trap_G2API_AttachG2Model(weaponInfo->viewG2Model,lHandIndex,weaponInfo->viewG2Model,
									 boltIndex,bufferIndex);
		}

		// Boltons like the M4's grenade launcher?
		if(wPI->mWeaponModel.mBolton)
		{
			// Add a bolton bolt to the buffer.
			boltIndex=trap_G2API_AddBolt(weaponInfo->viewG2Model,bufferIndex,
										 wPI->mWeaponModel.mBolton->mBoltToBone);

			// Bolton.. create this now in slot 4.
			indexes[4]=boltonIndex=trap_G2API_InitGhoul2Model(&weaponInfo->viewG2Model,
															  wPI->mWeaponModel.mBolton->mModel,
															  weaponIndex+4,0,0,0,0);
			// Bolt the bolton to the buffer.
			if ( boltonIndex != -1 )
			{
				trap_G2API_AttachG2Model(weaponInfo->viewG2Model,boltonIndex,weaponInfo->viewG2Model,
										 boltIndex,bufferIndex);
			}
		}

		// Turn of any weapon surfaces that are never seen.
		weaponModel=&wPI->mWeaponModel;
		i=0;
		while(weaponModel->mRightSideSurfaces[i])
		{
			trap_G2API_SetSurfaceOnOff(weaponInfo->viewG2Model,weaponInfo->viewG2Indexes[0],
									   weaponModel->mRightSideSurfaces[i],GHOUL2_NORENDER);
			i++;
		}
		i=0;
		while(weaponModel->mFrontSurfaces[i])
		{
			trap_G2API_SetSurfaceOnOff(weaponInfo->viewG2Model,weaponInfo->viewG2Indexes[0],
									   weaponModel->mFrontSurfaces[i],GHOUL2_NORENDER);
			i++;
		}
		if(weaponModel->mBolton && boltonIndex != -1 )
		{
			i=0;
			while(weaponModel->mBolton->mRightSide[i])
			{
				trap_G2API_SetSurfaceOnOff(weaponInfo->viewG2Model,boltonIndex,
										   weaponModel->mBolton->mRightSide[i],GHOUL2_NORENDER);
				i++;
			}
		}
		
		// Turn off optional parts as we don't want em.
		optionalPart=weaponModel->mOptionalList;
		while(optionalPart)
		{
			// No real way of knowing what single player intended so we have to
			// hard-code 'em.
			if((!strcmp(optionalPart->mName,"lasersight"))||
			   (!strcmp(optionalPart->mName,"silencer"))||
			   (!strcmp(optionalPart->mName,"utl")))
			{
				i=0;
				while(optionalPart->mSurfaces[i])
				{
					trap_G2API_SetSurfaceOnOff(weaponInfo->viewG2Model,weaponInfo->viewG2Indexes[0],
											   optionalPart->mSurfaces[i],GHOUL2_NORENDER);
					i++;
				}
			}
			optionalPart=optionalPart->mNext;
		}
	}
	else
	{
		Com_Printf("CG_RegisterWeapon: Unable to load weapon view model: %s\n", wPI->mWeaponModel.mModel);
	}

	weaponInfo->attack[ATTACK_NORMAL].missileTrailFunc = CG_ProjectileThink;
	weaponInfo->attack[ATTACK_ALTERNATE].missileTrailFunc = CG_ProjectileThink;

	// Register weapon list menu icons.
	if (weaponDat->menuImage[0])
	{
		weaponInfo->weaponIcon = trap_R_RegisterShader( va( "gfx/menus/%s", weaponDat->menuImage) );
	}

	// Register the alternate weapon ammo icon for the hud
	if (weaponDat->attack[ATTACK_ALTERNATE].icon[0] )
	{
		weaponInfo->attack[ATTACK_ALTERNATE].ammoIcon = trap_R_RegisterShader( va( "gfx/menus/%s", weaponDat->attack[ATTACK_ALTERNATE].icon ) );
	}

	// Register attack stuff
	for ( i = ATTACK_NORMAL; i < ATTACK_MAX; i ++ )
	{
		attackData_t*	attackData = &weaponDat->attack[i];
		attackInfo_t*	attackInfo = &weaponInfo->attack[i];

		// Primary muzzle-flash.
		if (attackData->muzzleEffect[0])
			attackInfo->muzzleEffect = trap_FX_RegisterEffect(attackData->muzzleEffect);
		if (attackData->muzzleEffectInWorld[0])
			attackInfo->muzzleEffectInWorld = trap_FX_RegisterEffect(attackData->muzzleEffectInWorld);
		if (!attackInfo->muzzleEffectInWorld)
			attackInfo->muzzleEffectInWorld = attackInfo->muzzleEffect;

		// Primary shell eject.
		if (attackData->shellEject[0])
			attackInfo->shellEject = trap_FX_RegisterEffect(attackData->shellEject);
		if (attackData->tracerEffect[0])
			attackInfo->tracerEffect = trap_FX_RegisterEffect(attackData->tracerEffect);

		// Primary explosion.
		if (attackData->explosionEffect[0])
			attackInfo->explosionEffect = trap_FX_RegisterEffect(attackData->explosionEffect);
		if (attackData->explosionSound[0])
			attackInfo->explosionSound = trap_S_RegisterSound(attackData->explosionSound);
		if (attackData->missileG2Model[0])
			trap_G2API_InitGhoul2Model(&attackInfo->missileG2Model,attackData->missileG2Model,0,0,0,0,0);

		// Add the bolts for muzzle flash and shell eject onto the view model.
		if (trap_G2_HaveWeGhoul2Models(weaponInfo->viewG2Model))
		{
			// shell eject bone.
			if ( attackData->ejectBone[0] )
			{
				attackInfo->shellEjectBoltView = 
					trap_G2API_AddBolt(weaponInfo->viewG2Model, 1, attackData->ejectBone);	
			}

			// Muzzle flash bone.
			attackInfo->muzzleFlashBoltView = -1; 
			if(attackData->muzzleEffect[0])
			{
				if ( attackData->muzzleEffectBone[0] )
				{
					attackInfo->muzzleFlashBoltView = 
						trap_G2API_AddBolt(weaponInfo->viewG2Model, 1, attackData->muzzleEffectBone );
				}
				else if (wPI->mWeaponModel.mBufferMuzzle[0])
				{
					attackInfo->muzzleFlashBoltView = 
						trap_G2API_AddBolt(weaponInfo->viewG2Model, 1, wPI->mWeaponModel.mBufferMuzzle);
				}
			}
		}

		// Add the bolts for muzzle flash and shell eject onto the world model.
		if (trap_G2_HaveWeGhoul2Models(weaponInfo->weaponG2Model))
		{
			// shell eject bone.
			if (attackData->ejectBone[0])
			{	
				attackInfo->shellEjectBoltWorld = 
					trap_G2API_AddBolt(weaponInfo->weaponG2Model, 0, attackData->ejectBone);	
			}
			else
			{
				attackInfo->shellEjectBoltWorld = -1;
			}

			// Muzzle flash bone.
			attackInfo->muzzleFlashBoltWorld = -1;
			if ( attackData->muzzleEffectBone[0] )
			{
				attackInfo->muzzleFlashBoltWorld = 
					trap_G2API_AddBolt(weaponInfo->weaponG2Model, 0, attackData->muzzleEffectBone );
			}
			else if (wPI->mWeaponModel.mBufferMuzzle[0])
			{
				attackInfo->muzzleFlashBoltWorld = 
					trap_G2API_AddBolt(weaponInfo->weaponG2Model, 0, wPI->mWeaponModel.mBufferMuzzle);
			}
		}
	}

	// Register sounds for weapon.
	for (soundSet = 0; soundSet < MAX_WEAPON_SOUNDS; soundSet++)
	{
		if (Q_stricmp(wPI->mSoundNames[soundSet], "fire")==0)
		{
			for (i=0; i<MAX_WEAPON_SOUND_SLOTS; i++)
			{
				if (wPI->mSounds[soundSet][i][0])
				{
					weaponInfo->attack[ATTACK_NORMAL].flashSound[i] = trap_S_RegisterSound(wPI->mSounds[soundSet][i]);
				}
			}
		}
		else if (Q_stricmp(wPI->mSoundNames[soundSet], "altfire")==0)
		{
			for (i=0; i<MAX_WEAPON_SOUND_SLOTS; i++)
			{
				if (wPI->mSounds[soundSet][i][0])
					weaponInfo->attack[ATTACK_ALTERNATE].flashSound[i] = trap_S_RegisterSound(wPI->mSounds[soundSet][i]);
			}
		}
		else
		{
			// All other sounds.
			for (i=0; i<MAX_WEAPON_SOUND_SLOTS; i++)
			{
				if (wPI->mSounds[soundSet][i][0])
					weaponInfo->otherWeaponSounds[soundSet][i]=trap_S_RegisterSound(wPI->mSounds[soundSet][i]);
			}
		}
	}

	// Special hard coded stuff
	switch ( weaponNum )
	{
		case WP_KNIFE:
		{
			attackInfo_t* attackInfo = &weaponInfo->attack[ATTACK_ALTERNATE];
			attackInfo->missileSound = trap_S_RegisterSound ( "sound/weapons/knife/throw_loop" );
			break;
		}

		case WP_RPG7_LAUNCHER:
		{
			attackInfo_t* attackInfo = &weaponInfo->attack[ATTACK_NORMAL];
			attackInfo->missileSound = trap_S_RegisterSound ( "sound/weapons/rpg7/flyby" );
			break;
		}			
	}	
}

/*
=================
CG_ShutDownWeapons

Clean out any g2 models
=================
*/
void CG_ShutDownWeapons ( void )
{
	int				i;
	attackType_t	a;

	for (i=0; i < WP_NUM_WEAPONS; i++)
	{	
		// free all ghoul2 models
		trap_G2API_CleanGhoul2Models(&cg_weapons[i].weaponG2Model);
		trap_G2API_CleanGhoul2Models(&cg_weapons[i].viewG2Model);
		trap_G2API_CleanGhoul2Models(&cg_weapons[i].ammoG2Model);

		for ( a = ATTACK_NORMAL; a < ATTACK_MAX; a ++ )
		{
			trap_G2API_CleanGhoul2Models(&cg_weapons[i].attack[a].missileG2Model);
		}
	}
}
