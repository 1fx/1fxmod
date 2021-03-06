/*
===========================================================================
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
Copyright (C) 2010 - 2020, Ane-Jouke Schat (Boe!Man)
Copyright (C) 2010 - 2020, Jordi Epema (Henkie)

This file is part of the 1fx. Mod source code.

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/

#include "g_local.h"
#include "boe_local.h"

#include "../ui/menudef.h"

// Henk 04/05/10 -> New command system(Yus this is very pro)
typedef struct
{
    char    *shortCmd; // Short admin command, example: !uc, !p(with space)
    char    *adminCmd; // Full Admin command for /adm and rcon, and with !.
    int     *adminLevel; // The level that the Admin needs to be in order to execute this command.
    int     (*Function)(int argNum, gentity_t *adm, qboolean shortCmd); // Store pointer to the given function so we can call it later.
    char    *desc; // Description of the command in /adm.
    char    *params; // Description of the command in /adm.
    char    *suffix; // Suffix for post processing broadcast, or NULL when function doesn't use it/has no suffix.
}admCmd_t;

// Boe!Man 11/24/15: Lowest Admin level is specified here.
// The commands that use this value have specific Admin level handling.
static int minimumAdmLevel = 2;

static admCmd_t AdminCommands[] =
{
    {"!adr",    "adminremove",      &minimumAdmLevel,           &adm_adminRemove,               "Remove an Admin from the list",    "<line #>",         NULL},
    {"!adl",    "adminlist",        &g_adminlist.integer,       &adm_adminList,                 "Show the Adminlist",               "",                 NULL},
    {"!al",     "adminlist",        &g_adminlist.integer,       &adm_adminList,                 "Show the Adminlist",               "",                 NULL},
    {"!ab",     "addbadmin",        &g_badmin.integer,          &adm_addAdmin,                  "Basic Admin",                      "<i/n>",            NULL},
    {"!aa",     "addadmin",         &g_admin.integer,           &adm_addAdmin,                  "Admin",                            "<i/n>",            NULL},
    {"!as",     "addsadmin",        &g_sadmin.integer,          &adm_addAdmin,                  "Server Admin",                     "<i/n>",            NULL},
    {"!sl",     "scorelimit",       &g_sl.integer,              &adm_scoreLimit,                "Change the scorelimit",            "<time>",           NULL},
    {"!tl",     "timelimit",        &g_tl.integer,              &adm_timeLimit,                 "Change the timelimit",             "<time>",           NULL},
    {"!sw",     "swapteams",        &g_swapteams.integer,       &adm_swapTeams,                 "Swap the players from team",       "",                 NULL},
    {"!rounds", "rounds",           &g_cm.integer,              &adm_Rounds,                    "Set the number of rounds",         "<rounds>",         NULL},
    // Boe!Man 6/2/15: Don't move or modify anything above this comment, the /adm list expects them in that specific order.
    {"!pl",     "plant",            &g_plant.integer,           &adm_Plant,                     "Plant or unplant a player",        "<i/n>",            "ed"},
    {"!ro",     "runover",          &g_runover.integer,         &adm_Runover,                   "Push a player backwards",          "<i/n>",            NULL},
    {"!r",      "respawn",          &g_respawn.integer,         &adm_Respawn,                   "Respawn a player",                 "<i/n>",            "ed"},
    {"!rs",     "respawn",          &g_respawn.integer,         &adm_Respawn,                   "Respawn a player",                 "<i/n>",            "ed" },
    {"!mr",     "maprestart",       &g_mapswitch.integer,       &adm_mapRestart,                "Restart the current map",          "",                 NULL},
    {"!mr",     "map_restart",      &g_mapswitch.integer,       &adm_mapRestart,                "Restart the current map",          "",                 NULL },
    {"!st",     "strip",            &g_strip.integer,           &adm_Strip,                     "Remove weapons from a player",     "<i/n>",            "ped"},
    {"!ra",     "removeadmin",      &minimumAdmLevel,           &adm_removeAdmin,               "Remove an Admin",                  "<i/n>",            NULL},
    {"!ft",     "forceteam",        &g_forceteam.integer,       &adm_forceTeam,                 "Force a player to join a team",    "<i/n> <team>",     "ed"},
    {"!nl",     "nolower",          &g_nosection.integer,       &adm_noLower,                   "Enable/Disable Nolower",           "",                 NULL},
    {"!nr",     "noroof",           &g_nosection.integer,       &adm_noRoof,                    "Enable/Disable Noroof",            "",                 NULL},
    {"!nm",     "nomiddle",         &g_nosection.integer,       &adm_noMiddle,                  "Enable/Disable Nomiddle",          "",                 NULL},
    {"!nw",     "nowhole",          &g_nosection.integer,       &adm_noWhole,                   "Enable/Disable Nowhole",           "",                 NULL},
    {"!sh",     "shuffleteams",     &g_shuffleteams.integer,    &adm_shuffleTeams,              "Mix the teams at random",          "",                 NULL},
    {"!nn",     "nonades",          &g_nades.integer,           &adm_noNades,                   "Enable or disable nades",          "",                 NULL},
    {"!ri",     "respawninterval",  &g_ri.integer,              &adm_respawnInterval,           "Change the respawn interval",      "<time>",           NULL},
    {"!rd",     "realdamage",       &g_damage.integer,          &adm_realDamage,                "Toggle Real damage",               "",                 NULL},
    {"!nd",     "normaldamage",     &g_damage.integer,          &adm_normalDamage,              "Toggle Normal damage",             "",                 NULL},
    {"!gr",     "gametyperestart",  &g_gr.integer,              &adm_gametypeRestart,           "Restart the current gametype",     "",                 NULL},
    {"!acl",    "addclan",          &g_clan.integer,            &adm_addClanMember,             "Add a clan member",                "<i/n>",            NULL},
    {"!rc",     "removeclan",       &g_clan.integer,            &adm_removeClanMember,          "Remove a clan member",             "<i/n>",            NULL},
    {"!rcl",    "removeclanlist",   &g_clan.integer,            &adm_removeClanMemberFromList,  "Remove a member from the list",    "<i/line #>",       NULL},
    {"!clr",    "clanlistremove",   &g_clan.integer,            &adm_removeClanMemberFromList,  "Remove a member from the list",    "<i/line #>",       NULL},
    {"!cl",     "clanlist",         &g_clan.integer,            &adm_clanList,                  "Show the clanlist",                "",                 NULL},
    {"!cm",     "compmode",         &g_cm.integer,              &adm_compMode,                  "Toggles Competition Mode",         "",                 NULL},
    {"!bl",     "banlist",          &g_ban.integer,             &adm_banList,                   "Shows the current banlist",        "",                 NULL},
    {"!ba",     "ban",              &g_ban.integer,             &adm_Ban,                       "Ban a player",                     "<i/n> <reason>",   "ned"},
    {"!ub",     "unban",            &g_ban.integer,             &adm_Unban,                     "Unban a banned IP (player)",       "<ip/line #>",      NULL},
    {"!uba",    "unban",            &g_ban.integer,             &adm_Unban,                     "Unban a banned IP (player)",       "<ip/line #>",      NULL},
    {"!br",     "broadcast",        &g_broadcast.integer,       &adm_Broadcast,                 "Broadcast a message",              "<message>",        NULL},
    {"!sbl",    "subnetbanlist",    &g_subnetban.integer,       &adm_subnetbanList,             "Shows the current subnetbanlist",  "",                 NULL},
    {"!sb",     "subnetban",        &g_subnetban.integer,       &adm_subnetBan,                 "Ban a players' subnet",            "<i/n> <reason>",   "ned"},
    {"!sbu",    "subnetunban",      &g_subnetban.integer,       &adm_subnetUnban,               "Unban a banned subnet",            "<ip/line #>",      NULL},
    {"!su",     "subnetunban",      &g_subnetban.integer,       &adm_subnetUnban,               "Unban a banned subnet",            "<ip/line #>",      NULL},
    {"!et",     "eventeams",        &g_eventeams.integer,       &adm_evenTeams,                 "Make the teams even",              "",                 NULL},
    {"!cva",    "clanvsall",        &g_clanvsall.integer,       &adm_clanVsAll,                 "Clan versus other players-mode",   "",                 NULL},
    {"!l",      "lock",             &g_lock.integer,            &adm_lockTeam,                  "Lock/unlock a team",               "<team>",           NULL},
    {"!fl",     "flash",            &g_flash.integer,           &adm_Flash,                     "Flash a player",                   "<i/n>",            "ed"},
    {"!g",      "gametype",         &g_mapswitch.integer,       &adm_Gametype,                  "Switch to the given gametype",     "<gametype>",       NULL},
    {"!gt",     "gametype",         &g_mapswitch.integer,       &adm_Gametype,                  "Switch to the given gametype",     "<gametype>",       NULL},
    {"!map",    "map",              &g_mapswitch.integer,       &adm_Map,                       "Switch to the specified map",      "<map name>",       NULL},
    {"!altmap", "altmap",           &g_mapswitch.integer,       &adm_Map,                       "Switch to the specified altmap",   "<map name>",       NULL},
    {"!devmap", "devmap",           &g_mapswitch.integer,       &adm_Map,                       "Switch to the specified devmap",   "<map name>",       NULL},
    {"!mc",     "mapcycle",         &g_mapswitch.integer,       &adm_mapCycle,                  "Switch to the next-defined map",   "",                 NULL},
    {"!pv",     "passvote",         &g_forcevote.integer,       &adm_passVote,                  "Pass the running vote",            "",                 NULL},
    {"!cv",     "cancelvote",       &g_forcevote.integer,       &adm_cancelVote,                "Cancel the running vote",          "",                 NULL},
    {"!pa",     "pause",            &g_pause.integer,           &adm_Pause,                     "Pause/resume the game",            "",                 NULL},
    {"!b",      "burn",             &g_burn.integer,            &adm_Burn,                      "Burn a player",                    "",                 "ed"},
    {"!k",      "kick",             &g_kick.integer,            &adm_Kick,                      "Kick a player",                    "<i/n>",            "ed"},
    {"!m",      "mute",             &g_mute.integer,            &adm_Mute,                      "Mute/unmute a player",             "<i/n> <time>",     NULL},
    {"!s",      "strip",            &g_strip.integer,           &adm_Strip,                     "Remove weapons from a player",     "<i/n>",            "ped"},
    {"!ff",     "friendlyfire",     &g_ff.integer,              &adm_friendlyFire,              "Enables/disables friendly fire",   "",                 NULL},
    {"!rn",     "rename",           &g_rename.integer,          &adm_Rename,                    "Renames a players' name",          "<i/n> <name>",     NULL},
    {"!swi",    "switch",           &g_forceteam.integer,       &adm_Switch,                    "Switch one to the opposite team",  "<i/n>",            "ed"},
    #ifndef _DEMO
    {"!3rd",    "3rd",              &g_3rd.integer,             &adm_Third,                     "Toggles Thirdperson on or off",    "",                 NULL},
    {"!third",  "third",            &g_3rd.integer,             &adm_Third,                     "Toggles Thirdperson on or off",    "",                 NULL},
    #endif // not _DEMO
    {"!wp",     "weapon",           &g_toggleweapon.integer,    &adm_toggleWeapon,              "Toggles weapon on or off",         "",                 NULL},
    {"!aca",    "anticamp",         &g_anticamp.integer,        &adm_Anticamp,                  "Toggles anticamp on or off",       "",                 NULL},
    {"!em",     "endmap",           &g_endmap.integer,          &adm_endMap,                    "Requests map to end",              "",                 NULL},
    {"!ml",     "maplist",          &g_mapswitch.integer,       &adm_mapList,                   "Lists all available maps",         "",                 NULL},
    // Boe!Man 6/2/15: Admin synonyms for Gold.
    // Pop/explode.
    {"!p",      "pop",              &g_pop.integer,             &adm_Pop,                       "Pop/explodes a player",            "<i/n>",            "ped"},
    #ifdef _GOLD
    {"!e",      "explode",          &g_pop.integer,             &adm_Pop,                       "Pop/explodes a player",            "<i/n>",            "ped"},
    #endif // _GOLD
    // Uppercut/launch.
    {"!u",      "uppercut",         &g_uppercut.integer,        &adm_Uppercut,                  "Launch a player upwards",          "<i/n>",            NULL},
    {"!uc",     "uppercut",         &g_uppercut.integer,        &adm_Uppercut,                  "Launch a player upwards",          "<i/n>",            NULL},
    #ifdef _GOLD
    {"!la",     "launch",           &g_uppercut.integer,        &adm_Uppercut,                  "Launch a player upwards",          "<i/n>",            "ed"},
    #endif // _GOLD
};

static int AdminCommandsSize = sizeof( AdminCommands ) / sizeof( AdminCommands[0] );
// End

int AcceptBotCommand(char *cmd, gentity_t *pl);

void trap_SendServerCommand2( int clientNum, const char *text );
void trap_SendServerCommand( int clientNum, const char *text ) {

        if( strlen( text ) > 2044 ) { // Henk 15/09/10 -> Scores reach 1022 length easily when there are more then 32 players.
            return;
        }

        trap_SendServerCommand2(clientNum, text);
}

void EvenTeams (gentity_t *adm, qboolean aet)
{
    int     counts[TEAM_NUM_TEAMS];
    int     diff = 0;
    int     highTeam, i, j, lastConnectedTime;
    gentity_t *lastConnected, *ent;
    clientSession_t *sess;
    qboolean canBeMoved = qfalse;

    if(current_gametype.value == GT_HS){
        if(level.cagefight){
            G_printInfoMessage(adm, "You cannot even the teams during cagefight.");
        }else{
            EvenTeams_HS(adm, aet);
        }
        return;
    }else if (current_gametype.value == GT_HZ){
        EvenTeams_HZ(adm, aet);
        return;
    }

    if(level.intermissiontime || level.changemap)
        return;

    if(!level.gametypeData->teams){
        if((adm && adm->client) || aet == qfalse){
            G_printInfoMessage(adm, "Not playing a team game.");
        }
        return;
    }

    if(level.blueLocked || level.redLocked){
        if((adm && adm->client) || aet == qfalse){
            G_printInfoMessage(adm, "Teams are locked.");
        }
        return;

    }

    counts[TEAM_BLUE] = TeamCount(-1, TEAM_BLUE, NULL );
    counts[TEAM_RED] = TeamCount(-1, TEAM_RED, NULL );

    if(counts[TEAM_BLUE] > counts[TEAM_RED]){
        highTeam = TEAM_BLUE;
        diff = (counts[TEAM_BLUE] - counts[TEAM_RED]);
    }
    else if(counts[TEAM_BLUE] < counts[TEAM_RED]){
        highTeam = TEAM_RED;
        diff = (counts[TEAM_RED] - counts[TEAM_BLUE]);
    }
    else {
        if((adm && adm->client) || aet == qfalse){
            G_printInfoMessage(adm, "Teams are as even as possible.");
        }
        return;
    }

    if(diff < 2){
        if((adm && adm->client) || aet == qfalse){
            G_printInfoMessage(adm, "Teams are as even as possible.");
        }

        return;
    }
    diff /= 2;
    for(i = 0; i < diff; i++){
        lastConnectedTime = 0;
        for ( j = 0; j < level.numConnectedClients ; j++ )  {
            ent = &g_entities[level.sortedClients[j]];
            sess = &ent->client->sess;


            if ( ent->client->pers.connected != CON_CONNECTED
                && ent->client->pers.connected != CON_CONNECTING )
                continue;
            if(sess->team != TEAM_RED && sess->team != TEAM_BLUE)
                continue;
            if(sess->team != highTeam)
                continue;
            if(ent->s.gametypeitems)
                continue;

            if(ent->client->pers.enterTime > lastConnectedTime) {
                lastConnectedTime = ent->client->pers.enterTime;
                lastConnected = ent;
            }
            canBeMoved = qtrue;
        }

        if(!canBeMoved || !highTeam){
            if((adm && adm->client) || aet == qfalse){
                G_printInfoMessage(adm, "Teams cannot be evened [all item holders].");
            }

            return;
        }

        // Boe!Man 7/13/12: Fix crash issue with auto eventeams too soon
        // before entering the map.
        if(lastConnected == NULL || lastConnected->client == NULL){
            if((adm && adm->client) || aet == qfalse){
                G_printInfoMessage(adm,
                    "You cannot even the teams this fast.");
            }
            return;
        }

        if(!G_IsClientDead(lastConnected->client)){
            lastConnected->client->ps.stats[STAT_WEAPONS] = 0;
            TossClientItems( lastConnected );
        }
        G_StartGhosting( lastConnected );

        if(highTeam == TEAM_RED){
            lastConnected->client->sess.team = TEAM_BLUE;
        }else{
            lastConnected->client->sess.team = TEAM_RED;
        }

        if(lastConnected->r.svFlags & SVF_BOT){
            char    userinfo[MAX_INFO_STRING];

            trap_GetUserinfo(lastConnected->s.number, userinfo,
                sizeof(userinfo));
            Info_SetValueForKey( userinfo, "team",
                lastConnected->client->sess.team == TEAM_RED? "red" : "blue");
            trap_SetUserinfo(lastConnected->s.number, userinfo);
        }

        lastConnected->client->pers.identity = NULL;
        ClientUserinfoChanged( lastConnected->s.number );
        CalculateRanks();

        G_StopFollowing(lastConnected);
        G_StopGhosting(lastConnected);
        trap_UnlinkEntity(lastConnected);
        ClientSpawn(lastConnected);
    }


    // Boe!Man 3/31/10: We tell 'em what happened.
    #ifndef _DEMO
    Boe_GlobalSound (G_SoundIndex("sound/misc/events/tut_lift02.mp3", qtrue));
    #endif // _DEMO

    if(adm && adm->client){
        G_Broadcast("\\Evening teams!", BROADCAST_CMD, NULL);
        trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Eventeams by %s.\n\"", adm->client->pers.cleanName));
        Boe_adminLog ("eventeams", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), "none");
    }else if (aet == qfalse){
        G_Broadcast("\\Evening teams!", BROADCAST_CMD, NULL);
        trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7Eventeams.\n\""));
        Boe_adminLog ("eventeams", "RCON", "none");
    }else{
        trap_SendServerCommand(-1, va("print\"^3[Auto Action] ^7Eventeams.\n\""));
    }
}

void SwapTeams (gentity_t *adm, qboolean aswap)
{
    int             i;
    int             rs = level.teamScores[TEAM_RED];
    int             bs = level.teamScores[TEAM_BLUE];
    int             rl = level.redLocked;
    int             bl = level.blueLocked;
    clientSession_t *sess;
    gentity_t       *ent, *find;
    qboolean        enabled;

    if(!aswap){
        if (!level.gametypeData->teams) {
            G_printInfoMessage(adm, "Currently not playing a team game.");
            return;
        }

        // Boe!Man 12/16/15: Don't allow swapteams if the weapons are already given out.
        if(current_gametype.value == GT_HS && level.messagedisplay1){
            G_printInfoMessage(adm, "Swapteams is only possible before the weapons are given out.");
            return;
        }

        // Toggle the setting if they're on the Match Settings screen instead of swapping the teams.
        if (cm_enabled.integer == 1){
            enabled = cm_aswap.integer == 1;
            if (enabled){
                trap_Cvar_Set("cm_aswap", "0"); // Disable it.
            }else{
                trap_Cvar_Set("cm_awap", "1");
            }
            trap_Cvar_Update(&cm_aswap);

            // Broadcast change.
            G_Broadcast(va("\\Autoswap %s!", (enabled) ? "disabled" : "enabled"), BROADCAST_CMD, NULL);

            if (adm&&adm->client){
                trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Auto swap %s by %s.\n\"", (enabled) ? "disabled" : "enabled", adm->client->pers.cleanName));
            }else{
                trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7Auto swap %s.\n\"", (enabled) ? "disabled" : "enabled"));
            }

            return;
        }
    }

    // Boe!Man 12/16/15: Enforce the new round start here so seekers won't get released sooner then they are supposed to.
    if (current_gametype.value == GT_HS){
        for (i = 0; i < level.numConnectedClients; i++){
            ent = &g_entities[level.sortedClients[i]];

            if (ent->client->sess.team == TEAM_RED){
                ent->client->sess.team = TEAM_BLUE;
            }else if (ent->client->sess.team == TEAM_BLUE){
                ent->client->sess.team = TEAM_RED;
            }

            // Take care of the bots.
            if (ent->r.svFlags & SVF_BOT){
                char    userinfo[MAX_INFO_STRING];

                trap_GetUserinfo(ent->s.number, userinfo, sizeof(userinfo));
                Info_SetValueForKey(userinfo, "team", ent->client->sess.team == TEAM_RED ? "red" : "blue");
                trap_SetUserinfo(ent->s.number, userinfo);
            }
        }

        if(!aswap){
            G_ResetGametype(qtrue, qfalse);
        }
    }else{
        for (i = 0; i < level.numConnectedClients; i++){
            ent = &g_entities[level.sortedClients[i]];
            sess = &ent->client->sess;

            // Do the team changing.
            if (ent->client->sess.team == TEAM_SPECTATOR)
                continue;
            if (ent->client->pers.connected != CON_CONNECTED)
                continue;

            // Drop any gametype items they might have.
            if (ent->s.gametypeitems > 0){
                G_DropGametypeItems(ent, 0);
            }

            ///01.24.06e - 07:42pm - remove their weapons
            ///and set them as a ghost
            ent->client->ps.stats[STAT_WEAPONS] = 0;
            TossClientItems(ent);
            G_StartGhosting(ent);
            ///End  - 01.24.06 - 07:43pm

            if (ent->client->sess.team == TEAM_RED){
                ent->client->sess.team = TEAM_BLUE;
            }else if (ent->client->sess.team == TEAM_BLUE){
                ent->client->sess.team = TEAM_RED;
            }

            // Take care of the bots.
            if (ent->r.svFlags & SVF_BOT){
                char    userinfo[MAX_INFO_STRING];

                trap_GetUserinfo(ent->s.number, userinfo, sizeof(userinfo));
                Info_SetValueForKey(userinfo, "team", sess->team == TEAM_RED ? "red" : "blue");
                trap_SetUserinfo(ent->s.number, userinfo);
            }

            ///Prepare the clients for team change then repawn
            ///01.24.06 - 07:43pm
            ent->client->pers.identity = NULL;
            ClientUserinfoChanged(ent->s.number);
            CalculateRanks();

            G_StopFollowing(ent);
            G_StopGhosting(ent);
            trap_UnlinkEntity(ent);
            ClientSpawn(ent);
        }

        // Reset gametype item.
        find = NULL;
        while (NULL != (find = G_Find(find, FOFS(classname), "gametype_item"))){
            G_ResetGametypeItem(find->item);
        }

        ///04.22.05 - 02:44am - swap scores & locks
        level.teamScores[TEAM_BLUE] = rs;
        level.teamScores[TEAM_RED] = bs;
        level.redLocked = bl;
        level.blueLocked = rl;
        ///End  - 04.22.05 - 02:45am

        // Enable roundtime for gametypes without respawn intervals.
        if (level.gametypeData->respawnType != RT_INTERVAL){
            level.gametypeDelayTime = level.time + g_roundstartdelay.integer * 1000;

            level.gametypeRoundTime = level.time + (g_roundtimelimit.integer * 60000);
            if (level.gametypeDelayTime != level.time){
                trap_SetConfigstring(CS_GAMETYPE_TIMER, va("%i", level.gametypeRoundTime));
            }
        }
    }

    // Proper messaging/logging.
    #ifndef _DEMO
    Boe_GlobalSound(G_SoundIndex("sound/misc/events/tut_lift02.mp3", qtrue));
    #endif // _DEMO

    if(!aswap){
        G_Broadcast("\\Swapteams!", BROADCAST_CMD, NULL);
    }

    if (adm && adm->client){
        trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Swap teams by %s.\n\"", adm->client->pers.cleanName));
        Boe_adminLog("swapteams", va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), "none");
    }else{
        // Boe!Man 11/17/10: Auto swap in compmode.
        if (aswap){
            trap_SendServerCommand(-1, va("print\"^3[Auto Action] ^7Swap teams.\n\""));
        }else{
            trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7Swap teams.\n\""));
            Boe_adminLog("swapteams", "RCON", "none");
        }
    }
}

/*
==================
DeathmatchScoreboardMessage
==================
*/
void DeathmatchScoreboardMessage( gentity_t *ent )
{
    char        entry[1024], entry2[1024];
    char        string[2048], string2[2048];
    int         stringlength, stringlength2;
    int         i, j;
    gclient_t   *cl;
    int         numSorted;

    //Ryan may 15 2004
    //dont let the scores update duing the awards
    if(level.awardTime)
    {
        return;
    }
    //Ryan

    // send the latest information on all clients
    string[0]       = 0;
    string2[0]      = 0;
    stringlength    = 0;
    stringlength2   = 0;

    numSorted = level.numConnectedClients;

    for (i=0 ; i < numSorted ; i++)
    {
        int ping;
        qboolean ghost;

        cl = &level.clients[level.sortedClients[i]];

        if ( cl->pers.connected == CON_CONNECTING )
        {
            ping = -1;
        }
        else
        {
            ping = cl->ps.ping < 999 ? cl->ps.ping : 999;
        }

        ghost = cl->sess.ghost;
        #ifdef _3DServer
        if (!ghost && cl->sess.deadMonkey){
            ghost = qtrue;
        }
        #endif // _3DServer

        //Ryan may 12 2004
        //Add some info for client-side users
        //RxCxW - 1.20.2005 - NOT compatible with 0.5. #Version
        #if !defined (_GOLD) && !defined(_DEMO)
        if (level.clientMod == CL_RPM){
            if(ent->client->sess.rpmClient > 0.5)
            {
                Com_sprintf (entry, sizeof(entry),
                    " %i %i %i %i %i %i %i %i %i %.2f %i",
                    level.sortedClients[i],
                    cl->sess.score,
                    cl->sess.kills,
                    (current_gametype.value != GT_HZ) ? cl->sess.deaths : cl->sess.killsAsZombie,
                    ping,
                    (level.time - cl->pers.enterTime)/60000,
                    (ghost || cl->ps.pm_type == PM_DEAD) ? qtrue : qfalse,
                    g_entities[level.sortedClients[i]].s.gametypeitems,
                    g_teamkillDamageMax.integer ? 100 * cl->sess.teamkillDamage / g_teamkillDamageMax.integer : 0,
                    cl->pers.statinfo.accuracy,
                    cl->pers.statinfo.headShotKills
                    //cl->pers.statinfo.damageDone
                    );
            }else if(ent->client->sess.rpmClient == 1.1){
                Com_sprintf (entry, sizeof(entry),
                " %i %i %i %i %i %i %i %i %i",
                level.sortedClients[i],
                cl->sess.score,
                cl->sess.kills,
                (current_gametype.value != GT_HZ) ? cl->sess.deaths : cl->sess.killsAsZombie,
                ping,
                (level.time - cl->pers.enterTime)/60000,
                (ghost || cl->ps.pm_type == PM_DEAD) ? qtrue : qfalse,
                g_entities[level.sortedClients[i]].s.gametypeitems,
                g_teamkillDamageMax.integer ? 100 * cl->sess.teamkillDamage / g_teamkillDamageMax.integer : 0
                );
            }
            else{
                Com_sprintf (entry, sizeof(entry),
                " %i %i %i %i %i %i %i %i %i",
                level.sortedClients[i],
                cl->sess.score,
                cl->sess.kills,
                (current_gametype.value != GT_HZ) ? cl->sess.deaths : cl->sess.killsAsZombie,
                ping,
                (level.time - cl->pers.enterTime)/60000,
                (ghost || cl->ps.pm_type == PM_DEAD) ? qtrue : qfalse,
                g_entities[level.sortedClients[i]].s.gametypeitems,
                g_teamkillDamageMax.integer ? 100 * cl->sess.teamkillDamage / g_teamkillDamageMax.integer : 0
                );
            }
        }
        #else
        Com_sprintf(entry, sizeof(entry),
            #ifndef _DEMO
            " %i %i %i %i %i %i %i %i %i",
            #else
            " %i %i %i %i %i %i %i %i",
            #endif // not _DEMO
            level.sortedClients[i],
            cl->sess.score,
            cl->sess.kills,
            cl->sess.deaths,
            ping,
            (level.time - cl->pers.enterTime) / 60000,
            (ghost || cl->ps.pm_type == PM_DEAD) ? qtrue : qfalse,
            #ifndef _DEMO
            g_entities[level.sortedClients[i]].s.gametypeitems,
            g_teamkillDamageMax.integer ? 100 * cl->sess.teamkillDamage / g_teamkillDamageMax.integer : 0
            #else
            g_entities[level.sortedClients[i]].s.gametypeitems
            #endif // not _DEMO
            );
        #endif // not _GOLD and not _DEMO

        j = strlen(entry);
        if (stringlength + j > 1022 )
            break;

        strcpy (string + stringlength, entry);
        stringlength += j;

        #ifdef _GOLD
        if (level.clientMod == CL_ROCMOD) {
            int admin = 0;

            if(cl->sess.admin == 4)
                admin = 3;
            else if (cl->sess.admin > 1)
                admin = 2;
            else if (cl->sess.referee)
                admin = 1;

            Com_sprintf(entry2, sizeof(entry2),
                " %i %i %i %i %i %i %i %i %i %i",
                admin,
                cl->pers.statinfo.hitcount,
                cl->pers.statinfo.shotcount,
                cl->pers.statinfo.headShotKills,
                cl->pers.statinfo.itemCaptures,
                cl->pers.statinfo.bestKillsInARow,
                cl->pers.statinfo.knifeKills,
                cl->pers.statinfo.explosiveKills,
                cl->sess.clanMember,
                cl->pers.statinfo.itemDefends
                );

            j = strlen(entry2);
            if (stringlength2 + j > 1022)
                break;

            strcpy(string2 + stringlength2, entry2);
            stringlength2 += j;
        }
        #endif // _GOLD
    }

    trap_SendServerCommand( ent-g_entities, va("scores %i %i %i%s", i,
                            level.teamScores[TEAM_RED],
                            level.teamScores[TEAM_BLUE],
                            string ) );

    #ifdef _GOLD
    if (level.clientMod == CL_ROCMOD && ent->client->sess.rocModClient){
        trap_SendServerCommand(ent - g_entities, va("scores4 %i 10 %i%s", g_timelimit.integer ? (g_timelimit.integer + level.timeExtension) : 0, i, string2));
    }
    #endif // _GOLD
}


