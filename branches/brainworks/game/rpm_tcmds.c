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
RPM_Tcmd
Heavily modified version by Boe!Man, 11/16/10
Updated 2/7/11
===========
*/
void RPM_Tcmd ( gentity_t *ent )
{
	char	arg1[MAX_STRING_TOKENS];
	char	arg2[MAX_STRING_TOKENS];

	trap_Argv( 1, arg1, sizeof( arg1 ) );
	trap_Argv( 2, arg2, sizeof( arg2 ) );

	#ifdef _DEBUG
	if (strstr(boe_log.string, "2"))
		G_LogPrintf("1s\n");
	#endif

	if ( !level.gametypeData->teams )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Not playing a team game!\n\""));
		return;
	}
	
	if(!g_enableTeamCmds.integer)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Team Commands are disabled on this server!\n\""));
		return;
	}

	///RxCxW - 09.18.06 - 04:57pm #compmode needed for tcmds no matter what
	///if (g_enableTeamCmds.integer == 1 && !g_compMode.integer)
	if (!g_compMode.integer && g_enableTeamCmds.integer != 2 && !cm_enabled.integer)
	///End  - 09.18.06 - 04:58pm
	{
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Competition Mode must be enabled to use team commands!\n\""));
		return;
	}
	
	if(ent->client->sess.team == TEAM_SPECTATOR){
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Access Denied: You are currently not in a valid team!\n\""));
		return;
	}

	// Boe!Man 11/16/10: Updated /tcmd help screen.
	if (!Q_stricmp ( arg1, "?" )||!Q_stricmp ( arg1, "" )){
		/*
		trap_SendServerCommand( ent-g_entities, va("print \"\n [^3Team commands^7]\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" ** All commands must start with a ^3/tcmd ^7in front of them! **\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" [^4info         ^7displays your teams info (whos ready whos specing you etc)]\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" [^4ready        ^7readys up your whole team so you can start the match      ]\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" [^4invite id#   ^7invtes a someone to spectate your team                    ]\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" [^4uninvite id# ^7un-invites someone from specing your team                 ]\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" [^4lock         ^7locks your team so nobody else can join it                ]\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" [^4unlock       ^7unlocks your team to allow people to join it              ]\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" [^4reset        ^7resets your teams settings (ready status etc...)          ]\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" [^4timeout      ^7pauses the game for a timeout                             ]\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" [^4timein       ^7starts the match from a timeout                           ]\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" ^3** Press PgUp and PgDn keys to scroll up and down the list in console! **\n\""));
		*/
		trap_SendServerCommand( ent-g_entities, va("print \"\n^3Commands         Arguments     Explanation\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \"----------------------------------------------------------\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \"info             <team>        ^7[^3Shows your team's info^7]\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \"invite           <id>          ^7[^3Invites a spectator^7]\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \"uninvite         <id>          ^7[^3Un-invites a spectator^7]\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \"lock                           ^7[^3Locks your team^7]\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \"unlock                         ^7[^3Unlocks your team^7]\n\""));
		// Boe!Man 2/7/11: Removed timeout/timein commands due to excessive abuse (who needs them anyway when you got admins).
		//trap_SendServerCommand( ent-g_entities, va("print \"timeout                        ^7[^3Request a pause^7]\n\""));
		//trap_SendServerCommand( ent-g_entities, va("print \"timein                         ^7[^3End a pause^7]\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \"\n^7Use ^3[Page Up]^7 and ^3[Page Down]^7 keys to scroll.\n\""));
		return;
	}

	
	if (!Q_stricmp ( arg1, "info" ))
	{
		RPM_TeamInfo(ent, arg2);
		return;
	}
	// Boe!Man 11/16/10: We do NOT use this!
	/*
	else if (!Q_stricmp ( arg1, "ready" )){
		RPM_ReadyTeam(ent, qfalse, arg2);
	}*/ 
	else if ( (!Q_stricmp ( arg1, "invite" )) || (!Q_stricmp ( arg1, "uninvite" ))){
		RPM_Invite_Spec(ent, arg2);
	}
	// Boe!Man 2/7/11: Cleaning up some code.
	/*
	else if (!Q_stricmp ( arg1, "uninvite" )){
		RPM_Invite_Spec(ent, arg2);
	}
	*/
	else if ( (!Q_stricmp ( arg1, "lock" )) || (!Q_stricmp ( arg1, "unlock" )) ){
		// Boe!Man 2/7/11: Everybody on a team should be able to lock/unlock their own team(!).
		if(ent->client->sess.team == TEAM_RED){
			RPM_lockTeam(ent, qfalse, "r");
		}else{
			RPM_lockTeam(ent, qfalse, "b");
		}
	}
	// Boe!Man 2/7/11: Remove reset.
	/*
	else if (!Q_stricmp ( arg1, "reset" )){
		RPM_Team_Reset(ent, qfalse, arg2);
	}
	*/
	// Boe!Man 2/7/11: Remove timeout and timein commands.
	/*
	else if (!Q_stricmp ( arg1, "timeout" )){
		RPM_Timeout(ent, qfalse);
	}
	else if (!Q_stricmp ( arg1, "timein" )){
		RPM_Timein(ent);
	}
	*/
	
	else 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Unknown Command  %s.\n\"", arg1));
		trap_SendServerCommand( ent-g_entities, va("print \"Usage: tcmd <command> <variable>\n\""));
	}	

	#ifdef _DEBUG
	if (strstr(boe_log.string, "2"))
		G_LogPrintf("1e\n");
	#endif
}

