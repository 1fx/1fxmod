// Copyright (C) 2009-2017 - Boe!Man & Henkie.
//
// g_vote.c - All the vote command functions go here.

#include "g_local.h"
#include "ai_main.h"
#include "boe_local.h"
#include "g_vote.h"

/*
==============
vote_mapRestart
5/28/17 - 11:54 AM
Start vote to restart the map.
==============
*/

static qboolean vote_mapRestart(gentity_t *ent)
{
    // Nothing can go wrong here, just append the command to the vote string.
    Com_sprintf(level.voteString, sizeof(level.voteString), "maprestart");
    Com_sprintf(level.voteDisplayString, sizeof(level.voteDisplayString),
        "restart map");

    return qtrue;
}

/*
==============
vote_mapCycle
5/28/17 - 11:58 AM
Start vote to go to the next
map in the cycle.
==============
*/

static qboolean vote_mapCycle(gentity_t *ent)
{
    if(!*g_mapcycle.string || !Q_stricmp(g_mapcycle.string, "none")){
        G_printInfoMessage(ent, "There is no mapcycle currently set up.");
        return qfalse;
    }

    Com_sprintf(level.voteString, sizeof(level.voteString), "mapcycle");
    Com_sprintf(level.voteDisplayString, sizeof(level.voteDisplayString),
        "nextmap");

    return qtrue;
}

/*
==============
vote_map
5/28/17 - 12:17 PM
Start vote to go to the
specified map.
==============
*/

static qboolean vote_map(gentity_t *ent)
{
    fileHandle_t    mapFile;
    char            arg2[MAX_STRING_TOKENS];

    // Get the second argument.
    trap_Argv(2, arg2, sizeof(arg2));
    Q_strlwr(arg2);

    // Map must be valid in order to switch to it.
    trap_FS_FOpenFile(va("maps/%s.bsp"), &mapFile, FS_READ);
    if(mapFile == 0){
        G_printInfoMessage(ent, "Map not found.");
        return qfalse;
    }

    trap_FS_FCloseFile(mapFile);

    // Valid map.
    Com_sprintf(level.voteString, sizeof(level.voteString),
        "map %s", arg2);
    Com_sprintf(level.voteDisplayString, sizeof(level.voteDisplayString),
        "%s", level.voteString);

    return qtrue;
}

/*
==============
vote_rmgMap
5/28/17 - 12:21 PM
Start vote to generate
the specified RMG map.
==============
*/

static qboolean vote_rmgMap(gentity_t *ent)
{
    char    arg2[MAX_STRING_TOKENS];
    char    arg3[MAX_STRING_TOKENS];
    char    arg4[MAX_STRING_TOKENS];

    // Get the required arguments.
    trap_Argv(2, arg2, sizeof(arg2));
    trap_Argv(3, arg3, sizeof(arg3));
    trap_Argv(4, arg4, sizeof(arg4));
    Q_strlwr(arg2);
    Q_strlwr(arg3);
    Q_strlwr(arg4);

    Com_sprintf(level.voteString, sizeof(level.voteString),
        "rmgmap 1 %s 2 %s 3 %s 4 \"%s\" 0", arg2, arg3, arg4,
        ConcatArgs(5));
    Com_sprintf(level.voteDisplayString, sizeof(level.voteDisplayString),
        "%s", level.voteString);

    return qtrue;
}

/*
==============
vote_gameType
5/28/17 - 12:23 PM
Start vote to switch the server
to the specified gametype.

Gametype must be valid on the
current map.
==============
*/

static qboolean vote_gameType(gentity_t *ent)
{
    char    arg2[MAX_STRING_TOKENS];

    // Get the required arguments.
    trap_Argv(2, arg2, sizeof(arg2));

    // Verify the gametype.
    if(!Henk_DoesMapSupportGametype(arg2, level.mapname)){
        G_printInfoMessage(ent,
            "This map does not support the gametype '%s'.", arg2);

        return qfalse;
    }

    Com_sprintf(level.voteString, sizeof(level.voteString),
        "gametype %s", arg2);
    Com_sprintf(level.voteDisplayString, sizeof(level.voteDisplayString),
        level.voteString);

    return qtrue;
}

/*
==============
vote_kick
5/28/17 - 12:26 PM
Start vote to kick a player from
the server, based on either ID
or name.
==============
*/