/*
==================
Cmd_Score_f

Request current scoreboard information
==================
*/
void Cmd_Score_f( gentity_t *ent )
{
    DeathmatchScoreboardMessage( ent );
}

/*
==================
CheatsOk
==================
*/
qboolean    CheatsOk( gentity_t *ent ) {
    if ( !g_cheats.integer ) {
        trap_SendServerCommand( ent-g_entities, va("print \"Cheats are not enabled on this server.\n\""));
        return qfalse;
    }
    if ( ent->health <= 0 ) {
        trap_SendServerCommand( ent-g_entities, va("print \"You must be alive to use this command.\n\""));
        return qfalse;
    }
    return qtrue;
}


/*
==================
ConcatArgs
==================
*/
char    *ConcatArgs( int start ) {
    int     i, c, tlen;
    static char line[MAX_STRING_CHARS];
    int     len;
    char    arg[MAX_STRING_CHARS];

    len = 0;
    c = trap_Argc();
    for ( i = start ; i < c ; i++ ) {
        trap_Argv( i, arg, sizeof( arg ) );
        tlen = strlen( arg );
        if ( len + tlen >= MAX_STRING_CHARS - 1 ) {
            break;
        }
        memcpy( line + len, arg, tlen );
        len += tlen;
        if ( i != c - 1 ) {
            line[len] = ' ';
            len++;
        }
    }

    line[len] = 0;

    return line;
}

/*
==================
Cmd_Drop_f

Drops the currenty selected weapon

Dupe fix by henk
==================
*/
void Cmd_Drop_f ( gentity_t* ent )
{
    gentity_t* dropped;

    // spectators cant drop anything since they dont have anything
    if ( ent->client->sess.team == TEAM_SPECTATOR )
    {
        return;
    }

    // Ghosts and followers cant drop stuff
    if ( ent->client->ps.pm_flags & (PMF_GHOST|PMF_FOLLOW) )
    {
        return;
    }
    if(current_gametype.value == GT_HS){
        if ( level.time > level.gametypeRoundTime ) // Henk  07/03/11 -> Don't let ppl drop stuff when the round has ended.
        {
            return;
        }
        if(!ent->client->ps.stats[STAT_WEAPONS] & ( 1 << atoi(ConcatArgs( 1 )) ))
        {
            // The client doesn't have any item to drop.
            ent->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_KNIFE );
            ent->client->ps.ammo[weaponData[WP_KNIFE].attack[ATTACK_NORMAL].ammoIndex]=0;
            ent->client->ps.clip[ATTACK_NORMAL][WP_KNIFE]=weaponData[WP_KNIFE].attack[ATTACK_NORMAL].clipSize;
            ent->client->ps.firemode[WP_KNIFE] = BG_FindFireMode ( WP_KNIFE, ATTACK_NORMAL, WP_FIREMODE_AUTO );
            ent->client->ps.weapon = WP_KNIFE;
            ent->client->ps.weaponstate = WEAPON_READY;
            return;
        }
    }
    if(level.time < ent->client->sess.lastpickup && current_gametype.value == GT_HS){
        return;
    }else{

        // Drop the weapon the client wanted to drop
        dropped = G_DropWeapon ( ent, (weapon_t)atoi(ConcatArgs( 1 )), 1000 );
        if ( !dropped )
        {
            // The client doesn't have any item to drop.
            return;
        }
    }
}


