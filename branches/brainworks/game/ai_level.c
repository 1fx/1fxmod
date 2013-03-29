// Copyright (C) 2013 - Boe!Man.

// Some portions:
// Copyright (C) 2001-2002 Raven Software
// Copyright (C) 2002-2007 Ted Vessenes

// ai_level.c - Functions that the bot uses to get information regarding the current level.

#include "ai_main.h"
#include "ai_level.h"

/*
=================
LevelLibrarySetup

Initialize a ton of random data.  Returns
true if the AAS was setup properly and false
if an error occurred.
=================
*/
qboolean LevelLibrarySetup(void)
{
	char buf[MAX_CHARACTERISTIC_PATH];

	// Set the maxclients and maxentities library variables before calling BotSetupLibrary
	trap_Cvar_VariableStringBuffer("sv_maxclients", buf, sizeof(buf));
	if (!strlen(buf)) strcpy(buf, "8");
	trap_BotLibVarSet("maxclients", buf);
	Com_sprintf(buf, sizeof(buf), "%d", MAX_GENTITIES);
	trap_BotLibVarSet("maxentities", buf);

	// Bsp checksum
	trap_Cvar_VariableStringBuffer("sv_mapChecksum", buf, sizeof(buf));
	if (strlen(buf)) trap_BotLibVarSet("sv_mapChecksum", buf);

	// Maximum number of aas links
	trap_Cvar_VariableStringBuffer("max_aaslinks", buf, sizeof(buf));
	if (strlen(buf)) trap_BotLibVarSet("max_aaslinks", buf);

	// Maximum number of items in a level
	trap_Cvar_VariableStringBuffer("max_levelitems", buf, sizeof(buf));
	if (strlen(buf)) trap_BotLibVarSet("max_levelitems", buf);

	// Game type
	trap_Cvar_VariableStringBuffer("g_gametype", buf, sizeof(buf));
	if (!strlen(buf)) strcpy(buf, "0");
	trap_BotLibVarSet("g_gametype", buf);

	// Bot developer mode and log file
	trap_BotLibVarSet("bot_developer", "0");
	trap_BotLibVarSet("log", buf);

	// No chatting
	trap_Cvar_VariableStringBuffer("bot_nochat", buf, sizeof(buf));
	if (strlen(buf)) trap_BotLibVarSet("nochat", "0");

	// Visualize jump pads
	trap_Cvar_VariableStringBuffer("bot_visualizejumppads", buf, sizeof(buf));
	if (strlen(buf)) trap_BotLibVarSet("bot_visualizejumppads", buf);

	// Forced clustering calculations
	trap_Cvar_VariableStringBuffer("bot_forceclustering", buf, sizeof(buf));
	if (strlen(buf)) trap_BotLibVarSet("forceclustering", buf);

	// Forced reachability calculations
	trap_Cvar_VariableStringBuffer("bot_forcereachability", buf, sizeof(buf));
	if (strlen(buf)) trap_BotLibVarSet("forcereachability", buf);

	// Force writing of AAS to file
	trap_Cvar_VariableStringBuffer("bot_forcewrite", buf, sizeof(buf));
	if (strlen(buf)) trap_BotLibVarSet("forcewrite", buf);

	// No AAS optimization
	trap_Cvar_VariableStringBuffer("bot_aasoptimize", buf, sizeof(buf));
	if (strlen(buf)) trap_BotLibVarSet("aasoptimize", buf);

	trap_Cvar_VariableStringBuffer("bot_saveroutingcache", buf, sizeof(buf));
	if (strlen(buf)) trap_BotLibVarSet("saveroutingcache", buf);

	// Reload instead of cache bot character files
	trap_Cvar_VariableStringBuffer("bot_reloadcharacters", buf, sizeof(buf));
	if (!strlen(buf)) strcpy(buf, "0");
	trap_BotLibVarSet("bot_reloadcharacters", buf);

	// Base directory
	trap_Cvar_VariableStringBuffer("fs_basepath", buf, sizeof(buf));
	if (strlen(buf)) trap_BotLibVarSet("basedir", buf);

	// Game directory
	trap_Cvar_VariableStringBuffer("fs_game", buf, sizeof(buf));
	if (strlen(buf)) trap_BotLibVarSet("gamedir", buf);

	// Cd directory
	trap_Cvar_VariableStringBuffer("fs_cdpath", buf, sizeof(buf));
	if (strlen(buf)) trap_BotLibVarSet("cddir", buf);

	// Boe!Man 3/29/13: Setup the bot library.
	// This is where the brainworks code links with the engine.
	return trap_BotLibSetup();
}