/*
================
RPM_TeamInfo
Recoded by Boe!Man, 11/16/10 3:11 PM
================
*/
void RPM_TeamInfo (gentity_t *ent, char *team)
{
	int t, i;
	int	invitedcount = 0;
	char *teamName;

	#ifdef _DEBUG
	if (strstr(boe_log.string, "2"))
		G_LogPrintf("2s\n");
	#endif

	t = ent->client->sess.team; // Boe!Man 11/16/10: 1 - Red, 2 - Blue, 3 - Spec.
	if (t == 1){
		teamName = "Red";
	}
	else if (t == 2){
		teamName = "Blue";
	}else{
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You must be in a team to use team commands!\n\""));
		return;
	}

	trap_SendServerCommand( ent-g_entities, va("print \"\n^3Server settings\n\""));
	trap_SendServerCommand( ent-g_entities, va("print \"--------------------------------------\n\""));
	trap_SendServerCommand( ent-g_entities, va("print \"[^3Mod used^7]            %s %s\n", INF_STRING, INF_VERSION_STRING));
	if (g_compMode.integer > 0){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3Competition Mode^7]    Yes\n"));
	}else{
		trap_SendServerCommand( ent-g_entities, va("print \"[^3Competition Mode^7]    No\n"));
	}
	trap_SendServerCommand( ent-g_entities, va("print \"[^3Scorelimit^7]          %i\n", g_scorelimit.integer));
	trap_SendServerCommand( ent-g_entities, va("print \"[^3Timelimit^7]           %i\n", g_timelimit.integer));
	if(strstr(g_gametype.string, "ctf")){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3Respawn interval^7]    %i\n", g_respawnInterval.integer));
	}
	if (g_allowthirdperson.integer > 0){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3Third person^7]        Yes\n"));
	}else{
		trap_SendServerCommand( ent-g_entities, va("print \"[^3Third person^7]        No\n"));
	}
	trap_SendServerCommand( ent-g_entities, va("print \"[^3Speed^7]               %i\n", g_speed.integer));
	trap_SendServerCommand( ent-g_entities, va("print \"[^3Gravity^7]             %i\n", g_gravity.integer));
	trap_SendServerCommand( ent-g_entities, va("print \"[^3Total clients^7]       %i\n", level.numConnectedClients));
	
	trap_SendServerCommand( ent-g_entities, va("print \"\n^3%s team\n\"", teamName));
	trap_SendServerCommand( ent-g_entities, va("print \"--------------------------------------\n\""));
	if (t == 1 && level.redLocked){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3Locked^7]              Yes\n"));
	}else if (t == 2 && level.blueLocked){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3Locked^7]              Yes\n"));
	}else{
		trap_SendServerCommand( ent-g_entities, va("print \"[^3Locked^7]              No\n"));
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
	trap_SendServerCommand( ent-g_entities, va("print \"[^3Invited^7]             %i\n", invitedcount));
	if (cm_enabled.integer == 2 || cm_enabled.integer == 3){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3Round^7]               1st\n"));
	}else if (cm_enabled.integer == 4){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3Round^7]               2nd\n"));
	}else if (cm_enabled.integer == 5){
		trap_SendServerCommand( ent-g_entities, va("print \"[^3Round^7]               Finished\n"));
	}else{
		trap_SendServerCommand( ent-g_entities, va("print \"[^3Round^7]               N/A\n"));
	}

	// Boe!Man 2/7/11: Show the invited people.
	if (invitedcount > 0){
		trap_SendServerCommand( ent-g_entities, va("print \"\n^3Id#  Invited spectators\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \"--------------------------------------\n\""));
		for ( i = 0; i < level.maxclients; i ++ ){
			if(g_entities[i].client->pers.connected == CON_DISCONNECTED )
				continue;
			if(g_entities[i].client->sess.team != TEAM_SPECTATOR)
				continue;
			if(t == TEAM_RED && !g_entities[i].client->sess.invitedByRed)
				continue;
			if(t == TEAM_BLUE && !g_entities[i].client->sess.invitedByBlue)
				continue;
			if(i < 10){
				trap_SendServerCommand( ent-g_entities, va("print \"^7[^3%i^7]  %s\n\"", i, g_entities[i].client->pers.cleanName));
			}else{
				trap_SendServerCommand( ent-g_entities, va("print \"^7[^3%i^7] %s\n\"", i, g_entities[i].client->pers.cleanName));
			}
		}
	}

	// Boe!Man 11/18/10: Print scores as well when the scrim's started.
	if (cm_enabled.integer > 1){
		trap_SendServerCommand( ent-g_entities, va("print \"\n^3Scores\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \"-------------------------------\n\""));
		if (cm_enabled.integer == 2 || cm_enabled.integer == 21){
			if (t == 1){
				trap_SendServerCommand( ent-g_entities, va("print \"[^31st Round^7]           %i - %i\n", level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE]));
			}else if (t == 2){
				trap_SendServerCommand( ent-g_entities, va("print \"[^31st Round^7]           %i - %i\n", level.teamScores[TEAM_BLUE], level.teamScores[TEAM_RED]));
			}
		}
		// Boe!Man 3/19/11: Print the scores written to the CVARs when displaying the 1st round result screen.
		else if(cm_enabled.integer == 3){
			if (t == 1){
				trap_SendServerCommand( ent-g_entities, va("print \"[^31st Round^7]           %i - %i\n", cm_sr.integer, cm_sb.integer));
			}else if (t == 2){
				trap_SendServerCommand( ent-g_entities, va("print \"[^31st Round^7]           %i - %i\n", cm_sb.integer, cm_sr.integer));
			}
		}
		else if (cm_enabled.integer == 4 || cm_enabled.integer == 5){
			if (t == 1){
				trap_SendServerCommand( ent-g_entities, va("print \"[^31st Round^7]           %i - %i\n", cm_sr.integer, cm_sb.integer));
				trap_SendServerCommand( ent-g_entities, va("print \"[^32nd Round^7]           %i - %i\n", level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE]));
				trap_SendServerCommand( ent-g_entities, va("print \"[^3Total^7]               %i - %i\n", level.teamScores[TEAM_RED]+cm_sr.integer, level.teamScores[TEAM_BLUE]+cm_sb.integer));
			}else if (t == 2){
				trap_SendServerCommand( ent-g_entities, va("print \"[^31st Round^7]           %i - %i\n", cm_sb.integer, cm_sr.integer));
				trap_SendServerCommand( ent-g_entities, va("print \"[^32nd Round^7]           %i - %i\n", level.teamScores[TEAM_BLUE], level.teamScores[TEAM_RED]));
				trap_SendServerCommand( ent-g_entities, va("print \"[^3Total^7]               %i - %i\n", level.teamScores[TEAM_BLUE]+cm_sb.integer, level.teamScores[TEAM_RED]+cm_sr.integer));
			}
		}
	}
	trap_SendServerCommand( ent-g_entities, va("print \"\nUse ^3[Page Up] ^7and ^3[Page Down] ^7keys to scroll\n\n\""));

	#ifdef _DEBUG
	if (strstr(boe_log.string, "2"))
		G_LogPrintf("2e\n");
	#endif
}

