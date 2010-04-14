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

/*
===========
RPM_Tcmd
===========
*/
void RPM_Tcmd ( gentity_t *ent )
{
	char	arg1[MAX_STRING_TOKENS];
	char	arg2[MAX_STRING_TOKENS];

	trap_Argv( 1, arg1, sizeof( arg1 ) );
	trap_Argv( 2, arg2, sizeof( arg2 ) );

	if (!Q_stricmp ( arg1, "?" )){
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
		return;
	}

	if ( !level.gametypeData->teams )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Not playing a team game!\n\""));
		return;
	}

	if (!Q_stricmp ( arg1, "info" ))
	{
		RPM_TeamInfo(ent, arg2);
		return;
	}
	
	if(!g_enableTeamCmds.integer)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Team Commands are disabled on this server!\n\""));
		return;
	}

	///RxCxW - 09.18.06 - 04:57pm #compmode needed for tcmds no matter what
	///if (g_enableTeamCmds.integer == 1 && !g_compMode.integer)
	if (!g_compMode.integer)
	///End  - 09.18.06 - 04:58pm
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Server must be in Competetion Mode to use Team Commands!\n\""));
		return;
	}

	if (!Q_stricmp ( arg1, "ready" )){
		RPM_ReadyTeam(ent, qfalse, arg2);
	}
	else if (!Q_stricmp ( arg1, "invite" )){
		RPM_Invite_Spec(ent, arg2);
	}
	else if (!Q_stricmp ( arg1, "uninvite" )){
		RPM_Invite_Spec(ent, arg2);
	}
	else if ( (!Q_stricmp ( arg1, "lock" )) || (!Q_stricmp ( arg1, "unlock" )) ){
		RPM_lockTeam(ent, qfalse, arg2);
	}
	else if (!Q_stricmp ( arg1, "reset" )){
		RPM_Team_Reset(ent, qfalse, arg2);
	}
	else if (!Q_stricmp ( arg1, "timeout" )){
		RPM_Timeout(ent, qfalse);
	}
	else if (!Q_stricmp ( arg1, "timein" )){
		RPM_Timein(ent);
	}
	
	else 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Unknown Command  %s.\n\"", arg1));
		trap_SendServerCommand( ent-g_entities, va("print \"Usage: tcmd <command> <variable>\n\""));
	}	
}

/*
================
RPM_TeamInfo
================
*/
void RPM_TeamInfo (gentity_t *ent, char *team)
{
	int		i, t;
	char	*ready = "";
	char    *teamName = "";

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
		t = ent->client->sess.team;
	}

	switch(t)
	{
		case TEAM_RED:
			teamName = "^1RED";
			break;
		case TEAM_BLUE:
			teamName = "^4BLUE";
			break;
		default:
			trap_SendServerCommand( ent-g_entities, va("print \"Not a valid Team!\n\""));
			return;
	}

	trap_SendServerCommand( ent-g_entities, va("print \"%s ^3Team Info\n\"", teamName));
	trap_SendServerCommand( ent-g_entities, va("print \"========================================\n\""));
	
	if(t == TEAM_RED && level.redLocked)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"^1Red ^3team is currently locked!\n\""));
	}

	else if( t == TEAM_BLUE && level.blueLocked)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"^4Blue ^3team is currently locked!\n\""));
	}

	trap_SendServerCommand( ent-g_entities, va("print \"^2These people are invited to spectate the %s ^2team:\n\"", t==TEAM_RED ? "^1Red" : "^4Blue"));
	trap_SendServerCommand( ent-g_entities, va("print \"  ^6Id#   Name\n\""));
	for ( i = 0; i < level.maxclients; i ++ )
	{
		if(g_entities[i].client->pers.connected == CON_DISCONNECTED )
		{
			continue;
		}
		if(g_entities[i].client->sess.team != TEAM_SPECTATOR)
		{
			continue;
		}
		if(t == TEAM_RED && !g_entities[i].client->sess.invitedByRed)
		{
			continue;
		}
		if(t == TEAM_BLUE && !g_entities[i].client->sess.invitedByBlue)
		{
			continue;
		}
		trap_SendServerCommand( ent-g_entities, va("print \"[%3d   %25s^7 ]\n\"", i, g_entities[i].client->pers.cleanName));
	}

	trap_SendServerCommand( ent-g_entities, va("print \"\n%s ^2team roster and ready status:\n\"", t==TEAM_RED ? "^1Red" : "^4Blue"));
	trap_SendServerCommand( ent-g_entities, va("print \"  ^6Id#   Name\n\""));
	for ( i = 0; i < level.maxclients; i ++ )
	{
		ready = "";	

		if(g_entities[i].client->pers.connected == CON_DISCONNECTED )
		{
			continue;
		}
		if(g_entities[i].client->sess.team != t)
		{
			continue;
		}
		if(g_entities[i].client->pers.ready)
		{
			ready = "^7<-^1Ready";
		}

		trap_SendServerCommand( ent-g_entities, va("print \"[%3d   %25s ^7] %s\n\"", i, g_entities[i].client->pers.cleanName, ready));
	}
	trap_SendServerCommand( ent-g_entities, va("print \"^3**Press Pgup(page up) and Pgdn(page down) keys to scroll up and down.**\n\""));
}

