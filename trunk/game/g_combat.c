// Copyright (C) 2001-2002 Raven Software.
//
// g_combat.c

#include "g_local.h"

void BotDamageNotification	( gclient_t *bot, gentity_t *attacker );

/*
============
G_AddScore

Adds score to both the client and his team
============
*/
void G_AddScore( gentity_t *ent, int score ) 
{
	if ( !ent->client ) 
	{
		return;
	}
	
	// no scoring during pre-match warmup
	if ( level.warmupTime ) 
	{
		return;
	}

	ent->client->sess.score += score;
	ent->client->ps.persistant[PERS_SCORE] = ent->client->sess.score;

	CalculateRanks();
}

/*
=================
TossClientItems

Toss the weapon and custom gametype items for the killed player
=================
*/
void TossClientItems( gentity_t *self ) 
{
	gitem_t		*item;
	int			weapon;
	float		angle;
	int			i;
	gentity_t	*drop;

	// drop the weapon if not a gauntlet or machinegun
	weapon = self->s.weapon;

	// make a special check to see if they are changing to a new
	// weapon that isn't the mg or gauntlet.  Without this, a client
	// can pick up a weapon, be killed, and not drop the weapon because
	// their weapon change hasn't completed yet and they are still holding the MG.
	if ( self->client->ps.weaponstate == WEAPON_DROPPING ) 
	{
		weapon = self->client->pers.cmd.weapon;
	}
		
	if ( !( self->client->ps.stats[STAT_WEAPONS] & ( 1 << weapon ) ) ) 
	{
		weapon = WP_NONE;
	}
	
	// If we have a valid weapon to drop and it has ammo then drop it
	if ( weapon > WP_KNIFE && weapon < WP_NUM_WEAPONS &&
		 (self->client->ps.ammo[ weaponData[weapon].attack[ATTACK_NORMAL].ammoIndex ] + self->client->ps.clip[weapon]) ) 
	{
		G_DropWeapon ( self, weapon, 0 );
	}

	// drop all custom gametype items
	angle = 45;
	for ( i = 0 ; i < MAX_GAMETYPE_ITEMS ; i++ ) 
	{
		// skip this gametype item if the client doenst have it
		if ( !(self->client->ps.stats[STAT_GAMETYPE_ITEMS] & (1<<i)) ) 
		{
			continue;
		}

		item = BG_FindGametypeItem ( i );
		if ( !item ) 
		{
			continue;
		}

		drop = G_DropItem( self, item, angle );
		drop->count = 1;
		angle += 45;
		
		// TAke it away from the client just in case
		self->client->ps.stats[STAT_GAMETYPE_ITEMS] &= ~(1<<i);

		if ( self->enemy && self->enemy->client && !OnSameTeam ( self->enemy, self ) )
		{
			trap_GT_SendEvent ( GTEV_ITEM_DEFEND, level.time, item->quantity, self->enemy->s.clientNum, self->enemy->client->sess.team, 0, 0  );
		}
	}

	self->client->ps.stats[STAT_GAMETYPE_ITEMS] = 0;
}

/*
==================
LookAtKiller
==================
*/
void LookAtKiller( gentity_t *self, gentity_t *inflictor, gentity_t *attacker ) {
	vec3_t		dir;
	vec3_t		angles;

	if ( attacker && attacker != self ) {
		VectorSubtract (attacker->s.pos.trBase, self->s.pos.trBase, dir);
	} else if ( inflictor && inflictor != self ) {
		VectorSubtract (inflictor->s.pos.trBase, self->s.pos.trBase, dir);
	} else {
		self->client->ps.stats[STAT_DEAD_YAW] = self->s.angles[YAW];
		return;
	}

	self->client->ps.stats[STAT_DEAD_YAW] = vectoyaw ( dir );

	angles[YAW] = vectoyaw ( dir );
	angles[PITCH] = 0; 
	angles[ROLL] = 0;
}

/*
==================
body_die
==================
*/
void body_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath, int hitLocation, vec3_t hitDir ) 
{
	return;
}


// these are just for logging, the client prints its own messages
char *modNames[] = 
{
	"MOD_UNKNOWN",

	"MOD_KNIFE",

	"MOD_M1911A1_PISTOL",
	"MOD_US_SOCOM_PISTOL",         

	"MOD_M590_SHOTGUN",            
	"MOD_MICRO_UZI_SUBMACHINEGUN", 
	"MOD_M3A1_SUBMACHINEGUN",      

	"MOD_USAS_12_SHOTGUN",         
	"MOD_M4_ASSAULT_RIFLE",        
	"MOD_AK74_ASSAULT_RIFLE",      
	"MOD_MSG90A1_SNIPER_RIFLE",    
	"MOD_M60_MACHINEGUN",          
	"MOD_MM1_GRENADE_LAUNCHER",    
	"MOD_RPG7_LAUNCHER",           

	"MOD_M67_GRENADE",             
	"MOD_M84_GRENADE",
	"MOD_F1_GRENADE",
	"MOD_L2A2_GRENADE",
	"MOD_MDN11_GRENADE",
	"MOD_SMOHG92_GRENADE",
	"MOD_ANM14_GRENADE",
	"MOD_M15_GRENADE",

	"MOD_WATER",
	"MOD_CRUSH",
	"MOD_TELEFRAG",
	"MOD_FALLING",
	"MOD_SUICIDE",
	"MOD_TEAMCHANGE",
	"MOD_TARGET_LASER",
	"MOD_TRIGGER_HURT"
};

