#include "g_local.h"
#include "boe_local.h"

void RPM_ReadyCheck (gentity_t *ent)
{
#ifdef _SOF2_BOTS
	if (ent->r.svFlags & SVF_BOT)
	{
		if(!ent->client->pers.ready)
		{
			ent->client->pers.ready = 1;
		}
		return;
	}
#endif
	if(level.time < ent->client->pers.readyMessageTime + 3000)
	{
		return;
	}

	if(g_doWarmup.integer == 2 && !ent->client->pers.ready)
	{
		G_Broadcast("You are not ready for the match\nPlease type ^ 1 / ready ^ 7in console", BROADCAST_GAME, ent);
		ent->client->pers.readyMessageTime = level.time;
	}
}

void RPM_ReadyUp (gentity_t *ent)
{
	if(ent->client->pers.ready)
	{
		return;
	}

	if(level.warmupTime > -1)
	{
		return;
	}

	ent->client->pers.ready = 1;
	trap_SendServerCommand( ent-g_entities, va("cp \"@ \n\"")); //send a blank message to clear the readyup message
}
/*
================
RPM_ReadyAll
================
*/
void RPM_ReadyAll (void)
{
	int i;
	gentity_t *ent;

	for ( i = 0; i < level.numPlayingClients; i ++ )
	{
		ent = &g_entities[level.sortedClients[i]];
		ent->client->pers.ready = 1;
	}
}


/*
============
RPM_Unpause
============
*/
void RPM_Unpause (gentity_t *adm)
{
	int			i;
	gentity_t	*ent;
//	gentity_t	*tent;

	if(!level.pause)
	{
		if(adm && adm->client)
			trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7The game is not currently paused.\n\"") );
		else
			Com_Printf("The game is not currently paused.\n");
		
		return;
	}

	if(level.time >= level.unpausetime + 1000)
	{
		level.unpausetime = level.time;
		level.pause--;

		G_Broadcast(va("\\Resuming in: %d sec", level.pause), BROADCAST_GAME, NULL);

		Boe_GlobalSound(G_SoundIndex("sound/misc/events/buzz02.wav"));

		if(!level.pause)
		{
			level.unpausetime = 0;
			
			// Boe!Man 4/22/12: When in CTF, flags should be unfreezed as well. Let the gt know this by sending this command.
			if(current_gametype.value == GT_CTF){
				trap_GT_SendEvent ( GTEV_PAUSE, level.time, 0, 0, 0, 0, 0);
			}

			///RxCxW - 08.30.06 - 03:06pm #reset clients (scoreboard) display time
			trap_SetConfigstring( CS_LEVEL_START_TIME, va("%i", level.startTime ) );
			trap_SetConfigstring ( CS_GAMETYPE_TIMER, va("%i", level.gametypeRoundTime) );
			///End  - 08.30.06 - 03:06pm

			if( level.gametypeData->respawnType == RT_INTERVAL)
			{
				level.gametypeRespawnTime[TEAM_RED]  = level.time + g_respawnInterval.integer * 1000;
				level.gametypeRespawnTime[TEAM_BLUE] = level.time + g_respawnInterval.integer * 1000;
				level.gametypeRespawnTime[TEAM_FREE] = level.time + g_respawnInterval.integer * 1000;
			}
			
			for (i=0 ; i< level.maxclients ; i++)
			{
				ent = g_entities + i;
				if (!ent->inuse)
				{
					continue;
				}
				if (!ent->client)
				{
					continue;
				}
				
				if(ent->client->sess.pausespawn == qtrue){
					ent->client->ps.RealSpawnTimer = level.time+ent->client->ps.oldTimer;
				}
				ent->client->ps.pm_type = PM_NORMAL;
			}
			G_Broadcast("\\Go!", BROADCAST_CMD, NULL);
		}
	}
}

void RPM_UpdateLoadScreenMessage (void)
{
	char	*ammo, *damage;

	switch (g_weaponModFlags.integer)
	{
		case 1:
			ammo = "ON";
			damage = "OFF";
			break;
		case 2:
			ammo = "OFF";
			damage = "ON";
			break;
		case 3:
			ammo = "ON";
			damage = "ON";
			break;
		default:
			ammo = "OFF";
			damage = "OFF";
	}

	trap_SetConfigstring( CS_MESSAGE, va(" Modified Damage: %s  Modified Ammo: %s\n", damage, ammo));
}

