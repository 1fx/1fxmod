// Copyright (C) 2001-2002 Raven Software
//
#include "g_local.h"
#include "boe_local.h"

/*

  Items are any object that a player can touch to gain some effect.

  Pickup will return the number of seconds until they should respawn.

  all items should pop when dropped in lava or slime

  Respawnable items don't actually go away when picked up, they are
  just made invisible and untouchable.  This allows them to ride
  movers and respawn apropriately.
*/


#define	RESPAWN_ARMOR		25
#define	RESPAWN_HEALTH		35
#define	RESPAWN_AMMO		40
#define RESPAWN_BACKPACK	40

extern gentity_t *droppedRedFlag;
extern gentity_t *droppedBlueFlag;

//======================================================================
void Add_Ammo (gentity_t *ent, int ammoindex, int count)
{
	ent->client->ps.ammo[ammoindex] += count;
	if ( ent->client->ps.ammo[ammoindex] > ammoData[ammoindex].max ) 
	{
		ent->client->ps.ammo[ammoindex] = ammoData[ammoindex].max;
	}
}

int Pickup_Ammo (gentity_t *ent, gentity_t *other)
{
	int	quantity;

	if ( ent->count ) 
	{
		quantity = ent->count;
	} 
	else 
	{
		quantity = ent->item->quantity;
	}

	Add_Ammo (other, ent->item->giTag, quantity);

	return RESPAWN_AMMO;
}

//======================================================================


int Pickup_Weapon (gentity_t *ent, gentity_t *other, qboolean* autoswitch ) 
{
	int			quantity;
	int			weaponNum = ent->item->giTag;
	qboolean	hasAltAmmo;

	if(current_gametype.value == GT_HS){
		// Henk 26/01/10 -> Notification if people pickup special weapons
		if(other){
			if(weaponNum == WP_M4_ASSAULT_RIFLE){
				G_FreeEntity(&g_entities[level.M4Flare]);
				level.M4ent = -1;
				level.M4Time = 0;
				trap_SendServerCommand(-1, va("print\"^3[H&S] ^7%s has taken the M4.\n\"", other->client->pers.netname));
				Com_sprintf(level.M4loc, sizeof(level.M4loc), "%s", other->client->pers.netname);
			}else if(weaponNum == WP_RPG7_LAUNCHER){
				G_FreeEntity(&g_entities[level.RPGFlare]);
				level.RPGent = -1;
				level.RPGTime = 0;
				trap_SendServerCommand(-1, va("print\"^3[H&S] ^7%s has taken the RPG.\n\"", other->client->pers.netname));
				Com_sprintf(level.RPGloc, sizeof(level.RPGloc), "%s", other->client->pers.netname);
			}else if(weaponNum == WP_MM1_GRENADE_LAUNCHER){
				G_FreeEntity(&g_entities[level.MM1Flare]);
				level.MM1ent = -1;
				level.MM1Time = 0;
				trap_SendServerCommand(-1, va("print\"^3[H&S] ^7%s has taken the MM1.\n\"", other->client->pers.netname));
				Com_sprintf(level.MM1loc, sizeof(level.MM1loc), "%s", other->client->pers.netname);
			}else if(weaponNum == WP_M67_GRENADE){
				trap_SendServerCommand(-1, va("print\"^3[H&S] ^7%s has taken the ? grenade.\n\"", other->client->pers.netname));
				Com_sprintf(level.RandomNadeLoc, sizeof(level.RandomNadeLoc), "%s", other->client->pers.netname);
			}
		}
	}
	hasAltAmmo = BG_WeaponHasAlternateAmmo ( weaponNum );

	if ( ent->count < 0 ) 
	{
		quantity = 0; // None for you, sir!
	} 
	else 
	{		
		if ( ent->count ) 
		{
			quantity = ent->count;
		} 
		else if((weaponNum == WP_RPG7_LAUNCHER || weaponNum == WP_AK74_ASSAULT_RIFLE || weaponNum == WP_MM1_GRENADE_LAUNCHER || weaponNum == WP_MSG90A1) && current_gametype.value == GT_HS){
		// Henk 28/01/10 -> Fixed weapons bug causing M4/RPG to dissapear at pickup
		quantity = 0;
		}
		else 
		{
			quantity = weaponData[weaponNum].attack[ATTACK_NORMAL].clipSize + (weaponData[weaponNum].attack[ATTACK_ALTERNATE].clipSize<<16);
		}
	}

	// Add the ammo in, dont use Add_Ammo because we may temporarily pass the max
	// ammo here and add_ammo wont let us do that.
	other->client->ps.ammo[weaponData[weaponNum].attack[ATTACK_NORMAL].ammoIndex] += (quantity&0xFF);
	other->client->ps.ammo[weaponData[weaponNum].attack[ATTACK_NORMAL].ammoIndex] += ((quantity>>8)&0xFF);

	if ( hasAltAmmo )
	{
		other->client->ps.ammo[weaponData[weaponNum].attack[ATTACK_ALTERNATE].ammoIndex] += ((quantity>>16)&0xFF);
		other->client->ps.ammo[weaponData[weaponNum].attack[ATTACK_ALTERNATE].ammoIndex] += ((quantity>>24)&0xFF);
	}

	// If just picked up move some ammo into the clip
	if ( !(other->client->ps.stats[STAT_WEAPONS]&(1<<weaponNum)) )
	{
		// Copy over the clips
		other->client->ps.clip[ATTACK_NORMAL][weaponNum] = (quantity&0xFF);
		other->client->ps.ammo[weaponData[weaponNum].attack[ATTACK_NORMAL].ammoIndex] -= other->client->ps.clip[ATTACK_NORMAL][weaponNum];

		if ( hasAltAmmo )
		{
			other->client->ps.clip[ATTACK_ALTERNATE][weaponNum] = ((quantity>>16)&0xFF);
			other->client->ps.ammo[weaponData[weaponNum].attack[ATTACK_ALTERNATE].ammoIndex] -= other->client->ps.clip[ATTACK_ALTERNATE][weaponNum];
		}
		
		if ( other->client->ps.weaponstate != WEAPON_CHARGING	  &&
			 other->client->ps.weaponstate != WEAPON_CHARGING_ALT    )
		{
			// Autoswitch the weapon
			*autoswitch = qtrue;
		}

		other->client->ps.firemode[weaponNum] = BG_FindFireMode ( (weapon_t)weaponNum, ATTACK_NORMAL, WP_FIREMODE_AUTO );
	}

	// Call add ammo with 0 ammo to force it to cap it at max
	if(current_gametype.value != GT_HZ)
	Add_Ammo( other, weaponData[weaponNum].attack[ATTACK_NORMAL].ammoIndex, 0 );

	if ( hasAltAmmo )
	{
		Add_Ammo( other, weaponData[weaponNum].attack[ATTACK_ALTERNATE].ammoIndex, 0 );
	}

	// add the weapon
	other->client->ps.stats[STAT_WEAPONS] |= ( 1 << weaponNum );

	if(current_gametype.value == GT_HS){
		// Henkie 26/01/10 -> Dupe fix
		other->client->ps.weapon = WP_KNIFE;
		other->client->ps.weaponstate = WEAPON_READY;
	}else if (current_gametype.value == GT_HZ && ent->item->addAllAmmo){
		int ammoIndex;

		// Boe!Man 11/9/14: H&Z is the only gametype where we immediately add all clips as defined in the weaponfile.
		ammoIndex = weaponData[weaponNum].attack[ATTACK_NORMAL].ammoIndex;
		other->client->ps.ammo[ammoIndex] += weaponData[weaponNum].attack[ATTACK_NORMAL].extraClips * weaponData[weaponNum].attack[ATTACK_NORMAL].clipSize;
		other->client->ps.clip[ATTACK_NORMAL][weaponNum] = weaponData[weaponNum].attack[ATTACK_NORMAL].clipSize;
		other->client->ps.firemode[weaponNum] = BG_FindFireMode(weaponNum, ATTACK_NORMAL, WP_FIREMODE_AUTO);

		ent->item->addAllAmmo = qfalse;
	}

	return g_weaponRespawn.integer;
}


