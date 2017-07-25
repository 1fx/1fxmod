// Copyright (C) 2009-2014 - Boe!Man & Henkie.
//
// g_admcmds.c - All the Admin command functions go here.

#include "g_local.h"
#include "boe_local.h"

// Local function definitions.
static char*adm_checkListFilters    (gentity_t *adm, int argNum, qboolean shortCmd, char *listName, char *byFieldName);
static void adm_addAdmin_f          (int argNum, gentity_t *adm, qboolean shortCmd, int level2, char *commandName);
static void adm_unPlant             (int idNum, gentity_t *adm);
static void adm_toggleSection       (gentity_t *adm, char *sectionName, int sectionID, int useSection);
static void adm_toggleCVAR          (gentity_t *adm, int argNum, qboolean shortCmd, char *cvarName, vmCvar_t *cvar1, qboolean availableInCM, char *cvarNameCM, vmCvar_t *cvar2);
static void adm_Damage              (gentity_t *adm, char *damageName, int value);
static void adm_showBanList         (int argNum, gentity_t *adm, qboolean shortCmd, qboolean subnet);
static void adm_unPause             (gentity_t *adm);
static void adm_unbanFromDatabase   (gentity_t *adm, char *ip, qboolean subnet);

/*
================
adm_Uppercut

Launches a player into the air.
================
*/

int adm_Uppercut(int argNum, gentity_t *adm, qboolean shortCmd)
{
    gentity_t       *ent;
    int             idNum;
    int             ucLevel;
    char            arg[64] = "\0";

    idNum = G_clientNumFromArg(adm, argNum, "uppercut", qtrue, qtrue, qtrue, shortCmd);
    if (idNum < 0) return idNum;

    ent = g_entities + idNum;
    ent->client->ps.pm_flags |= PMF_JUMPING;
    ent->client->ps.groundEntityNum = ENTITYNUM_NONE;

    // Boe!Man 5/3/10: We higher the uppercut.
    if (shortCmd && G_GetChatArgumentCount()){
        ucLevel = atoi(G_GetChatArgument(2));
    }else{
        trap_Argv(argNum + 1, arg, sizeof(arg));
        ucLevel = atoi(arg);
    }

    // Boe!Man 1/13/11: Changing uppercut levels.
    if (ucLevel == 0){
        ent->client->ps.velocity[2] = 1000;
    }else{
        ent->client->ps.velocity[2] = 200 * ucLevel;
    }

    // Boe!Man 11/2/10: Added client sound.
    Boe_ClientSound(ent, G_SoundIndex("sound/weapons/rpg7/fire01.mp3"));

    return idNum;
}

/*
================
adm_Pop

Instantly kills a player.
================
*/

int adm_Pop(int argNum, gentity_t *adm, qboolean shortCmd)
{
    gentity_t       *ent;
    int             idNum;

    idNum = G_clientNumFromArg(adm, argNum, "pop", qtrue, qfalse, qfalse, shortCmd);
    if (idNum < 0) return idNum;
    ent = g_entities + idNum;

    Boe_ClientSound(ent, G_SoundIndex("sound/npc/air1/guard02/laughs.mp3"));

    // Allow to pop players in godmode.
    ent->flags &= ~FL_GODMODE;

    // Do the actual damage.
    G_Damage(ent, NULL, NULL, NULL, NULL, 10000, 0, MOD_POP, HL_HEAD | HL_FOOT_RT | HL_FOOT_LT | HL_LEG_UPPER_RT | HL_LEG_UPPER_LT | HL_HAND_RT | HL_HAND_LT | HL_WAIST | HL_CHEST | HL_NECK);

    return idNum;
}

/*
================
adm_Kick

Kicks a player from the server.
================
*/

int adm_Kick(int argNum, gentity_t *adm, qboolean shortCmd){
    int     idNum;

    // Get the client ID.
    idNum = G_clientNumFromArg(adm, argNum, "kick", qfalse, qfalse, qfalse, shortCmd);
    if (idNum < 0) return idNum;

    // Kick the player.
    G_kickPlayer(&g_entities[idNum], adm, "kicked", GetReason());

    return idNum;
}

/*
================
adm_addAdmin
2/5/13 - 4:03 PM
Function that acts as a wrapper around addbadmin, addadmin and addsadmin.
Fetches the level the user wishes to add and then redirects them.
================
*/

int adm_addAdmin(int argNum, gentity_t *adm, qboolean shortCmd)
{
    char            command[64] = "\0";

    if (adm && adm->client){
        trap_Argv(1, command, sizeof(command));
    }
    else{
        trap_Argv(0, command, sizeof(command));
    }
    Q_strlwr(command); // Boe!Man 2/16/13: Fix capitalized Admin commands resulting in adding S-Admin by converting the command to lower case.

    if (strstr(command, "!ab") || strstr(command, "addbad")){
        adm_addAdmin_f(argNum, adm, shortCmd, 2, "addbadmin");
    }
    else if (strstr(command, "!aa") || strstr(command, "addad")){
        adm_addAdmin_f(argNum, adm, shortCmd, 3, "addadmin");
    }
    else{ // Must be add S-Admin..
        adm_addAdmin_f(argNum, adm, shortCmd, 4, "addsadmin");
    }

    return -1;
}

/*
================
adm_addAdmin_f

Does the actual logic of adding an Admin.
================
*/

static void adm_addAdmin_f(int argNum, gentity_t *adm, qboolean shortCmd, int level2, char *commandName)
{
    int             idNum;
    char            arg[64] = "\0";
    char            clientName[MAX_NETNAME];
    char            admName[MAX_NETNAME];
    char            ip[MAX_IP];
    char            admLevel[12];
    char            admLevelPrefixed[32];
    char            admLog[12];
    qboolean        passAdmin = qfalse;
    sqlite3         *db;

    idNum = G_clientNumFromArg(adm, argNum, "do this to", qfalse, qfalse, qfalse, shortCmd);
    if (idNum < 0) return;

    // Boe!Man 4/27/11: RCON has to be able to do everything. However, there are a few things that should be excluded. Instead of processing which command was entered in the ClientNumFromArg func, we deploy this check in the addxadmin functions (to save resources).
    if (!adm && g_entities[idNum].client->sess.admin > 1){
        Com_Printf("You cannot use this command on other Admins.\n");
        return;
    }

    if (shortCmd && G_GetChatArgumentCount()){
        strcpy(arg, G_GetChatArgument(2));
    }else{
        trap_Argv(argNum + 1, arg, sizeof(arg));
    }

    if (!Q_stricmp(arg, "pass")){
        if (g_passwordAdmins.integer){
            passAdmin = qtrue;
        }else{
            G_printInfoMessage(adm, "Access denied: No password logins allowed by the server!");
            return;
        }
    }else{
        if (!g_preferSubnets.integer){
            Q_strncpyz(ip, g_entities[idNum].client->pers.ip, sizeof(ip));
        }else{
            Q_strncpyz(ip, g_entities[idNum].client->pers.ip, 7);
        }
    }

    // Boe!Man 12/12/12: Check the names, SQLite has massive problems when using quotes in the (updated) query.
    Q_strncpyz(clientName, g_entities[idNum].client->pers.cleanName, sizeof(clientName));
    Boe_convertNonSQLChars(clientName);
    if (adm){
        Q_strncpyz(admName, adm->client->pers.cleanName, sizeof(admName));
        Boe_convertNonSQLChars(admName);
    }else{
        strcpy(admName, "RCON");
    }

    // Boe!Man 8/25/14: Instead of immediately inserting into the database, check if someone added this guy yet.
    if (passAdmin){
        if (Boe_checkPassAdmin2(clientName)){
            G_printInfoMessage(adm, "A client with this name was already added once!");
            G_printInfoMessage(adm, "He can now reset his password.");

            g_entities[idNum].client->sess.setAdminPassword = qtrue;
            return;
        }
    }

    // Boe!Man 2/5/13: Add Admin to the database.
    // Boe!Man 2/5/13: Open database.
    db = usersDb;

    // Boe!Man 2/5/13: Insert query.
    if (!passAdmin){
        if (sqlite3_exec(db, va("INSERT INTO admins (IP, name, by, level) values ('%s', '%s', '%s', '%i')", ip, clientName, admName, level2), 0, 0, 0) != SQLITE_OK){
            if (adm){
                trap_SendServerCommand(adm - g_entities, va("print \"^1[Error] ^7users database: %s\n\"", sqlite3_errmsg(db)));
            }else{
                G_LogPrintf("^1Error: ^7users database: %s\n", sqlite3_errmsg(db));
            }

            return;
        }
    }else{
        if (sqlite3_exec(db, va("INSERT INTO passadmins (name, by, level) values ('%s', '%s', '%i')", clientName, admName, level2), 0, 0, 0) != SQLITE_OK){
            if (adm){
                trap_SendServerCommand(adm - g_entities, va("print \"^1[Error] ^7users database: %s\n\"", sqlite3_errmsg(db)));
            }else{
                G_LogPrintf("^1Error: ^7users database: %s\n", sqlite3_errmsg(db));
            }

            return;
        }
    }

    if (level2 == 2){
        strcpy(admLevel, "B-Admin");
        Q_strncpyz(admLevelPrefixed, server_badminprefix.string, sizeof(admLevelPrefixed));
        Q_strncpyz(admLog, "add b-admin", sizeof(admLog));
    }else if (level2 == 3){
        strcpy(admLevel, "Admin");
        Q_strncpyz(admLevelPrefixed, server_adminprefix.string, sizeof(admLevelPrefixed));
        Q_strncpyz(admLog, "add admin", sizeof(admLog));
    }else{
        strcpy(admLevel, "S-Admin");
        Q_strncpyz(admLevelPrefixed, server_sadminprefix.string, sizeof(admLevelPrefixed));
        Q_strncpyz(admLog, "add s-admin", sizeof(admLog));
    }

    // *DON'T* let the Admin system handle the post processing on this command, because it simply works in a different fashion.
    if (!passAdmin){
        G_Broadcast(va("%s\nis now a %s", g_entities[idNum].client->pers.netname, admLevelPrefixed), BROADCAST_CMD, NULL);
        g_entities[idNum].client->sess.admin = level2;
    }

    Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
    if (adm){
        if (!passAdmin){
            trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was made %s by %s.\n\"", g_entities[idNum].client->pers.cleanName, admLevel, adm->client->pers.cleanName));
        }else{
            trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was added to the %s password list by %s.\n\"", g_entities[idNum].client->pers.cleanName, admLevel, adm->client->pers.cleanName));
        }

        Boe_adminLog(admLog, va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", g_entities[idNum].client->pers.ip, g_entities[idNum].client->pers.cleanName));
    }
    else{
        if (!passAdmin){
            trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s is now a %s.\n\"", g_entities[idNum].client->pers.cleanName, admLevel));
        }else{
            trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s was added to the %s password list.\n\"", g_entities[idNum].client->pers.cleanName, admLevel));
        }

        Boe_adminLog(admLog, "RCON", va("%s\\%s", g_entities[idNum].client->pers.ip, g_entities[idNum].client->pers.cleanName));
    }

    // Boe!Man 2/5/13: Inform a passworded Admin of the system he can now use.
    // Boe!Man 2/16/13: Only inform the Admin via chat. The Admin won't notice if it's being broadcast via console.
    if (passAdmin){
        char *info = G_ColorizeMessage("\\Info:");
        g_entities[idNum].client->sess.setAdminPassword = qtrue;

        trap_SendServerCommand(g_entities[idNum].s.number, va("chat -1 \"%s You need to login every time you enter the server.\n\"", info));
        trap_SendServerCommand(g_entities[idNum].s.number, va("chat -1 \"%s In order to do this, you need to set your own password.\n\"", info));
        trap_SendServerCommand(g_entities[idNum].s.number, va("chat -1 \"%s Do this by executing the following command: /adm pass 'yourpassword'.\n\"", info));
    }

    // Boe!Man 10/16/10: Is the Admin level allowed to spec the opposite team?
    if (!passAdmin && g_adminSpec.integer <= level2 && g_adminSpec.integer != 0 && cm_enabled.integer < 2){
        g_entities[idNum].client->sess.adminspec = qtrue;
    }
}

/*
================
adm_Plant

Plants a player into the ground.
================
*/

int adm_Plant(int argNum, gentity_t *adm, qboolean shortCmd)
{
    gentity_t   *ent;
    int         idNum;

    idNum = G_clientNumFromArg(adm, argNum, "plant", qtrue, qtrue, qtrue, shortCmd);
    if (idNum < 0) return idNum;

    ent = g_entities + idNum;

    // Boe!Man 1/13/11: If the client is already planted, unplant him.
    if (ent->client->pers.planted == qtrue){
        adm_unPlant(idNum, adm);
        return -1;
    }

    if (ent->client->ps.pm_flags & PMF_DUCKED){
        ent->client->ps.origin[2] -= 40;
    }else{
        ent->client->ps.origin[2] -= 65;
    }

    VectorCopy(ent->client->ps.origin, ent->s.origin);
    ent->client->pers.planted = qtrue;

    // Boe!Man 1/29/10: Only the victim will have to hear the wood breaking sound.
    Boe_ClientSound(ent, G_SoundIndex("sound/misc/confused/wood_break.mp3"));

    return idNum;
}

/*
================
adm_unPlant

Resets the earlier planted state.
================
*/

static void adm_unPlant(int idNum, gentity_t *adm)
{
    gentity_t   *ent;

    ent = g_entities + idNum;

    ent->client->ps.origin[2] += 65;
    VectorCopy(ent->client->ps.origin, ent->s.origin);
    ent->client->pers.planted = qfalse;

    Boe_ClientSound(ent, G_SoundIndex("sound/misc/confused/wood_break.mp3"));
    Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));

    if (adm && adm->client){
        G_Broadcast(va("%s\nwas \\unplanted\nby %s", ent->client->pers.netname, adm->client->pers.netname), BROADCAST_CMD, NULL);
        trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was unplanted by %s.\n\"", ent->client->pers.cleanName, adm->client->pers.cleanName));
    }else{
        G_Broadcast(va("%s\nwas \\unplanted", ent->client->pers.netname), BROADCAST_CMD, NULL);
        trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s was unplanted.\n\"", ent->client->pers.cleanName));
    }
}

/*
================
adm_Runover

Knocks a player back.
================
*/

int adm_Runover(int argNum, gentity_t *adm, qboolean shortCmd)
{
    gentity_t   *ent;
    vec3_t      dir;
    vec3_t      fireAngs;
    int         idNum;

    idNum = G_clientNumFromArg(adm, argNum, "runover", qtrue, qtrue, qtrue, shortCmd);
    if (idNum < 0) return idNum;

    ent = g_entities + idNum;

    VectorCopy(ent->client->ps.viewangles, fireAngs);
    AngleVectors(fireAngs, dir, NULL, NULL);
    dir[0] *= -1.0;
    dir[1] *= -1.0;
    dir[2] = 0.0;
    VectorNormalize(dir);

    Boe_ClientSound(ent, G_SoundIndex("sound/ambience/vehicles/hit_scrape.mp3"));
    // Do the actual action.
    ent->client->ps.velocity[2] = 20;
    ent->client->ps.weaponTime = 3000;
    G_Damage(ent, NULL, NULL, NULL, NULL, 15, 0, MOD_CAR, HL_NONE);
    G_ApplyKnockback(ent, dir, 400.0f);

    return idNum;
}

/*
================
adm_Respawn

Respawns a player.
================
*/

int adm_Respawn(int argNum, gentity_t *adm, qboolean shortCmd)
{
    gentity_t       *ent;
    int             idNum;

    idNum = G_clientNumFromArg(adm, argNum, "respawn", qfalse, qtrue, qtrue, shortCmd);
    if (idNum < 0) return idNum;

    ent = g_entities + idNum;

    if (ent->client->sess.team == TEAM_SPECTATOR){
        G_printInfoMessage(adm, "You cannot respawn a spectator.");
        return -1;
    }

    if (ent->client->sess.ghost){
        G_StopFollowing(ent);
        ent->client->ps.pm_flags &= ~PMF_GHOST;
        ent->client->ps.pm_type = PM_NORMAL;
        ent->client->sess.ghost = qfalse;
    }else{
        TossClientItems(ent);
    }

    ent->client->sess.noTeamChange = qfalse;
    trap_UnlinkEntity(ent);
    ClientSpawn(ent);
    Boe_ClientSound(ent, G_SoundIndex("sound/ambience/vehicles/telephone_pole.mp3")); // Let the client know something happened.

    return idNum;
}

/*
================
adm_mapRestart
1/13/11
Restarts the current map or starts a round in Competition Mode.
================
*/

