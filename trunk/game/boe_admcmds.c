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
		if(num == 0){ // Get second argument because they use it from the console
			trap_Argv( 2, arg, sizeof( arg ) );
			num = atoi(arg);
		}
	return num;
}

/*
==========
Boe_Mute
==========
*/

void Boe_UnMute(int argNum, gentity_t *ent, qboolean shortCmd){
	Boe_Mute (argNum, ent, qfalse, shortCmd);
}

void Boe_XMute(int argNum, gentity_t *ent, qboolean shortCmd){
	Boe_Mute (argNum, ent, qtrue, shortCmd);
}

/*
==========
Boe_Compmode
==========
*/

void Boe_CompMode(int argNum, gentity_t *ent, qboolean shotCmd){
	if(g_compMode.integer == 0){
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sC%so%sm%sp%se%stition mode enabled!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Competition mode enabled by %s.\n\"", ent->client->pers.netname));
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		trap_Cvar_Set("g_compMode", "1");
		//Boe_adminLog (va("%s - TIMELIMIT %i", ent->client->pers.cleanName, number)) ;
	}else{
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sC%so%sm%sp%se%stition mode disabled!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Competition mode disabled by %s.\n\"", ent->client->pers.netname));
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		trap_Cvar_Set("g_compMode", "0");
	}
	trap_Cvar_Update(&g_compMode);
}

/*
==========
Boe_Third
==========
*/

void Boe_Third(int argNum, gentity_t *ent, qboolean shortCmd){
	if(g_allowthirdperson.integer == 0){
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sT%sh%si%sr%sd%sperson enabled!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Thirdperson enabled by %s.\n\"", ent->client->pers.netname));
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		trap_Cvar_Set("g_allowthirdperson", "1");
		//Boe_adminLog (va("%s - TIMELIMIT %i", ent->client->pers.cleanName, number)) ;
	}else{
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sT%sh%si%sr%sd%sperson disabled!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7Thirdperson disabled by %s.\n\"", ent->client->pers.netname));
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		trap_Cvar_Set("g_allowthirdperson", "0");
	}
}

/*
==========
Boe_GametypeRestart
==========
*/

void Boe_GametypeRestart(int argNum, gentity_t *ent, qboolean shortCmd){
	Boe_GlobalSound (G_SoundIndex("sound/misc/menus/invalid.wav"));
	trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Gametype restart by %s.\n\"", ent->client->pers.netname));
	Boe_adminLog (va("%s - GAMETYPE RESTART", ent->client->pers.cleanName)) ;
	trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sG%sa%sm%se%st%sype restart!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
	trap_SendConsoleCommand( EXEC_APPEND, va("gametype_restart\n"));
}

/*
==========
Boe_Normaldamage
==========
*/

void Boe_NormalDamage(int argNum, gentity_t *ent, qboolean shortCmd){
	int i, ammoindex;
	g_instagib.integer = 0;
	BG_InitWeaponStats();
	for(i=0;i<=level.numConnectedClients;i++){
		level.clients[level.sortedClients[i]].noOutfittingChange = qfalse;
		G_UpdateOutfitting(g_entities[level.sortedClients[i]].s.number);
		ammoindex = weaponData[WP_KNIFE].attack[ATTACK_ALTERNATE].ammoIndex;
		ammoData[ammoindex].max = 5; 
	}
	trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sN%so%sr%sm%sa%sl damage!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
	trap_SendServerCommand( ent-g_entities, va("print \"^3[Admin Action] ^7Normal damage by %s.\n\"", ent->client->pers.netname));
}

/*
==========
Boe_Realdamage
==========
*/

void Boe_RealDamage(int argNum, gentity_t *ent, qboolean shortCmd){
	int i;
	g_instagib.integer = 1;
	BG_InitWeaponStats();
	for(i=0;i<=level.numConnectedClients;i++){
		level.clients[level.sortedClients[i]].noOutfittingChange = qfalse;
		G_UpdateOutfitting(g_entities[level.sortedClients[i]].s.number);
	}
	trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sR%se%sa%sl %sd%samage!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
	trap_SendServerCommand( ent-g_entities, va("print \"^3[Admin Action] ^7Real damage by %s.\n\"", ent->client->pers.netname));
}

/*
==========
Boe_RespawnInterval
==========
*/

void Boe_RespawnInterval(int argNum, gentity_t *ent, qboolean shortCmd){
	int number;
	number = GetArgument(argNum);
	if(number < 0)
		return;
	trap_SendConsoleCommand( EXEC_APPEND, va("g_respawninterval %i\n", number));
	trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sR%se%ss%sp%sa%swn interval %i!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, number));
	Boe_adminLog (va("%s - RESPAWN INTERVAL %i", ent->client->pers.cleanName, number)) ;
	trap_SendServerCommand( ent-g_entities, va("print \"^3[Admin Action] ^7Respawn interval changed to %i by %s.\n\"", number, ent->client->pers.netname));
}

/*
==========
Boe_Timelimit
==========
*/

void Boe_TimeLimit(int argNum, gentity_t *ent, qboolean shortCmd){
	int number;
	number = GetArgument(argNum);
	if(number < 0)
		return;
	trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sT%si%sm%se%sl%simit %i!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, number));
	trap_SendServerCommand( ent-g_entities, va("print \"^3[Admin Action] ^7Timelimit changed to %i by %s.\n\"", number, ent->client->pers.netname));
	trap_SendConsoleCommand( EXEC_APPEND, va("timelimit %i\n", number));
	Boe_adminLog (va("%s - TIMELIMIT %i", ent->client->pers.cleanName, number)) ;
}