void Cmd_DropItem_f ( gentity_t* ent )
{
    // spectators cant drop anything since they dont have anything
    if ( ent->client->sess.team == TEAM_SPECTATOR )
        return;

    // Ghosts and followers cant drop stuff
    if ( ent->client->ps.pm_flags & (PMF_GHOST|PMF_FOLLOW) )
        return;

    // Nothing to drop
    if ( !ent->client->ps.stats[STAT_GAMETYPE_ITEMS] ){
        G_printInfoMessage(ent, "You don't have any gametype item to drop.");
        return;
    }

    G_DropGametypeItems ( ent, 3000 );
}

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f (gentity_t *ent)
{
    char        *name;
    gitem_t     *it;
    int         i;
    qboolean    give_all;
    gentity_t       *it_ent;
    trace_t     trace;
    char        arg[MAX_QPATH];

    int start;
    int end;
    int l;

    trap_Argv( 1, arg, sizeof( arg ) );

    if ( !Q_stricmp ( arg, "me" ) )
    {
        start = ent->s.number;
        end = start + 1;
    }
    else if ( !Q_stricmp ( arg, "all" ) )
    {
        start = 0;
        end   = MAX_CLIENTS;
    }
    else
    {
        start = atoi ( arg );
        end = start + 1;
    }

    for ( l = start; l < end; l ++ )
    {
        ent = &g_entities[l];

        if ( !ent->inuse )
        {
            continue;
        }

        if ( G_IsClientDead ( ent->client ) )
        {
            continue;
        }

        if ( !CheatsOk( ent ) ) {
            return;
        }

    name = ConcatArgs( 2 );

    if (Q_stricmp(name, "all") == 0)
        give_all = qtrue;
    else
        give_all = qfalse;

    if (give_all || Q_stricmp( name, "health") == 0)
    {
        ent->health = MAX_HEALTH;
        if (!give_all)
            continue;
    }

    if (give_all || Q_stricmp(name, "weapons") == 0)
    {
        ent->client->ps.stats[STAT_WEAPONS] = (1 << level.wpNumWeapons) - 1 - ( 1 << WP_NONE );
        if (!give_all)
            continue;
    }

    if (give_all || Q_stricmp(name, "ammo") == 0)
    {
        for ( i = WP_NONE + 1 ; i < level.wpNumWeapons; i++ )
        {
            int a;

            #ifdef _GOLD
            // Boe!Man 11/11/15: Skip weapons (and remove from outfitting) that have shared ammo in specific gametypes in give me all.
            if(current_gametype.value != GT_HS && i >= WP_M67_GRENADE && i <= WP_MDN11_GRENADE
            && !(current_gametype.value == GT_HZ && (i == WP_M67_GRENADE || i == WP_L2A2_GRENADE))){
                ent->client->ps.stats[STAT_WEAPONS] &= ~(1 << i);
                continue;
            }else if(current_gametype.value == GT_HS && (i == WP_M60_MACHINEGUN || i == WP_MP5)){
                ent->client->ps.stats[STAT_WEAPONS] &= ~(1 << i);
                continue;
            }
            #endif // _GOLD

            for ( a = ATTACK_NORMAL; a < ATTACK_MAX; a ++ )
            {
                ent->client->ps.clip[a][i] = 250; //weaponData[i].attack[a].clipSize;
                ent->client->ps.ammo[weaponData[i].attack[a].ammoIndex] = 250;//ammoData[weaponData[i].attack[a].ammoIndex].max;
            }
        }

        if (!give_all)
            continue;
    }

    if (give_all || Q_stricmp(name, "armor") == 0)
    {
        ent->client->ps.stats[STAT_ARMOR] = MAX_ARMOR;

        if (!give_all)
            continue;
    }

    // spawn a specific item right on the player
    if ( !give_all )
    {
        it = BG_FindItem (name);
        if (!it)
        {
            continue;
        }

        if ( it->giType == IT_GAMETYPE )
        {
            continue;
        }

        it_ent = G_Spawn();
        VectorCopy( ent->r.currentOrigin, it_ent->s.origin );
        it_ent->classname = it->classname;
        G_SpawnItem (it_ent, it);
        FinishSpawningItem(it_ent );
        memset( &trace, 0, sizeof( trace ) );
        Touch_Item (it_ent, ent, &trace);
        if (it_ent->inuse)
        {
            G_FreeEntity( it_ent );
        }
    }

    }
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f (gentity_t *ent)
{
    char    *msg;

    if ( !CheatsOk( ent ) ) {
        return;
    }

    ent->flags ^= FL_GODMODE;
    if (!(ent->flags & FL_GODMODE) )
        msg = "godmode OFF\n";
    else
        msg = "godmode ON\n";

    trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f( gentity_t *ent ) {
    char    *msg;

    if ( !CheatsOk( ent ) ) {
        return;
    }

    ent->flags ^= FL_NOTARGET;
    if (!(ent->flags & FL_NOTARGET) )
        msg = "notarget OFF\n";
    else
        msg = "notarget ON\n";

    trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f( gentity_t *ent ) {
    char    *msg;

    if ( !CheatsOk( ent ) ) {
        return;
    }

    if ( ent->client->noclip ) {
        msg = "noclip OFF\n";
    } else {
        msg = "noclip ON\n";
    }
    ent->client->noclip = (qboolean)!ent->client->noclip;

    trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_LevelShot_f

This is just to help generate the level pictures
for the menus.  It goes to the intermission immediately
and sends over a command to the client to resize the view,
hide the scoreboard, and take a special screenshot
==================
*/
void Cmd_LevelShot_f( gentity_t *ent )
{
    if ( !CheatsOk( ent ) )
    {
        return;
    }

    BeginIntermission();

    trap_SendServerCommand( ent-g_entities, "clientLevelShot" );
}

/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f( gentity_t *ent )
{
    // No killing yourself if your a spectator
    if ( G_IsClientSpectating ( ent->client ) )
    {
        return;
    }

    // No killing yourself if your dead
    if ( G_IsClientDead ( ent->client ) )
    {
        return;
    }

    if (current_gametype.value == GT_HZ){
        if(ent->client->sess.killtime > level.time){ // Boe!Man 7/15/11: He's not allowed to kill himself. Inform him and return.
            if((ent->client->sess.killtime - level.time) > 1000){ // Boe!man 7/15/11: The counter should never show 0.. Just kill if it's actually under 1.
                trap_SendServerCommand ( ent->client - &level.clients[0], va("print\"^3[H&Z] ^7You cannot kill yourself for another %i second(s).\n\"", (ent->client->sess.killtime - level.time) / 1000 ));
                return;
            }
        }
    }

    if(/*current_gametype.value == GT_HS && */level.crossTheBridge && ent->client->sess.team == TEAM_BLUE)
    {
        trap_SendServerCommand ( ent->client - &level.clients[0], "print\"^3[Cross The Bridge] ^7Why would you want to kill yourself?\n\"" );
        return;
    }

    ent->flags &= ~FL_GODMODE;
    ent->client->ps.stats[STAT_HEALTH] = ent->health = -999;
    player_die (ent, ent, ent, 100000, MOD_SUICIDE, HL_NONE, vec3_origin );
}

/*
=================
BroadCastTeamChange

Let everyone know about a team change
=================
*/
void BroadcastTeamChange( gclient_t *client, int oldTeam )
{
    char        message[512];
    char        admin[46];
    char        clan[46];
    qboolean    noAdmin = qfalse;

    if(current_gametype.value == GT_HS){
        switch ( client->sess.team )
        {
            case TEAM_RED:
                strcpy(message, va("%s" S_COLOR_WHITE "\njoined the %s\n\"", client->pers.netname, server_hiderteamprefix.string));
                G_printInfoMessageToAll("%s joined the hiders.", client->pers.cleanName);
                break;

            case TEAM_BLUE:
                strcpy(message, va("%s" S_COLOR_WHITE "\njoined the %s\n\"", client->pers.netname, server_seekerteamprefix.string));
                G_printInfoMessageToAll("%s joined the seekers.", client->pers.cleanName);
                break;

            case TEAM_SPECTATOR:
                if ( oldTeam != TEAM_SPECTATOR )
                {
                    strcpy(message, va("%s" S_COLOR_WHITE "\njoined the %s\n\"", client->pers.netname, server_specteamprefix.string));
                    G_printInfoMessageToAll("%s joined the spectators.", client->pers.cleanName);
                }
                else
                {
                    return;
                }
                break;
        }
    }else if(current_gametype.value == GT_HZ){
        switch ( client->sess.team )
        {
            case TEAM_RED:
                strcpy(message, va("%s" S_COLOR_WHITE "\njoined the %s\n\"", client->pers.netname, server_humanteamprefix.string));
                G_printInfoMessageToAll("%s joined the humans.", client->pers.cleanName);
                break;

            case TEAM_BLUE:
                strcpy(message, va("%s" S_COLOR_WHITE "\njoined the %s\n\"", client->pers.netname, server_zombieteamprefix.string));
                G_printInfoMessageToAll("%s joined the zombies.", client->pers.cleanName);
                break;

            case TEAM_SPECTATOR:
                if ( oldTeam != TEAM_SPECTATOR )
                {
                    strcpy(message, va("%s" S_COLOR_WHITE "\njoined the %s\n\"", client->pers.netname, server_specteamprefix.string));
                    G_printInfoMessageToAll("%s joined the spectators.", client->pers.cleanName);
                }
                else
                {
                    return;
                }
                break;
        }
    }else{
        switch ( client->sess.team )
        {
            case TEAM_RED:
                strcpy(message, va("%s\n^7joined the %s ^7team", client->pers.netname, server_redteamprefix.string));
                G_printInfoMessageToAll("%s joined the red team.", client->pers.cleanName);
                break;

            case TEAM_BLUE:
                strcpy(message, va("%s" S_COLOR_WHITE "\njoined the %s ^7team\n\"", client->pers.netname, server_blueteamprefix.string));
                G_printInfoMessageToAll("%s joined the blue team.", client->pers.cleanName);
                break;

            case TEAM_SPECTATOR:
                if ( oldTeam != TEAM_SPECTATOR )
                {
                    strcpy(message, va("%s" S_COLOR_WHITE "\njoined the %s\n\"", client->pers.netname, server_specteamprefix.string));
                    G_printInfoMessageToAll("%s joined the spectators.", client->pers.cleanName);
                }else
                {
                    return;
                }
                break;

            case TEAM_FREE:
                strcpy(message, G_ColorizeMessage(va("%s" S_COLOR_WHITE "\njoined the \\battle\n\"", client->pers.netname)));
                G_printInfoMessageToAll("%s joined the battle.", client->pers.cleanName);
                break;
        }
    }

    // Boe!Man 6/2/11: Only broadcast the message if the player specifies to by enabling events during compmode. Always broadcast it when compmode's disabled.
    if(cm_enabled.integer == 0 || cm_devents.integer == 0 || cm_enabled.integer == 1 && cm_devents.integer == 1){
        // Boe!Man 3/16/11: Append Clan and/or Admin status if needed.
        if(client->sess.firstTime == qtrue){
            if(client->sess.clanMember == qtrue){
                strncpy(clan, server_ctprefix.string, 45);
            }else{
                strncpy(clan, "", 2);
            }
            if(client->sess.admin > 1){
                if(client->sess.admin == 2){
                    strncpy(admin, server_badminprefix.string, 45);
                }else if(client->sess.admin == 3){
                    strncpy(admin, server_adminprefix.string, 45);
                }else if(client->sess.admin == 4){
                    strncpy(admin, server_sadminprefix.string, 45);
                }
            }else{
                strncpy(admin, "", 2);
                noAdmin = qtrue; // Boe!Man 3/16/11: It doesn't matter if clan is empty, the lining won't fuck up. With an empty admin entry however, it will.. Hence this check.
            }

            // Boe!Man 3/16/11: Broadcast the teamchange.
            if(noAdmin == qfalse){
                G_Broadcast(va("%s\n%s %s", clan, admin, message), BROADCAST_GAME, NULL);
            }else{
                G_Broadcast(va("%s\n%s", clan, message), BROADCAST_GAME, NULL);
            }
        }else{ // Boe!Man 3/18/11: Else we just display the message itself.
            G_Broadcast(message, BROADCAST_GAME, NULL);
        }
    }
    return;

}

/*
=================
SetTeam
=================
*/
void SetTeam( gentity_t *ent, char *s, const char* identity, qboolean forced )
{
    int                 team;
    int                 oldTeam;
    gclient_t           *client;
    int                 clientNum;
    spectatorState_t    specState;
    int                 specClient;
    qboolean            ghost;
    qboolean            noOutfittingChange = qfalse;
    int                 seekers;
    int                 maxhiders;
    // see what change is requested
    //

    client = ent->client;

    clientNum = client - level.clients;
    specClient = 0;
    specState = SPECTATOR_NOT;

    // If an identity was specified then inject it into
    // the clients userinfo
    if ( identity )
    {
        char userinfo[MAX_INFO_STRING];

        ent->client->sess.lastIdentityChange = level.time;

        trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );

        if ( Q_stricmp ( identity, Info_ValueForKey ( userinfo, "identity" ) ) )
        {
            Info_SetValueForKey ( userinfo, "identity", identity );
            Info_SetValueForKey ( userinfo, "team_identity", identity );
            trap_SetUserinfo ( clientNum, userinfo );
        }
        else
        {
            identity = NULL;
        }
    }

    if ( !Q_stricmp( s, "follow1" ) )
    {
        team = TEAM_SPECTATOR;
        specState = SPECTATOR_FOLLOW;
        specClient = -1;
    }
    else if ( !Q_stricmp( s, "follow2" ) )
    {
        team = TEAM_SPECTATOR;
        specState = SPECTATOR_FOLLOW;
        specClient = -2;
    }
    else if ( !Q_stricmp( s, "spectator" ) || !Q_stricmp( s, "s" ) )
    {
        team = TEAM_SPECTATOR;
        specState = SPECTATOR_FREE;
    }
    else if ( level.gametypeData->teams )
    {
        // if running a team game, assign player to one of the teams
        specState = SPECTATOR_NOT;
        if ( !Q_stricmp( s, "red" ) || !Q_stricmp( s, "r" ) )
        {
            team = TEAM_RED;
        }
        else if ( !Q_stricmp( s, "blue" ) || !Q_stricmp( s, "b" ) )
        {
            team = TEAM_BLUE;
        }
        else
        {
            // pick the team with the least number of players
            team = PickTeam( clientNum );
            if(current_gametype.value == GT_HS){
                // Henk 19/01/10 -> Join hiders
                team = TEAM_RED;
            }
        }

        if ( g_teamForceBalance.integer && forced == qfalse /*|| current_gametype.value == GT_HS && forced == qfalse */) // Boe!Man 8/25/11: Don't force balance in H&S, the CVAR is defaulted to 1. In some cases users don't want it.
        {
            int     counts[TEAM_NUM_TEAMS];

            if(current_gametype.value == GT_HS){
                counts[TEAM_BLUE] = TeamCount1(TEAM_BLUE);
                counts[TEAM_RED] = TeamCount1(TEAM_RED);

                // Henk 19/01/10 -> Team balance hiders/seekers
                // Boe!Man 8/12/11: Modified the code (lowered) so there's ALWAYS room for one extra seeker (else it will result in both teams being locked in specific team layouts).
                if (level.customETHiderAmount[0]){
                    int i;

                    // The user put custom values here. Check them.
                    for (i = 0; i < sizeof(level.customETHiderAmount) - 1; i++){
                        if (level.customETHiderAmount[i + 1] == -1){
                            // It seems the maximum of hiders specified is reached. Use that amount of seekers.
                            seekers = i + 1;
                            maxhiders = g_maxclients.integer - seekers;
                            break;
                        }

                        if (counts[TEAM_RED] >= level.customETHiderAmount[i] && counts[TEAM_RED] <= level.customETHiderAmount[i + 1]){
                            seekers = i + 1;
                            maxhiders = level.customETHiderAmount[i + 1] + 1;
                            break;
                        }
                    }
                }else{
                    if(counts[TEAM_RED] < 4){
                        seekers = 1;
                    }else{
                        seekers = (counts[TEAM_RED] + 2) / 5 + 1;
                    }

                    if(counts[TEAM_BLUE] < 2){
                        maxhiders = 6; // Henkie 24/02/10 -> Was 4
                    }else{
                        maxhiders = seekers * 5 - 2;
                    }
                }

                if ( team == TEAM_BLUE && counts[TEAM_BLUE] >= seekers) {
                    trap_SendServerCommand ( client - &level.clients[0], "print\"^3[H&S] ^7Seekers have too many players.\n\"" );

                    // ignore the request
                    return;
                }else if(team == TEAM_RED && counts[TEAM_RED] >= maxhiders){
                    trap_SendServerCommand ( client - &level.clients[0], "print\"^3[H&S] ^7Hiders have too many players.\n\"" );

                    // ignore the request
                    return;
                }
            }else if (current_gametype.value == GT_HZ){
                if (team == TEAM_RED && level.gametypeStartTime != level.time && !level.blueLocked){
                    if(client->sess.team != TEAM_BLUE){
                        trap_SendServerCommand(client - &level.clients[0], "print\"^3[H&Z] ^7Round already started, forcing you to the zombie team.\n\"");
                    }else{
                        trap_SendServerCommand(client - &level.clients[0], "print\"^3[H&Z] ^7Humans are locked, stay put in the zombie team.\n\"");
                    }

                    team = TEAM_BLUE;
                }
            }else{
                counts[TEAM_BLUE] = TeamCount( ent->client->ps.clientNum, TEAM_BLUE, NULL );
                counts[TEAM_RED] = TeamCount( ent->client->ps.clientNum, TEAM_RED, NULL );

                // We allow a spread of two
                if ( team == TEAM_RED && counts[TEAM_RED] - counts[TEAM_BLUE] > 1 )
                {
                    G_printInfoMessage(ent, "Red team has too many players.");
                    // ignore the request
                    return;
                }
                if ( team == TEAM_BLUE && counts[TEAM_BLUE] - counts[TEAM_RED] > 1 )
                {
                    G_printInfoMessage(ent, "Blue team has too many players.");

                    // ignore the request
                    return;
                }
            }

            // It's ok, the team we are switching to has less or same number of players
        }
    }
    else
    {
        // force them to spectators if there aren't any spots free
        team = TEAM_FREE;
    }

    // override decision if limiting the players
    if ( g_maxGameClients.integer > 0 && level.numNonSpectatorClients >= g_maxGameClients.integer )
    {
        team = TEAM_SPECTATOR;
    }

    // decide if we will allow the change
    oldTeam = client->sess.team;
    ghost   = client->sess.ghost;

    if ( team == oldTeam && team != TEAM_SPECTATOR )
    {
        if ( identity )
        {
            // get and distribute relevent paramters
            client->pers.identity = NULL;
            ClientUserinfoChanged( clientNum );
        }

        return;
    }

    if (level.gametypeData->teams && !forced && (!client->sess.admin || (client->sess.admin && g_compMode.integer && cm_enabled.integer >= 1)))
    {
        ///RxCxW - 03.05.05 - 07:19am
        ///if ((team == TEAM_RED) && level.redLocked)
        if ((team == TEAM_RED) && level.redLocked){
            // Boe!Man 2/15/11: H&S messages are different as they use another team prefix.
            if(current_gametype.value == GT_HS){
                G_Broadcast(va("%s ^7are \\locked!", server_hiderteamprefix.string), BROADCAST_GAME, ent);
                G_printInfoMessage(ent, "Hiders are locked.");
            }else{
                G_Broadcast(va("%s ^7team is \\locked!", server_redteamprefix.string), BROADCAST_GAME, ent);
                G_printInfoMessage(ent, "Red team is locked.");
            }
            return;
        }else if ((team == TEAM_BLUE) && level.blueLocked){
            // Boe!Man 2/15/11: H&S messages are different as they use another team prefix.
            if(current_gametype.value == GT_HS){
                G_Broadcast(va("%s ^7are \\locked!", server_seekerteamprefix.string), BROADCAST_GAME, ent);
                G_printInfoMessage(ent, "Seekers are locked.");
            }else{
                G_Broadcast(va("%s ^7team is \\locked!", server_blueteamprefix.string), BROADCAST_GAME, ent);
                G_printInfoMessage(ent, "Blue team is locked.");
            }
            return;
        }
    }

    if(oldTeam == TEAM_SPECTATOR)
    {
        //Ryan march 28 2004 3:24pm
        //keep track of how much time the player spent spectating
        //we'll subtract this later from the players time for awards
        client->sess.totalSpectatorTime += level.time - client->sess.spectatorTime;
        //Ryan

        //clear the invites if we are joining game play
        if(team == TEAM_RED || team == TEAM_BLUE)
        {
            client->sess.invitedByRed = qfalse;
            client->sess.invitedByBlue = qfalse;
        }
    }

    noOutfittingChange = ent->client->noOutfittingChange;

    // he starts at 'base'
    client->pers.teamState.state = TEAM_BEGIN;

    if ( oldTeam != TEAM_SPECTATOR )
    {
        if(team == TEAM_RED || team == TEAM_BLUE)
        {
            client->sess.invitedByRed = qfalse;
            client->sess.invitedByBlue = qfalse;
        }

        if ( ghost )
        {
            G_StopGhosting ( ent );
        }
        else if ( !G_IsClientDead ( client ) )
        {
            // Boe!Man 7/5/13: Also toss the client items in H&S/H&Z (M4/RPG/MM1)..
            if(current_gametype.value == GT_HS || current_gametype.value == GT_HZ){
                TossClientItems(ent);
            }

            // Kill him (makes sure he loses flags, etc)
            ent->flags &= ~FL_GODMODE;
            ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
            player_die (ent, ent, ent, 100000, MOD_TEAMCHANGE, HL_NONE, vec3_origin );

            ent->client->sess.ghost = qfalse;
        }
    }

    // If respawn interval start as a ghost
    if ( level.gametypeRespawnTime[ team ] )
    {
        ghost = qtrue;
    }

    // they go to the end of the line
    if ( team == TEAM_SPECTATOR )
    {
        client->sess.spectatorTime = level.time;
    }

    client->sess.team = (team_t)team;
    client->sess.spectatorState = specState;
    client->sess.spectatorClient = specClient;

    // Kill any child entities of this client to protect against grenade team changers
    G_FreeEnitityChildren ( ent ); // Henk 14/01/11 -> Fix for specnade

    // Always spawn into a ctf game using a respawn timer.
    if(!level.pause)
    {
        if ( team != TEAM_SPECTATOR && level.gametypeData->respawnType == RT_INTERVAL )
        {
            G_SetRespawnTimer ( ent );
            ghost = qtrue;
        }
    }

    //Ryan
    //Ryan june 15 2003
    if(!level.pause && !client->sess.firstTime)
    {
        if(!forced)
        BroadcastTeamChange( client, oldTeam );
    }
    //Ryan
    //BroadcastTeamChange( client, oldTeam );

    // See if we should spawn as a ghost
    if ( team != TEAM_SPECTATOR && level.gametypeData->respawnType == RT_NONE )
    {
        // If there are ghosts already then spawn as a ghost because
        // the game is already in progress.
        if ( (level.gametypeJoinTime && (level.time - level.gametypeJoinTime) > 20000) || noOutfittingChange || client->sess.noTeamChange )
        {
            ghost = qtrue;
        }

        // Boe!Man 6/5/13: Check if the client wants to abuse the spec to team bug. Note this doesn't apply to H&S/H&Z.
        if((current_gametype.value != GT_HS && current_gametype.value != GT_HZ) && client->sess.deathTime > level.gametypeJoinTime)
        {
            ghost = qtrue;
        }

        if(current_gametype.value == GT_HZ){
            if(level.gametypeJoinTime && (level.time - level.gametypeJoinTime) > 10000){
                ghost = qtrue;
            }
        }

        // Boe!Man 9/4/11: Don't allow people to spawn that are reconnecting and are being put in the team.
        if(current_gametype.value == GT_HS && level.cagefight && !client->sess.cageFighter){
            ghost = qtrue;
        }

        if (current_gametype.value != GT_HS && current_gametype.value != GT_HZ){
            int i;
            int teamCount[2];
            int aliveCount[2];

            teamCount[0] = TeamCount(-1, TEAM_RED, &aliveCount[0]);
            teamCount[1] = TeamCount(-1, TEAM_BLUE, &aliveCount[1]);

            for (i = 0; i < 2; i++){
                if (teamCount[i] != aliveCount[i]){
                    ghost = qtrue;
                    break;
                }
            }
        }

        // Spectator to a team doesnt count
        if ( oldTeam != TEAM_SPECTATOR )
        {
            client->sess.noTeamChange = qtrue;
        }
    }

    // If a ghost, enforce it
    if ( ghost )
    {
        #ifdef _3DServer
        if (current_gametype.value == GT_HS && boe_deadMonkey.integer && level.monkeySpawnCount && !level.cagefight && !client->sess.monkeyPreferGhost && team == TEAM_RED){
            client->sess.deadMonkey = level.time;
            ghost = qfalse;
        } else if (team != TEAM_SPECTATOR)
        #else
        if ( team != TEAM_SPECTATOR )
        #endif // _3DServer
        {
            // Make them a ghost again
            G_StartGhosting ( ent );

            // get and distribute relevent paramters
            client->pers.identity = NULL;
            ClientUserinfoChanged( clientNum );

            // Henk 02/02/10 -> Set score to 0 when switching team.
            // Boe!Man 2/8/11: Forcing this to be H&S only.
            if (current_gametype.value == GT_HS){
                if(client->sess.lastTeam != team){
                    ent->client->sess.score = 0;
                    ent->client->sess.kills = 0;
                    ent->client->sess.deaths = 0;
                    client->sess.lastTeam = team;
                }

                ent->client->sess.timeOfDeath = 0; // Boe!Man 8/29/11: Also reset this when switching team (so seekers that won won't get RPG for example).
            }


            // Boe!Man 6/3/12: Reset noroof data when setting team.
            ent->client->sess.isOnRoof = qfalse;
            ent->client->sess.isOnRoofTime = 0;

            CalculateRanks();

            return;
        }
    }

    // get and distribute relevent paramters
    client->pers.identity = NULL;
    ClientUserinfoChanged( clientNum );

    // Henk 02/02/10 -> Set score to 0 when switching team.
    if (current_gametype.value == GT_HS){
        if(team != TEAM_SPECTATOR && client->sess.lastTeam != team){
            ent->client->sess.score = 0;
            ent->client->sess.kills = 0;
            ent->client->sess.deaths = 0;
            client->sess.lastTeam = team;
        }

        ent->client->sess.timeOfDeath = 0; // Boe!Man 8/29/11: Also reset this when switching team (so seekers that won won't get RPG for example).

        // Also check the random grenade.
        if(ent->client->sess.freeze){
            if(ent->client->sess.transformedEntity){
                G_FreeEntity(&g_entities[ent->client->sess.transformedEntity]);
                ent->client->sess.transformedEntity = 0;
            }

            if(ent->client->sess.transformedEntity2){
                G_FreeEntity(&g_entities[ent->client->sess.transformedEntity2]);
                ent->client->sess.transformedEntity2 = 0;
            }

            ent->client->sess.freeze = qfalse;
            ent->client->sess.invisibleGoggles = qfalse;
            strncpy(level.RandomNadeLoc, "Disappeared", sizeof(level.RandomNadeLoc));
        }
    }

    // Boe!Man 6/3/12: Reset noroof data when setting team.
    ent->client->sess.isOnRoof = qfalse;
    ent->client->sess.isOnRoofTime = 0;

    CalculateRanks();

    // Begin the clients new life on the their new team
    ClientBegin( clientNum, qfalse );
}

/*
=================
G_StartGhosting

Starts a client ghosting.  This essentially will kill a player which is alive
=================
*/
void G_StartGhosting ( gentity_t* ent )
{
    int i;

    // Dont start ghosting if already ghosting
    if ( ent->client->sess.ghost )
    {
        return;
    }

    ent->client->sess.ghost = qtrue;
    ent->client->sess.spectatorState = SPECTATOR_FREE;
    ent->client->sess.spectatorClient = -1;
    ent->client->ps.pm_flags |= PMF_GHOST;
    ent->client->ps.stats[STAT_HEALTH] = 100;
    ent->client->ps.pm_type = PM_SPECTATOR;
    ent->client->ps.pm_flags &= ~PMF_FOLLOW;

    trap_UnlinkEntity (ent);

    // stop any following clients
    for ( i = 0 ; i < level.maxclients ; i++ )
    {
        if ( G_IsClientSpectating ( &level.clients[i] )
            && level.clients[i].sess.spectatorState == SPECTATOR_FOLLOW
            && level.clients[i].sess.spectatorClient == ent->s.number )
        {
            G_StopFollowing( &g_entities[i] );
        }
    }
}

/*
=================
G_StopGhosting

Stops a client from ghosting.  The client will be dead after this
call
=================
*/
void G_StopGhosting ( gentity_t* ent )
{
    // Dont stop someone who isnt ghosting in the first place
    if ( !ent->client->sess.ghost )
    {
        return;
    }

    ent->client->sess.ghost = qfalse;
    ent->client->ps.pm_flags &= ~PMF_GHOST;
    ent->client->ps.pm_flags &= ~PMF_FOLLOW;

    if ( ent->client->sess.team == TEAM_SPECTATOR )
    {
        ent->client->ps.pm_type = PM_SPECTATOR;
    }
    else
    {
        ent->client->ps.pm_type = PM_DEAD;
        ent->health = ent->client->ps.stats[STAT_HEALTH] = 0;
    }
}

/*
=================
G_StopFollowing

If the client being followed leaves the game, or you just want to drop
to free floating spectator mode
=================
*/
void G_StopFollowing( gentity_t *ent )
{
    // Cant stop following if not following in the first place
    if ( !(ent->client->ps.pm_flags&PMF_FOLLOW) )
    {
        return;
    }

    // Clear the following variables
    ent->client->ps.pm_flags &= ~PMF_FOLLOW;
    ent->client->sess.spectatorState = SPECTATOR_FREE;
    ent->client->ps.clientNum = ent - g_entities;
    ent->client->ps.zoomFov = 0;
    ent->client->ps.pm_flags &= ~(PMF_GOGGLES_ON|PMF_ZOOM_FLAGS);
    ent->client->ps.persistant[PERS_TEAM] = ent->client->sess.team;
    ent->r.svFlags &= ~SVF_BOT;

    // Ghots dont really become spectators, just psuedo spectators
    if ( ent->client->sess.ghost )
    {
        // Do a start and stop to ensure the variables are all set properly
        G_StopGhosting ( ent );
        G_StartGhosting ( ent );
    }
    else
    {
        ent->client->sess.team = TEAM_SPECTATOR;
        ent->client->ps.persistant[ PERS_TEAM ] = TEAM_SPECTATOR;
    }

    // If we were in fact following someone, then make the angles and origin nice for
    // when we stop
    if ( ent->client->sess.spectatorClient != -1 )
    {
        gclient_t* cl = &level.clients[ent->client->sess.spectatorClient];

        int i;
        for ( i = 0; i < 3; i ++ )
        {
            ent->client->ps.delta_angles[i] = ANGLE2SHORT(cl->ps.viewangles[i] - SHORT2ANGLE(ent->client->pers.cmd.angles[i]));
        }

        VectorCopy ( cl->ps.viewangles, ent->client->ps.viewangles );
        VectorCopy ( cl->ps.origin, ent->client->ps.origin );
        VectorClear ( ent->client->ps.velocity );
        ent->client->ps.movementDir = 0;

        BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qtrue );
    }

    ent->client->sess.spectatorClient = -1;
}

/*
=================
Cmd_Team_f
=================
*/
void Cmd_Team_f( gentity_t *ent )
{
    char team[MAX_TOKEN_CHARS];
    char identity[MAX_TOKEN_CHARS];

    // Need at least the team specified in the arguments
    if ( trap_Argc() < 2 )
    {
        int oldTeam = ent->client->sess.team;
        switch ( oldTeam )
        {
            case TEAM_BLUE:
                trap_SendServerCommand( ent-g_entities, "print \"Blue team\n\"" );
                break;

            case TEAM_RED:
                trap_SendServerCommand( ent-g_entities, "print \"Red team\n\"" );
                break;

            case TEAM_FREE:
                trap_SendServerCommand( ent-g_entities, "print \"Free team\n\"" );
                break;

            case TEAM_SPECTATOR:
                trap_SendServerCommand( ent-g_entities, "print \"Spectator team\n\"" );
                break;
        }

        return;
    }

    // Limit how often one can switch team
    if ( ent->client->switchTeamTime > level.time )
    {
        trap_SendServerCommand( ent-g_entities, "print \"May not switch teams more than once per 3 seconds.\n\"" );
        return;
    }

    trap_Argv( 1, team, sizeof( team ) );
    trap_Argv( 2, identity, sizeof( identity ) );

    SetTeam( ent, team, identity[0]?identity:NULL, qfalse );

    // Remember the team switch time so they cant do it again really quick
    ent->client->switchTeamTime = level.time + 3000;
}


/*
=================
Cmd_Follow_f
=================
*/
void Cmd_Follow_f( gentity_t *ent )
{
    int clientNum;

    if ( trap_Argc() != 2 )
    {
        if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW )
        {
            G_StopFollowing( ent );
        }
        return;
    }

    clientNum = G_clientNumFromArg(ent, 1, "follow", qtrue, qtrue, qtrue, qfalse);

    if(clientNum == -1)
    {
        return;
    }

    // can't follow self
    if ( &level.clients[clientNum] == ent->client )
    {
        return;
    }

    if(ent->client->sess.team == TEAM_SPECTATOR && level.specsLocked)
    {
        if(level.clients[clientNum].sess.team == TEAM_RED && !ent->client->sess.invitedByRed)
        {
            return;
        }
        if(level.clients[clientNum].sess.team == TEAM_BLUE && !ent->client->sess.invitedByBlue)
        {
            return;
        }
    }

// Dissallow following of the enemy if the cvar is set
    //if ( level.gametypeData->teams && !g_followEnemy.integer && ent->client->sess.team != TEAM_SPECTATOR )
    if ( level.gametypeData->teams && (!g_followEnemy.integer || g_compMode.integer) &&
        ent->client->sess.team != TEAM_SPECTATOR && !ent->client->sess.adminspec)
    {
        // Are they on the same team?
        if ( level.clients[clientNum].sess.team != ent->client->sess.team )
        {
            return;
        }
    }

    // first set them to spectator as long as they arent a ghost
    if ( !ent->client->sess.ghost && ent->client->sess.team != TEAM_SPECTATOR )
    {
        SetTeam( ent, "spectator", NULL, qfalse );
    }

    ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
    ent->client->sess.spectatorClient = clientNum;
}

/*
=================
Cmd_FollowCycle_f
=================
*/
void Cmd_FollowCycle_f( gentity_t *ent, int dir )
{
    int     clientnum;
    int     deadclient;
    int     original;

    // first set them to spectator
    if ( !ent->client->sess.ghost && ent->client->sess.team != TEAM_SPECTATOR )
    {
        SetTeam( ent, "spectator", NULL, qfalse );
    }

    if ( dir != 1 && dir != -1 )
    {
        Com_Error(ERR_FATAL, "Cmd_FollowCycle_f: bad dir %i", dir);
    }

    if ( ent->client->sess.spectatorClient == -1 )
    {
        clientnum = original = ent->s.number;
    }
    else
    {
        clientnum = original = ent->client->sess.spectatorClient;
    }

    deadclient = -1;
    do
    {
        clientnum += dir;
        if ( clientnum >= level.maxclients )
        {
            clientnum = 0;
        }
        if ( clientnum < 0 )
        {
            clientnum = level.maxclients - 1;
        }

        // can only follow connected clients
        if ( level.clients[ clientnum ].pers.connected != CON_CONNECTED )
        {
            continue;
        }

        // can't follow another spectator
        if ( G_IsClientSpectating ( &level.clients[ clientnum ] ) )
        {
            continue;
        }

        // Cant switch to dead people unless there is nobody else to switch to
        #ifdef _3DServer
        if ( G_IsClientDead ( &level.clients[clientnum] ) && !level.clients[clientnum].sess.deadMonkey )
        #else
        if ( G_IsClientDead ( &level.clients[clientnum] ) )
        #endif // _3DServer
        {
            deadclient = clientnum;
            continue;
        }
        ent->client->sess.spectating = clientnum;
        //Ryan
        if(g_compMode.integer && !ent->client->sess.adminspec && !ent->client->sess.referee)
        {
            //No enemy following in compmode
            if(ent->client->sess.team != TEAM_SPECTATOR && level.clients[ clientnum ].sess.team != ent->client->sess.team)
            {
                continue;
            }

            if(ent->client->sess.team == TEAM_SPECTATOR)
            {
                if(level.clients[ clientnum ].sess.team == TEAM_RED)
                {
                    if(level.specsLocked && !ent->client->sess.invitedByRed)
                    {
                        continue;
                    }
                }
                else if(level.clients[ clientnum ].sess.team == TEAM_BLUE)
                {
                    if(level.specsLocked && !ent->client->sess.invitedByBlue)
                    {
                        continue;
                    }
                }
            }
        }
        //Ryan

        // Dissallow following of the enemy if the cvar is set
        //Ryan june 7 2003
            //if ( level.gametypeData->teams && !g_followEnemy.integer && ent->client->sess.team != TEAM_SPECTATOR )
        if ( level.gametypeData->teams && !g_followEnemy.integer && ent->client->sess.team != TEAM_SPECTATOR && !ent->client->sess.adminspec)
        //Ryan
        {
            // Are they on the same team?
            if ( level.clients[ clientnum ].sess.team != ent->client->sess.team )
            {
                continue;
            }
        }

        // this is good, we can use it
        ent->client->sess.spectatorClient = clientnum;
        ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
        return;

    } while ( clientnum != original );

    // If being forced to follow and there is a dead client to jump to, then jump to them now
    //Ryan
    //if ( deadclient != -1 && g_forceFollow.integer )
    if ( deadclient != -1 && (g_forceFollow.integer || g_compMode.integer))
    //Ryan
    {
        // this is good, we can use it
        ent->client->sess.spectatorClient = deadclient;
        ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
        return;
    }

    G_StopFollowing( ent );

    // leave it where it was
}

/*
==================
G_SayTo
==================
*/
static void G_SayTo( gentity_t *ent, gentity_t *other, int mode, const char *name, const char *message )
{
    qboolean     ghost = qfalse;
    qboolean     spec  = qfalse;
    qboolean     beep  = qfalse;
    char    type[128];
    char    admin[128];
    char    star[128];
    int i;

    //G_LogPrintf("Starting with chat\n");
    if (!other)
    {
        return;
    }

    if (!other->inuse)
    {
        return;
    }else if (!other->client)
    {
        return;
    }else if ( other->client->pers.connected != CON_CONNECTED )
    {
        return;
    }else if ( mode == SAY_TEAM  && !OnSameTeam(ent, other) )
    {
        return;
    }

    // Boe!Man 1/17/10: We do not wish to send the chat when it's Admin/Clan only..
    if ( mode == ADM_CHAT  && !other->client->sess.admin )
        return;
    else if (mode == REF_CHAT && !other->client->sess.referee && !other->client->sess.admin)
        return;
    else if ( mode == SADM_CHAT  && other->client->sess.admin != 4 )
        return;
    else if (mode == CLAN_CHAT && !other->client->sess.clanMember )
        return;
    else if (mode == CLAN_TALK && !other->client->sess.clanMember )
        return;
    // Boe!Man 9/26/10: Hey Admin! chat needs to be visible for admins AND the one saying it.
    else if (mode == CADM_CHAT && ent->s.number != other->s.number && other->client->sess.admin < 2)
        return;

    if ( !level.intermissiontime && !level.intermissionQueued )
    {
        // Spectators cant talk to alive people
        if ( ent->client->sess.team == TEAM_SPECTATOR )
        {
            spec = qtrue;
        }

        if ( level.gametypeData->respawnType == RT_NONE )
        {
            // Dead people cant talk to alive people
            if ( !spec && G_IsClientDead ( ent->client ) )
            {
                ghost = qtrue;
            }

            // Boe!Man 1/17/10: Admins need to be heard, even if they're dead/spec.
            if(mode >= ADM_TALK && mode <= CADM_CHAT)
            {
                ghost = qfalse;
                spec = qfalse;
            }

            // If the client we are talking to is alive then a check
            // must be made to see if this talker is alowed to speak to this person
            if ( ent->s.number != other->s.number && !G_IsClientDead ( other->client ) && !G_IsClientSpectating( other->client) && (ghost || spec))
            {
                return;
            }

        }
    }

    for(i=0;i<ent->client->sess.IgnoredClientCount;i++){
        if(ent->client->sess.IgnoredClients[i] == other->s.number){
            return;
        }
    }

    strcpy(admin, ""); // Boe!Man 1/18/10: Clean the Admin data.

    if (mode != SAY_TELL){
        // Boe!Man 1/7/10: Team prefixes.
        if (ghost)
        {
            strcpy(type, "^7[^Cg^7] ");
        }
        else if (spec)
        {
            strcpy(type, "^7[^Cs^7] ");
        }
        // Boe!Man 4/5/10: We delete the team prefixes.
        // Boe!Man 2/8/11: And we re-add them for Hide&Seek lol.

        else if (ent->client->sess.team == TEAM_RED && current_gametype.value == GT_HS)
        {
            strcpy(type, "^7[^1h^7] ");
        }
        else if (ent->client->sess.team == TEAM_BLUE && current_gametype.value == GT_HS)
        {
            strcpy(type, "^7[^ys^7] ");
        }
        else if (ent->client->sess.team == TEAM_RED && current_gametype.value == GT_HZ)
        {
            strcpy(type, "^7[^1h^7] ");
        }
        else if (ent->client->sess.team == TEAM_BLUE && current_gametype.value == GT_HZ)
        {
            strcpy(type, "^7[^yz^7] ");
        }

        // Boe!Man 4/6/10: And replace the type with something, well nothing.
        else{
            strcpy(type, "");
        }
    }else{
        strcpy(type, "");
    }

    // Boe!Man 1/17/10: Admin Talk/Chat.
    if(mode >= ADM_TALK && mode <= CADM_CHAT){ // Henk 03/05/10 -> Optimized so cpu can calculate faster
        strcpy(star, server_starprefix.string);
        strcpy(admin, "");
    }else if(mode != SAY_TELL){
        strcpy(star, "");
        // Boe!Man 1/6/10: Admin prefixes. - Update 1/18/10: New CVARs for prefixes if the hoster wishes to change the values.
        if(ent->client->sess.admin == 2){
            strcpy(admin, server_badminprefix.string);
        }else if(ent->client->sess.admin == 3){
            strcpy(admin, server_adminprefix.string);
        }else if(ent->client->sess.admin == 4){
            strcpy(admin, server_sadminprefix.string);
        }else{
            strcpy(admin, "");
        }

        // Boe!Man 11/5/12: Fix for space in chat when *adminprefix CVAR was empty but the client was still an Admin.
        if(strlen(admin) > 0){
            Q_strcat(admin, sizeof(admin), " ");
        }
    }else{
        strcpy(star, "");
        strcpy(admin, "");
    }

    // Boe!Man 1/17/10: Different kinds of Talking 'Modes'.


    // Boe!Man 4/20/11: Should the beep be enabled?
    if (cm_devents.integer == 1 && cm_enabled.integer == 1 || cm_devents.integer == 0 && cm_enabled.integer != 1){
        beep = qtrue;
    }

    switch(mode)
    {
        case ADM_CHAT:
            strcpy(type, server_acprefix.string);
            if (beep == qtrue && g_specialChatSound.integer){
                #ifndef _DEMO
                Boe_ClientSound(other, G_SoundIndex("sound/misc/c4/beep.mp3", qtrue));
                #else
                Boe_ClientSound(other, level.actionSoundIndex);
                #endif // not _DEMO
            }
            break;
        case SADM_CHAT:
            strcpy(type, server_scprefix.string);
            if (beep == qtrue && g_specialChatSound.integer){
                #ifndef _DEMO
                Boe_ClientSound(other, G_SoundIndex("sound/misc/c4/beep.mp3", qtrue));
                #else
                Boe_ClientSound(other, level.actionSoundIndex);
                #endif // not _DEMO
            }
            break;
        case ADM_TALK:
            if(ent->client->sess.admin == 2){
            strcpy(type, server_badminprefix.string);
            }else if(ent->client->sess.admin == 3){
            strcpy(type, server_adminprefix.string);
            }else if(ent->client->sess.admin == 4){
            strcpy(type, server_sadminprefix.string);
            }
            if (beep == qtrue){
                #ifndef _DEMO
                Boe_ClientSound(other, G_SoundIndex("sound/misc/c4/beep.mp3", qtrue));
                #else
                Boe_ClientSound(other, level.actionSoundIndex);
                #endif // not _DEMO
            }
            break;
        case CADM_CHAT:
            strcpy(type, server_caprefix.string);
            if (beep == qtrue && g_specialChatSound.integer){
                #ifndef _DEMO
                Boe_ClientSound(other, G_SoundIndex("sound/misc/c4/beep.mp3", qtrue));
                #else
                Boe_ClientSound(other, level.actionSoundIndex);
                #endif // not _DEMO
            }
            break;
        case CLAN_CHAT:
            strcpy(type, server_ccprefix.string);
            if (beep == qtrue && g_specialChatSound.integer){
                #ifndef _DEMO
                Boe_ClientSound(other, G_SoundIndex("sound/misc/c4/beep.mp3", qtrue));
                #else
                Boe_ClientSound(other, level.actionSoundIndex);
                #endif // not _DEMO
            }
            break;
        case CLAN_TALK:
            strcpy(type, server_ctprefix.string);
            if (beep == qtrue){
                #ifndef _DEMO
                Boe_ClientSound(other, G_SoundIndex("sound/misc/c4/beep.mp3", qtrue));
                #else
                Boe_ClientSound(other, level.actionSoundIndex);
                #endif // not _DEMO
            }
            break;
        default:
            break;
    }

    // Boe!Man 1/6/10 - Update 2/8/11: Finally fixing these space issues. This would be the best way for it.
    if(ent->client->sess.admin > 0 && mode >= ADM_TALK && mode <= SADM_CHAT || mode == CLAN_CHAT || mode == CADM_CHAT || mode == CLAN_TALK){ // Henk 13/02/11 -> Fixed.
        trap_SendServerCommand( other-g_entities, va("%s %d \"%s %s %s%s %s\"", // Boe!Man 1/6/10: Adding prefixes. - Update 1/17/10: Adding Admin Talk/Chat prefixes. - Update 5/8/10: Solved message problem.
                                mode == SAY_TEAM ? "tchat" : "chat",
                                ent->s.number,
                                // Boe!Man 1/6/10: Adding the Admin prefix in front of the chat. - Update 1/17/10.
                                star, type, name, message, star)); // Boe!Man 1/17/10: Adding stars.
    }else if(ent->client->sess.admin > 0){
        trap_SendServerCommand( other-g_entities, va("%s %d \"%s%s%s%s\"", // Boe!Man 1/6/10: Adding prefixes. - Update 1/17/10: Adding Admin Talk/Chat prefixes. - Update 5/8/10: Solved message problem.
                                mode == SAY_TEAM ? "tchat" : "chat",
                                ent->s.number,
                                // Boe!Man 1/6/10: Adding the Admin prefix in front of the chat. - Update 1/17/10.
                                type, admin, name, message));
    }else{
        trap_SendServerCommand( other-g_entities, va("%s %d \"%s%s%s\"", // Boe!Man 1/6/10: Adding prefixes. - Update 1/17/10: Adding Admin Talk/Chat prefixes. - Update 5/8/10: Solved message problem.
                            mode == SAY_TEAM ? "tchat" : "chat",
                            ent->s.number,
                            // Boe!Man 1/6/10: Adding the Admin prefix in front of the chat. - Update 1/17/10.
                            type, name, message));
    }
}

/*
==================
G_GetChatPrefix
==================
*/
void G_GetChatPrefix ( gentity_t* ent, gentity_t* target, int mode, char* name, int nameSize )
{
    const char* namecolor;
    char        location[64];
    qboolean    locationOk = qtrue;

    // Spectators and ghosts dont show locations
    if ( ent->client->ps.pm_type == PM_DEAD || G_IsClientSpectating ( ent->client ) )
    {
        locationOk = qfalse;
    }

    if ( !level.gametypeData->teams && mode == SAY_TEAM )
    {
        mode = SAY_ALL;
    }

    if ( level.gametypeData->teams )
    {
        switch ( ent->client->sess.team )
        {
            case TEAM_BLUE:
                namecolor = S_COLOR_BLUE;
                break;

            case TEAM_RED:
                namecolor = S_COLOR_RED;
                break;

            default:
                namecolor = S_COLOR_WHITE;
                break;
        }
    }
    else
    {
        namecolor = S_COLOR_WHITE;
    }

    switch ( mode )
    {
        default:
        case SAY_ALL:
            Com_sprintf (name, nameSize, "%s%s%s: ", namecolor, ent->client->pers.netname, S_COLOR_WHITE );

            break;

        case SAY_TEAM:

            if ( locationOk && Team_GetLocationMsg(ent, location, sizeof(location)))
            {
                Com_sprintf ( name, nameSize, "%s(%s%s) %s(%s): ",
                              namecolor,
                              ent->client->pers.netname,
                              namecolor,
                              S_COLOR_WHITE, location );
            }
            else
            {
                Com_sprintf ( name, nameSize, "%s(%s%s)%s: ",
                              namecolor,
                              ent->client->pers.netname,
                              namecolor,
                              S_COLOR_WHITE );
            }
            break;

        case SAY_TELL:
        case SAY_TELL_SELF:
            Com_sprintf ( name, nameSize, "^7[%sPM ^7%s %s^7]%s: ",
                            namecolor,
                            (mode == SAY_TELL) ? "from" : "to",
                            ent->client->pers.cleanName,
                            S_COLOR_WHITE );
            break;
    }

    strcat ( name, S_COLOR_GREEN );
}

/*
==================
G_Say
==================
*/
void G_Say ( gentity_t *ent, gentity_t *target, int mode, const char *chatText )
{
    int         j;
    gentity_t   *other;
    char        text[MAX_SAY_TEXT];
    char        name[128]; // Henk 15/12/2010 -> Increased from 64 -> 128; Will fix the name dissapearing.

    // Logging stuff
    switch ( mode )
    {
        case SAY_ALL:
            G_LogPrintf( "say: %s: %s\n", ent->client->pers.netname, chatText );
            break;

        case SAY_TEAM:
            G_LogPrintf( "sayteam: %s: %s\n", ent->client->pers.netname, chatText );
            break;
    }

    // Boe!Man 8/25/11: Also log special chat when the CVAR is enabled.
    if(g_logSpecialChat.integer){
        switch ( mode )
        {
            case ADM_TALK:
                G_LogPrintf( "admtalk: %s: %s\n", ent->client->pers.netname, chatText );
                break;

            case ADM_CHAT:
                G_LogPrintf( "admchat: %s: %s\n", ent->client->pers.netname, chatText );
                break;

            case SADM_CHAT:
                G_LogPrintf( "sadmchat: %s: %s\n", ent->client->pers.netname, chatText );
                break;

            case CLAN_TALK:
                G_LogPrintf( "clantalk: %s: %s\n", ent->client->pers.netname, chatText );
                break;

            case CLAN_CHAT:
                G_LogPrintf( "clanchat: %s: %s\n", ent->client->pers.netname, chatText );
                break;

            case CADM_CHAT:
                G_LogPrintf( "heyadmin: %s: %s\n", ent->client->pers.netname, chatText );
                break;
        }
    }

    // Generate the chat prefix
    G_GetChatPrefix ( ent, target, mode, name, sizeof(name) );
    if (mode == SAY_TELL_SELF){
        mode = SAY_TELL;
    }

    // Save off the chat text
    Q_strncpyz( text, chatText, sizeof(text) );

    if(ent && ent->client){ // Henk 03/07/11 -> Reset inactivity timer when chatting
        ent->client->inactivityTime = level.time + g_inactivity.integer * 1000;
        ent->client->inactivityWarning = qfalse;
    }

    if ( target && target->inuse)
    {
        G_SayTo( ent, target, mode, name, text );
        return;
    }

    // echo the text to the console
    if ( g_dedicated.integer )
    {
        Com_Printf( "%s%s\n", name, text);
    }

    // send it to all the apropriate clients
    for (j = 0; j < level.numConnectedClients; j++)
    {
        other = &g_entities[level.sortedClients[j]];
        G_SayTo( ent, other, mode, name, text );
    }
}

void Cmd_Say_f( gentity_t *ent, int mode, qboolean arg0 ) {
    char        *p;
    char        newp[512] = "";
    // Boe!Man 1/10/10
    int         i;
    int         a = 0;
    // Boe!Man 5/3/10: Fix.
    qboolean    acmd = qfalse;
    char        originalTextBuffer[MAX_SAY_TEXT];
    char        fullTextBuffer[MAX_SAY_TEXT];
    char        cmd[MAX_SAY_TEXT];
    int         ignore = -1;
    qboolean    command, parseTokens = qtrue;

    void    *GP2, *group;
    char txtlevel[2];
    char name[10];
    char action[512];
    char message[512];
    char broadcast[512];

    if(!ent || !ent->client){
        return;
    }

    // Boe!Man 4/30/14: Check if the client said this while in-game and not in the loading screen (to avoid spam).
    if(!ent->client->pers.enterTime){
        return;
    }

    if ( trap_Argc () < 2 && !arg0 )
        return;
    if (arg0)
        p = ConcatArgs( 0 );
    else if(mode >= ADM_TALK && mode <= CADM_CHAT)
        p = ConcatArgs( 2 );
    else
        p = ConcatArgs( 1 );

    // Save a copy of p on the stack.
    strncpy(originalTextBuffer, p, sizeof(originalTextBuffer));

    // Text might turn into something else (not terminated?), so null terminate the buffer now.
    originalTextBuffer[strlen(p)] = '\0';

    // Try to remove additional carets from the buffer, and make p point to our modified buffer.
    G_RemoveAdditionalCarets(originalTextBuffer);
    p = originalTextBuffer;

    // Is there anything to say now or did we remove everything from the buffer? If so, it's useless to continue here.
    if(strlen(p) == 0){
        return;
    }

    // Now fill the full text buffer which we're allowed to modify later on.
    strncpy(fullTextBuffer, originalTextBuffer, sizeof(fullTextBuffer));

    // Boe!Man 6/5/15: Get the first argument.
    if (trap_Argc() > 2) {
        trap_Argv(1, cmd, sizeof(cmd));
    }else{
        strncpy(cmd, G_GetChatArgument(0), sizeof(cmd));
    }

    Q_strlwr(cmd);
    Q_strlwr(fullTextBuffer);

    // Boe!Man 1/24/10: Different kinds of Talk during Gameplay. -- Update 3/21/11: No need to be admin in order to use this..
    if (strstr(fullTextBuffer, "!at")){
        if (ent->client->sess.admin){
            command = qtrue;
            p = ConcatArgs(1);

            for (i = 0; i <= strlen(p); i++) {
                if (p[i] == '!' && (p[i + 1] == 'a' || p[i + 1] == 'A') && (p[i + 2] == 't' || p[i + 2] == 'T')) {
                    if (strlen(p) > i + 3 && p[i + 3] != ' ') {
                        command = qfalse;
                        break;
                    }
                    ignore = i;
                }

                if (ignore == -1){
                    newp[a] = p[i];
                    a += 1;
                }else if (i == ignore || i == ignore + 1 || i == ignore + 2){
                    if (a != 0)
                        i += 1; // Fix for spaces
                }else{
                    if (a == 0 && p[i] == ' ') // Fix for spaces
                        continue;

                    newp[a] = p[i];
                    a += 1;
                }
            }

            if (command) {
                mode = ADM_TALK;
                acmd = qtrue;
                strcpy(p, newp);
            }
        }else{
            command = qtrue;
            p = ConcatArgs(1);

            for (i = 0; i <= strlen(p); i++) {
                if (p[i] == '!' && (p[i + 1] == 'a' || p[i + 1] == 'A') && (p[i + 2] == 't' || p[i + 2] == 'T')) {
                    if (strlen(p) > i + 3 && p[i + 3] != ' ') {
                        command = qfalse;
                        break;
                    }
                    ignore = i;
                }

                if (ignore == -1){
                    newp[a] = p[i];
                    a += 1;
                }else if (i == ignore || i == ignore + 1 || i == ignore + 2){
                    if (a != 0)
                        i += 1; // Fix for spaces
                }else{
                    if (a == 0 && p[i] == ' ') // Fix for spaces
                        continue;

                    newp[a] = p[i];
                    a += 1;
                }
            }

            if(command){
                mode = CADM_CHAT;
                acmd = qtrue;
                strcpy(p, newp);
            }
        }
    }
    else if (strstr(fullTextBuffer, "!ac")){
        command = qtrue;
        p = ConcatArgs(1);

        for (i = 0; i <= strlen(p); i++) {
            if (p[i] == '!' && (p[i + 1] == 'a' || p[i + 1] == 'A') && (p[i + 2] == 'c' || p[i + 2] == 'C')) {
                if (strlen(p) > i + 3 && p[i + 3] != ' ') {
                    command = qfalse;
                    break;
                }
                ignore = i;
            }

            if (ignore == -1){
                newp[a] = p[i];
                a += 1;
            }else if (i == ignore || i == ignore + 1 || i == ignore + 2){
                if (a != 0)
                    i += 1; // Fix for spaces
            }else{
                if (a == 0 && p[i] == ' ') // Fix for spaces
                    continue;

                newp[a] = p[i];
                a += 1;
            }
        }

        if(command){
            // Boe!Man 8/25/11: If a non-admin is using Admin Chat, force it to Hey Admin instead.
            if(ent->client->sess.admin){
                mode = ADM_CHAT;
            }else{
                mode = CADM_CHAT;
            }

            acmd = qtrue;
            strcpy(p, newp);
        }
    }
    else if (strstr(fullTextBuffer, "!sc") && ent->client->sess.admin == 4){
        command = qtrue;
        p = ConcatArgs(1);

        for (i = 0; i <= strlen(p); i++) {
            if (p[i] == '!' && (p[i + 1] == 's' || p[i + 1] == 'S') && (p[i + 2] == 'c' || p[i + 2] == 'C')) {
                if (strlen(p) > i + 3 && p[i + 3] != ' ') {
                    command = qfalse;
                    break;
                }
                ignore = i;
            }

            if (ignore == -1){
                newp[a] = p[i];
                a += 1;
            }else if (i == ignore || i == ignore + 1 || i == ignore + 2){
                if (a != 0)
                    i += 1; // Fix for spaces
            }else{
                if (a == 0 && p[i] == ' ') // Fix for spaces
                    continue;

                newp[a] = p[i];
                a += 1;
            }
        }

        if(command){
            mode = SADM_CHAT;
            acmd = qtrue;
            strcpy(p, newp);
        }
    }
    // Boe!Man 4/17/10: Clan chat.
    else if (strstr(fullTextBuffer, "!cc") && ent->client->sess.clanMember){
        command = qtrue;
        p = ConcatArgs(1);

        for (i = 0; i <= strlen(p); i++) {
            if (p[i] == '!' && (p[i + 1] == 'c' || p[i + 1] == 'C') && (p[i + 2] == 'c' || p[i + 2] == 'C')) {
                if (strlen(p) > i + 3 && p[i + 3] != ' ') {
                    command = qfalse;
                    break;
                }
                ignore = i;
            }

            if (ignore == -1){
                newp[a] = p[i];
                a += 1;
            }else if (i == ignore || i == ignore + 1 || i == ignore + 2){
                if (a != 0)
                    i += 1; // Fix for spaces
            }else{
                if (a == 0 && p[i] == ' ') // Fix for spaces
                    continue;

                newp[a] = p[i];
                a += 1;
            }
        }

        if (command) {
            mode = CLAN_CHAT;
            acmd = qtrue;
            strcpy(p, newp);
        }
    }
    else if (strstr(fullTextBuffer, "!ct") && ent->client->sess.clanMember){
        command = qtrue;
        p = ConcatArgs(1);

        for (i = 0; i <= strlen(p); i++){
            if (p[i] == '!' && (p[i + 1] == 'c' || p[i + 1] == 'C') && (p[i + 2] == 't' || p[i + 2] == 'T')) {
                if (strlen(p) > i + 3 && p[i + 3] != ' ') {
                    command = qfalse;
                    break;
                }

                ignore = i;
            }

            if (ignore == -1){
                newp[a] = p[i];
                a += 1;
            }else if (i == ignore || i == ignore + 1 || i == ignore + 2){
                if (a != 0)
                    i += 1; // Fix for spaces
            }else{
                if (a == 0 && p[i] == ' ') // Fix for spaces
                    continue;

                newp[a] = p[i];
                a += 1;
            }
        }

        if (command) {
            mode = CLAN_TALK;
            acmd = qtrue;
            strcpy(p, newp);
        }
    }
    else if (strstr(fullTextBuffer, "!ca")){
        p = ConcatArgs(1);
        for (i = 0; i <= strlen(p); i++){
            if (p[i] == '!' && (p[i + 1] == 'c' || p[i + 1] == 'C') && (p[i + 2] == 'a' || p[i + 2] == 'A')) {
                if (strlen(p) > i + 3 && p[i + 3] != ' ') {
                    command = qfalse;
                    break;
                }

                ignore = i;
            }

            if (ignore == -1){
                newp[a] = p[i];
                a += 1;
            }else if (i == ignore || i == ignore + 1 || i == ignore + 2){
                if (a != 0)
                    i += 1; // Fix for spaces
            }else{
                if (a == 0 && p[i] == ' ') // Fix for spaces
                    continue;

                newp[a] = p[i];
                a += 1;
            }
        }

        if(command){
            // Boe!Man 8/25/11: If an admin is using Hey Admin, force it to Admin Chat instead.
            if(ent->client->sess.admin){
                mode = ADM_CHAT;
            }else{
                mode = CADM_CHAT;
            }
            acmd = qtrue;
            strcpy(p, newp);
        }
    }
    else if (strcmp(cmd, "!pm") == 0){
        int client;
        gentity_t *target;

        // Boe!Man 1/29/15: Check for proper client.
        client = G_clientNumFromArg(ent, 1, "!pm", qfalse, qtrue, qtrue, qtrue);
        if (client < 0){
            return;
        }

        // Check target.
        target = &g_entities[client];
        if (target->client->pers.connected != CON_CONNECTED){
            G_printInfoMessage(ent, "The specified player isn't fully connected yet.");
        }

        if(ent == target){
            G_printInfoMessage(ent, "You cannot send a private message to yourself.");
            return;
        }

        if (g_compMode.integer > 0 && cm_enabled.integer > 1 && g_matchDisableTell.integer && ent->client->sess.team == TEAM_SPECTATOR && !OnSameTeam(ent, target)){
            G_printInfoMessage(ent, "You cannot send a private message to players in-game.");
            return;
        }

        // Copy the message contents.
        if (trap_Argc() >= 4){
            // Calling from console.
            p = ConcatArgs(3);
        }else if(trap_Argc() == 2){
            // Calling from chat.
            for (i = 0; i < 2; i++){
                p = strstr(p, " ");

                if (p == NULL || strlen(p) < 2){
                    G_printInfoMessage(ent, "You should specify a message to send.");
                    return;
                }
                p++;
            }
        }else{
            G_printInfoMessage(ent, "You should specify a message to send.");
            return;
        }

        Boe_Tokens(ent, p, SAY_TELL, qtrue);
        Boe_Tokens(ent, p, SAY_TELL, qfalse);

        G_Say(ent, target, SAY_TELL, p);
        // don't tell to the player self if it was already directed to this player
        // also don't send the chat back to a bot
        if (ent != target && !(ent->r.svFlags & SVF_BOT)) {
            G_Say(target, ent, SAY_TELL_SELF, p);
        }

        // Save the client ID of who we sent this to. This allows easy replying with !re.
        if (client != ent->client->sess.lastPmClient){
            ent->client->sess.lastPmClient = client;
        }
        if (ent->s.number != target->client->sess.lastPmClient){
            target->client->sess.lastPmClient = ent->s.number;
            target->client->sess.lastPmClientChange = level.time;
        }

        return;
    }
    else if (strcmp(cmd, "!re") == 0){
        int client = ent->client->sess.lastPmClient;
        gentity_t *target = NULL;

        if(client != -1){
            target = &g_entities[client];
        }

        if (client == -1 || !target || !target->client || target->client->pers.connected != CON_CONNECTED){
            G_printInfoMessage(ent, "You don't have anyone to reply to (or the client went offline).");
            G_printInfoMessage(ent, "Please send your message with '!pm <id/name> <message>' instead.");

            // Reset client.
            ent->client->sess.lastPmClient = -1;
            return;
        }else if ((ent->client->sess.lastPmClientChange + 1000) > level.time){
            G_printInfoMessage(ent, "You just got PMed by someone else - as security measure we're not sending your message.");

            ent->client->sess.lastPmClient = -1;
            return;
        }else if (g_compMode.integer > 0 && cm_enabled.integer > 1 && g_matchDisableTell.integer && ent->client->sess.team == TEAM_SPECTATOR && !OnSameTeam(ent, target)){
            G_printInfoMessage(ent, "You cannot send a private message to players in-game.");
            return;
        }

        // Copy the message contents.
        if (trap_Argc() >= 3){
            // Calling from console.
            p = ConcatArgs(2);
        }else if (trap_Argc() == 2){
            if (p == NULL || strlen(p) < 2){
                G_printInfoMessage(ent, "You should specify a message to send.");
                return;
            }
        }else{
            G_printInfoMessage(ent, "You should specify a message to send.");
            return;
        }

        for (i = 0; i <= strlen(p); i++){
            if (p[i] == '!' && (p[i + 1] == 'r' || p[i + 1] == 'R') && (p[i + 2] == 'e' || p[i + 2] == 'E')){
                ignore = i;
            }
            if (ignore == -1){
                newp[a] = p[i];
                a += 1;
            }
            else if (i == ignore || i == ignore + 1 || i == ignore + 2){
                if (a != 0)
                    i += 1; // Fix for spaces
            }
            else{
                if (a == 0 && p[i] == ' ') // Fix for spaces
                    continue;
                newp[a] = p[i];
                a += 1;
            }
        }

        Boe_Tokens(ent, newp, SAY_TELL, qtrue);
        Boe_Tokens(ent, newp, SAY_TELL, qfalse);

        G_Say(ent, target, SAY_TELL, newp);
        // don't tell to the player self if it was already directed to this player
        // also don't send the chat back to a bot
        if (ent != target && !(ent->r.svFlags & SVF_BOT)) {
            G_Say(target, ent, SAY_TELL_SELF, newp);
        }

        return;
    }

    // Boe!Man 1/8/11: Only go through this cycle if the client indeed has admin powers. If not, save on resources.
    if(ent->client->sess.admin > 0 && !acmd){
        command = qfalse;

        // Henk loop through my admin command array
        for(i = 0;i < AdminCommandsSize; i++){
            if (strcmp(cmd, AdminCommands[i].shortCmd) == 0 || strcmp(cmd, va("!%s", AdminCommands[i].adminCmd)) == 0){
                command = qtrue;
                if(ent->client->sess.admin >= *AdminCommands[i].adminLevel){
                    // Execute the Admin command and handle the post processing (logging, broadcast, etc.) for some commands.
                    G_postExecuteAdminCommand(i, AdminCommands[i].Function(2, ent, qtrue), ent);
                }else{
                    if(ent->client->sess.referee && strcmp(cmd, "!l") == 0){ // exception for referee lock
                        adm_lockTeam(1, ent, qtrue);
                    }
                    G_printInfoMessage(ent, "Your Admin level is too low to use this command.");
                }

                break;
            }
        }

        // check custom commands
        if(!command && g_enableCustomCommands.integer){
            GP2 = trap_GP_ParseFile(g_customCommandsFile.string, qtrue, qfalse);
            if(GP2){
                group = trap_GPG_GetSubGroups(GP2);
                while(group){
                    trap_GPG_FindPairValue(group, "ShortCommand", "none", name);
                    Q_strlwr(name);
                    if(!strstr(name, "none")){
                        if(strcmp(cmd, name) == 0){
                            if (level.custom == qtrue) {
                                G_printInfoMessage(ent, "There's already a custom command being executed.");
                                return;
                            }else{
                                command = qtrue;
                                trap_GPG_FindPairValue(group, "AdminLevel", "5", txtlevel);
                                if(ent->client->sess.admin >= atoi(txtlevel)){
                                    trap_GPG_FindPairValue(group, "Action", "say \"No custom action defined\"", action);
                                    if (strstr(action, "%arg")) { // Boe!Man 7/27/12: Doesn't matter what argument, as long as there's an argument to be replaced, do it.
                                        char *action2;

                                        action2 = Boe_parseCustomCommandArgs(ent, action, qtrue);
                                        if (action2 != NULL) {
                                            memset(action, 0, sizeof(action));
                                            strncpy(action, action2, strlen(action2));
                                        }else{
                                            command = qfalse;
                                        }
                                    }
                                    if (command){
                                        trap_GPG_FindPairValue(group, "Broadcast", "Custom action applied", broadcast);
                                        trap_GPG_FindPairValue(group, "Message", "Custom action has been applied.", message);
                                        trap_SendServerCommand(-1, va("print \"^3[Custom Admin Action] ^7%s\n\"", message));
                                        G_Broadcast(broadcast, BROADCAST_CMD, NULL);
                                        memset(level.action, 0, sizeof(level.action));
                                        Boe_GlobalSound(level.actionSoundIndex);
                                        strcpy(level.action, action);
                                        level.customtime = level.time + 2000;
                                        level.custom = qtrue;
                                    }
                                }else{
                                    G_printInfoMessage(ent, "Your admin level is too low to use this command.");
                                }
                            }
                        }
                    }
                    group = trap_GPG_GetNext ( group );
                }
                trap_GP_Delete(&GP2);
            }
        }
    }

    // Don't parse tokens in commands that start with !0-9.
    if (command && ent->client->sess.admin){
        for (i = 0; i <= strlen(p); i++){
            if (p[i] == '!' && i + 1 <= strlen(p) && p[i + 1] >= '0' && p[i + 1] <= '9'){
                parseTokens = qfalse;
                break;
            }
        }
    }

    if(parseTokens){
        // Boe!Man 12/20/09
        Boe_Tokens(ent, p, mode, qtrue);
        Boe_Tokens(ent, p, mode, qfalse);
    }

    // End
    G_Say( ent, NULL, mode, p );
}

/*
==================
Cmd_Tell_f
==================
*/
static void Cmd_Tell_f( gentity_t *ent ) {
    int         i, targetNum, numberOfClients;
    gentity_t   *target;
    char        *p;
    char        arg[MAX_TOKEN_CHARS];
    char        string[MAX_TOKEN_CHARS] = "\0", string1[MAX_TOKEN_CHARS] = "\0";
    char        cleanName[MAX_NETNAME];

    if ( trap_Argc () < 2 ) {
        return;
    }

    trap_Argv( 1, arg, sizeof( arg ) );
    if(!henk_ischar(arg[0])){
        targetNum = atoi(arg);
    }else if(henk_ischar(arg[0])){
        Q_strlwr(arg);
        memset(string, 0, sizeof(string));
        memset(string1, 0, sizeof(string1));
        numberOfClients = 0;
        for(i = 0; i < level.numConnectedClients; i++){
            Q_strncpyz(cleanName, g_entities[level.sortedClients[i]].client->pers.cleanName, sizeof(cleanName));
            if(strstr(Q_strlwr(cleanName), arg)){
                targetNum = level.sortedClients[i];
                numberOfClients += 1;
                Com_sprintf(string1, sizeof(string1), "^1[#%i] ^7%s, ",  targetNum, g_entities[level.sortedClients[i]].client->pers.cleanName);
                Q_strncpyz(string+strlen(string), string1, strlen(string1)+1);
            }
        }
        string[strlen(string)-2] = '\0';
        if(numberOfClients > 1){
            G_printInfoMessage(ent, "Multiple names found with ^3%s^7: %s", arg, string);
            return;
        }else if(numberOfClients == 0){
            targetNum = -1;
        }
    }else{
        targetNum = -1;
    }

    target = &g_entities[targetNum];
    if ( targetNum < 0 || targetNum >= g_maxclients.integer || !target || !target->inuse || !target->client ){
        G_printInfoMessage(ent, "You haven't entered a valid player ID/player name.");
        return;
    }

    if (target->client->pers.connected != CON_CONNECTED){
        G_printInfoMessage(ent, "The specified player isn't fully connected yet.");
    }

    if(ent == target){
        G_printInfoMessage(ent, "You cannot send a private message to yourself.");
        return;
    }

    if (g_compMode.integer > 0 && cm_enabled.integer > 1 && g_matchDisableTell.integer && ent->client->sess.team == TEAM_SPECTATOR && !OnSameTeam(ent, target)){
        G_printInfoMessage(ent, "You cannot send a private message to players in-game.");
        return;
    }

    p = ConcatArgs( 2 );
    G_LogPrintf( "tell: %s to %s: %s\n", ent->client->pers.netname, target->client->pers.netname, p );

    Boe_Tokens(ent, p, SAY_TELL, qtrue);
    Boe_Tokens(ent, p, SAY_TELL, qfalse);

    G_Say( ent, target, SAY_TELL, p );
    // don't tell to the player self if it was already directed to this player
    // also don't send the chat back to a bot
    if ( ent != target && !(ent->r.svFlags & SVF_BOT)) {
        G_Say( target, ent, SAY_TELL_SELF, p );
    }
}


static void G_VoiceTo ( gentity_t *ent, gentity_t *other, int mode, const char* name, const char *id, qboolean voiceonly )
{
    // Only team say is supported right now for voice chatting
    if (mode != SAY_TEAM)
    {
        return;
    }

    if (!other || !other->inuse || !other->client)
    {
        return;
    }

    if ( !OnSameTeam(ent, other) )
    {
        return;
    }

    trap_SendServerCommand( other-g_entities, va("%s %d %d \"%s\" \"%s\"", "vtchat", voiceonly, ent->s.number, name, id));
}

/*
==================
G_CanVoiceGlobal

Can we globaly speak right now
==================
*/
qboolean G_CanVoiceGlobal ( void )
{
    if ( level.gametypeData->teams && level.time - level.globalVoiceTime > 5000 )
    {
        return qtrue;
    }

    return qfalse;
}

/*
==================
G_VoiceGlobal

says something out loud that everyone in the radius can hear
==================
*/
void G_VoiceGlobal ( gentity_t* ent, const char* id, qboolean force )
{
    if ( !ent )
    {
        return;
    }

    if ( !level.gametypeData->teams )
    {
        return;
    }

    if ( !force && level.time - level.globalVoiceTime < 5000 )
    {
        return;
    }

    level.globalVoiceTime = level.time;

    trap_SendServerCommand( -1, va("vglobal %d \"%s\"", ent->s.number, id));
}

/*
==================
G_Voice
==================
*/
void G_Voice( gentity_t *ent, gentity_t *target, int mode, const char *id, qboolean voiceonly )
{
    int         j;
    gentity_t   *other;
    char        name[MAX_SAY_TEXT];

    if(strlen(id) > 64){
        return;
    }

    // Spectators and ghosts dont talk
    if ( ent->client->ps.pm_type == PM_DEAD || G_IsClientSpectating ( ent->client ) )
    {
        return;
    }

    // Voice flooding protection on?
    if ( g_voiceFloodCount.integer )
    {
        // If this client has been penalized for voice chatting to much then dont allow the voice chat
        if ( ent->client->sess.voiceFloodPenalty )
        {
            if ( ent->client->sess.voiceFloodPenalty > level.time )
            {
                return;
            }

            // No longer penalized
            ent->client->sess.voiceFloodPenalty = 0;
        }

        // See if this client flooded with voice chats
        ent->client->sess.voiceFloodCount++;
        if ( ent->client->sess.voiceFloodCount >= g_voiceFloodCount.integer )
        {
            ent->client->sess.voiceFloodCount = 0;
            ent->client->sess.voiceFloodTimer = 0;
            ent->client->sess.voiceFloodPenalty = level.time + g_voiceFloodPenalty.integer * 1000;

            trap_SendServerCommand( ent-g_entities, va("print \"Voice chat flooded, you will be able use voice chats again in (%d) seconds\n\"", g_voiceFloodPenalty.integer ) );

            return;
        }
    }

    G_GetChatPrefix ( ent, target, mode, name, sizeof(name) );

    if ( target )
    {
        G_VoiceTo( ent, target, mode, name, id, voiceonly );
        return;
    }

    // send it to all the apropriate clients
    for (j = 0; j < level.maxclients; j++)
    {
        other = &g_entities[j];
        G_VoiceTo( ent, other, mode, name, id, voiceonly );
    }
}

/*
==================
Cmd_Voice_f
==================
*/
static void Cmd_Voice_f( gentity_t *ent, int mode, qboolean arg0, qboolean voiceonly )
{
    char        *p;

    if ( trap_Argc () < 2 && !arg0 ) {
        return;
    }

    if (arg0)
    {
        p = ConcatArgs( 0 );
    }
    else
    {
        p = ConcatArgs( 1 );
    }

    G_Voice( ent, NULL, mode, p, voiceonly );
}

/*
==================
Cmd_Where_f
==================
*/
void Cmd_Where_f( gentity_t *ent )
{
    trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", vtos( ent->s.origin ) ) );
}

/*
=================
Cmd_SetViewpos_f
=================
*/
void Cmd_SetViewpos_f( gentity_t *ent )
{
    vec3_t      origin, angles;
    char        buffer[MAX_TOKEN_CHARS];
    int         i;

    if ( !g_cheats.integer )
    {
        trap_SendServerCommand( ent-g_entities, va("print \"Cheats are not enabled on this server.\n\""));
        return;
    }

    if ( trap_Argc() != 5 )
    {
        trap_SendServerCommand( ent-g_entities, va("print \"usage: setviewpos x y z yaw\n\""));
        return;
    }

    VectorClear( angles );
    for ( i = 0 ; i < 3 ; i++ )
    {
        trap_Argv( i + 1, buffer, sizeof( buffer ) );
        origin[i] = atof( buffer );
    }

    trap_Argv( 4, buffer, sizeof( buffer ) );
    angles[YAW] = atof( buffer );

    TeleportPlayer( ent, origin, angles, qfalse );
}

/*
=================
ClientCommand
=================
*/
void ClientCommand( int clientNum ) {
    gentity_t *ent;
    char    cmd[MAX_TOKEN_CHARS];
    ent = g_entities + clientNum;
    if ( !ent->client ) {
        return;     // not fully in game yet
    }


    trap_Argv( 0, cmd, sizeof( cmd ) );

    //rww - redirect bot commands
    if (strstr(cmd, "bot_") && AcceptBotCommand(cmd, ent))
    {
        return;
    }
    //end rww
    if (Q_stricmp (cmd, "say") == 0) {
        // Boe!Man 1/30/10: We need to make sure the clients aren't muted.. And otherwise prevent them talking.
        if(IsClientMuted(ent, qtrue)){
            return;
        }else if( g_compMode.integer > 0 && cm_enabled.integer > 1 && g_matchDisablePubChat.integer && ent->client->sess.team == TEAM_SPECTATOR){
            Cmd_Say_f (ent, SAY_TEAM, qfalse);
        }else{
            Cmd_Say_f (ent, SAY_ALL, qfalse);
        }

        return;
    }
    if (Q_stricmp (cmd, "say_team") == 0) {
        // Boe!Man 1/30/10: We need to make sure the clients aren't muted.. And otherwise prevent them talking.
        if(IsClientMuted(ent, qtrue)){
            return;
        }
        Cmd_Say_f (ent, SAY_TEAM, qfalse);
        return;
    }

    if (Q_stricmp (cmd, "tell") == 0)
    {
        // Boe!Man 1/30/10: We need to make sure the clients aren't muted.. And otherwise prevent them talking.
        if(IsClientMuted(ent, qtrue)){
            return;
        }
        Cmd_Tell_f ( ent );
        return;
    }

    if (Q_stricmp (cmd, "vsay_team") == 0)
    {
        // Boe!Man 1/30/10: We need to make sure the clients aren't muted.. And otherwise prevent them talking.
        if(IsClientMuted(ent, qtrue)){
            return;
        }
        Cmd_Voice_f (ent, SAY_TEAM, qfalse, qfalse);
        return;
    }

    if (Q_stricmp (cmd, "score") == 0) {
        Cmd_Score_f (ent);
        return;
    }

    if (Q_stricmp (cmd, "team") == 0)
    {
        Cmd_Team_f (ent);
        return;
    }

    if (Q_stricmp (cmd, "ignore") == 0)
    {
        Henk_Ignore (ent);
        return;
    }

    // ignore all other commands when at intermission
    if (level.intermissiontime || level.changemap)
    {
//      Cmd_Say_f (ent, qfalse, qtrue);
        return;
    }

    if ( Q_stricmp ( cmd, "drop" ) == 0 )
        Cmd_Drop_f ( ent );
    else if (Q_stricmp (cmd, "dropitem") == 0)
        Cmd_DropItem_f (ent);
    else if (Q_stricmp (cmd, "give") == 0)
        Cmd_Give_f (ent);
    else if (Q_stricmp (cmd, "god") == 0)
        Cmd_God_f (ent);
    else if (Q_stricmp (cmd, "notarget") == 0)
        Cmd_Notarget_f (ent);
    else if (Q_stricmp (cmd, "noclip") == 0)
        Cmd_Noclip_f (ent);
    else if (Q_stricmp (cmd, "kill") == 0)
        Cmd_Kill_f (ent);
    else if (Q_stricmp (cmd, "levelshot") == 0)
        Cmd_LevelShot_f (ent);
    else if (Q_stricmp (cmd, "follow") == 0)
        Cmd_Follow_f (ent);
    else if (Q_stricmp (cmd, "follownext") == 0)
        Cmd_FollowCycle_f (ent, 1);
    else if (Q_stricmp (cmd, "followprev") == 0)
        Cmd_FollowCycle_f (ent, -1);
    else if (Q_stricmp (cmd, "where") == 0)
        Cmd_Where_f (ent);
    else if (Q_stricmp (cmd, "callvote") == 0)
        vote_callVote_f(ent);
    else if (Q_stricmp (cmd, "vote") == 0)
        vote_clientVote_f(ent);
    else if (Q_stricmp (cmd, "setviewpos") == 0)
        Cmd_SetViewpos_f( ent );
    // Boe!Man 3/30/10
    else if (Q_stricmp ( cmd, "motd" ) == 0 && !level.intermissionQueued){
        ent->client->sess.motdStartTime = level.time;
        ent->client->sess.motdStopTime = level.time + 7000;
        Boe_Motd( ent );}
    else if (Q_stricmp (cmd, "adm") == 0)
        Boe_adm_f( ent );
    else if (Q_stricmp (cmd, "about") == 0)
        Boe_About ( ent );
    else if (Q_stricmp (cmd, "info") == 0)
        Boe_About ( ent );
    // Boe!Man 3/31/10
    else if (Q_stricmp (cmd, "players") == 0)
        Boe_Players ( ent );
    else if (Q_stricmp (cmd, "stats") == 0)
        Boe_Stats ( ent );
    else if (strncmp(cmd, "sounds", 6) == 0) {
        int soundPage = -1;

        // Determine what sound page we requested,
        // if any.
        if(strlen(cmd) == 7){
            soundPage = cmd[6] - '0';
        }

        // We can safely list sounds if the sound page is valid.
        if(soundPage > -1 && soundPage < 10){
            // List sounds for the specified page.
            mvchat_listSounds(ent, soundPage);
        }else{
            // Not a valid command or nothing appended.
            // Give the user instructions on how to
            // use the MVCHAT system.
            mvchat_printHelp(ent);
        }
    }else if (Q_stricmp (cmd, "tokens") == 0)
        Boe_displayTokens ( ent );

    // Boe!Man 4/3/10
    #ifdef _DEV
    else if (Q_stricmp (cmd, "dev") == 0){
        Boe_dev_f(ent);
    }
    #endif // _DEV

    // Henk 07/04/10 -> Send info to all players(for RPM scoreboard)
    #ifndef _GOLD
    else if (Q_stricmp (cmd, "tmi") == 0 && level.clientMod == CL_RPM)
        RPM_UpdateTMI();
    #else
    else if (Q_stricmp(cmd, "verified") == 0 && level.clientMod == CL_ROCMOD)
        ROCmod_verifyClient(ent, clientNum);
    else if (Q_stricmp(cmd, "uef") == 0 && level.clientMod == CL_ROCMOD)
        ROCmod_clientUpdate(ent, clientNum);
    #endif // not _GOLD
    else if (Q_stricmp (cmd, "refresh") == 0)
        G_Refresh( ent );
    else if (Q_stricmp (cmd, "ready") == 0)
        G_ReadyUp( ent );
    else if (Q_stricmp (cmd, "tcmd") == 0)
        G_Tcmd( ent );
    else if (Q_stricmp (cmd, "ref") == 0)
        G_ref_cmd( ent );
    // Boe!Man 10/5/10: New how-to command for CTB.
    else if (Q_stricmp (cmd, "howto") == 0)
        Boe_Howto( ent );
    // Boe!Man 11/4/15: Verify command for 1fx client additions.
    else if (Q_stricmp(cmd, "ca_verified") == 0 && ent->client->sess.checkClientAdditions){
        ent->client->sess.checkClientAdditions = 0;

        trap_Argv(1, cmd, sizeof(cmd));
        if(cmd[0]){
            strncpy(ent->client->sess.clientAdditionsVersion, cmd, sizeof(ent->client->sess.clientAdditionsVersion));
        }
    }
#ifdef _DEBUG
    /*else if (Q_stricmp (cmd, "henk_test") == 0){
        ent->client->sess.zombie = qfalse;
        ent->client->sess.zombiebody = -1;
        ent->client->sess.firstzombie = qfalse;
        level.zombie = 0;

    }*/
    //else if (Q_stricmp (cmd, "henk_test") == 0){
    //  trap_Argv( 1, cmd, sizeof( cmd ) );
    //  DropRandom(ent, atoi(cmd));
    //}
    // F1 effect: effects/explosions/col9_boat_explosion.efx
    // or effects/levels/osprey_death_explosion.efx
    // Boe!Man 4/8/11: DEBUG COMMANDS BELOW.
    // ======================================
    /*
    else if (Q_stricmp (cmd, "henk_score") == 0){
        UpdateScores();
        ShowScores();
    }
    else if (Q_stricmp (cmd, "henk1") == 0){
        ent->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_F1_GRENADE );
        ent->client->ps.clip[ATTACK_NORMAL][WP_F1_GRENADE]=1;
        ent->client->ps.ammo[weaponData[WP_F1_GRENADE].attack[ATTACK_NORMAL].ammoIndex]=10;
    }
    else if (Q_stricmp (cmd, "henk2") == 0){
        ent->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_L2A2_GRENADE );
        ent->client->ps.clip[ATTACK_NORMAL][WP_L2A2_GRENADE]=1;
        ent->client->ps.ammo[weaponData[WP_L2A2_GRENADE].attack[ATTACK_NORMAL].ammoIndex]=10;
    }
    else if (Q_stricmp (cmd, "boeboe") == 0){
        InitCagefight();
    }
    else if (Q_stricmp (cmd, "boeboe1") == 0){
        // Build a list of spawns
        for ( test = 0; test < level.spawnCount; test ++ ){
            trap_SendServerCommand( clientNum, va("print\"[%i] origin: %.2f %.2f %.2f\n\"", test, level.spawns[test].origin[0], level.spawns[test].origin[1], level.spawns[test].origin[2]) );
        }
    }
    else if(Q_stricmp(cmd, "listskins") == 0){
        int i;
        TIdentity   *oldidentity;

        for (i = 0; i < 105; i++){
            oldidentity = &bg_identities[i];
            if(oldidentity){
                trap_SendServerCommand( clientNum, va("print\"[%i] %s\n\"", i, oldidentity->mName) );
            }
        }
    }
    else if (Q_stricmp (cmd, "boeboe2") == 0){
        // Show tied players.
        trap_SendServerCommand( clientNum, va("print\"^3[Debug] ^7%i winners that are tied and should go into cagefight.\n\"", TiedPlayers() ));
        // Start cagefight.
        InitCagefight();
    }
    else if (Q_stricmp (cmd, "boeboe3") == 0){
        // Ban a player on specific arguments.
        // 12/8/12: SQLite3 test.
        char ip[MAX_TOKEN_CHARS];
        char name[MAX_TOKEN_CHARS];
        char by[MAX_TOKEN_CHARS];
        char reason[MAX_TOKEN_CHARS];
        sqlite3     *db;
        int          rc;

        trap_Argv( 1, ip, sizeof( ip ) );
        trap_Argv( 2, name, sizeof( name ) );
        trap_Argv( 3, by, sizeof( by ) );
        trap_Argv( 4, reason, sizeof( reason ) );

        /*
        trap_SendServerCommand(-1, va("print\"^3[Debug] ^7I'm going to ban the following player IP: %s\n\"", ip));
        trap_SendServerCommand(-1, va("print\"^3[Debug] ^7Whose name is: %s\n\"", name));
        trap_SendServerCommand(-1, va("print\"^3[Debug] ^7By this fella: %s\n\"", by));
        trap_SendServerCommand(-1, va("print\"^3[Debug] ^7For the reason: %s\n\n\"", reason));
        */

        /*
        Com_sprintf (banid, sizeof(banid), "%s\\%s//%s||%s",
        ip,
        name,
        by,
        reason);

        trap_SendServerCommand(-1, va("print\"^3[Debug] ^7This is the sprintf string I just created (banid): %s\n\n\"", banid));
        if(Boe_AddToList(banid, g_banfile.string, "Ban", NULL)){
            trap_SendServerCommand(-1, va("print\"^3[Debug] ^7Succesfully written to the file: %s\n\"", g_banfile.string));
        }else{
            trap_SendServerCommand(-1, va("print\"^3[Debug] ^7Write fail to: %s\n\"", g_banfile.string));
        }
        */ /*
        if(!level.altPath){
            rc = sqlite3_open_v2("./users/bans.db", &db, SQLITE_OPEN_READWRITE, NULL);
        }else{
            rc = sqlite3_open_v2(va("%s/users/bans.db", level.altString), &db, SQLITE_OPEN_READWRITE, NULL);
        }

        sqlite3_exec(db, "PRAGMA synchronous = OFF", NULL, NULL, NULL);

        int start = trap_Milliseconds();
        if(sqlite3_exec(db, va("INSERT INTO bans (ID, IP, name, by, reason) values (?, '%s', '%s', '%s', '%s')", ip, name, by, reason), 0, 0, 0) != SQLITE_OK){
            Com_Printf("Query: %s\n", va("INSERT INTO bans (ID, IP, name, by, reason) values (?, '%s', '%s', '%s', '%s')", ip, name, by, reason));
            G_LogPrintf("^1Error: ^7bans database: %s\n", sqlite3_errmsg(db));
            return;
        }
        sqlite3_close(db);
        Com_Printf("Insert took %dms\n", trap_Milliseconds()-start);
    }else if(Q_stricmp (cmd, "boeboe4") == 0){
        // Set enterTime A LOT higher so we can fool et that we entered last.
        ent->client->pers.enterTime = level.time;
    }else if(Q_stricmp (cmd, "boeboe5") == 0){
        trap_SetConfigstring( CS_MUSIC, "music/hongkong/hk1" );
    }
    else if(Q_stricmp(cmd, "boeboe6") == 0){ // Boe!Man 2/20/13: Print clean name.
        Com_Printf("Your cleanName: %s\n", ent->client->pers.cleanName);
    }
    else if(Q_stricmp(cmd, "boeboe7") == 0){ // Boe!Man 9/18/13: Print a random tip.
        Henk_Tip();
    }
    */
#endif // _DEBUG

#ifdef _3DServer
    else if (Q_stricmp(cmd, "ghost") == 0){ // Boe!Man 3/20/15: Switch between monkey-mode and actual ghost.
        Boe_switchGhost(ent);
    }
#endif // _3DServer

#ifdef _SOF2_BOTS
    else if (Q_stricmp (cmd, "addbot") == 0)
        trap_SendServerCommand( clientNum, va("print \"ADDBOT command can only be used via RCON\n\"" ) );
#endif

    else
        trap_SendServerCommand( clientNum, va("print \"unknown cmd %s\n\"", cmd ) );
}

/*
==========
Boe_Adm_f
==========
*/

void Boe_adm_f ( gentity_t *ent )
{
    int     i, x, adm, levelx, to, oldcolour, sendsize;
    int     admPassLevel;
    char    arg1[MAX_STRING_TOKENS];
    char    arg2[MAX_STRING_TOKENS];
    gclient_t   *client;

    void    *GP2, *group;
    char txtlevel[2];
    char name[10];
    char action[512];
    char *action2;
    char message[512];
    char broadcast[512];

    // Boe!Man 7/9/12: strcat everything to a 'big buffer', and afterwards
    // send big packets to the client (to keep the number of packets as small
    // as possible).
    char bigbuf[16384] = "\0";
    char sendbuf[1024] = "\0";
    char *point;

    client = ent->client;
    trap_Argv( 1, arg1, sizeof( arg1 ) );
    trap_Argv( 2, arg2, sizeof( arg2 ) );

    adm = ent->client->sess.admin;
    if(!Q_stricmp(arg1, "login")){ // Boe!Man 4/3/11: Very small optimize.
        if(g_passwordAdmins.integer && !adm){ // only check if its enabled -- Boe!Man 6/28/11: And check if they're admin-less still.
            if (!strlen(arg2)){
                G_printInfoMessage(ent, "You need to enter your password with this command!");
                return;
            }

            levelx = Boe_checkPassAdmin(ent->client->pers.cleanName, arg2);
            if(levelx){
                ent->client->sess.admin = levelx;
                if(levelx == 2){
                    G_printInfoMessageToAll("%s has been granted B-Admin.", ent->client->pers.cleanName);
                    G_LogLogin(va("Player '%s' has been granted B-Admin. IP: %s.", ent->client->pers.cleanName, ent->client->pers.ip));
                }else if(levelx == 3){
                    G_printInfoMessageToAll("%s has been granted Admin.", ent->client->pers.cleanName);
                    G_LogLogin(va("Player '%s' has been granted Admin. IP: %s.", ent->client->pers.cleanName, ent->client->pers.ip));
                }else if(levelx == 4){
                    G_printInfoMessageToAll("%s has been granted S-Admin.", ent->client->pers.cleanName);
                    G_LogLogin(va("Player '%s' has been granted S-Admin. IP: %s.", ent->client->pers.cleanName, ent->client->pers.ip));
                }
                // Boe!Man 5/27/11: Is the Admin level allowed to spec the opposite team?
                if (g_adminSpec.integer <= ent->client->sess.admin && g_adminSpec.integer != 0 && cm_enabled.integer < 2){
                    ent->client->sess.adminspec = qtrue;
                }
            }else{
                G_printInfoMessage(ent, "Access denied: Invalid password!");
            }
        }else if(!g_passwordAdmins.integer){ // Boe!Man 3/31/11: Else display an alternate message, instead of the "You don't have Admin powers!" message (confusing).
            G_printInfoMessage(ent, "Access denied: No password logins allowed by the server!");
        }else if(adm){
            G_printInfoMessage(ent, "Access denied: You already have Admin powers!");
        }

        return;
    }
    else if (!Q_stricmp(arg1, "pass")){
        admPassLevel = Boe_checkPassAdmin2(ent->client->pers.cleanName);
        if (ent->client->sess.setAdminPassword || admPassLevel){
            if (!ent->client->sess.admin && !ent->client->sess.setAdminPassword){
                G_printInfoMessage(ent, "Login prior to changing your password!");
                return;
            }
            if (ent->client->sess.admin && ent->client->sess.admin != admPassLevel){
                G_printInfoMessage(ent, "You are not authorized to change this password.");
                return;
            }
            if (!strlen(arg2)){
                G_printInfoMessage(ent, "You need to enter your password with this command!");
                return;
            }

            Boe_addPasswordToDatabase(ent->client->pers.ip, ent->client->pers.cleanName, arg2);
            ent->client->sess.setAdminPassword = qfalse;

            G_printInfoMessage(ent, "You updated your password to: %s.", arg2);
            if (!ent->client->sess.admin){
                G_printInfoMessage(ent, "Login with /adm login %s", arg2);
            }
        }
        else{
            G_printInfoMessage(ent, "Access denied: You are not on the password list!");
        }

        return;
    }

    if(!adm){
        G_printInfoMessage(ent, "Access denied: You don't have Admin powers!");
        return;
    }
    if (!Q_stricmp ( arg1, "?" )||!Q_stricmp ( arg1, "" )||!Q_stricmp(arg1, "list"))
    {
    if (adm > 1){
        int startAt;
        admCmd_t *cmd;

        trap_SendServerCommand( ent-g_entities, va("print \" \n^3%-5s %-20s %-14s Explanation\n\"", "Lvl", "Commands", "Arguments"));
        trap_SendServerCommand( ent-g_entities, va("print \" ----------------------------------------------------------\n\""));
        // Boe!Man 6/17/11: If they want to print the whole list of admin commands, be sure to do so.
        if(!Q_stricmp(arg1, "list")){
            to = 5; // Include RCON commands.
        }else{
            to = adm;
        }

        // Determine where to start listing.
        if (to != 5 && !cm_enabled.integer)
            startAt = 10;
        else
            startAt = 7;

        // Boe!Man 9/21/10: We loop the print process and make sure they get in proper order.
        for (levelx = 2; levelx <= to; levelx++){
            // Special "pass" commands.

            // Admin remove.
            cmd = &AdminCommands[0];
            if (*cmd->adminLevel == levelx) {
                Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7%-3s %-7s %-12s %-14s ^7[^3%s^7]\n", levelx, "]", cmd->shortCmd, cmd->adminCmd, cmd->params, cmd->desc));

                if(g_passwordAdmins.integer)
                    Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7%-3s %-7s %-12s %-14s ^7[^3Remove a pwd. Admin from list^7]\n", levelx, "]", cmd->shortCmd, cmd->adminCmd, "<line #>'pass'"));
            }
            // Admin list.
            cmd = &AdminCommands[1];
            if (*cmd->adminLevel == levelx) {
                admCmd_t *cmd2 = &AdminCommands[2];

                Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7%-3s %-7s %-12s %-14s ^7[^3%s^7]\n", levelx, "]", cmd->shortCmd, cmd->adminCmd, cmd->params, cmd->desc));
                Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7%-3s %-7s %-12s %-14s ^7[^3%s^7]\n", levelx, "]", cmd2->shortCmd, cmd2->adminCmd, cmd2->params, cmd2->desc));

                if(g_passwordAdmins.integer){
                    Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7%-3s %-7s %-12s %-14s ^7[^3Show the passworded Adminlist^7]\n", levelx, "]", cmd->shortCmd, cmd->adminCmd, "'pass'"));
                    Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7%-3s %-7s %-12s %-14s ^7[^3Show the passworded Adminlist^7]\n", levelx, "]", cmd2->shortCmd, cmd2->adminCmd, "'pass'"));
                }
            }
            // Add B, reg ("A"), and S-Admin.
            for (i = 3; i < 6; i++){
                cmd = &AdminCommands[i];
                if (*cmd->adminLevel == levelx) {
                    Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7%-3s %-7s %-12s %-14s ^7[^3Add %s %s^7]\n", levelx, "]", cmd->shortCmd, cmd->adminCmd, cmd->params, (i != 4) ? "a" : "an", cmd->desc));
                    if (g_passwordAdmins.integer)
                        Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7%-3s %-7s %-12s %-14s ^7[^3Add a passworded %s^7]\n", levelx, "]", cmd->shortCmd, cmd->adminCmd, "<i/n> 'pass'", cmd->desc));
                }
            }

            // Swapteams, score- and timelimit (which are also competition commands).
            if (!cm_enabled.integer) {
                for (i = 6; i < 9; i++) {
                    cmd = &AdminCommands[i];
                    if (*cmd->adminLevel == levelx) {
                        Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7%-3s %-7s %-12s %-14s ^7[^3%s^7]\n", levelx, "]", cmd->shortCmd, cmd->adminCmd, cmd->params, cmd->desc));
                    }
                }
            }

            // All generic commands.
            for (i = startAt; i < AdminCommandsSize; i++) {
                cmd = &AdminCommands[i];
                if (*cmd->adminLevel == levelx) {
                    Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7%-3s %-7s %-12.12s %-14s ^7[^3%.32s^7]\n", levelx, "]", cmd->shortCmd, cmd->adminCmd, cmd->params, cmd->desc));
                }
            }

            // Boe!Man 6/17/11: Display competition mode Admin commands when it's enabled in the starting stage.
            if(cm_enabled.integer == 1){
                if(to != 5 && g_cm.integer == levelx || to == 5 && levelx == 5){
                    Q_strcat(bigbuf, sizeof(bigbuf), va("\n^7[^3Competition Mode Commands^7]\n"));
                    Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7%-3s %-7s %-12.12s %-14s ^7[^3%.32s^7]\n", levelx, "]", "!rounds", "rounds", "'1' or '2'", "Set number of rounds"));
                    Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7%-3s %-7s %-12.12s %-14s ^7[^3%.32s^7]\n", levelx, "]", "!sw", "swapteams", "", "Toggle auto swap"));
                    Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7%-3s %-7s %-12.12s %-14s ^7[^3%.32s^7]\n", levelx, "]", "!sl", "scorelimit", "<time>", "Change the match scorelimit"));
                    Q_strcat(bigbuf, sizeof(bigbuf), va("[^3%i^7%-3s %-7s %-12.12s %-14s ^7[^3%.32s^7]\n", levelx, "]", "!tl", "timelimit", "<time>", "Change the match timelimit"));
                }
            }
        } // Boe!Man 9/21/10: End of Loop.

        // Boe!Man 7/9/12: Now process that loop. We put everything in a big buffer, split in packets of 1000 bytes ea (max is 1024, header should still be sent along with the packet).
        if(strlen(bigbuf) <= 1000){ // Buffer is still small (could be used for limited B-Admins or so), just send the packet as a whole.
            trap_SendServerCommand( ent-g_entities, va("print \"%s\"", bigbuf));
        }else{
            point = bigbuf; // pointer to bigbuf.
            oldcolour = -1;
            levelx = Boe_firstDigitOfInt(strlen(bigbuf));

            for(i=0;i<=levelx;i++){
                if(i != levelx){
                    if(point[999] == '^'){ // Boe!Man 7/17/12: Don't allow to copy in the middle of the color..
                        strncpy(sendbuf, point, 1001);
                        point += 1001;
                    }else{
                        strncpy(sendbuf, point, 1000);
                        point += 1000;
                    }
                }else{
                    strncpy(sendbuf, point, strlen(point));
                }

                if(oldcolour == 1){
                    trap_SendServerCommand( ent-g_entities, va("print \"^3%s\"", sendbuf));
                }else{
                    trap_SendServerCommand( ent-g_entities, va("print \"%s\"", sendbuf));
                }

                x = 0;
                oldcolour = -1;
                sendsize = strlen(sendbuf);
                while (oldcolour == -1){ // Check for the last colour, if we split the packet in a "yellow" colour, that won't get restored. Hence, search for it now.
                    if(sendbuf[sendsize-x] == '3' && sendbuf[sendsize-x-1] == '^'){
                        oldcolour = 1;
                        //Com_Printf("Old colour was yellow, supposeably. i = %i\n", i);
                    }else if(sendbuf[sendsize-x] == '7' && sendbuf[sendsize-x-1] == '^'){
                        oldcolour = 0;
                    }else if(x == 50){ // avoid endless loops. 50 should be sufficient.
                        oldcolour = 0;
                    }
                    x += 1;
                }

                memset(sendbuf, 0, sizeof(sendbuf));
            }
        }

        memset(bigbuf, 0, sizeof(bigbuf)); // Boe!Man 7/9/12: Clear memory (reset buffer).

        if(g_enableCustomCommands.integer == 1){
            trap_SendServerCommand( ent-g_entities, va("print \"\n^7[^3Custom Commands^7] \n\"")); // Boe!Man 3/6/11: Spaces fix (so layout doesn't mess up).
            PrintCustom(ent-g_entities);
            // Add wrapper for CustomCommands file
        }

        // Boe!Man 6/17/11: Add a note for RCON if they display the whole list.
        if(to == 5){
            trap_SendServerCommand( ent-g_entities, va("print \"%3s^32^7] %-17s^7[^33^7] %-15s^7[^34^7] %-15s^7[^35^7] RCON\n\"", "[", "B-Admin", "Admin", "S-Admin"));
        }else{
            trap_SendServerCommand( ent-g_entities, va("print \"%5s^32^7] %-17s^7[^33^7] %-15s^7[^34^7] S-Admin\n\"", "[", "B-Admin", "Admin"));
        }
        trap_SendServerCommand( ent-g_entities, va("print \"\n^7Use ^3[Page Up]^7 and ^3[Page Down]^7 keys to scroll.\n\""));
    }
    return;
    }

    // Henk loop through my admin command array

    if (!Q_stricmp ( arg1, "chat" ) || !Q_stricmp ( arg1, "adminchat" )){
        if(IsClientMuted(ent, qtrue)){
            return;
        }
        Cmd_Say_f (ent, ADM_CHAT, qfalse);
        return;
    }
    if (!Q_stricmp ( arg1, "talk" ) || !Q_stricmp ( arg1, "admintalk" )){
        if(IsClientMuted(ent, qtrue)){
            return;
        }
        Cmd_Say_f (ent, ADM_TALK, qfalse);
        return;
    }

    if (!Q_stricmp ( arg1, "clanchat" )){
        if(IsClientMuted(ent, qtrue)){
            return;
        }else if(!ent->client->sess.clanMember){
            G_printInfoMessage(ent, "You are no clan member.");
            return;
        }
        Cmd_Say_f (ent, CLAN_CHAT, qfalse);
        return;
    }

    for(i=0;i<AdminCommandsSize;i++){ // Henk 15/09/10 -> Fixed loop going outside array(causing crashes)
        //Com_Printf("Checking %s with %s\n", arg1, AdminCommands[i].adminCmd);
        if(!Q_stricmp(arg1, AdminCommands[i].adminCmd)){
            if(ent->client->sess.admin >= *AdminCommands[i].adminLevel){
                // Execute the Admin command and handle the post processing (logging, broadcast, etc.) for some commands.
                G_postExecuteAdminCommand(i, AdminCommands[i].Function(2, ent, qfalse), ent);
                return;
            }else{
                // Boe!Man 12/30/09: Putting two Info messages together.
                G_printInfoMessage(ent, "Your admin level is too low to use this command.");
                return;
            }
        }
    }
    if(g_enableCustomCommands.integer == 1){
        qboolean execute = qtrue;

        GP2 = trap_GP_ParseFile(g_customCommandsFile.string, qtrue, qfalse);
        if(!GP2){
            Com_Printf("Error while loading %s\n", g_customCommandsFile.string);
            return;
        }
        group = trap_GPG_GetSubGroups(GP2);
        while(group){
            trap_GPG_FindPairValue(group, "Command", "none", name);
            if(!Q_stricmp(arg1, name)){
                if (level.custom == qtrue) {
                    G_printInfoMessage(ent, "There's already a custom command being executed.");
                    return;
                }

                trap_GPG_FindPairValue(group, "AdminLevel", "5", txtlevel);
                if(ent->client->sess.admin >= atoi(txtlevel)){
                    trap_GPG_FindPairValue(group, "Action", "say \"No custom action defined\"", action);
                    if(strstr(action, "%arg")){ // Boe!Man 7/27/12: Doesn't matter what argument, as long as there's an argument to be replaced, do it.
                        action2 = Boe_parseCustomCommandArgs(ent, action, qfalse);
                        if (action2 != NULL) {
                            memset(action, 0, sizeof(action));
                            strncpy(action, action2, strlen(action2));
                        }else{
                            execute = qfalse;
                        }
                    }

                    if (execute) {
                        trap_GPG_FindPairValue(group, "Broadcast", "Custom action applied", broadcast);
                        trap_GPG_FindPairValue(group, "Message", "Custom action has been applied.", message);
                        trap_SendServerCommand(-1, va("print \"^3[Custom Admin action] ^7%s.\n\"", message));
                        G_Broadcast(broadcast, BROADCAST_CMD, NULL);
                        memset(level.action, 0, sizeof(level.action));
                        strcpy(level.action, action);
                        Boe_GlobalSound(level.actionSoundIndex);
                        level.customtime = level.time + 2000;
                        level.custom = qtrue;
                    }

                    return;
                }else{
                    G_printInfoMessage(ent, "Your admin level is too low to use this command.");
                    return;
                }
            }
            group = trap_GPG_GetNext ( group );
        }
        trap_GP_Delete(&GP2);
    }

    G_printInfoMessage(ent, "This command is not available in 1fx. Mod!");
    G_printInfoMessage(ent, "Type '/adm ?' to see commands for your level or '/adm list' for all commands.");
}