//======================================================================

int Pickup_Health (gentity_t *ent, gentity_t *other) 
{
	int			quantity;

	if ( ent->count ) 
	{
		quantity = ent->count;
	} 
	else 
	{
		quantity = ent->item->quantity;
	}

	other->health += quantity;

	if (other->health > MAX_HEALTH ) 
	{
		other->health = MAX_HEALTH;
	}

	other->client->ps.stats[STAT_HEALTH] = other->health;

	return RESPAWN_HEALTH;
}

//======================================================================

int Pickup_Armor( gentity_t *ent, gentity_t *other ) 
{
	other->client->ps.stats[STAT_ARMOR] += ent->item->quantity;

	if ( other->client->ps.stats[STAT_ARMOR] > MAX_ARMOR ) 
	{
		other->client->ps.stats[STAT_ARMOR] = MAX_ARMOR;
	}

	return RESPAWN_ARMOR;
}

//======================================================================

int Pickup_Gametype (gentity_t *ent, gentity_t *other) 
{
	other->client->ps.stats[STAT_GAMETYPE_ITEMS] |= (1<<ent->item->giTag);

	return -1;
}

int Pickup_Backpack ( gentity_t* ent, gentity_t* other )
{
	float			percent = (float)ent->item->quantity / 100.0f;
	int				i;
	playerState_t	*ps;

	ps = &other->client->ps;

	// Fill up their health
	ps->stats[STAT_HEALTH] += MAX_HEALTH * percent;
	if ( ps->stats[STAT_HEALTH] > MAX_HEALTH )
	{
		ps->stats[STAT_HEALTH] = MAX_HEALTH;
	}
	other->health = ps->stats[STAT_HEALTH];

	// Cant get armor when you have goggles -  Ahh but yes you can ... AND WILL!!
	//RxCxW - 1.30.2005 - #Armor
	if ( !ps->stats[STAT_GOGGLES])
//	if ( !ps->stats[STAT_GOGGLES] )
	//End
	{
		ps->stats[STAT_ARMOR] += MAX_HEALTH * percent;
		if ( ps->stats[STAT_ARMOR] > MAX_ARMOR )
		{
			ps->stats[STAT_ARMOR] = MAX_ARMOR;
		}
	}

	// Give them some ammo
	for ( i = 0; i < MAX_AMMO; i ++ )
	{
		int	maxammo;

		maxammo = BG_GetMaxAmmo ( ps, i);

		if ( !maxammo || ps->ammo[i] >= maxammo )
		{
			continue;
		}

		ps->ammo[i] += Com_Clamp ( 1, maxammo, maxammo * percent );
		if ( ps->ammo[i] >= maxammo )
		{
			ps->ammo[i] = maxammo;
		}
	}

	// Make sure you alwasy get grenades
	if ( level.pickupsDisabled && g_disableNades.integer == 0 && level.nadesFound)
	{
		weapon_t weapon = (weapon_t)ps->stats[STAT_OUTFIT_GRENADE];

		// If the client doesnt even have a greande then we need to give them one
		// and fill their clip.  They should have already been give ammo 
		if ( !(ps->stats[STAT_WEAPONS] & (1<<weapon)) )
		{
			int ammoIndex;

			ps->stats[STAT_WEAPONS] |= (1<<weapon);

			// Move over the ammo to a clip
			ammoIndex = weaponData[weapon].attack[ATTACK_NORMAL].ammoIndex;
			ps->ammo[ammoIndex] -= weaponData[weapon].attack[ATTACK_NORMAL].clipSize;
			ps->clip[ATTACK_NORMAL][weapon] = weaponData[weapon].attack[ATTACK_NORMAL].clipSize;
		}
	}

	return g_backpackRespawn.integer;
}