/*
================
RPM_Refresh
================
*/
void RPM_Refresh(gentity_t *ent)
{
	if(level.time <= level.gametypeStartTime+5000){
		trap_SendServerCommand (ent->s.number, "print\"^3[Info] ^7You shouldn't refresh at the start of a new round.\n\"" );
		return;
	}else if (current_gametype.value == GT_HZ){
		if (ent->client->sess.killtime > level.time){ // Boe!Man 7/15/11: He's not allowed to kill himself. Inform him and return.
			if ((ent->client->sess.killtime - level.time) > 1000){ // Boe!man 7/15/11: The counter should never show 0.. Just kill if it's actually under 1.
				trap_SendServerCommand(ent->client - &level.clients[0], va("print\"^3[H&Z] ^7You cannot refresh yourself for another %i second(s).\n\"", (ent->client->sess.killtime - level.time) / 1000));
				return;
			}
		}
	}
	#ifdef _DEBUG
	Com_Printf("%i - %i\n", level.time, level.gametypeStartTime);
	#endif
	if (!G_IsClientSpectating ( ent->client ) )
	{
		ent->flags &= ~FL_GODMODE;
		ent->client->ps.stats[STAT_HEALTH] = ent->health = -999;
		player_die (ent, ent, ent, 100000, MOD_REFRESH, HL_NONE, vec3_origin );
	}
	
	memset(&ent->client->pers.statinfo, 0, sizeof(ent->client->pers.statinfo));
	ent->client->pers.enterTime = level.time;
	ent->client->ps.persistant[PERS_SCORE] = 0;
	ent->client->sess.score = 0;
	ent->client->sess.deaths = 0;
	ent->client->sess.kills = 0;
	ent->client->sess.killsAsZombie = 0;
	ent->client->sess.teamkillDamage      = 0;
	ent->client->sess.teamkillForgiveTime = 0;
	ent->client->pers.statinfo.lastclient_hurt = -1;
	ent->client->pers.statinfo.lasthurtby = -1;
}

