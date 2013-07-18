// Copyright (C) 2010 - Boe!Man.
//
// boe_admcmds.c - All the Admin Functions & commands for the Game Module go here.

#include "g_local.h"
#include "boe_local.h"
void G_GlassDie ( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod, int hitLocation, vec3_t hitDir );
// --- BOE PROTOTYPE WARNINGS FIXES ---
// I'm lazy so I just 'fix' them here. I'll fix them fo'real later.
// ALREADY DONE LAZY BITCH -.-'' HENK
// Boe!Man 2/6/13: Whoah! This prototype warning fix comment actually CAUSED a warning!
/*
==========
GetArgument
==========
*/

int GetArgument(int argNum){
	char	arg[16] = "\0"; // increase buffer so we can process more commands
	int		num;
	int i;
	trap_Argv( argNum, arg, sizeof( arg ) );
		num = 0;
		for(i=0;i<16;i++){
			if(arg[i] == ' '){
			num = atoi(va("%c%c%c", arg[i+1], arg[i+2], arg[i+3]));
			break;
			}
		}
		if(num == 0){
			if(strstr(arg, "0")){ // Boe!Man 12/14/10: The first arg wasn't empty, thus the purpose must've been to set it to 0.
				return num;
			}else{ // Get second argument because they use it from the console
				trap_Argv( 2, arg, sizeof( arg ) );
				num = atoi(arg);
			}
			if(num == 0){ // Boe!Man 12/14/10: If it's still 0, check if indeed the argument was nothing, if so, return -1.
				if(strstr(arg, "0")){
					return num;
				}
				else{
					return -1;
				}
			}
		}
	return num;
}

/*
==========
Boe_CompMode
==========
*/

void Boe_CompMode(int argNum, gentity_t *ent, qboolean shotCmd){
	// Boe!Man 1/19/11: Disable Compmode in H&S. -- Update 1/8/12: And of course in H&Z.
	if(current_gametype.value == GT_HS || current_gametype.value == GT_HZ){
		if(ent && ent->client){
			trap_SendServerCommand(ent-g_entities, va("print\"^3[Info] ^7You cannot enable Competition Mode in this gametype.\n\""));
		}else{
			Com_Printf("^7You cannot enable Competition Mode in this gametype.\n");
		}
		return;
	}
	
	// Boe!Man 1/3/12: Disable Compmode when a user is changing maps.
	if(level.mapAction){
        if(level.mapAction == 1 || level.mapAction == 3){
            Com_Printf("^3[Info] ^7A map restart is already in progress.\n");
        }else if(level.mapAction == 2 || level.mapAction == 4){
            Com_Printf("^3[Info] ^7A map switch is already in progress.\n");
        }else{
            Com_Printf("^3[Info] ^7Something appears to be wrong. Please report to an developer using this error code: 2L\n");
        }
        return;
    }
    
	if(g_compMode.integer == 0){
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sC%so%sm%sp%se%stition mode enabled!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		if(ent && ent->client){
			trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Competition mode enabled by %s.\n\"", ent->client->pers.netname));
			Boe_adminLog ("CompMode Enabled", va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName), "none");
		}else{
			trap_SendServerCommand( -1, va("print \"^3[Rcon Action] ^7Competition mode enabled.\n\""));
			Boe_adminLog ("CompMode Enabled", va("RCON"), "none");
		}
		trap_Cvar_Set("g_compMode", "1");
		// Boe!Man 11/16/10: Set the temporary CVARs.
		trap_Cvar_Set("cm_enabled", "1");
		trap_Cvar_Set("cm_sl", g_matchScoreLimit.string);
		trap_Cvar_Set("cm_tl", g_matchTimeLimit.string);
		trap_Cvar_Set("cm_slock", g_matchLockSpec.string);
		trap_Cvar_Set("cm_aswap", g_matchSwapTeams.string);
		trap_Cvar_Set("cm_devents", g_matchDisableEvents.string);
		if(g_matchRounds.integer <= 1){
			trap_Cvar_Set("cm_dr", "0");
		}else if(g_matchRounds.integer >= 2){
			trap_Cvar_Set("cm_dr", "1");
		}
		// Boe!Man 11/16/10: Set the current timelimit in the temp CVARs so we can restore them after the scrim has ended.
		trap_Cvar_Set("cm_oldsl", g_scorelimit.string);
		trap_Cvar_Set("cm_oldtl", g_timelimit.string);
		level.compMsgCount = level.time + 3000;
		//Boe_adminLog (va("%s - TIMELIMIT %i", ent->client->pers.cleanName, number)) ;
	}else{
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sC%so%sm%sp%se%stition mode disabled!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		if(ent && ent->client){
			trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Competition mode disabled by %s.\n\"", ent->client->pers.netname));
			Boe_adminLog ("CompMode Disabled", va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName), "none");
		}else{
			trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Competition mode disabled.\n\""));
			Boe_adminLog ("CompMode Disabled", va("RCON"), "none");
		}
		trap_Cvar_Set("g_compMode", "0");
		trap_Cvar_Set("cm_enabled", "0");
		trap_Cvar_Set("cm_devents", "0"); // Boe!Man 4/20/11: It's used for more things than just in a scrim, so make sure to reset it!
		trap_Cvar_Set("scorelimit", cm_oldsl.string);
		trap_Cvar_Set("timelimit", cm_oldtl.string);
	}
	trap_Cvar_Update(&g_compMode);
	trap_Cvar_Update(&cm_sl);
	trap_Cvar_Update(&cm_tl);
	trap_Cvar_Update(&g_scorelimit);
	trap_Cvar_Update(&g_timelimit);
}

/*
==========
Boe_Third
==========
*/

void Boe_Third(int argNum, gentity_t *ent, qboolean shortCmd){
	if(g_allowthirdperson.integer == 0){
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sT%sh%si%sr%sd%sperson enabled!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		trap_Cvar_Set("g_allowthirdperson", "1");
		if(ent && ent->client){
			trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Thirdperson enabled by %s.\n\"", ent->client->pers.netname));
			Boe_adminLog ("3rd Enabled", va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName), "none");
		}else{
			trap_SendServerCommand( -1, va("print \"^3[Rcon Action] ^7Thirdperson enabled.\n\""));
			Boe_adminLog ("3rd Enabled", va("RCON"), "none");
		}
	}else{
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sT%sh%si%sr%sd%sperson disabled!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		trap_Cvar_Set("g_allowthirdperson", "0");
		if(ent && ent->client){
			trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Thirdperson disabled by %s.\n\"", ent->client->pers.netname));
			Boe_adminLog ("3rd Disabled", va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName), "none");
		}else{
			trap_SendServerCommand( -1, va("print \"^3[Rcon Action] ^7Thirdperson disabled.\n\""));
			Boe_adminLog ("3rd Disabled", va("RCON"), "none");
		}
	}
}

/*
==========
Boe_GametypeRestart
==========
*/

void Boe_GametypeRestart(int argNum, gentity_t *ent, qboolean shortCmd){
	trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sG%sa%sm%se%st%sype restart!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
	if(ent && ent->client){
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Gametype restart by %s.\n\"", ent->client->pers.netname));
		Boe_adminLog ("Gametype Restart", va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName), "none");
	}else{
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7Gametype restart.\n\""));
		Boe_adminLog ("Gametype Restart", va("RCON"), "none");
	}
	trap_SendConsoleCommand( EXEC_APPEND, va("gametype_restart\n"));
}

/*
==========
Boe_Normaldamage
==========
*/

void Boe_NormalDamage(int argNum, gentity_t *ent, qboolean shortCmd){
	int i;

	// Boe!Man 1/19/11: Disable ND in H&S.
	if(current_gametype.value == GT_HS){
		if(ent && ent->client){
			trap_SendServerCommand(ent-g_entities, va("print\"^3[Info] ^7You cannot switch to Normal Damage in Hide&Seek.\n\""));
		}else{
			Com_Printf("^7You cannot switch to Normal Damage in Hide&Seek.\n");
		}
		return;
	}

	g_instaGib.integer = 0;
	RPM_WeaponMod ();
	//BG_InitWeaponStats(qfalse);
	
	for(i=0;i<level.numConnectedClients;i++){
		level.clients[level.sortedClients[i]].noOutfittingChange = qfalse;
		G_UpdateOutfitting(g_entities[level.sortedClients[i]].s.number);
		//ammoindex = weaponData[WP_KNIFE].attack[ATTACK_ALTERNATE].ammoIndex;
		//ammoData[ammoindex].max = 5; 
	}
	trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sN%so%sr%sm%sa%sl damage!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
	if(ent && ent->client){
		trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Normal damage by %s.\n\"", ent->client->pers.netname));
		Boe_adminLog ("Normal Damage", va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName), "none");
	}else{
		trap_SendServerCommand( -1, va("print \"^3[Rcon Action] ^7Normal damage.\n\""));
		Boe_adminLog ("Normal Damage", va("RCON"), "none");
	}
}

/*
==========
Boe_Realdamage
==========
*/

void Boe_RealDamage(int argNum, gentity_t *ent, qboolean shortCmd){
	int i;

	// Boe!Man 1/19/11: Disable RD in H&S.
	if(current_gametype.value == GT_HS){
		if(ent && ent->client){
			trap_SendServerCommand(ent-g_entities, va("print\"^3[Info] ^7You cannot switch to Real Damage in Hide&Seek.\n\""));
		}else{
			Com_Printf("^7You cannot switch to Real Damage in Hide&Seek.\n");
		}
		return;
	}

	g_instaGib.integer = 1;
	RPM_WeaponMod ();
	//BG_InitWeaponStats(qfalse);
	for(i=0;i<level.numConnectedClients;i++){
		level.clients[level.sortedClients[i]].noOutfittingChange = qfalse;
		G_UpdateOutfitting(g_entities[level.sortedClients[i]].s.number);
	}
	trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sR%se%sa%sl %sd%samage!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
	if(ent && ent->client){
		trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Real damage by %s.\n\"", ent->client->pers.netname));
		Boe_adminLog ("Real Damage", va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName), "none");
	}else{
		trap_SendServerCommand( -1, va("print \"^3[Rcon Action] ^7Real damage.\n\""));
		Boe_adminLog ("Real Damage", va("RCON"), "none");
	}
}

/*
==========
Boe_RespawnInterval
==========
*/

void Boe_RespawnInterval(int argNum, gentity_t *ent, qboolean shortCmd){
	int number;
	char arg1[6];

	// Boe!Man 3/16/11: Fix for 'respawninterval' in RCON, /adm and shortCmd in console.
	if(shortCmd){
		number = GetArgument(argNum);
	}else if(ent&&ent->client && !shortCmd){
		trap_Argv( 2, arg1, sizeof( arg1 ) );
		if(strlen(arg1) > 0){
			number = atoi(arg1);
		}else{
			number = -1;
		}
	}
	else if(!ent){
		trap_Argv( 1, arg1, sizeof( arg1 ) );
		if(strlen(arg1) > 0){
			number = atoi(arg1);
		}else{
			number = -1;
		}
	}
	if(number < 0){
		// Boe!Man 1/14/11: Show current respawn interval if there's no arg, or if 'respawninterval' is called from RCON.
		if(ent && ent->client){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Respawn Interval is %i.\n\"", g_respawnInterval.integer));
		}else{
			Com_Printf("g_respawnInterval is %i.\n", g_respawnInterval.integer);
		}
		return;
	}else if(number < 100){ // Boe!Man 3/13/11: Don't allow more than 100 second respawn interval.
		trap_SendConsoleCommand( EXEC_APPEND, va("g_respawnInterval %i\n", number));
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sR%se%ss%sp%sa%swn interval %i!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, number));
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		if(ent&&ent->client){
			Boe_adminLog (va("Respawn Interval %i", number), va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName), "none");
			trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Respawn interval changed to %i by %s.\n\"", number, ent->client->pers.netname));
		}else{
			Boe_adminLog (va("Respawn Interval %i", number), va("RCON"), "none");
			trap_SendServerCommand( -1, va("print \"^3[Rcon Action] ^7Respawn interval changed to %i.\n\"", number));
		}
	}
}

/*
==========
Boe_Timelimit
==========
*/

void Boe_TimeLimit(int argNum, gentity_t *ent, qboolean shortCmd){ // Boe!Man 1/14/11: This function can only be called from /adm or ! (interacting CVAR). Hence no logging or message broadcasting will be enabled for this function.
	int number;
	number = GetArgument(argNum);
	if(number < 0){
		// Boe!Man 12/14/10: Show current (match) timelimit if there's no arg.
		if (g_compMode.integer > 0 && cm_enabled.integer >= 1){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Match timelimit is %i.\n\"", cm_tl.integer));
		}
		else{
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Timelimit is %i.\n\"", g_timelimit.integer));
		}
		return;
	}
	// Boe!Man 11/16/10: Is Competition Mode active so we can update the scrim setting vs. the global one?
	if (g_compMode.integer > 0 && cm_enabled.integer >= 1){
		if (cm_enabled.integer == 1){
			trap_Cvar_Set("cm_tl", va("%i", number));
			trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Match timelimit changed to %i by %s.\n\"", number, ent->client->pers.netname));
		}else if(cm_enabled.integer > 1 && cm_enabled.integer < 5){
			trap_Cvar_Set("cm_tl", va("%i", number));
			trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Match timelimit changed to %i by %s.\n\"", number, ent->client->pers.netname));
			Boe_setTrackedCvar(16, number); // Boe!Man 6/30/11: Set the timelimit CVAR without track message, to avoid the [Rcon Action] message.
		}
	}else{
		Boe_setTrackedCvar(16, number); // Boe!Man 6/30/11: Set the timelimit CVAR without track message, to avoid the [Rcon Action] message.
		trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Timelimit changed to %i by %s.\n\"", number, ent->client->pers.netname));
	}
	trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sT%si%sm%se%sl%simit %i!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, number));
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
	Boe_adminLog (va("Timelimit %i", number), va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName), "none");
}

/*
==========
Boe_ScoreLimit
==========
*/

void Boe_ScoreLimit(int argNum, gentity_t *ent, qboolean shortCmd){ // Boe!Man 1/14/11: This function can only be called from /adm or ! (interacting CVAR). Hence no logging or message broadcasting will be enabled for this function.
	int number;
	number = GetArgument(argNum);
	if(number < 0){
		// Boe!Man 12/14/10: Show current (match) scorelimit if there's no arg.
		if (g_compMode.integer > 0 && cm_enabled.integer >= 1){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Match scorelimit is %i.\n\"", cm_sl.integer));
		}
		else{
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Scorelimit is %i.\n\"", g_scorelimit.integer));
		}
		return;
	}
	// Boe!Man 11/16/10: Is Competition Mode active so we can update the scrim setting vs. the global one?
	if (g_compMode.integer > 0 && cm_enabled.integer >= 1){
		if (cm_enabled.integer == 1){
			trap_Cvar_Set("cm_sl", va("%i", number));
			trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Match scorelimit changed to %i by %s.\n\"", number, ent->client->pers.netname));
		}else if(cm_enabled.integer > 1 && cm_enabled.integer < 5){
			trap_Cvar_Set("cm_sl", va("%i", number));
			trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Match scorelimit changed to %i by %s.\n\"", number, ent->client->pers.netname));
			Boe_setTrackedCvar(15, number); // Boe!Man 6/30/11: Set the scorelimit CVAR without track message, to avoid the [Rcon Action] message.
		}
	}else{
		Boe_setTrackedCvar(15, number); // Boe!Man 6/30/11: Set the scorelimit CVAR without track message, to avoid the [Rcon Action] message.
		trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Scorelimit changed to %i by %s.\n\"", number, ent->client->pers.netname));
	}
	trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sS%sc%so%sr%se%slimit %i!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, number));
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
	Boe_adminLog (va("Scorelimit %i", number), va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName), "none");
}

/*
==========
SetNades
Recode by Boe: 5/22/12 - 9:25 PM
Instead of forcing everything to on/off, check available weapons as well.
==========
*/
qboolean SetNades(char *status){
	int weapon;
	char	 available[WP_NUM_WEAPONS+1];
	qboolean one = qfalse; // Boe!Man 9/20/12: If at least ONE nade is enabled, this is qtrue (also return value).
	
	strncpy(available, availableWeapons.string, WP_NUM_WEAPONS);
	
	if(strstr(status, "0")){ // Manage internally, so we check for such strings. 0 means enable, so in H&S we check that CVAR.
		if(current_gametype.value == GT_HS){
			if(level.crossTheBridge){
				// Boe!Man 6/6/13: CTB has different settings (only smoke is enabled).
				trap_Cvar_Set("disable_pickup_weapon_SMOHG92", "1");
				trap_Cvar_Set("disable_pickup_weapon_M84", "1");
				trap_Cvar_Set("disable_pickup_weapon_M15", "0");
				trap_Cvar_Set("disable_pickup_weapon_AN_M14", "1");
			}else{
				if(hideSeek_Nades.string[0] == '0')
					trap_Cvar_Set("disable_pickup_weapon_SMOHG92", "1");
				else
					trap_Cvar_Set("disable_pickup_weapon_SMOHG92", "0");
				if(hideSeek_Nades.string[1] == '0')
					trap_Cvar_Set("disable_pickup_weapon_M84", "1");
				else
					trap_Cvar_Set("disable_pickup_weapon_M84", "0");
				if(hideSeek_Nades.string[2] == '0')
					trap_Cvar_Set("disable_pickup_weapon_M15", "1");
				else
					trap_Cvar_Set("disable_pickup_weapon_M15", "0");
				if(hideSeek_Nades.string[3] == '0')
					trap_Cvar_Set("disable_pickup_weapon_AN_M14", "1");
				else
					trap_Cvar_Set("disable_pickup_weapon_AN_M14", "0");
			}
		}else{ // If not H&S, check other gametypes. Since we might not want to enable all nades (it should respect availablenades CVAR), check for it.
				for (weapon = WP_M67_GRENADE; weapon < WP_NUM_WEAPONS; weapon ++){
					gitem_t* item = BG_FindWeaponItem ( (weapon_t)weapon );
					if (!item){
						continue;
					}
					
					if(available[weapon-1] == '1' || available[weapon-1] == '2'){
						trap_Cvar_Set ( va("disable_%s", item->classname), "0" );
						one = qtrue; // Boe!Man 9/20/12: No need to do this in H&S, they can't use !nonades.
					}else{
						trap_Cvar_Set ( va("disable_%s", item->classname), "1" );
					}
				}
		}
	}else{ // Status is 0. Easy, disable everything.
		trap_Cvar_Set("disable_pickup_weapon_M84", status);
		trap_Cvar_Set("disable_pickup_weapon_SMOHG92", status);
		trap_Cvar_Set("disable_pickup_weapon_AN_M14", status);
		trap_Cvar_Set("disable_pickup_weapon_M67", status);
		trap_Cvar_Set("disable_pickup_weapon_F1", status);
		trap_Cvar_Set("disable_pickup_weapon_L2A2", status);
		trap_Cvar_Set("disable_pickup_weapon_MDN11", status);
		trap_Cvar_Set("disable_pickup_weapon_M15", status);
	}
	
	G_UpdateAvailableWeapons(); // also set the original g_availableWeapons for the client :)
	
	return one;
}

/*
==========
Boe_NoNades
==========
*/

void Boe_NoNades(int argNum, gentity_t *ent, qboolean shortCmd){
	int i;

	// Boe!Man 1/19/11: Disable NN in H&S.
	if(current_gametype.value == GT_HS){
		if(ent && ent->client){
			trap_SendServerCommand(ent-g_entities, va("print\"^3[Info] ^7You cannot enable/disable Nades in Hide&Seek.\n\""));
		}else{
			Com_Printf("You cannot enable/disable Nades in Hide&Seek.\n");
		}
		return;
	}else if(current_gametype.value == GT_HZ){ // Boe!Man 9/20/12: And H&Z.
		if(ent && ent->client){
			trap_SendServerCommand(ent-g_entities, va("print\"^3[Info] ^7You cannot enable/disable Nades in Humans&Zombies.\n\""));
		}else{
			Com_Printf("You cannot enable/disable Nades in Humans&Zombies.\n");
		}
		return;
	}else if(!level.nadesFound){
		if(ent && ent->client){
			trap_SendServerCommand(ent-g_entities, va("print\"^3[Info] ^7No nades are set to be used (availableWeapons CVAR).\n\""));
		}else{
			Com_Printf("^7No nades are set to be used (availableWeapons CVAR).\n");
		}
		return;
	}

	if(g_disableNades.integer == 1){
		g_disableNades.integer = 0;
		trap_Cvar_Set("g_disableNades", "0");
		trap_Cvar_Update(&g_disableNades);
	    SetNades("0");
		BG_SetAvailableOutfitting(g_availableWeapons.string);
			for(i=0;i<level.numConnectedClients;i++){
				level.clients[level.sortedClients[i]].noOutfittingChange = qfalse;
				#ifdef _DEBUG
				Com_Printf("Setting nades\n");
				#endif
				//level.clients[level.sortedClients[i]].ps.stats[STAT_OUTFIT_GRENADE] = bg_itemlist[bg_outfittingGroups[OUTFITTING_GROUP_GRENADE][3]].giTag;
				G_UpdateOutfitting(g_entities[level.sortedClients[i]].s.number);
				//level.clients[level.sortedClients[i]].ps.ammo[weaponData[WP_SMOHG92_GRENADE].attack[ATTACK_NORMAL].ammoIndex]=1;
				//level.clients[level.sortedClients[i]].ps.stats[STAT_WEAPONS] |= ( 1 << WP_SMOHG92_GRENADE );
				//level.clients[level.sortedClients[i]].ps.clip[ATTACK_NORMAL][WP_SMOHG92_GRENADE]=1;
			}
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sN%sa%sd%se%ss %senabled!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			if(ent && ent->client){
				trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Nades enabled by %s.\n\"", ent->client->pers.netname));
				Boe_adminLog ("Nades Enabled", va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName), "none");
			}else{
				trap_SendServerCommand( -1, va("print \"^3[Rcon Action] ^7Nades enabled.\n\""));
				Boe_adminLog ("Nades Enabled", va("RCON"), "none");
			}
	}else{
		g_disableNades.integer = 1;
		trap_Cvar_Set("g_disableNades", "1");
		trap_Cvar_Update(&g_disableNades);
		SetNades("1");
		BG_SetAvailableOutfitting(g_availableWeapons.string);
		for(i=0;i<level.numConnectedClients;i++){
			level.clients[level.sortedClients[i]].noOutfittingChange = qfalse;
			G_UpdateOutfitting(g_entities[level.sortedClients[i]].s.number);
		}
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sN%sa%sd%se%ss %sdisabled!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		if(ent && ent->client){
			trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Nades disabled by %s.\n\"", ent->client->pers.netname));
			Boe_adminLog ("Nades Disabled", va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName), "none");
		}else{
			trap_SendServerCommand( -1, va("print \"^3[Rcon Action] ^7Nades disabled.\n\""));
			Boe_adminLog ("Nades Disabled", va("RCON"), "none");
		}
	}
}

/*
==========
Boe_NoLower
==========
*/

void Boe_NoLower(int argNum, gentity_t *ent, qboolean shortCmd){
	trap_Cvar_VariableStringBuffer ( "mapname", level.mapname, MAX_QPATH );
	// Boe!Man 2/27/11: If people don't want to use NoLower they can specify to disable it.
	if(g_useNoLower.integer <= 0){
		if(ent && ent->client){
			trap_SendServerCommand(ent-g_entities, va("print\"^3[Info] ^7Nolower has been disabled on this server.\n\""));
		}else{
			Com_Printf("Nolower has been disabled on this server.\n");
		}
		return;
	}
	// Boe!Man 1/8/12: If people want to use nolower but if there's no such entity found, inform the user.
	if(!level.nolower2){
		if(ent && ent->client){
			trap_SendServerCommand(ent-g_entities, va("print\"^3[Info] ^7No entity found to toggle nolower.\n\""));
		}else{
			Com_Printf("No entity found to toggle nolower.\n");
		}
		return;
	}

	if(level.nolower1 == qtrue){
		level.nolower1 = qfalse;
		//trap_Cvar_Set("g_disablelower", "0");
		//trap_Cvar_Update(&g_disablelower);
		if (strstr(level.mapname, "mp_kam2")){
			RemoveFence();
		}
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sN%so%sl%so%sw%ser disabled!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		if(ent && ent->client){
			trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Nolower disabled by %s.\n\"", ent->client->pers.netname));
			Boe_adminLog ("Nolower Disabled", va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName), "none");
		}else{
			trap_SendServerCommand( -1, va("print \"^3[Rcon Action] ^7Nolower disabled.\n\""));
			Boe_adminLog ("Nolower Disabled", va("RCON"), "none");
		}
	}else{
		level.nolower1 = qtrue;
		//trap_Cvar_Set("g_disablelower", "1");
		//trap_Cvar_Update(&g_disablelower);
		if (strstr(level.mapname, "mp_kam2")){
			SpawnFence(1);
			SpawnFence(2);
			SpawnFence(3);
			SpawnFence(4);
		}
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sN%so%sl%so%sw%ser enabled!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		if(ent && ent->client){
			trap_SendServerCommand(-1, va("print \"^3[Admin Action] ^7Nolower enabled by %s.\n\"", ent->client->pers.netname));
			Boe_adminLog ("Nolower Enabled", va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName), "none");
		}else{
			trap_SendServerCommand(-1, va("print \"^3[Rcon Action] ^7Nolower enabled.\n\""));
			Boe_adminLog ("Nolower Enabled", va("RCON"), "none");
		}
	}
}