/*
===============
RespawnItem
===============
*/
void RespawnItem( gentity_t *ent ) 
{
	// randomly select from teamed entities
	if (ent->team) 
	{
		gentity_t	*master;
		int	count;
		int choice;

		if ( !ent->teammaster ) 
		{
			Com_Error( ERR_FATAL, "RespawnItem: bad teammaster");
		}
		master = ent->teammaster;

		for (count = 0, ent = master; ent; ent = ent->teamchain, count++)
			;

		choice = rand() % count;

		for (count = 0, ent = master; count < choice; ent = ent->teamchain, count++)
			;
	}

	ent->r.contents = CONTENTS_TRIGGER;
	ent->s.eFlags &= ~EF_NODRAW;
	ent->r.svFlags &= ~SVF_NOCLIENT;
	trap_LinkEntity (ent);

	// play the normal respawn sound only to nearby clients
	G_AddEvent( ent, EV_ITEM_RESPAWN, 0 );

	ent->nextthink = 0;
}


/*
===============
Touch_Item
===============
*/
void Touch_Item (gentity_t *ent, gentity_t *other, trace_t *trace) 
{
	int			respawn;
	qboolean	predict;
	qboolean	autoswitch;
	int			eventID;

	if (!other->client)
		return;

	// dead people can't pickup
	if (other->health < 1)
		return;	

	#ifdef _3DServer
	// Monkeys can't pickup weapons or items.
	if (other->client->sess.deadMonkey){
		return;
	}
	#endif // _3DServer

	// See if teh item can be picked up
	if( ent->s.eFlags & EF_NOPICKUP )
	{
		return;
	}

	if(current_gametype.value == GT_HS){
		if((ent->item->giTag == WP_MM1_GRENADE_LAUNCHER || ent->item->giTag == WP_F1_GRENADE || ent->item->giTag == WP_MDN11_GRENADE || ent->item->giTag == WP_L2A2_GRENADE) && other->client->sess.team == TEAM_RED){
			return;
		}else if((ent->item->giTag == WP_M4_ASSAULT_RIFLE || ent->item->giTag == WP_RPG7_LAUNCHER || ent->item->giTag == WP_M67_GRENADE) && other->client->sess.team == TEAM_BLUE){
			return;
		}
	}else if(current_gametype.value == GT_HZ){
		if(other->client->sess.team == TEAM_BLUE) // zombies can't pickup anything
			return;
	}

	// If its a gametype item the gametype handles it
	if ( ent->item->giType == IT_GAMETYPE )
	{		
		if(current_gametype.value == GT_HS){
			if(other->client){
				if(other->client->sess.team == TEAM_RED){ // Henkie 24/02/10 -> Hiders cannot pickup briefcase
					return;
				}
			}
		}else{
			// Let the gametype decide if it can be picked up
			if ( !trap_GT_SendEvent ( GTEV_ITEM_TOUCHED, level.time, ent->item->quantity, other->s.number, other->client->sess.team, 0, 0 ) )
			{
				return;
			}
		}
	}
	// the same pickup rules are used for client side and server side

	else if ( !BG_CanItemBeGrabbed( level.gametype, &ent->s, &other->client->ps ) && current_gametype.value != GT_HZ ) 
	{
		return;
	}

#ifdef _DEBUG
	G_LogPrintf( "Item: %i %s\n", other->s.number, ent->item->classname );
#endif

	// Initialize booleans
	predict    = other->client->pers.predictItemPickup;
	autoswitch = qfalse;
	// call the item-specific pickup function
	switch( ent->item->giType ) 
	{
		case IT_WEAPON:
			respawn = Pickup_Weapon(ent, other, &autoswitch );
			break;
		case IT_AMMO:
			respawn = Pickup_Ammo(ent, other);
			break;
		case IT_ARMOR:
			respawn = Pickup_Armor(ent, other);
			break;
		case IT_HEALTH:
			respawn = Pickup_Health(ent, other);
			break;
		case IT_GAMETYPE:
			if(current_gametype.value == GT_HS){
				trap_SendServerCommand(-1, va("print\"^3[H&S] ^7%s ^7has taken the briefcase.\n\"", other->client->pers.netname));
			}
			respawn = Pickup_Gametype(ent, other);
			predict = qfalse;
			break;
		case IT_BACKPACK:
			respawn = Pickup_Backpack(ent,other);
			predict = qfalse;
			break;
		default:
			return;
	}

	if ( !respawn ) 
	{
		return;
	}

	eventID = EV_ITEM_PICKUP;
	#ifdef _GOLD
	if (other->client && (other->client->ps.pm_flags & PMF_DUCKED))
	{
		eventID = EV_ITEM_PICKUP_QUIET;
	}
	#endif // _GOLD

	// play the normal pickup sound
	if (predict) 
	{
		G_AddPredictableEvent(other, eventID, ent->s.modelindex | (autoswitch ? ITEM_AUTOSWITCHBIT : 0));
	} 
	else 
	{
		G_AddEvent(other, eventID, ent->s.modelindex | (autoswitch ? ITEM_AUTOSWITCHBIT : 0));
	}

	// fire item targets
	G_UseTargets (ent, other);

	// wait of -1 will not respawn
	if ( ent->wait == -1 ) 
	{
		ent->r.svFlags |= SVF_NOCLIENT;
		ent->s.eFlags |= EF_NODRAW;
		ent->r.contents = 0;
		ent->unlinkAfterEvent = qtrue;
		return;
	}

	// non zero wait overrides respawn time
	if ( ent->wait ) 
	{
		respawn = ent->wait;
	}

	// random can be used to vary the respawn time
	if ( ent->random ) 
	{
		respawn += crandom() * ent->random;
		
		if ( respawn < 1 ) 
		{
			respawn = 1;
		}
	}

	// dropped items will not respawn
	if ( ent->flags & FL_DROPPED_ITEM ) 
	{
		ent->freeAfterEvent = qtrue;
	}

	// picked up items still stay around, they just don't
	// draw anything.  This allows respawnable items
	// to be placed on movers.
	ent->r.svFlags |= SVF_NOCLIENT;
	ent->s.eFlags |= EF_NODRAW;
	ent->r.contents = 0;

	// ZOID
	// A negative respawn times means to never respawn this item (but don't 
	// delete it).  This is used by items that are respawned by third party events
	if ( respawn <= 0 ) 
	{
		ent->nextthink = 0;
		ent->think = 0;
	} 
	else 
	{
		ent->nextthink = level.time + respawn * 1000;
		ent->think = RespawnItem;
	}

	trap_LinkEntity( ent );
}