void RPM_Obituary ( gentity_t *target, gentity_t *attacker, int mod, attackType_t attack, int hitLocation ) 
{
	int				targ, killer, gender, attackt, weapon;
	char			targetName[64];
	char			killerName[64];
	const char		*targetColor;
	const char		*killerColor;
	char			*message;
	char			*message2;
	char			*message3;
	qboolean		headShot = qfalse;
	qboolean		statOk = qfalse;
	statinfo_t		*atrstat = &attacker->client->pers.statinfo;
//	gentity_t *tent;

	if(!attacker)
		return;
	if(!target)
		return;
	if(!attacker->client)
		return;
	killerColor = S_COLOR_WHITE;
	targetColor   = S_COLOR_WHITE;
	message2 = "";
	message3 = "";
	///RxCxW - 01.08.06 - 09:14pm
	///attackt = attacker->client->pers.statinfo.attack;
	///weapon = attacker->client->pers.statinfo.weapon;
	//G_LogPrintf("Starting Obituary..\n");
	if(!level.gametypeData->teams || (level.gametypeData->teams && !OnSameTeam ( target, attacker )))
	{
		statOk = qtrue;
	}

	//was the kill hit a HEADSHOT?
	hitLocation = hitLocation & (~HL_DISMEMBERBIT);
	if (hitLocation == HL_HEAD)// && statOk)
	{
		//trap_SendServerCommand( -1, va("print \"^3[Debug]^7Headshot\n\""));
		message3 = "{^3HeaDShoT^7}";
		attackt = attacker->client->pers.statinfo.attack;
		weapon = attacker->client->pers.statinfo.weapon;
		//Boe_ClientSound(attacker, G_SoundIndex("sound/npc/col8/blakely/niceshot.mp3"));
		//trap_SendServerCommand( attacker->s.number, va("cp \"Headshot!\n\""));//g_headShotMessage.string));
		headShot = qtrue;
		//add to the total headshot count for this player
		atrstat->headShotKills++;
		atrstat->weapon_headshots[attackt][weapon]++;
	}

	targ = target->s.number;

	if ( targ < 0 || targ >= MAX_CLIENTS ) 
	{
		G_LogPrintf("Out of range..\n");
		Com_Error( ERR_FATAL, "RPM_Obituary: target out of range" );
	}
	Q_strncpyz( targetName, target->client->pers.netname, sizeof(targetName));
	strcat( targetName, S_COLOR_WHITE );

	//find out who or what killed the client
	if ( attacker->client ) 
	{
		killer = attacker->s.number;
		Q_strncpyz( killerName, attacker->client->pers.netname, sizeof(killerName));
		strcat( killerName, S_COLOR_WHITE );
	}	
	else
	{
		killer = ENTITYNUM_WORLD;
	}
	
	// Play the death sound, water if they drowned
	if ( mod == MOD_WATER )
	{
		Boe_ClientSound(target, G_SoundIndex("sound/pain_death/mullins/drown_dead.mp3"));
	}
	else		//play a random death sound out of 3 possible sounds
	{
		switch(level.time % 3)
		{
		case 0:
			Boe_ClientSound(target, G_SoundIndex("sound/pain_death/male/die01.mp3"));
			break;

		case 1:
			Boe_ClientSound(target, G_SoundIndex("sound/pain_death/male/die04.mp3"));
			break;

		case 2:
			Boe_ClientSound(target, G_SoundIndex("sound/pain_death/male/die05.mp3"));
			break;

		default:
			Boe_ClientSound(target, G_SoundIndex("sound/pain_death/male/die01.mp3"));
			break;
		}
	}
	// Play the frag sound, and make sure its not played more than every 250ms
	if ( attacker->client)// && level.time - attacker->client->lastKillTime > 250 )
	{
		//If the attacker killed themselves play the selffrag sound
		if ( killer == targ )
		{
			Boe_ClientSound(target, G_SoundIndex("sound/self_frag.mp3"));
		}else if(headShot){
			//if they use rpm client-side the client will 
			//handle the sound etc...
			if(attacker->client->sess.rpmClient)
			{
				trap_SendServerCommand( attacker->s.number, va("headshot \"Headshot\n\""));
				Boe_ClientSound(attacker, G_SoundIndex("sound/npc/col8/blakely/niceshot.mp3"));
			}
			//if not we'll send them the sound etc..
			else// if(g_allowDeathMessages.integer)
			{
				G_Broadcast("Headshot!", BROADCAST_GAME, attacker);
				Boe_ClientSound(attacker, G_SoundIndex("sound/npc/col8/blakely/niceshot.mp3"));
			}
			
			//if(g_allowDeathMessages.integer)
			//{
				//if we can show kills we'll display the 
				//heashot message with the "you killed" message
				if ( level.gametypeData->showKills )
				{	
					message2 = G_ColorizeMessage("Headshot!");
				}

				//We'll tack this on to the end of the kill broadcast to all
				message3 = "{^3HeaDShoT^7}";
			//}
			
		}
		//if not headshot, suicide or tk just play the normal sound
		else
		{	
			Boe_ClientSound(attacker, G_SoundIndex("sound/frag.mp3"));
		}
		
		//set the time here now 
		attacker->client->lastKillTime = level.time;
	}	
	//set the teamcolor of the killed client
	switch ( target->client->sess.team )
	{
		case TEAM_RED:
			targetColor = S_COLOR_RED;
			break;

		case TEAM_BLUE:
			targetColor = S_COLOR_BLUE;
			break;
	}

	gender = GENDER_MALE;

	if ( strstr ( target->client->pers.identity->mCharacter->mModel, "female" ) )
	{
		gender = GENDER_FEMALE;
	}
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
		case MOD_CAR:
			message = "was killed in a terrible car accident";
			break;
		case MOD_POP:
			return;
//RxCxW - 1.13.2005	- Dugup (Unplant) #MOD
		case MOD_DUGUP:
			message = "Looks like someone Dug too deep!";
			break;
		case MOD_BURN:
			message = "was BURNT to a Crisp!";
			break;
//End
		default:
			message = NULL;
			break;
	}
	// Attacker killed themselves.  Ridicule them for it.
	if (killer == targ) 
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

			case MOD_REFRESH:
				message = "Refreshed";
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
		trap_SendServerCommand( -1, va("print \"%s%s %s.\n\"", targetColor, targetName, message));
		return;
	}
	// check for kill messages
	if ( level.gametypeData->showKills )
	{
		if ( attacker && attacker->client ) 
		{
			//if not a team game display the kill and the rank
			if ( !level.gametypeData->teams ) 
			{

				G_Broadcast(va("%sYou killed %s%s\n%s place with %i\n\"", // %s after \n
					message2,
					targetColor,
					targetName, 
					G_PlaceString( attacker->client->ps.persistant[PERS_RANK] + 1 ),
					attacker->client->ps.persistant[PERS_SCORE]
				), BROADCAST_GAME, attacker);
			} 
			else //if team just display the kill
			{
				G_Broadcast(va("%sYou killed %s%s\n\"", message2, targetColor, targetName), BROADCAST_GAME, attacker);
			}
		}
	}
	// check for double client messages
	message2 = "";

	///RxCxW - 01.08.06 - 09:22pm - Used if/else if instead of switch which gave errors in dll game
		//switch ( attacker->client->sess.team )
		//{
		//	case TEAM_RED:
		//		killerColor = S_COLOR_RED;
		//		break;

		//	case TEAM_BLUE:
		//		killerColor = S_COLOR_BLUE;
		//		break;
		//}
	///End  - 01.08.06 - 09:31pm
			
	if ( killer != ENTITYNUM_WORLD ) 
	{
		///RxCxW - 01.08.06 - 09:31pm
		switch ( attacker->client->sess.team )
		{
			case TEAM_RED:
				killerColor = S_COLOR_RED;
				break;

			case TEAM_BLUE:
				killerColor = S_COLOR_BLUE;
				break;
		}
		///End  - 01.08.06 - 09:31pm

		switch (mod) 
		{
			case MOD_KNIFE:
				message = "was sliced by";
				if(statOk)
				{
					atrstat->knifeKills++;
				}
				break;

			case MOD_USAS_12_SHOTGUN:
			case MOD_M590_SHOTGUN:
				if ( attack == ATTACK_ALTERNATE )
				{
					message = "was bludgeoned by";
					message2 = va("'s %s", weaponParseInfo[mod].mName );
				}
				else
				{
					message = "was pumped full of lead by";
					message2 = va("'s %s", weaponParseInfo[mod].mName );
				}
				break;

			case MOD_M1911A1_PISTOL:
			case MOD_USSOCOM_PISTOL: 
				if ( attack == ATTACK_ALTERNATE )
				{
					message = "was pistol whipped by";
					message2 = va("'s %s", weaponParseInfo[mod].mName );
				}
				else
				{
					message = "was shot by";
					message2 = va("'s %s", weaponParseInfo[mod].mName );
				}
				break;

			case MOD_AK74_ASSAULT_RIFLE:
				if ( attack == ATTACK_ALTERNATE )
				{
					message = "was stabbed by";
					message2 = va("'s %s", weaponParseInfo[mod].mName );
				}
				else
				{
					message = "was shot by";
					message2 = va("'s %s", weaponParseInfo[mod].mName );
				}
				break;

			case MOD_M4_ASSAULT_RIFLE:
				if ( attack == ATTACK_ALTERNATE )
				{
					message = "was detonated by";
					message2 = "'s M203";

					if(statOk)
					{
						atrstat->explosiveKills++;
					}
				}
				else
				{
					message = "was shot by";
					message2 = va("'s %s", weaponParseInfo[mod].mName );
				}
				break;

			case MOD_M60_MACHINEGUN:
			case MOD_MICRO_UZI_SUBMACHINEGUN:
			case MOD_M3A1_SUBMACHINEGUN:
				message = "was shot by";
				message2 = va("'s %s", weaponParseInfo[mod].mName );
				break;

			case MOD_MSG90A1_SNIPER_RIFLE:    
				message = "was sniped by";
				message2 = va("'s %s", weaponParseInfo[mod].mName );
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
				if (current_gametype.value != GT_HZ){
					message2 = va("'s %s", weaponParseInfo[mod].mName);
				}else{
					if (mod == WP_L2A2_GRENADE || mod == altAttack(WP_L2A2_GRENADE)){
						message2 = "'s Claymore";
					}else{
						message2 = va("'s %s", weaponParseInfo[mod].mName);
					}
				}
				if(statOk)
				{
					if(mod == MOD_ANM14_GRENADE)
					{
						atrstat->hitcount++;
						atrstat->accuracy = (float)atrstat->hitcount / (float)atrstat->shotcount * 100;
						atrstat->weapon_hits[ATTACK_NORMAL][mod]++; // MIGHT BE BUG
					}

					atrstat->explosiveKills++;
				}
				break;

			case MOD_TELEFRAG:
				message = "was telefragged by";
				//message = "tried to invade";
				//message2 = "'s personal space";
				break;

			default:
				message = "was killed by";
				break;
		}

		if (message) {
			trap_SendServerCommand( -1, va("print \"%s%s %s %s%s%s %s\n\"", targetColor, targetName, message, killerColor, killerName, message2, message3));
			return;
		}
	}

	// we don't know what it was
	//G_LogPrintf("Done with RPM_Obituary..\n");
	trap_SendServerCommand( -1, va("print \"%s%s died.\n\"", targetColor, targetName ));
}