/*
==========
Boe_ScoreLimit
==========
*/

void Boe_ScoreLimit(int argNum, gentity_t *ent, qboolean shortCmd){
	int number;
	number = GetArgument(argNum);
	if(number < 0)
		return;
	trap_SendConsoleCommand( EXEC_APPEND, va("scorelimit %i\n", number));
	trap_SendServerCommand( ent-g_entities, va("print \"^3[Admin Action] ^7Scorelimit changed to %i by %s.\n\"", number, ent->client->pers.netname));
	trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sS%sc%so%sr%se%slimit %i!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, number));
	Boe_adminLog (va("%s - SCORELIMIT %i", ent->client->pers.cleanName, number)) ;
}

/*
==========
Boe_NoNades
==========
*/

void Boe_NoNades(int argNum, gentity_t *ent, qboolean shortCmd){
	int i;
	if(g_disablenades.integer == 1){
		g_disablenades.integer = 0;
		trap_Cvar_Set("g_disablenades", "0");
		trap_Cvar_Set("g_availableweapons", "200200002200000000200");
		trap_Cvar_Update(&g_availableWeapons);
		trap_Cvar_Update(&g_disablenades);
		BG_SetAvailableOutfitting("200200002200000000200");
		for(i=0;i<=level.numConnectedClients;i++){
			level.clients[level.sortedClients[i]].noOutfittingChange = qfalse;
			G_UpdateOutfitting(g_entities[level.sortedClients[i]].s.number);
			level.clients[level.sortedClients[i]].ps.ammo[weaponData[WP_SMOHG92_GRENADE].attack[ATTACK_NORMAL].ammoIndex]=1;
			level.clients[level.sortedClients[i]].ps.stats[STAT_WEAPONS] |= ( 1 << WP_SMOHG92_GRENADE );
			level.clients[level.sortedClients[i]].ps.clip[ATTACK_NORMAL][WP_SMOHG92_GRENADE]=1;
		}
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sN%sa%sd%se%ss %senabled!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Admin Action] ^7Nades enabled by %s.\n\"", ent->client->pers.netname));
		Boe_adminLog (va("%s - NADES ENABLED", ent->client->pers.cleanName)) ;
	}else{
		g_disablenades.integer = 1;
		trap_Cvar_Set("g_disablenades", "1");
		// change g_available
		trap_Cvar_Set("g_availableweapons", "200200002200000000000");
		trap_Cvar_Update(&g_availableWeapons);
		trap_Cvar_Update(&g_disablenades);
		BG_SetAvailableOutfitting("200200002200000000000");
		for(i=0;i<=level.numConnectedClients;i++){
			level.clients[level.sortedClients[i]].noOutfittingChange = qfalse;
			G_UpdateOutfitting(g_entities[level.sortedClients[i]].s.number);
		}
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sN%sa%sd%se%ss %sdisabled!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Admin Action] ^7Nades disabled by %s.\n\"", ent->client->pers.netname));
		Boe_adminLog (va("%s - NADES DISABLED", ent->client->pers.cleanName)) ;
	}
}

/*
==========
Boe_NoLower
==========
*/

void Boe_NoLower(int argNum, gentity_t *ent, qboolean shortCmd){
	if(level.nolower1 == qtrue){
		level.nolower1 = qfalse;
		trap_Cvar_Set("g_disablelower", "0");
		//trap_Cvar_Update(&g_disablelower);
		if (strstr(level.mapname, "mp_kam2")){
			RemoveFence();
		}
		trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7No lower has been disabled by %s.\n\"", ent->client->pers.netname));
		Boe_adminLog (va("%s - NOLOWER DISABLED", ent->client->pers.cleanName)) ;
	}else{
		level.nolower1 = qtrue;
		trap_Cvar_Set("g_disablelower", "1");
		//trap_Cvar_Update(&g_disablelower);
		if (strstr(level.mapname, "mp_kam2")){
			SpawnFence(1);
			SpawnFence(2);
			SpawnFence(3);
			SpawnFence(4);
		}
		trap_SendServerCommand(-1, va("print \"^3[Admin Action] ^7No lower has been enabled by %s.\n\"", ent->client->pers.netname));
		Boe_adminLog (va("%s - NOLOWER ENABLED", ent->client->pers.cleanName)) ;
	}
}

/*
==========
Boe_MapRestart
==========
*/

void Boe_MapRestart(int argNum, gentity_t *ent, qboolean shortCmd){
	trap_SendConsoleCommand( EXEC_APPEND, va("map_restart 5\n"));
	trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%sM%sa%sp %sr%se%sstart!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
	Boe_GlobalSound (G_SoundIndex("sound/misc/menus/invalid.wav"));
	trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Map restarted by %s.\n\"", ent->client->pers.netname));
	Boe_adminLog (va("%s - MAP RESTART", ent->client->pers.cleanName)) ;
}

/*
==========
Boe_Kick
==========
*/