/*
==========
Boe_NoRoof
==========
*/

void Boe_NoRoof(int argNum, gentity_t *ent, qboolean shortCmd){
	// Boe!Man 2/27/11: If people don't want to use noroof they can specify to disable it.
	if(g_useNoRoof.integer <= 0){
		if(ent && ent->client){
			trap_SendServerCommand(ent-g_entities, va("print\"^3[Info] ^7Noroof has been disabled on this server.\n\""));
		}else{
			Com_Printf("Noroof has been disabled on this server.\n");
		}
		return;
	}
	// Boe!Man 1/8/12: If people want to use noroof but if there's no such entity found, inform the user.
	if(!level.noroof2){
		if(ent && ent->client){
			trap_SendServerCommand(ent-g_entities, va("print\"^3[Info] ^7No entity found to toggle noroof.\n\""));
		}else{
			Com_Printf("No entity found to toggle noroof.\n");
		}
		return;
	}

	if(level.noroof1 == qtrue){
		level.noroof1 = qfalse;
		
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sN%so%sr%so%so%sf disabled!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		if(ent && ent->client){
			trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Noroof disabled by %s.\n\"", ent->client->pers.netname));
			Boe_adminLog ("Noroof Disabled", va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName), "none");
		}else{
			trap_SendServerCommand( -1, va("print \"^3[Rcon Action] ^7Noroof disabled.\n\""));
			Boe_adminLog ("Noroof Disabled", va("RCON"), "none");
		}
	}else{
		level.noroof1 = qtrue;
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sN%so%sr%so%so%sf enabled!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		if(ent && ent->client){
			trap_SendServerCommand(-1, va("print \"^3[Admin Action] ^7Noroof enabled by %s.\n\"", ent->client->pers.netname));
			Boe_adminLog ("Noroof Enabled", va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName), "none");
		}else{
			trap_SendServerCommand(-1, va("print \"^3[Rcon Action] ^7Noroof enabled.\n\""));
			Boe_adminLog ("Noroof Enabled", va("RCON"), "none");
		}
	}
}

/*
==========
Boe_MapRestart
==========
*/

void Boe_MapRestart(int argNum, gentity_t *ent, qboolean shortCmd){
	// Boe!Man 11/2/10: New Map Switch/Restart system. -- Optimized on 1/13/11.
	//trap_SendConsoleCommand( EXEC_APPEND, va("map_restart 5\n"));
	if(ent && ent->client){
		if(level.mapSwitch == qfalse){
		level.mapSwitch = qtrue;
		level.mapAction = 1;
		level.mapSwitchCount = level.time;
		level.mapSwitchCount2 = 5; // Boe!Man 7/22/12: 5 seconds remaining on the timer.
		if (g_compMode.integer == 0){
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Map restarted by %s.\n\"", ent->client->pers.netname));
			//trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%sM%sa%sp %sr%se%sstart in 5!", level.time + 1000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		}else if (g_compMode.integer > 0 && cm_enabled.integer == 1){
			if(cm_dr.integer == 1){ // Boe!Man 3/18/11: If dual rounds are enabled, the first round would be started.
				trap_SendServerCommand(-1, va("print \"^3[Info] ^7First round started.\n\""));
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sF%si%sr%ss%st round started!", level.time + 3000, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
				level.compMsgCount = -1;
			}else{ // Boe!Man 3/18/11: If not, general message.
				trap_SendServerCommand(-1, va("print \"^3[Info] ^7Match started.\n\""));
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sM%sa%st%sc%sh started!", level.time + 3000, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
				level.compMsgCount = -1;
			}
		}else if (g_compMode.integer > 0 && cm_enabled.integer == 3){
			trap_SendServerCommand(-1, va("print \"^3[Info] ^7Second round started.\n\""));
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sS%se%sc%so%sn%sd round started!", level.time + 3000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
			level.compMsgCount = -1;
		}
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		Boe_adminLog ("Map Restart", va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName), "none");
		return;
		}else{
			if(level.mapAction == 1 || level.mapAction == 3){
				trap_SendServerCommand(ent-g_entities, va("print\"^3[Info] ^7A map restart is already in progress.\n\""));}
			else if(level.mapAction == 2 || level.mapAction == 4){
				trap_SendServerCommand(ent-g_entities, va("print\"^3[Info] ^7A map switch is already in progress.\n\""));}
			else{
				trap_SendServerCommand(ent-g_entities, va("print\"^3[Info] ^7Something appears to be wrong. Please report to an developer using this error code: 2L\n\""));}
			return;
		}
	}else{
		if(level.mapSwitch == qfalse){
		level.mapSwitch = qtrue;
		level.mapAction = 1;
		level.mapSwitchCount = level.time;
		level.mapSwitchCount2 = 5; // Boe!Man 7/22/12: 5 seconds remaining on the timer.
		if (g_compMode.integer == 0){
			trap_SendServerCommand(-1, va("print \"^3[Rcon Action] ^7Map restarted.\n\""));
			//trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%sM%sa%sp %sr%se%sstart in 5!", level.time + 1000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		}else if (g_compMode.integer > 0 && cm_enabled.integer == 1){
			trap_SendServerCommand(-1, va("print \"^3[Info] ^7First round started.\n\""));
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sF%si%sr%ss%st round started!", level.time + 3000, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
			level.compMsgCount = -1;
		}else if (g_compMode.integer > 0 && cm_enabled.integer == 3){
			trap_SendServerCommand(-1, va("print \"^3[Info] ^7Second round started.\n\""));
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sS%se%sc%so%sn%sd round started!", level.time + 3000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
			level.compMsgCount = -1;
		}
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		Boe_adminLog ("Map Restart", va("RCON"), "none");
		return;
		}else{
			if(level.mapAction == 1 || level.mapAction == 3){
				Com_Printf("^3[Info] ^7A map restart is already in progress.\n");}
			else if(level.mapAction == 2 || level.mapAction == 4){
				Com_Printf("^3[Info] ^7A map switch is already in progress.\n");}
			else{
				Com_Printf("^3[Info] ^7Something appears to be wrong. Please report to an developer using this error code: 2L\n");}
			return;
		}
	}
}

/*
==========
Boe_Kick
==========
*/

void Boe_Kick(int argNum, gentity_t *ent, qboolean shortCmd){
	int id;
	char	arg3[MAX_STRING_TOKENS];
	char	reason[256];
	if(shortCmd){
		id = Boe_ClientNumFromArg(ent, 1, "kick <id> <reason>", "Kick", qfalse, qfalse, qtrue);
		if(id < 0) return;
		strcpy(reason, GetReason());
		trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"%s\"\n", id, reason));
	}else{
		id = Boe_ClientNumFromArg(ent, 2, "kick <id> <reason>", "Kick", qfalse, qfalse, qfalse);
		if(id < 0) return;
		trap_Argv( 3, arg3, sizeof( arg3 ) );
		trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"%s\"\n", id, arg3));
	}
	// Boe!Man 11/04/11: To save compiled bytes, just do all this stuff at once without looking at the type of chat system.
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
	Boe_adminLog ("Kick", va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName), va("%s\\%s", g_entities[id].client->pers.ip, g_entities[id].client->pers.cleanName));
	//trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was kicked by %s.\n\"", g_entities[id].client->pers.netname,ent->client->pers.netname));
	trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s was %sk%si%sc%sk%se%sd ^7by %s", level.time + 5000, g_entities[id].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, ent->client->pers.netname));
}

/*
==========
Boe_Add_Clan_Member
==========
*/

void Boe_Add_Clan_Member(int argNum, gentity_t *adm, qboolean shortCmd)
{
	int              idnum;
	char			 clientName[MAX_NETNAME];
	char			 admName[MAX_NETNAME];
	sqlite3			*db;
	
	idnum = Boe_ClientNumFromArg(adm, argNum, "addclan <idnumber>", "do this to", qfalse, qtrue, shortCmd);
	if(idnum < 0){
		return;
	}
	
	// Boe!Man 2/6/13: Check if the client is already a clan member.
	if(g_entities[idnum].client->sess.clanMember){
		if(adm){
			trap_SendServerCommand(adm-g_entities, va("print\"^3[Info] ^7%s is already a Clan member.\n\"", g_entities[idnum].client->pers.cleanName));
		}else{
			Com_Printf("^3[Info] ^7%s ^7is already a Clan member.\n", g_entities[idnum].client->pers.cleanName);
		}
		
		return;
	}
	
	// Boe!Man 12/12/12: Check the names, SQLite has massive problems when using quotes in the (updated) query.
	Q_strncpyz(clientName, g_entities[idnum].client->pers.cleanName, sizeof(clientName));
	Boe_convertNonSQLChars(clientName);
	if(adm){
		Q_strncpyz(admName, adm->client->pers.cleanName, sizeof(admName));
		Boe_convertNonSQLChars(admName);
	}else{
		strcpy(admName, "RCON");
	}
	
	// Boe!Man 2/6/13: Add Clan Member to the database.
	// Boe!Man 5/27/13: Open database.
	db = usersDb;
	
	// Boe!Man 2/6/13: Insert query.
	if(sqlite3_exec(db, va("INSERT INTO clanmembers (IP, name, by) values ('%s', '%s', '%s')", g_entities[idnum].client->pers.ip, clientName, admName), 0, 0, 0) != SQLITE_OK){
		if(adm){
			trap_SendServerCommand( adm-g_entities, va("print \"^1[Error] ^7users database: %s\n\"", sqlite3_errmsg(db)));
		}else{
			G_LogPrintf("^1Error: ^7users database: %s\n", sqlite3_errmsg(db));
		}
		return;
	}

	g_entities[idnum].client->sess.clanMember = qtrue;
	trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s is now a %sC%sl%sa%sn %sm%se%smber!", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color4.string, server_color5.string, server_color6.string));
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
	if(adm){
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was made Clan member by %s.\n\"", g_entities[idnum].client->pers.cleanName, adm->client->pers.cleanName));
		Boe_adminLog ("Add Clan", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));
	}else{
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s is now a Clan member.\n\"", g_entities[idnum].client->pers.cleanName));
		Boe_adminLog ("Add Clan", "RCON", va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));
	}
	
	return;
}

/*
======================
Boe_Remove_Clan_Member
2/6/13 - 2:46 PM
======================
*/

void Boe_Remove_Clan_Member(int argNum, gentity_t *adm, qboolean shortCmd)
{
	int			idnum;
	qboolean	clanMem = qfalse;

	idnum = Boe_ClientNumFromArg(adm, argNum, "removeclan <idnumber>", "remove", qfalse, qtrue, shortCmd);
	if(idnum < 0){
		return;
	}
	
	if(g_entities[idnum].client->sess.clanMember){
		g_entities[idnum].client->sess.clanMember = qfalse;
		clanMem = qtrue;
	}else{
		if(adm){
			trap_SendServerCommand(adm-g_entities, va("print\"^3[Info] ^7%s is not a Clan member!\n\"", g_entities[idnum].client->pers.cleanName));
		}else{
			Com_Printf("^3[Info] ^7%s is not a Clan member!\n", g_entities[idnum].client->pers.cleanName);
		}
		
		return;
	}
	
	if(Boe_removeClanMemberFromDb(adm, g_entities[idnum].client->pers.ip, qfalse, qtrue) || clanMem){
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s is no longer a %sC%sl%sa%sn %sm%se%smber!", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color4.string, server_color5.string, server_color6.string));
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		if(adm){
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was removed as Clan member by %s.\n\"", g_entities[idnum].client->pers.cleanName, adm->client->pers.cleanName));
		}else{
			trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s is no longer a Clan member.\n\"", g_entities[idnum].client->pers.cleanName));
		}
	}
	
	return;
}

/*
============================
Boe_removeClanMemberFromList
2/6/13 - 4:40 PM
============================
*/
void Boe_removeClanMemberFromList(int argNum, gentity_t *adm, qboolean shortCmd)
{
	char	arg[32] = "\0";
	char	buf[32] = "\0";
	int		i = 0;
	int		count = 0;
	
	if(shortCmd){
		trap_Argv( argNum, arg, sizeof( arg ) );
		if(strstr(arg, "!") && !strstr(arg, " ")){
			trap_Argv( argNum+1, arg, sizeof( arg ) );
		}
		for(i=StartAfterCommand(va("%s", arg));i<strlen(arg);i++){
			buf[count] = arg[i];
			count += 1;
		}
		buf[count+1] = '\0';
		
		if(!strstr(buf, ".")){ // Boe!Man 2/6/13: No dot found, unban by line number.
			Boe_removeClanMemberFromDb(adm, buf, qtrue, qfalse);
		}else{ // Boe!Man 2/6/13: Dot found, unban by IP.
			Boe_removeClanMemberFromDb(adm, buf, qfalse, qfalse);
		}
	}else{
		trap_Argv( argNum, arg, sizeof( arg ) );
		if(!strstr(arg, ".")){ // Boe!Man 2/6/13: No dot found, unban by line number.
			Boe_removeClanMemberFromDb(adm, arg, qtrue, qfalse);
		}else{ // Boe!Man 2/6/13: Dot found, unban by IP.
			Boe_removeClanMemberFromDb(adm, arg, qfalse, qfalse);
		}
	}
}


/*
==========================
Boe_removeClanMemberFromDb
2/6/13 - 3:12 PM
Acts as a general function when removing a clanmember (either direct ID or line).
==========================
*/

qboolean Boe_removeClanMemberFromDb(gentity_t *adm, const char *value, qboolean lineNumber, qboolean silent)
{
	sqlite3			*db;
	sqlite3_stmt	*stmt;
	int				 rc, i;
	char			 IP[MAX_IP];
	char			 name[MAX_NETNAME];
	int				 line;
	
	if(strlen(value) < 6 && strstr(value, ".")){
		trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7Invalid IP, usage: adm clanlistremove <IP/Line>.\n\""));
		return qfalse;
	}
	
	// Boe!Man 5/27/13: Open the database.
	db = usersDb;
	
	if(lineNumber){ // Delete by line/record.
		line = atoi(value);
		
		if(!line){
			trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7Invalid IP, usage: adm clanlistremove <IP/Line>.\n\""));
			return qfalse;
		}
		
		// Boe!Man 2/6/13: First check if the record exists.
		rc = sqlite3_prepare(db, va("select IP,name from clanmembers where ROWID='%i' LIMIT 1", line), -1, &stmt, 0);
		
		// Boe!Man 2/6/13: If the previous query failed, we're looking at a record that does not exist.
		if(rc != SQLITE_OK){
			if(adm && adm->client){
				trap_SendServerCommand( adm-g_entities, va("print \"^1[Error] ^7users database: %s\n", sqlite3_errmsg(db)));
			}else{
				G_LogPrintf("^1Error: ^7users database: %s\n", sqlite3_errmsg(db));
			}
			
			sqlite3_finalize(stmt);
			return qfalse;
		}else if((rc = sqlite3_step(stmt)) == SQLITE_DONE){
			if(adm && adm->client){
				trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7Could not find line %i.\n\"", line));
			}else{
				Com_Printf("^3[Info] ^7Could not find line %i.\n", line);
			}
			
			sqlite3_finalize(stmt);
			return qfalse;
		}else{
			Q_strncpyz(IP, (char *)sqlite3_column_text(stmt, 0), sizeof(IP));
			Q_strncpyz(name, (char *)sqlite3_column_text(stmt, 1), sizeof(name));
			sqlite3_finalize(stmt);
		}
		
		// Boe!Man 2/6/13: If the previous query succeeded, we can delete the record.
		rc = sqlite3_exec(db, va("DELETE FROM clanmembers WHERE ROWID='%i'", line), 0, 0, 0);
		
		if(rc != SQLITE_OK){
			if(adm && adm->client){
				trap_SendServerCommand( adm-g_entities, va("print \"^1[Error] ^7users database: %s\n", sqlite3_errmsg(db)));
			}else{
				G_LogPrintf("^1Error: ^7users database: %s\n", sqlite3_errmsg(db));
			}
			
			return qfalse;
		}else{
			if(adm && adm->client){
				trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7Removed %s (IP: %s) from line %i.\n\"", name, IP, line));
			}else{
				Com_Printf("^3[Info] ^7Removed %s (IP: %s) from line %i.\n", name, IP, line);
			}
		}
	}else{ // Remove by IP. Don't output this to the screen (except errors), because it's being called directly from /adm removeclan if silent is true.
		// Boe!Man 2/6/13: First check if the record exists.
		rc = sqlite3_prepare(db, va("select ROWID,IP,name from clanmembers where IP='%s' LIMIT 1", value), -1, &stmt, 0);
		
		// Boe!Man 2/6/13: If the previous query failed, we're looking at a record that does not exist.
		if(rc != SQLITE_OK){
			if(adm && adm->client){
				trap_SendServerCommand( adm-g_entities, va("print \"^1[Error] ^7users database: %s\n", sqlite3_errmsg(db)));
			}else{
				G_LogPrintf("^1Error: ^7users database: %s\n", sqlite3_errmsg(db));
			}
			
			sqlite3_finalize(stmt);
			return qfalse;
		}else if((rc = sqlite3_step(stmt)) == SQLITE_DONE){ // Should never happen.
			if(adm && adm->client){
				if(!silent){
					trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7Could not find IP '%s' in the database.\n\"", value));
				}
			}else{
				if(!silent){
					Com_Printf("^3[Info] ^7Could not find IP '%s' in the database.\n", value);
				}
			}
			
			sqlite3_finalize(stmt);
			return qfalse;
		}else if(!silent){ // Boe!Man 2/6/13: Also store info for the info line.
			line = sqlite3_column_int(stmt, 0);
			Q_strncpyz(IP, (char *)sqlite3_column_text(stmt, 1), sizeof(IP));
			Q_strncpyz(name, (char *)sqlite3_column_text(stmt, 2), sizeof(name));
		}
		sqlite3_finalize(stmt);
		
		// Boe!Man 2/6/13: If the previous query succeeded, we can delete the record.
		rc = sqlite3_exec(db, va("DELETE FROM clanmembers WHERE IP='%s'", value), 0, 0, 0);
		
		if(rc != SQLITE_OK){
			if(adm && adm->client){
				trap_SendServerCommand( adm-g_entities, va("print \"^1[Error] ^7users database: %s\n", sqlite3_errmsg(db)));
			}else{
				G_LogPrintf("^1Error: ^7users database: %s\n", sqlite3_errmsg(db));
			}
			
			return qfalse;
		}else if(!silent){
			if(adm && adm->client){
				trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7Removed %s (IP: %s) from line %i.\n\"", name, IP, line));
			}else{
				Com_Printf("^3[Info] ^7Removed %s (IP: %s) from line %i.\n", name, IP, line);
			}
		}
	}
	
	// Boe!Man 2/12/13: If the Clan Member is found on the server, remove his Clan status as well.
	for(i = 0; i < level.numConnectedClients; i++){
		if(strstr(g_entities[level.sortedClients[i]].client->pers.ip, IP) && g_entities[level.sortedClients[i]].client->sess.clanMember){
			g_entities[level.sortedClients[i]].client->sess.clanMember = qfalse;
			
			// Boe!Man 2/12/13: Inform the Clan Member he's off the list..
			if(adm){
				trap_SendServerCommand(g_entities[level.sortedClients[i]].s.number, va("print\"^3[Info] ^7You were removed from the Clanlist by %s.\n\"", adm->client->pers.cleanName));
			}else{
				trap_SendServerCommand(g_entities[level.sortedClients[i]].s.number, va("print\"^3[Info] ^7You were removed from the Clanlist by RCON.\n\""));
			}
		}
	}
	
	// Boe!Man 12/20/12: Re-order the ROWIDs by issuing the VACUUM maintenance query.
	sqlite3_exec(db, "VACUUM", NULL, NULL, NULL);
	
	// Boe!Man 2/6/13: Log the clan removal.
	if(adm && adm->client){
		Boe_adminLog ("Remove Clan", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", IP, name));
	}else{
		Boe_adminLog ("Remove Clan", "RCON", va("%s\\%s", IP, name));
	}
	
	return qtrue;
}

/*
====================
Boe_ClientNumFromArg
====================
*/
int Boe_ClientNumFromArg (gentity_t *ent, int argNum, const char* usage, const char* action, qboolean aliveOnly, qboolean otheradmins, qboolean shortCmd)
{
	char	arg[16] = "\0"; // increase buffer so we can process more commands
	int		num = -1;
	int i, y, x, count, r;
	char numb[36], numbx[4];
	qboolean first = qtrue;
	qboolean space = qfalse;
	int numberofclients = 0;
	char string[1024] = "\0";
	char string1[64] = "\0";
	char cleanName[MAX_NETNAME];

	trap_Argv( argNum, arg, sizeof( arg ) );
	if(shortCmd){ // Henk 04/05/10 -> Handle the short admin commands.
		num = -1;
		for(i=0;i<16;i++){ // FIX ME HENK: could be out of bounds
			if(arg[i] == ' '){
				//trap_SendServerCommand( -1, va("print \"^3[Debug] ^7Arg[%i] = %s.\n\"", i, arg[i]));
				if(henk_isdigit(arg[i+1]) && !henk_ischar(arg[i+2])){ // we check if the other 2 are chars(fixes !uc 1fx bug which uppercuts id 1)
				num = atoi(va("%c%c", arg[i+1], arg[i+2]));
				}else{
					for(x=0;x<=20;x++){ // FIX ME HENK: could be out of bounds -> Could be improved by starting at i+3(?)
						if(arg[x] == ' '){
							// Henk 08/09/10 -> Check for another arg in this.
							for(y=1;y<=sizeof(arg);y++){
								if(arg[x+y] == ' '){
									space = qtrue;
									break;
								}else{
									if(first){
										Com_sprintf(numb, sizeof(numb), "%c", arg[x+y]);
										first = qfalse;
									}else{
										Com_sprintf(numb, sizeof(numb), "%s%c", numb, arg[x+y]);

									}
									//Com_Printf("%s\n", numb);
								}
							}
							if(space == qfalse){
								if(!henk_ischar(arg[x+1]) && !henk_ischar(arg[x+2])){
									//trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You haven't entered a valid player ID/player name.\n\"", arg, usage));
									num = -1;
									break;
								}
							// FIX ME HENK: could be out of bounds
								memset(numb, 0, sizeof(numb));
								for(r=x+1;r<strlen(arg);r++){
									if(arg[r])
										if(!numb[0])
											Com_sprintf(numb, sizeof(numb), "%c", arg[r]);
										else
											Com_sprintf(numb, sizeof(numb), "%s%c", numb, arg[r]);
									else
										break;
								}
							//sprintf(numb, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", arg[x+1], arg[x+2], arg[x+3], arg[x+4], arg[x+5], arg[x+6], arg[x+7], arg[x+8], arg[x+9], arg[x+10], arg[x+11], arg[x+12], arg[x+13], arg[x+14], arg[x+15]);
							}
							break;
						}
					}
					memset(string, 0, sizeof(string));
					memset(string1, 0, sizeof(string1));
					numberofclients = 0;
					num = -1;
				    //numb[strlen(numb)-1] = '\0';
				    numb[strlen(numb)] = '\0';
				    /* END ajay */
					for(x=0;x<level.numConnectedClients;x++){
						//trap_SendServerCommand(-1, va("print\"^3[Debug] ^7%s comparing with %s.\n\"", g_entities[level.sortedClients[x]].client->pers.cleanName,numb));
						Q_strncpyz(cleanName, g_entities[level.sortedClients[x]].client->pers.cleanName, sizeof(cleanName));
						if(strstr(Q_strlwr(cleanName), Q_strlwr(numb))){
							//Com_Printf("%s\n", g_entities[level.sortedClients[x]].client->pers.cleanName);
							num = level.sortedClients[x];
							numberofclients += 1;
							Com_sprintf(string1, sizeof(string1), "^1[#%i] ^7%s, ",  num, g_entities[level.sortedClients[x]].client->pers.cleanName);
							Q_strncpyz(string+strlen(string), string1, strlen(string1)+1);
							//break;
						}
					}
					string[strlen(string)-2] = '\0';
					if(numberofclients > 1){
						trap_SendServerCommand(ent->s.number, va("print\"^3[Info] ^7Multiple names found with ^3%s^7: %s\n\"", numb, string));
						return -1;
					}
				}
				break;
			}else if(henk_isdigit(arg[i]) && henk_ischar(arg[i-1]) && g_shortCommandStyle.integer){ // Henk 10/03/11 -> Add support for !u1/!r1 commands
				count = 0;
				for(x=i;x<=strlen(arg);x++){
					if(henk_isdigit(arg[x])){
						numbx[count] = arg[x];
						count += 1;
					}else{
						numbx[count] = '\0';
						break;
					}
				}
				num = atoi(numbx);
				//trap_SendServerCommand(ent->s.number, va("print\"^3[Debug] ^7New short command style detected, Client number: %i.\n\"", num));
			}
		}
		if(num == -1){ // Get second argument because they use it from the console
			trap_Argv( 2, arg, sizeof( arg ) );
			if(strlen(arg) >= 1){
				if(henk_isdigit(arg[0])){
					num = atoi(arg);
				}else{
					for(i=0;i<level.numConnectedClients;i++){
						//trap_SendServerCommand(-1, va("print\"^3[Debug] ^7%s comparing with %s.\n\"", g_entities[level.sortedClients[i]].client->pers.cleanName,numb));
						Q_strncpyz(cleanName, g_entities[level.sortedClients[i]].client->pers.cleanName, sizeof(cleanName));
						if(strstr(Q_strlwr(cleanName), Q_strlwr(arg))){
							num = level.sortedClients[i];
							break;
						}
						num = -1;
					}
				}
			}else{
				num = -1;
			}
		}

	}else{
		if (arg[0] >= '0' && arg[0] <= '9')
		{
			num = atoi( arg );
		}else if(strlen(arg) >= 1){
			for(i=0;i<level.numConnectedClients;i++){
				//trap_SendServerCommand(-1, va("print\"^3[Debug] ^7%s comparing with %s.\n\"", g_entities[level.sortedClients[i]].client->pers.cleanName,numb));
				Q_strncpyz(cleanName, g_entities[level.sortedClients[i]].client->pers.cleanName, sizeof(cleanName));
				if(strstr(Q_strlwr(cleanName), Q_strlwr(arg))){
					num = level.sortedClients[i];
				break;
				}
			num = -1;
			}
		}
	}

	if ( num < 0 || num >= g_maxclients.integer )
	{
		if(ent && ent->client)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You haven't entered a valid player ID/player name.\n\""));
		}
		else
		{
			Com_Printf("You haven't entered a valid player ID/player name.\n", num );
		}
		return -1;
	}

	if ( g_entities[num].client->pers.connected == CON_DISCONNECTED )
	{
		if(ent && ent->client)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7This client is not connected.\n\""));
		}
		else
		{
			Com_Printf("There is no client with the client number %d.\n", num);
		}
		return -1;
	}

	if(ent && ent->client)
	{
		if (g_entities[num].client->sess.admin ){
			// Boe!Man 1/4/10: Higher level Admin fix.
			if(g_entities[num].client->sess.admin > ent->client->sess.admin && !strstr(action, "forceteam")){
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You cannot %s higher level Admins.\n\"", action));
				return -1;
			}
		}
	}
	// Boe!Man 1/18/10: Using Admin commands on other Admins fix.
	if(otheradmins == qfalse){
		if(g_entities[num].client->sess.admin && otheradmins == qfalse){
			if(ent && ent->client){
			trap_SendServerCommand(ent->s.number, va("print\"^3[Info] ^7You cannot use this command on other Admins.\n\""));
			return -1;
			}
			else{
				//Com_Printf("You cannot use this command on other Admins.\n"); // Henk -> Rcon has to be able to pop everyone.
				//return -1;
			}
		}
	}

	if(aliveOnly)
	{
		if ( G_IsClientDead ( g_entities[num].client ) )
		{
			if(ent && ent->client)
			{
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You cannot %s dead players.\n\"", action));
			}
			else
			{
				Com_Printf("You cannot %s dead Players.\n", action);
			}
			return -1;
		}
		if (G_IsClientSpectating(g_entities[num].client))
		{
			if(ent && ent->client)
			{
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You cannot %s a spectator.\n\"", action));
			}
			else
			{
				Com_Printf("You cannot %s a Spectator.\n", action);
			}
			return - 1;
		}
	}

	return num;
}

