// Copyright (C) 2004-2005 Ryan Powell

//  This file is part of RPM.

//  RPM is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation;either version 2 of the License, or
//  (at your option) any later version.

//  RPM is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with RPM; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
#include "g_local.h"
#include "boe_local.h"

/*
===========
G_Tcmd
Originally RPM_Tcmd
Heavily modified version by Boe!Man, 11/16/10
Updated 2/7/11
===========
*/
void G_Tcmd ( gentity_t *ent )
{
    char    arg1[MAX_STRING_TOKENS];
    char    arg2[MAX_STRING_TOKENS];

    trap_Argv( 1, arg1, sizeof( arg1 ) );
    trap_Argv( 2, arg2, sizeof( arg2 ) );

    if ( !level.gametypeData->teams )
    {
        G_printInfoMessage(ent, "Not playing a team game!");
        return;
    }

    if(!g_enableTeamCmds.integer)
    {
        G_printInfoMessage(ent, "Team commands are disabled on this server!");
        return;
    }

    ///RxCxW - 09.18.06 - 04:57pm #compmode needed for tcmds no matter what
    ///if (g_enableTeamCmds.integer == 1 && !g_compMode.integer)
    if (!g_compMode.integer && g_enableTeamCmds.integer != 2 && !cm_enabled.integer)
    ///End  - 09.18.06 - 04:58pm
    {
        G_printInfoMessage(ent, "Competition Mode must be enabled to use team commands!");
        return;
    }

    if(ent->client->sess.team == TEAM_SPECTATOR){
        G_printInfoMessage(ent, "Access Denied: You are currently not in a valid team!");
        return;
    }

    // Boe!Man 11/16/10: Updated /tcmd help screen.
    if (!Q_stricmp ( arg1, "?" )||!Q_stricmp ( arg1, "" )){
        trap_SendServerCommand( ent-g_entities, va("print \"\n^3%-16s %-13s Explanation\n\"", "Commands", "Arguments"));
        trap_SendServerCommand( ent-g_entities, "print \"----------------------------------------------------------\n\"");
        trap_SendServerCommand( ent-g_entities, va("print \"%-16s %-13s ^7[^3Shows your team's info^7]\n\"", "info", "<team>"));
        trap_SendServerCommand( ent-g_entities, va("print \"%-16s %-13s ^7[^3Invites a spectator^7]\n\"", "invite", "<id>"));
        trap_SendServerCommand( ent-g_entities, va("print \"%-16s %-13s ^7[^3Un-invites a spectator^7]\n\"", "uninvite", "<id>"));
        trap_SendServerCommand( ent-g_entities, va("print \"%-30s ^7[^3Locks your team^7]\n\"", "lock"));
        trap_SendServerCommand( ent-g_entities, va("print \"%-30s ^7[^3Unlocks your team^7]\n\"", "unlock"));
        trap_SendServerCommand( ent-g_entities, "print \"\n^7Use ^3[Page Up]^7 and ^3[Page Down]^7 keys to scroll.\n\"");
        return;
    }


    if (!Q_stricmp ( arg1, "info" ))
    {
        G_TeamInfo(ent, arg2);
        return;
    }
    else if ( (!Q_stricmp ( arg1, "invite" )) || (!Q_stricmp ( arg1, "uninvite" ))){
        G_Invite_Spec(ent, arg2);
    }
    else if ( (!Q_stricmp ( arg1, "lock" )) || (!Q_stricmp ( arg1, "unlock" )) ){
        // Boe!Man 2/7/11: Everybody on a team should be able to lock/unlock their own team(!).
        if(ent->client->sess.team == TEAM_RED){
            G_lockTeam(ent, qfalse, "r");
        }else{
            G_lockTeam(ent, qfalse, "b");
        }
    }
    else
    {
        trap_SendServerCommand( ent-g_entities, va("print \"Unknown Command  %s.\n\"", arg1));
        trap_SendServerCommand( ent-g_entities, "print \"Usage: tcmd <command> <variable>\n\"");
    }
}