//======================================================================

/*
================
LaunchItem

Spawns an item and tosses it forward
================
*/
gentity_t *LaunchItem( gitem_t *item, vec3_t origin, vec3_t velocity ) 
{
	gentity_t	*dropped;
	// Gametype items must be spawned using the spawn mission item function
	if ( item->giType == IT_GAMETYPE )
	{
		dropped = G_SpawnGametypeItem ( item->pickup_name, qtrue );
		dropped->nextthink = 0;
	}
	else
	{
		dropped = G_Spawn();
		dropped->think = G_FreeEntity;
		dropped->nextthink = level.time + 30000;
	}

	if ( !dropped )
	{
		return NULL;
	}

	dropped->s.eType = ET_ITEM;
	dropped->s.modelindex = item - bg_itemlist;	// store item number in modelindex
	dropped->s.modelindex2 = 1; // This is non-zero is it's a dropped item

	dropped->classname = item->classname;
	dropped->item = item;
	VectorSet (dropped->r.mins, -ITEM_RADIUS, -ITEM_RADIUS, -ITEM_RADIUS);
	VectorSet (dropped->r.maxs, ITEM_RADIUS, ITEM_RADIUS, ITEM_RADIUS);
	dropped->r.contents = CONTENTS_TRIGGER;

	dropped->touch = Touch_Item;

	G_SetOrigin( dropped, origin );
	dropped->s.pos.trType = TR_GRAVITY;
	dropped->s.pos.trTime = level.time;
	VectorCopy( velocity, dropped->s.pos.trDelta );

	dropped->s.eFlags |= EF_BOUNCE_HALF;

	dropped->flags = FL_DROPPED_ITEM;

	trap_LinkEntity (dropped);

	return dropped;
}

/*
================
G_DropItem

Spawns an item and tosses it forward
================
*/
gentity_t *G_DropItem( gentity_t *ent, gitem_t *item, float angle ) 
{
	vec3_t		velocity;
	vec3_t		angles;
	gentity_t*	dropped;
	gentity_t  *test;
	char		location[128] = "\0";

	VectorCopy( ent->s.apos.trBase, angles );
	angles[YAW] += angle;
	angles[PITCH] = 0;	// always forward

	AngleVectors( angles, velocity, NULL, NULL );
	VectorScale( velocity, 150, velocity );
	velocity[2] += 200 + crandom() * 50;
	
	if (item->giType != IT_GAMETYPE || !(current_gametype.value == GT_INF && g_caserun.integer)){
		dropped = LaunchItem(item, ent->r.currentOrigin, velocity);
	}

	if ( item->giType == IT_GAMETYPE )
	{
		// Boe!Man 8/19/11: Add drop location message for all major gametypes (using a simplified system). Do note that this should remain disabled during the scrim itself (public/match warmup are allowed).
		if (g_dropLocationMessage.integer >= 1 && cm_enabled.integer <= 1 && !(current_gametype.value == GT_INF && g_caserun.integer)){
			test = Team_GetLocation(dropped);
			if(test){
				strncpy(location, va(" at %s", test->message), sizeof(location));
			}
		}

		if(current_gametype.value == GT_HS){
			trap_GT_SendEvent ( GTEV_ITEM_DROPPED, level.time, level.gametypeItems[item->giTag].id, ent->s.number, 0, 0, 0 );
			trap_SendServerCommand(-1, va("print\"^3[H&S] %s ^7has dropped the briefcase%s.\n\"", ent->client->pers.netname, location)); // Henkie 24/02/10 -> Add drop briefcase msg
		}else{
			if(current_gametype.value == GT_CTF){
				if(item->quantity == 101){ // Blue flag.
					trap_SendServerCommand( -1, va("print \"^3[CTF] %s ^7has dropped the Blue Flag%s.\n\"", ent->client->pers.netname, location));
				}
				if(item->quantity == 100){ // Red flag.
					trap_SendServerCommand( -1, va("print \"^3[CTF] %s ^7has dropped the Red Flag%s.\n\"", ent->client->pers.netname, location));
				}
			}else if(current_gametype.value == GT_INF){
				if(item->quantity == 100){ // Briefcase.
					if (!g_caserun.integer){
						trap_SendServerCommand(-1, va("print \"^3[INF] %s ^7has dropped the briefcase%s.\n\"", ent->client->pers.netname, location));
					}else{
						// Boe!Man 12/2/14: Let the gametype handle the respawn.
						if (trap_GT_SendEvent(GTEV_ITEM_STUCK, level.time, item->quantity, 0, 0, 0, 0)){
							// just reset the gametype item
						}
						else if (!trap_GT_SendEvent(GTEV_ITEM_STUCK, level.time, item->quantity, 0, 0, 0, 0))
						{
							G_ResetGametypeItem(item);
						}

						ent->client->caserunHoldTime = 0;
						return NULL;
					}
				}
			}
			trap_GT_SendEvent ( GTEV_ITEM_DROPPED, level.time, item->quantity, ent->s.number, 0, 0, 0 );
		}
	}
	
	return dropped;
}