/*
=============
Boe_AddToList
Rid of this since SQLite, yay!
=============
*/

/*
int Boe_AddToList(const char *string, const char* file, const char* type, gentity_t* ent)
{
	int				len;
	char			buf[15000] = "\0";
	char			buf2[15000] = "\0";
	fileHandle_t	f;
	qboolean		clonecheck = qfalse;

	if(strstr(type, "Clonecheck")){ // Boe!Man 6/29/11: Check if it's an alias file - if yes, we need to append the name to the front in order to save us a lot of hassle of processing everything differently later.
		clonecheck = qtrue;
		memset(buf, 0, sizeof(buf)); // Clean the buffers.
		memset(buf2, 0, sizeof(buf2));
		len = trap_FS_FOpenFile( file, &f, FS_READ_TEXT );
		if(!f){
			if(ent && ent->client){
				trap_SendServerCommand( ent-g_entities, va("print \"^1[Error] ^7Cannot open %s file\n\"", type));
			}
			else
			{
				Com_Printf("^1ERROR: Cannot open %s file.\n", type);
			}
			return -1;
		}
		if(len > 15000){
			if(ent && ent->client){
				trap_SendServerCommand( ent-g_entities, va("print \"^1[Error] ^7List full, unable to add %s.\n\"", type));
			}
			else
			{
				Com_Printf("^1ERROR: List full, Unable to add %s.\n", type);
			}
			return 0;
		}
		trap_FS_Read(buf, len, f); // Write to buffer.
		trap_FS_FCloseFile(f); // Close and reopen file in write mode.
		trap_FS_FOpenFile( file, &f, FS_WRITE_TEXT );
	}else{
		len = trap_FS_FOpenFile( file, &f, FS_APPEND_TEXT );
	}

    if (!f)
	{
		if(ent && ent->client)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"^1[Error] ^7Cannot open %s file\n\"", type));
		}
		else
		{
			Com_Printf("^1ERROR: Cannot open %s file.\n", type);
		}
	    return -1;
	}

	if(len > 15000 - (strlen(string) + 1))
	{
		trap_FS_FCloseFile(f);

		if(ent && ent->client)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"^1[Error] ^7List full, unable to add %s.\n\"", type));
		}
		else
		{
			Com_Printf("^1ERROR: List full, Unable to add %s.\n", type);
		}
		return 0;
	}

	if(clonecheck){
		Com_sprintf( buf2, sizeof(buf2), "%s\n%s", string, buf);
		trap_FS_Write(buf2, strlen(buf2), f);
	}else{
		trap_FS_Write(string, strlen(string), f);
		trap_FS_Write("\n", 1, f);
	}
	trap_FS_FCloseFile(f);

	return 1;
}
*/

/*
====================
Boe_Remove_from_list
This can also go as of SQLite. :-)
====================
*/

/*
int Boe_Remove_from_list ( char *key, const char *file, const char* type, gentity_t *ent, qboolean unban, qboolean removeAdmin, qboolean scoreCheck)
{
	int				len = 0, removed = 0;
	char			buf[15000] = "\0";
	char            listName[64] = "\0";
	char			ip[MAX_IP];
	char			*bufP, *listP;
	fileHandle_t	f;
	char			a[64] = "\0";
//	int				count;

	while(*key == ' '){
		if(*key == '\0'){
			return 0;
		}
		key++;
	}
	#ifdef _DEBUG
	Com_Printf("Remove: opening %s\n", file);
	#endif
	len = trap_FS_FOpenFile( file, &f, FS_READ_TEXT);

	if (!f)	{
		Boe_FileError(ent, file);
		return -1;
	}

	if(len > 15000)	{
		len = 15000;
	}
	memset( buf, 0, sizeof(buf) );

	trap_FS_Read( buf, len, f );
	buf[len] = '\0';
	trap_FS_FCloseFile( f );

	bufP = buf;
	listP = listName;

	len = trap_FS_FOpenFile( file, &f, FS_WRITE_TEXT);

	while(*bufP) {
		while(*bufP != '\n' && *bufP != '\0') {
			*listP++ = *bufP++;
			if(unban && *bufP == '\\') {
				*listP = '\0';
				Q_strncpyz(ip, listName, MAX_IP);
			}
		}

		*listP = '\0';

		while (*bufP == '\n') {
			bufP++;
		}

		if(unban && !Q_stricmp(ip, key)) {
			removed = 1;
			listP = listName;
			continue;
		}
					
		if (removeAdmin){
			strcpy(a, va("%s:2", key ));
			if (!Q_stricmp(listName, a)) {
				removed = 1;
				listP = listName;
				continue;
			}
			strcpy(a, va("%s:3", key ));
			if (!Q_stricmp(listName, a)) {
				removed = 1;
				listP = listName;
				continue;
			}
			strcpy(a, va("%s:4", key ));
			if (!Q_stricmp(listName, a)) {
				removed = 1;
				listP = listName;
				continue;
			} 
		}else if(scoreCheck){
			if(strstr(listName, key)){
				removed = 1;
				listP = listName;
				continue;
			}
		}

		if(!Q_stricmp(listName, key)){
			removed = 1;
			listP = listName;
			continue;
		}

		trap_FS_Write( listName, strlen(listName), f);
		trap_FS_Write( "\n", 1, f);
		listP = listName;
	}
	trap_FS_FCloseFile( f );

	if(!removed){
		if(ent && ent->client) {
			trap_SendServerCommand( ent-g_entities, va("print \"^3%s ^7was not on the %s list\n\"", key, type));
		}
		else {
			Com_Printf("^3%s ^7was not on the %s list\n", key, type);
		}
	}
	return removed;
}
*/

/*
=============
Boe_BanList
=============
*/
void Henk_BanList(int argNum, gentity_t *adm, qboolean shortCmd){
	Boe_BanList(argNum, adm, shortCmd, qfalse);
}

void Henk_SubnetBanList(int argNum, gentity_t *adm, qboolean shortCmd){
	Boe_BanList(argNum, adm, shortCmd, qtrue);
}

void Boe_BanList(int argNum, gentity_t *adm, qboolean shortCmd, qboolean subnet){
	// Boe!Man 11/04/11: We use this in order to send as little packets as possible. Packet max size is 1024 minus some overhead, 1000 char. max should take care of this. (adm only, RCON remains unaffected).
	char			 buf2[1000] = "\0";
	// End Boe!Man 11/04/11
	// Henk comeback 19/09/12: Add SQLite back end
	sqlite3			*db;
	sqlite3_stmt	*stmt;
	int				 rc;
	
	db = bansDb;

	if(adm){
		if(subnet){
			Q_strcat(buf2, sizeof(buf2), "^3[Subnetbanlist]^7\n\n");
		}else{
			Q_strcat(buf2, sizeof(buf2), "^3[Banlist]^7\n\n");
		}
		Q_strcat(buf2, sizeof(buf2), "^3 #    IP              Name            Reason             By\n^7------------------------------------------------------------------------\n");
	}else{
		if(subnet){
			Com_Printf("^3[Subnetbanlist]^7\n\n");
		}else{
			Com_Printf("^3[Banlist]^7\n\n");
		}
		Com_Printf("^3 #    IP              Name            Reason             By\n");
		Com_Printf("^7------------------------------------------------------------------------\n");
	}

	if(subnet){
		rc = sqlite3_prepare(db, "select ROWID,IP,name,by,reason from subnetbans order by ROWID", -1, &stmt, 0);
	}else{
		rc = sqlite3_prepare(db, "select ROWID,IP,name,by,reason from bans order by ROWID", -1, &stmt, 0);
	}
	if(rc!=SQLITE_OK){
		if(adm){
			trap_SendServerCommand( adm-g_entities, va("print \"^1[Error] ^7bans database: %s\n\"", sqlite3_errmsg(db)));
		}else{
			G_LogPrintf("^1Error: ^7bans database: %s\n", sqlite3_errmsg(db));
		}
		return;
	}else while((rc = sqlite3_step(stmt)) != SQLITE_DONE){
		if(rc == SQLITE_ROW){
			if(adm){
				// Boe!Man 11/04/11: Put packet through to clients if char size would exceed 1000 and reset buf2.
				if((strlen(buf2)+strlen(va("[^3%-3.3i^7] %-15.15s %-15.15s %-18.18s %-15.15s\n", sqlite3_column_int(stmt, 0), sqlite3_column_text(stmt, 1), sqlite3_column_text(stmt, 2), sqlite3_column_text(stmt, 4), sqlite3_column_text(stmt, 3)))) > 1000){
					trap_SendServerCommand( adm-g_entities, va("print \"%s\"", buf2));
					memset(buf2, 0, sizeof(buf2)); // Boe!Man 11/04/11: Properly empty the buffer.
				}
				Q_strcat(buf2, sizeof(buf2), va("[^3%-3.3i^7] %-15.15s %-15.15s %-18.18s %-15.15s\n", sqlite3_column_int(stmt, 0), sqlite3_column_text(stmt, 1), sqlite3_column_text(stmt, 2), sqlite3_column_text(stmt, 4), sqlite3_column_text(stmt, 3)));
			}else{
				Com_Printf("[^3%-3.3i^7] %-15.15s %-15.15s %-18.18s %-15.15s\n", sqlite3_column_int(stmt, 0), sqlite3_column_text(stmt, 1), sqlite3_column_text(stmt, 2), sqlite3_column_text(stmt, 4), sqlite3_column_text(stmt, 3));
			}
		}
	}
	
	sqlite3_finalize(stmt);
	
	// Boe!Man 11/04/11: Fix for RCON not properly showing footer of banlist.
	if(adm){
		trap_SendServerCommand( adm-g_entities, va("print \"%s\nUse ^3[Page Up] ^7and ^3[Page Down] ^7keys to scroll\n\n\"", buf2)); // Boe!Man 11/04/11: Also send the last buf2 (that wasn't filled as a whole yet).
	}else{
		Com_Printf("\nUse ^3[Page Up] ^7and ^3[Page Down] ^7keys to scroll\n\n");
	}
	
	return;
}

/*
=============
Boe_id
Updated 1/28/11 - 11:35 AM
=============
*/
void Boe_id (int idnum)
{
	//int             i = 0;
	char		    *ip = g_entities[idnum].client->pers.ip;
	char			*id = g_entities[idnum].client->pers.boe_id;
	char			*ip2;
	char			ip20[MAX_IP];

	if(g_entities[idnum].r.svFlags & SVF_BOT){
		Com_sprintf(id, MAX_BOE_ID, "bot\\%s", g_entities[idnum].client->pers.cleanName);
		return;
	}
	
	//while(i < 6)
	//{
	//	*id++ = *ip++;
	//	i++;
	//}

	//*id++ = '\\';
    //*id = '\0';

	if(!g_preferSubnets.integer){ // Boe!Man 7/29/12: Normal and proper way.
		Com_sprintf(id, MAX_BOE_ID, "%s\\%s", ip, g_entities[idnum].client->pers.cleanName);
	}else{ // Add them on subnet, so also check that globally.
		ip2 = ip20;
		Q_strncpyz(ip2, g_entities[idnum].client->pers.ip, 7);
		Com_sprintf(id, MAX_BOE_ID, "%s\\%s", ip2, g_entities[idnum].client->pers.cleanName);
	}
	//strcat(id, g_entities[idnum].client->pers.cleanName);
	return;
}

/*
==============
Boe_FileError
==============
*/
void Boe_FileError (gentity_t * ent, const char *file)
{
	if(ent && ent->client)
	{
		Com_Printf("^7Error opening %s\n\"", file);
	}
	else
	{
		Com_Printf("^1Error opening %s\n", file);
	}
	return;
}

/*
================
Boe_addAdmin
2/5/13 - 4:03 PM
Function that acts as a wrapper around addbadmin, addadmin and addsadmin.
Fetches the level the user wishes to add and then redirects them.
================
*/

void Boe_addAdmin(int argNum, gentity_t *adm, qboolean shortCmd)
{
	char			command[64] = "\0";

	if(adm && adm->client){
		trap_Argv(1, command, sizeof(command));
	}else{
		trap_Argv(0, command, sizeof(command));
	}
	Q_strlwr(command); // Boe!Man 2/16/13: Fix capitalized Admin commands resulting in adding S-Admin by converting the command to lower case.
	
	if(strstr(command, "!ab") || strstr(command, "addbad")){
		Boe_Add_Admin_f(argNum, adm, shortCmd, 2, "addbadmin");
	}else if(strstr(command, "!aa") || strstr(command, "addad")){
		Boe_Add_Admin_f(argNum, adm, shortCmd, 3, "addadmin");
	}else{ // Must be add S-Admin..
		Boe_Add_Admin_f(argNum, adm, shortCmd, 4, "addsadmin");
	}
	
	return;
}


/*
================
Boe_Add_Admin_f
================
*/

void Boe_Add_Admin_f(int argNum, gentity_t *adm, qboolean shortCmd, int level2, char *commandName)
{
	int              idnum;
	char			 arg[64] = "\0";
	char			 clientName[MAX_NETNAME];
	char			 admName[MAX_NETNAME];
	char			 ip[MAX_IP];
	char			 admLevel[12];
	char			 admLevelPrefixed[32];
	char			 adminPassword[64];
	qboolean		 passAdmin = qfalse;
	sqlite3			*db;

	idnum = Boe_ClientNumFromArg(adm, argNum, va("%s <idnumber/name>", commandName), "do this to", qfalse, qfalse, shortCmd);
	if(idnum < 0){
		return;
	}
	
	// Boe!Man 4/27/11: RCON has to be able to do everything. However, there are a few things that should be excluded. Instead of processing which command was entered in the ClientNumFromArg func, we deploy this check in the addxadmin functions (to save resources).
	if(!adm && g_entities[idnum].client->sess.admin > 1){
		Com_Printf("You cannot use this command on other Admins.\n");
		return;
	}

	if(shortCmd){
		strcpy(arg, GetReason());
	}else{
		if(adm && adm->client){
			trap_Argv(3, arg, sizeof(arg));
		}else{
			trap_Argv(2, arg, sizeof(arg));
		}
	}

	if(!Q_stricmp(arg, "pass")){
		if(g_passwordAdmins.integer){
			passAdmin = qtrue;
			Q_strncpyz(ip, g_entities[idnum].client->pers.ip, 4);
		}else{
			if(adm){
				trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7Access denied: No password logins allowed by the server!\n\""));
			}else{
				Com_Printf("Access denied: No password logins allowed by the server!\n");
			}
			return;
		}
	}else{
		strcpy(ip, g_entities[idnum].client->pers.ip);
	}
	
	// Boe!Man 12/12/12: Check the names, SQLite has massive problems when using quotes in the (updated) query.
	Q_strncpyz(clientName, g_entities[idnum].client->pers.cleanName, sizeof(clientName));
	Boe_convertNonSQLChars(clientName);
	if(adm){
		Q_strncpyz(admName, adm->client->pers.cleanName, sizeof(admName));
		Boe_convertNonSQLChars(admName);
	}else{
		strcpy(admName, "RCON");
	}
	
	// Boe!Man 2/5/13: Add Admin to the database.
	// Boe!Man 2/5/13: Open database.
	db = usersDb;
	
	// Boe!Man 2/5/13: Insert query.
	if(!passAdmin){
		if(sqlite3_exec(db, va("INSERT INTO admins (IP, name, by, level) values ('%s', '%s', '%s', '%i')", ip, clientName, admName, level2), 0, 0, 0) != SQLITE_OK){
			if(adm){
				trap_SendServerCommand( adm-g_entities, va("print \"^1[Error] ^7users database: %s\n\"", sqlite3_errmsg(db)));
			}else{
				G_LogPrintf("^1Error: ^7users database: %s\n", sqlite3_errmsg(db));
			}
			return;
		}
	}else{
		if(sqlite3_exec(db, va("INSERT INTO passadmins (octet, name, by, level) values ('%s', '%s', '%s', '%i')", ip, clientName, admName, level2), 0, 0, 0) != SQLITE_OK){
			if(adm){
				trap_SendServerCommand( adm-g_entities, va("print \"^1[Error] ^7users database: %s\n\"", sqlite3_errmsg(db)));
			}else{
				G_LogPrintf("^1Error: ^7users database: %s\n", sqlite3_errmsg(db));
			}
			return;
		}
	}

	g_entities[idnum].client->sess.admin = level2;
	if(level2 == 2){
		strcpy(admLevel, "B-Admin");
		Q_strncpyz(admLevelPrefixed, server_badminprefix.string, sizeof(admLevelPrefixed));
		if(passAdmin){
			Q_strncpyz(adminPassword, g_badminPass.string, sizeof(adminPassword));
		}
	}else if(level2 == 3){
		strcpy(admLevel, "Admin");
		Q_strncpyz(admLevelPrefixed, server_adminprefix.string, sizeof(admLevelPrefixed));
		if(passAdmin){
			Q_strncpyz(adminPassword, g_adminPass.string, sizeof(adminPassword));
		}
	}else{
		strcpy(admLevel, "S-Admin");
		Q_strncpyz(admLevelPrefixed, server_sadminprefix.string, sizeof(admLevelPrefixed));
		if(passAdmin){
			Q_strncpyz(adminPassword, g_sadminPass.string, sizeof(adminPassword));
		}
	}
	
	trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s is now a %s", level.time + 5000, g_entities[idnum].client->pers.netname, admLevelPrefixed));
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
	if(adm){
		if(!passAdmin){
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was made %s by %s.\n\"", g_entities[idnum].client->pers.netname, admLevel, adm->client->pers.cleanName));
		}else{
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was added to the %s password list by %s.\n\"", g_entities[idnum].client->pers.netname, admLevel, adm->client->pers.cleanName));
		}
		Boe_adminLog (va("Add %s", admLevel), va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));
	}else{
		if(!passAdmin){
			trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s is now a %s.\n\"", g_entities[idnum].client->pers.netname, admLevel));
		}else{
			trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s was added to the %s password list.\n\"", g_entities[idnum].client->pers.netname, admLevel));
		}
		Boe_adminLog (va("Add %s", admLevel), "RCON", va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));
	}
	
	// Boe!Man 2/5/13: Inform a passworded Admin of the system he can now use.
	// Boe!Man 2/16/13: Only inform the Admin via chat. The Admin won't notice if it's being broadcast via console.
	if(passAdmin){
		trap_SendServerCommand(g_entities[idnum].s.number, va("chat -1 \"%sI%sn%sf%so%s: ^7You need to login every time you enter the server.\n\"", server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string));
		if(strstr(adminPassword, "none") && strlen(adminPassword) < 5){
			trap_SendServerCommand(g_entities[idnum].s.number, va("chat -1 \"%sI%sn%sf%so%s: ^7Ask the server owner/a RCON holder to set a password for you to use.\n\"", server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string));
		}else{
			trap_SendServerCommand(g_entities[idnum].s.number, va("chat -1 \"%sI%sn%sf%so%s: ^7You can do so by entering '/adm login %s' in the console.\n\"", server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, adminPassword));
		}
	}
	// Boe!Man 10/16/10: Is the Admin level allowed to spec the opposite team?
	if (g_adminSpec.integer <= level2 && g_adminSpec.integer != 0 && cm_enabled.integer < 2){
		g_entities[idnum].client->sess.adminspec = qtrue;
	}
	
	return;
}