/*
=================
ConsoleCommand
=================
*/
qboolean ConsoleCommand( void )
{
    char cmd[MAX_TOKEN_CHARS];
    int i;
    char arg1[64];
    char arg2[64];

    trap_Argv( 0, cmd, sizeof( cmd ) );
    trap_Argv( 1, arg1, sizeof( arg1 ) );
    trap_Argv( 2, arg2, sizeof( arg2 ) );

    // Check Admin commands for RCON/Console.
    for(i = 0; i < AdminCommandsSize; i++){
        if(!Q_stricmp(cmd, AdminCommands[i].adminCmd)){
            G_postExecuteAdminCommand(i, AdminCommands[i].Function(1, NULL, qfalse), NULL);
            return qtrue;
        }
    }

    if ( Q_stricmp (cmd, "entitylist") == 0 )
    {
        Svcmd_EntityList_f();
        return qtrue;
    }

#ifdef _SOF2_BOTS

    if (Q_stricmp (cmd, "addbot") == 0)
    {
        Svcmd_AddBot_f();
        return qtrue;
    }

    if (Q_stricmp (cmd, "botlist") == 0)
    {
        Svcmd_BotList_f();
        return qtrue;
    }

#endif

    // Boe!Man 10/31/11: Removing the original addip/removeip system since it doesn't get used. Updating it to use the current ban system.

    if (Q_stricmp (cmd, "addip") == 0)
    {
        Svcmd_AddIP_f();
        return qtrue;
    }

    // Boe!Man 11/04/11: This is obsolete and doesn't get used (they can use unban/subnetunban instead). Removed.
    /*
    if (Q_stricmp (cmd, "removeip") == 0)
    {
        Svcmd_RemoveIP_f();
        return qtrue;
    }
    */

    if (Q_stricmp (cmd, "listip") == 0)
    {
        // Boe!Man 10/31/11: This gets replaced with the 1fx. Mod-style banlist.
        trap_SendConsoleCommand( EXEC_NOW, "banlist\n" );
        return qtrue;
    }

    // End Boe!Man 10/31/11

    if (Q_stricmp (cmd, "gametype_restart" ) == 0 )
    {
        // Cannot restart the gametype in DM (at least, no visible effect).
        if(current_gametype.value == GT_DM){
            Com_Printf("You cannot restart this gametype.\n");
            return qtrue;
        }

        // Cannot restart the gametype in intermission mode.
        if(level.intermissionQueued || level.intermissiontime){
            Com_Printf("You cannot restart the gametype while being in intermission.\n");
            return qtrue;
        }

        // Cannot restart the gametype while being paused.
        if(level.pause){
            Com_Printf("You cannot restart the gametype while the game is paused.\n");
            return qtrue;
        }

        trap_Argv( 1, cmd, sizeof( cmd ) );
        G_ResetGametype ( (qboolean)(Q_stricmp ( cmd, "full" ) == 0), qfalse );
        return qtrue;
    }

    #ifndef _DEMO
    if (Q_stricmp (cmd, "extendtime" ) == 0 )
    {
        Svcmd_ExtendTime_f();
        return qtrue;
    }
    #endif // not _DEMO

    if (Q_stricmp (cmd, "sql_stats" ) == 0 )
    {
        Boe_SQLStats();
        return qtrue;
    }

    if (Q_stricmp (cmd, "clear" ) == 0 )
    {
        Boe_SQLTableClear();
        return qtrue;
    }

    if (Q_stricmp (cmd, "tell") == 0 )
    {
        BB_Tell_f(0);
        return qtrue;
    }

    if (Q_stricmp(cmd, "1fxver") == 0 )
    {
        Com_Printf("-------------------------------------\n");
        Com_Printf("This server is running 1fx. Mod!\n");
        Com_Printf("Version: " MODVERSION_NOSUFFIX "\n");
        Com_Printf("Date: " __DATE__ "\n");
        #ifdef MODFLAVORS
        Com_Printf("Flavors: " MODFLAVORS "\n");
        #endif // MODFLAVORS
        Com_Printf("-------------------------------------\n");

        return qtrue;
    }

    if(Q_stricmp(cmd, "hash") == 0 )
    {
        G_Hash();
        return qtrue;
    }

    if(Q_stricmp(cmd, "mapcycle_s") == 0 )
    {
        G_switchToNextMapInCycle(qtrue);
        return qtrue;
    }

    // Add synonym for the quit command.
    if(Q_stricmp(cmd, "exit") == 0 )
    {
        trap_SendConsoleCommand( EXEC_APPEND, "quit\n");
        return qtrue;
    }

    if (g_dedicated.integer)
    {
        if (Q_stricmp (cmd, "say") == 0)
        {
            trap_SendServerCommand( -1, va("chat -1 \"%s: %s\n\"", server_rconprefix.string, ConcatArgs(1) ) );
            return qtrue;
        }

        // everything else will also be printed as a say command
        trap_SendServerCommand( -1, va("chat -1 \"%s: %s\n\"", server_rconprefix.string, ConcatArgs(0) ) );
        return qtrue;
    }

    return qfalse;
}