int adm_mapRestart(int argNum, gentity_t *adm, qboolean shortCmd){
    if (adm && adm->client){
        if (level.mapSwitch == qfalse){
            level.mapSwitch = qtrue;
            level.mapAction = 1;
            level.mapSwitchCount = level.time;
            level.mapSwitchCount2 = 5; // Boe!Man 7/22/12: 5 seconds remaining on the timer.

            if (g_compMode.integer == 0){
                trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Map restarted by %s.\n\"", adm->client->pers.cleanName));
            }else if (g_compMode.integer > 0 && cm_enabled.integer == 1){
                if (cm_dr.integer == 1){ // Boe!Man 3/18/11: If dual rounds are enabled, the first round would be started.
                    G_printInfoMessageToAll("First round started.");
                    G_Broadcast("\\First round started!", BROADCAST_GAME, NULL);
                    level.compMsgCount = -1;
                }
                else{ // Boe!Man 3/18/11: If not, general message.
                    G_printInfoMessageToAll("Match started.");
                    G_Broadcast("\\Match started!", BROADCAST_GAME, NULL);
                    level.compMsgCount = -1;
                }

                #ifdef _GOLD
                if(level.clientMod == CL_ROCMOD){
                    // Enable the ROCmod match mode.
                    trap_Cvar_Set("inMatch",
                        (trap_Cvar_VariableIntegerValue("g_autoMatchDemo"))
                        ? "1" : "2");
                }
                #endif // _GOLD
            }else if (g_compMode.integer > 0 && cm_enabled.integer == 3){
                G_Broadcast("\\Second round started!", BROADCAST_GAME, NULL);
                G_printInfoMessageToAll("Second round started.");
                level.compMsgCount = -1;
            }

            Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
            Boe_adminLog("map restart", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), "none");
        }else{
            if (level.mapAction == 1 || level.mapAction == 3){
                G_printInfoMessage(adm, "A map restart is already in progress.");
            }else if (level.mapAction == 2 || level.mapAction == 4){
                G_printInfoMessage(adm, "A map switch is already in progress.");
            }else if(level.mapAction == 5){
                G_printInfoMessage(adm, "The map is already ending.");
            }
        }
    }else{
        if (level.mapSwitch == qfalse){
            level.mapSwitch = qtrue;
            level.mapAction = 1;
            level.mapSwitchCount = level.time;
            level.mapSwitchCount2 = 5; // Boe!Man 7/22/12: 5 seconds remaining on the timer.

            if (g_compMode.integer == 0){
                trap_SendServerCommand(-1, "print \"^3[Rcon Action] ^7Map restarted.\n\"");
            }else if (g_compMode.integer > 0 && cm_enabled.integer == 1){
                G_Broadcast("\\First round started!", BROADCAST_GAME, NULL);
                G_printInfoMessageToAll("First round started.");
                level.compMsgCount = -1;
            }else if (g_compMode.integer > 0 && cm_enabled.integer == 3){
                G_Broadcast("\\Second round started!", BROADCAST_GAME, NULL);
                G_printInfoMessageToAll("Second round started.");
                level.compMsgCount = -1;
            }

            Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
            Boe_adminLog("map restart", "RCON", "none");
        }else{
            if (level.mapAction == 1 || level.mapAction == 3){
                G_printInfoMessage(NULL, "A map restart is already in progress.");
            }else if (level.mapAction == 2 || level.mapAction == 4){
                G_printInfoMessage(NULL, "A map switch is already in progress.");
            }else if(level.mapAction == 5){
                G_printInfoMessage(NULL, "The map is already ending.");
            }
        }
    }

    return -1;
}

/*
================
adm_Burn

Burns a player and decreases their health.
================
*/

int adm_Burn(int argNum, gentity_t *adm, qboolean shortCmd)
{
    gentity_t       *ent, *tent;
    int             idNum;

    idNum = G_clientNumFromArg(adm, argNum, "burn", qtrue, qtrue, qtrue, shortCmd);
    if (idNum < 0) return idNum;

    ent = g_entities + idNum;

    // Create temporary entity for the burn effect.
    #ifndef _DEMO
    tent = G_TempEntity(g_entities[ent->s.number].r.currentOrigin, EV_EXPLOSION_HIT_FLESH);
    #else
    tent = G_TempEntity(g_entities[ent->s.number].r.currentOrigin, EV_BULLET_HIT_FLESH);
    #endif // not _DEMO
    tent->s.eventParm = 0;
    tent->s.otherEntityNum2 = g_entities[ent->s.number].s.number;
    tent->s.time = WP_ANM14_GRENADE + ((((int)g_entities[ent->s.number].s.apos.trBase[YAW] & 0x7FFF) % 360) << 16);
    VectorCopy(g_entities[ent->s.number].r.currentOrigin, tent->s.angles);
    SnapVector(tent->s.angles);

    // Set burn seconds and notify the player of what happened with a sound effect.
    ent->client->sess.burnSeconds = 4;
    Boe_ClientSound(ent, G_SoundIndex("/sound/weapons/incendiary_grenade/incen01.mp3"));

    return idNum;
}

/*
================
adm_Mute

Mutes the player or unmutes if the player is still muted.
================
*/

int adm_Mute(int argNum, gentity_t *adm, qboolean shortCmd)
{
    int         idNum, time;
    char        arg[128];
    qboolean    unMute = qfalse;

    idNum = G_clientNumFromArg(adm, argNum, "mute", qfalse, qfalse, qfalse, shortCmd);
    if (idNum < 0) return idNum;

    // Boe!Man 1/13/11: If the client's already muted, unmute him instead.
    if (g_entities[idNum].client->sess.mute == qtrue){
        unMute = qtrue;
    }

    // Get the time the client should be muted.
    if (shortCmd && G_GetChatArgumentCount()){
        time = atoi(G_GetChatArgument(2));
    }else{
        trap_Argv(argNum + 1, arg, sizeof(arg));
        time = atoi(arg);
    }

    // Check time given (or default to 5).
    if(time <= 0){
        time = 5;
    }else if(time > 60){
        G_printInfoMessage(adm, "The maximum time for mute is 60 minutes.");
        time = 60;
    }

    // Do the action.
    if (!unMute){
        AddMutedClient(&g_entities[idNum], time);
    }else{
        RemoveMutedClient(&g_entities[idNum]);
    }

    // Handle custom broadcasts for mute.
    Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
    if (adm && adm->client){
        G_Broadcast(va("%s\nwas \\%s\nby %s", g_entities[idNum].client->pers.netname, (unMute) ? "unmuted" : "muted", adm->client->pers.netname), BROADCAST_CMD, NULL);
        trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was %s by %s%s\n\"", g_entities[idNum].client->pers.cleanName, (unMute) ? "unmuted" : "muted", adm->client->pers.cleanName, (unMute) ? "." : va(" for %i minutes.", time)));
        Boe_adminLog("muted", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", g_entities[idNum].client->pers.ip, g_entities[idNum].client->pers.cleanName));
    }else{
        G_Broadcast(va("%s\nwas \\%s", g_entities[idNum].client->pers.netname, (unMute) ? "unmuted" : "muted"), BROADCAST_CMD, NULL);
        trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s was %s%s\n\"", g_entities[idNum].client->pers.cleanName, (unMute) ? "unmuted" : "muted", (unMute) ? "." : va(" for %i minutes.", time)));
        Boe_adminLog("muted", "RCON", va("%s\\%s", g_entities[idNum].client->pers.ip, g_entities[idNum].client->pers.cleanName));
    }

    return -1;
}

/*
================
adm_Strip

Strips the player from his weapons (except knife).
================
*/

int adm_Strip(int argNum, gentity_t *adm, qboolean shortCmd)
{
    gentity_t   *ent;
    int         idNum, idle;

    idNum = G_clientNumFromArg(adm, argNum, "strip", qtrue, qtrue, qtrue, shortCmd);
    if (idNum < 0) return idNum;

    ent = g_entities + idNum;

    // Strip the player.
    ent->client->ps.zoomFov = 0;
    ent->client->ps.pm_flags &= ~(PMF_GOGGLES_ON | PMF_ZOOM_FLAGS);
    ent->client->ps.stats[STAT_WEAPONS] = 0;
    ent->client->ps.stats[STAT_GOGGLES] = GOGGLES_NONE;
    memset(ent->client->ps.ammo, 0, sizeof(ent->client->ps.ammo));
    memset(ent->client->ps.clip, 0, sizeof(ent->client->ps.clip));
    ent->client->ps.stats[STAT_WEAPONS] |= (1 << WP_KNIFE);
    ent->client->ps.clip[ATTACK_NORMAL][WP_KNIFE] = weaponData[WP_KNIFE].attack[ATTACK_NORMAL].clipSize;
    ent->client->ps.firemode[WP_KNIFE] = BG_FindFireMode(WP_KNIFE, ATTACK_NORMAL, WP_FIREMODE_AUTO);
    ent->client->ps.weapon = WP_KNIFE;
    BG_GetInviewAnim(ent->client->ps.weapon, "idle", &idle);
    ent->client->ps.weaponAnimId = idle;
    ent->client->ps.weaponstate = WEAPON_READY;
    ent->client->ps.weaponTime = 0;
    ent->client->ps.weaponAnimTime = 0;
    // Boe!Man 6/23/13: Fixed crash bug in Linux, simply because 'bg_outfittingGroups[-1][client->pers.outfitting.items[-1]]' equals 0.
    ent->client->ps.stats[STAT_OUTFIT_GRENADE] = bg_itemlist[0].giTag;

    return idNum;
}

/*
================
adm_removeAdmin

Removes the players' Admin.
================
*/

int adm_removeAdmin(int argNum, gentity_t *adm, qboolean shortCmd)
{
    int         idNum, admLvl;

    idNum = G_clientNumFromArg(adm, argNum, "do this to", qfalse, qtrue, qtrue, shortCmd);
    if (idNum < 0) return idNum;

    // Boe!Man 11/24/15: User must have enough privileges to remove this Admin.
    if(adm && adm->client){
        admLvl = g_entities[idNum].client->sess.admin;

        if((admLvl == 2 && adm->client->sess.admin < g_badmin.integer)
        || (admLvl == 3 && adm->client->sess.admin < g_admin.integer)
        || (admLvl == 4 && adm->client->sess.admin < g_sadmin.integer)){
            G_printInfoMessage(adm, "Your Admin level is too low to remove this Admin.");
            return -1;
        }
    }

    // The user should probably be an Admin in order to remove him. ;-)
    if (!g_entities[idNum].client->sess.admin){
        G_printInfoMessage(adm, "%s is not an Admin!", g_entities[idNum].client->pers.cleanName);
        return -1;
    }

    // Remove the player from the database.
    Boe_removeAdminFromDb(adm, g_entities[idNum].client->pers.ip, qfalse, qfalse, qtrue);

    // Handle custom broadcast.
    G_Broadcast(va("%s\nis no longer an \\Admin", g_entities[idNum].client->pers.netname), BROADCAST_CMD, NULL);
    Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
    if (adm && adm->client){
        trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was removed as Admin by %s.\n\"", g_entities[idNum].client->pers.cleanName, adm->client->pers.cleanName));
    }else{
        trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s was removed as Admin.\n\"", g_entities[idNum].client->pers.cleanName));
    }

    // He's not an Admin anymore so it doesn't matter if he was a B-Admin, Admin or S-Admin: in any way he shouldn't be allowed to spec the opposite team.
    g_entities[idNum].client->sess.admin = 0;
    g_entities[idNum].client->sess.adminspec = qfalse;

    return -1;
}

/*
================
adm_forceTeam

Forces a player (or everyone) to a specific team.
================
*/

int adm_forceTeam(int argNum, gentity_t *adm, qboolean shortCmd)
{
    char        str[MAX_TOKEN_CHARS];
    int         idNum;
    int         i, xTeam, argc;
    qboolean    all = qfalse; // Boe!Man 4/15/13: If this is true, forceteam all players to a specific team.
    char        userinfo[MAX_INFO_STRING];

    if (shortCmd) {
        argc = G_GetChatArgumentCount();
    }

    if (!shortCmd || shortCmd && !argc){
        trap_Argv(argNum, str, sizeof(str));
    }else{
        Q_strncpyz(str, G_GetChatArgument(1), sizeof(str));
    }

    Q_strlwr(str);

    // Check for "all".
    if (strcmp(str, "all") == 0){
        all = qtrue;
    }else{
        // Find the player.
        idNum = G_clientNumFromArg(adm, argNum, "forceteam", qfalse, qtrue, qtrue, shortCmd);

        // Boe!Man 1/22/14: If "all" or the client wasn't found, return.
        if (idNum < 0) return -1;
    }

    // Set the team.
    memset(str, 0, sizeof(str));
    if(shortCmd && argc){
        Q_strncpyz(str, G_GetChatArgument(2), sizeof(str));
    }else{
        trap_Argv(argNum + 1, str, sizeof(str));
    }

    if (str[0] == 's' || str[0] == 'S'){
        strcpy(str, "spectator");
        xTeam = TEAM_SPECTATOR;
    }else if (str[0] == 'r' || str[0] == 'R'){
        strcpy(str, "red");
        xTeam = TEAM_RED;
    }else if (str[0] == 'b' || str[0] == 'B'){
        strcpy(str, "blue");
        xTeam = TEAM_BLUE;
    }else{
        G_printInfoMessage(adm, "Wrong team specified: %s.", str);
        return -1;
    }

    // Boe!Man 4/15/13: Check if we can forceteam all.
    if (all){
        for (i = 0; i<level.numConnectedClients; i++){
            if (g_entities[level.sortedClients[i]].r.svFlags & SVF_BOT){ // Henk 25/01/11 -> Reset bots to set them to another team
                trap_GetUserinfo(g_entities[level.sortedClients[i]].s.number, userinfo, sizeof(userinfo));
                Info_SetValueForKey(userinfo, "team", str);
                trap_SetUserinfo(g_entities[level.sortedClients[i]].s.number, userinfo);
                g_entities[level.sortedClients[i]].client->sess.team = (team_t)xTeam;

                if (current_gametype.value != GT_HS){
                    g_entities[level.sortedClients[i]].client->pers.identity = BG_FindTeamIdentity(level.gametypeTeam[xTeam], -1);
                }else{
                    // Boe!Man 8/3/16: Don't forget to wipe the scores of the bot in H&S.
                    g_entities[level.sortedClients[i]].client->sess.score = 0;
                    g_entities[level.sortedClients[i]].client->sess.kills = 0;
                    g_entities[level.sortedClients[i]].client->sess.deaths = 0;
                }
                ClientBegin(g_entities[level.sortedClients[i]].s.number, qfalse);
            }else{
                SetTeam(&g_entities[level.sortedClients[i]], str, NULL, qtrue);
            }
        }

        // Custom broadcast for all.
        Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
        if (adm){
            G_Broadcast(va("Everybody was \\forceteamed\nby %s", adm->client->pers.netname), BROADCAST_CMD, NULL);
            trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Everybody was forceteamed by %s.\n\"", adm->client->pers.cleanName));
            Boe_adminLog("forceteam", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), "All");
        }else{
            G_Broadcast("Everybody was \\forceteamed", BROADCAST_CMD, NULL);
            trap_SendServerCommand(-1, "print\"^3[Rcon Action] ^7Everybody was forceteamed.\n\"");
            Boe_adminLog("forceteam", "RCON", "All");
        }

        return -1;
    }

    if (g_entities[idNum].client){
        if (g_entities[idNum].r.svFlags & SVF_BOT){ // Henk 25/01/11 -> Reset bots to set them to another team
            trap_GetUserinfo(idNum, userinfo, sizeof(userinfo));
            Info_SetValueForKey(userinfo, "team", str);
            trap_SetUserinfo(idNum, userinfo);
            g_entities[idNum].client->sess.team = (team_t)xTeam;

            if (current_gametype.value != GT_HS){
                g_entities[idNum].client->pers.identity = BG_FindTeamIdentity(level.gametypeTeam[xTeam], -1);
            }else{
                // Boe!Man 8/3/16: Don't forget to wipe the scores of the bot in H&S.
                g_entities[idNum].client->sess.score = 0;
                g_entities[idNum].client->sess.kills = 0;
                g_entities[idNum].client->sess.deaths = 0;
            }

            ClientBegin(idNum, qfalse);
        }else{
            SetTeam(&g_entities[idNum], str, NULL, qtrue);
        }
    }

    return idNum;
}

/*
================
adm_noLower

Disables or enables nolower if an entity of such kind is specified on the current map.
================
*/

int adm_noLower(int argNum, gentity_t *adm, qboolean shortCmd)
{
    adm_toggleSection(adm, "Nolower", 0, g_useNoLower.integer);

    return -1;
}

/*
================
adm_noRoof

Disables or enables nolower if an entity of such kind is specified on the current map.
================
*/

int adm_noRoof(int argNum, gentity_t *adm, qboolean shortCmd)
{
    adm_toggleSection(adm, "Noroof", 1, g_useNoRoof.integer);

    return -1;
}

/*
================
adm_noMiddle

Disables or enables nomiddle if an entity of such kind is specified on the current map.
================
*/

int adm_noMiddle(int argNum, gentity_t *adm, qboolean shortCmd)
{
    adm_toggleSection(adm, "Nomiddle", 2, g_useNoMiddle.integer);

    return -1;
}

/*
================
adm_noWhole

Disables or enables nowhole if an entity of such kind is specified on the current map.
================
*/

int adm_noWhole(int argNum, gentity_t *adm, qboolean shortCmd)
{
    adm_toggleSection(adm, "Nowhole", 3, g_useNoWhole.integer);

    return -1;
}

/*
================
adm_toggleSection

Disables or enables a section if an entity of such kind is specified on the current map.
================
*/

static void adm_toggleSection(gentity_t *adm, char *sectionName, int sectionID, int useSection)
{
    char        sectionNameWithoutCap[16];
    qboolean    enabled;

    // Copy the name of the section to a variable that holds the name without capital.
    strncpy(sectionNameWithoutCap, sectionName, sizeof(sectionNameWithoutCap));
    Q_strlwr(sectionNameWithoutCap);

    // Boe!Man 2/27/11: If people don't want to use this section they can specify to disable it.
    if (useSection <= 0){
        G_printInfoMessage(adm, "%s has been disabled on this server.", sectionName);
        return;
    }
    // Boe!Man 1/8/12: If people want to use nolower but if there's no such entity found, inform the user.
    if (!level.noLRMWEntFound[sectionID]){
        G_printInfoMessage(adm, "No entity found to toggle %s.", sectionNameWithoutCap);
        return;
    }

    enabled = !level.noLRActive[sectionID];
    level.noLRActive[sectionID] = enabled;
    level.autoLRMWActive[sectionID] = enabled;

    // Custom broadcasts.
    Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
    G_Broadcast(va("\\%s %s!", sectionName, (enabled) ? "enabled" : "disabled"), BROADCAST_CMD, NULL);

    if (adm && adm->client){
        trap_SendServerCommand(-1, va("print \"^3[Admin Action] ^7%s %s by %s.\n\"", sectionName, (enabled) ? "enabled" : "disabled", adm->client->pers.cleanName));
        Boe_adminLog(va("%s %s", sectionNameWithoutCap, (enabled) ? "enabled" : "disabled"), va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), "none");
    }else{
        trap_SendServerCommand(-1, va("print \"^3[Rcon Action] ^7%s %s.\n\"", sectionName, (enabled) ? "enabled" : "disabled"));
        Boe_adminLog(va("%s %s", sectionNameWithoutCap, (enabled) ? "enabled" : "disabled"), va("RCON"), "none");
    }

    // Boe!Man 11/24/13: Also open the section.
    if (!level.noLRActive[sectionID]){
        gentity_t *ent = NULL;

        while (NULL != (ent = G_Find(ent, FOFS(classname), sectionNameWithoutCap))){
            g_sectionAddOrDelInstances(ent, qfalse);
            ent->sectionState = 2; // Reset state to Opened.
        }
    }
}

/*
================
adm_shuffleTeams

Shuffles team players randomly.
================
*/

