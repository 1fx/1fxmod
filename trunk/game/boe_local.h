// Copyright (C) 2010 - Boe!Man.
//
// boe_local.h - Local definitions from boe_*.c files are stored here.

//==================================================================

// Boe!Man 3/30/10
#define INF_VERSION_STRING "0.40t"
#define INF_STRING "1fx. Mod"
#define INF_VERSION_STRING_COLORED "^71fx^1. ^3Mod"
#define INF_VERSION_DATE "7/24/10"


// Infiltration
// Developed by: Boe!Man
// Codename: BoeMODV2
// Modname: 1fx. Infiltration
// Project started: 3/30/10
// 
// --- VERSION LOG 
//
// http://1fx.ipbfree.com/index.php?showtopic=115
//===================================================================

//==============================================
// boe_admcmds.c
//==============================================

// Boe!Man 3/30/10
void	Boe_adm_f ( gentity_t *ent );
int		Boe_ClientNumFromArg (gentity_t *ent, int argNum, const char* usage, const char* type, qboolean aliveOnly, qboolean otheradmins, qboolean shortCmd);

extern	vmCvar_t	g_kick;

extern  vmCvar_t	g_adminfile;
extern	vmCvar_t	g_addbadmin;
extern	vmCvar_t	g_addadmin;
extern	vmCvar_t	g_addsadmin;

void	Boe_FileError (gentity_t * ent, const char *file);
void	Boe_id (int);
int		Boe_AddToList(const char *string, const char* file, const char* type, gentity_t* ent);
int		Boe_Remove_from_list( char *key, const char *file, const char* type, gentity_t *ent, qboolean unban, qboolean removeAdmin, qboolean scoreCheck);
void	Boe_Unban(gentity_t *adm, char *ip, qboolean subnet);

void	Boe_subnetBan(int argNum, gentity_t *adm, qboolean shortCmd);
void	Boe_Remove_Admin_f(int argNum, gentity_t *adm, qboolean shortCmd);

extern	vmCvar_t	g_ban;
extern	vmCvar_t	g_subnetban;
extern	vmCvar_t	g_subnetbanlist;
extern	vmCvar_t	g_banlist;
extern	vmCvar_t	g_removeadmin;

extern	vmCvar_t	server_color1;
extern	vmCvar_t	server_color2;
extern	vmCvar_t	server_color3;
extern	vmCvar_t	server_color4;
extern	vmCvar_t	server_color5;
extern	vmCvar_t	server_color6;

void	Boe_Add_bAdmin_f (int argNum, gentity_t *adm, qboolean shortCmd);		// Prototype warnings.
void	Boe_Add_Admin_f (int argNum, gentity_t *adm, qboolean shortCmd);		// Prototype warnings.
void	Boe_Add_sAdmin_f (int argNum, gentity_t *adm, qboolean shortCmd);		// Prototype warnings.
void	Boe_Ban_f (int argNum, gentity_t *adm, qboolean shortCmd);				// Prototype warnings.
void	Boe_Uppercut (int argNum, gentity_t *adm, qboolean shortCmd);
extern	vmCvar_t	g_uppercut;

void	Boe_Twist (int argNum, gentity_t *adm, qboolean shortCmd);
void	Boe_unTwist (int argNum, gentity_t *adm, qboolean shortCmd);
extern	vmCvar_t	g_twist;
void	Boe_Respawn (int argNum, gentity_t *adm, qboolean shortCmd);
extern	vmCvar_t	g_respawn;
extern	vmCvar_t	g_runover;
void	Boe_Runover (int argNum, gentity_t *adm, qboolean shortCmd);

void	G_ApplyKnockback( gentity_t *targ, vec3_t newDir, float knockback );	// Prototype warnings.

extern	vmCvar_t	g_maprestart;

void	Boe_Flash(int argNum, gentity_t *ent, qboolean shortCmd);
extern	vmCvar_t	g_flash;

