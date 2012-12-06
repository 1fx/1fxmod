// Copyright (C) 2001-2002 Raven Software.
//
// cg_local.h --

#include "../game/q_shared.h"
#include "tr_types.h"
#include "../game/bg_public.h"
#include "cg_public.h"
#include "../ghoul2/g2.h"
#include "../ghoul2/G2_gore_shared.h"
#include "../game/inv.h"

#undef _SNAPSHOT_EXTRAPOLATION

// The entire cgame module is unloaded and reloaded on each level change,
// so there is NO persistant data between levels on the client side.
// If you absolutely need something stored, it can either be kept
// by the server in the server stored userinfos, or stashed in a cvar.

#define	FADE_TIME					200
#define	DAMAGE_DEFLECT_TIME			100
#define	DAMAGE_RETURN_TIME			400
#define DAMAGE_TIME					500
#define	LAND_DEFLECT_TIME			150
#define	LAND_RETURN_TIME			300
#define	STEP_TIME					200
#define	DUCK_TIME					100
#define	PAIN_TWITCH_TIME			200
									
// Zoom vars						
#define MAX_ZOOM_FOV				3.0f
#define ZOOM_OUT_TIME				100.0f

#define	MUZZLE_FLASH_TIME			20
#define	MUZZLE_FLASH_EFFECT_TIME	250
#define	SINK_TIME					1000		// time for fragments to sink into ground before going away

#define NUMFLYBYS					4
									
#define	MAX_STEP_CHANGE				32
									
#define	CHAR_WIDTH					32
#define	CHAR_HEIGHT					48
									
#define	CHAT_WIDTH					80
#define CHAT_HEIGHT					8

#define	NUM_CROSSHAIRS				5

#define	DEFAULT_IDENTITY			"mullinsjungle"

#define MAX_SOUNDBUFFER				20


#define	FX_DEPTH_HACK				0x00100000

typedef void*	TGhoul2;

/*
=================
player entities need to track more information
than any other type of entity.

note that not every player entity is a client entity,
because corpses after respawn are outside the normal
client numbering range

when changing animation, set animationTime to frameTime + lerping time
The current lerp will finish out, then it will lerp to the new animation
=================
*/
typedef struct playerEntity_s
{
	animInfo_t		torso;
	animInfo_t		legs;

	int				painTime;
	int				painDirection;		// flip from 0 to 1
	int				lightningFiring;

	int				weapon;
	int				weaponModelSpot;

	int				spawnCount;			// used to track spawn transitions

	vec3_t			ghoulLegsAngles;
	vec3_t			ghoulLowerTorsoAngles;
	vec3_t			ghoulUpperTorsoAngles;
	vec3_t			ghoulHeadAngles;

} playerEntity_t;

/*
=================
centity_t have a direct corespondence with gentity_t in the game, but
only the entityState_t is directly communicated to the cgame
=================
*/
typedef struct centity_s 
{
	entityState_t	currentState;		// from cg.frame
	entityState_t	nextState;			// from cg.nextFrame, if available
	qboolean		interpolate;		// true if next is valid to interpolate to
	qboolean		currentValid;		// true if cg.frame holds this entity

	int				muzzleFlashTime;	// move to playerEntity?
	attackType_t	muzzleFlashAttack;	// Which attack mode the muzzle flash was for
	int				previousEvent;
	int				teleportFlag;

	int				trailTime;			// so missile trails can handle dropped initial packets
	int				dustTrailTime;		
	int				miscTime;			
										
	int				snapShotTime;		// last time this entity was found in a snapshot
										
	playerEntity_t	pe;					
										
	int				errorTime;			// decay the error from this time
	vec3_t			errorOrigin;		
	vec3_t			errorAngles;		
										
	qboolean		extrapolated;		// false if origin / angles is an interpolation
	vec3_t			rawOrigin;
	vec3_t			rawAngles;

	vec3_t			beamEnd;

	// exact interpolated position of entity on this frame
	vec3_t			lerpOrigin;
	vec3_t			lerpAngles;
	vec3_t			lerpVelocity;
	float			lerpLeanOffset;

	int				ambientSetTime;

	void			*ghoul2;
	vec3_t			modelScale;
	float			radius;
	int				boltInfo;

	CFxBoltInterface	flashBoltInterface;
	CFxBoltInterface	ejectBoltInterface;

} centity_t;


typedef enum 
{
	LE_FRAGMENT,
	LE_MOVE_SCALE_FADE,
	LE_FALL_SCALE_FADE,
	LE_FADE_RGB,
	LE_SCALE_FADE,
	LE_SHOWREFENTITY,
	LE_LINE,
	LE_GIB

} leType_t;

typedef enum 
{
	LEF_PUFF_DONT_SCALE = 0x0001,			// do not scale size over time
	LEF_TUMBLE			= 0x0002,			// tumble over time, used for ejecting shells
	LEF_FADE_RGB		= 0x0004,			// explicitly fade
	LEF_NO_RANDOM_ROTATE= 0x0008			// MakeExplosion adds random rotate which could be bad in some cases

} leFlag_t;

typedef enum 
{
	LEMT_NONE,
	LEMT_BURN,
	LEMT_BLOOD

} leMarkType_t;			// fragment local entities can leave marks on walls

typedef enum 
{
	LEBS_NONE,
	LEBS_BRASS

} leBounceSoundType_t;	// fragment local entities can make sounds on impacts

typedef struct localEntity_s 
{
	struct localEntity_s*	prev;
	struct localEntity_s*	next;

	leType_t				leType;
	int						leFlags;
							
	int						startTime;
	int						endTime;
	int						fadeInTime;
							
	float					lifeRate;			// 1.0 / (endTime - startTime)
							
	trajectory_t			pos;
	trajectory_t			angles;
							
	float					bounceFactor;		// 0.0 = no bounce, 1.0 = perfect
							
	float					alpha;
	float					dalpha;
							
	float					color[4];
							
	float					radius;
							
	float					light;
	vec3_t					lightColor;

	leMarkType_t			leMarkType;			// mark to leave on fragment impact
	leBounceSoundType_t		leBounceSoundType;
							
	float					zOffset;
							
	refEntity_t				refEntity;		

} localEntity_t;

//======================================================================


typedef struct 
{
	int			client;
	int			score;
	int			kills;
	int			deaths;
	int			ping;
	int			time;
	int			scoreFlags;
	int			team;
	int			teamkillDamage;	// damage inflicted to teammates (0-100)

} score_t;

// each client has an associated clientInfo_t
// that contains media references necessary to present the
// client model and other color coded effects
// this is regenerated each time a client's configstring changes,
// usually as a result of a userinfo (name, model, etc) change
typedef enum
{
	SOUND_DIE_1 = 0,
	SOUND_DIE_2,
	SOUND_DIE_3,
	SOUND_PAIN_1,
	SOUND_PAIN_2,
	SOUND_PAIN_3,

	MAX_CUSTOM_SOUNDS

} ECustomSounds;