int adm_shuffleTeams(int argNum, gentity_t *adm, qboolean shortCmd){
    int i;
    int teamTotalRed = 0, teamTotalBlue = 0; // The counted team totals when shuffling.
    int teamTotalRed2 = 0, teamTotalBlue2 = 0; // The actual team totals prior to shuffling.
    int newTeam;
    char newTeam2[5]; // red or blue.
    char userinfo[MAX_INFO_STRING];

    // Boe!Man 7/13/12: Do not allow shuffleteams during Zombies.
    if (current_gametype.value == GT_HZ){
        G_printInfoMessage(adm, "You cannot shuffle the teams in this gametype.");
        return -1;
    }

    // Boe!Man 6/16/12: Check gametype first.
    if (!level.gametypeData->teams){
        G_printInfoMessage(adm, "Not playing a team game.");
        return -1;
    }

    if (level.blueLocked || level.redLocked){
        G_printInfoMessage(adm, "Teams are locked.");
        return -1;
    }

    // Preserve team balance.
    teamTotalRed2 = TeamCount(-1, TEAM_RED, NULL);
    teamTotalBlue2 = TeamCount(-1, TEAM_BLUE, NULL);

    for (i = 0; i < level.numConnectedClients; i++){
        // Skip clients that are spectating.
        if (g_entities[level.sortedClients[i]].client->sess.team == TEAM_SPECTATOR){
            continue;
        }

        // Start shuffling using irand, or put them to the team that needs more players when one is on its preserved rate.
        if (teamTotalRed == teamTotalRed2){ // Blimey, we're on max.
            newTeam = TEAM_BLUE;
        }else if (teamTotalBlue == teamTotalBlue2){
            newTeam = TEAM_RED;
        }else{
            newTeam = irand(1, 2);
            if (newTeam == TEAM_RED){
                teamTotalRed += 1;
            }else{
                teamTotalBlue += 1;
            }
        }

        // Drop any gametype items they might have.
        if (g_entities[level.sortedClients[i]].s.gametypeitems > 0){
            G_DropGametypeItems(&g_entities[level.sortedClients[i]], 0);
        }

        // Remove their weps and set as ghost.
        g_entities[level.sortedClients[i]].client->ps.stats[STAT_WEAPONS] = 0;
        G_StartGhosting(&g_entities[level.sortedClients[i]]);

        // Do the team changing.
        g_entities[level.sortedClients[i]].client->sess.team = (team_t)newTeam;

        // Take care of the bots.
        if (g_entities[level.sortedClients[i]].r.svFlags & SVF_BOT){ // Reset bots to set them to another team
            trap_GetUserinfo(level.sortedClients[i], userinfo, sizeof(userinfo));

            if (newTeam == TEAM_RED){
                strncpy(newTeam2, "red", sizeof(newTeam2));
            }
            else if (newTeam == TEAM_BLUE){
                strncpy(newTeam2, "blue", sizeof(newTeam2));
            }

            Info_SetValueForKey(userinfo, "team", newTeam2);
            trap_SetUserinfo(level.sortedClients[i], userinfo);
            g_entities[level.sortedClients[i]].client->sess.team = (team_t)newTeam;
            if (current_gametype.value != GT_HS){
                g_entities[level.sortedClients[i]].client->pers.identity = BG_FindTeamIdentity(level.gametypeTeam[newTeam], -1);
            }
        }

        // Prep. for change & respawn.
        g_entities[level.sortedClients[i]].client->pers.identity = NULL;
        ClientUserinfoChanged(level.sortedClients[i]);
        CalculateRanks();

        G_StopFollowing(&g_entities[level.sortedClients[i]]);
        G_StopGhosting(&g_entities[level.sortedClients[i]]);
        trap_UnlinkEntity(&g_entities[level.sortedClients[i]]);
        ClientSpawn(&g_entities[level.sortedClients[i]]);
    }

    // Custom messaging/logging.
    Boe_GlobalSound(G_SoundIndex("sound/misc/events/tut_lift02.mp3"));
    G_Broadcast("\\Shuffle teams!", BROADCAST_CMD, NULL);

    if (adm && adm->client){
        trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Shuffle teams by %s.\n\"", adm->client->pers.cleanName));
        Boe_adminLog("shuffleteams", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), "none");
    }
    else{
        trap_SendServerCommand(-1, "print\"^3[Rcon Action] ^7Shuffle teams.\n\"");
        Boe_adminLog("shuffleteams", "RCON", "none");
    }

    return -1;
}

/*
================
adm_noNades

Disables or enables nades.
================
*/

int adm_noNades(int argNum, gentity_t *adm, qboolean shortCmd){
    int     i, state;
    char    *available;

    // Boe!Man 1/19/11: Disable NN in H&S.
    if (current_gametype.value == GT_HS){
        G_printInfoMessage(adm, "You cannot enable/disable Nades in Hide&Seek.");
        return -1;
    }else if (current_gametype.value == GT_HZ){ // Boe!Man 9/20/12: And H&Z.
        G_printInfoMessage(adm, "You cannot enable/disable Nades in Humans&Zombies.");
        return -1;
    }

    // Check if there are nades ready to be re-enabled.
    // Dynamically enabling nades in-game can alter the nades found state,
    // meaning we have to re-check if they were enabled to begin with (in the availableWeapons CVAR).
    if (g_disableNades.integer > 0){
        level.nadesFound = qfalse;

        available = calloc(level.wpNumWeapons + 1, sizeof(char));
        if(available != NULL){
            strncpy(available, availableWeapons.string, (strlen(availableWeapons.string) <= level.wpNumWeapons) ? strlen(availableWeapons.string) : level.wpNumWeapons);

            for(i = level.grenadeMin; i <= level.grenadeMax; i++){
                if(available[i - 1] == '1' || available[i - 1] == '2'){
                    level.nadesFound = qtrue;
                    break;
                }
            }

            free(available);
        }

        state = 0;
    }else{
        state = 1;
    }

    if(!level.nadesFound){
        if (adm && adm->client){
            G_printInfoMessage(adm, "No nades are set to be used (enable nades with /adm toggleweapon or !wp).");
        }else{
            G_printInfoMessage(NULL, "No nades are set to be used (enable nades with toggleweapon).");
        }

        return -1;
    }

    g_disableNades.integer = state;
    trap_Cvar_Set("g_disableNades", va("%i", state));
    trap_Cvar_Update(&g_disableNades);
    SetNades(va("%i", state));
    BG_SetAvailableOutfitting(g_availableWeapons.string);

    for (i = 0; i<level.numConnectedClients; i++){
        level.clients[level.sortedClients[i]].noOutfittingChange = qfalse;
        G_UpdateOutfitting(g_entities[level.sortedClients[i]].s.number);
    }

    // Custom broadcasts/logging.
    Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
    G_Broadcast(va("Nades \\%s!", (!state) ? "enabled" : "disabled"), BROADCAST_CMD, NULL);
    if (adm && adm->client){
        trap_SendServerCommand(-1, va("print \"^3[Admin Action] ^7Nades %s by %s.\n\"", (!state) ? "enabled" : "disabled", adm->client->pers.cleanName));
        Boe_adminLog(va("nades %s", (!state) ? "enabled" : "disabled"), va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), "none");
    }else{
        trap_SendServerCommand(-1, va("print \"^3[Rcon Action] ^7Nades %s.\n\"", (!state) ? "enabled" : "disabled"));
        Boe_adminLog(va("nades %s", (!state) ? "enabled" : "disabled"), "RCON", "none");
    }

    return -1;
}

/*
================
adm_scoreLimit

Sets the score limit or shows it to the one issuing the command.
================
*/

int adm_scoreLimit(int argNum, gentity_t *adm, qboolean shortCmd)
{
    adm_toggleCVAR(adm, argNum, shortCmd, "Scorelimit", &g_scorelimit, qtrue, "cm_sl", &cm_sl);

    return -1;
}

/*
================
adm_timeLimit

Sets the time limit or shows it to the one issuing the command.
================
*/

int adm_timeLimit(int argNum, gentity_t *adm, qboolean shortCmd)
{
    adm_toggleCVAR(adm, argNum, shortCmd, "Timelimit", &g_timelimit, qtrue, "cm_tl", &cm_tl);

    // Boe!Man 5/7/17: If the timelimit is hit already, check
    // if the new value makes the timelimit not being hit anymore.
    if(level.timelimithit && level.time - level.startTime
        #ifndef _DEMO
        < (g_timelimit.integer + level.timeExtension)
        #else
        < (g_timelimit.integer + g_timeextension.integer
        * g_timeextensionmultiplier.integer)
        #endif // not _DEMO
        * 60000
    ){
        level.timelimithit = qfalse;
        G_printInfoMessageToAll("Timelimit is not hit anymore, " \
            "continuing map.");
    }

    return -1;
}

/*
================
adm_respawnInterval

Sets the respawninterval or shows it to the one issuing the command.
================
*/

int adm_respawnInterval(int argNum, gentity_t *adm, qboolean shortCmd)
{
    char    arg[32];

    if(adm == NULL){
        trap_Argv(argNum, arg, sizeof(arg));

        if(!arg[0]){
            Com_Printf("Respawn interval is %d.\n", g_respawnInterval.integer);
            return -1;
        }

        trap_Cvar_Set("g_respawninterval", arg);
        trap_Cvar_Update(&g_respawnInterval);

        trap_SendServerCommand(-1, va("print \"^3[Rcon Action] ^7Respawn interval changed to %d.\n\"", g_respawnInterval.integer));
        G_Broadcast(va("\\Respawn interval %d!", g_respawnInterval.integer), BROADCAST_CMD, NULL);
        Boe_adminLog(va("Respawn interval %d", g_respawnInterval.integer), "RCON", "none");

        return -1;
    }

    adm_toggleCVAR(adm, argNum, shortCmd, "Respawn interval", &g_respawnInterval, qfalse, NULL, NULL);

    return -1;
}

/*
================
adm_toggleCVAR

Complicated way of toggling or showing a CVAR for both scrim CVARs or real ones (depends on arguments supplied).
================
*/

static void adm_toggleCVAR(gentity_t *adm, int argNum, qboolean shortCmd, char *cvarName, vmCvar_t *cvar1, qboolean availableInCM, char *cvarNameCM, vmCvar_t *cvar2)
{
    char    cvarNameWithoutCap[32];
    char    arg[32];
    int     cvarValue;
    #ifdef _GOLD
    int i;
    #endif // _GOLD

    // Copy the name of the CVAR without capital.
    strncpy(cvarNameWithoutCap, cvarName, sizeof(cvarNameWithoutCap));

    if (shortCmd) {
        // Grab the new cvar value.
        cvarValue = GetArgument(argNum);
    }else{
        arg[0] = 0;

        trap_Argv(argNum, arg, sizeof(arg));
        if (arg[0]) {
            cvarValue = atoi(arg);
        }else{
            cvarValue = -1;
        }
    }

    // Check if the player supplied an argument. If not, show him the current values.
    if (cvarValue < 0){
        if (availableInCM && g_compMode.integer > 0 && cm_enabled.integer >= 1){
            G_printInfoMessage(adm, "Match %s is %d.", cvarNameWithoutCap, (cvar2 != NULL) ? cvar2->integer : cvar1->integer);
        }else{
            G_printInfoMessage(adm, "%s is %d.", cvarName, cvar1->integer);
        }

        return;
    }

    // Is Competition Mode active so we can update the scrim setting vs. the global one?
    if (availableInCM && g_compMode.integer > 0 && cm_enabled.integer >= 1){
        if (cm_enabled.integer == 1){
            trap_Cvar_Set(cvarNameCM, va("%i", cvarValue));
            trap_SendServerCommand(-1, va("print \"^3[Admin Action] ^7Match %s changed to %i by %s.\n\"", cvarNameWithoutCap, cvarValue, adm->client->pers.cleanName));
        }else if (cm_enabled.integer > 1 && cm_enabled.integer < 5){
            trap_Cvar_Set(cvarNameCM, va("%i", cvarValue));
            trap_SendServerCommand(-1, va("print \"^3[Admin Action] ^7Match %s changed to %i by %s.\n\"", cvarNameWithoutCap, cvarValue, adm->client->pers.cleanName));
            Boe_setTrackedCvar(cvar1, cvarValue); // Avoid the [Rcon Action] message for the real CVAR.
        }
    }else{
        if (cvar1 != NULL){
            Boe_setTrackedCvar(cvar1, cvarValue); // Avoid the [Rcon Action] message for the real CVAR.
        }
        else{
            trap_Cvar_Set(cvarNameWithoutCap, va("%i", cvarValue));
            trap_Cvar_Update(cvar1);
        }

        trap_SendServerCommand(-1, va("print \"^3[Admin Action] ^7%s changed to %i by %s.\n\"", cvarName, cvarValue, adm->client->pers.cleanName));
    }

    Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
    G_Broadcast(va("\\%s %i!", cvarName, cvarValue), BROADCAST_CMD, NULL);
    Boe_adminLog(va("%s %i", cvarName, cvarValue), va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), "none");

    #ifdef _GOLD
    // Send scoreboard to all clients in ROCmod (so variables are properly instantly re-calculated).
    for (i = 0; i < level.maxclients; i++) {
        if (level.clients[i].pers.connected == CON_CONNECTED) {
            DeathmatchScoreboardMessage(g_entities + i);
        }
    }
    #endif // _GOLD
}

/*
================
adm_realDamage

Enables real damage.
================
*/

int adm_realDamage(int argNum, gentity_t *adm, qboolean shortCmd)
{
    adm_Damage(adm, "Real damage", 1);

    return -1;
}

/*
================
adm_normalDamage

Enables normal damage.
================
*/

int adm_normalDamage(int argNum, gentity_t *adm, qboolean shortCmd)
{
    adm_Damage(adm, "Normal damage", 0);

    return -1;
}

/*
================
adm_normalDamage

Enables normal damage.
================
*/

static void adm_Damage(gentity_t *adm, char *damageName, int value)
{
    int i;

    // No such thing as ND or RD in Hide&Seek or Humans&Zombies.
    if(current_gametype.value == GT_HS || current_gametype.value == GT_HZ){
        G_printInfoMessage(adm, "You cannot switch to %s in this gametype.", damageName);
        return;
    }

    g_instaGib.integer = value;
    G_WeaponMod();

    for (i = 0; i < level.numConnectedClients; i++){
        level.clients[level.sortedClients[i]].noOutfittingChange = qfalse;
        G_UpdateOutfitting(g_entities[level.sortedClients[i]].s.number);
    }

    Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
    G_Broadcast(va("\\%s!", damageName), BROADCAST_CMD, NULL);

    if (adm && adm->client){
        trap_SendServerCommand(-1, va("print \"^3[Admin Action] ^7%s by %s.\n\"", damageName, adm->client->pers.cleanName));
        Boe_adminLog(damageName, va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), "none");
    }
    else{
        trap_SendServerCommand(-1, va("print \"^3[Rcon Action] ^7%s.\n\"", damageName));
        Boe_adminLog(damageName, "RCON", "none");
    }
}

/*
================
adm_gametypeRestart

Restarts gametype.
================
*/

int adm_gametypeRestart(int argNum, gentity_t *adm, qboolean shortCmd)
{
    // Cannot restart the gametype in DM (at least, no visible effect).
    if(current_gametype.value == GT_DM){
        G_printInfoMessage(adm, "You cannot restart this gametype.");
        return -1;
    }

    // Broadcast the change and restart it.
    Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
    G_Broadcast("\\Gametype restart!", BROADCAST_CMD, NULL);

    if (adm && adm->client){
        trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Gametype restart by %s.\n\"", adm->client->pers.cleanName));
        Boe_adminLog("gametype restart", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), "none");
    }else{
        trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7Gametype restart.\n\""));
        Boe_adminLog("gametype restart", "RCON", "none");
    }

    trap_SendConsoleCommand(EXEC_APPEND, "gametype_restart\n");

    return -1;
}

/*
================
adm_addClanMember

Adds a clan member.
================
*/

int adm_addClanMember(int argNum, gentity_t *adm, qboolean shortCmd)
{
    int         idNum;
    char        clientName[MAX_NETNAME];
    char        admName[MAX_NETNAME];
    char        ip[MAX_IP];
    sqlite3     *db;

    idNum = G_clientNumFromArg(adm, argNum, "do this to", qfalse, qtrue, qtrue, shortCmd);
    if (idNum < 0) return idNum;

    // Boe!Man 2/6/13: Check if the client is already a clan member.
    if (g_entities[idNum].client->sess.clanMember){
        G_printInfoMessage(adm, "%s already is a clan member.", g_entities[idNum].client->pers.cleanName);
        return -1;
    }

    // Boe!Man 12/12/12: Check the names, SQLite has massive problems when using quotes in the (updated) query.
    Q_strncpyz(clientName, g_entities[idNum].client->pers.cleanName, sizeof(clientName));
    Boe_convertNonSQLChars(clientName);
    if (adm){
        Q_strncpyz(admName, adm->client->pers.cleanName, sizeof(admName));
        Boe_convertNonSQLChars(admName);
    }
    else{
        strcpy(admName, "RCON");
    }

    // Boe!Man 3/17/15: Check if the server owner has subnets rather than full IPs enabled.
    if (g_preferSubnets.integer){
        Q_strncpyz(ip, g_entities[idNum].client->pers.ip, 7);
    }else{
        Q_strncpyz(ip, g_entities[idNum].client->pers.ip, sizeof(ip));
    }

    // Boe!Man 2/6/13: Add Clan Member to the database.
    // Boe!Man 5/27/13: Open database.
    db = usersDb;

    // Boe!Man 2/6/13: Insert query.
    if (sqlite3_exec(db, va("INSERT INTO clanmembers (IP, name, by) values ('%s', '%s', '%s')", ip, clientName, admName), 0, 0, 0) != SQLITE_OK){
        if (adm){
            trap_SendServerCommand(adm-g_entities, va("print \"^1[Error] ^7users database: %s\n\"", sqlite3_errmsg(db)));
        }
        else{
            G_LogPrintf("^1Error: ^7users database: %s\n", sqlite3_errmsg(db));
        }

        return -1;
    }

    g_entities[idNum].client->sess.clanMember = qtrue;
    Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
    G_Broadcast(va("%s\nis now a \\Clan member!", g_entities[idNum].client->pers.netname), BROADCAST_CMD, NULL);

    if (adm && adm->client){
        trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was made clan member by %s.\n\"", g_entities[idNum].client->pers.cleanName, adm->client->pers.cleanName));
        Boe_adminLog("add clan", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", g_entities[idNum].client->pers.ip, g_entities[idNum].client->pers.cleanName));
    }else{
        trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s is now a clan member.\n\"", g_entities[idNum].client->pers.cleanName));
        Boe_adminLog("add clan", "RCON", va("%s\\%s", g_entities[idNum].client->pers.ip, g_entities[idNum].client->pers.cleanName));
    }

    return -1;
}