/*
==================
RPM_lockTeam
==================
*/
void RPM_lockTeam(gentity_t *ent, qboolean referee, char *team)
{
	///RxCxW - 1.14.2005 - #compMode check - dont need
	///if (!g_compMode.integer)
	///{
	///	trap_SendServerCommand( ent-g_entities, "print \"Competition mode is not enabled on this server.\n\"");
	///	return;
	///}
	///End - 1.14.2005

	if(referee)
	{
		if (team[0] == 'r' || team[0] == 'R')
		{
			if(level.redLocked)
			{
				level.redLocked = 0;
				///RxCxW - 09.18.06 - 04:47pm #lock,unlock
				///trap_SendServerCommand(-1, va("cp \"^3Referee ^7has UNLOCKED the ^1Red ^7team\n\""));
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i, ^3%s has UNLOCKED the ^1Red ^7team", level.time + 5000, ent->client->pers.netname ) );
				///End  - 09.18.06 - 04:47pm
			}
			else
			{
				level.redLocked = 1;
				///RxCxW - 09.18.06 - 04:47pm #lock,unlock
				///trap_SendServerCommand(-1, va("cp \"^3Referee ^7has LOCKED the ^1Red ^7team\n\""));
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i, ^3%s has LOCKED the ^1Red ^7team", level.time + 5000, ent->client->pers.netname ) );
				///End  - 09.18.06 - 04:47pm
			}
		}
		else if(team[0] == 'b'|| team[0] == 'B')
		{
			if(level.blueLocked)
			{
				level.blueLocked = 0;
				///RxCxW - 09.18.06 - 04:47pm #lock,unlock
				///trap_SendServerCommand(-1, va("cp \"^3Referee ^7has UNLOCKED the ^4Blue ^7team\n\""));
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i, ^3%s has UNLOCKED the ^4Blue ^7team", level.time + 5000, ent->client->pers.netname ) );
				///End  - 09.18.06 - 04:47pm
			}
			else
			{
				level.blueLocked = 1;
				///RxCxW - 09.18.06 - 04:47pm #lock,unlock
				///trap_SendServerCommand(-1, va("cp \"^3Referee ^7has LOCKED the ^4Blue ^7team\n\""));
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i, ^3%s has LOCKED the ^4Blue ^7team", level.time + 5000, ent->client->pers.netname ) );
				///End  - 09.18.06 - 04:47pm
			}
		}
		else if(team[0] == 's' || team[0] == 'S')
		{
			if(level.specsLocked)
			{
				level.specsLocked = 0;
				///RxCxW - 09.18.06 - 04:47pm #lock,unlock
				///trap_SendServerCommand(-1, va("cp \"^3Referee ^7has UNLOCKED the Spectators\n\""));
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i, ^3%s ^7has UNLOCKED the ^3Spectators", level.time + 5000, ent->client->pers.netname ) );
				///End  - 09.18.06 - 04:47pm
			}
			else
			{
				level.specsLocked = 1;
				///RxCxW - 09.18.06 - 04:47pm #lock,unlock
				///trap_SendServerCommand(-1, va("cp \"^3Referee ^7has LOCKED the Spectators\n\""));
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i, ^3%s ^7has LOCKED the ^3Spectators", level.time + 5000, ent->client->pers.netname ) );
				///End  - 09.18.06 - 04:47pm
			}
		}
		else
		{
			trap_SendServerCommand(ent - g_entities, va("print \"Invalid Command string.\n\""));
			trap_SendServerCommand(ent - g_entities, va("print \"Usage: ref lock/unlock <team>\n\""));
		}
	}
	else 
	{
	switch (ent->client->sess.team)	{
		case TEAM_RED:
			if(level.redLocked)	{
				level.redLocked = 0;
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i, ^3%s has UNLOCKED the ^1Red ^7team", level.time + 5000, ent->client->pers.netname ) );
				break;
			}
			else {
				level.redLocked = 1;
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i, ^3%s has LOCKED the ^1Red ^7team", level.time + 5000, ent->client->pers.netname ) );
				break;
			}
		case TEAM_BLUE:
			if(level.blueLocked) {
				level.blueLocked = 0;
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i, ^3%s has UNLOCKED the ^4Blue ^7team", level.time + 5000, ent->client->pers.netname ) );
				break;
			}
			else {
				level.blueLocked = 1;
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i, ^3%s has LOCKED the ^4Blue ^7team", level.time + 5000, ent->client->pers.netname ) );
				break;
			}
		}
	}
}
/*
===============
RPM_ReadyTeam
===============
*/
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
=================
RPM_Invite_Spec
=================
*/
void RPM_Invite_Spec(gentity_t *ent, char *arg2)
{
	int		id;

	///RxCxW - 1.14.2005 - #compMode check - dont need
	///if (!g_compMode.integer)
	///{
	///	trap_SendServerCommand( ent-g_entities, "print \"Competition mode is not enabled on this server.\n\"");
	///	return;
	///}
	///End - 1.14.2005

	if (!level.specsLocked)
	{
		trap_SendServerCommand( ent-g_entities, "print \"Spectators are not currently locked.\n\"");
		return;
	}

	if ((ent->client->sess.team != TEAM_RED) && (ent->client->sess.team != TEAM_BLUE) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"Not on a valid team.\n\"");
		return;
	}

	if (arg2[0] >= '0' && arg2[0] <= '9') 
	{
		id = atoi( arg2 );
	}
	else
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Bad client slot: %s\n\"", arg2));
		trap_SendServerCommand( ent-g_entities, va("print \"Usage: tcmd invite/uninvite <idnumber>\n\""));
		return;
	}

	if ( id < 0 || id >= g_maxclients.integer )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Invalid client number %d.\n\"",id));
		return;
	}

	if ( g_entities[id].client->pers.connected == CON_DISCONNECTED )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"There is no client with the client number %d.\n\"", id));
		return;
	}
	if( g_entities[id].client->sess.team != TEAM_SPECTATOR)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"That player is not currently spectating.\n\""));
		return;
	}
	
	if (ent->client->sess.team == TEAM_RED)
	{
		if(g_entities[id].client->sess.invitedByRed)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"%s was Un-invited to spectate your team.\n\"", g_entities[id].client->pers.netname ));
			trap_SendServerCommand( g_entities[id].s.number, va("cp \"You were Un-invited to spectate the ^1Red ^7team\n\""));
			g_entities[id].client->sess.invitedByRed = qfalse;
			return;
		}
		else
		{
			trap_SendServerCommand( ent-g_entities, va("print \"%s was invited to spectate your team.\n\"", g_entities[id].client->pers.netname ));
			trap_SendServerCommand( g_entities[id].s.number, va("cp \"You were invited to spectate the ^1Red ^7team\n\""));
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
			trap_SendServerCommand( ent-g_entities, va("print \"%s was Un-invited to spectate your team.\n\"", g_entities[id].client->pers.netname ));
			trap_SendServerCommand( g_entities[id].s.number, va("cp \"You were Un-invited to spectate the ^4Blue ^7team\n\""));
			g_entities[id].client->sess.invitedByBlue = qfalse;
			return;
		}
		else
		{
			trap_SendServerCommand( ent-g_entities, va("print \"%s was invited to spectate your team.\n\"", g_entities[id].client->pers.netname ));
			trap_SendServerCommand( g_entities[id].s.number, va("cp \"You were invited to spectate the ^4Blue ^7team\n\""));
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
*/

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
==============
RPM_Timeout
==============
*/

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
/*
=============
RPM_Timein
=============
*/
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