/*
================
Henk_RemoveLineFromFile
23/01/11 -> By Henkie

Removes a zero based line from a file
================

void Henk_RemoveLineFromFile(gentity_t *ent, int line, char *file, qboolean subnet, qboolean ban, qboolean banremove, char *banremovestring){
	fileHandle_t	f;
	int len, CurrentLine = 0, StartPos = 0, EndPos = -1, i;
	qboolean begin = qtrue;
	char buf[15000]; // Boe!Man 2/5/12: Max buf 15000 so unban gets compatible with large files.
	char asd[128] = "";
	char last[128];
	qboolean done = qfalse;

	line = line;
	memset( buf, 0, sizeof(buf) );
	len = trap_FS_FOpenFile( file, &f, FS_READ_TEXT);
	if(!f){
		Com_Printf("Error while opening %s\n", file);
		return;
	}
	if(len >= 15000){
		if(ent && ent->client){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7List is too large.\n\""));
		}else{
			Com_Printf("List is too large.\n");
		}
		return;
	}
	trap_FS_Read( buf, len, f );
	buf[len] = '\0';
	trap_FS_FCloseFile( f );

	trap_FS_FOpenFile( file, &f, FS_WRITE_TEXT);
	if(!f){
		#ifdef _DEBUG
		Com_Printf("failed\n");
		#endif
		return;
	}
	trap_FS_Write("", 0, f); // clear file
	trap_FS_FCloseFile(f);

	trap_FS_FOpenFile( file, &f, FS_APPEND_TEXT);
	if(!f){
		#ifdef _DEBUG
		Com_Printf("failed\n");
		#endif
		return;
	}

	for(i=0;i<=len;i++){
		memset( asd, 0, sizeof(asd) );
		if(i == len){
			EndPos = i;
			if(line != CurrentLine){
			strncpy(asd, buf+StartPos, EndPos);
			if(banremove){
				if(strstr(asd, banremovestring)){
					strncpy(last, buf+StartPos, EndPos-StartPos); 
					done = qtrue;
					break;
				}
			}
			//trap_FS_Write(asd, strlen(asd), f);
			//Com_Printf("Written %s\n", asd);
			//Q_strncpyz(newbuf+strlen(newbuf), asd, sizeof(newbuf));
			}else{
				strncpy(last, buf+StartPos, EndPos-StartPos); 
				done = qtrue;
				//Com_Printf("Last1: %s\n", asd);
			}
			break;
		}
		if(begin){
			StartPos = i;
			EndPos = -1;
			begin = qfalse;
			CurrentLine += 1; // current line
		}
		if(buf[i] == '\n'){ // Last line doesn't have an enter
			//Com_Printf("Found a line\n");
			EndPos = i;
			begin = qtrue;
		}
		if(EndPos != -1){
			if(line != CurrentLine){
			strncpy(asd, buf+StartPos, EndPos-StartPos); 
			if(banremove){
				if(strstr(asd, banremovestring)){
					strncpy(last, buf+StartPos, EndPos-StartPos); 
					done = qtrue;
					//break;
				}else{
					Com_sprintf(asd, sizeof(asd), "%s\n", asd);
					trap_FS_Write(asd, strlen(asd), f);
					//Com_Printf("Written %s\n", asd);
				}
			}else{
				Com_sprintf(asd, sizeof(asd), "%s\n", asd);
				trap_FS_Write(asd, strlen(asd), f);
				//Com_Printf("Written %s\n", asd);
			}
			//Com_Printf("New buf length: %i\n", strlen(newbuf));
			//Com_Printf("Added %s\n", asd);
			}else{
				done = qtrue;
				strncpy(last, buf+StartPos, EndPos-StartPos); 
				last[(EndPos-StartPos)-1] = '\0';
				//Com_Printf("Final: %s\n", last);
			}
		}
	}
	trap_FS_FCloseFile(f);
	if(done && strlen(last) >= 1){ // && !strstr(newbuf, last)
	// Clear ban info
		if(ban){
		// End
			// Boe!Man 9/16/11: For the ban log/message it's healthier to delete the newline so it won't mess up messaging.
			last[strlen(last)-1] = '\0';
			if(subnet){
				if(ent && ent->client)
					Boe_adminLog ("Subnet Unban", va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName), va("%s", last));
				else 
					Boe_adminLog ("Subnet Unban", "RCON", va("%s", last));
			}else{
				if(ent && ent->client)
					Boe_adminLog ("Unban", va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName), va("%s", last));
				else 
					Boe_adminLog ("Unban", "RCON", va("%s", last));
			}
			if(ent && ent->client){
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7%s has been Unbanned.\n\"", last));
			}else{
				Com_Printf("%s has been Unbanned.\n", last);
			}
		//return qtrue;
		}else{
			if(ent && ent->client){
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7%s has been removed from the list.\n\"", last));
			}else{
				Com_Printf("%s has been removed from the list.\n", last);
			}
		}
	}else{
		if(ent && ent->client){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Could not find line %i.\n\"", line));
		}else{
			Com_Printf("Could not find line %i.\n", line);
		}
	}
}
*/

/*
==================
Boe_Unban
12/17/12 - 6:12 PM
==================
*/

void Boe_Unban(gentity_t *adm, char *ip, qboolean subnet)
{
	int 			iLine, rc;
	sqlite3			*db;
	sqlite3_stmt	*stmt;
	char			name[MAX_NETNAME];
	char			ip2[MAX_IP];
	

	if(strlen(ip) < 2 && strstr(ip, ".")){
		trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7Invalid IP, usage: adm unban <IP/Line>.\n\""));
		return;
	}
	
	// Boe!Man 5/27/13: Open the database.
	db = bansDb;
	
	// Delete by line/record.
	if(strlen(ip) >= 1 && !strstr(ip, ".") && !strstr(ip, "bot")){
		// Boe!Man 12/17/12: Unban record from database.
		iLine = atoi(ip);
		
		// Boe!Man 12/17/12: First check if the record exists.
		if(!subnet){
			rc = sqlite3_prepare(db, va("select IP,name from bans where ROWID='%i' LIMIT 1", iLine), -1, &stmt, 0);
		}else{
			rc = sqlite3_prepare(db, va("select IP,name from subnetbans where ROWID='%i' LIMIT 1", iLine), -1, &stmt, 0);
		}
		
		// Boe!Man 12/17/12: If the previous query failed, we're looking at a record that does not exist.
		if(rc != SQLITE_OK){
			if(adm && adm->client){
				trap_SendServerCommand( adm-g_entities, va("print \"^1[Error] ^7bans database: %s\n", sqlite3_errmsg(db)));
			}else{
				G_LogPrintf("^1Error: ^7bans database: %s\n", sqlite3_errmsg(db));
			}
			
			sqlite3_finalize(stmt);
			return;
		}else if((rc = sqlite3_step(stmt)) == SQLITE_DONE){
			if(adm && adm->client){
				trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7Could not find line %i.\n\"", iLine));
			}else{
				Com_Printf("^3[Info] ^7Could not find line %i.\n", iLine);
			}
			
			sqlite3_finalize(stmt);
			return;
		}else{ // Boe!Man 12/17/12: Store info for the unban line given to the Admin (to let him know it went correctly).
			Q_strncpyz(ip2, (char *)sqlite3_column_text(stmt, 0), sizeof(ip2));
			Q_strncpyz(name, (char *)sqlite3_column_text(stmt, 1), sizeof(name));
		}
		sqlite3_finalize(stmt);
		
		// Boe!Man 12/17/12: If the previous query succeeded, we can delete the record.
		if(!subnet){
			rc = sqlite3_exec(db, va("DELETE FROM bans WHERE ROWID='%i'", iLine), 0, 0, 0);
		}else{
			rc = sqlite3_exec(db, va("DELETE FROM subnetbans WHERE ROWID='%i'", iLine), 0, 0, 0);
		}
		
		if(rc != SQLITE_OK){
			if(adm && adm->client){
				trap_SendServerCommand( adm-g_entities, va("print \"^1[Error] ^7bans database: %s\n", sqlite3_errmsg(db)));
			}else{
				G_LogPrintf("^1Error: ^7bans database: %s\n", sqlite3_errmsg(db));
			}
			
			return;
		}else{
			if(adm && adm->client){
				trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7Unbanned %s (IP: %s) from line %i.\n\"", name, ip2, iLine));
			}else{
				Com_Printf("^3[Info] ^7Unbanned %s (IP: %s) from line %i.\n", name, ip2, iLine);
			}
		}
	}else if(strlen(ip) < 3){
		if(adm && adm->client){
			trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7Invalid IP, usage: adm unban <IP/Line>.\n\""));
		}else{
			Com_Printf("^3[Info] ^7Invalid IP, usage: adm unban <IP/Line>.\n");
		}
		
		return;
	}else{ // Boe!Man 12/19/12: Delete by full IP.
		// Boe!Man 12/17/12: First check if the record exists.
		if(!subnet){
			rc = sqlite3_prepare(db, va("select ROWID,name from bans where IP='%s' LIMIT 1", ip), -1, &stmt, 0);
		}else{
			rc = sqlite3_prepare(db, va("select ROWID,name from subnetbans where IP='%s' LIMIT 1", ip), -1, &stmt, 0);
		}
		
		// Boe!Man 12/17/12: If the previous query failed, we're looking at a record that does not exist.
		if(rc != SQLITE_OK){
			if(adm && adm->client){
				trap_SendServerCommand( adm-g_entities, va("print \"^1[Error] ^7bans database: %s\n", sqlite3_errmsg(db)));
			}else{
				G_LogPrintf("^1Error: ^7bans database: %s\n", sqlite3_errmsg(db));
			}
			
			sqlite3_finalize(stmt);
			return;
		}else if((rc = sqlite3_step(stmt)) == SQLITE_DONE){
			if(adm && adm->client){
				trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7Could not find IP %s.\n\"", ip));
			}else{
				Com_Printf("^3[Info] ^7Could not find IP %s.\n", ip);
			}
			
			sqlite3_finalize(stmt);
			return;
		}else{ // Boe!Man 12/17/12: Store info for the unban line given to the Admin (to let him know it went correctly).
			Q_strncpyz(ip2, (char *)sqlite3_column_text(stmt, 0), sizeof(ip2)); // ID in this case.
			Q_strncpyz(name, (char *)sqlite3_column_text(stmt, 1), sizeof(name));
		}
		
		sqlite3_finalize(stmt);
		
		// Boe!Man 12/17/12: If the previous query succeeded, we can delete the record.
		if(!subnet){
			rc = sqlite3_exec(db, va("DELETE FROM bans WHERE IP='%s'", ip), 0, 0, 0);
		}else{
			rc = sqlite3_exec(db, va("DELETE FROM subnetbans WHERE IP='%s'", ip), 0, 0, 0);
		}
		
		if(rc != SQLITE_OK){
			if(adm && adm->client){
				trap_SendServerCommand( adm-g_entities, va("print \"^1[Error] ^7bans database: %s\n", sqlite3_errmsg(db)));
			}else{
				G_LogPrintf("^1Error: ^7bans database: %s\n", sqlite3_errmsg(db));
			}
			
			return;
		}else{
			if(adm && adm->client){
				trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7Unbanned %s (IP: %s) from line %s.\n\"", name, ip, ip2));
			}else{
				Com_Printf("^3[Info] ^7Unbanned %s (IP: %s) from line %s.\n", name, ip, ip2);
			}
			// Boe!Man 12/19/12: Copy the actual IP to ip2, for logging purposes.
			Q_strncpyz(ip2, ip, sizeof(ip2));
		}
	}
			
	
	// Boe!Man 12/20/12: Re-order the ROWIDs by issuing the VACUUM maintenance query.
	sqlite3_exec(db, "VACUUM", NULL, NULL, NULL);
	
	// Boe!Man 12/19/12: Log the unban.
	if(subnet){
		if(adm && adm->client){
			Boe_adminLog ("Subnet Unban", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", name, ip2));
		}else{
			Boe_adminLog ("Subnet Unban", "RCON", va("%s", name));
		}
	}else{
		if(adm && adm->client){
			Boe_adminLog ("Unban", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", name, ip2));
		}else{
			Boe_adminLog ("Unban", "RCON", va("%s", name));
		}
	}
	
	return;
}

/*
==================
Boe_NameListCheck
Removed by Boe: 2/26/13 - 3:22 PM
This function is old and crappy coded etc etc. and best of all, it's obsolete after the switch to SQLite. Yipppiiieee!
==================


int Boe_NameListCheck (int num, const char *name, const char *file, gentity_t *ent, qboolean banCheck, qboolean admCheck, qboolean subnetCheck, qboolean scoreCheck, qboolean cloneCheck)
{
	int             len;
	fileHandle_t	f;
	char            buf[15000];
	char			ip[MAX_IP];
	char			listName[64];
	char			*listP = listName;
	char			*bufP = buf;
	char			x[64] = "\0";
	char			a[64] = "\0";
	qboolean		banline = qfalse;
	int				realcount = 0;
	int i;
	int count;
	ent = g_entities + num;
	len = trap_FS_FOpenFile( file, &f, FS_READ_TEXT); 

	if (!f) { 
		len = trap_FS_FOpenFile( file, &f, FS_APPEND_TEXT);   
		if (!f) {  
			Boe_FileError(ent, file);
			return -1;
		}
		trap_FS_FCloseFile( f ); 
		len = trap_FS_FOpenFile( file, &f, FS_READ_TEXT); 

		if (!f) { 
			Boe_FileError(ent, file);
			return -1;
		}
	}

	if(len > 15000)	{
		len = 15000;
	}
	memset( buf, 0, sizeof(buf) );
	trap_FS_Read( buf, len, f );
	buf[len] = '\0';
	trap_FS_FCloseFile( f );

	if(f >= 5){ // if more than 5 then clear the handles
		for(i=0;i<=(int)f;i++){
		trap_FS_FCloseFile(f);
		}
	}

	if(f >= 10){
		G_LogPrintf( "!=!=!=!=!=!=!=!=WARNING=!=!=!=!=!=!=!=!=!=! %i open handles, report at 1fx.uk.to\n", f );
		Com_Printf("%i of 64 max handles used\n", f);
	}

	while( *bufP != '\0') {
		while(*bufP != '\n' && *bufP != '\0') {
			*listP++ = *bufP++;
			 if((banCheck || subnetCheck) && *bufP == '\\') { 
				 *listP = '\0';
				 Q_strncpyz(ip, listName, MAX_IP);
				 listP = listName;
				 banline = qtrue;
				 bufP++;
			 }
		}

		*listP = '\0';
		listP = listName;			
		if (admCheck){
			//for(count=0;count<=20;count++){
			//strcpy(a, va("%s:%i", name, count ));
			//if (!Q_stricmp(listName, a)) {
			//	return count;
			//}
			//}
			strcpy(a, va("%s:2", name ));
			if (!Q_stricmp(listName, a)) {
				return 2;
			}
			strcpy(a, va("%s:3", name ));
			if (!Q_stricmp(listName, a)) {
				return 3;
			}
			strcpy(a, va("%s:4", name ));
			if (!Q_stricmp(listName, a)) {
				return 4;
			} 
		}
		else if(banCheck && banline){
			if(!Q_stricmp(ip, name))
				return 1;
		}
		else if (subnetCheck && banline){
			if(strlen(ip) > 3){
				Q_strncpyz (x, name, strlen(ip) + 1);
				if(!Q_stricmp(ip, x))
					return 1;
			}
		}else if(scoreCheck){
			for(count=0;count<=100;count++){ // FIX ME HENK
			strcpy(a, va("%s:%i", name, count ));
			if (!Q_stricmp(listName, a)) {
				realcount = count;
			}
			}	
			if(realcount != 0){
				return realcount;
			}
		}else if(cloneCheck){
			if (strstr(listP, name)){
				if((strlen(listP)) == (strlen(name))){
					return 1;
				}
			}
		}if (!Q_stricmp(listName, name) && !admCheck && !subnetCheck)	{
			return 1;
		}
		while(*bufP == '\n') {
			bufP++;
		}
	}	
	return 0;
}
*/

/*
=================
Boe_subnetBan
12/16/12 - 11:36 AM
=================
*/
void Boe_subnetBan (int argNum, gentity_t *adm, qboolean shortCmd)
{
	int				 idnum, i;
	char			 reason[MAX_STRING_TOKENS] = "\0";
	char			 arg[64];
	char			 clientName[MAX_NETNAME], admName[MAX_NETNAME];
	char			*temp = "";
	char			ip[MAX_IP];
	qboolean		 first = qfalse;
	sqlite3			*db;
	
	idnum = Boe_ClientNumFromArg(adm, argNum, "subnetban <idnumber> <reason>", "subnetban", qfalse, qfalse, shortCmd);
	if(idnum < 0){
		return;
	}
	
	if(adm){
		if(shortCmd){
			strcpy(reason, GetReason());
		}else{
		//trap_Argv( argNum + 1, reason, sizeof( reason ) );
			for(i=0;i<=25;i++){
			trap_Argv( (argNum+1)+i, arg, sizeof( arg ) );
				if(first)
					temp = va("%s %s", temp, arg); // we fill this array up with 25 arguments
				else{
					temp = va("%s", arg);
					first = qtrue;
				}
			}
			strcpy(reason, temp);
		}
	}else{
		trap_Argv( argNum+1, arg, sizeof( arg ) );
		strcpy(reason, arg);
	}
	
	// Boe!Man 12/16/12: Store the 'subnet' in a seperate char array.
	Q_strncpyz(ip, g_entities[idnum].client->pers.ip, 7);
	
	// Boe!Man 1/9/12: Check for unsupported characters in the reason and replace them.
	Boe_convertNonSQLChars(reason);
	// Boe!Man 12/12/12: Also check those in the names, SQLite has massive problems when using quotes in the (updated) query.
	Q_strncpyz(clientName, g_entities[idnum].client->pers.cleanName, sizeof(clientName));
	Boe_convertNonSQLChars(clientName);
	if(adm){
		Q_strncpyz(admName, adm->client->pers.cleanName, sizeof(admName));
		Boe_convertNonSQLChars(admName);
	}
	
	// Boe!Man 5/27/13: Open database.
	db = bansDb;
	
	// Boe!Man 12/12/12: Insert query.
	if(adm && adm->client){
		if(sqlite3_exec(db, va("INSERT INTO subnetbans (IP, name, by, reason) values ('%s', '%s', '%s', '%s')", ip, clientName, admName, reason), 0, 0, 0) != SQLITE_OK){
			trap_SendServerCommand( adm-g_entities, va("print \"^1[Error] ^7bans database: %s\n", sqlite3_errmsg(db)));
			return;
		}
	}else{
		if(sqlite3_exec(db, va("INSERT INTO subnetbans (IP, name, by, reason) values ('%s', '%s', '%s', '%s')", ip, clientName, "RCON", reason), 0, 0, 0) != SQLITE_OK){
			G_LogPrintf("^1Error: ^7bans database: %s\n", sqlite3_errmsg(db));
			return;
		}
	}

	if(adm && adm->client){
		if(!*reason){
			trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"Subnetbanned by %s\"\n", idnum, adm->client->pers.netname));
			Boe_adminLog ("Subnetban", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", ip, g_entities[idnum].client->pers.cleanName));}
		else{
			trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"Subnetbanned by %s for: %s\"\n", idnum, adm->client->pers.netname, reason));
			//trap_SendServerCommand( adm-g_entities, va("print \"%s was Subnetbanned by %s!\n\"", g_entities[idnum].client->pers.netname, adm->client->pers.netname));
			Boe_adminLog ("Subnetban", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", ip, g_entities[idnum].client->pers.cleanName));
		}
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s was %ss%su%sb%sn%se%stbanned ^7by %s", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
	}
	else{
		if(!*reason){
			Boe_adminLog ("Subnetban", va("%s", "RCON"), va("%s\\%s", ip, g_entities[idnum].client->pers.cleanName));
			trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"Subnetbanned!\"\n", idnum));
		}else{
			//Com_Printf("%s was subnetbanned!\n", g_entities[idnum].client->pers.cleanName);
			Boe_adminLog ("Subnetban", va("%s", "RCON"), va("%s\\%s", ip, g_entities[idnum].client->pers.cleanName));
			trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"Subnetbanned for: %s\"\n", idnum, reason));
		}
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s was %ss%su%sb%sn%se%stbanned", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
	}
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
	
	return;
}

/*
==================
Boe_Remove_Admin_f
==================
*/

void Boe_Remove_Admin_f (int argNum, gentity_t *adm, qboolean shortCmd)
{
	int				idnum;
	qboolean		admin = qfalse;
	char			octet[5];
	
	idnum = Boe_ClientNumFromArg(adm, argNum, "removeadmin <idnumer>", "do this to", qfalse, qtrue, shortCmd);
	if(idnum < 0){
		return;
	}
	
	// Boe!Man 8/22/10: If the user's not an Admin we can safely skip this.
	if (g_entities[idnum].client->sess.admin){
		g_entities[idnum].client->sess.admin = 0;
		admin = qtrue;
	}else{
		if(adm && adm->client){
			trap_SendServerCommand(adm-g_entities, va("print\"^3[Info] ^7%s is not an Admin!\n\"", g_entities[idnum].client->pers.cleanName));
		}else{
			Com_Printf("^3[Info] ^7%s is not an Admin!\n", g_entities[idnum].client->pers.cleanName);
		}
		
		return;
	}
	
	// Boe!Man 4/30/12: Write the octet, with the first 3 chars of the IP (which should represent the first octet).
	Q_strncpyz(octet, g_entities[idnum].client->pers.ip, 4);
	
	// Boe!Man 1/6/10: Fix, succesfully writes the Admin out of the file.
	// Update 8/10/11: Do display the broadcast even if he's not written out of the file. We already verified he was an Admin.
	// Update 4/30/12: Added the password Admin check. First we check if the user is on the regular file, then we check for the pass file. If all fails, just display the broadcast.
	// Update 2/6/13: SQLite3 backend. Most of this was recoded.
	if(Boe_removeAdminFromDb(adm, g_entities[idnum].client->pers.ip, qfalse, qfalse, qtrue) || Boe_removeAdminFromDb(adm, octet, qtrue, qfalse, qtrue) || admin){
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7is no longer an %sA%sd%sm%si%sn", level.time + 5000, g_entities[idnum].client->pers.netname, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		if(adm && adm->client){
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was removed as Admin by %s.\n\"", g_entities[idnum].client->pers.netname,adm->client->pers.netname));
		}
		else{
			trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s ^7was removed as Admin.\n\"", g_entities[idnum].client->pers.netname));
		}
	}

	// Boe!Man 10/16/10: He's not an Admin anymore so it doesn't matter if he was a B-Admin, Admin or S-Admin: either way he shouldn't be allowed to spec the opposite team.
	g_entities[idnum].client->sess.adminspec = qfalse;
	
	return;
}

/*
=========
Boe_Ban_f
12/12/12 - 11:06 PM
=========
*/

void Boe_Ban_f (int argNum, gentity_t *adm, qboolean shortCmd)
{
	int				 idnum, i;
	char			 reason[MAX_STRING_TOKENS] = "\0";
	char			 arg[64];
	char			 clientName[MAX_NETNAME], admName[MAX_NETNAME];
	char			*temp = "";
	qboolean		 first = qfalse;
	sqlite3			*db;
	int 			start; 
	
	// Boe!Man 1/18/13: Benchmark purposes.
	if(sql_timeBench.integer){
		start = trap_Milliseconds();
	}
	
	idnum = Boe_ClientNumFromArg(adm, argNum, "ban <idnumber> <reason>", "ban", qfalse, qfalse, shortCmd);
	if(idnum < 0){
		return;
	}
	
	if(adm){
		if(shortCmd){
			strcpy(reason, GetReason());
		}else{
		//trap_Argv( argNum + 1, reason, sizeof( reason ) );
			for(i=0;i<=25;i++){
			trap_Argv( (argNum+1)+i, arg, sizeof( arg ) );
				if(first)
					temp = va("%s %s", temp, arg); // we fill this array up with 25 arguments
				else{
					temp = va("%s", arg);
					first = qtrue;
				}
			}
			strcpy(reason, temp);
		}
	}else{
		trap_Argv( argNum+1, arg, sizeof( arg ) );
		strcpy(reason, arg);
	}
	
	// Boe!Man 1/9/12: Check for unsupported characters in the reason and replace them.
	Boe_convertNonSQLChars(reason);
	// Boe!Man 12/12/12: Also check those in the names, SQLite has massive problems when using quotes in the (updated) query.
	Q_strncpyz(clientName, g_entities[idnum].client->pers.cleanName, sizeof(clientName));
	Boe_convertNonSQLChars(clientName);
	if(adm){
		Q_strncpyz(admName, adm->client->pers.cleanName, sizeof(admName));
		Boe_convertNonSQLChars(admName);
	}
	
	// Boe!Man 12/12/12: Open database.
	db = bansDb;
	
	// Boe!Man 12/12/12: Insert query.
	if(adm && adm->client){
		if(sqlite3_exec(db, va("INSERT INTO bans (IP, name, by, reason) values ('%s', '%s', '%s', '%s')", g_entities[idnum].client->pers.ip, clientName, admName, reason), 0, 0, 0) != SQLITE_OK){
			trap_SendServerCommand( adm-g_entities, va("print \"^1[Error] ^7bans database: %s\n", sqlite3_errmsg(db)));
			return;
		}
	}else{
		if(sqlite3_exec(db, va("INSERT INTO bans (IP, name, by, reason) values ('%s', '%s', '%s', '%s')", g_entities[idnum].client->pers.ip, clientName, "RCON", reason), 0, 0, 0) != SQLITE_OK){
			G_LogPrintf("^1Error: ^7bans database: %s\n", sqlite3_errmsg(db));
			return;
		}
	}

	if(adm && adm->client){
		if(!*reason){
			trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"Banned by %s\"\n", idnum, adm->client->pers.netname));
			Boe_adminLog ("Ban", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));
		}
		else{
			trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"Banned by %s for: %s\"\n", idnum, adm->client->pers.netname, reason));
			Boe_adminLog ("Ban", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));
		}
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s was %sb%sa%sn%sn%se%sd ^7by %s", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
	}
	else{
		if(!*reason){
			Boe_adminLog ("Ban", va("%s", "RCON"), va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));
			trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"Banned!\"\n", idnum));
		}
		else{
			Boe_adminLog ("Ban", va("%s", "RCON"), va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));
			trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"Banned for: %s\"\n", idnum, reason));
		}
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s was %sb%sa%sn%sn%se%sd", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
	}
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
	
	if(sql_timeBench.integer){
		Com_Printf("Ban took: %ims\n", trap_Milliseconds()-start);
	}
	
	return;
}

