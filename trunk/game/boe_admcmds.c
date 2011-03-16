// Copyright (C) 2010 - Boe!Man.
//
// boe_admcmds.c - All the Admin Functions & commands for the Game Module go here.

#include "g_local.h"
#include "boe_local.h"

// --- BOE PROTOTYPE WARNINGS FIXES --- \\
// I'm lazy so I just 'fix' them here. I'll fix them fo'real later.
// ALREADY DONE LAZY BITCH -.-'' HENK
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
	// Boe!Man 1/19/11: Disable Compmode in H&S.
	if(current_gametype.value == GT_HS){
		if(ent && ent->client){
			trap_SendServerCommand(ent-g_entities, va("print\"^3[Info] ^7You cannot enable Competition Mode in Hide&Seek.\n\""));
		}else{
			Com_Printf("^7You cannot enable Competition Mode in Hide&Seek.\n");
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
		trap_Cvar_Set("cm_slock", g_autoLockSpec.string);
		trap_Cvar_Set("cm_aswap", g_autoSwapTeams.string);
		trap_Cvar_Set("cm_dsounds", g_matchDisableSounds.string);
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

	g_instagib.integer = 0;
	RPM_WeaponMod ();
	//BG_InitWeaponStats(qfalse);
	
	for(i=0;i<=level.numConnectedClients;i++){
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

	g_instagib.integer = 1;
	RPM_WeaponMod ();
	//BG_InitWeaponStats(qfalse);
	for(i=0;i<=level.numConnectedClients;i++){
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
		if (g_compMode.integer > 0){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Match timelimit is %i.\n\"", cm_tl.integer));
		}
		else{
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Timelimit is %i.\n\"", g_timelimit.integer));
		}
		return;
	}
	// Boe!Man 11/16/10: Is Competition Mode active so we can update the scrim setting vs. the global one?
	if (g_compMode.integer > 0){
		if (cm_enabled.integer == 1){
			trap_Cvar_Set("cm_tl", va("%i", number));
			trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Match timelimit changed to %i by %s.\n\"", number, ent->client->pers.netname));
		}else if(cm_enabled.integer > 1 && cm_enabled.integer < 5){
			trap_Cvar_Set("cm_tl", va("%i", number));
			trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Match timelimit changed to %i by %s.\n\"", number, ent->client->pers.netname));
			trap_Cvar_Set("timelimit", va("%i", number));
		}
	}else{
		trap_SendConsoleCommand( EXEC_APPEND, va("timelimit %i\n", number));
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
		if (g_compMode.integer > 0){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Match scorelimit is %i.\n\"", cm_sl.integer));
		}
		else{
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Scorelimit is %i.\n\"", g_scorelimit.integer));
		}
		return;
	}
	// Boe!Man 11/16/10: Is Competition Mode active so we can update the scrim setting vs. the global one?
	if (g_compMode.integer > 0){
		if (cm_enabled.integer == 1){
			trap_Cvar_Set("cm_sl", va("%i", number));
			trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Match scorelimit changed to %i by %s.\n\"", number, ent->client->pers.netname));
		}else if(cm_enabled.integer > 1 && cm_enabled.integer < 5){
			trap_Cvar_Set("cm_sl", va("%i", number));
			trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Match scorelimit changed to %i by %s.\n\"", number, ent->client->pers.netname));
			trap_Cvar_Set("scorelimit", va("%i", number));
		}
	}else{
		trap_SendConsoleCommand( EXEC_APPEND, va("scorelimit %i\n", number));
		trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Scorelimit changed to %i by %s.\n\"", number, ent->client->pers.netname));
	}
	trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sS%sc%so%sr%se%slimit %i!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, number));
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
	Boe_adminLog (va("Scorelimit %i", number), va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName), "none");
}

void SetNades(char *status){
	trap_Cvar_Set("disable_pickup_weapon_M84", status);
	trap_Cvar_Set("disable_pickup_weapon_SMOHG92", status);
	trap_Cvar_Set("disable_pickup_weapon_AN_M14", status);
	trap_Cvar_Set("disable_pickup_weapon_M67", status);
	trap_Cvar_Set("disable_pickup_weapon_F1", status);
	trap_Cvar_Set("disable_pickup_weapon_L2A2", status);
	trap_Cvar_Set("disable_pickup_weapon_MDN11", status);
	trap_Cvar_Set("disable_pickup_weapon_M15", status);
	G_UpdateAvailableWeapons(); // also set the original g_availableWeapons for the client :)
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
	}

	if(g_disablenades.integer == 1){
		g_disablenades.integer = 0;
		trap_Cvar_Set("g_disablenades", "0");
	    SetNades("0");
		trap_Cvar_Update(&g_disablenades);
		BG_SetAvailableOutfitting(g_availableWeapons.string);
		for(i=0;i<level.numConnectedClients;i++){
			level.clients[level.sortedClients[i]].noOutfittingChange = qfalse;
			Com_Printf("Setting nades\n");
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
		g_disablenades.integer = 1;
		trap_Cvar_Set("g_disablenades", "1");
		SetNades("1");
		trap_Cvar_Update(&g_disablenades);
		BG_SetAvailableOutfitting(g_availableWeapons.string);
		for(i=0;i<=level.numConnectedClients;i++){
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
		if (g_compMode.integer == 0){
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Map restarted by %s.\n\"", ent->client->pers.netname));
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%sM%sa%sp %sr%se%sstart in 5!", level.time + 1000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
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
		if (g_compMode.integer == 0){
			trap_SendServerCommand(-1, va("print \"^3[Rcon Action] ^7Map restarted.\n\""));
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%sM%sa%sp %sr%se%sstart in 5!", level.time + 1000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
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
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s was %sk%si%sc%sk%se%sd ^7by %s", level.time + 5000, g_entities[id].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, ent->client->pers.netname));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was kicked by %s.\n\"", g_entities[id].client->pers.netname,ent->client->pers.netname));
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		Boe_adminLog ("Kick", va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName), va("%s\\%s", g_entities[id].client->pers.ip, g_entities[id].client->pers.cleanName));
	}else{
		id = Boe_ClientNumFromArg(ent, 2, "kick <id> <reason>", "Kick", qfalse, qfalse, qfalse);
		if(id < 0) return;
		trap_Argv( 3, arg3, sizeof( arg3 ) );
		trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"%s\"\n", id, arg3));
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s was %sk%si%sc%sk%se%sd ^7by %s", level.time + 5000, g_entities[id].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, ent->client->pers.netname));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was kicked by %s.\n\"", g_entities[id].client->pers.netname,ent->client->pers.netname));
		Boe_adminLog ("Kick", va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName), va("%s\\%s", g_entities[id].client->pers.ip, g_entities[id].client->pers.cleanName));
	}
}

/*
==========
Boe_Add_Clan_Member
==========
*/