static qboolean vote_kick(gentity_t *ent)
{
    int clientid;

    clientid = G_clientNumFromArg(ent, 2, "kick", qfalse, qtrue, qtrue, qfalse);

    if(clientid == -1){
        return qfalse;
    }

    Com_sprintf(level.voteString, sizeof(level.voteString),
        "clientkick %d", clientid);
    Com_sprintf(level.voteDisplayString, sizeof(level.voteDisplayString),
        "kick #%d: %s", clientid,
        g_entities[clientid].client->pers.cleanName);

    return qtrue;
}

/*
==============
vote_clientKick
5/28/17 - 12:28 PM
Start vote to kick a player from
the server, based on ID.
==============
*/

static qboolean vote_clientKick(gentity_t *ent)
{
    int     clientid;
    char    arg2[MAX_STRING_TOKENS];

    // Get the required arguments.
    trap_Argv(2, arg2, sizeof(arg2));

    clientid = atoi(arg2);

    // Verify client number.
    if(clientid < 0 || clientid >= MAX_CLIENTS){
        G_printInfoMessage(ent, "Invalid client number: %d.", clientid);
        return qfalse;
    }

    if(!g_entities[clientid].client
        || g_entities[clientid].client->pers.connected == CON_DISCONNECTED)
    {
        G_printInfoMessage(ent, "There is no client with the client " \
            "number: %d.", clientid);

        return qfalse;
    }

    // Verified, we can start the vote.
    Com_sprintf(level.voteString, sizeof(level.voteString), "clientkick %s",
        arg2);
    Com_sprintf(level.voteDisplayString, sizeof(level.voteDisplayString),
        "kick #%d: %s", clientid, g_entities[clientid].client->pers.cleanName);

    return qtrue;
}

/*
==============
vote_doWarmup
5/28/17 - 12:30 PM
Start vote to enable or
disable the warmup.
==============
*/

static qboolean vote_doWarmup(gentity_t *ent)
{
    char        arg2[MAX_STRING_TOKENS];

    // Get the required arguments.
    trap_Argv(2, arg2, sizeof(arg2));

    if(!strlen(arg2) || strlen(arg2) > 1 || (arg2[0] != '1' && arg2[0] != '0')){
        G_printInfoMessage(ent, "You need to specify either 1 or 0.");
        return qfalse;
    }

    // Start the vote.
    Com_sprintf(level.voteString, sizeof(level.voteString), "g_doWarmup %s",
        arg2);
    Com_sprintf(level.voteDisplayString, sizeof(level.voteDisplayString),
        "%s warmup", (arg2[0] == '1') ? "enable" : "disable");

    return qtrue;
}

/*
==============
vote_toggleIntegerCVAR
5/28/17 - 12:30 PM
Toggles a generic CVAR
where any integer is possible (except empty).
==============
*/

static qboolean vote_toggleIntegerCVAR(gentity_t *ent)
{
    char        arg1[MAX_STRING_TOKENS];
    char        arg2[MAX_STRING_TOKENS];
    int         cvarValue;

    // Get the required arguments.
    trap_Argv(1, arg1, sizeof(arg1));
    trap_Argv(2, arg2, sizeof(arg2));
    cvarValue = atoi(arg2);

    if(!strlen(arg2)){
        G_printInfoMessage(ent, "You need to specify a value.");
        return qfalse;
    }

    // Start the vote.
    Com_sprintf(level.voteString, sizeof(level.voteString), "%s %d",
        arg1, cvarValue);
    Com_sprintf(level.voteDisplayString, sizeof(level.voteDisplayString),
        level.voteString);

    return qtrue;
}

/*
==============
vote_timelimitExt
5/28/17 - 12:48 PM
Start vote to extend
the timelimit.
==============
*/

static qboolean vote_timelimitExt(gentity_t *ent)
{
    if(!g_timelimit.integer){
        G_printInfoMessage(ent, "There is no timelimit to extend.");
        return qfalse;
    }

    if(!g_timeextension.integer){
        G_printInfoMessage(ent,
            "This server does not allow time extensions.");
        return qfalse;
    }

    Com_sprintf(level.voteString, sizeof(level.voteString),
        "extendtime %d", g_timeextension.integer);
    Com_sprintf(level.voteDisplayString, sizeof(level.voteDisplayString),
        "extend timelimit by %d minutes", g_timeextension.integer);

    return qtrue;
}

/*
==============
vote_endMap
5/28/17 - 12:50 PM
Start vote to end
the current map.
==============
*/