typedef struct 
{
	qboolean		infoValid;
	qboolean		ghost;

	char			name[MAX_QPATH];
	team_t			team;

	int				botSkill;		// 0 = not bot, 1-5 = bot

	vec3_t			color1;
	vec3_t			color2;

	int				score;			// updated by score servercmds
	int				location;		// location index for team mode
	int				health;			// you only get this info about your teammates
	int				armor;

	int				handicap;
	int				wins;
	int				losses;	

	int				mLastChatTime;

	int				teamTask;		// task in teamplay (offence/defence)
	qboolean		teamLeader;		// true when this is a team leader

	int				gametypeitems;	// gametype items being carried

	// when clientinfo is changed, the loading of models/skins/sounds
	// can be deferred until you are dead, to prevent hitches in
	// gameplay
	char			identityName[MAX_QPATH];
	TIdentity*		identity;
	qboolean		deferred;
	qboolean		isMale;

	gender_t		gender;			// from model

	void			*ghoul2Model;
	int				boltWorldWeapon;						// bolt to use to hang world model of weapon on.
	int				boltGametypeItems[MAX_GAMETYPE_ITEMS];	// bolt to use to hang gametype items to the player
	int				boltNightvision;
	
	animation_t		animations[MAX_ANIMATIONS];
	sfxHandle_t		sounds[MAX_CUSTOM_SOUNDS];

} clientInfo_t;

// Each weapon can have multiple attacsk.  SOF2 by default has 2 attacks 
// per weapon, the standard attack and an alternate attack.
typedef struct weaponAttack_s
{
	qhandle_t		ammoIcon;

	sfxHandle_t		flashSound[4];			// fast firing weapons randomly choose
	sfxHandle_t		firingSound;
	sfxHandle_t		missileSound;
	float			missileDlight;
	vec3_t			missileDlightColor;
	sfxHandle_t		missileHitSound;
	int				shellEjectBoltView;		// bolt for shell eject fx 
	int				muzzleFlashBoltView;	// bolt for flash fx
	int				shellEjectBoltWorld;	// bolt for shell eject fx 
	int				muzzleFlashBoltWorld;	// bolt for flash fx
	fxHandle_t		muzzleEffect;			// muzzle flash effect in view
	fxHandle_t		muzzleEffectInWorld;
	fxHandle_t		tracerEffect;			// used for either a bullet tracer OR for a projectile trail effect
	fxHandle_t		shellEject;				// shell eject effect
	sfxHandle_t		explosionSound;			// used for when a projectile explodes
	fxHandle_t		explosionEffect;
	void 			*missileG2Model;		// ghoul2 model for missle

	void			(*missileTrailFunc)( centity_t *, int weaponNum );

} attackInfo_t;

// each WP_* weapon enum has an associated weaponInfo_t
// that contains media references necessary to present the
// weapon and its effects
typedef struct weaponInfo_s 
{
	qboolean		registered;
	gitem_t			*item;

	void			*weaponG2Model;		// ghoul2 model for 3rd person and in game world
	void			*viewG2Model;		// ghoul2 model for 1st person view
	int				viewG2Indexes[8];	// indexes for models in viewG2Model because ghoul2 is whacky!

	vec3_t			weaponMidpoint;		// so it will rotate centered instead of by tag

	qhandle_t		weaponIcon;

	void			*ammoG2Model;	 	// ghoul2 model for ammo

	attackInfo_t	attack[ATTACK_MAX];

	// All
	sfxHandle_t		otherWeaponSounds[MAX_WEAPON_SOUNDS][MAX_WEAPON_SOUND_SLOTS];

	int				goreStyle;

} weaponInfo_t;


// each IT_* item has an associated itemInfo_t
// that constains media references necessary to present the
// item and its effects
typedef struct 
{
	qboolean		registered;
	qhandle_t		models[MAX_ITEM_MODELS];
	qhandle_t		icon, mSimpleIcon;

	void			*g2Models[MAX_ITEM_MODELS];
	float			radius[MAX_ITEM_MODELS];

	void*			boltModel;

} itemInfo_t;


//======================================================================

// all cg.stepTime, cg.duckTime, cg.landTime, etc are set to cg.time when the action
// occurs, and they will have visible effects for #define STEP_TIME or whatever msec after

#define MAX_PREDICTED_EVENTS	16
 
typedef struct 
{
	int				clientFrame;			// incremented each frame
					
	int				clientNum;
					
	qboolean		demoPlayback;
	qboolean		levelShot;				// taking a level menu screenshot
	int				deferredPlayerLoading;
	qboolean		loading;				// don't defer players at initial startup
	qboolean		intermissionStarted;	// don't play voice rewards, because game will end shortly

	// there are only one or two snapshot_t that are relevent at a time
	int				latestSnapshotNum;		// the number of snapshots the client system has received
	int				latestSnapshotTime;		// the time from latestSnapshotNum, so we don't need to read the snapshot yet
											
	snapshot_t		*snap;					// cg.snap->serverTime <= cg.time
	snapshot_t		*nextSnap;				// cg.nextSnap->serverTime > cg.time, or NULL
	qboolean		needNextSnap;
//	snapshot_t		activeSnapshots[2];

	float			frameInterpolation;		// (float)( cg.time - cg.frame->serverTime ) / (cg.nextFrame->serverTime - cg.frame->serverTime)
					
	qboolean		mMapChange;				// true if map is changing
					
	qboolean		thisFrameTeleport;
	qboolean		nextFrameTeleport;
					
	int				frametime;				// cg.time - cg.oldTime
					
	int				time;					// this is the time value that the client
											// is rendering at.
	int				oldTime;				// time at last frame, used for missile trails and prediction checking
											
	int				physicsTime;			// either cg.snap->time or cg.nextSnap->time
														
	qboolean		mapRestart;				// set on a map restart to set back the weapon
	qboolean		gametypeStarted;		// has the gametype started yet?
	qboolean		mInRMG;
					
	qboolean		renderingThirdPerson;	// during deaths, chasecams, etc

	// prediction state
	qboolean		hyperspace;				// true if prediction has hit a trigger_teleport
	playerState_t	predictedPlayerState;
	qboolean		validPPS;				// clear until the first call to CG_PredictPlayerState
	int				predictedErrorTime;
	vec3_t			predictedError;
					
	int				eventSequence;
	int				predictableEvents[MAX_PREDICTED_EVENTS];
					
	float			stepChange;				// for stair up smoothing
	int				stepTime;
					
	float			duckChange;				// for duck viewheight smoothing
	int				duckTime;
					
	float			landChange;				// for landing hard
	int				landTime;
					
	int				deathTime;				// time of death

	// input state sent to server
	int				weaponSelect;
	int				weaponLastSelect;
	int				weaponOldSelect;
	int				weaponMenuSelect;
					
	qboolean		weaponMenuUp;			// weapon menu shown

	// auto rotating items
	vec3_t			autoAngles;
	vec3_t			autoAxis[3];
	vec3_t			autoAnglesFast;
	vec3_t			autoAxisFast[3];

	// view rendering
	refdef_t		refdef;
					
	// zoom key		
	int				zoomTime;
	float			zoomSensitivity;

	// information screen text during loading
	char			infoScreenText[MAX_STRING_CHARS];

	// scoreboard
	int				scoresRequestTime;
	int				numScores;
	int				teamScores[2];
	score_t			scores[MAX_CLIENTS];
	qboolean		showScores;
	qboolean		scoreBoardShowing;							// Whether or not the scoreboard is showing
	float			scoreBoardBottom;							// Bottom coordinate of the scoreboard
	char			scoreBoardSpectators[1024];					// spectators string
	int				scoreFadeTime;
	qboolean		showAutomap;
	char			killerName[MAX_NAME_LENGTH];
	int				spectatorTime;								// next time to offset

	// centerprinting
	int				centerPrintTime;
	char			centerPrint[1024];
	int				centerPrintLines;
	float			centerPrintScale;

	// kill timers for carnage reward
	int				lastKillTime;

	// crosshair client ID
	int				crosshairClientNum;
	int				crosshairClientTime;
	int				crosshairColorClientNum;

	vec4_t			crosshairRGBA;
	vec4_t			crosshairFriendRGBA;

	// attacking player
	int				attackerTime;
	int				voiceTime;

	// sound buffer mainly for announcer sounds
	int				soundBufferIn;
	int				soundBufferOut;
	int				soundTime;
	qhandle_t		soundBuffer[MAX_SOUNDBUFFER];

	// for voice chat buffer
	int				voiceChatTime;
	int				voiceChatBufferIn;
	int				voiceChatBufferOut;

	// warmup countdown
	int				warmup;
	int				warmupCount;

	//==========================

	int				itemPickup;

	int				weaponSelectTime;
	int				weaponAnimation;
	int				weaponAnimationTime;

	// blend blobs
	float			damageTime;
	float			damageX;
	float			damageY;

	// view movement
	float			v_dmg_pitch;
	float			v_dmg_roll;

	// temp working variables for player view
	float			bobfracsin;
	int				bobcycle;
	float			xyspeed;
	int				nextOrbitTime;

	int				loadStage;

	// development tool
	refEntity_t		testModelEntity;
	char			testModelName[MAX_QPATH];
	qboolean		testGun;
	int				testModel;

	// had to be moved so we wouldn't wipe these out with the memset
	snapshot_t		activeSnapshots[3];
	int				activeSnapshot;

	TAnimInfoWeapon*	viewWeaponAnim[5];

	int					weaponHideModels;

	CFxBoltInterface	flashBoltInterface;

	qboolean			cheats;

	qboolean			popupObjectives;

	float				shakeIntensity;
	int					shakeDuration;
	int					shakeStart;

	char				sharedBuffer[MAX_CG_SHARED_BUFFER_SIZE];
	
	centity_t*			radarEntities[MAX_GAMETYPE_ITEMS+MAX_CLIENTS];
	int					radarEntityCount;

	int					flashbangTime;
	int					flashbangFadeTime;
	float				flashbangAlpha;

	animation_t			hitAnimations[MAX_ANIMATIONS];
	TGhoul2				hitModel;

} cg_t;

