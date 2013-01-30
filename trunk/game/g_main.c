// Copyright (C) 2001-2002 Raven Software.
//

#include "g_local.h"
#include "boe_local.h"

#ifdef __linux__
unsigned char	memsys5[31457280]; // Boe!Man 1/29/13: Buffer of 30 MB, available for SQLite memory management (Linux).
#endif

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
vmCvar_t	g_rpmEnt;
vmCvar_t	g_passwordAdmins;
vmCvar_t	g_shortCommandStyle;
vmCvar_t	g_boxAttempts;
vmCvar_t	g_cageAttempts;
vmCvar_t	g_RpgStyle;
vmCvar_t	g_badminPass;
vmCvar_t	g_adminPass;
vmCvar_t	g_sadminPass;
vmCvar_t	g_badminPassword;
vmCvar_t	g_adminPassword;
vmCvar_t	g_sadminPassword;
vmCvar_t	g_adminPassFile;
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
vmCvar_t	hideSeek_roundstartdelay;
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
vmCvar_t    g_adminremove;
vmCvar_t	g_runover;						// Admin CVAR.
vmCvar_t	g_twist;						// Admin CVAR.
vmCvar_t	g_respawn;						// Admin CVAR.
vmCvar_t	server_badminprefix;
vmCvar_t	server_adminprefix;
vmCvar_t	server_sadminprefix;
vmCvar_t	server_acprefix;
vmCvar_t	server_scprefix;
vmCvar_t	server_caprefix;
vmCvar_t	server_ccprefix;
vmCvar_t	server_ctprefix;
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
vmCvar_t	g_noHighFps;
//vmCvar_t	g_333;							// Admin CVAR.
vmCvar_t	g_forceteam;					// Admin CVAR.
vmCvar_t	g_adminlist;
vmCvar_t	Owner;
vmCvar_t	Clan;
vmCvar_t	ClanURL;
vmCvar_t	HostedBy;
// Boe!Man 4/1/10
vmCvar_t	server_redteamprefix;
vmCvar_t	server_blueteamprefix;
// Boe!Man 1/19/11
vmCvar_t	server_hiderteamprefix;
vmCvar_t	server_seekerteamprefix;
vmCvar_t	server_humanteamprefix;
vmCvar_t	server_zombieteamprefix;
// Henk 01/04/10 -> Add g_disableNades
vmCvar_t	g_disableNades;
vmCvar_t	g_instaGib;
vmCvar_t	g_weaponModFlags;
vmCvar_t	g_allowthirdperson;
vmCvar_t	g_weaponswitch;
vmCvar_t	g_compMode;	
vmCvar_t	g_clanfile;
vmCvar_t	g_enableTeamCmds;
vmCvar_t	g_refpassword;
vmCvar_t	g_checkCountry;
// Boe!Man 2/27/11: Some new CVARs for nolower.
vmCvar_t	g_useNoLower;
vmCvar_t	g_autoNoLower;

// Boe!Man 6/2/12: CVAR for noroof.
vmCvar_t	g_useNoRoof;

// Boe!Man 4/15/10: Some level commands (Admin).
vmCvar_t	g_nolower;
vmCvar_t	g_noroof;
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
vmCvar_t	g_autoEvenTeams;
vmCvar_t	server_msgInterval;
vmCvar_t	server_enableServerMsgs;
vmCvar_t	server_enableTips;
vmCvar_t	server_message1;
vmCvar_t	server_message2;
vmCvar_t	server_message3;
vmCvar_t	server_message4;
vmCvar_t	server_message5;
vmCvar_t	server_msgDelay;

// Boe!Man 10/16/10
vmCvar_t	g_adminSpec;

// Boe!Man 11/16/10: Default scrim settings.
vmCvar_t	g_matchLockSpec;
vmCvar_t	g_matchSwapTeams;
vmCvar_t	g_matchTimeLimit;
vmCvar_t	g_matchScoreLimit;
vmCvar_t	g_matchDisableEvents;
vmCvar_t	g_matchRounds;
vmCvar_t	cm_enabled;
vmCvar_t	cm_sl;
vmCvar_t	cm_tl;
vmCvar_t	cm_slock;
vmCvar_t	cm_aswap;
vmCvar_t	cm_devents;
vmCvar_t	cm_dr;
vmCvar_t	cm_oldsl;
vmCvar_t	cm_oldtl;
vmCvar_t	cm_sr;
vmCvar_t	cm_sb;

// Boe!Man 12/13/10: Clonecheck CVARs.
vmCvar_t	g_aliasCheck;
vmCvar_t	g_aliasCount;
vmCvar_t	g_alternateMap;

// Boe!Man 1/26/11
vmCvar_t	g_cm;
vmCvar_t	g_3rd;
vmCvar_t	g_enableCustomCommands;

// Boe!Man 2/13/11
vmCvar_t	g_forcevote;

// Boe!Man 3/6/11
vmCvar_t	g_customCommandsFile;
vmCvar_t	g_tipsFile;
vmCvar_t	g_banfile;
vmCvar_t	hideSeek_Extra;
vmCvar_t	hideSeek_Nades;
vmCvar_t	hideSeek_Weapons;

// Boe!Man 3/8/11
vmCvar_t	g_enableAdminLog;
vmCvar_t	g_adminlog;

// Boe!Man 3/16/11
vmCvar_t	server_rconprefix;
vmCvar_t	server_specteamprefix;

// Boe!Man 4/20/11
//vmCvar_t	g_crossTheBridge;

// Boe!Man 5/28/11
vmCvar_t	g_customWeaponFile;

// Boe!man 6/17/11
vmCvar_t	g_pause;

// Boe!Man 8/18/11
vmCvar_t	g_dropLocationMessage;

// Boe!Man 8/25/11
vmCvar_t	g_logSpecialChat;

// Boe!Man 5/20/12
vmCvar_t	g_publicIPs;

// Boe!man 6/16/12
vmCvar_t	g_shuffleteams;

// Boe!Man 7/29/12
vmCvar_t	g_preferSubnets;

// Henk 10/30/12
vmCvar_t g_clientDeathMessages;

// Boe!Man 9/2/12
vmCvar_t	hideSeek_ExtendedRoundStats;

// Boe!Man 11/5/12
vmCvar_t	g_ff;

// Boe!Man 1/2/13: --- SQLite3 Related CVARs ---
vmCvar_t	sql_aliasFlushCount;
vmCvar_t	sql_timeBench;