void Boe_Kick(int argNum, gentity_t *ent, qboolean shortCmd){
	int id;
	char	arg3[MAX_STRING_TOKENS];
	if(shortCmd){
		id = Boe_ClientNumFromArg(ent, 1, "kick <id> <reason>", "Kick", qfalse, qfalse, qtrue);
		if(id < 0) return;
		trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"%s\"\n", id, GetReason()));
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s was %sk%si%sc%sk%se%sd ^7by %s", level.time + 5000, g_entities[id].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, ent->client->pers.netname));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was kicked by %s.\n\"", g_entities[id].client->pers.netname,ent->client->pers.netname));
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		Boe_adminLog (va("%s - KICK: %s", ent->client->pers.cleanName, g_entities[id].client->pers.cleanName  )) ;
	}else{
		id = Boe_ClientNumFromArg(ent, 2, "kick <id> <reason>", "Kick", qfalse, qfalse, qfalse);
		if(id < 0) return;
		trap_Argv( 3, arg3, sizeof( arg3 ) );
		trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"%s\"\n", id, arg3));
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s was %sk%si%sc%sk%se%sd ^7by %s", level.time + 5000, g_entities[id].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, ent->client->pers.netname));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was kicked by %s.\n\"", g_entities[id].client->pers.netname,ent->client->pers.netname));
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

	onlist = Boe_NameListCheck (0, id, g_clanfile.string, NULL, qfalse, qfalse, qfalse, qfalse);

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
		if(adm && adm->client)	{
			if(Boe_AddToList(id, g_clanfile.string, "Clan", NULL)) {
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s is now a %sC%sl%sa%sn %sm%se%smber!", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color4.string, server_color5.string, server_color6.string));
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s is now a Clan member.\n\"", g_entities[idnum].client->pers.netname));
			Boe_adminLog (va("%s - ADD CLAN: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
			}
		}else {
			if(Boe_AddToList(id, g_clanfile.string, "Clan", NULL)) {
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s is now a %sC%sl%sa%sn %sm%se%smber!", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color4.string, server_color5.string, server_color6.string));
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s is now a Clan member.\n\"", g_entities[idnum].client->pers.netname));
			Boe_adminLog (va("%s - ADD CLAN: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
			}
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
			trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s is no longer a Clan member.\n\"", g_entities[idnum].client->pers.netname));
			Boe_adminLog (va("%s - REMOVE CLAN: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
		}else {
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s is no longer a %sC%sl%sa%sn %sm%se%smber!", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color4.string, server_color5.string, server_color6.string));
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s is no longer a Clan member.\n\"", g_entities[idnum].client->pers.netname));
			Boe_adminLog (va("%s - REMOVE CLAN: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
		}
	}
}

qboolean isdigit(char c){ // by henk
	if(c >= 48 && c <= 57){
		return qtrue;
	}else{
		return qfalse;
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
	int i, y;
	char *numb;
	qboolean first = qtrue;
	qboolean space = qfalse;
	trap_Argv( argNum, arg, sizeof( arg ) );
	if(shortCmd){ // Henk 04/05/10 -> Handle the short admin commands.
		num = -1;
		for(i=0;i<16;i++){ // import from Boe_Uppercut -> Bug: id's return when good?
			if(arg[i] == ' '){
				if(isdigit(arg[i+1])){
				num = atoi(va("%c%c", arg[i+1], arg[i+2]));
				}else{
					for(i=0;i<=20;i++){
						if(arg[i] == ' '){
							// Henk 08/09/10 -> Check for another arg in this.
							for(y=1;y<=sizeof(arg);y++){
								if(arg[i+y] == ' '){
									space = qtrue;
									break;
								}else{
									if(first){
										numb = va("%c", arg[i+y]);
										first = qfalse;
									}else{
										numb = va("%s%c", numb, arg[i+y]);
									}
									//Com_Printf("%s\n", numb);
								}
							}
							if(space == qfalse){
							numb = va("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", arg[i+1], arg[i+2], arg[i+3], arg[i+4], arg[i+5], arg[i+6], arg[i+7], arg[i+8], arg[i+9], arg[i+10], arg[i+11], arg[i+12], arg[i+13], arg[i+14], arg[i+15]);
							}
							break;
						}
					}
					for(i=0;i<=level.numConnectedClients;i++){
						//trap_SendServerCommand(-1, va("print\"^3[Debug] ^7%s comparing with %s.\n\"", g_entities[level.sortedClients[i]].client->pers.cleanName,numb));
						if(strstr(Q_strlwr(g_entities[level.sortedClients[i]].client->pers.cleanName), Q_strlwr(numb))){
							num = level.sortedClients[i];
							break;
						}
						num = -1;
					}
				}
				//trap_SendServerCommand( -1, va("print \"^3[Debug] ^7Client: %i(%s)\n\"", num, arg));
				break;
			}
		}
		if(num == -1){ // Get second argument because they use it from the console
			trap_Argv( 2, arg, sizeof( arg ) );
			if(strlen(arg) >= 1){
			num = atoi(arg);
			}else{
				num = -1;
			}
		}

	}else{
		if (arg[0] >= '0' && arg[0] <= '9')
		{
			num = atoi( arg );
		}
		else if(ent && ent->client)
		{
			// Boe!Man 1/4/10: Putting two Info messages together.
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Bad client slot: %s. ^7Usage: adm %s\n\"", arg, usage));
			return -1;
		}
		else
		{
			Com_Printf("Bad client slot: %s\n", arg);
			Com_Printf("Usage: rcon %s\n", usage);
			return -1;
		}
	}

	if ( num < 0 || num >= g_maxclients.integer )
	{
		if(ent && ent->client)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Invalid client number %d.\n\"", num));
		}
		else
		{
			Com_Printf("Invalid client number %d.\n", num );
		}
		return -1;
	}

	if ( g_entities[num].client->pers.connected == CON_DISCONNECTED )
	{
		if(ent && ent->client)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7There is no client with the client number %d.\n\"", num));
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
				Com_Printf("You cannot use this command on other Admins.\n");
				return -1;
			}
		}
	}

	if(aliveOnly)
	{
		if ( G_IsClientDead ( g_entities[num].client ) )
		{
			if(ent && ent->client)
			{
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You cannot %s dead Players.\n\"", action));
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
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You cannot %s a Spectator.\n\"", action));
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
	int				count;

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