extern centity_t			cg_entities[MAX_GENTITIES];
extern centity_t			*cg_permanents[MAX_GENTITIES];
extern int					cg_numpermanents;


// all of the model, shader, and sound references that are
// loaded at gamestate time are stored in cgMedia_t
// Other media that can be tied to clients, weapons, or items are
// stored in the clientInfo_t, itemInfo_t, weaponInfo_t
typedef struct 
{
	qhandle_t	whiteShader;

	qhandle_t	armorShader;

	qhandle_t	blueFriendShader;
	qhandle_t	redFriendShader;
	qhandle_t	deadShader;

	qhandle_t	radarShader;

	qhandle_t	botSmallShader;

	qhandle_t	crosshairShader[NUM_CROSSHAIRS];
	qhandle_t	lagometerShader;
	qhandle_t	disconnectShader;
	qhandle_t	backTileShader;

	qhandle_t	loadClipShader;
	qhandle_t	loadBulletShader;

	qhandle_t	smokePuffShader;
	qhandle_t	waterBubbleShader;

	qhandle_t	shadowMarkShader;

	// wall mark shaders
	qhandle_t	wakeMarkShader;
	qhandle_t	bloodMarkShader;
	qhandle_t	burnMarkShader;

	// scoreboard headers
	qhandle_t	scoreboard;
	qhandle_t	scoreboardHeader;
	qhandle_t	scoreboardLine;
	qhandle_t	scoreboardFooter;
	qhandle_t	scoreboardTotals;

	// sounds
	sfxHandle_t talkSound;

	sfxHandle_t	waterLeave;
	sfxHandle_t waterJumpIn;
	sfxHandle_t	waterFootstep[2];
	sfxHandle_t	waterWade[2];

	sfxHandle_t	armorHitSound[2];
	sfxHandle_t fleshHitSound[2];

	qhandle_t	cursor;

	// Fonts
	qhandle_t	lcdFont;
	qhandle_t	hudFont;

	qhandle_t	glassBreakSound;
	qhandle_t	respawnSound;
	qhandle_t	itemRespawnSound;
	qhandle_t	fragSound;
	qhandle_t	fragSelfSound;

	qhandle_t	goSound;

	qhandle_t	zoomSound;
	qhandle_t	gogglesOnSound;
	qhandle_t	gogglesOffSound;

	qhandle_t	flybySounds[NUMFLYBYS];

	qhandle_t	drownPainSound[2];
	qhandle_t	drownDeathSound;

	qhandle_t	damageDirShader;
	qhandle_t	playerFleshImpactEffect;
	qhandle_t	glassChunkEffect;

	qhandle_t	mAutomap;
	qhandle_t	mAutomapPlayerIcon;

	TGhoul2		nightVisionModel;

	qhandle_t	mBloodSmall;

	qhandle_t	test;

} cgMedia_t;


// The client game static (cgs) structure hold everything
// loaded or calculated from the gamestate.  It will NOT
// be cleared when a map_restart is done
typedef struct 
{
	gameState_t		gameState;			// gamestate from server
	glconfig_t		glconfig;			// rendering configuration
	float			screenXScale;		// derived from glconfig
	float			screenYScale;
	float			screenXBias;

	int				gameID;

	int				serverCommandSequence;	// reliable command stream counter
	int				processedSnapshotNum;// the number of snapshots cgame has requested

	qboolean		localServer;		// detected on startup by checking sv_running

	// parsed from serverinfo
	int				gametype;
	gametypeData_t*	gametypeData;
	int				dmflags;
	int				teamflags;
	int				scorelimit;
	int				timelimit;
	int				maxclients;
	qboolean		friendlyFire;
	char			mapname[MAX_QPATH];
	char			gameover[MAX_QPATH];

	qboolean		pickupsDisabled;

	int				voteTime;
	int				voteDuration;
	int				voteYes;
	int				voteNeeded;
	int				voteNo;
	qboolean		voteModified;			// beep whenever changed
	char			voteString[MAX_STRING_TOKENS];

	int				levelStartTime;
	int				gametypeTimerTime;
	int				gametypeMessageTime;
	char			gametypeMessage[MAX_STRING_TOKENS];

	int				scores1;
	int				scores2;

	// locally derived information from gamestate
	qhandle_t		gameModels[MAX_MODELS];
	sfxHandle_t		gameSounds[MAX_SOUNDS];
	qhandle_t		gameIcons[MAX_ICONS];
	qhandle_t		skins[MAX_CHARSKINS];

	int				numInlineModels;
	qhandle_t		inlineDrawModel[MAX_MODELS];
	vec3_t			inlineModelMidpoints[MAX_MODELS];

	clientInfo_t	clientinfo[MAX_CLIENTS];

	// teamchat width is *3 because of embedded color codes
	char			chatText[CHAT_HEIGHT][CHAT_WIDTH*3+1];
	int				chatTime[CHAT_HEIGHT];
	int				chatPos;
	int				chatLastPos;

	int				cursorX;
	int				cursorY;
	qboolean		eventHandling;
	void*			capturedItem;
	qhandle_t		activeCursor;

	// media
	cgMedia_t		media;

	vec3_t			mWorldMins;
	vec3_t			mWorldMaxs;
	float			mIRDist;
	float			mIRSeeThrough;

} cgs_t;