/*
==================
G_kickPlayer

Kicks player from the server. Includes who did it and the reason.
==================
*/

void G_kickPlayer(gentity_t *to, gentity_t *by, char *action, char *reason)
{
    // Kick the player.
    trap_DropClient(to - g_entities,
        va("You were %s from the server.\nBy: %s\nReason: %s",
        action, by ? by->client->pers.cleanName : "RCON",
        strlen(reason) ? reason : "None given."));
}

/*
==================
G_Broadcast

Broadcasts a message to clients that are supposed to receive it.
==================
*/

void G_Broadcast(char *broadcast, int broadcastLevel, gentity_t *to)
{
    int i;
    char *newBroadcast;

    newBroadcast = G_ColorizeMessage(broadcast);

    // If to is NULL, we're dealing with a global message (equals old way of broadcasting to -1).
    if (to == NULL){
        if (!level.pause){
            for (i = 0; i < level.numConnectedClients; i++){
                gentity_t* other = &g_entities[level.sortedClients[i]];

                // Skip any client that isn't connected.
                if (other->client->pers.connected != CON_CONNECTED){
                    continue;
                }
                // Skip any client that received a more important message in the last 5 seconds.
                if (other->client->sess.lastMessagePriority > broadcastLevel && level.time < (other->client->sess.lastMessage + 4000)){
                    continue;
                }

                trap_SendServerCommand(other-g_entities, va("cp \"@%s\n\"", newBroadcast));
                other->client->sess.lastMessagePriority = broadcastLevel;
                other->client->sess.lastMessage = level.time;
            }
        }else{
            // Boe!Man 3/13/15: In order to print messages during pause we need to call a different function.
            trap_SetConfigstring(CS_GAMETYPE_MESSAGE, va("%i,@%s", level.time + 5000, newBroadcast));
        }
    }else if (broadcastLevel >= to->client->sess.lastMessagePriority || level.time > (to->client->sess.lastMessage + 4000)){
        trap_SendServerCommand(to-g_entities, va("cp \"@%s\n\"", newBroadcast));
        to->client->sess.lastMessagePriority = broadcastLevel;
        to->client->sess.lastMessage = level.time;
    }
}

