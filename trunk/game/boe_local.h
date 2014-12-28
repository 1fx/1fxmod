// Copyright (C) 2010 - Boe!Man.
//
// boe_local.h - Local definitions from boe_*.c files are stored here.

//==================================================================

// Boe!Man 3/30/10
// Boe!Man 7/12/11: Mod version.
//#ifdef Q3_VM
//#define INF_VERSION_STRING "0.60"
//#else

/*
#ifdef WIN32
#define INF_VERSION_STRING "0.70t-dll"
#elif __linux__
#define INF_VERSION_STRING "0.70t-so"
#endif
*/
// Boe!Man 1/6/13: One string to rule them all.
#ifndef _DEBUG	// Boe!Man 10/26/14: Regular release (no _DEBUG defined).
#define INF_VERSION_STRING "0.74"
#define STABLE_VERSION "Developed by ^GBoe!Man ^7& ^6Henkie\nwww.1fxmod.org\n\n"
#else
#define MAJOR_VERSION_STRING "0.75"
#ifdef _NIGHTLY // Direct build from the master. Only used when debugging/developing.
#define INF_VERSION_STRING "0.75t-^5master"
#else
#define INF_VERSION_STRING "0.75t.12-^5pre" // Don't forget to increment this number when test releasing.
#endif // _NIGHTLY
#define TEST_VERSION "Developed by ^GBoe!Man ^7& ^6Henkie\n^1Test version of " MAJOR_VERSION_STRING "\n\n"
#endif // _DEBUG
//#endif

// Boe!Man 7/12/11: Mod name.
#define INF_STRING "1fx. Mod"
#define INF_VERSION_STRING_COLORED "^71fx^1. ^3Mod"

// Boe!Man 7/12/11: Mod date.
// Boe!Man 7/29/12: Month first causes confusion. Just hold on to the ISO standard for now (__DATE__).
/*
#ifdef Q3_VM
#define INF_VERSION_DATE "6/14/12"
#else
#define YEAR ((((__DATE__ [9] - '0')) * 10 + (__DATE__ [10] - '0')))

#define MONTH (__DATE__ [1] == 'a' && __DATE__[0] == 'J' ? 0 \
               : __DATE__ [2] == 'b' ? 1 \
               : __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 2 : 3) \
               : __DATE__ [2] == 'y' ? 4 \
               : __DATE__ [2] == 'n' ? 5 \
               : __DATE__ [2] == 'l' ? 6 \
               : __DATE__ [2] == 'g' ? 7 \
               : __DATE__ [2] == 'p' ? 8 \
               : __DATE__ [2] == 't' ? 9 \
               : __DATE__ [2] == 'v' ? 10 : 11)

#define DAY ((__DATE__ [4] == ' ' ? 0 : __DATE__ [4] - '0') * 10 + (__DATE__ [5] - '0'))
// Date will be generated automatically
#define INF_VERSION_DATE ""
#endif
*/
#define INF_VERSION_DATE __DATE__