void Boe_Add_Clan_Member(int argNum, gentity_t *adm, qboolean shortCmd)
{
	int             idnum, onlist;
	char			*id;

	idnum = Boe_ClientNumFromArg(adm, argNum, "addclan <idnumber>", "do this to", qfalse, qtrue, shortCmd);

	if(idnum < 0)
		return;

	g_entities[idnum].client->sess.clanMember = 1;

	id = g_entities[idnum].client->pers.boe_id;

	onlist = Boe_NameListCheck (0, id, g_clanfile.string, NULL, qfalse, qfalse, qfalse, qfalse, qfalse);

	if(onlist){
		if (adm && adm->client)
			trap_SendServerCommand(adm-g_entities, va("print\"^3[Info] ^7%s is already a Clan Member.\n\"", g_entities[idnum].client->pers.netname));
		else
		Com_Printf("^3[Info] ^7%s ^7is already a Clan Member.\n", g_entities[idnum].client->pers.netname);
		return;
	}
	if(onlist == -1)
		return;

	if(Boe_AddToList(id, g_clanfile.string, "Clan", NULL))
	{
		// Boe!Man 1/28/11: Fixed clan members getting added to the file twice.
		if(adm && adm->client)	{
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s is now a %sC%sl%sa%sn %sm%se%smber!", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color4.string, server_color5.string, server_color6.string));
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was made Clan member by %s.\n\"", g_entities[idnum].client->pers.netname, adm->client->pers.cleanName));
			Boe_adminLog ("Add Clan", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));
		}else {
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s is now a %sC%sl%sa%sn %sm%se%smber!", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color4.string, server_color5.string, server_color6.string));
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s is now a Clan member.\n\"", g_entities[idnum].client->pers.netname));
			Boe_adminLog ("Add Clan", va("%s", "RCON"), va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));
		}
	}
}
/*
======================
Boe_Remove_Clan_Member
======================
*/