static qboolean vote_endMap(gentity_t *ent)
{
    Com_sprintf(level.voteString, sizeof(level.voteString), "endmap");
    Com_sprintf(level.voteDisplayString, sizeof(level.voteDisplayString),
        "end the map");

    return qtrue;
}

/*
==============
vote_shuffleTeams
5/28/17 - 12:52 PM
Start vote to shuffle
the teams.
==============
*/

static qboolean vote_shuffleTeams(gentity_t *ent)
{
    // Do not allow shuffleteams during Zombies.
    if(current_gametype.value == GT_HZ){
        G_printInfoMessage(ent, "You cannot shuffle the teams " \
            "in this gametype.");

        return qfalse;
    }

    // Check gametype layout compatibility.
    if(!level.gametypeData->teams){
        G_printInfoMessage(ent, "Not playing a team game.");

        return qfalse;
    }

    // Ensure teams are not locked.
    if(level.blueLocked || level.redLocked){
        G_printInfoMessage(ent, "Teams are locked.");

        return qfalse;
    }

    Com_sprintf(level.voteString, sizeof(level.voteString), "shuffleteams");
    Com_sprintf(level.voteDisplayString, sizeof(level.voteDisplayString),
        level.voteString);

    return qtrue;
}

/*
==============
vote_swapTeams
5/28/17 - 12:54 PM
Start vote to swap
the teams.
==============
*/

static qboolean vote_swapTeams(gentity_t *ent)
{
    // Check gametype first.
    if(!level.gametypeData->teams){
        G_printInfoMessage(ent, "Not playing a team game.");
        return qfalse;
    }

    Com_sprintf(level.voteString, sizeof(level.voteString), "swapteams");
    Com_sprintf(level.voteDisplayString, sizeof(level.voteDisplayString),
        level.voteString);

    return qtrue;
}

/*
==============
vote_mute
5/28/17 - 12:56 PM
Start vote to mute or
unmute a client.
==============
*/

static qboolean vote_mute(gentity_t *ent)
{
    char    arg3[MAX_STRING_TOKENS];
    int     clientid;
    int     muteTime;

    // Check if the client ID specified is valid.
    clientid = G_clientNumFromArg(ent, 2, "mute", qfalse, qfalse,
        qfalse, qfalse);
    if(clientid == -1){
        return qfalse;
    }

    // Check if the voter specified a mute time.
    trap_Argv(3, arg3, sizeof(arg3));
    muteTime = atoi(arg3);

    if(!muteTime){
        // If no mutetime is specified, set it to the default 5 minutes.
        muteTime = 5;
    }else if(muteTime > 60){
        // Always check for the 60 minute mute max.
        G_printInfoMessage(ent, "The maximum time for mute is 60 minutes.");
        muteTime = 60;
    }

    Com_sprintf(level.voteString, sizeof(level.voteString),
        "mute %d %d", clientid, muteTime);
    Com_sprintf(level.voteDisplayString, sizeof(level.voteDisplayString),
        "mute #%d: %s %d min", clientid,
        g_entities[clientid].client->pers.cleanName, muteTime);

    return qtrue;
}

/*
==============
vote_clanVsAll
5/28/17 - 1:00 PM
Start vote to enable
clan vs all.
==============
*/

static qboolean vote_clanVsAll(gentity_t *ent)
{
    char    arg2[MAX_STRING_TOKENS];
    char    team;

    // Check gametype first.
    if(!level.gametypeData->teams){
        G_printInfoMessage(ent, "Not playing a team game.");
        return qfalse;
    }

    trap_Argv(2, arg2, sizeof(arg2));
    Q_strlwr(arg2);

    // If no team is specified, default to the red team.
    if(!strlen(arg2)){
        G_printInfoMessage(ent, "No team specified, defaulting to " \
            "the red team.");
        team = 'r';
    }else{
        if(arg2[0] == 'r'){
            team = 'r';
        }else if(arg2[0] == 'b'){
            team = 'b';
        }else{
            G_printInfoMessage(ent, "Invalid team specified, defaulting to " \
            "the red team.");
            team = 'r';
        }
    }

    Com_sprintf(level.voteString, sizeof(level.voteString),
        "clanvsall %c", team);
    Com_sprintf(level.voteDisplayString, sizeof(level.voteDisplayString),
        "clanvsall (clan in %s team)", (team == 'r') ? "red" : "blue");

    return qtrue;
}

/*
==============
vote_poll
5/28/17 - 1:06 PM
Start a poll vote.
==============
*/