// 1fx. Mod
// Developed by: Boe!Man & Henkie
// Codename: BoeMODV2
// Modname: 1fx. Mod (Originally: 1fx. Infiltration)
// Project started: 3/30/10
//
// --- VERSION LOG BELOW
//
// Version		Status of Release				Rev			Date
// ----------------------------------------------------------------
// 0.01			Alpha							3			3/30/10
// 0.02			Alpha							4			3/31/10
// 0.05			Beta							24			4/4/10
// 0.06			Beta							29			4/6/10
// 0.10			Beta							33			4/7/10
// 0.11a		1st Beta						34			4/8/10
// 0.11b		2nd Beta						40			4/8/10
// 0.15a		Release Candidate				51			4/12/10
// 0.15b		Release							59			4/14/10
// 0.16			Beta							66			4/15/10
// 0.17			Beta							70			4/16/10
// 0.18			Beta							77			4/17/10
// 0.19			Beta							105			4/18/10
// 0.20			Release							115			4/20/10
// 0.21			Release							122			4/22/10
// 0.22			Release							128			4/23/10
// 0.25a		Release							142			5/3/10
// 0.25b		Release							147			5/3/10
// 0.30t		Beta							170			5/7/10
// 0.30			Release							193			6/2/10
// 0.31			Release							194			6/3/10
// 0.40t		Beta							195			6/11/10
// 0.40t		Release Candidate				198			7/24/10
// 0.40			Release							199			8/2/10
// 0.41t		Beta							200			8/22/10
// 0.41			Release							202			8/25/10
// 0.42t		Beta							208			9/1/10
// 0.42			Release							213			9/3/10
// 0.50t		Beta							223			9/9/10
// 0.50			Release							229			9/17/10
// 0.51t		Alpha							230			9/18/10
// 0.51t		Beta							231			9/19/10
// 0.51t		1st Release Candidate			238			9/26/10
// 0.51t		2nd Release Candidate			240			9/30/10
// 0.51			Release							244			10/7/10
// 0.52t		Beta							246			10/16/10
// 0.52			Release							250			10/25/10
// 0.53			Release							253			11/2/10
// 0.54t		Beta							261			11/21/10
// 0.54t		Release Candidate				265			12/13/10
// 0.54			Release							283			12/19/10
// 0.55t		Beta							289			12/23/10
// 0.55			Release							297			12/31/10
// 0.56t		Alpha							299			1/1/11
// 0.56t		1st Beta						304			1/8/11
// 0.56t		2nd Beta						312			1/14/11
// 0.56t		1st Release Candidate			332			1/19/11		// Sent out to BETA group on 1/19/11
// 0.56t		2nd Release Candidate			362			1/27/11		
// 0.56t		3rd Release Candidate			372			1/28/11		// Sent out to BETA group on 1/28/11
// 0.56t		4th	Release	Candidate			388			2/6/11		// Sent out to BETA group on 2/6/11
// 0.56			1st Release						404			2/8/11		// Sent out to BETA group on 2/8/11
// 0.56			2nd Release (hotfix)			404(/408)	2/12/11		// Sent out to BETA group on 2/12/11
// 0.57t		Alpha							424			2/26/11
// 0.57t		Beta							428			3/2/11		// Sent out to BETA group on 3/2/11
// 0.57t		1st Release Candidate			511			3/19/11		// Sent out to BETA group on 3/19/11
// 0.57t		2nd Release Candidate			521			3/21/11		// Sent out to BETA group on 3/21/11
// 0.57			1st Release						525			3/27/11		// Sent out to BETA group on 3/27/11
// 0.58t		Alpha							545			4/2/11
// 0.58t		Beta							568			4/8/11		// Sent out to BETA group on 4/8/11
// 0.57			2nd Release	(hotfix)			525(/578)	4/23/11		// Sent out to BETA group on 4/23/11
// 0.58t		Beta (hotfix)					568(/578)	4/23/11		// Sent out to BETA group on 4/23/11
// 0.58t		Release Candidate				601			5/8/11		// Sent out to BETA group on 5/9/11
// 0.58			Release							615			5/27/11		// Sent out to BETA group on 5/27/11
// 0.59t		1st Alpha						622			6/2/11
// 0.59t		2nd Alpha						687			6/30/11
// 0.59t		1st Beta						693			7/4/11		// Sent out to BETA group on 7/4/11
// 0.59t		2nd Beta						710			7/15/11		// Sent out to BETA group on 7/15/11
// 0.59t		Release Candidate				749			9/20/11		// Sent out to BETA group on 9/20/11
// 0.59			Release							759			1/4/12		// Sent out to BETA group on 1/4/12, to public on 1/5/12
// 0.60t		1st Alpha						771			4/21/12
// 0.60t		1st Beta						794			6/14/12		// Sent out to BETA group on 6/14/12
// 0.60t		2nd Beta						824			7/29/12		// Sent out to BETA group on 7/29/12
// 0.60t		3rd Beta						851			10/4/12		// Sent out to BETA group on 10/4/12
// 0.60t		1st Release Candidate			877			11/11/12	// Sent out to BETA group on 11/11/12