void Boe_Remove_Clan_Member(int argNum, gentity_t *adm, qboolean shortCmd)
{
	int			idnum;
	char		*id;

	idnum = Boe_ClientNumFromArg(adm, argNum, "removeclan <idnumber>", "remove", qfalse, qtrue, shortCmd);

	///if invalid client etc.. idnum will == -1 and we abort
	if(idnum < 0)
		return;

	///make sure they are not admins even if not on list
	g_entities[idnum].client->sess.clanMember = 0;

	id = g_entities[idnum].client->pers.boe_id;

	if(Boe_Remove_from_list(id, g_clanfile.string, "Clan", NULL, qfalse, qfalse, qfalse))
	{
		if(adm && adm->client)	{
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s is no longer a %sC%sl%sa%sn %sm%se%smber!", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color4.string, server_color5.string, server_color6.string));
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was removed as Clan member by %s.\n\"", g_entities[idnum].client->pers.netname, adm->client->pers.cleanName));
			Boe_adminLog ("Remove Clan", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));
		}else {
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s is no longer a %sC%sl%sa%sn %sm%se%smber!", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color4.string, server_color5.string, server_color6.string));
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s is no longer a Clan member.\n\"", g_entities[idnum].client->pers.netname));
			Boe_adminLog ("Remove Clan", va("%s", "RCON"), va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));
		}
	}
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
	int i, y, x, count;
	char *numb;
	qboolean first = qtrue;
	qboolean space = qfalse;
	trap_Argv( argNum, arg, sizeof( arg ) );
	if(shortCmd){ // Henk 04/05/10 -> Handle the short admin commands.
		num = -1;
		for(i=0;i<16;i++){ // FIX ME HENK: could be out of bounds
			if(arg[i] == ' '){
				//trap_SendServerCommand( -1, va("print \"^3[Debug] ^7Arg[%i] = %s.\n\"", i, arg[i]));
				if(henk_isdigit(arg[i+1]) && !henk_ischar(arg[i+2])){ // we check if the other 2 are chars(fixes !uc 1fx bug which uppercuts id 1)
				num = atoi(va("%c%c", arg[i+1], arg[i+2]));
				}else{
					for(x=0;x<=20;x++){ // FIX ME HENK: could be out of bounds
						if(arg[x] == ' '){
							// Henk 08/09/10 -> Check for another arg in this.
							for(y=1;y<=sizeof(arg);y++){
								if(arg[x+y] == ' '){
									space = qtrue;
									break;
								}else{
									if(first){
										numb = va("%c", arg[x+y]);
										first = qfalse;
									}else{
										numb = va("%s%c", numb, arg[x+y]);
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
							numb = va("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", arg[x+1], arg[x+2], arg[x+3], arg[x+4], arg[x+5], arg[x+6], arg[x+7], arg[x+8], arg[x+9], arg[x+10], arg[x+11], arg[x+12], arg[x+13], arg[x+14], arg[x+15]);
							}
							break;
						}
					}
					for(x=0;x<=level.numConnectedClients;x++){
						//trap_SendServerCommand(-1, va("print\"^3[Debug] ^7%s comparing with %s.\n\"", g_entities[level.sortedClients[i]].client->pers.cleanName,numb));
						if(strstr(Q_strlwr(g_entities[level.sortedClients[x]].client->pers.cleanName), Q_strlwr(numb))){
							num = level.sortedClients[x];
							break;
						}
						num = -1;
					}
				}
				break;
			}else if(henk_isdigit(arg[i]) && henk_ischar(arg[i-1])){ // Henk 10/03/11 -> Add support for !u1/!r1 commands
				count = 0;
				for(x=i;x<strlen(arg);x++){
					if(henk_isdigit(arg[x])){
						numb[count] = arg[x];
						count += 1;
					}else{
						numb[count] = '\0';
						break;
					}
				}
				num = atoi(numb);
				trap_SendServerCommand(ent->s.number, va("print\"^3[Debug] ^7New short command style detected, Client number: %i.\n\"", num));
			}
		}
		if(num == -1){ // Get second argument because they use it from the console
			trap_Argv( 2, arg, sizeof( arg ) );
			if(strlen(arg) >= 1){
				if(henk_isdigit(arg[0])){
					num = atoi(arg);
				}else{
					for(i=0;i<=level.numConnectedClients;i++){
						//trap_SendServerCommand(-1, va("print\"^3[Debug] ^7%s comparing with %s.\n\"", g_entities[level.sortedClients[i]].client->pers.cleanName,numb));
						if(strstr(Q_strlwr(g_entities[level.sortedClients[i]].client->pers.cleanName), Q_strlwr(arg))){
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
			for(i=0;i<=level.numConnectedClients;i++){
				//trap_SendServerCommand(-1, va("print\"^3[Debug] ^7%s comparing with %s.\n\"", g_entities[level.sortedClients[i]].client->pers.cleanName,numb));
				if(strstr(Q_strlwr(g_entities[level.sortedClients[i]].client->pers.cleanName), Q_strlwr(arg))){
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
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7This client has been disconnected.\n\""));
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
			if(g_entities[num].client->sess.admin > ent->client->sess.admin){
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
=============
*/

int Boe_AddToList(const char *string, const char* file, const char* type, gentity_t* ent)
{
	int				len;
	fileHandle_t	f;
	len = trap_FS_FOpenFile( file, &f, FS_APPEND_TEXT );

    if (!f)
	{
		if(ent && ent->client)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"^1Error opening %s File\n\"", type));
		}
		else
		{
			Com_Printf("^1Error opening %s File\n", type);
		}
	    return -1;
	}

	if(len > 15000 - (strlen(string) + 1))
	{
		trap_FS_FCloseFile(f);

		if(ent && ent->client)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"^3List full, Unable to add %s.\n\"", type));
		}
		else
		{
			Com_Printf("^3List full, Unable to add %s.\n", type);
		}
		return 0;
	}
	trap_FS_Write(string, strlen(string), f);
	trap_FS_Write("\n", 1, f);
	trap_FS_FCloseFile(f);

	return 1;
}

/*
====================
Boe_Remove_from_list
====================
*/

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
	Com_Printf("Remove: opening %s\n", file);
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
	void	*GP2, *group;
	char	ip[64], name[64], reason[64], by[64], test = ' ';
	char	column1[20], column2[20], column3[20];
	int		spaces = 0, length = 0, z;
	char	*file;
	fileHandle_t f;
	char            buf[15000] = "\0";
	int len, i, count = 0, EndPos = -1, StartPos = 0;
	qboolean begin = qtrue;
	int r, lcount = -1, lenx;
	char xip[64];
	char fileName[128];
	//wrapper for interface
	if(adm){
		if(subnet)
			trap_SendServerCommand( adm-g_entities, va("print \"^3[Subnetbanlist]^7\n\n\""));
		else
			trap_SendServerCommand( adm-g_entities, va("print \"^3[Banlist]^7\n\n\""));
		trap_SendServerCommand( adm-g_entities, va("print \"^3 #    IP              Name                Reason             By\n\""));
		trap_SendServerCommand( adm-g_entities, va("print \"^7------------------------------------------------------------------------\n\""));
	}else{
		if(subnet)
			Com_Printf("^3[Subnetbanlist]^7\n\n");
		else
			Com_Printf("^3[Banlist]^7\n\n");
		Com_Printf("^3 #    IP              Name                Reason             By\n");
		Com_Printf("^7------------------------------------------------------------------------\n");
	}
	if(subnet)
		strcpy(fileName, "users/subnetbans.txt");
	else
		strcpy(fileName, g_banfile.string);

	len = trap_FS_FOpenFile(fileName, &f, FS_READ);
	if(!test){
		Com_Printf("Error while reading %s\n", fileName);
		return;
	}
	trap_FS_Read( buf, len, f );
	buf[len] = '\0';
	trap_FS_FCloseFile(f);
	// We parse the buffer, each IP follow it to the file in baninfo(if exists), if empty ignore and LINES +1(CUZ IT IS A LINE IN BANLIST!)
	for(i=0;i<len;i++){
		if(begin && buf[i] == '\n'){
			StartPos = i+1;
			EndPos = -1;
			begin = qfalse;
		}
		if(buf[i] == '\\' && buf[i] != '\n'){
			EndPos = i;
			begin = qtrue;
			memset(xip, 0, sizeof(xip));
			strncpy(xip, buf+StartPos, EndPos-StartPos);
			lcount = -1;
			for(r=i;r<strlen(buf);r++){
				if(buf[r] == '\n'){
					lcount = r-1;
					break;
				}
			}
			if(lcount == -1){
				lcount = strlen(buf); // last line
			}
			strncpy(name, buf+EndPos+1, lcount-(EndPos+1));
			Com_Printf("IP: %s\n", xip);
			Com_Printf("Name: %s\n", name);
			count += 1; // Henk 25/01/11 -> Fix wrong ban lines.
			// Start extracting and printing baninfo
			if(subnet)
				file = va("users/baninfo/subnet/%s.IP", xip);
			else
				file = va("users/baninfo/%s.IP", xip);
			lenx = trap_FS_FOpenFile(file, &f, FS_READ);
			if(f){
				if(lenx >= 1){
					GP2 = trap_GP_ParseFile(file, qtrue, qfalse);
					group = trap_GPG_GetSubGroups(GP2);
					trap_GPG_FindPairValue(group, "ip", "0", ip);
					trap_GPG_FindPairValue(group, "name", "", name);
					trap_GPG_FindPairValue(group, "reason", "", reason);
					trap_GPG_FindPairValue(group, "by", "", by);
					trap_GP_Delete(&GP2);
				}else{
					strcpy(ip, xip);
					strcpy(reason, "");
					strcpy(by, "");
					Com_Printf("Not opening because file is too small.\n");
				}
				trap_FS_FCloseFile(f);
			}else{
				strcpy(ip, xip);
				strcpy(reason, "");
				strcpy(by, "");
				Com_Printf("No baninfo available\n");
			}
			length = strlen(ip);
			if(length > 15){
				ip[15] = '\0';
				length = 15;
			}
			spaces = 16-length;
			for(z=0;z<spaces;z++){
			column1[z] = test;
			}
			column1[spaces] = '\0';
			//trap_SendServerCommand( adm-g_entities, va("print \"%s%s", ip, column1)); // Boe!Man 9/16/10: Print tier 1.
			length = strlen(name);
			if(length > 19){
				name[19] = '\0';
				length = 19;
			}
			spaces = 20-length;
			for(z=0;z<spaces;z++){
			column2[z] = test;
			}
			column2[spaces] = '\0';
			//trap_SendServerCommand( adm-g_entities, va("print \"%s%s", name, column2)); // Boe!Man 9/16/10: Print tier 2.
			length = strlen(reason);
			if(length > 18){
				reason[18] = '\0';
				length = 18;
			}
			spaces = 19-length;
			for(z=0;z<spaces;z++){
			column3[z] = test;
			}
			column3[spaces] = '\0';
			length = strlen(by);
			if(length > 20){
				by[20] = '\0';
			}
			if(adm){
				// Boe!Man 1/24/11: Print the banline as well (for easy unbanning).
				if(count <= 9){
					trap_SendServerCommand( adm-g_entities, va("print \"[^3%i^7]   %s%s%s%s%s%s%s\n", count, ip, column1, name, column2, reason, column3, by)); // Boe!Man 9/16/10: Print ban.
				}else if(count > 9 && count < 100){
					trap_SendServerCommand( adm-g_entities, va("print \"[^3%i^7]  %s%s%s%s%s%s%s\n", count, ip, column1, name, column2, reason, column3, by)); // Boe!Man 9/16/10: Print ban.
				}else{
					trap_SendServerCommand( adm-g_entities, va("print \"[^3%i^7] %s%s%s%s%s%s%s\n", count, ip, column1, name, column2, reason, column3, by)); // Boe!Man 9/16/10: Print ban.
				}
			}else{
				if(count <= 9){
					Com_Printf("[^3%i^7]   %s%s%s%s%s%s%s\n", count, ip, column1, name, column2, reason, column3, by);
				}else if(count > 9 && count < 100){
					Com_Printf("[^3%i^7]  %s%s%s%s%s%s%s\n", count, ip, column1, name, column2, reason, column3, by);
				}else{
					Com_Printf("[^3%i^7] %s%s%s%s%s%s%s\n", count, ip, column1, name, column2, reason, column3, by);
				}
			}
			//trap_SendServerCommand( adm-g_entities, va("print \"%s\n", by)); // Boe!Man 9/16/10: Print tier 4.
			//trap_SendServerCommand( adm-g_entities, va("print \"%s\n%s\n%s\n%s\n\"", ip, name, reason, by)); // print result
			// End
		}
	}
	// End
	trap_SendServerCommand( adm-g_entities, va("print \"\nUse ^3[Page Up] ^7and ^3[Page Down] ^7keys to scroll\n\n\""));
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
	int             i = 0;
	char		    *ip = g_entities[idnum].client->pers.ip;
	char			*id = g_entities[idnum].client->pers.boe_id;

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

	Com_sprintf(id, MAX_BOE_ID, "%s\\%s", ip, g_entities[idnum].client->pers.cleanName);
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
Boe_Add_bAdmin_f
================
*/

void Boe_Add_bAdmin_f(int argNum, gentity_t *adm, qboolean shortCmd)
{
	int             idnum;
	char			*id;
	char			id2[64];
	char			arg[64] = "\0";

	idnum = Boe_ClientNumFromArg(adm, argNum, "addbadmin <idnumber>", "do this to", qfalse, qfalse, shortCmd);
	if(idnum < 0) return;

	if(shortCmd){
		strcpy(GetReason(), arg);
	}else{
		if(adm && adm->client){
			trap_Argv(3, arg, sizeof(arg));
		}else{
			trap_Argv(2, arg, sizeof(arg));
		}
	}

	if(!Q_stricmp(arg, "pass")){
		AddToPasswordList(&g_entities[idnum], 2);
		g_entities[idnum].client->sess.admin = 2;
		return;
	}

	id = g_entities[idnum].client->pers.boe_id;

	if(Boe_Remove_from_list(id, g_adminfile.string,	 "admin", NULL, qfalse, qtrue, qfalse));

	g_entities[idnum].client->sess.admin = 2;
	Q_strncpyz (id2, id, 64);
	strcat ( id2, ":2" );

	// Boe!Man 1/14/11
	if(adm && adm->client)	{
		if(Boe_AddToList(id2, g_adminfile.string, "Admin", NULL)) {
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s is now a %sB^7-%sA%sd%sm%si%sn", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was made B-Admin by %s.\n\"", g_entities[idnum].client->pers.netname, adm->client->pers.cleanName));
		Boe_adminLog ("Add B-Admin", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));
		}
	}
	else {
		if(Boe_AddToList(id2, g_adminfile.string, "Admin", NULL)) {
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s is now a %sB^7-%sA%sd%sm%si%sn", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s is now a B-Admin.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog ("Add B-Admin", va("%s", "RCON"), va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));
		}
	}
	// Boe!Man 10/16/10: Is the Admin level allowed to spec the opposite team?
	if (g_adminspec.integer <= 2)
		g_entities[idnum].client->sess.adminspec = qtrue;
}