void Boe_BanList(int argNum, gentity_t *adm, qboolean shortCmd){
	void	*GP2, *group;
	char	ip[64], name[64], reason[64], by[64], test = ' ';
	char	column1[20], column2[20], column3[20];
	int		spaces = 0, length = 0, z;

	GP2 = trap_GP_ParseFile(g_banlist1.string, qtrue, qfalse);
	if (!GP2)
	{
		G_LogPrintf("Error in file: \"%s\" or file not found.\n", g_banlist1.string);
	}
	//wrapper for interface
	trap_SendServerCommand( adm-g_entities, va("print \"^3[Banlist]^7\n\n\""));
	trap_SendServerCommand( adm-g_entities, va("print \"^3IP              Name                Reason             By\n\""));
	trap_SendServerCommand( adm-g_entities, va("print \"^7------------------------------------------------------------------------\n\""));
	group = trap_GPG_GetSubGroups(GP2);
	while(group)
	{
		trap_GPG_FindPairValue(group, "ip", "0", ip);
		trap_GPG_FindPairValue(group, "name", "", name);
		trap_GPG_FindPairValue(group, "reason", "", reason);
		trap_GPG_FindPairValue(group, "by", "", by);
		// FIXED(lul) ME: We need to add a check here if ip exists in banfile, because this banlist will not delete unbanned people
		if(Boe_NameListCheck (adm->s.number, ip, g_banlist.string, NULL, qtrue, qfalse, qfalse, qfalse)){
			// exists so print
			length = strlen(ip);
			spaces = 16-length;
			for(z=0;z<spaces;z++){
			column1[z] = test;
			}
			column1[spaces] = '\0';
			//trap_SendServerCommand( adm-g_entities, va("print \"%s%s", ip, column1)); // Boe!Man 9/16/10: Print tier 1.
			length = strlen(name);
			spaces = 20-length;
			for(z=0;z<spaces;z++){
			column2[z] = test;
			}
			column2[spaces] = '\0';
			//trap_SendServerCommand( adm-g_entities, va("print \"%s%s", name, column2)); // Boe!Man 9/16/10: Print tier 2.
			length = strlen(reason);
			spaces = 19-length;
			for(z=0;z<spaces;z++){
			column3[z] = test;
			}
			column3[spaces] = '\0';
			trap_SendServerCommand( adm-g_entities, va("print \"%s%s%s%s%s%s%s", ip, column1, name, column2, reason, column3, by)); // Boe!Man 9/16/10: Print ban.
			//trap_SendServerCommand( adm-g_entities, va("print \"%s\n", by)); // Boe!Man 9/16/10: Print tier 4.
			//trap_SendServerCommand( adm-g_entities, va("print \"%s\n%s\n%s\n%s\n\"", ip, name, reason, by)); // print result
		}
		group = trap_GPG_GetNext(group); // switch to next group and loop again
	}
	trap_GP_Delete(&GP2);
	trap_SendServerCommand( adm-g_entities, va("print \"\nUse ^3[Page Up] ^7and ^3[Page Down] ^7keys to scroll\n\n\""));
	return;
}

/*
=============
Boe_id
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
	while(i < 6)
	{
		*id++ = *ip++;
		i++;
	}

	*id++ = '\\';
    *id = '\0';
	strcat(id, g_entities[idnum].client->pers.cleanName);
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
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Error opening %s\n\"", file));
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

	idnum = Boe_ClientNumFromArg(adm, argNum, "addbadmin <idnumber>", "do this to", qfalse, qfalse, shortCmd);
	if(idnum < 0) return;

	id = g_entities[idnum].client->pers.boe_id;

	if(Boe_Remove_from_list(id, g_adminfile.string,	 "admin", NULL, qfalse, qtrue, qfalse));

	g_entities[idnum].client->sess.admin = 2;
	Q_strncpyz (id2, id, 64);
	strcat ( id2, ":2" );

	// Boe!Man 1/14/10
	if(adm && adm->client)	{
		if(Boe_AddToList(id2, g_adminfile.string, "Admin", NULL)) {
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s is now a %sB^7-%sA%sd%sm%si%sn", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s is now a B-Admin.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("%s - ADD B-ADMIN: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
		}
	}
	else {
		if(Boe_AddToList(id2, g_adminfile.string, "Admin", NULL)) {
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s is now a %sB^7-%sA%sd%sm%si%sn", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s is now a B-Admin.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("%s - ADD B-ADMIN: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
		}
	}
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

	idnum = Boe_ClientNumFromArg(adm, argNum, "addadmin <idnumber>", "do this to", qfalse, qfalse, shortCmd);
	if(idnum < 0) return;

	id = g_entities[idnum].client->pers.boe_id; 

	if(Boe_Remove_from_list(id, g_adminfile.string, "admin", NULL, qfalse, qtrue, qfalse));

	g_entities[idnum].client->sess.admin = 3;
	Q_strncpyz (id2, id, 64);
	strcat ( id2, ":3" );

	// Boe!Man 1/14/09
	if(adm && adm->client)	{
		if(Boe_AddToList(id2, g_adminfile.string, "Admin", NULL)) {
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s is now an %sA%sd%sm%si%sn", level.time + 5000, g_entities[idnum].client->pers.netname, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s is now an Admin.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("%s - ADD ADMIN: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
		}
	}
	else {
		if(Boe_AddToList(id2, g_adminfile.string, "Admin", NULL)) {
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s is now an %sA%sd%sm%si%sn", level.time + 5000, g_entities[idnum].client->pers.netname, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s is now an Admin.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("%s - ADD ADMIN: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
		}
	}
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

	idnum = Boe_ClientNumFromArg(adm, argNum, "addsadmin <idnumber>", "do this to", qfalse, qfalse, shortCmd);
	if(idnum < 0) return;

	id = g_entities[idnum].client->pers.boe_id; 

	if(Boe_Remove_from_list(id, g_adminfile.string, "admin", NULL, qfalse, qtrue, qfalse));

	g_entities[idnum].client->sess.admin = 4;
	Q_strncpyz (id2, id, 64);
	strcat ( id2, ":4" );

	// Boe!Man 1/14/10
	if(adm && adm->client)	{
		if(Boe_AddToList(id2, g_adminfile.string, "Admin", NULL)) {
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s is now a %sS^7-%sA%sd%sm%si%sn", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s is now a S-Admin.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("%s - ADD S-ADMIN: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
		}
	}
	else {
		if(Boe_AddToList(id2, g_adminfile.string, "Admin", NULL)) {
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s is now a %sS^7-%sA%sd%sm%si%sn", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s is now a S-Admin.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("%s - ADD S-ADMIN: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
		}
	}
}
		/*
		if(adm && adm->client)
			SC_adminLog (va("%s - AddAdmin: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
		else 
			SC_adminLog (va("%s - AddAdmin: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
		*/