#ifdef _DEBUG
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
	{ &g_log, "g_log", "logs/games.log", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
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
	{ &g_forcerespawn, "g_forcerespawn", "0", 0, 0.0, 0.0, 0, qtrue },
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

	{ &g_roundstartdelay,	"g_roundstartdelay", "3",		CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
	{ &hideSeek_roundstartdelay,	"hideSeek_roundstartdelay", "30",		CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },

	{ &g_availableWeapons,	"g_availableWeapons", "2222222222211", CVAR_ARCHIVE|CVAR_SERVERINFO|CVAR_LATCH|CVAR_CHEAT, 0.0, 0.0, 0, qfalse },
	{ &hideSeek_availableWeapons,	"hideSeek_availableWeapons", "200000000000022222222", CVAR_INTERNAL|CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse },
	{ &availableWeapons,	"availableWeapons", "2222222222211", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse },
	// Henk 01/04/10
	{ &g_disableNades,	"g_disableNades", "1", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse },
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
	{ &g_subnetban,					"g_subnetban",			"5",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
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
	{ &g_noroof,					"g_noroof",				"4",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
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
	{ &g_forcevote,					"g_forcevote",			"3",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
	{ &g_adminremove,				"g_adminremove",		"4",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
	{ &g_pause,						"g_pause",				"3",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
	{ &g_adminlist,					"g_adminlist",			"4",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse }, // Boe!Man 6/17/11: Solved bug where ALL admins could see the list (wasn't added to the CVAR table and thus 0).
	{ &g_shuffleteams,				"g_shuffleteams",		"3",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
	{ &g_ff,						"g_ff",					"4",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },

	{ &g_clanfile,			"g_clanfile",			"users/clanmembers.txt",	CVAR_ARCHIVE,	0.0,	0.0,  0, qfalse  },
	{ &g_banfile,			"g_banfile",			"users/bans.txt",		CVAR_ARCHIVE,	0.0,	0.0,  0, qfalse  },

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
	{ &server_scprefix, "server_scprefix", "^GS-^gA^Kdm^7in Only", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse},
	{ &server_caprefix, "server_caprefix", "^GH^ge^Ky ^7Admin!", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
	{ &server_ccprefix, "server_ccprefix", "^GC^gl^Kan ^7Only", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
	{ &server_starprefix, "server_starprefix", "^<*", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
	{ &server_ctprefix, "server_ctprefix", "^GC^gl^Ka^7n", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
	{ &server_rconprefix, "server_rconprefix", "^CS^be^kr^+v^7er", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
	{ &server_specteamprefix, "server_specteamprefix", "^CS^bp^ke^+c^7tators", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },

	{ &g_maxIPConnections,			"g_maxIPConnections",	"3",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse},
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
	{ &g_weaponModFlags, "g_weaponModFlags", "0", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
	{ &g_allowthirdperson, "g_allowThirdPerson", "1", CVAR_ARCHIVE|CVAR_SERVERINFO, 0.0, 0.0, 0,  qfalse },
	{ &g_compMode, "g_compMode", "0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0, 0, qfalse  },
	{ &g_enableTeamCmds, "g_enableTeamCmds", "1", CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue  },
	{ &g_refpassword, "g_refpassword", "none", CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue  },
	{ &g_checkCountry, "g_checkCountry", "1", CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue  },
	{ &g_useNoLower, "g_useNoLower", "1", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse  },
	{ &g_autoNoLower, "g_autoNoLower", "1", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
	{ &g_useNoRoof, "g_useNoRoof", "0", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse  },
	{ &g_autoEvenTeams, "g_autoEvenTeams", "1", CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue  },

	{ &server_enableServerMsgs, "server_enableServerMsgs", "1", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
	{ &server_enableTips, "server_enableTips", "0", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
	{ &server_msgInterval, "server_msgInterval", "5", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
	{ &server_msgDelay, "server_msgDelay", "2", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
	{ &server_message1, "server_message1", "Welcome to 1fx. Mod.", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
	{ &server_message2, "server_message2", "Development forums can be found on 1fx.uk.to.", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
	{ &server_message3, "server_message3", "Have fun!", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
	{ &server_message4, "server_message4", "", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },
	{ &server_message5, "server_message5", "", CVAR_ARCHIVE, 0.0, 0.0, 0,  qfalse },

	// Boe!Man 10/16/10
	{ &g_adminSpec, "g_adminSpec", "4", CVAR_ARCHIVE | CVAR_LATCH, 0.0, 0.0, 0,  qfalse },

	// Boe!Man 11/16/10: Default scrim settings.
	{ &g_matchLockSpec, "g_matchLockSpec", "1", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
	{ &g_matchSwapTeams, "g_matchSwapTeams", "1", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
	{ &g_matchTimeLimit, "g_matchTimeLimit", "0", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
	{ &g_matchScoreLimit, "g_matchScoreLimit", "10", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
	{ &g_matchDisableEvents, "g_matchDisableEvents", "1", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
	{ &g_matchRounds, "g_matchRounds", "2", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },

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
	{ &g_enableCustomCommands, "g_enableCustomCommands", "0", CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue  },
	{ &g_customCommandsFile,			"g_customCommandsFile",			"files/CustomCommands.txt",	CVAR_ARCHIVE,	0.0,	0.0,  0, qfalse  }, // Boe!Man 3/6/11: So users can change if desired.
	{ &g_tipsFile,						"g_tipsFile",					"files/tips.txt",	CVAR_ARCHIVE,	0.0,	0.0,  0, qfalse  }, // Boe!Man 6/24/11: So users can change if desired.
	{ &hideSeek_Extra,			"hideSeek_Extra",			"11011",	CVAR_ARCHIVE|CVAR_LATCH,	0.0,	0.0,  0, qfalse  }, // Boe!Man 3/6/11: So users can change if desired.
	{ &hideSeek_Nades,			"hideSeek_Nades",			"1111",	CVAR_ARCHIVE|CVAR_LATCH,	0.0,	0.0,  0, qfalse  }, // Boe!Man 3/6/11: So users can change if desired.
	{ &hideSeek_Weapons,		"hideSeek_Weapons",			"111",	CVAR_ARCHIVE|CVAR_LATCH,	0.0,	0.0,  0, qfalse  }, // Boe!Man 3/6/11: So users can change if desired.
	{ &hideSeek_ExtendedRoundStats,		"hideSeek_ExtendedRoundStats",		"1",	CVAR_ARCHIVE,	0.0,	0.0,  0, qfalse  }, // Boe!Man 9/2/12: CVAR for extended round stats.

	// Boe!Man 3/8/11: CVAR for the Admin logging.
	{ &g_enableAdminLog, "g_enableAdminLog", "1", CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue  },
	{ &g_adminlog, "g_adminlog", "logs/admin.log", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },

	{ &g_rpmEnt, "g_rpmEnt", "1", CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue  },

	{ &g_passwordAdmins, "g_passwordAdmins", "0", CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse  },
	{ &g_badminPass, "g_badminPass", "none", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
	{ &g_adminPass, "g_adminPass", "none", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
	{ &g_sadminPass, "g_sadminPass", "none", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
	// Boe!Man 9/1/12: Synonyms for the 'pass' CVARs.
	{ &g_badminPassword, "g_badminPassword", "none", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
	{ &g_adminPassword, "g_adminPassword", "none", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
	{ &g_sadminPassword, "g_sadminPassword", "none", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
	
	{ &g_adminPassFile, "g_adminPassFile", "users/passfile.txt", CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse  },
	{ &g_shortCommandStyle, "g_shortCommandStyle", "0", CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue  },
	{ &g_boxAttempts, "g_boxAttempts", "3",	CVAR_ARCHIVE,	0.0,	0.0,  0, qtrue  }, 
	{ &g_cageAttempts, "g_cageAttempts", "3",	CVAR_ARCHIVE,	0.0,	0.0,  0, qtrue  },
	{ &g_RpgStyle, "g_RpgStyle", "0",	CVAR_ARCHIVE,	0.0,	0.0,  0, qtrue  },
//	{ &g_crossTheBridge, "g_crossTheBridge", "0",	CVAR_ARCHIVE,	0.0,	0.0,  0, qfalse  },
	{ &g_noHighFps, "g_noHighFps", "0",	CVAR_ARCHIVE,	0.0,	0.0,  0, qtrue  },

	// Boe!Man 5/28/11
	{ &g_customWeaponFile,			"g_customWeaponFile",		 "none",		CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },

	// Boe!Man 8/18/11
	{ &g_dropLocationMessage,			"g_dropLocationMessage",		 "1",		CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue },

	// Boe!Man 8/25/11
	{ &g_logSpecialChat,			"g_logSpecialChat",		 "1",		CVAR_ARCHIVE, 0.0, 0.0, 0, qfalse },
	
	// Boe!Man 5/20/12
	{ &g_publicIPs,					"g_publicIPs",		 	"1",		CVAR_ARCHIVE, 0.0, 0.0, 0, qtrue },
	
	// Boe!Man 7/29/12
	{ &g_preferSubnets,				"g_preferSubnets",		 	"0",		CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse },
	
	// Boe!Man 1/2/13: --- SQLite3 Related CVARs ---
	{ &sql_aliasFlushCount,			"sql_aliasFlushCount",		"7500",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
	#ifdef _DEBUG
	{ &sql_timeBench,				"sql_timeBench",			"1",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
	#else
	{ &sql_timeBench,				"sql_timeBench",			"0",				CVAR_ARCHIVE,	0.0f,   0.0f, 0,  qfalse },
	#endif

	//{ &g_clientDeathMessages,		"g_clientDeathMessages",		 	"0",		CVAR_ARCHIVE|CVAR_LATCH, 0.0, 0.0, 0, qfalse },
	//http://1fx.uk.to/forums/index.php?/topic/1230-1fx-anticheat/page__view__findpost__p__13498
#ifdef _DEBUG
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
					}
				}
				
				// Boe!Man 9/1/12: Handle password CVAR synonyms here..
				if (strstr(cv->cvarName, "adminPass")){
					if(strstr(cv->cvarName, "badmin")){ // B-Admin synonym here..
						if(strstr(cv->cvarName, "Password")){
							trap_Cvar_Set("g_badminPass", cv->vmCvar->string);
						}else{
							trap_Cvar_Set("g_badminPassword", cv->vmCvar->string);
						}
					}else if(strstr(cv->cvarName, "g_admin")){ // Admin synonym here..
						if(strstr(cv->cvarName, "Password")){
							trap_Cvar_Set("g_adminPass", cv->vmCvar->string);
						}else{
							trap_Cvar_Set("g_adminPassword", cv->vmCvar->string);
						}
					}else if(strstr(cv->cvarName, "sadmin")){ // S-Admin synonym here..
						if(strstr(cv->cvarName, "Password")){
							trap_Cvar_Set("g_sadminPass", cv->vmCvar->string);
						}else{
							trap_Cvar_Set("g_sadminPassword", cv->vmCvar->string);
						}
					}
				}
				// End Boe!Man - CVAR synonyms.
					

				cv->modificationCount = cv->vmCvar->modificationCount;

				if ( cv->trackChange ) 
				{
					//trap_SendServerCommand( -1, va("print \"Server: %s changed to %s\n\"", cv->cvarName, cv->vmCvar->string ) );
					trap_SendServerCommand( -1, va("print \"^3[Rcon Action] ^7%s changed to %s.\n\"", cv->cvarName, cv->vmCvar->string ) );
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
	int weapon;
	char	 available[WP_NUM_WEAPONS+1];

	if(current_gametype.value == GT_HS)
		strcpy(available, hideSeek_availableWeapons.string);
	else
		strcpy(available, availableWeapons.string);

	for ( weapon = WP_KNIFE; weapon < WP_NUM_WEAPONS; weapon++)
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
}

/*
===============
G_UpdateAvailableWeapons

Updates the g_availableWeapons cvar using the disable cvars.
===============
*/
void G_UpdateAvailableWeapons ( void )
{
	int weapon;
	char	 available[WP_NUM_WEAPONS+1];

	memset ( available, 0, sizeof(available) );

	for ( weapon = WP_KNIFE; weapon < WP_NUM_WEAPONS; weapon ++ )
	{
		gitem_t* item = BG_FindWeaponItem ( (weapon_t)weapon );
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
	char realgametype[8] = "0000";
	char mapname[MAX_QPATH];
	qboolean doit = qfalse;
	qboolean check, check1;
	if(strstr(gametype, "h&s")){
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
	//	|| G_DoesMapSupportGametype ( bg_gametypeData[level.gametype].basegametype ) )
	//	|| level.gametype == -1 )
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
	/*
	char test[128];
	char stable[128];
	char version[64];
	*/
	// Boe!Man 3/30/10
	Com_Printf ("------- Game Initialization -------\n");
	Com_Printf ("Mod: %s %s\n", INF_STRING, INF_VERSION_STRING);
//#ifdef Q3_VM
	Com_Printf ("Date: %s\n", INF_VERSION_DATE);
/*#else
	Com_Printf ("Date: %d/%d/%02d\n", MONTH+1, DAY, YEAR );
#endif*/
	#ifdef __linux__
	// Boe!Man 1/29/13: Initialize the in-game memory-management buffer on Linux (SQLite3 memsys5).
	sqlite3_config(SQLITE_CONFIG_HEAP, memsys5, 31457280, 64);
	sqlite3_soft_heap_limit(31457280);
	#endif
	
	//Henk 12/10/12 -> Copy disk database to memory database.
	LoadCountries();

	srand( randomSeed );

	// set some level globals
	memset( &level, 0, sizeof( level ) );
	level.time = levelTime;
	level.startTime = levelTime;
	level.nolower[2] = -10000000;
	level.cagefightloaded = qfalse;
	G_RegisterCvars();

	//G_ProcessIPBans();

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
			// Boe!Man 10/4/12: Reset g_gametype to set the gt latched, so it will remain effective upon the next /rcon map switch..
			trap_SendConsoleCommand( EXEC_APPEND, va("g_gametype h&s\n"));
		}else if(strstr(g_gametype.string, "h&z")){
			trap_Cvar_Set("current_gametype", "8");
			trap_Cvar_Set( "g_gametype", "inf" );
			trap_Cvar_Update(&g_gametype);
			// Boe!Man 10/4/12: Reset g_gametype to set the gt latched, so it will remain effective upon the next /rcon map switch..
			trap_SendConsoleCommand( EXEC_APPEND, va("g_gametype h&z\n"));
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

	// Boe!Man 2/5/11: Force col9 to be a H&S map only.
	// Boe!Man 4/20/11: Removed on request: http://1fx.uk.to/index.php?/tracker/issue-198-col9-not-working-with-ctf/ -- We now check if the g_crossTheBridge CVAR is enabled.
	// Boe!Man 10/14/12: A CVAR for this looks SO messy, FIX by checking this in the worldspawn.
	/*trap_Cvar_VariableStringBuffer ( "mapname", level.mapname, MAX_QPATH );
	if(strstr(level.mapname, "col9") && current_gametype.value == GT_HS && g_crossTheBridge.integer > 0){
		level.crossTheBridge = qtrue;
	}*/

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
	if(current_gametype.value == GT_HS){
		AddSpawnField("classname", "gametype_item");
		AddSpawnField("targetname", "briefcase");
		AddSpawnField("gametype", "inf");
		AddSpawnField("origin", "9999 9999 9999"); // Boe!Man 9/10/11: "999 999 999" bugs in a few maps (start solid), so changed the origin a bit.
		trap_UnlinkEntity(&g_entities[G_SpawnGEntityFromSpawnVars(qtrue)]);
	}

	// parse the key/value pairs and spawn gentities
	G_SpawnEntitiesFromString(qfalse);

	InitSpawn(1);
	InitSpawn(2);
	InitSpawn(3);
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
		G_InitBots( (qboolean)restart );
	}
#endif

	G_RemapTeamShaders();
	// Henk 22/01/11 -> New weapon cvars.
	G_UpdateDisableCvars(); // Set the disabled_ cvars from availableWeapons and hideSeek_availableWeapons

	G_UpdateAvailableWeapons(); // also set the original g_availableWeapons for the client :)
	// End
	if(current_gametype.value != GT_HS){
		//trap_Cvar_Set("g_disableNades", "1");
		//trap_Cvar_Update(&g_disableNades);
		//trap_Cvar_Set("g_roundstartdelay", "3");
		//trap_Cvar_Update(&g_roundstartdelay);
	}


	// Boe!Man 11/13/12: New check for Nades, see if they are used at all in the server (faster !nn checking, and proper backpack fix).
	if(SetNades("0")){ // 0 means disable_* CVAR to 0, so enabled.
		level.nadesFound = qtrue;
	}
	// Set the available outfitting -- Boe!Man 5/22/12: Update, now also checks H&S settings.
	if(g_disableNades.integer){
		SetNades("1");
	}
	BG_SetAvailableOutfitting(g_availableWeapons.string);

	// Initialize the gametype
	/*
	if(current_gametype.value == GT_HS)
		trap_GT_Init ( "h&s", restart );
	else if(current_gametype.value == GT_HZ)
		trap_GT_Init ( "h&z", restart );
	else
	*/
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

	trap_SetConfigstring( CS_VOTE_TIME, "" );
	
	// setup settings for h&s
	if(current_gametype.value == GT_HS || current_gametype.value == GT_HZ){
		SetNades("0"); // Enable them, but check them individually in that func cause' it's H&S.
	}
	
	// Boe!Man 3/30/10
	Boe_ParseChatSounds();

	// Boe!Man 12/8/12: Check database integrity.
	Boe_userdataIntegrity();

	// Boe!Man 1/17/11: Check for modifications on the motd and version.
	/*
	strncpy(test, TEST_VERSION, 127);
	strncpy(stable, STABLE_VERSION, 127);
	strncpy(version, INF_VERSION_STRING_COLORED, 63);
	if(test[15] != 'B' || test[18] != '!' || test[21] != 'n' || test[25] != '&' || test[29] != 'H' || test[32] != 'k' || test[38] != 'R' || test[43] != 'n' || test[48] != 'T' || test[53] != 'v' || test[68] != 'M'){
		Com_Error(ERR_FATAL, "Unexpected return on static value.");
	}else if (stable[15] != 'B' || stable[18] != '!' || stable[21] != 'n' || stable[25] != '&' || stable[29] != 'H' || stable[32] != 'k' || stable[36] != '1' || stable[40] != 'u' || stable[48] != '|' || stable[53] != '3'  || stable[56] != 'n'){
		Com_Error(ERR_FATAL, "Unexpected return on static value.");
	}else if (version[2] != '1' || version[4] != 'x' || version[7] != '.' || version[11] != 'M' || version[13] != 'd'){
		Com_Error(ERR_FATAL, "Unexpected return on static value.");
	}*/

	if(g_useNoLower.integer){
		level.nolower1 = qtrue;
		if (strstr(level.mapname, "mp_kam2")){
			SpawnFence(1); // first one fails dunno why..
			SpawnFence(1);
			SpawnFence(2);
			SpawnFence(3);
			SpawnFence(4);
		}
	}else{
		level.nolower1 = qfalse;
		if (strstr(level.mapname, "mp_kam2")){
			RemoveFence();
		}
	}
	
	// Boe!Man 6/2/12: Check for noroof.
	if(g_useNoRoof.integer){
		level.noroof1 = qtrue;
	}else{
		level.noroof1 = qfalse;
	}
	
	// Boe!Man 7/29/12: Check for g_preferSubnets and g_passwordAdmins not both being set to 1 (incompatible).
	if(g_passwordAdmins.integer && g_preferSubnets.integer){
		trap_Cvar_Set("g_passwordAdmins", "0");
		trap_Cvar_Update(&g_passwordAdmins);
		Com_Printf("Info: g_passwordAdmins has been set to 0 due to g_preferSubnets being set to %i. Set g_preferSubnets to 0 to allow Admins to login using a password.\n", g_preferSubnets.integer);
	}

	// Boe!Man 11/16/10: Scrim settings.
	if (g_compMode.integer > 0){
		#ifdef _DEBUG
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
		}
		if (cm_enabled.integer == 0){ // Boe!Man 6/2/11: Little piece of error handling. When someone decides to reboot the server; either a crash or intented, Competition Mode didn't completely shut down. We don't worry about the score- or timelimit, Config should always be executed first?
			trap_Cvar_Set("g_compMode", "0");
			level.compMsgCount = 0;
		}
		#ifdef _DEBUG
		if (strstr(boe_log.string, "2"))
			G_LogPrintf("3e\n");
		#endif
	}
	
	// Boe!Man 9/1/12: Check CVAR synonyms at start up to keep them in sync.
	if(!strstr(g_badminPassword.string, g_badminPass.string)){
		trap_Cvar_Set("g_badminPassword", g_badminPass.string);
	}
	if(!strstr(g_adminPassword.string, g_adminPass.string)){
		trap_Cvar_Set("g_adminPassword", g_adminPass.string);
	}
	if(!strstr(g_sadminPassword.string, g_sadminPass.string)){
		trap_Cvar_Set("g_sadminPassword", g_sadminPass.string);
	}
	// End Boe!Man - CVAR synonyms.
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
	
	// Boe!Man 1/27/13: Reset the internal g_alternateMap CVAR if an alt map is loaded and the map is restarted.
	if(restart){
		if(level.altEnt){
			trap_Cvar_Set( "g_alternateMap", "1");
			trap_Cvar_Update ( &g_alternateMap );
		}
	}

	//Henk 12/10/12 -> Detach and close memory database
	UnloadCountries();

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
		gspawn_t* spawn = G_SelectRandomSpawnPoint ( (team_t)-1 );
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

	#ifdef _DEBUG
		if (strstr(boe_log.string, "2"))
			G_LogPrintf("4s\n");
	#endif

	///Ryan march 21 2004 9:19am
	if ((!*g_mapcycle.string || !Q_stricmp ( g_mapcycle.string, "none" ) || g_compMode.integer > 0 && cm_enabled.integer >= 1))
	{
		trap_SendServerCommand( -1, va("cp \"@ \n\""));
		if (g_compMode.integer > 0 && cm_enabled.integer == 5){
			#ifdef _DEBUG
			if (strstr(boe_log.string, "2"))
				G_LogPrintf("4 - 1\n");
			#endif
			trap_SendConsoleCommand( EXEC_APPEND, va("map %s\n", level.mapname ));
		}
		else{
			#ifdef _DEBUG
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

	#ifdef _DEBUG
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
		return (qboolean)(level.teamScores[TEAM_RED] == level.teamScores[TEAM_BLUE]);
	}

	a = level.clients[level.sortedClients[0]].sess.score;
	b = level.clients[level.sortedClients[1]].sess.score;

	return (qboolean)(a == b);
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
	/* Henk 07/03/11 -> This will destroy our timelimit hit message and action.
	if ( ScoreIsTied() ) // HENK FIX ME -> Add allow tie variable? 
	{
		// always wait for sudden death
		return;
	}*/

	// Check to see if the timelimit was hit
	if ( g_timelimit.integer && !level.warmupTime && level.timelimithit != qtrue && level.cagefight != qtrue ) 
	{
		if ( level.time - level.startTime >= (g_timelimit.integer + level.timeExtension)*60000 ) 
		{
			if(strstr(g_gametype.string, "inf") || strstr(g_gametype.string, "elim")){
			trap_SendServerCommand( -1, va("print \"^3[Info] ^7Timelimit hit, waiting for round to finish.\n\"") );
			level.timelimithit = qtrue;
			}else{
				gentity_t*	tent;
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
								trap_SendServerCommand( -1, va("print \"^3[Info] ^7Timelimit hit, waiting for the final flag to be captured.\n\""));
								level.timelimitMsg = qtrue;
							}
							return;
						}
					}
					Boe_compTimeLimitCheck();
				}else{
					LogExit( "Timelimit hit." );
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
				gentity_t* tent;
				tent = G_TempEntity( vec3_origin, EV_GAME_OVER );
				if(cm_enabled.integer > 0){ // Boe!Man 3/18/11: Only change the entry if competition mode's enabled.
					//if (cm_enabled.integer < 4 && cm_dr.integer == 1){
					//	tent->s.eventParm = GAME_OVER_SCORELIMIT;
					//}else{
						tent->s.eventParm = LEEG;
					//}
				}else{
					tent->s.eventParm = GAME_OVER_SCORELIMIT;
				}
				tent->r.svFlags = SVF_BROADCAST;	
				tent->s.otherEntityNum = TEAM_RED;
				#ifdef _DEBUG
				if (strstr(boe_log.string, "2"))
					G_LogPrintf("5\n");
				#endif
				if (g_compMode.integer > 0 && cm_enabled.integer == 2){
					//LogExit(va("%s ^7team wins 1st round with %i - %i", server_redteamprefix, level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE] ));
					if(cm_dr.integer == 1){ // Boe!Man 3/18/11: If dual rounds are enabled, make use of them and display the temporary stuff.
						trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%s ^7team wins the 1st round with %i - %i!", level.time + 10000, server_redteamprefix.string, level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE]));
						// Boe!Man 11/18/10: Set the scores right (for logging purposes).
						if (cm_aswap.integer == 0){
							trap_Cvar_Set("cm_sr", va("%i", level.teamScores[TEAM_RED]));
							trap_Cvar_Set("cm_sb", va("%i", level.teamScores[TEAM_BLUE]));
						}else{
							// Boe!Man 11/19/10: Log the scores the other way around as the teams will get swapped the next round.
							trap_Cvar_Set("cm_sr", va("%i", level.teamScores[TEAM_BLUE]));
							trap_Cvar_Set("cm_sb", va("%i", level.teamScores[TEAM_RED]));
						}
						trap_SendServerCommand(-1, va("print\"^3[Info] ^7Red team wins the 1st round with %i - %i.\n\"", level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE] ));
						trap_Cvar_Set("cm_enabled", "21"); // Boe!Man 3/18/11: Display the awards prior to displaying this view again.
						LogExit("Red team wins the 1st round.");
					}else{ // Boe!Man 3/18/11: Red team won the match.
						trap_SendServerCommand(-1, va("print\"^3[Info] ^7Red team wins the match with %i - %i.\n\"", level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE]));
						trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%s ^7team wins the match with %i - %i!", level.time + 10000, server_redteamprefix.string, level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE]));
						trap_Cvar_Set("cm_enabled", "5"); // Boe!Man 11/18/10: 5 - Scrim Ended.
						LogExit("Red team wins the match.");
					}
				}else if (g_compMode.integer > 0 && cm_enabled.integer == 4){
					if (cm_sr.integer > cm_sb.integer){
						trap_SendServerCommand(-1, va("print\"^3[Info] ^7Red team won the 1st round with %i - %i.\n\"", cm_sr.integer, cm_sb.integer));
					}else if(cm_sr.integer < cm_sb.integer){
						trap_SendServerCommand(-1, va("print\"^3[Info] ^7Blue team won the 1st round with %i - %i.\n\"", cm_sb.integer, cm_sr.integer));
					}else{
						trap_SendServerCommand(-1, va("print\"^3[Info] ^7Round draw 1st round with %i - %i.\n\"", cm_sb.integer, cm_sr.integer));
					}
					trap_SendServerCommand(-1, va("print\"^3[Info] ^7Red team won the 2nd round with %i - %i.\n\"", level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE] ));
					trap_Cvar_Set("cm_enabled", "5"); // Boe!Man 11/18/10: 5 - Scrim Ended.
					Boe_calcMatchScores();
				}else{
					if(current_gametype.value == GT_HS){
						#ifdef _DEBUG
						Com_Printf("Updating scores..\n");
						#endif
						UpdateScores();
						LogExit("Hiders have won");
					}else
					LogExit( "Red team hit the score limit." );
				}
				return;
			}

			if ( level.teamScores[TEAM_BLUE] >= g_scorelimit.integer ) 
			{
				gentity_t* tent;
				tent = G_TempEntity( vec3_origin, EV_GAME_OVER );
				if(cm_enabled.integer > 0){ // Boe!Man 3/18/11: Only change the entry if competition mode's enabled.
					//if (cm_enabled.integer < 4 && cm_dr.integer == 1){
					//	tent->s.eventParm = GAME_OVER_SCORELIMIT;
					//}else{
						tent->s.eventParm = LEEG;
					//}
				}else{
					tent->s.eventParm = GAME_OVER_SCORELIMIT;
				}
				tent->r.svFlags = SVF_BROADCAST;	
				tent->s.otherEntityNum = TEAM_BLUE;
				#ifdef _DEBUG
				if (strstr(boe_log.string, "2"))
					G_LogPrintf("5\n");
				#endif
				if (g_compMode.integer > 0 && cm_enabled.integer == 2){
					if(cm_dr.integer == 1){ // Boe!Man 3/18/11: If dual rounds are enabled, make use of them and display the temporary stuff.
						trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%s ^7team wins the 1st round with %i - %i!", level.time + 10000, server_blueteamprefix.string, level.teamScores[TEAM_BLUE], level.teamScores[TEAM_RED]));
						// Boe!Man 11/18/10: Set the scores right (for logging purposes).
						if (cm_aswap.integer == 0){
							trap_Cvar_Set("cm_sr", va("%i", level.teamScores[TEAM_RED]));
							trap_Cvar_Set("cm_sb", va("%i", level.teamScores[TEAM_BLUE]));
						}else{
							// Boe!Man 11/19/10: Log the scores the other way around as the teams will get swapped the next round.
							trap_Cvar_Set("cm_sr", va("%i", level.teamScores[TEAM_BLUE]));
							trap_Cvar_Set("cm_sb", va("%i", level.teamScores[TEAM_RED]));
						}
						trap_SendServerCommand(-1, va("print\"^3[Info] ^7Blue team wins the 1st round with %i - %i.\n\"", level.teamScores[TEAM_BLUE], level.teamScores[TEAM_RED] ));
						trap_Cvar_Set("cm_enabled", "21");
						LogExit("Blue team wins the 1st round.");
					}else{ // Boe!Man 3/18/11: Blue team won the match.
						trap_SendServerCommand(-1, va("print\"^3[Info] ^7Blue team wins the match with %i - %i.\n\"", level.teamScores[TEAM_BLUE], level.teamScores[TEAM_RED]));
						trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@%s ^7team wins the match with %i - %i!", level.time + 10000, server_blueteamprefix.string, level.teamScores[TEAM_BLUE], level.teamScores[TEAM_RED]));
						trap_Cvar_Set("cm_enabled", "5"); // Boe!Man 11/18/10: 5 - Scrim Ended.
						LogExit("Blue team wins the match.");
					}
				}else if (g_compMode.integer > 0 && cm_enabled.integer == 4){
					if (cm_sr.integer > cm_sb.integer){
						trap_SendServerCommand(-1, va("print\"^3[Info] ^7Red team won the 1st round with %i - %i.\n\"", cm_sr.integer, cm_sb.integer));
					}else if(cm_sr.integer < cm_sb.integer){
						trap_SendServerCommand(-1, va("print\"^3[Info] ^7Blue team won the 1st round with %i - %i.\n\"", cm_sb.integer, cm_sr.integer));
					}else{
						trap_SendServerCommand(-1, va("print\"^3[Info] ^7Round draw 1st round with %i - %i.\n\"", cm_sb.integer, cm_sr.integer));
					}
					trap_SendServerCommand(-1, va("print\"^3[Info] ^7Blue team won the 2nd round with %i - %i.\n\"", level.teamScores[TEAM_BLUE], level.teamScores[TEAM_RED] ));
					trap_Cvar_Set("cm_enabled", "5"); // Boe!Man 11/18/10: 5 - Scrim Ended.
					Boe_calcMatchScores();
				}
				else{
					if(current_gametype.value == GT_HS){
						#ifdef _DEBUG
						Com_Printf("Updating scores..\n");
						#endif
						UpdateScores();
						LogExit("Seekers have won");
					}else
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
		// Boe!Man 9/10/11: Fix for calling a vote for a map defaulting to INF under H&S/H&Z.
		if(strstr(level.voteString, "map")){
			if(current_gametype.value == GT_HS){
				trap_Cvar_Set( "g_gametype", "h&s");
			}else if(current_gametype.value == GT_HZ){
				trap_Cvar_Set( "g_gametype", "h&z");
			}
		}
		
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
			// Boe!Man 2/13/11: Make sure the vote hasn't been forced by an Admin.
			if (level.forceVote == qfalse){
				trap_SendServerCommand( -1, "print \"Vote passed.\n\"" );
			}else{ // Boe!Man 2/13/11: Else it must've been, just reset the forceVote state and don't broadcast the message.. again..
				level.forceVote = qfalse;
			}
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
	attackData_t *attack;
	for ( i = 0; i < level.numConnectedClients; i ++ )
	{
		g_entities[level.sortedClients[i]].client->sess.timeOfDeath = 0;
		g_entities[level.sortedClients[i]].client->noOutfittingChange = qfalse;
		G_UpdateOutfitting ( g_entities[level.sortedClients[i]].s.number );
		if(g_entities[level.sortedClients[i]].client->sess.team == TEAM_BLUE){
			if(hideSeek_Extra.string[MDN11] == '1'){
				attack = &weaponData[WP_MDN11_GRENADE].attack[ATTACK_NORMAL];
				g_entities[level.sortedClients[i]].client->ps.ammo[weaponData[WP_MDN11_GRENADE].attack[ATTACK_NORMAL].ammoIndex]=attack->clipSize;
				g_entities[level.sortedClients[i]].client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_MDN11_GRENADE );
				g_entities[level.sortedClients[i]].client->ps.clip[ATTACK_NORMAL][WP_MDN11_GRENADE]=attack->extraClips;
			}
			if(hideSeek_Extra.string[F1] == '1'){
				attack = &weaponData[WP_F1_GRENADE].attack[ATTACK_NORMAL];
				g_entities[level.sortedClients[i]].client->ps.ammo[weaponData[WP_F1_GRENADE].attack[ATTACK_NORMAL].ammoIndex]=attack->clipSize;
				g_entities[level.sortedClients[i]].client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_F1_GRENADE );
				g_entities[level.sortedClients[i]].client->ps.clip[ATTACK_NORMAL][WP_F1_GRENADE]=attack->extraClips;
			}
			if(hideSeek_Extra.string[L2A2] == '1'){
				attack = &weaponData[WP_L2A2_GRENADE].attack[ATTACK_NORMAL];
				g_entities[level.sortedClients[i]].client->ps.ammo[weaponData[WP_L2A2_GRENADE].attack[ATTACK_NORMAL].ammoIndex]=attack->clipSize;
				g_entities[level.sortedClients[i]].client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_L2A2_GRENADE );
				g_entities[level.sortedClients[i]].client->ps.clip[ATTACK_NORMAL][WP_L2A2_GRENADE]=attack->extraClips;
			}
		}
	}
}

void Henk_CheckZombie(void){
	int i, random;
	gentity_t *ent;
	if(TeamCount1(TEAM_BLUE) >= 3 && level.messagedisplay1 == qfalse){
		trap_SendServerCommand(-1, va("print \"^3[H&Z] ^7Zombie team has been unlocked.\n\"") );
		level.messagedisplay1 = qtrue;
	}else if(TeamCount1(TEAM_BLUE) < 3 && level.messagedisplay1 == qtrue){
		level.messagedisplay1 = qfalse;
	}

	if(level.time >= level.gametypeStartTime+5000 && TeamCount1(TEAM_RED) >= 2){
		if(TeamCount1(TEAM_BLUE) == 0 && TeamCount1(TEAM_RED) >= 1 && level.messagedisplay2 == qtrue){
			level.zombie = -1;
			level.messagedisplay2 = qfalse;
		}
		if(level.zombie == -1 && TeamCount1(TEAM_BLUE) == 0 && level.time >= level.zombietime+10000){
			random = irand(0, level.numConnectedClients);
			ent = &g_entities[level.sortedClients[random]];
			if(ent->client->sess.team == TEAM_RED && ent->client->pers.connected == CON_CONNECTED && !G_IsClientDead(ent->client)){
				if(ent->client->sess.team != TEAM_SPECTATOR){ // extra check
				level.zombie = ent->s.number;
				level.zombietime = level.time+5000;
				trap_SendServerCommand( ent->s.number, va("cp \"You will turn into a zombie in 5 seconds!\n\""));
				}else{
					#ifdef _DEBUG
					Com_Printf("Weird case\n");
					#endif
				}
			}
		}
	}

	if(level.time >= level.zombietime && level.messagedisplay2 == qfalse){
		if(TeamCount1(TEAM_RED) >= 2 && TeamCount1(TEAM_BLUE) == 0){
			if(level.zombie != -1){
				if(g_entities[level.zombie].client->sess.team != TEAM_SPECTATOR && !G_IsClientDead(g_entities[level.zombie].client)){
					trap_SendServerCommand(-1, va("print \"^3[H&Z] ^7%s suddenly turned into a zombie!\n\"", g_entities[level.zombie].client->pers.netname) );
					trap_SendServerCommand( -1, va("cp \"%s ^7has turned into a zombie!\n\"", g_entities[level.zombie].client->pers.netname));
					// turn into zombie
					CloneBody(&g_entities[level.zombie], g_entities[level.zombie].s.number);
					g_entities[level.zombie].client->sess.firstzombie = qtrue;
					level.messagedisplay2 = qtrue;
					level.zombie = -1;
				}else{
					level.zombie = -1;
				}
			}
		}else if(TeamCount1(TEAM_RED) < 2 && TeamCount1(TEAM_BLUE ) == 0 && level.zombie != -1){
			level.zombie = -1;
		}
	}

	if(level.time >= level.gametypeStartTime+8000 && level.messagedisplay == qfalse && level.gametypeStartTime >= 5000){
		trap_SendServerCommand(-1, va("print \"^3[H&Z] ^7Shotguns distributed.\n\"") );
		trap_SendServerCommand( -1, va("cp \"^7%sS%sh%so%st%sg%suns distributed!\n\"", server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		Boe_GlobalSound( G_SoundIndex("sound/misc/menus/click.wav")); 

		for(i=0;i<level.numConnectedClients;i++){
			ent = &g_entities[level.sortedClients[i]];
			if(!ent)
				continue;

			if(ent->client->sess.team == TEAM_RED){
				ent->client->ps.ammo[weaponData[WP_M590_SHOTGUN].attack[ATTACK_NORMAL].ammoIndex]=9;
				ent->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_M590_SHOTGUN );
				ent->client->ps.clip[ATTACK_NORMAL][WP_M590_SHOTGUN]=9;
				ent->client->ps.firemode[WP_RPG7_LAUNCHER] = BG_FindFireMode ( WP_M590_SHOTGUN, ATTACK_NORMAL, WP_FIREMODE_AUTO );
				//ent->client->ps.weapon = WP_KNIFE;
				ent->client->ps.weaponstate = WEAPON_READY;
			}
		}
		level.messagedisplay = qtrue;
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

	//g_autoEvenTeams.integer == 1
	if(level.time >= level.gametypeDelayTime-1000 && level.aetdone == qfalse && level.gametypeStartTime >= 5000 && g_autoEvenTeams.integer == 1 && level.cagefight != qtrue){
		EvenTeams_HS(NULL, qtrue);
		level.aetdone = qtrue;
	}

	if(level.time >= level.gametypeDelayTime && level.messagedisplay == qfalse && level.gametypeStartTime >= 5000 && !level.crossTheBridge && level.cagefight != qtrue && level.time < level.gametypeRoundTime){
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
						if(hideSeek_Extra.string[BRIEFCASE] == '1'){
							G_RealSpawnGametypeItem1 ( BG_FindGametypeItem (0), g_entities[level.lastseek].r.currentOrigin, g_entities[level.lastseek].s.angles, qtrue );
							trap_SendServerCommand(-1, va("print\"^3[H&S] ^7Briefcase given to round winner %s: %i kills last round.\n\"", g_entities[level.lastseek].client->pers.netname, level.rememberSeekKills));
						}
					}else{
						if(hideSeek_Extra.string[BRIEFCASE] == '1'){
							G_RealSpawnGametypeItem1 ( BG_FindGametypeItem (0), g_entities[level.sortedClients[random]].r.currentOrigin, g_entities[level.sortedClients[random]].s.angles, qtrue );
							trap_SendServerCommand(-1, va("print\"^3[H&S] ^7Briefcase given at random to %s.\n\"", g_entities[level.sortedClients[random]].client->pers.netname));
						}
					}
				}
			}else{
				trap_SendServerCommand(-1, va("print\"^3[H&S] ^7Can't find any seeker.\n\""));
			}
		}else{
			if(hideSeek_Extra.string[BRIEFCASE] == '1'){
				spawnPoint = G_SelectRandomSpawnPoint ( TEAM_BLUE );
				// Boe!Man 5/7/12: Fixing crash issue. The briefcase MUST have a location to spawn.
				if(spawnPoint){
					G_RealSpawnGametypeItem1 ( BG_FindGametypeItem (0), spawnPoint->origin, spawnPoint->angles, qtrue );			
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
			trap_SendServerCommand(-1, va("print\"^3[H&S] ^7RPG given to round winner %s.\n\"", g_entities[rpgwinner].client->pers.netname));
			g_entities[rpgwinner].client->sess.takenRPG += 1;
			trap_SendServerCommand(g_entities[rpgwinner].s.number, va("cp \"^7You now have the %sR%sP%sG^7!\n\"", server_color1.string, server_color2.string, server_color3.string));
			// End
		}else if(rpgwinner >= 100 && m4winner < 100){
				spawnPoint = G_SelectRandomSpawnPoint ( TEAM_BLUE );
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
					trap_SendServerCommand(g_entities[level.sortedClients[random]].s.number, va("cp \"^7You now have the %sR%sP%sG^7!\n\"", server_color1.string, server_color2.string, server_color3.string));
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
			trap_SendServerCommand(-1, va("print\"^3[H&S] ^7M4 given to round winner %s.\n\"", g_entities[m4winner].client->pers.netname));
			g_entities[m4winner].client->sess.takenM4 += 1;
			trap_SendServerCommand(g_entities[m4winner].s.number, va("cp \"^7You now have the %sM%s4^7!\n\"", server_color1.string, server_color2.string));
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
				trap_SendServerCommand(g_entities[level.sortedClients[random]].s.number, va("cp \"^7You now have the %sM%s4^7!\n\"", server_color1.string, server_color2.string));
				break;
			}
		}else{
			spawnPoint = G_SelectRandomSpawnPoint ( TEAM_BLUE );
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
				if(g_entities[ent->client->sess.zombiebody].s.pos.trType == TR_STATIONARY){
					SetTeam(ent, "blue", NULL, qtrue);
					G_StopFollowing ( ent );
					ent->client->ps.pm_flags &= ~PMF_GHOST;
					ent->client->ps.pm_type = PM_NORMAL;
					ent->client->sess.ghost = qfalse;
					trap_UnlinkEntity (ent);
					ClientSpawn(ent);
					//trap_SendServerCommand(-1, va("print\"^3[Debug] ^7Teleport angles: %s.\n\"", vtos(ent->client->sess.tempangles)));
					TeleportPlayer(ent, g_entities[ent->client->sess.zombiebody].r.currentOrigin, ent->client->sess.tempangles, qtrue);
					SetClientViewAngle(ent, ent->client->sess.tempangles, qfalse);
					//G_FreeEntity(&g_entities[ent->client->sess.zombiebody]);
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
			}else if(current_gametype.value == GT_HZ && G_IsClientDead(ent->client)){
				if(ent->client->sess.team == TEAM_BLUE || TeamCount1(TEAM_BLUE) == 0 && ent->client->sess.team == TEAM_RED){
				if(ent->client->sess.zombie == qfalse){
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
	// Boe!Man 8/25/10: Auto restart after 60000000 milliseconds, or 1000 minutes with an empty server. This ensures no crashes.
	// FIX ME (Prio low): Bots aren't supported as of right now.
	if ( level.time - level.startTime >= 60000000 && level.numConnectedClients == 0){
		#ifdef _DEBUG
			if (strstr(boe_log.string, "1"))
				G_LogPrintf("2\n");
		#endif
		trap_Cvar_VariableStringBuffer ( "mapname", level.mapname, MAX_QPATH );
		trap_SendConsoleCommand( EXEC_APPEND, va("map %s\n", level.mapname));}

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
				trap_SendServerCommand(-1, va("cp \"@%sMatch settings\n\n^7[^3Gametype^7]  %s%s %s\n^7[^3Scorelimit^7]  %s%i\n^7[^3Timelimit^7]  %s%i\n^7[^3Specs locked^7] %s%s\n^7[^3Disable events^7] %s%s\n^7[^3# of Rounds^7] %s%s\n^7[^3Auto swap^7] %s%s\n\n%sRestart map to start the first round!\"", 
					server_color3.string, server_color3.string, level.mapname, g_gametype.string, server_color3.string, cm_sl.integer, server_color3.string, cm_tl.integer, server_color3.string, sl, server_color3.string, ds, server_color3.string, dr, server_color3.string, as, server_color3.string));
			}else{ // Boe!Man 3/18/11: Hide it when it's disabled.
				trap_SendServerCommand(-1, va("cp \"@%sMatch settings\n\n^7[^3Gametype^7]  %s%s %s\n^7[^3Scorelimit^7]  %s%i\n^7[^3Timelimit^7]  %s%i\n^7[^3Specs locked^7] %s%s\n^7[^3Disable events^7] %s%s\n^7[^3# of Rounds^7] %s%s\n\n%sRestart map to start the first round!\"", 
					server_color3.string, server_color3.string, level.mapname, g_gametype.string, server_color3.string, cm_sl.integer, server_color3.string, cm_tl.integer, server_color3.string, sl, server_color3.string, ds, server_color3.string, dr, server_color3.string));
			}
			level.compMsgCount = level.time + 3000;
			}
		else if(cm_enabled.integer == 3 && level.compMsgCount < level.time){ // Boe!Man 3/19/11: Fixing possible shortcome. High pingers might not always receive them properly this way..
			// Boe!Man 1/24/11: Swap the teams.
			if (cm_aswap.integer > 0 && level.swappedteams == qfalse){
				Boe_SwapTeams(NULL);
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
				trap_SendServerCommand(-1, va("cp \"@%sFirst round ended!\n\n^7[^3Red team^7] %sleads with %i - %i\n\n%sRestart map to start the second round!",
					server_color3.string, server_color3.string, cm_sr.integer, cm_sb.integer, server_color3.string));
			}
			else if (cm_sb.integer > cm_sr.integer){
				trap_SendServerCommand(-1, va("cp \"@%sFirst round ended!\n\n^7[^3Blue team^7] %sleads with %i - %i\n\n%sRestart map to start the second round!",
					server_color3.string, server_color3.string, cm_sb.integer, cm_sr.integer, server_color3.string));
			}
			else{ // Boe!Man 3/19/11: It could be a round draw with the timelimit.
				trap_SendServerCommand(-1, va("cp \"@%sFirst round ended!\n\n^7[^3Round draw^7] %swith %i - %i\n\n%sRestart map to start the second round!",
					server_color3.string, server_color3.string, cm_sb.integer, cm_sr.integer, server_color3.string));
			}
			level.compMsgCount = level.time + 3000;
		}
	}

	// Boe!Man 6/14/12: Check noroof actions.
	Boe_checkRoofGlobal();

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

void Boe_setTrackedCvar(int num, int value)
{
	cvarTable_t	*cv;
	cv = gameCvarTable;

	cv[num].trackChange = qfalse;
	trap_Cvar_Set(cv[num].cvarName, va("%i", value));
	G_UpdateCvars(); // Boe!Man 6/30/11: Update it manually this time.
	cv[num].trackChange = qtrue;

	return;
}