/*
===============
Boe_Add_Admin_f
===============
*/

void Boe_Add_Admin_f(int argNum, gentity_t *adm, qboolean shortCmd)
{
	int             idnum;
	char			*id;
	char			id2[64];
	char			arg[64] = "\0";
	idnum = Boe_ClientNumFromArg(adm, argNum, "addadmin <idnumber>", "do this to", qfalse, qfalse, shortCmd);
	if(idnum < 0) return;

	if(shortCmd){
		strcpy(GetReason(), arg);
	}else{
		if(adm && adm->client){
			trap_Argv(3, arg, sizeof(arg));
		}else{
			trap_Argv(2, arg, sizeof(arg));
		}
	}

	if(!Q_stricmp(arg, "pass")){
		AddToPasswordList(&g_entities[idnum], 3);
		g_entities[idnum].client->sess.admin = 3;
		return;
	}

	id = g_entities[idnum].client->pers.boe_id; 

	if(Boe_Remove_from_list(id, g_adminfile.string, "admin", NULL, qfalse, qtrue, qfalse));

	g_entities[idnum].client->sess.admin = 3;
	Q_strncpyz (id2, id, 64);
	strcat ( id2, ":3" );

	// Boe!Man 1/14/10
	if(adm && adm->client)	{
		if(Boe_AddToList(id2, g_adminfile.string, "Admin", NULL)) {
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s is now an %sA%sd%sm%si%sn", level.time + 5000, g_entities[idnum].client->pers.netname, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was made Admin by %s.\n\"", g_entities[idnum].client->pers.netname, adm->client->pers.cleanName));
		Boe_adminLog ("Add Admin", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));
		}
	}
	else {
		if(Boe_AddToList(id2, g_adminfile.string, "Admin", NULL)) {
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s is now an %sA%sd%sm%si%sn", level.time + 5000, g_entities[idnum].client->pers.netname, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s is now an Admin.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog ("Add Admin", va("%s", "RCON"), va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));
		}
	}
	// Boe!Man 10/16/10: Is the Admin level allowed to spec the opposite team?
	if (g_adminspec.integer <= 3)
		g_entities[idnum].client->sess.adminspec = qtrue;
}

/*
================
Boe_Add_sAdmin_f
================
*/

void Boe_Add_sAdmin_f(int argNum, gentity_t *adm, qboolean shortCmd)
{
	int             idnum;
	char			*id;
	char			id2[64];
	char			arg[64] = "\0";

	idnum = Boe_ClientNumFromArg(adm, argNum, "addsadmin <idnumber>", "do this to", qfalse, qfalse, shortCmd);
	if(idnum < 0) return;

	if(shortCmd){
		strcpy(GetReason(), arg);
	}else{
		if(adm && adm->client){
			trap_Argv(3, arg, sizeof(arg));
		}else{
			trap_Argv(2, arg, sizeof(arg));
		}
	}

	if(!Q_stricmp(arg, "pass")){
		AddToPasswordList(&g_entities[idnum], 4);
		g_entities[idnum].client->sess.admin = 4;
		return;
	}

	id = g_entities[idnum].client->pers.boe_id; 

	if(Boe_Remove_from_list(id, g_adminfile.string, "admin", NULL, qfalse, qtrue, qfalse));

	g_entities[idnum].client->sess.admin = 4;
	Q_strncpyz (id2, id, 64);
	strcat ( id2, ":4" );

	// Boe!Man 1/14/11
	if(adm && adm->client)	{
		if(Boe_AddToList(id2, g_adminfile.string, "Admin", NULL)) {
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s is now a %sS^7-%sA%sd%sm%si%sn", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was made S-Admin by %s.\n\"", g_entities[idnum].client->pers.netname, adm->client->pers.cleanName));
		Boe_adminLog ("Add S-Admin", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));
		}
	}
	else {
		if(Boe_AddToList(id2, g_adminfile.string, "Admin", NULL)) {
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s is now a %sS^7-%sA%sd%sm%si%sn", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s is now a S-Admin.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog ("Add S-Admin", va("%s", "RCON"), va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));
		}
	}

	// Boe!Man 10/16/10: Is the Admin level allowed to spec the opposite team?
	if (g_adminspec.integer <= 4)
		g_entities[idnum].client->sess.adminspec = qtrue;
}
		/*
		if(adm && adm->client)
			SC_adminLog (va("%s - AddAdmin: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
		else 
			SC_adminLog (va("%s - AddAdmin: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
		*/

