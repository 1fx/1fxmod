// Copyright (C) 2009-2017 - Boe!Man, Henkie.
//
// g_vote.h - All local vote function definitions are defined here.
// Only included by g_vote.c

//==================================================================

#ifndef _G_VOTE_H
#define _G_VOTE_H

static qboolean vote_mapRestart         ( gentity_t *ent );
static qboolean vote_mapCycle           ( gentity_t *ent );
static qboolean vote_map                ( gentity_t *ent );
static qboolean vote_rmgMap             ( gentity_t *ent );
static qboolean vote_gameType           ( gentity_t *ent );
static qboolean vote_kick               ( gentity_t *ent );
static qboolean vote_clientKick         ( gentity_t *ent );
static qboolean vote_doWarmup           ( gentity_t *ent );
static qboolean vote_toggleIntegerCVAR  ( gentity_t *ent );
static qboolean vote_timelimitExt       ( gentity_t *ent );
static qboolean vote_endMap             ( gentity_t *ent );
static qboolean vote_shuffleTeams       ( gentity_t *ent );
static qboolean vote_swapTeams          ( gentity_t *ent );
static qboolean vote_mute               ( gentity_t *ent );
static qboolean vote_clanVsAll          ( gentity_t *ent );
static qboolean vote_poll               ( gentity_t *ent );

static int      vote_isDisabled         ( const char* callvote );

typedef struct{
    char        *voteCmd;   // Full vote command.
    qboolean    (*voteFunc) (gentity_t *ent);
                            // Store pointer to the given function so
                            // we can call it later.
    char        *desc;      // Description of the command in '/callvote ?'.
    char        *params;    // Additional parameters printed in '/callvote ?'.
}voteCmd_t;

static voteCmd_t VoteCommands[] =
{
    {"maprestart",      &vote_mapRestart,
        "Restart current map",                  ""},
    {"mapcycle",        &vote_mapCycle,
        "Go to next map in mapcycle",           ""},
    {"map",             &vote_map,
        "Switch server to the specified map",   "<mapname>"},
    {"rmgmap",          &vote_rmgMap,
        "Generate RMG map",                     "size loc time seed"},
    {"gametype",        &vote_gameType,
        "Load a gametype",                      "<gametype>"},
    {"kick",            &vote_kick,
        "Kick a player",                        "<id/name>"},
    {"clientkick",      &vote_clientKick,
        "Kick a player",                        "<id>"},
    {"dowarmup",        &vote_doWarmup,
        "Enable/disable warmup",                "<0 or 1>"},
    {"timelimit",       &vote_toggleIntegerCVAR,
        "Change the timelimit",                 "<time>"},
    {"timeextension",   &vote_timelimitExt,
        "Extends the timelimit",                ""},
    {"scorelimit",      &vote_toggleIntegerCVAR,
        "Change the scorelimit",                "<time>"},
    {"endmap",          &vote_endMap,
        "End the current map",                  ""},
    {"shuffleteams",    &vote_shuffleTeams,
        "Shuffles the teams",                   ""},
    {"swapteams",       &vote_swapTeams,
        "Swaps the teams",                      ""},
    {"mute",            &vote_mute,
        "Mute/unmute a player",                 "<id/name> <time>"},
    {"clanvsall",       &vote_clanVsAll,
        "Clan versus other players-mode",       "<team>"},
    {"poll",            &vote_poll,
        "Starts a poll",                        "<poll question>"},
};

static int voteCmdSize = sizeof(VoteCommands) / sizeof(VoteCommands[0]);

#endif // _G_VOTE_H