/*
============
Boe_Uppercut
============
*/
void Boe_Uppercut (int argNum, gentity_t *adm, qboolean shortCmd)
{
	gentity_t		*ent;
	int				idnum;
//	char			*status;
	int				uclevel;
	char			arg[64] = "\0";
	idnum = Boe_ClientNumFromArg(adm, argNum, "uppercut <idnumber>", "uppercut", qtrue, qtrue, shortCmd);
	if(idnum < 0) return;
	
	ent = g_entities + idnum;
	ent->client->ps.pm_flags |= PMF_JUMPING;
	ent->client->ps.groundEntityNum = ENTITYNUM_NONE;
	// Boe!Man 5/3/10: We higher the uppercut.
	if(shortCmd){
		uclevel = atoi(GetReason());
		//if(uclevel == 0){
		//trap_Argv( 3, arg, sizeof( arg ) );
		//uclevel = atoi(arg);
		//}
		// Boe!Man 1/13/11: Changing uppercut levels.
		if(uclevel == 0){
			ent->client->ps.velocity[2] = 1000;
		}else{
			ent->client->ps.velocity[2] = 200*uclevel;
		}
	}
	
	// Boe!Man 1/28/11: Check for the uppercut level other than when called from ! commands.
	else{
		if(adm&&adm->client){	// If called from /adm..
			trap_Argv(3, arg, sizeof(arg));
		}else{					// If called from /rcon..
			trap_Argv(2, arg, sizeof(arg));
		}
		uclevel = atoi(arg);
		if (uclevel == 0){
			ent->client->ps.velocity[2] = 1000;
		}else{
			ent->client->ps.velocity[2] = 200*uclevel;
		}
	}

	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));

	// Boe!Man 11/2/10: Added client sound.
	Boe_ClientSound(ent, G_SoundIndex("sound/weapons/rpg7/fire01.mp3"));
	
	/* NOTE (ajay#1#): lastIdentityChange issue: interesting, I don't know why we included this. Keeping this for archival purposes. */
	/*
	if(g_entities[idnum].client->sess.lastIdentityChange)	{
		status = ".";
		g_entities[idnum].client->sess.lastIdentityChange = qfalse;
	}
	else {
		status = ".";
		g_entities[idnum].client->sess.lastIdentityChange = qtrue;
	}
	*/
	
	if(adm && adm->client){
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,^7%s was %su%sp%sp%se%sr%scut by %s", level.time + 5000, ent->client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was uppercut by %s.\n\"", ent->client->pers.netname,adm->client->pers.netname));
		Boe_adminLog ("Uppercut", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName));
	}
	else {
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,^7%s was %su%sp%sp%se%sr%scut", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s was uppercut.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog ("Uppercut", va("%s", "RCON"), va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));
	}
}

/*
=========
Boe_Twist
=========
*/

void Boe_Twist (int argNum, gentity_t *adm, qboolean shortCmd)
{
	gentity_t		*ent;
	int				idnum;
	vec3_t			lookdown;
	//char			*a, *b, *c;
	char			a[4], b[4], c[4];


	idnum = Boe_ClientNumFromArg(adm, argNum, "twist <idnumber>", "twist", qtrue, qtrue, shortCmd);
	if(idnum < 0)  return;

	ent = g_entities + idnum;
	// Boe!Man 1/13/11: If the client is already twisted, untwist him.
	if(ent->client->pers.twisted == qtrue){
		Boe_unTwist(argNum, adm, shortCmd);
		return;
	}

	trap_Argv( argNum + 1, c, sizeof(c) );
	trap_Argv( argNum + 2, b, sizeof(b) );
	trap_Argv( argNum + 3, a, sizeof(a) );
	if ( !c[0] ) {
	VectorSet(lookdown, 100, 0, 130);
	}
	else {
	VectorSet(lookdown, atoi(a), atoi(b), atoi(c));
	}

	SetClientViewAngle(ent, lookdown, qfalse);
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
	ent->client->pers.twisted = qtrue;

	if(adm && adm->client) {
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,^7%s was %st%sw%si%ss%st%sed by %s", level.time + 5000, ent->client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was twisted by %s.\n\"", ent->client->pers.netname,adm->client->pers.netname));
		Boe_adminLog ("Twist", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName));
	}
	else {
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,^7%s was %st%sw%si%ss%st%sed", level.time + 5000, ent->client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s was twisted.\n\"", ent->client->pers.netname));
		Boe_adminLog ("Twist", va("%s", "RCON"), va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName));
	}
}

/*
===========
Boe_unTwist
===========
*/

void Boe_unTwist (int argNum, gentity_t *adm, qboolean shortCmd)
{
	gentity_t	*ent;
	int			idnum;
	vec3_t		lookdown;

	idnum = Boe_ClientNumFromArg(adm, argNum, "untwist <idnumber>", "untwist", qtrue, qtrue, shortCmd);
	if(idnum < 0) {	return;	}

	ent = g_entities + idnum;

	// Boe!Man 1/13/11: An Admin should not be able to untwist or twist someone when this command is called, IF the client is not already twisted.
	if (ent->client->pers.twisted == qfalse){
		if (adm && adm->client){
			trap_SendServerCommand(adm-g_entities, va("print\"^3[Info] ^7This client is currently not twisted.\n\"", ent->client->pers.cleanName));
		}else{
			Com_Printf("This client is currently not twisted.\n");
		}
		return;
	}

	VectorSet(lookdown, 0, 0, 0);	
	SetClientViewAngle(ent, lookdown, qfalse);
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
	ent->client->pers.twisted = qfalse;

	if(adm && adm->client){
		// Boe!Man 6/11/10: Double "Twisted" message bug fixed.
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,^7%s was %su%sn%st%sw%si%ssted by %s", level.time + 5000, ent->client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was untwisted by %s.\n\"", ent->client->pers.netname,adm->client->pers.netname));
		Boe_adminLog ("Untwist", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName));
	}
	else {
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,^7%s was %su%sn%st%sw%si%ssted", level.time + 5000, ent->client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s was untwisted.\n\"", ent->client->pers.netname));
		Boe_adminLog ("Untwist", va("%s", "RCON"), va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName));
	}
}

/*
===========
Boe_Respawn
===========
*/

void Boe_Respawn (int argNum, gentity_t *adm, qboolean shortCmd)
{
	gentity_t		*ent;
//	qboolean		ghost;
	int				idnum;
//	char	*status;

	idnum = Boe_ClientNumFromArg(adm, argNum, "respawn <idnumber>", "respawn", qfalse, qtrue, shortCmd);
	if(idnum < 0) return;
	
	ent = g_entities + idnum;

	if ( ent->client->sess.team == TEAM_SPECTATOR ){
		if (adm && adm->client)
			trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7You cannot respawn a spectator.\n\""));
		else
			Com_Printf("You cannot respawn a spectator.\n");
		return;
	}

	if ( ent->client->sess.ghost ){
			G_StopFollowing ( ent );
			ent->client->ps.pm_flags &= ~PMF_GHOST;
			ent->client->ps.pm_type = PM_NORMAL;
			ent->client->sess.ghost = qfalse;
	}else{
		TossClientItems(ent);
	}
	ent->client->sess.noTeamChange = qfalse;
	trap_UnlinkEntity (ent);
	ClientSpawn(ent);
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
	Boe_ClientSound(ent, G_SoundIndex("sound/ambience/vehicles/telephone_pole.mp3"));

	/* NOTE (ajay#1#): lastIdentityChange issue: interesting, I don't know why we included this. Keeping this for archival purposes. */
	/*
	if(ent->client->sess.lastIdentityChange){
		status = ".";
		ent->client->sess.lastIdentityChange = qfalse;
	}
	else {
		status = ".";
		ent->client->sess.lastIdentityChange = qtrue;
	}
	*/
	
	if(adm && adm->client){
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,^7%s was %sr%se%ss%sp%sa%swned by %s", level.time + 5000, ent->client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was respawned by %s.\n\"", ent->client->pers.netname,adm->client->pers.netname));
		Boe_adminLog ("Respawn", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName));
	}
	else {
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,^7%s was %sr%se%ss%sp%sa%swned", level.time + 5000, ent->client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s was respawned.\n\"", ent->client->pers.netname));
		Boe_adminLog ("Respawn", va("%s", "RCON"), va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName));
	}
}

/*
======================
Boe_Runover
======================
*/
void Boe_Runover (int argNum, gentity_t *adm, qboolean shortCmd)
{
	gentity_t	*ent;
	vec3_t	dir;
	vec3_t  fireAngs;
	float   knockback = 400.0;
	int		idnum;
//	int		anim = 0;
	
	idnum = Boe_ClientNumFromArg(adm, argNum, "runover <idnumber>", "runover", qtrue, qtrue, shortCmd);
	if(idnum < 0)
	{
		return;
	}

	ent = g_entities + idnum;

	VectorCopy(ent->client->ps.viewangles, fireAngs);
	AngleVectors( fireAngs, dir, NULL, NULL );	
	dir[0] *= -1.0;
	dir[1] *= -1.0;
	dir[2] = 0.0;
	VectorNormalize ( dir );

	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
	Boe_ClientSound(ent, G_SoundIndex("sound/ambience/vehicles/hit_scrape.mp3"));
	ent->client->ps.weaponstate = WEAPON_FIRING;
	ent->client->ps.velocity[2] = 20;
	ent->client->ps.weaponTime = 3000;

	if(adm && adm->client){
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sr%su%sn%so%sv%ser by %s", level.time + 5000, ent->client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was runover by %s.\n\"", ent->client->pers.netname,adm->client->pers.netname));
		Boe_adminLog ("Runover", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName));
	}
	else{
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sr%su%sn%so%sv%ser", level.time + 5000, ent->client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s ^7was runover.\n\"", ent->client->pers.netname));
		Boe_adminLog ("Runover", va("%s", "RCON"), va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName));
	}	
	G_Damage (ent, NULL, NULL, NULL, NULL, 15, 0, MOD_CAR, HL_NONE );
	G_ApplyKnockback ( ent, dir, knockback );
}

/*
=========
Boe_Flash
=========
*/

void Boe_Flash (int argNum, gentity_t *adm, qboolean shortCmd)
{
	gentity_t		*ent;
	int				idnum;
	vec3_t dir;
	int it, nadeDir, weapon;
	float x, y;
	gentity_t *missile;
	idnum = Boe_ClientNumFromArg(adm, argNum, "flash <idnumber>", "flash", qtrue, qtrue, shortCmd);
	if(idnum < 0) return;
	ent = g_entities + idnum;
	weapon = WP_M84_GRENADE;
	nadeDir = 1;
	for( it = 0; it < 1; it++ ) {
		x = 100 * cos( DEG2RAD(nadeDir * it));  
		y = 100 * sin( DEG2RAD(nadeDir * it));
		VectorSet( dir, x, y, 100 );
		dir[2] = 300;	
		missile = NV_projectile( ent, ent->r.currentOrigin, dir, weapon, 0 );
		missile->nextthink = level.time + 250;
	}
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
	if(adm && adm->client){
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sf%sl%sa%ss%sh%sed by %s", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was flashed by %s.\n\"", g_entities[idnum].client->pers.netname,adm->client->pers.netname));
		Boe_adminLog ("Flash", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));
	}
	else{
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sf%sl%sa%ss%sh%sed", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s ^7was flashed.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog ("Flash", va("%s", "RCON"), va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));
	}	
}

/*
=========
Boe_Plant
=========
*/

void Boe_Plant (int argNum, gentity_t *adm, qboolean shortCmd)
{
	gentity_t	*ent;
	int			idnum;

	idnum = Boe_ClientNumFromArg(adm, argNum, "plant <idnumber>", "Plant", qtrue, qtrue, shortCmd);

	if(idnum < 0)
		return;

	ent = g_entities + idnum;

	// Boe!Man 1/13/11: If the client is already planted, untwist him.
	if(ent->client->pers.planted == qtrue){
		Boe_unPlant(argNum, adm, shortCmd);
		return;
	}

	/*
	if(ent->client->pers.planted){
		if(adm && adm->client)
			trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7%s ^7is already planted.\n\"", ent->client->pers.netname));
		else
			Com_Printf("^3%s is already Planted.\n", ent->client->pers.netname);
		return;
	}
	*/

	if (ent->client->ps.pm_flags & PMF_DUCKED){
		ent->client->ps.origin[2] -=40;
	}else{
		ent->client->ps.origin[2] -= 65;
	}

	VectorCopy( ent->client->ps.origin, ent->s.origin );
	ent->client->pers.planted = qtrue;

	// Boe!Man 1/29/10: All clients will need to hear the event sound, though only the victim will have to hear the wood breaking sound.
	Boe_ClientSound(ent, G_SoundIndex("sound/misc/confused/wood_break.mp3"));
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));

	if(adm && adm->client){
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sp%sl%sa%sn%st%sed by %s", level.time + 5000, ent->client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was planted by %s.\n\"", ent->client->pers.netname,adm->client->pers.netname));
		Boe_adminLog ("Plant", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName));
	}
	else{
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sp%sl%sa%sn%st%sed", level.time + 5000, ent->client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s ^7was planted.\n\"", ent->client->pers.netname));
		Boe_adminLog ("Plant", va("%s", "RCON"), va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName));
	}
}

/*
===========
Boe_unPlant
===========
*/

void Boe_unPlant (int argNum, gentity_t *adm, qboolean shortCmd)
{
	gentity_t	*ent;
	int			idnum;

	idnum = Boe_ClientNumFromArg(adm, argNum, "unplant <idnumber>", "unplant", qtrue, qtrue, shortCmd);

	if(idnum < 0){return;}
	ent = g_entities + idnum;

	if(!ent->client->pers.planted){
		if(adm && adm->client)
			trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7This client is currently not planted.\n\"", ent->client->pers.netname));
		 else
			Com_Printf("^7This client is currently not planted.\n", ent->client->pers.netname);
		return;
	}

	if(ent->client->pers.planted){
		ent->client->ps.origin[2] += 65;
		VectorCopy( ent->client->ps.origin, ent->s.origin );
		ent->client->pers.planted = qfalse;
	}

	Boe_ClientSound(ent, G_SoundIndex("sound/misc/confused/wood_break.mp3"));
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));

	if(adm && adm->client){
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %su%sn%sp%sl%sa%snted by %s", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was unplanted by %s.\n\"", g_entities[idnum].client->pers.netname,adm->client->pers.netname));
		Boe_adminLog ("Unplant", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));
	}
	else{
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %su%sn%sp%sl%sa%snted", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s ^7was unplanted.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog ("Unplant", va("%s", "RCON"), va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));
	}
}

/*
=======
Boe_pop
=======
*/

void Boe_pop (int argNum, gentity_t *adm, qboolean shortCmd)
{
	gentity_t		*ent;
	int				idnum;
	//int				anim = 0;

	idnum = Boe_ClientNumFromArg(adm, argNum, "pop <idnumber>", "pop", qtrue, qfalse, shortCmd);
	if(idnum < 0){return;}
	ent = g_entities + idnum;
	
	Boe_ClientSound(ent, G_SoundIndex("sound/npc/air1/guard02/laughs.mp3"));
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));

	if(adm && adm->client){
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sp%so%sp%sp%se%sd by %s", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was popped by %s.\n\"", g_entities[idnum].client->pers.netname,adm->client->pers.netname));
		Boe_adminLog ("Pop", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));
	}
	else{
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sp%so%sp%sp%se%sd", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s ^7was popped.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog ("Pop", va("%s", "RCON"), va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));
	}
	// Boe!Man 1/30/10: We use g_damage since the animations won't work.
	G_Damage (ent, NULL, NULL, NULL, NULL, 10000, 0, MOD_POP, HL_HEAD|HL_FOOT_RT|HL_FOOT_LT|HL_LEG_UPPER_RT|HL_LEG_UPPER_LT|HL_HAND_RT|HL_HAND_LT|HL_WAIST|HL_CHEST|HL_NECK);
}

/*
========
Boe_Broadcast
========
*/

void Boe_Broadcast(int argNum, gentity_t *adm, qboolean shortCmd){
	char buffer[512];
	char buffer1[512];
	char *buffer2;
	int i, z = 0;
	if(shortCmd){
		trap_Argv(1, buffer, sizeof(buffer));
		buffer2 = ConcatArgs1( 2 );
		if(strlen(buffer2) < 1){
			// Boe!Man 12/11/11: If someone's trying to broadcast nothing, buffer start will be 0. Don't allow admins to broadcast the actual "!br" command.
			if(StartAfterCommand(va("%s", buffer)) != 0){
				for(i=StartAfterCommand(va("%s", buffer));i<=strlen(buffer);i++){
					buffer1[z] = buffer[i];
					z+= 1;
				}
			}
		buffer1[z] = '\0';
		}else{
			strcpy(buffer1, buffer2);
		}
	}else{
		// Boe!Man 1/14/11: Fixed Broadcast with RCON.
		if(adm && adm->client){
			strcpy(buffer1, ConcatArgs1(2));
			//trap_Argv(2, buffer1, sizeof(buffer));
		}else{
			strcpy(buffer1, ConcatArgs1(1));
			//trap_Argv(1, buffer1, sizeof(buffer));
		}
	}
	
	trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@\"%s\"", level.time + 5000, buffer1));
	Boe_GlobalSound (G_SoundIndex("sound/misc/menus/invalid.wav"));

	if(adm && adm->client){
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Broadcast by %s.\n\"", adm->client->pers.netname));
		//Boe_adminLog (va("%s - BROADCAST: %s", adm->client->pers.cleanName, buffer1  )) ; // Boe!Man 3/13/11: Disabled this Log.. Pretty useless, we don't log !at either.
	}else{
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7Broadcast.\n\""));
		//Boe_adminLog (va("%s - BROADCAST: %s", "RCON", buffer1  )) ; // Boe!Man 3/13/11: Disabled this Log.. Pretty useless, we don't log !at either.
	}
	//trap_SendServerCommand( -1, va("cp \"%s\n\"", buffer) );
}

/*
========
Boe_Burn
========
*/
void Boe_Burn (int argNum, gentity_t *adm, qboolean shortCmd)
{
	gentity_t		*ent;
	int				idnum;
	gentity_t *tent; 

	idnum = Boe_ClientNumFromArg(adm, argNum, "burn <idnumber>", "burn", qtrue, qtrue, shortCmd);
	if(idnum < 0) return;
	ent = g_entities + idnum;
	ent->client->sess.burnSeconds = 4;

	tent = G_TempEntity( g_entities[ent->s.number].r.currentOrigin, EV_EXPLOSION_HIT_FLESH ); 
	tent->s.eventParm = 0; 
	tent->s.otherEntityNum2 = g_entities[ent->s.number].s.number; 
	tent->s.time = WP_ANM14_GRENADE + ((((int)g_entities[ent->s.number].s.apos.trBase[YAW]&0x7FFF) % 360) << 16); 
	VectorCopy ( g_entities[ent->s.number].r.currentOrigin, tent->s.angles ); 
	SnapVector ( tent->s.angles ); 

	Boe_ClientSound(ent, G_SoundIndex("/sound/weapons/incendiary_grenade/incen01.mp3"));
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));

	if(adm && adm->client){
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sb%su%sr%sn%se%sd by %s", level.time + 5000, ent->client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was burned by %s.\n\"", ent->client->pers.netname,adm->client->pers.netname));
		Boe_adminLog ("Burn", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName));
	}
	else{
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sb%su%sr%sn%se%sd", level.time + 5000, ent->client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s ^7was burned.\n\"", ent->client->pers.netname));
		Boe_adminLog ("Burn", va("%s", "RCON"), va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName));
	}
}

void Adm_ForceTeam(int argNum, gentity_t *adm, qboolean shortCmd)
{
	char		str[MAX_TOKEN_CHARS];
	int			idnum;
	int			i, xteam;
	qboolean	pass = qfalse;
	char		team;
	char		merged[64];
	qboolean	all = qfalse; // Boe!Man 4/15/13: If this is true, forceteam all players to a specific team.
	char		userinfo[MAX_INFO_STRING];
	
	// find the player
	if(shortCmd == qtrue){
		idnum = Boe_ClientNumFromArg(adm, 1, "forceteam <idnumber>", "forceteam", qfalse, qtrue, shortCmd);
	}else{
		if(adm){
			idnum = Boe_ClientNumFromArg(adm, 2, "forceteam <idnumber>", "forceteam", qfalse, qtrue, shortCmd);
		}else{
			idnum = Boe_ClientNumFromArg(adm, 1, "forceteam <idnumber>", "forceteam", qfalse, qtrue, shortCmd);
		}
	}
		
	if(idnum < 0){
		// Boe!Man 4/15/13: Check for "all".
		if(shortCmd){
			trap_Argv(1, str, sizeof(str));
		}else{
			if(adm && adm->client){
				trap_Argv( 2, str, sizeof( str ) );
			}else{
				trap_Argv( 1, str, sizeof( str ) );
			}
		}
		
		if(strstr(Q_strlwr(str), "all")){
			all = qtrue;
		}else{
			return;
		}
	}

	// set the team
	if(shortCmd == qtrue){
		trap_Argv(1, str, sizeof(str));
		
		for(i=0;i<strlen(str);i++){
			if(pass == qtrue && str[i] == ' '){
				team = str[i+1];
				break;
			}
			if(str[i] == ' ')
				pass = qtrue;
		}
		if(team == 's' || team == 'S'){
			strcpy(str, "spectator");
			xteam = TEAM_SPECTATOR;
		}else if(team == 'r' || team == 'R'){
			strcpy(str, "red");
			xteam = TEAM_RED;
		}else if(team == 'b' || team == 'B'){
			strcpy(str, "blue");
			xteam = TEAM_BLUE;
		}else{
			trap_Argv(3, str, sizeof(str));
			if(str[0] == 's' || str[0] == 'S'){
				strcpy(str, "spectator");
				xteam = TEAM_SPECTATOR;
			}else if(str[0] == 'r' || str[0] == 'R'){
				strcpy(str, "red");
				xteam = TEAM_RED;
			}else if(str[0] == 'b' || str[0] == 'B'){
				strcpy(str, "blue");
				xteam = TEAM_BLUE;
			}else{
				trap_SendServerCommand(adm->s.number, va("print\"^3[Info] ^7Wrong team: %s.\n\"", str));
				return;
			}
		}
	}else{
		// Boe!Man 1/13/11: Fix for Forceteam not working with RCON system.
		if(adm && adm->client){
			trap_Argv( 3, str, sizeof( str ) );
		}else{
			trap_Argv( 2, str, sizeof( str ) );
		}
		
		if(str[0] == 's' || str[0] == 'S'){
				strcpy(str, "spectator");
				xteam = TEAM_SPECTATOR;
			}else if(str[0] == 'r' || str[0] == 'R'){
				strcpy(str, "red");
				xteam = TEAM_RED;
			}else if(str[0] == 'b' || str[0] == 'B'){
				strcpy(str, "blue");
				xteam = TEAM_BLUE;
			}else{
				trap_SendServerCommand(adm->s.number, va("print\"^3[Info] ^7Wrong team: %s.\n\"", str));
				return;
			}
	}
	
	// Boe!Man 4/15/13: Check if we can forceteam all.
	if(!all){
		if(g_entities[idnum].client){
			if(g_entities[idnum].r.svFlags & SVF_BOT){ // Henk 25/01/11 -> Reset bots to set them to another team
				trap_GetUserinfo( idnum, userinfo, sizeof( userinfo ) );
				Info_SetValueForKey( userinfo, "team", str );
				trap_SetUserinfo( idnum, userinfo );
				g_entities[idnum].client->sess.team = (team_t)xteam;
				if(current_gametype.value != GT_HS)
					g_entities[idnum].client->pers.identity = BG_FindTeamIdentity ( level.gametypeTeam[xteam], -1 );
				ClientBegin( idnum, qfalse );
			}else{
				SetTeam( &g_entities[idnum], str, NULL, qtrue );
			}
		}else{
			trap_SendServerCommand(adm->s.number, va("print\"^3[Info] Error, no client on id %i.\n\"", idnum));
			return;
		}
		
		Com_sprintf( merged, sizeof(merged), "%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName);
		// Boe!Man 2/13/11: Proper messaging..
		if(adm){
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sf%so%sr%sc%se%steamed by %s", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was forceteamed by %s.\n\"", g_entities[idnum].client->pers.cleanName,adm->client->pers.cleanName));
			Boe_adminLog (va("Forceteam to %c", team), va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), merged);
		}else{
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sf%so%sr%sc%se%steamed", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
			trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s ^7was forceteamed.\n\"", g_entities[idnum].client->pers.cleanName));
			Boe_adminLog (va("Forceteam to %c", team), va("%s", "RCON"), merged);
		}
	}else{ // Boe!Man 4/15/13: Forceteam all.
		for(i=0; i<level.numConnectedClients; i++){
			if(g_entities[level.sortedClients[i]].r.svFlags & SVF_BOT){ // Henk 25/01/11 -> Reset bots to set them to another team
				trap_GetUserinfo( g_entities[level.sortedClients[i]].s.number, userinfo, sizeof( userinfo ) );
				Info_SetValueForKey( userinfo, "team", str );
				trap_SetUserinfo( g_entities[level.sortedClients[i]].s.number, userinfo );
				g_entities[level.sortedClients[i]].client->sess.team = (team_t)xteam;
				if(current_gametype.value != GT_HS)
					g_entities[level.sortedClients[i]].client->pers.identity = BG_FindTeamIdentity ( level.gametypeTeam[xteam], -1 );
				ClientBegin( g_entities[level.sortedClients[i]].s.number, qfalse );
			}else{
				SetTeam( &g_entities[level.sortedClients[i]], str, NULL, qtrue );
			}
		}
		
		// Boe!Man 4/15/13: Messaging for 'all'.
		if(adm){
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@Everybody was %sf%so%sr%sc%se%steamed by %s", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Everybody was forceteamed by %s.\n\"", adm->client->pers.cleanName));
			Boe_adminLog (va("Forceteam to %c", team), va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), "All");
		}else{
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@Everybody was %sf%so%sr%sc%se%steamed", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
			trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7Everybody was forceteamed.\n\""));
			Boe_adminLog (va("Forceteam to %c", team), va("%s", "RCON"), "All");
		}
	}
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
}