/*
================
Henk_RemoveLineFromFile
23/01/11 -> By Henkie

Removes a zero based line from a file
================
*/
void Henk_RemoveLineFromFile(gentity_t *ent, int line, char *file, qboolean subnet){
	fileHandle_t	f;
	int len, CurrentLine = 0, StartPos = 0, EndPos = -1, i;
	qboolean begin = qtrue;
	char buf[15000];
	char newbuf[15000];
	char asd[128] = "";
	char last[128] = "", lastip[64] = "";
	char fileName[128];
	qboolean done = qfalse;
	line = line;
	memset( buf, 0, sizeof(buf) );
	memset( newbuf, 0, sizeof(newbuf) );
	memset( last, 0, sizeof(last) );
	memset( lastip, 0, sizeof(lastip) );
	memset( fileName, 0, sizeof(fileName) );
	len = trap_FS_FOpenFile( file, &f, FS_READ_TEXT);
	if(!f)
		return;
	trap_FS_Read( buf, len, f );
	buf[len] = '\0';
	trap_FS_FCloseFile( f );
	for(i=0;i<=len;i++){
		memset( asd, 0, sizeof(asd) );
		if(i == len){
			EndPos = i;
			if(line != CurrentLine){
			strncpy(asd, buf+StartPos, EndPos);
			// Boe!Man 1/24/11: Fixed compiling errors using sprintf.
#ifdef Q3_VM
			Com_sprintf(newbuf, sizeof(newbuf), va("%s%s", newbuf, asd));
#else
			sprintf(newbuf, "%s%s", newbuf, asd);
#endif
			
			}else{
				strncpy(last, buf+StartPos, EndPos-StartPos); 
				done = qtrue;
				//Com_Printf("Last: %s\n", asd);
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
			EndPos = i;
			begin = qtrue;
		}
		if(EndPos != -1){
			if(line != CurrentLine){
			strncpy(asd, buf+StartPos, EndPos-StartPos); 
			// Boe!Man 1/24/11: Fixed compiling errors using sprintf.
#ifdef Q3_VM
			Com_sprintf(newbuf, sizeof(newbuf), va("%s%s\n", newbuf, asd));
#else
			sprintf(newbuf, "%s%s\n", newbuf, asd);
#endif
			}else{
				done = qtrue;
				strncpy(last, buf+StartPos, EndPos-StartPos); 
				//Com_Printf("Final: %s\n", asd);
			}
		}
	}
	if(done && strlen(last) >= 1){ // && !strstr(newbuf, last)
	// Start writing our new created file
	len = trap_FS_FOpenFile( file, &f, FS_WRITE_TEXT);
	if(!f)
		return;
	trap_FS_Write(newbuf, strlen(newbuf), f);
	trap_FS_FCloseFile( f );
	// Clear ban info
	for(i=0;i<strlen(last);i++){
		if(last[i] == '\\'){
			strncpy(lastip, last, i);
			break;
		}
	}
	Com_Printf("Last ip: %s\n", lastip);
	if(subnet)
		strcpy(fileName, va("users\\baninfo\\subnet\\%s.IP", lastip));
	else
		strcpy(fileName, va("users\\baninfo\\%s.IP", lastip));

	len = trap_FS_FOpenFile(fileName , &f, FS_WRITE );
	if(f){
		trap_FS_Write("", 0, f);
		trap_FS_FCloseFile(f);
	}else{
		Com_Printf("Error while opening file in unban\n");
	}
	// End
		if(subnet){
			if(ent && ent->client)
				Boe_adminLog ("Subnet Unban", va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName), va("%s", last));
			else 
				Boe_adminLog ("Subnet Unban", va("RCON"), va("%s", last));
		}else{
			if(ent && ent->client)
				Boe_adminLog ("Unban", va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName), va("%s", last));
			else 
				Boe_adminLog ("Unban", va("RCON"), va("%s", last));
		}
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7%s has been Unbanned.\n\"", last));
	//return qtrue;
	}else{
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Could not find line %i.\n\"", line));
	}
}

/*
==================
Boe_Unban
==================
*/

void Boe_Unban(gentity_t *adm, char *ip, qboolean subnet)
{
	int iLine;
	fileHandle_t f;

	if(strlen(ip) < 2 && strstr(ip, ".")){
		trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7Invalid IP, Usage: adm unban <IP/Line>.\n\""));
		return;
	}else if(strlen(ip) >= 1 && !strstr(ip, ".")){
		// unban by line
		iLine = atoi(ip);
		if(subnet)
			Henk_RemoveLineFromFile(adm, iLine, "users/subnetbans.txt", qtrue);
		else
			Henk_RemoveLineFromFile(adm, iLine, g_banfile.string, qfalse);
		return;
	}else if(strlen(ip) < 2){
		trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7Invalid IP, Usage: adm unban <IP/Line>.\n\""));
		return;
	}

	if(!subnet){
			if(Boe_Remove_from_list(ip, g_banfile.string, "Ban", adm, qtrue, qfalse, qfalse )){
				trap_SendServerCommand( adm-g_entities, va("print \"^3%s ^7has been Unbanned.\n\"", ip));
				trap_FS_FOpenFile( va("users\\baninfo\\%s.IP", ip), &f, FS_WRITE );
				if(f){
					trap_FS_Write("", 0, f);
					trap_FS_FCloseFile(f);
				}else{
					Com_Printf("Error while opening file in unban\n");
				}
				
				//if(adm && adm->client)
				//	Boe_adminLog (va("%s - UNBAN: %s", adm->client->pers.cleanName, ip  )) ;
				//else 
				//	Boe_adminLog (va("%s - UNBAN: %s", "RCON", ip  )) ;
				
				return;
			}
	}
	else {
		if(Boe_Remove_from_list(ip, "users/subnetbans.txt", "SubnetBan", adm, qtrue, qfalse, qfalse )){
			trap_SendServerCommand( adm-g_entities, va("print \"^3%s's Subnet ^7has been Unbanned.\n\"", ip));
			//if(adm && adm->client)
			//	Boe_adminLog (va("%s - SUBNET UNBAN: %s", adm->client->pers.cleanName, ip  )) ;
			//else 
			//	Boe_adminLog (va("%s - SUBNET UNBAN: %s", "RCON", ip  )) ;
			return;
		}
	}
}

/*
==================
Boe_NameListCheck
==================
*/

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

/*
=================
Boe_subnetBan
=================
*/
void Boe_subnetBan (int argNum, gentity_t *adm, qboolean shortCmd){
	int				idnum;
	char			reason[MAX_STRING_TOKENS] = "\0";
	fileHandle_t	f;
	char			string[1024];
	int i;
	char arg[64];
	char *temp = "";
	char			ip[16];
	char			info[1024];
	qboolean first = qfalse;
	idnum = Boe_ClientNumFromArg(adm, argNum, "subnetban <idnumber> <reason>", "subnetban", qfalse, qfalse, shortCmd);
	if(idnum < 0)
		return;
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
	Q_strncpyz(ip, g_entities[idnum].client->pers.ip, 7);
	// Boe!Man 9/7/10: Example of ban.
		trap_FS_FOpenFile( va("users/baninfo/subnet/%s.IP", ip), &f, FS_WRITE );
		if (!f)
		{
			Com_Printf("^1Error opening File\n");
			return;
		}
		if (adm && adm->client)
		strcpy(string, va("1\n{\nip \"%s\"\nname \"%s\"\nreason \"%s\"\nby \"%s\"\n}\n", ip, g_entities[idnum].client->pers.cleanName, reason, adm->client->pers.cleanName));
		else
		strcpy(string, va("1\n{\nip \"%s\"\nname \"%s\"\nreason \"%s\"\nby \"RCON\"\n}\n", ip, g_entities[idnum].client->pers.cleanName, reason));

		trap_FS_Write(string, strlen(string), f);
		trap_FS_Write("\n", 1, f);
		trap_FS_FCloseFile(f);

	// Boe!Man 1/7/10: No more logging of reason in the ban file.
	strcpy(info, va("%s\\%s", ip, g_entities[idnum].client->pers.cleanName));

	if(Boe_AddToList(info, "users/subnetbans.txt", "Subnet ban", adm)){
		if(adm && adm->client)	{
			if(!*reason){
				trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"Subnetbanned by %s\"\n", idnum, adm->client->pers.netname));
				Boe_adminLog ("Subnetban", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));}
			else{
				trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"Subnetbanned by %s for: %s\"\n", idnum, adm->client->pers.netname, reason));
				trap_SendServerCommand( adm-g_entities, va("print \"%s was Subnetbanned by %s!\n\"", g_entities[idnum].client->pers.netname, adm->client->pers.netname));
				Boe_adminLog ("Subnetban", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));
			}
		}
		else{
			if(!*reason){
				Boe_adminLog ("Subnetban", va("%s", "RCON"), va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));
				trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"Subnetbanned!\"\n", idnum));
			}else{
				Com_Printf("%s was subnetbanned!\n", g_entities[idnum].client->pers.cleanName);
				Boe_adminLog ("Subnetban", va("%s", "RCON"), va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));
				trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"Subnetbanned for: %s\"\n", idnum, reason));
			}
		}
	}
}