// Henk 07/04/10 -> Copied from RPM to make scoreboard from RPM client working
/*
=================	
RPM_UpdateTMI

//Used to only send teammate info hence "TMI" but I have decided to
//send all player infos so specs can see health etc.. of both teams
//and its also kinda cool in deathmatches to see the health of the enemy
=================
*/
void RPM_UpdateTMI(void)
{
	int			i, j;
	char		entry[1024];
	char		infoString[2048];
	int			infoStringLength;
	gclient_t	*cl;
	int			numAdded, location;
	gentity_t	*bestLoc;
	int			adm = 0;
	char		*s;
	char		userinfo[MAX_INFO_STRING];
	int			damage;
	char		*string;
	int			UpdateTime;
/*	if (!level.gametypeData->teams)
	{	// only bother if a league game with teams
		return;
	}
*/
	// if we are waiting for the level to restart, do nothing, even if forced
	if (level.restarted){
		return;
	}

	if(level.pause)
		return;

	if(level.numConnectedClients >= 0 && level.numConnectedClients <= 10)
		UpdateTime = 2000;
	else if(level.numConnectedClients >= 11 && level.numConnectedClients <= 20)
		UpdateTime = 3000;
	else if(level.numConnectedClients > 20)
		UpdateTime = 4000;
	else
		UpdateTime = 2000;

	if (level.time - level.lastTMIupdate < UpdateTime) // Henk 06/04/10 -> Increase to reduce lagg
	{
		return;
	}

	level.lastTMIupdate = level.time;

	entry[0] = 0;
	infoString[0] = 0;
	infoStringLength = 0;
	numAdded = 0;
	
	for (i = 0; i < level.numConnectedClients; i++)
	{
		cl = &level.clients[level.sortedClients[i]];
		
		//IsClientMuted(&g_entities[cl->ps.clientNum], qfalse);
		IsClientMuted(&g_entities[level.sortedClients[i]], qfalse);
		if (G_IsClientSpectating(cl))// || G_IsClientDead (cl)) 
		{
			continue;
		}

		bestLoc = Team_GetLocation(&g_entities[level.sortedClients[i]]);

		location = 0;
		if (bestLoc)
		{
			location = bestLoc->health;
		}
		// Henk 06/04/10 -> Check what view person is in
		trap_GetUserinfo( g_entities[level.sortedClients[i]].s.number, userinfo, sizeof( userinfo ) );
		if(g_entities[level.sortedClients[i]].client->sess.rpmClient > 0.7){
		s = Info_ValueForKey( userinfo, "cg_thirdperson" );
		cl->sess.thirdperson = atoi(s);
		if(cl->sess.thirdperson >= 1){
			cl->sess.thirdperson = 1;
		}else{
			cl->sess.thirdperson = 0;
		}
		}else{
			cl->sess.thirdperson = 2;
		}
			#ifdef _awesomeToAbuse
			if (cl->sess.admin == 2){
				adm = 3;
			}else if(cl->sess.admin >= 3){
				adm = 4;
			}else{
				adm = 0;
			}
			#else
			adm = cl->sess.admin;
			#endif // _awesomeToAbuse
			damage = cl->pers.statinfo.damageDone;
			if(damage < 100){
				string = va("%i", cl->ps.weapon);
			}else{
				damage = FormatDamage(damage);
				if(cl->ps.weapon >= 10){ // fix for large weapon string = damage *10 on scoreboard
					damage = damage/10;
				}
				string = va("%i%i", damage, cl->ps.weapon);	
			}
			
			//if(cl->sess.rpmClient > 0.6){
			Com_sprintf (entry, sizeof(entry),
				" %i %i %i %i %i %i %s %i %i",
				level.sortedClients[i],
				cl->ps.stats[STAT_HEALTH],
				cl->ps.stats[STAT_ARMOR],
				location,
				cl->sess.thirdperson, // 1 = third | 0 = first | 2 = n/a, aka no client
				adm,
				string,
				//cl->ps.weapon, //cl->pers.statinfo.damageDone
				cl->sess.mute,		// Confirmed
				cl->sess.clanMember	// Confirmed
				);
			/*}else if(cl->sess.proClient > 4.0){
			Com_sprintf (entry, sizeof(entry),
				" %i %i %i %i %i %i %s %i %i",
				level.sortedClients[i],
				cl->ps.stats[STAT_HEALTH],
				cl->ps.stats[STAT_ARMOR],
				cl->ps.weapon,
				location,
				adm
				);
			}*/

		j = strlen(entry);

		if (infoStringLength + j > 2046)
		{
			break;
		}
		strcpy (infoString + infoStringLength, entry);
		infoStringLength += j;
		numAdded++;
	}
	for (i = 0 ; i < level.numConnectedClients; i++)
	{
		cl = &level.clients[level.sortedClients[i]];
		
		// Boe!Man 6/5/13: Don't send if the player is timing out..
		if(cl->ps.ping >= 999)
		{
			continue;
		}

//RxCxW - 1.20.2005 - #scoreboard #Version compatiblity
		if(cl->sess.rpmClient > 0.6)	
			trap_SendServerCommand(level.sortedClients[i], va("tmi %i%s", numAdded, infoString));
		else if(cl->sess.proClient > 4.0)
			trap_SendServerCommand(level.sortedClients[i], va("tmi %i%s", numAdded, infoString));
	}
}

