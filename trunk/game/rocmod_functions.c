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

#endif // _GOLD