/*
==================
RPM_lockTeam
==================
*/
void RPM_lockTeam(gentity_t *ent, qboolean referee, char *team)
{
		if (team[0] == 'r' || team[0] == 'R')
		{
			if(level.redLocked)
			{
				level.redLocked = 0;
				if(ent && ent->client){
					// Boe!Man 2/15/11: We need H&S messages as well.
					if(current_gametype.value == GT_HS){
						trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i, ^3%s ^7has %su%sn%sl%so%sc%sked the %s", level.time + 5000, ent->client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, server_hiderteamprefix.string ) );
					}else{
						trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i, ^3%s ^7has %su%sn%sl%so%sc%sked the %s ^7team", level.time + 5000, ent->client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, server_redteamprefix.string ) );
					}
					if (ent->client->sess.admin > 1){
						if(current_gametype.value == GT_HS){
							trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7%s has unlocked the Hiders.\n\"", ent->client->pers.netname));
						}else{
							trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7%s has unlocked the Red team.\n\"", ent->client->pers.netname));
						}
					}
					else if(referee){ // Boe!Man 2/15/11: Not used in H&S.
						trap_SendServerCommand( -1, va("print \"^3[Team Action] ^7%s has unlocked the Red team.\n\"", ent->client->pers.netname));
					}
				}else{
					if(current_gametype.value == GT_HS){
						trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i, ^3%s ^7have been %su%sn%sl%so%sc%sked!", level.time + 5000, server_hiderteamprefix.string, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string ) );
						trap_SendServerCommand( -1, va("print \"^3[Rcon Action] ^7Hiders have been unlocked.\n\""));
					}else{
						trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i, ^3%s ^7team has been %su%sn%sl%so%sc%sked!", level.time + 5000, server_redteamprefix.string, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string ) );
						trap_SendServerCommand( -1, va("print \"^3[Rcon Action] ^7Red team has been unlocked.\n\""));
					}
				}
			}
			else
			{
				level.redLocked = 1;
				if(ent && ent->client){
					// Boe!Man 2/15/11: We need H&S messages as well.
					if(current_gametype.value == GT_HS){
						trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i, ^3%s ^7has %sl%so%sc%sk%se%sd the %s", level.time + 5000, ent->client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, server_hiderteamprefix.string ) );
					}else{
						trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i, ^3%s ^7has %sl%so%sc%sk%se%sd the %s ^7team", level.time + 5000, ent->client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, server_redteamprefix.string ) );
					}

					if (ent->client->sess.admin > 1){
						if(current_gametype.value == GT_HS){
							trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7%s has locked the Hiders.\n\"", ent->client->pers.netname));
						}else{
							trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7%s has locked the Red team.\n\"", ent->client->pers.netname));
						}
					}
					else if(referee){ // Boe!Man 2/15/11: Not used in H&S.
						trap_SendServerCommand( -1, va("print \"^3[Team Action] ^7%s has locked the Red team.\n\"", ent->client->pers.netname));
					}
				}else{
					if(current_gametype.value == GT_HS){
						trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i, ^3%s ^7have been %sl%so%sc%sk%se%sd!", level.time + 5000, server_hiderteamprefix.string, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string ) );
						trap_SendServerCommand( -1, va("print \"^3[Rcon Action] ^7Hiders have been locked.\n\""));
					}else{
						trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i, ^3%s ^7team has been %sl%so%sc%sk%se%sd!", level.time + 5000, server_redteamprefix.string, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string ) );
						trap_SendServerCommand( -1, va("print \"^3[Rcon Action] ^7Red team has been locked.\n\""));
					}
				}
			}
		}
		else if(team[0] == 'b'|| team[0] == 'B')
		{
			if(level.blueLocked)
			{
				level.blueLocked = 0;
				if(ent && ent->client){
					// Boe!Man 2/15/11: We need H&S messages as well.
					if(current_gametype.value == GT_HS){
						trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i, ^3%s ^7has %su%sn%sl%so%sc%sked the %s", level.time + 5000, ent->client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, server_seekerteamprefix.string ) );
					}else{
						trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i, ^3%s ^7has %su%sn%sl%so%sc%sked the %s ^7team", level.time + 5000, ent->client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, server_blueteamprefix.string ) );
					}
					if (ent->client->sess.admin > 1){
						if(current_gametype.value == GT_HS){
							trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7%s has unlocked the Seekers.\n\"", ent->client->pers.netname));
						}else{
							trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7%s has unlocked the Blue team.\n\"", ent->client->pers.netname));
						}
					}
					else if(referee){ // Boe!Man 2/15/11: Not used in H&S.
						trap_SendServerCommand( -1, va("print \"^3[Team Action] ^7%s has unlocked the Blue team.\n\"", ent->client->pers.netname));
					}
				}else{
					if(current_gametype.value == GT_HS){
						trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i, ^3%s ^7have been %su%sn%sl%so%sc%sked!", level.time + 5000, server_seekerteamprefix.string, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string ) );
						trap_SendServerCommand( -1, va("print \"^3[Rcon Action] ^7Seekers have been unlocked.\n\""));
					}else{
						trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i, ^3%s ^7team has been %su%sn%sl%so%sc%sked!", level.time + 5000, server_blueteamprefix.string, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string ) );
						trap_SendServerCommand( -1, va("print \"^3[Rcon Action] ^7Blue team has been unlocked.\n\""));
					}
				}
			}
			else
			{
				level.blueLocked = 1;
				if(ent && ent->client){
					// Boe!Man 2/15/11: We need H&S messages as well.
					if(current_gametype.value == GT_HS){
						trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i, ^3%s ^7has %sl%so%sc%sk%se%sd the %s", level.time + 5000, ent->client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, server_seekerteamprefix.string ) );
					}else{
						trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i, ^3%s ^7has %sl%so%sc%sk%se%sd the %s ^7team", level.time + 5000, ent->client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, server_blueteamprefix.string ) );
					}

					if (ent->client->sess.admin > 1){
						if(current_gametype.value == GT_HS){
							trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7%s has locked the Seekers.\n\"", ent->client->pers.netname));
						}else{
							trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7%s has locked the Blue team.\n\"", ent->client->pers.netname));
						}
					}
					else if(referee){ // Boe!Man 2/15/11: Not used in H&S.
						trap_SendServerCommand( -1, va("print \"^3[Team Action] ^7%s has locked the Blue team.\n\"", ent->client->pers.netname));
					}
				}else{
					if(current_gametype.value == GT_HS){
						trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i, ^3%s ^7have been %sl%so%sc%sk%se%sd!", level.time + 5000, server_seekerteamprefix.string, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string ) );
						trap_SendServerCommand( -1, va("print \"^3[Rcon Action] ^7Seekers have been locked.\n\""));
					}else{
						trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i, ^3%s ^7team has been %sl%so%sc%sk%se%sd!", level.time + 5000, server_blueteamprefix.string, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string ) );
						trap_SendServerCommand( -1, va("print \"^3[Rcon Action] ^7Blue team has been locked.\n\""));
					}
				}
			}
		}
		else if(team[0] == 's' || team[0] == 'S')
		{
			if(level.specsLocked)
			{
				level.specsLocked = 0;
				if(ent && ent->client){
					trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i, ^3%s ^7has %su%sn%sl%so%sc%sked the spectators", level.time + 5000, ent->client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string) );
					if (ent->client->sess.admin > 1)
					trap_SendServerCommand(-1, va("print \"^3[Admin Action] ^7%s has unlocked the Spectators.\n\"", ent->client->pers.netname));
					else
					trap_SendServerCommand( -1, va("print \"^3[Referee Action] ^7%s has unlocked the Spectators.\n\"", ent->client->pers.netname));
				}else{
					// BOE FIXME: We need a CVAR for specs!
					trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i, ^7Spectators have been %su%sn%sl%so%sc%sked!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string ) );
					trap_SendServerCommand( -1, va("print \"^3[Rcon Action] ^7Spectators have been unlocked.\n\""));
				}
			}
			else
			{
				level.specsLocked = 1;
				if(ent && ent->client){
					trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i, ^3%s ^7has %sl%so%sc%sk%se%sd the spectators", level.time + 5000, ent->client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string) );
					if (ent->client->sess.admin > 1)
					trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7%s has locked the Specators.\n\"", ent->client->pers.netname));
					else
					trap_SendServerCommand( -1, va("print \"^3[Referee Action] ^7%s has locked the Spectators.\n\"", ent->client->pers.netname));
				}else{
					// BOE FIXME: We need a CVAR for specs!
					trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,^7Spectators have been %sl%so%sc%sk%se%sd!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string ) );
					trap_SendServerCommand( -1, va("print \"^3[Rcon Action] ^7Spectators have been locked.\n\""));
				}
			}
		}else if(team[0] == 'a' || team[0] == 'A')
		{
			// Boe!Man 1/21/11: There's no specific team for 'all'. So if we lock all teams, we make sure none of the teams are locked (and vice versa).
			// Boe!Man 5/24/12: When unlocking all teams, we need ALL teams to be locked. If there's at least one team unlocked, we lock everything.
			//if(level.blueLocked || level.redLocked || level.specsLocked) // Old stuff from 1/21/11.
			if(level.blueLocked && level.redLocked && level.specsLocked)
			{
				level.specsLocked = 0;
				level.redLocked = 0;
				level.blueLocked = 0;
				if(ent && ent->client){
					trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i, ^3%s ^7has %su%sn%sl%so%sc%sked all the teams", level.time + 5000, ent->client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string) );
					if (ent->client->sess.admin > 1)
					trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7%s has unlocked all the teams.\n\"", ent->client->pers.netname));
					else
					trap_SendServerCommand( -1, va("print \"^3[Referee Action] ^7%s has unlocked all the teams.\n\"", ent->client->pers.netname));
				}else{
					trap_SendServerCommand( -1, va("print \"^3[Rcon Action] ^7All teams have been unlocked.\n\""));
				}
			}
			else // Else at least one team is unlocked, meaning we lock all. Simple as that.
			{
				level.specsLocked = 1;
				level.blueLocked = 1;
				level.redLocked = 1;
				if(ent && ent->client){
					trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i, ^3%s ^7has %sl%so%sc%sk%se%sd all the teams", level.time + 5000, ent->client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string) );
					if (ent->client->sess.admin > 1)
					trap_SendServerCommand( -1, va("print \"^3[Admin Action] ^7%s has locked all the teams.\n\"", ent->client->pers.netname));
					else
					trap_SendServerCommand( -1, va("print \"^3[Referee Action] ^7%s has locked all the teams.\n\"", ent->client->pers.netname));
				}else{
					trap_SendServerCommand( -1, va("print \"^3[Rcon Action] ^7All teams have been locked.\n\""));
				}
			}
			// End Boe!Man 5/24/12.
		}
}

