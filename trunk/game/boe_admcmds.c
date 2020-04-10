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
    char    arg[16] = "\0"; // increase buffer so we can process more commands
    int     num;
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
SetNades
Recode by Boe: 5/22/12 - 9:25 PM
Instead of forcing everything to on/off, check available weapons as well.
==========
*/
qboolean SetNades(char *status){
    int weapon;
    char *available = calloc(level.wpNumWeapons + 1, sizeof(char));
    qboolean one = qfalse; // Boe!Man 9/20/12: If at least ONE nade is enabled, this is qtrue (also return value).

    if(current_gametype.value == GT_HS){
        strncpy(available, hideSeek_availableWeapons.string, (strlen(hideSeek_availableWeapons.string) <= level.wpNumWeapons) ? strlen(hideSeek_availableWeapons.string) : level.wpNumWeapons);
    }else{
        strncpy(available, availableWeapons.string, (strlen(availableWeapons.string) <= level.wpNumWeapons) ? strlen(availableWeapons.string) : level.wpNumWeapons);
    }

    if(strcmp(status, "0") == 0){ // Manage internally, so we check for such strings. 0 means enable, so in H&S we check that CVAR.
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
                for (weapon = level.grenadeMin; weapon <= level.grenadeMax; weapon ++)
                {
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

    // Free allocated memory.
    if(available != NULL){
        free(available);
    }

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
    sqlite3         *db;
    sqlite3_stmt    *stmt;
    int              rc, i;
    char             IP[MAX_IP], *ip2;
    char             name[MAX_NETNAME];
    int              line;

    if(strlen(value) < 6 && strstr(value, ".")){
        G_printInfoMessage(adm, "Invalid IP, usage: adm clanlistremove <IP/Line>.");
        return qfalse;
    }

    // Boe!Man 5/27/13: Open the database.
    db = usersDb;

    if(lineNumber){ // Delete by line/record.
        line = atoi(value);

        if(!line){
            G_printInfoMessage(adm, "Invalid IP, usage: adm clanlistremove <IP/Line>.");
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
            G_printInfoMessage(adm, "Could not find line %d.", line);
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
            G_printInfoMessage(adm, "Removed %s (IP: %s) from line %d.", name, IP, line);
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
            if(!silent){
                G_printInfoMessage(adm, "Could not find IP '%s' in the database.", value);
            }

            sqlite3_finalize(stmt);
            return qfalse;
        }else{ // Boe!Man 2/6/13: Also store info for the info line and/or the Admin log.
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
            G_printInfoMessage(adm, "Removed %s (IP: %s) from line %d.", name, IP, line);
        }
    }

    // Boe!Man 2/12/13: If the Clan Member is found on the server, remove his Clan status as well.
    if(IP[0] && IP[1]){
        for(i = 0; i < level.numConnectedClients; i++){
            ip2 = g_entities[level.sortedClients[i]].client->pers.ip;

            if(strstr(ip2, IP) == ip2 && g_entities[level.sortedClients[i]].client->sess.clanMember){
                g_entities[level.sortedClients[i]].client->sess.clanMember = qfalse;

                // Boe!Man 2/12/13: Inform the Clan Member he's off the list..
                if(adm && adm->client){
                    G_printInfoMessage(&g_entities[level.sortedClients[i]], "You were removed from the clanlist by %s.", adm->client->pers.cleanName);
                }else{
                    G_printInfoMessage(&g_entities[level.sortedClients[i]], "You were removed from the clanlist by RCON.");
                }

                break;
            }
        }
    }

    // Boe!Man 12/20/12: Re-order the ROWIDs by issuing the VACUUM maintenance query.
    sqlite3_exec(db, "VACUUM", NULL, NULL, NULL);

    // Boe!Man 2/6/13: Log the clan removal.
    if(adm && adm->client){
        Boe_adminLog ("remove clan", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", IP, name));
    }else{
        Boe_adminLog ("remove clan", "RCON", va("%s\\%s", IP, name));
    }

    return qtrue;
}

/*
=============
Boe_id
Updated 1/28/11 - 11:35 AM
=============
*/
void Boe_id (int idnum)
{
    char            *ip = g_entities[idnum].client->pers.ip;
    char            *id = g_entities[idnum].client->pers.boe_id;
    char            *ip2;
    char            ip20[MAX_IP];

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

/*
==========
Boe_Dev_f
Update by Boe!Man: 11/12/14 - 10:24 PM
==========
*/
#ifdef _DEV
qboolean Boe_dev_f ( gentity_t *ent )
{
    int i, idnum;
    char    arg1[MAX_STRING_TOKENS];
    char    arg2[MAX_STRING_TOKENS];
    char    arg3[MAX_STRING_TOKENS];
    char    arg4[MAX_STRING_TOKENS];

    trap_Argv(1, arg1, sizeof(arg1));
    trap_Argv(2, arg2, sizeof(arg2));
    trap_Argv(3, arg3, sizeof(arg3));
    trap_Argv(4, arg4, sizeof(arg4));

    if (!Q_stricmp(arg1, "?") || !Q_stricmp(arg1, "") || !Q_stricmp(arg1, "list"))
    {
        // Boe!Man 11/12/14: Print version info as header.
        trap_SendServerCommand(ent - g_entities, va("print \"%s - %s on %s [%s %s]\n\"", MODFULL, __DATE__,
            #ifdef _WIN32
            "_WIN32",
            #elif __linux__
            "__linux__",
            #endif
            #ifdef __GNUC__
            "GCC",
            #ifdef __GNUC_PATCHLEVEL__
            va("%i.%i.%i", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__)
            #else
            va("%i.%i", __GNUC__, __GNUC_MINOR__)
            #endif // _WIN32
            #else
            "MSVC",
            va("%i", _MSC_VER)
            #endif // __GNUC__
        ));

        trap_SendServerCommand(ent - g_entities, "print \"^1\nCommand     Args      Chts Expl\n\"");
        trap_SendServerCommand(ent - g_entities, "print \"----------------------------------------------------------\n\"");

        // List public dev commands.
        trap_SendServerCommand(ent - g_entities, va("print \"%-11s %-9s %-4s ^7[^1Shows players w/ debug info^7]\n\"", "players", "none", "No"));
        trap_SendServerCommand(ent - g_entities, va("print \"%-11s %-9s %-4s ^7[^1Name of player in hex format^7]\n\"", "namehex", "id/name", "No"));
        trap_SendServerCommand(ent - g_entities, va("print \"%-11s %-9s %-4s ^7[^1Spawn empty model on specified loc^7]\n\"", "showloc", "x y z", "Yes"));
        #ifndef _DEMO
        trap_SendServerCommand(ent - g_entities, va("print \"%-11s %-9s %-4s ^7[^1Spawn box on specified loc^7]\n\"", "bsp", "x y z", "Yes"));
        #endif // not _DEMO
        trap_SendServerCommand(ent - g_entities, va("print \"%-11s %-9s %-4s ^7[^1View sound cache^7]\n\"", "soundcache", "none", "No"));

        trap_SendServerCommand(ent - g_entities, "print \"\n^1[Dev] ^7/condump filename.txt to create a report after any command\n\n\"");
        return qtrue;
    }

    if (Q_stricmp(arg1, "players") == 0){
        // List player counts.
        trap_SendServerCommand(ent - g_entities, va("print \"^4[blue] ^7teamcount %i teamcount1 %i teamcountalive %i\n\"", TeamCount(-1, TEAM_BLUE, NULL), TeamCount1(TEAM_BLUE), TeamCountAlive(TEAM_BLUE)));
        trap_SendServerCommand(ent - g_entities, va("print \"^1[red] ^7teamcount %i teamcount1 %i teamcountalive %i\n\"", TeamCount(-1, TEAM_RED, NULL), TeamCount1(TEAM_RED), TeamCountAlive(TEAM_RED)));
        trap_SendServerCommand(ent - g_entities, va("print \"^3[spec] ^7teamcount %i teamcount1 %i teamcountalive %i\n\"", TeamCount(-1, TEAM_SPECTATOR, NULL), TeamCount1(TEAM_SPECTATOR), TeamCountAlive(TEAM_SPECTATOR)));
        trap_SendServerCommand(ent - g_entities, "print \"^1\n\nid pers.con sess.team dead ghost spec\n\"");
        trap_SendServerCommand(ent - g_entities, "print \"----------------------------------------------------------\n\"");

        // List player info (detailed).
        for (i = 0; i < 64; i++)
        {
            if (level.clients[i].pers.connected != CON_CONNECTED)
            {
                continue;
            }

            trap_SendServerCommand(ent - g_entities, va("print \"%-4i %-7s ^1%-8i ^7%-4s ^7%-5s ^7%s\n\"",
                i,
                "2",
                level.clients[i].sess.team,
                (G_IsClientDead(&level.clients[i]) ? "T": "F"),
                (level.clients[i].sess.ghost ? "T" : "F"),
                (G_IsClientSpectating(&level.clients[i]) ? "T" : "F")));
        }

        trap_SendServerCommand(ent - g_entities, "print \"\n^1[Dev] ^7/condump filename.txt to create a report\n\n\"");
        return qtrue;
    }else if (Q_stricmp(arg1, "namehex") == 0){
        char userinfo[MAX_INFO_STRING];
        char *nameKey;

        // Print name as ASCII values.
        idnum = G_clientNumFromArg(ent, 2, "do this to", qfalse, qtrue, qtrue, qfalse);
        if (idnum < 0)
            return qtrue;

        trap_SendServerCommand(ent - g_entities, va("print \"^1[Dev] ^7ASCII name for player %s (%s)^7:\n", level.clients[idnum].pers.cleanName, level.clients[idnum].pers.netname));
        trap_SendServerCommand(ent - g_entities, "print \"clean:\n\"");
        for (i = 0; i < MAX_NETNAME; i++){
            trap_SendServerCommand(ent - g_entities, va("print \"%u \"", level.clients[idnum].pers.cleanName[i]));
        }
        trap_SendServerCommand(ent - g_entities, "print \"\nnet:\n\"");
        for (i = 0; i < MAX_NETNAME; i++){
            trap_SendServerCommand(ent - g_entities, va("print \"%u \"", level.clients[idnum].pers.netname[i]));
        }

        trap_GetUserinfo(g_entities[idnum].s.number, userinfo, sizeof(userinfo));
        nameKey = Info_ValueForKey (userinfo, "name");
        trap_SendServerCommand(ent - g_entities, va("print \"\n\nUserinfo: %s\nDec: \"", nameKey));
        for (i = 0; i < strlen(nameKey); i++){
            trap_SendServerCommand(ent - g_entities, va("print \"%u \"", nameKey[i]));
        }

        trap_SendServerCommand(ent - g_entities, "print \"\n\n^1[Dev] ^7/condump filename.txt to create a report\n\n\"");
        return qtrue;
    }else if (Q_stricmp(arg1, "showloc") == 0){
        char origin[64];

        // Don't allow this command without cheats.
        if (!CheatsOk(ent)) {
            return qtrue;
        }

        AddSpawnField("classname", "model_static");
        AddSpawnField("model", "a");

        // Default to r.currentOrigin with no or missing params.
        if (!strlen(arg2) || !strlen(arg3) || !strlen(arg4)){
            trap_SendServerCommand(ent - g_entities, va("print \"^1[Dev] ^7Arg(s) missing%s defaulting to r.currentOrigin.\n\"", (!strlen(arg2) ? ",": " (accidently?),")));

            Q_strncpyz(origin, va("%2f %2f %2f", ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2]), sizeof(origin));
        }else{
            Q_strncpyz(origin, va("%s %s %s", arg2, arg3, arg4), sizeof(origin));
        }

        trap_SendServerCommand(ent - g_entities, va("print \"^1[Dev] ^7Spawning empty model on %s.\n\"", origin));
        AddSpawnField("origin", origin);
        G_SpawnGEntityFromSpawnVars(qfalse);

        return qtrue;
    #ifndef _DEMO
    }else if (Q_stricmp(arg1, "bsp") == 0){
        // Don't allow this command without cheats.
        if (!CheatsOk(ent)) {
            return qtrue;
        }

        if (!ent->client->noclip){
            trap_SendServerCommand(ent - g_entities, "print \"^1[Dev] ^7Turned on noclip (you'd be stuck otherwise).\n\"");
            ent->client->noclip = qtrue;
        }

        AddSpawnField("classname", "misc_bsp");
        AddSpawnField("bspmodel", "instances/Colombia/npc_jump1");

        // Default to r.currentOrigin with no or missing params.
        if (!strlen(arg2) || !strlen(arg3) || !strlen(arg4)){
            trap_SendServerCommand(ent - g_entities, va("print \"^1[Dev] ^7Arg(s) missing%s defaulting to r.currentOrigin.\n\"", (!strlen(arg2) ? "," : " (accidently?),")));

            AddSpawnField("origin", va("%2f %2f %2f", ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2]));
        }else{
            AddSpawnField("origin", va("%s %s %s", arg2, arg3, arg4));
        }

        trap_SendServerCommand(ent - g_entities, va("print \"^1[Dev] ^7Spawning your BSP model.\n\""));
        AddSpawnField("angles", "0 90 0");
        AddSpawnField("model", "trigger_hurt"); //blocked_trigger
        AddSpawnField("count", "1");
        G_SpawnGEntityFromSpawnVars(qtrue);

        return qtrue;
    #endif // not _DEMO
    }else if (Q_stricmp(arg1, "soundcache") == 0){
        G_ViewSoundCache(ent);
        return qtrue;
    }

    trap_SendServerCommand(ent - g_entities, "print \"^1[Dev] ^7Unknown cmd.\n\n\"");
    return qfalse;
}
#endif // _DEV

/*
=====================
Boe_removeAdminFromDb
2/6/13 - 6:23 PM
=====================
*/

qboolean Boe_removeAdminFromDb(gentity_t *adm, const char *value, qboolean passAdmin, qboolean lineNumber, qboolean silent)
{
    sqlite3         *db;
    sqlite3_stmt    *stmt;
    int              rc, i;
    char             IP[MAX_IP];
    char             name[MAX_NETNAME];
    int              line;
    int              level2;

    if((strlen(value) < 6 && strstr(value, ".")) && !silent){
        G_printInfoMessage(adm, "Invalid IP, usage: adm adminremove <IP/Line>.");
        return qfalse;
    }

    // Boe!Man 5/27/13: Open the database.
    db = usersDb;

    if(lineNumber){ // Delete by line/record.
        line = atoi(value);

        if(!line){
            G_printInfoMessage(adm, "Invalid IP, usage: adm adminremove <IP/Line>.");
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
            G_printInfoMessage(adm, "Could not find line %d.", line);

            sqlite3_finalize(stmt);
            return qfalse;
        }else{
            Q_strncpyz(IP, (char *)sqlite3_column_text(stmt, 0), sizeof(IP));
            Q_strncpyz(name, (char *)sqlite3_column_text(stmt, 1), sizeof(name));
            level2 = sqlite3_column_int(stmt, 2);
            sqlite3_finalize(stmt);
        }

        // Boe!Man 11/24/15: User must have enough privileges to remove this Admin.
        if(adm && adm->client && !silent){
            // Boe!Man 11/24/15: User must have enough privileges to remove this Admin.
            if((level2 == 2 && adm->client->sess.admin < g_badmin.integer)
            || (level2 == 3 && adm->client->sess.admin < g_admin.integer)
            || (level2 == 4 && adm->client->sess.admin < g_sadmin.integer)){
                G_printInfoMessage(adm, "Your Admin level is too low to remove this Admin.");
                return qfalse;
            }
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
                G_printInfoMessage(adm, "Removed %s (IP: %s) from line %d.", name, IP, line);
            }
            else{
                G_printInfoMessage(adm, "Removed %s from line %d.", name, line);
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
            if(!silent){
                G_printInfoMessage(adm, "Could not find IP '%s' in the database.", value);
            }

            sqlite3_finalize(stmt);
            return qfalse;
        }else{ // Boe!Man 2/6/13: Also store info for the info line and/or the Admin log.
            line = sqlite3_column_int(stmt, 0);
            Q_strncpyz(IP, (char *)sqlite3_column_text(stmt, 1), sizeof(IP));
            Q_strncpyz(name, (char *)sqlite3_column_text(stmt, 2), sizeof(name));
            level2 = sqlite3_column_int(stmt, 3);
        }
        sqlite3_finalize(stmt);

        // Boe!Man 11/24/15: User must have enough privileges to remove this Admin.
        if(adm && adm->client && !silent){
            // Boe!Man 11/24/15: User must have enough privileges to remove this Admin.
            if((level2 == 2 && adm->client->sess.admin < g_badmin.integer)
            || (level2 == 3 && adm->client->sess.admin < g_admin.integer)
            || (level2 == 4 && adm->client->sess.admin < g_sadmin.integer)){
                G_printInfoMessage(adm, "Your Admin level is too low to remove this Admin.");
                return qfalse;
            }
        }

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
            G_printInfoMessage(adm, "Removed %s (IP: %s) from line %d.", name, IP, line);
        }
    }

    // Boe!Man 2/12/13: If the Admin is found on the server, remove his Admin as well.
    if (!passAdmin){
        for (i = 0; i < level.numConnectedClients; i++){
            if (strstr(g_entities[level.sortedClients[i]].client->pers.ip, IP) && g_entities[level.sortedClients[i]].client->sess.admin == level2){
                g_entities[level.sortedClients[i]].client->sess.admin = 0;
                g_entities[level.sortedClients[i]].client->sess.adminspec = qfalse;

                // Boe!Man 2/12/13: Inform the Admin he's off the list..
                if(!silent){
                    if(adm && adm->client){
                        G_printInfoMessage(&g_entities[level.sortedClients[i]], "You were removed from the Adminlist by %s.", adm->client->pers.cleanName);
                    }else{
                        G_printInfoMessage(&g_entities[level.sortedClients[i]], "You were removed from the Adminlist by RCON.");
                    }
                }
            }
        }
    }else{
        for (i = 0; i < level.numConnectedClients; i++){
            if (strstr(g_entities[level.sortedClients[i]].client->pers.cleanName, name) && g_entities[level.sortedClients[i]].client->sess.admin == level2){
                g_entities[level.sortedClients[i]].client->sess.admin = 0;
                g_entities[level.sortedClients[i]].client->sess.adminspec = qfalse;

                // Boe!Man 2/12/13: Inform the Admin he's off the list..
                if(!silent){
                    if(adm && adm->client){
                        G_printInfoMessage(&g_entities[level.sortedClients[i]], "You were removed from the Adminlist by %s.", adm->client->pers.cleanName);
                    }else{
                        G_printInfoMessage(&g_entities[level.sortedClients[i]], "You were removed from the Adminlist by RCON.");
                    }
                }
            }
        }
    }

    // Boe!Man 12/20/12: Re-order the ROWIDs by issuing the VACUUM maintenance query.
    sqlite3_exec(db, "VACUUM", NULL, NULL, NULL);

    // Boe!Man 2/6/13: Log the admin removal.
    if(adm && adm->client){
        Boe_adminLog ("remove admin", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", IP, name));
    }else{
        Boe_adminLog ("remove admin", "RCON", va("%s\\%s", IP, name));
    }

    return qtrue;
}

