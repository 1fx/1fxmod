// Copyright (C) 2001-2002 Raven Software
//
// cg_event.c -- handle entity events at snapshot or playerstate transitions

#include "cg_local.h"
#include "..\ghoul2\g2.h"
#include "../../ui/menudef.h"

//==========================================================================

/*
===================
CG_PlaceString

Also called by scoreboard drawing
===================
*/
const char	*CG_PlaceString( int rank ) {
	static char	str[64];
	char	*s, *t;

	if ( rank & RANK_TIED_FLAG ) {
		rank &= ~RANK_TIED_FLAG;
		t = "Tied for ";
	} else {
		t = "";
	}

	if ( rank == 1 ) {
		s = S_COLOR_BLUE "1st" S_COLOR_WHITE;		// draw in blue
	} else if ( rank == 2 ) {
		s = S_COLOR_RED "2nd" S_COLOR_WHITE;		// draw in red
	} else if ( rank == 3 ) {
		s = S_COLOR_YELLOW "3rd" S_COLOR_WHITE;		// draw in yellow
	} else if ( rank == 11 ) {
		s = "11th";
	} else if ( rank == 12 ) {
		s = "12th";
	} else if ( rank == 13 ) {
		s = "13th";
	} else if ( rank % 10 == 1 ) {
		s = va("%ist", rank);
	} else if ( rank % 10 == 2 ) {
		s = va("%ind", rank);
	} else if ( rank % 10 == 3 ) {
		s = va("%ird", rank);
	} else {
		s = va("%ith", rank);
	}

	Com_sprintf( str, sizeof( str ), "%s%s", t, s );
	return str;
}

/*
=============
CG_GameOver
=============
*/
static void CG_GameOver ( entityState_t *ent )
{
	switch ( ent->eventParm )
	{
		case GAME_OVER_TIMELIMIT:
			Com_sprintf ( cgs.gameover, MAX_QPATH, "Timelimit Hit" );
			break;

		case GAME_OVER_SCORELIMIT:
			if ( cgs.gametypeData->teams )
			{
				switch ( ent->otherEntityNum )
				{
					case TEAM_RED:					
						Com_sprintf ( cgs.gameover, MAX_QPATH, "Red Team hit the score limit" );
						break;

					case TEAM_BLUE:
						Com_sprintf ( cgs.gameover, MAX_QPATH, "Blue Team hit the score limit" );
						break;
				}
			}
			else
			{
				Com_sprintf ( cgs.gameover, MAX_QPATH, "%s" S_COLOR_WHITE " hit the score limit", cgs.clientinfo[ent->otherEntityNum].name );
			}
			break;

		default:
			return;
	} 	

	CG_CenterPrint ( cgs.gameover, 0.43f );

	Com_Printf ( "@%s\n", cgs.gameover );
}