//==============================================================================

extern	cgs_t			cgs;
extern	cg_t			cg;
extern	weaponInfo_t	cg_weapons[MAX_WEAPONS];
extern	itemInfo_t		cg_items[MAX_ITEMS];

extern	vmCvar_t		con_notifyTime;
extern	vmCvar_t		cg_centertime;
extern	vmCvar_t		cg_centerY;
extern	vmCvar_t		cg_runpitch;
extern	vmCvar_t		cg_runroll;
extern	vmCvar_t		cg_bobup;
extern	vmCvar_t		cg_bobpitch;
extern	vmCvar_t		cg_bobroll;
extern	vmCvar_t		cg_shadows;
extern	vmCvar_t		cg_drawTimer;
extern	vmCvar_t		cg_drawFPS;
extern	vmCvar_t		cg_drawSnapshot;
extern	vmCvar_t		cg_drawCrosshair;
extern	vmCvar_t		cg_drawCrosshairNames;
extern  vmCvar_t		cg_drawRadar;
extern	vmCvar_t		cg_drawTeamScores;
extern	vmCvar_t		cg_crosshairX;
extern	vmCvar_t		cg_crosshairY;
extern	vmCvar_t		cg_crosshairSize;
extern	vmCvar_t		cg_crosshairRGBA;
extern	vmCvar_t		cg_crosshairFriendRGBA;
extern	vmCvar_t		cg_draw2D;
extern	vmCvar_t		cg_debugEvents;
extern	vmCvar_t		cg_errorDecay;
extern	vmCvar_t		cg_nopredict;
extern	vmCvar_t		cg_showmiss;
extern	vmCvar_t		cg_footsteps;
extern	vmCvar_t		cg_gun_x;
extern	vmCvar_t		cg_gun_y;
extern	vmCvar_t		cg_gun_z;
extern	vmCvar_t		cg_drawGun;
extern	vmCvar_t		cg_autoswitch;
extern	vmCvar_t		cg_ignore;
extern	vmCvar_t		cg_simpleItems;
extern	vmCvar_t		cg_fov;
extern	vmCvar_t		cg_shellEjection;

extern	vmCvar_t		cg_goreDetail;

extern	vmCvar_t		cg_thirdPerson;
extern	vmCvar_t		cg_thirdPersonRange;
extern	vmCvar_t		cg_thirdPersonYaw;
extern	vmCvar_t		cg_thirdPersonPitch;

extern	vmCvar_t		cg_thirdPersonHorzOffset;

extern	vmCvar_t		cg_stereoSeparation;
extern	vmCvar_t		cg_lagometer;
extern	vmCvar_t		cg_synchronousClients;
extern	vmCvar_t		cg_chatTime;
extern	vmCvar_t		cg_chatHeight;
extern	vmCvar_t		cg_stats;
extern	vmCvar_t 		cg_forceModel;
extern	vmCvar_t 		cg_buildScript;
extern	vmCvar_t		cg_paused;
extern	vmCvar_t		cg_predictItems;
extern	vmCvar_t		cg_antiLag;
extern	vmCvar_t		cg_impactPrediction;
extern	vmCvar_t		cg_autoReload;
extern	vmCvar_t		cg_deferPlayers;
extern	vmCvar_t		cg_drawFriend;
extern	vmCvar_t		cg_teamChatsOnly;
extern	vmCvar_t		cg_noVoiceText;
extern	vmCvar_t		cg_smoothClients;
extern	vmCvar_t		pmove_fixed;
extern	vmCvar_t		pmove_msec;
extern	vmCvar_t		cg_cameraOrbit;
extern	vmCvar_t		cg_cameraOrbitDelay;
extern	vmCvar_t		cg_timescale;
extern	vmCvar_t		cg_cameraMode;
extern	vmCvar_t		cg_noTaunt;
extern	vmCvar_t		cg_noProjectileTrail;
extern	vmCvar_t		cg_currentSelectedPlayer;
extern	vmCvar_t		cg_currentSelectedPlayerName;

extern	vmCvar_t		cg_DebugGore;

extern	vmCvar_t		cg_lockSever;
extern	vmCvar_t		cg_lockBlood;
extern	vmCvar_t		cg_lockDeaths;

extern	vmCvar_t		RMG_distancecull;
extern	vmCvar_t		cg_damageindicator;
extern	vmCvar_t		cg_tracerChance;

extern	vmCvar_t		cg_animBlend;

extern	vmCvar_t		cg_automap_x;
extern	vmCvar_t		cg_automap_y;
extern	vmCvar_t		cg_automap_w;
extern	vmCvar_t		cg_automap_h;
extern	vmCvar_t		cg_automap_a;

extern	vmCvar_t		ui_info_redcount;
extern	vmCvar_t		ui_info_bluecount;
extern	vmCvar_t		ui_info_speccount;
extern	vmCvar_t		ui_info_freecount;
extern  vmCvar_t		ui_info_pickupsdisabled;
extern	vmCvar_t		ui_info_seenobjectives;

extern	vmCvar_t		cg_voiceRadio;
extern	vmCvar_t		cg_voiceGlobal;
extern	vmCvar_t		cg_soundGlobal;
extern	vmCvar_t		cg_soundFrag;
extern	vmCvar_t		cg_weaponMenuFast;

extern	vmCvar_t		cg_bodyTime;

extern	vmCvar_t		rw_enabled;

//
// cg_main.c
//
const char *CG_ConfigString					( int index );
const char *CG_Argv							( int arg );
centity_t*	CG_GetEntity					( int index );
void		CG_StartMusic					( qboolean bForceStart );
void		CG_UpdateCvars					( void );
void		CG_UpdateTeamCountCvars			( void );
int			CG_CrosshairPlayer				( void );
int			CG_LastAttacker					( void );
void		CG_LoadMenus					( const char *menuFile);
void		CG_KeyEvent						( int key, qboolean down);
void		CG_MouseEvent					( int x, int y);
void		CG_EventHandling				( int type);
											
//											
// cg_view.c								
//											
void		CG_TestModel_f					(void);
void		CG_TestGun_f					(void);
void		CG_TestModelNextFrame_f			(void);
void		CG_TestModelPrevFrame_f			(void);
void		CG_TestModelNextSkin_f			(void);
void		CG_TestModelPrevSkin_f			(void);
void		CG_AddBufferedSound				( sfxHandle_t sfx);
void		CG_DrawActiveFrame				( int serverTime, stereoFrame_t stereoView, qboolean demoPlayback );
void		CG_CameraShake					( float* origin, float intensity, int radius, int time );
void		CG_UpdateCameraShake			( vec3_t origin, vec3_t angles );		
											