/*
================
G_TeamInfo
Originally RPM_TeamInfo
Recoded by Boe!Man, 11/16/10 3:11 PM
================
*/
void G_TeamInfo (gentity_t *ent, char *team)
{
    int t, i;
    int invitedcount = 0;
    char *teamName;

    t = ent->client->sess.team; // Boe!Man 11/16/10: 1 - Red, 2 - Blue, 3 - Spec.
    if (t == 1){
        teamName = "Red";
    }
    else if (t == 2){
        teamName = "Blue";
    }else{
        G_printInfoMessage(ent, "You must be in a team to use team commands!");
        return;
    }

    trap_SendServerCommand( ent-g_entities, "print \"\n^3Server settings\n\"");
    trap_SendServerCommand( ent-g_entities, "print \"--------------------------------------\n\"");
    trap_SendServerCommand( ent-g_entities, va("print \"%-25s %s\n", "[^3Mod used^7]", MODFULL));
    if (g_compMode.integer > 0){
        trap_SendServerCommand( ent-g_entities, va("print \"%-25s Yes\n", "[^3Competition Mode^7]"));
    }else{
        trap_SendServerCommand( ent-g_entities, va("print \"%-25s No\n", "[^3Competition Mode^7]"));
    }
    trap_SendServerCommand( ent-g_entities, va("print \"%-25s %i\n", "[^3Scorelimit^7]", g_scorelimit.integer));
    trap_SendServerCommand( ent-g_entities, va("print \"%-25s %i\n", "[^3Timelimit^7]", g_timelimit.integer));
    if(strstr(g_gametype.string, "ctf")){
        trap_SendServerCommand( ent-g_entities, va("print \"%-25s %i\n", "[^3Respawn interval^7]", g_respawnInterval.integer));
    }
    #ifndef _DEMO
    #ifndef _GOLD
    if (g_allowthirdperson.integer > 0){
    #else
    if (g_enforce1fxAdditions.integer && g_allowthirdperson.integer){
    #endif // not _GOLD
        trap_SendServerCommand( ent-g_entities, va("print \"%-25s Yes\n", "[^3Third person^7]"));
    }else{
        trap_SendServerCommand( ent-g_entities, va("print \"%-25s No\n", "[^3Third person^7]"));
    }
    #endif // not _DEMO
    trap_SendServerCommand( ent-g_entities, va("print \"%-25s %i\n", "[^3Speed^7]", g_speed.integer));
    trap_SendServerCommand( ent-g_entities, va("print \"%-25s %i\n", "[^3Gravity^7]", g_gravity.integer));
    trap_SendServerCommand( ent-g_entities, va("print \"%-25s %i\n", "[^3Total clients^7]", level.numConnectedClients));

    trap_SendServerCommand( ent-g_entities, va("print \"\n^3%s team\n\"", teamName));
    trap_SendServerCommand( ent-g_entities, "print \"--------------------------------------\n\"");
    if ((t == TEAM_RED && level.redLocked) || (t == TEAM_BLUE && level.blueLocked)){
        trap_SendServerCommand( ent-g_entities, va("print \"%-25s Yes\n", "[^3Locked^7]"));
    }else{
        trap_SendServerCommand( ent-g_entities, va("print \"%-25s No\n", "[^3Locked^7]"));
    }
    // Boe!Man 2/7/11: Display the amount of invited specs to the team.
    for ( i = 0; i < level.maxclients; i ++ ){
        if(g_entities[i].client->pers.connected == CON_DISCONNECTED )
            continue;
        if(g_entities[i].client->sess.team != TEAM_SPECTATOR)
            continue;
        if(t == 1 && !g_entities[i].client->sess.invitedByRed)
            continue;
        if(t == 2 && !g_entities[i].client->sess.invitedByBlue)
            continue;
        invitedcount += 1;
    }
    trap_SendServerCommand( ent-g_entities, va("print \"%-25s %i\n", "[^3Invited^7]", invitedcount));
    if (cm_enabled.integer == 2 || cm_enabled.integer == 3){
        trap_SendServerCommand( ent-g_entities, va("print \"%-25s 1st\n", "[^3Round^7]"));
    }else if (cm_enabled.integer == 4){
        trap_SendServerCommand( ent-g_entities, va("print \"%-25s 2nd\n", "[^3Round^7]"));
    }else if (cm_enabled.integer == 5){
        trap_SendServerCommand( ent-g_entities, va("print \"%-25s Finished\n", "[^3Round^7]"));
    }else{
        trap_SendServerCommand( ent-g_entities, va("print \"%-25s N/A\n", "[^3Round^7]"));
    }

    // Boe!Man 2/7/11: Show the invited people.
    if (invitedcount > 0){
        trap_SendServerCommand( ent-g_entities, va("print \"\n^3%-4s Invited spectators\n\"", "Id#"));
        trap_SendServerCommand( ent-g_entities, "print \"--------------------------------------\n\"");
        for ( i = 0; i < level.maxclients; i ++ ){
            if(g_entities[i].client->pers.connected == CON_DISCONNECTED )
                continue;
            if(g_entities[i].client->sess.team != TEAM_SPECTATOR)
                continue;
            if(t == TEAM_RED && !g_entities[i].client->sess.invitedByRed)
                continue;
            if(t == TEAM_BLUE && !g_entities[i].client->sess.invitedByBlue)
                continue;

            trap_SendServerCommand( ent-g_entities, va("print \"^7[^3%i^7] %2s\n\"", i, g_entities[i].client->pers.cleanName));
        }
    }

    // Boe!Man 11/18/10: Print scores as well when the scrim's started.
    if (cm_enabled.integer > 1){
        trap_SendServerCommand( ent-g_entities, "print \"\n^3Scores\n\"");
        trap_SendServerCommand( ent-g_entities, "print \"-------------------------------\n\"");
        if (cm_enabled.integer == 2 || cm_enabled.integer == 21){
            if (t == 1){
                trap_SendServerCommand( ent-g_entities, va("print \"%-25s %i - %i\n", "[^31st Round^7]", level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE]));
            }else if (t == 2){
                trap_SendServerCommand( ent-g_entities, va("print \"%-25s %i - %i\n", "[^31st Round^7]", level.teamScores[TEAM_BLUE], level.teamScores[TEAM_RED]));
            }
        }
        // Boe!Man 3/19/11: Print the scores written to the CVARs when displaying the 1st round result screen.
        else if(cm_enabled.integer == 3){
            if (t == 1){
                trap_SendServerCommand( ent-g_entities, va("print \"%-25s %i - %i\n", "[^31st Round^7]", cm_sr.integer, cm_sb.integer));
            }else if (t == 2){
                trap_SendServerCommand( ent-g_entities, va("print \"%-25s %i - %i\n", "[^31st Round^7]", cm_sb.integer, cm_sr.integer));
            }
        }
        else if (cm_enabled.integer == 4 || cm_enabled.integer == 5){
            if (t == 1){
                trap_SendServerCommand( ent-g_entities, va("print \"%-25s %i - %i\n", "[^31st Round^7]", cm_sr.integer, cm_sb.integer));
                trap_SendServerCommand( ent-g_entities, va("print \"%-25s %i - %i\n", "[^32nd Round^7]", level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE]));
                trap_SendServerCommand( ent-g_entities, va("print \"%-25s %i - %i\n", "[^3Total^7]", level.teamScores[TEAM_RED]+cm_sr.integer, level.teamScores[TEAM_BLUE]+cm_sb.integer));
            }else if (t == 2){
                trap_SendServerCommand( ent-g_entities, va("print \"%-25s %i - %i\n", "[^31st Round^7]", cm_sb.integer, cm_sr.integer));
                trap_SendServerCommand( ent-g_entities, va("print \"%-25s %i - %i\n", "[^32nd Round^7]", level.teamScores[TEAM_BLUE], level.teamScores[TEAM_RED]));
                trap_SendServerCommand( ent-g_entities, va("print \"%-25s %i - %i\n", "[^3Total^7]", level.teamScores[TEAM_BLUE]+cm_sb.integer, level.teamScores[TEAM_RED]+cm_sr.integer));
            }
        }
    }
    trap_SendServerCommand( ent-g_entities, "print \"\nUse ^3[Page Up] ^7and ^3[Page Down] ^7keys to scroll\n\n\"");
}