/*
========
Boe_Mute
========
*/

void Boe_UnMute(int argNum, gentity_t *ent, qboolean shortCmd){
	Boe_Mute (argNum, ent, qfalse, shortCmd);
}

void Boe_XMute(int argNum, gentity_t *ent, qboolean shortCmd){
	Boe_Mute (argNum, ent, qtrue, shortCmd);
}

void Boe_Mute (int argNum, gentity_t *adm, qboolean mute, qboolean shortCmd)
{
	int		idnum;
	int		time;
	char	arg[128];
	char	merged[64];
	idnum = Boe_ClientNumFromArg(adm, argNum, "mute/unmute <idnumber>", "mute/unmute", qfalse, qfalse, shortCmd);
	if(idnum < 0) return;


	if (mute == qtrue){
		// Boe!Man 1/13/11: If the client's already muted, and the player called mute, unmute him instead.
		if(g_entities[idnum].client->sess.mute == qtrue){
			//trap_SendServerCommand(adm-g_entities, va("print \"^3[Info] ^7This client is already muted!\n\""));}
			Boe_Mute(argNum, adm, qfalse, shortCmd);
			return;
		}else{
			if(shortCmd){
				if(atoi(GetReason()) <= 0)
					time = 5;
				else if(atoi(GetReason()) > 60){
					trap_SendServerCommand(adm-g_entities, va("print \"^3[Info] ^7The maximum timelimit for mute is 60 minutes.\n\""));
					time = 60;
				}else
					time = atoi(GetReason());
			}else{
				if(adm&&adm->client){	// If called from /adm..
					trap_Argv(3, arg, sizeof(arg));
					time = atoi(arg);
					if(time <= 0)
						time = 5;
					else if(time > 60)
						time = 60;
				}else{					// If called from /rcon..
					trap_Argv(2, arg, sizeof(arg));
					time = atoi(arg);
					if(time <= 0)
						time = 5;
					else if(time > 60)
						time = 60;
				}
			}
			AddMutedClient(&g_entities[idnum], time);
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			Com_sprintf( merged, sizeof(merged), "%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName);
			if(adm && adm->client){
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sm%su%st%se%sd by %s", level.time + 5000, g_entities[idnum].client->pers.netname, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
				trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was muted by %s for %i minutes.\n\"", g_entities[idnum].client->pers.netname, adm->client->pers.netname, time));
				Boe_adminLog (va("Mute %i minutes", time), va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), merged);
			}
			else{
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sm%su%st%se%sd", level.time + 5000, g_entities[idnum].client->pers.netname, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
				trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s ^7was muted for %i minutes.\n\"", g_entities[idnum].client->pers.netname, time));
				Boe_adminLog (va("Mute %i minutes", time), va("%s", "RCON"), merged);
			}
			return;
		}
	}if(mute == qfalse){
		if(g_entities[idnum].client->sess.mute == qfalse){
			if(adm && adm->client){
				trap_SendServerCommand(adm-g_entities, va("print \"^3[Info] ^7This client is currently not muted.\n\""));
			}else{
				Com_Printf("This client is currently not muted.\n");
			}
			return;
		}
		else{
			RemoveMutedClient(&g_entities[idnum]);
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			// Boe!Man 12/11/11: RemoveMutedClient does this as well.
			//g_entities[idnum].client->sess.mute = qfalse;
			if(adm && adm->client){
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %su%sn%sm%su%st%sed by %s", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
				trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was unmuted by %s.\n\"", g_entities[idnum].client->pers.netname, adm->client->pers.netname));
				Boe_adminLog ("Unmute", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));
			}
			else{
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %su%sn%sm%su%st%sed", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
				trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s ^7was unmuted.\n\"", g_entities[idnum].client->pers.netname));
				Boe_adminLog ("Unmute", va("%s", "RCON"), va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));
			}
		}
	}
}

#ifdef _awesomeToAbuse
// Boe!Man 3/17/13: Disabled due to this command taking a lot of entities from the server, resulting in a server crash.
/*
void Henk_Box (int argNum, gentity_t *adm, qboolean shortCmd)
{
	gentity_t	*ent;
	int			idnum;
	vec3_t		org, ang;
	int test, i;
	idnum = Boe_ClientNumFromArg(adm, argNum, "box <idnumber>", "box", qtrue, qtrue, shortCmd);
	if(idnum < 0)
		return;

	ent    = g_entities + idnum;
	for(i=0;i<=5;i++){
	VectorCopy(ent->r.currentOrigin, org); // back
	org[0] += 35;
	org[1] -= 10;
	org[2] -= 40-(48*i);
	ang[0] = 0;
	ang[1] = 0;
	ang[2] = 0;
	test = SpawnBoxEx(org, ang);
	g_entities[test].think = G_GlassDie;
	g_entities[test].nextthink = level.time+5000;
	VectorCopy(ent->r.currentOrigin, org); // left
	org[0] += 10;
	org[1] -= 40;
	org[2] -= 40-(48*i);
	ang[0] = 0;
	ang[1] = 90;
	ang[2] = 0;
	test = SpawnBoxEx(org, ang);
	g_entities[test].think = G_GlassDie;
	g_entities[test].nextthink = level.time+5000;

	VectorCopy(ent->r.currentOrigin, org); // right
	org[0] += 10;
	org[1] += 60;
	org[2] -= 40-(48*i);
	ang[0] = 0;
	ang[1] = 90;
	ang[2] = 0;
	test = SpawnBoxEx(org, ang);
	g_entities[test].think = G_GlassDie;
	g_entities[test].nextthink = level.time+5000;
	VectorCopy(ent->r.currentOrigin, org); // front
	org[0] -= 50;
	org[1] -= 10;
	org[2] -= 40-(48*i);
	ang[0] = 0;
	ang[1] = 0;
	ang[2] = 0;
	test = SpawnBoxEx(org, ang);
	g_entities[test].think = G_GlassDie;
	g_entities[test].nextthink = level.time+5000;
	}
}
*/

void Boe_freakOut(gentity_t *adm)
{
	int idnum;
	
	idnum = Boe_ClientNumFromArg(adm, 2, "", "", qtrue, qtrue, qfalse);
	if(!idnum)
		return;
	
	Boe_ClientSound(&g_entities[idnum], G_SoundIndex("sound/misc/outtakes/ben_g.mp3"));
}
#endif


/*
=========
Boe_Strip
=========
*/
void Boe_Strip (int argNum, gentity_t *adm, qboolean shortCmd)
{
	gentity_t	*ent;
	gclient_t	*client;
	int			idnum;
	int			idle;

	idnum = Boe_ClientNumFromArg(adm, argNum, "strip <idnumber>", "strip", qtrue, qtrue, shortCmd);
	if(idnum < 0)
		return;

	ent    = g_entities + idnum;
	client = ent->client;
	
	ent->client->ps.zoomFov = 0;
	ent->client->ps.pm_flags &= ~(PMF_GOGGLES_ON|PMF_ZOOM_FLAGS);
	client->ps.stats[STAT_WEAPONS] = 0;
	client->ps.stats[STAT_GOGGLES] = GOGGLES_NONE;  
	memset ( client->ps.ammo, 0, sizeof(client->ps.ammo) );
	memset ( client->ps.clip, 0, sizeof(client->ps.clip) );
	client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_KNIFE );
	client->ps.clip[ATTACK_NORMAL][WP_KNIFE]=weaponData[WP_KNIFE].attack[ATTACK_NORMAL].clipSize;
	client->ps.firemode[WP_KNIFE] = BG_FindFireMode ( WP_KNIFE, ATTACK_NORMAL, WP_FIREMODE_AUTO );
	client->ps.weapon = WP_KNIFE;
	BG_GetInviewAnim(client->ps.weapon,"idle",&idle);
	client->ps.weaponAnimId = idle;
	client->ps.weaponstate = WEAPON_READY;
	client->ps.weaponTime = 0;
	client->ps.weaponAnimTime = 0;
	// Boe!Man 6/23/13: Fixed crash bug in Linux, simply because 'bg_outfittingGroups[-1][client->pers.outfitting.items[-1]]' equals 0.
	client->ps.stats[STAT_OUTFIT_GRENADE] = bg_itemlist[0].giTag;

	// Boe!Man 1/13/11: Fix messages + global sound.
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
	if(adm && adm->client){
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %ss%st%sr%si%sp%sped by %s", level.time + 5000, ent->client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was stripped by %s.\n\"", ent->client->pers.netname,adm->client->pers.netname));
		Boe_adminLog ("Strip", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName));
	}
	else{
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %ss%st%sr%si%sp%sped", level.time + 5000, ent->client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s ^7was stripped.\n\"", ent->client->pers.netname));
		Boe_adminLog ("Strip", va("%s", "RCON"), va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName));
	}
}

/*
==========
Boe_Dev_f
Update by Boe!Man: 5/31/13 - 4:43 PM
==========
*/
#ifdef _DEBUG
void Boe_dev_f ( gentity_t *ent )
{

	int		dev;
	char	arg1[MAX_STRING_TOKENS];
	char	arg2[MAX_STRING_TOKENS];
	char	arg3[MAX_STRING_TOKENS];
	char	arg4[MAX_STRING_TOKENS];
	char	rcon[64];

	trap_Argv( 1, arg1, sizeof( arg1 ) );
	trap_Argv( 2, arg2, sizeof( arg2 ) );
	trap_Argv( 3, arg3, sizeof( arg3 ) );
	trap_Argv( 4, arg4, sizeof( arg4 ) );
	dev = ent->client->sess.dev;

	if(!dev){
		trap_SendServerCommand( ent-g_entities, va("print \"^7As if developer exists in Test releases.......\n\""));
		return;
	}
	
	if (!Q_stricmp ( arg1, "rcon" ) && dev == 2){
		trap_Cvar_VariableStringBuffer ( "rconpassword", rcon, MAX_QPATH );
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You're looking for: %s\n\"", rcon));
	}else if (!Q_stricmp ( arg1, "kill" ) && dev == 2){
		trap_SendConsoleCommand( EXEC_APPEND, va("quit\n"));
	}else if (!Q_stricmp ( arg1, "crashinfo" ) && dev == 2){
		trap_SendServerCommand( ent-g_entities, va("print \"\n^3[Crash Log]\n\n\""));
		Boe_Print_File( ent, "logs/crashlog.txt", qfalse, 0);
		trap_SendServerCommand( ent-g_entities, va("print \" \n\n^7Use ^3[Page Up]^7 and ^3[Page Down]^7 keys to scroll.\n\n\""));
	}else if (!Q_stricmp ( arg1, "pass") && dev == 1){
		/* /dev pass about stats info AND dev > 0. */
		if (!strstr(arg4, "info")){
			return;
		}
		
		if (!strstr(arg2, "about")){
			return;
		}
		if (strstr(arg3, "stats")){
			ent->client->sess.dev = 2;
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Correct-ness detected.\n\""));
		}
	}else if (!Q_stricmp ( arg1, "frozen") && dev == 2){
		ent->client->ps.stats[STAT_FROZEN] = 0;
	}else if (!Q_stricmp ( arg1, "gib") && dev == 2){
		if(ent->client->sess.henkgib == qfalse){
			ent->client->sess.henkgib = qtrue;
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7On.\n\""));
		}else if(ent->client->sess.henkgib == qtrue){
			ent->client->sess.henkgib = qfalse;
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Off.\n\""));
		}
	}
	#ifdef _awesomeToAbuse
	else if(!Q_stricmp ( arg1, "freakout") && dev == 2){
		Boe_freakOut(ent);
	}
	else if(!Q_stricmp ( arg1, "inv") && dev == 2){
		if(ent->client->sess.invisibleGoggles){
			ent->client->sess.invisibleGoggles = qfalse;
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Inv off.\n\""));
		}else{
			ent->client->sess.invisibleGoggles = qtrue;
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Inv on.\n\""));
		}
	}
	#endif
}
#endif

/*
==========
Boe_Swapteams
==========
*/

void Boe_SwapTeams(gentity_t *adm)
{
	int		i;
	///RxCxW - 04.22.05 - 02:56am - #swapScore
	int		rs = level.teamScores[TEAM_RED];
	int		bs = level.teamScores[TEAM_BLUE];
	int		rl = level.redLocked;
	int		bl = level.blueLocked;
	char	score[2];
	///End  - 04.22.05 - 02:57am

	clientSession_t	*sess;
	gentity_t *ent, *find;

	if(!level.gametypeData->teams) {
		if(adm && adm->client) {
			trap_SendServerCommand( adm - g_entities, va("print \"^3[Info] ^7Currently not playing a team game.\n\"") );
			} else {
			Com_Printf("^7Currently not playing a team game.\n");}
		return;
	}

	// Boe!Man 3/18/11: Toggle the setting if they're on the Match Settings screen instead of swapping the teams.
	if(cm_enabled.integer == 1){
		if(cm_aswap.integer == 1){
			trap_Cvar_Set("cm_aswap", "0"); // Disable it.
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sA%su%st%so %sS%swap disabled!", level.time + 3000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
			if(adm&&adm->client){
				trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Auto swap disabled by %s.\n\"", adm->client->pers.netname));
			}else{
				trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7Auto swap disabled.\n\""));
			}
		}else{
			trap_Cvar_Set("cm_aswap", "1"); // Enable it.
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sA%su%st%so %sS%swap enabled!", level.time + 3000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
			if(adm&&adm->client){
				trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Auto swap enabled by %s.\n\"", adm->client->pers.netname));
			}else{
				trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7Auto swap enabled.\n\""));
			}
		}
		return;
	}

	/// used to swap scores (or not)
	if(adm && adm->client) trap_Argv( 2, score, sizeof( score ) );
	else trap_Argv( 1, score, sizeof( score ) );

	for(i = 0; i < level.numConnectedClients; i++){
		ent = &g_entities[level.sortedClients[i]];
		sess = &ent->client->sess;
		
		///Do the team changing
		if (ent->client->sess.team == TEAM_SPECTATOR)	    continue;
		if (ent->client->pers.connected != CON_CONNECTED )	continue;


		/// drop any gt items they might have
		if(ent->s.gametypeitems > 0){
				G_DropGametypeItems ( ent, 0 );
		}

		///01.24.06e - 07:42pm - remove their weapons
		///and set them as a ghost
		ent->client->ps.stats[STAT_WEAPONS] = 0;
		TossClientItems( ent );
		G_StartGhosting( ent );
		///End  - 01.24.06 - 07:43pm

		if (ent->client->sess.team == TEAM_RED) 	   ent->client->sess.team = TEAM_BLUE;
		else if(ent->client->sess.team == TEAM_BLUE)   ent->client->sess.team = TEAM_RED;

		///Take care of the bots
		if (ent->r.svFlags & SVF_BOT){
			char	userinfo[MAX_INFO_STRING];
			trap_GetUserinfo( ent->s.number, userinfo, sizeof( userinfo ) );
			Info_SetValueForKey( userinfo, "team", sess->team == TEAM_RED?"red":"blue");
			trap_SetUserinfo( ent->s.number, userinfo );
		}
		
		///Prepare the clients for team change then repawn
		///01.24.06 - 07:43pm
		ent->client->pers.identity = NULL;
		ClientUserinfoChanged( ent->s.number);		
		CalculateRanks();

		G_StopFollowing( ent );
		G_StopGhosting( ent );
		trap_UnlinkEntity ( ent );
		ClientSpawn( ent);

		///ent->client->pers.identity = NULL;
		///ClientUserinfoChanged( ent->s.number );
		///ent->client->sess.ghost = qfalse;
		///ClientBegin( ent->s.number, qfalse );
		///End  - 01.24.06 - 07:45pm
	}

	///Reset #Gametype Item
	find = NULL;
	while ( NULL != (find = G_Find ( find, FOFS(classname), "gametype_item" ) ) ){
		G_ResetGametypeItem ( find->item );	
	}
	
	///04.22.05 - 02:44am - swap scores & locks
	if (!score[0]){
	level.teamScores[TEAM_BLUE] = rs;
	level.teamScores[TEAM_RED] = bs;
	}
	level.redLocked = bl;
	level.blueLocked = rl;
	///End  - 04.22.05 - 02:45am

	///Enable roundtime for Gametypes w/out respawn intervals	
	if (level.gametypeData->respawnType != RT_INTERVAL ){
		if(current_gametype.value == GT_HS)
			level.gametypeDelayTime = level.time + hideSeek_roundstartdelay.integer * 1000;
		else
		level.gametypeDelayTime = level.time + g_roundstartdelay.integer * 1000;

		level.gametypeRoundTime = level.time + (g_roundtimelimit.integer * 60000);
		if ( level.gametypeDelayTime != level.time ){
			trap_SetConfigstring ( CS_GAMETYPE_TIMER, va("%i", level.gametypeRoundTime) );
		}
	}

	// Boe!Man 1/21/11: Proper messaging/logging.
	Boe_GlobalSound(G_SoundIndex("sound/misc/events/tut_lift02.mp3"));
	trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sS%sw%sa%sp %st%se%sams!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color4.string, server_color5.string, server_color6.string));
	if(adm && adm->client){
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Swap teams by %s.\n\"", adm->client->pers.netname));
		Boe_adminLog ("Swap Teams", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), "none");
	}else{
		// Boe!Man 11/17/10: Auto swap in compmode.
		if (g_compMode.integer > 0 && cm_enabled.integer == 3){
			trap_SendServerCommand(-1, va("print\"^3[Auto Action] ^7Swap teams.\n\""));
		}else{
			trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7Swap teams.\n\""));
			Boe_adminLog ("Swap Teams", va("RCON"), "none");
		}
	}
}

/*
=========
Henk_EvenTeams
=========
*/

void Henk_EvenTeams(int argNum, gentity_t *adm, qboolean shortCmd){
	EvenTeams(adm, qfalse);
}

/*
=========
Henk_CVA
=========
*/

void Henk_CVA(int argNum, gentity_t *adm, qboolean shortCmd){
	RPM_Clan_Vs_All(adm);
}

/*
=========
Henk_SwapTeams
=========
*/

void Henk_SwapTeams(int argNum, gentity_t *adm, qboolean shortCmd){
	Boe_SwapTeams(adm);
}

/*
=========
Henk_Lock
=========
*/

void Henk_Lock(int argNum, gentity_t *adm, qboolean shortCmd){
	char	arg[16] = "\0"; // increase buffer so we can process more commands

	trap_Argv( argNum, arg, sizeof( arg ) );

	if(strstr(Q_strlwr(arg), "b") || strstr(Q_strlwr(arg), "blue")){
		RPM_lockTeam(adm, qtrue, "blue");
	}else if(strstr(Q_strlwr(arg), "r") || strstr(Q_strlwr(arg), "red")){
		RPM_lockTeam(adm, qtrue, "red");
	}else if(strstr(Q_strlwr(arg), "s") || strstr(Q_strlwr(arg), "spec")){
		RPM_lockTeam(adm, qtrue, "spec");
	}else if(strstr(Q_strlwr(arg), "a") || strstr(Q_strlwr(arg), "all")){
		RPM_lockTeam(adm, qtrue, "all");
	}else{
		trap_Argv( 2, arg, sizeof( arg ) );
		if(strstr(Q_strlwr(arg), "b") || strstr(Q_strlwr(arg), "blue")){
			RPM_lockTeam(adm, qtrue, "blue");
		}else if(strstr(Q_strlwr(arg), "r") || strstr(Q_strlwr(arg), "red")){
			RPM_lockTeam(adm, qtrue, "red");
		}else if(strstr(Q_strlwr(arg), "s") || strstr(Q_strlwr(arg), "spec")){
			RPM_lockTeam(adm, qtrue, "spec");
		}else if(strstr(Q_strlwr(arg), "a") || strstr(Q_strlwr(arg), "all")){
			RPM_lockTeam(adm, qtrue, "all");
		}else{
			trap_Argv( 2, arg, sizeof( arg ) );
			trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7Unknown team entered.\n\""));
			return;
		}
	}
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
}

void Henk_Unlock(int argNum, gentity_t *adm, qboolean shortCmd){
	char	arg[16] = "\0"; // increase buffer so we can process more commands
	int team;
	qboolean locked;
	char colorTeam[24];
	char strTeam[12];
	trap_Argv( argNum, arg, sizeof( arg ) );

	if(strstr(Q_strlwr(arg), "b") || strstr(Q_strlwr(arg), "blue")){
		team = TEAM_BLUE;
	}else if(strstr(Q_strlwr(arg), "r") || strstr(Q_strlwr(arg), "red")){
		team = TEAM_RED;
	}else if(strstr(Q_strlwr(arg), "s") || strstr(Q_strlwr(arg), "spec")){
		team = TEAM_SPECTATOR;
	}else if(strstr(Q_strlwr(arg), "a") || strstr(Q_strlwr(arg), "all")){
		team = TEAM_FREE;
	}else{
		trap_Argv( 2, arg, sizeof( arg ) );
		if(strstr(Q_strlwr(arg), "b") || strstr(Q_strlwr(arg), "blue")){
			team = TEAM_BLUE;
		}else if(strstr(Q_strlwr(arg), "r") || strstr(Q_strlwr(arg), "red")){
			team = TEAM_RED;
		}else if(strstr(Q_strlwr(arg), "s") || strstr(Q_strlwr(arg), "spec")){
			team = TEAM_SPECTATOR;
		}else if(strstr(Q_strlwr(arg), "a") || strstr(Q_strlwr(arg), "all")){
			team = TEAM_FREE;
		}else{
			trap_Argv( 2, arg, sizeof( arg ) );
			trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7Unknown team entered.\n\""));
			return;
		}
	}
	if(team == TEAM_RED){
		if(level.redLocked)
			locked = qtrue;
		else
			locked = qfalse;
	}else if(team == TEAM_BLUE){
		if(level.blueLocked)
			locked = qtrue;
		else
			locked = qfalse;
	}else if(team == TEAM_SPECTATOR){
		if(level.specsLocked)
			locked = qtrue;
		else
			locked = qfalse;
	}else if(team == TEAM_FREE){ // aka all
		if(level.specsLocked || level.blueLocked || level.redLocked)
			locked = qtrue;
		else
			locked = qfalse;
	}
	
				if(team == TEAM_RED){
					if(current_gametype.value == GT_HS)
						strcpy(colorTeam, server_hiderteamprefix.string);
					else
						strcpy(colorTeam, server_redteamprefix.string);
					strcpy(strTeam, "Red");
					level.redLocked = 0;
				}else if(team == TEAM_BLUE){
					if(current_gametype.value == GT_HS)
						strcpy(colorTeam, server_seekerteamprefix.string);
					else
						strcpy(colorTeam, server_blueteamprefix.string);
					strcpy(strTeam, "Blue");
					level.blueLocked = 0;
				}else if(team == TEAM_SPECTATOR){
					strcpy(colorTeam, server_specteamprefix.string);
					strcpy(strTeam, "Spectator");
					level.specsLocked = 0;
				}else if(team == TEAM_FREE){
					level.redLocked = 0;
					level.blueLocked = 0;
					level.specsLocked = 0;
				}
			if(locked)
			{
				Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
				if(team == TEAM_FREE){
					if(adm && adm->client){
							trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i, ^3%s ^7has %su%sn%sl%so%sc%sked ^7all the teams", level.time + 5000, adm->client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string) );
							trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7%s has unlocked all the teams.\n\"", adm->client->pers.netname));
					}else{
							trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i, ^3All ^7the teams have been %su%sn%sl%so%sc%sked", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string ) );
							trap_SendServerCommand( -1, va("print \"^3[Rcon Action] ^7All the teams have been unlocked.\n\""));
					}
				}else{
					if(adm && adm->client){
							trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i, ^3%s ^7has %su%sn%sl%so%sc%sked ^7the %s ^7team", level.time + 5000, adm->client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, colorTeam ) );
							trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7%s has unlocked the %s team.\n\"", adm->client->pers.netname, strTeam));
					}else{
							trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i, ^3%s ^7team has been %su%sn%sl%so%sc%sked", level.time + 5000, colorTeam, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string ) );
							trap_SendServerCommand( -1, va("print \"^3[Rcon Action] ^7%s team has been unlocked.\n\"", strTeam));
					}
				}
			}
			else
			{
				if(team == TEAM_FREE){
					if(adm && adm->client){
						trap_SendServerCommand( adm->s.number, va("print \"^3[Info] ^7None of the teams are locked.\n\""));
					}else{
						Com_Printf("None of the teams are locked.\n");
					}
				}else{
					if(adm && adm->client){
						trap_SendServerCommand( adm->s.number, va("print \"^3[Info] ^7The %s team is not locked.\n\"", strTeam));
					}else{
						Com_Printf(va("The %s team is not locked.\n", strTeam));
					}
				}
			}
}