/*
==================
Boe_Remove_Admin_f
==================
*/

void Boe_Remove_Admin_f (int argNum, gentity_t *adm, qboolean shortCmd)

{
	int				idnum;
	char			*id;

#ifdef _BOE_DBG
	if (strstr(boe_log.string, "1"))
		G_LogPrintf("4s\n");
#endif

	idnum = Boe_ClientNumFromArg(adm, argNum, "removeadmin <idnumer>", "do this to", qfalse, qtrue, shortCmd);
	if(idnum < 0) return;

	id = g_entities[idnum].client->pers.boe_id;
	
	// Boe!Man 8/22/10: If the user's not an Admin we can safely skip this.
	if (g_entities[idnum].client->sess.admin == 0){
		if(adm && adm->client){
			trap_SendServerCommand(adm-g_entities, va("print\"^3[Info] ^7This client is currently not an Admin.\n\"", g_entities[idnum].client->pers.netname));
		}else{
			Com_Printf("This client is currently not an Admin.\n");
		}
		return;
	}
	
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
	g_entities[idnum].client->sess.admin = 0;
	// Boe!Man 1/6/10: Fix, succesfully writes the Admin out of the file.
	if(Boe_Remove_from_list(id, g_adminfile.string, "admin", NULL, qfalse, qtrue, qfalse)){
		if(adm && adm->client){
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7is no longer an %sA%sd%sm%si%sn", level.time + 5000, g_entities[idnum].client->pers.netname, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was removed as Admin by %s.\n\"", g_entities[idnum].client->pers.netname,adm->client->pers.netname));
			Boe_adminLog ("Remove Admin", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));
		}
		else{
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7is no longer an %sA%sd%sm%si%sn", level.time + 5000, g_entities[idnum].client->pers.netname, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
			trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s ^7was removed as Admin.\n\"", g_entities[idnum].client->pers.netname));
			Boe_adminLog ("Remove Admin", va("%s", "RCON"), va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));
		}
	}

	// Boe!Man 10/16/10: He's not an Admin anymore so it doesn't matter if he was a B-Admin, Admin or S-Admin: either way he shouldn't be allowed to spec the opposite team.
	g_entities[idnum].client->sess.adminspec = qfalse;

#ifdef _BOE_DBG
	if (strstr(boe_log.string, "1"))
		G_LogPrintf("4e\n");
#endif
}

/*
=========
Boe_Ban_f
=========
*/