static qboolean vote_poll(gentity_t *ent)
{
    Com_sprintf(level.voteDisplayString, sizeof(level.voteDisplayString),
        "Poll: %s", ConcatArgs(2));
    Com_sprintf(level.voteString, sizeof(level.voteString), "poll");

    return qtrue;
}

/*
============
vote_isDisabled

Determins if the given
vote is disabled.
============
*/
static int vote_isDisabled(const char* callvote)
{
    return trap_Cvar_VariableIntegerValue(va("novote_%s", callvote));
}

/*
==================
vote_callVote_f
5/28/17 - 1:25 PM

This function is the entry point
that handles the call vote completely.
==================
*/

void vote_callVote_f(gentity_t *ent)
{
    int     i, voteCmd;
    char    arg1[MAX_STRING_TOKENS];
    char    arg2[MAX_STRING_TOKENS];

    if(!g_allowVote.integer){
        G_printInfoMessage(ent, "Voting not allowed here.");
        return;
    }

    if(level.intermissiontime
        || level.intermissionQueued
        || level.changemap)
    {
        G_printInfoMessage(ent, "Voting not allowed during intermission.");
        return;
    }

    if(level.numConnectedClients >= 1 && level.numVotingClients == 1){
        G_printInfoMessage(ent, "You need at least 2 clients to call a vote.");
        return;
    }

    if(level.voteTime){
        G_printInfoMessage(ent, "A vote is already in progress.");
        return;
    }

    if(ent->client->pers.voteCount >= MAX_VOTE_COUNT){
        G_printInfoMessage(ent, "You have called the maximum number of votes.");
        return;
    }

    if(ent->client->sess.team == TEAM_SPECTATOR){
        G_printInfoMessage(ent, "Not allowed to call a vote as spectator.");
        return;
    }

    if (ent->client->voteDelayTime > level.time){
        G_printInfoMessage(ent, "You are not allowed to vote within %d " \
            "minute of a failed vote.", g_failedVoteDelay.integer);

        return;
    }

    if(g_callvote.integer > 1){
        if(ent->client->sess.admin < g_callvote.integer){
            G_printInfoMessage(ent, "Your admin level is too low to be able " \
                "to call a vote.");

            return;
        }
    }

    // Save the voting client ID.
    level.voteClient = ent->s.number;

    // Make sure it is a valid command to vote on.
    trap_Argv(1, arg1, sizeof(arg1));
    trap_Argv(2, arg2, sizeof(arg2));

    // Ensure the voting command is lowercase.
    Q_strlwr(arg1);
    Q_strlwr(arg2);

    // Check vote string, don't allow malformed characters.
    if(strchr(arg1, ';') || strchr(arg2, ';')){
        G_printInfoMessage(ent, "Invalid vote string.");
        return;
    }

    // Iterate through the vote commands. See what vote command the client
    // wants to execute.
    voteCmd = -1;
    for(i = 0; i < voteCmdSize; i++){
        if(!strcmp(arg1, VoteCommands[i].voteCmd)){
            voteCmd = i;
            break;
        }
    }

    // Check if the vote command is invalid.
    // If so, display all available vote commands or display what went wrong.
    if(voteCmd == -1){
        if(strlen(arg1) && strcmp(arg1, "?")){
            // The user specified a vote string but it is invalid.
            G_printInfoMessage(ent, "Invalid vote string.");
            G_printInfoMessage(ent, "Type '/callvote ?' for a list with all " \
                "available votes.");

            return;
        }

        // Print available vote commands.
        // Header.
        trap_SendServerCommand(ent-g_entities, va(
            "print \"\n^3%-14s %-18s Explanation\n\"", "Vote command",
            "Arguments"));
        trap_SendServerCommand(ent-g_entities,
            "print \"------------------------" \
            "----------------------------------\n\"");

        // Iterate through vote commands and print info.
        for(i = 0; i < voteCmdSize; i++){
            trap_SendServerCommand(ent-g_entities,
                va("print \"^7%-14s %-18s [^3%s^7]\n",
                VoteCommands[i].voteCmd, VoteCommands[i].params,
                VoteCommands[i].desc));
        }

        // Print footer.
        trap_SendServerCommand(ent-g_entities,
            "print \"\nUse ^3[Page Up] ^7and ^3[Page Down] ^7keys to scroll" \
            "\n\n\"");

        return;
    }

    // We can continue.
    // See if this particular vote is disabled.
    if(vote_isDisabled(arg1)){
        G_printInfoMessage(ent, "The '%s' vote has been disabled on this " \
            "server.", arg1);

        return;
    }

    // If there is still a vote to be executed,
    // execute it right now.
    if(level.voteExecuteTime){
        level.voteExecuteTime = 0;
        trap_SendConsoleCommand(EXEC_APPEND, va("%s\n", level.voteString));
    }

    // Execute the command.
    if(!VoteCommands[voteCmd].voteFunc(ent)){
        // Something went wrong, usually a parameter the client forgot
        // or a client not connected, etc.
        return;
    }

    G_printInfoMessageToAll("%s called a vote.", ent->client->pers.cleanName);

    // Start the voting.
    level.voteTime = level.time;
    level.voteYes = 0;
    level.voteNo = 0;
    level.voteMsg30Sec = qfalse;
    level.voteMsg10Sec = qfalse;

    for (i = 0 ; i < level.maxclients; i++){
        level.clients[i].ps.eFlags &= ~EF_VOTED;
    }

    trap_SetConfigstring(CS_VOTE_TIME, va("%i,%i", level.voteTime,
        g_voteDuration.integer*1000));
    trap_SetConfigstring(CS_VOTE_STRING, level.voteDisplayString);
    trap_SetConfigstring(CS_VOTE_YES, va("%i", level.voteYes));
    trap_SetConfigstring(CS_VOTE_NO, va("%i", level.voteNo));
    trap_SetConfigstring(CS_VOTE_NEEDED, va("%i", level.numVotingClients / 2));
}