/*
=============
RPM_Awards
=============
*/
void RPM_Awards(void)
{
	static int overallScore = 0, headshots = 0, damage = 0, explosiveKills = 0, knifeKills = 0;
	static float  accuracy = 0, ratio = 0;
	unsigned int i, numPlayers = 0;
	//I changed this to an integer so players who connect
	//faster than the rest don't bump up the avg. time
	//float avgtime = 0.0, playerTime = 0.0;
	int avgtime = 0, playerTime = 0;
	statinfo_t     *stat;
	// Boe!Man 2/20/13: No need to be static again, the struct already is.
	gentity_t *bestOverall = NULL, *headshooter = NULL, *killer = NULL;
	gentity_t *accurate = NULL, *bestRatio = NULL, *explosive = NULL, *knifer = NULL;
	gentity_t *ent;
	
	/*
	Boe!Man 2/20/13: Oh noooess this is crappy. ;_; But there's no other fix for dynlib it seems.
	Structure:
	[0] - bestOverall
	[1] - headshooter
	[2] - killer
	[3] - accurate
	[4] - bestRatio
	[5] - explosive
	[6] - knifer
	*/
	typedef struct {
		char name[MAX_NETNAME];
		int  number;
	}scores_t;
	static scores_t		bestScores[7];

	if(!level.awardTime)
	{
		//find the average time player by all connected clients
		for (i=0; i < level.maxclients ; i++)
		{
			ent = g_entities + i;
			if (!ent->inuse)
			{
				continue;
			}
			numPlayers++;
			avgtime += ((level.time - ent->client->pers.enterTime) - ent->client->sess.totalSpectatorTime) / 60000;
		}
		//incase timelimit runs out with nobody in the server
		//I'll use an "if" here
		if(numPlayers)
		{
			avgtime /= numPlayers;
			//Com_Printf("^3Averagetime %d\n", avgtime);
		}

		for (i = 0; i < level.maxclients ; i++)
		{
			ent = g_entities + i;
			if (!ent->inuse)
			{
				continue;
			}

			stat = &ent->client->pers.statinfo;

			playerTime = ((level.time - ent->client->pers.enterTime) - ent->client->sess.totalSpectatorTime) / 60000;
			//Com_Printf("%s playtime %d, Level time: %d, Enter Time: %d SPECTIME: %d\n", ent->client->pers.netname, playerTime, level.time, ent->client->pers.enterTime, ent->client->sess.totalSpectatorTime);

			//RxCxW - 1.20.2005 - #Version 0.5 compatible. Right?
			if(ent->client->sess.rpmClient >= 0.5)
//			if(ent->client->sess.rpmClient >= RPM_VERSION)
			{
				trap_SendServerCommand( i, va("stats %i %i %i %i %.2f %i %i %i %i",
				playerTime,
				(((level.time - ent->client->pers.enterTime) - ent->client->sess.totalSpectatorTime) % 60000) / 1000,
				stat->damageDone,
				stat->damageTaken,
				stat->ratio,
				stat->shotcount,
				stat->hitcount,
				stat->explosiveKills,
				stat->knifeKills));
			}

			//this will remove the SPECTATOR Press ESC etc.. from the specs screen
			if(ent->client->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR)
			{
				ent->client->ps.persistant[PERS_TEAM] = TEAM_FREE;
			}

			//Make sure they can't move
			ent->client->ps.pm_type = PM_FREEZE;

			//This will remove the HUD icons etc.. from the players screen
			ent->client->ps.stats[STAT_HEALTH] = -1;

			stat->overallScore = ent->client->sess.score + (int)(100 * (stat->accuracy + stat->ratio)) + (stat->damageDone - stat->damageTaken);

			if(stat->overallScore > overallScore)
			{
				overallScore = stat->overallScore;
				bestOverall = ent;
			}
			else if(stat->overallScore == overallScore && overallScore)
			{
				//if they got the same amout of points in less time
				//make them mvp
				if(ent->client->pers.enterTime > bestOverall->client->pers.enterTime)
				{
					overallScore = stat->overallScore;
					bestOverall = ent;
				}
			}

			if(stat->headShotKills > headshots)
			{
				headshots = stat->headShotKills;
				headshooter = ent;
			}
			else if(stat->headShotKills == headshots && headshots)
			{
				if(ent->client->pers.enterTime > headshooter->client->pers.enterTime)
				{
					headshots = stat->headShotKills;
					headshooter = ent;
				}
			}

			if(stat->damageDone > damage)
			{
				damage = stat->damageDone;
				killer = ent;
			}
			else if(stat->damageDone == damage && damage)
			{
				if(ent->client->pers.enterTime > killer->client->pers.enterTime)
				{
					damage = stat->damageDone;
					killer = ent;
				}
			}

			if(stat->explosiveKills > explosiveKills)
			{
				explosiveKills = stat->explosiveKills;
				explosive = ent;
			}
			else if(stat->explosiveKills == explosiveKills && explosiveKills)
			{
				if(ent->client->pers.enterTime > explosive->client->pers.enterTime)
				{
					explosiveKills = stat->explosiveKills;
					explosive = ent;
				}
			}

			if(stat->knifeKills > knifeKills )
			{
				knifeKills = stat->knifeKills;
				knifer = ent;
			}
			else if(stat->knifeKills == knifeKills && knifeKills)
			{
				if(ent->client->pers.enterTime > knifer->client->pers.enterTime)
				{
					knifeKills = stat->knifeKills;
					knifer = ent;
				}
			}

			//The awards below here are time sensitive so players
			//with 1/0 for ratio etc.. dont get the award
			if(playerTime < avgtime)
			{
				continue;
			}

			if(stat->accuracy > accuracy)
			{
				accuracy = stat->accuracy;
				accurate = ent;
			}
			else if(stat->accuracy == accuracy && accuracy)
			{
				//if this player held the accuracy longer give him the award
				if(ent->client->pers.enterTime < accurate->client->pers.enterTime)
				{
					accuracy = stat->accuracy;
					accurate = ent;
				}
			}

			if(stat->ratio > ratio)
			{
				ratio = stat->ratio;
				bestRatio = ent;
			}
			else if(stat->ratio == ratio && ratio)
			{
				if(ent->client->pers.enterTime < bestRatio->client->pers.enterTime)
				{
					ratio = stat->ratio;
					bestRatio = ent;
				}
			}
		}
		
		// Boe!Man 2/20/13: Put data in struct. Nasty piece of code but effective.
		if(bestOverall){
			Q_strncpyz(bestScores[0].name, g_entities[bestOverall->s.number].client->pers.netname, sizeof(bestScores[0].name));
			bestScores[0].number = bestOverall->s.number;
		}else{
			strcpy(bestScores[0].name, "None");
			bestScores[0].number = -1;
		}
		if(headshooter){
			Q_strncpyz(bestScores[1].name, g_entities[headshooter->s.number].client->pers.netname, sizeof(bestScores[1].name));
			bestScores[1].number = headshooter->s.number;
		}else{
			strcpy(bestScores[1].name, "None");
			bestScores[1].number = -1;
		}
		if(killer){
			Q_strncpyz(bestScores[2].name, g_entities[killer->s.number].client->pers.netname, sizeof(bestScores[2].name));
			bestScores[2].number = killer->s.number;
		}else{
			strcpy(bestScores[2].name, "None");
			bestScores[2].number = -1;
		}
		if(accurate){
			Q_strncpyz(bestScores[3].name, g_entities[accurate->s.number].client->pers.netname, sizeof(bestScores[3].name));
			bestScores[3].number = accurate->s.number;
		}else{
			strcpy(bestScores[3].name, "None");
			bestScores[3].number = -1;
		}
		if(bestRatio){
			Q_strncpyz(bestScores[4].name, g_entities[bestRatio->s.number].client->pers.netname, sizeof(bestScores[4].name));
			bestScores[4].number = bestRatio->s.number;
		}else{
			strcpy(bestScores[4].name, "None");
			bestScores[4].number = -1;
		}
		if(explosive){
			Q_strncpyz(bestScores[5].name, g_entities[explosive->s.number].client->pers.netname, sizeof(bestScores[5].name));
			bestScores[5].number = explosive->s.number;
		}else{
			strcpy(bestScores[5].name, "None");
			bestScores[5].number = -1;
		}
		if(knifer){
			Q_strncpyz(bestScores[6].name, g_entities[knifer->s.number].client->pers.netname, sizeof(bestScores[6].name));
			bestScores[6].number = knifer->s.number;
		}else{
			strcpy(bestScores[6].name, "None");
			bestScores[6].number = -1;
		}
	}
	
	for (i = 0; i < level.maxclients ; i++)
	{
		ent = g_entities + i;

		if (!ent->inuse)
		{
			continue;
		}

		///RxCxW - 01.12.06 - 03:03pm - dont send to bots
		if (ent->r.svFlags & SVF_BOT)
			continue;
		///End  - 01.12.06 - 03:03pm

		if(ent->client->sess.rpmClient < 0.5)
		{
			G_Broadcast(va("^1Best Overall: ^7%s - %i\n^1Headshots: ^7%s - %i\n^1Killer: ^7%s - %i\n^1Accurate: ^7%s - %.2f percent\n^1Best Ratio: ^7%s - %.2f\n^1Nades: ^7%s - %i detonated\n^1Knifer: ^7%s - %i shanked",
				bestScores[0].name,
				overallScore,
				bestScores[1].name,
				headshots,
				bestScores[2].name,
				damage,
				bestScores[3].name,
				accuracy,
				bestScores[4].name,
				ratio,
				bestScores[5].name,
				explosiveKills,
				bestScores[6].name,
				knifeKills ), BROADCAST_AWARDS, ent);
			continue;
		}
		if(!level.awardTime)
		{
			trap_SendServerCommand( i, va("awards %i %i %i %i %i %i %i %.2f %i %.2f %i %i %i %i", 
				bestScores[0].number,
				overallScore,
				bestScores[1].number,
				headshots, 
				bestScores[2].number,
				damage, 
				bestScores[3].number,
				accuracy,
				bestScores[4].number,
				ratio, 
				bestScores[5].number,
				explosiveKills,	
				bestScores[6].number,
				knifeKills 
				));
		}
	}
}


