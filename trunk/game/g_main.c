// Copyright (C) 2001-2002 Raven Software.
//

#include "g_local.h"
#include "boe_local.h"

level_locals_t	level;

typedef struct
{
	vmCvar_t	*vmCvar;
	char		*cvarName;
	char		*defaultString;
	int			cvarFlags;
	float		mMinValue, mMaxValue;
	int			modificationCount;  // for tracking changes
	qboolean	trackChange;	    // track this variable, and announce if changed
	qboolean	teamShader;			// track and if changed, update shader state

} cvarTable_t;

gentity_t		g_entities[MAX_GENTITIES];
gclient_t		g_clients[MAX_CLIENTS];

vmCvar_t	g_gametype;
vmCvar_t	g_dmflags;
vmCvar_t	g_scorelimit;
vmCvar_t	g_timelimit;
vmCvar_t	g_friendlyFire;
vmCvar_t	g_password;
vmCvar_t	g_needpass;
vmCvar_t	g_maxclients;
vmCvar_t	g_maxGameClients;
vmCvar_t	g_dedicated;
vmCvar_t	g_speed;
vmCvar_t	g_gravity;
vmCvar_t	g_cheats;
vmCvar_t	g_knockback;
vmCvar_t	g_forcerespawn;
vmCvar_t	g_inactivity;
vmCvar_t	g_debugMove;
vmCvar_t	g_debugDamage;
vmCvar_t	g_debugAlloc;
vmCvar_t	g_weaponRespawn;
vmCvar_t	g_backpackRespawn;
vmCvar_t	g_motd;
vmCvar_t	g_synchronousClients;
vmCvar_t	g_warmup;
vmCvar_t	g_doWarmup;
vmCvar_t	g_restarted;
vmCvar_t    current_gametype;
vmCvar_t	g_log;
vmCvar_t	g_logSync;
vmCvar_t	g_logHits;
vmCvar_t	g_allowVote;
vmCvar_t	g_voteDuration;
vmCvar_t	g_failedVoteDelay;
vmCvar_t	g_teamAutoJoin;
vmCvar_t	g_teamForceBalance;
vmCvar_t	g_banIPs;
vmCvar_t	g_filterBan;
vmCvar_t	g_smoothClients;
vmCvar_t	pmove_fixed;
vmCvar_t	pmove_msec;
vmCvar_t	g_rankings;
vmCvar_t	g_listEntity;
vmCvar_t	g_fps;
vmCvar_t	g_respawnInterval;
vmCvar_t	g_respawnInvulnerability;
vmCvar_t	g_roundtimelimit;
vmCvar_t	g_timeextension;
vmCvar_t	g_timeouttospec;
vmCvar_t	g_roundstartdelay;
vmCvar_t	g_availableWeapons;
vmCvar_t	hideSeek_availableWeapons;
vmCvar_t	availableWeapons;
vmCvar_t	g_forceFollow;
vmCvar_t	g_followEnemy;
vmCvar_t	g_mapcycle;
vmCvar_t	g_pickupsDisabled;				// Whether or not pickups are available in a map (uses outfitting if not)
vmCvar_t	g_suicidePenalty;				// Amount of score added for killing yourself (typically negative)
vmCvar_t	g_teamkillPenalty;				// Amount of score added for killing a teammates (typically negative)
vmCvar_t	g_teamkillDamageMax;			// max damage one can do to teammates before being kicked
vmCvar_t	g_teamkillDamageForgive;		// amount of teamkill damage forgiven each minute
vmCvar_t	g_voiceFloodCount;				// Number of voice messages in one minute to be concidered flooding
vmCvar_t	g_voiceFloodPenalty;			// Amount of time a void flooder must wait before they can use voice again

vmCvar_t	RMG;
vmCvar_t	g_debugRMG;

// Boe!Man 3/30/10
vmCvar_t	server_motd1;					// Server message 1-6.
vmCvar_t	server_motd2;					// Used for displaying the Motd when the player connects,
vmCvar_t	server_motd3;					// or decides to view it using the /motd command.
vmCvar_t	server_motd4;
vmCvar_t	server_motd5;
vmCvar_t	server_motd6;
vmCvar_t	g_adminfile;					// Stores the admins in the server.
vmCvar_t	g_addbadmin;					// Adds a B-Admin to the Admin file.
vmCvar_t	g_addadmin;						// Adds an Admin to the Admin file.
vmCvar_t	g_addsadmin;					// Adds a S-Admin to the Admin file.
vmCvar_t	g_kick;							// Admin CVAR.
vmCvar_t	g_subnetban;					// Admin CVAR.
vmCvar_t	g_ban;							// Admin CVAR.
vmCvar_t	g_broadcast;
vmCvar_t	g_removeadmin;					// Admin CVAR.
vmCvar_t	server_color1;					// Server color 1-6.
vmCvar_t	server_color2;					// Used for displaying colors.
vmCvar_t	server_color3;					// In either commands or prefixes.
vmCvar_t	server_color4;
vmCvar_t	server_color5;
vmCvar_t	server_color6;
vmCvar_t	g_uppercut;						// Admin CVAR.
vmCvar_t	g_runover;						// Admin CVAR.
vmCvar_t	g_twist;						// Admin CVAR.
vmCvar_t	g_respawn;						// Admin CVAR.
vmCvar_t	server_badminprefix;
vmCvar_t	server_adminprefix;
vmCvar_t	server_sadminprefix;
vmCvar_t	server_acprefix;
vmCvar_t	server_sacprefix;
vmCvar_t	server_caprefix;
vmCvar_t	server_ccprefix;
vmCvar_t	server_starprefix;
vmCvar_t	g_mapswitch;					// Admin CVAR.
vmCvar_t	g_flash;						// Admin CVAR.
vmCvar_t	g_plant;						// Admin CVAR.
vmCvar_t	g_pop;							// Admin CVAR.
vmCvar_t	g_burn;							// Admin CVAR.
vmCvar_t	g_mute;							// Admin CVAR.
vmCvar_t	g_strip;						// Admin CVAR.
vmCvar_t	g_eventeams;					// Admin CVAR.
vmCvar_t	g_maxIPConnections;
//vmCvar_t	g_333;							// Admin CVAR.
vmCvar_t	g_forceteam;					// Admin CVAR.
vmCvar_t	Owner;
vmCvar_t	Clan;
vmCvar_t	ClanURL;
// Boe!Man 4/1/10
vmCvar_t	server_redteamprefix;
vmCvar_t	server_blueteamprefix;
// Boe!Man 1/19/11
vmCvar_t	server_hiderteamprefix;
vmCvar_t	server_seekerteamprefix;
// Henk 01/04/10 -> Add g_disablenades
vmCvar_t	g_disablenades;
vmCvar_t	g_instagib;
vmCvar_t	g_weaponModFlags;
vmCvar_t	g_allowthirdperson;
vmCvar_t	g_weaponswitch;
vmCvar_t	g_compMode;	
vmCvar_t	g_clanfile;
vmCvar_t	g_enableTeamCmds;
vmCvar_t	g_refpassword;
vmCvar_t	g_checkcountry;
vmCvar_t	g_disablelower;

// Boe!Man 4/15/10: Some level commands (Admin).
vmCvar_t	g_nolower;
vmCvar_t	g_nades;
vmCvar_t	g_sl;
vmCvar_t	g_tl;
vmCvar_t	g_ri;
vmCvar_t	g_damage;
vmCvar_t	g_gr;
vmCvar_t	g_clanvsall;
vmCvar_t	g_swapteams;
vmCvar_t	g_lock;
vmCvar_t	g_clan;

// Boe!Man 6/2/10
vmCvar_t	g_autoeventeams;
vmCvar_t	server_msgInterval;
vmCvar_t	server_enableServerMsgs;
vmCvar_t	server_message1;
vmCvar_t	server_message2;
vmCvar_t	server_message3;
vmCvar_t	server_message4;
vmCvar_t	server_message5;
vmCvar_t	server_msgDelay;

// Boe!Man 10/16/10
vmCvar_t	g_adminspec;

// Boe!Man 11/16/10: Default scrim settings.
vmCvar_t	g_autoLockSpec;
vmCvar_t	g_autoSwapTeams;
vmCvar_t	g_matchTimeLimit;
vmCvar_t	g_matchScoreLimit;
vmCvar_t	g_matchDisableSounds;
vmCvar_t	cm_enabled;
vmCvar_t	cm_sl;
vmCvar_t	cm_tl;
vmCvar_t	cm_slock;
vmCvar_t	cm_aswap;
vmCvar_t	cm_dsounds;
vmCvar_t	cm_oldsl;
vmCvar_t	cm_oldtl;
vmCvar_t	cm_sr;
vmCvar_t	cm_sb;

// Boe!Man 12/13/10: Clonecheck CVARs.
vmCvar_t	g_aliasCheck;
vmCvar_t	g_alternateMap;

// Boe!Man 1/26/11
vmCvar_t	g_cm;
vmCvar_t	g_3rd;
vmCvar_t	g_enableCustomCommands;
vmCvar_t	g_rpmEnt;

#ifdef _BOE_DBG
vmCvar_t	boe_log;
#endif