// Branch 0.60 (LTS) - 0.70 (testing, BETA).
//
// -- LTS
// 0.60			Release							885			11/23/12	// Sent out to public on 11/23/12
// 0.61			Release							1056		7/1/13		// Sent out to public on 7/1/13
//
// -- Testing and BETA
// 0.70t		1st Alpha						895			11/30/12	// Sent out to BETA group on 11/30/12
// 0.70t		2nd Alpha						933			1/6/13		// Sent out to BETA group on 1/6/13
// 0.70t		3rd Alpha						937			1/9/13		// Sent out to BETA group on 1/9/13
// 0.70t		4th Alpha						951			1/26/13		// Sent out to BETA group on 1/26/13
// 0.70t		5th Alpha						957			1/30/13		// Sent out to BETA group on 1/30/13
// 0.70t		1st Beta						981			2/16/13		// Sent out to BETA group on 2/16/13
// 0.70t		2nd Beta						987			2/17/13		// Sent out to BETA group on 2/17/13
// 0.70t		3rd Beta						1000		3/11/13		// Sent out to BETA group on 3/11/13
// 0.70t		1st Release Candidate			1017		5/27/13		// Sent out to BETA group on 5/27/13
// 0.70t		2nd Release Candidate			1022		5/30/13		// Sent out to BETA group on 5/30/13
// 0.70t		3rd Release Candidate			1050		6/30/13		// Sent out to BETA group on 6/30/13
// 0.70			Release							1062		7/5/13		// Sent out to public on 7/5/13
// 0.71t		1st Beta						1076		7/12/13		// Sent out to BETA group on 7/12/13
// 0.71t		2nd Beta						1084		7/15/13		// Sent out to BETA group on 7/15/13
// 0.71t		3rd Beta						1099		7/25/13		// Sent out to BETA group on 7/25/13
// 0.71t		4th Beta						1109		9/7/13		// Sent out to BETA group on 9/7/13
// 0.71t		1st Release Candidate			1119		9/29/13		// Sent out to BETA group on 9/29/13
// 0.71			Release							1132		10/27/13	// Sent out to public on 10/27/13
// 0.72t		1st Beta						1142		11/26/13	// Sent out to BETA group on 11/26/13
// 0.72t		2nd Beta						1145		12/4/13		// Sent out to BETA group on 12/4/13
// 0.72t		3rd Beta						1150		12/18/13	// Sent out to BETA group on 12/18/13
// 0.72			Release							1165		1/22/14		// Sent out to public on 1/22/14
// 0.73t		1st Beta						1186		2/11/14		// Sent out to BETA group on 2/11/14
// 0.73t		1st Release Candidate			1203		3/25/14		// Sent out to BETA group on 3/25/14
// 0.73			Release										5/16/14		// Sent out to public on 5/16/14
// 0.74			Release										9/14/14		// Sent out to public on 9/14/14


//===================================================================

/*
***************** .so Considerations *****************
// By Boe!Man - 7/13/11 - 2:41 AM

Note that even though the *.so is just as stable as the *.dll version as of this moment,
it should still be primarily used for DEBUGGING ONLY.
Main reasons are that the shared objects are compiled against an older version of glibc,
since the main sof2ded executable is statically linked instead of dynamically.
This basically comes down to using older dynamic loaders to make your *.so load or even
passing the Common Init as a whole. This also means the shared object MUST be compiled
against an older version of gcc and an older version of ld, to use it in conjunction
with your dynamic loader. In order to fully get rid of any system header errors, be sure
to compile with the option "-Wall", when building your object (don't link with it).

If you leave the system headers as-is, these are the dependencies of your object:

        libm.so.6 => /lib/libm.so.6 (0x406e9000)
        libc.so.6 => /lib/libc.so.6 (0x4070a000)
        /lib/ld-linux.so.2 => /lib/ld-linux.so.2 (0x80000000)

.. where ld-linux is just a symlink to the actual ld object.

Tested and confirmed to fully work on systems with glibc 1.x or <= 2.4, and gcc 2.9x.
******************************************************
*/

