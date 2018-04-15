// Copyright (C) 2001-2002 Raven Software.
//

#include "g_local.h"
#include "boe_local.h"

#ifdef __linux__
#include "./tadns/tadns.h"
#include <netdb.h>
#ifndef __USE_POSIX199309
#include <asm/sigcontext.h>
#endif // __USE_POSIX199309

#if (defined(__GNUC__) && __GNUC__ < 3)
unsigned char   memsys5[41943040]; // Boe!Man 1/29/13: Buffer of 40 MB, available for SQLite memory management (Linux).
#endif // GNUC < 3
#elif WIN32
#include <windows.h>
HANDLE lockFile;
#endif

// Boe!Man 6/26/13: The in-memory databases to be used globally across the Mod.
sqlite3         *aliasesDb;
sqlite3         *bansDb;
sqlite3         *usersDb;
qboolean        dbsLoaded = qfalse;
char            tempName[16];

level_locals_t  level;

typedef struct
{
    vmCvar_t    *vmCvar;
    char        *cvarName;
    char        *defaultString;
    int         cvarFlags;
    float       mMinValue, mMaxValue;
    int         modificationCount;  // for tracking changes
    qboolean    trackChange;        // track this variable, and announce if changed
    qboolean    teamShader;         // track and if changed, update shader state

} cvarTable_t;

gentity_t       g_entities[MAX_GENTITIES];
gclient_t       g_clients[MAX_CLIENTS];

vmCvar_t    g_gametype;
vmCvar_t    g_dmflags;
vmCvar_t    g_scorelimit;
vmCvar_t    g_timelimit;
vmCvar_t    g_friendlyFire;
vmCvar_t    g_password;
vmCvar_t    g_needpass;
vmCvar_t    g_maxclients;
vmCvar_t    g_maxGameClients;
vmCvar_t    g_dedicated;
vmCvar_t    g_speed;
vmCvar_t    g_gravity;
vmCvar_t    g_cheats;
vmCvar_t    g_knockback;
vmCvar_t    g_forcerespawn;
vmCvar_t    g_inactivity;
vmCvar_t    g_debugMove;
vmCvar_t    g_debugDamage;
vmCvar_t    g_debugAlloc;
vmCvar_t    g_weaponRespawn;
vmCvar_t    g_backpackRespawn;
vmCvar_t    g_motd;
vmCvar_t    g_synchronousClients;
vmCvar_t    g_warmup;
vmCvar_t    g_doWarmup;
vmCvar_t    g_restarted;
vmCvar_t    current_gametype;
vmCvar_t    g_clientMod;
vmCvar_t    g_rpmEnt;
vmCvar_t    g_passwordAdmins;
vmCvar_t    g_boxAttempts;
vmCvar_t    g_cageAttempts;
vmCvar_t    g_RpgStyle;
vmCvar_t    g_log;
vmCvar_t    g_logSync;
vmCvar_t    g_logHits;
vmCvar_t    g_logExtra;
vmCvar_t    g_allowVote;
vmCvar_t    g_voteDuration;
vmCvar_t    g_failedVoteDelay;
vmCvar_t    g_callvote;
vmCvar_t    g_teamAutoJoin;
vmCvar_t    g_teamForceBalance;
vmCvar_t    g_banIPs;
vmCvar_t    g_filterBan;
vmCvar_t    g_smoothClients;
vmCvar_t    pmove_fixed;
vmCvar_t    pmove_msec;
vmCvar_t    g_rankings;
vmCvar_t    g_listEntity;
vmCvar_t    g_fps;
vmCvar_t    g_enableHash;
vmCvar_t    g_respawnInterval;
vmCvar_t    g_respawnInvulnerability;
vmCvar_t    g_roundtimelimit;
vmCvar_t    g_timeextension;
#ifdef _DEMO
vmCvar_t    g_timeextensionmultiplier;
#endif // _DEMO
vmCvar_t    g_timeouttospec;
vmCvar_t    g_roundstartdelay;
vmCvar_t    hideSeek_roundstartdelay;
#ifdef _DEMO
vmCvar_t    g_availableWeaponsOutfitting;
#endif // _DEMO
vmCvar_t    g_availableWeapons;
vmCvar_t    hideSeek_availableWeapons;
vmCvar_t    availableWeapons;
vmCvar_t    g_forceFollow;
vmCvar_t    g_followEnemy;
vmCvar_t    g_mapcycle;
vmCvar_t    g_pickupsDisabled;              // Whether or not pickups are available in a map (uses outfitting if not)
vmCvar_t    g_enableM203;                   // Whether or not M203 grenades are enabled on the M4.
vmCvar_t    g_suicidePenalty;               // Amount of score added for killing yourself (typically negative)
vmCvar_t    g_teamkillPenalty;              // Amount of score added for killing a teammates (typically negative)
#ifndef _DEMO
vmCvar_t    g_teamkillDamageMax;            // max damage one can do to teammates before being kicked
#endif // not _DEMO
vmCvar_t    g_teamkillDamageForgive;        // amount of teamkill damage forgiven each minute
vmCvar_t    g_voiceFloodCount;              // Number of voice messages in one minute to be concidered flooding
vmCvar_t    g_voiceFloodPenalty;            // Amount of time a void flooder must wait before they can use voice again
vmCvar_t    g_ctfClassic;                   // Boe!Man 2/1/13: Setting this to 1 enables classic CTF mode (touch flag = return).
vmCvar_t    g_tdmUseTeamSpawns;

vmCvar_t    RMG;
vmCvar_t    g_debugRMG;

// Boe!Man 3/30/10
vmCvar_t    server_motd1;                   // Server message 1-6.
vmCvar_t    server_motd2;                   // Used for displaying the Motd when the player connects,
vmCvar_t    server_motd3;                   // or decides to view it using the /motd command.
vmCvar_t    server_motd4;
vmCvar_t    server_motd5;
vmCvar_t    server_motd6;

vmCvar_t    g_badmin;                       // Adds or removes a B-Admin.
vmCvar_t    g_admin;                        // Adds or removes an Admin.
vmCvar_t    g_sadmin;                       // Adds or removes a S-Admin.
vmCvar_t    g_kick;                         // Admin CVAR.
vmCvar_t    g_subnetban;                    // Admin CVAR.
vmCvar_t    g_ban;                          // Admin CVAR.
vmCvar_t    g_broadcast;
vmCvar_t    server_colors;                  // Broadcast color fades.
vmCvar_t    g_uppercut;                     // Admin CVAR.
vmCvar_t    g_runover;                      // Admin CVAR.
vmCvar_t    g_respawn;                      // Admin CVAR.
vmCvar_t    server_badminprefix;
vmCvar_t    server_adminprefix;
vmCvar_t    server_sadminprefix;
vmCvar_t    server_acprefix;
vmCvar_t    server_scprefix;
vmCvar_t    server_caprefix;
vmCvar_t    server_ccprefix;
vmCvar_t    server_ctprefix;
vmCvar_t    server_starprefix;
vmCvar_t    g_mapswitch;                    // Admin CVAR.
vmCvar_t    g_flash;                        // Admin CVAR.
vmCvar_t    g_plant;                        // Admin CVAR.
vmCvar_t    g_pop;                          // Admin CVAR.
vmCvar_t    g_burn;                         // Admin CVAR.
vmCvar_t    g_mute;                         // Admin CVAR.
vmCvar_t    g_strip;                        // Admin CVAR.
vmCvar_t    g_eventeams;                    // Admin CVAR.
vmCvar_t    g_maxIPConnections;
vmCvar_t    g_noHighFps;
//vmCvar_t  g_333;                          // Admin CVAR.
vmCvar_t    g_forceteam;                    // Admin CVAR.
vmCvar_t    g_adminlist;
vmCvar_t    Owner;
vmCvar_t    Clan;
vmCvar_t    ClanURL;
vmCvar_t    HostedBy;
// Boe!Man 4/1/10
vmCvar_t    server_redteamprefix;
vmCvar_t    server_blueteamprefix;
// Boe!Man 1/19/11
vmCvar_t    server_hiderteamprefix;
vmCvar_t    server_seekerteamprefix;
vmCvar_t    server_humanteamprefix;
vmCvar_t    server_zombieteamprefix;
// Henk 01/04/10 -> Add g_disableNades
vmCvar_t    g_disableNades;
vmCvar_t    g_instaGib;
vmCvar_t    g_weaponswitch;
vmCvar_t    g_compMode;
//vmCvar_t  g_clanfile;
vmCvar_t    g_enableTeamCmds;
vmCvar_t    g_refpassword;
vmCvar_t    g_checkCountry;
// Boe!Man 2/27/11: Some new CVARs for nolower.
vmCvar_t    g_useNoLower;
vmCvar_t    g_useNoRoof;
vmCvar_t    g_useNoMiddle;
vmCvar_t    g_useNoWhole;

// Boe!Man 4/15/10: Some level commands (Admin).
vmCvar_t    g_nosection;
vmCvar_t    g_nades;
vmCvar_t    g_sl;
vmCvar_t    g_tl;
vmCvar_t    g_ri;
vmCvar_t    g_damage;
vmCvar_t    g_gr;
vmCvar_t    g_clanvsall;
vmCvar_t    g_swapteams;
vmCvar_t    g_lock;
vmCvar_t    g_clan;

// Boe!Man 6/2/10
vmCvar_t    g_autoEvenTeams;
vmCvar_t    g_autoSwapTeams;
vmCvar_t    server_msgInterval;
vmCvar_t    server_enableServerMsgs;
vmCvar_t    server_enableTips;
vmCvar_t    server_message1;
vmCvar_t    server_message2;
vmCvar_t    server_message3;
vmCvar_t    server_message4;
vmCvar_t    server_message5;
vmCvar_t    server_msgDelay;

// Boe!Man 10/16/10
vmCvar_t    g_adminSpec;

// Boe!Man 11/16/10: Default scrim settings.
vmCvar_t    g_matchLockSpec;
vmCvar_t    g_matchSwapTeams;
vmCvar_t    g_matchTimeLimit;
vmCvar_t    g_matchScoreLimit;
vmCvar_t    g_matchDisableEvents;
vmCvar_t    g_matchRounds;
vmCvar_t    g_matchDisablePubChat;
vmCvar_t    g_matchDisableTell;

vmCvar_t    cm_enabled;
vmCvar_t    cm_sl;
vmCvar_t    cm_tl;
vmCvar_t    cm_slock;
vmCvar_t    cm_aswap;
vmCvar_t    cm_devents;
vmCvar_t    cm_dr;
vmCvar_t    cm_oldsl;
vmCvar_t    cm_oldtl;
vmCvar_t    cm_sr;
vmCvar_t    cm_sb;

// Boe!Man 12/13/10: Clonecheck CVARs.
vmCvar_t    g_aliasCheck;
vmCvar_t    g_aliasCount;

// Extra map switch CVARs.
vmCvar_t    g_alternateMap;
#ifdef _DEMO
vmCvar_t    g_developerMap;
#endif // _DEMO

// Boe!Man 1/26/11
vmCvar_t    g_cm;
#ifndef _DEMO
vmCvar_t    g_3rd;
#endif // not _DEMO
vmCvar_t    g_enableCustomCommands;

// Boe!Man 2/13/11
vmCvar_t    g_forcevote;

// Boe!Man 3/6/11
vmCvar_t    g_customCommandsFile;
vmCvar_t    g_tipsFile;
//vmCvar_t  g_banfile;
vmCvar_t    hideSeek_Extra;
vmCvar_t    hideSeek_Nades;
vmCvar_t    hideSeek_Weapons;

// Boe!Man 3/8/11
vmCvar_t    g_enableAdminLog;
vmCvar_t    g_enableRconLog;
vmCvar_t    g_adminlog;
vmCvar_t    g_rconlog;
vmCvar_t    g_loginlog;

// Boe!Man 3/16/11
vmCvar_t    server_rconprefix;
vmCvar_t    server_specteamprefix;

// Boe!Man 4/20/11
//vmCvar_t  g_crossTheBridge;

// Boe!Man 5/28/11
vmCvar_t    g_customWeaponFile;

// Boe!man 6/17/11
vmCvar_t    g_pause;

// Boe!Man 8/18/11
vmCvar_t    g_objectiveLocations;

// Boe!Man 8/25/11
vmCvar_t    g_logSpecialChat;
vmCvar_t    g_specialChatSound;

// Boe!Man 5/20/12
vmCvar_t    g_publicIPs;

// Boe!man 6/16/12
vmCvar_t    g_shuffleteams;

// Boe!Man 7/29/12
vmCvar_t    g_preferSubnets;

// Henk 10/30/12
vmCvar_t g_clientDeathMessages;

// Boe!Man 9/2/12
vmCvar_t    hideSeek_ExtendedRoundStats;

// Boe!Man 11/5/12
vmCvar_t    g_ff;
vmCvar_t    g_rename;
vmCvar_t    g_toggleweapon;
vmCvar_t    g_anticamp;
vmCvar_t    g_endmap;

vmCvar_t    g_dosPatch;

// Boe!Mn 12/2/14: Caserun.
vmCvar_t    g_caserun;

// Boe!Man 1/2/13: --- SQLite3 Related CVARs ---
vmCvar_t    sql_aliasFlushCount;
vmCvar_t    sql_timeBench;
vmCvar_t    sql_automaticBackup;
vmCvar_t    g_inviewDb;

#ifdef _3DServer
vmCvar_t    boe_fragWars;
vmCvar_t    boe_deadMonkey;
#endif // _3DServer

#ifndef _DEMO
vmCvar_t    g_allowthirdperson;
#endif // not _DEMO

vmCvar_t    g_enforce1fxAdditions;
vmCvar_t    g_httpRefPaks;
vmCvar_t    g_httpBaseURL;
vmCvar_t    g_httpMaxSpeed;
vmCvar_t    g_recoilRatio;
vmCvar_t    g_inaccuracyRatio;

vmCvar_t    g_minRate;

// Boe!Man 11/26/15: --- Anticamp CVARs, from RPM 0.75.x ---
vmCvar_t    g_camperAllowTime;
vmCvar_t    g_camperPunish;
vmCvar_t    g_camperRadius;
vmCvar_t    g_camperSniper;
vmCvar_t    g_camperPunishment;

vmCvar_t    g_enforceArenaCheck;
vmCvar_t    g_checkTimeRemaining;

// Boe!Man 3/19/18
vmCvar_t    g_mvchatCheckSoundFiles;