static cvarTable_t gameCvarTable[] = 
{
	// don't override the cheat state set by the system
	{ &g_cheats, "sv_cheats", "", 0, 0.0, 0.0, 0, qfalse },
	{ NULL, "^3Mod Name", INF_STRING, CVAR_SERVERINFO | CVAR_ROM, 0.0, 0.0, 0, qfalse  },
	{ NULL, "^3Mod Version", INF_VERSION_STRING, CVAR_SERVERINFO | CVAR_ROM, 0.0, 0.0, 0, qfalse  },
	{ NULL, "^3Mod URL", "1fx.uk.to", CVAR_SERVERINFO | CVAR_ROM, 0.0, 0.0, 0, qfalse  },
	{ &current_gametype, "current_gametype", "3", CVAR_SERVERINFO | CVAR_ROM | CVAR_LATCH | CVAR_INTERNAL | CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
	{ NULL, "modname", "RPM 2 k 3 v1.71 ^_- ^31fx.uk.to", CVAR_SERVERINFO | CVAR_ROM, 0.0, 0.0, 0, qfalse  },

	// noset vars
	{ NULL, "gamename", GAMEVERSION , CVAR_SERVERINFO | CVAR_ROM, 0.0, 0.0, 0, qfalse  },
	{ NULL, "gamedate", __DATE__ , CVAR_ROM, 0.0, 0.0, 0, qfalse  },
	{ &g_restarted, "g_restarted", "0", CVAR_ROM, 0.0, 0.0, 0, qfalse  },
	{ NULL, "sv_mapname", "", CVAR_SERVERINFO | CVAR_ROM, 0.0, 0.0, 0, qfalse  },

	{ &g_fps, "sv_fps", "", CVAR_ROM, 0.0, 0.0, 0, qfalse },

	// latched vars
	{ &g_gametype, "g_gametype", "dm", CVAR_SERVERINFO | CVAR_LATCH, 0.0, 0.0, 0, qfalse  }, 

	{ &g_maxclients, "sv_maxclients", "8", CVAR_SERVERINFO | CVAR_CHEAT | CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
	{ &g_maxGameClients, "g_maxGameClients", "0", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },

	{ &g_dmflags, "dmflags", "8", CVAR_SERVERINFO | CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue  },
	{ &g_scorelimit, "scorelimit", "0", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0.0, 0.0, 0, qtrue },
	{ &g_timelimit, "timelimit", "0", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0.0, 0.0, 0, qtrue },

	{ &g_synchronousClients, "g_synchronousClients", "0", CVAR_SYSTEMINFO, 0.0, 0.0, 0, qfalse  },

	{ &g_friendlyFire, "g_friendlyFire", "0", CVAR_SERVERINFO|CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue  },

	{ &g_teamAutoJoin, "g_teamAutoJoin", "0", CVAR_ARCHIVE, 0.0, 0.0,   },
	{ &g_teamForceBalance, "g_teamForceBalance", "1", CVAR_ARCHIVE, 0.0, 0.0,   },

	{ &g_warmup, "g_warmup", "20", CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue  },
	{ &g_doWarmup, "g_doWarmup", "0", 0, 0.0, 0.0, 0, qtrue  },
	{ &g_log, "g_log", "games.log", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
	{ &g_logSync, "g_logSync", "0", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
	{ &g_logHits, "g_logHits", "0", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },

	{ &g_password, "g_password", "", CVAR_USERINFO, 0.0, 0.0, 0, qfalse  },

	{ &g_banIPs, "g_banIPs", "", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
	{ &g_filterBan, "g_filterBan", "1", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },

	{ &g_needpass, "g_needpass", "0", CVAR_SERVERINFO | CVAR_ROM, 0.0, 0.0, 0, qfalse },

	{ &g_dedicated, "dedicated", "0", 0, 0.0, 0.0, 0, qfalse  },

	{ &g_speed, "g_speed", "300", 0, 0.0, 0.0, 0, qtrue  },
	{ &g_gravity, "g_gravity", "800", 0, 0.0, 0.0, 0, qtrue  },
	{ &g_knockback, "g_knockback", "700", 0, 0.0, 0.0, 0, qtrue  },
	{ &g_weaponRespawn, "g_weaponrespawn", "15", 0, 0.0, 0.0, 0, qtrue  },
	{ &g_backpackRespawn, "g_backpackrespawn", "40", 0, 0.0, 0.0, 0, qtrue  },
	{ &g_forcerespawn, "g_forcerespawn", "20", 0, 0.0, 0.0, 0, qtrue },
	{ &g_inactivity, "g_inactivity", "180", CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue },
	{ &g_debugMove, "g_debugMove", "0", 0, 0.0, 0.0, 0, qfalse },
	{ &g_debugDamage, "g_debugDamage", "0", 0, 0.0, 0.0, 0, qfalse },
	{ &g_debugAlloc, "g_debugAlloc", "0", 0, 0.0, 0.0, 0, qfalse },
	{ &g_motd, "g_motd", "", 0, 0.0, 0.0, 0, qfalse },

	{ &g_allowVote, "g_allowVote", "0", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
	{ &g_voteDuration, "g_voteDuration", "60", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
	{ &g_failedVoteDelay, "g_failedVoteDelay", "1", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
	{ &g_listEntity, "g_listEntity", "0", 0, 0.0, 0.0, 0, qfalse },

	{ &g_smoothClients, "g_smoothClients", "1", 0, 0.0, 0.0, 0, qfalse},
	{ &pmove_fixed, "pmove_fixed", "0", CVAR_SYSTEMINFO, 0.0, 0.0, 0, qfalse},
	{ &pmove_msec, "pmove_msec", "8", CVAR_SYSTEMINFO, 0.0, 0.0, 0, qfalse},

	{ &g_rankings, "g_rankings", "0", 0, 0.0, 0.0, 0, qfalse},

	{ &g_respawnInterval, "g_respawnInterval", "15", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
	{ &g_respawnInvulnerability, "g_respawnInvulnerability", "5", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },

	{ &RMG, "RMG", "0", 0, 0.0, 0.0, },
	{ &g_debugRMG, "g_debugRMG", "0", 0, 0.0f, 0.0f },

	{ &g_timeouttospec,		"g_timeouttospec",	"15",		CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
	{ &g_roundtimelimit,	"g_roundtimelimit",	"5",		CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
	{ &g_timeextension,		"g_timeextension",	"15",		CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },

	{ &g_roundstartdelay,	"g_roundstartdelay", "5",		CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },

	{ &g_availableWeapons,	"g_availableWeapons", "2222222222211", CVAR_ARCHIVE|CVAR_SERVERINFO|CVAR_LATCH|CVAR_INTERNAL, 0.0, 0.0, 0, qfalse },
	{ &hideSeek_availableWeapons,	"hideSeek_availableWeapons", "200000000000022222222", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse },
	{ &availableWeapons,	"availableWeapons", "2222222222211", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse },
	// Henk 01/04/10
	{ &g_disablenades,	"g_disablenades", "1", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
	// End
	{ &g_forceFollow,		"g_forceFollow",	 "1",			  CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
	{ &g_followEnemy,		"g_followEnemy",	 "1",			  CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },

	{ &g_mapcycle,			"sv_mapcycle",		 "none",		CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },

	{ &g_pickupsDisabled,	"g_pickupsDisabled", "0",					CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse },

	{ &g_suicidePenalty,	"g_suicidePenalty",  "-1",					CVAR_ARCHIVE,	0.0f,	0.0f,	0,	qfalse },

	{ &g_voiceFloodCount,	"g_voiceFloodCount",	"6",				CVAR_ARCHIVE,	0.0f,	0.0f,	0,  qfalse },
	{ &g_voiceFloodPenalty,	"g_voiceFloodPenalty",	"60",				CVAR_ARCHIVE,	0.0f,	0.0f,	0,  qfalse },

	{ &g_teamkillPenalty,		"g_teamkillPenalty",		"-1",		CVAR_ARCHIVE,	0.0f,	0.0f,	0,	qfalse },
	{ &g_teamkillDamageMax,		"g_teamkillDamageMax",		"300",		CVAR_ARCHIVE,	0.0f,	0.0f,	0,  qfalse },
	{ &g_teamkillDamageForgive,	"g_teamkillDamageForgive",	"50",		CVAR_ARCHIVE,	0.0f,	0.0f,	0,  qfalse },

	// Boe!Man 3/30/10
	{ &server_motd1, "server_motd1", "", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
	{ &server_motd2, "server_motd2", "", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
	{ &server_motd3, "server_motd3", "", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
	{ &server_motd4, "server_motd4", "", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
	{ &server_motd5, "server_motd5", "", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
	{ &server_motd6, "server_motd6", "", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },

	{ &g_adminfile,			"g_adminfile", "users/admins.txt", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },

	// Boe!Man 3/30/10: --- ADMIN--- Updated: 3/30/10
	{ &g_kick,						"g_kick",				"3",				CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
	{ &g_addbadmin,					"g_addbadmin",          "3",				CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
	{ &g_addadmin,					"g_addadmin",           "4",				CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
	{ &g_addsadmin,					"g_addsadmin",          "5",				CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
	{ &g_ban,						"g_ban",				"4",				CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
	{ &g_broadcast,					"g_broadcast",			"3",				CVAR_ARCHIVE,   0.0f,   0.0f, 0,  qfalse },
	{ &g_subnetban,					"g_subnetban",			"4",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
	{ &g_removeadmin,				"g_removeadmin",		"4",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
	{ &g_uppercut,					"g_uppercut",			"3",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
	{ &g_runover,					"g_runover",			"2",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
	{ &g_twist,						"g_twist",				"2",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
	{ &g_respawn,					"g_respawn",			"4",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
	{ &g_mapswitch,					"g_mapswitch",			"4",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
	{ &g_flash,						"g_flash",				"3",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
	{ &g_plant,						"g_plant",				"2",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
	{ &g_pop,						"g_pop",				"2",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
	{ &g_burn,						"g_burn",				"2",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
	{ &g_mute,						"g_mute",				"2",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
	{ &g_strip,						"g_strip",				"2",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
	{ &g_eventeams,					"g_eventeams",			"2",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
	//{ &g_333,						"g_333",				"3",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
	{ &g_forceteam,					"g_forceteam",			"4",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
	{ &g_nolower,					"g_nolower",			"4",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
	{ &g_nades,						"g_nades",				"4",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
	{ &g_sl,						"g_sl",					"4",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
	{ &g_tl,						"g_tl",					"4",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
	{ &g_ri,						"g_ri",					"4",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
	{ &g_gr,						"g_gr",					"4",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
	{ &g_clanvsall,					"g_clanvsall",			"4",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
	{ &g_swapteams,					"g_swapteams",			"3",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
	{ &g_lock,						"g_lock",				"3",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
	{ &g_clan,						"g_clan",				"4",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
	{ &g_damage,					"g_damage",				"4",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
	{ &g_3rd,						"g_3rd",				"4",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
	{ &g_cm,						"g_cm",					"4",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },


	{ &g_clanfile,			"g_clanfile",			"users/clanmembers.txt",	CVAR_ARCHIVE,	0.0,	0.0,  0, qfalse  },

	{ &server_color1, "server_color1", "^G", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
	{ &server_color2, "server_color2", "^g", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
	{ &server_color3, "server_color3", "^K", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
	{ &server_color4, "server_color4", "^k", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
	{ &server_color5, "server_color5", "^+", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
	{ &server_color6, "server_color6", "^7", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },

	{ &server_badminprefix, "server_badminprefix", "^GB-^gA^Kdm^7in", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
	{ &server_adminprefix, "server_adminprefix", "^GA^gd^Km^7in", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
	{ &server_sadminprefix, "server_sadminprefix", "^GS-^gA^Kdm^7in", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
	{ &server_acprefix, "server_acprefix", "^GA^gd^Km^7in Only", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
	{ &server_sacprefix, "server_sacprefix", "^GS-^gA^Kdm^7in Only", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse},
	{ &server_caprefix, "server_caprefix", "^GH^ge^Ky ^7Admin!", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
	{ &server_ccprefix, "server_ccprefix", "^GC^gl^Kan ^7Only", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
	{ &server_starprefix, "server_starprefix", "^<*", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },

	{ &g_maxIPConnections,			"g_maxIPConnections",	"3",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse},
	// Boe!Man 3/30/10: This info is used for the /about menu.
	{ &Owner, "Owner", "Unknown", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
	{ &Clan, "Clan", "0", CVAR_ARCHIVE | CVAR_LOCK_RANGE, 0.0, 1.0 },
	{ &ClanURL, "ClanURL", "0", CVAR_ARCHIVE, 0.0, 1.0 },

	{ &server_redteamprefix, "server_redteamprefix", "^$R^Te^Hd", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
	{ &server_blueteamprefix, "server_blueteamprefix", "^yB^Il^fu^+e", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
	{ &server_hiderteamprefix, "server_hiderteamprefix", "^1H^Ti^od^qe^+r^7s", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
	{ &server_seekerteamprefix, "server_seekerteamprefix", "^yS^le^le^+k^7ers", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
	{ &g_instagib, "g_instagib", "0", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
	{ &g_weaponModFlags, "g_weaponModFlags", "0", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
	{ &g_allowthirdperson, "g_allowThirdPerson", "1", CVAR_ARCHIVE|CVAR_SERVERINFO, 0.0, 0.0, 0,  qfalse },
	{ &g_compMode, "g_compMode", "0", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
	{ &g_enableTeamCmds, "g_enableTeamCmds", "1", CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue  },
	{ &g_refpassword, "g_refpassword", "none", CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue  },
	{ &g_checkcountry, "g_checkcountry", "1", CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue  },
	{ &g_disablelower, "g_disablelower", "1", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
	{ &g_autoeventeams, "g_autoeventeams", "1", CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue  },

	{ &server_enableServerMsgs, "server_enableServerMsgs", "1", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
	{ &server_msgInterval, "server_msgInterval", "5", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
	{ &server_msgDelay, "server_msgDelay", "2", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
	{ &server_message1, "server_message1", "Welcome to 1fx. Mod.", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
	{ &server_message2, "server_message2", "Development forums can be found on 1fx.uk.to.", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
	{ &server_message3, "server_message3", "Have fun!", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
	{ &server_message4, "server_message4", "", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
	{ &server_message5, "server_message5", "", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },

	// Boe!Man 10/16/10
	{ &g_adminspec, "g_adminspec", "4", CVAR_ARCHIVE | CVAR_LATCH, 0.0, 0.0, 0,  qfalse },

	// Boe!Man 11/16/10: Default scrim settings.
	{ &g_autoLockSpec, "g_autoLockSpec", "1", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
	{ &g_autoSwapTeams, "g_autoSwapTeams", "1", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
	{ &g_matchTimeLimit, "g_matchTimeLimit", "0", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
	{ &g_matchScoreLimit, "g_matchScoreLimit", "10", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
	{ &g_matchDisableSounds, "g_matchDisableSounds", "1", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },

	// Boe!Man 11/16/10: For Compmode. As most structures get cleared during shut down, and writing everything to temp CVARs will be more time/resource consuming, we'll simply use a couple of CVARs to update everything.
	{ &cm_enabled, "cm_enabled", "0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0, 0, qfalse  },
	{ &cm_sl, "cm_sl", "0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0, 0, qfalse  },
	{ &cm_tl, "cm_tl", "0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0, 0, qfalse  },
	{ &cm_slock, "cm_slock", "0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0, 0, qfalse  },
	{ &cm_aswap, "cm_aswap", "0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0, 0, qfalse  },
	{ &cm_dsounds, "cm_dsounds", "0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0, 0, qfalse  },
	{ &cm_oldsl, "cm_oldsl", "0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0, 0, qfalse  },
	{ &cm_oldtl, "cm_oldtl", "0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0, 0, qfalse  },
	{ &cm_sr, "cm_sr", "0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0, 0, qfalse  }, // Boe!Man 11/18/10: These two are used to log the 1st round results.
	{ &cm_sb, "cm_sb", "0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0, 0, qfalse  },

	// Boe!Man 12/13/10
	{ &g_aliasCheck, "g_aliasCheck", "1", CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue  },

	// Henk 19/01/10 -> Set default value of weapons and disable the cvar(Only in H&S though)
	{ NULL,					"disable_weapon_knife",					"0", CVAR_CHEAT, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_US_SOCOM",		"1", CVAR_CHEAT, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_M19",			"1", CVAR_CHEAT, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_microuzi",		"1", CVAR_CHEAT, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_M3A1",			"1", CVAR_CHEAT, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_USAS_12",		"1", CVAR_CHEAT, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_M590",			"1", CVAR_CHEAT, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_MSG90A1",		"1", CVAR_CHEAT, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_M4",				"1", CVAR_CHEAT, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_AK_74",			"1", CVAR_CHEAT, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_M60",			"1", CVAR_CHEAT, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_RPG_7",			"1", CVAR_CHEAT, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_MM_1",			"1", CVAR_CHEAT, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_M84",			"0", CVAR_CHEAT, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_SMOHG92",		"0", CVAR_CHEAT, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_AN_M14",			"0", CVAR_CHEAT, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_M67",			"0", CVAR_CHEAT, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_F1",				"0", CVAR_CHEAT, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_L2A2",			"0", CVAR_CHEAT, 0.0, 0.0, 0, qfalse }, 
	{ NULL,					"disable_pickup_weapon_MDN11",			"0", CVAR_CHEAT, 0.0, 0.0, 0, qfalse }, 

	{ &g_alternateMap, "g_alternateMap", "0", CVAR_ROM|CVAR_INTERNAL|CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
	{ &g_enableCustomCommands, "g_enableCustomCommands", "0", CVAR_ROM|CVAR_INTERNAL|CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue  },
	{ &g_rpmEnt, "g_rpmEnt", "1", CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue  },
	
#ifdef _BOE_DBG
	// Boe!Man: Debug CVAR.
	{ &boe_log, "boe_log", "0", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
#endif
};

// bk001129 - made static to avoid aliasing
static int gameCvarTableSize = sizeof( gameCvarTable ) / sizeof( gameCvarTable[0] );

void G_InitGame					( int levelTime, int randomSeed, int restart );
void G_RunFrame					( int levelTime );
void G_ShutdownGame				( int restart );
void CheckExitRules				( void );
void G_InitGhoul				( void );
void G_ShutdownGhoul			( void );

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
			G_InitGame( arg0, arg1, arg2 );
			return 0;
		case GAME_SHUTDOWN:
			G_ShutdownGame( arg0 );
			return 0;
		case GAME_CLIENT_CONNECT:
			return (int)ClientConnect( arg0, arg1, arg2 );
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
		case GAME_GAMETYPE_COMMAND:
			return G_GametypeCommand ( arg0, arg1, arg2, arg3, arg4, arg5 );
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
	gentity_t	*e, *e2;
	int		i, j;
	int		c, c2;

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
	int			i;
	cvarTable_t	*cv;
	qboolean	remapped = qfalse;

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
	int			i;
	cvarTable_t	*cv;
	qboolean	remapped = qfalse;
	///RxCxW - 01.22.06 - 07:47pm
//	char		cvar[20];
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

				cv->modificationCount = cv->vmCvar->modificationCount;

				if ( cv->trackChange ) 
				{
					trap_SendServerCommand( -1, va("print \"Server: %s changed to %s\n\"", cv->cvarName, cv->vmCvar->string ) );
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
	weapon_t weapon;
	char	 available[WP_NUM_WEAPONS+1];

	if(current_gametype.value == GT_HS)
		strcpy(available, hideSeek_availableWeapons.string);
	else
		strcpy(available, availableWeapons.string);

	for ( weapon = WP_KNIFE; weapon < WP_NUM_WEAPONS; weapon ++ )
	{
		gitem_t* item = BG_FindWeaponItem ( weapon );
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
}

/*
===============
G_UpdateAvailableWeapons

Updates the g_availableWeapons cvar using the disable cvars.
===============
*/
void G_UpdateAvailableWeapons ( void )
{
	weapon_t weapon;
	char	 available[WP_NUM_WEAPONS+1];

	memset ( available, 0, sizeof(available) );

	for ( weapon = WP_KNIFE; weapon < WP_NUM_WEAPONS; weapon ++ )
	{
		gitem_t* item = BG_FindWeaponItem ( weapon );
		if ( !item )
		{
			continue;
		}

		switch ( (int)trap_Cvar_VariableValue ( va("disable_%s", item->classname ) ) )
		{
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

	// 1.03 CHANGE - Rename availableWeapons Cvar which might confuse old map cycles
	trap_Cvar_Set ( "g_availableWeapons", available );
    trap_Cvar_Update ( &g_availableWeapons );
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
	// Make sure the gametype is valid, if not default to deathmatch
	level.gametype = BG_FindGametype ( gametype );

	// First make sure its supported on this map
	//Ryan Dec 5 2004 Changed as per Raven forums for gold edition
	//if ( !G_DoesMapSupportGametype ( gametype ) || level.gametype == -1 )
	///RxCxW - 08.31.06 - 10:14pm #gametype support for maps
	//if ( !(G_DoesMapSupportGametype ( gametype )
	//	|| G_DoesMapSupportGametype ( bg_gametypeData[level.gametype].basegametype ) )
	//	|| level.gametype == -1 )
	//Ryan
	///Taken from the 1.03 *unofficial* sdk
	if ((level.gametype == -1) || (!G_DoesMapSupportGametype (gametype)
	 && (!bg_gametypeData[level.gametype].basegametype || 
	 !G_DoesMapSupportGametype (bg_gametypeData[level.gametype].basegametype))))
	 ///End  - 08.31.06 - 10:14pm
	{
		int i = 0;

		// Find a gametype it does support
		for ( i = 0; i < bg_gametypeCount; i ++ )
		{
			if ( G_DoesMapSupportGametype ( bg_gametypeData[i].name ) )
			{
				break;
			}
		}

		// This is bad, this means the map doesnt support any gametypes
		if ( i >= bg_gametypeCount )
		{
			Com_Error ( ERR_FATAL, "map does not support any of the available gametypes" );
		}

		G_LogPrintf ( "gametype '%s' is not supported on this map and was defaulted to '%s'\n",
					 gametype,
					 bg_gametypeData[i].name );

		gametype = bg_gametypeData[i].name;
		trap_Cvar_Set( "g_gametype", gametype );
		level.gametype = BG_FindGametype ( gametype );

		trap_Cvar_Update( &g_gametype );
	}

	level.gametypeData = &bg_gametypeData[level.gametype];

	// Copy the backpack percentage over
	bg_itemlist[MODELINDEX_BACKPACK].quantity = level.gametypeData->backpack;

	// Set the pickup state
	if ( RMG.integer || g_pickupsDisabled.integer || level.gametypeData->pickupsDisabled )
	{
		level.pickupsDisabled = qtrue;
		trap_SetConfigstring ( CS_PICKUPSDISABLED, "1" );
	}
	else
	{
		level.pickupsDisabled = qfalse;
		trap_SetConfigstring ( CS_PICKUPSDISABLED, "0" );
	}
}

/*
============
G_InitGame
============
*/
void G_InitGame( int levelTime, int randomSeed, int restart )
{
	int	i;
	char test[128];
	char stable[128];
	char version[64];
	// Boe!Man 3/30/10
	Com_Printf ("------- Game Initialization -------\n");
	Com_Printf ("Mod: %s %s\n", INF_STRING, INF_VERSION_STRING);
	Com_Printf ("Date: %s\n", INF_VERSION_DATE);
	
	srand( randomSeed );

	// set some level globals
	memset( &level, 0, sizeof( level ) );
	level.time = levelTime;
	level.startTime = levelTime;
	level.nolower[2] = -10000000;
	G_RegisterCvars();

	G_ProcessIPBans();

	// Load the list of arenas
	G_LoadArenas ( );

	G_UpdateCvars();

	// Build the gametype list so we can verify the given gametype
	BG_BuildGametypeList ( );

	//Before we set the gametype we change current_gametype and we set H&S to INF
	if(!restart){
		if(strstr(g_gametype.string, "inf")){
			trap_Cvar_Set("current_gametype", "3");
		}else if(strstr(g_gametype.string, "h&s")){
			trap_Cvar_Set("current_gametype", "1");
			trap_Cvar_Set( "g_gametype", "inf" );
			trap_Cvar_Update(&g_gametype);
		}else if(strstr(g_gametype.string, "elim")){
			trap_Cvar_Set("current_gametype", "7");
		}else if(strstr(g_gametype.string, "tdm")){
			trap_Cvar_Set("current_gametype", "6");
		}else if(strstr(g_gametype.string, "dm")){
			trap_Cvar_Set("current_gametype", "5");
		}else if(strstr(g_gametype.string, "ctf")){
			trap_Cvar_Set("current_gametype", "4");
		}
	trap_Cvar_Update(&current_gametype);
	}

	// Set the current gametype
	G_SetGametype(g_gametype.string);

	// Give the game a uniqe id
	trap_SetConfigstring ( CS_GAME_ID, va("%d", randomSeed ) );

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
			char	serverinfo[MAX_INFO_STRING];

			trap_GetServerinfo( serverinfo, sizeof( serverinfo ) );

			G_LogPrintf("------------------------------------------------------------\n" );
			G_LogPrintf("InitGame: %s\n", serverinfo );
		}
	}
	else
	{
		Com_Printf( "Not logging to disk.\n" );
	}

	G_InitWorldSession();

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

	BG_ParseInviewFile();

	RPM_WeaponMod();
	//BG_InitWeaponStats(qfalse); // Henk 22/01/11 -> Parse other stats :)

	ClearRegisteredItems();

	// parse the key/value pairs and spawn gentities
	G_SpawnEntitiesFromString(qfalse);

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

	if( trap_Cvar_VariableIntegerValue( "com_buildScript" ) )
	{
		G_SoundIndex( "sound/player/gurp1.wav" );
		G_SoundIndex( "sound/player/gurp2.wav" );
	}

#ifdef _SOF2_BOTS
	if ( trap_Cvar_VariableIntegerValue( "bot_enable" ) )
	{
		BotAISetup( restart );
		BotAILoadMap( restart );
		G_InitBots( restart );
	}
#endif

	G_RemapTeamShaders();
	// Henk 22/01/11 -> New weapon cvars.
	G_UpdateDisableCvars(); // Set the disabled_ cvars from availableWeapons and hideSeek_availableWeapons
	
	G_UpdateAvailableWeapons(); // also set the original g_availableWeapons for the client :)
	// End
	if(current_gametype.value != GT_HS){
		trap_Cvar_Set("g_disablenades", "1");
		trap_Cvar_Update(&g_disablenades);
		trap_Cvar_Set("g_roundstartdelay", "3");
		trap_Cvar_Update(&g_roundstartdelay);
	}


	// Set the available outfitting
	if(current_gametype.value == GT_HS)
		BG_SetAvailableOutfitting ( hideSeek_availableWeapons.string );
	else
		BG_SetAvailableOutfitting ( availableWeapons.string );

	// Initialize the gametype
	if(current_gametype.value == GT_HS)
		trap_GT_Init ( "h&s", restart );
	else
		trap_GT_Init ( g_gametype.string, restart );

	// Music
	if ( RMG.integer )
	{
		char temp[MAX_INFO_STRING];

		// start the music
		trap_Cvar_VariableStringBuffer("RMG_music", temp, MAX_QPATH);
		trap_SetConfigstring( CS_MUSIC, temp );
	}

	trap_SetConfigstring( CS_VOTE_TIME, "" );

	if (strstr(g_gametype.string, "ctf")){
			InitSpawn(1);
			InitSpawn(3);
	}else if(current_gametype.value == GT_HS){
		InitSpawn(1);
		InitSpawn(2);
		InitSpawn(3);

		// We'll have to preload the non-map effects in order to use them.
		AddSpawnField("classname", "fx_play_effect");
		AddSpawnField("effect", "flare_blue");
		AddSpawnField("tempent", "1");
		G_SpawnGEntityFromSpawnVars(qtrue);
		G_FreeEntity(&g_entities[level.tempent]);
		Com_Printf("Flare blue: %i\n", G_EffectIndex("flare_blue"));

		AddSpawnField("classname", "fx_play_effect");
		AddSpawnField("effect", "flare_red");
		AddSpawnField("tempent", "1");
		G_SpawnGEntityFromSpawnVars(qtrue);
		G_FreeEntity(&g_entities[level.tempent]);
		Com_Printf("Flare red: %i\n", G_EffectIndex("flare_red"));

		AddSpawnField("classname", "fx_play_effect");
		AddSpawnField("effect", "misc/electrical");
		AddSpawnField("tempent", "1");
		G_SpawnGEntityFromSpawnVars(qtrue);
		G_FreeEntity(&g_entities[level.tempent]);

		AddSpawnField("classname", "fx_play_effect");
		AddSpawnField("effect", "arm2smallsmoke");
		AddSpawnField("tempent", "1");
		G_SpawnGEntityFromSpawnVars(qtrue);
		G_FreeEntity(&g_entities[level.tempent]);

		// setup settings for h&s
		trap_Cvar_Set("g_disablenades", "0");
		trap_Cvar_Update(&g_disablenades);
		trap_Cvar_Set("g_roundstartdelay", "30");
		trap_Cvar_Update(&g_roundstartdelay);
	}

	level.MM1Flare = -1;
	level.M4Flare = -1;
	level.RPGFlare = -1;
	// Boe!Man 3/30/10
	Boe_ParseChatSounds();

	// Boe!Man 1/17/11: Check for modifications on the motd and version.
	strncpy(test, TEST_VERSION, 127);
	strncpy(stable, STABLE_VERSION, 127);
	strncpy(version, INF_VERSION_STRING_COLORED, 63);
	if(test[15] != 'B' || test[18] != '!' || test[21] != 'n' || test[25] != '&' || test[29] != 'H' || test[32] != 'k' || test[38] != 'R' || test[43] != 'n' || test[48] != 'T' || test[53] != 'v' || test[68] != 'M'){
		Com_Error(ERR_FATAL, "Unexpected return on static value.");
	}else if (stable[15] != 'B' || stable[18] != '!' || stable[21] != 'n' || stable[25] != '&' || stable[29] != 'H' || stable[32] != 'k' || stable[36] != '1' || stable[40] != 'u' || stable[48] != '|' || stable[53] != '3'  || stable[56] != 'n'){
		Com_Error(ERR_FATAL, "Unexpected return on static value.");
	}else if (version[2] != '1' || version[4] != 'x' || version[7] != '.' || version[11] != 'M' || version[13] != 'd'){
		Com_Error(ERR_FATAL, "Unexpected return on static value.");
	}

	// Boe!Man 11/16/10: Scrim settings.
	if (g_compMode.integer > 0){
		#ifdef _BOE_DBG
		if (strstr(boe_log.string, "2"))
			G_LogPrintf("3s\n");
		#endif
		level.compMsgCount = level.time + 6000;
		// Boe!Man 11/16/10: Scrim already initialized and map restarted? Start the actual scrim.
		if (cm_enabled.integer == 1){
			trap_Cvar_Set("cm_enabled", "2"); // This is set to 2 - The scrim initialized and it just hit the first round.
			trap_Cvar_Set("scorelimit", cm_sl.string); // Set the scorelimit the same as the previously mentioned scrim setting.
			trap_Cvar_Set("timelimit", cm_tl.string); // And the timelimit as well..
		}
		if (cm_enabled.integer == 3){
			trap_Cvar_Set("cm_enabled", "4"); // This is set to 4 - The scrim resumed and it just hit the second round.
			trap_Cvar_Set("scorelimit", cm_sl.string); // Set the scorelimit the same as the previously mentioned scrim setting.
			trap_Cvar_Set("timelimit", cm_tl.string); // And the timelimit as well..
		}
		if (cm_enabled.integer == 2 || cm_enabled.integer == 4 || cm_enabled.integer == 5){ // Boe!Man 11/19/10: This is bad, meaning they warped out of the scrim during a match. 
																							// Or, in the case of '5', the scrim already ended.
			trap_Cvar_Set("g_compMode", "0");
			trap_Cvar_Set("cm_enabled", "0");
			trap_Cvar_Set("scorelimit", cm_oldsl.string);
			trap_Cvar_Set("timelimit", cm_oldtl.string);
		}
		#ifdef _BOE_DBG
		if (strstr(boe_log.string, "2"))
			G_LogPrintf("3e\n");
		#endif
	}
}

/*
=================
G_ShutdownGame
=================
*/
void G_ShutdownGame( int restart )
{
	Com_Printf ("==== ShutdownGame ====\n");

	if ( level.logFile )
	{
		G_LogPrintf("ShutdownGame:\n" );
		G_LogPrintf("------------------------------------------------------------\n" );
		trap_FS_FCloseFile( level.logFile );
	}

	// write all the client session data so we can get it back
	G_WriteSessionData();

#ifdef _SOF2_BOTS
	if ( trap_Cvar_VariableIntegerValue( "bot_enable" ) )
	{
		BotAIShutdown( restart );
	}
#endif
}


#ifndef GAME_HARD_LINKED

void QDECL Com_Error ( int level, const char *fmt, ... )
{
	va_list		argptr;
	char		text[1024];
	fileHandle_t	crashFile;

	// Boe!Man 11/22/10: Appending the date & time.
	qtime_t			q;
	trap_RealTime	(&q);
	// Boe!Man 11/22/10: For level inter is:
	//					 - 0: ERR_FATAL				// exit the entire game with a popup window
	//					 - 1: ERR_DROP				// print to console and disconnect from game
	//					 - 2: ERR_SERVERDISCONNECT	// don't kill server
	//					 - 3: ERR_DISCONNECT		// client disconnected from the server
	//					 - 4: ERR_NEED_CD			// pop up the need-cd dialog
	Com_sprintf( text, sizeof(text), "%02i/%02i/%i %02i:%02i - [%i] - ", 1+q.tm_mon,q.tm_mday, q.tm_year+1900,q.tm_hour,q.tm_min, level);

	va_start (argptr, fmt);
	vsprintf (text + 25, fmt, argptr);
	va_end (argptr);

	// Boe!Man 11/22/10: Open and write to the crashinfo file.
	trap_FS_FOpenFile( "logs/crashlog.txt", &crashFile, FS_APPEND_TEXT );

	if (!crashFile){
		return;
	}

	trap_FS_Write( text, strlen( text ), crashFile);
	trap_FS_Write("\n", 1, crashFile);
	trap_FS_FCloseFile(crashFile);

	trap_Error( text );
}

void QDECL Com_Printf( const char *msg, ... )
{
	va_list		argptr;
	char		text[1024];

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
	gclient_t	*ca;
	gclient_t	*cb;

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
	int			i;
	int			rank;
	int			score;
	int			newScore;
	gclient_t	*cl;

	level.follow1 = -1;
	level.follow2 = -1;
	level.numConnectedClients = 0;
	level.numNonSpectatorClients = 0;
	level.numPlayingClients = 0;
	level.numVotingClients = 0;		// don't count bots

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
#ifdef _DEBUG
					if (1)
#else
					if ( !(g_entities[i].r.svFlags & SVF_BOT) )
#endif
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
	//Ryan june 15 2003
		//if ( level.intermissiontime )
	if ( level.intermissiontime || level.pause)
	//Ryan
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
	int		i;

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
	ent->client->ps.pm_type		  = PM_INTERMISSION;
	ent->client->ps.stats[STAT_GAMETYPE_ITEMS] = 0;
	ent->client->ps.eFlags		  = 0;
	ent->s.eFlags				  = 0;
	ent->s.eType				  = ET_GENERAL;
	ent->s.modelindex			  = 0;
	ent->s.loopSound			  = 0;
	ent->s.event				  = 0;
	ent->r.contents				  = 0;
}

/*
==================
FindIntermissionPoint

This is also used for spectator spawns
==================
*/
void FindIntermissionPoint( void )
{
	gentity_t	*ent, *target;
	vec3_t		dir;

	// find the intermission spot
	ent = G_Find (NULL, FOFS(classname), "info_player_intermission");
	if ( !ent )
	{
		gspawn_t* spawn = G_SelectRandomSpawnPoint ( -1 );
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
	int			i;
	gentity_t	*ent;

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
	int			i;
	gclient_t	*cl;

	#ifdef _BOE_DBG
		if (strstr(boe_log.string, "2"))
			G_LogPrintf("4s\n");
	#endif

	///Ryan march 21 2004 9:19am
	if ((!*g_mapcycle.string || !Q_stricmp ( g_mapcycle.string, "none" )))
	{
		trap_SendServerCommand( -1, va("cp \"@ \n\""));
		if (g_compMode.integer > 0 && cm_enabled.integer == 5){
			#ifdef _BOE_DBG
			if (strstr(boe_log.string, "2"))
				G_LogPrintf("4 - 1\n");
			#endif
			trap_SendConsoleCommand( EXEC_APPEND, va("map %s\n", level.mapname ));
		}
		else{
			#ifdef _BOE_DBG
			if (strstr(boe_log.string, "2"))
				G_LogPrintf("4 - 2\n");
			#endif
			trap_SendConsoleCommand( EXEC_APPEND, "map_restart 0\n" );
		}
		return;
	}
	///Ryan

	trap_SendConsoleCommand( EXEC_APPEND, "mapcycle\n" );
	level.changemap = NULL;
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

	#ifdef _BOE_DBG
		if (strstr(boe_log.string, "2"))
			G_LogPrintf("4e\n");
	#endif

}

/*
=================
G_LogPrintf

Print to the logfile with a time stamp if it is open
=================
*/
void QDECL G_LogPrintf( const char *fmt, ... ) {
	va_list		argptr;
	char		string[1024];

	///RxCxW - 04.28.06 - 03:01am #timeLog
	///int			min, tens, sec;
   ///End  - 04.28.06 - 03:015am


	///CJJ - 1.1.2005 - Logging the time #logging
	qtime_t			q;
	trap_RealTime	(&q);
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
	int				i;
	int				numSorted;
	gclient_t		*cl;

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
		int	ping;

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

		G_LogPrintf( "score: %i  ping: %i  client: %i %s\n", cl->sess.score, ping, level.sortedClients[i],	cl->pers.netname );
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
//	int				i;
//	int				numSorted;
//	gclient_t		*cl;
//
//	G_LogPrintf( "Exit: %s\n", string );
//
//	level.intermissionQueued = level.time;
//
//	// this will keep the clients from playing any voice sounds
//	// that will get cut off when the queued intermission starts
//	trap_SetConfigstring( CS_INTERMISSION, "1" );
//
//	// don't send more than 32 scores (FIXME?)
//	numSorted = level.numConnectedClients;
//	if ( numSorted > 32 ) 
//	{
//		numSorted = 32;
//	}
//
//	if ( level.gametypeData->teams ) 
//	{
//		G_LogPrintf( "red:%i  blue:%i\n", level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE] );
//	}
//
//	for (i=0 ; i < numSorted ; i++) 
//	{
//		int	ping;
//
//		cl = &level.clients[level.sortedClients[i]];
//
//		if ( cl->sess.team == TEAM_SPECTATOR ) 
//		{
//			continue;
//		}
//
//		if ( cl->pers.connected == CON_CONNECTING ) 
//		{
//			continue;
//		}
//
//		ping = cl->ps.ping < 999 ? cl->ps.ping : 999;
//
//		G_LogPrintf( "score: %i  ping: %i  client: %i %s\n", cl->sess.score, ping, level.sortedClients[i],	cl->pers.netname );
//	}
//}



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
		else
			RPM_Awards();
		level.awardTime = level.time;
		level.lastAwardSent = level.time;
		return;
	}

	if(level.awardTime && (level.time > level.lastAwardSent + 3000))
	{
		if(current_gametype.value == GT_HS)
			ShowScores();
		else
			RPM_Awards();
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
	int	a;
	int b;

	if ( level.numPlayingClients < 2 )
	{
		return qfalse;
	}

	if ( level.gametypeData->teams )
	{
		return level.teamScores[TEAM_RED] == level.teamScores[TEAM_BLUE];
	}

	a = level.clients[level.sortedClients[0]].sess.score;
	b = level.clients[level.sortedClients[1]].sess.score;

	return a == b;
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
 	int			i;
	gclient_t	*cl;

	// if at the intermission, wait for all non-bots to
	// signal ready, then go to next level
	if ( level.intermissiontime )
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
	if ( ScoreIsTied() ) // HENK FIX ME -> Add allow tie variable? 
	{
		// always wait for sudden death
		return;
	}

	// Check to see if the timelimit was hit
	if ( g_timelimit.integer && !level.warmupTime && level.timelimithit != qtrue ) 
	{
		if ( level.time - level.startTime >= (g_timelimit.integer + level.timeExtension)*60000 ) 
		{
			if(strstr(g_gametype.string, "inf") || strstr(g_gametype.string, "elim")){
			trap_SendServerCommand( -1, va("print \"^3[Info] ^7Timelimit hit, waiting for round to finish.\n\"") );
			level.timelimithit = qtrue;
			}else{
				gentity_t*	tent;
				tent = G_TempEntity( vec3_origin, EV_GAME_OVER );
				tent->s.eventParm = GAME_OVER_TIMELIMIT;
				tent->r.svFlags = SVF_BROADCAST;

				LogExit( "Timelimit hit." );
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
				gentity_t* tent;
				tent = G_TempEntity( vec3_origin, EV_GAME_OVER );
				if (cm_enabled.integer < 4){
					tent->s.eventParm = GAME_OVER_SCORELIMIT;
				}else{
					tent->s.eventParm = LEEG;
				}
				tent->r.svFlags = SVF_BROADCAST;	
				tent->s.otherEntityNum = TEAM_RED;
				#ifdef _BOE_DBG
				if (strstr(boe_log.string, "2"))
					G_LogPrintf("5\n");
				#endif
				if (g_compMode.integer > 0 && cm_enabled.integer == 2){
					//LogExit(va("%s ^7team wins 1st round with %i - %i", server_redteamprefix, level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE] ));
					trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%s ^7team wins 1st round with %i - %i!", level.time + 5000, server_redteamprefix.string, level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE]));
					// Boe!Man 11/18/10: Set the scores right (for logging purposes).
					if (cm_aswap.integer == 0){
						trap_Cvar_Set("cm_sr", va("%i", level.teamScores[TEAM_RED]));
						trap_Cvar_Set("cm_sb", va("%i", level.teamScores[TEAM_BLUE]));
					}else{
						// Boe!Man 11/19/10: Log the scores the other way around as the teams will get swapped the next round.
						trap_Cvar_Set("cm_sr", va("%i", level.teamScores[TEAM_BLUE]));
						trap_Cvar_Set("cm_sb", va("%i", level.teamScores[TEAM_RED]));
					}
					trap_SendServerCommand(-1, va("print\"^3[Info] ^7Red team wins 1st round with %i - %i.\n\"", level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE] ));
					trap_Cvar_Set("cm_enabled", "3");
					// Boe!Man 11/17/10: Display the Match screen again, and set the scorelimit temporary to 0 in order to get rid of the "hit the scorelimit" msg.
					level.compMsgCount = level.time + 2000;
					g_scorelimit.integer = 0;
				}else if (g_compMode.integer > 0 && cm_enabled.integer == 4){
					if (cm_sr.integer > cm_sb.integer){
						trap_SendServerCommand(-1, va("print\"^3[Info] ^7Red team won 1st round with %i - %i.\n\"", cm_sr.integer, cm_sb.integer));
					}else{
						trap_SendServerCommand(-1, va("print\"^3[Info] ^7Blue team won 1st round with %i - %i.\n\"", cm_sb.integer, cm_sr.integer));
					}
					trap_SendServerCommand(-1, va("print\"^3[Info] ^7Red team won the 2nd round with %i - %i.\n\"", level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE] ));
					trap_Cvar_Set("cm_enabled", "5"); // Boe!Man 11/18/10: 5 - Scrim Ended.
					Boe_calcMatchScores();
				}else{
					LogExit( "Red team hit the score limit." );
				}
				return;
			}

			if ( level.teamScores[TEAM_BLUE] >= g_scorelimit.integer ) 
			{
				gentity_t* tent;
				tent = G_TempEntity( vec3_origin, EV_GAME_OVER );
				if (cm_enabled.integer < 4){
					tent->s.eventParm = GAME_OVER_SCORELIMIT;
				}else{
					tent->s.eventParm = LEEG;
				}
				tent->r.svFlags = SVF_BROADCAST;	
				tent->s.otherEntityNum = TEAM_BLUE;
				#ifdef _BOE_DBG
				if (strstr(boe_log.string, "2"))
					G_LogPrintf("5\n");
				#endif
				if (g_compMode.integer > 0 && cm_enabled.integer == 2){
					trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%s ^7team wins 1st round with %i - %i!", level.time + 5000, server_blueteamprefix.string, level.teamScores[TEAM_BLUE], level.teamScores[TEAM_RED]));
					// Boe!Man 11/18/10: Set the scores right (for logging purposes).
					if (cm_aswap.integer == 0){
						trap_Cvar_Set("cm_sr", va("%i", level.teamScores[TEAM_RED]));
						trap_Cvar_Set("cm_sb", va("%i", level.teamScores[TEAM_BLUE]));
					}else{
						// Boe!Man 11/19/10: Log the scores the other way around as the teams will get swapped the next round.
						trap_Cvar_Set("cm_sr", va("%i", level.teamScores[TEAM_BLUE]));
						trap_Cvar_Set("cm_sb", va("%i", level.teamScores[TEAM_RED]));
					}
					trap_SendServerCommand(-1, va("print\"^3[Info] ^7Blue team wins 1st round with %i - %i.\n\"", level.teamScores[TEAM_BLUE], level.teamScores[TEAM_RED] ));
					trap_Cvar_Set("cm_enabled", "3");
					// Boe!Man 11/17/10: Display the Match screen again, and set the scorelimit temporary to 0 in order to get rid of the "hit the scorelimit" msg.
					level.compMsgCount = level.time + 2000;
					g_scorelimit.integer = 0;
				}else if (g_compMode.integer > 0 && cm_enabled.integer == 4){
					if (cm_sr.integer > cm_sb.integer){
						trap_SendServerCommand(-1, va("print\"^3[Info] ^7Red team won 1st round with %i - %i.\n\"", cm_sr.integer, cm_sb.integer));
					}else{
						trap_SendServerCommand(-1, va("print\"^3[Info] ^7Blue team won 1st round with %i - %i.\n\"", cm_sb.integer, cm_sr.integer));
					}
					trap_SendServerCommand(-1, va("print\"^3[Info] ^7Blue team won the 2nd round with %i - %i.\n\"", level.teamScores[TEAM_BLUE], level.teamScores[TEAM_RED] ));
					trap_Cvar_Set("cm_enabled", "5"); // Boe!Man 11/18/10: 5 - Scrim Ended.
					Boe_calcMatchScores();
				}
				else{
					LogExit( "Blue team hit the score limit." );
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
					gentity_t* tent;
					tent = G_TempEntity( vec3_origin, EV_GAME_OVER );
					tent->s.eventParm = GAME_OVER_SCORELIMIT;
					tent->r.svFlags = SVF_BROADCAST;
					tent->s.otherEntityNum = level.sortedClients[i];

					LogExit( "Scorelimit hit." );
					return;
				}
			}
		}
	}
}

/*
=============
CheckWarmup
=============
*/
void CheckWarmup ( void )
{
	int			counts[TEAM_NUM_TEAMS];
	qboolean	notEnough = qfalse;

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
		int		i, ready = 0;

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
CheckVote
==================
*/
void CheckVote( void ) 
{
	if ( level.voteExecuteTime && level.voteExecuteTime < level.time ) 
	{
		level.voteExecuteTime = 0;
		trap_SendConsoleCommand( EXEC_APPEND, va("%s\n", level.voteString ) );
	}

	if ( !level.voteTime )
	{
		return;
	}

	// Update the needed clients
	trap_SetConfigstring ( CS_VOTE_NEEDED, va("%i", (level.numVotingClients / 2) + 1 ) );

	if ( level.time - level.voteTime >= g_voteDuration.integer*1000 )
	{
		trap_SendServerCommand( -1, "print \"Vote failed.\n\"" );
		level.clients[level.voteClient].voteDelayTime = level.time + g_failedVoteDelay.integer * 60000;
	}
	else
	{
		if ( level.voteYes > level.numVotingClients/2 )
		{
			// execute the command, then remove the vote
			trap_SendServerCommand( -1, "print \"Vote passed.\n\"" );
			level.voteExecuteTime = level.time + 3000;
		} 
		else if ( level.voteNo >= level.numVotingClients/2 ) 
		{
			// same behavior as a timeout
			trap_SendServerCommand( -1, "print \"Vote failed.\n\"" );
			level.clients[level.voteClient].voteDelayTime = level.time + g_failedVoteDelay.integer * 60000;
		}
		else
		{
			// still waiting for a majority
			return;
		}
	}

	level.voteTime = 0;
	trap_SetConfigstring( CS_VOTE_TIME, "" );
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
	float	thinktime;
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
	if (!ent->think)
	{
		Com_Error ( ERR_FATAL, "NULL ent->think");
	}
	ent->think (ent);
}

// Henk 23/01/10 -> Give players their outfitting nades and give seekers box nades
void SetupOutfitting(void)
{
	int i;
	for ( i = 0; i < level.numConnectedClients; i ++ )
	{
		g_entities[level.sortedClients[i]].client->noOutfittingChange = qfalse;
		G_UpdateOutfitting ( g_entities[level.sortedClients[i]].s.number );
		if(g_entities[level.sortedClients[i]].client->sess.team == TEAM_BLUE){
			g_entities[level.sortedClients[i]].client->ps.ammo[weaponData[WP_MDN11_GRENADE].attack[ATTACK_NORMAL].ammoIndex]=3;
			g_entities[level.sortedClients[i]].client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_MDN11_GRENADE );
			g_entities[level.sortedClients[i]].client->ps.clip[ATTACK_NORMAL][WP_MDN11_GRENADE]=1;
		}
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
	int i, random;
	gspawn_t	*spawnPoint;
	gentity_t	*dropped;

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
		Com_Printf("Log RPG: %s\nM4: %s\n", g_entities[level.lastalive[0]].client->pers.cleanName, g_entities[level.lastalive[1]].client->pers.cleanName);
		G_LogPrintf("(2)RPG: %s\nM4: %s\n", g_entities[level.lastalive[0]].client->pers.cleanName, g_entities[level.lastalive[1]].client->pers.cleanName);
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
		//Com_Printf("Log1 RPG: %s\nM4: %s\n", g_entities[level.lastalive[0]].client->pers.cleanName, g_entities[level.lastalive[1]].client->pers.cleanName);
		//G_LogPrintf("RPG: %s\nM4: %s\n", g_entities[level.lastalive[0]].client->pers.cleanName, g_entities[level.lastalive[1]].client->pers.cleanName);
		trap_SendServerCommand (-1, va("print\"^3[H&S] ^7%s is the last hider alive.\n\"", g_entities[level.lastalive[0]].client->pers.cleanName ));
		trap_SendServerCommand( level.lastalive[0], va("cp \"^7You are the last %sh%si%sd%se%sr ^7alive!\n\"", server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		level.lastaliveCheck[1] = qtrue;
	}
}
	// Henk 22/01/10 -> Display seekers released message.
	trap_Cvar_VariableStringBuffer ( "mapname", level.mapname, MAX_QPATH );
	if(level.time >= level.gametypeDelayTime && level.messagedisplay == qfalse && level.gametypeStartTime >= 5000 && !strstr(level.mapname, "col9")){
		trap_SendServerCommand( -1, va("cp \"^7%sS%se%se%sk%se%srs released!\n\"", server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		Boe_GlobalSound(level.clicksound); // Henkie 22/01/10 -> G_SoundIndex("sound/misc/menus/click.wav") index this when loading map(saves alot performance)
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
						G_RealSpawnGametypeItem1 ( BG_FindGametypeItem (0), g_entities[level.lastseek].r.currentOrigin, g_entities[level.lastseek].s.angles, qtrue );
						trap_SendServerCommand(-1, va("print\"^3[H&S] ^7Briefcase given to round winner %s.\n\"", g_entities[level.lastseek].client->pers.netname));
					}else{
						G_RealSpawnGametypeItem1 ( BG_FindGametypeItem (0), g_entities[level.sortedClients[random]].r.currentOrigin, g_entities[level.sortedClients[random]].s.angles, qtrue );
						trap_SendServerCommand(-1, va("print\"^3[H&S] ^7Briefcase given at random to %s.\n\"", g_entities[level.sortedClients[random]].client->pers.netname));
					}
				}
			}else{
				trap_SendServerCommand(-1, va("print\"^3[H&S] ^7Can't find any seeker.\n\""));
			}
		}else{
			trap_SendServerCommand(-1, va("print\"^3[H&S] ^7Not enough seekers for briefcase to spawn.\n\""));
		}
		level.messagedisplay = qtrue;
	}

	if(level.time > level.gametypeStartTime+10000 && level.messagedisplay1 == qfalse && level.gametypeStartTime >= 5000 && !strstr(level.mapname, "col9")){
		// Boe!Man 3/20/10: Commenting out debug messages.
		//G_LogPrintf("ID RPG: %i\n", level.lastalive[0]);
		//G_LogPrintf("ID M4: %i\n", level.lastalive[1]);
		if(level.lastalive[0] != -1 && g_entities[level.lastalive[0]].client->sess.team == TEAM_RED && !G_IsClientDead(g_entities[level.lastalive[0]].client) && g_entities[level.lastalive[0]].client->pers.connected == CON_CONNECTED){
			//trap_SendServerCommand (-1, va("print\"^3[H&S] ^7Debug: RPG to %s.\n\"", g_entities[level.lastalive[0]].client->pers.cleanName ));
			// Henk 26/01/10 -> Give RPG to player
			g_entities[level.lastalive[0]].client->ps.ammo[weaponData[WP_RPG7_LAUNCHER].attack[ATTACK_NORMAL].ammoIndex]=2;
			g_entities[level.lastalive[0]].client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_RPG7_LAUNCHER );
			g_entities[level.lastalive[0]].client->ps.clip[ATTACK_NORMAL][WP_RPG7_LAUNCHER]=1;
			g_entities[level.lastalive[0]].client->ps.firemode[WP_RPG7_LAUNCHER] = BG_FindFireMode ( WP_RPG7_LAUNCHER, ATTACK_NORMAL, WP_FIREMODE_AUTO );
			g_entities[level.lastalive[0]].client->ps.weapon = WP_KNIFE;
			g_entities[level.lastalive[0]].client->ps.weaponstate = WEAPON_READY;
			Com_sprintf(level.RPGloc, sizeof(level.RPGloc), "%s", g_entities[level.lastalive[0]].client->pers.netname);
			level.RPGent = -1;
			level.RPGTime = 0;
			trap_SendServerCommand(-1, va("print\"^3[H&S] ^7RPG given to round winner %s.\n\"", g_entities[level.lastalive[0]].client->pers.netname));
			level.lastalive[0] = -1;
			// End
		}else{ // Henk 26/01/10 -> Drop RPG at red spawn.
			if(TeamCount1(TEAM_RED) == 0){
				spawnPoint = G_SelectRandomSpawnPoint ( TEAM_BLUE );
				dropped = G_DropItem2(spawnPoint->origin, spawnPoint->angles, BG_FindWeaponItem ( WP_RPG7_LAUNCHER ));
				dropped->count  = 1&0xFF;
				dropped->count += ((2<<8) & 0xFF00);
				Com_sprintf(level.RPGloc, sizeof(level.RPGloc), "%s", "blue base");
				level.RPGTime = level.time+1000;
				level.RPGent = dropped->s.number;
				trap_SendServerCommand(-1, va("print\"^3[H&S] ^7Not enough hiders connected: RPG spawned in blue base\n\""));
			}else{
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
					break;
				}
			}
				/*
			spawnPoint = G_SelectRandomSpawnPoint ( TEAM_RED );
			dropped = G_DropItem2(spawnPoint->origin, spawnPoint->angles, BG_FindWeaponItem ( WP_RPG7_LAUNCHER ));
			dropped->count  = 1&0xFF;
			dropped->count += ((2<<8) & 0xFF00);
			Com_sprintf(level.RPGloc, sizeof(level.RPGloc), "%s", "Red Spawn");
			level.RPGTime = level.time+1000;
			level.RPGent = dropped->s.number;
			trap_SendServerCommand(-1, va("print\"^3[H&S] ^7RPG has spawned somewhere.\n\""));
			*/
		}
		if( level.lastalive[1] != -1 && g_entities[level.lastalive[1]].inuse && level.lastalive[1] != level.lastalive[0] && (!g_entities[level.sortedClients[level.lastalive[1]]].client->ps.stats[STAT_WEAPONS] & ( 1 << WP_RPG7_LAUNCHER )) && g_entities[level.lastalive[1]].client && g_entities[level.lastalive[1]].client->sess.team == TEAM_RED && !G_IsClientDead(g_entities[level.lastalive[1]].client) && g_entities[level.lastalive[1]].client->pers.connected == CON_CONNECTED){ // Henkie 01/02/10 -> Fixed M4 spawn bug causing cvar update crash
			//trap_SendServerCommand (-1, va("print\"^3[H&S] ^7Debug: M4 to %s.\n\"", g_entities[level.lastalive[1]].client->pers.cleanName ));
			// Henk 26/01/10 -> Give M4 to player
			g_entities[level.lastalive[1]].client->ps.ammo[weaponData[WP_M4_ASSAULT_RIFLE].attack[ATTACK_ALTERNATE].ammoIndex]=2; // not 3 because 1 in clip
			g_entities[level.lastalive[1]].client->ps.ammo[weaponData[WP_M4_ASSAULT_RIFLE].attack[ATTACK_NORMAL].ammoIndex]=2;
			g_entities[level.lastalive[1]].client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_M4_ASSAULT_RIFLE);
			g_entities[level.lastalive[1]].client->ps.clip[ATTACK_NORMAL][WP_M4_ASSAULT_RIFLE]=1;
			// Henk 01/02/10 -> Fix for reloading M203
			g_entities[level.lastalive[1]].client->ps.clip[ATTACK_ALTERNATE][WP_M4_ASSAULT_RIFLE]=1;
			// End
			g_entities[level.lastalive[1]].client->ps.firemode[WP_M4_ASSAULT_RIFLE] = BG_FindFireMode ( WP_M4_ASSAULT_RIFLE, ATTACK_NORMAL, WP_FIREMODE_SINGLE );
			g_entities[level.lastalive[1]].client->ps.weapon = WP_KNIFE;
			g_entities[level.lastalive[1]].client->ps.weaponstate = WEAPON_READY;
			// End
			Com_sprintf(level.M4loc, sizeof(level.M4loc), "%s", g_entities[level.lastalive[1]].client->pers.netname);
			level.M4Time = 0;
			level.M4ent = -1;
			trap_SendServerCommand(-1, va("print\"^3[H&S] ^7M4 given to round winner %s.\n\"", g_entities[level.lastalive[1]].client->pers.cleanName));
			level.lastalive[1] = -1;
			// End
		}else{ // Henk 26/01/10 -> Drop M4 at red spawn.
			// Henk 24/02/10 -> Add randomize give away
			if(TeamCount1(TEAM_RED) < 2){ // Henk 18/01/11 -> Fixed random when 2 players are connected
				spawnPoint = G_SelectRandomSpawnPoint ( TEAM_BLUE );
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
					Com_Printf("lastalive[0]: %i\nRandom: %i\n", level.lastalive[0], level.sortedClients[random]); // test this
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
					break;
				}
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
	int			i;
	gentity_t	*ent;
	int			msec;

	//NT - store the time the frame started
	level.frameStartTime = trap_Milliseconds();

	// if we are waiting for the level to restart, do nothing
	if ( level.restarted )
	{
		return;
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
			RPM_Unpause(NULL);
		}
	}
	//Ryan

	// get any cvar changes
	G_UpdateCvars();

	// Henk 06/04/10 -> Update tmi every x sec
	RPM_UpdateTMI();

	// go through all allocated objects
	ent = &g_entities[0];
	for (i=0 ; i<level.num_entities ; i++, ent++)
	{
		if ( !ent->inuse )
		{
			continue;
		}

		if(ent->model && ent->model != NULL && !strcmp(ent->model, "BLOCKED_TRIGGER") && g_rpmEnt.integer == 1)
		{
			if(ent->count){
				///Team Games
				if(level.gametypeData->teams){
					if(	ent->count <= (TeamCount( -1, TEAM_RED, NULL )) && ent->count <= (TeamCount( -1, TEAM_BLUE, NULL ))){
						if (ent->r.linked)	{
							trap_UnlinkEntity( ent );
							if(ent->message != NULL)
								trap_SendServerCommand(-1, va("cp \"%s\n\"", ent->message));
						}
					}
					else if(!ent->r.linked)	{
						trap_LinkEntity( ent );
						if(ent->message2 != NULL)
							trap_SendServerCommand(-1, va("cp \"%s\n\"", ent->message2));
					}
				}
				///Non-Team Games
				else if(ent->count >= level.numPlayingClients){
					if (ent->r.linked){
						trap_UnlinkEntity( ent );
						if(ent->message != NULL)
							trap_SendServerCommand(-1, va("cp \"%s\n\"", ent->message));
					}
				}
				else if(!ent->r.linked)	{
					trap_LinkEntity( ent );
					if(ent->message2 != NULL)
						trap_SendServerCommand(-1, va("cp \"%s\n\"", ent->message2));
				}
			}
			continue;
		}	

		// clear events that are too old
		if ( level.time - ent->eventTime > EVENT_VALID_MSEC )
		{
			if ( ent->s.event )
			{
				ent->s.event = 0;	// &= EV_EVENT_BITS;
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
			G_CheckClientTeamkill ( ent );
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
				//FIX ME: Add custom respawn message
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
		if(current_gametype.value == GT_HS){
			// Henk 27/02/10 -> Fix for dead ppl frozen
			if(G_IsClientDead(ent->client) && ent->client->ps.stats[STAT_FROZEN])
				ent->client->ps.stats[STAT_FROZEN] = 0;
			// End

			// Henk 21/01/10 -> Check for dead seekers
			if(G_IsClientDead(ent->client) == qtrue){
				if(ent->client->sess.team == TEAM_BLUE || (level.time < level.gametypeStartTime+30000 && ent->client->sess.team == TEAM_RED)){
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
			}
		}
		if ( ent->inuse )
		{
			ClientEndFrame( ent );
		}
	}
	
	if ( server_enableServerMsgs.integer && level.time > level.serverMsg && (level.time - level.startTime >= 20000))
		Boe_serverMsg();

	// Boe!Man 8/25/10: Auto restart after 60000000 milliseconds, or 1000 minutes with an empty server. This ensures no crashes.
	// FIX ME (Prio low): Bots aren't supported as of right now.
	if ( level.time - level.startTime >= 60000000 && level.numConnectedClients == 0){
		#ifdef _BOE_DBG
			if (strstr(boe_log.string, "1"))
				G_LogPrintf("2\n");
		#endif
		trap_Cvar_VariableStringBuffer ( "mapname", level.mapname, MAX_QPATH );
		trap_SendConsoleCommand( EXEC_APPEND, va("map %s\n", level.mapname));}

	// Boe!Man 11/2/10: New Map Switch/Restart system.
	if (level.mapSwitch == qtrue /* && level.mapSwitchCount == level.time */){
		if(level.mapAction == 1){
			if (g_compMode.integer > 0 && cm_enabled.integer == 1){
				if(level.time == level.mapSwitchCount + 2000){
					trap_SendConsoleCommand( EXEC_APPEND, va("map_restart 0\n"));
				}
			}
			else if (g_compMode.integer > 0 && cm_enabled.integer == 3){
				if(level.time == level.mapSwitchCount + 2000){
					/*
					// Boe!Man 11/17/10: Is auto swap enabled? -- Update 1/24/11: Swap the teams when the round has just ended.
					if (cm_aswap.integer > 0){
						Boe_SwapTeams(NULL);
					}*/
					trap_SendConsoleCommand( EXEC_APPEND, va("map_restart 0\n"));
				}
			}
			else{
			if(level.time == level.mapSwitchCount + 1000){
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%sM%sa%sp %sr%se%sstart in 4!", level.time + 1000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));}
			else if(level.time == level.mapSwitchCount + 2000){
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%sM%sa%sp %sr%se%sstart in 3!", level.time + 1000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));}
			else if(level.time == level.mapSwitchCount + 3000){
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%sM%sa%sp %sr%se%sstart in 2!", level.time + 1000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));}
			else if(level.time == level.mapSwitchCount + 4000){
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%sM%sa%sp %sr%se%sstart in 1!", level.time + 1000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));}
			else if(level.time == level.mapSwitchCount + 5000){
				if(current_gametype.value == GT_HS){
					trap_SendConsoleCommand( EXEC_APPEND, va("g_gametype h&s\n"));
				}
				trap_SendConsoleCommand( EXEC_APPEND, va("map_restart 0\n"));
			}
			}
		}
		else if(level.mapAction == 2){
			if(level.time == level.mapSwitchCount + 1000){
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%sM%sa%sp ^7%s in 4!", level.time + 1000, server_color1.string, server_color2.string, server_color3.string, level.mapSwitchName));}
			else if(level.time == level.mapSwitchCount + 2000){
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%sM%sa%sp ^7%s in 3!", level.time + 1000, server_color1.string, server_color2.string, server_color3.string, level.mapSwitchName));}
			else if(level.time == level.mapSwitchCount + 3000){
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%sM%sa%sp ^7%s in 2!", level.time + 1000, server_color1.string, server_color2.string, server_color3.string, level.mapSwitchName));}
			else if(level.time == level.mapSwitchCount + 4000){
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%sM%sa%sp ^7%s in 1!", level.time + 1000, server_color1.string, server_color2.string, server_color3.string, level.mapSwitchName));}
			else if(level.time == level.mapSwitchCount + 5000){
				trap_SendConsoleCommand( EXEC_APPEND, va("map %s\n", level.mapSwitchName));}
		}
		else if(level.mapAction == 3){
			if(level.time == level.mapSwitchCount + 3000){
				trap_Cvar_VariableStringBuffer ( "mapname", level.mapname, MAX_QPATH );
				trap_SendConsoleCommand( EXEC_APPEND, va("map %s\n", level.mapname));
			}
		}
	}

	// Boe!Man 11/16/10: Is compmode enabled?
	if (g_compMode.integer > 0){
		// Boe!Man 11/16/10: Keep displaying the scrim settings.
		if(cm_enabled.integer == 1 && level.compMsgCount == level.time){
			char *sl;
			char *as;
			char *ds;
			if (strstr(cm_slock.string, "1"))
				sl = "Yes";
			else
				sl = "No";
			if (strstr(cm_aswap.string, "1"))
				as = "Yes";
			else
				as = "No";
			if (strstr(cm_dsounds.string, "1"))
				ds = "Yes";
			else
				ds = "No";
			trap_SendServerCommand(-1, va("cp \"@%sMatch settings\n\n^7[^3Gametype^7]  %s%s %s\n^7[^3Scorelimit^7]  %s%i\n^7[^3Timelimit^7]  %s%i\n^7[^3Specs locked^7] %s%s\n^7[^3Auto swap^7] %s%s\n^7[^3Disable sounds^7] %s%s\n\n%sRestart map to start the first round!\"", 
				server_color3.string, server_color3.string, level.mapname, g_gametype.string, server_color3.string, cm_sl.integer, server_color3.string, cm_tl.integer, server_color3.string, sl, server_color3.string, as, server_color3.string, ds, server_color3.string));
			level.compMsgCount = level.time + 3000;
			}
		else if(cm_enabled.integer == 3 && level.compMsgCount == level.time){
			// Boe!Man 1/24/11: Swap the teams.
			if (cm_aswap.integer > 0 && level.swappedteams == qfalse){
				Boe_SwapTeams(NULL);
				level.swappedteams = qtrue;
			}
			if (level.teamScores[TEAM_RED] > level.teamScores[TEAM_BLUE]){
				trap_SendServerCommand(-1, va("cp \"@%sFirst round ended!\n\n^7[^3Red team^7] %sleads with %i - %i\n\n%sRestart map to start the second round!",
					server_color3.string, server_color3.string, level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE], server_color3.string));}
			else if (level.teamScores[TEAM_BLUE] > level.teamScores[TEAM_RED]){
				trap_SendServerCommand(-1, va("cp \"@%sFirst round ended!\n\n^7[^3Blue team^7] %sleads with %i - %i\n\n%sRestart map to start the second round!",
					server_color3.string, server_color3.string, level.teamScores[TEAM_BLUE], level.teamScores[TEAM_RED], server_color3.string));}
			level.compMsgCount = level.time + 3000;
		}
	}


	// Check warmup rules
	CheckWarmup();

	// see if it is time to end the level
	CheckExitRules();

	// Update gametype stuff
	CheckGametype ();

	trap_GT_RunFrame ( level.time );

	// cancel vote if timed out
	CheckVote();

	// for tracking changes
	CheckCvars();
	
	// Henk 21/01/10 -> Hide&Seek actions
	if(current_gametype.value == GT_HS)
	Henk_CheckHS();

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