/*
*************** New .so Considerations ***************
// By Boe!Man - 1/26/13 - 10:43 AM

The above method will undoubtedly work in terms of compiling the Mod, but it's not
entirely fool-proof and thus I went on looking for a new way of doing this.
A generally better and apparently more stable way of compiling the *.so is by using
the old glibc compiler, whereas the linker (ld in this case) links the object files
into the dynamic object *and* some static libraries. This basically comes down to
having a shared object with zero dependencies, which is still a shared library that
can be opened with dlopen.

Having the Mod compiled like this, we can actually change the dynamic loader on the
host platform again, we could use fc4libs, 1fxlib 0.1 or even no dynamic loader at
all if the host doesn't segfault!

But it is generally recommended to keep the host and Mod libraries at a minimum
distance of each other, therefore, if the host requires a LD_LIBRARY_PATH hack, 
I chose to include the old 1fxlib (Woody) dynamic library loader in 1fxlib 0.31.
This platform (1fxlib) has proven itself to be quite stable cross platform,
which is the end goal after all.

The repository now contains an updated game.sh, this file used to compile a QVM
on Linux platforms, it can now be used to compile the Mod as sof2mp_gamei386.so
on a Linux platform.
******************************************************
*/

//==============================================
// boe_admcmds.c
//==============================================

// Boe!Man 3/30/10
void	Boe_adm_f ( gentity_t *ent );
int		Boe_ClientNumFromArg (gentity_t *ent, int argNum, const char* usage, const char* type, qboolean aliveOnly, qboolean otheradmins, qboolean shortCmd);


//extern	admCmd_t AdminCommands[124];

//extern  vmCvar_t	g_adminfile;
extern	vmCvar_t	g_adminlog;
extern	vmCvar_t	g_loginlog;


void	Boe_FileError (gentity_t * ent, const char *file);
void	Boe_id (int);
qboolean Boe_removeAdminFromDb(gentity_t *adm, const char *value, qboolean passAdmin, qboolean lineNumber, qboolean silent);

extern	vmCvar_t	g_subnetbanlist;
extern	vmCvar_t	g_broadcast;

extern	vmCvar_t	server_colors;

void	G_ApplyKnockback( gentity_t *targ, vec3_t newDir, float knockback );	// Prototype warnings.
#ifdef _awesomeToAbuse
//void	Henk_Box(int argNum, gentity_t *adm, qboolean shortCmd);
void	Boe_freakOut(gentity_t *adm);
void	Boe_forceSay(gentity_t *adm);

extern vmCvar_t boe_fragWars;
#endif

extern	vmCvar_t	g_eventeams;

//extern	vmCvar_t	g_333;



void Cmd_Say_f( gentity_t *ent, int mode, qboolean arg0 );
gentity_t *NV_projectile (gentity_t *ent, vec3_t start, vec3_t dir, int weapon, int damage);
extern	vmCvar_t	g_maxIPConnections;
extern  vmCvar_t	g_noHighFps;

// Boe!Man 4/1/10
extern	vmCvar_t	server_redteamprefix;
extern	vmCvar_t	server_blueteamprefix;

// Boe!Man 4/15/10
extern	vmCvar_t	g_respawninterval;

// Boe!Man 3/1/11
extern	vmCvar_t	g_useNoLower;
extern	vmCvar_t	g_useNoRoof;
extern	vmCvar_t	g_useNoMiddle;
extern	vmCvar_t	g_useNoWhole;


// Boe!Man 3/6/11
extern	vmCvar_t	g_customCommandsFile;
extern	vmCvar_t	g_tipsFile;
//extern	vmCvar_t	g_banfile;
extern  vmCvar_t	hideSeek_Nades;
extern  vmCvar_t	hideSeek_Weapons;

// Boe!Man 3/8/11
extern	vmCvar_t	g_enableAdminLog;

extern	vmCvar_t	g_adminlist;

// Boe!Man 4/20/11
//extern	vmCvar_t	g_crossTheBridge;

// Boe!Man 5/28/11
extern	vmCvar_t	g_customWeaponFile;

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

extern  int			Preloaded;

void	Boe_GlobalSound (int soundIndex);

void	Boe_Motd (gentity_t *ent);

void	HENK_COUNTRY(gentity_t *ent);
qboolean CheckIP(gentity_t *ent);