//											
// cg_drawtools.c							
//											
void		CG_AdjustFrom640				( float *x, float *y, float *w, float *h );
void		CG_FillRect						( float x, float y, float width, float height, const float *color );
void		CG_DrawPic						( float x, float y, float width, float height, qhandle_t hShader );
void		CG_DrawStretchPic				( float x, float y, float width, float height, float sx, float sy, float sw, float sh, const float* color, qhandle_t hShader );
void		CG_DrawRotatePic				( float x, float y, float width, float height,float angle, qhandle_t hShader );
void		CG_DrawRotatePic2				( float x, float y, float width, float height,float angle, qhandle_t hShader );
int			CG_DrawStrlen					( const char *str );
float*		CG_FadeColor					( int startMsec, int totalMsec );
float*		CG_TeamColor					( int team );
void		CG_TileClear					( void );
void		CG_DrawRect						( float x, float y, float width, float height, float size, const float *color );
void		CG_DrawSides					( float x, float y, float w, float h, float size);
void		CG_DrawTopBottom				( float x, float y, float w, float h, float size);
void		CG_GetColorForHealth			( vec4_t color, int health, int armor );
void		CG_DrawText						( float x, float y, qhandle_t font, float scale, vec4_t color, const char* text, int limit, int flags );
void		CG_DrawTextWithCursor			( float x, float y, qhandle_t font, float scale, vec4_t color, const char* text, int limit, int flags, int cursorPos, char cursor );
void		CG_DrawTimer					( float x, float y, qhandle_t font, float scale, vec4_t color, int flags, int msec );

//
// cg_draw.c, cg_newDraw.c
//
void		CG_AddLagometerFrameInfo		( void );
void		CG_AddLagometerSnapshotInfo		( snapshot_t *snap );
void		CG_CenterPrint					( const char *str, float scale );
void		CG_DrawActive					( stereoFrame_t stereoView );
void		CG_OwnerDraw					( float x, float y, float w, float h, float text_x, float text_y, int ownerDraw, int ownerDrawFlags, int align, float special, qhandle_t font, float scale, vec4_t color, qhandle_t shader, int textStyle, const char* param );
float		CG_GetValue						( int ownerDraw );
qboolean	CG_OwnerDrawVisible				( int flags, const char* param );
qboolean	CG_OwnerDrawDisabled			( int flags, const char* param );
void		CG_RunMenuScript				( const char **args);
void		CG_GetTeamColor					( vec4_t *color);
const char*	CG_GetGameStatusText			( void );
void		CG_Draw3DModel					( float x, float y, float w, float h, qhandle_t model, qhandle_t skin, vec3_t origin, vec3_t angles );
void		CG_Draw3DG2Model				( float x, float y, float w, float h, void *ghoul2, qhandle_t skin, vec3_t origin, vec3_t angles ); 
void		CG_Text_PaintChar				( float x, float y, float width, float height, float scale, float s, float t, float s2, float t2, qhandle_t hShader );
const char*	CG_GameTypeString				( void );
void		CG_DrawMapChange				( void );

//
// cg_player.c
//
void		CG_Player						( centity_t *cent );
void		CG_ResetPlayerEntity			( centity_t *cent );
void		CG_NewClientInfo				( int clientNum );
sfxHandle_t	CG_CustomSound					( int clientNum, const char *soundName);
sfxHandle_t	CG_CustomPlayerSound			( int clientNum, ECustomSounds sound); 
void		CG_UpdatePlayerModel			( centity_t* cent);
TGhoul2		CG_RegisterIdentity				( TIdentity* identity, char *animationFile, gender_t* gender );
void		CG_RemoveIdentityItemsOnBack	( centity_t* cent );
qboolean	CG_PlayerShadow					( centity_t *cent, float *shadowPlane );

//
// cg_predict.c
//
void		CG_BuildSolidList				( void );
int			CG_PointContents				( const vec3_t point, int passEntityNum );
void		CG_Trace						( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int skipNumber, int mask );
void		CG_PlayerTrace					( trace_t* tr, const vec3_t start, const vec3_t end, int skipNumber );
void		CG_PredictPlayerState			( void );
void		CG_LoadDeferredPlayers			( void );
void		CG_InitHitModel					( void );

//
// cg_events.c
//
void		CG_CheckEvents					( centity_t *cent );
const char*	CG_PlaceString					( int rank );
void		CG_EntityEvent					( centity_t *cent, vec3_t position );
void		CG_PainEvent					( centity_t *cent, int health );

//
// cg_ents.c
//
float*		CG_SetEntitySoundPosition		( centity_t *cent );
void		CG_AddPacketEntities			( void );
void		CG_Beam							( centity_t *cent );
void		CG_AdjustPositionForMover		( const vec3_t in, int moverNum, int fromTime, int toTime, vec3_t out );
void		CG_ScaleModelAxis				( refEntity_t* ent );

//
// cg_weapons.c
//
void		CG_NextWeapon					( qboolean allowEmpty, int exclude );
void		CG_PrevWeapon					( qboolean allowEmpty, int exclude );
void		CG_Weapon_f						( void );
qboolean	CG_WeaponSelectable				( int i, qboolean allowEmpty );
void		CG_RegisterWeapon				( int weaponNum);
void		CG_ShutDownWeapons				( void);
void		CG_RegisterItemVisuals			( int itemNum );
void		CG_UpdateViewWeaponSurfaces		( playerState_t* ps );
void		CG_SetWeaponAnim				( int weaponAnim, playerState_t *ps );
void		CG_FireWeapon					( centity_t *cent, attackType_t attack );
void		CG_ProjectileThink				( centity_t *cent, int weaponNum );
void		CG_MissileHitWall				( int weapon, vec3_t origin, vec3_t dir, int material, attackType_t attack );
void		CG_MissileHitPlayer				( int weapon, vec3_t origin, vec3_t dir, int entityNum, attackType_t attack );
void		CG_HandleStickyMissile			( centity_t *cent,entityState_t *es,vec3_t dir,int targetEnt);
void		CG_Bullet						( vec3_t end, int sourceEntityNum, int weapon, vec3_t normal, int fleshEntityNum, int material, attackType_t attack );
void		CG_AnimateViewWeapon			( playerState_t *ps );
void		CG_AddViewWeapon				( playerState_t *ps );
void		CG_PlayerWeaponEffects			( refEntity_t *parent, centity_t *cent, int team, vec3_t newAngles );
void		CG_DrawWeaponSelect				( void );
void		CG_OutOfAmmoChange				( int lastWeapon );	
void		CG_WeaponCallback				( playerState_t* ps, centity_t* cent, int weapon, int anim, int animChoice, int step );

//
// cg_localents.c
//
void			CG_InitLocalEntities		( void );
void			CG_FreeLocalEntity			( localEntity_t *le );
localEntity_t*	CG_AllocLocalEntity			( void );
void			CG_AddLocalEntities			( void );

//
// cg_effects.c
//
void		CG_BubbleTrail					( vec3_t start, vec3_t end, float spacing );
void		CG_BulletFlyBySound				( vec3_t start, vec3_t end );
void		CG_GlassShatter					( int entnum, vec3_t org, vec3_t mins, vec3_t maxs);
void		CG_TestLine						( vec3_t start, vec3_t end, int time, unsigned int color, int radius);

//
// cg_snapshot.c
//
void		CG_ProcessSnapshots				( void );

//
// cg_info.c
//
void		CG_LoadingString				( const char *s );
void		CG_LoadingStage					( int stage );
void		CG_LoadingItem					( int itemNum );
void		CG_LoadingClient				( int clientNum );
void		CG_DrawInformation				( void );
													
//													
// cg_scoreboard.c									
//												
qboolean	CG_DrawScoreboard				( void );
													