/*
================
G_AdjustClientBBox
================
*/
void G_AdjustClientBBox(gentity_t *other)
{
	other->client->minSave[0] = other->r.mins[0];
	other->client->minSave[1] = other->r.mins[1];
	other->client->minSave[2] = other->r.mins[2];

	other->client->maxSave[0] = other->r.maxs[0];
	other->client->maxSave[1] = other->r.maxs[1];
	other->client->maxSave[2] = other->r.maxs[2];

	// Adjust the hit box to account for hands and such 
	// that are sticking out of the normal bounding box
	if (other->client->ps.pm_flags & PMF_LEANING)
	{
		other->r.maxs[0] *= 3.0f;
		other->r.maxs[1] *= 3.0f;
		other->r.mins[0] *= 3.0f;
		other->r.mins[1] *= 3.0f;
		other->r.svFlags |= SVF_DOUBLED_BBOX;
	}

	// Relink the entity into the world
	trap_LinkEntity (other);
}


/*
================
G_AdjustClientBBoxs

Inflates every clients bbox to take leaning/jumping etc into account this frame.
This is all very nasty really...
================
*/
void G_AdjustClientBBoxs(void)
{
	int i;

	// Move all the clients back into the reference clients time frame.
	for (i = 0; i < level.numConnectedClients; i++)
	{
		gentity_t* other = &g_entities[level.sortedClients[i]];

		if (other->client->pers.connected != CON_CONNECTED)
		{
			continue;
		}

		// Skip entities not in use
		if (!other->inuse)
		{
			continue;
		}

		// Skip clients that are spectating
		if (G_IsClientSpectating(other->client) || G_IsClientDead(other->client))
		{
			continue;
		}

		G_AdjustClientBBox(other);
	}	
}