void Boe_Ban_f (int argNum, gentity_t *adm, qboolean shortCmd)
{
	int				idnum;
	char            banid[1024]; // Henk 07/10/10 -> Needs to be bigger if we want to add reason so 128 -> 1024
	char			reason[MAX_STRING_TOKENS] = "\0";
	fileHandle_t	f;
	char			string[1024];
	int i;
	char arg[64];
	char *temp = "";
	qboolean first = qfalse;
	idnum = Boe_ClientNumFromArg(adm, argNum, "ban <idnumber> <reason>", "ban", qfalse, qfalse, shortCmd);
	if(idnum < 0)
		return;
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
	// Boe!Man 9/7/10: Example of ban.
		trap_FS_FOpenFile( va("users/baninfo/%s.IP", g_entities[idnum].client->pers.ip), &f, FS_WRITE );
		if (!f)
		{
			Com_Printf("^1Error opening File\n");
			return;
		}
		if (adm && adm->client)
		strcpy(string, va("1\n{\nip \"%s\"\nname \"%s\"\nreason \"%s\"\nby \"%s\"\n}\n", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName, reason, adm->client->pers.cleanName));
		else
		strcpy(string, va("1\n{\nip \"%s\"\nname \"%s\"\nreason \"%s\"\nby \"RCON\"\n}\n", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName, reason));

		trap_FS_Write(string, strlen(string), f);
		trap_FS_Write("\n", 1, f);
		trap_FS_FCloseFile(f);

		Com_sprintf (banid, sizeof(banid), "%s\\%s",
		g_entities[idnum].client->pers.ip,
		g_entities[idnum].client->pers.cleanName);

	if(Boe_AddToList(banid, g_banfile.string, "Ban", adm)){
		if(adm && adm->client)	{
			if(!*reason){
				trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"Banned by %s\"\n", idnum, adm->client->pers.netname));
				Boe_adminLog ("Ban", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));
			}
			else{
				trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"Banned by %s for: %s\"\n", idnum, adm->client->pers.netname, reason));
				trap_SendServerCommand( adm-g_entities, va("print \"%s was banned by %s!\n\"", g_entities[idnum].client->pers.netname, adm->client->pers.netname));
				Boe_adminLog ("Ban", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));
			}
		}
		else{
			if(!*reason){
				Boe_adminLog ("Ban", va("%s", "RCON"), va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));
				trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"Banned!\"\n", idnum));
			}
			else{
				Com_Printf("%s was banned!\n", g_entities[idnum].client->pers.cleanName);
				Boe_adminLog ("Ban", va("%s", "RCON"), va("%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName));
				trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"Banned for: %s\"\n", idnum, reason));
			}
		}
	}
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
	char			*status;
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
	
	if(g_entities[idnum].client->sess.lastIdentityChange)	{
		status = ".";
		g_entities[idnum].client->sess.lastIdentityChange = qfalse;
	}
	else {
		status = ".";
		g_entities[idnum].client->sess.lastIdentityChange = qtrue;
	}
	
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

	SetClientViewAngle(ent, lookdown);
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
	ent->client->pers.twisted = qtrue;
	
	/*
	for (i = 0; i < level.numConnectedClients; i++)	{
		if (level.clients[level.sortedClients[i]].sess.admin){	
			if(adm != NULL)
				//CPx(level.sortedClients[i], va("print \"^7%s^7: ^3[%s] %s\n\"", adm->client->pers.netname, action, target));
				trap_SendServerCommand(level.sortedClients[i], va("print \"^7%s^7: ^3[Twisted] ^7%s\n\"", adm->client->pers.netname, g_entities[idnum].client->pers.netname));
			else 
				trap_SendServerCommand(level.sortedClients[i], va("print \"^7ADMIN^7: ^3[Twisted] ^7%s\n\"", g_entities[idnum].client->pers.netname));
		}
	}
	*/

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
	SetClientViewAngle(ent, lookdown);
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
	char	*status;

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


	if(ent->client->sess.lastIdentityChange){
		status = ".";
		ent->client->sess.lastIdentityChange = qfalse;
	}
	else {
		status = ".";
		ent->client->sess.lastIdentityChange = qtrue;
	}
	
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
	int				anim = 0;

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
		for(i=StartAfterCommand(va("%s", buffer));i<=strlen(buffer);i++){
			buffer1[z] = buffer[i];
			z+= 1;
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
	char userinfo[MAX_INFO_STRING];
	// find the player
	if(shortCmd == qtrue){
		idnum = Boe_ClientNumFromArg(adm, 1, "forceteam <idnumber>", "forceteam", qfalse, qtrue, shortCmd);
	}else{
		if(adm)
			idnum = Boe_ClientNumFromArg(adm, 2, "forceteam <idnumber>", "forceteam", qfalse, qtrue, shortCmd);
		else
			idnum = Boe_ClientNumFromArg(adm, 1, "forceteam <idnumber>", "forceteam", qfalse, qtrue, shortCmd);
	}
	if(idnum < 0) return;

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
		if(adm && adm->client)
			trap_Argv( 3, str, sizeof( str ) );
		else
			trap_Argv( 2, str, sizeof( str ) );

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
	if(g_entities[idnum].client){
		if(g_entities[idnum].r.svFlags & SVF_BOT){ // Henk 25/01/11 -> Reset bots to set them to another team
			trap_GetUserinfo( idnum, userinfo, sizeof( userinfo ) );
			Info_SetValueForKey( userinfo, "team", str );
			trap_SetUserinfo( idnum, userinfo );
			g_entities[idnum].client->sess.team = xteam;
			if(current_gametype.value != GT_HS)
			g_entities[idnum].client->pers.identity = BG_FindTeamIdentity ( level.gametypeTeam[xteam], -1 );
			ClientBegin( idnum, qfalse );
		}else
		SetTeam( &g_entities[idnum], str, NULL, qtrue );
	}else{
		trap_SendServerCommand(adm->s.number, va("print\"^3[Info] Error, no client on id %i.\n\"", idnum));
		return;
	}
	Com_sprintf( merged, sizeof(merged), "%s\\%s", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName);
	// Boe!Man 2/13/11: Proper messaging..
	if(adm){
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sf%so%sr%sc%se%steamed by %s", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was forceteamed by %s.\n\"", g_entities[idnum].client->pers.netname,adm->client->pers.netname));
		Boe_adminLog (va("Forceteam to %c", team), va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), merged);
	}else{
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sf%so%sr%sc%se%steamed", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s ^7was forceteamed.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("Forceteam to %c", team), va("%s", "RCON"), merged);
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
			AddMutedClient(&g_entities[idnum], time); // fix me: slots of muted clients(20) could be full.. never happens i guess
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			g_entities[idnum].client->sess.mute = qtrue;
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
			g_entities[idnum].client->sess.mute = qfalse;
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
	client->ps.stats[STAT_OUTFIT_GRENADE] = bg_itemlist[bg_outfittingGroups[-1][client->pers.outfitting.items[-1]]].giTag;

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
==========
*/

void Boe_dev_f ( gentity_t *ent )
{

	int		dev;
	char	arg1[MAX_STRING_TOKENS];
	char	arg2[MAX_STRING_TOKENS];
	char	arg3[MAX_STRING_TOKENS];
	char	rcon[64];
	gclient_t	*client;
	client = ent->client;
	trap_Argv( 1, arg1, sizeof( arg1 ) );
	trap_Argv( 2, arg2, sizeof( arg2 ) );
	trap_Argv( 3, arg3, sizeof( arg3 ) );
	dev = ent->client->sess.dev;

	if(dev == 0){
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Access denied: You don't have Developer powers!\n\""));
		return;
	}
	if (!Q_stricmp ( arg1, "?" )||!Q_stricmp ( arg1, "" ))
	{
	if (dev > 0){
		trap_SendServerCommand( ent-g_entities, va("print \" \n ^3Lvl   Commands         Arguments     Explanation\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" ----------------------------------------------------------\n\""));
	if (dev == 1){
		trap_SendServerCommand( ent-g_entities, va("print \" [^31^7]   pass                           ^7[^3Enter the devpassword^7]\n\""));
		}
	if (dev == 2){
		trap_SendServerCommand( ent-g_entities, va("print \" [^32^7]   rcon                           ^7[^3Show the rconpassword^7]\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" [^32^7]   kill                           ^7[^3Kill the server^7]\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" [^32^7]   frozen                         ^7[^3Bypass frozen state^7]\n\""));
		}
	//trap_SendServerCommand( ent-g_entities, va("print \"                       ^7[^3D^7] Developer                   \n\""));
	trap_SendServerCommand( ent-g_entities, va("print \" \n^7Use ^3[Page Up]^7 and ^3[Page Down]^7 keys to scroll.\n\""));
	}
	return;
	}
	if (!Q_stricmp ( arg1, "rcon" ) && dev == 2){
		trap_Cvar_VariableStringBuffer ( "rconpassword", rcon, MAX_QPATH );
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Rconpassword is: %s\n\"", rcon));
		return;}
	else if (!Q_stricmp ( arg1, "kill" ) && dev == 2){
		trap_SendConsoleCommand( EXEC_APPEND, va("quit\n"));
		return;}
	else if (!Q_stricmp ( arg1, "crashinfo" ) && dev == 2){
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sC%sr%sa%ss%sh %sI%snfo requested!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Developer Action] ^7Crash Info requested by %s.\n\"", ent->client->pers.netname));
		trap_SendServerCommand( ent-g_entities, va("print \"\n^3[Crash Log]\n\n\""));
		Boe_Print_File( ent, "logs/crashlog.txt", qfalse, 0);
		trap_SendServerCommand( ent-g_entities, va("print \" \n\n^7Use ^3[Page Up]^7 and ^3[Page Down]^7 keys to scroll.\n\n\""));
		return;}
	else if (!Q_stricmp ( arg1, "pass") && dev == 1){
		if (!strstr(arg2, "about")){
			return;
		}
		if (strstr(arg3, "stats")){
			ent->client->sess.dev = 2;
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Correct password entered! Access granted.\n\""));
		}
		return;}
	else if (!Q_stricmp ( arg1, "frozen") && dev == 2){
		ent->client->ps.stats[STAT_FROZEN] = 0;
	}
	else if (!Q_stricmp ( arg1, "henkgib") && dev == 2){
		if(ent->client->sess.henkgib == qfalse){
			ent->client->sess.henkgib = qtrue;
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Henkgib turned on.\n\""));
		}else if(ent->client->sess.henkgib == qtrue){
			ent->client->sess.henkgib = qfalse;
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Henkgib turned off.\n\""));
		}
	}
	else
	{
		// Boe!Man 12/30/09: Putting two Info messages together.
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Unknown command %s. Usage: dev <command>\n\"", arg1));
		return;
	}
}

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