//													
// cg_consolecmds.c									
//													
qboolean	CG_ConsoleCommand				( void );
void		CG_InitConsoleCommands			( void );
													
//													
// cg_servercmds.c									
//													
void		CG_MapRestart					( qboolean gametypeRestart );
void		CG_ExecuteNewServerCommands		( int latestSequence );
void		CG_ParseServerinfo				( void );
void		CG_SetConfigValues				( void );
void		CG_LoadVoiceChats				( void );
void		CG_ShaderStateChanged			( void );
void		CG_VoiceChatLocal				( qboolean voiceOnly, int clientNum, const char* chatprefix, const char *cmd );
void		CG_PlayBufferedVoiceChats		( void );

//
// cg_playerstate.c
//
void		CG_Respawn							( void );
void		CG_TransitionPlayerState			( playerState_t *ps, playerState_t *ops );
void		CG_CheckChangedPredictableEvents	( playerState_t *ps );

//
// cg_playeranim.c
//
qboolean	CG_ParseAnimationFile	( const char *filename, clientInfo_t *ci) ;

//
// cg_gore.c
//
qboolean	CG_ParseGore	( void );
void		CG_ApplyGore	( int clientNum, centity_t *cent, int hitLocation, vec3_t Direction);


//===============================================

//
// system traps
// These functions are how the cgame communicates with the main game system
//
// print message on the local console
void		trap_Print						( const char *fmt );

// abort the game
void		trap_Error						( const char *fmt );

// milliseconds should only be used for performance tuning, never
// for anything game related.  Get time from the CG_DrawActiveFrame parameter
int			trap_Milliseconds				( void );

// console variable interaction
void		trap_Cvar_Register				( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags, float MinValue, float MaxValue );
void		trap_Cvar_Update				( vmCvar_t *vmCvar );
void		trap_Cvar_Set					( const char *var_name, const char *value );
void		trap_Cvar_VariableStringBuffer	( const char *var_name, char *buffer, int bufsize );

// ServerCommand and ConsoleCommand parameter access
int			trap_Argc						( void );
void		trap_Argv						( int n, char *buffer, int bufferLength );
void		trap_Args						( char *buffer, int bufferLength );

// filesystem access
// returns length of file
int			trap_FS_FOpenFile				( const char *qpath, fileHandle_t *f, fsMode_t mode );
void		trap_FS_Read					( void *buffer, int len, fileHandle_t f );
void		trap_FS_Write					( const void *buffer, int len, fileHandle_t f );
void		trap_FS_FCloseFile				( fileHandle_t f );
int			trap_FS_GetFileList				( const char *path, const char *extension, char *listbuf, int bufsize );

// add commands to the local console as if they were typed in
// for map changing, etc.  The command is not executed immediately,
// but will be executed in order the next time console commands
// are processed
void		trap_SendConsoleCommand			( const char *text );

// register a command name so the console can perform command completion.
// FIXME: replace this with a normal console command "defineCommand"?
void		trap_AddCommand					( const char *cmdName );

// send a string to the server over the network
void		trap_SendClientCommand			( const char *s );

// force a screen update, only used during gamestate load
void		trap_UpdateScreen				( void );

// model collision
void		 trap_CM_LoadMap					( const char *mapname, qboolean SubBSP );
int			 trap_CM_NumInlineModels			( void );
clipHandle_t trap_CM_InlineModel				( int index );		// 0 = world, 1+ = bmodels
clipHandle_t trap_CM_TempBoxModel				( const vec3_t mins, const vec3_t maxs );
int			 trap_CM_PointContents				( const vec3_t p, clipHandle_t model );
int			 trap_CM_TransformedPointContents	( const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles );
void		 trap_CM_BoxTrace					( trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask );
void		 trap_CM_TransformedBoxTrace		( trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask, const vec3_t origin, const vec3_t angles );

// Returns the projection of a polygon onto the solid brushes in the world
int			trap_CM_MarkFragments				( int numPoints, const vec3_t *points, const vec3_t projection, int maxPoints, vec3_t pointBuffer, int maxFragments, markFragment_t *fragmentBuffer );

// normal sounds will have their volume dynamically changed as their entity
// moves and the listener moves
void		trap_S_StopAllSounds		( void );
void		trap_S_StartSound			( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx, int volume, int radius );
void		trap_S_StopLoopingSound		( int entnum );

// a local sound is always played full volume
void		trap_S_StartLocalSound		( sfxHandle_t sfx, int channelNum );
void		trap_S_ClearLoopingSounds	( qboolean killall );
void		trap_S_AddLoopingSound		( int entityNum, const vec3_t origin, const vec3_t velocity, float radius, sfxHandle_t sfx );
void		trap_S_AddRealLoopingSound	( int entityNum, const vec3_t origin, const vec3_t velocity, float radius, sfxHandle_t sfx );
void		trap_S_UpdateEntityPosition	( int entityNum, const vec3_t origin );

// repatialize recalculates the volumes of sound as they should be heard by the
// given entityNum and position
void		trap_S_Respatialize			( int entityNum, const vec3_t origin, vec3_t axis[3], int inwater );
sfxHandle_t	trap_S_RegisterSound		( const char *sample);		// returns buzz if not found
void		trap_S_StartBackgroundTrack	( const char *intro, const char *loop, qboolean bReturnWithoutStarting);	// empty name stops music
void		trap_S_StopBackgroundTrack	( void );

void		trap_AS_AddPrecacheEntry	( const char *set);
void		trap_AS_ParseSets			( void );
void		trap_AS_UpdateAmbientSet	( const char *name, vec3_t origin);
int			trap_AS_AddLocalSet			( const char *name, vec3_t listener_origin, vec3_t origin, int entID, int time);
sfxHandle_t	trap_AS_GetBModelSound		( const char *name, int stage);


void		trap_R_LoadWorldMap			( const char *mapname );

// all media should be registered during level startup to prevent
// hitches during gameplay
qhandle_t	trap_R_RegisterModel		( const char *name );			// returns rgb axis if not found
qhandle_t	trap_R_RegisterSkin			( const char *name );			// returns all white if not found
qhandle_t	trap_R_RegisterShader		( const char *name );			// returns all white if not found
qhandle_t	trap_R_RegisterShaderNoMip	( const char *name );			// returns all white if not found
qhandle_t	trap_R_RegisterFont			( const char *fontName );

// a scene is built up by calls to R_ClearScene and the various R_Add functions.
// Nothing is drawn until R_RenderScene is called.
void		trap_R_ClearScene			( void );
void		trap_R_ClearDecals			( void );
void		trap_R_AddRefEntityToScene	( const refEntity_t *re );

int			trap_R_GetTextWidth			( const char* text, qhandle_t font, float scale, int limit );
int			trap_R_GetTextHeight		( const char* text, qhandle_t font, float scale, int limit );
void		trap_R_DrawText				( int x, int y, qhandle_t font, float scale, vec4_t color, const char* text, int limit, int flags );
void		trap_R_DrawTextWithCursor	( int x, int y, qhandle_t font, float scale, vec4_t color, const char* text, int limit, int flags, int cursorPos, char cursor );