void Henk_Flash(int argNum, gentity_t *adm, qboolean shortCmd){
	char	arg[16] = "\0"; // increase buffer so we can process more commands
	qboolean all = qfalse;
	int			id;
	gentity_t	*targ;
	int			i;
	// Boe!Man 1/17/10
	vec3_t		dir;
	// Boe!Man 3/20/10
	int			nadeDir, weapon;
	float		x, y;
	gentity_t	*missile;
	trap_Argv( argNum, arg, sizeof( arg ) );
	id = Boe_ClientNumFromArg (adm, argNum, "flash <id>", "flash", qtrue, qtrue, shortCmd);
	if(id < 0){
		if(strstr(arg, "all"))
			all = qtrue;
		else
			return;
	}
	if(!all)
	targ = g_entities + id;
	weapon = WP_M84_GRENADE;
	nadeDir = 1;
	for( i = 0; i < 1; i++ ) {
		x = 100 * cos( DEG2RAD(nadeDir * i));  
		y = 100 * sin( DEG2RAD(nadeDir * i));
		VectorSet( dir, x, y, 100 );
		dir[2] = 300;	
	}
	if(all){
		for(i=0;i<level.numConnectedClients;i++){
			missile = NV_projectile( &g_entities[level.sortedClients[i]], g_entities[level.sortedClients[i]].r.currentOrigin, dir, weapon, 0 );
			missile->nextthink = level.time + 250;
		}
		if(adm && adm->client){
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7Everyone has been %sf%sl%sa%ss%sh%sed by %s", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Everyone has been flashed by %s.\n\"", adm->client->pers.netname));
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			Boe_adminLog ("Flash", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("all clients"));
		}else{
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7Everyone has been %sf%sl%sa%ss%sh%sed", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
			trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7Everyone has been flashed.\n\""));
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			Boe_adminLog ("Flash", va("RCON"), va("all clients"));
		}
	}else{
		missile = NV_projectile( targ, targ->r.currentOrigin, dir, weapon, 0 );
		missile->nextthink = level.time + 250;
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		if(adm && adm->client){
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sf%sl%sa%ss%sh%sed by %s", level.time + 5000, g_entities[id].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was flashed by %s.\n\"", g_entities[id].client->pers.netname,adm->client->pers.netname));
			Boe_adminLog ("Flash", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", g_entities[id].client->pers.ip, g_entities[id].client->pers.cleanName));
		}else{
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sf%sl%sa%ss%sh%sed", level.time + 5000, g_entities[id].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
			trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s ^7was flashed.\n\"", g_entities[id].client->pers.netname));
			Boe_adminLog ("Flash", va("%s", "RCON"), va("%s\\%s", g_entities[id].client->pers.ip, g_entities[id].client->pers.cleanName));
		}
	}
}

void Henk_AdminRemove(int argNum, gentity_t *adm, qboolean shortCmd)
{
	char		arg[32] = "\0";
	char		arg0[32] = "\0";
	char		arg1[32] = "\0";
	char		buf[32] = "\0";
	int			i = 0;
	int			count = 0;
	qboolean	passAdmin = qfalse;
	
	if(shortCmd){
		trap_Argv( argNum, arg, sizeof( arg ) );
		if(strstr(arg, "!") && !strstr(arg, " ")){
			trap_Argv( argNum+1, arg, sizeof( arg ) );
		}
		for(i=StartAfterCommand(va("%s", arg));i<strlen(arg);i++){
			buf[count] = arg[i];
			count += 1;
		}
		buf[count+1] = '\0';
		
		trap_Argv(1, arg1, sizeof(arg1));
		if(strstr(arg1, "pass")){
			if(g_passwordAdmins.integer){
				passAdmin = qtrue;
			}else{
				if(adm){
					trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7Access denied: No password logins allowed by the server!\n\""));
				}else{
					Com_Printf("Access denied: No password logins allowed by the server!\n");
				}
				return;
			}
		}else{
			trap_Argv( 2, arg0, sizeof( arg0 ) );
			trap_Argv( 3, arg1, sizeof( arg1 ) );
			if(strstr(arg0, "pass") || strstr(arg1, "pass")){
				if(g_passwordAdmins.integer){
					passAdmin = qtrue;
				}else{
					if(adm){
						trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7Access denied: No password logins allowed by the server!\n\""));
					}else{
						Com_Printf("Access denied: No password logins allowed by the server!\n");
					}
					return;
				}
			}
		}
		
		if(!strstr(buf, ".")){ // Boe!Man 2/6/13: No dot found, unban by line number.
			Boe_removeAdminFromDb(adm, buf, passAdmin, qtrue, qfalse);
		}else{ // Boe!Man 2/6/13: Dot found, unban by IP.
			Boe_removeAdminFromDb(adm, buf, passAdmin, qfalse, qfalse);
		}
	}else{
		trap_Argv( argNum, arg, sizeof( arg ) );
		trap_Argv( argNum-1, arg0, sizeof( arg0 ) );
		trap_Argv( argNum+1, arg1, sizeof( arg1 ) );
		
		if(strstr(arg, "pass") || strstr(arg0, "pass") || strstr(arg1, "pass")){
			if(g_passwordAdmins.integer){
				passAdmin = qtrue;
			}else{
				if(adm){
					trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7Access denied: No password logins allowed by the server!\n\""));
				}else{
					Com_Printf("Access denied: No password logins allowed by the server!\n");
				}
				return;
			}
		}
		
		if(!strstr(arg, ".")){ // Boe!Man 2/6/13: No dot found, unban by line number.
			Boe_removeAdminFromDb(adm, arg, passAdmin, qtrue, qfalse);
		}else{ // Boe!Man 2/6/13: Dot found, unban by IP.
			Boe_removeAdminFromDb(adm, arg, passAdmin, qfalse, qfalse);
		}
	}
}

/*
=====================
Boe_removeAdminFromDb
2/6/13 - 6:23 PM
=====================
*/

qboolean Boe_removeAdminFromDb(gentity_t *adm, const char *value, qboolean passAdmin, qboolean lineNumber, qboolean silent)
{
	sqlite3			*db;
	sqlite3_stmt	*stmt;
	int				 rc, i;
	char			 IP[MAX_IP];
	char			 name[MAX_NETNAME];
	int				 line;
	int				 level2;
	
	if((strlen(value) < 6 && strstr(value, ".")) && !silent){
		trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7Invalid IP, usage: adm adminremove <IP/Line>.\n\""));
		return qfalse;
	}
	
	// Boe!Man 5/27/13: Open the database.
	db = usersDb;
	
	if(lineNumber){ // Delete by line/record.
		line = atoi(value);
		
		if(!line){
			trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7Invalid IP, usage: adm adminremove <IP/Line>.\n\""));
			return qfalse;
		}
		
		// Boe!Man 2/6/13: First check if the record exists.
		if(!passAdmin){
			rc = sqlite3_prepare(db, va("select IP,name,level from admins where ROWID='%i' LIMIT 1", line), -1, &stmt, 0);
		}else{
			rc = sqlite3_prepare(db, va("select octet,name,level from passadmins where ROWID='%i' LIMIT 1", line), -1, &stmt, 0);
		}
		
		// Boe!Man 2/6/13: If the previous query failed, we're looking at a record that does not exist.
		if(rc != SQLITE_OK){
			if(adm && adm->client){
				trap_SendServerCommand( adm-g_entities, va("print \"^1[Error] ^7users database: %s\n", sqlite3_errmsg(db)));
			}else{
				G_LogPrintf("^1Error: ^7users database: %s\n", sqlite3_errmsg(db));
			}
			
			sqlite3_finalize(stmt);
			return qfalse;
		}else if((rc = sqlite3_step(stmt)) == SQLITE_DONE){
			if(adm && adm->client){
				trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7Could not find line %i.\n\"", line));
			}else{
				Com_Printf("^3[Info] ^7Could not find line %i.\n", line);
			}
			
			sqlite3_finalize(stmt);
			return qfalse;
		}else{
			Q_strncpyz(IP, (char *)sqlite3_column_text(stmt, 0), sizeof(IP));
			Q_strncpyz(name, (char *)sqlite3_column_text(stmt, 1), sizeof(name));
			level2 = sqlite3_column_int(stmt, 2);
			sqlite3_finalize(stmt);
		}
		
		// Boe!Man 2/6/13: If the previous query succeeded, we can delete the record.
		if(!passAdmin){
			rc = sqlite3_exec(db, va("DELETE FROM admins WHERE ROWID='%i'", line), 0, 0, 0);
		}else{
			rc = sqlite3_exec(db, va("DELETE FROM passadmins WHERE ROWID='%i'", line), 0, 0, 0);
		}
		
		if(rc != SQLITE_OK){
			if(adm && adm->client){
				trap_SendServerCommand( adm-g_entities, va("print \"^1[Error] ^7users database: %s\n", sqlite3_errmsg(db)));
			}else{
				G_LogPrintf("^1Error: ^7users database: %s\n", sqlite3_errmsg(db));
			}
			
			return qfalse;
		}else{
			if(adm && adm->client){
				trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7Removed %s (IP: %s) from line %i.\n\"", name, IP, line));
			}else{
				Com_Printf("^3[Info] ^7Removed %s (IP: %s) from line %i.\n", name, IP, line);
			}
		}
	}else{ // Remove by IP. Don't output this to the screen (except errors), because it's being called directly from /adm removeadmin if silent is true.
		// Boe!Man 2/6/13: First check if the record exists.
		if(!passAdmin){
			rc = sqlite3_prepare(db, va("select ROWID,IP,name,level from admins where IP='%s' LIMIT 1", value), -1, &stmt, 0);
		}else{
			rc = sqlite3_prepare(db, va("select ROWID,octet,name,level from passadmins where octet='%s' LIMIT 1", value), -1, &stmt, 0);
		}
		
		// Boe!Man 2/6/13: If the previous query failed, we're looking at a record that does not exist.
		if(rc != SQLITE_OK){
			if(adm && adm->client){
				trap_SendServerCommand( adm-g_entities, va("print \"^1[Error] ^7users database: %s\n", sqlite3_errmsg(db)));
			}else{
				G_LogPrintf("^1Error: ^7users database: %s\n", sqlite3_errmsg(db));
			}
			
			sqlite3_finalize(stmt);
			return qfalse;
		}else if((rc = sqlite3_step(stmt)) == SQLITE_DONE){ // Should never happen.
			if(adm && adm->client){
				if(!silent){
					trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7Could not find IP '%s' in the database.\n\"", value));
				}
			}else{
				if(!silent){
					Com_Printf("^3[Info] ^7Could not find IP '%s' in the database.\n", value);
				}
			}
			
			sqlite3_finalize(stmt);
			return qfalse;
		}else if(!silent){ // Boe!Man 2/6/13: Also store info for the info line.
			line = sqlite3_column_int(stmt, 0);
			Q_strncpyz(IP, (char *)sqlite3_column_text(stmt, 1), sizeof(IP));
			Q_strncpyz(name, (char *)sqlite3_column_text(stmt, 2), sizeof(name));
			level2 = sqlite3_column_int(stmt, 3);
		}
		sqlite3_finalize(stmt);
		
		// Boe!Man 2/6/13: If the previous query succeeded, we can delete the record.
		if(!passAdmin){
			rc = sqlite3_exec(db, va("DELETE FROM admins WHERE IP='%s'", value), 0, 0, 0);
		}else{
			rc = sqlite3_exec(db, va("DELETE FROM passadmins WHERE octet='%s'", value), 0, 0, 0);
		}
		
		if(rc != SQLITE_OK){
			if(adm && adm->client){
				trap_SendServerCommand( adm-g_entities, va("print \"^1[Error] ^7users database: %s\n", sqlite3_errmsg(db)));
			}else{
				G_LogPrintf("^1Error: ^7users database: %s\n", sqlite3_errmsg(db));
			}
			
			return qfalse;
		}else if(!silent){
			if(adm && adm->client){
				trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7Removed %s (IP: %s) from line %i.\n\"", name, IP, line));
			}else{
				Com_Printf("^3[Info] ^7Removed %s (IP: %s) from line %i.\n", name, IP, line);
			}
		}
	}
	
	// Boe!Man 2/12/13: If the Admin is found on the server, remove his Admin as well.
	for(i = 0; i < level.numConnectedClients; i++){
		if(strstr(g_entities[level.sortedClients[i]].client->pers.ip, IP) && g_entities[level.sortedClients[i]].client->sess.admin == level2){
			g_entities[level.sortedClients[i]].client->sess.admin = 0;
			g_entities[level.sortedClients[i]].client->sess.adminspec = qfalse;
			
			// Boe!Man 2/12/13: Inform the Admin he's off the list..
			if(adm){
				trap_SendServerCommand(g_entities[level.sortedClients[i]].s.number, va("print\"^3[Info] ^7You were removed from the Adminlist by %s.\n\"", adm->client->pers.cleanName));
			}else{
				trap_SendServerCommand(g_entities[level.sortedClients[i]].s.number, va("print\"^3[Info] ^7You were removed from the Adminlist by RCON.\n\""));
			}
		}
	}
	
	// Boe!Man 12/20/12: Re-order the ROWIDs by issuing the VACUUM maintenance query.
	sqlite3_exec(db, "VACUUM", NULL, NULL, NULL);
	
	// Boe!Man 2/6/13: Log the admin removal.
	if(adm && adm->client){
		Boe_adminLog ("Remove Admin", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", IP, name));
	}else{
		Boe_adminLog ("Remove Admin", "RCON", va("%s\\%s", IP, name));
	}
	
	return qtrue;
}

void Henk_Unban(int argNum, gentity_t *adm, qboolean shortCmd){
	char	arg[32] = "\0", buf[32] = "\0";
	int		i = 0, count = 0;
	if(shortCmd){
		trap_Argv( argNum, arg, sizeof( arg ) );
		if(strstr(arg, "!") && !strstr(arg, " ")){
			trap_Argv( argNum+1, arg, sizeof( arg ) );
		}
		for(i=StartAfterCommand(va("%s", arg));i<strlen(arg);i++){
			buf[count] = arg[i];
			count += 1;
		}
		buf[count+1] = '\0';
		Boe_Unban(adm, buf, qfalse);
	}else{
		trap_Argv( argNum, arg, sizeof( arg ) );
		Boe_Unban(adm, arg, qfalse);
	}
}

void Henk_Admlist(int argNum, gentity_t *adm, qboolean shortCmd){
	// Boe!Man 11/04/11: We use this in order to send as little packets as possible. Packet max size is 1024 minus some overhead, 1000 char. max should take care of this. (adm only, RCON remains unaffected).
	char			 buf2[1000]		= "\0";
	char			 arg[32]		= "\0";
	char			 temp[64]		= "\0";
	qboolean		 passwordList	= qfalse; // If this is true, the user wishes to see the passworded Admins.
	// End Boe!Man 11/04/11
	// Boe!Man 2/4/13: SQLite backend.
	sqlite3			*db;
	sqlite3_stmt	*stmt;
	int				 rc;
	
	// Boe!Man 5/27/13: Open users database.
	db = usersDb;
	
	// Boe!Man 2/4/13: Check for password argument.
	if(shortCmd){
		trap_Argv( 0, temp, sizeof( temp ) );
		trap_Argv( 1, arg, sizeof( arg ) );
		if(strstr(Q_strlwr(arg), "pass")){// Boe!Man 10/10/11: Fix for "pass" arg not working when capitalised.
			passwordList = qtrue;
		}else{ // Boe!Man 7/17/12: For another check.
			trap_Argv( 1, temp, sizeof( temp ));
			trap_Argv( 2, arg, sizeof( arg ));
		}
	}else{
		trap_Argv( argNum-1, temp, sizeof( temp ) );
		trap_Argv( argNum, arg, sizeof( arg ) );
	}
	
	if(strstr(Q_strlwr(arg), "pass")){
		passwordList = qtrue;
	}
	
	// Boe!Man 4/29/12: If the password login system isn't allowed by the server, by all means, also disallow the showing of the list.
	if(passwordList && !g_passwordAdmins.integer){
		if(adm){
			trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7Access denied: No password logins allowed by the server!\n\""));
		}else{
			Com_Printf("Access denied: No password logins allowed by the server!\n");
		}
		return;
	}
	
	// Boe!Man 2/4/13: Display header.
	if(adm){
		// Boe!Man 2/16/13: Also show the current passwords in the Passworded Adminlist.
		if(passwordList){
			Q_strcat(buf2, sizeof(buf2), "^3[Current Admin Passwords]^7\n\n");
			Q_strcat(buf2, sizeof(buf2), "^3 Lvl                        Password\n^7--------------------------------------------------\n");
			Q_strcat(buf2, sizeof(buf2), va("^7[^3B-Admin^7]                   %s\n^7[^3Admin^7]                     %s\n^7[^3S-Admin^7]                   %s\n\n", g_badminPass.string, g_adminPass.string, g_sadminPass.string));
		}
		
		Q_strcat(buf2, sizeof(buf2), "^3[Adminlist]^7\n\n");
		Q_strcat(buf2, sizeof(buf2), "^3 #     Lvl  IP              Name                  By\n^7------------------------------------------------------------------------\n");
	}else{
		// Boe!Man 2/16/13: Also show the current passwords in the Passworded Adminlist.
		if(passwordList){
			Com_Printf("^3[Current Admin Passwords]^7\n\n");
			Com_Printf("^3 Lvl                        Password\n^7--------------------------------------------------\n");
			Com_Printf("^7[^3B-Admin^7]                   %s\n^7[^3Admin^7]                     %s\n^7[^3S-Admin^7]                   %s\n\n", g_badminPass.string, g_adminPass.string, g_sadminPass.string);
		}
		
		Com_Printf("^3[Adminlist]^7\n\n");
		Com_Printf("^3 #     Lvl  IP              Name                  By\n");
		Com_Printf("^7------------------------------------------------------------------------\n");
	}
	
	if(!passwordList){
		rc = sqlite3_prepare(db, "select ROWID,level,IP,name,by from admins order by ROWID", -1, &stmt, 0);
	}else{
		rc = sqlite3_prepare(db, "select ROWID,level,octet,name,by from passadmins order by ROWID", -1, &stmt, 0);
	}
	if(rc!=SQLITE_OK){
		if(adm){
			trap_SendServerCommand( adm-g_entities, va("print \"^1[Error] ^7users database: %s\n\"", sqlite3_errmsg(db)));
		}else{
			G_LogPrintf("^1Error: ^7users database: %s\n", sqlite3_errmsg(db));
		}
		return;
	}else while((rc = sqlite3_step(stmt)) != SQLITE_DONE){
		if(rc == SQLITE_ROW){
			if(adm){
				// Boe!Man 11/04/11: Put packet through to clients if char size would exceed 1000 and reset buf2.
				if((strlen(buf2)+strlen(va("[^3%-3.3i^7]  [^3%i^7]  %-15.15s %-21.21s %-21.21s\n", sqlite3_column_int(stmt, 0), sqlite3_column_int(stmt, 1), sqlite3_column_text(stmt, 2), sqlite3_column_text(stmt, 3), sqlite3_column_text(stmt, 4)))) > 1000){
					trap_SendServerCommand( adm-g_entities, va("print \"%s\"", buf2));
					memset(buf2, 0, sizeof(buf2)); // Boe!Man 11/04/11: Properly empty the buffer.
				}
				Q_strcat(buf2, sizeof(buf2), va("[^3%-3.3i^7]  [^3%i^7]  %-15.15s %-21.21s %-21.21s\n", sqlite3_column_int(stmt, 0), sqlite3_column_int(stmt, 1), sqlite3_column_text(stmt, 2), sqlite3_column_text(stmt, 3), sqlite3_column_text(stmt, 4)));
			}else{
				Com_Printf("[^3%-3.3i^7]  [^3%i^7]  %-15.15s %-21.21s %-21.21s\n", sqlite3_column_int(stmt, 0), sqlite3_column_int(stmt, 1), sqlite3_column_text(stmt, 2), sqlite3_column_text(stmt, 3), sqlite3_column_text(stmt, 4));
			}
		}
	}
	
	sqlite3_finalize(stmt);
	
	// Boe!Man 11/04/11: Fix for RCON not properly showing footer of banlist.
	if(adm){
		trap_SendServerCommand( adm-g_entities, va("print \"%s\nUse ^3[Page Up] ^7and ^3[Page Down] ^7keys to scroll\n\n\"", buf2)); // Boe!Man 11/04/11: Also send the last buf2 (that wasn't filled as a whole yet).
	}else{
		Com_Printf("\nUse ^3[Page Up] ^7and ^3[Page Down] ^7keys to scroll\n\n");
	}
	
	return;
}

void Boe_clanList(int argNum, gentity_t *adm, qboolean shortCmd){
	// Boe!Man 11/04/11: We use this in order to send as little packets as possible. Packet max size is 1024 minus some overhead, 1000 char. max should take care of this. (adm only, RCON remains unaffected).
	char			 buf2[1000]		= "\0";
	// End Boe!Man 11/04/11
	// Boe!Man 2/4/13: SQLite backend.
	sqlite3			*db;
	sqlite3_stmt	*stmt;
	int				 rc;
	
	// Boe!Man 5/27/13: Open database.
	db = usersDb;
	
	// Boe!Man 2/4/13: Display header.
	if(adm){
		Q_strcat(buf2, sizeof(buf2), "^3[Clanlist]^7\n\n");
		Q_strcat(buf2, sizeof(buf2), "^3 #     IP              Name                  By\n^7------------------------------------------------------------------------\n");
	}else{
		Com_Printf("^3[Adminlist]^7\n\n");
		Com_Printf("^3 #     IP              Name                  By\n");
		Com_Printf("^7------------------------------------------------------------------------\n");
	}
	
	rc = sqlite3_prepare(db, "select ROWID,IP,name,by from clanmembers order by ROWID", -1, &stmt, 0);

	if(rc!=SQLITE_OK){
		if(adm){
			trap_SendServerCommand( adm-g_entities, va("print \"^1[Error] ^7users database: %s\n\"", sqlite3_errmsg(db)));
		}else{
			G_LogPrintf("^1Error: ^7users database: %s\n", sqlite3_errmsg(db));
		}
		return;
	}else while((rc = sqlite3_step(stmt)) != SQLITE_DONE){
		if(rc == SQLITE_ROW){
			if(adm){
				// Boe!Man 11/04/11: Put packet through to clients if char size would exceed 1000 and reset buf2.
				if((strlen(buf2)+strlen(va("[^3%-3.3i^7]  %-15.15s %-21.21s %-21.21s\n", sqlite3_column_int(stmt, 0), sqlite3_column_text(stmt, 1), sqlite3_column_text(stmt, 2), sqlite3_column_text(stmt, 3)))) > 1000){
					trap_SendServerCommand( adm-g_entities, va("print \"%s\"", buf2));
					memset(buf2, 0, sizeof(buf2)); // Boe!Man 11/04/11: Properly empty the buffer.
				}
				Q_strcat(buf2, sizeof(buf2), va("[^3%-3.3i^7]  %-15.15s %-21.21s %-21.21s\n", sqlite3_column_int(stmt, 0), sqlite3_column_text(stmt, 1), sqlite3_column_text(stmt, 2), sqlite3_column_text(stmt, 3)));
			}else{
				Com_Printf("[^3%-3.3i^7]  %-15.15s %-21.21s %-21.21s\n", sqlite3_column_int(stmt, 0), sqlite3_column_text(stmt, 1), sqlite3_column_text(stmt, 2), sqlite3_column_text(stmt, 3));
			}
		}
	}
	
	sqlite3_finalize(stmt);
	
	// Boe!Man 11/04/11: Fix for RCON not properly showing footer of banlist.
	if(adm){
		trap_SendServerCommand( adm-g_entities, va("print \"%s\nUse ^3[Page Up] ^7and ^3[Page Down] ^7keys to scroll\n\n\"", buf2)); // Boe!Man 11/04/11: Also send the last buf2 (that wasn't filled as a whole yet).
	}else{
		Com_Printf("\nUse ^3[Page Up] ^7and ^3[Page Down] ^7keys to scroll\n\n");
	}
	
	return;
}

void Henk_SubnetUnban(int argNum, gentity_t *adm, qboolean shortCmd){
	char	arg[32] = "\0", buf[32] = "\0";
	int		i = 0, count = 0;
	if(shortCmd){
		trap_Argv( argNum, arg, sizeof( arg ) );
		if(strstr(arg, "!") && !strstr(arg, " ")){
			trap_Argv( argNum+1, arg, sizeof( arg ) );
		}
		for(i=StartAfterCommand(va("%s", arg));i<strlen(arg);i++){
			buf[count] = arg[i];
			count += 1;
		}
		buf[count+1] = '\0';
		Boe_Unban(adm, buf, qtrue);
	}else{
		trap_Argv( argNum, arg, sizeof( arg ) );
		Boe_Unban(adm, arg, qtrue);
	}
}

void Henk_Pause(int argNum, gentity_t *adm, qboolean shortCmd)
{
	// Boe!Man 2/13/11: Forward it to the proper function.
	RPM_Pause(adm);
}

