/*
===========================================================================
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
Copyright (C) 2010 - 2020, Ane-Jouke Schat (Boe!Man)
Copyright (C) 2010 - 2020, Jordi Epema (Henkie)

This file is part of the 1fx. Mod source code.

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/
// cg_main.c -- initialization and primary entry point for cgame

#include "cg_local.h"
#include "../ui/ui_shared.h"

// display context for new ui stuff
displayContextDef_t cgDC;

#if !defined(CL_LIGHT_H_INC)
	#include "cg_lights.h"
#endif


int forceModelModificationCount = -1;

void CG_Init		( int serverMessageNum, int serverCommandSequence, int clientNum );
void CG_InitItems	( void );
void CG_Shutdown	( void );

void CG_CalcEntityLerpPositions( centity_t *cent );

static int	C_PointContents			(void);
static void C_GetLerpOrigin			(void);
static void C_GetLerpAngles			(void);
static void C_GetModelScale			(void);
static void C_Trace					(void);
static void C_CameraShake			(void);

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
		case CG_INIT:
			CG_Init( arg0, arg1, arg2 );
			return 0;

		case CG_SHUTDOWN:
			CG_Shutdown();
			return 0;

		case CG_CONSOLE_COMMAND:
			return CG_ConsoleCommand();

		case CG_DRAW_ACTIVE_FRAME:
			CG_DrawActiveFrame( arg0, arg1, arg2 );
			return 0;

		case CG_CROSSHAIR_PLAYER:
			return CG_CrosshairPlayer();
		
		case CG_LAST_ATTACKER:
			return CG_LastAttacker();
		
		case CG_KEY_EVENT:
			CG_KeyEvent(arg0, arg1);
			return 0;

		case CG_MOUSE_EVENT:
			cgDC.cursorx = cgs.cursorX;
			cgDC.cursory = cgs.cursorY;
			
			CG_MouseEvent(arg0, arg1);
			return 0;

		case CG_EVENT_HANDLING:
			CG_EventHandling(arg0);
			return 0;

		case CG_POINT_CONTENTS:
			return C_PointContents();

		case CG_GET_LERP_ORIGIN:
			C_GetLerpOrigin();
			return 0;

		case CG_GET_LERP_ANGLES:
			C_GetLerpAngles();
			return 0;

		case CG_GET_MODEL_SCALE:
			C_GetModelScale();
			return 0;

		case CG_GET_GHOUL2:
			return (int)CG_GetEntity (arg0)->ghoul2;

		case CG_GET_MODEL_LIST:
			return (int)cgs.gameModels;

		case CG_CALC_LERP_POSITIONS:
			CG_CalcEntityLerpPositions( CG_GetEntity ( arg0 ) );
			return 0;

		case CG_TRACE:
			C_Trace();
			return 0;

		case CG_GET_ORIGIN:
			VectorCopy( CG_GetEntity (arg0)->currentState.pos.trBase, (float *)arg1);
			return 0;

		case CG_GET_ANGLES:
			VectorCopy( CG_GetEntity (arg0)->currentState.apos.trBase, (float *)arg1);
			return 0;

		case CG_GET_ORIGIN_TRAJECTORY:
			return (int)&CG_GetEntity (arg0)->nextState.pos;

		case CG_GET_ANGLE_TRAJECTORY:
			return (int)&CG_GetEntity (arg0)->nextState.apos;

		case CG_FX_CAMERASHAKE:
			C_CameraShake();
			return 0;

		case CG_MISC_ENT:
			CG_MiscEnt();
			return 0;

		case CG_MAP_CHANGE:
			// this trap map be called more than once for a given map change, as the
			// server is going to attempt to send out multiple broadcasts in hopes that
			// the client will receive one of them
			cg.mMapChange = qtrue;
			trap_S_ClearLoopingSounds ( qtrue );
			trap_S_StopAllSounds ( );
			trap_UI_CloseAll ( );
			return 0;

		default:
			Com_Error( ERR_FATAL, "vmMain: unknown command %i", command );
			break;
	}

	return -1;
}

static int C_PointContents(void)
{
	TCGPointContents	*data = (TCGPointContents *)cg.sharedBuffer;

	return CG_PointContents( data->mPoint, data->mPassEntityNum );
}

static void C_GetLerpOrigin(void)
{
	TCGVectorData		*data = (TCGVectorData *)cg.sharedBuffer;

	VectorCopy( CG_GetEntity (data->mEntityNum)->lerpOrigin, data->mPoint);
}

static void C_GetLerpAngles(void)
{
	TCGVectorData		*data = (TCGVectorData *)cg.sharedBuffer;

	VectorCopy(CG_GetEntity(data->mEntityNum)->lerpAngles, data->mPoint);
}

static void C_GetModelScale(void)
{
	TCGVectorData		*data = (TCGVectorData *)cg.sharedBuffer;

	VectorCopy(CG_GetEntity(data->mEntityNum)->modelScale, data->mPoint);
}

static void C_Trace(void)
{
	TCGTrace	*td = (TCGTrace *)cg.sharedBuffer;

	CG_Trace(&td->mResult, td->mStart, td->mMins, td->mMaxs, td->mEnd, td->mSkipNumber, td->mMask);
}

static void C_CameraShake(void)
{
	TCGCameraShake	*data = (TCGCameraShake *)cg.sharedBuffer;

	CG_CameraShake ( data->mOrigin, data->mIntensity, data->mRadius, data->mTime );
}

cg_t				cg;
cgs_t				cgs;
centity_t			cg_entities[MAX_GENTITIES];
centity_t			*cg_permanents[MAX_GENTITIES];
int					cg_numpermanents = 0;
weaponInfo_t		cg_weapons[MAX_WEAPONS];
itemInfo_t			cg_items[MAX_ITEMS];

vmCvar_t	con_notifyTime;
vmCvar_t	cg_centertime;
vmCvar_t	cg_centerY;
vmCvar_t	cg_runpitch;
vmCvar_t	cg_runroll;
vmCvar_t	cg_bobup;
vmCvar_t	cg_bobpitch;
vmCvar_t	cg_bobroll;
vmCvar_t	cg_shadows;
vmCvar_t	cg_drawTimer;
vmCvar_t	cg_drawFPS;
vmCvar_t	cg_drawSnapshot;
vmCvar_t	cg_drawCrosshair;
vmCvar_t	cg_drawCrosshairNames;
vmCvar_t	cg_drawRadar;
vmCvar_t	cg_drawTeamScores;
vmCvar_t	cg_crosshairSize;
vmCvar_t	cg_crosshairX;
vmCvar_t	cg_crosshairY;
vmCvar_t	cg_crosshairRGBA;
vmCvar_t	cg_crosshairFriendRGBA;
vmCvar_t	cg_draw2D;
vmCvar_t	cg_debugEvents;
vmCvar_t	cg_errorDecay;
vmCvar_t	cg_nopredict;
vmCvar_t	cg_showmiss;
vmCvar_t	cg_footsteps;
vmCvar_t	cg_drawGun;
vmCvar_t	cg_gun_x;
vmCvar_t	cg_gun_y;
vmCvar_t	cg_gun_z;
vmCvar_t	cg_autoswitch;
vmCvar_t	cg_ignore;
vmCvar_t	cg_simpleItems;
vmCvar_t	cg_fov;
vmCvar_t	cg_goreDetail;
vmCvar_t	cg_thirdPerson;
vmCvar_t	cg_thirdPersonRange;
vmCvar_t	cg_thirdPersonYaw;
vmCvar_t	cg_thirdPersonPitch;
vmCvar_t	cg_thirdPersonHorzOffset;
vmCvar_t	cg_stereoSeparation;
vmCvar_t	cg_lagometer;
vmCvar_t	cg_synchronousClients;
vmCvar_t 	cg_chatTime;
vmCvar_t 	cg_chatHeight;
vmCvar_t 	cg_stats;
vmCvar_t 	cg_buildScript;
vmCvar_t 	cg_forceModel;
vmCvar_t	cg_paused;
vmCvar_t	cg_predictItems;
vmCvar_t	cg_antiLag;
vmCvar_t	cg_impactPrediction;
vmCvar_t	cg_autoReload;
vmCvar_t	cg_deferPlayers;
vmCvar_t	cg_drawFriend;
vmCvar_t	cg_teamChatsOnly;
vmCvar_t	cg_noVoiceText;
vmCvar_t	cg_hudFiles;
vmCvar_t 	cg_smoothClients;
vmCvar_t	pmove_fixed;
vmCvar_t	pmove_msec;
vmCvar_t	cg_pmove_msec;
vmCvar_t	cg_cameraMode;
vmCvar_t	cg_cameraOrbit;
vmCvar_t	cg_cameraOrbitDelay;
vmCvar_t	cg_timescale;
vmCvar_t	cg_noProjectileTrail;
vmCvar_t	cg_currentSelectedPlayer;
vmCvar_t	cg_currentSelectedPlayerName;
vmCvar_t	cg_DebugGore;
vmCvar_t	cg_lockBlood;
vmCvar_t	cg_lockSever;
vmCvar_t	cg_lockDeaths;
vmCvar_t	cg_marks;
vmCvar_t	cg_shellEjection;
vmCvar_t	RMG_distancecull;

vmCvar_t	cg_blueteamname;
vmCvar_t	cg_redteamname;

vmCvar_t	cg_damageindicator;
vmCvar_t	cg_tracerChance;
vmCvar_t	cg_animBlend;

vmCvar_t	cg_automap_x;
vmCvar_t	cg_automap_y;
vmCvar_t	cg_automap_w;
vmCvar_t	cg_automap_h;
vmCvar_t	cg_automap_a;

vmCvar_t	ui_info_redcount;
vmCvar_t	ui_info_bluecount;
vmCvar_t	ui_info_speccount;
vmCvar_t	ui_info_freecount;
vmCvar_t	ui_info_pickupsdisabled;
vmCvar_t	ui_info_seenobjectives;

vmCvar_t	cg_voiceRadio;
vmCvar_t	cg_voiceGlobal;
vmCvar_t	cg_soundGlobal;
vmCvar_t	cg_soundFrag;

vmCvar_t	cg_weaponMenuFast;

vmCvar_t	cg_bodyTime;

vmCvar_t	rw_enabled;

typedef struct 
{
	vmCvar_t	*vmCvar;
	char		*cvarName;
	char		*defaultString;
	int			cvarFlags;
	float		mMinValue, mMaxValue;
} cvarTable_t;

static cvarTable_t cvarTable[] = 
{ 
	{ &cg_lockBlood, "lock_blood", "1", 0 },
	{ &cg_lockSever, "lock_sever", "1", 0 },
	{ &cg_lockDeaths, "lock_deaths", "1", 0 },
	{ &rw_enabled,	  "rw_enabled",	"0", 0 },

	{ &con_notifyTime, "con_notifyTime", "3", 0 },

	{ &cg_shellEjection, "cg_shellEjection", "1", CVAR_ARCHIVE },
	{ &cg_ignore, "cg_ignore", "0", 0 },
	{ &cg_autoswitch, "cg_autoswitch", "2", CVAR_ARCHIVE },
	{ &cg_drawGun, "cg_drawGun", "1", CVAR_ARCHIVE },
	{ &cg_fov, "cg_fov", "80", CVAR_ARCHIVE | CVAR_LOCK_RANGE, 80.0, 100.0 },
	{ &cg_stereoSeparation, "cg_stereoSeparation", "0.4", CVAR_ARCHIVE  },
	{ &cg_shadows, "cg_shadows", "1", CVAR_ARCHIVE  },
	{ &cg_draw2D, "cg_draw2D", "1", CVAR_ARCHIVE|CVAR_CHEAT  },
	{ &cg_drawTimer, "cg_drawTimer", "1", CVAR_ARCHIVE  },
	{ &cg_drawFPS, "cg_drawFPS", "0", CVAR_ARCHIVE  },
	{ &cg_drawSnapshot, "cg_drawSnapshot", "0", CVAR_ARCHIVE  },
	{ &cg_drawCrosshair, "cg_drawCrosshair", "4", CVAR_ARCHIVE },
	{ &cg_drawCrosshairNames, "cg_drawCrosshairNames", "1", CVAR_ARCHIVE },
	{ &cg_crosshairSize, "cg_crosshairSize", "24", CVAR_ARCHIVE },
	{ &cg_crosshairX, "cg_crosshairX", "0", CVAR_ARCHIVE },
	{ &cg_crosshairY, "cg_crosshairY", "0", CVAR_ARCHIVE },
	{ &cg_crosshairRGBA, "cg_crosshairRGBA", "1,1,1,1", CVAR_ARCHIVE },
	{ &cg_crosshairFriendRGBA, "cg_crosshairFriendRGBA", "1,0,0,1", CVAR_ARCHIVE },
	{ &cg_simpleItems, "cg_simpleItems", "0", CVAR_ARCHIVE },
	{ &cg_lagometer, "cg_lagometer", "0", CVAR_ARCHIVE },
	{ &cg_gun_x, "cg_gunX", "0", CVAR_CHEAT },
	{ &cg_gun_y, "cg_gunY", "0", CVAR_CHEAT },
	{ &cg_gun_z, "cg_gunZ", "0", CVAR_CHEAT },
	{ &cg_centertime, "cg_centertime", "4", CVAR_ARCHIVE },
	{ &cg_centerY,	  "cg_centerY",  "70", CVAR_ARCHIVE },
	{ &cg_runpitch, "cg_runpitch", "0.002", CVAR_ARCHIVE|CVAR_LOCK_RANGE, 0.0f, 0.002f },
	{ &cg_runroll, "cg_runroll", "0.005", CVAR_ARCHIVE|CVAR_LOCK_RANGE, 0.0f, 0.005f },
	{ &cg_bobup , "cg_bobup", "0.005", CVAR_ARCHIVE|CVAR_LOCK_RANGE, 0.0f, 0.005f },
	{ &cg_bobpitch, "cg_bobpitch", "0.001", CVAR_ARCHIVE|CVAR_LOCK_RANGE, 0.0f, 0.001f },
	{ &cg_bobroll, "cg_bobroll", "0.001", CVAR_ARCHIVE|CVAR_LOCK_RANGE, 0.0f, 0.001f },
	{ &cg_debugEvents, "cg_debugevents", "0", CVAR_CHEAT },
	{ &cg_errorDecay, "cg_errordecay", "100", 0 },
	{ &cg_nopredict, "cg_nopredict", "0", 0 },
	{ &cg_showmiss, "cg_showmiss", "0", 0 },
	{ &cg_footsteps, "cg_footsteps", "1", CVAR_CHEAT },

	{ &cg_goreDetail, "cg_goreDetail", "1", CVAR_ARCHIVE },

	{ &cg_thirdPerson, "cg_thirdPerson", "0", CVAR_CHEAT },
	{ &cg_thirdPersonRange, "cg_thirdPersonRange", "80", CVAR_CHEAT },
	{ &cg_thirdPersonYaw, "cg_thirdPersonYaw", "0", CVAR_CHEAT },
	{ &cg_thirdPersonPitch, "cg_thirdPersonPitch", "15", CVAR_CHEAT },
	
	{ &cg_thirdPersonHorzOffset, "cg_thirdPersonHorzOffset", "0", CVAR_CHEAT },

	{ &cg_chatTime, "cg_chatTime", "8000", CVAR_ARCHIVE  },
	{ &cg_chatHeight, "cg_chatHeight", "4", CVAR_ARCHIVE  },
	{ &cg_forceModel, "cg_forceModel", "0", CVAR_ARCHIVE  },
	{ &cg_predictItems, "cg_predictItems", "1", CVAR_ARCHIVE },
	{ &cg_antiLag, "cg_antiLag", "1", CVAR_ARCHIVE | CVAR_USERINFO },
#ifdef _SOF2_FLESHIMPACTPREDICTION
	{ &cg_impactPrediction, "cg_impactPrediction", "2", CVAR_ARCHIVE },
#else
	{ &cg_impactPrediction, "cg_impactPrediction", "1", CVAR_ARCHIVE },
#endif
	{ &cg_autoReload, "cg_autoReload", "1", CVAR_ARCHIVE | CVAR_USERINFO },
	{ &cg_deferPlayers, "cg_deferPlayers", "1", CVAR_ARCHIVE },
	{ &cg_stats, "cg_stats", "0", 0 },
	{ &cg_drawFriend, "cg_drawFriend", "1", CVAR_ARCHIVE },
	{ &cg_teamChatsOnly, "cg_teamChatsOnly", "0", CVAR_ARCHIVE },
	{ &cg_noVoiceText, "cg_noVoiceText", "0", CVAR_ARCHIVE },
	// the following variables are created in other parts of the system,
	// but we also reference them here
	{ &cg_buildScript, "com_buildScript", "0", 0 },	// force loading of all possible data amd error on failures
	{ &cg_paused, "cl_paused", "0", CVAR_ROM },
	{ &cg_synchronousClients, "g_synchronousClients", "0", 0 },	// communicated by systeminfo

	{ &cg_currentSelectedPlayer, "cg_currentSelectedPlayer", "0", CVAR_ARCHIVE},
	{ &cg_currentSelectedPlayerName, "cg_currentSelectedPlayerName", "", CVAR_ARCHIVE},
	{ &cg_hudFiles, "cg_hudFiles", "ui/hud.txt", CVAR_ARCHIVE},

	{ &cg_cameraOrbit, "cg_cameraOrbit", "0", CVAR_CHEAT},
	{ &cg_cameraOrbitDelay, "cg_cameraOrbitDelay", "50", CVAR_ARCHIVE},
	{ &cg_timescale, "timescale", "1", 0},
	{ &cg_smoothClients, "cg_smoothClients", "0", CVAR_USERINFO | CVAR_ARCHIVE},
	{ &cg_cameraMode, "com_cameraMode", "0", CVAR_CHEAT},

	{ &pmove_fixed, "pmove_fixed", "0", 0},
	{ &pmove_msec, "pmove_msec", "8", 0},
	{ &cg_noProjectileTrail, "cg_noProjectileTrail", "0", CVAR_ARCHIVE},

	{ &cg_DebugGore, "cg_debuggore", "", CVAR_CHEAT },
	{ &RMG_distancecull, "RMG_distancecull", "5000", CVAR_CHEAT },

	{ &cg_blueteamname, "ui_blueteamname", "0", CVAR_ROM|CVAR_INTERNAL },
	{ &cg_redteamname, "ui_redteamname", "0", CVAR_ROM|CVAR_INTERNAL },

	{ &cg_damageindicator, "cg_damageindicator", "1.5", CVAR_ARCHIVE },
	{ &cg_tracerChance,	   "cg_tracerChance", "1", CVAR_ARCHIVE | CVAR_LOCK_RANGE, 0.0f, 0.50f },

	{ &cg_animBlend, "cg_animBlend", "1", CVAR_ARCHIVE },

	{ &cg_automap_x, "cg_automap_x", "485", CVAR_ARCHIVE | CVAR_LOCK_RANGE, 0.0, 629.0 },
	{ &cg_automap_y, "cg_automap_y", "5", CVAR_ARCHIVE  | CVAR_LOCK_RANGE, 0.0, 469.0},
	{ &cg_automap_w, "cg_automap_w", "150", CVAR_ARCHIVE | CVAR_LOCK_RANGE, 10.0, 640.0 },
	{ &cg_automap_h, "cg_automap_h", "150", CVAR_ARCHIVE | CVAR_LOCK_RANGE, 10.0, 480.0},
	{ &cg_automap_a, "cg_automap_a", "0.75", CVAR_ARCHIVE | CVAR_LOCK_RANGE, 0.0, 1.0},

	{ &cg_drawRadar, "cg_drawRadar", "2", CVAR_ARCHIVE },
	{ &cg_drawTeamScores, "cg_drawTeamScores", "1", CVAR_ARCHIVE },

	{ &ui_info_redcount, "ui_info_redcount", "0", CVAR_INTERNAL|CVAR_ROM },
	{ &ui_info_bluecount, "ui_info_bluecount", "0", CVAR_INTERNAL|CVAR_ROM },
	{ &ui_info_speccount, "ui_info_speccount", "0", CVAR_INTERNAL|CVAR_ROM },
	{ &ui_info_freecount, "ui_info_freecount", "0", CVAR_INTERNAL|CVAR_ROM },
	{ &ui_info_pickupsdisabled, "ui_info_pickupsdisabled", "0", CVAR_INTERNAL|CVAR_ROM },
	{ &ui_info_seenobjectives, "ui_info_seenobjectives", "0", CVAR_INTERNAL|CVAR_ROM },

	{ &cg_voiceRadio,	"cg_voiceRadio",	"1",	CVAR_ARCHIVE,	0.0f,	0.0f },
	{ &cg_voiceGlobal,	"cg_voiceGlobal",	"1",	CVAR_ARCHIVE,	0.0f,	0.0f },
	{ &cg_soundGlobal,	"cg_soundGlobal",	"1",	CVAR_ARCHIVE,	0.0f,	0.0f },
	{ &cg_soundFrag,	"cg_soundFrag",		"1",	CVAR_ARCHIVE,	0.0f,	0.0f },
	
	{ &cg_weaponMenuFast,  "cg_weaponMenuFast", "0", CVAR_ARCHIVE },

	{ &cg_bodyTime,		"cg_bodyTime",			"0",	CVAR_ARCHIVE,	0.0f, 0.0f },
};

static int  cvarTableSize = sizeof( cvarTable ) / sizeof( cvarTable[0] );

/*
=================
CG_RegisterCvars
=================
*/
void CG_RegisterCvars( void ) 
{
	int			i;
	cvarTable_t	*cv;
	char		var[MAX_TOKEN_CHARS];

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) 
	{
		trap_Cvar_Register( cv->vmCvar, cv->cvarName, cv->defaultString, cv->cvarFlags, cv->mMinValue, cv->mMaxValue );
	}

	// see if we are also running the server on this machine
	trap_Cvar_VariableStringBuffer( "sv_running", var, sizeof( var ) );
	cgs.localServer = atoi( var );

	forceModelModificationCount = cg_forceModel.modificationCount;

	trap_Cvar_Register(NULL, "identity",		DEFAULT_IDENTITY, CVAR_USERINFO | CVAR_ARCHIVE, 0.0, 0.0 );
	trap_Cvar_Register(NULL, "team_identity",	DEFAULT_IDENTITY, CVAR_USERINFO | CVAR_ARCHIVE, 0.0, 0.0 );
	trap_Cvar_Register(NULL, "outfitting",		"AAAA",			  CVAR_USERINFO | CVAR_ARCHIVE, 0.0, 0.0 );

	// Cvars uses for transferring data between client and server
	trap_Cvar_Register(NULL, "ui_about_gametype",		"0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0 );
	trap_Cvar_Register(NULL, "ui_about_gametypename",	"0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0 );
	trap_Cvar_Register(NULL, "ui_about_scorelimit",		"0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0 );
	trap_Cvar_Register(NULL, "ui_about_timelimit",		"0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0 );
	trap_Cvar_Register(NULL, "ui_about_maxclients",		"0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0 );
	trap_Cvar_Register(NULL, "ui_about_dmflags",		"0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0 );
	trap_Cvar_Register(NULL, "ui_about_mapname",		"0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0 );
	trap_Cvar_Register(NULL, "ui_about_hostname",		"0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0 );
	trap_Cvar_Register(NULL, "ui_about_needpass",		"0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0 );
	trap_Cvar_Register(NULL, "ui_about_botminplayers",	"0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0 );
	trap_Cvar_Register(NULL, "ui_info_availableweapons","0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0 );

	trap_Cvar_Register(NULL, "ui_info_redscore",		"0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0 );
	trap_Cvar_Register(NULL, "ui_info_bluescore",		"0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0 );

	trap_Cvar_Register(NULL, "cg_lastobjectives",		"0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0 );

	trap_Cvar_Register(NULL, "ui_about_seed",			"0", CVAR_ROM|CVAR_INTERNAL, 0.0, 0.0 );

	trap_Cvar_Set ( "ui_info_seenobjectives", "0" );
	trap_Cvar_Set ( "ui_info_redscore", "0" );
	trap_Cvar_Set ( "ui_info_bluescore", "0" );
}

/*																																			
===================
CG_ForceModelChange
===================
*/
static void CG_ForceModelChange( void ) 
{
	int		i;

	for (i=0 ; i<MAX_CLIENTS ; i++) 
	{
		const char		*clientInfo;

		clientInfo = CG_ConfigString( CS_PLAYERS+i );
		if ( !clientInfo[0] ) 
		{
			continue;
		}

		CG_NewClientInfo( i );
	}
}

/*
==================
CG_UpdateTeamCountCvars
==================
*/
void CG_UpdateTeamCountCvars ( void )
{
	int i;
	int counts[TEAM_NUM_TEAMS];

	memset ( counts, 0, sizeof(counts) );

	for ( i = 0 ; i < cgs.maxclients ; i++ ) 
	{
		clientInfo_t* cl;
		
		cl = &cgs.clientinfo[ i ];

		if ( !cl->infoValid )
		{
			continue;
		}
		
		counts[cl->team]++;
	}

	trap_Cvar_Set ( "ui_info_redcount",  va("%i", counts[TEAM_RED] ) );
	trap_Cvar_Set ( "ui_info_bluecount", va("%i", counts[TEAM_BLUE]) );
	trap_Cvar_Set ( "ui_info_freecount", va("%i", counts[TEAM_FREE]) );
	trap_Cvar_Set ( "ui_info_speccount", va("%i", counts[TEAM_SPECTATOR] ) );
}

/*
=================
CG_UpdateCvars
=================
*/
void CG_UpdateCvars( void ) 
{
	int			i;
	cvarTable_t	*cv;

	static int crosshairRGBAModificationCount = -1;
	static int crosshairFriendRGBAModificationCount = -1;

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) 
	{
		trap_Cvar_Update( cv->vmCvar );
	}

	// if force model changed
	if ( forceModelModificationCount != cg_forceModel.modificationCount ) 
	{
		forceModelModificationCount = cg_forceModel.modificationCount;
		CG_ForceModelChange();
	}

	// If the crosshair RGBA was modified then update the globals
	if ( crosshairRGBAModificationCount != cg_crosshairRGBA.modificationCount )
	{
		sscanf ( cg_crosshairRGBA.string, "%f,%f,%f,%f", 
				 &cg.crosshairRGBA[0],
				 &cg.crosshairRGBA[1],
				 &cg.crosshairRGBA[2],
				 &cg.crosshairRGBA[3] );

		crosshairRGBAModificationCount = cg_crosshairRGBA.modificationCount;
	}

	// If the crosshair RGBA was modified then update the globals
	if ( crosshairFriendRGBAModificationCount != cg_crosshairFriendRGBA.modificationCount )
	{
		sscanf ( cg_crosshairFriendRGBA.string, "%f,%f,%f,%f", 
				 &cg.crosshairFriendRGBA[0],
				 &cg.crosshairFriendRGBA[1],
				 &cg.crosshairFriendRGBA[2],
				 &cg.crosshairFriendRGBA[3] );

		crosshairFriendRGBAModificationCount = cg_crosshairFriendRGBA.modificationCount;
	}
}

int CG_CrosshairPlayer( void ) 
{
	if ( cg.time > ( cg.crosshairClientTime + 1000 ) ) 
	{
		return -1;
	}
	return cg.crosshairClientNum;
}

int CG_LastAttacker( void ) {
	if ( !cg.attackerTime ) {
		return -1;
	}
	return cg.snap->ps.persistant[PERS_ATTACKER];
}

#ifndef CGAME_HARD_LINKED
// this is only here so the functions in q_shared.c and bg_*.c can link (FIXME)

void QDECL Com_Error( int level, const char *msg, ... ) 
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	trap_Error( text );
}

void QDECL Com_Printf( const char *msg, ... ) 
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	trap_Print( text );
}

#endif

/*
=================
CG_GetEntity

This function handles the predicted player entitiy specially.  The player
should always be using the predicted entity rather than whats in the main 
entity array.
=================
*/
centity_t* CG_GetEntity ( int index )
{
/*
	if ( index == cg.clientNum )
	{
		return &cg.predictedPlayerEntity;
	}
*/

	return &cg_entities[index];
}

/*
================
CG_Argv
================
*/
const char *CG_Argv( int arg ) {
	static char	buffer[MAX_STRING_CHARS];

	trap_Argv( arg, buffer, sizeof( buffer ) );

	return buffer;
}


//========================================================================

/*
=================
CG_RegisterItemSounds

The server says this item is used on this level
=================
*/
static void CG_RegisterItemSounds( int itemNum ) {
	gitem_t			*item;
	char			data[MAX_QPATH];
	char			*s, *start;
	int				len;

	item = &bg_itemlist[ itemNum ];

	if( item->pickup_sound ) {
		trap_S_RegisterSound( item->pickup_sound );
	}

	// parse the space seperated precache string for other media
	s = item->sounds;
	if (!s || !s[0])
	{
		return;
	}

	while (*s) {
		start = s;
		while (*s && *s != ' ') 
		{
			s++;
		}

		len = s-start;
		if (len >= MAX_QPATH || len < 5) 
		{
			Com_Error( ERR_FATAL, "PrecacheItem: %s has bad precache string", item->classname);
			return;
		}
		memcpy (data, start, len);
		data[len] = 0;
		if ( *s ) 
		{
			s++;
		}

		if ( !strcmp(data+len-3, "wav" )) 
		{
			trap_S_RegisterSound( data );
		}
	}
}

/*
=================
CG_RegisterLadders

Registers all the ladders in the current map
=================
*/
static void CG_RegisterLadders ( void )
{
	int			i;
	const char	*ladder;

	for ( i = 0 ; i < MAX_LADDERS ; i++ ) 
	{
		vec3_t absmin;
		vec3_t absmax;
		vec3_t fwd;
		vec3_t angles;

		ladder = CG_ConfigString( CS_LADDERS + i );
		
		// No ladder?	
		if ( !ladder || !ladder[0] )
		{
			continue;
		}

		VectorClear ( angles );
		sscanf ( ladder, "%f,%f,%f,%f,%f,%f,%f",
				 &absmin[0], &absmin[1], &absmin[2],
				 &absmax[0], &absmax[1], &absmax[2],
				 &angles[YAW] );

		AngleVectors( angles, fwd, 0, 0);

		BG_AddLadder ( absmin, absmax, fwd );
	}
}

/*
=================
CG_RegisterSounds

called during a precache command
=================
*/
static void CG_RegisterSounds( void ) 
{
	int		i;
	char	items[MAX_ITEMS+1];
	const char	*soundName;

	// voice commands
	CG_LoadVoiceChats();

	CG_LoadingStage ( 3 );

	cgs.media.talkSound = trap_S_RegisterSound( "sound/player/talk.wav" );

	cgs.media.waterFootstep[0] = trap_S_RegisterSound( "sound/player/steps/water/water_step0");	
	cgs.media.waterFootstep[1] = trap_S_RegisterSound( "sound/player/steps/water/water_step1");	

	cgs.media.waterWade[0] = trap_S_RegisterSound( "sound/player/steps/water/water_wade0");	
	cgs.media.waterWade[1] = trap_S_RegisterSound( "sound/player/steps/water/water_wade1");	

	cgs.media.waterJumpIn	= trap_S_RegisterSound( "sound/player/jumps/water_jump");	
	cgs.media.waterLeave    = trap_S_RegisterSound( "sound/pain_death/mullins/water_surface");	

	cgs.media.armorHitSound[0] = trap_S_RegisterSound( "sound/pain_death/mullins/armor01");	
	cgs.media.armorHitSound[1] = trap_S_RegisterSound( "sound/pain_death/mullins/armor02");	
	cgs.media.fleshHitSound[0] = trap_S_RegisterSound( "sound/pain_death/mullins/hit01");	
	cgs.media.fleshHitSound[1] = trap_S_RegisterSound( "sound/pain_death/mullins/hit02");	

	// only register the items that the server says we need
	strcpy( items, CG_ConfigString( CS_ITEMS ) );

	CG_LoadingStage ( 4 );

	for (i = 0; i < MAX_AMBIENT_SOUNDSETS; i++)	// 0 is reserved for current
	{
		const char *psString = CG_ConfigString( CS_AMBIENT_SOUNDSETS + i);			

		if (psString && strlen(psString))
		{
			trap_AS_AddPrecacheEntry( psString );
		}
		else
		{
//			break;	// EOL
		}
	}

	trap_AS_ParseSets();

	for ( i = 1 ; i < bg_numItems ; i++ ) 
	{
		CG_RegisterItemSounds( i );
	}

	for ( i = 1 ; i < MAX_SOUNDS ; i++ ) 
	{
		soundName = CG_ConfigString( CS_SOUNDS+i );
		
		if ( !soundName[0] ) 
		{
			break;
		}
		
		// custom sound
		if ( soundName[0] == '*' ) 
		{
			continue;	
		}

		cgs.gameSounds[i] = trap_S_RegisterSound( soundName );
	}


	cgs.media.glassBreakSound = trap_S_RegisterSound("sound/effects/glassbreak1.wav");

	// Respawn sound is only used in gametypes where you respawn
	if ( cgs.gametypeData->respawnType != RT_NONE )
	{
		cgs.media.respawnSound = trap_S_RegisterSound("sound/player_respawn.wav");
	}

	cgs.media.itemRespawnSound = trap_S_RegisterSound ( "sound/item_respawn.mp3" );

	cgs.media.fragSound       = trap_S_RegisterSound ( "sound/frag.mp3" );
	cgs.media.fragSelfSound   = trap_S_RegisterSound ( "sound/self_frag.mp3" );
	cgs.media.zoomSound       = trap_S_RegisterSound ( "sound/weapons/sniper/scope_zoom.wav" );
	cgs.media.gogglesOnSound  = trap_S_RegisterSound ( "sound/weapons/goggles/turn_on.mp3" );
	cgs.media.gogglesOffSound = trap_S_RegisterSound ( "sound/weapons/goggles/turn_off.mp3" );

	// Move out sound
	if ( cgs.gametypeData->respawnType == RT_NONE )
	{
		cgs.media.goSound = trap_S_RegisterSound ( "sound/radio/male/move.mp3" );
	}

	// bullet flyby sounds
	for (i = 0; i < NUMFLYBYS; i++)
	{
		cgs.media.flybySounds[i] = trap_S_RegisterSound ( va("sound/player/bullet_impacts/whine/b_whine%d", i));
	}

	// Water sounds
	cgs.media.drownPainSound[0] = trap_S_RegisterSound ( "sound/pain_death/mullins/drown01.mp3" );
	cgs.media.drownPainSound[1] = trap_S_RegisterSound ( "sound/pain_death/mullins/drown02.mp3" );
	cgs.media.drownDeathSound   = trap_S_RegisterSound ( "sound/pain_death/mullins/drown_dead.mp3" );

	CG_LoadingStage ( 5 );
}


//-------------------------------------
// CG_RegisterEffects
// 
// Handles precaching all effect files
//	and any shader, model, or sound
//	files an effect may use.
//-------------------------------------
static void CG_RegisterEffects( void )
{
	const char	*effectName;
	int			i;

	for ( i = 1 ; i < MAX_FX ; i++ ) 
	{
		effectName = CG_ConfigString( CS_EFFECTS + i );

		if ( !effectName[0] ) 
		{
			break;
		}

		trap_FX_RegisterEffect( effectName );
	}
}

//===================================================================================


/*
=================
CG_RegisterGraphics

This function may execute for a couple of minutes with a slow disk.
=================
*/
static void CG_RegisterGraphics( void ) 
{
	int			i;
	char		items[MAX_ITEMS+1];
	int			sub;
	char		temp[MAX_QPATH];
	int			breakPoint;
	const char	*terrainInfo;
	int			terrainID;
	vec3_t		distance;

	// clear any references to old media
	memset( &cg.refdef, 0, sizeof( cg.refdef ) );
	trap_R_ClearScene();

	// Load the map
	CG_LoadingString( cgs.mapname );

	trap_R_LoadWorldMap( cgs.mapname );

	// precache status bar pics
	CG_LoadingString( "game media" );

	CG_LoadingStage ( 6 );

	cgs.media.cursor = trap_R_RegisterShaderNoMip( "gfx/menus/cursor/cursor" );

	// Register the graphics for the scoreboard
	cgs.media.scoreboard	   = trap_R_RegisterShaderNoMip ( "gfx/menus/scoreboard/scoreboard.tga" );
	cgs.media.scoreboardHeader = trap_R_RegisterShaderNoMip ( "gfx/menus/scoreboard/scorelineheader.tga" );
	cgs.media.scoreboardLine   = trap_R_RegisterShaderNoMip ( "gfx/menus/scoreboard/scoreline.tga" );
	cgs.media.scoreboardFooter = trap_R_RegisterShaderNoMip ( "gfx/menus/scoreboard/scorelinefooter.tga" );
	cgs.media.scoreboardTotals = trap_R_RegisterShaderNoMip ( "gfx/menus/scoreboard/scorelinetotals.tga" );

	cgs.media.smokePuffShader = trap_R_RegisterShader( "smokePuff" );
	cgs.media.lagometerShader = trap_R_RegisterShader("gfx/menus/hud/lagometer.tga" );
	cgs.media.disconnectShader = trap_R_RegisterShader("gfx/menus/hud/disconnect.tga" );
	cgs.media.waterBubbleShader = trap_R_RegisterShader( "gfx/misc/bubble" );
	cgs.media.backTileShader = trap_R_RegisterShader( "gfx/2d/backtile" );

	cgs.media.mAutomapPlayerIcon = trap_R_RegisterShader( "gfx/menus/rmg/arrow_w" );

	CG_LoadingStage ( 7 );

	Com_Printf( S_COLOR_CYAN "---------- Fx System Initialization ---------\n" );
	trap_FX_InitSystem( &cg.refdef);

	Com_Printf( S_COLOR_CYAN "----- Fx System Initialization Complete -----\n" );
	CG_RegisterEffects();

	// Initialize the materials
	trap_MAT_Init();

	CG_LoadingStage ( 8 );

	// Initialize the crosshairs
	for ( i = 0 ; i < NUM_CROSSHAIRS ; i++ ) 
	{
		cgs.media.crosshairShader[i] = trap_R_RegisterShader( va("gfx/menus/crosshairs/ch%i", i) );
	}

	CG_LoadingStage ( 9 );

	if ( cgs.gametypeData->teams ) 
	{
		cgs.media.blueFriendShader = trap_R_RegisterShader ( "gfx/menus/hud/team_blue" );
		cgs.media.redFriendShader  = trap_R_RegisterShader ( "gfx/menus/hud/team_red" );
		cgs.media.radarShader	   = trap_R_RegisterShaderNoMip ( "gfx/menus/hud/radar.png" );
	}

	cgs.media.botSmallShader = trap_R_RegisterShaderNoMip ( "gfx/menus/hud/bot_small" );
	cgs.media.armorShader	 = trap_R_RegisterShaderNoMip ( "gfx/menus/hud/hud_armor" );
	cgs.media.deadShader	 = trap_R_RegisterShaderNoMip ( "gfx/menus/hud/dead" );

	CG_LoadingStage ( 10 );

	// Initialize weapons, load them all 
	memset( cg_weapons, 0, sizeof( cg_weapons ) );

	for (i = WP_NONE + 1; i < WP_NUM_WEAPONS; i ++ )
	{
		if ( cgs.pickupsDisabled && !BG_IsWeaponAvailableForOutfitting ( i, 2 ) )
		{
			continue;
		}

		CG_RegisterWeapon ( i );
	}

	CG_LoadingStage ( 11 );

	// only register the items that the server says we need
	strcpy( items, CG_ConfigString( CS_ITEMS) );

	for ( i = 1 ; i < bg_numItems ; i++ ) 
	{
		// Dont bother with weapons, theyare already loaded

		if ( items[ i ] == '1' || cg_buildScript.integer ) 
		{
			CG_RegisterItemVisuals( i );
		}
	}

	CG_LoadingStage ( 12 );

	// wall marks
	cgs.media.burnMarkShader	= trap_R_RegisterShader( "gfx/damage/burnmark5" );
	cgs.media.shadowMarkShader	= trap_R_RegisterShader( "markShadow" );
	cgs.media.wakeMarkShader	= trap_R_RegisterShader( "wake" );
	cgs.media.bloodMarkShader	= trap_R_RegisterShader( "bloodMark" );
	cgs.media.mAutomap			= 0;

	// register the inline models
	breakPoint = cgs.numInlineModels = trap_CM_NumInlineModels();
	for ( i = 0 ; i < cgs.numInlineModels ; i++ ) 
	{
		char	name[10];
		vec3_t	mins;
		vec3_t	maxs;
		int		j;

		Com_sprintf( name, sizeof(name), "*%i", i );
		cgs.inlineDrawModel[i] = trap_R_RegisterModel( name );
		if (!cgs.inlineDrawModel[i])
		{
			breakPoint = i;
			break;
		}

		trap_R_ModelBounds( cgs.inlineDrawModel[i], mins, maxs );
		for ( j = 0 ; j < 3 ; j++ ) 
		{
			cgs.inlineModelMidpoints[i][j] = mins[j] + 0.5 * ( maxs[j] - mins[j] );
		}
	}

	// register all the server specified models
	for (i=1 ; i<MAX_MODELS ; i++) 
	{
		const char	*modelName;

		modelName = CG_ConfigString( CS_MODELS+i );
		if ( !modelName[0] ) 
		{
			break;
		}
		cgs.gameModels[i] = trap_R_RegisterModel( modelName );
	}

	// register all the server specified icons
	for ( i = 1; i < MAX_ICONS; i ++ )
	{
		const char* iconName;

		iconName = CG_ConfigString ( CS_ICONS + i );
		if ( !iconName[0] )
		{
			break;
		}

		cgs.gameIcons[i] = trap_R_RegisterShaderNoMip ( iconName );
	}

	CG_LoadingString( "BSP instances" );

	for(i = 1; i < MAX_SUB_BSP; i++)
	{
		const char		*bspName = 0;
		vec3_t			mins, maxs;
		int				j;

		bspName = CG_ConfigString( CS_BSP_MODELS+i );
		if ( !bspName[0] ) 
		{
			break;
		}

		trap_CM_LoadMap( bspName, qtrue );
		cgs.inlineDrawModel[breakPoint] = trap_R_RegisterModel( bspName );
		trap_R_ModelBounds( cgs.inlineDrawModel[breakPoint], mins, maxs );
		for ( j = 0 ; j < 3 ; j++ ) 
		{
			cgs.inlineModelMidpoints[breakPoint][j] = mins[j] + 0.5 * ( maxs[j] - mins[j] );
		}
		breakPoint++;
		for(sub=1;sub<MAX_MODELS;sub++)
		{
			Com_sprintf(temp, MAX_QPATH, "*%d-%d", i, sub);
			cgs.inlineDrawModel[breakPoint] = trap_R_RegisterModel( temp );
			if (!cgs.inlineDrawModel[breakPoint])
			{
				break;
			}
			trap_R_ModelBounds( cgs.inlineDrawModel[breakPoint], mins, maxs );
			for ( j = 0 ; j < 3 ; j++ ) 
			{
				cgs.inlineModelMidpoints[breakPoint][j] = mins[j] + 0.5 * ( maxs[j] - mins[j] );
			}
			breakPoint++;
		}
	}

//	cg.mInRMG = qfalse;

	// Register  all the terrain ents
	CG_LoadingString( "Creating terrain" );
	for(i = 1; i < MAX_TERRAINS; i++)
	{
		terrainInfo = CG_ConfigString( CS_TERRAINS + i );
		if ( !terrainInfo[0] )
		{
			break;
		}

		terrainID = trap_CM_RegisterTerrain(terrainInfo);
/*
		if (i == 1)
		{
			cg.mInRMG = qtrue;
		}
*/

		trap_RMG_Init(terrainID, terrainInfo);

		// Send off the terrainInfo to the renderer
		trap_RE_InitRendererTerrain( terrainInfo );
	}

	if (cg.mInRMG)
	{
		const char* info;

		trap_CM_TM_Upload(0, 0);		// do not draw the origin / angles on the map itself
		cgs.media.mAutomap = trap_R_RegisterShader( "gfx/menus/rmg/automap" );

		cgs.mIRDist = RMG_distancecull.integer;

		info = CG_ConfigString ( CS_SYSTEMINFO );
		trap_Cvar_Set ( "ui_about_seed", Info_ValueForKey ( info,"RMG_textseed" ) );
	}
	else
	{
		trap_R_ModelBounds( cgs.inlineDrawModel[0], cgs.mWorldMins, cgs.mWorldMaxs );
		VectorSubtract(cgs.mWorldMaxs, cgs.mWorldMins, distance);
		cgs.mIRDist = VectorLength(distance);
//		cgs.mIRSeeThrough = cgs.mIRDist / 2.0;
	}
	cgs.mIRDist = 2500.0;
	cgs.mIRSeeThrough = 150.0;

	CG_LoadingString("skins");

	cgs.media.damageDirShader = trap_R_RegisterShader("gfx/misc/hit_direction_final");
	cgs.media.glassChunkEffect = trap_FX_RegisterEffect ( "chunks/debris_glass.efx" );

	cgs.media.playerFleshImpactEffect = trap_FX_RegisterEffect ( "blood_squirt_mp.efx" );
	cgs.media.mBloodSmall = trap_FX_RegisterEffect ( "impact_player_mp.efx" );

	cgs.media.test = trap_FX_RegisterEffect ( "arterial_squirt_small.efx" );

	trap_G2API_InitGhoul2Model ( &cgs.media.nightVisionModel, "models/characters/bolt_ons/nightvision_bolt_on.glm", 0, 0, 0, 0, 0 );

	CG_LoadingStage ( 13 );
}

/*																																			
===================
CG_RegisterClients
===================
*/
static void CG_RegisterClients( void ) 
{
	int		i;

	CG_LoadingClient(cg.clientNum);
	CG_NewClientInfo(cg.clientNum);

	for (i=0 ; i<MAX_CLIENTS ; i++) 
	{
		const char		*clientInfo;

		if (cg.clientNum == i) 
		{
			continue;
		}

		clientInfo = CG_ConfigString( CS_PLAYERS+i );
		
		if ( !clientInfo[0]) 
		{
			continue;
		}
		
		CG_LoadingClient( i );
		CG_NewClientInfo( i );
	}
}

//===========================================================================

/*
=================
CG_ConfigString
=================
*/
const char *CG_ConfigString( int index ) 
{
	if ( index < 0 || index >= MAX_CONFIGSTRINGS ) 
	{
		Com_Error( ERR_FATAL, "CG_ConfigString: bad index: %i", index );
	}

	return cgs.gameState.stringData + cgs.gameState.stringOffsets[ index ];
}

//==================================================================

/*
======================
CG_StartMusic

======================
*/
void CG_StartMusic( qboolean bForceStart ) 
{
	const char	*s;
	char		parm1[MAX_QPATH], parm2[MAX_QPATH];

	// start the background music
	s = (char *)CG_ConfigString( CS_MUSIC );
	Q_strncpyz( parm1, COM_Parse( &s ), sizeof( parm1 ) );
	Q_strncpyz( parm2, COM_Parse( &s ), sizeof( parm2 ) );

	if (!parm2[0])
	{
		strcpy(parm2, parm1);
	}
	if ( *parm1 && *parm2 )
	{
		trap_S_StartBackgroundTrack( parm1, parm2, !bForceStart );
	}
}

char *CG_GetMenuBuffer(const char *filename) 
{
	int	len;
	fileHandle_t	f;
	static char buf[MAX_MENUFILE];

	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( !f ) {
		trap_Print( va( S_COLOR_RED "menu file not found: %s, using default\n", filename ) );
		return NULL;
	}
	if ( len >= MAX_MENUFILE ) {
		trap_Print( va( S_COLOR_RED "menu file too large: %s is %i, max allowed is %i", filename, len, MAX_MENUFILE ) );
		trap_FS_FCloseFile( f );
		return NULL;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );

	return buf;
}

//
// ==============================
// new hud stuff ( mission pack )
// ==============================
//
qboolean CG_Asset_Parse(int handle) {
	pc_token_t token;
	const char *tempStr;

	if (!trap_PC_ReadToken(handle, &token))
		return qfalse;
	if (Q_stricmp(token.string, "{") != 0) {
		return qfalse;
	}
    
	while ( 1 ) 
	{
		if (!trap_PC_ReadToken(handle, &token))
			return qfalse;

		if (Q_stricmp(token.string, "}") == 0) 
		{
			return qtrue;
		}

		// font
		if (Q_stricmp(token.string, "defaultFont") == 0) 
		{
			if (!PC_String_Parse(handle, &tempStr) ) 
			{
				return qfalse;
			}
			cgDC.Assets.defaultFont = cgDC.registerFont(tempStr);
			continue;
		}

		// itemFocusSound
		if (Q_stricmp(token.string, "itemFocusSound") == 0) 
		{
			if (!PC_String_Parse(handle, &tempStr)) 
			{
				return qfalse;
			}
			cgDC.Assets.itemFocusSound = trap_S_RegisterSound( tempStr );
			continue;
		}

		if (Q_stricmp(token.string, "cursor") == 0) {
			if (!PC_String_Parse(handle, &cgDC.Assets.cursorStr)) {
				return qfalse;
			}
			cgDC.Assets.cursor = trap_R_RegisterShaderNoMip( cgDC.Assets.cursorStr);
			continue;
		}

		if (Q_stricmp(token.string, "fadeClamp") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.fadeClamp)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "fadeCycle") == 0) {
			if (!PC_Int_Parse(handle, &cgDC.Assets.fadeCycle)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "fadeAmount") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.fadeAmount)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "shadowX") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.shadowX)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "shadowY") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.shadowY)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "shader") == 0) 
		{
			if (!PC_String_Parse(handle, &tempStr)) 
			{
				return qfalse;
			}
			trap_R_RegisterShaderNoMip ( tempStr );
			continue;
		}

		if (Q_stricmp(token.string, "shadowColor") == 0) {
			if (!PC_Color_Parse(handle, &cgDC.Assets.shadowColor)) {
				return qfalse;
			}
			cgDC.Assets.shadowFadeClamp = cgDC.Assets.shadowColor[3];
			continue;
		}
	}
	return qfalse; // bk001204 - why not?
}

/*
======================
CG_ParseMenu
======================
*/
void CG_ParseMenu ( const char *menuFile )
{
	pc_token_t	token;
	int			handle;

	// Load the menu file
	handle = trap_PC_LoadSource(menuFile);
	if (!handle)
	{
		return;
	}

	while ( 1 ) 
	{
		if (!trap_PC_ReadToken( handle, &token )) 
		{
			break;
		}

		if ( token.string[0] == '}' ) 
		{
			break;
		}

		if (Q_stricmp(token.string, "assetGlobalDef") == 0) 
		{
			if (CG_Asset_Parse(handle)) 
			{
				continue;
			} 
			else 
			{
				break;
			}
		}

		if (Q_stricmp(token.string, "menudef") == 0) 
		{
			// start a new menu
			Menu_New ( handle );
		}
	}

	trap_PC_FreeSource(handle);
}

/*
======================
CG_LoadMenu
======================
*/
qboolean CG_LoadMenu ( const char **p ) 
{
	char *token;

	token = COM_ParseExt(p, qtrue);

	if (token[0] != '{') 
	{
		return qfalse;
	}

	while ( 1 ) 
	{
		token = COM_ParseExt(p, qtrue);
    
		if (Q_stricmp(token, "}") == 0) 
		{
			return qtrue;
		}

		if ( !token || token[0] == 0 ) 
		{
			return qfalse;
		}

		CG_ParseMenu(token); 
	}

	return qfalse;
}

/*
======================
CG_LoadMenus
======================
*/
void CG_LoadMenus ( const char *menuFile ) 
{
	char			*token;
	const char		*p;
	int				len, start;
	fileHandle_t	f;
	static char		buf[MAX_MENUDEFFILE];

	start = trap_Milliseconds();

	trap_PC_LoadGlobalDefines ( "ui/menudef.h" );

	// Open the hud menu file
	len = trap_FS_FOpenFile( menuFile, &f, FS_READ );
	if ( !f ) 
	{
		Com_Error( ERR_FATAL, va( S_COLOR_RED "hud menu file not found: ui/hud.txt, unable to continue!\n", menuFile ) );
		return;
	}

	// Too big?
	if ( len >= MAX_MENUDEFFILE ) 
	{
		Com_Error( ERR_FATAL, va( S_COLOR_RED "menu file too large: %s is %i, max allowed is %i", menuFile, len, MAX_MENUDEFFILE ) );
		trap_FS_FCloseFile( f );
		return;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );
	
	COM_Compress(buf);

	Menu_Reset();

	p = buf;

	while ( 1 ) 
	{
		token = COM_ParseExt( &p, qtrue );
		if( !token || token[0] == 0 || token[0] == '}') 
		{
			break;
		}

		if ( Q_stricmp( token, "}" ) == 0 ) 
		{
			break;
		}

		if (Q_stricmp(token, "loadmenu") == 0) 
		{
			if ( CG_LoadMenu ( &p ) ) 
			{
				continue;
			} 
			else 
			{
				break;
			}
		}
	}

	trap_PC_RemoveAllGlobalDefines ( );

	Com_Printf("HUD menu load time = %d milli seconds\n", trap_Milliseconds() - start);
}

/*
======================
CG_LoadMenus
======================
*/
static qboolean CG_OwnerDrawHandleKey(int ownerDraw, int flags, float *special, int key, const char* param ) 
{
	return qfalse;
}


static int CG_FeederCount(float feederID) 
{
	return 0;
}

static const char *CG_FeederItemText ( float feederID, int index, int column, qhandle_t *handle) 
{
	return "";
}

static qhandle_t CG_FeederItemImage(float feederID, int index) 
{
	return 0;
}

static void CG_FeederSelection(float feederID, int index) 
{
}

static float CG_Cvar_Get(const char *cvar) 
{
	char buff[128];
	memset(buff, 0, sizeof(buff));
	trap_Cvar_VariableStringBuffer(cvar, buff, sizeof(buff));

	return atof(buff);
}

static int CG_OwnerDrawWidth(int ownerDraw, qhandle_t font, float scale ) 
{
	switch (ownerDraw) 
	{
		case CG_GAME_TYPE:
			return trap_R_GetTextWidth (CG_GameTypeString(), font, scale, 0 );

		case CG_GAME_STATUS:
			return trap_R_GetTextWidth (CG_GetGameStatusText(), font, scale, 0 );
	}

	return 0;
}

static int CG_PlayCinematic(const char *name, float x, float y, float w, float h) {
  return trap_CIN_PlayCinematic(name, x, y, w, h, CIN_loop);
}

static void CG_StopCinematic(int handle) {
  trap_CIN_StopCinematic(handle);
}

static void CG_DrawCinematic(int handle, float x, float y, float w, float h) {
  trap_CIN_SetExtents(handle, x, y, w, h);
  trap_CIN_DrawCinematic(handle);
}

static void CG_RunCinematicFrame(int handle) {
  trap_CIN_RunCinematic(handle);
}

/*
=================
CG_LoadHudMenu
=================
*/
void CG_LoadHudMenu() 
{
	cgDC.registerShaderNoMip	= &trap_R_RegisterShaderNoMip;
	cgDC.setColor				= &trap_R_SetColor;
	cgDC.drawHandlePic			= &CG_DrawPic;
	cgDC.drawStretchPic			= &trap_R_DrawStretchPic;
	cgDC.drawText				= &CG_DrawText;
	cgDC.drawTextWithCursor		= &CG_DrawTextWithCursor;
	cgDC.getTextWidth			= &trap_R_GetTextWidth;
	cgDC.getTextHeight			= &trap_R_GetTextHeight;
	cgDC.registerModel			= &trap_R_RegisterModel;
	cgDC.modelBounds			= &trap_R_ModelBounds;
	cgDC.fillRect				= &CG_FillRect;
	cgDC.drawRect				= &CG_DrawRect;   
	cgDC.drawSides				= &CG_DrawSides;
	cgDC.drawTopBottom			= &CG_DrawTopBottom;
	cgDC.clearScene				= &trap_R_ClearScene;
	cgDC.addRefEntityToScene	= &trap_R_AddRefEntityToScene;
	cgDC.renderScene			= &trap_R_RenderScene;
	cgDC.registerFont			= &trap_R_RegisterFont;
	cgDC.ownerDrawItem			= &CG_OwnerDraw;
	cgDC.getValue				= &CG_GetValue;
	cgDC.ownerDrawVisible		= &CG_OwnerDrawVisible;
	cgDC.ownerDrawDisabled		= &CG_OwnerDrawDisabled;
	cgDC.runScript				= &CG_RunMenuScript;
	cgDC.getTeamColor			= &CG_GetTeamColor;
	cgDC.setCVar				= trap_Cvar_Set;
	cgDC.getCVarString			= trap_Cvar_VariableStringBuffer;
	cgDC.getCVarValue			= CG_Cvar_Get;
	cgDC.startLocalSound		= &trap_S_StartLocalSound;
	cgDC.ownerDrawHandleKey		= &CG_OwnerDrawHandleKey;
	cgDC.feederCount			= &CG_FeederCount;
	cgDC.feederItemImage		= &CG_FeederItemImage;
	cgDC.feederItemText			= &CG_FeederItemText;
	cgDC.feederSelection		= &CG_FeederSelection;
	cgDC.Error					= &Com_Error; 
	cgDC.Print					= &Com_Printf; 
	cgDC.ownerDrawWidth			= &CG_OwnerDrawWidth;
	cgDC.registerSound			= &trap_S_RegisterSound;
	cgDC.startBackgroundTrack	= &trap_S_StartBackgroundTrack;
	cgDC.stopBackgroundTrack	= &trap_S_StopBackgroundTrack;
	cgDC.playCinematic			= &CG_PlayCinematic;
	cgDC.stopCinematic			= &CG_StopCinematic;
	cgDC.drawCinematic =		 &CG_DrawCinematic;
	cgDC.runCinematicFrame		= &CG_RunCinematicFrame;

	cgDC.yscale = cgDC.glconfig.vidHeight * (1.0/480.0);
	cgDC.xscale = cgDC.glconfig.vidWidth * (1.0/640.0);

	Init_Display(&cgDC);

	Menu_Reset();

	CG_LoadMenus( "ui/hud.txt" );
}

/*
=================
CG_AssetCache
=================
*/
void CG_AssetCache() 
{
	cgDC.Assets.scrollBar			= trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR );
	cgDC.Assets.scrollBarArrowDown	= trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWDOWN );
	cgDC.Assets.scrollBarArrowUp	= trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWUP );
	cgDC.Assets.scrollBarArrowLeft	= trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWLEFT );
	cgDC.Assets.scrollBarArrowRight = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWRIGHT );
	cgDC.Assets.scrollBarThumb		= trap_R_RegisterShaderNoMip( ASSET_SCROLL_THUMB );
	cgDC.Assets.sliderBar			= trap_R_RegisterShaderNoMip( ASSET_SLIDER_BAR );
	cgDC.Assets.sliderThumb			= trap_R_RegisterShaderNoMip( ASSET_SLIDER_THUMB );
}