// polys are intended for simple wall marks, not really for doing
// significant construction
void		trap_R_AddPolyToScene		( qhandle_t hShader , int numVerts, const polyVert_t *verts );
void		trap_R_AddPolysToScene		( qhandle_t hShader , int numVerts, const polyVert_t *verts, int numPolys );
void		trap_R_AddDecalToScene		( qhandle_t shader, const vec3_t origin, const vec3_t dir, float orientation, float r, float g, float b, float a, qboolean alphaFade, float radius, qboolean temporary );
void		trap_R_AddLightToScene		( const vec3_t org, float intensity, float r, float g, float b );
int			trap_R_LightForPoint		( vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir );
void		trap_R_RenderScene			( const refdef_t *fd );
void		trap_R_DrawVisualOverlay	( visual_t type, qboolean preProcess, float parm1, float parm2 );
void		trap_R_SetColor				( const float *rgba );	// NULL = 1,1,1,1
void		trap_R_DrawStretchPic		( float x, float y, float w, float h, float s1, float t1, float s2, float t2, const float* color, qhandle_t hShader );
void		trap_R_ModelBounds			( clipHandle_t model, vec3_t mins, vec3_t maxs );
int			trap_R_LerpTag				( orientation_t *tag, clipHandle_t mod, int startFrame, int endFrame, float frac, const char *tagName );

// Does weird, barely controllable rotation behaviour
void		trap_R_DrawRotatePic		( float x, float y, float w, float h, float s1, float t1, float s2, float t2,float a, qhandle_t hShader );

// rotates image around exact center point of passed in coords
void		trap_R_DrawRotatePic2		( float x, float y, float w, float h, float s1, float t1, float s2, float t2,float a, qhandle_t hShader );
void		trap_R_RemapShader			( const char *oldShader, const char *newShader, const char *timeOffset );

void		trap_R_GetLightStyle		( int style, color4ub_t color);
void		trap_R_SetLightStyle		( int style, int color);

// The glconfig_t will not change during the life of a cgame.
// If it needs to change, the entire cgame will be restarted, because
// all the qhandle_t are then invalid.
void		trap_GetGlconfig( glconfig_t *glconfig );

// the gamestate should be grabbed at startup, and whenever a
// configstring changes
void		trap_GetGameState( gameState_t *gamestate );

// cgame will poll each frame to see if a newer snapshot has arrived
// that it is interested in.  The time is returned seperately so that
// snapshot latency can be calculated.
void		trap_GetCurrentSnapshotNumber( int *snapshotNumber, int *serverTime );

// a snapshot get can fail if the snapshot (or the entties it holds) is so
// old that it has fallen out of the client system queue
qboolean	trap_GetSnapshot( int snapshotNumber, snapshot_t *snapshot );


qboolean	trap_GetDefaultState(int entityIndex, entityState_t *state );

// retrieve a text command from the server stream
// the current snapshot will hold the number of the most recent command
// qfalse can be returned if the client system handled the command
// argc() / argv() can be used to examine the parameters of the command
qboolean	trap_GetServerCommand( int serverCommandNumber );

// returns the most recent command number that can be passed to GetUserCmd
// this will always be at least one higher than the number in the current
// snapshot, and it may be quite a few higher if it is a fast computer on
// a lagged connection
int			trap_GetCurrentCmdNumber( void );	

qboolean	trap_GetUserCmd( int cmdNumber, usercmd_t *ucmd );

// used for the weapon select and zoom
void		trap_SetUserCmdValue( int stateValue, float sensitivityScale );

void		trap_RW_SetTeam(int team, qboolean dead);

void		trap_ResetAutorun ( void );

// aids for VM testing
void		testPrintInt( char *string, int i );
void		testPrintFloat( char *string, float f );

// Cinematic playing
int			trap_CIN_PlayCinematic	( const char *arg0, int xpos, int ypos, int width, int height, int bits);
e_status	trap_CIN_StopCinematic	( int handle);
e_status	trap_CIN_RunCinematic	( int handle);
void		trap_CIN_DrawCinematic	( int handle);
void		trap_CIN_SetExtents		( int handle, int x, int y, int w, int h);

// Special FX system traps
int			trap_FX_InitSystem			( refdef_t* );
int			trap_FX_RegisterEffect		( const char *file);
void		trap_FX_PlaySimpleEffect	( const char *file, vec3_t org, int vol, int rad );					// uses a default up axis
void		trap_FX_PlayEffect			( const char *file, vec3_t org, vec3_t fwd, int vol, int rad );		// builds arbitrary perp. right vector, does a cross product to define up
void		trap_FX_PlayEntityEffect	( const char *file, vec3_t org, vec3_t axis[3], const int boltInfo, const int entNum, int vol, int rad );
void		trap_FX_PlaySimpleEffectID	( int id, vec3_t org, int vol, int rad );					// uses a default up axis
void		trap_FX_PlayEffectID		( int id, vec3_t org, vec3_t fwd, int vol, int rad );		// builds arbitrary perp. right vector, does a cross product to define up
void		trap_FX_PlayEntityEffectID	( int id, vec3_t org, vec3_t axis[3], const int boltInfo, const int entNum, int vol, int rad );
void		trap_FX_PlayBoltedEffectID	(int id, CFxBoltInterface *obj, int vol, int rad );
void		trap_FX_AddScheduledEffects	( void );
void		trap_FX_Draw2DEffects		( float screenXScale, float screenYScale );
qboolean	trap_FX_FreeSystem			( void );
void		trap_FX_AdjustTime			( int time );
void		trap_FX_Reset				( void );
void		trap_FX_AddLine				( const vec3_t start, const vec3_t end, float size1, float size2, float sizeParm,
										  float alpha1, float alpha2, float alphaParm,
										  const vec3_t sRGB, const vec3_t eRGB, float rgbParm,
										  int killTime, qhandle_t shader, int flags);

int			trap_MemoryRemaining		( void );

// Keycatcher traps
qboolean	trap_Key_IsDown				( int keynum );
int			trap_Key_GetCatcher			( void );
void		trap_Key_SetCatcher			( int catcher );
int			trap_Key_GetKey				( const char *binding );

// RMG traps
void		trap_RMG_Init				( int terrainID, const char *terrainInfo );


void trap_SnapVector( float *v );

qboolean	trap_GetEntityToken( char *buffer, int bufferSize );
qboolean	trap_R_inPVS( const vec3_t p1, const vec3_t p2 );

#define G2SURFACEFLAG_ISBOLT		0x00000001
#define G2SURFACEFLAG_OFF			0x00000002	// saves strcmp()ing for "_off" in surface names
#define G2SURFACEFLAG_SPARE0		0x00000004	// future-expansion fields, saves invalidating models if we add more
#define G2SURFACEFLAG_SPARE1		0x00000008	//   
#define G2SURFACEFLAG_SPARE2		0x00000010	// 
#define G2SURFACEFLAG_SPARE3		0x00000020	// 
#define G2SURFACEFLAG_SPARE4		0x00000040	// 
#define G2SURFACEFLAG_SPARE5		0x00000080	// 
//
#define G2SURFACEFLAG_NODESCENDANTS 0x00000100	// ingame-stuff, never generated by Carcass....
#define G2SURFACEFLAG_GENERATED		0x00000200	//


void		trap_G2API_CollisionDetect		( CollisionRecord_t *collRecMap, void* ghoul2, const vec3_t angles, const vec3_t position,int frameNumber, int entNum, const vec3_t rayStart, const vec3_t rayEnd, const vec3_t scale, int traceFlags, int useLod );


