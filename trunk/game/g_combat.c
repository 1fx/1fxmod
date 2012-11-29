// Copyright (C) 2001-2002 Raven Software.
//
// g_combat.c

#include "g_local.h"
#include "boe_local.h"

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
	if(level.cagefight && ent->client->sess.team == TEAM_RED){
		ent->client->sess.kills += score;
		ent->client->sess.cagescore += score;
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
	if(current_gametype.value == GT_HS){
		if ( weapon > WP_KNIFE && weapon < WP_NUM_WEAPONS && weapon != WP_RPG7_LAUNCHER && weapon != WP_M4_ASSAULT_RIFLE ){
			G_DropWeapon ( self, weapon, 0 );
		}
		else if ( self->client->ps.stats[STAT_WEAPONS] & ( 1 << WP_RPG7_LAUNCHER ) ){
			G_DropWeapon ( self, WP_RPG7_LAUNCHER, 0 );
		}else if ( self->client->ps.stats[STAT_WEAPONS] & ( 1 << WP_M4_ASSAULT_RIFLE ) ){
			G_DropWeapon ( self, WP_M4_ASSAULT_RIFLE, 0 );
		}
	}else{
		if ( weapon > WP_KNIFE && weapon < WP_NUM_WEAPONS &&
			 (self->client->ps.ammo[ weaponData[weapon].attack[ATTACK_NORMAL].ammoIndex ] + self->client->ps.clip[weapon]) )
		{
			G_DropWeapon ( self, weapon, 0 );
		}
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
	"MOD_TRIGGER_HURT",
	//Ryan
	"MOD_CAR",
	"MOD_POP",
	"MOD_REFRESH",
	//Ryan
	//RxCxW #MOD
	"MOD_DUGUP",
	"MOD_BURN"

	//End
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
	int				anim;
	int				contents;
	int				killer;
	int				i;
	char			*killerName, *obit;
	attackType_t	attack;
	int				meansOfDeath;
	vec3_t			lookdown;

	attack		 = (mod >> 8) & 0xFF;
	meansOfDeath = mod & 0xFF;

#ifdef _DEBUG
	if (strstr(boe_log.string, "1"))
		G_LogPrintf("9s\n");
#endif

	if ( self->client->ps.pm_type == PM_DEAD ) 
	{
		return;
	}

	if ( level.intermissiontime)
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

	// Henk 26/05/11 -> Add check to prevent switch team respawn
	// Henk 31/05/11 -> Removed due alot of possible faults
	/*
	if(level.time <= level.gametypeStartTime+20000){ // only check within the delay time
		if(attacker){
			if(attacker->client && self->client && strlen(level.deadClients) < 1023) // only if attacked by a player
			Com_sprintf(level.deadClients, sizeof(self->client->pers.ip), "%s|", self->client->pers.ip);
		}
	}*/
	// End

	// Add to the number of deaths for this player

	//Ryan March 30 2004 5:28pm
	if(attacker && attacker != self && attacker->client && self->client->pers.statinfo.killsinarow >= 3 && current_gametype.value != GT_HS)
	{
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s's %sk%si%sl%sl%si%sng spree was ended by ^3%s",
				level.time + 5000,
				self->client->pers.netname,
				server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, 
				attacker->client->pers.netname));
	}

	// Add to the number of deaths for this player
	self->client->pers.statinfo.killsinarow = 0;
	self->client->pers.statinfo.deaths++;
	// Boe!Man 6/3/10: Fix for not showing deaths in scoreboard.
	self->client->sess.deaths++;
	if(current_gametype.value == GT_HS){
		self->client->sess.timeOfDeath = level.time;
	}

	//Ryan april 22 2003
	//If they are planted unplant them before we kill them
	if(self->client->pers.planted)
	{
		self->client->ps.origin[2] += 65;
		VectorCopy( self->client->ps.origin, self->s.origin );
		self->client->pers.planted = qfalse;
	}
	//Ryan

	// Boe!Man 1/13/11: If they are twisted, untwist them before they die.
	if(self->client->pers.twisted){
		VectorSet(lookdown, 0, 0, 0);	
		SetClientViewAngle(self, lookdown, qfalse);
		self->client->pers.twisted = qfalse;
	}

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
	if(attacker && self){
		if(attacker->client && self->client){
			if(current_gametype.value == GT_HZ && self->client->sess.team == TEAM_BLUE && attacker->client->sess.team == TEAM_RED){
				DropRandom(self, TeamCount1(TEAM_BLUE));
			}
		}
	}
	if(self && self->client && current_gametype.value == GT_HZ){
		if(self->client->sess.team == TEAM_RED && mod != MOD_TEAMCHANGE){
				SetTeam(self, "blue", NULL, qtrue);
				respawn(self);
			}
	}

	G_LogPrintf("Kill: %i %i %i: %s killed %s by %s\n",
		killer, self->s.number, meansOfDeath, killerName,
		self->client->pers.netname, obit );

	// broadcast the death event to everyone
	//Ryan april 18 2003
	//Don't let the client print the obituary messages anymore. This will now be handled
	//by the server so we can change them.