/*
==================
G_lockTeam
Originally RPM_lockTeam
==================
*/
qboolean G_lockTeam(gentity_t *ent, qboolean referee, char *team)
{
    if (!team || !team[0]){
        return qfalse;
    }

    if (team[0] == 'r' || team[0] == 'R'){
        if(level.redLocked){
            level.redLocked = 0;
            if(ent && ent->client){
                // Boe!Man 2/15/11: We need H&S messages as well.
                if(current_gametype.value == GT_HS){
                    G_Broadcast(va("%s\nhas \\unlocked the %s", ent->client->pers.netname, server_hiderteamprefix.string), BROADCAST_CMD, NULL);
                }else{
                    G_Broadcast(va("%s\nhas \\unlocked the %s ^7team", ent->client->pers.netname, server_redteamprefix.string), BROADCAST_CMD, NULL);
                }

                if (ent->client->sess.admin > 1){
                    if(current_gametype.value == GT_HS){
                        trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7%s has unlocked the hiders.\n\"", ent->client->pers.cleanName));
                    }else{
                        trap_SendServerCommand(-1, va("print \"^3[Admin Action] ^7%s has unlocked the red team.\n\"", ent->client->pers.cleanName));
                    }
                }else if(referee){ // Boe!Man 2/15/11: Not used in H&S.
                    trap_SendServerCommand(-1, va("print \"^3[Team Action] ^7%s has unlocked the red team.\n\"", ent->client->pers.cleanName));
                }
            }else{
                if(current_gametype.value == GT_HS){
                    G_Broadcast(va("%s\n^7have been \\unlocked", server_hiderteamprefix.string), BROADCAST_CMD, NULL);
                    trap_SendServerCommand( -1, "print \"^3[Rcon Action] ^7Hiders have been unlocked.\n\"");
                }else{
                    G_Broadcast(va("%s ^7team\nhas been \\unlocked", server_redteamprefix.string), BROADCAST_CMD, NULL);
                    trap_SendServerCommand( -1, "print \"^3[Rcon Action] ^7Red team has been unlocked.\n\"");
                }
            }
        }else{
            level.redLocked = 1;

            if(ent && ent->client){
                // Boe!Man 2/15/11: We need H&S messages as well.
                if(current_gametype.value == GT_HS){
                    G_Broadcast(va("%s\nhas \\locked the %s", ent->client->pers.netname, server_hiderteamprefix.string), BROADCAST_CMD, NULL);
                }else{
                    G_Broadcast(va("%s\nhas \\locked the %s ^7team", ent->client->pers.netname, server_redteamprefix.string), BROADCAST_CMD, NULL);
                }

                if (ent->client->sess.admin > 1){
                    if(current_gametype.value == GT_HS){
                        trap_SendServerCommand(-1, va("print \"^3[Admin Action] ^7%s has locked the hiders.\n\"", ent->client->pers.cleanName));
                    }else{
                        trap_SendServerCommand(-1, va("print \"^3[Admin Action] ^7%s has locked the red team.\n\"", ent->client->pers.cleanName));
                    }
                }else if(referee){ // Boe!Man 2/15/11: Not used in H&S.
                    trap_SendServerCommand(-1, va("print \"^3[Team Action] ^7%s has locked the red team.\n\"", ent->client->pers.cleanName));
                }
            }else{
                if(current_gametype.value == GT_HS){
                    G_Broadcast(va("%s\n^7have been \\locked", server_hiderteamprefix.string), BROADCAST_CMD, NULL);
                    trap_SendServerCommand(-1, "print \"^3[Rcon Action] ^7Hiders have been locked.\n\"");
                }else{
                    G_Broadcast(va("%s ^7team\nhas been \\locked", server_redteamprefix.string), BROADCAST_CMD, NULL);
                    trap_SendServerCommand(-1, "print \"^3[Rcon Action] ^7Red team has been locked.\n\"");
                }
            }
        }
    }else if(team[0] == 'b'|| team[0] == 'B'){
        if(level.blueLocked){
            level.blueLocked = 0;
            if(ent && ent->client){
                // Boe!Man 2/15/11: We need H&S messages as well.
                if(current_gametype.value == GT_HS){
                    G_Broadcast(va("%s\nhas \\unlocked the %s", ent->client->pers.netname, server_seekerteamprefix.string), BROADCAST_CMD, NULL);
                }else{
                    G_Broadcast(va("%s\nhas \\unlocked the %s ^7team", ent->client->pers.netname, server_blueteamprefix.string), BROADCAST_CMD, NULL);
                }

                if (ent->client->sess.admin > 1){
                    if(current_gametype.value == GT_HS){
                        trap_SendServerCommand(-1, va("print \"^3[Admin Action] ^7%s has unlocked the seekers.\n\"", ent->client->pers.cleanName));
                    }else{
                        trap_SendServerCommand(-1, va("print \"^3[Admin Action] ^7%s has unlocked the blue team.\n\"", ent->client->pers.cleanName));
                    }
                }else if(referee){ // Boe!Man 2/15/11: Not used in H&S.
                    trap_SendServerCommand(-1, va("print \"^3[Team Action] ^7%s has unlocked the blue team.\n\"", ent->client->pers.cleanName));
                }
            }else{
                if(current_gametype.value == GT_HS){
                    G_Broadcast(va("%s\n^7have been \\unlocked", server_seekerteamprefix.string), BROADCAST_CMD, NULL);
                    trap_SendServerCommand(-1, "print \"^3[Rcon Action] ^7Seekers have been unlocked.\n\"");
                }else{
                    G_Broadcast(va("%s ^7team\nhas been \\unlocked", server_blueteamprefix.string), BROADCAST_CMD, NULL);
                    trap_SendServerCommand(-1, "print \"^3[Rcon Action] ^7Blue team has been unlocked.\n\"");
                }
            }
        }else{
            level.blueLocked = 1;
            if(ent && ent->client){
                // Boe!Man 2/15/11: We need H&S messages as well.
                if(current_gametype.value == GT_HS){
                    G_Broadcast(va("%s\nhas \\locked the %s", ent->client->pers.netname, server_seekerteamprefix.string), BROADCAST_CMD, NULL);
                }else{
                    G_Broadcast(va("%s\nhas \\locked the %s ^7team", ent->client->pers.netname, server_blueteamprefix.string), BROADCAST_CMD, NULL);
                }

                if (ent->client->sess.admin > 1){
                    if(current_gametype.value == GT_HS){
                        trap_SendServerCommand(-1, va("print \"^3[Admin Action] ^7%s has locked the seekers.\n\"", ent->client->pers.cleanName));
                    }else{
                        trap_SendServerCommand(-1, va("print \"^3[Admin Action] ^7%s has locked the blue team.\n\"", ent->client->pers.cleanName));
                    }
                }else if(referee){ // Boe!Man 2/15/11: Not used in H&S.
                    trap_SendServerCommand(-1, va("print \"^3[Team Action] ^7%s has locked the blue team.\n\"", ent->client->pers.cleanName));
                }
            }else{
                if(current_gametype.value == GT_HS){
                    G_Broadcast(va("%s\n^7have been \\locked", server_seekerteamprefix.string), BROADCAST_CMD, NULL);
                    trap_SendServerCommand(-1, "print \"^3[Rcon Action] ^7Seekers have been locked.\n\"");
                }else{
                    G_Broadcast(va("%s ^7team\nhas been \\locked", server_blueteamprefix.string), BROADCAST_CMD, NULL);
                    trap_SendServerCommand(-1, "print \"^3[Rcon Action] ^7Blue team has been locked.\n\"");
                }
            }
        }
    }else if(team[0] == 's' || team[0] == 'S'){
        if(level.specsLocked){
            level.specsLocked = 0;
            if(ent && ent->client){
                G_Broadcast(va("%s\nhas \\unlocked the spectators", ent->client->pers.netname), BROADCAST_CMD, NULL);

                if (ent->client->sess.admin > 1){
                    trap_SendServerCommand(-1, va("print \"^3[Admin Action] ^7%s has unlocked the spectators.\n\"", ent->client->pers.cleanName));
                }else{
                    trap_SendServerCommand(-1, va("print \"^3[Referee Action] ^7%s has unlocked the spectators.\n\"", ent->client->pers.cleanName));
                }
            }else{
                G_Broadcast("Spectators have been \\unlocked!", BROADCAST_CMD, NULL);
                trap_SendServerCommand( -1, "print \"^3[Rcon Action] ^7Spectators have been unlocked.\n\"");
            }
        }else{
            level.specsLocked = 1;
            if(ent && ent->client){
                G_Broadcast(va("%s\nhas \\locked the spectators", ent->client->pers.netname), BROADCAST_CMD, NULL);

                if (ent->client->sess.admin > 1){
                    trap_SendServerCommand(-1, va("print \"^3[Admin Action] ^7%s has locked the specators.\n\"", ent->client->pers.cleanName));
                }else{
                    trap_SendServerCommand(-1, va("print \"^3[Referee Action] ^7%s has locked the spectators.\n\"", ent->client->pers.cleanName));
                }
            }else{
                G_Broadcast("Spectators have been \\locked!", BROADCAST_CMD, NULL);
                trap_SendServerCommand( -1, "print \"^3[Rcon Action] ^7Spectators have been locked.\n\"");
            }
        }
    }else if(team[0] == 'a' || team[0] == 'A'){
        // There's no specific team for 'all'. So if we lock all teams, we make sure none of the teams are locked (and vice versa).
        // When unlocking all teams, we need ALL teams to be locked. If there's at least one team unlocked, we lock everything.
        if(level.blueLocked && level.redLocked && level.specsLocked){
            level.specsLocked = 0;
            level.redLocked = 0;
            level.blueLocked = 0;
            if(ent && ent->client){
                G_Broadcast(va("%s\nhas \\unlocked all teams", ent->client->pers.netname), BROADCAST_CMD, NULL);

                if (ent->client->sess.admin > 1){
                    trap_SendServerCommand(-1, va("print \"^3[Admin Action] ^7%s has unlocked all the teams.\n\"", ent->client->pers.cleanName));
                }else{
                    trap_SendServerCommand(-1, va("print \"^3[Referee Action] ^7%s has unlocked all the teams.\n\"", ent->client->pers.cleanName));
                }
            }else{
                G_Broadcast("All teams have been \\unlocked!", BROADCAST_CMD, NULL);
                trap_SendServerCommand(-1, "print \"^3[Rcon Action] ^7All teams have been unlocked.\n\"");
            }
        }else{
            // Else at least one team is unlocked, meaning we lock all. Simple as that.
            level.specsLocked = 1;
            level.blueLocked = 1;
            level.redLocked = 1;

            if(ent && ent->client){
                G_Broadcast(va("%s\nhas \\locked all teams", ent->client->pers.netname), BROADCAST_CMD, NULL);

                if (ent->client->sess.admin > 1){
                    trap_SendServerCommand(-1, va("print \"^3[Admin Action] ^7%s has locked all the teams.\n\"", ent->client->pers.cleanName));
                }else{
                    trap_SendServerCommand(-1, va("print \"^3[Referee Action] ^7%s has locked all the teams.\n\"", ent->client->pers.cleanName));
                }
            }else{
                G_Broadcast("All teams have been \\locked!", BROADCAST_CMD, NULL);
                trap_SendServerCommand(-1, "print \"^3[Rcon Action] ^7All teams have been locked.\n\"");
            }
        }
    }else{
        return qfalse;
    }

    Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
    return qtrue;
}