/*
==================
G_ColorizeMessage

Adds server colors into a broadcast message (if required).
==================
*/
#define MAX_BROADCAST_LINE 48

char *G_ColorizeMessage(char *broadcast)
{
    int i, newWordLength;
    char *tempBroadcast;
    static char newBroadcast[MAX_STRING_CHARS];
    char line[128], remainingLine[128];
    int lineLength;
    int newWordPosition = 0;
    qboolean parseLine = qtrue;
    char *tempNewline = broadcast;

    // Reset broadcast buffer.
    memset(newBroadcast, 0, sizeof(newBroadcast));

    // Loop through whole broadcast.
    while (parseLine){
        // Properly reset line to avoid having to NULL terminate it everywhere.
        memset(line, 0, sizeof(line));

        // Check for more lines.
        tempNewline = strstr(broadcast, "\n");
        if (tempNewline == NULL){
            lineLength = strlen(broadcast);

            // Last line, don't go further after this.
            parseLine = qfalse;
        }else{
            lineLength = tempNewline - broadcast + 1;
        }

        // Check if line is too big to copy.
        if (lineLength >= sizeof(line)){
            lineLength = sizeof(line) - 1;
        }

        // Copy line.
        strncpy(line, broadcast, lineLength);

        // First we check the line. Should colours be applied to it?
        // A backslash can never be applied to any command in-game, like broadcast. Use that to determine what word should be highlighted.
        tempBroadcast = strstr(line, "\\");
        if (tempBroadcast != NULL){
            // OK, a word is in the line. Find position in actual broadcast.
            tempBroadcast = strstr(broadcast, "\\");

            // A word is found that should be highlighted.
            // First determine the start position of the word in the broadcast.
            newWordPosition = (int)(tempBroadcast - broadcast);

            // Now determine the length of the string.
            strncpy(remainingLine, tempBroadcast, strlen(line) - newWordPosition);
            remainingLine[strlen(line) - newWordPosition] = '\0';
            tempBroadcast = strstr(remainingLine, " ");
            if (tempBroadcast == NULL){
                // Word is as long as the remaining string.
                newWordLength = strlen(remainingLine) - 1;
            }else{
                // Make sure we properly get the address.
                char *c = broadcast;
                c += newWordPosition + 1;

                // Word ends somewhere.
                newWordLength = strstr(c, " ") - (broadcast + newWordPosition) - 1;
            }

            // Now we apply the colors to the broadcast if there are colors to apply it to.
            // Do check the size of the line, if it's getting too big simply cut some colors to make room for the line to fit.
            if (newWordLength > 0 && newWordLength < 64){
                // Check how many colors there are available to use.
                int availableColors = strlen(server_colors.string);
                strncat(newBroadcast, broadcast, newWordPosition);

                if (!availableColors){
                    // Copy the string to it, excluding backslash.
                    strncat(newBroadcast, broadcast + newWordPosition + 1, strlen(broadcast) - newWordPosition);
                }else{
                    // Apply colors.
                    char wordToCopy[64];
                    int color, newLineLen;

                    // Check how many colors we can copy.
                    if (newWordLength >= availableColors){
                        color = 0;
                    }else{
                        color = availableColors - newWordLength;
                    }

                    // Check if everything fits properly.
                    newLineLen = lineLength - 1 + ((availableColors - color) * 2); // Exclude the \ character and add the colors.

                    while (newLineLen > MAX_BROADCAST_LINE){
                        color++;
                        if (color >= availableColors)
                            break; // Line will never fit.

                        if ((lineLength - 1 + ((availableColors - color) * 2)) <= MAX_BROADCAST_LINE){
                            newLineLen = lineLength - 1 + ((availableColors - color) * 2);
                            break;
                        }
                    }

                    // Copy each individual char to the new buffer, plus the color associated to it.
                    strncpy(wordToCopy, broadcast + newWordPosition + 1, newWordLength);
                    for (i = 0; i < newWordLength; i++){
                        if (color < availableColors){
                            strcat(newBroadcast, va("^%c%c", server_colors.string[color], wordToCopy[i]));
                            color++;
                        }else{
                            strcat(newBroadcast, va("%c", wordToCopy[i]));
                        }
                    }

                    // Copy a trailing ^7 so all text won't be messed up if the colors aren't set to fade out correctly.
                    strcat(newBroadcast, "^7");

                    // Is there a remaining string in the line?
                    if (tempBroadcast){
                        strncat(newBroadcast, broadcast + newWordPosition + newWordLength + 1, strlen(line) - newWordLength - newWordPosition - 1);
                    }
                }
            }
        }else{
            strncat(newBroadcast, line, strlen(line));
        }

        // Advance broadcast.
        broadcast += (int)(tempNewline - broadcast) + 1;
    }

    // Boe!Man 3/13/15: Replace newlines with spaces when the game is paused.
    if (level.pause){
        for (i = 0; i < strlen(newBroadcast); i++){
            if (newBroadcast[i] == '\n'){
                newBroadcast[i] = ' ';
            }
        }
    }

    return newBroadcast;
}