/*
=============
CG_Obituary
=============
*/
static void CG_Obituary( entityState_t *ent ) 
{
	int				mod;
	int				target, attacker;
	char			*message;
	char			*message2;
	const char		*targetInfo;
	const char		*attackerInfo;
	char			targetName[32];
	char			attackerName[32];
	const char		*targetColor;
	const char		*attackerColor;
	attackType_t	attack;
	gender_t		gender;
	clientInfo_t	*ci;

	target		= ent->otherEntityNum;
	attacker	= ent->otherEntityNum2;
	mod			= ent->eventParm & 0xFF;
	attack		= (ent->eventParm >> 8) & 0xFF;
	attackerColor = S_COLOR_WHITE;
	targetColor   = S_COLOR_WHITE;

	if ( target < 0 || target >= MAX_CLIENTS ) 
	{
		Com_Error( ERR_FATAL, "CG_Obituary: target out of range" );
	}

	// Play the death sound, water if they drowned
	if ( mod == MOD_WATER )
	{
		trap_S_StartSound ( NULL, target, CHAN_AUTO, cgs.media.drownDeathSound, -1, -1 );
	}
	else
	{
		trap_S_StartSound( NULL, target, CHAN_VOICE, CG_CustomPlayerSound(target, SOUND_DIE_1 + (cg.time%3)), -1, -1);
	}

	// Play the frag sound, and make sure its not played more than every 250ms
	if ( cg.time - cg.lastKillTime > 250 && attacker == cg.snap->ps.clientNum )
	{
		if ( cg_soundFrag.integer )
		{
			// If the attacker killed themselves play the selffrag sound
			if ( attacker == target )
			{
				trap_S_StartLocalSound ( cgs.media.fragSelfSound, CHAN_AUTO );
			}
			else
			{
				// In a team game a kill of a teammate will play the self frag sound rather 
				// than the frag sound
				if ( cgs.gametypeData->teams )
				{
					if ( cgs.clientinfo[target].team == cgs.clientinfo[attacker].team )
					{
						trap_S_StartLocalSound ( cgs.media.fragSelfSound, CHAN_AUTO );
					}
					else
					{
						trap_S_StartLocalSound ( cgs.media.fragSound, CHAN_AUTO );
					}
				}
				else
				{
					trap_S_StartLocalSound ( cgs.media.fragSound, CHAN_AUTO );
				}
			}
		}

		cg.lastKillTime = cg.time;
	}			

	ci = &cgs.clientinfo[target];

	if ( attacker < 0 || attacker >= MAX_CLIENTS ) 
	{
		attacker = ENTITYNUM_WORLD;
		attackerInfo = NULL;
	} 
	else 
	{
		attackerInfo = CG_ConfigString( CS_PLAYERS + attacker );
	}

	targetInfo = CG_ConfigString( CS_PLAYERS + target );
	if ( !targetInfo ) {
		return;
	}
	Q_strncpyz( targetName, Info_ValueForKey( targetInfo, "n" ), sizeof(targetName) - 2);
	strcat( targetName, S_COLOR_WHITE );

	switch ( cgs.clientinfo[target].team )
	{
		case TEAM_RED:
			targetColor = S_COLOR_RED;
			break;

		case TEAM_BLUE:
			targetColor = S_COLOR_BLUE;
			break;
	}

	message2 = "";

	// check for single client messages

	gender = ci->gender;

	switch( mod ) 
	{
		case MOD_SUICIDE:
			message = "suicides";
			break;
		case MOD_FALLING:
			if ( gender == GENDER_FEMALE )
				message = "fell to her death";
			else
				message = "fell to his death";
			break;
		case MOD_CRUSH:
			message = "was squished";
			break;
		case MOD_WATER:
			message = "sank like a rock";
			break;
		case MOD_TARGET_LASER:
			message = "saw the light";
			break;
		case MOD_TRIGGER_HURT:
			message = "was in the wrong place";
			break;
		case MOD_TEAMCHANGE:
			return;

		default:
			message = NULL;
			break;
	}

	// Attacker killed themselves.  Ridicule them for it.
	if (attacker == target) 
	{
		switch (mod) 
		{
			case MOD_MM1_GRENADE_LAUNCHER:    
			case MOD_RPG7_LAUNCHER:           
			case MOD_M67_GRENADE:            
			case MOD_M84_GRENADE:
			case MOD_F1_GRENADE:
			case MOD_L2A2_GRENADE:
			case MOD_MDN11_GRENADE:
			case MOD_SMOHG92_GRENADE:
			case MOD_ANM14_GRENADE:
			case MOD_M15_GRENADE:
				if ( gender == GENDER_FEMALE )
					message = "blew herself up";
				else if ( gender == GENDER_NEUTER )
					message = "blew itself up";
				else
					message = "blew himself up";
				break;

			default:
				if ( gender == GENDER_FEMALE )
					message = "killed herself";
				else if ( gender == GENDER_NEUTER )
					message = "killed itself";
				else
					message = "killed himself";
				break;
		}
	}

	if (message) 
	{
		Com_Printf( "%s%s %s.\n", targetColor, targetName, message);
		return;
	}

	// check for kill messages from the current clientNum when
	// not in a team game.
	if ( cgs.gametypeData->showKills )
	{
		if ( attacker == cg.snap->ps.clientNum ) 
		{
			char	*s;

			if ( !cgs.gametypeData->teams ) 
			{
				s = va("You killed %s%s\n%s place with %i", targetColor, targetName, 
					CG_PlaceString( cg.snap->ps.persistant[PERS_RANK] + 1 ),
					cg.snap->ps.persistant[PERS_SCORE] );
			} 
			else 
			{
				s = va("You killed %s%s", targetColor, targetName );
			}

			CG_CenterPrint( s, 0.43f );
		}
	}


	// check for double client messages
	if ( !attackerInfo ) 
	{
		attacker = ENTITYNUM_WORLD;
		strcpy( attackerName, "noname" );
	} 
	else 
	{
		Q_strncpyz( attackerName, Info_ValueForKey( attackerInfo, "n" ), sizeof(attackerName) - 2);
		strcat( attackerName, S_COLOR_WHITE );
		// check for kill messages about the current clientNum
		if ( target == cg.snap->ps.clientNum ) 
		{
			Q_strncpyz( cg.killerName, attackerName, sizeof( cg.killerName ) );
		}

		switch ( cgs.clientinfo[attacker].team )
		{
			case TEAM_RED:
				attackerColor = S_COLOR_RED;
				break;

			case TEAM_BLUE:
				attackerColor = S_COLOR_BLUE;
				break;
		}
	}

			
	if ( attacker != ENTITYNUM_WORLD ) 
	{
		switch (mod) 
		{
			case MOD_KNIFE:
				message = "was sliced by";
				break;

			case MOD_USAS_12_SHOTGUN:
			case MOD_M590_SHOTGUN:
				if ( attack == ATTACK_ALTERNATE )
				{
					message = "was bludgeoned by";
				}
				else
				{
					message = "was pumped full of lead by";
				}
				break;

			case MOD_M1911A1_PISTOL:
			case MOD_USSOCOM_PISTOL: 
				if ( attack == ATTACK_ALTERNATE )
				{
					message = "was pistol whipped by";
				}
				else
				{
					message = "was shot by";
				}
				break;

			case MOD_AK74_ASSAULT_RIFLE:
				if ( attack == ATTACK_ALTERNATE )
				{
					message = "was stabbed by";
				}
				else
				{
					message = "was shot by";
				}
				break;

			case MOD_M60_MACHINEGUN:
			case MOD_MICRO_UZI_SUBMACHINEGUN:
			case MOD_M3A1_SUBMACHINEGUN:
			case MOD_M4_ASSAULT_RIFLE:
				message = "was shot by";
				break;

			case MOD_MSG90A1_SNIPER_RIFLE:    
				message = "was sniped by";
				break;

			case MOD_MM1_GRENADE_LAUNCHER:    
			case MOD_RPG7_LAUNCHER:           
			case MOD_M67_GRENADE:            
			case MOD_M84_GRENADE:
			case MOD_F1_GRENADE:
			case MOD_L2A2_GRENADE:
			case MOD_MDN11_GRENADE:
			case MOD_SMOHG92_GRENADE:
			case MOD_ANM14_GRENADE:
			case MOD_M15_GRENADE:
				message = "was detonated by";
				break;

			case MOD_TELEFRAG:
				message = "tried to invade";
				message2 = "'s personal space";
				break;

			default:
				message = "was killed by";
				break;
		}

		if (message) {
			Com_Printf( "%s%s %s %s%s%s\n", targetColor, targetName, message, attackerColor, attackerName, message2);
			return;
		}
	}

	// we don't know what it was
	Com_Printf( "%s%s died.\n", targetColor, targetName );
}

