// Copyright (C) 2015 - Boe!Man, Henkie.
//
// rocmod_functions.c - Functions specific for ROCmod.

//==================================================================

#ifdef _GOLD
#include "g_local.h"
#include "boe_local.h"

/*
=============
ROCmod_verifyClient
=============
*/

void ROCmod_verifyClient(gentity_t *ent, int clientNum)
{
	char	buffer[MAX_TOKEN_CHARS];
	int		value;

	if (trap_Argc() == 1)
		return;

	trap_Argv(1, buffer, sizeof(buffer));
	value = atoi(buffer);

	if (value < 2012) // 2.1c
		return;

	#ifdef _DEBUG
	Com_Printf("ROCmod 2.1c verified on client #%d\n", clientNum);
	#endif // _DEBUG
	ent->client->sess.rocModClient = qtrue;
	strncpy(ent->client->sess.strClient, "2.1c", sizeof(ent->client->sess.strClient));
}

/*
==================
ROCmod_clientUpdate
==================
*/
void ROCmod_clientUpdate(gentity_t *ent, int clientNum)
{
	char	buffer[MAX_TOKEN_CHARS];

	if (trap_Argc() == 1)
		return;

	trap_Argv(1, buffer, sizeof(buffer));
	ent->client->sess.rocExtraFeatures = atoi(buffer);

	trap_SendServerCommand(ent - g_entities, va("efack %i", ent->client->sess.rocExtraFeatures));
	if (ent->client->sess.rocExtraFeatures & ROC_TEAMINFO)
	{
		ROCmod_sendExtraTeamInfo(ent);
	}
}

/*
==================
ROCmod_sendExtraTeamInfo
==================
*/
void ROCmod_sendExtraTeamInfo(gentity_t *ent)
{
	char		entry[64];
	char		red[1024];
	char		blue[1024];
	int			redlength;
	int			bluelength;
	int			entrylength;
	int			i, j;
	int			redcount;
	int			bluecount;
	gclient_t	*cl;
	gentity_t	*loc;

	// Don't update them if they don't want it
	if (ent && !(ent->client->sess.rocExtraFeatures & ROC_TEAMINFO))
		return;

	// send the latest information on all clients
	red[0] = 0;
	redlength = 0;
	redcount = 0;
	blue[0] = 0;
	bluelength = 0;
	bluecount = 0;

	for (i = 0; i < level.numConnectedClients; i++)
	{
		cl = &level.clients[level.sortedClients[i]];

		if (cl->pers.connected != CON_CONNECTED || cl->ps.stats[STAT_HEALTH] <= 0)
			continue;

		Com_sprintf(entry, sizeof(entry), " %i", level.sortedClients[i]);
		entrylength = strlen(entry);
		
		// Get location.
		loc = Team_GetLocation(&g_entities[level.sortedClients[i]]);

		// Add all extra information from the client.
		Com_sprintf(entry + entrylength, sizeof(entry) - entrylength, "h%i a%i w%i l%i g%i", 
			cl->ps.stats[STAT_HEALTH],
			cl->ps.stats[STAT_ARMOR],
			g_entities[level.sortedClients[i]].s.weapon,
			(loc != NULL) ? loc->health : -1,
			cl->sess.ghost);
		entrylength = strlen(entry);

		j = strlen(entry);
		if (cl->sess.team == TEAM_RED)
		{
			if (redlength + j > 1022)
				break;

			strcpy(red + redlength, entry);
			redlength += j;
			redcount++;
		}
		else if (cl->sess.team == TEAM_BLUE)
		{
			if (bluelength + j > 1022)
				break;

			strcpy(blue + bluelength, entry);
			bluelength += j;
			bluecount++;
		}
	}

	if (ent)
	{
		if (ent->client->sess.team == TEAM_RED)
			trap_SendServerCommand(ent - g_entities, va("eti2 %i%s", redcount, red));
		else if (ent->client->sess.team == TEAM_BLUE)
			trap_SendServerCommand(ent - g_entities, va("eti2 %i%s", bluecount, blue));
	}
	else
	{
		for (i = 0; i < level.numConnectedClients; i++)
		{
			cl = &level.clients[level.sortedClients[i]];

			if (cl->pers.connected != CON_CONNECTED)
				continue;

			if (cl->sess.rocExtraFeatures & ROC_TEAMINFO) {
				if (cl->sess.team == TEAM_RED && redcount)
					trap_SendServerCommand(level.sortedClients[i], va("eti2 %i%s", redcount, red));
				else if (cl->sess.team == TEAM_BLUE && bluecount)
					trap_SendServerCommand(level.sortedClients[i], va("eti2 %i%s", bluecount, blue));
			}
		}
	}
}
#endif // _GOLD