// Henk 26/01/10 -> Bit different from G_DropItem(), used for spawning RPG/M4.
gentity_t *G_DropItem2( vec3_t Origin, vec3_t angles, gitem_t *item) 
{
	vec3_t		velocity;
	gentity_t*	dropped;

	AngleVectors( angles, velocity, NULL, NULL );
	VectorScale( velocity, 150, velocity );
	velocity[2] += 200 + crandom() * 50;
	
	dropped = LaunchItem( item, Origin, velocity );
	dropped->nextthink = 0;	
	return dropped;
}

/*
=================
G_EnablePickup

Re-enables pickup on a entity that has it disabled
=================
*/
void G_EnablePickup ( gentity_t* ent )
{
	ent->s.eFlags &= ~EF_NOPICKUP;

	// If this is a no respawn game we can keep the weapons around till round end
	if ( level.gametypeData->respawnType == RT_NONE )
	{
		return;
	}

	// Go away in 30 seconds
	ent->think = G_FreeEntity;
	ent->nextthink = level.time + 30000;
}

/*
=================
G_IsAmmoBeingShared

determins if the given ammo index is being shared by any other weapon in the players
inventory.  Excluding the specified weapon
=================
*/
qboolean G_IsAmmoBeingShared ( gentity_t* ent, int ammoIndex, weapon_t exclude )
{
	int weapon;

	// Need to figure out if this guy has any other guns that use the ammo for the gun
	// being dropped.
	for ( weapon = WP_KNIFE + 1; weapon < WP_NUM_WEAPONS; weapon ++ )
	{
		// Does the player have this weapon?
		if ( !(ent->client->ps.stats[STAT_WEAPONS] & (1<<weapon) ) )
		{
			continue;
		}

		// Dont include the weapon being dropped
		if ( exclude == (weapon_t)weapon )
		{
			continue;
		}

		// Does this attack use the specified ammo?
		if ( weaponData[weapon].attack[ATTACK_NORMAL].ammoIndex == ammoIndex )
		{
			return qtrue;
		}

		// Does this attack use the specified ammo?
		if ( weaponData[weapon].attack[ATTACK_ALTERNATE].ammoIndex == ammoIndex )
		{
			return qtrue;
		}
	}

	return qfalse;
}

