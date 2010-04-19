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

void Cmd_Say_f( gentity_t *ent, int mode, qboolean arg0 );

/*
=============
RPM_ref_cmd
=============
*/
void RPM_ref_cmd( gentity_t *ent)
{
	int		i;
	char	arg1[MAX_STRING_TOKENS];
	char	arg2[MAX_STRING_TOKENS];
	char	arg3[MAX_STRING_TOKENS];
	gentity_t *tent;

	trap_Argv( 1, arg1, sizeof( arg1 ) );
	trap_Argv( 2, arg2, sizeof( arg2 ) );
//	if (!Q_stricmp ( arg1, "?" ))
	if (!Q_stricmp ( arg1, "?" )||!Q_stricmp ( arg1, "" ))
	{
		trap_SendServerCommand( ent-g_entities, va("print \"\n [^3Referee commands^7]\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" ** All commands must start with a ^3/ref ^7in front of them! **\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" [^4password      ^7use the password to become a referee                        ]\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" [^4info <team>   ^7displays the specified teams info (whos ready, locked etc..)]\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" [^4ready <team>  ^7readys up the specified team so you can start the match     ]\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" [^4lock <team>   ^7locks the specified team so nobody else can join it         ]\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" [^4unlock <team> ^7unlocks the specified team to allow people to join it       ]\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" [^4reset <team>  ^7resets the specified teams settings (ready status etc...)   ]\n\""));
		//if(sc_allowRefKick.integer)
			trap_SendServerCommand( ent-g_entities, va("print \" [^4kick    id#   ^7kicks the player id#                                        ]\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" [^4chat          ^7sends a chat message to referees only                       ]\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" [^4talk          ^7sends a referee message to all players                      ]\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" [^4timeout       ^7pauses the game for a timeout                               ]\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" [^4timein        ^7starts the match from a timeout                             ]\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" ^3** Press PgUp and PgDn keys to scroll up and down the list in console! **\n\""));
		return;
	}
	
	/*if(!g_enableRefs.integer)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Referee's are disabled on this server\n\""));
		return;
	}*/
	
	if (/*g_enableRefs.integer == 1 && */!g_compMode.integer) {
		trap_SendServerCommand( ent-g_entities, va("print \"Server must be in Competetion Mode to be a Referee\n\""));
		return;
	}


	if (!Q_stricmp ( arg1, "password" ))
	{
		if(ent->client->sess.referee)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"You are already a referee.\n\""));
			return;
		}

		if(ent->client->sess.admin)
		{	
			trap_SendServerCommand( ent-g_entities, va("print \"You are already an ^6Admin.\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"Type ^1\\adm suspend ^7to become a referee.\n\""));
			return;
		}

		if(!Q_stricmp ( g_refpassword.string, "none" )||!Q_stricmp ( g_refpassword.string, "\0" ))
		{
			trap_SendServerCommand( ent-g_entities, va("print \"This server does not have a referee password.\n\""));
			return;
		}
		if(!Q_stricmp ( g_refpassword.string, arg2 ))
		{
			ent->client->sess.referee = 1;
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,^3%s is now a ^3Referee", level.time + 5000, ent->client->pers.netname ) );
			return;
		}
		else
		{
			trap_SendServerCommand( ent-g_entities, va("print \"Incorrect Referee password.\n\""));
			return;
		}
	}
	//RxCxW - #Admin uses #ref commands - //03.20.05 - 01.40pm
	////Fixed the bug where non-admin refs couldnt use there powers.
	////if(!ent->client->sess.referee )
	if(!ent->client->sess.referee && !ent->client->sess.admin)
	//End  - 03.20.05 - 01:40pm
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You are not currently a referee.\n\""));
		return;
	}

	/*if (!Q_stricmp ( arg1, "chat" ))
	{
		if(ent->client->sess.mute)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"You are currently muted by admin.\n\"") );
			return;
		}
		for(i = 0; i < level.maxclients; i++)
		{
			tent = &g_entities[i];
			if (!tent->inuse)
			{
				continue;
			}
			if(tent->client->sess.admin || tent->client->sess.referee)
			{
				Boe_GlobalSound(tent, G_SoundIndex("sound/misc/c4/beep.mp3"));
			}
		}
		Cmd_Say_f (ent, REF_CHAT, qfalse);
	}
	else */if (!Q_stricmp ( arg1, "talk" ))
	{
		if(ent->client->sess.mute){
		trap_SendServerCommand( ent-g_entities, va("print \"You are currently muted by admin.\n\"") );
		return;
		}
		Boe_GlobalSound(G_SoundIndex("sound/misc/c4/beep.mp3"));
		/*tent = G_TempEntity( vec3_origin, EV_GLOBAL_SOUND );
		tent->s.eventParm = G_SoundIndex("sound/misc/c4/beep.mp3");
		tent->r.svFlags = SVF_BROADCAST;*/
		Cmd_Say_f (ent, REF_TALK, qfalse);
	}
	else if (!Q_stricmp ( arg1, "kick" ) /*&& sc_allowRefKick.integer*/) {
		int id;

		id = Boe_ClientNumFromArg(ent, 2, "kick <idnumber> <reason>", "Kick", qfalse, qfalse);

		if(id < 0)
		{
			return;
		}

		trap_Argv( 3, arg3, sizeof( arg3 ) );
		trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"%s\"\n", id, arg3));
		return;
	}

	else if (!Q_stricmp ( arg1, "ready" ))
	{
		if(level.warmupTime == 0 || g_doWarmup.integer != 2)
		{
			trap_SendServerCommand(ent - g_entities, va("print \"Not currently in Ready-Up mode.\n\""));
			return;
		}
		if( !level.gametypeData->teams || arg2[0] == '\0')
		{
			//RPM_ReadyAll();
			trap_SendServerCommand( -1, va("cp \"Readied by ^3Referee\n\""));
			return;
		}
		else
		{
			//RPM_ReadyTeam(ent, qtrue, arg2);
		}
	}
	else if (!Q_stricmp ( arg1, "reset" ))
	{
		if(!level.gametypeData->teams || arg2[0] == '\0')
		{
			RPM_Team_Reset(ent, qtrue, "all");
		}
		else
		{
			RPM_Team_Reset(ent, qtrue, arg2);
		}
	}
	else if (!Q_stricmp ( arg1, "timeout" ))
	{
		RPM_Timeout(ent, qtrue);
	}
	else if (!Q_stricmp ( arg1, "timein" ))
	{
		RPM_Timein(ent);
	}
	else if (!Q_stricmp ( arg1, "info" ))
	{
		if ( !level.gametypeData->teams )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"Not playing a team game!\n\""));
			return;
		}
		RPM_TeamInfo(ent, arg2);
	}
	else if ( (!Q_stricmp ( arg1, "lock" )) || (!Q_stricmp ( arg1, "unlock" )) )
	{
		if ( !level.gametypeData->teams )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"Not playing a team game!\n\""));
			return;
		}
		RPM_lockTeam(ent, qtrue, arg2);
	}
	else 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Unknown Command  %s.\n\"", arg1));
		trap_SendServerCommand( ent-g_entities, va("print \"Usage: ref <command> <variable>\n\""));
	}	
}