void G_postExecuteAdminCommand(int funcNum, int idNum, gentity_t *adm)
{
    if (idNum < 0){
        return;
    }

    // Broadcast the change and log it.
    if (adm != NULL){
        // Admin action.
        G_Broadcast(va("%s\nwas \\%s%s\nby %s", g_entities[idNum].client->pers.netname, AdminCommands[funcNum].adminCmd, (AdminCommands[funcNum].suffix != NULL) ? AdminCommands[funcNum].suffix : "", adm->client->pers.netname), BROADCAST_CMD, NULL);
        trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was %s%s by %s.\n\"", g_entities[idNum].client->pers.cleanName, AdminCommands[funcNum].adminCmd, (AdminCommands[funcNum].suffix != NULL) ? AdminCommands[funcNum].suffix : "", adm->client->pers.cleanName));
        Boe_adminLog(AdminCommands[funcNum].adminCmd, va("%s\\%s", adm->client->pers.ip, adm->client->pers.cleanName), va("%s\\%s", g_entities[idNum].client->pers.ip, g_entities[idNum].client->pers.cleanName));
    }else{
        // RCON action.
        if(!g_entities[idNum].client->sess.camper){
            // Regular Admin command, not invoked because of camping.
            G_Broadcast(va("%s\nwas \\%s%s", g_entities[idNum].client->pers.netname, AdminCommands[funcNum].adminCmd, (AdminCommands[funcNum].suffix != NULL) ? AdminCommands[funcNum].suffix : ""), BROADCAST_CMD, NULL);
            trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s was %s%s.\n\"", g_entities[idNum].client->pers.cleanName, AdminCommands[funcNum].adminCmd, (AdminCommands[funcNum].suffix != NULL) ? AdminCommands[funcNum].suffix : ""));
        }else{
            // Punished for camping.
            G_Broadcast(va("%s\nwas \\punished for camping!", g_entities[idNum].client->pers.netname), BROADCAST_CMD, NULL);
            G_printInfoMessageToAll("%s was punished for camping (%s%s).", g_entities[idNum].client->pers.cleanName, AdminCommands[funcNum].adminCmd, (AdminCommands[funcNum].suffix != NULL) ? AdminCommands[funcNum].suffix : "");
            g_entities[idNum].client->sess.camper = qfalse;
        }
        Boe_adminLog(AdminCommands[funcNum].adminCmd, va("%s", "RCON"), va("%s\\%s", g_entities[idNum].client->pers.ip, g_entities[idNum].client->pers.cleanName));
    }

    // Let users know something happened with a sound as well.
    Boe_GlobalSound(level.actionSoundIndex);
}