/*
=================
G_Invite_Spec
Originally RPM_Invite_Spec
=================
*/
void G_Invite_Spec(gentity_t *ent, char *arg2)
{
    int     id;

    if ((ent->client->sess.team != TEAM_RED) && (ent->client->sess.team != TEAM_BLUE) )
    {
        G_printInfoMessage(ent, "You're not on a valid team.");
        return;
    }

    if (arg2[0] >= '0' && arg2[0] <= '9')
    {
        id = atoi( arg2 );
    }
    else
    {
        // Boe!Man 2/7/11: Merging two messages into one.
        G_printInfoMessage(ent, "Bad client slot: %s. Usage: tcmd invite/uninvite <idnumber>.", arg2);
        return;
    }

    if ( id < 0 || id >= g_maxclients.integer )
    {
        G_printInfoMessage(ent, "Invalid client number %d.", id);
        return;
    }

    if ( g_entities[id].client->pers.connected == CON_DISCONNECTED )
    {
        G_printInfoMessage(ent, "There is no client with the client number %d.", id);
        return;
    }
    if( g_entities[id].client->sess.team != TEAM_SPECTATOR)
    {
        G_printInfoMessage(ent, "That player is not currently spectating.");
        return;
    }

    if (ent->client->sess.team == TEAM_RED)
    {
        if(g_entities[id].client->sess.invitedByRed)
        {
            G_printInfoMessage(ent, "%s was un-invited to spectate the Red team.", g_entities[id].client->pers.cleanName);
            G_Broadcast(va("You were \\un-invited \nto spectate the %s ^7team!", server_redteamprefix.string), BROADCAST_CMD, &g_entities[id]);
            g_entities[id].client->sess.invitedByRed = qfalse;
            return;
        }
        else
        {
            G_printInfoMessage(ent, "%s was invited to spectate the Red team.", g_entities[id].client->pers.cleanName);
            G_Broadcast(va("You were \\invited \nto spectate the %s ^7team!", server_redteamprefix.string), BROADCAST_CMD, &g_entities[id]);
            g_entities[id].client->sess.invitedByRed = qtrue;
            g_entities[id].client->ps.pm_type = PM_SPECTATOR;
            g_entities[id].client->sess.spectatorClient = ent->s.number;
            g_entities[id].client->sess.spectatorState = SPECTATOR_FOLLOW;
            return;
        }
    }
    if (ent->client->sess.team == TEAM_BLUE)
    {
        if(g_entities[id].client->sess.invitedByBlue)
        {
            G_printInfoMessage(ent, "%s was un-invited to spectate the Blue team.", g_entities[id].client->pers.cleanName);
            G_Broadcast(va("You were \\un-invited \nto spectate the %s ^7team!", server_blueteamprefix.string), BROADCAST_CMD, &g_entities[id]);
            g_entities[id].client->sess.invitedByBlue = qfalse;
            return;
        }
        else
        {
            G_printInfoMessage(ent, "%s was invited to spectate the Blue team.", g_entities[id].client->pers.cleanName);
            G_Broadcast(va("You were \\invited \nto spectate the %s ^7team!", server_blueteamprefix.string), BROADCAST_CMD, &g_entities[id]);
            g_entities[id].client->sess.invitedByBlue = qtrue;
            g_entities[id].client->ps.pm_type = PM_SPECTATOR;
            g_entities[id].client->sess.spectatorClient = ent->s.number;
            g_entities[id].client->sess.spectatorState = SPECTATOR_FOLLOW;
            return;
        }
    }
}