/*
================
adm_removeClanMember
2/6/13 - 2:46 PM
Removes a clan member.
================
*/

int adm_removeClanMember(int argNum, gentity_t *adm, qboolean shortCmd)
{
    int         idNum;

    idNum = G_clientNumFromArg(adm, argNum, "remove", qfalse, qtrue, qtrue, shortCmd);
    if (idNum < 0) return idNum;

    if(!g_entities[idNum].client->sess.clanMember){
        G_printInfoMessage(adm, "%s is not a clan member!", g_entities[idNum].client->pers.cleanName);
        return -1;
    }

    // Remove the client both in-game and from the database.
    g_entities[idNum].client->sess.clanMember = qfalse;
    Boe_removeClanMemberFromDb(adm, g_entities[idNum].client->pers.ip, qfalse, qtrue);

    // Broadcast the change and log it.
    Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
    G_Broadcast(va("%s\nis no longer a \\Clan member!", g_entities[idNum].client->pers.netname), BROADCAST_CMD, NULL);

    if (adm && adm->client){
        trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was removed as clan member by %s.\n\"", g_entities[idNum].client->pers.cleanName, adm->client->pers.cleanName));
    }else{
        trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s is no longer a clan member.\n\"", g_entities[idNum].client->pers.cleanName));
    }

    return -1;
}

/*
================
adm_removeClanMemberFromList
2/6/13 - 4:40 PM
Removes a clan member from the clanlist.
================
*/

int adm_removeClanMemberFromList(int argNum, gentity_t *adm, qboolean shortCmd)
{
    char    arg[32];

    memset(arg, 0, sizeof(arg));

    if(shortCmd && G_GetChatArgumentCount()){
        strncpy(arg, G_GetChatArgument(1), sizeof(arg));
    }else{
        trap_Argv(argNum, arg, sizeof(arg));
    }

    if (!strstr(arg, ".")){ // Boe!Man 2/6/13: No dot found, unban by line number.
        Boe_removeClanMemberFromDb(adm, arg, qtrue, qfalse);
    }else{ // Boe!Man 2/6/13: Dot found, unban by IP.
        Boe_removeClanMemberFromDb(adm, arg, qfalse, qfalse);
    }

    return -1;
}

/*
================
adm_compMode

Toggles Competition Mode.
================
*/

int adm_compMode(int argNum, gentity_t *adm, qboolean shortCmd)
{
    qboolean enabled;

    // Don't allow the client to toggle CM in H&S/H&Z.
    if (current_gametype.value == GT_HS || current_gametype.value == GT_HZ){
        G_printInfoMessage(adm, "You cannot enable Competition Mode in this gametype.");
        return -1;
    }

    // Disable Competition Mode when a user is changing maps.
    if (level.mapAction){
        if (level.mapAction == 1 || level.mapAction == 3){
            G_printInfoMessage(adm, "A map restart is already in progress.");
        }else if (level.mapAction == 2 || level.mapAction == 4){
            G_printInfoMessage(adm, "A map switch is already in progress.");
        }else if(level.mapAction == 5){
            G_printInfoMessage(adm, "The map is already ending.");
        }

        return -1;
    }

    enabled = g_compMode.integer == 0;

    // Set settings depending on disabled/enabled.
    if (enabled){
        trap_Cvar_Set("g_compMode", "1");
        // Set the temporary CVARs.
        trap_Cvar_Set("cm_enabled", "1");
        trap_Cvar_Set("cm_sl", g_matchScoreLimit.string);
        trap_Cvar_Set("cm_tl", g_matchTimeLimit.string);
        trap_Cvar_Set("cm_slock", g_matchLockSpec.string);
        trap_Cvar_Set("cm_aswap", g_matchSwapTeams.string);
        trap_Cvar_Set("cm_devents", g_matchDisableEvents.string);
        if (g_matchRounds.integer <= 1){
            trap_Cvar_Set("cm_dr", "0");
        }else if (g_matchRounds.integer >= 2){
            trap_Cvar_Set("cm_dr", "1");
        }
        // Set the current timelimit in the temp CVARs so we can restore them after the scrim has ended.
        trap_Cvar_Set("cm_oldsl", g_scorelimit.string);
        trap_Cvar_Set("cm_oldtl", g_timelimit.string);
        level.compMsgCount = level.time + 3000;
    }else{
        trap_Cvar_Set("g_compMode", "0");
        trap_Cvar_Set("cm_enabled", "0");
        trap_Cvar_Set("cm_devents", "0"); // It's used for more things than just in a scrim, so make sure to reset it!
        trap_Cvar_Set("scorelimit", cm_oldsl.string);
        trap_Cvar_Set("timelimit", cm_oldtl.string);

        // Ensure teams are also unlocked now.
        level.blueLocked = 0;
        level.redLocked = 0;
        level.specsLocked = 0;

        #ifdef _GOLD
        if(level.clientMod == CL_ROCMOD){
            // Ensure we stop the match mode.
            trap_Cvar_Set("inMatch", "0");
        }
        #endif // _GOLD
    }

    // Broadcast the change.
    Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
    G_Broadcast(va("\\Competition mode %s!", (enabled) ? "enabled" : "disabled"), BROADCAST_CMD, NULL);

    if (adm && adm->client){
        trap_SendServerCommand(-1, va("print \"^3[Admin Action] ^7Competition mode %s by %s.\n\"", (enabled) ? "enabled" : "disabled", adm->client->pers.cleanName));
        Boe_adminLog(va("compmode %s", (enabled) ? "enabled" : "disabled"), va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), "none");
    }else{
        trap_SendServerCommand(-1, va("print \"^3[Rcon Action] ^7Competition mode %s.\n\"", (enabled) ? "enabled" : "disabled"));
        Boe_adminLog(va("compmode %s", (enabled) ? "enabled" : "disabled"), "RCON", "none");
    }

    // Update CVARs.
    trap_Cvar_Update(&g_compMode);
    trap_Cvar_Update(&cm_sl);
    trap_Cvar_Update(&cm_tl);
    trap_Cvar_Update(&g_scorelimit);
    trap_Cvar_Update(&g_timelimit);

    return -1;
}

/*
================
adm_banList

Show the banlist to the client/RCON.
================
*/

int adm_banList(int argNum, gentity_t *adm, qboolean shortCmd)
{
    adm_showBanList(argNum, adm, shortCmd, qfalse);

    return -1;
}

/*
================
adm_subnetbanList

Show the subnet banlist to the client/RCON.
================
*/

int adm_subnetbanList(int argNum, gentity_t *adm, qboolean shortCmd)
{
    adm_showBanList(argNum, adm, shortCmd, qtrue);

    return -1;
}

/*
================
adm_checkListFilters

Checks if filters are supplied
in lists in /adm or /rcon mode.

Returns NULL if we should
exit from the calling function
too.
================
*/

static char *adm_checkListFilters(gentity_t *adm, int argNum, qboolean shortCmd, char *listName, char *byFieldName)
{
    char            buf2[1000] = "\0";
    char            listNameReal[32];
    int             rc;
    char            arg[32];
    char            arg2[32];
    int             argCount;
    char            filterIP[32] = "\0";
    char            filterName[32] = "\0";
    char            filterBy[32] = "\0";
    static char     filterQuery[144] = "\0";
    qboolean        filterActive = qfalse;
    qboolean        chatFromConsole = qfalse;

    memset(filterQuery, 0, sizeof(filterQuery));
    strcpy(filterQuery, " ");

    // Convert the listname to the real, capitalized, list name.
    Q_strncpyz(listNameReal, listName, sizeof(listNameReal));

    if (strlen(listNameReal) < 1)
        return filterQuery;

    listNameReal[0] -= 32; // Convert first letter to uppercase.
    if (adm && adm->client)
        Q_strcat(buf2, sizeof(buf2), va("^3[%s]^7\n", listNameReal));
    else
        Com_Printf("^3[%s]^7\n", listNameReal);

    // Only check for filters if the argument count is > 2 and we're working in the console.
    if (adm && adm->client){
        Q_strcat(buf2, sizeof(buf2), "^5[Filter options]^7\n\n");
        Q_strcat(buf2, sizeof(buf2), va("^5%-21s Value\n" \
                                        "^7--------------------------------------------------\n", "Filter"));
    }else{
        Com_Printf("^5[Filter options]^7\n\n");
        Com_Printf("^5%-21s Value\n" \
                    "^7--------------------------------------------------\n", "Filter");
    }

    // Get amount of arguments.
    if (shortCmd){
        argCount = G_GetChatArgumentCount();

        if (!argCount) {
            argCount = trap_Argc();
            chatFromConsole = qtrue;
        }
    }else{
        argCount = trap_Argc();
    }

    if(adm && !shortCmd && argCount > 2 || adm && shortCmd && argCount > 0 || !adm && argCount > 1){
        if(shortCmd && !chatFromConsole){
            rc = argNum -1;
        }else{
            rc = argNum;
        }

        while(rc <= argCount){
            memset(arg, 0, sizeof(arg));
            memset(arg2, 0, sizeof(arg2));

            if (!shortCmd || shortCmd && chatFromConsole) {
                trap_Argv(rc, arg, sizeof(arg));
                trap_Argv(rc + 1, arg2, sizeof(arg2));
            }else{
                Q_strncpyz(arg, G_GetChatArgument(rc), sizeof(arg));
                Q_strncpyz(arg2, G_GetChatArgument(rc + 1), sizeof(arg2));
            }

            Q_strlwr(arg);
            Q_strlwr(arg2);

            if(!strstr(arg, "-") || !strstr(arg, "-h") && strlen(arg2) == 0){
                break;
            }else{ // Valid argument it seems, so far.
                if(strstr(arg, "-h")){ // Client wants help with this, no problem.
                    if(adm && adm->client){
                        Q_strcat(buf2, sizeof(buf2), va(\
                                                        "%-27s ^7There are several filter options for you to use:\n" \
                                                        "%-27s ^7Filters on IP.\n" \
                                                        "%-27s ^7Filters on name.\n" \
                                                        "%-27s ^7Filters on %s by.\n\n" \
                                                        "%-27s ^7/adm %s -i 172.16 -n boe -b RCON\n",
                                                    "^7[^1Help^7]", "^7[^5-i^7]", "^7[^5-n^7]", "^7[^5-b^7]", byFieldName, "^7[^5Example usage^7]", listName));

                        trap_SendServerCommand( adm-g_entities, va("print \"%s\nUse ^3[Page Up] ^7and ^3[Page Down] ^7keys to scroll\n\n\"", buf2));
                    }else{
                        Com_Printf("%-27s ^7There are several filter options for you to use:\n" \
                                    "%-27s ^7Filters on IP.\n" \
                                    "%-27s ^7Filters on name.\n" \
                                    "%-27s ^7Filters on %s by.\n\n" \
                                    "%-27s ^7/adm %s -i 172.16 -n boe -b RCON\n",
                                    "^7[^1Help^7]", "^7[^5-i^7]", "^7[^5-n^7]", "^7[^5-b^7]", byFieldName, "^7[^5Example usage^7]", listName);

                        Com_Printf("\nUse ^3[Page Up] ^7and ^3[Page Down] ^7keys to scroll\n\n");
                    }

                    return NULL;
                }else if(strstr(arg, "-i")){ // Client wants to filter on an IP.
                    strcpy(filterIP, arg2);
                    filterActive = qtrue;
                }else if(strstr(arg, "-n")){ // Client wants to filter on a name.
                    strcpy(filterName, arg2);
                    filterActive = qtrue;
                }else if(strstr(arg, "-b")){ // Client wants to filter on by.
                    strcpy(filterBy, arg2);
                    filterActive = qtrue;
                }else{ // Invalid argument, break.
                    if(adm){
                        Q_strcat(buf2, sizeof(buf2), va("%-27s ^7Invalid argument: %s\n", "^7[^1Error^7]", arg));
                    }else{
                        Com_Printf("%-27s ^7Invalid argument: %s\n", "^7[^1Error^7]", arg);
                    }

                    break;
                }
            }

            rc += 2;
        }
    }

    if(!filterActive){
        if(adm){
            Q_strcat(buf2, sizeof(buf2), va("%-27s ^7Call the %s with -h for more information\n\n", "^7[^5None applied^7]", listName));
        }else{
            Com_Printf("%-27s ^7Call the %s with -h for more information\n\n", "^7[^5None applied^7]", listName);
        }
    }else{
        // Prepare query as well.
        strcat(filterQuery, " WHERE ");
        rc = 0;

        if(strlen(filterIP) > 0){
            Boe_convertNonSQLChars(filterIP);
            if(adm){
                Q_strcat(buf2, sizeof(buf2), va("%-27s ^7%s\n", "^7[^5IP^7]", filterIP));
            }else{
                Com_Printf("%-27s ^7%s\n", "^7[^5IP^7]", filterIP);
            }
            strcat(filterQuery, va("IP LIKE '%%%s%%'", filterIP));
            rc++;
        }

        if(strlen(filterName) > 0){
            Boe_convertNonSQLChars(filterName);
            if(adm){
                Q_strcat(buf2, sizeof(buf2), va("%-27s ^7%s\n", "^7[^5Name^7]", filterName));
            }else{
                Com_Printf("%-27s ^7%s\n", "^7[^5Name^7]", filterName);
            }
            if(rc){
                strcat(filterQuery, " AND ");
            }
            strcat(filterQuery, va("name LIKE '%%%s%%'", filterName));
            rc++;
        }

        if(strlen(filterBy) > 0){
            Boe_convertNonSQLChars(filterBy);
            if(adm){
                Q_strcat(buf2, sizeof(buf2), va("%-27s ^7%s\n", "^7[^5By^7]", filterBy));
            }else{
                Com_Printf("%-27s ^7%s\n", "^7[^5By^7]", filterBy);
            }
            if(rc){
                strcat(filterQuery, " AND ");
            }
            strcat(filterQuery, va("by LIKE '%%%s%%'", filterBy));
        }

        strcat(filterQuery, " ");
        if(adm){
            Q_strcat(buf2, sizeof(buf2), "\n");
        }else{
            Com_Printf("\n");
        }
    }

    if(adm && adm->client){
        trap_SendServerCommand( adm-g_entities, va("print \"%s\"", buf2));
    }

    return filterQuery;
}

/*
================
adm_showBanList

Show the banlist to the client/RCON.
================
*/

static void adm_showBanList(int argNum, gentity_t *adm, qboolean shortCmd, qboolean subnet)
{
    // We use this in order to send as little packets as possible.
    // Packet max size is 1024 minus some overhead, 1000 char.max should take care of this. (adm only, RCON remains unaffected).
    char             buf2[1000] = "\0", arg[10];
    // SQLite variables.
    sqlite3         *db;
    sqlite3_stmt    *stmt;
    int              rc;
    // We use the following variable for filter options.
    char            *filterQuery;
    qboolean        noFormat = qfalse;

    db = bansDb;
    memset(buf2, 0, sizeof(buf2));

    // Check if we should apply no formatting to the output (API).
    if(!adm){
        trap_Argv(argNum, arg, sizeof(arg));

        if(Q_stricmp(arg, "noformat") == 0){
            noFormat = qtrue;
        }
    }

    if(!noFormat){
        // Check if we can apply filters.
        if(subnet)
            filterQuery = adm_checkListFilters(adm, argNum, shortCmd, "subnetbanlist", "banned");
        else
            filterQuery = adm_checkListFilters(adm, argNum, shortCmd, "banlist", "banned");

        if (!filterQuery) // The call returned NULL, meaning we should quit parsing the list.
            return;
    }else{
        filterQuery = " ";
    }

    if(adm){
        Q_strcat(buf2, sizeof(buf2), va("^3 %-4s %-15s %-15s %-18s By\n" \
                                     "^7------------------------------------------------------------------------\n",
                                     "#", "IP", "Name", "Reason"));
    }else if(!noFormat){
        Com_Printf("^3 %-4s %-15s %-15s %-18s By\n", "#", "IP", "Name", "Reason");
        Com_Printf("^7------------------------------------------------------------------------\n");
    }else{
        Com_Printf("#\tIP\tName\tReason\tBy\n");
    }

    if(subnet){
        rc = sqlite3_prepare(db, va("select ROWID,IP,name,by,reason from subnetbans%sorder by ROWID", filterQuery), -1, &stmt, 0);
    }else{
        rc = sqlite3_prepare(db, va("select ROWID,IP,name,by,reason from bans%sorder by ROWID", filterQuery), -1, &stmt, 0);
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
                    memset(buf2, 0, sizeof(buf2)); // Boe!Man 11/04/11: Empty the buffer.
                }
                Q_strcat(buf2, sizeof(buf2), va("[^3%-3.3i^7] %-15.15s %-15.15s %-18.18s %-15.15s\n", sqlite3_column_int(stmt, 0), sqlite3_column_text(stmt, 1), sqlite3_column_text(stmt, 2), sqlite3_column_text(stmt, 4), sqlite3_column_text(stmt, 3)));
            }else{
                if(!noFormat){
                    Com_Printf("[^3%-3.3d^7] %-15.15s %-15.15s %-18.18s %-15.15s\n", sqlite3_column_int(stmt, 0), sqlite3_column_text(stmt, 1), sqlite3_column_text(stmt, 2), sqlite3_column_text(stmt, 4), sqlite3_column_text(stmt, 3));
                }else{
                    Com_Printf("%d\t%s\t%s\t%s\t%s\n", sqlite3_column_int(stmt, 0), sqlite3_column_text(stmt, 1), sqlite3_column_text(stmt, 2), sqlite3_column_text(stmt, 4), sqlite3_column_text(stmt, 3));
                }
            }
        }
    }

    sqlite3_finalize(stmt);

    // Boe!Man 11/04/11: Fix for RCON not properly showing footer of banlist.
    if(adm && adm->client){
        trap_SendServerCommand( adm-g_entities, va("print \"%s\nUse ^3[Page Up] ^7and ^3[Page Down] ^7keys to scroll\n\n\"", buf2)); // Boe!Man 11/04/11: Also send the last buf2 (that wasn't filled as a whole yet).
    }else if(!noFormat){
        Com_Printf("\nUse ^3[Page Up] ^7and ^3[Page Down] ^7keys to scroll\n\n");
    }else{
        Com_Printf("\n");
    }

    return;
}