/*
================
CG_ItemPickup

A new item was picked up this frame
================
*/
static void CG_ItemPickup( int itemNum, qboolean autoswitch ) 
{
	cg.itemPickup = itemNum;

	// see if it should be the grabbed weapon
	if ( cg_autoswitch.integer && bg_itemlist[itemNum].giType == IT_WEAPON && autoswitch ) 
	{
		if ( cg_autoswitch.integer >= 2 )
		{
			if ( weaponData[bg_itemlist[itemNum].giTag].safe )
			{
				cg.weaponSelectTime = cg.time;
				cg.weaponSelect = bg_itemlist[itemNum].giTag;
			}
		}
		else
		{
			cg.weaponSelectTime = cg.time;
			cg.weaponSelect = bg_itemlist[itemNum].giTag;
		}
	}

	Com_Printf ( "You picked up %s %s!\n", bg_itemlist[itemNum].pickup_prefix, bg_itemlist[itemNum].pickup_name );
}


/*
================
CG_PainEvent

Also called by playerstate transition
================
*/
void CG_PainEvent( centity_t *cent, int health ) 
{
	ECustomSounds	sound;

	// don't do more than two pain sounds a second
	if ( cg.time - cent->pe.painTime < 500 ) {
		return;
	}

	if (health <= 0 )
	{
		return;
	}
	else if ( health < 25 ) 
	{
		sound = SOUND_PAIN_1;
	} 
	else if ( health < 50 ) 
	{
		sound = SOUND_PAIN_2;
	} 
	else if ( health < 75 ) 
	{
		sound = SOUND_PAIN_3;
	} 
	else 
	{
		sound = SOUND_PAIN_3;
	}
	trap_S_StartSound( NULL, cent->currentState.number, CHAN_VOICE, 
		CG_CustomPlayerSound( cent->currentState.number, sound ), -1, -1 );

	// save pain time for programitic twitch animation
	cent->pe.painTime = cg.time;
	cent->pe.painDirection	^= 1;
}