/*
==============
G_printInfoMessage
10/30/16 - 6:06 PM

Prints [Info] message to the console of a player
or regular message to server console (or RCON).
==============
*/

void QDECL G_printInfoMessage(gentity_t *ent, const char *msg, ...)
{
    va_list     argptr;
    int         len;
    char        text[1024];

    memset(text, 0, sizeof(text));

    // If we're printing this message to a client,
    // ensure we prepend the [Info] part.
    if(ent && ent->client){
        // Prepend the [Info].
        strncat(text, "print \"^3[Info] ^7", sizeof(text));

        // Concatenate the message.
        va_start(argptr, msg);
        len = vsnprintf(text + 18, sizeof(text) - 20, msg, argptr) + 18;
        va_end(argptr);

        // Append the newline.
        strncat(text + len, "\n\"", sizeof(text) - len);
    }else{
        // Concatenate the message.
        va_start(argptr, msg);
        len = vsnprintf(text, sizeof(text) - 1, msg, argptr);
        va_end(argptr);

        // Append the newline.
        strncat(text + len, "\n", sizeof(text) - len);
    }

    // Print the info message.
    if(ent && ent->client){
        trap_SendServerCommand(ent-g_entities, text);
    }else{
        trap_Printf(text);
    }
}

/*
==============
G_printInfoMessageToAll
10/30/16 - 6:06 PM

Prints [Info] message to the console
of all players.
==============
*/

void QDECL G_printInfoMessageToAll(const char *msg, ...)
{
    va_list     argptr;
    int         len;
    char        text[1024];

    memset(text, 0, sizeof(text));

    // If we're printing this message to a client,
    // ensure we prepend the [Info] part.
    strncat(text, "print \"^3[Info] ^7", sizeof(text));

    // Concatenate the message.
    va_start(argptr, msg);
    len = vsnprintf(text + 18, sizeof(text) - 20, msg, argptr) + 18;
    va_end(argptr);

    // Append the newline.
    strncat(text + len, "\n\"", sizeof(text) - len);

    // Print the info message.
    trap_SendServerCommand(-1, text);
}

/*
=================
BB_Tell_f (B3 Private Messaging)
=================
*/

void BB_Tell_f(gentity_t *ent)
{
    int             id;
    gentity_t       *id_ent;

    id = G_clientNumFromArg(ent, 1, "do this to", qfalse, qtrue, qtrue, qfalse);
    if (id == -1){
        return;
    }

    id_ent = g_entities + id;
    trap_SendServerCommand( id_ent-g_entities, va("chat -1 \"%s\n\"", ConcatArgs(2)));
}

#ifdef _3DServer
void Boe_switchGhost(gentity_t *ent)
{
    // System only works in H&S.
    if (current_gametype.value != GT_HS){
        G_printInfoMessage(ent, "This command only works in Hide&Seek!");
        return;
    }

    // System must be enabled.
    if (!boe_deadMonkey.integer){
        G_printInfoMessage(ent, "This feature is disabled on the server!");
        return;
    }

    if (!level.monkeySpawnCount){
        G_printInfoMessage(ent, "No monkey spawns available on the server, so this feature is disabled!");
        return;
    }

    // Check if the client wishes to become a ghost or prefers to "play" instead.
    ent->client->sess.monkeyPreferGhost = !ent->client->sess.monkeyPreferGhost;

    if (ent->client->sess.monkeyPreferGhost){
        // Is the client dead already and in monkey mode? If so, respawn.
        if (ent->client->sess.deadMonkey && ent->client->ps.pm_type == PM_NORMAL){
            // Kill player, he won't be respawned next round.
            ent->flags &= ~FL_GODMODE;
            ent->client->ps.stats[STAT_HEALTH] = ent->health = -999;
            ent->client->sess.deadMonkey = 0;
            ent->client->sess.deadMonkeyDie = qtrue;
            player_die(ent, ent, ent, 100000, MOD_SUICIDE, HL_NONE, vec3_origin);
        }

        G_printInfoMessage(ent, "You won't be respawned as dead player anymore.");
    }else{
        // Is the client dead and in ghost mode? If so, respawn.
        if ((ent->client->sess.ghost || ent->client->ps.stats[STAT_HEALTH] < 1) && !level.cagefight && ent->client->sess.team == TEAM_RED){
            ent->client->sess.deadMonkey = level.time;
            ent->client->sess.ghost = qfalse;
            ClientSpawn(ent);
        }

        G_printInfoMessage(ent, "You will be respawned as a dead player from now on.");
    }

    G_printInfoMessage(ent, "Issue the ^3/ghost ^7command again to toggle the behavior.");
}
#endif // _3DServer