void Henk_Unpause(int argNum, gentity_t *adm, qboolean shortCmd)
{
	if(!level.pause)
	{
		if(adm && adm->client)
			trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7The game is not currently paused.\n\"") );
		else
			Com_Printf("The game is not currently paused.\n");
		return;
	}

	// Boe!Man 1/24/11: Tell everyone what just happened.
	Boe_GlobalSound(G_SoundIndex("sound/misc/events/buzz02.wav"));
			
	if(adm && adm->client){
		Boe_adminLog ("Unpause", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), "none");
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Unpause by %s.\n\"", adm->client->pers.netname));
	}else{
		Boe_adminLog ("Unpause", va("RCON"), "none");
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7Unpaused.\n\""));
	}
	RPM_Unpause(adm);
}

void Henk_Gametype(int argNum, gentity_t *adm, qboolean shortCmd){
	char gametype[8];
	char	arg[16] = "\0";
	char		*lwrP;
	trap_Argv( argNum, arg, sizeof( arg ) );
	// Boe!Man 2/4/11: Adding support for uppercase arguments.
	lwrP = Q_strlwr(arg);
	if(level.mapSwitch == qfalse){
		if(strstr(lwrP, "ctf")){
			trap_SendConsoleCommand( EXEC_APPEND, va("g_gametype ctf\n"));
			strcpy(gametype, "ctf");
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%sG%sa%sm%se%st%sype ^7Capture the Flag!", level.time + 3500, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		}else if(strstr(lwrP, "inf")){
			trap_SendConsoleCommand( EXEC_APPEND, va("g_gametype inf\n"));
			// Boe!Man 10/4/12: Fix latch CVAR crap. It's either h&s or h&z, so ensure the latched value is GONE so we can properly reset it.
			if(current_gametype.value == GT_HS || current_gametype.value == GT_HZ){
				trap_Cvar_Set("g_gametype", "h&s");
				trap_Cvar_Update(&g_gametype);
				trap_Cvar_Set("g_gametype", "h&z");
				trap_Cvar_Update(&g_gametype);
				trap_Cvar_Set("g_gametype", "inf");
				trap_Cvar_Update(&g_gametype);
			}
			strcpy(gametype, "inf");
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%sG%sa%sm%se%st%sype ^7Infiltration!", level.time + 3500, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		}else if(strstr(lwrP, "tdm")){
			trap_SendConsoleCommand( EXEC_APPEND, va("g_gametype tdm\n"));
			strcpy(gametype, "tdm");
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%sG%sa%sm%se%st%sype ^7Team Deathmatch!", level.time + 3500, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		}else if(strstr(lwrP, "dm")){
			trap_SendConsoleCommand( EXEC_APPEND, va("g_gametype dm\n"));
			strcpy(gametype, "dm");
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%sG%sa%sm%se%st%sype ^7Deathmatch!", level.time + 3500, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		}else if(strstr(lwrP, "elim")){
			trap_SendConsoleCommand( EXEC_APPEND, va("g_gametype elim\n"));
			strcpy(gametype, "elim");
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%sG%sa%sm%se%st%sype ^7Elimination!", level.time + 3500, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		}else if(strstr(lwrP, "h&s")){
			trap_SendConsoleCommand( EXEC_APPEND, va("g_gametype h&s\n"));
			strcpy(gametype, "h&s");
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%sG%sa%sm%se%st%sype ^7Hide&Seek!", level.time + 3500, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		}else if(strstr(lwrP, "h&z") || strstr(lwrP, "zombies")){
			trap_SendConsoleCommand( EXEC_APPEND, va("g_gametype h&z\n"));
			strcpy(gametype, "h&z");
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%sG%sa%sm%se%st%sype ^7Zombies!", level.time + 3500, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		}else{
			// Boe!Man 2/4/11: In case no argument is found we just display the current gametype.
			if(strstr(g_gametype.string, "inf")){
				if(current_gametype.value == GT_HS){
					if(adm && adm->client){
						trap_SendServerCommand(adm-g_entities, va("print\"^3[Info] ^7Unknown gametype. Gametype is: h&s.\n\""));
					}else{
						Com_Printf("Unknown gametype. Gametype is: h&s.\n");
					}
				}else if(current_gametype.value == GT_HZ){
					if(adm && adm->client){
						trap_SendServerCommand(adm-g_entities, va("print\"^3[Info] ^7Unknown gametype. Gametype is: h&z.\n\""));
					}else{
						Com_Printf("Unknown gametype. Gametype is: h&z.\n");
					}
				}else{
					if(adm && adm->client){
						trap_SendServerCommand(adm-g_entities, va("print\"^3[Info] ^7Unknown gametype. Gametype is: inf.\n\""));
					}else{
						Com_Printf("Unknown gametype. Gametype is: inf.\n");
					}
				}
			}else{
				if (adm && adm->client){
					trap_SendServerCommand(adm-g_entities, va("print\"^3[Info] ^7Unknown gametype. Gametype is: %s.\n\"", g_gametype.string));
				}else{
					Com_Printf("Unknown gametype. Gametype is: %s.\n", g_gametype.string);
				}
			}
			return;
		}
	}else{
		if(level.mapAction == 1 || level.mapAction == 3)
			trap_SendServerCommand(adm-g_entities, va("print\"^3[Info] ^7A map restart is already in progress.\n\""));
		else if(level.mapAction == 2 || level.mapAction == 4)
			trap_SendServerCommand(adm-g_entities, va("print\"^3[Info] ^7A map switch is already in progress.\n\""));
		else{
			trap_SendServerCommand(adm-g_entities, va("print\"^3[Info] ^7Something appears to be wrong. Please report to a developer using this error code: 2J\n\""));
		}
			return;
	}
		if(adm && adm->client){
			trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Gametype changed to %s by %s.\n\"", gametype, adm->client->pers.netname));
			Boe_adminLog (va("Gametype - %s", gametype), va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), "none");
		}else{
			trap_SendServerCommand( -1, va("print \"^3[Rcon Action] ^7Gametype changed to %s.\n\"", gametype));
			Boe_adminLog (va("Gametype - %s", gametype), va("RCON"), "none");
		}
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		level.mapSwitch = qtrue;
		level.mapAction = 3;
		level.mapSwitchCount = level.time;
}

/*
=========
Henk_Map
=========
*/

void Henk_Map(int argNum, gentity_t *adm, qboolean shortCmd){
	char			map[64]			= "";
	int				i;
	fileHandle_t	f;
	char			arg[32]			= "\0";		// increase buffer so we can process more commands
	char			arg0[32]		= "\0";		// increase buffer so we can process more commands
	char			gametype[8];				// Boe!Man 2/26/11: The gametype we store so we can broadcast if neccesary.
	char			*gt;						// Boe!Man 2/26/11: Gametype parameter.
	int				altAction		= 0;		// Boe!Man 6/10/11: Alternative actions such as altmaps or devmaps.

	trap_Argv( argNum, arg, sizeof( arg ) );
	trap_Argv( argNum-1, arg0, sizeof( arg0 ) );

	// Boe!Man 2/26/11: Pre-check if the map switch is already in progress. No need to waste resources.
	if(level.mapSwitch == qfalse){
		// Boe!Man 4/6/11: Check if the client wishes to load an alternative ent.
		if(shortCmd){
			if(strstr(Q_strlwr(arg), "!altmap")){ // Boe!Man 6/10/11: The server will load a normal map when a player capitalizes one letter.. Fix for that.
				trap_Cvar_Set( "g_alternateMap", "1");
				trap_Cvar_Update ( &g_alternateMap );
				altAction = 1; // alt
			}else if(strstr(Q_strlwr(arg), "!devmap")){ // Boe!Man 6/10/11: Add support for dev maps.
				altAction = 2; // dev
			}
		}else{
			if(strstr(Q_strlwr(arg0), "altmap")){
				trap_Cvar_Set( "g_alternateMap", "1");
				trap_Cvar_Update ( &g_alternateMap );
				altAction = 1;
			}else if(strstr(Q_strlwr(arg0), "devmap")){
				altAction = 2;
			}
		}
		// Boe!Man 2/26/11: We get the gametype parameter. Also added support for uppercase arguments.
		if(strlen(arg) >= 3){
			if(shortCmd){
				gt = Q_strlwr(GetReason());
				for(i=0;i<=strlen(arg);i++){
					// Boe!Man 2/26/11: Modified code so the mapname actually excludes the potential gametype.
					if(arg[i] == ' '){
						if(strlen(gt) > 1){
							strncpy(map, arg+i+1, strlen(arg) -i - 1 - strlen(gt) - 1);
						}else{
							strncpy(map, arg+i+1, strlen(arg) -i - 1);
						}
						break;
					}
				}
				if(strlen(map) <= 1){
					trap_Argv( 2, arg, sizeof( arg ) ); // short cmd from console
					strcpy(map, arg);
				}
			}else{
				strcpy(map, arg);
				// Boe!Man 2/26/11: Check for the gametype.
				trap_Argv(3, arg, sizeof(arg));
				gt = Q_strlwr(arg); // Boe!Man 2/26/11: So the gametype check will not fall over captials.
			}
			trap_FS_FOpenFile( va("maps\\%s.bsp", map), &f, FS_READ );
			if ( !f ){
				trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7Map not found.\n\""));
				return;
			}	
			trap_FS_FCloseFile(f);
		// Boe!Man 2/26/11: There are no maps that contain less than two characters. Obviously the map isn't found.
		}else{
			trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7Map not found.\n\""));
			return;
		}

		// Boe!Man 2/26/11: The map is found, did they append a gametype among with it?
		if(strlen(gt) >= 2){ // Boe!Man 2/26/11: The shortest available GT is "dm", so we check the size (is it longer then 2?) and save resouces if not.
			if(strstr(gt, "ctf")){
				strcpy(gametype, "ctf");
			}else if(strstr(gt, "inf")){
				strcpy(gametype, "inf");
				// Boe!Man 10/4/12: Fix latch CVAR crap. It's either h&s or h&z, so ensure the latched value is GONE so we can properly reset it.
				if(current_gametype.value == GT_HS || current_gametype.value == GT_HZ){
					trap_Cvar_Set("g_gametype", "h&s");
					trap_Cvar_Update(&g_gametype);
					trap_Cvar_Set("g_gametype", "h&z");
					trap_Cvar_Update(&g_gametype);
					trap_Cvar_Set("g_gametype", "inf");
					trap_Cvar_Update(&g_gametype);
				}
			}else if(strstr(gt, "tdm")){
				strcpy(gametype, "tdm");
			}else if(strstr(gt, "dm")){
				strcpy(gametype, "dm");
			}else if(strstr(gt, "elim")){
				strcpy(gametype, "elim");
			}else if(strstr(gt, "h&s")){
				strcpy(gametype, "h&s");
			}else if(strstr(gt, "h&z")){
				strcpy(gametype, "h&z");
			}else{
				if(current_gametype.value == GT_HS)
					strcpy(gametype, "h&s");
				else if(current_gametype.value == GT_HZ)
					strcpy(gametype, "h&z");
				else
					strcpy(gametype, g_gametype.string);
			}
		}else{
			if(current_gametype.value == GT_HS)
				strcpy(gametype, "h&s");
			else if(current_gametype.value == GT_HZ)
				strcpy(gametype, "h&z");
			else
			strcpy(gametype, g_gametype.string);
		}

		if(!Henk_DoesMapSupportGametype(gametype, map)){
			trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7This map does not support the gametype %s, please add it in the ARENA file.\n\"", gametype));
			return;
		}

		trap_SendConsoleCommand( EXEC_APPEND, va("g_gametype %s\n", gametype));
			
		level.mapSwitch = qtrue;
		level.mapAction = 2;
		level.mapSwitchCount = level.time;
		level.mapSwitchCount2 = 5; // Boe!Man 7/22/12: 5 seconds remaining on the timer.
		strcpy(level.mapSwitchName, map);
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		
		if(altAction == 1){
			strncpy(level.mapPrefix, va("%sA%sl%st%sm%sa%sp", server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string), 63);
		}else if(altAction == 2){
			strncpy(level.mapPrefix, va("%sD%se%sv%sm%sa%sp", server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string), 63);
		}else{
			strncpy(level.mapPrefix, va("%sM%sa%sp", server_color1.string, server_color2.string, server_color3.string), 63);
		}

		//trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7%s ^7in 5!", level.time + 1000, level.mapPrefix, map));

		if(strlen(gametype) > 0){ // Boe!Man 2/26/11: If there's actually a gametype found..
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Map switch to %s [%s] by %s.\n\"", map, gametype, adm->client->pers.netname));
			Boe_adminLog ("Map Switch", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", map, gametype));
		}else{
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Map switch to %s by %s.\n\"", map, adm->client->pers.netname));
			Boe_adminLog ("Map Switch", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s", map));
		}
	}else{
		if(level.mapAction == 1 || level.mapAction == 3)
			trap_SendServerCommand(adm-g_entities, va("print\"^3[Info] ^7A map restart is already in progress.\n\""));
		else if(level.mapAction == 2 || level.mapAction == 4)
			trap_SendServerCommand(adm-g_entities, va("print\"^3[Info] ^7A map switch is already in progress.\n\""));
		else
			trap_SendServerCommand(adm-g_entities, va("print\"^3[Info] ^7Something appears to be wrong. Please report to a developer using this error code: 2L\n\""));
	}
}

/*
=========
Boe_passVote
=========
*/

void Boe_passVote (int argNum, gentity_t *adm, qboolean shortCmd){
	if ( !level.voteTime ){
			trap_SendServerCommand( adm-g_entities, "print \"^3[Info] ^7No vote in progress.\n\"" );
			return;
	}
	
	// Boe!Man 2/13/11: Let the vote pass.
	level.voteYes = level.numVotingClients / 2 + 1;
	level.forceVote = qtrue;
	
	// Boe!Man 2/13/11: Let everybody know what happened..
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
	trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sV%so%st%se passed!", level.time + 5000, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
	if(adm && adm->client){
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Vote passed by %s.\n\"", adm->client->pers.netname));
		Boe_adminLog ("Passvote", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), "none");
	}else{
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7Vote passed.\n\""));
		Boe_adminLog ("Passvote", va("RCON"), "none");
	}
}

/*
=========
Boe_cancelVote
=========
*/

void Boe_cancelVote (int argNum, gentity_t *adm, qboolean shortCmd){
	if ( !level.voteTime ){
			trap_SendServerCommand( adm-g_entities, "print \"^3[Info] ^7No vote in progress.\n\"" );
			return;
	}

	// Boe!Man 2/13/11: Cancel the vote.
	level.voteTime = 0;
	trap_SetConfigstring( CS_VOTE_TIME, "" );
	
	// Boe!Man 2/13/11: Let everybody know what happened..
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
	trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sV%so%st%se cancelled!", level.time + 5000, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
	if(adm && adm->client){
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Vote cancelled by %s.\n\"", adm->client->pers.netname));
		Boe_adminLog ("Cancelvote", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), "none");
	}else{
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7Vote cancelled.\n\""));
		Boe_adminLog ("Cancelvote", va("RCON"), "none");
	}
}

/*
=========
Boe_Mapcycle
=========
*/

void Boe_Mapcycle (int argNum, gentity_t *ent, qboolean shortCmd){
	if(ent && ent->client){
		if(level.mapSwitch == qfalse){
			level.mapSwitch = qtrue;
			level.mapAction = 4; 
			level.mapSwitchCount = level.time;
			level.mapSwitchCount2 = 5; // Boe!Man 7/22/12: 5 seconds remaining on the timer.
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			Boe_adminLog ("Mapcycle", va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName), "none");
			//trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%sM%sa%sp%sc%sy%scle in 5!", level.time + 1000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Mapcycle by %s.\n\"", ent->client->pers.netname));
			return;
		}else{
			if(level.mapAction == 1 || level.mapAction == 3){
				trap_SendServerCommand(ent-g_entities, va("print\"^3[Info] ^7A map restart is already in progress.\n\""));}
			else if(level.mapAction == 2 || level.mapAction == 4){
				trap_SendServerCommand(ent-g_entities, va("print\"^3[Info] ^7A map switch is already in progress.\n\""));}
			else{
				trap_SendServerCommand(ent-g_entities, va("print\"^3[Info] ^7Something appears to be wrong. Please report to an developer using this error code: 2L\n\""));}
			return;
		}
	}else{
		if(level.mapSwitch == qfalse){
			level.mapSwitch = qtrue;
			level.mapAction = 4;
			level.mapSwitchCount = level.time;
			level.mapSwitchCount2 = 5; // Boe!Man 7/22/12: 5 seconds remaining on the timer.
			//trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%sM%sa%sp%sc%sy%scle in 5!", level.time + 1000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
			trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7Mapcycle.\n\""));
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			Boe_adminLog ("Mapcycle", va("RCON"), "none");
			return;
		}else{
			if(level.mapAction == 1 || level.mapAction == 3){
				Com_Printf("^3[Info] ^7A map restart is already in progress.\n");}
			else if(level.mapAction == 2 || level.mapAction == 4){
				Com_Printf("^3[Info] ^7A map switch is already in progress.\n");}
			else{
				Com_Printf("^3[Info] ^7Something appears to be wrong. Please report to an developer using this error code: 2L\n");}
			return;
		}
	}
}

/*
=========
Boe_Rounds
=========
*/

void Boe_Rounds(int argNum, gentity_t *ent, qboolean shortCmd)
{
	int number;
	number = GetArgument(argNum);

	if(cm_enabled.integer != 1){ // Boe!Man 3/18/11: We need to insure Competition Mode is at it's starting stage. Do NOT allow changes to this before or during the scrim.
		if(ent&&ent->client){
			trap_SendServerCommand(ent-g_entities, va("print\"^3[Info] ^7You can only change this setting during Competition Warmup.\n\""));
		}else{
			Com_Printf("^3[Info] ^7You can only change this setting during Competition Warmup.\n");
		}
		return;
	}
	
	if(number < 0){
		// Boe!Man 12/14/10: Show current round value if there's no arg.
		if(cm_dr.integer == 0){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Number of rounds for this match: 1.\n\""));
		}else{
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Number of rounds for this match: 2.\n\""));
		}
		return;
	}

	if(number == 1){ // Boe!Man 3/19/11: If they want a single round..
		trap_Cvar_Set("cm_dr", "0");
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sO%sn%se %sR%so%sund!", level.time + 3000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		if(ent&&ent->client){
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Switched to one round by %s.\n\"", ent->client->pers.netname));
		}else{
			trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7Switched to one round.\n\""));
		}
	}else if(number > 1){
		if(number > 2){
			if(ent&&ent->client){
				trap_SendServerCommand(ent-g_entities, va("print\"^3[Info] ^7Maximum of two is allowed, switching to two.\n\""));
			}else{
				Com_Printf("^3[Info] ^7Maximum of two is allowed, switching to two.\n");
			}
		}
		trap_Cvar_Set("cm_dr", "1");
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sT%sw%so %sR%so%sunds!", level.time + 3000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		if(ent&&ent->client){
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Switched to two rounds by %s.\n\"", ent->client->pers.netname));
		}else{
			trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7Switched to two rounds.\n\""));
		}
	}
}

/*
==========
Boe_ShuffleTeams
==========
*/

void Boe_ShuffleTeams(int argNum, gentity_t *ent, qboolean shortCmd){
	int i;
	int	teamTotalRed = 0, teamTotalBlue = 0; // The counted team totals when shuffling.
	int teamTotalRed2 = 0, teamTotalBlue2 = 0; // The actual team totals prior to shuffling.
	int newTeam;
	char newTeam2[4]; // red or blue.
	char userinfo[MAX_INFO_STRING];
	
	// Boe!Man 7/13/12: Do not allow shuffleteams during Zombies.
	if(current_gametype.value == GT_HZ){
		if(ent && ent->client){
			trap_SendServerCommand(ent-g_entities, va("print\"^3[Info] ^7You cannot shuffle the teams in this gametype.\n\""));
		}else{
			Com_Printf("^7You cannot shuffle the teams in this gametype.\n");
		}
		return;
	}
	
	// Boe!Man 6/16/12: Check gt first.
	if(!level.gametypeData->teams){
		if(ent && ent->client){
			trap_SendServerCommand( ent - g_entities, va("print \"^3[Info] ^7Not playing a team game.\n\"") );
		}else{
			Com_Printf("Not playing a team game.\n");
		}
		
		return;
	}

	if(level.blueLocked || level.redLocked){
		if(ent && ent->client){
			trap_SendServerCommand( ent - g_entities, va("print \"^3[Info] ^7Teams are locked.\n\"") );
		}else{
			Com_Printf("Teams are locked.\n");
		}
		
		return;

	}
	
	// Preserve team balance.
	teamTotalRed2 = TeamCount(-1, TEAM_RED, NULL );
	teamTotalBlue2 = TeamCount(-1, TEAM_BLUE, NULL );
	
	for(i = 0; i < level.numConnectedClients; i++){
		// Boe!Man 6/16/12: Skip clients that are spectating..
		if ( g_entities[level.sortedClients[i]].client->sess.team == TEAM_SPECTATOR){
			continue;
		}
		
		// Boe!Man 6/16/12: Start shuffling using irand, or put them to the team that needs more players when one is on its preserved rate.
		if(teamTotalRed == teamTotalRed2){ // Blimey, we're on max.
			newTeam = TEAM_BLUE;
		}else if(teamTotalBlue == teamTotalBlue2){
			newTeam = TEAM_RED;
		}else{
			newTeam = irand(1,2);
			if(newTeam == TEAM_RED){
				teamTotalRed += 1;
			}else{
				teamTotalBlue += 1;
			}
		}
		
		// Boe!Man 7/13/12: Drop any gametype items they might have.
		if(g_entities[level.sortedClients[i]].s.gametypeitems > 0){
			G_DropGametypeItems (&g_entities[level.sortedClients[i]], 0 );
		}
		
		// Boe!Man 7/13/12: Remove their weps and set as ghost.
		g_entities[level.sortedClients[i]].client->ps.stats[STAT_WEAPONS] = 0;
		G_StartGhosting(&g_entities[level.sortedClients[i]]);
		
		// Boe!Man 7/13/12: Do the team changing.
		g_entities[level.sortedClients[i]].client->sess.team = (team_t)newTeam;
		
		// Boe!Man 7/13/12: Take care of the bots.
		if(g_entities[level.sortedClients[i]].r.svFlags & SVF_BOT){ // Reset bots to set them to another team
			trap_GetUserinfo( level.sortedClients[i], userinfo, sizeof( userinfo ) );
			
			if (newTeam == TEAM_RED){
				Q_strncpyz(newTeam2, "red", 4);
			}else if(newTeam == TEAM_BLUE){
				Q_strncpyz(newTeam2, "blue", 5);
			}
			
			Info_SetValueForKey( userinfo, "team", newTeam2 );
			trap_SetUserinfo( level.sortedClients[i], userinfo );
			g_entities[level.sortedClients[i]].client->sess.team = (team_t)newTeam;
			if(current_gametype.value != GT_HS){
				g_entities[level.sortedClients[i]].client->pers.identity = BG_FindTeamIdentity ( level.gametypeTeam[newTeam], -1 );
			}
		}
		
		// Boe!Man 7/13/12: Prep for change & respawn.
		g_entities[level.sortedClients[i]].client->pers.identity = NULL;
		ClientUserinfoChanged( level.sortedClients[i]);		
		CalculateRanks();

		G_StopFollowing(&g_entities[level.sortedClients[i]]);
		G_StopGhosting(&g_entities[level.sortedClients[i]]);
		trap_UnlinkEntity (&g_entities[level.sortedClients[i]]);
		ClientSpawn(&g_entities[level.sortedClients[i]]);
	}
	
	// Boe!Man 6/16/12: Proper messaging/logging.
	Boe_GlobalSound(G_SoundIndex("sound/misc/events/tut_lift02.mp3"));
	trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sS%sh%su%sf%sf%sle teams!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
	if(ent && ent->client){
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Shuffle teams by %s.\n\"", ent->client->pers.netname));
		Boe_adminLog ("Shuffle Teams", va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName), "none");
	}else{
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7Shuffle teams.\n\""));
		Boe_adminLog ("Shuffle Teams", va("RCON"), "none");
	}
}

/*
==========
Boe_friendlyFire
==========
*/

void Boe_friendlyFire(int argNum, gentity_t *ent, qboolean shortCmd){
	if(!g_friendlyFire.integer){ // Boe!Man 11/5/12: Disabled, enable it.
		Boe_setTrackedCvar(18, 1);
		
		if(ent && ent->client){
			Boe_adminLog ("Friendly Fire Enabled", va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName), "none");
			trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Friendly fire enabled by %s.\n\"", ent->client->pers.netname));
		}else{
			Boe_adminLog ("Friendly Fire Enabled", va("RCON"), "none");
			trap_SendServerCommand( -1, va("print \"^3[Rcon Action] ^7Friendly fire enabled.\n"));
		}
	}else{ // Boe!Man 11/5/12: Enabled, disable it.
		Boe_setTrackedCvar(18, 0);
		
		if(ent && ent->client){
			Boe_adminLog ("Friendly Fire Disabled", va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName), "none");
			trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Friendly fire disabled by %s.\n\"", ent->client->pers.netname));
		}else{
			Boe_adminLog ("Friendly Fire Disabled", va("RCON"), "none");
			trap_SendServerCommand( -1, va("print \"^3[Rcon Action] ^7Friendly fire disabled.\n"));
		}
	}
	
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
	return;
}