static void CG_BodyQueueCopy(centity_t *cent, int clientNum, int hitLocation, vec3_t direction )
{
	centity_t		*source;
	animation_t		*anim;
	float			animSpeed;
	int				flags=BONE_ANIM_OVERRIDE_FREEZE;
	clientInfo_t	*ci;
	int				i;

	if (cent->ghoul2)
	{
		trap_G2API_CleanGhoul2Models(&cent->ghoul2);
		cent->ghoul2 = 0;
	}

	if (clientNum < 0 || clientNum >= MAX_CLIENTS)
	{
		return;
	}

	source = CG_GetEntity ( clientNum );

	ci = &cgs.clientinfo[ clientNum ];

	cent->radius = 100;

	if (!source)
	{
		return;
	}

	// Make sure the player model is updated before copying it to the body queue
	CG_UpdatePlayerModel ( source );

	if (!source->ghoul2)
	{	
		// some how we don't have a g2 model, so don't do anything
		return;
	}

	// Remove the weapon bolt for the death
	if ( source->pe.weaponModelSpot )
	{
		trap_G2API_DetachG2Model ( source->ghoul2, source->pe.weaponModelSpot );
		trap_G2API_RemoveGhoul2Model ( &source->ghoul2, source->pe.weaponModelSpot );
		source->pe.weaponModelSpot = 0;

		source->flashBoltInterface.isValid = qfalse;
		source->ejectBoltInterface.isValid = qfalse;
	}

	trap_G2API_DuplicateGhoul2Instance(source->ghoul2, &cent->ghoul2);
	
	// Reset all mision bolt positions
	for ( i = 0; i < MAX_GAMETYPE_ITEMS; i ++ )
	{
		ci->boltGametypeItems[i] = -1;
	}

	ci->boltNightvision = -1;

	// Clear the source's ghoul2 model to force it to be re-duplicatd.  This will
	// then cause all the gore attached to it to be cleared
	trap_G2API_CleanGhoul2Models ( &source->ghoul2 );
	source->ghoul2 = NULL;

	if (cg_lockDeaths.integer)
	{
		anim = &ci->animations[ BOTH_DEATH_CHEST_1 ];
	}
	else
	{
		anim = &ci->animations[ cent->currentState.torsoAnim & ~(ANIM_TOGGLEBIT) ];
	}
	animSpeed = 50.0f / anim->frameLerp;

	// Clear any bone angles

	trap_G2API_SetBoneAngles(cent->ghoul2, 0, "lower_lumbar", vec3_origin, BONE_ANGLES_POSTMULT, POSITIVE_X, NEGATIVE_Y, NEGATIVE_Z, cgs.gameModels, 0, cg.time ); 
	trap_G2API_SetBoneAngles(cent->ghoul2, 0, "upper_lumbar", vec3_origin, BONE_ANGLES_POSTMULT, POSITIVE_X, NEGATIVE_Y, NEGATIVE_Z, cgs.gameModels, 0, cg.time ); 
	trap_G2API_SetBoneAngles(cent->ghoul2, 0, "cranium", vec3_origin, BONE_ANGLES_POSTMULT, POSITIVE_Z, NEGATIVE_Y, POSITIVE_X, cgs.gameModels,0, cg.time ); 

	// Set the death animation
	trap_G2API_SetBoneAnim(cent->ghoul2, 0, "model_root", anim->firstFrame, anim->firstFrame + anim->numFrames, flags, animSpeed, cg.time, -1, 150);
	trap_G2API_SetBoneAnim(cent->ghoul2, 0, "lower_lumbar", anim->firstFrame, anim->firstFrame + anim->numFrames, flags, animSpeed, cg.time, -1, 150);

	// hit location is a bit field and we need to iterate through the bits
	for ( i = 0; (1<<i) < HL_MAX; i++ )
	{
		if (hitLocation & (1<<i))
		{
			CG_ApplyGore(clientNum, cent, i+1, direction);
		}
	}
}