/*
================
G_UndoAdjustedClientBBox
================
*/
void G_UndoAdjustedClientBBox(gentity_t *other)
{
	// Put the hitbox back the way it was
	other->r.maxs[0] = other->client->maxSave[0];
	other->r.maxs[1] = other->client->maxSave[1];
	other->r.maxs[2] = other->client->maxSave[2];

	other->r.mins[0] = other->client->minSave[0];
	other->r.mins[1] = other->client->minSave[1];
	other->r.mins[2] = other->client->minSave[2];

	other->r.svFlags &= (~SVF_DOUBLED_BBOX);
}


/*
================
G_UndoAdjustedClientBBoxs
================
*/
void G_UndoAdjustedClientBBoxs(void)
{
	int i;

	for ( i = 0; i < level.numConnectedClients; i ++ )
	{
		gentity_t* other = &g_entities[level.sortedClients[i]];
		
		if ( other->client->pers.connected != CON_CONNECTED )
		{
			continue;
		}

		// Skip clients that are spectating
		if ( G_IsClientSpectating ( other->client ) || G_IsClientDead ( other->client ) )
		{
			continue;
		}

		G_UndoAdjustedClientBBox(other);

		// Relink the entity into the world
		trap_LinkEntity ( other );
	}
}

/*
================
G_SetClientPreLeaningBBox
================
*/
void G_SetClientPreLeaningBBox(gentity_t *ent)
{
	//Ryan
	//ent->r.maxs[2] += g_adjPLBMaxsZ.value;
	ent->r.maxs[2] += PLB_MAXZ;
	//Ryan
}

