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
//	int		i;
	char	arg1[MAX_STRING_TOKENS];
	char	arg2[MAX_STRING_TOKENS];
//	char	arg3[MAX_STRING_TOKENS];
//	gentity_t *tent;

	trap_Argv( 1, arg1, sizeof( arg1 ) );
	trap_Argv( 2, arg2, sizeof( arg2 ) );
//	if (!Q_stricmp ( arg1, "?" ))
	if (!Q_stricmp ( arg1, "?" )||!Q_stricmp ( arg1, "" ))
	{
		trap_SendServerCommand( ent-g_entities, va("print \" \n ^3Lvl   Commands         Arguments     Explanation\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" ----------------------------------------------------------\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" [^3R^7]       password                   ^7[^3Enter the Referee password^7]\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" [^3R^7]   i   info         <team>        ^7[^3Info of the specified team^7]\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" [^3R^7]   rt  ready                      ^7[^3Ready up the teams^7]\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" [^3R^7]   l   lock         <team>        ^7[^3Lock a team^7]\n\""));
		// Boe: Hieronder mot nog.. Updated soonish.
		trap_SendServerCommand( ent-g_entities, va("print \" [^3R^7]   re  reset        <team>        ^7[^3Reset a team their settings^7]\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" [^3R^7]   k   kick         <id> <reason> ^7[^3Kick a player^7]\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" [^3R^7]   rch chat         <message>     ^7[^3Sends a message to Referees^7]\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" [^3R^7]   rt  reftalk      <message>     ^7[^3Sends a message to all players^7]\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" [^3R^7]   pa  pause                      ^7[^3Pauses/resumes the game^7]\n\""));
		return;
	}
	
	/*if(!g_enableRefs.integer)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Referee's are disabled on this server\n\""));
		return;
	}*/
	
	if (/*g_enableRefs.integer == 1 && */!g_compMode.integer) {
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Server must be in Competetion Mode to become a Referee.\n\""));
		return;
	}


	if (!Q_stricmp ( arg1, "password" ))
	{
		if(ent->client->sess.referee)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You are already a Referee.\n\""));
			return;
		}

		if(ent->client->sess.admin)
		{	
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You are already an Admin.\n\""));
			//trap_SendServerCommand( ent-g_entities, va("print \"Type ^1\\adm suspend ^7to become a referee.\n\""));
			return;
		}

		if(!Q_stricmp ( g_refpassword.string, "none" )||!Q_stricmp ( g_refpassword.string, "\0" ))
		{
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7This server does not have a Referee password.\n\""));
			return;
		}
		if(!Q_stricmp ( g_refpassword.string, arg2 ))
		{
			ent->client->sess.referee = 1;
			//trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,^3%s is now a ^3Referee", level.time + 5000, ent->client->pers.netname ) );
			trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s is now a %sR%se%sf%se%sr%see", level.time + 5000, ent->client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
			trap_SendServerCommand(-1, va("print\"^3[Referee Action] ^7%s ^7is now a Referee.\n\"", ent->client->pers.netname));
			return;
		}
		else
		{
			trap_SendServerCommand( ent-g_entities, va("print \"[^3Info] ^7Incorrect Referee password entered.\n\""));
			return;
		}
	}
}