/*
==============
CG_EntityEvent

An entity has an event value
also called by CG_CheckPlayerstateEvents
==============
*/
#define	DEBUGNAME(x) if(cg_debugEvents.integer){Com_Printf(x"\n");}
void CG_EntityEvent( centity_t *cent, vec3_t position ) 
{
	entityState_t	*es;
	int				event;
	vec3_t			dir;
	const char		*s;
	int				clientNum;
	clientInfo_t	*ci;

	es = &cent->currentState;
	event = es->event & ~EV_EVENT_BITS;

	if ( cg_debugEvents.integer ) 
	{
		Com_Printf( "ent:%3i  event:%3i ", es->number, event );
	}

	// Ignore all events until map is done changing
	if ( cg.mMapChange )
	{
		return;
	}

	if ( !event ) 
	{
		DEBUGNAME("ZEROEVENT");
		return;
	}

	clientNum = es->clientNum;
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) 
	{
		clientNum = 0;
	}

	ci = &cgs.clientinfo[ clientNum ];

	switch ( event ) 
	{
		//
		// movement generated events
		//
		case EV_FOOTSTEP:
			DEBUGNAME("EV_FOOTSTEP");
			if (cg_footsteps.integer) 
			{
				trap_S_StartSound (NULL, es->number, CHAN_BODY, trap_MAT_GetSound(MAT_FOOTSTEP_NORMAL, (es->eventParm&MATERIAL_MASK)), 180, 1000 );
			}
			break;

		case EV_FOOTWADE:
			DEBUGNAME("EV_FOOTWADE");
			break;

		case EV_FALL_SHORT:
			DEBUGNAME("EV_FALL_SHORT");
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, trap_MAT_GetSound(MAT_LAND_NORMAL, es->eventParm&MATERIAL_MASK), 150, 900 );
			if ( clientNum == cg.predictedPlayerState.clientNum ) 
			{
				// smooth landing z changes
				cg.landChange = -8;
				cg.landTime = cg.time;
			}
			break;
	
		case EV_FALL_MEDIUM:
			DEBUGNAME("EV_FALL_MEDIUM");
			// use normal pain sound
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, trap_MAT_GetSound(MAT_LAND_PAIN, (es->eventParm>>8)&MATERIAL_MASK), 150, 900 );
			trap_S_StartSound( NULL, es->number, CHAN_VOICE, CG_CustomPlayerSound( es->number, SOUND_PAIN_3 ), -1, -1 );
			if ( clientNum == cg.predictedPlayerState.clientNum ) 
			{
				// smooth landing z changes
				cg.landChange = -16;
				cg.landTime = cg.time;
			}
			break;
	
		case EV_FALL_FAR:
			DEBUGNAME("EV_FALL_FAR");
	
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, trap_MAT_GetSound(MAT_LAND_DEATH, (es->eventParm>>8)&MATERIAL_MASK), -1, -1 );
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, CG_CustomPlayerSound( cent->currentState.number, SOUND_PAIN_2 ), 150, 900 );
		
			// don't play a pain sound right after this
			cent->pe.painTime = cg.time;	
			
			if ( clientNum == cg.predictedPlayerState.clientNum ) 
			{
				// smooth landing z changes
				cg.landChange = -24;
				cg.landTime = cg.time;
			}
			break;

		case EV_STEP_4:
		case EV_STEP_8:
		case EV_STEP_12:
		case EV_STEP_16:		// smooth out step up transitions
		{
			float	oldStep;
			int		delta;
			int		step;

			DEBUGNAME("EV_STEP");

			if ( clientNum != cg.predictedPlayerState.clientNum ) {
				break;
			}
			// if we are interpolating, we don't need to smooth steps
			if ( cg.demoPlayback || (cg.snap->ps.pm_flags & PMF_FOLLOW) ||
				cg_nopredict.integer || cg_synchronousClients.integer ) {
				break;
			}
			// check for stepping up before a previous step is completed
			delta = cg.time - cg.stepTime;
			
			if (delta < STEP_TIME) 
			{
				oldStep = cg.stepChange * (STEP_TIME - delta) / STEP_TIME;
			} 
			else 
			{
				oldStep = 0;
			}

			// add this amount
			step = 4 * (event - EV_STEP_4 + 1 );
			cg.stepChange = oldStep + step;
			
			if ( cg.stepChange > MAX_STEP_CHANGE ) 
			{
				cg.stepChange = MAX_STEP_CHANGE;
			}
			cg.stepTime = cg.time;

			break;
		}

		case EV_JUMP:
			DEBUGNAME("EV_JUMP");
			trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*jump1.wav" ), -1, -1 );
			break;

		case EV_WATER_FOOTSTEP:
			DEBUGNAME("EV_WATER_FOOTSTEP");
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.waterFootstep[rand()%2], -1, -1);
			break;

		case EV_WATER_TOUCH:
			DEBUGNAME("EV_WATER_TOUCH");
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.waterFootstep[rand()%2], -1, -1);
			break;

		case EV_WATER_LAND:
			DEBUGNAME("EV_WATER_LAND");
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.waterJumpIn, -1, -1 );
			break;

		case EV_WATER_CLEAR:
			DEBUGNAME("EV_WATER_CLEAR");
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.waterLeave, -1, -1 );
			break;

		case EV_SWIM:
			DEBUGNAME("EV_SWIM");
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.waterWade[rand()%2], -1, -1 );
			break;

		case EV_ITEM_PICKUP:
		{
			gitem_t		*item;
			int			index;
			qboolean	autoswitch = qfalse;

			DEBUGNAME("EV_ITEM_PICKUP");

			// Dtermine if this item should autoswitch
			autoswitch = (es->eventParm & ITEM_AUTOSWITCHBIT)?qtrue:qfalse;

			// player predicted index
			index = es->eventParm & ~ITEM_AUTOSWITCHBIT;		

			if ( index < 1 || index >= bg_numItems ) 
			{
				break;
			}

			item = &bg_itemlist[ index ];

			if ( item->pickup_sound )
			{
				trap_S_StartSound (NULL, es->number, CHAN_AUTO,	trap_S_RegisterSound( item->pickup_sound ), -1, -1 );
			}

			// show icon and name on status bar
			if ( es->number == cg.snap->ps.clientNum ) 
			{
				if ( cg.predictedPlayerState.pm_type == PM_NORMAL )
				{
					CG_ItemPickup( index, autoswitch );
				}
			}

			break;
		}

		//=================================================================
		//
		// weapon events
		//
		//=================================================================
		case EV_CHANGE_WEAPON_CANCELLED:
		case EV_CHANGE_WEAPON:

			DEBUGNAME("EV_CHANGE_WEAPON");
			
			// Determine whether or not the alt fire popup should show up
			if(es->number==cg.snap->ps.clientNum && cg.weaponMenuUp )
			{
				// done with weapon menu
				cg.weaponMenuUp = qfalse;

				cg.weaponSelect = cg.weaponMenuSelect;
			}

			break;

		case EV_READY_WEAPON:
			DEBUGNAME("EV_READY_WEAPON");
			break;	

		case EV_FIRE_WEAPON:
			DEBUGNAME("EV_FIRE_WEAPON");
			CG_FireWeapon( cent, ATTACK_NORMAL );
			break;

		case EV_ALT_FIRE:
			DEBUGNAME("EV_ALT_FIRE");
			CG_FireWeapon( cent, ATTACK_ALTERNATE );
			break;

		case EV_NOAMMO:
			DEBUGNAME("EV_NOAMMO");

			if(es->number==cg.snap->ps.clientNum)
			{
				CG_OutOfAmmoChange( es->eventParm );
			}
			break;

		case EV_ITEM_POP:
			DEBUGNAME("EV_ITEM_POP");
			break;

		case EV_ITEM_RESPAWN:
			DEBUGNAME("EV_ITEM_RESPAWN");
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.itemRespawnSound, -1, -1 );
			cent->miscTime = cg.time;
			break;

		//=================================================================
		//
		// other events
		//
		//=================================================================

		case EV_PLAYER_TELEPORT_IN:
			DEBUGNAME("EV_PLAYER_TELEPORT_IN");
			if ( cgs.gametypeData->respawnType != RT_NONE )
			{
				trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.respawnSound, -1, -1 );
			}
			break;

		case EV_PLAYER_TELEPORT_OUT:
			DEBUGNAME("EV_PLAYER_TELEPORT_OUT");
			break;

		case EV_GRENADE_BOUNCE:
			DEBUGNAME("EV_GRENADE_BOUNCE");
			if ( rand() & 1 ) 
			{
				trap_S_StartSound (NULL, es->number, CHAN_AUTO, trap_MAT_GetSound(MAT_BOUNCEMETAL_1, (es->eventParm&MATERIAL_MASK)), -1, -1 );
			} 
			else 
			{
				trap_S_StartSound (NULL, es->number, CHAN_AUTO, trap_MAT_GetSound(MAT_BOUNCEMETAL_2, (es->eventParm&MATERIAL_MASK)), -1, -1 );
			}
			break;

		case EV_DESTROY_GHOUL2_INSTANCE:
		{
			centity_t* cent2 = CG_GetEntity (es->eventParm);
			DEBUGNAME("EV_DESTROY_GHOUL2_INSTANCE");
			if ( cent2->ghoul2 && trap_G2_HaveWeGhoul2Models( cent2->ghoul2))
			{
				trap_G2API_CleanGhoul2Models(&(cent2->ghoul2));
			}
			break;
		}
		
	//=================================================================

	//
	// missile impacts
	//

	case EV_MISSILE_HIT:
		DEBUGNAME("EV_MISSILE_HIT");
		ByteToDir( (es->eventParm >> MATERIAL_BITS), dir );
		CG_MissileHitPlayer( es->weapon, position, dir, es->otherEntityNum,
							(cent->currentState.eFlags & EF_ALT_FIRING)?ATTACK_ALTERNATE:ATTACK_NORMAL );
		if ( es->otherEntityNum != cg.snap->ps.clientNum ) 
		{
			// Some missiles - e.g. thrown knives stick in players (for visual effect only).
			CG_HandleStickyMissile(cent,es,dir,es->otherEntityNum);
		}
		break;

	case EV_MISSILE_MISS:
		DEBUGNAME("EV_MISSILE_MISS");
		ByteToDir( (es->eventParm >> MATERIAL_BITS), dir );
		CG_MissileHitWall(es->weapon, position, dir, 
						(es->eventParm & MATERIAL_MASK), (cent->currentState.eFlags & EF_ALT_FIRING)?ATTACK_ALTERNATE:ATTACK_NORMAL );
		break;

	case EV_BULLET_HIT_WALL:
		DEBUGNAME("EV_BULLET_HIT_WALL");
		
		if ( !(cg_antiLag.integer && cg_impactPrediction.integer && es->otherEntityNum == cg.predictedPlayerState.clientNum ) )
		{
			// eventParm contains the direction byte and the material id
			ByteToDir( (es->eventParm >> MATERIAL_BITS), dir );
			
			// time contains the weapon and attack of the shot
			CG_Bullet( es->pos.trBase, es->otherEntityNum, (es->time&0xFF), 
					   dir, ENTITYNUM_WORLD, (es->eventParm & MATERIAL_MASK),
					   ((es->time>>8)&0xFF) );
		}

		break;

	case EV_BULLET_HIT_FLESH:

		DEBUGNAME("EV_BULLET_HIT_FLESH");

		// Play hit sounds for local player
		if ( es->otherEntityNum2 == cg.snap->ps.clientNum )
		{
			if ( cg.snap->ps.stats[STAT_ARMOR] )
			{
				trap_S_StartLocalSound ( cgs.media.armorHitSound[rand()%2], CHAN_AUTO  );
			}
			else
			{
				trap_S_StartLocalSound ( cgs.media.fleshHitSound[rand()%2], CHAN_AUTO  );
			}
		}