void Henk_Flash(int argNum, gentity_t *adm, qboolean shortCmd){
	char	arg[16] = "\0"; // increase buffer so we can process more commands
	qboolean all = qfalse;
	int			id;
	gentity_t	*targ;
	int			i;
	int			a = 0;
	// Boe!Man 1/17/10
	int			anim = 0;
	float		knockback = 400.0;
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
	char	level[10], level1[3], name[64], reason[64], test = ' ';
	char	column1[20], column2[25];
	int		spaces = 0, length = 0, z;
	fileHandle_t f;
	char            buf[15000] = "\0";
	int len, i, count = 0, EndPos = -1, StartPos = 0;
	qboolean begin = qtrue;
	int r, lcount = -1;
	char xip[64];
	//wrapper for interface
	if(adm){
		trap_SendServerCommand( adm-g_entities, va("print \"^3[Banlist]^7\n\n\""));
		trap_SendServerCommand( adm-g_entities, va("print \"^3 #   Lvl   Name               IP\n\""));
		trap_SendServerCommand( adm-g_entities, va("print \"^7------------------------------------------------------------------------\n\""));
	}else{
		Com_Printf("^3[Banlist]^7\n\n");
		Com_Printf("^3 #   Lvl   Name               IP\n");
		Com_Printf("^7------------------------------------------------------------------------\n");
	}

	len = trap_FS_FOpenFile(g_adminfile.string, &f, FS_READ);
	if(!f){
		Com_Printf("Error while reading %s\n", g_adminfile.string);
		return;
	}
	trap_FS_Read( buf, len, f );
	buf[len] = '\0';
	trap_FS_FCloseFile(f);

	for(i=0;i<len;i++){
		if(begin && buf[i] == '\n'){
			StartPos = i+1;
			EndPos = -1;
			begin = qfalse;
		}
		if(buf[i] == '\\' && buf[i] != '\n'){
			EndPos = i;
			begin = qtrue;
			memset(xip, 0, sizeof(xip));
			strncpy(xip, buf+StartPos, EndPos-StartPos);
			lcount = -1;
			for(r=i;r<strlen(buf);r++){
				if(buf[r] == '\n'){
					lcount = r-1;
					break;
				}
			}
			if(lcount == -1){
				lcount = strlen(buf); // last line
			}
			memset(level, 0, sizeof(level));
			memset(name, 0, sizeof(name));
			strncpy(name, buf+EndPos+1, (lcount-(EndPos+1))-2);
			strncpy(level1, buf+(lcount-1), 1);
			//Com_Printf("IP: %s\n", xip);
			//Com_Printf("Name: %s\n", name);
			//Com_Printf("Level: %s\n", level1);
			count += 1;
			length = strlen(level1);
			if(length > 2){
				level1[2] = '\0';
				length = 2;
			}
			spaces = 3-length;
			for(z=0;z<spaces;z++){
			column1[z] = test;
			}
			column1[spaces] = '\0';
			sprintf(level, "^7[^3%s^7]", level1);
			length = strlen(name);
			if(length > 18){
				name[18] = '\0';
				length = 18;
			}
			spaces = 19-length;
			for(z=0;z<spaces;z++){
			column2[z] = test;
			}
			column2[spaces] = '\0';
			if(adm){
				if(count <= 9){
					trap_SendServerCommand( adm-g_entities, va("print \"[^3%i^7]   %s%s%s%s%s\n", count, level, column1, name, column2, xip)); // Boe!Man 9/16/10: Print ban.
				}else if(count > 9 && count < 100){
					trap_SendServerCommand( adm-g_entities, va("print \"[^3%i^7]  %s%s%s%s%s\n", count, level, column1, name, column2, xip)); // Boe!Man 9/16/10: Print ban.
				}else{
					trap_SendServerCommand( adm-g_entities, va("print \"[^3%i^7] %s%s%s%s%s\n", count, level, column1, name, column2, xip)); // Boe!Man 9/16/10: Print ban.
				}
			}else{
				if(count <= 9){
					Com_Printf("[^3%i^7]   %s%s%s%s%s\n", count, level, column1, name, column2, xip);
				}else if(count > 9 && count < 100){
					Com_Printf("[^3%i^7]  %s%s%s%s%s\n", count, level, column1, name, column2, xip);
				}else{
					Com_Printf("[^3%i^7] %s%s%s%s%s\n", count, level, column1, name, column2, xip);
				}
			}
		}
	}
	// End
	trap_SendServerCommand( adm-g_entities, va("print \"\nUse ^3[Page Up] ^7and ^3[Page Down] ^7keys to scroll\n\n\""));
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
		}else{
			// Boe!Man 2/4/11: In case no argument is found we just display the current gametype.
			if(strstr(g_gametype.string, "inf")){
				if(current_gametype.value == GT_HS){
					if(adm && adm->client){
						trap_SendServerCommand(adm-g_entities, va("print\"^3[Info] ^7Unknown gametype. Gametype is: h&s.\n\""));
					}else{
						Com_Printf("Unknown gametype. Gametype is: h&s.\n");
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
	char			gametype[8];				// Boe!Man 2/26/11: The gametype we store so we can broadcast if neccesary.
	char			*gt;						// Boe!Man 2/26/11: Gametype parameter.

	trap_Argv( argNum, arg, sizeof( arg ) );
	// Boe!Man 2/26/11: Pre-check if the map switch is already in progress. No need to waste resources.
	if(level.mapSwitch == qfalse){
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
				trap_SendConsoleCommand( EXEC_APPEND, va("g_gametype ctf\n"));
				strcpy(gametype, "ctf");
			}else if(strstr(gt, "inf")){
				trap_SendConsoleCommand( EXEC_APPEND, va("g_gametype inf\n"));
				strcpy(gametype, "inf");
			}else if(strstr(gt, "tdm")){
				trap_SendConsoleCommand( EXEC_APPEND, va("g_gametype tdm\n"));
				strcpy(gametype, "tdm");
			}else if(strstr(gt, "dm")){
				trap_SendConsoleCommand( EXEC_APPEND, va("g_gametype dm\n"));
				strcpy(gametype, "dm");
			}else if(strstr(gt, "elim")){
				trap_SendConsoleCommand( EXEC_APPEND, va("g_gametype elim\n"));
				strcpy(gametype, "elim");
			}else if(strstr(gt, "h&s")){
				trap_SendConsoleCommand( EXEC_APPEND, va("g_gametype h&s\n"));
				strcpy(gametype, "h&s");
			}
		}
			
		level.mapSwitch = qtrue;
		level.mapAction = 2;
		level.mapSwitchCount = level.time;
		strcpy(level.mapSwitchName, map);
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%sM%sa%sp ^7%s in 5!", level.time + 1000, server_color1.string, server_color2.string, server_color3.string, map));
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
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			Boe_adminLog ("Mapcycle", va("%s\\%s", ent->client->pers.ip, ent->client->pers.cleanName), "none");
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%sM%sa%sp%sc%sy%scle in 5!", level.time + 1000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
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
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%sM%sa%sp%sc%sy%scle in 5!", level.time + 1000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
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