/*
=================
G_DropWeapon

Drops the weapon and all its ammo
=================
*/
gentity_t* G_DropWeapon ( gentity_t* ent, weapon_t weapon, int pickupDelay )
{
	gentity_t*	dropped;
	gitem_t*	item;
	vec3_t		angles;
	// Henk 26/01/10 -> Store location of dropped weapons
	char		location[64];
	qboolean	noloc = qfalse;

	if ( weapon <= WP_KNIFE || weapon >= WP_NUM_WEAPONS )
	{
		return NULL;
	}

	// Make sure they have the weapon
	if ( !(ent->client->ps.stats[STAT_WEAPONS] & (1<<weapon) ) )
	{
		return NULL;
	}

	// No more outfitting changes
	ent->client->noOutfittingChange = qtrue;

	// find the item type for this weapon
	item = BG_FindWeaponItem ( weapon );

	// spawn the item
	dropped = G_DropItem( ent, item, 0 );
	if (dropped == NULL){
		return NULL;
	}

	// Pack all the ammo into the count field
	dropped->s.angles[YAW] = rand()%360;
	dropped->count  = (ent->client->ps.clip[ATTACK_NORMAL][weapon]&0xFF);

	// If the ammo isnt being shared then send it all with the gun
	if ( !G_IsAmmoBeingShared ( ent, weaponData[weapon].attack[ATTACK_NORMAL].ammoIndex, weapon ) )
	{
		dropped->count += ((ent->client->ps.ammo[weaponData[weapon].attack[ATTACK_NORMAL].ammoIndex]<<8) & 0xFF00);
		ent->client->ps.ammo[weaponData[weapon].attack[ATTACK_NORMAL].ammoIndex] = 0;
	}

	// Dont bother if the weapon doesnt have alternate ammo
	if ( BG_WeaponHasAlternateAmmo ( weapon ) )
	{
		dropped->count += ((ent->client->ps.clip[ATTACK_ALTERNATE][weapon] << 16) & 0xFF0000 );	

		// If the ammo isnt being shared then send it all with the gun
		if ( !G_IsAmmoBeingShared ( ent, weaponData[weapon].attack[ATTACK_ALTERNATE].ammoIndex, weapon ) )
		{
			dropped->count += ((ent->client->ps.ammo[weaponData[weapon].attack[ATTACK_ALTERNATE].ammoIndex] << 24) & 0xFF000000 );
			ent->client->ps.ammo[weaponData[weapon].attack[ATTACK_ALTERNATE].ammoIndex] = 0;
		}
	}

	// Clear the clips
	ent->client->ps.clip[ATTACK_NORMAL][weapon] = 0;
	ent->client->ps.clip[ATTACK_ALTERNATE][weapon] = 0;	

	// Take the weapon away
	ent->client->ps.stats[STAT_WEAPONS] &= ~(1<<weapon);

	// if the gun is empty then just kill it soon after its dropped
	if ( !dropped->count && current_gametype.value != GT_HS)
	{
		dropped->nextthink = level.time + 2500;
		dropped->think     = G_FreeEntity;
		dropped->s.eFlags |= EF_NOPICKUP;
	}else if ( !dropped->count && weapon == WP_M4_ASSAULT_RIFLE && current_gametype.value == GT_HS){
		// Henk 26/01/10 -> Just leave empty weapons on the ground, only m4 is useless without ammo so remove it.
		dropped->nextthink = level.time;
		dropped->think     = G_FreeEntity;
		dropped->s.eFlags |= EF_NOPICKUP;
		Com_sprintf(level.M4loc, sizeof(level.M4loc), "%s", "Disappeared");
		trap_SendServerCommand(-1, va("print\"^3[H&S] ^7M4 has disappeared\n\""));
		
		return NULL;
	}
	// Dont allow the item to be picked up againt for 3 seconds if in a no pickup game, otherwise
	// let them pick it up immediately
	else if ( pickupDelay )
	{
		dropped->nextthink = level.time + 1000;	// Henk 18/01/11 -> Lower the delay
		dropped->s.eFlags |= EF_NOPICKUP;
		dropped->think = G_EnablePickup;
	}
	// Always need a tad bit of delay on pickup for prediction issues
	else
	{
		dropped->nextthink = level.time + 200;	
		dropped->s.eFlags |= EF_NOPICKUP;
		dropped->think = G_EnablePickup;
	}

	// Throw the gun forward
	VectorCopy( ent->s.apos.trBase, angles );
	angles[PITCH] = 0;

	// Some random velocity
	AngleVectors( angles, ent->s.pos.trDelta, NULL, NULL );
	VectorScale( ent->s.pos.trDelta, 150, ent->s.pos.trDelta );
	ent->s.pos.trDelta[2] += 200 + random() * 50;

	if(current_gametype.value == GT_HS){
		// Henk 26/01/10 -> Show people that special weapons have been dropped and on what location.
		// Boe!Man 8/28/11: Show "Dropped" instead of nothing and no location in the message when there's no valid location found.
		if(!Team_GetLocationMsg(dropped, location, sizeof(location))){
			noloc = qtrue;
			strncpy(location, "Dropped", sizeof(location));
		}
		if(weapon == WP_RPG7_LAUNCHER){
			Com_sprintf(level.RPGloc, sizeof(level.RPGloc), "%s", location);
			level.RPGent = dropped->s.number;
			level.RPGTime = level.time+500;
			if(!noloc){
				trap_SendServerCommand(-1, va("print\"^3[H&S] ^7%s has dropped the RPG at %s.\n\"", ent->client->pers.netname, level.RPGloc));
			}else{
				trap_SendServerCommand(-1, va("print\"^3[H&S] ^7%s has dropped the RPG.\n\"", ent->client->pers.netname));
			}
		}else if(weapon == WP_M4_ASSAULT_RIFLE){
			Com_sprintf(level.M4loc, sizeof(level.M4loc), "%s", location);
			level.M4ent = dropped->s.number;
			level.M4Time = level.time+500;
			if(!noloc){
				trap_SendServerCommand(-1, va("print\"^3[H&S] ^7%s has dropped the M4 at %s.\n\"", ent->client->pers.netname, level.M4loc));
			}else{
				trap_SendServerCommand(-1, va("print\"^3[H&S] ^7%s has dropped the M4.\n\"", ent->client->pers.netname));
			}
		}else if(weapon == WP_MM1_GRENADE_LAUNCHER){
			Com_sprintf(level.MM1loc, sizeof(level.MM1loc), "%s", location);
			level.MM1ent = dropped->s.number;
			level.MM1Time = level.time+500;
			if(!noloc){
				trap_SendServerCommand(-1, va("print\"^3[H&S] ^7%s has dropped the MM1 at %s.\n\"", ent->client->pers.netname, level.MM1loc));
			}else{
				trap_SendServerCommand(-1, va("print\"^3[H&S] ^7%s has dropped the MM1.\n\"", ent->client->pers.netname));
			}
		}else if(weapon == WP_M67_GRENADE){
			Com_sprintf(level.RandomNadeLoc, sizeof(level.RandomNadeLoc), "%s", location);
			if(!noloc){
				trap_SendServerCommand(-1, va("print\"^3[H&S] ^7%s has dropped the ? grenade at %s.\n\"", ent->client->pers.netname, location));
			}else{
				trap_SendServerCommand(-1, va("print\"^3[H&S] ^7%s has dropped the ? grenade.\n\"", ent->client->pers.netname));
			}
		}
		ent->client->ps.weapon = WP_KNIFE;
		ent->client->ps.weaponstate = WEAPON_READY;
	}
	
	return dropped;
}

/*
================
Use_Item

Respawn the item
================
*/
void Use_Item( gentity_t *ent, gentity_t *other, gentity_t *activator ) 
{
	RespawnItem( ent );
}