/*
================
adm_Ban
12/12/12 - 11:06 PM
Bans a client.
================
*/

int adm_Ban(int argNum, gentity_t *adm, qboolean shortCmd)
{
    int             idNum;
    char            reason[MAX_STRING_CHARS];
    char            clientName[MAX_NETNAME], admName[MAX_NETNAME];
    sqlite3         *db;
    int             start;

    // Boe!Man 1/18/13: Benchmark purposes.
    if (sql_timeBench.integer){
        start = trap_Milliseconds();
    }

    idNum = G_clientNumFromArg(adm, argNum, "ban",
        qfalse, qfalse, qfalse, shortCmd);
    if (idNum < 0) return idNum;

    // Boe!Man 7/21/17: Get the reason.
    if(shortCmd && G_GetChatArgumentCount()){
        Q_strncpyz(reason, GetReason(), sizeof(reason));
    }else{
        Q_strncpyz(reason, ConcatArgs1(argNum + 1), sizeof(reason));
    }

    // Boe!Man 1/9/12: Check for unsupported characters in the reason
    // and replace them.
    Boe_convertNonSQLChars(reason);

    // Boe!Man 12/12/12: Also check those in the names, SQLite has massive
    // problems when using quotes in the (updated) query.
    Q_strncpyz(clientName, g_entities[idNum].client->pers.cleanName,
        sizeof(clientName));
    Boe_convertNonSQLChars(clientName);
    if (adm && adm->client){
        Q_strncpyz(admName, adm->client->pers.cleanName, sizeof(admName));
        Boe_convertNonSQLChars(admName);
    }

    // Boe!Man 12/12/12: Open database.
    db = bansDb;

    // Boe!Man 12/12/12: Insert query.
    if (adm && adm->client){
        if (sqlite3_exec(db, va("INSERT INTO bans (IP, name, by, reason) " \
            "values ('%s', '%s', '%s', '%s')",
            g_entities[idNum].client->pers.ip, clientName, admName, reason),
            0, 0, 0) != SQLITE_OK)
        {
            trap_SendServerCommand(adm-g_entities,
                va("print \"^1[Error] ^7bans database: %s\n",
                sqlite3_errmsg(db)));
            return -1;
        }
    }else{
        if (sqlite3_exec(db, va("INSERT INTO bans (IP, name, by, reason) " \
            "values ('%s', '%s', '%s', '%s')",
            g_entities[idNum].client->pers.ip, clientName, "RCON", reason),
            0, 0, 0) != SQLITE_OK)
        {
            G_LogPrintf("^1Error: ^7bans database: %s\n", sqlite3_errmsg(db));
            return -1;
        }
    }

    // Kick the player and inform them what happened.
    G_kickPlayer(&g_entities[idNum], adm, "banned", reason);

    if(sql_timeBench.integer){
        Com_Printf("Ban took: %ims\n", trap_Milliseconds() - start);
    }

    return idNum;
}

/*
================
adm_subnetBan
12/16/12 - 11:36 AM
Subnet bans a client.
================
*/

int adm_subnetBan(int argNum, gentity_t *adm, qboolean shortCmd)
{
    int         idNum;
    char        reason[MAX_STRING_CHARS];
    char        clientName[MAX_NETNAME], admName[MAX_NETNAME];
    char        ip[MAX_IP];
    sqlite3     *db;

    idNum = G_clientNumFromArg(adm, argNum, "subnetban", qfalse,
        qfalse, qfalse, shortCmd);
    if (idNum < 0) return idNum;

    // Store the 'subnet' in a seperate char array.
    Q_strncpyz(ip, g_entities[idNum].client->pers.ip, 7);

    // Boe!Man 7/21/17: Get the reason.
    if(shortCmd && G_GetChatArgumentCount()){
        Q_strncpyz(reason, GetReason(), sizeof(reason));
    }else{
        Q_strncpyz(reason, ConcatArgs1(argNum + 1), sizeof(reason));
    }

    // Check for unsupported characters in the reason and replace them.
    Boe_convertNonSQLChars(reason);

    // Also check those in the names, SQLite has massive problems when
    // using quotes in the (updated) query.
    Q_strncpyz(clientName, g_entities[idNum].client->pers.cleanName,
        sizeof(clientName));
    Boe_convertNonSQLChars(clientName);

    if(adm && adm->client){
        Q_strncpyz(admName, adm->client->pers.cleanName, sizeof(admName));
        Boe_convertNonSQLChars(admName);
    }

    // Boe!Man 5/27/13: Open database.
    db = bansDb;

    // Boe!Man 12/12/12: Insert query.
    if(adm && adm->client){
        if(sqlite3_exec(db, va("INSERT INTO subnetbans (IP, name, " \
            "by, reason) values ('%s', '%s', '%s', '%s')",
            ip, clientName, admName, reason), 0, 0, 0) != SQLITE_OK)
        {
            trap_SendServerCommand( adm-g_entities,
                va("print \"^1[Error] ^7bans database: %s\n",
                sqlite3_errmsg(db)));
            return -1;
        }
    }else{
        if(sqlite3_exec(db, va("INSERT INTO subnetbans (IP, name, " \
            "by, reason) values ('%s', '%s', '%s', '%s')",
            ip, clientName, "RCON", reason), 0, 0, 0) != SQLITE_OK)
        {
            G_LogPrintf("^1Error: ^7bans database: %s\n", sqlite3_errmsg(db));
            return -1;
        }
    }

    // Kick the player and inform them what happened.
    G_kickPlayer(&g_entities[idNum], adm, "subnetbanned", reason);

    return idNum;
}

/*
================
adm_Broadcast

Broadcasts a message to all clients.
================
*/

int adm_Broadcast(int argNum, gentity_t *adm, qboolean shortCmd)
{
    char buffer[512];
    char buffer1[512];
    char *buffer2;
    int i, z = 0;

    if (shortCmd){
        trap_Argv(argNum - 1, buffer, sizeof(buffer));
        buffer2 = ConcatArgs1(argNum);
        if (strlen(buffer2) < 1){
            // If someone's trying to broadcast nothing, buffer start will be 0. Don't allow admins to broadcast the actual "!br" command.
            if (StartAfterCommand(buffer) != 0){
                for (i = StartAfterCommand(buffer); i <= strlen(buffer); i++){
                    buffer1[z] = buffer[i];
                    z += 1;
                }
            }
            buffer1[z] = '\0';
        }else{
            strcpy(buffer1, buffer2);
        }
    }else{
        if(adm && adm->client){
            strcpy(buffer1, ConcatArgs1(argNum));
        }else{
            strcpy(buffer1, ConcatArgs1(argNum));
        }
    }

    // Boe!Man 11/16/15: Parse the tokens from the chat if we're broadcasting in-game.
    if(adm && adm->client){
        Boe_Tokens(adm, buffer1, SAY_TELL, qfalse);
    }

    // Broadcast the message.
    Boe_GlobalSound(G_SoundIndex("sound/misc/menus/invalid.wav"));
    G_Broadcast(buffer1, BROADCAST_CMD, NULL);

    if (adm && adm->client){
        trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Broadcast by %s.\n\"", adm->client->pers.cleanName));
    }else{
        trap_SendServerCommand(-1, "print\"^3[Rcon Action] ^7Broadcast.\n\"");
    }

    return -1;
}

/*
================
adm_evenTeams

Evens the team balance.
================
*/

int adm_evenTeams(int argNum, gentity_t *adm, qboolean shortCmd)
{
    EvenTeams(adm, qfalse);

    return -1;
}

/*
================
adm_clanVsAll

Forces all clan members in one team.
================
*/

int adm_clanVsAll(int argNum, gentity_t *adm, qboolean shortCmd)
{
    int             counts[TEAM_NUM_TEAMS] = { 0 };
    int             argc, i, clanTeam, othersTeam;
    char            team[6];
    clientSession_t *sess;
    gentity_t       *ent;

    // Verify that we're playing a team game before continuing.
    if (!level.gametypeData->teams){
        G_printInfoMessage(adm, "Not playing a team game.");
        return -1;
    }

    argc = G_GetChatArgumentCount();

    if (!shortCmd || shortCmd && !argc){
        trap_Argv(argNum, team, sizeof(team));
    }else{
        Q_strncpyz(team, G_GetChatArgument(1), sizeof(team));
    }

    // Which team has the most clan members on it?
    for (i = 0; i < level.numConnectedClients; i++) {
        sess = &g_entities[level.sortedClients[i]].client->sess;

        if (!sess->clanMember)
            continue;
        if (sess->team != TEAM_RED && sess->team != TEAM_BLUE)
            continue;

        counts[sess->team]++;
    }

    if (team[0] == 'r' || team[0] == 'R'){
        clanTeam = TEAM_RED;
        othersTeam = TEAM_BLUE;
    }else if (team[0] == 'b' || team[0] == 'B'){
        clanTeam = TEAM_BLUE;
        othersTeam = TEAM_RED;
    }else{
        if (counts[TEAM_RED] >= counts[TEAM_BLUE]){
            clanTeam = TEAM_RED;
            othersTeam = TEAM_BLUE;
        }else{
            clanTeam = TEAM_BLUE;
            othersTeam = TEAM_RED;
        }
    }

    for (i = 0; i < level.numConnectedClients; i++){
        ent = &g_entities[level.sortedClients[i]];
        sess = &ent->client->sess;

        if (sess->team != TEAM_RED && sess->team != TEAM_BLUE)
            continue;

        if (sess->clanMember){
            if (sess->team != clanTeam){
                if (!G_IsClientDead(ent->client)){
                    ent->client->ps.stats[STAT_WEAPONS] = 0;
                    TossClientItems(ent);
                }

                G_StartGhosting(ent);
                sess->team = (team_t)clanTeam;
            }else{
                continue;
            }
        }else{
            if (sess->team != othersTeam) {
                if (!G_IsClientDead(ent->client)){
                    ent->client->ps.stats[STAT_WEAPONS] = 0;
                    TossClientItems(ent);
                }

                G_StartGhosting(ent);
                sess->team = (team_t)othersTeam;
            }else{
                continue;
            }
        }

        if (ent->r.svFlags & SVF_BOT){
            char    userinfo[MAX_INFO_STRING];

            trap_GetUserinfo(ent->s.number, userinfo, sizeof(userinfo));
            Info_SetValueForKey(userinfo, "team", sess->team == TEAM_RED ? "red" : "blue");
            trap_SetUserinfo(ent->s.number, userinfo);
        }

        ent->client->pers.identity = NULL;
        ClientUserinfoChanged(ent->s.number);
        CalculateRanks();

        G_StopFollowing(ent);
        G_StopGhosting(ent);
        trap_UnlinkEntity(ent);
        ClientSpawn(ent);
    }

    if (clanTeam == TEAM_BLUE){
        level.blueLocked = 1;
        level.redLocked = 0;
    }else{
        level.redLocked = 1;
        level.blueLocked = 0;
    }

    // Tell everyone what just happened.
    Boe_GlobalSound(G_SoundIndex("sound/misc/events/tut_lift02.mp3"));
    G_Broadcast("\\Clan vs all!", BROADCAST_CMD, NULL);

    if(adm && adm->client){
        Boe_adminLog("clan vs all", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), "none");
        trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Clan vs all by %s.\n\"", adm->client->pers.cleanName));
    }else{
        Boe_adminLog("clan vs all", "RCON", "none");
        trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7Clan vs all.\n\""));
    }

    return -1;
}

/*
================
adm_swapTeams

Swaps all clients from the blue team to the red one, and vice versa.
================
*/

int adm_swapTeams(int argNum, gentity_t *adm, qboolean shortCmd)
{
    SwapTeams(adm, qfalse);

    return -1;
}

/*
================
adm_lockTeam

Locks or unlocks specified team.
================
*/

int adm_lockTeam(int argNum, gentity_t *adm, qboolean shortCmd)
{
    char        arg[16] = "\0"; // increase buffer so we can process more commands

    trap_Argv(argNum, arg, sizeof(arg));

    if (shortCmd){
        if(!G_GetChatArgumentCount() || !G_lockTeam(adm, qfalse, G_GetChatArgument(1))){
            G_printInfoMessage(adm, "Unknown team entered.");
        }
    }else{
        if (!G_lockTeam(adm, qfalse, arg)){
            G_printInfoMessage(adm, "Unknown team entered.");
        }
    }

    return -1;
}

/*
================
adm_Flash

Flashes a player.
================
*/

int adm_Flash(int argNum, gentity_t *adm, qboolean shortCmd)
{
    char        arg[16] = "\0"; // increase buffer so we can process more commands
    qboolean    all = qfalse;
    int         idNum, i, nadeDir, weapon;
    gentity_t   *targ, *missile;
    vec3_t      dir;
    float       x, y;

    if (!shortCmd || shortCmd && !G_GetChatArgumentCount()){
        trap_Argv(argNum, arg, sizeof(arg));
    }else{
        Q_strncpyz(arg, G_GetChatArgument(1), sizeof(arg));
    }

    Q_strlwr(arg);

    // Check for "all".
    if (strcmp(arg, "all") == 0){
        all = qtrue;
    }else{
        idNum = G_clientNumFromArg(adm, argNum, "flash", qtrue, qtrue, qtrue, shortCmd);
        if (idNum < 0) return idNum;

        targ = g_entities + idNum;
    }

    weapon = WP_M84_GRENADE;
    nadeDir = 1;
    for (i = 0; i < 1; i++) {
        x = 100 * cos(DEG2RAD(nadeDir * i));
        y = 100 * sin(DEG2RAD(nadeDir * i));
        VectorSet(dir, x, y, 100);
        dir[2] = 300;
    }

    if (all){
        for (i = 0; i<level.numConnectedClients; i++){
            missile = NV_projectile(&g_entities[level.sortedClients[i]], g_entities[level.sortedClients[i]].r.currentOrigin, dir, weapon, 0);
            missile->nextthink = level.time + 250;
        }

        Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
        if (adm && adm->client){
            G_Broadcast(va("Everyone\nhas been \\flashed by %s", adm->client->pers.netname), BROADCAST_CMD, NULL);
            trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Everyone has been flashed by %s.\n\"", adm->client->pers.cleanName));
            Boe_adminLog("flash", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), "all clients");
        }else{
            G_Broadcast("Everyone\nhas been \\flashed", BROADCAST_CMD, NULL);
            trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7Everyone has been flashed.\n\""));
            Boe_adminLog("flash", "RCON", "all clients");
        }

        return -1;
    }

    missile = NV_projectile(targ, targ->r.currentOrigin, dir, weapon, 0);
    missile->nextthink = level.time + 250;

    return idNum;
}

/*
================
adm_Gametype

Switches the current map to another gametype, or displays current gametype.
================
*/

int adm_Gametype(int argNum, gentity_t *adm, qboolean shortCmd)
{
    char        gametype[8];
    char        arg[16] = "\0";
    int         argc;

    if (shortCmd) {
        argc = G_GetChatArgumentCount();
    }

    if (!shortCmd || shortCmd && !argc){
        trap_Argv(argNum, arg, sizeof(arg));
    }else{
        Q_strncpyz(arg, G_GetChatArgument(1), sizeof(arg));
    }

    // Boe!Man 2/4/11: Adding support for uppercase arguments.
    Q_strlwr(arg);

    if (level.mapSwitch == qfalse){
        if (strstr(arg, "ctf")){
            trap_SendConsoleCommand(EXEC_APPEND, "g_gametype ctf\n");
            strcpy(gametype, "ctf");
            G_Broadcast("\\Gametype Capture the Flag!", BROADCAST_CMD, NULL);
        }else if (strstr(arg, "inf")){
            trap_SendConsoleCommand(EXEC_APPEND, "g_gametype inf\n");
            // Boe!Man 10/4/12: Fix latch CVAR crap. It's either h&s or h&z, so ensure the latched value is GONE so we can properly reset it.
            if (current_gametype.value == GT_HS || current_gametype.value == GT_HZ){
                trap_Cvar_Set("g_gametype", "h&s");
                trap_Cvar_Update(&g_gametype);
                trap_Cvar_Set("g_gametype", "h&z");
                trap_Cvar_Update(&g_gametype);
                trap_Cvar_Set("g_gametype", "inf");
                trap_Cvar_Update(&g_gametype);
            }
            strcpy(gametype, "inf");
            G_Broadcast("\\Gametype Infiltration!", BROADCAST_CMD, NULL);
        }else if (strstr(arg, "tdm")){
            trap_SendConsoleCommand(EXEC_APPEND, "g_gametype tdm\n");
            strcpy(gametype, "tdm");
            G_Broadcast("\\Gametype Team Deathmatch!", BROADCAST_CMD, NULL);
        }else if (strstr(arg, "dm")){
            trap_SendConsoleCommand(EXEC_APPEND, "g_gametype dm\n");
            strcpy(gametype, "dm");
            G_Broadcast("\\Gametype Deathmatch!", BROADCAST_CMD, NULL);
        }else if (strstr(arg, "elim")){
            trap_SendConsoleCommand(EXEC_APPEND, "g_gametype elim\n");
            strcpy(gametype, "elim");
            G_Broadcast("\\Gametype Elimination!", BROADCAST_CMD, NULL);
        #ifdef _GOLD
        }else if (strstr(arg, "dem")){
            trap_SendConsoleCommand(EXEC_APPEND, "g_gametype dem\n");
            strcpy(gametype, "dem");
            G_Broadcast("\\Gametype Demolition!", BROADCAST_CMD, NULL);
        }else if (g_enforce1fxAdditions.integer && strstr(arg, "h&s")){
        #else
        }else if (strstr(arg, "h&s")){
        #endif // _GOLD
            trap_SendConsoleCommand(EXEC_APPEND, "g_gametype h&s\n");
            strcpy(gametype, "h&s");
            G_Broadcast("\\Gametype Hide&Seek!", BROADCAST_CMD, NULL);
        #ifdef _GOLD
        }else if (g_enforce1fxAdditions.integer && (strstr(arg, "h&z") || strstr(arg, "zombies"))){
        #else
        }else if (strstr(arg, "h&z") || strstr(arg, "zombies")){
        #endif // _GOLD
            trap_SendConsoleCommand(EXEC_APPEND, "g_gametype h&z\n");
            strcpy(gametype, "h&z");
            G_Broadcast("\\Gametype Zombies!", BROADCAST_CMD, NULL);
        #ifdef _GOLD
        }else if (!g_enforce1fxAdditions.integer && (strstr(arg, "h&s") || strstr(arg, "h&z") || strstr(arg, "zombies"))){
            G_printInfoMessage(adm, "This gametype is unavailable when you're not enforcing 1fx. Client Additions.");
            G_printInfoMessage(adm, "Put g_enforce1fxAdditions to 1 and restart the map to enable the additional gametypes.");
        #endif // GOLD
        }else{
            // Boe!Man 2/4/11: In case no argument is found we just display the current gametype.
            if (strstr(g_gametype.string, "inf")){
                if (current_gametype.value == GT_HS){
                    G_printInfoMessage(adm, "Unknown gametype. Gametype is: h&s.");
                }else if (current_gametype.value == GT_HZ){
                    G_printInfoMessage(adm, "Unknown gametype. Gametype is: h&z.");
                }else{
                    G_printInfoMessage(adm, "Unknown gametype. Gametype is: inf.");
                }
            }else{
                G_printInfoMessage(adm, "Unknown gametype. Gametype is: %s.", g_gametype.string);
            }
            return -1;
        }
    }else{
        if (level.mapAction == 1 || level.mapAction == 3){
            G_printInfoMessage(adm, "A map restart is already in progress.");
        }else if (level.mapAction == 2 || level.mapAction == 4){
            G_printInfoMessage(adm, "A map switch is already in progress.");
        }else if(level.mapAction == 5){
            G_printInfoMessage(adm, "The map is already ending.");
        }

        return -1;
    }

    Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
    if (adm && adm->client){
        trap_SendServerCommand(-1, va("print \"^3[Admin Action] ^7Gametype changed to %s by %s.\n\"", gametype, adm->client->pers.cleanName));
        Boe_adminLog(va("gametype - %s", gametype), va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), "none");
    }
    else{
        trap_SendServerCommand(-1, va("print \"^3[Rcon Action] ^7Gametype changed to %s.\n\"", gametype));
        Boe_adminLog(va("gametype - %s", gametype), "RCON", "none");
    }

    level.mapSwitch = qtrue;
    level.mapAction = 3;
    level.mapSwitchCount = level.time;

    return -1;
}