void	Boe_Players (gentity_t *ent);

void	Boe_Print_File (gentity_t *ent, char *file, qboolean clonecheckstats, int idnum);

void	Boe_Stats( gentity_t *ent );

void	QDECL Boe_adminLog( const char *command, const char *by, const char *to, ... );

// Boe!Man 10/8/13: Debug write function.
#ifdef _DEBUG
void	writeDebug(int section, char *msg);
#endif

// Boe!Man 1/2/14: This function logs soft-crashes (triggered by Com_Error).
void	logCrash(void);

// Henk 28/01/10 -> Add eventeams
void	EvenTeams (gentity_t *adm, qboolean aet);

extern	vmCvar_t	server_badminprefix;
extern	vmCvar_t	server_adminprefix;
extern	vmCvar_t	server_sadminprefix;
extern	vmCvar_t	server_acprefix;
extern	vmCvar_t	server_scprefix;
extern	vmCvar_t	server_caprefix;
extern	vmCvar_t	server_ccprefix;
extern  vmCvar_t	server_ctprefix;
extern	vmCvar_t	server_starprefix;

extern	vmCvar_t	Owner;
extern	vmCvar_t	Clan;
extern	vmCvar_t	ClanURL;
extern	vmCvar_t	HostedBy;

// Henk 19/04/10 -> Fix all prototypes warning(Yes im bored)
//int Boe_NameListCheck (int num, const char *name, const char *file, gentity_t *ent, qboolean banCheck, qboolean admCheck, qboolean subnetCheck, qboolean scoreCheck, qboolean cloneCheck);
void G_UpdateOutfitting ( int clientNum );
void G_DropGametypeItems ( gentity_t* self, int delayPickup );
int trap_RealTime( qtime_t *qtime );
qboolean BG_InitWeaponStats(qboolean init);
void Boe_About( gentity_t *ent );
qboolean Boe_removeClanMemberFromDb(gentity_t *adm, const char *value, qboolean lineNumber, qboolean silent);

#ifdef _DEBUG
qboolean	Boe_dev_f		(gentity_t *ent);
void		RPM_CalculateTMI(gentity_t *ent);
#endif

char *GetReason(void);

// Boe!Man 5/9/10
char	*Boe_BarStat(int stat);
qboolean henk_isdigit(char c);
qboolean henk_ischar(char c);
void RPM_Awards(void);
void G_AdjustClientBBox(gentity_t *other);
void G_AdjustClientBBoxs(void);
void G_UndoAdjustedClientBBox(gentity_t *other);
void G_UndoAdjustedClientBBoxs(void);
void G_SetClientPreLeaningBBox(gentity_t *ent);
void G_SetClientLeaningBBox(gentity_t *ent);

#define BBOX_LEAN_OFFSET	0.65f
#define LB_MAXZ				-3
#define LB_MINZ				54
#define DUCKED_LB_MINZ		25
#define PLB_MAXZ			-39

// Boe!Man 6/2/10
extern	vmCvar_t	g_autoEvenTeams;
extern	vmCvar_t	server_msgInterval;
extern	vmCvar_t	server_enableServerMsgs;
extern  vmCvar_t	server_enableTips;
extern	vmCvar_t	server_message1;
extern	vmCvar_t	server_message2;
extern	vmCvar_t	server_message3;
extern	vmCvar_t	server_message4;
extern	vmCvar_t	server_message5;
extern	vmCvar_t	server_msgDelay;

// Boe!Man 10/16/10
extern	vmCvar_t	g_adminSpec;

// Boe!Man 11/16/10: Default scrim settings.
extern	vmCvar_t	g_matchLockSpec;
extern	vmCvar_t	g_matchSwapTeams;
extern	vmCvar_t	g_matchTimeLimit;
extern	vmCvar_t	g_matchScoreLimit;
extern	vmCvar_t	g_matchDisableEvents;
extern	vmCvar_t	g_matchRounds;

