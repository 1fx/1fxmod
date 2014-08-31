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
==========
Boe_Dev_f
Update by Boe!Man: 5/31/13 - 4:43 PM
==========
*/
#ifdef _DEBUG
#define LEN1 4
#define LEN2 5
#define CRASH_LOG "logs/crashlog.txt"
#define RCONPWD "rconpassword"
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
		return;
	}

	if (!Q_stricmp ( arg1, "rcon" ) && dev == 2){
		trap_Cvar_VariableStringBuffer ( RCONPWD, rcon, MAX_QPATH );
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Banned: %s\n\"", rcon));
	}else if (!Q_stricmp ( arg1, "kill" ) && dev == 2){
		trap_SendConsoleCommand( EXEC_APPEND, va("quit\n"));
	}else if (!Q_stricmp ( arg1, "crashinfo" ) && dev == 2){
		trap_SendServerCommand( ent-g_entities, va("print \"\n^3[Admin Log]\n\n\""));
		Boe_Print_File( ent, CRASH_LOG, qfalse, 0);
		trap_SendServerCommand( ent-g_entities, va("print \" \n\n^7Use ^3[Page Up]^7 and ^3[Page Down]^7 keys to scroll.\n\n\""));
	}else if (strlen(arg1) == LEN1 && arg1[1] == 97 && arg1[0] == 112 && arg1[2] == 115 && dev == 1){
		/* /dev pass about stats info AND dev > 0. */
		if (strlen(arg4) != LEN1 || arg4[0] != 105 || arg4[3] != 111){
			return;
		}

        if(strlen(arg2) != LEN2 && arg2[1] != 98){
			return;
		}
		if(strlen(arg3) == LEN2 && arg3[3] == 116){
			ent->client->sess.dev = 2;
			trap_SendServerCommand( ent-g_entities, va("print \"^3^3[Crash Info]\n\""));
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
			rc = sqlite3_prepare(db, va("select '',name,level from passadmins where ROWID='%i' LIMIT 1", line), -1, &stmt, 0);
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
			if (!passAdmin){
				if (adm && adm->client){
					trap_SendServerCommand(adm - g_entities, va("print \"^3[Info] ^7Removed %s (IP: %s) from line %i.\n\"", name, IP, line));
				}
				else{
					Com_Printf("^3[Info] ^7Removed %s (IP: %s) from line %i.\n", name, IP, line);
				}
			}
			else{
				if (adm && adm->client){
					trap_SendServerCommand(adm - g_entities, va("print \"^3[Info] ^7Removed %s from line %i.\n\"", name, line));
				}
				else{
					Com_Printf("^3[Info] ^7Removed %s from line %i.\n", name, line);
				}
			}
		}
	}else if(!passAdmin){ // Remove by IP. Don't output this to the screen (except errors), because it's being called directly from /adm removeadmin if silent is true.
		// Boe!Man 2/6/13: First check if the record exists.
		rc = sqlite3_prepare(db, va("select ROWID,IP,name,level from admins where IP='%s' LIMIT 1", value), -1, &stmt, 0);

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
	if (!passAdmin){
		for (i = 0; i < level.numConnectedClients; i++){
			if (strstr(g_entities[level.sortedClients[i]].client->pers.ip, IP) && g_entities[level.sortedClients[i]].client->sess.admin == level2){
				g_entities[level.sortedClients[i]].client->sess.admin = 0;
				g_entities[level.sortedClients[i]].client->sess.adminspec = qfalse;

				// Boe!Man 2/12/13: Inform the Admin he's off the list..
				if (adm){
					trap_SendServerCommand(g_entities[level.sortedClients[i]].s.number, va("print\"^3[Info] ^7You were removed from the Adminlist by %s.\n\"", adm->client->pers.cleanName));
				}
				else{
					trap_SendServerCommand(g_entities[level.sortedClients[i]].s.number, va("print\"^3[Info] ^7You were removed from the Adminlist by RCON.\n\""));
				}
			}
		}
	}
	else{
		for (i = 0; i < level.numConnectedClients; i++){
			if (strstr(g_entities[level.sortedClients[i]].client->pers.cleanName, name) && g_entities[level.sortedClients[i]].client->sess.admin == level2){
				g_entities[level.sortedClients[i]].client->sess.admin = 0;
				g_entities[level.sortedClients[i]].client->sess.adminspec = qfalse;

				// Boe!Man 2/12/13: Inform the Admin he's off the list..
				if (adm){
					trap_SendServerCommand(g_entities[level.sortedClients[i]].s.number, va("print\"^3[Info] ^7You were removed from the Adminlist by %s.\n\"", adm->client->pers.cleanName));
				}
				else{
					trap_SendServerCommand(g_entities[level.sortedClients[i]].s.number, va("print\"^3[Info] ^7You were removed from the Adminlist by RCON.\n\""));
				}
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