// Boe!Man 11/16/10: We don't need this function. Fate: Removed.
/*
===============
RPM_ReadyTeam
===============

void RPM_ReadyTeam(gentity_t *ent, qboolean referee, char *team)
{
	int		i, t;
	
	if(level.warmupTime == 0 || g_doWarmup.integer != 2)
	{
		trap_SendServerCommand(ent - g_entities, va("print \"Not currently in Ready-Up mode.\n\""));
		return;
	}

	if(referee)
	{
		if(team[0] == 'r' || team[0] == 'R')
		{
			t = TEAM_RED;
		}
		else if(team[0] == 'b'|| team[0] == 'B')
		{
			t = TEAM_BLUE;
		}
		else
		{
			trap_SendServerCommand(ent - g_entities, va("print \"Invalid Command string.\n\""));
			trap_SendServerCommand(ent - g_entities, va("print \"Useage: ref ready <team>\n\""));
		}
	}

	else
	{
		if (ent->client->sess.team != TEAM_RED && ent->client->sess.team != TEAM_BLUE)
		{
			trap_SendServerCommand( ent-g_entities, "print \"Not on a valid team.\n\"");
			return;
		}
		t = ent->client->sess.team;
	}

	

	for ( i = 0; i < level.maxclients; i ++ )
	{	
		if(g_entities[i].client->pers.connected == CON_DISCONNECTED )
		{
			continue;
		}
		if(g_entities[i].client->sess.team != t)
		{
			continue;
		}
		g_entities[i].client->pers.ready = 1;
	}

	trap_SendServerCommand( -1, va("cp \"%s ^7Team Readied by ^3%s\n\"", t == TEAM_RED ? "^1Red" : "^4Blue",  referee ? "Referee" : ent->client->pers.netname));
}
/*

/*
=================
RPM_Invite_Spec
=================
*/
void RPM_Invite_Spec(gentity_t *ent, char *arg2)
{
	int		id;

	// Boe!Man 2/7/11: We don't need this as you can only invite when Competition Mode's enabled anyway.
	//if (!level.specsLocked)
	//{
	//	trap_SendServerCommand( ent-g_entities, "print \"^3[Info] ^7Spectator team is currently not locked.\n\"");
	//	return;
	//}

	if ((ent->client->sess.team != TEAM_RED) && (ent->client->sess.team != TEAM_BLUE) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"^3[Info] ^7Not on a valid team.\n\"");
		return;
	}

	if (arg2[0] >= '0' && arg2[0] <= '9') 
	{
		id = atoi( arg2 );
	}
	else
	{
		// Boe!Man 2/7/11: Merging two messages into one.
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Bad client slot: %s. Usage: tcmd invite/uninvite <idnumber>\n\"", arg2));
		//trap_SendServerCommand( ent-g_entities, va("print \"\n\""));
		return;
	}

	if ( id < 0 || id >= g_maxclients.integer )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Invalid client number %d.\n\"",id));
		return;
	}

	if ( g_entities[id].client->pers.connected == CON_DISCONNECTED )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7There is no client with the client number %d.\n\"", id));
		return;
	}
	if( g_entities[id].client->sess.team != TEAM_SPECTATOR)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7That player is not currently spectating.\n\""));
		return;
	}
	
	if (ent->client->sess.team == TEAM_RED)
	{
		if(g_entities[id].client->sess.invitedByRed)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^3%s ^7was un-invited to spectate your team.\n\"", g_entities[id].client->pers.cleanName));
			trap_SendServerCommand( g_entities[id].s.number, va("cp \"^3You ^7were %su%sn^7-%si%sn%sv%si%sted to spectate\nthe %s ^7team\n\"", server_color1.string, server_color2.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, server_redteamprefix.string));
			g_entities[id].client->sess.invitedByRed = qfalse;
			return;
		}
		else
		{
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^3%s ^7was invited to spectate your team.\n\"", g_entities[id].client->pers.cleanName));
			trap_SendServerCommand( g_entities[id].s.number, va("cp \"^3You ^7were %si%sn%sv%si%st%sed to spectate\nthe %s ^7team\n\"", server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, server_redteamprefix.string));
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
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^3%s ^7was un-invited to spectate your team.\n\"", g_entities[id].client->pers.cleanName));
			trap_SendServerCommand( g_entities[id].s.number, va("cp \"^3You ^7were %su%sn^7-%si%sn%sv%si%sted to spectate\nthe %s ^7team\n\"", server_color1.string, server_color2.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, server_blueteamprefix.string));
			g_entities[id].client->sess.invitedByBlue = qfalse;
			return;
		}
		else
		{
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^3%s ^7was invited to spectate your team.\n\"", g_entities[id].client->pers.cleanName ));
			trap_SendServerCommand( g_entities[id].s.number, va("cp \"^3You ^7were %si%sn%sv%si%st%sed to spectate\nthe %s ^7team\n\"", server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, server_blueteamprefix.string));
			g_entities[id].client->sess.invitedByBlue = qtrue;
			g_entities[id].client->ps.pm_type = PM_SPECTATOR;
			g_entities[id].client->sess.spectatorClient = ent->s.number;
			g_entities[id].client->sess.spectatorState = SPECTATOR_FOLLOW;
			return;
		}
	}
}
/*
==============
RPM_Team_Reset
==============

void RPM_Team_Reset(gentity_t *ent, qboolean referee, char *team)
{
	int		t, i;

	if(referee )
	{
		if(team[0] == 'r' || team[0] == 'R')
		{
			t = TEAM_RED;
		}
		else if(team[0] == 'b' || team[0] == 'B')
		{
			t = TEAM_BLUE;
		}
		else if (team[0] == 'a' || team[0] == 'A')
		{
			t = -1;
		}
		else
		{
			trap_SendServerCommand(ent - g_entities, va("print \"Invalid Command string.\n\""));
			trap_SendServerCommand(ent - g_entities, va("print \"Useage: ref reset Red/Blue\n\""));
		}
	}

	else
	{
		t = ent->client->sess.team;
	}

	if(t == TEAM_SPECTATOR)
	{
		trap_SendServerCommand( ent-g_entities, "print \"Not on a valid team.\n\"");
		return;
	}

	switch (t)
	{
		case TEAM_RED:
			level.redLocked = 0;
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,^1Red ^7team Reset ^7by ^3%s", level.time + 5000, referee ? "Referee" : ent->client->pers.netname));		
			break;
		case TEAM_BLUE:
			level.blueLocked = 0;
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,^4Blue ^7team Reset ^7by ^3%s", level.time + 5000, referee ? "Referee" : ent->client->pers.netname));
			break;
		case -1:
			level.redLocked = 0;
			level.blueLocked = 0;
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,Teams Reset by ^3Referee", level.time + 5000));
			break;
	}
	
	for ( i = 0; i < level.maxclients; i ++ )
	{	
		if(g_entities[i].client->pers.connected == CON_DISCONNECTED )
		{
			continue;
		}

		if(t == TEAM_RED || t == -1)
		{
			g_entities[i].client->sess.invitedByRed = qfalse;
		}

		if(t == TEAM_BLUE || t == -1)
		{
			g_entities[i].client->sess.invitedByBlue = qfalse;
		}

		if(level.warmupTime == 0 || g_doWarmup.integer != 2)
		{
			continue;
		}

		if(g_entities[i].client->sess.team != t && t != -1)
		{
			continue;
		}
		if(g_entities[i].client->pers.ready)
		{
			g_entities[i].client->pers.ready = 0;
		}
	}
}
/*

/*
==============
RPM_Timeout
==============


void RPM_Timeout(gentity_t *ent, qboolean referee)
{
	///RxCxW - 1.14.2005 - #compMode check - dont need
	///if (!g_compMode.integer)
	///{
	///	trap_SendServerCommand( ent-g_entities, "print \"Competition mode is not enabled on this server.\n\"");
	///	return;
	///}
	///End - 1.14.2005

	if(level.warmupTime != 0)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Cannot call a timout in warmup period\n\""));
		return;
	}

	if(level.timeout || level.pause || level.intermissiontime ||  level.intermissionQueued)
	{
		return;
	}

	if(referee)
	{
		level.timeout = 3;
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@Timeout called by ^3Referee", level.time + 10000 ) );
		RPM_Pause(ent);	
		return;
	}

	if ((ent->client->sess.team != TEAM_RED) && (ent->client->sess.team != TEAM_BLUE) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"Not on a valid team.\n\"");
		return;
	}
	
	switch(ent->client->sess.team)
	{
		case TEAM_RED:
			if(level.redtimeouts == 3)
			{
				trap_SendServerCommand( ent-g_entities, "print \"Your team has used all of its timeouts for this round.\n\"");
				return;
			}
			level.timeout = TEAM_RED;
			level.redtimeouts++;
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@Timeout called by ^1Red ^7team", level.time + 10000 ) );
			RPM_Pause(ent);	
			return;
		case TEAM_BLUE:
			if(level.bluetimeouts == 3)
			{
				trap_SendServerCommand( ent-g_entities, "print \"Your team has used all of its timeouts for this round.\n\"");
				return;
			}
			level.timeout = TEAM_BLUE;
			level.bluetimeouts++;
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@Timeout called by ^4Blue ^7team", level.time + 10000 ) );
			RPM_Pause(ent);
			return;
		default:
			trap_SendServerCommand( ent-g_entities, "print \"Not on a valid Team!\n\"");
			return;
	}
}
*/

/*
=============
RPM_Timein
=============

void RPM_Timein (gentity_t *ent)
{
	if(!level.timeout)
	{
		trap_SendServerCommand( ent-g_entities, "print \"The game is not in a timeout!\n\"");
		return;
	}

	if(level.timeout == 3)
	{
		level.timeout = 0;
		RPM_Unpause(ent);
		return;
	}

	if ((ent->client->sess.team != TEAM_RED) && (ent->client->sess.team != TEAM_BLUE) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"Not on a valid team.\n\"");
		return;
	}

	if(level.timeout != ent->client->sess.team)
	{
		trap_SendServerCommand( ent-g_entities, "print \"Your team did not call the timeout!\n\"");
		return;
	}
	level.timeout = 0;
	RPM_Unpause(ent);
}

*/