void	Boe_unPlant (int argNum, gentity_t *adm, qboolean shortCmd);
void	Boe_Plant (int argNum, gentity_t *adm, qboolean shortCmd);
extern	vmCvar_t	g_plant;

void	Boe_pop (int argNum, gentity_t *adm, qboolean shortCmd);
void	Boe_Burn (int argNum, gentity_t *adm, qboolean shortCmd);
void	Boe_Mute (int argNum, gentity_t *adm, qboolean mute, qboolean shortCmd);
void	Boe_Strip (int argNum, gentity_t *adm, qboolean shortCmd);
void	Adm_ForceTeam(int argNum, gentity_t *adm, qboolean shortCmd);
extern	vmCvar_t	g_pop;
extern	vmCvar_t	g_burn;
extern	vmCvar_t	g_mute;
extern	vmCvar_t	g_strip;
extern	vmCvar_t	g_eventeams;

extern	vmCvar_t	g_333;

extern	vmCvar_t	g_forceteam;

void Cmd_Say_f( gentity_t *ent, int mode, qboolean arg0 );
gentity_t *NV_projectile (gentity_t *ent, vec3_t start, vec3_t dir, int weapon, int damage);
extern	vmCvar_t	g_maxIPConnections;

// Boe!Man 4/1/10
extern	vmCvar_t	server_redteamprefix;
extern	vmCvar_t	server_blueteamprefix;

// Boe!Man 4/5/10
// EMPTY

// Boe!Man 4/15/10
extern	vmCvar_t	g_nolower;
extern	vmCvar_t	g_nades;
extern	vmCvar_t	g_sl;
extern	vmCvar_t	g_tl;
extern	vmCvar_t	g_ri;
extern	vmCvar_t	g_respawninterval;
extern	vmCvar_t	g_damage;
extern	vmCvar_t	g_gr;
extern	vmCvar_t	g_clanvsall;
extern	vmCvar_t	g_swapteams;
extern	vmCvar_t	g_lock;
extern	vmCvar_t	g_clan;

//==============================================
// boe_utils.c
//==============================================

// Boe!Man 3/30/10
extern  vmCvar_t	server_motd1;
extern  vmCvar_t	server_motd2;
extern  vmCvar_t	server_motd3;
extern  vmCvar_t	server_motd4;
extern  vmCvar_t	server_motd5;
extern  vmCvar_t	server_motd6;

char	*Boe_StatColor(int stat);
int		Boe_FindClosestTeammate(gentity_t *ent, qboolean bot);
int		Boe_FindClosestEnemy(gentity_t *ent, qboolean bot);
void	Boe_ParseChatSounds (void);
void	Boe_Tokens(gentity_t *ent, char *chatText, int mode, qboolean CheckSounds);

extern	chatSounds_t chatSounds[MAX_BOE_CHATS];

void	Boe_ClientSound (gentity_t *ent, int soundIndex);
void	Boe_GlobalSound (int soundIndex);

void	Boe_Display_Sounds( gentity_t *ent );
void	Boe_Display_Sounds2( gentity_t *ent );
void	Boe_Display_ExtraSounds (gentity_t *ent);

extern	vmCvar_t	server_color1;
extern	vmCvar_t	server_color2;
extern	vmCvar_t	server_color3;
extern	vmCvar_t	server_color4;
extern	vmCvar_t	server_color5;
extern	vmCvar_t	server_color6;

extern  void		*DB195;
extern  void		*DB194;
extern  void		*DB212;
extern  void		*DB62;
extern  void		*DB193;
extern  void		*DB213;
extern  void		*DB217;
extern  void		*DB192;
extern  void		*DB216;
extern  void		*DB91;
extern  void		*DB209;
extern  void		*DB80;
extern  void		*DB64;
extern  void		*DB202;
extern  void		*DB66;
extern  void		*DB203;
extern  int			Preloaded;

void	Boe_GlobalSound (int soundIndex);

void	Boe_Motd (gentity_t *ent);