/*
================
adm_Pause

Pauses the game, or unpauses it when paused.
================
*/

int adm_Pause(int argNum, gentity_t *adm, qboolean shortCmd)
{
    int         i;
    gentity_t   *ent;

    if (level.intermissiontime || level.intermissionQueued){
        return -1;
    }else if (level.pause){
        adm_unPause(adm);

        return -1;
    }

    level.pause = 6;

    // When in CTF, flags should be freezed as well. Let the gt know this by sending this command.
    if (current_gametype.value == GT_CTF){
        trap_GT_SendEvent(GTEV_PAUSE, level.time, 1, 0, 0, 0, 0);
    }

    for (i = 0; i< level.maxclients; i++)
    {
        ent = g_entities + i;
        if (!ent->inuse)
            continue;

        if (G_IsClientDead(ent->client)){
            ent->client->sess.pausespawn = qtrue;
            ent->client->ps.oldTimer = ent->client->ps.respawnTimer - level.time;
        }
        ent->client->ps.pm_type = PM_INTERMISSION;
    }

    // Send the current scoring to all clients.
    SendScoreboardMessageToAllClients();

    // Tell everyone what just happened.
    Boe_GlobalSound(G_SoundIndex("sound/misc/events/buzz02.wav"));
    G_Broadcast("\\Paused!", BROADCAST_CMD, NULL);

    if (adm && adm->client){
        Boe_adminLog("paused", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), "none");
        trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Paused by %s.\n\"", adm->client->pers.cleanName));
    }else{
        Boe_adminLog("paused", "RCON", "none");
        trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7Paused.\n\""));
    }

    return -1;
}

/*
================
adm_unPause

Unpauses the game.
================
*/

static void adm_unPause(gentity_t *adm)
{
    // Boe!Man 1/24/11: Tell everyone what's about to happen.
    Boe_GlobalSound(G_SoundIndex("sound/misc/events/buzz02.wav"));

    if (adm && adm->client){
        Boe_adminLog("unpause", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), "none");
        trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Unpause by %s.\n\"", adm->client->pers.cleanName));
    }else{
        trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7Unpaused.\n\""));
        Boe_adminLog("unpause", "RCON", "none");
    }

    G_unPause(adm);
}

/*
================
adm_Unban

Unbans a client.
================
*/

int adm_Unban(int argNum, gentity_t *adm, qboolean shortCmd)
{
    char    arg[32];

    memset(arg, 0, sizeof(arg));

    if (shortCmd && G_GetChatArgumentCount()){
        strncpy(arg, G_GetChatArgument(1), sizeof(arg));
    }else{
        trap_Argv(argNum, arg, sizeof(arg));
    }

    adm_unbanFromDatabase(adm, arg, qfalse);

    return -1;
}

/*
================
adm_subnetUnban

Subnet unbans a client.
================
*/

int adm_subnetUnban(int argNum, gentity_t *adm, qboolean shortCmd)
{
    char    arg[32];

    memset(arg, 0, sizeof(arg));

    if (shortCmd && G_GetChatArgumentCount()){
        strncpy(arg, G_GetChatArgument(1), sizeof(arg));
    }else{
        trap_Argv(argNum, arg, sizeof(arg));
    }

    adm_unbanFromDatabase(adm, arg, qtrue);

    return -1;
}

/*
==================
adm_unbanFromDatabase
12/17/12 - 6:12 PM
Unbans player from the database.
==================
*/

static void adm_unbanFromDatabase(gentity_t *adm, char *ip, qboolean subnet)
{
    int             iLine, rc;
    sqlite3         *db;
    sqlite3_stmt    *stmt;
    char            name[MAX_NETNAME];
    char            ip2[MAX_IP];


    if(strlen(ip) < 2 && strstr(ip, ".")){
        G_printInfoMessage(adm, "Invalid IP, usage: adm unban <IP/Line>.");
        return;
    }

    // Open the database.
    db = bansDb;

    // Delete by line/record.
    if(ip[0] && !strstr(ip, ".") && !strstr(ip, "bot")){
        // Unban record from database.
        iLine = atoi(ip);

        // First check if the record exists.
        if(!subnet){
            rc = sqlite3_prepare(db, va("select IP,name from bans where ROWID='%i' LIMIT 1", iLine), -1, &stmt, 0);
        }else{
            rc = sqlite3_prepare(db, va("select IP,name from subnetbans where ROWID='%i' LIMIT 1", iLine), -1, &stmt, 0);
        }

        // If the previous query failed, we're looking at a record that does not exist.
        if(rc != SQLITE_OK){
            if(adm && adm->client){
                trap_SendServerCommand( adm-g_entities, va("print \"^1[Error] ^7bans database: %s\n", sqlite3_errmsg(db)));
            }else{
                G_LogPrintf("^1Error: ^7bans database: %s\n", sqlite3_errmsg(db));
            }

            sqlite3_finalize(stmt);
            return;
        }else if((rc = sqlite3_step(stmt)) == SQLITE_DONE){
            G_printInfoMessage(adm, "Could not find line %d.", iLine);
            sqlite3_finalize(stmt);
            return;
        }else{ // Store info for the unban line given to the Admin (to let him know it went correctly).
            Q_strncpyz(ip2, (char *)sqlite3_column_text(stmt, 0), sizeof(ip2));
            Q_strncpyz(name, (char *)sqlite3_column_text(stmt, 1), sizeof(name));
        }
        sqlite3_finalize(stmt);

        // If the previous query succeeded, we can delete the record.
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
            G_printInfoMessage(adm, "Unbanned %s (IP: %s) from line %d.", name, ip2, iLine);
        }
    }else if(strlen(ip) < 3){
        G_printInfoMessage(adm, "Invalid IP, usage: adm unban <IP/Line>.");
        return;
    }else{ // Delete by full IP.
        // First check if the record exists.
        if(!subnet){
            rc = sqlite3_prepare(db, va("select ROWID,name from bans where IP='%s' LIMIT 1", ip), -1, &stmt, 0);
        }else{
            rc = sqlite3_prepare(db, va("select ROWID,name from subnetbans where IP='%s' LIMIT 1", ip), -1, &stmt, 0);
        }

        // If the previous query failed, we're looking at a record that does not exist.
        if(rc != SQLITE_OK){
            if(adm && adm->client){
                trap_SendServerCommand( adm-g_entities, va("print \"^1[Error] ^7bans database: %s\n", sqlite3_errmsg(db)));
            }else{
                G_LogPrintf("^1Error: ^7bans database: %s\n", sqlite3_errmsg(db));
            }

            sqlite3_finalize(stmt);
            return;
        }else if((rc = sqlite3_step(stmt)) == SQLITE_DONE){
            G_printInfoMessage(adm, "Could not find IP %s.", ip);
            sqlite3_finalize(stmt);
            return;
        }else{ // Store info for the unban line given to the Admin (to let him know it went correctly).
            Q_strncpyz(ip2, (char *)sqlite3_column_text(stmt, 0), sizeof(ip2)); // ID in this case.
            Q_strncpyz(name, (char *)sqlite3_column_text(stmt, 1), sizeof(name));
        }

        sqlite3_finalize(stmt);

        // If the previous query succeeded, we can delete the record.
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
            G_printInfoMessage(adm, "Unbanned %s (IP: %s) from line %s.", name, ip, ip2);
            // Copy the actual IP to ip2, for logging purposes.
            Q_strncpyz(ip2, ip, sizeof(ip2));
        }
    }


    // Re-order the ROWIDs by issuing the VACUUM maintenance query.
    sqlite3_exec(db, "VACUUM", NULL, NULL, NULL);

    // Log the unban.
    if(subnet){
        if(adm && adm->client){
            Boe_adminLog ("subnet unban", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", ip2, name));
        }else{
            Boe_adminLog ("subnet unban", "RCON", va("%s\\%s", ip2, name));
        }
    }else{
        if(adm && adm->client){
            Boe_adminLog ("unban", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", ip2, name));
        }else{
            Boe_adminLog ("unban", "RCON", va("%s\\%s", ip2, name));
        }
    }
}

/*
==================
adm_passVote

Passes a running vote.
==================
*/

int adm_passVote(int argNum, gentity_t *adm, qboolean shortCmd)
{
    if ( !level.voteTime ){
        G_printInfoMessage(adm, "No vote in progress.");
        return -1;
    }

    // Let the vote pass.
    if(strcmp(level.voteString, "poll") != 0){
        // Only set the voters who voted yes to a higher number
        // if we're not doing a poll.
        level.voteYes = level.numVotingClients / 2 + 1;
    }

    level.forceVote = qtrue;

    // Let everybody know what happened.
    Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
    G_Broadcast("\\Vote passed!", BROADCAST_CMD, NULL);

    if(adm && adm->client){
        trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Vote passed by %s.\n\"", adm->client->pers.cleanName));
        Boe_adminLog ("passvote", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), "none");
    }else{
        trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7Vote passed.\n\""));
        Boe_adminLog ("passvote", "RCON", "none");
    }

    return -1;
}

/*
==================
adm_cancelVote

Cancels a running vote.
==================
*/

int adm_cancelVote(int argNum, gentity_t *adm, qboolean shortCmd)
{
    if ( !level.voteTime ){
        G_printInfoMessage(adm, "No vote in progress.");
        return -1;
    }

    // Cancel the vote.
    level.voteTime = 0;
    trap_SetConfigstring( CS_VOTE_TIME, "" );

    // Let everybody know what happened..
    Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
    G_Broadcast("\\Vote cancelled!", BROADCAST_CMD, NULL);

    if(adm && adm->client){
        trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Vote cancelled by %s.\n\"", adm->client->pers.cleanName));
        Boe_adminLog ("cancelvote", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), "none");
    }else{
        trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7Vote cancelled.\n\""));
        Boe_adminLog ("cancelvote", "RCON", "none");
    }

    return -1;
}

/*
==================
adm_mapCycle

Issues mapcycle command to go to the next map in the cycle.
==================
*/

int adm_mapCycle(int argNum, gentity_t *adm, qboolean shortCmd)
{
    if (adm && adm->client){
        if(level.mapSwitch == qfalse){
            level.mapSwitch = qtrue;
            level.mapAction = 4;
            level.mapSwitchCount = level.time;
            level.mapSwitchCount2 = 5; // 5 seconds remaining on the timer.

            Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
            Boe_adminLog("mapcycle", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), "none");
            trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Mapcycle by %s.\n\"", adm->client->pers.cleanName));
        }else{
            if(level.mapAction == 1 || level.mapAction == 3){
                G_printInfoMessage(adm, "A map restart is already in progress.");
            }else if(level.mapAction == 2 || level.mapAction == 4){
                G_printInfoMessage(adm, "A map switch is already in progress.");
            }else if(level.mapAction == 5){
                G_printInfoMessage(adm, "The map is already ending.");
            }
        }
    }else{
        if(level.mapSwitch == qfalse){
            level.mapSwitch = qtrue;
            level.mapAction = 4;
            level.mapSwitchCount = level.time;
            level.mapSwitchCount2 = 5; // 5 seconds remaining on the timer.

            Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
            Boe_adminLog("mapcycle", "RCON", "none");
            trap_SendServerCommand(-1, "print\"^3[Rcon Action] ^7Mapcycle.\n\"");
        }else{
            if(level.mapAction == 1 || level.mapAction == 3){
                G_printInfoMessage(NULL, "A map restart is already in progress.");
            }else if(level.mapAction == 2 || level.mapAction == 4){
                G_printInfoMessage(NULL, "A map switch is already in progress.");
            }else if(level.mapAction == 5){
                G_printInfoMessage(NULL, "The map is already ending.");
            }
        }
    }

    return -1;
}

/*
==================
adm_adminList

Show the admin list to the client/RCON.
==================
*/

int adm_adminList(int argNum, gentity_t *adm, qboolean shortCmd)
{
    // We use this in order to send as little packets as possible.
    // Packet max size is 1024 minus some overhead, 1000 char. max should take care of this. (adm only, RCON remains unaffected).
    char             arg[32];
    char             buf2[1000];
    qboolean         passwordList = qfalse; // If this is true, the user wishes to see the passworded Admins.
    // SQLite variables.
    sqlite3         *db;
    sqlite3_stmt    *stmt;
    int              rc;
    // We use the following variable for filter options.
    char            *filterQuery;
    qboolean        noFormat = qfalse;

    // Open users database.
    db = usersDb;
    memset(arg, 0, sizeof(arg));
    memset(buf2, 0, sizeof(buf2));

    // Check for password argument.
    if(shortCmd && G_GetChatArgumentCount()){
        strncpy(arg, G_GetChatArgument(1), sizeof(arg));
    }else{
        trap_Argv(argNum, arg, sizeof(arg));
    }

    if(Q_stricmp(arg, "pass") == 0){
        passwordList = qtrue;
    }

    // If the password login system isn't allowed by the server, by all means, also disallow the showing of the list.
    if(passwordList && !g_passwordAdmins.integer){
        G_printInfoMessage(adm, "Access denied: No password logins allowed by the server!");
        return -1;
    }

    // Check if we should apply no formatting to the output (API).
    if(!adm){
        if(!passwordList){
            trap_Argv(argNum, arg, sizeof(arg));
        }else{
            trap_Argv(argNum + 1, arg, sizeof(arg));
        }

        if(Q_stricmp(arg, "noformat") == 0){
            noFormat = qtrue;
        }
    }

    if(!noFormat){
        // Check if we can apply filters.
        filterQuery = adm_checkListFilters(adm, argNum, shortCmd, "adminlist", "added");
        if (!filterQuery) // The call returned NULL, meaning we should quit parsing the list.
            return -1;
    }else{
        filterQuery = " ";
    }

    // Display header.
    if(adm && adm->client){
        Q_strcat(buf2, sizeof(buf2), va("^3 %-6s%-5s%-16s%-22sBy\n^7------------------------------------------------------------------------\n", "#", "Lvl", "IP", "Name"));
    }else if(!noFormat){
        Com_Printf("^3 %-6s%-5s%-16s%-22sBy\n", "#", "Lvl", "IP", "Name");
        Com_Printf("^7------------------------------------------------------------------------\n");
    }else{
        Com_Printf("#\tLvl\tIP\tName\tBy\n");
    }

    if(!passwordList){
        rc = sqlite3_prepare(db, va("select ROWID,level,IP,name,by from admins%sorder by ROWID", filterQuery), -1, &stmt, 0);
    }else{
        rc = sqlite3_prepare(db, va("select ROWID,level,'',name,by from passadmins%sorder by ROWID", filterQuery), -1, &stmt, 0);
    }
    if(rc!=SQLITE_OK){
        if(adm && adm->client){
            trap_SendServerCommand( adm-g_entities, va("print \"^1[Error] ^7users database: %s\n\"", sqlite3_errmsg(db)));
        }else{
            G_LogPrintf("^1Error: ^7users database: %s\n", sqlite3_errmsg(db));
        }

        return -1;
    }else while((rc = sqlite3_step(stmt)) != SQLITE_DONE){
        if(rc == SQLITE_ROW){
            if(adm && adm->client){
                // Boe!Man 11/04/11: Put packet through to clients if char size would exceed 1000 and reset buf2.
                if((strlen(buf2)+strlen(va("[^3%-3.3i^7]  [^3%i^7]  %-15.15s %-21.21s %-21.21s\n", sqlite3_column_int(stmt, 0), sqlite3_column_int(stmt, 1), sqlite3_column_text(stmt, 2), sqlite3_column_text(stmt, 3), sqlite3_column_text(stmt, 4)))) > 1000){
                    trap_SendServerCommand( adm-g_entities, va("print \"%s\"", buf2));
                    memset(buf2, 0, sizeof(buf2)); // Boe!Man 11/04/11: Properly empty the buffer.
                }
                Q_strcat(buf2, sizeof(buf2), va("[^3%-3.3i^7%-3s[^3%i^7%-3s%-15.15s %-21.21s %-21.21s\n", sqlite3_column_int(stmt, 0), "]", sqlite3_column_int(stmt, 1), "]", sqlite3_column_text(stmt, 2), sqlite3_column_text(stmt, 3), sqlite3_column_text(stmt, 4)));
            }else{
                if(!noFormat){
                    Com_Printf("[^3%-3.3i^7%-3s[^3%i^7%-3s%-15.15s %-21.21s %-21.21s\n", sqlite3_column_int(stmt, 0), "]", sqlite3_column_int(stmt, 1), "]", sqlite3_column_text(stmt, 2), sqlite3_column_text(stmt, 3), sqlite3_column_text(stmt, 4));
                }else{
                    Com_Printf("%d\t%d\t%s\t%s\t%s\n", sqlite3_column_int(stmt, 0), sqlite3_column_int(stmt, 1), sqlite3_column_text(stmt, 2), sqlite3_column_text(stmt, 3),sqlite3_column_text(stmt, 4));
                }
            }
        }
    }

    sqlite3_finalize(stmt);

    // Boe!Man 11/04/11: Fix for RCON not properly showing footer of banlist.
    if(adm && adm->client){
        trap_SendServerCommand( adm-g_entities, va("print \"%s\nUse ^3[Page Up] ^7and ^3[Page Down] ^7keys to scroll\n\n\"", buf2)); // Boe!Man 11/04/11: Also send the last buf2 (that wasn't filled as a whole yet).
    }else if(!noFormat){
        Com_Printf("\nUse ^3[Page Up] ^7and ^3[Page Down] ^7keys to scroll\n\n");
    }else{
        Com_Printf("\n");
    }

    return -1;
}

