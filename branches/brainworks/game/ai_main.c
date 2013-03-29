// Copyright (C) 2013 - Boe!Man.

// Some portions:
// Copyright (C) 2001-2002 Raven Software
// Copyright (C) 2002-2007 Ted Vessenes

// ai_main.c - The AI front-end.

#include "ai_main.h"
#include "ai_vars.h"

/*
==========
BotAISetup
==========
*/
int BotAISetup(int restart)
{
	// Reset the time tracking variables
	server_time_ms = level.time;
	server_time = server_time_ms * 0.001;
	ai_time_ms = level.time;
	ai_time = ai_time_ms * 0.001;

	// Reload all of the variables associated with the AI
	BotAIVariableSetup();

	// Reset the motion tracking system
	//BotAIMotionReset();

	// Initialize player areas
	//LevelPlayerAreasReset();

	// Boe!Man 3/29/13: Don't reset most data when restarting for competition mode or a map restart.
	if (restart)
		return (qboolean)qtrue;

	// Initialize the bot states
	memset(bot_states, 0, sizeof(bot_states));

	// Initialize the AI library
	return (LevelLibrarySetup() == BLERR_NOERROR);
}