/*
==================
Boe_Unban
==================
*/

void Boe_Unban(gentity_t *adm, char *ip, qboolean subnet)
{
	int		count = 0, count2 = 0;

	while (ip[count] != '\0'){
		if(ip[count] == ' '){
			count++;
			continue;
		}

		if(ip[count] == '.'){
			count++;
			count2++;
		}
		if(ip[count] < '0' || ip[count] > '9')	{
			trap_SendServerCommand( adm-g_entities, va("print \"^3%s ^7is an invalid ip address\n\"", ip));
			return;
		}
		count++;
	}

	if(!subnet){
		if(count2 < 3){
			trap_SendServerCommand( adm-g_entities, va("print \"^3%s ^7is an invalid ip address\n\"", ip));
			return;
		}
			if(Boe_Remove_from_list(ip, g_banlist.string, "Ban", adm, qtrue, qfalse, qfalse )){
				trap_SendServerCommand( adm-g_entities, va("print \"^3%s ^7has been Unbanned.\n\"", ip));
				/*
				if(adm && adm->client)
					SC_adminLog (va("%s - UNBAN: %s", adm->client->pers.cleanName, ip  )) ;
				else 
					SC_adminLog (va("%s - UNBAN: %s", "RCON", ip  )) ;
				*/
				return;
			}
	}
	else {
		if(count2 < 1){
			trap_SendServerCommand( adm-g_entities, va("print \"^3%s ^7is an invalid ip address\n\"", ip));
			return;
		}
		if(Boe_Remove_from_list(ip, g_subnetbanlist.string, "SubnetBan", adm, qtrue, qfalse, qfalse )){
			trap_SendServerCommand( adm-g_entities, va("print \"^3%s's Subnet ^7has been Unbanned.\n\"", ip));
			if(adm && adm->client)
				Boe_adminLog (va("%s - SUBNET UNBAN: %s", adm->client->pers.cleanName, ip  )) ;
			else 
				Boe_adminLog (va("%s - SUBNET UNBAN: %s", "RCON", ip  )) ;
			return;
		}
	}
}

/*
==================
Boe_NameListCheck
==================
*/