/*
==================
vote_clientVote_f
5/28/17 - 1:36 PM

This function is the entry point
that handles the client vote.
==================
*/

void vote_clientVote_f(gentity_t *ent)
{
    char    msg[64];

    if(!level.voteTime){
        G_printInfoMessage(ent, "No vote in progress.");
        return;
    }

    if(ent->client->ps.eFlags & EF_VOTED){
        G_printInfoMessage(ent, "Vote already cast.");
        return;
    }

    if(ent->client->sess.team == TEAM_SPECTATOR){
        G_printInfoMessage(ent, "Not allowed to vote as spectator.");
        return;
    }

    G_printInfoMessage(ent, "Vote cast.");

    ent->client->ps.eFlags |= EF_VOTED;
    trap_Argv(1, msg, sizeof(msg));

    if(msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1'){
        level.voteYes++;
        trap_SetConfigstring(CS_VOTE_YES, va("%d", level.voteYes));
    }else{
        level.voteNo++;
        trap_SetConfigstring(CS_VOTE_NO, va("%d", level.voteNo));
    }

    // A majority will be determined in CheckVote, which will also account
    // for players entering or leaving.
}

/*
==================
vote_checkVote_f
5/28/17 - 4:04 PM

This function is the entry point
that handles the vote background logic.
==================
*/