static cvarTable_t gameCvarTable[] =
{
    // don't override the cheat state set by the system
    { &g_cheats, "sv_cheats", "", 0, 0.0, 0.0, 0, qfalse },
    { NULL, "^3Mod Name", MODNAME, CVAR_SERVERINFO | CVAR_ROM, 0.0, 0.0, 0, qfalse  },
    { NULL, "^3Mod Version", MODVERSION_NOSUFFIX, CVAR_SERVERINFO | CVAR_ROM, 0.0, 0.0, 0, qfalse  },
    #ifdef MODFLAVORS
    { NULL, "^3Mod Flavor", MODFLAVORS, CVAR_SERVERINFO | CVAR_ROM, 0.0, 0.0, 0, qfalse },
    #endif // MODFLAVORS
    { NULL, "^3Mod URL", "1fxmod.org", CVAR_SERVERINFO | CVAR_ROM, 0.0, 0.0, 0, qfalse  },
    { &g_clientMod, "g_clientMod", "none", CVAR_LATCH | CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },

    // noset vars
    { NULL, "gamename", GAMEVERSION , CVAR_SERVERINFO | CVAR_ROM, 0.0, 0.0, 0, qfalse  },
    { NULL, "gamedate", __DATE__ , CVAR_ROM, 0.0, 0.0, 0, qfalse  },
    { &g_restarted, "g_restarted", "0", CVAR_ROM, 0.0, 0.0, 0, qfalse  },
    { NULL, "sv_mapname", "", CVAR_SERVERINFO | CVAR_ROM, 0.0, 0.0, 0, qfalse  },

    { &g_fps, "sv_fps", "", CVAR_ROM, 0.0, 0.0, 0, qfalse },
    { &g_enableHash, "g_enableHash", "0", CVAR_ROM, 0.0, 0.0, 0, qfalse },

    // latched vars
    { &g_gametype, "g_gametype", "dm", CVAR_SERVERINFO | CVAR_LATCH, 0.0, 0.0, 0, qfalse },

    { &g_maxclients, "sv_maxclients", "8", CVAR_SERVERINFO | CVAR_CHEAT | CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
    { &g_maxGameClients, "g_maxGameClients", "0", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },

    { &g_dmflags, "dmflags", "8", CVAR_SERVERINFO | CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue  },
    #ifndef _DEMO
    { &g_scorelimit, "scorelimit", "0", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0.0, 0.0, 0, qtrue },
    #else
    { &g_scorelimit, "fraglimit",  "0", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0.0, 0.0, 0, qtrue },
    #endif // not _DEMO
    { &g_timelimit,  "timelimit",  "0", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0.0, 0.0, 0, qtrue },

    { &g_minRate, "g_minRate", "0", CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue },

    { &g_respawnInterval, "g_respawnInterval", "15", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
    { &g_respawnInvulnerability, "g_respawnInvulnerability", "5", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },

    { &g_synchronousClients, "g_synchronousClients", "0", CVAR_SYSTEMINFO, 0.0, 0.0, 0, qfalse  },

    { &g_friendlyFire, "g_friendlyFire", "0", CVAR_SERVERINFO|CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue  },

    { &g_teamAutoJoin, "g_teamAutoJoin", "0", CVAR_ARCHIVE, 0.0, 0.0,   },
    { &g_teamForceBalance, "g_teamForceBalance", "1", CVAR_ARCHIVE, 0.0, 0.0,   },

    { &g_warmup, "g_warmup", "20", CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue  },
    { &g_doWarmup, "g_doWarmup", "0", 0, 0.0, 0.0, 0, qtrue  },
    { &g_log, "g_log", "logs/games.log", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
    { &g_logSync, "g_logSync", "0", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
    { &g_logHits, "g_logHits", "0", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
    { &g_logExtra,"g_logExtra","0", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },

    { &g_password, "g_password", "", CVAR_USERINFO, 0.0, 0.0, 0, qfalse  },

    { &g_banIPs, "g_banIPs", "", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
    { &g_filterBan, "g_filterBan", "1", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },

    { &g_needpass, "g_needpass", "0", CVAR_SERVERINFO | CVAR_ROM, 0.0, 0.0, 0, qfalse },

    { &g_dedicated, "dedicated", "0", 0, 0.0, 0.0, 0, qfalse  },

    #if !defined(_GOLD) && !defined(_DEMO)
    { &g_speed, "g_speed", "300", 0, 0.0, 0.0, 0, qtrue  },
    #else
    { &g_speed, "g_speed", "280", 0, 0.0, 0.0, 0, qtrue  },
    #endif // not _GOLD and not _DEMO
    { &g_gravity, "g_gravity", "800", 0, 0.0, 0.0, 0, qtrue  },
    { &g_knockback, "g_knockback", "700", 0, 0.0, 0.0, 0, qtrue  },
    { &g_weaponRespawn, "g_weaponrespawn", "15", 0, 0.0, 0.0, 0, qtrue  },
    { &g_backpackRespawn, "g_backpackrespawn", "40", 0, 0.0, 0.0, 0, qtrue  },
    { &g_forcerespawn, "g_forcerespawn", "0", 0, 0.0, 0.0, 0, qtrue },
    { &g_inactivity, "g_inactivity", "180", CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue },
    { &g_debugMove, "g_debugMove", "0", 0, 0.0, 0.0, 0, qfalse },
    { &g_debugDamage, "g_debugDamage", "0", 0, 0.0, 0.0, 0, qfalse },
    { &g_debugAlloc, "g_debugAlloc", "0", 0, 0.0, 0.0, 0, qfalse },
    { &g_motd, "g_motd", "", 0, 0.0, 0.0, 0, qfalse },

    { &g_allowVote, "g_allowVote", "0", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
    { &g_voteDuration, "g_voteDuration", "60", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
    { &g_failedVoteDelay, "g_failedVoteDelay", "0", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
    { &g_callvote, "g_callvote", "4", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
    { &g_listEntity, "g_listEntity", "0", 0, 0.0, 0.0, 0, qfalse },

    { &g_smoothClients, "g_smoothClients", "1", 0, 0.0, 0.0, 0, qfalse},
    { &pmove_fixed, "pmove_fixed", "0", CVAR_SYSTEMINFO, 0.0, 0.0, 0, qfalse},
    { &pmove_msec, "pmove_msec", "8", CVAR_SYSTEMINFO, 0.0, 0.0, 0, qfalse},

    { &g_rankings, "g_rankings", "0", 0, 0.0, 0.0, 0, qfalse},

    { &g_ctfClassic,        "g_ctfClassic",         "0", CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue },
    { &g_tdmUseTeamSpawns,  "g_tdmUseTeamSpawns",   "0", CVAR_ARCHIVE | CVAR_LATCH, 0.0, 0.0, 0, qtrue },

    { &RMG, "RMG", "0", 0, 0.0, 0.0, },
    { &g_debugRMG, "g_debugRMG", "0", 0, 0.0f, 0.0f },

    { &g_timeouttospec,     "g_timeouttospec",  "15",       CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
    { &g_roundtimelimit,    "g_roundtimelimit", "5",        CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
    { &g_timeextension,     "g_timeextension",  "15",       CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
    #ifdef _DEMO
    { &g_timeextensionmultiplier,       "g_timeextensionmultiplier",    "0", 0, 0.0, 0.0, 0, qfalse },
    #endif // _DEMO

    { &g_roundstartdelay,   "g_roundstartdelay", "3",       CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
    { &hideSeek_roundstartdelay,    "hideSeek_roundstartdelay", "30",       CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },

    #ifdef _GOLD
    { &g_availableWeapons,              "g_available",                  "2222222222211",            CVAR_SERVERINFO | CVAR_INTERNAL | CVAR_ROM, 0.0, 0.0, 0, qfalse },
    { &hideSeek_availableWeapons,       "hideSeek_availableWeapons",    "200000000000000022220000", CVAR_INTERNAL | CVAR_ROM, 0.0, 0.0, 0, qfalse },
    #elif _DEMO
    // Demo has a special "fake" availableWeapons CVAR presented to the players.
    // This CVAR always includes at least one weapon enabled in each category,
    // even if this is not the case in the server.
    { &g_availableWeaponsOutfitting,    "g_availableWeapons",           "2222222222211",            CVAR_SERVERINFO | CVAR_INTERNAL | CVAR_ROM, 0.0, 0.0, 0, qfalse },
    { &g_availableWeapons,              "g_availableWeaponsReal",       "2222222222211",            CVAR_INTERNAL | CVAR_ROM, 0.0, 0.0, 0, qfalse },
    { &hideSeek_availableWeapons,       "hideSeek_availableWeapons",    "200000000000022222222",    CVAR_INTERNAL | CVAR_ROM, 0.0, 0.0, 0, qfalse },
    #else
    // v1.00.
    { &g_availableWeapons,              "g_availableWeapons",           "2222222222211",            CVAR_SERVERINFO | CVAR_INTERNAL | CVAR_ROM, 0.0, 0.0, 0, qfalse },
    { &hideSeek_availableWeapons,       "hideSeek_availableWeapons",    "200000000000022222222",    CVAR_INTERNAL | CVAR_ROM, 0.0, 0.0, 0, qfalse },
    #endif
    { &availableWeapons,                "availableWeapons",             "2222222222211",            CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse },

    #if !defined(_GOLD) && !defined(_DEMO)
    { &g_disableNades,  "g_disableNades", "1", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse },
    #else
    // Nades are enabled by default on any build except v1.00.
    { &g_disableNades,  "g_disableNades", "0", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse },
    #endif // not _GOLD and not _DEMO
    // End
    { &g_forceFollow,       "g_forceFollow",     "1",             CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
    { &g_followEnemy,       "g_followEnemy",     "1",             CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },

    { &g_mapcycle,          "sv_mapcycle",       "none",        CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },

    #ifndef _DEMO
    { &g_pickupsDisabled,   "g_pickupsDisabled", "0",                   CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse },
    #else
    // The client knows whether pickups are enabled or disabled through the server info in demo.
    { &g_pickupsDisabled,   "g_pickupsDisabled", "0",                   CVAR_ARCHIVE|CVAR_LATCH|CVAR_SERVERINFO, 0.0, 0.0, 0, qfalse },
    #endif // not _DEMO
    { &g_enableM203,        "g_enableM203",      "0",                   CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse },

    { &g_suicidePenalty,    "g_suicidePenalty",  "-1",                  CVAR_ARCHIVE,   0.0f,   0.0f,   0,  qfalse },

    { &g_voiceFloodCount,   "g_voiceFloodCount",    "6",                CVAR_ARCHIVE,   0.0f,   0.0f,   0,  qfalse },
    { &g_voiceFloodPenalty, "g_voiceFloodPenalty",  "60",               CVAR_ARCHIVE,   0.0f,   0.0f,   0,  qfalse },

    { &g_teamkillPenalty,       "g_teamkillPenalty",        "-1",       CVAR_ARCHIVE,   0.0f,   0.0f,   0,  qfalse },
    #ifndef _DEMO
    { &g_teamkillDamageMax,     "g_teamkillDamageMax",      "300",      CVAR_ARCHIVE,   0.0f,   0.0f,   0,  qfalse },
    #endif // not _DEMO
    { &g_teamkillDamageForgive, "g_teamkillDamageForgive",  "50",       CVAR_ARCHIVE,   0.0f,   0.0f,   0,  qfalse },

    // Boe!Man 3/30/10
    { &server_motd1, "server_motd1", "", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
    { &server_motd2, "server_motd2", "", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
    { &server_motd3, "server_motd3", "", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
    { &server_motd4, "server_motd4", "", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
    { &server_motd5, "server_motd5", "", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
    { &server_motd6, "server_motd6", "", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },

    // Boe!Man 3/30/10: --- ADMIN--- Updated: 11/24/15
    { &g_kick,                      "g_kick",               "3",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_badmin,                    "g_badmin",             "3",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_admin,                     "g_admin",              "4",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_sadmin,                    "g_sadmin",             "5",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_ban,                       "g_ban",                "4",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_broadcast,                 "g_broadcast",          "3",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_subnetban,                 "g_subnetban",          "5",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_uppercut,                  "g_uppercut",           "3",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_runover,                   "g_runover",            "2",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_respawn,                   "g_respawn",            "4",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_mapswitch,                 "g_mapswitch",          "4",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_flash,                     "g_flash",              "3",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_plant,                     "g_plant",              "2",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_pop,                       "g_pop",                "2",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_burn,                      "g_burn",               "2",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_mute,                      "g_mute",               "2",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_strip,                     "g_strip",              "2",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_eventeams,                 "g_eventeams",          "2",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_forceteam,                 "g_forceteam",          "4",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_nosection,                 "g_nosection",          "4",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_nades,                     "g_nades",              "4",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_sl,                        "g_sl",                 "4",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_tl,                        "g_tl",                 "4",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_ri,                        "g_ri",                 "4",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_gr,                        "g_gr",                 "4",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_clanvsall,                 "g_clanvsall",          "4",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_swapteams,                 "g_swapteams",          "3",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_lock,                      "g_lock",               "3",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_clan,                      "g_clan",               "4",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_damage,                    "g_damage",             "4",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    #ifndef _DEMO
    { &g_3rd,                       "g_3rd",                "4",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    #endif // not _DEMO
    { &g_cm,                        "g_cm",                 "4",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_forcevote,                 "g_forcevote",          "3",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_pause,                     "g_pause",              "3",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_adminlist,                 "g_adminlist",          "4",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_shuffleteams,              "g_shuffleteams",       "3",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_ff,                        "g_ff",                 "4",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_rename,                    "g_rename",             "4",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_toggleweapon,              "g_toggleweapon",       "4",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_anticamp,                  "g_anticamp",           "4",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
    { &g_endmap,                    "g_endmap",             "4",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },

    { &server_colors, "server_colors", "GgKk+7", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },

    { &server_badminprefix, "server_badminprefix", "^GB-^gA^Kdm^7in", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
    { &server_adminprefix, "server_adminprefix", "^GA^gd^Km^7in", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
    { &server_sadminprefix, "server_sadminprefix", "^GS-^gA^Kdm^7in", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
    { &server_acprefix, "server_acprefix", "^GA^gd^Km^7in Only", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
    { &server_scprefix, "server_scprefix", "^GS-^gA^Kdm^7in Only", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse},
    { &server_caprefix, "server_caprefix", "^GH^ge^Ky ^7Admin!", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
    { &server_ccprefix, "server_ccprefix", "^GC^gl^Kan ^7Only", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
    { &server_starprefix, "server_starprefix", "^<*", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
    { &server_ctprefix, "server_ctprefix", "^GC^gl^Ka^7n", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
    { &server_rconprefix, "server_rconprefix", "^CS^be^kr^+v^7er", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
    { &server_specteamprefix, "server_specteamprefix", "^CS^bp^ke^+c^7tators", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },

    { &g_maxIPConnections,          "g_maxIPConnections",   "3",                CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse},
    // Boe!Man 3/30/10: This info is used for the /about menu.
    { &Owner, "Owner", "Unknown", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
    { &Clan, "Clan", "0", CVAR_ARCHIVE | CVAR_LOCK_RANGE, 0.0, 1.0 },
    { &ClanURL, "ClanURL", "0", CVAR_ARCHIVE, 0.0, 1.0 },
    // Boe!Man 2/25/11: As 1fx. Mod isn't going to be exclusive to i3D.net, we need to give providers/users the chance to set a CVAR to point to where they ARE hosting it.
    { &HostedBy, "HostedBy", "Other", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },

    { &server_redteamprefix, "server_redteamprefix", "^$R^Te^Hd", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
    { &server_blueteamprefix, "server_blueteamprefix", "^yB^Il^fu^+e", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
    { &server_hiderteamprefix, "server_hiderteamprefix", "^1H^Ti^od^qe^+r^7s", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
    { &server_seekerteamprefix, "server_seekerteamprefix", "^yS^le^le^+k^7ers", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
    { &server_humanteamprefix, "server_humanteamprefix", "^1H^Tu^om^qa^+n^7s", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
    { &server_zombieteamprefix, "server_zombieteamprefix", "^yZ^lo^lm^+b^7ies", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
    { &g_instaGib, "g_instaGib", "0", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
    { &g_compMode, "g_compMode", "0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0, 0, qfalse  },
    { &g_enableTeamCmds, "g_enableTeamCmds", "1", CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue  },
    { &g_refpassword, "g_refpassword", "none", CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue  },
    { &g_checkCountry, "g_checkCountry", "1", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse  },
    { &g_autoEvenTeams, "g_autoEvenTeams", "1", CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue },
    { &g_autoSwapTeams, "g_autoSwapTeams", "0", CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue },

    // Boe!Man 11/24/13: No section CVARs.
    { &g_useNoLower,    "g_useNoLower", "1", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse  },
    { &g_useNoRoof,     "g_useNoRoof",  "1", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse  },
    { &g_useNoMiddle,   "g_useNoMiddle","1", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse  },
    { &g_useNoWhole,    "g_useNoWhole", "1", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse  },

    { &server_enableServerMsgs, "server_enableServerMsgs", "1", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
    { &server_enableTips, "server_enableTips", "0", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
    { &server_msgInterval, "server_msgInterval", "5", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
    { &server_msgDelay, "server_msgDelay", "2", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
    { &server_message1, "server_message1", "Welcome to 1fx. Mod.", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
    { &server_message2, "server_message2", "Development forums can be found on 1fxmod.org.", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
    { &server_message3, "server_message3", "Have fun!", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
    { &server_message4, "server_message4", "", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
    { &server_message5, "server_message5", "", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },

    // Boe!Man 10/16/10
    { &g_adminSpec, "g_adminSpec", "4", CVAR_ARCHIVE | CVAR_LATCH, 0.0, 0.0, 0,  qfalse },

    // Boe!Man 11/16/10: Default scrim settings.
    { &g_matchLockSpec, "g_matchLockSpec", "1", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
    { &g_matchSwapTeams, "g_matchSwapTeams", "1", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
    { &g_matchTimeLimit, "g_matchTimeLimit", "0", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
    { &g_matchScoreLimit, "g_matchScoreLimit", "10", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
    { &g_matchDisableEvents, "g_matchDisableEvents", "1", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
    { &g_matchRounds, "g_matchRounds", "2", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
    { &g_matchDisablePubChat, "g_matchDisablePubChat", "1", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
    { &g_matchDisableTell, "g_matchDisableTell", "1", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },

    // Boe!Man 11/16/10: For Compmode. As most structures get cleared during shut down, and writing everything to temp CVARs will be more time/resource consuming, we'll simply use a couple of CVARs to update everything.
    { &cm_enabled, "cm_enabled", "0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0, 0, qfalse  },
    { &cm_sl, "cm_sl", "0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0, 0, qfalse  },
    { &cm_tl, "cm_tl", "0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0, 0, qfalse  },
    { &cm_slock, "cm_slock", "0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0, 0, qfalse  },
    { &cm_aswap, "cm_aswap", "0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0, 0, qfalse  },
    { &cm_dr, "cm_dr", "0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0, 0, qfalse  },
    { &cm_devents, "cm_devents", "0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0, 0, qfalse  },
    { &cm_oldsl, "cm_oldsl", "0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0, 0, qfalse  },
    { &cm_oldtl, "cm_oldtl", "0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0, 0, qfalse  },
    { &cm_sr, "cm_sr", "0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0, 0, qfalse  }, // Boe!Man 11/18/10: These two are used to log the 1st round results.
    { &cm_sb, "cm_sb", "0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0, 0, qfalse  },

    // Boe!Man 12/13/10
    { &g_aliasCheck, "g_aliasCheck", "1", CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue  },
    { &g_aliasCount, "g_aliasCount", "10", CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue  },

    // Henk 19/01/10 -> Set default value of weapons and disable the cvar(Only in H&S though)
    { NULL,                 "disable_weapon_knife",                 "0", CVAR_CHEAT, 0.0, 0.0, 0, qfalse },
    { NULL,                 "disable_pickup_weapon_US_SOCOM",       "1", CVAR_CHEAT, 0.0, 0.0, 0, qfalse },
    { NULL,                 "disable_pickup_weapon_M19",            "1", CVAR_CHEAT, 0.0, 0.0, 0, qfalse },
    { NULL,                 "disable_pickup_weapon_microuzi",       "1", CVAR_CHEAT, 0.0, 0.0, 0, qfalse },
    { NULL,                 "disable_pickup_weapon_M3A1",           "1", CVAR_CHEAT, 0.0, 0.0, 0, qfalse },
    { NULL,                 "disable_pickup_weapon_USAS_12",        "1", CVAR_CHEAT, 0.0, 0.0, 0, qfalse },
    { NULL,                 "disable_pickup_weapon_M590",           "1", CVAR_CHEAT, 0.0, 0.0, 0, qfalse },
    { NULL,                 "disable_pickup_weapon_MSG90A1",        "1", CVAR_CHEAT, 0.0, 0.0, 0, qfalse },
    { NULL,                 "disable_pickup_weapon_M4",             "1", CVAR_CHEAT, 0.0, 0.0, 0, qfalse },
    { NULL,                 "disable_pickup_weapon_AK_74",          "1", CVAR_CHEAT, 0.0, 0.0, 0, qfalse },
    { NULL,                 "disable_pickup_weapon_M60",            "1", CVAR_CHEAT, 0.0, 0.0, 0, qfalse },
    { NULL,                 "disable_pickup_weapon_RPG_7",          "1", CVAR_CHEAT, 0.0, 0.0, 0, qfalse },
    { NULL,                 "disable_pickup_weapon_MM_1",           "1", CVAR_CHEAT, 0.0, 0.0, 0, qfalse },
    { NULL,                 "disable_pickup_weapon_M84",            "0", CVAR_CHEAT, 0.0, 0.0, 0, qfalse },
    { NULL,                 "disable_pickup_weapon_SMOHG92",        "0", CVAR_CHEAT, 0.0, 0.0, 0, qfalse },
    { NULL,                 "disable_pickup_weapon_AN_M14",         "0", CVAR_CHEAT, 0.0, 0.0, 0, qfalse },
    { NULL,                 "disable_pickup_weapon_M15",            "0", CVAR_CHEAT, 0.0, 0.0, 0, qfalse },
    { NULL,                 "disable_pickup_weapon_M67",            "0", CVAR_CHEAT, 0.0, 0.0, 0, qfalse },
    { NULL,                 "disable_pickup_weapon_F1",             "0", CVAR_CHEAT, 0.0, 0.0, 0, qfalse },
    { NULL,                 "disable_pickup_weapon_L2A2",           "0", CVAR_CHEAT, 0.0, 0.0, 0, qfalse },
    { NULL,                 "disable_pickup_weapon_MDN11",          "0", CVAR_CHEAT, 0.0, 0.0, 0, qfalse },
    #ifdef _GOLD
    { NULL,                 "disable_pickup_weapon_MP5",            "0", CVAR_CHEAT, 0.0, 0.0, 0, qfalse },
    { NULL,                 "disable_pickup_weapon_SIG551",         "0", CVAR_CHEAT, 0.0, 0.0, 0, qfalse },
    #endif // _GOLD

    { &g_alternateMap,  "g_alternateMap", "0", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse },
    #ifdef _DEMO
    { &g_developerMap,  "g_developerMap", "0", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse },
    #endif // _DEMO

    { &g_enableCustomCommands, "g_enableCustomCommands", "0", CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue  },
    { &g_customCommandsFile,            "g_customCommandsFile",         "files/CustomCommands.txt", CVAR_ARCHIVE,   0.0,    0.0,  0, qfalse  }, // Boe!Man 3/6/11: So users can change if desired.
    { &g_tipsFile,                      "g_tipsFile",                   "files/tips.txt",   CVAR_ARCHIVE,   0.0,    0.0,  0, qfalse  }, // Boe!Man 6/24/11: So users can change if desired.
    { &hideSeek_Extra,          "hideSeek_Extra",           "110110",   CVAR_ARCHIVE|CVAR_LATCH,    0.0,    0.0,  0, qfalse  }, // Boe!Man 3/6/11: So users can change if desired.
    { &hideSeek_Nades,          "hideSeek_Nades",           "1111", CVAR_ARCHIVE|CVAR_LATCH,    0.0,    0.0,  0, qfalse  }, // Boe!Man 3/6/11: So users can change if desired.
    { &hideSeek_Weapons,        "hideSeek_Weapons",         "111",  CVAR_ARCHIVE|CVAR_LATCH,    0.0,    0.0,  0, qfalse  }, // Boe!Man 3/6/11: So users can change if desired.
    { &hideSeek_ExtendedRoundStats,     "hideSeek_ExtendedRoundStats",      "1",    CVAR_ARCHIVE,   0.0,    0.0,  0, qfalse  }, // Boe!Man 9/2/12: CVAR for extended round stats.

    // Boe!Man 3/8/11: CVAR for the Admin logging.
    { &g_enableAdminLog, "g_enableAdminLog", "1", CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue  },
    { &g_enableRconLog, "g_enableRconLog", "1", CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue },
    { &g_adminlog, "g_adminlog", "logs/admin.log", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
    { &g_rconlog, "g_rconlog", "logs/rcon.log", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
    { &g_loginlog, "g_loginlog", "logs/login.log", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },


    { &g_rpmEnt, "g_rpmEnt", "1", CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue  },

    { &g_passwordAdmins, "g_passwordAdmins", "0", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse  },

    { &g_boxAttempts, "g_boxAttempts", "3", CVAR_ARCHIVE,   0.0,    0.0,  0, qtrue  },
    { &g_cageAttempts, "g_cageAttempts", "3",   CVAR_ARCHIVE,   0.0,    0.0,  0, qtrue  },
    { &g_RpgStyle, "g_RpgStyle", "0",   CVAR_ARCHIVE,   0.0,    0.0,  0, qtrue  },
//  { &g_crossTheBridge, "g_crossTheBridge", "0",   CVAR_ARCHIVE,   0.0,    0.0,  0, qfalse  },
    { &g_noHighFps, "g_noHighFps", "0", CVAR_ARCHIVE,   0.0,    0.0,  0, qtrue  },

    // Boe!Man 5/28/11
    { &g_customWeaponFile,          "g_customWeaponFile",        "none",        CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },

    // Boe!Man 8/18/11
    { &g_objectiveLocations,           "g_objectiveLocations",         "1",       CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue },

    // Boe!Man 8/25/11
    { &g_logSpecialChat,            "g_logSpecialChat",      "1",       CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
    { &g_specialChatSound,          "g_specialChatSound",    "1",       CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },

    // Boe!Man 5/20/12
    { &g_publicIPs,                 "g_publicIPs",          "0",        CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue },

    // Boe!Man 7/29/12
    { &g_preferSubnets,             "g_preferSubnets",          "0",        CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse },

    // Boe!Man 10/19/13
    { &g_dosPatch,                  "g_DoSPatch",               "1",        CVAR_ROM, 0.0, 0.0, 0, qfalse },

    // Boe!Man 12/2/14: Caserun.
    { &g_caserun,                   "g_caserun",                "0",        CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse },

    { &g_enforceArenaCheck,         "g_enforceArenaCheck",      "1",        CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue },

    // Boe!Man 3/19/18: MVCHAT system.
    { &g_mvchatCheckSoundFiles,     "g_mvchatCheckSoundFiles",  "0",                CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },

    // Boe!Man 11/26/15: --- Anticamp CVARs, from RPM 0.75.x ---
    { &g_camperAllowTime,           "g_camperAllowTime",        "30",               CVAR_ARCHIVE, 0.0f,     0.0f,       0, qtrue },
    { &g_camperPunish,              "g_camperPunish",           "0",                CVAR_ARCHIVE, 0.0f,     0.0f,       0, qtrue },
    { &g_camperRadius,              "g_camperRadius",           "300",              CVAR_ARCHIVE, 20.0f,    2000.0f,    0, qtrue },
    { &g_camperPunishment,          "g_camperPunishment",       "pop",              CVAR_ARCHIVE, 0.0f,     0.0f,       0, qtrue },
    { &g_camperSniper,              "g_camperSniper",           "1",                CVAR_ARCHIVE, 0.0f,     0.0f,       0, qtrue },

    // Boe!Man 1/2/13: --- SQLite3 Related CVARs ---
    { &sql_aliasFlushCount,         "sql_aliasFlushCount",      "7500",             CVAR_ARCHIVE,               0.0f,   0.0f, 0,  qfalse },
    { &sql_automaticBackup,         "sql_automaticBackup",      "1",                CVAR_ARCHIVE,               0.0f,   0.0f, 0,  qfalse },
    { &g_inviewDb,                  "g_inviewDb",               "core/inview.db",   CVAR_ARCHIVE|CVAR_LATCH,    0.0f,   0.0f, 0,  qfalse },
    #ifdef _DEBUG
    { &sql_timeBench,               "sql_timeBench",            "1",                CVAR_ARCHIVE,               0.0f,   0.0f, 0,  qfalse },
    #else
    { &sql_timeBench,               "sql_timeBench",            "0",                CVAR_ARCHIVE,               0.0f,   0.0f, 0,  qfalse },
    #endif

    { &g_checkTimeRemaining,        "g_checkTimeRemaining",     "7",                CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue },

    //http://1fx.uk.to/forums/index.php?/topic/1230-1fx-anticheat/page__view__findpost__p__13498

#ifdef _3DServer
    { &boe_fragWars, "3d_fragWars", "0", CVAR_ARCHIVE | CVAR_LATCH, 0.0, 0.0, 0, qfalse },
    { &boe_deadMonkey, "3d_deadMonkey", "0", CVAR_ARCHIVE | CVAR_LATCH, 0.0, 0.0, 0, qfalse },
#endif // _3DServer

    #ifndef _DEMO
    { &g_allowthirdperson, "g_allowThirdPerson", "1", CVAR_ARCHIVE | CVAR_SERVERINFO, 0.0, 0.0, 0, qfalse },
    #endif // not _DEMO

    // Boe!Man 7/7/15: HTTP downloading and other client addition CVARs.
    { &g_enforce1fxAdditions,   "g_enforce1fxAdditions",    "0",    CVAR_ARCHIVE | CVAR_LATCH,                                      0.0, 0.0, 0, qfalse },
    { &g_httpRefPaks,           "g_httpRefPaks",            "none", CVAR_ARCHIVE | CVAR_SYSTEMINFO,                                 0.0, 0.0, 0, qfalse },
    { &g_httpBaseURL,           "g_httpBaseURL",            "none", CVAR_ARCHIVE | CVAR_SYSTEMINFO,                                 0.0, 0.0, 0, qfalse },
    { &g_httpMaxSpeed,          "g_httpMaxSpeed",           "0",    CVAR_ARCHIVE | CVAR_SYSTEMINFO,                                 0.0, 0.0, 0, qfalse },
    { &g_recoilRatio,           "g_recoilRatio",            "1.0",  CVAR_ARCHIVE | CVAR_SYSTEMINFO | CVAR_LATCH | CVAR_LOCK_RANGE,  0.0, 1.0, 0, qfalse },
    { &g_inaccuracyRatio,       "g_inaccuracyRatio",        "1.0",  CVAR_ARCHIVE | CVAR_SYSTEMINFO | CVAR_LATCH | CVAR_LOCK_RANGE,  0.0, 1.0, 0, qfalse },
};

// bk001129 - made static to avoid aliasing
static int gameCvarTableSize = sizeof( gameCvarTable ) / sizeof( gameCvarTable[0] );

void G_initClientMod            ( void );
void G_InitGame                 ( int levelTime, int randomSeed, int restart );
void G_RunFrame                 ( int levelTime );
void CheckExitRules             ( void );
void G_InitGhoul                ( void );
void G_ShutdownGhoul            ( void );

/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .q3vm file
================
*/
int vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11  )
{
    switch ( command )
    {
        case GAME_GHOUL_INIT:
            G_InitGhoul ( );
            return 0;
        case GAME_GHOUL_SHUTDOWN:
            G_ShutdownGhoul ( );
            return 0;
        case GAME_INIT:
            #ifdef _DEV
            // Boe!Man 10/11/15: Enable crash logging prior to initializing the game.
            enableCrashHandler();
            #endif // _DEV

            G_InitGame( arg0, arg1, arg2 );
            return 0;
        case GAME_SHUTDOWN:
            G_ShutdownGame( arg0 );

            #ifdef _DEV
            // Boe!Man 10/13/15: Cleanup after shutdown.
            disableCrashHandler();
            #endif // _DEV
            return 0;
        case GAME_CLIENT_CONNECT:
            return (int)ClientConnect( arg0, (qboolean)arg1, (qboolean)arg2 );
        case GAME_CLIENT_THINK:
            ClientThink( arg0 );
            return 0;
        case GAME_CLIENT_USERINFO_CHANGED:
            ClientUserinfoChanged( arg0 );
            return 0;
        case GAME_CLIENT_DISCONNECT:
            ClientDisconnect( arg0 );
            return 0;
        case GAME_CLIENT_BEGIN:
            //Ryan april 10 2004 10:04am
            //ClientBegin( arg0 );
            ClientBegin( arg0, qtrue );
            //Ryan
            return 0;
        case GAME_CLIENT_COMMAND:
            ClientCommand( arg0 );
            return 0;
        /*case GAME_GAMETYPE_COMMAND:
            return G_GametypeCommand ( arg0, arg1, arg2, arg3, arg4, arg5 );*/
        case GAME_RUN_FRAME:
            G_RunFrame( arg0 );
            return 0;
        case GAME_CONSOLE_COMMAND:
            return ConsoleCommand();
        case BOTAI_START_FRAME:
            return BotAIStartFrame( arg0 );
        case GAME_SPAWN_RMG_ENTITY:
            if (G_ParseSpawnVars(qfalse))
            {
                G_SpawnGEntityFromSpawnVars(qfalse);
            }
            return 0;
    }
    return -1;
}

/*
================
G_FindTeams

Chain together all entities with a matching team field.
Entity teams are used for item groups and multi-entity mover groups.

All but the first will have the FL_TEAMSLAVE flag set and teammaster field set
All but the last will have the teamchain field set to the next one
================
*/
void G_FindTeams( void ) {
    gentity_t   *e, *e2;
    int     i, j;
    int     c, c2;

    c = 0;
    c2 = 0;
    for ( i=1, e=g_entities+i ; i < level.num_entities ; i++,e++ ){
        if (!e->inuse)
            continue;
        if (!e->team)
            continue;
        if (e->flags & FL_TEAMSLAVE)
            continue;
        e->teammaster = e;
        c++;
        c2++;
        for (j=i+1, e2=e+1 ; j < level.num_entities ; j++,e2++)
        {
            if (!e2->inuse)
                continue;
            if (!e2->team)
                continue;
            if (e2->flags & FL_TEAMSLAVE)
                continue;
            if (!strcmp(e->team, e2->team))
            {
                c2++;
                e2->teamchain = e->teamchain;
                e->teamchain = e2;
                e2->teammaster = e;
                e2->flags |= FL_TEAMSLAVE;

                // make sure that targets only point at the master
                if ( e2->targetname ) {
                    e->targetname = e2->targetname;
                    e2->targetname = NULL;
                }
            }
        }
    }

    Com_Printf ("%i teams with %i entities\n", c, c2);
}

/*
=================
G_RemapTeamShaders
=================
*/
void G_RemapTeamShaders(void)
{
    trap_SetConfigstring(CS_SHADERSTATE, BuildShaderStateConfig());
}

/*
=================
G_RegisterCvars
=================
*/
void G_RegisterCvars( void )
{
    int         i;
    cvarTable_t *cv;
    qboolean    remapped = qfalse;

    for ( i = 0, cv = gameCvarTable ; i < gameCvarTableSize ; i++, cv++ )
    {
        trap_Cvar_Register( cv->vmCvar, cv->cvarName, cv->defaultString, cv->cvarFlags, cv->mMinValue, cv->mMaxValue );

        if ( cv->vmCvar )
        {
            cv->modificationCount = cv->vmCvar->modificationCount;
        }

        if (cv->teamShader)
        {
            remapped = qtrue;
        }
    }

    if (remapped)
    {
        G_RemapTeamShaders();
    }

    level.warmupModificationCount = g_warmup.modificationCount;
}

/*
=================
G_UpdateCvars
=================
*/
void G_UpdateCvars( void )
{
    int         i;
    cvarTable_t *cv;
    qboolean    remapped = qfalse;
    ///RxCxW - 01.22.06 - 07:47pm
//  char        cvar[20];
    ///End  - 01.22.06 - 07:47pm

    for ( i = 0, cv = gameCvarTable ; i < gameCvarTableSize ; i++, cv++ )
    {
        if ( cv->vmCvar )
        {
            trap_Cvar_Update( cv->vmCvar );

            if ( cv->modificationCount != cv->vmCvar->modificationCount )
            {
                // Handle any modified cvar checks
                if ( !Q_stricmp ( cv->cvarName, "sv_fps" ) )
                {
                    if ( cv->vmCvar->integer > 50 )
                    {
                        trap_Cvar_Set ( "sv_fps", "50" );
                        trap_Cvar_Update ( cv->vmCvar );
                    }
                    else if ( cv->vmCvar->integer < 10 )
                    {
                        trap_Cvar_Set ( "sv_fps", "10" );
                        trap_Cvar_Update ( cv->vmCvar );
                    }
                }

                // Boe!Man 6/2/11: Handle Competition Mode CVARs (the ones not toggable in-game).
                if (g_compMode.integer > 0 && cm_enabled.integer > 0){
                    if (!Q_stricmp (cv->cvarName, "g_matchDisableEvents")){
                        if(cv->vmCvar->integer == 0){
                            trap_Cvar_Set("cm_devents", "0");
                            trap_SendServerCommand(-1, va("print \"^3[Rcon Action] ^7Match events enabled.\n\""));
                        }else if(cv->vmCvar->integer > 0){
                            trap_Cvar_Set("cm_devents", "1");
                            trap_SendServerCommand(-1, va("print \"^3[Rcon Action] ^7Match events disabled.\n\""));
                        }
                    }else if (!Q_stricmp (cv->cvarName, "g_matchLockSpec")){
                        if(cv->vmCvar->integer == 0){
                            trap_Cvar_Set("cm_slock", "0");
                            trap_SendServerCommand(-1, va("print \"^3[Rcon Action] ^7Spectator team unlocked during match.\n\""));
                        }else if(cv->vmCvar->integer > 0){
                            trap_Cvar_Set("cm_slock", "1");
                            trap_SendServerCommand(-1, va("print \"^3[Rcon Action] ^7Spectator team locked during match.\n\""));
                        }
                        level.specsLocked = (cv->vmCvar->integer) ? 1 : 0;
                    }
                }

                // Boe!Man 10/26/15: Handle referenced paks and base URL CVARs here.
                if(g_enforce1fxAdditions.integer && !strlen(cv->vmCvar->string)){
                    if (!Q_stricmp (cv->cvarName, "g_httpRefPaks")){
                        trap_Cvar_Set("g_httpRefPaks", "none");
                    }
                    if (!Q_stricmp (cv->cvarName, "g_httpBaseURL")){
                        trap_Cvar_Set("g_httpBaseURL", "none");
                    }
                }

                #ifdef _DEMO
                if(level.enableCheats && !Q_stricmp (cv->cvarName, "sv_cheats")
                    && cv->modificationCount == level.enableCheats)
                {
                    // Re-enable the sv_cheats CVAR.
                    trap_Cvar_Set("sv_cheats", "1");
                    trap_Cvar_Update(&g_cheats);

                    level.enableCheats = 0;
                }
                #endif // _DEMO

                cv->modificationCount = cv->vmCvar->modificationCount;

                if ( cv->trackChange )
                {
                    //trap_SendServerCommand( -1, va("print \"Server: %s changed to %s\n\"", cv->cvarName, cv->vmCvar->string ) );
                    trap_SendServerCommand( -1, va("print \"^3[Rcon Action] ^7%s changed to %s.\n\"", cv->cvarName, cv->vmCvar->string ) );

                    if(level.timelimithit
                        && !Q_stricmp (cv->cvarName, "timelimit")){
                        // Boe!Man 5/7/17: If the timelimit is hit already, check
                        // if the new value makes the timelimit not being hit
                        // anymore.
                        if(level.time - level.startTime
                            #ifndef _DEMO
                            < (g_timelimit.integer + level.timeExtension)
                            #else
                            < (g_timelimit.integer + g_timeextension.integer
                            * g_timeextensionmultiplier.integer)
                            #endif // not _DEMO
                            * 60000){
                            level.timelimithit = qfalse;
                            G_printInfoMessageToAll("Timelimit is not hit " \
                                "anymore, continuing map.");
                        }
                    }
                }

                if (cv->teamShader)
                {
                    remapped = qtrue;
                }
            }
        }
    }

    if (remapped)
    {
        G_RemapTeamShaders();
    }
}

/*
===============
G_UpdateDisableCvars

Updates the disable cvars using the g_availableWeapons cvar.

Reversed -.-''
===============
*/
void G_UpdateDisableCvars ( void )
{
    int     weapon;
    char    *available;

    available = calloc(level.wpNumWeapons + 1, sizeof(char));

    if(current_gametype.value == GT_HS){
        strncpy(available, hideSeek_availableWeapons.string, (strlen(hideSeek_availableWeapons.string) <= level.wpNumWeapons) ? strlen(hideSeek_availableWeapons.string) : level.wpNumWeapons);
    }else{
        strncpy(available, availableWeapons.string, (strlen(availableWeapons.string) <= level.wpNumWeapons) ? strlen(availableWeapons.string) : level.wpNumWeapons);
    }

    for ( weapon = WP_KNIFE; weapon < level.wpNumWeapons; weapon++)
    {
        gitem_t* item = BG_FindWeaponItem ( (weapon_t)weapon );
        if ( !item )
        {
            continue;
        }

        if(available[weapon-1] == '1' || available[weapon-1] == '2'){
            trap_Cvar_Set ( va("disable_%s", item->classname), "0" );
        }else{
            trap_Cvar_Set ( va("disable_%s", item->classname), "1" );
        }
    }

    // Free memory.
    if(available != NULL){
        free(available);
    }
}

/*
===============
G_UpdateAvailableWeapons

Updates the g_availableWeapons cvar using the disable cvars.
===============
*/
void G_UpdateAvailableWeapons ( void )
{
    int         weapon;
    char        *available;
    int         weaponStatus;
    #ifdef _DEMO
    qboolean    groupAvailable[OUTFITTING_GROUP_MAX - 1];
    int         groupWeapon[OUTFITTING_GROUP_MAX - 1] = {
        WP_AK74_ASSAULT_RIFLE, WP_M590_SHOTGUN,
        WP_M1911A1_PISTOL, WP_SMOHG92_GRENADE
    };
    int         i;

    memset(groupAvailable, 0, sizeof(groupAvailable));
    #endif // _DEMO

    available = calloc(level.wpNumWeapons + 1, sizeof(char));

    for ( weapon = WP_KNIFE; weapon < level.wpNumWeapons; weapon ++ )
    {
        gitem_t* item = BG_FindWeaponItem ( (weapon_t)weapon );
        if ( !item )
        {
            continue;
        }

        // Determine weapon status.
        weaponStatus =
            trap_Cvar_VariableIntegerValue(va("disable_%s", item->classname));

        #ifdef _DEMO
        // See if we can mark this group as enabled.
        if(item->outfittingGroup < OUTFITTING_GROUP_MAX - 1
            && !groupAvailable[item->outfittingGroup])
        {
            groupAvailable[item->outfittingGroup] = !weaponStatus;
        }
        #endif // _DEMO

        switch(weaponStatus){
            case 0:
                available[weapon-1] = '2';
                break;

            case 1:
                available[weapon-1] = '0';
                break;

            case 2:
                available[weapon-1] = '1';
                break;
        }
    }

    #ifdef _GOLD
    trap_Cvar_Set("g_available", available);
    #elif _DEMO
    trap_Cvar_Set("g_availableWeaponsReal", available);
    #else
    // v1.00.
    trap_Cvar_Set("g_availableWeapons", available);
    #endif // _GOLD, _DEMO or Full
    trap_Cvar_Update (&g_availableWeapons);

    #ifdef _DEMO
    // In demo, we ensure at least one weapon is enabled in each group,
    // even if this really isn't the case.
    for(i = 0; i < OUTFITTING_GROUP_MAX - 1; i++){
        if(!groupAvailable[i]){
            // Mark this group available by enabling the first weapon
            // found present in this group.
            available[groupWeapon[i] - 1] = '2';
        }
    }

    // Update the outfitting CVAR so it seems all required groups are enabled.
    trap_Cvar_Set("g_availableWeapons", available);
    trap_Cvar_Update(&g_availableWeaponsOutfitting);
    #endif // _DEMO

    // Free memory allocated.
    if(available != NULL){
        free(available);
    }
}

/*
===============
G_initClientMod

Initializes the client Mod
specified by the server owner.
===============
*/
void G_initClientMod()
{
    // Always register current_gametype even if the client mod doesn't require it.
    trap_Cvar_Register(&current_gametype, "current_gametype", "3", CVAR_SERVERINFO | CVAR_ROM | CVAR_INTERNAL, 0.0, 0.0);

    #ifdef _GOLD
    if (strcmp(g_clientMod.string, "rocmod") == 0){
        level.clientMod = CL_ROCMOD;

        // Register ROCmod specific CVARs.
        if(!g_enforce1fxAdditions.integer){
            // Our ROCmod client with 1fx. additions properly reads the 1fx. Mod server version.
            trap_Cvar_Register(NULL, "sv_modVersion", "| ^71fx^1.    2.1c" , CVAR_SYSTEMINFO | CVAR_ROM, 0.0, 0.0);
        }

        trap_Cvar_Register(NULL, "g_allowCustomTeams", "1", CVAR_SYSTEMINFO | CVAR_ROM, 0.0, 0.0);
        trap_Cvar_Register(NULL, "g_customRedName", va("%s^7 Team", server_redteamprefix.string), CVAR_SYSTEMINFO | CVAR_ROM, 0.0, 0.0);
        trap_Cvar_Register(NULL, "g_customBlueName", va("%s^7 Team", server_blueteamprefix.string), CVAR_SYSTEMINFO | CVAR_ROM, 0.0, 0.0);

        // Automatic demo recording in ROCmod.
        trap_Cvar_Register(NULL, "g_autoMatchDemo", "1", CVAR_ARCHIVE, 0.0, 0.0);
        trap_Cvar_Register(NULL, "inMatch", "0", CVAR_SYSTEMINFO|CVAR_ROM|CVAR_TEMP, 0.0, 0.0);

        // Client death messages are handled by client.
        g_clientDeathMessages.integer = 1;
    }
    #else
    if(strcmp(g_clientMod.string, "RPM") == 0){
        level.clientMod = CL_RPM;

        // Register RPM 2k3 specific CVARs.
        trap_Cvar_Register(NULL, "modname", "RPM 2 k 3 v2.00 ^_- ^31fxmod.org", CVAR_SERVERINFO | CVAR_ROM, 0.0, 0.0);

        // Client death messages are handled by server.
        g_clientDeathMessages.integer = 0;
    }
    #endif // _GOLD

    if (level.clientMod != CL_NONE){
        Com_Printf("Using %s client-side modifications.\n", g_clientMod.string);

        // Also add the .pk3s from this fake fs_game in the sv_ref* list,
        // if 1fx. Additions are not enforced.
        if(!g_enforce1fxAdditions.integer){
            Patch_addAdditionalFSGame(g_clientMod.string);
        }
    }else{
        if(strcmp(g_clientMod.string, "none") != 0){
            Com_Printf("WARNING: Unknown client-side modification specified: %s\n", g_clientMod.string);
        }
        Com_Printf("Not using any client-side modifications (defaulting to \"1fx\" folder)\n");

        // Client death messages are handled by server.
        g_clientDeathMessages.integer = 0;
    }
}

/*
===============
G_setCurrentGametype

Sets the current_gametype CVAR, so the game
knows how to handle the gametype.
===============
*/

static void G_setCurrentGametype()
{
    if(Q_stricmp(g_gametype.string, "inf") == 0){
        trap_Cvar_Set("current_gametype", "3");
    #ifdef _GOLD
    }else if(g_enforce1fxAdditions.integer && Q_stricmp(g_gametype.string, "h&s") == 0){
    #else
    }else if(Q_stricmp(g_gametype.string, "h&s") == 0){
    #endif // _GOLD
        trap_Cvar_Set("current_gametype", "1");
        trap_Cvar_Set( "g_gametype", "inf" );
        trap_Cvar_Update(&g_gametype);
        // Boe!Man 10/4/12: Reset g_gametype to set the gt latched, so it will remain effective upon the next /rcon map switch..
        trap_SendConsoleCommand( EXEC_APPEND, "g_gametype h&s\n");
    #ifdef _GOLD
    }else if(g_enforce1fxAdditions.integer && Q_stricmp(g_gametype.string, "h&z") == 0){
    #else
    }else if(Q_stricmp(g_gametype.string, "h&z") == 0){
    #endif // _GOLD
        trap_Cvar_Set("current_gametype", "8");
        trap_Cvar_Set( "g_gametype", "inf" );
        trap_Cvar_Update(&g_gametype);
        // Boe!Man 10/4/12: Reset g_gametype to set the gt latched, so it will remain effective upon the next /rcon map switch..
        trap_SendConsoleCommand( EXEC_APPEND, "g_gametype h&z\n");
    #ifdef _GOLD
    }else if (!g_enforce1fxAdditions.integer && (Q_stricmp(g_gametype.string, "h&s") == 0 || Q_stricmp(g_gametype.string, "h&z") == 0)) {
        Com_Printf("This gametype is unavailable when you're not enforcing 1fx. Client Additions.\n");
        Com_Printf("Please set g_enforce1fxAdditions to 1 and restart the map, reverting to INF now.\n");
        trap_Cvar_Set("current_gametype", "3");
        trap_Cvar_Set("g_gametype", "inf");
        trap_Cvar_Update(&g_gametype);
    #endif // _GOLD
    }else if(Q_stricmp(g_gametype.string, "elim") == 0){
        trap_Cvar_Set("current_gametype", "7");
    }else if(Q_stricmp(g_gametype.string, "tdm") == 0){
        trap_Cvar_Set("current_gametype", "6");
    }else if(Q_stricmp(g_gametype.string, "dm") == 0){
        trap_Cvar_Set("current_gametype", "5");
    }else if(Q_stricmp(g_gametype.string, "ctf") == 0){
        trap_Cvar_Set("current_gametype", "4");
    #ifdef _GOLD
    }else if(Q_stricmp(g_gametype.string, "dem") == 0){
        trap_Cvar_Set("current_gametype", "2");
    #endif // _GOLD
    }else{
        // Unsupported map.
        trap_Cvar_Set("current_gametype", "0");
    }

    trap_Cvar_Update(&current_gametype);
}

/*
===============
G_SetGametype

Sets the current gametype to the given value, if the map doesnt support it then it will
use the first gametype that it does support
===============
*/
void G_SetGametype ( const char* gametype )
{
    char realgametype[8] = "0000";
    char mapname[MAX_QPATH];
    qboolean doit = qfalse;
    qboolean check, check1;
    if(current_gametype.value == GT_HS){
        gametype = "inf";
        strcpy(realgametype, "h&s");
        doit = qtrue;
    }
    trap_Cvar_VariableStringBuffer ( "mapname", mapname, MAX_QPATH );
    // Make sure the gametype is valid, if not default to deathmatch
    level.gametype = BG_FindGametype ( gametype );

    // First make sure its supported on this map
    //Ryan Dec 5 2004 Changed as per Raven forums for gold edition
    //if ( !G_DoesMapSupportGametype ( gametype ) || level.gametype == -1 )
    ///RxCxW - 08.31.06 - 10:14pm #gametype support for maps
    //if ( !(G_DoesMapSupportGametype ( gametype )
    //  || G_DoesMapSupportGametype ( bg_gametypeData[level.gametype].basegametype ) )
    //  || level.gametype == -1 )
    //Ryan
    ///Taken from the 1.03 *unofficial* sdk
    if(doit){
        check = qfalse;
        check1 = Henk_DoesMapSupportGametype (realgametype, mapname);
    }else{
        check1 = G_DoesMapSupportGametype (gametype);
        if(!G_DoesMapSupportGametype (bg_gametypeData[level.gametype].basegametype))
            check = qtrue;
        else
            check = qfalse;
    }
    if ((level.gametype == -1) || (!check
     && (!bg_gametypeData[level.gametype].basegametype || check == qtrue)))
     ///End  - 08.31.06 - 10:14pm
    {
        int i = 0;

        // Boe!Man 7/22/13: On H&S/H&Z, don't reset a map if the gametype isn't found so that we can ensure that all the game mechanics work.
        if(current_gametype.value == GT_HS || current_gametype.value == GT_HZ){
            Com_Printf("WARNING: Map does not support inf (or it is not added in the arena file)\n");
        }else{
            // Boe!Man 12/24/15: Check if this map has a valid arena file.
            // If it doesn't, we should force the default to dm (instead of dem on Gold).
            if(G_GetArenaInfoByMap(mapname) == NULL){
                for ( i = 0; i < bg_gametypeCount; i ++ )
                {
                    if(Q_stricmp(bg_gametypeData[i].name, "dm") == 0){
                        break;
                    }
                }
            }else{
                // We have a valid arena file.
                // Find a gametype that this map does support.
                for ( i = 0; i < bg_gametypeCount; i ++ )
                {
                    if ( G_DoesMapSupportGametype ( bg_gametypeData[i].name ) ){
                        break;
                    }
                }
            }

            // This is bad, this means the map doesnt support any gametypes
            if ( i >= bg_gametypeCount )
            {
                Com_Error(ERR_FATAL_NOLOG, "Map does not support any of the available " \
                    "gametypes.");
            }

            G_LogPrintf("Gametype '%s' is not supported on this map and was " \
                "defaulted to '%s'.\n", gametype, bg_gametypeData[i].name);

            gametype = bg_gametypeData[i].name;
            trap_Cvar_Set( "g_gametype", gametype );
            level.gametype = BG_FindGametype ( gametype );
            G_setCurrentGametype(); // Also force the current_gametype CVAR to the new value.

            trap_Cvar_Update( &g_gametype );
        }
    }

    level.gametypeData = &bg_gametypeData[level.gametype];

    // Copy the backpack percentage over
    bg_itemlist[MODELINDEX_BACKPACK].quantity = level.gametypeData->backpack;

    // Set the pickup state
    if ( RMG.integer || g_pickupsDisabled.integer || level.gametypeData->pickupsDisabled )
    {
        level.pickupsDisabled = qtrue;
        #ifndef _DEMO
        trap_SetConfigstring ( CS_PICKUPSDISABLED, "1" );
        #endif // not _DEMO
    }
    else
    {
        level.pickupsDisabled = qfalse;
        #ifndef _DEMO
        trap_SetConfigstring ( CS_PICKUPSDISABLED, "0" );
        #endif // not _DEMO
    }
}

// Master IPs.
static const char *masterIPs[2] = { "master.sof2.ravensoft.com", "master.1fxmod.org" };
static const int numMasterServers = 2;

#if (defined(__linux__) && defined(__GNUC__) && __GNUC__ < 3)
static void G_ResolveCallback(struct dns_cb_data *cbd)
{
    int     i;
    char    master[12];
    char    ip[MAX_IP];

    switch (cbd->error) {
    case DNS_OK:
        switch (cbd->query_type) {
        // All master servers have a valid A record, so this is what we're looking for.
        case DNS_A_RECORD:
            for(i = 0; i < numMasterServers; i++){
                if(!strcmp(masterIPs[i], cbd->name)){
                    // Note the sv_master CVAR num.
                    Com_sprintf(master, sizeof(master), "sv_master%d", i+1);
                    // Make a copy of the IP in a local buffer.
                    Com_sprintf(ip, sizeof(ip), "%u.%u.%u.%u",
                        cbd->addr[0], cbd->addr[1],
                        cbd->addr[2], cbd->addr[3]);

                    trap_Cvar_Set(master, ip);
                    Com_Printf("Set %s to: %s (resolved from: %s)\n", master, ip, cbd->name);
                }
            }
            break;
        default:
            Com_Printf("Unexpected query type %u for host %s.\n", cbd->query_type, cbd->name);
            break;
        }
        break;
    case DNS_TIMEOUT:
        Com_Printf("Query timeout for host %s.\n", cbd->name);
        break;
    case DNS_DOES_NOT_EXIST:
        Com_Printf("No such address for host %s.\n", cbd->name);
        break;
    case DNS_ERROR:
        Com_Printf("System error occured while looking up host.\n");
        break;
    }
}

static void G_ResolveMasterIPs()
{
    struct dns          *dns;
    fd_set              set;
    int                 i;
    struct timeval      tv = { 2, 0 }; // Timeout of 1 second per host, wait 2 seconds in-code because this is not the same as the real time out.

    // Initialize the resolver.
    if ((dns = dns_init()) == NULL) {
        Com_Printf("ERROR: Couldn't initialize the DNS resolver.\n");
        Com_Printf("Not making changes to sv_master CVARs due to error.\n");
        return;
    }

    for(i = 0; i < numMasterServers; i++){
        dns_queue(dns, &masterIPs[i], masterIPs[i], DNS_A_RECORD, G_ResolveCallback);

        // Select on resolver socket.
        FD_ZERO(&set);
        FD_SET(dns_get_fd(dns), &set);

        // Wait for the DNS resolver to finish.
        if (select(dns_get_fd(dns) + 1, &set, NULL, NULL, &tv) == 1)
            dns_poll(dns);
    }

    dns_fini(dns);
}
#endif // __linux__ && __GNUC__ < 3

#ifdef __linux__
// SIGINT handler.
__sighandler_t G_InterruptHandler(int signal, struct sigcontext ctx)
{
    Com_Printf("\n-------------------------------------------------\n");
    Com_Printf("Ctrl-C is not the proper way to kill the server.\n");
    Com_Printf("Please use the 'quit' command to gracefully exit.\n");
    Com_Printf("-------------------------------------------------\n");
    return 0;
}
#endif // __linux__

/*
============
G_InitGame
============
*/
void G_InitGame( int levelTime, int randomSeed, int restart )
{
    int         i;
    #ifndef _DEBUG
    char        fs_game[MAX_CVAR_VALUE_STRING];
    #endif // _DEBUG

    // Boe!Man 3/30/10
    Com_Printf ("------- Game Initialization -------\n");
    #ifdef _PRE
    Com_Printf("! PRE-RELEASE !\n");
    #endif // _PRE
    Com_Printf ("Mod: %s\n", MODFULL);
    Com_Printf ("Date: %s\n", __DATE__);
    #ifdef _GOLD
    Com_Printf("Platform: SoF2 - v1.03 (Gold)\n");
    #elif _DEMO
    Com_Printf("Platform: SoF2 - v1.02t (MP TEST)\n");
    #else
    Com_Printf("Platform: SoF2 - v1.00 (Full)\n");
    #endif // _GOLD, _DEMO or Full

    // Boe!Man 3/14/14: Check if we can actually start the server.
    if(G_CheckAlive()){
        // Try to delete the file and re-running the check.
        #ifdef _WIN32
        char fsGame[MAX_QPATH];
        trap_Cvar_VariableStringBuffer("fs_game", fsGame, sizeof(fsGame));
        DeleteFile(TEXT(va("%s\\srv.lck", fsGame)));

        if (G_CheckAlive()){
            Com_Error(ERR_FATAL_NOLOG, "Another instance is running!");
        }
        #elif __linux__
        G_LogPrintf("********************\n" \
                    "ERROR: Another instance is running!\n" \
                    "********************\n");

        level.forceExit = qtrue;
        trap_SendConsoleCommand(EXEC_NOW, "quit\n");
        #endif
    }

    // Register signal handlers.
    #ifdef __linux__
    // Boe!Man 10/17/15: Trap Ctrl+C on Linux.
    signal(SIGINT, (void *)G_InterruptHandler);
    #endif // __linux__

    // Boe!Man 4/28/15: Check fs_game string.
    #ifndef _DEBUG
    trap_Cvar_VariableStringBuffer("fs_game", fs_game, sizeof(fs_game));
    if(strcmp(fs_game, "1fx") != 0){
        Com_Printf("---------------------------\n");
        Com_Printf("ERROR: fs_game doesn't match the exact value of \"1fx\" (with or without quotes).\n" \
            "Please update your fs_game value to match the exact value.\n");
        Com_Error(ERR_FATAL_NOLOG, "Invalid fs_game value detected (must be set to \"1fx\")!");
    }
    #endif // not _DEBUG

    // Boe!Man 7/28/15: Enable proper multithreading for SQLite.
    sqlite3_config(SQLITE_CONFIG_SERIALIZED);

    #if (defined(__linux__) && defined(__GNUC__) && __GNUC__ < 3)
    // Boe!Man 1/29/13: Initialize the in-game memory-management buffer on Linux (SQLite3 memsys5).
    memset(memsys5, 0, sizeof(memsys5));
    sqlite3_config(SQLITE_CONFIG_HEAP, memsys5, 41943040, 64);
    sqlite3_soft_heap_limit(40894464);

    // Boe!Man 3/5/15: Force master to direct IP instead of hostname on Linux.
    // Resolve the IP of the master servers using TADNS.
    Com_Printf("------------------------------------------\n");
    Com_Printf("Attempting to resolve master hostnames to IPs...\n");
    i = trap_Milliseconds();

    // Do the actual lookups.
    G_ResolveMasterIPs();

    Com_Printf("Lookups took %d milliseconds.\n", trap_Milliseconds() - i);
    Com_Printf("------------------------------------------\n");
    #else
    Com_Printf("------------------------------------------\n");

    // Boe!Man 8/22/14: Users on platforms other than SoF2 v1.00 (Linux) get the regular DNS entries as master servers.
    for (i = 0; i < numMasterServers; i++) {
        char *master = va("sv_master%d", i + 1);
        // Reset master value first prior to continuing (this forces an update).
        trap_Cvar_Set(master, "");

        // Update master to use DNS address.
        trap_Cvar_Set(master, masterIPs[i]);

        // Give the user some info of what we did.
        Com_Printf("Set %s to: %s\n", master, masterIPs[i]);
    }
    Com_Printf("------------------------------------------\n");
    #endif // __linux__ && __GNUC__ < 3
    trap_Cvar_Update(gameCvarTable->vmCvar);

    // Boe!Man 1/17/16: There's a small chance this will ever happen, but
    // do make sure the server can properly recover from time or numSnapshotEntities wrapping.
    // We also have our own mechanism in place to restart after a week, but this is the
    // emergency backup so to speak.
    // N.B.: We use map mp_shop instead of mapcycle, since a mapcycle file can contain non-recoverable errors.
    // If mp_shop is not present there is something terribly, truly terribly.. wrong..
    trap_Cvar_Register(NULL, "nextmap", "map mp_shop", CVAR_ROM | CVAR_INTERNAL, 0.0, 0.0);

    srand( randomSeed );

    // set some level globals
    memset( &level, 0, sizeof( level ) );
    level.time = levelTime;
    level.startTime = levelTime;
    level.cagefightloaded = qfalse;
    G_RegisterCvars();

    // Boe!Man 4/30/15: Initialize client-side modifications.
    G_initClientMod();

    //Henk 12/10/12 -> Copy disk database to memory database.
    // Boe!Man 6/25/13: Only load if g_checkCountry is enabled, do this *after* the CVARs are initialized.
    if(g_checkCountry.integer){
        LoadCountries();
    }

    // Load the list of arenas
    G_LoadArenas ( );

    G_UpdateCvars();

    // Boe!Man 10/26/15: Ensure referenced paks and base URL CVARs aren't empty.
    if(g_enforce1fxAdditions.integer){
        if (!strlen(g_httpRefPaks.string)){
            trap_Cvar_Set("g_httpRefPaks", "none");
            trap_Cvar_Update(&g_httpRefPaks);
        }
        if (!strlen(g_httpBaseURL.string)){
            trap_Cvar_Set("g_httpBaseURL", "none");
            trap_Cvar_Update(&g_httpBaseURL);
        }

        // Servers must always have download enabled with 1fx. Additions,
        // otherwise clients can't download the Core UI.
        trap_Cvar_Set("sv_allowDownload", "1");

        #ifndef _GOLD
        // Boe!Man 10/30/15: Temporarily disable client additions for v1.00.
        trap_Cvar_Set("g_enforce1fxAdditions", "0");
        trap_Cvar_Update(&g_enforce1fxAdditions);
        #endif // not _GOLD
    }else{
        if(g_recoilRatio.value != 1.0f){
            Com_Printf("Info: g_recoilRatio has been reset to 1.0, needs Client Additions enabled.\n");
            trap_Cvar_Set("g_recoilRatio", "1.0");
            trap_Cvar_Update(&g_recoilRatio);
        }
        if(g_inaccuracyRatio.value != 1.0f){
            Com_Printf("Info: g_inaccuracyRatio has been reset to 1.0, needs Client Additions enabled.\n");
            trap_Cvar_Set("g_inaccuracyRatio", "1.0");
            trap_Cvar_Update(&g_inaccuracyRatio);
        }
    }

    #ifdef _DEMO
    // Check if we should enable cheats because the user
    // requested a developer map.
    if(g_developerMap.integer){
        // We enable the sv_cheats CVAR, but the engine will disable this again
        // at the end of the game initialization.
        // We re-enable the sv_cheats CVAR at that point.
        trap_Cvar_Set("sv_cheats", "1");
        trap_Cvar_Update(&g_cheats);
        level.enableCheats = g_cheats.modificationCount;

        trap_Cvar_Set("g_developerMap", "0");
        trap_Cvar_Update(&g_developerMap);
    }
    #endif // _DEMO

    // Build the gametype list so we can verify the given gametype
    BG_BuildGametypeList ( );

    //Before we set the gametype we change current_gametype and we set H&S to INF
    if(!restart){
        G_setCurrentGametype();
    }

    // Set the current gametype
    G_SetGametype(g_gametype.string);

    #ifndef _DEMO
    // Give the game a uniqe id
    trap_SetConfigstring ( CS_GAME_ID, va("%d", randomSeed ) );
    #endif // not _DEMO

    // Apply memory runtime modifications.
    Patch_Main();

    G_LogPrintf("------------------------------------------------------------\n");
    if ( g_log.string[0] )
    {
        if ( g_logSync.integer )
        {
            trap_FS_FOpenFile( g_log.string, &level.logFile, FS_APPEND_SYNC_TEXT );
        }
        else
        {
            trap_FS_FOpenFile( g_log.string, &level.logFile, FS_APPEND_TEXT );
        }

        if ( !level.logFile )
        {
            Com_Printf( "WARNING: Couldn't open logfile: %s\n", g_log.string );
        }
        else
        {
            char    serverinfo[MAX_INFO_STRING];

            trap_GetServerinfo( serverinfo, sizeof( serverinfo ) );
            G_LogPrintf("InitGame: %s\n", serverinfo );
        }
    }
    else
    {
        Com_Printf( "Not logging to disk.\n" );
    }

    G_InitWorldSession();
    G_InitMutesFromSession();

    // initialize all entities for this game
    memset( g_entities, 0, MAX_GENTITIES * sizeof(g_entities[0]) );
    level.gentities = g_entities;

    // initialize all clients for this game
    level.maxclients = g_maxclients.integer;
    memset( g_clients, 0, MAX_CLIENTS * sizeof(g_clients[0]) );
    level.clients = g_clients;

    // set client fields on player ents
    for ( i=0 ; i<level.maxclients ; i++ )
    {
        g_entities[i].client = level.clients + i;
    }

    // always leave room for the max number of clients,
    // even if they aren't all used, so numbers inside that
    // range are NEVER anything but clients
    level.num_entities = MAX_CLIENTS;

    // let the server system know where the entites are
    trap_LocateGameData( level.gentities,
                         level.num_entities,
                         sizeof( gentity_t ),
                         &level.clients[0].ps,
                         sizeof( level.clients[0] ) );

    // Get the boundaries of the world
    trap_GetWorldBounds ( level.worldMins, level.worldMaxs );

    // reserve some spots for dead player bodies
    G_InitBodyQueue();

    // Boe!Man 11/11/15: Initialize stats memory.
    G_AllocateStatsMemory(NULL);

    // Boe!Man 7/27/15: Initialize weapon and ammo globals.
    #ifndef _GOLD
    level.wpNumWeapons = 22;
    level.ammoMax = 16;

    level.grenadeMin = 14;
    level.grenadeMax = 21;
    #else
    level.grenadeMin = 17;

    if (g_enforce1fxAdditions.integer) {
        level.wpNumWeapons = 25;
        level.ammoMax = 18;
        level.grenadeMax = 24;
    }else{
        level.wpNumWeapons = 21;
        level.ammoMax = 14;
        level.grenadeMax = 20;
    }
    #endif // not _GOLD

    BG_InitializeWeaponsAndAmmo();

    BG_ParseInviewFile();

    G_WeaponMod();

    ClearRegisteredItems();

    // Henk 22/01/11 -> New weapon cvars.
    G_UpdateDisableCvars(); // Set the disabled_ cvars from availableWeapons and hideSeek_availableWeapons

    G_UpdateAvailableWeapons(); // also set the original g_availableWeapons for the client :)
    // End

    // Boe!Man 11/13/12: New check for Nades, see if they are used at all in the server (faster !nn checking, and proper backpack fix).
    if(SetNades("0")){ // 0 means disable_* CVAR to 0, so enabled.
        level.nadesFound = qtrue;
    }
    // Set the available outfitting -- Boe!Man 5/22/12: Update, now also checks H&S settings.
    if(g_disableNades.integer){
        SetNades("1");
    }

    BG_SetAvailableOutfitting(g_availableWeapons.string);

    if(current_gametype.value == GT_HS){
        AddSpawnField("classname", "gametype_item");
        AddSpawnField("targetname", "briefcase");
        AddSpawnField("gametype", "inf");
        AddSpawnField("origin", "9999 9999 9999"); // Boe!Man 9/10/11: "999 999 999" bugs in a few maps (start solid), so changed the origin a bit.
        trap_UnlinkEntity(&g_entities[G_SpawnGEntityFromSpawnVars(qtrue)]);

        // No case fight winner at first.
        strcpy(level.cagewinner, "none");
    }

    // parse the key/value pairs and spawn gentities
    G_SpawnEntitiesFromString(qfalse);

    #ifndef _DEMO
    InitSpawn(3);

    if(current_gametype.value == GT_HS){
        // Boe!Man 2/5/14: Specific BSPs get loaded for H&S as well.
        InitSpawn(1);
        InitSpawn(2);
        InitSpawn(4);
        InitSpawn(5);
        InitSpawn(6);
    }

    #if defined _DEV || defined _awesomeToAbuse
    // Boe!Man 3/19/15: For /dev bsp.
    if (current_gametype.value != GT_HS){
        InitSpawn(2);
    }
    #endif // _DEV
    #endif // not _DEMO

    // Now parse the gametype information that we need.  This needs to be
    // done after the entity spawn so that the items and triggers can be
    // linked up properly
    G_ParseGametypeFile ( );

    // Load in the identities
    BG_ParseNPCFiles ( );

    // general initialization
    G_FindTeams();

    SaveRegisteredItems();

    Com_Printf ("-----------------------------------\n");

    #ifndef _DEMO
    if( trap_Cvar_VariableIntegerValue( "com_buildScript" ) )
    {
        G_SoundIndex( "sound/player/gurp1.wav", qtrue );
        G_SoundIndex( "sound/player/gurp2.wav", qtrue );
    }
    #endif // not _DEMO

#ifdef _SOF2_BOTS
    if ( trap_Cvar_VariableIntegerValue( "bot_enable" ) )
    {
        BotAISetup( restart );
        BotAILoadMap( restart );
        G_InitBots( (qboolean)restart );
    }
#endif

    G_RemapTeamShaders();

    // Cache the global action sound index.
    #ifndef _DEMO
    level.actionSoundIndex = G_SoundIndex("sound/misc/menus/click.wav", qtrue);
    #else
    level.actionSoundIndex = G_SoundIndex("sound/misc/menus/select.wav", qtrue);
    #endif // not _DEMO

    // Initialize the gametype
    // Boe!Man 11/29/12: Now that the gametype is in the game, the gt can check what gametype it is using current_gametype.
    trap_GT_Init ();

    // Music
    if ( RMG.integer )
    {
        char temp[MAX_INFO_STRING];

        // start the music
        trap_Cvar_VariableStringBuffer("RMG_music", temp, MAX_QPATH);
        trap_SetConfigstring( CS_MUSIC, temp );
    }

    #ifdef _DEMO
    trap_Cvar_Set("g_timeextensionmultiplier", "0");
    #endif // _DEMO

    trap_SetConfigstring( CS_VOTE_TIME, "" );

    // setup settings for h&s
    if(current_gametype.value == GT_HS || current_gametype.value == GT_HZ){
        SetNades("0"); // Enable them, but check them individually in that func cause' it's H&S.
    }

    // Setup settings for caserun if enabled.
    if (current_gametype.value == GT_INF && g_caserun.integer){
        level.gametypeData->respawnType = RT_INTERVAL;
    }

    // Boe!Man 3/30/10
    mvchat_parseFiles();

    // Boe!Man 1/30/14: If we're not in a game that has rounds and the user wishes to have automatic backups (enabled by default), do this every 5 minutes.
    if(sql_automaticBackup.integer && level.gametypeData->respawnType == RT_INTERVAL){
        level.sqlBackupTime = level.time + 50000;
    }
    // Boe!Man 12/8/12: Check database integrity.
    Boe_userdataIntegrity();

    // Boe!Man 11/24/13: Check the state.
    g_checkSectionState();

    if (current_gametype.value == GT_HZ){
        level.nextZombie = -1;
        level.lastHuman = -1;
    }

    // Boe!Man 7/29/12: Check for g_preferSubnets and g_passwordAdmins not both being set to 1 (incompatible).
    if(g_passwordAdmins.integer && g_preferSubnets.integer){
        trap_Cvar_Set("g_passwordAdmins", "0");
        trap_Cvar_Update(&g_passwordAdmins);
        Com_Printf("Info: g_passwordAdmins has been set to 0 due to g_preferSubnets being set to %i. Set g_preferSubnets to 0 to allow Admins to login using a password.\n", g_preferSubnets.integer);
    }

    // Boe!Man 11/16/10: Scrim settings.
    if (g_compMode.integer > 0){
        level.compMsgCount = level.time + 6000;
        // Boe!Man 11/16/10: Scrim already initialized and map restarted? Start the actual scrim.
        if (cm_enabled.integer == 1){
            trap_Cvar_Set("cm_enabled", "2"); // This is set to 2 - The scrim initialized and it just hit the first round.
            trap_Cvar_Set("scorelimit", cm_sl.string); // Set the scorelimit the same as the previously mentioned scrim setting.
            trap_Cvar_Set("timelimit", cm_tl.string); // And the timelimit as well..
        }
        else if (cm_enabled.integer == 21){
            trap_Cvar_Set("cm_enabled", "3"); // This is set to 3 - The scrim just showed the awards and is waiting for user input.
            // Boe!Man 11/17/10: Display the Match screen again, and set the scorelimit and timelimit temporary to 0 in order to make sure they don't hit the limit... again.
            level.compMsgCount = level.time + 2000;
            g_scorelimit.integer = 0;
            g_timelimit.integer = 0;
        }
        else if (cm_enabled.integer == 3){
            trap_Cvar_Set("cm_enabled", "4"); // This is set to 4 - The scrim resumed and it just hit the second round.
            trap_Cvar_Set("scorelimit", cm_sl.string); // Set the scorelimit the same as the previously mentioned scrim setting.
            trap_Cvar_Set("timelimit", cm_tl.string); // And the timelimit as well..
        }
        else if (cm_enabled.integer == 2 || cm_enabled.integer == 4 || cm_enabled.integer == 5){ // Boe!Man 11/19/10: This is bad, meaning they warped out of the scrim during a match.
                                                                                                 // Or, in the case of '5', the scrim already ended.
            trap_Cvar_Set("g_compMode", "0");
            trap_Cvar_Set("cm_enabled", "0");
            trap_Cvar_Set("cm_devents", "0");
            trap_Cvar_Set("scorelimit", cm_oldsl.string);
            trap_Cvar_Set("timelimit", cm_oldtl.string);
            // Boe!Man 2/7/11: Reset the invatations after the scrim has ended/had an unexpected ending.
            for ( i = 0; i < level.numConnectedClients; i ++ ){
                g_entities[level.sortedClients[i]].client->sess.invitedByBlue = qfalse;
                g_entities[level.sortedClients[i]].client->sess.invitedByRed = qfalse;
            }

            #ifdef _GOLD
            if(level.clientMod == CL_ROCMOD){
                // Ensure we stop the match mode.
                trap_Cvar_Set("inMatch", "0");
            }
            #endif // _GOLD
        }
        if (cm_enabled.integer == 0){ // Boe!Man 6/2/11: Little piece of error handling. When someone decides to reboot the server; either a crash or intented, Competition Mode didn't completely shut down. We don't worry about the score- or timelimit, Config should always be executed first?
            trap_Cvar_Set("g_compMode", "0");
            level.compMsgCount = 0;
        }
    }
}

/*
=================
G_ShutdownGame
=================
*/
void G_ShutdownGame( int restart )
{
    #ifdef _WIN32
    char    fsGame[MAX_QPATH];
    #endif // _WIN32

    Com_Printf ("==== ShutdownGame ====\n");

    if ( level.logFile )
    {
        G_LogPrintf("ShutdownGame:\n" );
        G_LogPrintf("------------------------------------------------------------\n" );
        trap_FS_FCloseFile( level.logFile );
    }

    // Boe!Man 1/27/13: Reset the internal g_alternateMap CVAR if an alt map is loaded and the map is restarted.
    if(restart){
        if(level.altEnt){
            trap_Cvar_Set( "g_alternateMap", "1");
            trap_Cvar_Update ( &g_alternateMap );
        }
    }

    #ifdef _DEMO
    if(g_cheats.integer){
        // Disable cheats prior to restarting to avoid getting
        // "CHEATS ARE ENABLED" on the loading screen.
        trap_Cvar_Set("sv_cheats", "0");
        trap_Cvar_Update(&g_cheats);
    }
    #endif // _DEMO

    //Henk 12/10/12 -> Detach and close memory database
    if(g_checkCountry.integer){
        UnloadCountries();
    }

    // Boe!Man 5/27/13: Also detach and close userdata databases.
    Boe_unloadUserdataDbs();

    // write all the client session data so we can get it back
    G_WriteSessionData();

    #ifdef __linux__
    // Boe!Man 10/30/15: Restore sigint handler.
    signal(SIGINT, SIG_DFL);
    #endif // __linux__

    #if defined(__linux__) && (defined(__GNUC__) && __GNUC__ < 3)
    sqlite3_shutdown();
    memset(memsys5, 0, sizeof(memsys5));
    Com_Printf("SQLite3 shutdown.\n");

    // Boe!Man 1/28/15: Thanks to LinuxThreads, also kill the Thread Manager. This "fixes" a crash on "new" (> Linux 2.4) systems.
    pthread_kill_other_threads_np();
    Com_Printf("Thread Manager shutdown.\n");
    #elif _WIN32
    UnlockFile(lockFile, 0, 0, 0xffffff, 0xffffff);
    CloseHandle(lockFile);

    trap_Cvar_VariableStringBuffer("fs_game", fsGame, sizeof(fsGame));
    DeleteFile(TEXT(va("%s\\srv.lck", fsGame)));
    #endif // __linux__ && GNUC < 3

    #ifdef _SOF2_BOTS
    if(trap_Cvar_VariableIntegerValue("bot_enable")){
        BotAIShutdown( restart );
    }
    #endif

    // Boe!Man 7/27/15: Free statinfo memory of all clients.
    G_FreeStatsMemory(NULL);

    #ifdef __linux__
    if(level.forceExit){
        // Raise SIGUSR1 when we want to force an unsuccessful exit.
        raise(SIGUSR1);
    }
    #endif // __linux__
}

/*
==============
G_softCrashHandler
6/2/17 - 8:15 PM
Handles the soft crash
that just happened.
==============
*/

static void G_softCrashHandler(int lvl, const char *msg)
{
    #ifdef _DEV
    if(lvl == ERR_FATAL_NOLOG){
        trap_Error(msg);
        return;
    }

    trap_Cvar_Set("com_errorMessage", msg);

    #ifdef __linux__
    level.forceExit = qtrue;
    raise(SIGUSR2);
    #else // _WIN32
    RaiseException(EXCEPTION_NONCONTINUABLE_EXCEPTION,
        EXCEPTION_NONCONTINUABLE_EXCEPTION, 0, NULL);
    #endif // __linux__
    #else
    trap_Error(msg);
    #endif // _DEV
}

#ifndef GAME_HARD_LINKED

void QDECL Com_Error(int level, const char *fmt, ...)
{
    va_list     argptr;
    char        text[1024];

    va_start (argptr, fmt);
    vsprintf (text, fmt, argptr);
    va_end (argptr);

    G_softCrashHandler(level, text);
}

void QDECL Com_Printf( const char *msg, ... )
{
    va_list     argptr;
    char        text[1024];

    va_start (argptr, msg);
    vsprintf (text, msg, argptr);
    va_end (argptr);

    trap_Printf( text );
}

#endif

/*
=============
SortRanks
=============
*/
int QDECL SortRanks( const void *a, const void *b )
{
    gclient_t   *ca;
    gclient_t   *cb;

    ca = &level.clients[*(int *)a];
    cb = &level.clients[*(int *)b];

    // sort special clients last
    if ( ca->sess.spectatorState == SPECTATOR_SCOREBOARD )
    {
        return 1;
    }

    if ( cb->sess.spectatorState == SPECTATOR_SCOREBOARD )
    {
        return -1;
    }

    // then connecting clients
    if ( ca->pers.connected == CON_CONNECTING )
    {
        return 1;
    }

    if ( cb->pers.connected == CON_CONNECTING )
    {
        return -1;
    }

    // then spectators
    if ( ca->sess.team == TEAM_SPECTATOR && cb->sess.team == TEAM_SPECTATOR )
    {
        if ( ca->sess.spectatorTime < cb->sess.spectatorTime )
        {
            return -1;
        }
        if ( ca->sess.spectatorTime > cb->sess.spectatorTime )
        {
            return 1;
        }
        return 0;
    }

    if ( ca->sess.team == TEAM_SPECTATOR )
    {
        return 1;
    }

    if ( cb->sess.team == TEAM_SPECTATOR )
    {
        return -1;
    }

    // then sort by score
    if ( ca->sess.score > cb->sess.score )
    {
        return -1;
    }

    if ( ca->sess.score < cb->sess.score )
    {
        return 1;
    }

    // then sort by kills
    if ( ca->sess.kills > cb->sess.kills )
    {
        return -1;
    }

    if ( ca->sess.kills < cb->sess.kills )
    {
        return 1;
    }

    // then sort by deaths
    if ( ca->sess.deaths > cb->sess.deaths )
    {
        return -1;
    }

    if ( ca->sess.deaths < cb->sess.deaths )
    {
        return 1;
    }

    return 0;
}

/*
============
CalculateRanks

Recalculates the score ranks of all players
This will be called on every client connect, begin, disconnect, death,
and team change.
============
*/
void CalculateRanks( void )
{
    int         i;
    int         rank;
    int         score;
    int         newScore;
    gclient_t   *cl;

    level.follow1 = -1;
    level.follow2 = -1;
    level.numConnectedClients = 0;
    level.numNonSpectatorClients = 0;
    level.numPlayingClients = 0;
    level.numVotingClients = 0;     // don't count bots

    for ( i = 0 ; i < level.maxclients ; i++ )
    {
        if ( level.clients[i].pers.connected != CON_DISCONNECTED )
        {
            level.sortedClients[level.numConnectedClients] = i;
            level.numConnectedClients++;

            if ( level.clients[i].sess.team != TEAM_SPECTATOR )
            {
                level.numNonSpectatorClients++;

                // decide if this should be auto-followed
                if ( level.clients[i].pers.connected == CON_CONNECTED )
                {
                    level.numPlayingClients++;
                    if ( !(g_entities[i].r.svFlags & SVF_BOT) )
                    {
                        level.numVotingClients++;
                    }
                    if ( level.follow1 == -1 )
                    {
                        level.follow1 = i;
                    }
                    else if ( level.follow2 == -1 )
                    {
                        level.follow2 = i;
                    }
                }
            }
        }
    }

    qsort( level.sortedClients, level.numConnectedClients,
           sizeof(level.sortedClients[0]), SortRanks );

    // set the rank value for all clients that are connected and not spectators
    if ( level.gametypeData->teams )
    {
        int rank;
        if ( level.teamScores[TEAM_RED] == level.teamScores[TEAM_BLUE] )
        {
            rank = 2;
        }
        else if ( level.teamScores[TEAM_RED] > level.teamScores[TEAM_BLUE] )
        {
            rank = 0;
        }
        else
        {
            rank = 1;
        }

        // in team games, rank is just the order of the teams, 0=red, 1=blue, 2=tied
        for ( i = 0;  i < level.numConnectedClients; i++ )
        {
            cl = &level.clients[ level.sortedClients[i] ];
            cl->ps.persistant[PERS_RANK] = rank;
        }
    }
    else
    {
        rank = -1;
        score = 0;
        for ( i = 0;  i < level.numPlayingClients; i++ )
        {
            cl = &level.clients[ level.sortedClients[i] ];
            newScore = cl->sess.score;
            if ( i == 0 || newScore != score )
            {
                rank = i;
                // assume we aren't tied until the next client is checked
                level.clients[ level.sortedClients[i] ].ps.persistant[PERS_RANK] = rank;
            }
            else
            {
                // we are tied with the previous client
                level.clients[ level.sortedClients[i-1] ].ps.persistant[PERS_RANK] = rank | RANK_TIED_FLAG;
                level.clients[ level.sortedClients[i] ].ps.persistant[PERS_RANK] = rank | RANK_TIED_FLAG;
            }
            score = newScore;
        }
    }


    // see if it is time to end the level
    CheckExitRules();

    // if we are at the intermission, send the new info to everyone
    if (level.intermissiontime || level.pause)
    {
        SendScoreboardMessageToAllClients();
    }
}


/*
========================================================================

MAP CHANGING

========================================================================
*/

/*
========================
SendScoreboardMessageToAllClients

Do this at BeginIntermission time and whenever ranks are recalculated
due to enters/exits/forced team changes
========================
*/
void SendScoreboardMessageToAllClients( void ) {
    int     i;

    for ( i = 0 ; i < level.maxclients ; i++ ) {
        if ( level.clients[ i ].pers.connected == CON_CONNECTED ) {
            DeathmatchScoreboardMessage( g_entities + i );
        }
    }
}

/*
========================
MoveClientToIntermission

When the intermission starts, this will be called for all players.
If a new client connects, this will be called after the spawn function.
========================
*/
void MoveClientToIntermission( gentity_t *ent )
{
    // take out of follow mode if needed
    if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW )
    {
        G_StopFollowing( ent );
    }

    // move to the spot
    VectorCopy( level.intermission_origin, ent->s.origin );
    VectorCopy( level.intermission_origin, ent->client->ps.origin );
    VectorCopy (level.intermission_angle, ent->client->ps.viewangles);

    // Reset some client variables
    ent->client->ps.pm_type       = PM_INTERMISSION;
    ent->client->ps.stats[STAT_GAMETYPE_ITEMS] = 0;
    ent->client->ps.eFlags        = 0;
    ent->s.eFlags                 = 0;
    ent->s.eType                  = ET_GENERAL;
    ent->s.modelindex             = 0;
    ent->s.loopSound              = 0;
    ent->s.event                  = 0;
    ent->r.contents               = 0;
}

/*
==================
FindIntermissionPoint

This is also used for spectator spawns
==================
*/
void FindIntermissionPoint( void )
{
    gentity_t   *ent, *target;
    vec3_t      dir;

    // find the intermission spot
    ent = G_Find (NULL, FOFS(classname), "info_player_intermission");
    if ( !ent )
    {
        gspawn_t* spawn = G_SelectRandomSpawnPoint ( (team_t)-1, NULL, qfalse );
        if ( spawn )
        {
            VectorCopy (spawn->origin, level.intermission_origin);
            VectorCopy (spawn->angles, level.intermission_angle);
        }

        return;
    }

    VectorCopy (ent->s.origin, level.intermission_origin);
    VectorCopy (ent->s.angles, level.intermission_angle);

    // if it has a target, look towards it
    if ( ent->target )
    {
        target = G_PickTarget( ent->target );
        if ( target )
        {
            VectorSubtract( target->s.origin, level.intermission_origin, dir );
            vectoangles( dir, level.intermission_angle );
        }
    }
}

/*
==================
BeginIntermission
==================
*/
void BeginIntermission( void )
{
    int         i;
    gentity_t   *ent;

    if ( level.intermissiontime )
    {
        // already active
        return;
    }

    // Kill any votes
    level.voteTime = 0;
    trap_SetConfigstring( CS_VOTE_TIME, "" );

    level.intermissiontime  = level.time;
    level.gametypeResetTime = level.time;
    FindIntermissionPoint();

    // move all clients to the intermission point
    for (i=0 ; i< level.maxclients ; i++)
    {
        ent = g_entities + i;
        if (!ent->inuse)
        {
            continue;
        }

        //Ryan april 10 2004
        //total these up for the awards
        if(ent->client->sess.team == TEAM_SPECTATOR)
        {
            ent->client->sess.totalSpectatorTime += level.time - ent->client->sess.spectatorTime;
        }
        else if(ent->client->sess.ghostStartTime)
        {
            ent->client->sess.totalSpectatorTime += level.time - ent->client->sess.ghostStartTime;
        }
        //Ryan

        // take out of follow mode
        G_StopFollowing( ent );

        // Get rid of ghost state
        G_StopGhosting ( ent );

        // respawn if dead
        if ( G_IsClientDead ( ent->client ) )
        {
            respawn ( ent );
        }

        MoveClientToIntermission( ent );
    }

    // send the current scoring to all clients
    SendScoreboardMessageToAllClients();

    #ifdef _GOLD
    if (level.clientMod == CL_ROCMOD) {
        // Send the Awards to the players.
        ROCmod_sendBestPlayerStats();
    }
    #endif // _GOLD
}


/*
=============
ExitLevel

When the intermission has been exited, the server is either killed
or moved to a new level based on the "nextmap" cvar

=============
*/
void ExitLevel (void)
{
    int         i;
    gclient_t   *cl;

    ///Ryan march 21 2004 9:19am
    if ((!*g_mapcycle.string || !Q_stricmp ( g_mapcycle.string, "none" ) || g_compMode.integer > 0 && cm_enabled.integer >= 1))
    {
        trap_SendServerCommand( -1, va("cp \"@ \n\""));
        if (g_compMode.integer > 0 && cm_enabled.integer == 5){
            trap_SendConsoleCommand( EXEC_APPEND, va("map %s\n", level.mapname ));
        }
        else{
            trap_SendConsoleCommand( EXEC_APPEND, "map_restart 0\n" );
        }
        return;
    }
    ///Ryan

    G_switchToNextMapInCycle(qtrue);
    level.changemap = 1;
    level.intermissiontime = 0;


    // reset all the scores so we don't enter the intermission again
    level.teamScores[TEAM_RED] = 0;
    level.teamScores[TEAM_BLUE] = 0;
    for ( i=0 ; i< g_maxclients.integer ; i++ )
    {
        cl = level.clients + i;
        if ( cl->pers.connected != CON_CONNECTED )
        {
            continue;
        }

        cl->sess.score = 0;
        cl->ps.persistant[PERS_SCORE] = 0;
    }

    // we need to do this here before chaning to CON_CONNECTING
    G_WriteSessionData();

    // change all client states to connecting, so the early players into the
    // next level will know the others aren't done reconnecting
    for (i=0 ; i< g_maxclients.integer ; i++)
    {
        if ( level.clients[i].pers.connected == CON_CONNECTED )
        {
            level.clients[i].pers.connected = CON_CONNECTING;
        }
    }
}

/*
=================
G_LogPrintf

Print to the logfile with a time stamp if it is open
=================
*/
void QDECL G_LogPrintf( const char *fmt, ... ) {
    va_list     argptr;
    char        string[1024];

    ///RxCxW - 04.28.06 - 03:01am #timeLog
    ///int          min, tens, sec;
   ///End  - 04.28.06 - 03:015am


    ///CJJ - 1.1.2005 - Logging the time #logging
    qtime_t         q;
    trap_RealTime   (&q);
    ///END - 1.1.2005

    ///RxCxW - 04.28.06 - 03:01am #timeLog
    ///sec = level.time / 1000;
    ///min = sec / 60;
    ///sec -= min * 60;
    ///tens = sec / 10;
    ///sec -= tens * 10;

    ///Com_sprintf( string, sizeof(string), "%4i:%i%i ", min, tens, sec );
   Com_sprintf( string, sizeof(string), "%02i/%02i/%i %02i:%02i - ", 1+q.tm_mon,q.tm_mday, q.tm_year+1900,q.tm_hour,q.tm_min);
   ///End  - 04.28.06 - 03:01am

    va_start( argptr, fmt );
    ///RxCxW - 04.28.06 - 03:01am #timeLog
    ///vsprintf( string +8 , fmt,argptr );
    vsprintf( string + 19 , fmt,argptr );
    ///End  - 04.28.06 - 03:02am
    va_end( argptr );

#ifndef _DEBUG
   if ( g_dedicated.integer ) {
#endif
        ///RxCxW - 04.28.06 - 03:02am #timelog
        ///Com_Printf( "%s", string + 8 );
        Com_Printf( "%s", string + 19 );
        ///End  - 04.28.06 - 03:02am

#ifndef _DEBUG
    }
#endif

    if ( !level.logFile ) {
        return;
    }

    trap_FS_Write( string, strlen( string ), level.logFile );
}

/*
================
LogExit

Append information about this game to the log file
================
*/
void LogExit( const char *string )
{
    int             i;
    int             numSorted;
    gclient_t       *cl;

    G_LogPrintf( "Exit: %s\n", string );

    level.intermissionQueued = level.time;

    // this will keep the clients from playing any voice sounds
    // that will get cut off when the queued intermission starts
    trap_SetConfigstring( CS_INTERMISSION, "1" );

    // don't send more than 32 scores (FIXME?)
    numSorted = level.numConnectedClients;
    if ( numSorted > 32 )
    {
        numSorted = 32;
    }

    if ( level.gametypeData->teams )
    {
        G_LogPrintf( "red:%i  blue:%i\n", level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE] );
    }

    for (i=0 ; i < numSorted ; i++)
    {
        int ping;

        cl = &level.clients[level.sortedClients[i]];

        if ( cl->sess.team == TEAM_SPECTATOR )
        {
            continue;
        }

        if ( cl->pers.connected == CON_CONNECTING )
        {
            continue;
        }

        ping = cl->ps.ping < 999 ? cl->ps.ping : 999;

        G_LogPrintf( "score: %i  ping: %i  client: %i %s\n", cl->sess.score, ping, level.sortedClients[i],  cl->pers.netname );
    }
}


///*
//================
//LogExit
//
//Append information about this game to the log file
//================
//*/
//void LogExit( const char *string )
//{
//  int             i;
//  int             numSorted;
//  gclient_t       *cl;
//
//  G_LogPrintf( "Exit: %s\n", string );
//
//  level.intermissionQueued = level.time;
//
//  // this will keep the clients from playing any voice sounds
//  // that will get cut off when the queued intermission starts
//  trap_SetConfigstring( CS_INTERMISSION, "1" );
//
//  // don't send more than 32 scores (FIXME?)
//  numSorted = level.numConnectedClients;
//  if ( numSorted > 32 )
//  {
//      numSorted = 32;
//  }
//
//  if ( level.gametypeData->teams )
//  {
//      G_LogPrintf( "red:%i  blue:%i\n", level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE] );
//  }
//
//  for (i=0 ; i < numSorted ; i++)
//  {
//      int ping;
//
//      cl = &level.clients[level.sortedClients[i]];
//
//      if ( cl->sess.team == TEAM_SPECTATOR )
//      {
//          continue;
//      }
//
//      if ( cl->pers.connected == CON_CONNECTING )
//      {
//          continue;
//      }
//
//      ping = cl->ps.ping < 999 ? cl->ps.ping : 999;
//
//      G_LogPrintf( "score: %i  ping: %i  client: %i %s\n", cl->sess.score, ping, level.sortedClients[i],  cl->pers.netname );
//  }
//}

/*
=================
G_LogLogin

Print to the login file with a time stamp.
=================
*/
void QDECL G_LogLogin(const char *fmt, ...) {
    fileHandle_t    loginFile;
    va_list     argptr;
    char        string[1024];
    qtime_t         q;

    trap_RealTime(&q);

    Com_sprintf(string, sizeof(string), "%02i/%02i/%i %02i:%02i - ", 1 + q.tm_mon, q.tm_mday, q.tm_year + 1900, q.tm_hour, q.tm_min);

    va_start(argptr, fmt);
    vsprintf(string + 19, fmt, argptr);
    va_end(argptr);

    // Boe!Man 11/22/10: Open and write to the crashinfo file.
    trap_FS_FOpenFile(g_loginlog.string, &loginFile, FS_APPEND_TEXT);
    if (!loginFile){
        return;
    }

    trap_FS_Write(string, strlen(string), loginFile);
    trap_FS_Write("\n", 1, loginFile);
    trap_FS_FCloseFile(loginFile);
}

/*
=================
G_LogRcon

Print to the rcon action file with a time stamp.
=================
*/
void QDECL G_LogRcon(const char *fmt, ...) {
    fileHandle_t    rconFile;
    va_list         argptr;
    char            string[1024];
    qtime_t         q;

    trap_RealTime(&q);
    Com_sprintf(string, sizeof(string), "%02i/%02i/%i %02i:%02i ", 1 + q.tm_mon, q.tm_mday, q.tm_year + 1900, q.tm_hour, q.tm_min);

    va_start(argptr, fmt);
    vsprintf(string + 17, fmt, argptr);
    va_end(argptr);

    // Boe!Man 11/22/10: Open and write to the crashinfo file.
    trap_FS_FOpenFile(g_rconlog.string, &rconFile, FS_APPEND_TEXT);
    if (!rconFile){
        return;
    }

    trap_FS_Write(string, strlen(string), rconFile);
    trap_FS_Write("\n", 1, rconFile);
    trap_FS_FCloseFile(rconFile);
}

/*
=================
CheckIntermissionExit

The level will stay at the intermission for a minimum of 5 seconds
If all players wish to continue, the level will then exit.
If one or more players have not acknowledged the continue, the game will
wait 10 seconds before going on.
=================
*/
void CheckIntermissionExit( void )
{
//Ryan
    if ( !level.exitTime )
    {
        level.exitTime = level.time;
    }

    if ( level.time < level.exitTime + 5000 )
    {
        return;
    }

    if(!level.awardTime)
    {
        if(current_gametype.value == GT_HS)
            ShowScores();
        #ifndef _GOLD
        else
            RPM_Awards();
        #endif // not _GOLD
        level.awardTime = level.time;
        level.lastAwardSent = level.time;

        // Boe!Man 10/27/14: Make sure that in H&Z players are all forceteamed to spec.
        if (current_gametype.value == GT_HZ){
            int i;
            gentity_t *ent;

            #ifdef _DEBUG
            Com_Printf("Start forceteam to spec.\n");
            #endif
            for(i = 0; i < level.maxclients; i++){
                ent = g_entities + i;
                if (ent->client->pers.connected != CON_CONNECTED)
                    continue;

                if (ent->client->sess.team == TEAM_SPECTATOR)
                    continue;

                SetTeam(ent, "spectator", NULL, qtrue);
            }
            #ifdef _DEBUG
            Com_Printf("End forceteam to spec.\n");
            #endif
        }
        return;
    }

    if(level.awardTime && (level.time > level.lastAwardSent + 3000))
    {
        if(current_gametype.value == GT_HS)
            ShowScores();
        #ifndef _GOLD
        else
            RPM_Awards();
        #endif // not _GOLD
        level.lastAwardSent = level.time;
    }

    if(level.time < level.awardTime + 15000)
    {
        return;
    }
    //RPM_LogAwards();
    ExitLevel();
}

/*
=============
ScoreIsTied
=============
*/
qboolean ScoreIsTied( void )
{
    int a;
    int b;

    if ( level.numPlayingClients < 2 )
    {
        return qfalse;
    }

    if ( level.gametypeData->teams )
    {
        return (qboolean)(level.teamScores[TEAM_RED] == level.teamScores[TEAM_BLUE]);
    }

    a = level.clients[level.sortedClients[0]].sess.score;
    b = level.clients[level.sortedClients[1]].sess.score;

    return (qboolean)(a == b);
}

/*
=================
checkExitTimes

Check if the level is about to end in a couple of minutes.
If so, inform all players.
=================
*/

void checkExitTimes ( void )
{
    int timeRemaining;
    int roundLevelTime;

    // Don't check exit times if we don't enforce a timelimit.
    if(!g_timelimit.integer){
        return;
    }

    // Only check the exit times once a second.
    if(level.lastTimeEndCheck > level.time){
        return;
    }
    level.lastTimeEndCheck = level.time + 1000;

    // Determine the minutes remaining.
    roundLevelTime = level.time - level.startTime;
    timeRemaining = g_timelimit.integer - roundLevelTime / 60000;

    // Only print in the first second of a whole minute.
    if(roundLevelTime % 60000 < 1000){
        // Check if we should print this time remaining message.
        // The g_checkTimeRemaining CVAR value is bitflag based.
        // 0 = disabled.
        // 1 = print 1 minute remaining.
        // 2 = print 3 minutes remaining.
        // 4 = print 5 minutes remaining.
        if((g_checkTimeRemaining.integer & 1 && timeRemaining == 1)
            || (g_checkTimeRemaining.integer & 2 && timeRemaining == 3)
            || (g_checkTimeRemaining.integer & 4 && timeRemaining == 5)){

            char color[4];
            memset(color, 0, sizeof(color));

            // Use a color from the server_colors CVAR so the results are
            // always somewhat in-style with the current color-scheme.
            if (server_colors.string != NULL
                && strlen(server_colors.string) >= 3){
                strncpy(color, va("^%c", server_colors.string[3]),
                    sizeof(color));
            }else{
                // Always have some sort of backup in case no colors are set.
                strncpy(color, "^1", sizeof(color));
            }

            // Broadcast the change.
            G_Broadcast(va("%s%d \\minute%s remaining!", color, timeRemaining,
                (timeRemaining != 1) ? "s" : ""), BROADCAST_GAME2, NULL);
            G_printInfoMessageToAll("%d minute%s remaining!", timeRemaining,
                (timeRemaining != 1) ? "s" : "");
        }
    }
}

/*
=================
CheckExitRules

There will be a delay between the time the exit is qualified for
and the time everyone is moved to the intermission spot, so you
can see the last frag.
=================
*/
void CheckExitRules( void )
{
    int         i;
    gclient_t   *cl;
    gentity_t*  tent;

    // if at the intermission, wait for all non-bots to
    // signal ready, then go to next level
    if ( level.intermissiontime || level.changemap )
    {
        CheckIntermissionExit ();
        return;
    }

    if ( level.intermissionQueued )
    {
        int time = INTERMISSION_DELAY_TIME;
        if ( level.time - level.intermissionQueued >= time )
        {
            level.intermissionQueued = 0;
            BeginIntermission();
        }

        return;
    }

    // check for sudden death
    /* Henk 07/03/11 -> This will destroy our timelimit hit message and action.
    if ( ScoreIsTied() ) // HENK FIX ME -> Add allow tie variable?
    {
        // always wait for sudden death
        return;
    }*/

    // Check to see if the timelimit was hit
    if (g_timelimit.integer && !level.warmupTime && !level.timelimithit && !level.cagefight)
    {
        #ifndef _DEMO
        if (level.time - level.startTime >= (g_timelimit.integer + level.timeExtension)*60000)
        #else
        if (level.time - level.startTime >= (g_timelimit.integer + g_timeextension.integer * g_timeextensionmultiplier.integer) * 60000)
        #endif // not _DEMO
        {
            if(current_gametype.value == GT_INF
                || current_gametype.value == GT_ELIM
                #ifdef _GOLD
                || current_gametype.value == GT_DEM
                #endif // _GOLD
                || current_gametype.value == GT_HS
                || current_gametype.value == GT_HZ){
                G_printInfoMessageToAll("Timelimit hit, waiting for round to finish.");
                level.timelimithit = qtrue;
            }else{
                tent = G_TempEntity( vec3_origin, EV_GAME_OVER );
                if(cm_enabled.integer > 0){ // Boe!Man 3/18/11: Only change the entry if competition mode's enabled.
                    tent->s.eventParm = LEEG;
                }else{
                    tent->s.eventParm = GAME_OVER_TIMELIMIT;
                }
                tent->r.svFlags = SVF_BROADCAST;

                if(g_compMode.integer > 0 && cm_enabled.integer > 1){
                    if(current_gametype.value == GT_CTF){
                        if ( level.teamScores[TEAM_BLUE] == level.teamScores[TEAM_RED] ){
                        // Boe!Man 3/21/11 FIXME: Might need a CVAR for this in the future..
                            if(level.timelimitMsg == qfalse){
                                G_printInfoMessageToAll("Timelimit hit, waiting for the final flag to be captured.");
                                level.timelimitMsg = qtrue;
                            }
                            return;
                        }
                    }
                    Boe_compTimeLimitCheck();
                }else{
                    LogExit("Timelimit hit.");
                }
            }
            return;
        }
    }

    // Check to see if the score was hit
    if ( g_scorelimit.integer )
    {
        if ( level.gametypeData->teams )
        {
            if ( level.teamScores[TEAM_RED] >= g_scorelimit.integer )
            {
                tent = G_TempEntity( vec3_origin, EV_GAME_OVER );
                if(cm_enabled.integer > 0){ // Boe!Man 3/18/11: Only change the entry if competition mode's enabled.
                    //if (cm_enabled.integer < 4 && cm_dr.integer == 1){
                    //  tent->s.eventParm = GAME_OVER_SCORELIMIT;
                    //}else{
                        tent->s.eventParm = LEEG;
                    //}
                }else{
                    tent->s.eventParm = GAME_OVER_SCORELIMIT;
                }
                tent->r.svFlags = SVF_BROADCAST;
                tent->s.otherEntityNum = TEAM_RED;
                if (g_compMode.integer > 0 && cm_enabled.integer == 2){
                    //LogExit(va("%s ^7team wins 1st round with %i - %i", server_redteamprefix, level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE] ));
                    if(cm_dr.integer == 1){ // Boe!Man 3/18/11: If dual rounds are enabled, make use of them and display the temporary stuff.
                        G_Broadcast(va("%s ^7team wins the 1st round with %i - %i!", server_redteamprefix.string, level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE]), BROADCAST_AWARDS, NULL);
                        // Boe!Man 11/18/10: Set the scores right (for logging purposes).
                        if (cm_aswap.integer == 0){
                            trap_Cvar_Set("cm_sr", va("%i", level.teamScores[TEAM_RED]));
                            trap_Cvar_Set("cm_sb", va("%i", level.teamScores[TEAM_BLUE]));
                        }else{
                            // Boe!Man 11/19/10: Log the scores the other way around as the teams will get swapped the next round.
                            trap_Cvar_Set("cm_sr", va("%i", level.teamScores[TEAM_BLUE]));
                            trap_Cvar_Set("cm_sb", va("%i", level.teamScores[TEAM_RED]));
                        }
                        G_printInfoMessageToAll("Red team wins the 1st round with %d - %d.", level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE]);
                        trap_Cvar_Set("cm_enabled", "21"); // Boe!Man 3/18/11: Display the awards prior to displaying this view again.
                        LogExit("Red team wins the 1st round.");
                    }else{ // Boe!Man 3/18/11: Red team won the match.
                        G_printInfoMessageToAll("Red team wins the match with %d - %d.", level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE]);
                        G_Broadcast(va("%s ^7team wins the match with %i - %i!", server_redteamprefix.string, level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE]), BROADCAST_AWARDS, NULL);
                        trap_Cvar_Set("cm_enabled", "5"); // Boe!Man 11/18/10: 5 - Scrim Ended.
                        LogExit("Red team wins the match.");
                    }
                }else if (g_compMode.integer > 0 && cm_enabled.integer == 4){
                    if (cm_sr.integer > cm_sb.integer){
                        G_printInfoMessageToAll("Red team won the 1st round with %d - %d.", cm_sr.integer, cm_sb.integer);
                    }else if(cm_sr.integer < cm_sb.integer){
                        G_printInfoMessageToAll("Blue team won the 1st round with %d - %d.", cm_sb.integer, cm_sr.integer);
                    }else{
                        G_printInfoMessageToAll("Round draw 1st round with %d - %d.", cm_sb.integer, cm_sr.integer);
                    }
                    G_printInfoMessageToAll("Red team won the 2nd round with %d - %d.", level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE]);
                    trap_Cvar_Set("cm_enabled", "5"); // Boe!Man 11/18/10: 5 - Scrim Ended.
                    Boe_calcMatchScores();
                }else{
                    if(current_gametype.value == GT_HS){
                        #ifdef _DEBUG
                        Com_Printf("Updating scores..\n");
                        #endif
                        UpdateScores();
                        LogExit("Hiders hit the score limit.");
                    }else{
                    #ifndef _DEMO
                    LogExit("Red team hit the score limit.");
                    #else
                    LogExit("Red Team hit the Team Score Limit.");
                    #endif // not _DEMO
                    }
                }

                return;
            }

            if ( level.teamScores[TEAM_BLUE] >= g_scorelimit.integer )
            {
                tent = G_TempEntity( vec3_origin, EV_GAME_OVER );
                if(cm_enabled.integer > 0){ // Boe!Man 3/18/11: Only change the entry if competition mode's enabled.
                    //if (cm_enabled.integer < 4 && cm_dr.integer == 1){
                    //  tent->s.eventParm = GAME_OVER_SCORELIMIT;
                    //}else{
                        tent->s.eventParm = LEEG;
                    //}
                }else{
                    tent->s.eventParm = GAME_OVER_SCORELIMIT;
                }
                tent->r.svFlags = SVF_BROADCAST;
                tent->s.otherEntityNum = TEAM_BLUE;
                if (g_compMode.integer > 0 && cm_enabled.integer == 2){
                    if(cm_dr.integer == 1){ // Boe!Man 3/18/11: If dual rounds are enabled, make use of them and display the temporary stuff.
                        G_Broadcast(va("%s ^7team wins the 1st round with %i - %i!", server_blueteamprefix.string, level.teamScores[TEAM_BLUE], level.teamScores[TEAM_RED]), BROADCAST_AWARDS, NULL);
                        // Boe!Man 11/18/10: Set the scores right (for logging purposes).
                        if (cm_aswap.integer == 0){
                            trap_Cvar_Set("cm_sr", va("%i", level.teamScores[TEAM_RED]));
                            trap_Cvar_Set("cm_sb", va("%i", level.teamScores[TEAM_BLUE]));
                        }else{
                            // Boe!Man 11/19/10: Log the scores the other way around as the teams will get swapped the next round.
                            trap_Cvar_Set("cm_sr", va("%i", level.teamScores[TEAM_BLUE]));
                            trap_Cvar_Set("cm_sb", va("%i", level.teamScores[TEAM_RED]));
                        }
                        G_printInfoMessageToAll("Blue team wins the 1st round with %d - %d.", level.teamScores[TEAM_BLUE], level.teamScores[TEAM_RED]);
                        trap_Cvar_Set("cm_enabled", "21");
                        LogExit("Blue team wins the 1st round.");
                    }else{ // Boe!Man 3/18/11: Blue team won the match.
                        G_printInfoMessageToAll("Blue team wins the match with %d - %d.", level.teamScores[TEAM_BLUE], level.teamScores[TEAM_RED]);
                        G_Broadcast(va("%s ^7team wins the match with %i - %i!", server_blueteamprefix.string, level.teamScores[TEAM_BLUE], level.teamScores[TEAM_RED]), BROADCAST_AWARDS, NULL);
                        trap_Cvar_Set("cm_enabled", "5"); // Boe!Man 11/18/10: 5 - Scrim Ended.
                        LogExit("Blue team wins the match.");
                    }
                }else if (g_compMode.integer > 0 && cm_enabled.integer == 4){
                    if (cm_sr.integer > cm_sb.integer){
                        G_printInfoMessageToAll("Red team won the 1st round with %d - %d.", cm_sr.integer, cm_sb.integer);
                    }else if(cm_sr.integer < cm_sb.integer){
                        G_printInfoMessageToAll("Blue team won the 1st round with %d - %d.", cm_sb.integer, cm_sr.integer);
                    }else{
                        G_printInfoMessageToAll("Round draw 1st round with %d - %d.", cm_sb.integer, cm_sr.integer);
                    }
                    G_printInfoMessageToAll("Blue team won the 2nd round with %d - %d.", level.teamScores[TEAM_BLUE], level.teamScores[TEAM_RED]);
                    trap_Cvar_Set("cm_enabled", "5"); // Boe!Man 11/18/10: 5 - Scrim Ended.
                    Boe_calcMatchScores();
                }
                else{
                    if(current_gametype.value == GT_HS){
                        #ifdef _DEBUG
                        Com_Printf("Updating scores..\n");
                        #endif
                        UpdateScores();
                        LogExit("Seekers hit the score limit.");
                    }else{
                        #ifndef _DEMO
                        LogExit("Blue team hit the score limit.");
                        #else
                        LogExit("Blue Team hit the Team Score Limit.");
                        #endif // not _DEMO
                    }
                }
                return;
            }
        }
        else
        {
            // Check to see if any of the clients scores have crossed the scorelimit
            for ( i = 0 ; i < level.numConnectedClients ; i++ )
            {
                cl = g_entities[level.sortedClients[i]].client;

                if ( cl->pers.connected != CON_CONNECTED )
                {
                    continue;
                }

                if ( cl->sess.team != TEAM_FREE )
                {
                    continue;
                }

                if ( cl->sess.score >= g_scorelimit.integer )
                {
                    tent = G_TempEntity( vec3_origin, EV_GAME_OVER );
                    tent->s.eventParm = GAME_OVER_SCORELIMIT;
                    tent->r.svFlags = SVF_BROADCAST;
                    tent->s.otherEntityNum = level.sortedClients[i];

                    #ifndef _DEMO
                    LogExit("Scorelimit hit.");
                    #else
                    LogExit("Fraglimit hit.");
                    #endif // not _DEMO
                    return;
                }
            }
        }
    }

    // Boe!Man 12/28/16: Check if the map is requested to end.
    if(level.endMap && level.endMap == level.time){
        // Let the clients know the map is ended.
        tent = G_TempEntity(vec3_origin, EV_GAME_OVER);
        tent->s.eventParm = LEEG;
        tent->r.svFlags = SVF_BROADCAST;
        tent->s.otherEntityNum = level.sortedClients[i];

        // Reset the game type timer.
        if(level.gametypeData->respawnType != RT_INTERVAL){
            trap_SetConfigstring(CS_GAMETYPE_TIMER, va("%d", level.time));
        }

        // Update the H&S scores.
        if(current_gametype.value == GT_HS){
            UpdateScores();
        }

        // Begin intermission.
        LogExit("Map ended on request.");
    }
}

/*
=============
CheckWarmup
=============
*/
void CheckWarmup ( void )
{
    int         counts[TEAM_NUM_TEAMS];
    qboolean    notEnough = qfalse;

    // check because we run 3 game frames before calling Connect and/or ClientBegin
    // for clients on a map_restart
    if ( level.numPlayingClients == 0 )
    {
        return;
    }

    if ( !level.warmupTime  )
    {
        return;
    }

    //Ryan   if there are any players that are not ready do no start the warmup countdown untill they are
    if (g_doWarmup.integer == 2 && level.warmupTime < 0)
    {
        int     i, ready = 0;

        for ( i = 0; i < level.numPlayingClients; i ++ )
        {
            if (g_entities[level.sortedClients[i]].client->sess.team == TEAM_SPECTATOR)
            {
                continue;
            }

            if ( g_entities[level.sortedClients[i]].client->pers.connected != CON_CONNECTED )
            {
                continue;
            }

            if (!g_entities[level.sortedClients[i]].client->pers.ready)
            {
                continue;
            }

            ready++;
        }
    }
    //Ryan

    if ( level.gametypeData->teams )
    {
        counts[TEAM_BLUE] = TeamCount( -1, TEAM_BLUE, NULL );
        counts[TEAM_RED] = TeamCount( -1, TEAM_RED, NULL );

        if (counts[TEAM_RED] < 1 || counts[TEAM_BLUE] < 1)
        {
            notEnough = qtrue;
        }
    }
    else if ( level.numPlayingClients < 2 )
    {
        notEnough = qtrue;
    }

    //Ryan if only 1 player and the swap_teams vote passes let it execute
    if(level.swapteams)
    {
        notEnough = qfalse;
    }
    //Ryan

    if ( notEnough )
    {
        if ( level.warmupTime != -1 )
        {
            level.warmupTime = -1;
            trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );
            G_LogPrintf( "Warmup:\n" );
        }

        return; // still waiting for team members
    }

    if ( level.warmupTime == 0 )
    {
        return;
    }

    // if the warmup is changed at the console, restart it
    if ( g_warmup.modificationCount != level.warmupModificationCount )
    {
        level.warmupModificationCount = g_warmup.modificationCount;
        level.warmupTime = -1;
    }

    // if all players have arrived, start the countdown
    if ( level.warmupTime < 0 )
    {
        // fudge by -1 to account for extra delays
        level.warmupTime = level.time + ( g_warmup.integer - 1 ) * 1000;
        trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );
        return;
    }

    // if the warmup time has counted down, restart
    if ( level.time > level.warmupTime )
    {
        level.warmupTime += 10000;
        trap_Cvar_Set( "g_restarted", "1" );
        trap_SendConsoleCommand( EXEC_APPEND, "map_restart 0\n" );
        level.restarted = qtrue;
        return;
    }
}

/*
==================
PrintTeam
==================
*/
void PrintTeam(int team, char *message)
{
    int i;

    for ( i = 0 ; i < level.maxclients ; i++ )
    {
        if (level.clients[i].sess.team != team)
        {
            continue;
        }

        trap_SendServerCommand( i, message );
    }
}

/*
==================
CheckCvars
==================
*/
void CheckCvars( void )
{
    static int lastMod = -1;

    if ( g_password.modificationCount != lastMod )
    {
        lastMod = g_password.modificationCount;
        if ( *g_password.string && Q_stricmp( g_password.string, "none" ) )
        {
            trap_Cvar_Set( "g_needpass", "1" );
        } else
        {
            trap_Cvar_Set( "g_needpass", "0" );
        }
    }
}

/*
=============
G_RunThink

Runs thinking code for this frame if necessary
=============
*/
void G_RunThink (gentity_t *ent)
{
    float   thinktime;
    thinktime = ent->nextthink;

    if (thinktime <= 0)
    {
        return;
    }
    if (thinktime > level.time)
    {
        return;
    }

    ent->nextthink = 0;
    if(!ent->think){
        Com_Error(ERR_FATAL, "NULL ent->think");
    }
    ent->think (ent);
}

// Henk 23/01/10 -> Give players their outfitting nades and give seekers box nades
void SetupOutfitting(void)
{
    int i;
    int start_ammo_type;

    for ( i = 0; i < level.numConnectedClients; i ++ )
    {
        g_entities[level.sortedClients[i]].client->sess.timeOfDeath = 0;
        g_entities[level.sortedClients[i]].client->noOutfittingChange = qfalse;
        G_UpdateOutfitting ( g_entities[level.sortedClients[i]].s.number );
        if(g_entities[level.sortedClients[i]].client->sess.team == TEAM_BLUE){
            if(hideSeek_Extra.string[MDN11] == '1'){
                g_entities[level.sortedClients[i]].client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_MDN11_GRENADE );
                start_ammo_type = weaponData[WP_MDN11_GRENADE].attack[ATTACK_NORMAL].ammoIndex;
                g_entities[level.sortedClients[i]].client->ps.ammo[start_ammo_type] = weaponData[WP_MDN11_GRENADE].attack[ATTACK_NORMAL].clipSize;
                g_entities[level.sortedClients[i]].client->ps.clip[ATTACK_NORMAL][WP_MDN11_GRENADE] = weaponData[WP_MDN11_GRENADE].attack[ATTACK_NORMAL].clipSize;
                g_entities[level.sortedClients[i]].client->ps.firemode[WP_MDN11_GRENADE] = BG_FindFireMode ( WP_MDN11_GRENADE, ATTACK_NORMAL, WP_FIREMODE_AUTO );

                // alt-fire ammo
                start_ammo_type = weaponData[WP_MDN11_GRENADE].attack[ATTACK_ALTERNATE].ammoIndex;
                if (AMMO_NONE != start_ammo_type)
                {
                    g_entities[level.sortedClients[i]].client->ps.ammo[start_ammo_type] = ammoData[start_ammo_type].max;
                }
            }
            if(hideSeek_Extra.string[F1] == '1'){
                g_entities[level.sortedClients[i]].client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_F1_GRENADE );
                start_ammo_type = weaponData[WP_F1_GRENADE].attack[ATTACK_NORMAL].ammoIndex;
                g_entities[level.sortedClients[i]].client->ps.ammo[start_ammo_type] = weaponData[WP_F1_GRENADE].attack[ATTACK_NORMAL].clipSize;
                g_entities[level.sortedClients[i]].client->ps.clip[ATTACK_NORMAL][WP_F1_GRENADE] = weaponData[WP_F1_GRENADE].attack[ATTACK_NORMAL].clipSize;
                g_entities[level.sortedClients[i]].client->ps.firemode[WP_F1_GRENADE] = BG_FindFireMode ( WP_F1_GRENADE, ATTACK_NORMAL, WP_FIREMODE_AUTO );

                // alt-fire ammo
                start_ammo_type = weaponData[WP_F1_GRENADE].attack[ATTACK_ALTERNATE].ammoIndex;
                if (AMMO_NONE != start_ammo_type)
                {
                    g_entities[level.sortedClients[i]].client->ps.ammo[start_ammo_type] = ammoData[start_ammo_type].max;
                }
            }
            if(hideSeek_Extra.string[L2A2] == '1'){
                g_entities[level.sortedClients[i]].client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_L2A2_GRENADE );
                start_ammo_type = weaponData[WP_L2A2_GRENADE].attack[ATTACK_NORMAL].ammoIndex;
                g_entities[level.sortedClients[i]].client->ps.ammo[start_ammo_type] = weaponData[WP_L2A2_GRENADE].attack[ATTACK_NORMAL].clipSize;
                g_entities[level.sortedClients[i]].client->ps.clip[ATTACK_NORMAL][WP_L2A2_GRENADE] = weaponData[WP_L2A2_GRENADE].attack[ATTACK_NORMAL].clipSize;
                g_entities[level.sortedClients[i]].client->ps.firemode[WP_L2A2_GRENADE] = BG_FindFireMode ( WP_L2A2_GRENADE, ATTACK_NORMAL, WP_FIREMODE_AUTO );

                // alt-fire ammo
                start_ammo_type = weaponData[WP_L2A2_GRENADE].attack[ATTACK_ALTERNATE].ammoIndex;
                if (AMMO_NONE != start_ammo_type)
                {
                    g_entities[level.sortedClients[i]].client->ps.ammo[start_ammo_type] = ammoData[start_ammo_type].max;
                }
            }
        }
    }
}

void Henk_CheckZombie(void){
    int i, random, teamCountBlue, teamCountRed;
    gentity_t *ent = NULL;

    // Don't continue if we are in intermission.
    if(level.intermissionQueued || level.intermissiontime || level.pause || level.changemap){
        return;
    }

    teamCountBlue = TeamCount1(TEAM_BLUE);
    teamCountRed = TeamCount1(TEAM_RED);

    if(level.numConnectedClients > 0 && level.time >= level.gametypeStartTime+5000 && teamCountRed >= 2){
        if (teamCountBlue == 0 && teamCountRed >= 1 && level.messagedisplay2 == qtrue){
            level.zombie = -1;
            level.messagedisplay2 = qfalse;
        }

        if(level.zombie == -1 && teamCountBlue == 0 && level.time >= level.zombietime+10000){
            if (level.nextZombie != -1){
                ent = &g_entities[level.nextZombie];
                level.nextZombie = -1;
            }

            if (!ent || !ent->client){
                random = irand(0, level.numConnectedClients);
                ent = &g_entities[level.sortedClients[random]];
            }

            if(ent && ent->client && ent->client->sess.team == TEAM_RED && !G_IsClientDead(ent->client)){
                if(ent->client->sess.team != TEAM_SPECTATOR){ // extra check
                    level.zombie = ent->s.number;
                    level.zombietime = level.time + 5000;
                    G_Broadcast(va("%s\nwill turn into a \\zombie\nin ^15 ^7seconds!", ent->client->pers.netname), BROADCAST_GAME2, NULL);
                    G_Broadcast("You will turn into a \\zombie\nin ^15 ^7seconds!", BROADCAST_GAME2, ent);
                }
                #ifdef _DEBUG
                else{
                    Com_Printf("Weird case\n");
                }
                #endif // _DEBUG
            }
        }
    }

    if(level.time >= level.zombietime && level.messagedisplay2 == qfalse){
        if (teamCountRed >= 2 && teamCountBlue == 0){
            if(level.zombie != -1){
                if (g_entities[level.zombie].client && g_entities[level.zombie].client->pers.connected == CON_CONNECTED && g_entities[level.zombie].client->sess.team != TEAM_SPECTATOR && !G_IsClientDead(g_entities[level.zombie].client)){
                    trap_SendServerCommand(-1, va("print \"^3[H&Z] ^7%s suddenly turned into a zombie!\n\"", g_entities[level.zombie].client->pers.cleanName) );
                    G_Broadcast(va("%s\nturned into a \\Zombie!", g_entities[level.zombie].client->pers.netname), BROADCAST_GAME2, NULL);
                    // turn into zombie
                    CloneBody(&g_entities[level.zombie], g_entities[level.zombie].s.number);
                    level.messagedisplay2 = qtrue;
                    level.zombie = -1;
                }else{
                    level.zombie = -1;
                }
            }
        }else if(teamCountRed < 2 && teamCountBlue == 0 && level.zombie != -1){
            level.zombie = -1;
        }
    }

    if(level.time >= level.gametypeStartTime+8000 && level.messagedisplay == qfalse && level.gametypeStartTime >= 5000){
        trap_SendServerCommand(-1, va("print \"^3[H&Z] ^7Shotguns distributed.\n\""));
        G_Broadcast("\\Shotguns distributed!", BROADCAST_GAME, NULL);
        Boe_GlobalSound(level.actionSoundIndex);

        for(i=0;i<level.numConnectedClients;i++){
            ent = &g_entities[level.sortedClients[i]];
            if(!ent)
                continue;

            if(ent->client->sess.team == TEAM_RED){
                ent->client->ps.ammo[weaponData[WP_M590_SHOTGUN].attack[ATTACK_NORMAL].ammoIndex]=9;
                ent->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_M590_SHOTGUN );
                ent->client->ps.clip[ATTACK_NORMAL][WP_M590_SHOTGUN]=9;
                ent->client->ps.firemode[WP_M590_SHOTGUN] = BG_FindFireMode ( WP_M590_SHOTGUN, ATTACK_NORMAL, WP_FIREMODE_AUTO );

                ent->client->ps.weapon = WP_M590_SHOTGUN;
                ent->client->ps.weaponstate = WEAPON_READY;

                if (level.lastHuman == ent->s.number){
                    ent->client->ps.ammo[weaponData[WP_M67_GRENADE].attack[ATTACK_NORMAL].ammoIndex] = 1;
                    ent->client->ps.stats[STAT_WEAPONS] |= (1 << WP_M67_GRENADE);
                    ent->client->ps.clip[ATTACK_NORMAL][WP_M67_GRENADE] = 0;
                    ent->client->ps.firemode[WP_M67_GRENADE] = BG_FindFireMode(WP_M67_GRENADE, ATTACK_NORMAL, WP_FIREMODE_AUTO);

                    trap_SendServerCommand(ent-g_entities, va("print \"^3[H&Z] ^7You now have the forcefield grenade.\n\""));
                }
            }
        }
        level.messagedisplay = qtrue;
        level.lastHuman = -1;
    }

}

/*
================
Henk_CheckHS

Check H&S events
================
*/
void Henk_CheckHS(void)
{
    int countred;
    int i, random, rpgwinner = -1, m4winner = -1;
    gspawn_t    *spawnPoint;
    gentity_t   *dropped;

    // Don't continue if we are in intermission.
    if(level.intermissionQueued || level.intermissiontime || level.pause || level.changemap){
        return;
    }

    // Henk 19/02/10 -> Copy origin of dropped weapon to flare
    if(g_entities[level.MM1ent].s.pos.trType == TR_STATIONARY && level.MM1Time != 0 && level.gametypeStartTime >= 5000){
        Effect(g_entities[level.MM1ent].r.currentOrigin, "flare_blue", qfalse);
        level.MM1Time = 0;
    }
    if(g_entities[level.M4ent].s.pos.trType == TR_STATIONARY && level.M4Time != 0 && level.gametypeStartTime >= 5000){
        Effect(g_entities[level.M4ent].r.currentOrigin, "flare_red", qfalse);
        level.M4Time = 0;
    }
    if(g_entities[level.RPGent].s.pos.trType == TR_STATIONARY && level.RPGTime != 0 && level.gametypeStartTime >= 5000){
        Effect(g_entities[level.RPGent].r.currentOrigin, "flare_red", qtrue);
        level.RPGTime = 0;
    }
    // Henk 19/01/10 -> Last man standing
    if(level.time > level.gametypeDelayTime && level.gametypeStartTime >= 5000){
        countred = level.teamAliveCount[TEAM_RED];
        if(countred == 2 && level.lastaliveCheck[0] == qfalse){ // 2 hiders alive
            level.lastalive[0] = -1;
            level.lastalive[1] = -1;
            for(i = 0; i < level.numConnectedClients; i++){
                if (g_entities[level.sortedClients[i]].inuse){
                    if(g_entities[level.sortedClients[i]].client->sess.team == TEAM_RED && !G_IsClientDead(g_entities[level.sortedClients[i]].client) && g_entities[level.sortedClients[i]].client->pers.connected == CON_CONNECTED){
                        if(level.lastalive[0] == -1){
                            level.lastalive[0] = level.sortedClients[i];
                        }else{
                            level.lastalive[1] = level.sortedClients[i];
                            break;
                        }
                    }
                }
            }
            level.lastaliveCheck[0] = qtrue;
        }
        if(countred == 1 && level.lastaliveCheck[1] == qfalse){
            for(i = 0; i < level.numConnectedClients; i++){
                if (g_entities[level.sortedClients[i]].inuse){
                    if(g_entities[level.sortedClients[i]].client->sess.team == TEAM_RED && !G_IsClientDead(g_entities[level.sortedClients[i]].client) && g_entities[level.sortedClients[i]].client->pers.connected == CON_CONNECTED){
                        if(level.lastalive[0] == level.sortedClients[i]){
                            // leave as it is
                            break;
                        }else if(level.lastalive[1] == level.sortedClients[i]){
                            level.lastalive[1] = level.lastalive[0];
                            level.lastalive[0] = level.sortedClients[i];
                            break;
                        }else{
                            level.lastalive[0] = level.sortedClients[i];
                            break;
                        }
                    }
                }
            }

            if(g_entities[level.lastalive[0]].client){
                trap_SendServerCommand (-1, va("print\"^3[H&S] ^7%s is the last hider alive.\n\"", g_entities[level.lastalive[0]].client->pers.cleanName ));
                G_Broadcast("You are the last \\hider alive!", BROADCAST_GAME, &g_entities[level.lastalive[0]]);
            }
            level.lastaliveCheck[1] = qtrue;
        }
    }
    // Henk 22/01/10 -> Display seekers released message.
    trap_Cvar_VariableStringBuffer ( "mapname", level.mapname, MAX_QPATH );

    //g_autoEvenTeams.integer == 1
    if(level.time >= level.gametypeDelayTime-50 && level.aetdone == qfalse && level.gametypeStartTime >= 5000 && g_autoEvenTeams.integer == 1 && level.cagefight != qtrue){
        EvenTeams_HS(NULL, qtrue);
        level.aetdone = qtrue;
    }

    if(level.time >= level.gametypeDelayTime && level.messagedisplay == qfalse && level.gametypeStartTime >= 5000 && !level.crossTheBridge && level.cagefight != qtrue && level.time < level.gametypeRoundTime){
        G_Broadcast("\\Seekers released!", BROADCAST_GAME, NULL);
        Boe_GlobalSound(level.actionSoundIndex);

        // give nades to all players
        SetupOutfitting();
        //G_LogPrintf("Giving away briefcase...\n");
        if(TeamCount1(TEAM_BLUE) >= 1){
            for(i=0;i<=200;i++){
            random = irand(0, level.numConnectedClients);
            if(!g_entities[level.sortedClients[random]].inuse)
                continue;

                if(g_entities[level.sortedClients[random]].client->sess.team == TEAM_BLUE){
                    break;
                }
            }
            if(g_entities[level.sortedClients[random]].client){ // cvar update crash if client does not exist
                if(g_entities[level.sortedClients[random]].client->sess.team != TEAM_BLUE){
                    trap_SendServerCommand(-1, va("print\"^3[H&S] ^7Can't find any seeker.\n\""));
                }else{
                    if(level.lastseek != -1 && g_entities[level.lastseek].client && g_entities[level.lastseek].client->sess.team == TEAM_BLUE){
                        if(hideSeek_Extra.string[BRIEFCASE] == '1'){
                            G_RealSpawnGametypeItem ( BG_FindGametypeItem (0), g_entities[level.lastseek].r.currentOrigin, g_entities[level.lastseek].s.angles, qtrue );
                            trap_SendServerCommand(-1, va("print\"^3[H&S] ^7Briefcase given to round winner %s: %i kills last round.\n\"", g_entities[level.lastseek].client->pers.cleanName, level.rememberSeekKills));
                        }
                    }else{
                        if(hideSeek_Extra.string[BRIEFCASE] == '1'){
                            G_RealSpawnGametypeItem ( BG_FindGametypeItem (0), g_entities[level.sortedClients[random]].r.currentOrigin, g_entities[level.sortedClients[random]].s.angles, qtrue );
                            trap_SendServerCommand(-1, va("print\"^3[H&S] ^7Briefcase given at random to %s.\n\"", g_entities[level.sortedClients[random]].client->pers.cleanName));
                        }
                    }
                }
            }else{
                trap_SendServerCommand(-1, va("print\"^3[H&S] ^7Can't find any seeker.\n\""));
            }
        }else{
            if(hideSeek_Extra.string[BRIEFCASE] == '1'){
                spawnPoint = G_SelectRandomSpawnPoint ( TEAM_BLUE, NULL, qfalse );
                // Boe!Man 5/7/12: Fixing crash issue. The briefcase MUST have a location to spawn.
                if(spawnPoint){
                    G_RealSpawnGametypeItem ( BG_FindGametypeItem (0), spawnPoint->origin, spawnPoint->angles, qtrue );
                    trap_SendServerCommand(-1, va("print\"^3[H&S] ^7Not enough seekers: Briefcase was spawned in the blue base.\n\""));
                }else{
                    trap_SendServerCommand(-1, va("print\"^3[H&S] ^7The briefcase could not be spawned.\n\""));
                }
                //trap_SendServerCommand(-1, va("print\"^3[H&S] ^7Not enough seekers for briefcase to spawn.\n\""));
            }
        }
        level.messagedisplay = qtrue;
    }

    if(level.time > level.gametypeStartTime+10000 && level.messagedisplay1 == qfalse && level.gametypeStartTime >= 5000 && !level.crossTheBridge && level.cagefight != qtrue){
        if(hideSeek_Weapons.string[0] == '1'){
        rpgwinner = GetRpgWinner();
        if(rpgwinner != -1 && rpgwinner < 100){
            // Henk 26/01/10 -> Give RPG to player
            // Add it here aswell //attack = &weaponData[WP_MDN11_GRENADE].attack[ATTACK_NORMAL];
            g_entities[rpgwinner].client->ps.ammo[weaponData[WP_RPG7_LAUNCHER].attack[ATTACK_NORMAL].ammoIndex]=2;
            g_entities[rpgwinner].client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_RPG7_LAUNCHER );
            g_entities[rpgwinner].client->ps.clip[ATTACK_NORMAL][WP_RPG7_LAUNCHER]=1;
            g_entities[rpgwinner].client->ps.firemode[WP_RPG7_LAUNCHER] = BG_FindFireMode ( WP_RPG7_LAUNCHER, ATTACK_NORMAL, WP_FIREMODE_AUTO );
            g_entities[rpgwinner].client->ps.weapon = WP_KNIFE;
            g_entities[rpgwinner].client->ps.weaponstate = WEAPON_READY;
            Com_sprintf(level.RPGloc, sizeof(level.RPGloc), "%s", g_entities[rpgwinner].client->pers.netname);
            level.RPGent = -1;
            level.RPGTime = 0;
            trap_SendServerCommand(-1, va("print\"^3[H&S] ^7RPG given to round winner %s.\n\"", g_entities[rpgwinner].client->pers.cleanName));
            g_entities[rpgwinner].client->sess.takenRPG += 1;
            G_Broadcast("You now have the \\RPG!", BROADCAST_GAME, &g_entities[rpgwinner]);
            // End
        }else if(rpgwinner >= 100 && m4winner < 100){
                spawnPoint = G_SelectRandomSpawnPoint ( TEAM_BLUE, NULL, qfalse );
                // Boe!Man 5/7/12: Fixing crash issue. The RPG MUST have a location to spawn.
                if(spawnPoint){
                    dropped = G_DropItem2(spawnPoint->origin, spawnPoint->angles, BG_FindWeaponItem ( WP_RPG7_LAUNCHER ));
                    dropped->count  = 1&0xFF;
                    dropped->count += ((2<<8) & 0xFF00);
                    Com_sprintf(level.RPGloc, sizeof(level.RPGloc), "%s", "blue base");
                    level.RPGTime = level.time+1000;
                    level.RPGent = dropped->s.number;
                    trap_SendServerCommand(-1, va("print\"^3[H&S] ^7Not enough hiders connected: RPG spawned in blue base\n\""));
                }else{
                    trap_SendServerCommand(-1, va("print\"^3[H&S] ^7The RPG could not be spawned.\n\""));
                }
        }else{ // Henk 26/01/10 -> Drop RPG at red spawn.
                for(i=0;i<=200;i++){
                    random = irand(0, level.numConnectedClients);
                    if(!g_entities[level.sortedClients[random]].inuse)
                        continue;
                    if(g_entities[level.sortedClients[random]].client->sess.team != TEAM_RED)
                        continue;
                    if(g_entities[level.sortedClients[random]].client->pers.connected != CON_CONNECTED)
                        continue;
                    if(G_IsClientDead(g_entities[level.sortedClients[random]].client))
                        continue;
                    // Henk 26/01/10 -> Give RPG to player
                    g_entities[level.sortedClients[random]].client->ps.ammo[weaponData[WP_RPG7_LAUNCHER].attack[ATTACK_NORMAL].ammoIndex]=2;
                    g_entities[level.sortedClients[random]].client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_RPG7_LAUNCHER );
                    g_entities[level.sortedClients[random]].client->ps.clip[ATTACK_NORMAL][WP_RPG7_LAUNCHER]=1;
                    g_entities[level.sortedClients[random]].client->ps.firemode[WP_RPG7_LAUNCHER] = BG_FindFireMode ( WP_RPG7_LAUNCHER, ATTACK_NORMAL, WP_FIREMODE_AUTO );
                    g_entities[level.sortedClients[random]].client->ps.weapon = WP_KNIFE;
                    g_entities[level.sortedClients[random]].client->ps.weaponstate = WEAPON_READY;
                    Com_sprintf(level.RPGloc, sizeof(level.RPGloc), "%s", g_entities[level.sortedClients[random]].client->pers.netname);
                    level.RPGent = -1;
                    level.RPGTime = 0;
                    level.lastalive[0] = -1;
                    trap_SendServerCommand(-1, va("print\"^3[H&S] ^7RPG given at random to %s.\n\"", g_entities[level.sortedClients[random]].client->pers.cleanName));
                    g_entities[level.sortedClients[random]].client->sess.takenRPG += 1;
                    G_Broadcast("You now have the \\RPG!", BROADCAST_GAME, &g_entities[level.sortedClients[random]]);
                    break;
                }
            }
        }
        if(hideSeek_Weapons.string[1] == '1'){
        m4winner = GetM4Winner(rpgwinner);
        if(m4winner != -1 && m4winner < 100){
            // Henk 26/01/10 -> Give M4 to player
            g_entities[m4winner].client->ps.ammo[weaponData[WP_M4_ASSAULT_RIFLE].attack[ATTACK_ALTERNATE].ammoIndex]=2; // not 3 because 1 in clip
            g_entities[m4winner].client->ps.ammo[weaponData[WP_M4_ASSAULT_RIFLE].attack[ATTACK_NORMAL].ammoIndex]=2;
            g_entities[m4winner].client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_M4_ASSAULT_RIFLE);
            g_entities[m4winner].client->ps.clip[ATTACK_NORMAL][WP_M4_ASSAULT_RIFLE]=1;
            // Henk 01/02/10 -> Fix for reloading M203
            g_entities[m4winner].client->ps.clip[ATTACK_ALTERNATE][WP_M4_ASSAULT_RIFLE]=1;
            // End
            g_entities[m4winner].client->ps.firemode[WP_M4_ASSAULT_RIFLE] = BG_FindFireMode ( WP_M4_ASSAULT_RIFLE, ATTACK_NORMAL, WP_FIREMODE_SINGLE );
            g_entities[m4winner].client->ps.weapon = WP_KNIFE;
            g_entities[m4winner].client->ps.weaponstate = WEAPON_READY;
            // End
            Com_sprintf(level.M4loc, sizeof(level.M4loc), "%s", g_entities[m4winner].client->pers.netname);
            level.M4Time = 0;
            level.M4ent = -1;
            trap_SendServerCommand(-1, va("print\"^3[H&S] ^7M4 given to round winner %s.\n\"", g_entities[m4winner].client->pers.cleanName));
            g_entities[m4winner].client->sess.takenM4 += 1;
            G_Broadcast("You now have the \\M4!", BROADCAST_GAME, &g_entities[m4winner]);
            // End
        }else if(m4winner != 100){ // Henk 26/01/10 -> Drop M4 at red spawn.
            // Henk 24/02/10 -> Add randomize give away
            for(i=0;i<=200;i++){
                random = irand(0, level.numConnectedClients);
                if(!g_entities[level.sortedClients[random]].inuse)
                    continue;
                if(g_entities[level.sortedClients[random]].client->sess.team != TEAM_RED)
                    continue;
                if(g_entities[level.sortedClients[random]].client->pers.connected != CON_CONNECTED)
                    continue;
                if(G_IsClientDead(g_entities[level.sortedClients[random]].client))
                    continue;
                if(g_entities[level.sortedClients[random]].client->ps.stats[STAT_WEAPONS] & ( 1 << WP_RPG7_LAUNCHER )) // person already has RPG
                    continue;
                g_entities[level.sortedClients[random]].client->ps.ammo[weaponData[WP_M4_ASSAULT_RIFLE].attack[ATTACK_ALTERNATE].ammoIndex]=2; // not 3 because 1 in clip
                g_entities[level.sortedClients[random]].client->ps.ammo[weaponData[WP_M4_ASSAULT_RIFLE].attack[ATTACK_NORMAL].ammoIndex]=2;
                g_entities[level.sortedClients[random]].client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_M4_ASSAULT_RIFLE);
                g_entities[level.sortedClients[random]].client->ps.clip[ATTACK_NORMAL][WP_M4_ASSAULT_RIFLE]=1;
                // Henk 01/02/10 -> Fix for reloading M203
                g_entities[level.sortedClients[random]].client->ps.clip[ATTACK_ALTERNATE][WP_M4_ASSAULT_RIFLE]=1;
                // End
                g_entities[level.sortedClients[random]].client->ps.firemode[WP_M4_ASSAULT_RIFLE] = BG_FindFireMode ( WP_M4_ASSAULT_RIFLE, ATTACK_NORMAL, WP_FIREMODE_SINGLE );
                g_entities[level.sortedClients[random]].client->ps.weapon = WP_KNIFE;
                g_entities[level.sortedClients[random]].client->ps.weaponstate = WEAPON_READY;
                level.lastalive[1] = -1;
                // End
                Com_sprintf(level.M4loc, sizeof(level.M4loc), "%s", g_entities[level.sortedClients[random]].client->pers.netname);
                level.M4Time = 0;
                level.M4ent = -1;
                trap_SendServerCommand(-1, va("print\"^3[H&S] ^7M4 given at random to %s.\n\"", g_entities[level.sortedClients[random]].client->pers.cleanName));
                g_entities[level.sortedClients[random]].client->sess.takenM4 += 1;
                G_Broadcast("You now have the \\M4!", BROADCAST_GAME, &g_entities[level.sortedClients[random]]);
                break;
            }
        }else{
            spawnPoint = G_SelectRandomSpawnPoint ( TEAM_BLUE, NULL, qfalse );
            // Boe!Man 5/7/12: Fixing crash issue. The M4 MUST have a location to spawn.
            if(spawnPoint){
                dropped = G_DropItem2(spawnPoint->origin, spawnPoint->angles, BG_FindWeaponItem ( WP_M4_ASSAULT_RIFLE ));
                dropped->count  = 1&0xFF;
                dropped->count += ((2<<8) & 0xFF00);
                dropped->count += ((1 << 16) & 0xFF0000 );
                dropped->count += ((2 << 24) & 0xFF000000 );
                Com_sprintf(level.M4loc, sizeof(level.M4loc), "%s", "blue base");
                level.M4Time = level.time+1000;
                level.M4ent = dropped->s.number;
                trap_SendServerCommand(-1, va("print\"^3[H&S] ^7Not enough hiders connected: M4 spawned in blue base\n\""));
            }else{
                trap_SendServerCommand(-1, va("print\"^3[H&S] ^7The M4 could not be spawned.\n\""));
            }
        }
        }

        if(hideSeek_Extra.string[RANDOMGRENADE] == '1'){
            // The random name, aka transform nade, has only one go at spawning. If this fails, the nade is NOT given out at all.
            random = irand(0, level.numConnectedClients);

            if(g_entities[level.sortedClients[random]].inuse && g_entities[level.sortedClients[random]].client->sess.team == TEAM_RED && !G_IsClientDead(g_entities[level.sortedClients[random]].client)){
                int start_ammo_type;

                // The hider can get the nade now.
                trap_SendServerCommand(-1, va("print\"^3[H&S] ^7? grenade given at random to %s.\n\"", g_entities[level.sortedClients[random]].client->pers.cleanName));

                g_entities[level.sortedClients[random]].client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_M67_GRENADE );
                start_ammo_type = weaponData[WP_M67_GRENADE].attack[ATTACK_NORMAL].ammoIndex;
                g_entities[level.sortedClients[random]].client->ps.ammo[start_ammo_type] = weaponData[WP_M67_GRENADE].attack[ATTACK_NORMAL].clipSize;
                g_entities[level.sortedClients[random]].client->ps.clip[ATTACK_NORMAL][WP_M67_GRENADE] = weaponData[WP_M67_GRENADE].attack[ATTACK_NORMAL].clipSize;
                g_entities[level.sortedClients[random]].client->ps.firemode[WP_M67_GRENADE] = BG_FindFireMode ( WP_M67_GRENADE, ATTACK_NORMAL, WP_FIREMODE_AUTO );

                // alt-fire ammo
                start_ammo_type = weaponData[WP_M67_GRENADE].attack[ATTACK_ALTERNATE].ammoIndex;
                if (AMMO_NONE != start_ammo_type)
                {
                    g_entities[level.sortedClients[random]].client->ps.ammo[start_ammo_type] = ammoData[start_ammo_type].max;
                }

                Com_sprintf(level.RandomNadeLoc, sizeof(level.RandomNadeLoc), "%s", g_entities[level.sortedClients[random]].client->pers.netname);

                // Boe!Man 2/26/14: Broadcast all the weapons the random nade carrier got, he might have RPG/M4 as well.
                if(g_entities[level.sortedClients[random]].client->ps.stats[STAT_WEAPONS] & (1 << WP_RPG7_LAUNCHER)){
                    G_Broadcast("You now have the \\RPG & ? grenade!", BROADCAST_GAME, &g_entities[level.sortedClients[random]]);
                }else if(g_entities[level.sortedClients[random]].client->ps.stats[STAT_WEAPONS] & (1 << WP_M4_ASSAULT_RIFLE)){
                    G_Broadcast("You now have the \\M4 & ? grenade!", BROADCAST_GAME, &g_entities[level.sortedClients[random]]);
                }else{
                    G_Broadcast("You now have the ? \\grenade!", BROADCAST_GAME, &g_entities[level.sortedClients[random]]);
                }
            }else{
                // Boe!Man 2/20/14: Do set the location message if it's not given out this round.
                strncpy(level.RandomNadeLoc, "Not given this round", sizeof(level.RandomNadeLoc));
            }
        }
        level.messagedisplay1 = qtrue;
    }
}

/*
================
G_RunFrame

Advances the non-player objects in the world
================
*/
void G_RunFrame( int levelTime )
{
    int         i;
    gentity_t   *ent;
    int         msec;

    //NT - store the time the frame started
    level.frameStartTime = trap_Milliseconds();

    // if we are waiting for the level to restart, do nothing
    if ( level.restarted )
    {
        return;
    }
    // Or if we're waiting for the server to switch to a valid next map..
    else if ( level.mcSkipMaps )
    {
        if(level.mcKillServer && level.mcSkipMaps == 1){
            trap_SendConsoleCommand(EXEC_APPEND, "killserver; ");
        }
        trap_SendConsoleCommand(EXEC_APPEND, "mapcycle\n");

        level.mcSkipMaps--;
    }

    level.framenum++;
    level.previousTime = level.time;
    level.time = levelTime;
    msec = level.time - level.previousTime;

    //Ryan
    if(level.pause)
    {
        ///RxCxW - 08.30.06 - 03:33pm #paused - add to level.startTime so pausing wont take away from timelimit
        level.startTime += msec;
        if(level.gametypeRoundTime)
            level.gametypeRoundTime += msec;
        ///End  - 08.30.06 - 03:34pm

        if(level.unpausetime)
        {
            G_unPause(NULL);
        }
    }
    //Ryan

    // get any cvar changes
    G_UpdateCvars();


    #ifndef _GOLD
    if(level.clientMod == CL_RPM){
        // Henk 06/04/10 -> Update tmi every x sec
        RPM_UpdateTMI();
    }
    #else
    if (level.clientMod == CL_ROCMOD && level.time > level.lastETIupdate) {
        ROCmod_sendExtraTeamInfo(NULL);

        level.lastETIupdate = level.time + 1000;
    }
    #endif // not _GOLD

    // go through all allocated objects
    ent = &g_entities[0];
    for (i=0 ; i<level.num_entities ; i++, ent++)
    {
        if ( !ent->inuse )
        {
            continue;
        }

        // clear events that are too old
        if ( level.time - ent->eventTime > EVENT_VALID_MSEC )
        {
            if ( ent->s.event )
            {
                ent->s.event = 0;   // &= EV_EVENT_BITS;
                if ( ent->client )
                {
                    ent->client->ps.externalEvent = 0;
                    // predicted events should never be set to zero
                    //ent->client->ps.events[0] = 0;
                    //ent->client->ps.events[1] = 0;
                }
            }

            if ( ent->freeAfterEvent )
            {
                // tempEntities or dropped items completely go away after their event
                G_FreeEntity( ent );
                continue;
            }
            else if ( ent->unlinkAfterEvent )
            {
                // items that will respawn will hide themselves after their pickup event
                ent->unlinkAfterEvent = qfalse;
                trap_UnlinkEntity( ent );
            }
        }
                CheckEnts(ent);
        // temporary entities don't think
        if ( ent->freeAfterEvent )
        {
            continue;
        }

        if ( !ent->r.linked && ent->neverFree )
        {
            continue;
        }

        if ( ent->s.eType == ET_MISSILE )
        {
            G_RunMissile( ent );
            continue;
        }

        if ( ent->s.eType == ET_ITEM || ent->physicsObject )
        {
            G_RunItem( ent );
            continue;
        }

        if ( ent->s.eType == ET_MOVER )
        {
            G_RunMover( ent );
            continue;
        }

        if ( i < MAX_CLIENTS )
        {
            G_CheckClientTimeouts ( ent );
            G_RunClient( ent );
            #ifndef _DEMO
            G_CheckClientTeamkill ( ent );
            #endif // not _DEMO
            continue;
        }

        G_RunThink( ent );

    }

    // perform final fixups on the players
    ent = &g_entities[0];
    for (i=0 ; i < level.maxclients ; i++, ent++ )
    {
        if(current_gametype.value == GT_CTF){
            if(ent->client->sess.pausespawn == qtrue && !level.pause){
                //FIX ME: Add custom respawn message(eg. Respawn in x sec), original will be frozen on 0
                if(level.time > ent->client->ps.RealSpawnTimer){
                    ent->client->sess.pausespawn = qfalse;
                    G_StopFollowing ( ent );
                    ent->client->ps.pm_flags &= ~PMF_GHOST;
                    ent->client->ps.pm_type = PM_NORMAL;
                    ent->client->sess.ghost = qfalse;
                    trap_UnlinkEntity (ent);
                    ClientSpawn(ent);
                }
            }
        }

        if(ent->client->sess.team != TEAM_SPECTATOR && ent->client->sess.zombie == qtrue && ent->client->sess.zombiebody != -1 && current_gametype.value == GT_HZ){
                if(g_entities[ent->client->sess.zombiebody].s.pos.trType == TR_STATIONARY || level.time > g_entities[ent->client->sess.zombiebody].timestamp + 5000){
                    SetTeam(ent, "blue", NULL, qtrue);
                    G_StopFollowing ( ent );
                    ent->client->ps.pm_flags &= ~PMF_GHOST;
                    ent->client->ps.pm_type = PM_NORMAL;
                    ent->client->sess.ghost = qfalse;
                    trap_UnlinkEntity (ent);
                    ClientSpawn(ent);

                    // Boe!Man 11/16/15: Only teleport if their new location was stationary.
                    if(g_entities[ent->client->sess.zombiebody].s.pos.trType == TR_STATIONARY){
                        TeleportPlayer(ent, g_entities[ent->client->sess.zombiebody].r.currentOrigin, ent->client->sess.tempangles, qtrue);
                        SetClientViewAngle(ent, ent->client->sess.tempangles);
                    }

                    g_entities[ent->client->sess.zombiebody].nextthink = level.time+1000;
                    g_entities[ent->client->sess.zombiebody].think = G_FreeEntity;
                    ent->client->sess.zombie = qfalse;
                    ent->client->sess.zombiebody = -1;
                }
            }

        if(current_gametype.value == GT_HS || current_gametype.value == GT_HZ){
            // Henk 27/02/10 -> Fix for dead ppl frozen
            if(G_IsClientDead(ent->client) && ent->client->ps.stats[STAT_FROZEN])
                ent->client->ps.stats[STAT_FROZEN] = 0;
            // End

            // Henk 21/01/10 -> Check for dead seekers
            if(ent->client->sess.team != TEAM_SPECTATOR && G_IsClientDead(ent->client) == qtrue && level.cagefight == qfalse && level.cagefightdone != qtrue && current_gametype.value != GT_HZ){
                if(ent->client->sess.team == TEAM_BLUE || (level.time < (level.gametypeStartTime+hideSeek_roundstartdelay.integer*1000) && ent->client->sess.team == TEAM_RED)){
                    // If the client is a ghost then revert them
                    if ( ent->client->sess.ghost )
                    {
                        // Clean up any following monkey business
                        G_StopFollowing ( ent );

                        // Disable being a ghost
                        ent->client->ps.pm_flags &= ~PMF_GHOST;
                        ent->client->ps.pm_type = PM_NORMAL;
                        ent->client->sess.ghost = qfalse;
                    }

                ent->client->sess.noTeamChange = qfalse;

                trap_UnlinkEntity (ent);
                ClientSpawn ( ent );
                }
            }else if(current_gametype.value == GT_HZ){
                int teamCountBlue = TeamCount1(TEAM_BLUE);

                if (G_IsClientDead(ent->client)){
                    if (ent->client->sess.team == TEAM_BLUE || (teamCountBlue == 0 && ent->client->sess.team == TEAM_RED)){
                        if (ent->client->sess.zombie == qfalse){
                            if (ent->client->sess.ghost){
                                // Clean up any following monkey business
                                G_StopFollowing(ent);

                                // Disable being a ghost
                                ent->client->ps.pm_flags &= ~PMF_GHOST;
                                ent->client->ps.pm_type = PM_NORMAL;
                                ent->client->sess.ghost = qfalse;
                            }

                            ent->client->sess.noTeamChange = qfalse;

                            trap_UnlinkEntity(ent);
                            ClientSpawn(ent);
                        }
                    }
                }else if (ent->client->sess.regentime && ent->client->sess.team == TEAM_BLUE && level.time >= ent->client->sess.regentime){
                    // Boe!Man 10/28/14: Auto health renegeration.
                    ent->client->ps.stats[STAT_HEALTH]++;
                    ent->health++;

                    if (ent->client->ps.stats[STAT_HEALTH] >= MAX_HEALTH){
                        ent->client->sess.regentime = 0;
                    }else{
                        // Based on the current clients.
                        ent->client->sess.regentime = level.time + 50 + (150 / (TeamCount1(TEAM_RED) + teamCountBlue) * teamCountBlue);
                    }
                }
            }
        }
        if ( ent->inuse )
        {
            ClientEndFrame( ent );
        }
    }

    if ( server_enableServerMsgs.integer && level.time > level.serverMsg && (level.time - level.startTime >= 20000))
        Boe_serverMsg();
    if(server_enableTips.integer && level.time > level.tipMsg && (level.time - level.startTime >= 20000))
        Henk_Tip();

    if(level.custom == qtrue){ // apply custom commands with a small delay
        if(level.time >= level.customtime){
        level.custom = qfalse;
        trap_SendConsoleCommand( EXEC_APPEND, va("%s\n", level.action) );
        }
    }

    // Boe!Man 1/17/16: Try to kill the server from time to time, a week to be exact.
    // We make sure no actual players are connected to the server.
    // If we don't do this the server will kill the server anyway around the 23-day mark,
    // this is just safer and way more user friendly than just kicking everybody.
    if(level.time > 604800000 && level.checkServerShutdown != -1 && level.time > level.checkServerShutdown){
        if(level.numConnectedClients == 0 || NumBots() == level.numConnectedClients){
            if (*g_mapcycle.string && Q_stricmp (g_mapcycle.string, "none")){
                level.mcKillServer = qtrue;
                G_switchToNextMapInCycle(qtrue);
            }else{
                // 'mapcycle' should restart the current map since there is none.
                trap_SendConsoleCommand( EXEC_APPEND, "killserver; mapcycle\n");
            }

            // Make sure we don't process this again.
            level.checkServerShutdown = -1;
        }else{
            // Check again the next second.
            level.checkServerShutdown = level.time + 1000;
        }
    }

    // Boe!Man 5/27/13: The automatic in-memory to disk backup. Note this only happens when playing a game which doesn't have a round limit.
    if(level.sqlBackupTime && level.time >= level.sqlBackupTime){
        Boe_backupInMemoryDbs("users.db", usersDb);
        Boe_backupInMemoryDbs("aliases.db", aliasesDb);
        Boe_backupInMemoryDbs("bans.db", bansDb);

        // Do this again in the next 5 minutes.
        level.sqlBackupTime = level.time + 50000;
    }

    // Boe!Man 11/2/10: New Map Switch/Restart system.
    if (level.mapSwitch == qtrue /* && level.mapSwitchCount == level.time */){
        Boe_mapEvents();
    }

    // Boe!Man 11/16/10: Is compmode enabled?
    if (g_compMode.integer > 0){
        // Boe!Man 11/16/10: Keep displaying the scrim settings.
        if(cm_enabled.integer == 1 && level.compMsgCount < level.time){ // Boe!Man 3/19/11: Fixing possible shortcome. High pingers might not always receive them properly this way..
            char *sl;
            char *as;
            char *ds;
            char *dr;
            char color[4];

            if (server_colors.string != NULL && strlen(server_colors.string) >= 3){
                strncpy(color, va("^%c", server_colors.string[3]), sizeof(color));
            }
            else{
                // Always have some sort of backup in case no colors are set (highly unlikely though).
                strncpy(color, "^1", sizeof(color));
            }

            if (strstr(cm_slock.string, "1"))
                sl = "Yes";
            else
                sl = "No";
            if (strstr(cm_aswap.string, "1"))
                as = "Yes";
            else
                as = "No";
            if (strstr(cm_devents.string, "1"))
                ds = "Yes";
            else
                ds = "No";
            if (strstr(cm_dr.string, "1"))
                dr = "Two";
            else
                dr = "One";

            if(dr == "Two"){ // Boe!Man 3/18/11: Display the Auto Swap setting when Dual Rounds are enabled.
                G_Broadcast(va("%sMatch settings\n\n^7[^3Gametype^7] %s%s %s\n^7[^3Scorelimit^7]  %s%i\n^7[^3Timelimit^7]  %s%i\n^7[^3Specs locked^7] %s%s\n^7[^3Disable events^7] %s%s\n^7[^3# of Rounds^7] %s%s\n^7[^3Auto swap^7] %s%s\n\n%sRestart map to start the first round!",
                    color, color, level.mapname, g_gametype.string, color, cm_sl.integer, color, cm_tl.integer, color, sl, color, ds, color, dr, color, as, color), BROADCAST_GAME, NULL);
            }else{ // Boe!Man 3/18/11: Hide it when it's disabled.
                G_Broadcast(va("%sMatch settings\n\n^7[^3Gametype^7] %s%s %s\n^7[^3Scorelimit^7]  %s%i\n^7[^3Timelimit^7]  %s%i\n^7[^3Specs locked^7] %s%s\n^7[^3Disable events^7] %s%s\n^7[^3# of Rounds^7] %s%s\n\n%sRestart map to start the first round!",
                    color, color, level.mapname, g_gametype.string, color, cm_sl.integer, color, cm_tl.integer, color, sl, color, ds, color, dr, color), BROADCAST_GAME, NULL);
            }
            level.compMsgCount = level.time + 3000;
        }else if(cm_enabled.integer == 3 && level.compMsgCount < level.time){ // Boe!Man 3/19/11: Fixing possible shortcome. High pingers might not always receive them properly this way..
            char color[4];

            if (server_colors.string != NULL && strlen(server_colors.string) >= 3){
                strncpy(color, va("^%c", server_colors.string[3]), sizeof(color));
            }else{
                // Always have some sort of backup in case no colors are set (highly unlikely though).
                strncpy(color, "^1", sizeof(color));
            }

            // Boe!Man 1/24/11: Swap the teams.
            if (cm_aswap.integer > 0 && level.swappedteams == qfalse){
                SwapTeams(NULL, qtrue);
                for(i=0;i<level.numConnectedClients;i++){ // Henk swap the invites aswell
                    if(level.clients[level.sortedClients[i]].sess.invitedByRed == qtrue){
                        level.clients[level.sortedClients[i]].sess.invitedByRed = qfalse;
                        level.clients[level.sortedClients[i]].sess.invitedByBlue = qtrue;
                    }else if(level.clients[level.sortedClients[i]].sess.invitedByBlue == qtrue){
                        level.clients[level.sortedClients[i]].sess.invitedByRed = qtrue;
                        level.clients[level.sortedClients[i]].sess.invitedByBlue = qfalse;
                    }
                }
                level.swappedteams = qtrue;
            }
            if (cm_sr.integer > cm_sb.integer){
                G_Broadcast(va("%sFirst round ended!\n\n^7[^3Red team^7] %sleads with %i - %i\n\n%sRestart map to start the second round!",
                    color, color, cm_sr.integer, cm_sb.integer, color), BROADCAST_GAME, NULL);
            }
            else if (cm_sb.integer > cm_sr.integer){
                G_Broadcast(va("%sFirst round ended!\n\n^7[^3Blue team^7] %sleads with %i - %i\n\n%sRestart map to start the second round!",
                    color, color, cm_sb.integer, cm_sr.integer, color), BROADCAST_GAME, NULL);
            }
            else{ // Boe!Man 3/19/11: It could be a round draw with the timelimit.
                G_Broadcast(va("%sFirst round ended!\n\n^7[^3Round draw^7] %swith %i - %i\n\n%sRestart map to start the second round!",
                    color, color, cm_sb.integer, cm_sr.integer, color), BROADCAST_GAME, NULL);
            }

            level.compMsgCount = level.time + 3000;
        }
    }

    // Check warmup rules
    CheckWarmup();

    // Boe!Man 2/10/17: Check if it's nearly time to end the level.
    checkExitTimes();

    // see if it is time to end the level
    CheckExitRules();

    // Update gametype stuff
    CheckGametype ();

    trap_GT_RunFrame ( level.time );

    // cancel vote if timed out
    vote_checkVote_f();

    // for tracking changes
    CheckCvars();

    // Henk 21/01/10 -> Hide&Seek actions
    if(current_gametype.value == GT_HS)
        Henk_CheckHS();
    if(current_gametype.value == GT_HZ)
        Henk_CheckZombie();

    if (g_listEntity.integer)
    {
        for (i = 0; i < MAX_GENTITIES; i++)
        {
            Com_Printf("%4i: %s\n", i, g_entities[i].classname);
        }
        trap_Cvar_Set("g_listEntity", "0");
    }
}

void G_InitGhoul ( void )
{
    G_InitHitModels ( );
}

void G_ShutdownGhoul ( void )
{
    if ( !level.serverGhoul2 )
    {
        return;
    }

    trap_G2API_CleanGhoul2Models ( &level.serverGhoul2 );
    level.serverGhoul2 = NULL;
}

/*
================
Boe_setTrackedCvar
6/30/11 - 6:47 PM
Purpose: Set a Tracked CVAR without track message (useful for Admin commands and such).
================
*/

void Boe_setTrackedCvar(vmCvar_t *cvar, int value)
{
    cvarTable_t *cv;
    int i, num;
    cv = gameCvarTable;

    for (i = 0; i < gameCvarTableSize; i++){
        if (cv[i].vmCvar == cvar){
            num = i;
            break;
        }
    }

    if (num == -1){
        return;
    }

    cv[num].trackChange = qfalse;
    trap_Cvar_Set(cv[num].cvarName, va("%i", value));
    G_UpdateCvars(); // Boe!Man 6/30/11: Update it manually this time.
    cv[num].trackChange = qtrue;
}

/*
================
G_CheckAlive
3/14/14 - 11:15 PM
Purpose: Checks if the server is alive at this moment.
================
*/

qboolean G_CheckAlive(void)
{
    #ifdef __linux__
    struct flock fl;
    int fdlock;

    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 1;

    if((fdlock = open("srv.lck", O_WRONLY|O_CREAT, 0666)) == -1){
        return qtrue;
    }

    if(fcntl(fdlock, F_SETLK, &fl) == -1){
        return qtrue;
    }
    #elif WIN32
    char fsGame[MAX_QPATH];
    BOOL success;

    trap_Cvar_VariableStringBuffer("fs_game", fsGame, sizeof(fsGame));

    // Boe!Man 3/20/14: Open the lock file.
    lockFile = CreateFile(TEXT(va("%s\\srv.lck", fsGame)), GENERIC_WRITE, 0, NULL, CREATE_NEW, 0, NULL);

    if (lockFile == INVALID_HANDLE_VALUE){
        return qtrue;
    }

    // Lock the whole file.
    success = LockFile(lockFile, 0, 0, 0xffffffff, 0xffffffff);
    if(!success){
        return qtrue;
    }
    #endif

    return qfalse;
}
