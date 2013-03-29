// Copyright (C) 2013 - Boe!Man.

// Some portions:
// Copyright (C) 2001-2002 Raven Software
// Copyright (C) 2002-2007 Ted Vessenes

// ai_vars.c - The bot variables.

#include "ai_main.h"
#include "ai_vars.h"

// NOTE: These variables do not use cvars because cvars can be updated during
// gameplay.  These variables are fixed whenever the game is reloaded.
int gametype;
int game_style;
int maxclients;

// Bot states
bot_state_t	*bot_states[MAX_CLIENTS];

// Timestamp of the last executed server frame in seconds and milliseconds
//
// NOTE: server_time_ms is just a copy of level.time.  It's only purpose is
// to standardize the time interface, at least as far as the AI code is concerned.
float server_time;
int server_time_ms;

// Timestamps for the current AI frame in seconds and milliseconds
//
// NOTE: These timestamps are only loosely related to other timestamps used
// in the code.  Avoid using these whenever possible.
float ai_time;
int ai_time_ms;


// Default bot skill
vmCvar_t g_spSkill;

// Variables for internal information
vmCvar_t bot_thinktime;
vmCvar_t bot_memorydump;
vmCvar_t bot_saveroutingcache;

// Variables generating extra information
vmCvar_t bot_report;
vmCvar_t bot_testsolid;
vmCvar_t bot_testclusters;

// Variables modifying chat behavior
vmCvar_t bot_fastchat;
vmCvar_t bot_nochat;
vmCvar_t bot_testrchat;

// Variables modifying movement behavior
vmCvar_t bot_grapple;					// False if bots should never grapple
vmCvar_t bot_rocketjump;				// False if bots should never rocket jump

// Variables modifying dodging
vmCvar_t bot_dodge_rate;				// Percent of time to spend dodging when going somewhere and being attacked
vmCvar_t bot_dodge_min;					// Minimum amount of time to continue dodging in one direction
vmCvar_t bot_dodge_max;					// Maximum amount of time to continue dodging in one direction

// Variables modifying perception
vmCvar_t bot_lag_min;					// The minimum amount of lag a bot can have relative to other targets

// Variables modifying item selection
vmCvar_t bot_item_path_neighbor_weight;	// The weighting between starting and ending regions when computing path
										// neighbors of an item cluster.  0.0 means only consider the start; 1.0
										// means only consider the end.
vmCvar_t bot_item_predict_time_min;		// The bot will predict for at least this many seconds in its final
										// location when considering a set of item pickups
vmCvar_t bot_item_change_penalty_time;	// Estimate item pickup will take this much extra time when selecting
										// a different item from last frame.  (Changing movement direction
										// requires extra acceleration and deceleration the travel time estimates
vmCvar_t bot_item_change_penalty_factor;// Only select a new cluster if it's this many times as valuable
										// as the currently selected cluster
vmCvar_t bot_item_autopickup_time;		// Always pickup any item this many seconds or closer


// Variables modifying awareness
vmCvar_t bot_aware_duration;			// How many seconds the most aware bot remains aware of things
vmCvar_t bot_aware_skill_factor;		// The least aware bot's awareness is this many times as good as the best
vmCvar_t bot_aware_refresh_factor;		// Bot may be this many times further away from a target it's already
										// aware of and still refresh its awareness.

// Variables modifying reaction time
vmCvar_t bot_reaction_min;				// The fastest a bot will start reacting to a change
vmCvar_t bot_reaction_max;				// The slowest a bot will start reacting to a change

// Variables modifying the focus of the bot's view
vmCvar_t bot_view_focus_head_dist;		// Bot focuses on the heads of player targets closer than this distance
vmCvar_t bot_view_focus_body_dist;		// Bot focuses on the bodies of player targets farther than this distance

// Variables modifying the ideal view state's behavior
// NOTE: Changes in bot_view_ideal_error_min/max don't seem to have much effect
vmCvar_t bot_view_ideal_error_min;		// Minimum ideal view error value as a percentage of target's velocity
vmCvar_t bot_view_ideal_error_max;		// Maximum ideal view error value as a percentage of target's velocity
vmCvar_t bot_view_ideal_correct_factor;	// Multiplied by bot's reaction time to produce time to delay between ideal view corrections

// Variables modifying the actual view state's behavior
vmCvar_t bot_view_actual_accel_min;		// Minimum actual view acceleration in degrees per second
vmCvar_t bot_view_actual_accel_max;		// Maximum actual view acceleration in degrees per second
vmCvar_t bot_view_actual_error_min;		// Minimum actual view error value as a percentage of velocity change
vmCvar_t bot_view_actual_error_max;		// Maximum actual view error value as a percentage of velocity change
vmCvar_t bot_view_actual_correct_factor;// Multiplied by bot's reaction time to produce time to delay between actual view corrections

// Variables defining how the bot attacks
vmCvar_t bot_attack_careless_reload;	// Bots are careless when firing weapons with reload times no greater than this value
vmCvar_t bot_attack_careless_factor;	// Bots scale targets' bounding boxes by this percent when aiming carelessly
vmCvar_t bot_attack_careful_factor_min;	// The best bots scale targets' bounding boxes by this percent when aiming carefully
vmCvar_t bot_attack_careful_factor_max;	// The worst bots scale targets' bounding boxes by this percent when aiming carefully
vmCvar_t bot_attack_continue_factor;	// Once a bot stops attacking, it continues firing for this many times their reaction time
vmCvar_t bot_attack_lead_time_full;		// Bots will lead the full distance when the amount of time they need to lead is no more than this
vmCvar_t bot_attack_lead_time_scale;	// The percentage of time beyond bot_attack_lead_time_full that the bot actually leads

/*
==================
BotAIVariableSetup
==================
*/
void BotAIVariableSetup(void)
{
	trap_Cvar_Register(&bot_thinktime, "bot_thinktime", "100", CVAR_CHEAT, 0.0, 0.0);
	trap_Cvar_Register(&bot_memorydump, "bot_memorydump", "0", CVAR_CHEAT, 0.0, 0.0);
	trap_Cvar_Register(&bot_saveroutingcache, "bot_saveroutingcache", "0", CVAR_CHEAT, 0.0, 0.0);
	trap_Cvar_Register(&bot_report, "bot_report", "0", CVAR_CHEAT, 0.0, 0.0);
	trap_Cvar_Register(&bot_testsolid, "bot_testsolid", "0", CVAR_CHEAT, 0.0, 0.0);
	trap_Cvar_Register(&bot_testclusters, "bot_testclusters", "0", CVAR_CHEAT, 0.0, 0.0);
}