//======================================================================

/*
================
FinishSpawningItem

Traces down to find where an item should rest, instead of letting them
free fall from their spawn points
================
*/
void FinishSpawningItem( gentity_t *ent ) 
{
	trace_t		tr;
	vec3_t		dest;
	vec3_t		src;
//	gitem_t		*item;

	VectorSet( ent->r.mins, -ITEM_RADIUS, -ITEM_RADIUS, -ITEM_RADIUS );
	VectorSet( ent->r.maxs, ITEM_RADIUS, ITEM_RADIUS, ITEM_RADIUS );

	ent->s.eType = ET_ITEM;
	ent->s.modelindex = ent->item - bg_itemlist;		// store item number in modelindex
	ent->s.modelindex2 = 0; // zero indicates this isn't a dropped item

	ent->r.contents = CONTENTS_TRIGGER;
	ent->touch = Touch_Item;
	// useing an item causes it to respawn
	ent->use = Use_Item;

	// create a Ghoul2 model if the world model is a glm
/*	item = &bg_itemlist[ ent->s.modelindex ];
	if (!stricmp(&item->world_model[0][strlen(item->world_model[0]) - 4], ".glm"))
	{
		trap_G2API_InitGhoul2Model(&ent->s, item->world_model[0], G_ModelIndex(item->world_model[0] ), 0, 0, 0, 0);
		ent->s.radius = 60;
	}
*/
	if ( ent->item->giType != IT_GAMETYPE && ent->spawnflags & 1 ) 
	{
		// suspended
		G_SetOrigin( ent, ent->s.origin );
	} 
	else 
	{
		// drop to floor
		VectorSet( src, ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] + 1 );
		VectorSet( dest, ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] - 4096 );
		trap_Trace( &tr, src, ent->r.mins, ent->r.maxs, dest, ent->s.number, MASK_SOLID );
		if ( tr.startsolid ) 
		{
			Com_Printf ("FinishSpawningItem: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
			G_FreeEntity( ent );
			return;
		}

		// allow to ride movers
		ent->s.groundEntityNum = tr.entityNum;

		G_SetOrigin( ent, tr.endpos );
	}

	// team slaves and targeted items aren't present at start
	if ( ( ent->flags & FL_TEAMSLAVE ) || ent->targetname ) {
		ent->s.eFlags |= EF_NODRAW;
		ent->r.contents = 0;
		return;
	}

	trap_LinkEntity (ent);
}


qboolean	itemRegistered[MAX_ITEMS];

/*
==============
ClearRegisteredItems
==============
*/
void ClearRegisteredItems( void ) 
{
	memset( itemRegistered, 0, sizeof( itemRegistered ) );

	// players always start with the base weapon
	RegisterItem( BG_FindWeaponItem ( WP_FIRST_RANGED_WEAPON ) );
	RegisterItem( BG_FindWeaponItem ( WP_KNIFE ) );
}

/*
===============
RegisterItem

The item will be added to the precache list
===============
*/
void RegisterItem( gitem_t *item ) 
{
	if ( !item ) 
	{
		Com_Printf( "RegisterItem: NULL" );
	}

	itemRegistered[ item - bg_itemlist ] = qtrue;
}

/*
===============
SaveRegisteredItems

Write the needed items to a config string
so the client will know which ones to precache
===============
*/
void SaveRegisteredItems( void ) 
{
	char	string[MAX_ITEMS+1];
	int		i;
	int		count;

	count = 0;
	for ( i = 0 ; i < bg_numItems ; i++ ) 
	{
		if ( itemRegistered[i] ) 
		{
			count++;
			string[i] = '1';
		}
		else 
		{
			string[i] = '0';
		}
	}
	
	string[ bg_numItems ] = 0;

	Com_Printf( "%i items registered\n", count );
	trap_SetConfigstring(CS_ITEMS, string);
}

/*
============
G_ItemDisabled
============
*/
int G_ItemDisabled( gitem_t *item ) 
{
	char name[128];
	Com_sprintf(name, sizeof(name), "disable_%s", item->classname);
	return trap_Cvar_VariableIntegerValue( name );
}

/*
============
G_SpawnItem

Sets the clipping size and plants the object on the floor.

Items can't be immediately dropped to floor, because they might
be on an entity that hasn't spawned yet.
============
*/
void G_SpawnItem (gentity_t *ent, gitem_t *item) 
{
	// Weapons can be disabled
	if ( item->giType == IT_WEAPON )
	{
		if ( !BG_IsWeaponAvailableForOutfitting ( (weapon_t)item->giTag, 1 ) )
		{
			return;
		}
	}	

	G_SpawnFloat( "random", "0", &ent->random );
	G_SpawnFloat( "wait", "0", &ent->wait );

	RegisterItem( item );
	if ( G_ItemDisabled(item) )
		return;

	ent->item = item;
	// some movers spawn on the second frame, so delay item
	// spawns until the third frame so they can ride trains
	ent->nextthink = level.time + FRAMETIME * 2;
	ent->think = FinishSpawningItem;

	ent->physicsBounce = 0.50;		// items are bouncy
}