/*
==================
adm_clanList

Show the clanlist to the client/RCON.
==================
*/

int adm_clanList(int argNum, gentity_t *adm, qboolean shortCmd)
{
    // We use this in order to send as little packets as possible.
    // Packet max size is 1024 minus some overhead, 1000 char. max should take care of this. (adm only, RCON remains unaffected).
    char            arg[32];
    char            buf2[1000];
    // SQLite variables.
    sqlite3         *db;
    sqlite3_stmt    *stmt;
    int             rc;
    // We use the following variable for filter options.
    char            *filterQuery;
    qboolean        noFormat = qfalse;

    // Open database.
    db = usersDb;
    memset(buf2, 0, sizeof(buf2));

    // Check if we should apply no formatting to the output (API).
    if(!adm){
        trap_Argv(argNum, arg, sizeof(arg));

        if(Q_stricmp(arg, "noformat") == 0){
            noFormat = qtrue;
        }
    }

    if(!noFormat){
        // Check if we can apply filters.
        filterQuery = adm_checkListFilters(adm, argNum, shortCmd, "clanlist", "added");
        if (!filterQuery) // The call returned NULL, meaning we should quit parsing the list.
            return -1;
    }else{
        filterQuery = " ";
    }

    // Display header.
    if(adm && adm->client){
        Q_strcat(buf2, sizeof(buf2), va("^3 %-6s%-16s%-22sBy\n^7------------------------------------------------------------------------\n", "#", "IP", "Name"));
    }else if(!noFormat){
        Com_Printf("^3 %-6s%-16s%-22sBy\n", "#", "IP", "Name");
        Com_Printf("^7------------------------------------------------------------------------\n");
    }else{
        Com_Printf("#\tIP\tName\tBy\n");
    }

    rc = sqlite3_prepare(db, va("select ROWID,IP,name,by from clanmembers%sorder by ROWID", filterQuery), -1, &stmt, 0);

    if(rc != SQLITE_OK){
        if(adm && adm->client){
            trap_SendServerCommand( adm-g_entities, va("print \"^1[Error] ^7users database: %s\n\"", sqlite3_errmsg(db)));
        }else{
            G_LogPrintf("^1Error: ^7users database: %s\n", sqlite3_errmsg(db));
        }

        return -1;
    }else while((rc = sqlite3_step(stmt)) != SQLITE_DONE){
        if(rc == SQLITE_ROW){
            if(adm && adm->client){
                // Boe!Man 11/04/11: Put packet through to clients if char size would exceed 1000 and reset buf2.
                if((strlen(buf2)+strlen(va("[^3%-3.3i^7]  %-15.15s %-21.21s %-21.21s\n", sqlite3_column_int(stmt, 0), sqlite3_column_text(stmt, 1), sqlite3_column_text(stmt, 2), sqlite3_column_text(stmt, 3)))) > 1000){
                    trap_SendServerCommand( adm-g_entities, va("print \"%s\"", buf2));
                    memset(buf2, 0, sizeof(buf2)); // Boe!Man 11/04/11: Properly empty the buffer.
                }
                Q_strcat(buf2, sizeof(buf2), va("[^3%-3.3i^7%-3s%-15.15s %-21.21s %-21.21s\n", sqlite3_column_int(stmt, 0),"]", sqlite3_column_text(stmt, 1), sqlite3_column_text(stmt, 2), sqlite3_column_text(stmt, 3)));
            }else{
                if(!noFormat){
                    Com_Printf("[^3%-3.3i^7%-3s%-15.15s %-21.21s %-21.21s\n", sqlite3_column_int(stmt, 0), "]", sqlite3_column_text(stmt, 1), sqlite3_column_text(stmt, 2), sqlite3_column_text(stmt, 3));
                }else{
                    Com_Printf("%d\t%s\t%s\t%s\n", sqlite3_column_int(stmt, 0), sqlite3_column_text(stmt, 1), sqlite3_column_text(stmt, 2), sqlite3_column_text(stmt, 3));
                }
            }
        }
    }

    sqlite3_finalize(stmt);

    // Boe!Man 11/04/11: Fix for RCON not properly showing footer of banlist.
    if(adm && adm->client){
        trap_SendServerCommand( adm-g_entities, va("print \"%s\nUse ^3[Page Up] ^7and ^3[Page Down] ^7keys to scroll\n\n\"", buf2)); // Boe!Man 11/04/11: Also send the last buf2 (that wasn't filled as a whole yet).
    }else if(!noFormat){
        Com_Printf("\nUse ^3[Page Up] ^7and ^3[Page Down] ^7keys to scroll\n\n");
    }else{
        Com_Printf("\n");
    }

    return -1;
}

/*
==================
adm_adminRemove

Removes an Admin from a list.
==================
*/

int adm_adminRemove(int argNum, gentity_t *adm, qboolean shortCmd)
{
    char        arg[32] = "\0";
    char        arg0[32] = "\0";
    char        arg1[32] = "\0";
    char        buf[32] = "\0";
    int         i = 0;
    int         count = 0;
    qboolean    passAdmin = qfalse;

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
                G_printInfoMessage(adm, "Access denied: No password logins allowed by the server!");
                return -1;
            }
        }else{
            trap_Argv( 2, arg0, sizeof( arg0 ) );
            trap_Argv( 3, arg1, sizeof( arg1 ) );
            if(strstr(arg0, "pass") || strstr(arg1, "pass")){
                if(g_passwordAdmins.integer){
                    passAdmin = qtrue;
                }else{
                    G_printInfoMessage(adm, "Access denied: No password logins allowed by the server!");
                    return -1;
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
                G_printInfoMessage(adm, "Access denied: No password logins allowed by the server!");
                return -1;
            }
        }

        if(!strstr(arg, ".")){ // Boe!Man 2/6/13: No dot found, unban by line number.
            Boe_removeAdminFromDb(adm, arg, passAdmin, qtrue, qfalse);
        }else{ // Boe!Man 2/6/13: Dot found, unban by IP.
            Boe_removeAdminFromDb(adm, arg, passAdmin, qfalse, qfalse);
        }
    }

    return -1;
}

/*
==================
adm_friendlyFire

Toggle friendly fire.
==================
*/

int adm_friendlyFire(int argNum, gentity_t *adm, qboolean shortCmd)
{
    qboolean enable = !g_friendlyFire.integer;

    // Toggle state.
    Boe_setTrackedCvar(&g_friendlyFire, enable);

    // Broadcast change.
    Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
    G_Broadcast(va("\\Friendlyfire %s", (enable) ? "enabled!" : "disabled!"), BROADCAST_CMD, NULL);
    if (adm && adm->client){
        Boe_adminLog(va("friendlyfire %s", (enable) ? "enabled" : "disabled"), va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), "none");
        trap_SendServerCommand(-1, va("print \"^3[Admin Action] ^7Friendly fire %s by %s.\n\"", (enable) ? "enabled" : "disabled", adm->client->pers.cleanName));
    }else{
        Boe_adminLog(va("friendly fire %s", (enable) ? "enabled" : "disabled"), "RCON", "none");
        trap_SendServerCommand(-1, va("print \"^3[Rcon Action] ^7Friendly fire %s.\n", (enable) ? "enabled" : "disabled"));
    }

    return -1;
}

/*
==================
adm_Rename

Renames a player.
==================
*/

int adm_Rename(int argNum, gentity_t *adm, qboolean shortCmd)
{
    int idNum;
    char oldName[MAX_NETNAME];
    char oldNameClean[MAX_NETNAME];
    char newName[MAX_NETNAME];
    char userinfo[MAX_INFO_STRING];

    idNum = G_clientNumFromArg(adm, argNum, "rename", qfalse, qfalse, qfalse, shortCmd);
    if (idNum < 0) return idNum;

    // Fetch the new name from the arguments given.
    if(shortCmd){
        strncpy(newName, GetReason(), sizeof(newName));
    }else{
        strncpy(newName, ConcatArgs1(2), sizeof(newName));
    }

    if (!newName || !newName[0] || newName[0] == '\\' || strlen(newName) == 0){
        if (!g_entities[idNum].client->sess.noNameChange){
            G_printInfoMessage(adm, "You cannot set an empty name or unlock someone that's not locked from changing names.");
            return -1;
        }else{
            g_entities[idNum].client->sess.noNameChange = qfalse;
            ClientUserinfoChanged(idNum);

            // Broadcast the unlock.
            G_Broadcast(va("%s\ncan now \\rename again!", g_entities[idNum].client->pers.netname), BROADCAST_CMD, NULL);
            if (adm && adm->client){
                trap_SendServerCommand(-1, va("print \"^3[Admin Action] ^7%s can now rename again.\n\"", g_entities[idNum].client->pers.cleanName));
            }else{
                trap_SendServerCommand(-1, va("print \"^3[Rcon Action] ^7%s can now rename again.\n\"", g_entities[idNum].client->pers.cleanName));
            }

            return -1;
        }
    }else if (strlen(newName) > MAX_NETNAME){
        newName[MAX_NETNAME] = '\0'; // Make sure to NULL terminate it if the argument given is too large.
    }

    trap_GetUserinfo(idNum, userinfo, sizeof(userinfo));
    Info_SetValueForKey(userinfo, "name", newName);
    trap_SetUserinfo(idNum, userinfo);

    // Fetch the old name to use in the broadcasts/logging.
    strncpy(oldName, g_entities[idNum].client->pers.netname, sizeof(oldName));
    strncpy(oldNameClean, g_entities[idNum].client->pers.cleanName, sizeof(oldNameClean));

    G_ClientCleanName(newName, g_entities[idNum].client->pers.netname, sizeof(g_entities[idNum].client->pers.netname), qtrue);
    G_ClientCleanName(newName, g_entities[idNum].client->pers.talkname, sizeof(g_entities[idNum].client->pers.talkname), qtrue);
    G_ClientCleanName(newName, g_entities[idNum].client->pers.cleanName, sizeof(g_entities[idNum].client->pers.cleanName), qfalse);
    g_entities[idNum].client->sess.noNameChange = qtrue;

    ClientUserinfoChanged(idNum);

    // Log and broadcast this change.
    Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
    G_Broadcast(va("%s\nwas \\renamed to %s!", oldName, g_entities[idNum].client->pers.netname), BROADCAST_CMD, NULL);

    if (adm && adm->client){
        Boe_adminLog("renamed", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s\\%s", g_entities[idNum].client->pers.ip, oldNameClean, g_entities[idNum].client->pers.cleanName));
        trap_SendServerCommand(-1, va("print \"^3[Admin Action] ^7%s was renamed to %s by %s.\n\"", oldNameClean, g_entities[idNum].client->pers.cleanName, adm->client->pers.cleanName));
    }
    else{
        Boe_adminLog("renamed", "RCON", va("%s\\%s\\%s", g_entities[idNum].client->pers.ip, oldNameClean, g_entities[idNum].client->pers.cleanName));
        trap_SendServerCommand(-1, va("print \"^3[Rcon Action] ^7%s was renamed to %s.\n", oldNameClean, g_entities[idNum].client->pers.cleanName));
    }

    return -1;
}

/*
==================
adm_Map

Switches the server to the specified map.
==================
*/

int adm_Map(int argNum, gentity_t *adm, qboolean shortCmd)
{
    char            command[64];
    char            map[64];
    char            gametype[8];
    fileHandle_t    f;
    // Alternative actions such as altmaps or devmaps.
    int             altAction       = 0;

    // Reset buffers.
    memset(command, 0, sizeof(command));
    memset(map, 0, sizeof(map));
    memset(gametype, 0, sizeof(gametype));

    // Get specified map and gametype, if given.
    if(shortCmd && G_GetChatArgumentCount()){
        strncpy(command, G_GetChatArgument(argNum - 2), sizeof(command));
        strncpy(map, G_GetChatArgument(argNum - 1), sizeof(map));
        strncpy(gametype, G_GetChatArgument(argNum), sizeof(gametype));
    }else{
        trap_Argv(argNum - 1, command, sizeof(command));
        trap_Argv(argNum, map, sizeof(map));
        trap_Argv(argNum + 1, gametype, sizeof(gametype));
    }

    // Make sure we're checking against strings that are lowercase only.
    Q_strlwr(command);
    Q_strlwr(map);
    Q_strlwr(gametype);

    // Check if a map switch is already in progress.
    if(level.mapSwitch){
        if(level.mapAction == 1 || level.mapAction == 3){
            G_printInfoMessage(adm, "A map restart is already in progress.");
        }else if(level.mapAction == 2 || level.mapAction == 4){
            G_printInfoMessage(adm, "A map switch is already in progress.");
        }else if(level.mapAction == 5){
            G_printInfoMessage(adm, "The map is already ending.");
        }

        return -1;
    }

    // Check for alternative maps to load.
    if(strstr(command, "altmap")){
        trap_Cvar_Set( "g_alternateMap", "1");
        trap_Cvar_Update ( &g_alternateMap );
        altAction = 1;
    }else if(strstr(command, "devmap")){
        altAction = 2; // dev
    }

    // Check if the map exists.
   trap_FS_FOpenFile( va("maps\\%s.bsp", map), &f, FS_READ );
    if(!f){
        G_printInfoMessage(adm, "Map not found.");
        return -1;
    }
    trap_FS_FCloseFile(f);

    // The map is found, did they append a gametype among with it?
    if(strlen(gametype)){
        if(strstr(gametype, "ctf")){
            strcpy(gametype, "ctf");
        }else if(strstr(gametype, "inf")){
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
        }else if(strstr(gametype, "tdm")){
            strcpy(gametype, "tdm");
        }else if(strstr(gametype, "dm")){
            strcpy(gametype, "dm");
        }else if(strstr(gametype, "elim")){
            strcpy(gametype, "elim");
        }else if(strstr(gametype, "h&s")){
            strcpy(gametype, "h&s");
        }else if(strstr(gametype, "h&z")){
            strcpy(gametype, "h&z");
        #ifdef _GOLD
        }else if(strstr(gametype, "dem")){
            strcpy(gametype, "dem");
        #endif // _GOLD
        }else{
            if(current_gametype.value == GT_HS){
                strcpy(gametype, "h&s");
            }else if(current_gametype.value == GT_HZ){
                strcpy(gametype, "h&z");
            }else{
                strcpy(gametype, g_gametype.string);
            }
        }
    }else{
        if(current_gametype.value == GT_HS){
            strcpy(gametype, "h&s");
        }else if(current_gametype.value == GT_HZ){
            strcpy(gametype, "h&z");
        }else{
            strcpy(gametype, g_gametype.string);
        }
    }

    if(g_enforceArenaCheck.integer && !Henk_DoesMapSupportGametype(gametype, map)){
        G_printInfoMessage(adm, "This map does not support the gametype %s, please add it in the ARENA file.", gametype);
        return -1;
    }
    #ifdef _GOLD
    if(!g_enforce1fxAdditions.integer && (strcmp(gametype, "h&s") == 0 || strcmp(gametype, "h&z") == 0)){
        G_printInfoMessage(adm, "This gametype is unavailable when you're not enforcing 1fx. Client Additions.");
        G_printInfoMessage(adm, "Please have someone with RCON access put g_enforce1fxAdditions to 1 and restart the map.");

        return -1;
    }
    #endif // _GOLD

    trap_SendConsoleCommand( EXEC_APPEND, va("g_gametype %s\n", gametype));

    level.mapSwitch = qtrue;
    level.mapAction = 2;
    level.mapSwitchCount = level.time;
    level.mapSwitchCount2 = 5; // 5 seconds remaining on the timer.
    strcpy(level.mapSwitchName, map);

    // Broadcast and logging.
    Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
    if(altAction == 1){
        strncpy(level.mapPrefix, G_ColorizeMessage("\\Altmap"), sizeof(level.mapPrefix));
    }else if(altAction == 2){
        strncpy(level.mapPrefix, G_ColorizeMessage("\\Devmap"), sizeof(level.mapPrefix));
    }else{
        strncpy(level.mapPrefix, G_ColorizeMessage("\\Map"), sizeof(level.mapPrefix));
    }

    if(gametype[0]){ // Boe!Man 2/26/11: If there's actually a gametype found..
        trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Map switch to %s [%s] by %s.\n\"", map, gametype, adm->client->pers.cleanName));
        Boe_adminLog ("map switch", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", map, gametype));
    }else{
        trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Map switch to %s by %s.\n\"", map, adm->client->pers.cleanName));
        Boe_adminLog ("map switch", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), map);
    }

    return -1;
}

/*
==================
adm_Third

Enables or disables thirdperson view.
==================
*/

#ifndef _DEMO