// initialise the cg_entities structure - take into account the ghoul2 stl stuff in the active snap shots
void CG_Init_CG(void)
{
	memset( &cg, 0, sizeof(cg));
}

// initialise the cg_entities structure - take into account the ghoul2 stl stuff
void CG_Init_CGents(void)
{
	memset(&cg_entities, 0, sizeof(cg_entities));
}

void CG_InitItems(void)
{
	memset( cg_items, 0, sizeof( cg_items ) );
}

void CG_TransitionPermanent(void)
{
	centity_t	*cent = cg_entities;
	int			i;

	cg_numpermanents = 0;
	for(i=0;i<MAX_GENTITIES;i++,cent++)
	{
		if (trap_GetDefaultState(i, &cent->currentState))
		{
			cent->nextState = cent->currentState;
			VectorCopy (cent->currentState.origin, cent->lerpOrigin);
			VectorCopy (cent->currentState.angles, cent->lerpAngles);
			cent->currentValid = qtrue;

			cg_permanents[cg_numpermanents++] = cent;
		}
	}
}

/*
=================
CG_Init

Called after every level change or subsystem restart
Will perform callbacks to make the loading info screen update.
=================
*/
void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum ) 
{
	const char	*s;

	// Some of the new SOF2 features required more traffic through 
	// the trap calls than than the original vm code would allow for.  Therefore
	// a block of shared memory is registered with the engine to allow for
	// larger pieced of data to be transferred back and forth.
	trap_CG_RegisterSharedMemory(cg.sharedBuffer);

	// clear everything
	CG_Init_CGents();
	CG_Init_CG();
	CG_InitItems();

	BG_ParseNPCFiles ( );

	memset( &cgs, 0, sizeof( cgs ) );
	memset( cg_weapons, 0, sizeof(cg_weapons) );

	// Load these first since they are used to display the status
	cgs.media.loadBulletShader	= trap_R_RegisterShaderNoMip ( "gfx/menus/misc/load_bullet.tga" );
	cgs.media.loadClipShader	= trap_R_RegisterShaderNoMip ( "gfx/menus/misc/load_clip.tga" );
	cgs.media.whiteShader		= trap_R_RegisterShaderNoMip ( "white" );

	cg.clientNum = clientNum;

	cgs.processedSnapshotNum = serverMessageNum;
	cgs.serverCommandSequence = serverCommandSequence;

	CG_LoadingStage ( 0 );

	CG_RegisterCvars();

	CG_InitConsoleCommands();

	// Populate the gametypeData array with the gametypes available
	// in the scripts directory
	BG_BuildGametypeList();

	cg.weaponSelect = WP_USSOCOM_PISTOL;

	// get the rendering configuration from the client system
	trap_GetGlconfig( &cgs.glconfig );
	cgs.screenXScale = cgs.glconfig.vidWidth / 640.0;
	cgs.screenYScale = cgs.glconfig.vidHeight / 480.0;

	// Register the fonts
	cgs.media.hudFont = trap_R_RegisterFont ( "hud" );
	cgs.media.lcdFont = trap_R_RegisterFont ( "lcdsmall" );

	// Load the user interface
	String_Init();
	CG_AssetCache();
	CG_LoadHudMenu();

	// get the gamestate from the client system
	trap_GetGameState( &cgs.gameState );

	CG_TransitionPermanent();

	// check version
	s = CG_ConfigString( CS_GAME_VERSION );
	if ( strcmp( s, GAME_VERSION ) ) 
	{
		Com_Error( ERR_FATAL, "Client/Server game mismatch: %s/%s", GAME_VERSION, s );
	}

	s = CG_ConfigString( CS_LEVEL_START_TIME );
	cgs.levelStartTime = atoi( s );
	
	CG_ParseServerinfo();

	CG_ParseGametypeFile ( );

	// load the new map
	CG_LoadingString( "collision map" );

	CG_LoadingStage ( 1 );

	trap_CM_LoadMap( cgs.mapname, qfalse );

	// Make sure we have update values (scores)
	CG_SetConfigValues();

	CG_LoadingStage ( 2 );

	// force players to load instead of defer
	cg.loading = qtrue;		

	CG_LoadingString( "sounds" );

	CG_RegisterLadders ( );
	CG_RegisterSounds();
	CG_InitHitModel( );

	CG_LoadingString( "graphics" );
	BG_ParseInviewFile ( );
	CG_RegisterGraphics ( );

//	CG_RegisterMission ( );

	CG_ParseGore();

	CG_LoadingString( "clients" );

	CG_RegisterClients();		// if low on memory, some clients will be deferred

	CG_LoadingStage ( 14 );

	CG_InitLocalEntities();

	CG_LoadingStage ( 15 );

	CG_StartMusic(qfalse);

	CG_LoadingString( "Clearing light styles" );
	CG_ClearLightStyles();

	CG_LoadingString( "" );

	CG_ShaderStateChanged();

	trap_S_ClearLoopingSounds( qtrue );

	// In a mission game we want to bring up the objective screen on initial connection
	if ( cgs.gametypeData->description || cgs.pickupsDisabled || cgs.gametypeData->teams  )
	{
		cg.popupObjectives = qtrue;
	}

	trap_Cvar_Set ( "ui_info_gametype", va("%i",cgs.gametype ) );
	trap_Cvar_Set ( "ui_info_objectives", cgs.gametypeData->description );

	// remove the last loading update
	cg.infoScreenText[0] = 0;

	cg.loading = qfalse;	// future players will be deferred
}