/*
==================
player_die
==================
*/
void player_die( 
	gentity_t	*self, 
	gentity_t	*inflictor, 
	gentity_t	*attacker, 
	int			damage, 
	int			mod, 
	int			hitLocation,
	vec3_t		hitDir
	) 
{
	gentity_t		*ent;
	int				anim;
	int				contents;
	int				killer;
	int				i;
	char			*killerName, *obit;
	attackType_t	attack;
	int				meansOfDeath;

	attack		 = (mod >> 8) & 0xFF;
	meansOfDeath = mod & 0xFF;

	if ( self->client->ps.pm_type == PM_DEAD ) 
	{
		return;
	}

	if ( level.intermissiontime ) 
	{
		return;
	}

	// Let the gametype know about the player death so it can adjust anything
	// it needs to adjust
	if ( attacker && attacker->client )
	{
		trap_GT_SendEvent ( GTEV_CLIENT_DEATH, level.time, self->s.number, self->client->sess.team, attacker->s.number, attacker->client->sess.team, 0 ); 
	}
	else
	{
		trap_GT_SendEvent ( GTEV_CLIENT_DEATH, level.time, self->s.number, self->client->sess.team, -1, -1, 0 ); 
	}

	// Add to the number of deaths for this player
	self->client->sess.deaths++;

	// This is just to ensure that the player wont render for even a single frame
	self->s.eFlags |= EF_DEAD;

	self->client->ps.pm_type = PM_DEAD;

	if ( attacker ) 
	{
		killer = attacker->s.number;
		if ( attacker->client ) 
		{
			killerName = attacker->client->pers.netname;
		} 
		else 
		{
			killerName = "<non-client>";
		}
	} 
	else 
	{
		killer = ENTITYNUM_WORLD;
		killerName = "<world>";
	}

	if ( killer < 0 || killer >= MAX_CLIENTS ) 
	{
		killer = ENTITYNUM_WORLD;
		killerName = "<world>";
	}

	if ( meansOfDeath < 0 || meansOfDeath >= sizeof( modNames ) / sizeof( modNames[0] ) ) 
	{
		obit = "<bad obituary>";
	} 
	else 
	{
		if ( attack == ATTACK_ALTERNATE )
		{
			obit = va ( "%s_ALT", modNames[ meansOfDeath ] );
		}
		else
		{
			obit = modNames[ meansOfDeath ];
		}
	}

	// If the weapon was charging then drop it with no forward velocity
	if ( self->client->ps.grenadeTimer )
	{
		gentity_t* missile;
		missile = G_FireWeapon( self, ATTACK_NORMAL );
		if ( missile )
		{
			VectorClear ( missile->s.pos.trDelta );
		}
	}

	G_LogPrintf("Kill: %i %i %i: %s killed %s by %s\n", 
		killer, self->s.number, meansOfDeath, killerName, 
		self->client->pers.netname, obit );

	// broadcast the death event to everyone
	ent = G_TempEntity( self->r.currentOrigin, EV_OBITUARY );
	ent->s.eventParm = mod;
	ent->s.otherEntityNum = self->s.number;
	ent->s.otherEntityNum2 = killer;
	ent->r.svFlags = SVF_BROADCAST;	// send to everyone

	self->enemy = attacker;

	if (attacker && attacker->client) 
	{
		attacker->client->lastkilled_client = self->s.number;

		if ( attacker == self )
		{
			if ( mod != MOD_TEAMCHANGE )
			{
				G_AddScore( attacker, g_suicidePenalty.integer );
			}
		} 
		else if ( OnSameTeam ( self, attacker ) ) 
		{
			if ( mod != MOD_TELEFRAG )
			{
				G_AddScore( attacker, g_teamkillPenalty.integer );
			}
		}
		else 
		{
			G_AddScore( attacker, 1 );
			attacker->client->sess.kills++;
	
			attacker->client->lastKillTime = level.time;
		}
	}
	else if ( mod != MOD_TEAMCHANGE )
	{
		G_AddScore( self, g_suicidePenalty.integer );
	}

	// If client is in a nodrop area, don't drop anything
	contents = trap_PointContents( self->r.currentOrigin, -1 );
	if ( !( contents & CONTENTS_NODROP ) ) 
	{
		// People who kill themselves dont drop guns
		if ( attacker == self )
		{
			self->client->ps.stats[STAT_WEAPONS] = 0;
		}

		TossClientItems( self );
	}
	else 
	{
		// Any gametype items that are dropped into a no drop area need to be reported
		// to the gametype so it can handle it accordingly
		for ( i = 0 ; i < MAX_GAMETYPE_ITEMS ; i++ ) 
		{
			gitem_t* item;

			// skip this gametype item if the client doenst have it
			if ( !(self->client->ps.stats[STAT_GAMETYPE_ITEMS] & (1<<i)) ) 
			{
				continue;
			}

			item = BG_FindGametypeItem ( i );

			// Let the gametype handle the problem, if it doenst handle it and return 1 then 
			if ( trap_GT_SendEvent ( GTEV_ITEM_STUCK, level.time, item->quantity, 0, 0, 0, 0 ) )
			{
				// Boe!Man 4/4/10: When in a rare case the combat DOES handle this problem (instead of g_items.c), we just declare the chars here.
				char color1[64];
				char color2[64];
				char color3[64];
				char color4[64];
				char color5[64];
				char color6[64];
				trap_Cvar_VariableStringBuffer ( "server_color1", color1, MAX_QPATH );
				trap_Cvar_VariableStringBuffer ( "server_color2", color2, MAX_QPATH );
				trap_Cvar_VariableStringBuffer ( "server_color3", color3, MAX_QPATH );
				trap_Cvar_VariableStringBuffer ( "server_color4", color4, MAX_QPATH );
				trap_Cvar_VariableStringBuffer ( "server_color5", color5, MAX_QPATH );
				trap_Cvar_VariableStringBuffer ( "server_color6", color6, MAX_QPATH );
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@The briefcase has %sr%se%st%su%sr%sned!", level.time + 5000, color1, color2, color3, color4, color5, color6));
				trap_SendServerCommand( -1, va("print \"^3[INF] ^7The briefcase has returned\n\""));
			}
			// just reset the gametype item
			else if ( !trap_GT_SendEvent ( GTEV_ITEM_STUCK, level.time, item->quantity, 0, 0, 0, 0 ) )
			{
				G_ResetGametypeItem ( item );
			}
		}
	}

	Cmd_Score_f( self );

	// Henk 01/04/10 -> Hp/armor message if you are killed
	if(attacker->client && self->client){ // if the attacker and target are both clients
		trap_SendServerCommand( self->s.number, va("print \"^3[Info] ^7%s ^7had ^3%i ^7health and ^3%i ^7armor left.\n\"", attacker->client->pers.netname, attacker->health, attacker->client->ps.stats[STAT_ARMOR]));
	}
	// End

	// send updated scores to any clients that are following this one,
	// or they would get stale scoreboards
	for ( i = 0 ; i < level.numConnectedClients; i++ ) 
	{
		gclient_t	*client;

		client = g_entities[level.sortedClients[i]].client;

		if ( client->pers.connected != CON_CONNECTED )
		{
			continue;
		}
		
		if ( !G_IsClientSpectating ( client ) ) 
		{
			continue;
		}
		
		if ( client->sess.spectatorClient == self->s.number ) 
		{
			Cmd_Score_f( g_entities + i );
		}
	}

	self->s.weapon					= WP_NONE;
	self->s.gametypeitems			= 0;

	// no gibbing right now
//	self->r.contents				= CONTENTS_CORPSE;
//	self->takedamage				= qtrue;			// can still be gibbed
	self->r.contents				= CONTENTS_CORPSE;
	self->takedamage				= qfalse;

	self->client->ps.zoomFov		= 0;			// Turn off zooming when we die
	self->client->ps.stats[STAT_GAMETYPE_ITEMS] = 0;
	self->client->ps.pm_flags &= ~(PMF_GOGGLES_ON|PMF_ZOOM_FLAGS);

	self->s.angles[0]				= 0;
	self->s.angles[2]				= 0;
	self->s.loopSound				= 0;
	self->r.maxs[2]					= -8;

	LookAtKiller (self, inflictor, attacker);

	VectorCopy( self->s.angles, self->client->ps.viewangles );

	// don't allow respawn until the death anim is done
	// g_forcerespawn may force spawning at some later time
	self->client->respawnTime = level.time + 1700;	
					
	switch ( hitLocation & (~HL_DISMEMBERBIT) )
	{
		case HL_WAIST:
			if ( rand() %2 )
			{
				anim = BOTH_DEATH_GROIN_1 + (rand()%2);
			}
			else
			{
				anim = BOTH_DEATH_GUT_1 + (rand()%2);	// GUT2 is being shot from the back.
			}
			break;

		default:			
		case HL_CHEST:
			anim = BOTH_DEATH_CHEST_1 + (rand()%2);
			break;

		case HL_CHEST_RT:
			if ( irand(1,10) < 8 )
			{
				anim = BOTH_DEATH_SHOULDER_RIGHT_1 + (rand()%2);
			}
			else
			{
				anim = BOTH_DEATH_CHEST_1 + (rand()%2);
			}
			break;

		case HL_CHEST_LT:

			if ( irand(1,10) < 8 )
			{
				anim = BOTH_DEATH_SHOULDER_LEFT_1 + (rand()%2);
			}
			else
			{
				anim = BOTH_DEATH_CHEST_1 + (rand()%2);
			}

			break;

		case HL_NECK:
			anim = BOTH_DEATH_NECK;
			break;

		case HL_HEAD:
			anim = BOTH_DEATH_HEAD_1 + (rand()%2);
			break;

		case HL_LEG_UPPER_LT:
			anim = BOTH_DEATH_THIGH_LEFT_1 + (rand()%2);
			break;

		case HL_LEG_LOWER_LT:
		case HL_FOOT_LT:
			anim = BOTH_DEATH_LEGS_LEFT_1 + (rand()%3);
			break;

		case HL_ARM_LT:

			if ( rand()%2 )
				anim = BOTH_DEATH_ARMS_LEFT_1 + (rand()%2);
			else
				anim = BOTH_DEATH_SHOULDER_LEFT_1 + (rand()%2);

			break;

		case HL_ARM_RT:

			if ( rand()%2 )
				anim = BOTH_DEATH_ARMS_RIGHT_1 + (rand()%2);
			else
				anim = BOTH_DEATH_SHOULDER_RIGHT_1 + (rand()%2);

			break;

		case HL_LEG_UPPER_RT:
			anim = BOTH_DEATH_THIGH_RIGHT_1 + (rand()%2);
			break;

		case HL_LEG_LOWER_RT:
		case HL_FOOT_RT:
			anim = BOTH_DEATH_LEGS_RIGHT_1 + (rand()%3);
			break;
	}

	self->client->ps.legsAnim = 
		( ( self->client->ps.legsAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;
	self->client->ps.torsoAnim = 
		( ( self->client->ps.torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;

	// If the dismember bit is set then make sure the body queue dismembers
	// the location that was hit
	if ( hitLocation & HL_DISMEMBERBIT )
	{
		CopyToBodyQue (self, hitLocation & (~HL_DISMEMBERBIT), hitDir );
	}
	else
	{
		CopyToBodyQue (self, HL_NONE, hitDir );
	}

	// the body can still be gibbed
	self->die = body_die;

	trap_LinkEntity (self);
}

/*
================
CheckArmor
================
*/
int CheckArmor (gentity_t *ent, int damage, int dflags)
{
	gclient_t	*client;
	int			save;
	int			count;

	if (!damage)
		return 0;

	client = ent->client;

	if (!client)
		return 0;

	if (dflags & DAMAGE_NO_ARMOR)
		return 0;

	// armor
	count = client->ps.stats[STAT_ARMOR];
	save = ceil( damage * ARMOR_PROTECTION );
	if (save >= count)
		save = count;

	if (!save)
		return 0;

	client->ps.stats[STAT_ARMOR] -= save;

	return save;
}


void G_ApplyKnockback( gentity_t *targ, vec3_t newDir, float knockback )
{
	vec3_t	kvel;
	float	mass;

	if ( targ->physicsBounce > 0 )	//overide the mass
		mass = targ->physicsBounce;
	else
		mass = 200;

	if ( g_gravity.value > 0 )
	{
		VectorScale( newDir, g_knockback.value * (float)knockback / mass * 0.8, kvel );
//		kvel[2] = newDir[2] * g_knockback.value * (float)knockback / mass * 1.5;
	}
	else
	{
		VectorScale( newDir, g_knockback.value * (float)knockback / mass, kvel );
	}

	if ( targ->client )
	{
		VectorAdd( targ->client->ps.velocity, kvel, targ->client->ps.velocity );
	}
	else if ( targ->s.pos.trType != TR_STATIONARY && targ->s.pos.trType != TR_LINEAR_STOP )
	{
		VectorAdd( targ->s.pos.trDelta, kvel, targ->s.pos.trDelta );
		VectorCopy( targ->r.currentOrigin, targ->s.pos.trBase );
		targ->s.pos.trTime = level.time;
	}

	// set the timer so that the other client can't cancel
	// out the movement immediately
	if ( targ->client && !targ->client->ps.pm_time ) 
	{
		int		t;

		t = knockback * 2;
		if ( t < 50 ) {
			t = 50;
		}
		if ( t > 200 ) {
			t = 200;
		}
		targ->client->ps.pm_time = t;
		targ->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
	}
}

/*
================
RaySphereIntersections
================
*/
int RaySphereIntersections( vec3_t origin, float radius, vec3_t point, vec3_t dir, vec3_t intersections[2] ) {
	float b, c, d, t;

	//	| origin - (point + t * dir) | = radius
	//	a = dir[0]^2 + dir[1]^2 + dir[2]^2;
	//	b = 2 * (dir[0] * (point[0] - origin[0]) + dir[1] * (point[1] - origin[1]) + dir[2] * (point[2] - origin[2]));
	//	c = (point[0] - origin[0])^2 + (point[1] - origin[1])^2 + (point[2] - origin[2])^2 - radius^2;

	// normalize dir so a = 1
	VectorNormalize(dir);
	b = 2 * (dir[0] * (point[0] - origin[0]) + dir[1] * (point[1] - origin[1]) + dir[2] * (point[2] - origin[2]));
	c = (point[0] - origin[0]) * (point[0] - origin[0]) +
		(point[1] - origin[1]) * (point[1] - origin[1]) +
		(point[2] - origin[2]) * (point[2] - origin[2]) -
		radius * radius;

	d = b * b - 4 * c;
	if (d > 0) {
		t = (- b + sqrt(d)) / 2;
		VectorMA(point, t, dir, intersections[0]);
		t = (- b - sqrt(d)) / 2;
		VectorMA(point, t, dir, intersections[1]);
		return 2;
	}
	else if (d == 0) {
		t = (- b ) / 2;
		VectorMA(point, t, dir, intersections[0]);
		return 1;
	}
	return 0;
}

int G_GetHitLocation(gentity_t *target, vec3_t ppoint, vec3_t dir )
{
	float		fdot;
	float		rdot;
	vec3_t		tangles;
	vec3_t		forward;
	vec3_t		up;
	vec3_t		right;
	vec3_t		distance;
	vec3_t		tcenter;
	vec3_t		temp;
	vec3_t		hit;

	// We are only interested in the YAW angle of the target
	VectorSet( tangles, 0, target->client->ps.viewangles[YAW], 0);

	// Extract the forward, right, and up vectors
	AngleVectors ( tangles, forward, right, up );

	// Determine the center of the target entity
	VectorAdd(target->r.absmin, target->r.absmax, tcenter);
	VectorScale(tcenter, 0.5, tcenter);


/* NOTE: This would work to figure out shots that go across the front of someone and
         hit the opposite side, but had an error in it when a shot came from either
		 the immediate left or right of the player.
*/

	// Calculate the distnace from the shooter to the target
	VectorCopy ( dir, temp );
	VectorSubtract ( tcenter, ppoint, distance );

	// Use that distnace to determine the point of tangent in relation to
	// the center of the player entity
	VectorMA ( ppoint, DotProduct ( temp, distance ), temp, hit );

	// Create a vector from the tangent point to the center.  This will
	// be used to determine which side was hit
	VectorSubtract ( tcenter, hit, temp );
	VectorCopy ( temp, distance );

	VectorSubtract ( tcenter, ppoint, temp );
	VectorNormalize ( temp );

	// Determine the shot in relation to the forward vector
	fdot = DotProduct ( forward, temp );

	// Determine the shot in relation to the right vector
	rdot = DotProduct ( right, temp );

	if ( distance[2] < -35 )
	{
		return HL_HEAD;
	}
	else if ( distance[2] < -32 )
	{
		return HL_NECK;
	}
	else if ( distance[2] < -27 )
	{
		if ( rdot > 0 )
			return HL_ARM_LT;

		return HL_ARM_RT;
	}
	else if ( distance[2] < -3 )
	{
		if ( fdot > 0 )
		{
			if ( rdot > 0 )
			{
				return HL_CHEST_LT;
			}

			return HL_CHEST_RT;
		}

		if ( rdot > 0 )
		{
			return HL_BACK_LT;
		}

		return HL_BACK_RT;
	}
	else if ( distance[2] < 4 )
	{
		return HL_WAIST;
	}
	else if ( distance[2] < 18 )
	{
		if ( rdot > 0 )
			return HL_LEG_UPPER_LT;

		return HL_LEG_UPPER_RT;
	}
	else if ( distance[2] < 33 )
	{
		if ( rdot > 0 )
			return HL_LEG_LOWER_LT;

		return HL_LEG_LOWER_RT;
	}

	if ( rdot > 0 )
		return HL_FOOT_LT;

	return HL_FOOT_RT;
}

/*
============
T_Damage

targ		entity that is being damaged
inflictor	entity that is causing the damage
attacker	entity that caused the inflictor to damage targ
	example: targ=monster, inflictor=rocket, attacker=player

dir			direction of the attack for knockback
point		point at which the damage is being inflicted, used for headshots
damage		amount of damage being inflicted
knockback	force to be applied against targ as a result of the damage

inflictor, attacker, dir, and point can be NULL for environmental effects

dflags		these flags are used to control how T_Damage works
	DAMAGE_RADIUS			damage was indirect (from a nearby explosion)
	DAMAGE_NO_ARMOR			armor does not protect from this damage
	DAMAGE_NO_KNOCKBACK		do not affect velocity, just view angles
	DAMAGE_NO_PROTECTION	kills godmode, armor, everything
============
*/

int G_Damage ( 
	gentity_t		*targ, 
	gentity_t		*inflictor, 
	gentity_t		*attacker,
	vec3_t			dir, 
	vec3_t			point,
	int				damage,
	int				dflags,
	int				mod,
	int				location
	) 
{
	gclient_t		*client;
	int				take;
	int				save;
	int				asave;
	int				knockback;

	if (!targ->takedamage) 
	{
		return 0;
	}

	// See if they are invulnerable
	if ( (mod&0xFF) < MOD_WATER )
	{
		if ( targ->client && (level.time - targ->client->invulnerableTime < g_respawnInvulnerability.integer * 1000) )
		{
			return 0;
		}
	}

	// the intermission has allready been qualified for, so don't
	// allow any extra scoring
	if ( level.intermissionQueued ) 
	{
		return 0;
	}

	// Cant change outfitting after being shot
	if ( targ->client )
	{
		targ->client->noOutfittingChange = qtrue;
	}

	if ( !inflictor ) 
	{
		inflictor = &g_entities[ENTITYNUM_WORLD];
	}
	if ( !attacker ) 
	{
		attacker = &g_entities[ENTITYNUM_WORLD];
	}

	// shootable doors / buttons don't actually have any health
	if ( targ->s.eType == ET_MOVER ) 
	{
		if ( targ->use && targ->moverState == MOVER_POS1 ) 
		{
			targ->use( targ, inflictor, attacker );
		}
		return 0;
	}

	client = targ->client;

	if ( client ) 
	{
		if ( client->noclip ) 
		{
			return 0;
		}
	}

	if ( !dir ) 
	{
		dflags |= DAMAGE_NO_KNOCKBACK;
	} 
	else 
	{
		VectorNormalize(dir);
	}

	knockback = damage;
	if ( knockback > 200 ) 
	{
		knockback = 200;
	}
	if ( targ->flags & FL_NO_KNOCKBACK ) {
		knockback = 0;
	}
	if ( dflags & DAMAGE_NO_KNOCKBACK ) {
		knockback = 0;
	}

/*
	// figure momentum add, even if the damage won't be taken
	if ( knockback && targ->client ) 
	{
		G_ApplyKnockback ( targ, dir, knockback );
		vec3_t	kvel;
		float	mass;

		mass = 200;

		VectorScale (dir, g_knockback.value * (float)knockback / mass, kvel);
		VectorAdd (targ->client->ps.velocity, kvel, targ->client->ps.velocity);

		// set the timer so that the other client can't cancel
		// out the movement immediately
		if ( !targ->client->ps.pm_time ) {
			int		t;

			t = knockback * 2;
			if ( t < 50 ) {
				t = 50;
			}
			if ( t > 200 ) {
				t = 200;
			}
			targ->client->ps.pm_time = t;
			targ->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
		}
	}
*/

	// check for completely getting out of the damage
	if ( !(dflags & DAMAGE_NO_PROTECTION) ) {

		// if TF_NO_FRIENDLY_FIRE is set, don't do damage to the target
		// if the attacker was on the same team
		if ( targ != attacker && OnSameTeam (targ, attacker)  ) 
		{
			if ( !g_friendlyFire.integer ) 
			{
				return 0;
			}
		}

		// check for godmode
		if ( targ->flags & FL_GODMODE ) 
		{
			return 0;
		}
	}

	if ( damage < 1 ) 
	{
		damage = 1;
	}

	take = damage;
	save = 0;

	// Be careful with grenades
	if ( attacker == targ )
	{
		take *= 2;
	}

	// save some from armor
	asave = CheckArmor (targ, take, dflags);
	take -= asave;

	// Teamkill dmage thats not caused by a telefrag?
	if ( g_teamkillDamageMax.integer && mod != MOD_TELEFRAG )
	{
		if ( level.gametypeData->teams && targ && attacker && targ != attacker )
		{
			// Hurt your own team?
			if ( OnSameTeam ( targ, attacker ) )
			{
				// Dont count more than one damage call per frame (grenades!)
				if ( level.time != attacker->client->sess.teamkillForgiveTime )
				{
					int actualtake = Com_Clamp ( 0, targ->health, take );

					attacker->client->sess.teamkillDamage	   += actualtake;
					attacker->client->sess.teamkillForgiveTime  = level.time;
				}
			}
		}
	}

	// Output hits
	if ( g_logHits.integer && attacker && targ && attacker->client && targ->client )
	{
		G_LogPrintf ( "hit: %i %i %i %i %i: %s hit %s at location %i for %i\n",
						  attacker->s.number,
						  targ->s.number,
						  location,
						  take,
						  asave,
						  attacker->client->pers.netname,
						  targ->client->pers.netname,
						  location,
						  (int)((float)take) );
	}

	if ( g_debugDamage.integer ) 
	{
		Com_Printf( "%i: client:%i health:%i damage:%i armor:%i\n", level.time, targ->s.number, targ->health, take, asave );
	}

	// add to the damage inflicted on a player this frame
	// the total will be turned into screen blends and view angle kicks
	// at the end of the frame
	if ( client ) 
	{
		if ( attacker ) 
		{
			client->ps.persistant[PERS_ATTACKER] = attacker->s.number;
		} 
		else 
		{
			client->ps.persistant[PERS_ATTACKER] = ENTITYNUM_WORLD;
		}

		if ( mod != MOD_WATER )
		{
			client->damage_armor += asave;
			client->damage_blood += take;
		}

		client->damage_knockback += knockback;
		
		if ( dir ) 
		{
			VectorCopy ( dir, client->damage_from );
			client->damage_fromWorld = qfalse;
		} 
		else 
		{
			VectorCopy ( targ->r.currentOrigin, client->damage_from );
			client->damage_fromWorld = qtrue;
		}

		if (attacker && attacker->client)
		{
			BotDamageNotification(client, attacker);
		}
		else if (inflictor && inflictor->client)
		{
			BotDamageNotification(client, inflictor);
		}
	}

	if (targ->client) 
	{
		// set the last client who damaged the target
		targ->client->lasthurt_client = attacker->s.number;
		targ->client->lasthurt_time = level.time;
		targ->client->lasthurt_mod = mod;
	}

	// do the damage
	if (take)
	{
		targ->health = targ->health - take;

		if ( targ->client )
		{
			targ->client->ps.stats[STAT_HEALTH] = targ->health;

			if ( targ->health > 0 )
			{
				// Slow down the client at bit when they get hit
				targ->client->ps.velocity[0] *= 0.25f;
				targ->client->ps.velocity[1] *= 0.25f;

				// figure momentum add, even if the damage won't be taken
				if ( knockback ) 
				{
					G_ApplyKnockback ( targ, dir, knockback );
				}

				// Friendly fire
				if ( g_friendlyFire.integer && targ != attacker && OnSameTeam ( targ, attacker ) )
				{
					vec3_t diff;
					
					// Make sure the attacker is close enough to hear the guy whining
					VectorSubtract ( targ->r.currentOrigin, attacker->r.currentOrigin, diff );
					if ( VectorLengthSquared ( diff ) < 800 * 800 )
					{
						G_VoiceGlobal ( targ, "check_fire", qfalse );
					}
				}
			}
		}

		if ( targ->health <= 0 )
		{
			// Something dismembered?
			if ( (targ->health < DISMEMBER_HEALTH && !(dflags&DAMAGE_NO_GORE)) || (dflags&DAMAGE_FORCE_GORE) )
			{
				location |= HL_DISMEMBERBIT;
			}

			if ( client )
				targ->flags |= FL_NO_KNOCKBACK;

			if (targ->health < -999)
				targ->health = -999;

			targ->enemy = attacker;
			targ->die (targ, inflictor, attacker, take, mod, location, dir );
		} 
		else if ( targ->pain )
		{
			targ->pain (targ, attacker, take);
		}
	}

	return take;
}

/*
============
CanDamage

Returns qtrue if the inflictor can directly damage the target.  Used for
explosions and melee attacks.
============
*/
qboolean CanDamage (gentity_t *targ, vec3_t origin) {
	vec3_t	dest;
	trace_t	tr;
	vec3_t	midpoint;

	// use the midpoint of the bounds instead of the origin, because
	// bmodels may have their origin is 0,0,0
	VectorAdd (targ->r.absmin, targ->r.absmax, midpoint);
	VectorScale (midpoint, 0.5, midpoint);

	VectorCopy (midpoint, dest);
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0 || tr.entityNum == targ->s.number)
		return qtrue;

	// this should probably check in the plane of projection, 
	// rather than in world coordinate, and also include Z
	VectorCopy (midpoint, dest);
	dest[0] += 15.0;
	dest[1] += 15.0;
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0)
		return qtrue;

	VectorCopy (midpoint, dest);
	dest[2] = targ->r.absmax[2];
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0)
		return qtrue;

	VectorCopy (midpoint, dest);
	dest[2] = targ->r.absmin[2];
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0)
		return qtrue;

	VectorCopy (midpoint, dest);
	dest[0] += 15.0;
	dest[1] -= 15.0;
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0)
		return qtrue;

	VectorCopy (midpoint, dest);
	dest[0] -= 15.0;
	dest[1] += 15.0;
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0)
		return qtrue;

	VectorCopy (midpoint, dest);
	dest[0] -= 15.0;
	dest[1] -= 15.0;
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0)
		return qtrue;


	return qfalse;
}
/*
============
G_MultipleDamageLocations
============
*/
int G_MultipleDamageLocations(int hitLocation)
{

	switch ( hitLocation & (~HL_DISMEMBERBIT) )
	{
		case HL_FOOT_RT:
		case HL_FOOT_LT:
			hitLocation |= (HL_FOOT_RT | HL_FOOT_LT);
			break;
		case HL_LEG_UPPER_RT:
			hitLocation |= (HL_LEG_UPPER_RT | HL_LEG_LOWER_LT);
			if ( rand() %2 )
			{
				hitLocation |= HL_HAND_RT;
			}
			break;
		case HL_LEG_UPPER_LT:
			hitLocation |= (HL_LEG_UPPER_LT | HL_LEG_LOWER_RT);
			if ( rand() %2 )
			{
				hitLocation |= HL_HAND_LT;
			}
			break;
		case HL_LEG_LOWER_RT:
			hitLocation |= (HL_LEG_LOWER_RT | HL_FOOT_LT);
			break;
		case HL_LEG_LOWER_LT:
			hitLocation |= (HL_LEG_LOWER_LT | HL_FOOT_RT);
			break;
		case HL_HAND_RT:
			hitLocation |= HL_HAND_RT;
			break;
		case HL_HAND_LT:
			hitLocation |= HL_HAND_LT;
			break;
		case HL_ARM_RT:
			hitLocation |= (HL_ARM_RT | HL_LEG_UPPER_RT) ;
			break;
		case HL_ARM_LT:
			hitLocation |= (HL_ARM_LT | HL_LEG_UPPER_LT) ;
			break;
		case HL_HEAD:
			hitLocation |= HL_HEAD ;
			if ( rand() %2 )
			{
				hitLocation |= HL_ARM_RT;
			}
			else
			{
				hitLocation |= HL_ARM_LT;
			}
			break;
		case HL_WAIST:
			hitLocation |= (HL_LEG_UPPER_RT | HL_LEG_UPPER_LT) ;

			if ( rand() %2 )
			{
				if ( rand() %2 )
				{
					hitLocation |= HL_HAND_RT;
				}
				else
				{
					hitLocation |= HL_HAND_LT;
				}
			}
			break;
		case HL_BACK_RT:
		case HL_CHEST_RT:
			hitLocation |= HL_ARM_RT;
			hitLocation |= HL_HEAD;
			break;
		case HL_BACK_LT:
		case HL_CHEST_LT:
			hitLocation |= HL_ARM_LT;
			hitLocation |= HL_HEAD;
			break;
		case HL_BACK:
		case HL_CHEST:
			hitLocation |= (HL_ARM_RT | HL_ARM_LT);
			hitLocation |= HL_HEAD;
			break;

	}

	return (hitLocation);
}