void vote_checkVote_f()
{
    int         i;
    gentity_t   *ent;

    if(level.voteExecuteTime && level.voteExecuteTime < level.time){
        level.voteExecuteTime = 0;
        // Fix for calling a vote for a map defaulting to INF under H&S/H&Z.
        if(strstr(level.voteString, "map")){
            if(current_gametype.value == GT_HS){
                trap_Cvar_Set("g_gametype", "h&s");
            }else if(current_gametype.value == GT_HZ){
                trap_Cvar_Set("g_gametype", "h&z");
            }
        }

        if(strcmp(level.voteString, "mapcycle") == 0){
            G_switchToNextMapInCycle(qfalse);
        }else if(strcmp(level.voteString, "poll") == 0){
            char color[4] = "\0";
            char outcome[50] = "\0";

            // Use a color from the server_colors CVAR so the results are
            // always somewhat in-style with the current color-scheme.
            if (server_colors.string != NULL
                && strlen(server_colors.string) >= 3)
            {
                strncpy(color, va("^%c", server_colors.string[3]),
                    sizeof(color));
            }else{
                // Always have some sort of backup in case no colors are set
                // (highly unlikely though).
                strncpy(color, "^1", sizeof(color));
            }

            // Determine outcome.
            if(level.voteYes > level.voteNo){
                strncpy(outcome, "Most players agreed on the question!",
                    sizeof(outcome));
            }else if(level.voteNo > level.voteYes){
                strncpy(outcome, "Most players didn't agree on the question!",
                    sizeof(outcome));
            }else{
                strncpy(outcome, "Poll draw!",
                    sizeof(outcome));
            }

            // Broadcast the outcome.
            G_Broadcast(va("%sPoll results\n\n^7[^3Question^7] %s%s\n" \
                "^7[^3Voted yes^7]  %s%d\n^7[^3Voted no^7]  %s%d\n\n%s%s",
                color, color, level.voteDisplayString + 6, color, level.voteYes,
                color, level.voteNo, color, outcome), BROADCAST_GAME2, NULL);

            if(level.votePollDisplayMsg < 2){
                level.voteExecuteTime = level.time + 3500;
                level.votePollDisplayMsg++;
            }
        }else if(strcmp(level.voteString, "swapteams") == 0){
            // Don't allow swapteams if the weapons are already given out.
            if(current_gametype.value == GT_HS && level.messagedisplay1){
                G_printInfoMessageToAll("Swapteams is only possible before " \
                    "the weapons are given out!");
            }else{
                SwapTeams(NULL, qtrue);
            }
        }else{
            trap_SendConsoleCommand(EXEC_APPEND, va("%s\n", level.voteString));
        }
    }

    if(!level.voteTime){
        return;
    }

    // Check if we displayed the vote expiry warning messages yet.
    if(!level.voteMsg30Sec && level.time > level.voteTime + 30000){
        for (i = 0; i < level.numConnectedClients; i++){
            ent = &g_entities[level.sortedClients[i]];

            if(~ent->client->ps.eFlags & EF_VOTED){
                G_printInfoMessage(ent, "It seems you haven't voted yet!");
                G_printInfoMessage(ent, "You can usually do this with " \
                    "F1 (yes) or F2 (no).");
            }
        }

        level.voteMsg30Sec = qtrue;
    }else if(!level.voteMsg10Sec && level.time > level.voteTime + 50000){
        for (i = 0; i < level.numConnectedClients; i++){
            ent = &g_entities[level.sortedClients[i]];

            if(~ent->client->ps.eFlags & EF_VOTED){
                G_printInfoMessage(ent, "Hurry up! Just 10 seconds left " \
                    "to vote!");
                G_printInfoMessage(ent, "You can usually do this with " \
                    "F1 (yes) or F2 (no).");
            }
        }

        level.voteMsg10Sec = qtrue;
    }

    // If we're just asking a poll, the vote can never fail.
    // Because of this, the poll vote has its own routine.
    if(strcmp(level.voteString, "poll") == 0){
        // There are never votes needed to pass since this is a poll.
        trap_SetConfigstring (CS_VOTE_NEEDED, "0");

        // Check if the poll time ended.
        if (level.time - level.voteTime >= g_voteDuration.integer*1000){
            G_printInfoMessageToAll("Poll time ended.");
            level.voteExecuteTime = level.time + 3000;
            level.votePollDisplayMsg = 0;
        }else{
            if(level.forceVote){
                // An Admin decided the poll should be ended.
                G_printInfoMessageToAll("Poll time ended.");
                level.voteExecuteTime = level.time + 3000;
                level.votePollDisplayMsg = 0;

                level.forceVote = qfalse;
            }else{
                // Still waiting for a majority.
                return;
            }
        }
    }else{
        // Regular routines.

        // Update the needed clients.
        trap_SetConfigstring(CS_VOTE_NEEDED,
            va("%i", (level.numVotingClients / 2) + 1));

        if(level.time - level.voteTime >= g_voteDuration.integer * 1000){
            G_printInfoMessageToAll("Vote failed.");
            level.clients[level.voteClient].voteDelayTime
                = level.time + g_failedVoteDelay.integer * 60000;
        }else{
            if(level.voteYes > level.numVotingClients / 2){
                // Execute the command, then remove the vote.
                // Also, make sure the vote hasn't been forced by an Admin.
                if(level.forceVote == qfalse){
                    G_printInfoMessageToAll("Vote passed.");
                }else{
                    // Else it must've been, just reset the forceVote state
                    // and don't broadcast the message.. again..
                    level.forceVote = qfalse;
                }
                level.voteExecuteTime = level.time + 3000;
            }else if(level.voteNo >= level.numVotingClients / 2){
                // Same behavior as a timeout.
                G_printInfoMessageToAll("Vote failed.");
                level.clients[level.voteClient].voteDelayTime
                    = level.time + g_failedVoteDelay.integer * 60000;
            }else{
                // Still waiting for a majority.
                return;
            }
        }
    }

    level.voteTime = 0;
    trap_SetConfigstring(CS_VOTE_TIME, "");
}