void CG_ShutdownG2Models(void)
{
	int				i, j;
	clientInfo_t	*ci;

	for(i=0;i<MAX_ITEMS;i++)
	{
		for(j=0;j<MAX_ITEM_MODELS;j++)
		{
			if (cg_items[i].g2Models[j])
			{
				trap_G2API_CleanGhoul2Models(&cg_items[i].g2Models[j]);
			}
		}
	}

	for(i=0;i<MAX_GENTITIES;i++)
	{
		if (cg_entities[i].ghoul2)
		{
			trap_G2API_CleanGhoul2Models(&cg_entities[i].ghoul2);
		}
	}

	for(i=0;i<MAX_CLIENTS;i++)
	{
		ci = &cgs.clientinfo[i];
		if (ci->ghoul2Model)
		{
			trap_G2API_CleanGhoul2Models(&ci->ghoul2Model);
		}
	}
}

void CG_ShutdownGore(void);

/*
=================
CG_Shutdown

Called before every level change or subsystem restart
=================
*/
void CG_Shutdown( void ) 
{
	trap_FX_FreeSystem();

	CG_ShutDownWeapons();
	CG_ShutdownG2Models();

	CG_ShutdownGore();

	// some mods may need to do cleanup work here,
	// like closing files or archiving session data
}