int Boe_NameListCheck (int num, const char *name, const char *file, gentity_t *ent, qboolean banCheck, qboolean admCheck, qboolean subnetCheck, qboolean scoreCheck)
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
	if(f >= 5){
		Com_Printf("%i of 64 max handles used\n", f);
		G_LogPrintf( "!=!=!=!=!=!=!=!=WARNING=!=!=!=!=!=!=!=!=!=! File handles are not closing properly  [handle count: ( %i )]\n", f );
	}

	if(len > 15000)	{
		len = 15000;
	}
	memset( buf, 0, sizeof(buf) );
	trap_FS_Read( buf, len, f );
	buf[len] = '\0';
	trap_FS_FCloseFile( f );

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
		}
		if (!Q_stricmp(listName, name) && !admCheck && !subnetCheck)	{
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

void Boe_subnetBan (int argNum, gentity_t *adm, qboolean shortCmd)  
{
	gentity_t		*ent;
	char			ip[16];
	char			reason[16] = "\0";
	char			size[4];
	char			info[96];
	int				idnum, s;

	if(adm && adm->client) argNum = 2;
	else argNum = 1;

	idnum = Boe_ClientNumFromArg(adm, argNum, "subnetban <id> <size> <reason>", "subnetban", qfalse, qfalse, shortCmd);
	if(idnum < 0) return;
	ent = g_entities + idnum;

	trap_Argv( argNum + 1, size, sizeof( size ) );
	trap_Argv( argNum + 2, reason, sizeof( reason ) );
	s = atoi(size) + 1;
	
	if (!size[0] || s <= 4 ) s = 8;
	if (s > 16) s = 16;		

	Q_strncpyz(ip, ent->client->pers.ip, s);
	// Boe!Man 1/7/10: No more logging of reason in the ban file.
	strcpy(info, va("%s\\%s", ip, ent->client->pers.cleanName));
	
	if(Boe_AddToList(info, g_subnetbanlist.string, "Subnet ban", adm)){
		if(adm && adm->client)	{
			if(!*reason){
				trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"Subnetbanned by %s\"\n", idnum, adm->client->pers.netname)); }
			else if(reason != ""){
				trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"Subnetbanned by %s for: %s\"\n", idnum, adm->client->pers.netname, reason));}
			else {
				trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"Subnetbanned by %s\"\n", idnum, adm->client->pers.netname));}
			// trap_SendServerCommand( adm-g_entities, va("print \"%s's ^4SUBNET ^7WAS ^1BANNED^7!\n\"", ent->client->pers.netname));
			Boe_adminLog (va("%s - SUBNETBAN: %s", adm->client->pers.cleanName, ent->client->pers.cleanName  )) ;
		}
		else {
			if (!*reason){
				trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"Subnetbanned for: %s\"\n", idnum, reason));
			}
			else {
				trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"Subnetbanned!\"\n", idnum));
			}
			Boe_adminLog (va("%s - SUBNETBAN: %s", "RCON", ent->client->pers.cleanName  )) ;
			// Com_Printf("%s's ^4SUBNET ^7WAS ^1BANNED^7!\n", ent->client->pers.netname);
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
		trap_SendServerCommand(adm-g_entities, va("print\"^3[Info] ^7%s is not an Admin!\n\"", g_entities[idnum].client->pers.netname));
		return;}
	
	// Boe!Man 1/6/10: Fix, succesfully writes the Admin out of the file.
	if(Boe_Remove_from_list(id, g_adminfile.string, "admin", NULL, qfalse, qtrue, qfalse)){
		// Boe: We'll log this later.
		//
		// SC_adminLog (va("%s - RemoveAdmin: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
	}
	g_entities[idnum].client->sess.admin = 0;

	if(adm && adm->client){
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7is no longer an %sA%sd%sm%si%sn", level.time + 5000, g_entities[idnum].client->pers.netname, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was removed as Admin by %s.\n\"", g_entities[idnum].client->pers.netname,adm->client->pers.netname));
		Boe_adminLog (va("%s - REMOVE ADMIN: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
	}
	else{
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7is no longer an %sA%sd%sm%si%sn", level.time + 5000, g_entities[idnum].client->pers.netname, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s ^7was removed as Admin.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("%s - REMOVE ADMIN: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
	}
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

	idnum = Boe_ClientNumFromArg(adm, argNum, "ban <idnumber> <reason>", "ban", qfalse, qfalse, shortCmd);

	if(idnum < 0)
		return;
	
	if(shortCmd)
		strcpy(reason, GetReason());
	else
	trap_Argv( argNum + 1, reason, sizeof( reason ) );

	// Boe!Man 9/7/10: Example of ban.
	if (adm && adm->client){

		trap_FS_FOpenFile( g_banlist1.string, &f, FS_APPEND_TEXT );
		if (!f)
		{
			Com_Printf("^1Error opening File\n");
			return;
		}
		strcpy(string, va("1\n{\nip \"%s\"\nname \"%s\"\nreason \"%s\"\nby \"%s\"\n}\n", g_entities[idnum].client->pers.ip, g_entities[idnum].client->pers.cleanName, reason, adm->client->pers.cleanName));

		trap_FS_Write(string, strlen(string), f);
		trap_FS_Write("\n", 1, f);
		trap_FS_FCloseFile(f);

		Com_sprintf (banid, sizeof(banid), "%s\\%s",
		g_entities[idnum].client->pers.ip,
		g_entities[idnum].client->pers.cleanName);}

	if(Boe_AddToList(banid, g_banlist.string, "Ban", adm)){
		if(adm && adm->client)	{
			if(!*reason){
				trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"Banned by %s\"\n", idnum, adm->client->pers.netname));
				Boe_adminLog (va("%s - BAN: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;}
			else{
				trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"Banned by %s for: %s\"\n", idnum, adm->client->pers.netname, reason));
				trap_SendServerCommand( adm-g_entities, va("print \"%s was banned by %s!\n\"", g_entities[idnum].client->pers.netname, adm->client->pers.netname));
				Boe_adminLog (va("%s - BAN: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
			}
		}
		else{
			if(!*reason){
				trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"Banned!\"\n", idnum));
				Boe_adminLog (va("%s - RUNOVER: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;}
			else{
				trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"Banned for: %s\"\n", idnum, reason));
				Com_Printf("%s was banned!\n", g_entities[idnum].client->pers.netname);
				Boe_adminLog (va("%s - BAN: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
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

	idnum = Boe_ClientNumFromArg(adm, argNum, "uppercut <idnumber>", "uppercut", qtrue, qtrue, shortCmd);
	if(idnum < 0) return;
	
	ent = g_entities + idnum;
	ent->client->ps.pm_flags |= PMF_JUMPING;
	ent->client->ps.groundEntityNum = ENTITYNUM_NONE;
	// Boe!Man 5/3/10: We higher the uppercut.
	if(shortCmd){
	uclevel = atoi(GetReason());
	if(uclevel == 0)
		ent->client->ps.velocity[2] = 1400;
	else
		ent->client->ps.velocity[2] = 300*uclevel;
	}else{
		ent->client->ps.velocity[2] = 1400;
	}
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
	
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
		Boe_adminLog (va("%s - UPPERCUT: %s", adm->client->pers.cleanName, ent->client->pers.cleanName  )) ;
	}
	else {
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,^7%s was %su%sp%sp%se%sr%scut", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s was uppercut.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("%s - UPPERCUT: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
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
	int				idnum, i;
	vec3_t			lookdown;
	//char			*a, *b, *c;
	char			a[4], b[4], c[4];


	idnum = Boe_ClientNumFromArg(adm, argNum, "twist <idnumber>", "twist", qtrue, qtrue, shortCmd);
	if(idnum < 0)  return;

	ent = g_entities + idnum;

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
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,^7%s was %st%sw%si%ss%st%sed by %s", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was twisted by %s.\n\"", g_entities[idnum].client->pers.netname,adm->client->pers.netname));
		Boe_adminLog (va("%s - TWIST: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;}
	else {
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,^7%s was %st%sw%si%ss%st%sed", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s was twisted.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("%s - TWIST: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
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

	VectorSet(lookdown, 0, 0, 0);	
	SetClientViewAngle(ent, lookdown);
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));

	if(adm && adm->client){
		// Boe!Man 6/11/10: Double "Twisted" message bug fixed.
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,^7%s was %su%sn%st%sw%si%ssted by %s", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was untwisted by %s.\n\"", g_entities[idnum].client->pers.netname,adm->client->pers.netname));
		Boe_adminLog (va("%s - UNTWIST: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
	}
	else {
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,^7%s was %su%sn%st%sw%si%ssted", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s was untwisted.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("%s - UNTWIST: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
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
	qboolean		ghost;
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
			//if(ent->client->sess.ghostStartTime)
			//{
			//	ent->client->sess.totalSpectatorTime += level.time - ent->client->sess.ghostStartTime;
			//	ent->client->sess.ghostStartTime = 0;
			//}
	}else{
		TossClientItems(ent);
	}
	ent->client->sess.noTeamChange = qfalse;
	trap_UnlinkEntity (ent);
	ClientSpawn(ent);
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
	Boe_ClientSound(ent, G_SoundIndex("sound/ambience/vehicles/telephone_pole.mp3"));


	if(g_entities[idnum].client->sess.lastIdentityChange){
		status = ".";
		g_entities[idnum].client->sess.lastIdentityChange = qfalse;
	}
	else {
		status = ".";
		g_entities[idnum].client->sess.lastIdentityChange = qtrue;
	}
	
	if(adm && adm->client){
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,^7%s was %sr%se%ss%sp%sa%swned by %s", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was respawned by %s.\n\"", g_entities[idnum].client->pers.netname,adm->client->pers.netname));
		Boe_adminLog (va("%s - RESPAWN: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
	}
	else {
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,^7%s was %sr%se%ss%sp%sa%swned", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s was respawned.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("%s - RESPAWN: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
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
	int		anim = 0;
	
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
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sr%su%sn%so%sv%ser by %s", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was runover by %s.\n\"", g_entities[idnum].client->pers.netname,adm->client->pers.netname));
		Boe_adminLog (va("%s - RUNOVER: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
	}
	else{
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sr%su%sn%so%sv%ser", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s ^7was runover.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("%s - RUNOVER: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
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
		Boe_adminLog (va("%s - FLASH: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
	}
	else{
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sf%sl%sa%ss%sh%sed", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s ^7was flashed.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("%s - FLASH: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
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
	vec3_t dir;
	int it, nadeDir, weapon;
	float x, y;
	gentity_t *missile;

	idnum = Boe_ClientNumFromArg(adm, argNum, "plant <idnumber>", "Plant", qtrue, qtrue, shortCmd);

	if(idnum < 0)
		return;

	ent = g_entities + idnum;

	if(ent->client->pers.planted){
		if(adm && adm->client)
			trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7%s ^7is already planted.\n\"", ent->client->pers.netname));
		else
			Com_Printf("^3%s is already Planted.\n", ent->client->pers.netname);
		return;
	}

	if ( ent->client->ps.pm_flags & PMF_DUCKED )
		ent->client->ps.origin[2] -=40;
	else
		ent->client->ps.origin[2] -= 65;

	VectorCopy( ent->client->ps.origin, ent->s.origin );
	ent->client->pers.planted = qtrue;

	// Boe!Man 1/29/10: All clients will need to hear the event sound, though only the victim will have to hear the wood breaking sound.
	Boe_ClientSound(ent, G_SoundIndex("sound/misc/confused/wood_break.mp3"));
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));

	if(adm && adm->client){
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sp%sl%sa%sn%st%sed by %s", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was planted by %s.\n\"", g_entities[idnum].client->pers.netname,adm->client->pers.netname));
		Boe_adminLog (va("%s - PLANT: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
	}
	else{
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sp%sl%sa%sn%st%sed", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s ^7was planted.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("%s - PLANT: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
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
	vec3_t dir;
	int it, nadeDir, weapon;
	float x, y;
	gentity_t *missile;

	idnum = Boe_ClientNumFromArg(adm, argNum, "unplant <idnumber>", "unplant", qtrue, qtrue, shortCmd);

	if(idnum < 0){return;}
	ent = g_entities + idnum;

	if(!ent->client->pers.planted){
		if(adm && adm->client)
			trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7%s ^7is not planted.\n\"", ent->client->pers.netname));
		 else
			Com_Printf("^3%s is not planted.\n", ent->client->pers.netname);
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
		Boe_adminLog (va("%s - UNPLANT: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
	}
	else{
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %su%sn%sp%sl%sa%snted", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s ^7was unplanted.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("%s - UNPLANT: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
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
		Boe_adminLog (va("%s - POP: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
	}
	else{
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sp%so%sp%sp%se%sd", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s ^7was popped.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("%s - POP: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
	}
	// Boe!Man 1/30/10: We use g_damage since the animations won't work.
	G_Damage (ent, NULL, NULL, NULL, NULL, 10000, 0, MOD_POP, HL_HEAD|HL_FOOT_RT|HL_FOOT_LT|HL_LEG_UPPER_RT|HL_LEG_UPPER_LT|HL_HAND_RT|HL_HAND_LT|HL_WAIST|HL_CHEST|HL_NECK);
}

void Boe_Broadcast(int argNum, gentity_t *adm, qboolean shortCmd){
	char buffer[512];
	char buffer1[512];
	int i;
	if(shortCmd){
		trap_Argv(1, buffer, sizeof(buffer));
		for(i=4;i<=strlen(buffer);i++){
			strcpy(buffer1, va("%s%c", buffer1, buffer[i]));
		}
		buffer[i+1] = '\0';
	}else{
		trap_Argv(2, buffer, sizeof(buffer));
	}

	trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%s", level.time + 5000, buffer1));
	Boe_GlobalSound (G_SoundIndex("sound/misc/menus/invalid.wav"));
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
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sb%su%sr%sn%se%sd by %s", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was burned by %s.\n\"", g_entities[idnum].client->pers.netname,adm->client->pers.netname));
		Boe_adminLog (va("%s - BURN: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
	}
	else{
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sb%su%sr%sn%se%sd", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s ^7was burned.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("%s - BURN: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
	}
}

void Adm_ForceTeam(int argNum, gentity_t *adm, qboolean shortCmd)
{
	char		str[MAX_TOKEN_CHARS];
	int			idnum;

	// find the player
	if(shortCmd == qtrue){
		idnum = Boe_ClientNumFromArg(adm, 1, "forceteam <idnumber>", "forceteam", qfalse, qtrue, shortCmd);
	}else{
		idnum = Boe_ClientNumFromArg(adm, 2, "forceteam <idnumber>", "forceteam", qfalse, qtrue, shortCmd);
	}
	if(idnum < 0) return;

	// set the team
	if(shortCmd == qtrue){
		trap_Argv(1, str, sizeof(str));
		if(strstr(Q_CleanStr(str), "s")){
			strcpy(str, "s");
		}else if(strstr(Q_CleanStr(str), "r")){
			strcpy(str, "r");
		}else if(strstr(Q_CleanStr(str), "b")){
			strcpy(str, "b");
		}else{
			trap_SendServerCommand(adm->s.number, va("print\"^3[Info] Wrong team: %s.\n\"", str));
		}
	}else{
		trap_Argv( 3, str, sizeof( str ) );
	}
	SetTeam( &g_entities[idnum], str, NULL );
}

/*
========
Boe_Mute
========
*/

void Boe_Mute (int argNum, gentity_t *adm, qboolean mute, qboolean shortCmd)
{
	int		idnum;
	char	*status;
	char	*status2;

	idnum = Boe_ClientNumFromArg(adm, argNum, "mute/unmute <idnumber>", "mute/unmute", qfalse, qfalse, shortCmd);

	if(idnum < 0) return;

	if (mute == qtrue){
		if(g_entities[idnum].client->sess.mute == qtrue){
			trap_SendServerCommand(adm-g_entities, va("print \"^3[Info] ^7This client is already muted!\n\""));}
		else{
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			g_entities[idnum].client->sess.mute = qtrue;
			if(adm && adm->client){
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sm%su%st%se%sd by %s", level.time + 5000, g_entities[idnum].client->pers.netname, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
				trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was muted by %s.\n\"", g_entities[idnum].client->pers.netname, adm->client->pers.netname));
				Boe_adminLog (va("%s - MUTE: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
			}
			else{
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sm%su%st%se%sd", level.time + 5000, g_entities[idnum].client->pers.netname, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
				trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s ^7was muted.\n\"", g_entities[idnum].client->pers.netname));
				Boe_adminLog (va("%s - MUTE: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
			}
		}
	}else{
		if(g_entities[idnum].client->sess.mute == qfalse){
			trap_SendServerCommand(adm-g_entities, va("print \"^3[Info] ^7This client is not muted!\n\""));}
		else{
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			g_entities[idnum].client->sess.mute = qfalse;
			if(adm && adm->client){
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %su%sn%sm%su%st%sed by %s", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
				trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was unmuted by %s.\n\"", g_entities[idnum].client->pers.netname, adm->client->pers.netname));
				Boe_adminLog (va("%s - UNMUTE: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
			}
			else{
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %su%sn%sm%su%st%sed", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
				trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s ^7was unmuted.\n\"", g_entities[idnum].client->pers.netname));
				Boe_adminLog (va("%s - UNMUTE: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
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

	if(adm && adm->client){
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %ss%st%sr%si%sp%sped by %s", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was stripped by %s.\n\"", g_entities[idnum].client->pers.netname,adm->client->pers.netname));
		Boe_adminLog (va("%s - STRIP: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
	}
	else{
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %ss%st%sr%si%sp%sped", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s ^7was stripped.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("%s - STRIP: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
	}
}

/*
==========
Boe_Dev_f
==========
*/

void Boe_dev_f ( gentity_t *ent )
{

	int		id, i, dev;
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
		Boe_Print_File( ent, "logs/crashlog.txt");
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
			trap_SendServerCommand( adm - g_entities, va("print \"Not playing a team game.\n\"") );
			} else {
			Com_Printf("Not playing a team game.\n");}
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
	///Tell Everyone what happend
	trap_SendServerCommand( -1, va("cp \"^_**^7Admin Action^_**\n^3Swap Teams\n\"") );
	Boe_GlobalSound(G_SoundIndex("sound/misc/events/tut_lift02.mp3"));
	trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%sS%sw%sa%sp %st%se%sams!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color4.string, server_color5.string, server_color6.string));
	trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Swap teams by %s.\n\"", adm->client->pers.netname));

	if(adm && adm->client) { // Admin Action
		Boe_adminLog (va("%s - SwapTeams", adm->client->pers.cleanName )) ;
	} else{ // Rcon must've done it. ;]
		Boe_adminLog (va("%s - SwapTeams", "RCON" )) ;
	}
	///CalculateRanks();
}