int adm_Third(int argNum, gentity_t *adm, qboolean shortCmd)
{
    qboolean enable = g_allowthirdperson.integer == 0;

    // Enable or disable it via CVAR.
    trap_Cvar_Set("g_allowthirdperson", (enable) ? "1" : "0");

    // Broadcast the change.
    Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
    G_Broadcast(va("\\Thirdperson %s!", (enable) ? "enabled" : "disabled"), BROADCAST_CMD, NULL);
    if (adm && adm->client){
        trap_SendServerCommand(-1, va("print \"^3[Admin Action] ^7Thirdperson %s by %s.\n\"", (enable) ? "enabled" : "disabled", adm->client->pers.cleanName));
        Boe_adminLog(va("3rd %s", (enable) ? "enabled" : "disabled"), va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), "none");
    }else{
        trap_SendServerCommand( -1, va("print \"^3[Rcon Action] ^7Thirdperson %s.\n\"", (enable) ? "enabled" : "disabled"));
        Boe_adminLog(va("3rd %s", (enable) ? "enabled" : "disabled"), "RCON", "none");
    }

    return -1;
}

#endif // not _DEMO

/*
==================
adm_Rounds

Sets or views number of rounds.
==================
*/

int adm_Rounds(int argNum, gentity_t *adm, qboolean shortCmd)
{
    int number;
    number = GetArgument(argNum);

    if(cm_enabled.integer != 1){ // We need to insure Competition Mode is at it's starting stage. Do NOT allow changes to this before or during the scrim.
        G_printInfoMessage(adm, "You can only change this setting during Competition Warmup.");
        return -1;
    }

    if(number <= 0){
        // Show current round value if there's no arg.
        G_printInfoMessage(adm, "Number of rounds for this match: %s.", (cm_dr.integer == 0) ? "1" : "2");
        return -1;
    }else if(number == 1){ // If they want a single round..
        trap_Cvar_Set("cm_dr", "0");

        G_Broadcast("\\One Round!", BROADCAST_CMD, NULL);
        if(adm && adm->client){
            trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Switched to one round by %s.\n\"", adm->client->pers.cleanName));
        }else{
            trap_SendServerCommand(-1, "print\"^3[Rcon Action] ^7Switched to one round.\n\"");
        }
    }else if(number > 1){ // Or two.
        if(number > 2){
            G_printInfoMessage(adm, "Maximum of two is allowed, switching to two.");
        }
        trap_Cvar_Set("cm_dr", "1");

        G_Broadcast("\\Two Rounds!", BROADCAST_CMD, NULL);

        if(adm && adm->client){
            trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Switched to two rounds by %s.\n\"", adm->client->pers.cleanName));
        }else{
            trap_SendServerCommand(-1, "print\"^3[Rcon Action] ^7Switched to two rounds.\n\"");
        }
    }

    return -1;
}

/*
================
adm_Switch

Forces a player to the opposite team.
================
*/

int adm_Switch(int argNum, gentity_t *adm, qboolean shortCmd)
{
    char        str[MAX_TOKEN_CHARS];
    int         idNum, xTeam;
    char        userinfo[MAX_INFO_STRING];

    // Boe!Man 6/16/12: Check gametype first.
    if (!level.gametypeData->teams) {
        G_printInfoMessage(adm, "Not playing a team game.");
        return -1;
    }

    // Check argument.
    if (shortCmd){
        trap_Argv(1, str, sizeof(str));
    }else{
        if (adm && adm->client){
            trap_Argv(2, str, sizeof(str));
        }else{
            trap_Argv(1, str, sizeof(str));
        }
    }
    Q_strlwr(str);

    // Find the player.
    idNum = G_clientNumFromArg(adm, argNum, "switch", qtrue, qtrue, qtrue, shortCmd);

    // Boe!Man 1/22/14: If the client wasn't found, return.
    if (idNum < 0) return -1;

    if (g_entities[idNum].client){
        // Switch team.
        if (g_entities[idNum].client->sess.team == TEAM_RED) {
            strncpy(str, "b", sizeof(str));
            xTeam = TEAM_BLUE;
        }else if (g_entities[idNum].client->sess.team == TEAM_BLUE) {
            strncpy(str, "r", sizeof(str));
            xTeam = TEAM_RED;
        }

        if (g_entities[idNum].r.svFlags & SVF_BOT){ // Henk 25/01/11 -> Reset bots to set them to another team
            trap_GetUserinfo(idNum, userinfo, sizeof(userinfo));
            Info_SetValueForKey(userinfo, "team", str);
            trap_SetUserinfo(idNum, userinfo);
            g_entities[idNum].client->sess.team = (team_t)xTeam;

            if (current_gametype.value != GT_HS){
                g_entities[idNum].client->pers.identity = BG_FindTeamIdentity(level.gametypeTeam[xTeam], -1);
            }
            ClientBegin(idNum, qfalse);
        }else{
            SetTeam(&g_entities[idNum], str, NULL, qtrue);
        }

        // Respawn player.
        if (g_entities[idNum].client->sess.ghost) {
            G_StopFollowing(&g_entities[idNum]);
            g_entities[idNum].client->ps.pm_flags &= ~PMF_GHOST;
            g_entities[idNum].client->ps.pm_type = PM_NORMAL;
            g_entities[idNum].client->sess.ghost = qfalse;
        }else{
            TossClientItems(&g_entities[idNum]);
        }

        g_entities[idNum].client->sess.noTeamChange = qfalse;
        trap_UnlinkEntity(&g_entities[idNum]);
        ClientSpawn(&g_entities[idNum]);
    }

    return idNum;
}

/*
================
adm_toggleWeapon

Enables or disables the specified weapon.
================
*/

int adm_toggleWeapon(int argNum, gentity_t *adm, qboolean shortCmd)
{
    char        arg[16] = "\0";
    char        userinfo[MAX_INFO_STRING];
    int         argc, i, wpNum = 0;
    qboolean    enable;
    gitem_t     *item;
    gentity_t   *ent;

    // Can't toggle weapons like this in Hide&Seek.
    if(current_gametype.value == GT_HS || current_gametype.value == GT_HZ){
        G_printInfoMessage(adm, "You cannot use this command in %s.", (current_gametype.value == GT_HS) ? "Hide&Seek" : "Humans&Zombies");
        return -1;
    }

    // Can't toggle weapons when pickups are enabled.
    if(!level.pickupsDisabled){
        G_printInfoMessage(adm, "You cannot toggle weapons when pickups " \
            "are enabled.");
        return -1;
    }

    // Get the argument.
    if(shortCmd){
        argc = G_GetChatArgumentCount();
    }

    if(!shortCmd || shortCmd && !argc){
        trap_Argv(argNum, arg, sizeof(arg));
    }else{
        Q_strncpyz(arg, G_GetChatArgument(1), sizeof(arg));
    }
    Q_strlwr(arg);

    // Check if there was a weapon specified.
    if(strlen(arg) == 0){
        G_printInfoMessage(adm, "You must specify a weapon to toggle.");
        return -1;
    }

    // Get the weapon number.
    for(i = WP_KNIFE + 1; i < level.wpNumWeapons; i++){
        if(strstr(Q_strlwr(va("%s", bg_weaponNames[i])), arg)){
            wpNum = i;
            item = BG_FindWeaponItem((weapon_t)wpNum);
            break;
        }
    }

    // Not an existing weapon specified.
    if(!wpNum || !item){
        G_printInfoMessage(adm, "Weapon %s was not found!", arg);
        return -1;
    }

    // Toggle the weapon state.
	enable = trap_Cvar_VariableIntegerValue(va("disable_%s", item->classname)) > 0;
    trap_Cvar_Set(va("disable_%s", item->classname), (enable) ? "0" : "1");

    // Set the available weapons for the client.
    G_UpdateAvailableWeapons();
    BG_SetAvailableOutfitting(g_availableWeapons.string);

    // Grenades need to be enabled for the nonades Admin command as well.
    if(wpNum >= level.grenadeMin && wpNum <= level.grenadeMax){
        if(enable){
            // Disable the nonades check, since we just enabled one.
            if(g_disableNades.integer > 0){
                trap_Cvar_Set("g_disableNades", "0");
                trap_Cvar_Update(&g_disableNades);
            }

            level.nadesFound = qtrue;
        }else{
            level.nadesFound = qfalse;

            for(i = level.grenadeMin; i <= level.grenadeMax; i++){
                item = BG_FindWeaponItem((weapon_t)i);

                if(!item){
                    continue;
                }

                if(trap_Cvar_VariableIntegerValue(va("disable_%s", item->classname)) == 0){
                    // One grenade is enabled, so there are still nades in play.
                    level.nadesFound = qtrue;
                    break;
                }
            }

            // No nades left in play but they are enabled, disable them so nonades knows they can be re-enabled.
            if(!level.nadesFound && g_disableNades.integer == 0){
                trap_Cvar_Set("g_disableNades", "1");
                trap_Cvar_Update(&g_disableNades);
            }
        }
    }

    for (i = 0; i < level.numConnectedClients; i++){
        ent = &g_entities[level.sortedClients[i]];
        ent->client->noOutfittingChange = qfalse;
        trap_GetUserinfo(ent->s.number, userinfo, sizeof(userinfo));
        BG_DecompressOutfitting(Info_ValueForKey(userinfo, "outfitting"), &ent->client->pers.outfitting);
        G_UpdateOutfitting(ent->s.number);
    }

    // Broadcast the change.
    Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
    G_Broadcast(va("\\Weapon %s %s!", bg_weaponNames[wpNum], (enable) ? "enabled" : "disabled"), BROADCAST_CMD, NULL);
    if (adm && adm->client){
        trap_SendServerCommand(-1, va("print \"^3[Admin Action] ^7Weapon %s %s by %s.\n\"", bg_weaponNames[wpNum], (enable) ? "enabled" : "disabled", adm->client->pers.cleanName));
        Boe_adminLog(va("weapon %d %s", wpNum, (enable) ? "enabled" : "disabled"), va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), "none");
    }else{
        trap_SendServerCommand(-1, va("print \"^3[Rcon Action] ^7Weapon %s %s.\n\"", bg_weaponNames[wpNum], (enable) ? "enabled" : "disabled"));
        Boe_adminLog(va("weapon %d %s", wpNum, (enable) ? "enabled" : "disabled"), "RCON", "none");
    }

    return -1;
}

/*
==================
adm_Anticamp

Enables or disables anticamp.
==================
*/

int adm_Anticamp(int argNum, gentity_t *adm, qboolean shortCmd)
{
    qboolean enable = g_camperPunish.integer == 0;

    // Enable or disable it via CVAR.
    Boe_setTrackedCvar(&g_camperPunish, enable);

    // Broadcast the change.
    Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
    G_Broadcast(va("\\Anticamp %s!", (enable) ? "enabled" : "disabled"), BROADCAST_CMD, NULL);
    if (adm && adm->client){
        trap_SendServerCommand(-1, va("print \"^3[Admin Action] ^7Anticamp %s by %s.\n\"", (enable) ? "enabled" : "disabled", adm->client->pers.cleanName));
        Boe_adminLog(va("anticamp %s", (enable) ? "enabled" : "disabled"), va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), "none");
    }else{
        trap_SendServerCommand( -1, va("print \"^3[Rcon Action] ^7Anticamp %s.\n\"", (enable) ? "enabled" : "disabled"));
        Boe_adminLog(va("anticamp %s", (enable) ? "enabled" : "disabled"), "RCON", "none");
    }

    return -1;
}

/*
==================
adm_endMap

Ends the current map.
==================
*/

int adm_endMap(int argNum, gentity_t *adm, qboolean shortCmd)
{
    // Check if we already requested to end the map,
    // or are doing another map switch.
    if(level.mapSwitch){
        if(level.mapAction == 1 || level.mapAction == 3){
            G_printInfoMessage(adm, "A map restart is already in progress.");
        }else if(level.mapAction == 2 || level.mapAction == 4){
            G_printInfoMessage(adm, "A map switch is already in progress.");
        }else if(level.mapAction == 5){
            // Cancel the request.
            level.mapSwitch = qfalse;
            level.mapAction = 0;

            // Broadcast the change.
            Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
            G_Broadcast("\\Map end cancelled!", BROADCAST_CMD, NULL);
            if (adm && adm->client){
                trap_SendServerCommand(-1, va("print \"^3[Admin Action] ^7Map end cancelled by %s.\n\"", adm->client->pers.cleanName));
                Boe_adminLog("map end cancel", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), "none");
            }else{
                trap_SendServerCommand(-1, "print \"^3[Rcon Action] ^7Map end cancelled.\n\"");
                Boe_adminLog("map end cancel", "RCON", "none");
            }
        }

        return -1;
    }

    // Request the map to end in 5 seconds.
    level.mapSwitch = qtrue;
    level.mapAction = 5;
    level.mapSwitchCount = level.time;
    level.mapSwitchCount2 = 5; // Boe!Man 7/22/12: 5 seconds remaining on the timer.

    // Broadcast the change.
    Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
    G_Broadcast("\\Map ends in 5 sec!", BROADCAST_CMD, NULL);
    if (adm && adm->client){
        trap_SendServerCommand(-1, va("print \"^3[Admin Action] ^7Requested map end by %s.\n\"", adm->client->pers.cleanName));
        Boe_adminLog("map end", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), "none");
    }else{
        trap_SendServerCommand(-1, "print \"^3[Rcon Action] ^7Requested map end.\n\"");
        Boe_adminLog("map end", "RCON", "none");
    }

    return -1;
}

/*
==================
adm_mapList

Shows the map list.
==================
*/

static int sortAlpha(const void *a, const void *b)
{
    return strcmp(*(const char **)a, *(const char **)b);
}

int adm_mapList(int argNum, gentity_t *adm, qboolean shortCmd)
{
    char        buf2[1000] = "\0";
    char        gtbuf[128];
    char        **arenaNames;
    int         arenaCount, i, n;
    qboolean    gtAvailable;
    char        *gameTypes[] = {"dm", "tdm", "ctf", "inf", "elim", "h&s", "h&z"
        #ifdef _GOLD
        , "dem"};
        #else
        };
        #endif // _GOLD

    // Print header.
    if(adm){
        Q_strcat(buf2, sizeof(buf2),
            #ifdef _GOLD
            va("^3[Maplist]\n\n^7" \
                " %-22s^1%-6s^2%-6s^3%-6s^4%-6s^5%-6s^6%-6s^1%-6s^2%-6s\n%s",
            #else
            va("^3[Maplist]\n\n^7" \
                " %-28s^1%-6s^2%-6s^3%-6s^4%-6s^5%-6s^6%-6s^1%-6s\n%s",
            #endif // _GOLD
            "Mapname", "  DM", "  TDM", "  CTF", "  INF", "  ELIM", "  H&S",
            "  H&Z"
            #ifdef _GOLD
            , "  DEM"
            #endif // _GOLD
            ,
            "^7-----------------------------------" \
            "-------------------------------------\n"
            ));
    }else{
        Com_Printf("^3[Maplist]\n\n");
        Com_Printf(
            #ifdef _GOLD
            " %-22s^1%-6s^2%-6s^3%-6s^4%-6s^5%-6s^6%-6s^1%-6s^2%-6s\n",
            #else
            " %-28s^1%-6s^2%-6s^3%-6s^4%-6s^5%-6s^6%-6s^1%-6s\n",
            #endif // _GOLD
            "Mapname", "  DM", "  TDM", "  CTF", "  INF", "  ELIM", "  H&S",
            "  H&Z"
            #ifdef _GOLD
            , "  DEM"
            #endif // _GOLD
            );
        Com_Printf("^7-----------------------------------"
            "-------------------------------------\n");
    }

    // Get the amount of maps available.
    arenaCount = G_getArenaCount();

    // Allocate memory to store the map names.
    arenaNames = malloc(arenaCount * sizeof(char *));
    for(i = 0; i < arenaCount; i++){
        arenaNames[i] = malloc(32 * sizeof(char *));
    }

    // Fetch the map names.
    G_getArenaNames(arenaNames, 32);

    // We now got all the available maps that are defined through arena files.
    // Sort alphabetically.
    qsort(arenaNames, arenaCount, sizeof(char *), sortAlpha);

    // Start iterating through them.
    for(i = 0; i < arenaCount; i++){
        // Skip special "RMG" map.
        if(strcmp(arenaNames[i], "*random") == 0){
            free(arenaNames[i]);
            continue;
        }

        // Print mapname.
        if(adm){
            #ifdef _GOLD
            Q_strcat(buf2, sizeof(buf2), va(" %-22s", arenaNames[i]));
            #else
            Q_strcat(buf2, sizeof(buf2), va(" %-28s", arenaNames[i]));
            #endif // _GOLD
        }else{
            #ifdef _GOLD
            Com_Printf(" %-22s", arenaNames[i]);
            #else
            Com_Printf(" %-28s", arenaNames[i]);
            #endif // _GOLD
        }

        // Iterate through gametypes.
        memset(gtbuf, 0, sizeof(gtbuf));
        for(n = 0; n < sizeof(gameTypes)/sizeof(gameTypes[0]); n++){
            gtAvailable = Henk_DoesMapSupportGametype(gameTypes[n],
                arenaNames[i]);

            // Append gametype info the buffer.
            Q_strcat(gtbuf, sizeof(gtbuf),
                (gtAvailable) ? va("|  ^%dX^7  ", n % 6 + 1) : "|     ");
        }

        // Print available gametypes and leading characters.
        if(adm){
            Q_strcat(buf2, sizeof(buf2), va("%s|\n", gtbuf));
        }else{
            Com_Printf("%s|\n", gtbuf);
        }

        // Put packet through to clients if char size would exceed 1000
        // and reset buf2.
        if(strlen(buf2) > 900){
            trap_SendServerCommand(adm-g_entities, va("print \"%s\"", buf2));
            memset(buf2, 0, sizeof(buf2));
        }

        // Free allocated entry.
        free(arenaNames[i]);
    }

    // Free allocated pointers.
    free(arenaNames);

    // Print footer, and in the case of calling via adm, also the last entry.
    if(adm){
        trap_SendServerCommand(adm-g_entities,
            va("print \"%s\nUse ^3[Page Up] ^7and ^3[Page Down] ^7keys " \
            "to scroll\n\n\"", buf2));
    }else{
        Com_Printf("\nUse ^3[Page Up] ^7and ^3[Page Down] ^7keys " \
            "to scroll\n\n");
    }

    return -1;
}