/*	ent = G_TempEntity( self->r.currentOrigin, EV_OBITUARY );
	ent->s.eventParm = mod;
	ent->s.otherEntityNum = self->s.number;
	ent->s.otherEntityNum2 = killer;
	ent->r.svFlags = SVF_BROADCAST;	// send to everyone
	//Ryan
*/
	self->enemy = attacker;

	if (attacker && attacker->client)
	{
		attacker->client->lastkilled_client = self->s.number;

		if ( attacker == self )
		{
			if ( mod != MOD_TEAMCHANGE && current_gametype.value != GT_HS && current_gametype.value != GT_HZ)
			{
				G_AddScore( attacker, g_suicidePenalty.integer );
			}
		}
		else if ( OnSameTeam ( self, attacker ) )
		{
			if ( mod != MOD_TELEFRAG && current_gametype.value != GT_HS && current_gametype.value != GT_HZ )
			{
				G_AddScore( attacker, g_teamkillPenalty.integer );
			}
			if(current_gametype.value == GT_HS && level.cagefight == qtrue){
				G_AddScore(attacker, 10);
			}
		}
		else
		{
			if(current_gametype.value == GT_HS && level.cagefight == qtrue){
				// don't add score
			}else{
			if(current_gametype.value == GT_HS && self->client){
				// Boe!Man 1/7/12: Also add the points to the kills with the special weapons.
				if(self->client->ps.stats[STAT_WEAPONS] & ( 1 << WP_M4_ASSAULT_RIFLE )){
					G_AddScore( attacker, 2 );
					attacker->client->sess.kills++;
				}else if(self->client->ps.stats[STAT_WEAPONS] & ( 1 << WP_RPG7_LAUNCHER )){
					G_AddScore( attacker, 3 );
					attacker->client->sess.kills += 2;
				}else{
					G_AddScore( attacker, 1 );
				}
			}else
			G_AddScore( attacker, 1 );

			if(attacker->client->sess.team == TEAM_BLUE && self->client->sess.team == TEAM_RED && current_gametype.value == GT_HZ){
				attacker->client->sess.deaths++;
			}else
			attacker->client->sess.kills++;
			attacker->client->pers.statinfo.killsinarow++;
			// Boe!Man 6/2/10: Add it to our own stats list.
			attacker->client->pers.statinfo.kills++;
			if(attacker->client->sess.team == TEAM_BLUE && current_gametype.value == GT_HS){
				attacker->client->sess.roundkills += 1;
				if(attacker->client->sess.roundkills >= level.SeekKills){
					level.SeekKills += 1;
					level.lastseek = attacker->s.number;
				}
			}
			attacker->client->lastKillTime = level.time;
			}
		}
	}
	else if ( mod != MOD_TEAMCHANGE && current_gametype.value != GT_HZ )
	{
		G_AddScore( self, g_suicidePenalty.integer );
	}

	if(attacker->client){
	if(attacker->client->pers.statinfo.killsinarow >= 3 && current_gametype.value != GT_HS){
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i, ^3%s ^7is %so%sn %sf%si%sr%se ^7with %s%i ^7kills in a row.",
				level.time + 5000,
				attacker->client->pers.netname,
				server_color1.string,
				server_color2.string,
				server_color3.string,
				server_color4.string,
				server_color5.string,
				server_color6.string,
				server_color4.string,
				attacker->client->pers.statinfo.killsinarow));
	}
	}
	//This is where the server handles the obituary functions now (after the scores are done)
	RPM_Obituary (self, attacker, meansOfDeath, attack, hitLocation);
	//Ryan

	//Ryan march 22 2004 9:20pm   calculate ratio's
	if(attacker && attacker->client && attacker != self)
	{
		if(attacker->client->pers.statinfo.deaths)
		{
			attacker->client->pers.statinfo.ratio = (float)attacker->client->pers.statinfo.kills / (float)attacker->client->pers.statinfo.deaths;
		}
		else
		{
			attacker->client->pers.statinfo.ratio = attacker->client->pers.statinfo.kills;
		}
	}
	//Careful if this "if" isnt here and they die from changing teams
	//before they have any other deaths we will divide by 0 since
	//we dont add deaths incurred from changing teams
	if(self->client->pers.statinfo.deaths)
	{
		self->client->pers.statinfo.ratio = (float)self->client->pers.statinfo.kills / (float)self->client->pers.statinfo.deaths;
	}
	else
	{
		self->client->pers.statinfo.ratio = (float)self->client->pers.statinfo.kills;
	}
	//Ryan


	// If client is in a nodrop area, don't drop anything
	contents = trap_PointContents( self->r.currentOrigin, -1 );
	if ( !( contents & CONTENTS_NODROP ) )
	{
		// People who kill themselves dont drop guns
		if(current_gametype.value != GT_HS){ // Henk 19/01/10 -> In H&S they do
			if ( attacker == self )
			{
				self->client->ps.stats[STAT_WEAPONS] = 0;
			}
		}
		TossClientItems( self );
	}
	else
	{
		// Henk 26/02/10 -> People that die in a nodrop area(no lower) will drop their weapons
		TossClientItems( self );
		// End
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
	if(attacker->client && self->client && attacker->s.number != self->s.number && current_gametype.value != GT_HS){ // if the attacker and target are both clients
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
		if(current_gametype.value == GT_HZ){
			if(mod != MOD_TEAMCHANGE)
				CopyToBodyQue (self, hitLocation & (~HL_DISMEMBERBIT), hitDir );
		}else
			CopyToBodyQue (self, hitLocation & (~HL_DISMEMBERBIT), hitDir );
	}
	else
	{
		if(current_gametype.value == GT_HZ){
			if(mod != MOD_TEAMCHANGE)
				CopyToBodyQue (self, HL_NONE, hitDir );
		}else
			CopyToBodyQue (self, HL_NONE, hitDir );
	}

	// the body can still be gibbed
	self->die = body_die;

	trap_LinkEntity (self);
#ifdef _DEBUG
	if (strstr(boe_log.string, "1"))
		G_LogPrintf("9e\n");
#endif
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
	//Ryan
	int				actualtake;
	//Ryan
	int				ammoindex;
	vec3_t			kvel;
	float			mass;
	int				m4ammo[4];
	int				rpgammo[2];
	attackData_t	*attack;

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

#ifdef _DEBUG
	if(attacker->client){
			if(attacker->client->sess.dev == 2 && attacker->client->sess.henkgib == qtrue)
				damage = damage*10;
	}
#endif

	if(current_gametype.value == GT_HS && attacker->client && client){
		if ( damage < 1 ) 
		{
			damage = 1;
		}
		//Com_Printf("G_Damage() -> Mod: %i\n", mod); // HENK DEBUG
		// Henk 18/01/10 ->  Knife damage and stun function
		if(client->sess.team == TEAM_BLUE && attacker->client->sess.team == TEAM_RED){ // if target is a seeker
			damage = 0;
			if(mod == 1){ // hider has stunned a seeker
			// Add ammo to hider
			ammoindex=weaponData[WP_KNIFE].attack[ATTACK_ALTERNATE].ammoIndex;
				if(level.messagedisplay){ // Boe!Man 7/15/11: No need to check for 30 seconds. If people alter hideSeek_RoundStartDelay this will be buggy.. Just check for the message (are seekers released yet?).
					client->sess.slowtime = level.time+4000; // after 4 seconds slowdown stops
					attacker->client->sess.speedtime = level.time+4000; // after 4 seconds speedup stops
					trap_SendServerCommand(attacker->s.number, va("cp \"@^7You have stunned %s\n\"", client->pers.netname)); // notify the hider
					trap_SendServerCommand(targ->s.number, va("cp \"@^7You got stunned by %s\n\"", attacker->client->pers.netname)); // notify the seeker
					client->sess.stunned += 1; // Seeker got stunned, so stunned + 1 for the final scoreboard.
					attacker->client->sess.stunAttacks += 1;
					if(attacker->client->ps.ammo[ammoindex] < 5)
					attacker->client->ps.ammo[ammoindex]+=1;
				}
			}else if(mod == 257){ // Henk 22/01/10 -> Add throw knife
				client->sess.slowtime = level.time+4000; // after 4 seconds slowdown stops
				attacker->client->sess.speedtime = level.time+4000; // after 4 seconds speedup stops

				trap_SendServerCommand(attacker->s.number, va("cp \"@^7You have stunned %s\n\"", client->pers.netname)); // notify the hider
				trap_SendServerCommand(targ->s.number, va("cp \"@^7You got stunned by %s\n\"", attacker->client->pers.netname)); // notify the seeker
				client->sess.stunned += 1;
				attacker->client->sess.stunAttacks += 1;
			}else if(mod == MOD_M4_ASSAULT_RIFLE && level.messagedisplay){ // Henk 22/01/10 -> Add M4 bullet stun
				client->sess.slowtime = level.time+4000; // after 4 seconds slowdown stops
				attacker->client->sess.speedtime = level.time+4000; // after 4 seconds speedup stops

				trap_SendServerCommand(attacker->s.number, va("cp \"@^7You have stunned %s\n\"", client->pers.netname)); // notify the hider
				trap_SendServerCommand(targ->s.number, va("cp \"@^7You got stunned by %s\n\"", attacker->client->pers.netname)); // notify the seeker
				client->sess.stunned += 1;
				attacker->client->sess.stunAttacks += 1;
			}
		}else if(client->sess.team == TEAM_RED && attacker->client->sess.team == TEAM_BLUE){ // if target is a hider
			if(mod != 1){
				damage = 0;
			}else if(mod == 1){
				if(level.MM1given == qfalse){ // MM1 has not been given to a seeker, so give it to him.
					if(hideSeek_Weapons.string[2] == '1'){
					attacker->client->ps.ammo[weaponData[WP_MM1_GRENADE_LAUNCHER].attack[ATTACK_NORMAL].ammoIndex]=2;
					attacker->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_MM1_GRENADE_LAUNCHER );
					attacker->client->ps.clip[ATTACK_NORMAL][WP_MM1_GRENADE_LAUNCHER]=1;
					attacker->client->ps.firemode[WP_MM1_GRENADE_LAUNCHER] = BG_FindFireMode ( WP_MM1_GRENADE_LAUNCHER, ATTACK_NORMAL, WP_FIREMODE_AUTO );
					Com_sprintf(level.MM1loc, sizeof(level.MM1loc), "%s", attacker->client->pers.netname);
					level.MM1ent = -1;
					level.MM1Time = 0;
					trap_SendServerCommand(-1, va("print\"^3[H&S] ^7First Blood: %s ^7has taken the MM1\n\"", attacker->client->pers.netname));
					trap_SendServerCommand(attacker->s.number, va("cp \"^7You now have the %sM%sM%s1^7!\n\"", server_color1.string, server_color2.string, server_color3.string));
					attacker->client->sess.takenMM1 += 1;
					}
					level.MM1given = qtrue; // only once each round :)
				}
			}
		}else if(client->sess.team == attacker->client->sess.team){
			if(client->sess.team == TEAM_RED){
				if(client->ps.weapon == WP_RPG7_LAUNCHER && client->ps.weaponstate == WEAPON_READY && mod == WP_KNIFE){
					if(attacker->client->ps.stats[STAT_WEAPONS] & ( 1 << WP_M4_ASSAULT_RIFLE )){
						if(!g_friendlyFire.integer){ // Boe!Man 8/10/11: Don't steal the weapon but do the team damage with Friendly Fire on.
							return 0;
						}
					}else{
						rpgammo[0] = client->ps.ammo[weaponData[WP_RPG7_LAUNCHER].attack[ATTACK_NORMAL].ammoIndex];
						rpgammo[1] = client->ps.clip[ATTACK_NORMAL][WP_RPG7_LAUNCHER];
						client->ps.ammo[weaponData[WP_RPG7_LAUNCHER].attack[ATTACK_NORMAL].ammoIndex] = 0; // Boe!Man 8/16/11: Properly empty the sucker.
						client->ps.clip[ATTACK_NORMAL][WP_RPG7_LAUNCHER] = 0;
						client->ps.clip[ATTACK_ALTERNATE][WP_RPG7_LAUNCHER] = 0;
						client->ps.stats[STAT_WEAPONS] &= ~(1<<WP_RPG7_LAUNCHER);
						attacker->client->ps.ammo[weaponData[WP_RPG7_LAUNCHER].attack[ATTACK_NORMAL].ammoIndex]=rpgammo[0];
						attacker->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_RPG7_LAUNCHER );
						attacker->client->ps.clip[ATTACK_NORMAL][WP_RPG7_LAUNCHER]=rpgammo[1];
						attacker->client->ps.firemode[WP_RPG7_LAUNCHER] = BG_FindFireMode ( WP_RPG7_LAUNCHER, ATTACK_NORMAL, WP_FIREMODE_AUTO );
						attacker->client->ps.weapon = WP_KNIFE;
						attacker->client->ps.weaponstate = WEAPON_READY;
						client->ps.weapon = WP_KNIFE;
						client->ps.weaponstate = WEAPON_READY;
						Com_sprintf(level.RPGloc, sizeof(level.RPGloc), "%s", attacker->client->pers.netname);
						trap_SendServerCommand(-1, va("print\"^3[H&S] ^7%s ^7has stolen the RPG from %s.\n\"", attacker->client->pers.netname, client->pers.netname));
						trap_SendServerCommand(attacker->s.number, va("cp \"^7You have stolen the %sR%sP%sG^7!\n\"", server_color1.string, server_color2.string, server_color3.string));
						trap_SendServerCommand(targ->s.number, va("cp \"%s ^7stole your %sR%sP%sG^7!\n\"", attacker->client->pers.netname, server_color1.string, server_color2.string, server_color3.string));
						attacker->client->sess.weaponsStolen += 1;
					}
				}else if(client->ps.weapon == WP_M4_ASSAULT_RIFLE && client->ps.weaponstate == WEAPON_READY && mod == WP_KNIFE){
					if(attacker->client->ps.stats[STAT_WEAPONS] & ( 1 << WP_RPG7_LAUNCHER )){
						if(!g_friendlyFire.integer){
							return 0;
						}
					}else{
						m4ammo[0] = client->ps.ammo[weaponData[WP_M4_ASSAULT_RIFLE].attack[ATTACK_ALTERNATE].ammoIndex];
						m4ammo[1] = client->ps.ammo[weaponData[WP_M4_ASSAULT_RIFLE].attack[ATTACK_NORMAL].ammoIndex];
						m4ammo[2] = client->ps.clip[ATTACK_NORMAL][WP_M4_ASSAULT_RIFLE];
						m4ammo[3] = client->ps.clip[ATTACK_ALTERNATE][WP_M4_ASSAULT_RIFLE];
						client->ps.ammo[weaponData[WP_M4_ASSAULT_RIFLE].attack[ATTACK_ALTERNATE].ammoIndex] = 0; // empty ammo/clips
						client->ps.ammo[weaponData[WP_M4_ASSAULT_RIFLE].attack[ATTACK_NORMAL].ammoIndex] = 0;
						client->ps.clip[ATTACK_NORMAL][WP_M4_ASSAULT_RIFLE] = 0;
						client->ps.clip[ATTACK_ALTERNATE][WP_M4_ASSAULT_RIFLE] = 0;
						client->ps.stats[STAT_WEAPONS] &= ~(1<<WP_M4_ASSAULT_RIFLE); // remove weapon

						attacker->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_M4_ASSAULT_RIFLE);
						attacker->client->ps.ammo[weaponData[WP_M4_ASSAULT_RIFLE].attack[ATTACK_ALTERNATE].ammoIndex]=m4ammo[0];
						attacker->client->ps.ammo[weaponData[WP_M4_ASSAULT_RIFLE].attack[ATTACK_NORMAL].ammoIndex]=m4ammo[1];
						attacker->client->ps.clip[ATTACK_NORMAL][WP_M4_ASSAULT_RIFLE]=m4ammo[2];
						attacker->client->ps.clip[ATTACK_ALTERNATE][WP_M4_ASSAULT_RIFLE]=m4ammo[3];
						attacker->client->ps.firemode[WP_M4_ASSAULT_RIFLE] = BG_FindFireMode ( WP_M4_ASSAULT_RIFLE, ATTACK_NORMAL, WP_FIREMODE_AUTO );
						attacker->client->ps.weapon = WP_KNIFE;
						attacker->client->ps.weaponstate = WEAPON_READY;
						client->ps.weapon = WP_KNIFE;
						client->ps.weaponstate = WEAPON_READY;
						Com_sprintf(level.M4loc, sizeof(level.M4loc), "%s", attacker->client->pers.netname);
						trap_SendServerCommand(-1, va("print\"^3[H&S] ^7%s ^7has taken the M4 from %s.\n\"", attacker->client->pers.netname, client->pers.netname));
						trap_SendServerCommand(attacker->s.number, va("cp \"^7You have stolen the %sM%s4^7!\n\"", server_color1.string, server_color2.string));
						trap_SendServerCommand(targ->s.number, va("cp \"%s ^7stole your %sM%s4^7!\n\"", attacker->client->pers.netname, server_color1.string, server_color2.string));
						attacker->client->sess.weaponsStolen += 1;
					}
				}
			}
			// Boe!Man 9/20/12: Fix for telefrag not working.
			if(!g_friendlyFire.integer && mod != MOD_TELEFRAG){
				damage = 0;
			}else if(g_friendlyFire.integer && ((attacker->client->ps.weapon == WP_M4_ASSAULT_RIFLE || attacker->client->ps.weapon >= 12) || (mod == 8 || (mod >= 12 && mod <= 21)))){ // Boe!Man 8/10/11: Deal with Friendly fire in H&S.
				damage = 0;
			}
		}
	
		if(level.time > level.gametypeRoundTime && mod != MOD_POP){
			damage = 0;
		}
		 if(mod == MOD_MSG90A1_SNIPER_RIFLE && level.crossTheBridge){
			attack = &weaponData[WP_MSG90A1].attack[ATTACK_NORMAL];
			damage = attack->damage;
		 }else if(mod == MOD_AK74_ASSAULT_RIFLE && level.crossTheBridge){
			attack = &weaponData[WP_AK74_ASSAULT_RIFLE].attack[ATTACK_NORMAL];
			damage = attack->damage;
		 }else if(level.cagefight == qtrue && mod == 265){
			attack = &weaponData[WP_AK74_ASSAULT_RIFLE].attack[ATTACK_ALTERNATE];
			damage = attack->damage;
			if(location == HL_HEAD)
				damage *= 2;
		}
		
		if(mod == MOD_ANM14_GRENADE && attacker->client->sess.team != targ->client->sess.team){
			if(!targ && attacker->client->sess.team != TEAM_BLUE){ // no target so mm1 firenade SO do NOT slowdown blue
				targ->client->sess.slowtime = level.time+1500;
				#ifdef _DEBUG
				Com_Printf("No target..\n");
				#endif
			}else if(targ->client->sess.team == TEAM_RED && attacker->client->sess.team == TEAM_BLUE){
				targ->client->sess.slowtime = level.time+1500;
			}else if(targ->client->sess.team == TEAM_BLUE && attacker->client->sess.team == TEAM_RED){
				targ->client->sess.slowtime = level.time+1500;
			}
		
			damage = 0;
		}
	if(targ->client->sess.team != attacker->client->sess.team && (mod == MOD_RPG7_LAUNCHER || mod == MOD_SMOHG92_GRENADE)){//  || mod == WP_MM1_GRENADE_LAUNCHER)){ // Henk 22/01/10 -> Added better knockback(only on seekers)
		if (targ->client ) 
		{
			knockback = 50; // Henkie
			G_ApplyKnockback ( targ, dir, knockback );
			targ->client->ps.velocity[2] = 230;

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
	}
} // End H&S stuff
if(current_gametype.value == GT_HZ && attacker && targ && mod == MOD_KNIFE){
	if(attacker->client && targ->client){
		if(attacker->client->sess.team == TEAM_BLUE && targ->client->sess.team == TEAM_RED){
			// targ has to die
			//player_die (targ, targ, attacker, 100000, MOD_TEAMCHANGE, HL_NONE, vec3_origin );
			G_Damage (targ, NULL, NULL, NULL, NULL, 10000, 0, MOD_TEAMCHANGE, HL_HEAD);
			CloneBody(attacker, targ->s.number);
			damage = 0;
			trap_SendServerCommand(-1, va("print \"^3[H&Z] ^7%s was zombified by %s.\n\"", targ->client->pers.netname, attacker->client->pers.netname) );
		}else if(attacker->client->sess.team == TEAM_RED && targ->client->sess.team == TEAM_BLUE){
			targ->client->sess.killtime = level.time + 10000; // Boe!Man 7/15/11: Don't allow the zombie to kill himself in the next 10 secs.
			damage = 12;
		}
	}
}

	// check for completely getting out of the damage
	if ( !(dflags & DAMAGE_NO_PROTECTION) ) {

		// if TF_NO_FRIENDLY_FIRE is set, don't do damage to the target
		// if the attacker was on the same team
		if ( targ != attacker && OnSameTeam (targ, attacker)  )
		{
			if ( !g_friendlyFire.integer && !level.cagefight)
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

	if ( damage < 1 && current_gametype.value != GT_HS && current_gametype.value != GT_HZ)
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
	if(current_gametype.value != GT_HS){
	asave = CheckArmor (targ, take, dflags);
	take -= asave;
	}

	//Ryan
	actualtake = Com_Clamp ( 0, targ->health, take );
	//Ryan

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
					/*
					//Ryan june 7 2003
					//admins are not effected by tk damage
					//RxCxW - 03.01.05 - 10:13pm - Decided to let admin have tk but use pBox.. no kick still.
					//if(attacker->client->sess.admin != 1)
					//if(attacker->client->sess.admin < 2) //03.01.05 - 10:13pm 
					{										
						attacker->client->sess.teamkillDamage	   += actualtake;
						attacker->client->sess.teamkillForgiveTime  = level.time;
					}
					//Ryan
					*/
					//RxCxW - 03.01.05 - 10:13pm #pBoxTeamKill
					//Decided to let admin have tk and use pBox to punish abusers
					//Admins will NOT get kicked!
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

		if(attacker->client) // Henk 20/02/11 -> Fix for no lower kills
		targ->client->pers.statinfo.lasthurtby = attacker->s.number;
		if(attacker->client)
		{
			attacker->client->pers.statinfo.lastclient_hurt = targ->s.number;
		}
		//Ryan
	}

	// do the damage
	if (take)
	{
		targ->health = targ->health - take;

		if ( targ->client )
		{
			//Ryan
			//Punish them for killing themselves by not adding to
			//their damagedone
			if(targ == attacker)
			{
				targ->client->pers.statinfo.damageTaken += actualtake;
			}
			////Punish them for killing a teammate and dont add to the
			////victims damage taken
			///RxCxW - 11.01.05 - 06:21am - #DLL barfs on this
			else if(level.gametypeData->teams && OnSameTeam(targ,attacker))
			{
				attacker->client->pers.statinfo.damageDone -= actualtake;
			}
			///RxCxW - 01.08.06 - 09:50pm - all below as added by me
			///else if (attacker && attacker->s.number != ENTITYNUM_WORLD)
			///{
			///Com_Printf("DAMAGEDONE: %i, AT: %i, D+AT: %i\n", attacker->client->pers.statinfo.damageDone, actualtake, attacker->client->pers.statinfo.damageDone += actualtake);
			else if(attacker && attacker->client){
			///else { //RPM 0.6 ELSE
			///End  - 01.10.06 - 02:26pm
				attacker->client->pers.statinfo.damageDone += actualtake;
				targ->client->pers.statinfo.damageTaken += actualtake;
				
			}

			///else
			///{
			///	attacker->client->pers.statinfo.damageDone += actualtake;
			///	targ->client->pers.statinfo.damageTaken += actualtake;
			///}
			///End  - 11.01.05 - 06:21am
			//End - R

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
			targ->die(targ, inflictor, attacker, take, mod, location, dir );
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
	int			entityList[MAX_GENTITIES], entityList1[MAX_GENTITIES];
	int			numListedEntities;
	vec3_t		mins, maxs, mins1, maxs1;
	vec3_t		v;
	vec3_t		dir, rpgAngs, rpgdir;
	int			i, e, a;
	qboolean	hitClient = qfalse;
	qboolean	CageOutOfBoundaries = qfalse;
	qboolean	NadeOutOfBoundaries = qfalse;
	int			ammoindex;
	// Henk 08/02/10 -> Add mm1 firenade
	gentity_t *missile;
	int num;
	// Henk 24/01/11 -> Store hiders caught in a cage
	int countCaught = 0;
	int lastCaught = -1; // store ent number so we can get his name
	// End
	int			index,index1;

	if ( radius < 1 )
	{
		radius = 1;
	}

	for ( i = 0 ; i < 3 ; i++ )
	{
		mins[i] = origin[i] - radius;
		maxs[i] = origin[i] + radius;
	}

	if(mod == 264 && current_gametype.value == GT_HS){ // m4 cage
		for ( i = 0 ; i < 3 ; i++ ) 
		{
			mins1[i] = origin[i] - 90;
			maxs1[i] = origin[i] + 90;
		}
	}
	maxs1[2] += 50;
	//Com_Printf("Mod: %i\n", mod); // HENK DEBUG

	numListedEntities = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );

	for ( e = 0 ; e < numListedEntities ; e++ ) // henk note: Loop through all entities caught in the radius
	{
		memset(entityList1, 0, sizeof(entityList1)); // clean memory where entitylist is stored to prevent problems
		ent = &g_entities[entityList[ e ]];

		if(current_gametype.value == GT_HS){
			
			ent = &g_entities[entityList[ e ]];

			// Henk 14/01/10 -> M4 Cage
			if(mod == 264){ // m4
				if(ent->client){
					if(ent->client->sess.team == TEAM_RED && G_IsClientDead(ent->client) == qfalse){ // check for hiders inside cage
							CageOutOfBoundaries = qtrue;
							break; // break the loop to save performance, no more loops needed because cage already failed no matter what.
					}
					else if(ent->client->sess.team == TEAM_BLUE){ // seeker caught in radius
						CageOutOfBoundaries = qtrue;
						num = trap_EntitiesInBox( mins1, maxs1, entityList1, MAX_GENTITIES );
						for ( a = 0 ; a < num ; a++ ) // Loop until seeker has been found in safe radius
						{	
							if(g_entities[entityList1[ a ]].client && g_entities[entityList1[ a ]].client->sess.team == TEAM_BLUE){
								if(g_entities[entityList1[ a ]].s.number == ent->s.number){ // seeker not at boundary
									CageOutOfBoundaries = qfalse;
									countCaught += 1;
									ent->client->sess.trappedInCage += 1;
									lastCaught = ent->s.number;
									//trap_SendServerCommand(-1, va("print \"^3[H&S] ^7%s was trapped in a cage by %s\n\"", ent->client->pers.netname, attacker->client->pers.netname));
									break; 
								}
							}
						}	// loop
						if(CageOutOfBoundaries == qtrue){
							break;
						}
					}
				}
			}else if(mod == WP_MM1_GRENADE_LAUNCHER){ // Boe!Man 9/2/12: Add score for MM1.
				if(ent->client != NULL && ent->client->sess.team == TEAM_RED){
					ent->client->sess.MM1HitsTaken += 1;
					
					// Boe!Man 11/10/12: Add small knockback for hiders.
					VectorCopy(ent->client->ps.viewangles, rpgAngs);
					AngleVectors( rpgAngs, NULL , NULL, rpgdir );
					rpgdir[0] *= 0.0;
					rpgdir[1] *= 0.0;
					rpgdir[2] = 0.75;
					VectorNormalize ( rpgdir );
					G_ApplyKnockback(ent, rpgdir, 115);
					ent->client->ps.velocity[1] -= 50.0f;
				}
			}
			// End

			// Henk 23/01/10 -> Add	box nade.
			if(mod == WP_MDN11_GRENADE){
				if(ent->client){
					if(ent->client->sess.team == TEAM_RED || ent->client->sess.team == TEAM_BLUE){
						NadeOutOfBoundaries = qtrue;
						break; // break the loop to save performance, no more loops needed because box already failed no matter what.
					}
				}
			}
			// End

			if(mod == WP_L2A2_GRENADE){
				if(ent == attacker){
					Henk_CloseSound(attacker->r.currentOrigin, G_SoundIndex("sound/weapons/rpg7/fire01.mp3"));
					attacker->client->ps.pm_flags |= PMF_JUMPING;
					attacker->client->ps.groundEntityNum = ENTITYNUM_NONE;
					attacker->client->ps.velocity[2] = 450;
				}
			}

			// Henk 18/01/10 -> RPG Boost
			if(mod == WP_RPG7_LAUNCHER){
				if(ent == attacker){ // RPG hits attacker self so boost him
					if(g_RpgStyle.integer == 0){
						attacker->client->ps.pm_flags |= PMF_JUMPING;
						attacker->client->ps.groundEntityNum = ENTITYNUM_NONE;
						attacker->client->ps.velocity[2] = 450;
					}else{
						VectorCopy(attacker->client->ps.viewangles, rpgAngs);
						AngleVectors( rpgAngs, NULL , NULL, rpgdir );	
						rpgdir[0] *= 0.0;
						rpgdir[1] *= 0.0;
						rpgdir[2] = 1.0;
						VectorNormalize ( rpgdir );
						G_ApplyKnockback(attacker, rpgdir, 115);
						attacker->client->ps.velocity[0] *= 0.25f;
						attacker->client->ps.velocity[1] *= 0.25f;
					}
					attacker->client->sess.RPGBoosts += 1;
				}
			}
			// End
		}

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
				//Ryan may 16 2004
				//log the hit into our stats
				statinfo_t *stat = &attacker->client->pers.statinfo;

				//Fire nades only count a hit if they kill
				if(mod != MOD_ANM14_GRENADE && (!level.gametypeData->teams || (level.gametypeData->teams && !OnSameTeam ( attacker, ent ))))
				{
					stat->hitcount++;
					stat->accuracy = (float)stat->hitcount / (float)stat->shotcount * 100;

					if(mod - 256 == MOD_M4_ASSAULT_RIFLE)
					{
						stat->weapon_hits[ATTACK_ALTERNATE][WP_M4_ASSAULT_RIFLE]++;
					}
					else
					{
						stat->weapon_hits[ATTACK_NORMAL][mod]++;
					}
				}
				//Ryan

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

	if(current_gametype.value == GT_HZ){
			if(mod == WP_M67_GRENADE){

				G_CreateDamageArea ( origin, attacker, 0.00, radius, 30000, mod );
			}
	}

	if(current_gametype.value == GT_HS){
		if(mod == 264){
			if(CageOutOfBoundaries == qtrue){
				ammoindex=weaponData[WP_M4_ASSAULT_RIFLE].attack[ATTACK_ALTERNATE].ammoIndex;
				if(g_cageAttempts.integer != 0){
					if(attacker->client->sess.cageAttempts < 3){
						attacker->client->ps.ammo[ammoindex]+=1;
						attacker->client->sess.cageAttempts += 1;
						trap_SendServerCommand(attacker-g_entities, va("print \"^3[Info] ^7M4 cage failed %i of %i: Seeker at boundary or hider caught in cage.\n\"", attacker->client->sess.cageAttempts, g_cageAttempts.integer)); 
					}else{
						trap_SendServerCommand(attacker-g_entities, va("print \"^3[Info] ^7M4 cage failed too many times: not adding ammo.\n\"")); 
					}
				}else{
					attacker->client->ps.ammo[ammoindex]+=1;
					trap_SendServerCommand(attacker-g_entities, va("print \"^3[Info] ^7Seeker at boundary or hider caught in cage.\n\"")); 
				}
			}else{
				if(lastCaught != -1 && countCaught == 1){
					trap_SendServerCommand(-1, va("print \"^3[H&S] ^7%s was trapped in a cage by %s\n\"", g_entities[lastCaught].client->pers.netname, attacker->client->pers.netname));
				}else if(countCaught > 1){
					trap_SendServerCommand(-1, va("print \"^3[H&S] ^7%i seekers were trapped in a cage by %s\n\"", countCaught, attacker->client->pers.netname));
				}
				attacker->client->sess.seekersCaged += 1;
				// Check if ammo is empty
				index = weaponData[WP_M4_ASSAULT_RIFLE].attack[ATTACK_ALTERNATE].ammoIndex;
				index1 = weaponData[WP_M4_ASSAULT_RIFLE].attack[ATTACK_NORMAL].ammoIndex;
				if(attacker->client->ps.ammo[index] == 0 && attacker->client->ps.ammo[index1] == 0 && attacker->client->ps.clip[ATTACK_NORMAL][WP_M4_ASSAULT_RIFLE] == 0 && attacker->client->ps.clip[ATTACK_ALTERNATE][WP_M4_ASSAULT_RIFLE] == 0){
					attacker->client->ps.clip[ATTACK_NORMAL][WP_M4_ASSAULT_RIFLE] = 0;
					attacker->client->ps.clip[ATTACK_ALTERNATE][WP_M4_ASSAULT_RIFLE] = 0;
					attacker->client->ps.stats[STAT_WEAPONS] &= ~(1<<WP_M4_ASSAULT_RIFLE);
					attacker->client->ps.weapon = WP_KNIFE;
					attacker->client->ps.weaponstate = WEAPON_READY;
					Com_sprintf(level.M4loc, sizeof(level.M4loc), "%s", "Disappeared");
					trap_SendServerCommand(-1, va("print\"^3[H&S] ^7M4 has disappeared\n\""));
				}
				// End
				SpawnCage(origin, attacker, qfalse, qfalse);
			}
		}
		if(mod == MOD_F1_GRENADE || mod == 272){ // Boe!Man 8/2/12: Fix for Altattack of tele nade not doing anything.
			if(origin[2] <= attacker->r.currentOrigin[2]){
				static vec3_t	mins = {-15,-15,-45};
				static vec3_t	maxs = {15,15,46};
				vec3_t			org1, org2;
				trace_t			tr;
				tr.fraction = 0.0f;
				VectorCopy(origin, org1);
				VectorCopy(origin, org2);
				org1[2] += 50;
				trap_Trace ( &tr, org1, mins, maxs, org2, attacker->s.number, MASK_ALL); //MASK_SOLID
				if ( !tr.startsolid && !tr.allsolid )
				{
				DoTeleport(attacker, origin);
				}else{
					ammoindex=weaponData[WP_F1_GRENADE].attack[ATTACK_ALTERNATE].ammoIndex;
					attacker->client->ps.ammo[ammoindex]+=1;

					if (!(attacker->client->ps.stats[STAT_WEAPONS] & (1<<WP_F1_GRENADE))){ // Boe!Man 8/22/11: Make sure the attacker has the weapon, if not, re-add it (fixes bug which made weapon disappear on last throw).
						attacker->client->ps.stats[STAT_WEAPONS] |= (1 << WP_F1_GRENADE);
					}
					
					trap_SendServerCommand(attacker-g_entities, va("print \"^3[Info] ^7Surface is not empty.\n\""));
				}
			}else{
				ammoindex=weaponData[WP_F1_GRENADE].attack[ATTACK_ALTERNATE].ammoIndex;
				attacker->client->ps.ammo[ammoindex]+=1;

				if (!(attacker->client->ps.stats[STAT_WEAPONS] & (1<<WP_F1_GRENADE))){ // Boe!Man 8/22/11: Make sure the attacker has the weapon, if not, re-add it (fixes bug which made weapon disappear on last throw).
					attacker->client->ps.stats[STAT_WEAPONS] |= (1 << WP_F1_GRENADE);
				}

				trap_SendServerCommand(attacker-g_entities, va("print \"^3[Info] ^7Surface is too high.\n\""));
			}
		}

		if(mod == WP_MDN11_GRENADE || mod == 274){ // Boe!Man 8/2/12: Fix for Altattack of box nade not doing anything.
			if(NadeOutOfBoundaries == qtrue){
				ammoindex=weaponData[WP_MDN11_GRENADE].attack[ATTACK_ALTERNATE].ammoIndex;
				if(g_boxAttempts.integer != 0){
					if(attacker->client->sess.mdnAttempts < 3){
						attacker->client->sess.mdnAttempts += 1;
						attacker->client->ps.ammo[ammoindex]+=1;

						if (!(attacker->client->ps.stats[STAT_WEAPONS] & (1<<WP_MDN11_GRENADE))){ // Boe!Man 8/22/11: Make sure the attacker has the weapon, if not, re-add it (fixes bug which made weapon disappear on last throw).
							attacker->client->ps.stats[STAT_WEAPONS] |= (1 << WP_MDN11_GRENADE);
						}
						
						trap_SendServerCommand(attacker-g_entities, va("print \"^3[Info] ^7MDN box failed %i of %i: You cannot throw a box at a person.\n\"", attacker->client->sess.mdnAttempts, g_boxAttempts.integer)); 
					}else{
						trap_SendServerCommand(attacker-g_entities, va("print \"^3[Info] ^7MDN box failed too many times: not adding ammo.\n\"")); 
					}
				}else{
					attacker->client->ps.ammo[ammoindex]+=1;
					trap_SendServerCommand(attacker-g_entities, va("print \"^3[Info] ^7You cannot throw a box at a person.\n\"")); 
				}
			}else{
				SpawnBox(origin);
			}
		}else if(mod == WP_MM1_GRENADE_LAUNCHER){
			missile = NV_projectile( attacker, origin, dir, WP_ANM14_GRENADE, 0 );
			missile->nextthink = level.time + 250;
		}
	}

	return hitClient;
}