// CG specific API access
void		trap_G2_ListModelSurfaces(void *ghlInfo);
void		trap_G2_ListModelBones(void *ghlInfo, int frame);
int			trap_G2API_AddBolt(void *ghoul2, const int modelIndex, const char *boneName);
void		trap_G2API_SetBoltInfo(void *ghoul2, int modelIndex, int boltInfo);
qboolean	trap_G2API_RemoveBolt(void *ghlInfo, const int modelIndex, const int index);
qboolean	trap_G2API_AttachG2Model(void *ghoul2From, int modelFrom, void *ghoul2To, int toBoltIndex, int toModel);
qboolean	trap_G2API_DetachG2Model(void *ghoul2, int modelIndex);
void		trap_G2_SetGhoul2ModelIndexes(void *ghoul2, qhandle_t *modelList, qhandle_t *skinList);
qboolean	trap_G2_HaveWeGhoul2Models(void *ghoul2);
qboolean	trap_G2API_GetBoltMatrix(void *ghoul2, const int modelIndex, const int boltIndex, mdxaBone_t *matrix,
								const vec3_t angles, const vec3_t position, const int frameNum, qhandle_t *modelList, vec3_t scale);
int			trap_G2API_InitGhoul2Model(void **ghoul2Ptr, const char *fileName, int modelIndex, qhandle_t customSkin,
						  qhandle_t customShader, int modelFlags, int lodBias);
qboolean	trap_G2API_GetAnimFileNameIndex (TGhoul2 ghoul2, qhandle_t modelIndex, const char* name );

int			trap_G2API_CopyGhoul2Instance(void *g2From, void *g2To, int modelIndex);
int			trap_G2API_CopySpecificGhoul2Model(void *g2From, int modelFrom, void *g2To, int modelTo);
void		trap_G2API_DuplicateGhoul2Instance(void *g2From, void **g2To);
qboolean	trap_G2API_RemoveGhoul2Model(void **ghlInfo, int modelIndex);
void		trap_G2API_CleanGhoul2Models(void **ghoul2Ptr);

qboolean	trap_G2API_SetBoneAngles(void *ghoul2, int modelIndex, const char *boneName, const vec3_t angles, const int flags,
								const int up, const int right, const int forward, qhandle_t *modelList,
								int blendTime , int currentTime );
char		*trap_G2API_GetGLAName(void *ghoul2, int modelIndex);

qboolean	trap_G2API_SetBoneAnim( void *ghoul2, const int modelIndex, const char *boneName, const int startFrame, const int endFrame,
									const int flags, const float animSpeed, const int currentTime, const float setFrame , const int blendTime );
qboolean	trap_G2API_GetBoneAnim( void *ghoul2, const int modelIndex, const char *boneName, const int currentTime, float* frame );

qboolean	trap_G2API_SetSurfaceOnOff(void *ghoul2, const int modelIndex, const char *surfaceName, const int flags);
qboolean	trap_G2API_SetRootSurface(void **ghoul2, const int modelIndex, const char *surfaceName);
qboolean	trap_G2API_SetNewOrigin(void *ghoul2, const int modelIndex, const int boltIndex);
void		trap_G2API_AddSkinGore(void *ghlInfo,SSkinGoreData *gore);
void		trap_G2API_ClearSkinGore ( TGhoul2 ghoul2 );
qboolean	trap_G2API_SetGhoul2ModelFlags(void *ghlInfo,int flags);
int			trap_G2API_GetGhoul2ModelFlags(void *ghlInfo);
qboolean	trap_G2API_SetGhoul2ModelFlagsByIndex(void *ghoul2,int index,int flags);
int			trap_G2API_GetGhoul2ModelFlagsByIndex(void *ghoul2,int index);
int			trap_G2API_GetNumModels(TGhoul2 ghoul2);
int			trap_G2API_FindBoltIndex(TGhoul2 ghoul2, const int modelIndex, const char *boneName);
int			trap_G2API_GetBoltIndex(TGhoul2 ghoul2, const int modelIndex);

qhandle_t	trap_G2API_RegisterSkin ( const char *skinName, int numPairs, const char *skinPairs);
qboolean	trap_G2API_SetSkin		( TGhoul2 ghoul2, int modelIndex, qhandle_t customSkin);

void		trap_MAT_Init(void);
void		trap_MAT_Reset(void);
sfxHandle_t trap_MAT_GetSound(char *key, int material);
qhandle_t	trap_MAT_GetDecal(char *key, int material);
const float trap_MAT_GetDecalScale(char *key, int material);
qhandle_t	trap_MAT_GetEffect(char *key, int material);
qhandle_t	trap_MAT_GetDebris(char *key, int material);
const float trap_MAT_GetDebrisScale(char *key, int material);

#define MAT_FOOTSTEP_NORMAL		"footstep"
#define MAT_FOOTSTEP_STEALTH	"footstepStealth"
#define MAT_FOOTSTEP_PRONE		"footstepProne"
#define MAT_AMMO_556			"5.56mm"
#define MAT_AMMO_9  			"9mm"
#define MAT_AMMO_12 			"12 gauge"
#define MAT_AMMO_045			"0.45 ACP"
#define MAT_AMMO_762			"7.62mm belt"
#define MAT_AMMO_127			"12.7mm"
#define MAT_LAND_NORMAL			"land"
#define MAT_LAND_PAIN			"land_pain"
#define MAT_LAND_DEATH			"land_death"
#define MAT_BOUNCEMETAL_1		"bouncemetal0"
#define MAT_BOUNCEMETAL_2		"bouncemetal1"


void		trap_CM_TM_Create(int terrainID);
void		trap_CM_TM_AddBuilding(int x, int y, int side);
void		trap_CM_TM_AddSpot(int x, int y);
void		trap_CM_TM_AddTarget(int x, int y, float radius);
void		trap_CM_TM_Upload(const vec3_t origin, const vec3_t angle);
void		trap_CM_TM_ConvertPosition(void);

int			trap_CM_RegisterTerrain(const char *config);
void		trap_RE_InitRendererTerrain( const char *info );
void		trap_CG_RegisterSharedMemory(char *memory);

// Memory allocation (note this memory is allocated from cgames vm hunk)
void*		trap_VM_LocalAlloc				( int size );
void*		trap_VM_LocalAllocUnaligned		( int size );			// WARNING!!!! USE WITH CAUTION!!! BEWARE OF DOG!!!
void*		trap_VM_LocalTempAlloc			( int size );
void		trap_VM_LocalTempFree			( int size );			// free must be in opposite order of allocation!
const char*	trap_VM_LocalStringAlloc		( const char *source );

// Direct communication with the UI module
void		trap_UI_CloseAll				( void );
void		trap_UI_SetActiveMenu			( int menu );


void		CG_Init_CG(void);
void		CG_Init_CGents(void);


void CG_SetGhoul2Info( refEntity_t *ent, centity_t *cent);
void CG_AddProcGore(centity_t *cent);
void CG_PredictedProcGore ( int weaponnum, int attack, vec3_t start, vec3_t end, centity_t* cent );

qboolean G2_PositionEntityOnBolt(refEntity_t *ent, 
							 void *ghoul2, int modelIndex, int boltIndex, 
							 vec3_t origin, vec3_t angles, vec3_t modelScale);


void		CG_MiscEnt				( void );
void		CG_DrawMiscEnts			( void );
qboolean	CG_ParseGametypeFile	( void );