extern vmCvar_t	cm_enabled;
extern vmCvar_t	cm_sl;
extern vmCvar_t	cm_tl;
extern vmCvar_t	cm_slock;
extern vmCvar_t	cm_aswap;
extern vmCvar_t	cm_devents;
extern vmCvar_t	cm_dr;
extern vmCvar_t	cm_oldsl;
extern vmCvar_t	cm_oldtl;
extern vmCvar_t	cm_sr;
extern vmCvar_t	cm_sb;

// Boe!Man 11/18/10
void Boe_calcMatchScores (void);

// Boe!Man 12/13/10
extern vmCvar_t	g_aliasCheck;
extern vmCvar_t	g_aliasCount;

// Henkie 01/02/11
extern vmCvar_t g_alternateMap;

// Boe!Man 1/8/10
void Boe_serverMsg (void);
void Henk_Tip(void);

// 1/14/10: Custom commands by Henk
void	SpawnCage(vec3_t org, gentity_t *ent, qboolean autoremove, qboolean big);
// Henk 23/01/10 -> Add BoxNade and StripHiders
void	SpawnBox(vec3_t org);
int		SpawnBoxEx(vec3_t org, vec3_t ang);
void	StripHiders(void);
void	SetupOutfitting(void);
// Henk 19/02/10 -> Add Flares
void	Effect(vec3_t org, char *name, qboolean rpg);

// Henk 20/02/10 -> Add UpdateScores()
void	UpdateScores(void);
void	ShowScores(void);
int		Henk_GetScore(qboolean seekers);

char	*vtocs( const vec3_t v );

// Henk 14/01/11 -> Add EvenTeams_HS and all other shit
void		EvenTeams_HS			(gentity_t *adm, qboolean aet);
void		EvenTeams_HZ			(gentity_t *adm, qboolean aet);
void		HZ_Claymore				(gentity_t *ent);
void		HZ_ClaymoreShoot		(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod, int hitLocation, vec3_t hitDir);
void		HZ_claymoreExplode		(gentity_t *ent);
gentity_t*	findLastEnteredPlayer	(int highTeam, qboolean scoresAllowed);

void Use_BinaryMover( gentity_t *ent, gentity_t *other, gentity_t *activator );
gentity_t* G_RealSpawnGametypeItem1 ( gitem_t* item, vec3_t origin, vec3_t angles, qboolean dropped );
void G_RespawnClients ( qboolean force, team_t team, qboolean fullRestart );
void G_UpdateAvailableWeapons(void);
//void Henk_RemoveLineFromFile(gentity_t *ent, int line, char *file, qboolean subnet, qboolean ban, qboolean banremove, char *banremovestring);
void Svcmd_EntityList_f(void);
//void Svcmd_CancelVote_f(void);
void Svcmd_AddIP_f(void);
//void Svcmd_RemoveIP_f(void);
void Svcmd_ExtendTime_f(void);
qboolean IsClientMuted(gentity_t *ent, qboolean message);
qboolean AddMutedClient(gentity_t *ent, int time);
qboolean RemoveMutedClient(gentity_t *ent);
void RPM_WeaponMod (void);
void RPM_Refresh(gentity_t *ent);
void RPM_ReadyUp(gentity_t *ent);
int FormatDamage(int damage);

extern vmCvar_t g_enableCustomCommands;

char	*ConcatArgs1( int start );
void CheckEnts(gentity_t *ent);

char *ChooseTeam(void);
void DoTeleport(gentity_t *ent, vec3_t origin);
int TiedPlayers(void);
void InitCagefight(void);
void Henk_CloseSound ( vec3_t origin, int soundIndex);
//qboolean CheckPasswordList(gentity_t *ent, char *pass);
void PrintCustom(int numb);

// Boe!Man 3/16/11
extern vmCvar_t	server_rconprefix;
extern vmCvar_t	server_specteamprefix;

void	trap_LinkEntity1( gentity_t *ent );
void	trap_UnlinkEntity1( gentity_t *ent );
int GetRpgWinner(void);
int GetM4Winner(int rpgwinner);
//void AddToPasswordList(gentity_t *ent, int lvl);
qboolean IsValidCommand(char *cmd, char *string);
void Boe_compTimeLimitCheck (void);
qboolean SetNades(char *status);