/*
================
G_BounceItem

================
*/
void G_BounceItem( gentity_t *ent, trace_t *trace ) {
	vec3_t	velocity;
	float	dot;
	int		hitTime;

	// reflect the velocity on the trace plane
	hitTime = level.previousTime + ( level.time - level.previousTime ) * trace->fraction;
	BG_EvaluateTrajectoryDelta( &ent->s.pos, hitTime, velocity );
	dot = DotProduct( velocity, trace->plane.normal );
	VectorMA( velocity, -2*dot, trace->plane.normal, ent->s.pos.trDelta );

	// cut the velocity to keep from bouncing forever
	VectorScale( ent->s.pos.trDelta, ent->physicsBounce, ent->s.pos.trDelta );

	// check for stop
	if ( trace->plane.normal[2] > 0 && ent->s.pos.trDelta[2] < 40 ) {
		trace->endpos[2] += 1.0;	// make sure it is off ground
		SnapVector( trace->endpos );
		G_SetOrigin( ent, trace->endpos );
		ent->s.groundEntityNum = trace->entityNum;
		return;
	}

	VectorAdd( ent->r.currentOrigin, trace->plane.normal, ent->r.currentOrigin);
	VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
	ent->s.pos.trTime = level.time;
}


/*
================
G_RunItem
================
*/
void G_RunItem( gentity_t *ent ) 
{
	vec3_t		origin;
	trace_t		tr;
	int			contents;
	int			mask;

	// if groundentity has been set to -1, it may have been pushed off an edge
	if ( ent->s.groundEntityNum == -1 ) 
	{
		if ( ent->s.pos.trType != TR_GRAVITY ) 
		{
			ent->s.pos.trType = TR_GRAVITY;
			ent->s.pos.trTime = level.time;
		}
	}

	if ( ent->s.pos.trType == TR_STATIONARY ) 
	{
		G_RunThink( ent );
		return;
	}

	// get current position
	BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );

	// trace a line from the previous position to the current position
	if ( ent->clipmask ) 
	{
		mask = ent->clipmask;
	} 
	else 
	{
		mask = MASK_PLAYERSOLID & ~CONTENTS_BODY;//MASK_SOLID;
	}

	trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, ent->r.ownerNum, mask );

	VectorCopy( tr.endpos, ent->r.currentOrigin );

	if ( tr.startsolid ) 
	{
		tr.fraction = 0;
	}

	// FIXME: avoid this for stationary?
	trap_LinkEntity( ent );	

	// check think function
	G_RunThink( ent );

	if ( tr.fraction == 1 ) 
	{
		return;
	}

	// if it is in a nodrop volume, remove it
	contents = trap_PointContents( ent->r.currentOrigin, -1 );
	if ( contents & CONTENTS_NODROP ) 
	{
		if(current_gametype.value == GT_HS && ent->item){
			if(ent->item->giTag == WP_RPG7_LAUNCHER){
				level.RPGTime = 0;
				Com_sprintf(level.RPGloc, sizeof(level.RPGloc), "Disappeared");
				trap_SendServerCommand (-1, va("print\"^3[H&S] ^7RPG has disappeared.\n\""));
			}else if(ent->item->giTag == WP_M4_ASSAULT_RIFLE){
				level.M4Time = 0;
				Com_sprintf(level.M4loc, sizeof(level.M4loc), "Disappeared");
				trap_SendServerCommand (-1, va("print\"^3[H&S] ^7M4 has disappeared.\n\""));
			}else if(ent->item->giTag == WP_MM1_GRENADE_LAUNCHER){
				level.MM1Time = 0;
				Com_sprintf(level.MM1loc, sizeof(level.MM1loc), "Disappeared");
				trap_SendServerCommand (-1, va("print\"^3[H&S] ^7MM1 has disappeared.\n\""));
			}
		}
		// Gametype items are reported to the gametype when they are stuck like this
		if ( ent->item && ent->item->giType == IT_GAMETYPE)
		{
			// Let the gametype handle the problem, if it doenst handle it and return 1 then 
			if ( trap_GT_SendEvent ( GTEV_ITEM_STUCK, level.time, ent->item->quantity, 0, 0, 0, 0 ) ){
			// just reset the gametype item
			}
			else if ( !trap_GT_SendEvent ( GTEV_ITEM_STUCK, level.time, ent->item->quantity, 0, 0, 0, 0 ) )
			{
				G_ResetGametypeItem ( ent->item );
			}
		}

		G_FreeEntity( ent );
		return;
	}

	G_BounceItem( ent, &tr );
}

gentity_t *CreateWeaponPickup(vec3_t pos,weapon_t weapon)
{
	gentity_t	*dropped;
	gitem_t		*item;

	if ( weapon < WP_KNIFE || weapon >= WP_NUM_WEAPONS )
	{
		return 0;
	}

	item = BG_FindWeaponItem (weapon);
	if(!item)
	{
		return(0);
	}

	dropped = G_Spawn();
	dropped->s.eType = ET_ITEM;
	dropped->s.modelindex = item - bg_itemlist;	// store item number in modelindex
	dropped->s.modelindex2 = 1;					// This is non-zero is it's a dropped item

	dropped->classname = item->classname;
	dropped->item = item;
	VectorSet (dropped->r.mins, -ITEM_RADIUS, -ITEM_RADIUS, -ITEM_RADIUS);
	VectorSet (dropped->r.maxs, ITEM_RADIUS, ITEM_RADIUS, ITEM_RADIUS);
	dropped->r.contents = CONTENTS_TRIGGER;

	dropped->touch = Touch_Item;

	G_SetOrigin( dropped, pos );
	dropped->s.pos.trType = TR_GRAVITY;
	dropped->s.pos.trTime = level.time;
	VectorSet( dropped->s.pos.trDelta,0.0,0.0,0.0);

	dropped->s.eFlags |= EF_BOUNCE_HALF;
	dropped->flags = FL_DROPPED_ITEM;

	trap_LinkEntity (dropped);

	return dropped;
}