#ifdef _SOF2_FLESHIMPACTPREDICTION
		if ( !(cg_antiLag.integer && cg_impactPrediction.integer >= 2 && es->otherEntityNum == cg.predictedPlayerState.clientNum ) )
#endif
		{
			int	fxtype = MATERIAL_FLESH;

			// eventParm contains the direction byte
			ByteToDir( es->eventParm, dir );

			if (cg_lockBlood.integer)
			{
				fxtype = MATERIAL_NONE;
			}

			CG_Bullet( es->pos.trBase, es->otherEntityNum, (es->time&0xFF), dir,  
					   es->otherEntityNum2, fxtype,
					   ((es->time>>8)&0xFF) );

			CG_AddProcGore ( cent );
		}

		break;

	case EV_EXPLOSION_HIT_FLESH:

		DEBUGNAME("EV_EXPLOSION_HIT_FLESH");

		CG_AddProcGore ( cent );
		break;

	case EV_PLAY_EFFECT:
		DEBUGNAME("EV_PLAY_EFFECT");
		if (es->eventParm != -1)
		{
			trap_FX_PlayEffectID(es->eventParm, es->origin, es->angles, -1, -1 );
		}
		break;

	case EV_GENERAL_SOUND:
		DEBUGNAME("EV_GENERAL_SOUND");
		if ( cgs.gameSounds[ es->eventParm ] ) {
			trap_S_StartSound (NULL, es->number, CHAN_VOICE, cgs.gameSounds[ es->eventParm ], -1, -1 );
		} else {
			s = CG_ConfigString( CS_SOUNDS + es->eventParm );
			trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, s ), -1, -1 );
		}
		break;

	case EV_GLOBAL_SOUND:
		if ( cg_soundGlobal.integer )
		{
			DEBUGNAME("EV_GLOBAL_SOUND");
			if ( cgs.gameSounds[ es->eventParm ] ) 
			{
				trap_S_StartLocalSound ( cgs.gameSounds[ es->eventParm ], CHAN_AUTO );
			} 
			else 
			{
				s = CG_ConfigString( CS_SOUNDS + es->eventParm );
				trap_S_StartLocalSound ( CG_CustomSound( es->number, s ), CHAN_AUTO );
			}
		}
		break;

	case EV_ENTITY_SOUND:
		DEBUGNAME("EV_ENTITY_SOUND");
		//somewhat of a hack - weapon is the caller entity's index, trickedentindex is the proper sound channel
		if ( cgs.gameSounds[ es->eventParm ] ) {
			trap_S_StartSound (NULL, es->weapon, 0, cgs.gameSounds[ es->eventParm ], -1, -1 );
		} else {
			s = CG_ConfigString( CS_SOUNDS + es->eventParm );
			trap_S_StartSound (NULL, es->weapon, 0, CG_CustomSound( es->weapon, s ), -1, -1 );
		}
		break;

	case EV_GLASS_SHATTER:
		DEBUGNAME("EV_GLASS_SHATTER");
		CG_GlassShatter(es->number, es->pos.trBase, es->angles, es->origin);
		break;

	case EV_PAIN:
		// local player sounds are triggered in CG_CheckLocalSounds,
		// so ignore events on the player
		DEBUGNAME("EV_PAIN");
		if ( cent->currentState.number != cg.snap->ps.clientNum ) 
		{
			CG_PainEvent( cent, es->eventParm );
		}
		break;

	case EV_PAIN_WATER:
	{
		static drownIndex = 0;
		DEBUGNAME("EV_PAIN_WATER");
		trap_S_StartSound ( NULL, es->number, CHAN_VOICE, cgs.media.drownPainSound[(drownIndex++)%2], -1, -1 );
		break;
	}

	case EV_GAME_OVER:
		DEBUGNAME("EV_OBITUARY");
		CG_GameOver ( es );
		break;

	case EV_GOGGLES:
		DEBUGNAME("EV_GOGGLES");

		// Sound is handled elsewhere for local client
		trap_S_StartSound ( NULL, es->number, CHAN_AUTO, es->eventParm?cgs.media.gogglesOnSound:cgs.media.gogglesOffSound, 120, -1 );
		break;

	case EV_OBITUARY:
		DEBUGNAME("EV_OBITUARY");
		CG_Obituary( es );
		break;

	case EV_STOPLOOPINGSOUND:
		DEBUGNAME("EV_STOPLOOPINGSOUND");
		trap_S_StopLoopingSound( es->number );
		es->loopSound = 0;
		break;

	case EV_DEBUG_LINE:
		DEBUGNAME("EV_DEBUG_LINE");
		CG_Beam( cent );
		break;

	case EV_TESTLINE:
		DEBUGNAME("EV_TESTLINE");
		CG_TestLine(es->origin, es->origin2, 0, es->weapon, 1);
		break;

	case EV_BODY_QUEUE_COPY:
		DEBUGNAME("EV_BODY_QUEUE_COPY");

		// First byte of eventParm is the client Number
		// Second byte of eventParm is the direction of the incoming shot
		// Third and fourth byte of eventParm is the hit location
		ByteToDir( (es->eventParm & 0xFF), dir );

		CG_BodyQueueCopy(cent, es->otherEntityNum, (es->eventParm>>8), dir);
		break;

	case EV_PROC_GORE:
		DEBUGNAME("EV_PROC_GORE");
		CG_AddProcGore(cent);
		break;

	case EV_BOTWAYPOINT:
		DEBUGNAME("EV_BOTWAYPOINT");
		CG_TestLine(cent->lerpOrigin, es->angles, 30000, 0x0000ff, 3);
		//Just render for 30 seconds because this waypoint might not be rendered again for quite some time.
		break;

	case EV_GAMETYPE_RESTART:
		CG_MapRestart ( qtrue );
		break;

	case EV_USE:
		break;

	case EV_WEAPON_CALLBACK:
		DEBUGNAME("EV_WEAPON_CALLBACK");
		if ( cent->currentState.number == cg.snap->ps.clientNum ) 
		{
			CG_WeaponCallback ( &cg.predictedPlayerState,
								&cg_entities[cg.predictedPlayerState.clientNum], 
								(es->eventParm&0xFF),			// Weapon id
								((es->eventParm>>8)&0xFF),		// Anim id
								((es->eventParm>>16)&0xFF),		// Anim choice
								((es->eventParm>>24)&0xFF) );	// Callback step
		}
		break;

	default:
		DEBUGNAME("UNKNOWN");
		Com_Error( ERR_FATAL, "Unknown event: %i", event );
		break;
	}

}

/*
==============
CG_CheckEvents
==============
*/
void CG_CheckEvents( centity_t *cent ) 
{
	// check for event-only entities
	if ( cent->currentState.eType > ET_EVENTS ) 
	{
		// already fired
		if ( cent->previousEvent ) 
		{			
			return;	
		}
		
		// if this is a player event set the entity number of the client entity number
		if ( cent->currentState.eFlags & EF_PLAYER_EVENT )
		{
			cent->currentState.number = cent->currentState.otherEntityNum;
		}

		cent->previousEvent = 1;

		cent->currentState.event = cent->currentState.eType - ET_EVENTS;
	} 
	else 
	{
		// check for events riding with another entity
		if ( cent->currentState.event == cent->previousEvent ) 
		{
			return;
		}

		cent->previousEvent = cent->currentState.event;

		if ( ( cent->currentState.event & ~EV_EVENT_BITS ) == 0 ) 
		{
			return;
		}
	}

	// calculate the position at exactly the frame time
	BG_EvaluateTrajectory( &cent->currentState.pos, cg.snap->serverTime, cent->lerpOrigin );

	CG_SetEntitySoundPosition( cent );

	CG_EntityEvent( cent, cent->lerpOrigin );
}