/*
================
G_SetClientLeaningBBox
================
*/

void G_SetClientLeaningBBox(gentity_t *ent)
{
	float	leanOffset;
	vec3_t	up;
	vec3_t	right;
	
	// adjust origin for leaning
	//BG_ApplyLeanOffset(&ent->client->ps, org);//ent->r.currentOrigin);
	leanOffset = (float)(ent->client->ps.leanTime - LEAN_TIME) / LEAN_TIME * LEAN_OFFSET;
	
	//Ryan
	//leanOffset *= g_adjLeanOffset.value;
	leanOffset *= BBOX_LEAN_OFFSET;
	//Ryan

	AngleVectors( ent->client->ps.viewangles, NULL, right, up);
	VectorMA(ent->r.currentOrigin, leanOffset, right, ent->r.currentOrigin);
	VectorMA(ent->r.currentOrigin, Q_fabs(leanOffset) * -0.20f, up, ent->r.currentOrigin);

	//Ryan
	//ent->r.maxs[2] += g_adjLBMaxsZ.value;
	ent->r.maxs[2] += LB_MAXZ;
	//Ryan
	
	if (ent->client->ps.pm_flags & PMF_DUCKED)
	{
		//Ryan
		//ent->r.mins[2] += g_adjDuckedLBMinsZ.value;
		ent->r.mins[2] += DUCKED_LB_MINZ;
		//Ryan
	}
	else
	{
		//Ryan
		//ent->r.mins[2] += g_adjLBMinsZ.value;
		ent->r.mins[2] += LB_MINZ;
		//Ryan
	}
}

/*
===================
G_PlaceString	//stole code from CG_Placestring
===================
*/
const char	*G_PlaceString( int rank ) {
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