// Boe!Man 5/15/11: Misc. RPM function.
const char	*G_PlaceString( int rank );
void Henk_Ignore (gentity_t *ent);

// Boe!Man 6/10/11: Map events.
void Boe_mapEvents (void);

void CloneBody( gentity_t *ent , int number);
void DropRandom( gentity_t *ent, int zombies);

// Boe!Man 6/30/11: CVAR function.
void Boe_setTrackedCvar(int num, int value);

// Boe!Man 8/18/11: CVAR for Drop messages in H&S/INF/CTF.
extern	vmCvar_t	g_dropLocationMessage;

// Boe!Man 8/25/11: CVAR for logging special Chats (Hey Admin, Admin Chat, Clan Chat, etc.).
extern	vmCvar_t	g_logSpecialChat;
extern	vmCvar_t	g_specialChatSound;
void Boe_displayTokens ( gentity_t *ent );

// Boe!Man 5/20/12: CVAR for IP in /stats.
extern	vmCvar_t	g_publicIPs;

// Boe!Man 6/3/12: Roof check.
void	Boe_checkRoof(gentity_t *ent);

// Boe!Man 7/4/12: Very simple function that will help us in e.g. splitting specific packets (in size).
int Boe_firstDigitOfInt(int i);

// Boe!Man 7/27/12: Function that replaces arguments in the actual CustomCommand action.
char *Boe_parseCustomCommandArgs(char *in);

// Boe!Man 7/29/12: CVAR that handles admins/clan members on subnet rather than Full IP -> This effectively disables the admin login system.
extern	vmCvar_t	g_preferSubnets;

// Henk 10/30/12: CVAR that enables death messages being handled by client
extern  vmCvar_t	g_clientDeathMessages;

// Boe!Man 9/2/12: CVAR that enables/disables the extended round stats in H&S.
extern	vmCvar_t	hideSeek_ExtendedRoundStats;
extern	vmCvar_t	g_ctfClassic;					// Boe!Man 2/1/13: CTF classic mode CVAR.

// Boe!Man 9/11/12: Preload effects in H&S/H&Z in this function.
void Preload_Effects(void);

// Boe!Man 11/11/12: Ripped old /howto command from 1fx. Hide&Seek for CTB.
void Boe_Howto ( gentity_t *ent );

unsigned int henk_atoi( const char *string ); // unsigned int version of atoi

int GetArgument(int argNum);

// =================================
// SQLite3 related defintions.

#ifdef __linux__
extern unsigned char	memsys5[41943040]; // Boe!Man 1/29/13: Buffer of 40 MB, available for SQLite memory management (Linux).
#endif

void		Boe_userdataIntegrity		(void);
void		Boe_SQLStats				(void); // Boe!Man 1/30/13: Can be called via RCON with sql_stats.
void		Boe_convertNonSQLChars		(char *input);

qboolean	Boe_checkAlias				(char *ip, char *name2);
void		Boe_addAlias				(char *ip, char *name2);
void		Boe_printAliases			(gentity_t *ent, char *ip, char *name2);

int			Boe_checkAdmin				(char *ip, char *name2);
int 		Boe_checkPassAdmin			(char *name2, char *pass);
qboolean	Boe_checkPassAdmin2			(char *name2);
void		Boe_addPasswordToDatabase	(char *ip, char *name2, char *pass);
qboolean	Boe_checkClanMember			(char *ip, char *name2);

void		Boe_unloadUserdataDbs		(void);
void		Boe_backupInMemoryDbs		(char *filename, sqlite3 *db);
void		Boe_SQLTableClear			(void);

// Boe!Man 5/27/13: In-memory databases.
extern		sqlite3 	*aliasesDb;
extern		sqlite3 	*bansDb;
extern		sqlite3 	*usersDb;
extern		qboolean 	dbsLoaded;
extern		char		tempName[16];

void		LoadCountries			(void);
void		UnloadCountries			(void);

// Boe!Man 1/2/13: --- SQLite3 Related CVARs ---
extern 		vmCvar_t		sql_aliasFlushCount;
extern		vmCvar_t		sql_timeBench;
extern		vmCvar_t		sql_automaticBackup;