void	HENK_COUNTRY(gentity_t *ent);
qboolean CheckIP(gentity_t *ent);
void	HENK_CHECKFP(gentity_t *ent);

void	Boe_Players (gentity_t *ent);

void	Boe_Print_File (gentity_t *ent, char *file);

void	Boe_Stats( gentity_t *ent );

void	QDECL Boe_adminLog	(const char *text, ... );

// Boe!Man 4/2/10: Crash log.
void	QDECL Boe_crashLog	(const char *text);

// Henk 28/01/10 -> Add eventeams
void	EvenTeams (gentity_t *adm, qboolean aet);

void	Boe_SwapTeams(gentity_t *adm);

extern	vmCvar_t	server_badminprefix;
extern	vmCvar_t	server_adminprefix;
extern	vmCvar_t	server_sadminprefix;
extern	vmCvar_t	server_acprefix;
extern	vmCvar_t	server_caprefix;
extern	vmCvar_t	server_ccprefix;
extern	vmCvar_t	server_starprefix;

extern	vmCvar_t	Owner;
extern	vmCvar_t	Clan;
extern	vmCvar_t	ClanURL;

// Henk 19/04/10 -> Fix all prototypes warning(Yes im bored)
int Boe_NameListCheck (int num, const char *name, const char *file, gentity_t *ent, qboolean banCheck, qboolean admCheck, qboolean subnetCheck, qboolean scoreCheck);
void G_UpdateOutfitting ( int clientNum );
void G_DropGametypeItems ( gentity_t* self, int delayPickup );
int trap_RealTime( qtime_t *qtime );
qboolean BG_InitWeaponStats(void);
void Boe_About( gentity_t *ent );
void Boe_dev_f ( gentity_t *ent );
void Boe_Add_Clan_Member(int argNum, gentity_t *adm, qboolean shortCmd);
void Boe_Remove_Clan_Member(int argNum, gentity_t *adm, qboolean shortCmd);
void RPM_Clan_Vs_All(gentity_t *adm);
void Preload(void);

extern  vmCvar_t	g_disablelower;

char *GetReason(void);

// Boe!Man 5/9/10
char	*Boe_BarStat(int stat);
void	Boe_Kick(int argNum, gentity_t *ent, qboolean ShortCmd);
void	Boe_XMute(int argNum, gentity_t *ent, qboolean ShortCmd);
void	Boe_UnMute(int argNum, gentity_t *ent, qboolean ShortCmd);
void	Boe_MapRestart(int argNum, gentity_t *ent, qboolean ShortCmd);
void	Boe_NoLower(int argNum, gentity_t *ent, qboolean ShortCmd);
void	Boe_NoNades(int argNum, gentity_t *ent, qboolean ShortCmd);
void	Boe_TimeLimit(int argNum, gentity_t *ent, qboolean ShortCmd);
void	Boe_ScoreLimit(int argNum, gentity_t *ent, qboolean ShortCmd);
void	Boe_RespawnInterval(int argNum, gentity_t *ent, qboolean ShortCmd);
void	Boe_RealDamage(int argNum, gentity_t *ent, qboolean ShortCmd);
void	Boe_NormalDamage(int argNum, gentity_t *ent, qboolean ShortCmd);
void	Boe_GametypeRestart(int argNum, gentity_t *ent, qboolean ShortCmd);
void	Boe_Third(int argNum, gentity_t *ent, qboolean ShortCmd);
void	Boe_CompMode(int argNum, gentity_t *ent, qboolean ShortCmd);

void RPM_Awards(void);

// Boe!Man 6/2/10
extern	vmCvar_t	g_autoeventeams;
extern	vmCvar_t	server_msgInterval;
extern	vmCvar_t	server_enableServerMsgs;
extern	vmCvar_t	server_message1;
extern	vmCvar_t	server_message2;
extern	vmCvar_t	server_message3;
extern	vmCvar_t	server_message4;
extern	vmCvar_t	server_message5;
extern	vmCvar_t	server_msgDelay;