/*
============
G_RadiusDamage
============
*/
qboolean G_RadiusDamage ( 
	vec3_t		origin, 
	gentity_t*	attacker, 
	float		damage, 
	float		radius,
	gentity_t*	ignore, 
	int			power, 
	int			mod
	) 
{
	float		points, dist;
	gentity_t	*ent, *tent;
	int			entityList[MAX_GENTITIES];
	int			numListedEntities;
	vec3_t		mins, maxs;
	vec3_t		v;
	vec3_t		dir;
	int			i, e;
	qboolean	hitClient = qfalse;

	if ( radius < 1 ) 
	{
		radius = 1;
	}

	for ( i = 0 ; i < 3 ; i++ ) 
	{
		mins[i] = origin[i] - radius;
		maxs[i] = origin[i] + radius;
	}

	numListedEntities = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );

	for ( e = 0 ; e < numListedEntities ; e++ ) 
	{
		ent = &g_entities[entityList[ e ]];

		if (ent == ignore)
		{
			continue;
		}
		
		if (!ent->takedamage)
		{
			continue;
		}

		// find the distance from the edge of the bounding box
		for ( i = 0 ; i < 3 ; i++ ) 
		{
			if ( origin[i] < ent->r.absmin[i] ) 
			{
				v[i] = ent->r.absmin[i] - origin[i];
			} 
			else if ( origin[i] > ent->r.absmax[i] ) 
			{
				v[i] = origin[i] - ent->r.absmax[i];
			} 
			else 
			{
				v[i] = 0;
			}
		}

		dist = VectorLength( v );
		if ( dist >= radius ) 
		{
			continue;
		}

		points = damage * ( 1.0 - powf((dist / radius), power));

		if( CanDamage (ent, origin) ) 
		{
			int		location;
			int		weapon;
			vec3_t	hitdir;
			int		d;

			VectorSubtract (ent->r.currentOrigin, origin, dir);
			// push the center of mass higher than the origin so players
			// get knocked into the air more

			location = HL_NONE;
			if ( ent->client )
			{
				VectorNormalize ( dir );								
				VectorCopy(dir, hitdir);
				dir[2] = 0;
				location = G_GetHitLocation ( ent, origin, dir );
				location = G_MultipleDamageLocations ( location );
			}

			d = G_Damage (ent, NULL, attacker, dir, origin, (int)points, DAMAGE_RADIUS|DAMAGE_NO_ARMOR, mod, location );

			if ( d && ent->client )
			{
				// Put some procedural gore on the target.
				tent = G_TempEntity( origin, EV_EXPLOSION_HIT_FLESH );
				
				// send entity and direction
				tent->s.eventParm = DirToByte( hitdir );
				if (ignore && ignore->s.weapon)
				{
					weapon = ignore->s.weapon;		// Weapon type number
				}
				else if (points >= 10)
				{	// dangerous weapon
					weapon = WP_M67_GRENADE;
				}
				else
				{	// Just a flesh wound
					weapon = WP_M84_GRENADE;
				}
				tent->s.otherEntityNum2 = ent->s.number;			// Victim entity number

				// Pack the shot info into the temp end for gore
				tent->s.time  = weapon + ((((int)ent->s.apos.trBase[YAW]&0x7FFF) % 360) << 16);		
				if ( attacker->s.eFlags & EF_ALT_FIRING )
				{
					tent->s.time += (ATTACK_ALTERNATE<<8);
				}
				VectorCopy ( ent->r.currentOrigin, tent->s.angles );
				SnapVector ( tent->s.angles );
			}
		}
	}

	return hitClient;
}
