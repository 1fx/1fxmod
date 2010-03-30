// Copyright (C) 2001-2002 Raven Software.
//
// cg_gore.c -- handle client-side gore

#include "cg_local.h"
#if !defined(GHOUL2_SHARED_H_INC)
#include "..\ghoul2\G2_gore_shared.h"
#endif


void CG_AddGore(int type, float size, vec3_t hitloc, vec3_t hitdirection, 
				int entnum, vec3_t entposition, float entangle, void *ghoul2)
{
	SSkinGoreData goreSkin;

	memset ( &goreSkin, 0, sizeof(goreSkin) );

	goreSkin.growDuration = -1; // default expandy time
	goreSkin.goreScaleStartFraction = 1.0; // default start scale
	goreSkin.frontFaces = qtrue; // forever
	goreSkin.backFaces = qtrue; // forever
	goreSkin.lifeTime = 0;
	goreSkin.baseModelOnly = qfalse;
	
	goreSkin.currentTime = cg.time;
	goreSkin.entNum      = entnum;
	goreSkin.SSize		 = size;
	goreSkin.TSize		 = size;
	goreSkin.theta		 = flrand(0,6.28);
	goreSkin.shaderEnum  = type;

	VectorSet ( goreSkin.scale, 1, 1, 1 );

	VectorCopy ( hitdirection, goreSkin.rayDirection);

	VectorCopy ( hitloc, goreSkin.hitLocation );
	VectorCopy ( entposition, goreSkin.position );
	goreSkin.angles[YAW] = entangle;

	trap_G2API_AddSkinGore(ghoul2,&goreSkin);
}

void CG_AddGrowGore(int type, float size, int growtime, float startfrac, vec3_t hitloc, vec3_t hitdirection,
				int entnum, vec3_t entposition, float entangle, void *ghoul2)
{
	SSkinGoreData goreSkin;

	memset ( &goreSkin, 0, sizeof(goreSkin) );

	goreSkin.frontFaces = qtrue; // forever
	goreSkin.backFaces = qtrue; // forever
	goreSkin.lifeTime = 0;
	goreSkin.baseModelOnly = qfalse;
	
	goreSkin.currentTime = cg.time;
	goreSkin.entNum      = entnum;
	goreSkin.SSize		 = size;
	goreSkin.TSize		 = size;
	goreSkin.theta		 = flrand(0,6.28);
	goreSkin.shaderEnum  = type;
	goreSkin.growDuration = growtime; // default expandy time
	goreSkin.goreScaleStartFraction = startfrac; // default start scale

	VectorSet ( goreSkin.scale, 1, 1, 1 );

	VectorCopy ( hitdirection, goreSkin.rayDirection);

	VectorCopy ( hitloc, goreSkin.hitLocation );
	VectorCopy ( entposition, goreSkin.position );
	goreSkin.angles[YAW] = entangle;

	trap_G2API_AddSkinGore(ghoul2,&goreSkin);
}

void CG_AddSlashGore(int type, float angle, float ssize, float tsize, vec3_t hitloc, vec3_t hitdirection, 
					 int entnum, vec3_t entposition, float entangle, void *ghoul2)
{
	SSkinGoreData goreSkin;

	memset ( &goreSkin, 0, sizeof(goreSkin) );

	goreSkin.growDuration = -1; // default expandy time
	goreSkin.goreScaleStartFraction = 1.0; // default start scale
	goreSkin.frontFaces = qtrue; // forever
	goreSkin.backFaces = qtrue; // forever
	goreSkin.lifeTime = 0;
	goreSkin.baseModelOnly = qfalse;

	goreSkin.currentTime = cg.time;
	goreSkin.entNum      = entnum;
	goreSkin.SSize		 = ssize;
	goreSkin.TSize		 = tsize;
	goreSkin.theta		 = angle;
	goreSkin.shaderEnum  = type;

	VectorSet ( goreSkin.scale, 1, 1, 1 );

	VectorCopy ( hitdirection, goreSkin.rayDirection);

	VectorCopy ( hitloc, goreSkin.hitLocation );
	VectorCopy ( entposition, goreSkin.position );
	goreSkin.angles[YAW] = entangle;

	trap_G2API_AddSkinGore(ghoul2,&goreSkin);
}

void CG_AddSlashGrowGore(int type, float angle, float ssize, float tsize, int growtime, float startfrac, 
							vec3_t hitloc, vec3_t hitdirection, 
							int entnum, vec3_t entposition, float entangle, void *ghoul2)
{
	SSkinGoreData goreSkin;

	memset ( &goreSkin, 0, sizeof(goreSkin) );

	goreSkin.frontFaces = qtrue; // forever
	goreSkin.backFaces = qtrue; // forever
	goreSkin.lifeTime = 0;
	goreSkin.baseModelOnly = qfalse;

	goreSkin.growDuration = growtime; // default expandy time
	goreSkin.goreScaleStartFraction = startfrac; // default start scale
	goreSkin.currentTime = cg.time;
	goreSkin.entNum      = entnum;
	goreSkin.SSize		 = ssize;
	goreSkin.TSize		 = tsize;
	goreSkin.theta		 = angle;
	goreSkin.shaderEnum  = type;

	VectorSet ( goreSkin.scale, 1, 1, 1 );

	VectorCopy ( hitdirection, goreSkin.rayDirection);

	VectorCopy ( hitloc, goreSkin.hitLocation );
	VectorCopy ( entposition, goreSkin.position );
	goreSkin.angles[YAW] = entangle;

	trap_G2API_AddSkinGore(ghoul2,&goreSkin);
}


void CG_AddTimedGore(int type, float size, int duration, vec3_t hitloc, vec3_t hitdirection, 
					int entnum, vec3_t entposition, float entangle, void *ghoul2)
{
	SSkinGoreData goreSkin;

	memset ( &goreSkin, 0, sizeof(goreSkin) );

	goreSkin.growDuration = -1; // default expandy time
	goreSkin.goreScaleStartFraction = 1.0; // default start scale
	goreSkin.frontFaces = qtrue; // forever
	goreSkin.backFaces = qtrue; // forever
	goreSkin.baseModelOnly = qfalse;

	goreSkin.currentTime = cg.time;
	goreSkin.entNum      = entnum;
	goreSkin.SSize		 = size;
	goreSkin.TSize		 = size;
	goreSkin.theta		 = flrand(0,6.28);
	goreSkin.shaderEnum  = type;
	goreSkin.lifeTime	 = duration;

	VectorSet ( goreSkin.scale, 1, 1, 1 );

	VectorCopy ( hitdirection, goreSkin.rayDirection);

	VectorCopy ( hitloc, goreSkin.hitLocation );
	VectorCopy ( entposition, goreSkin.position );
	goreSkin.angles[YAW] = entangle;

	trap_G2API_AddSkinGore(ghoul2,&goreSkin);
}


void CG_DoGoreFromWeapon( int weaponnum, int attack, vec3_t hitloc, vec3_t hitdirection, 
						 int entnum, vec3_t entposition, float entangle, void *ghoul2)
{
	float angle, size, size2;

	switch (weaponnum)
	{
	case WP_KNIFE:
		if (attack==ATTACK_ALTERNATE)
		{
			CG_AddGore(PGORE_PUNCTURE, flrand(3.5, 4.0), 
						hitloc, hitdirection, entnum, entposition, entangle, ghoul2);
			if (cg_goreDetail.integer>0)
			{
				CG_AddGrowGore(PGORE_KNIFE_SOAK, 4.0*1.4, 15000, 0.1,
							hitloc, hitdirection, entnum, entposition, entangle, ghoul2);
			}
		}
		else
		{
			angle= (M_PI / 2 * (1+2*irand(0,1))) + flrand( .7, .7);
			switch(irand(1,3))
			{
			case 1:
				size = flrand(2.8, 3.2);
				size2 = flrand(1.8, 2.2);
				CG_AddSlashGore(PGORE_KNIFESLASH, 
						angle, size, size2, 
						hitloc, hitdirection, entnum, entposition, entangle, ghoul2);
				if (cg_goreDetail.integer>0)
				{
					CG_AddSlashGrowGore(PGORE_KNIFE_SOAK, angle, size*1.2, size2*2.0, 15000, 0.1,
								hitloc, hitdirection, entnum, entposition, entangle, ghoul2);
				}
				break;
			case 2:
				size = 8.0f*flrand(.8, 1.2);
				size2 = 1.75f*flrand(.8, 1.2);
				CG_AddSlashGore(PGORE_KNIFESLASH2, 
						angle, size, size2, 
						hitloc, hitdirection, entnum, entposition, entangle, ghoul2);
				if (cg_goreDetail.integer>0)
				{
					CG_AddSlashGrowGore(PGORE_KNIFE_SOAK, angle, size*1.2, size2*2.0, 15000, 0.1,
								hitloc, hitdirection, entnum, entposition, entangle, ghoul2);
				}
				break;
			default:
				size = flrand(3.0f,4.0f);
				size2 = flrand(0.5f,1.0f);
				CG_AddSlashGore(PGORE_KNIFESLASH3, 
						angle, size, size2, 
						hitloc, hitdirection, entnum, entposition, entangle, ghoul2);
				if (cg_goreDetail.integer>0)
				{
					CG_AddSlashGrowGore(PGORE_KNIFE_SOAK, angle, size*1.2, size2*2.0, 15000, 0.1,
								hitloc, hitdirection, entnum, entposition, entangle, ghoul2);
				}
				break;
			}
		}
		break;

	// Smaller guns with pistol whip altfires
	case WP_M1911A1_PISTOL:
	case WP_USSOCOM_PISTOL:
		if (attack==ATTACK_ALTERNATE)
		{	// Bonk on the head
			CG_AddGore(PGORE_BLOODY_SPLOTCH2, flrand(5.25f, 7.5f), 
						hitloc, hitdirection, entnum, entposition, entangle, ghoul2);

		}
		else
		{
			CG_AddGore(irand(PGORE_BULLET_E, PGORE_BULLET_G), flrand( 3.75f, 4.5f), 
						hitloc, hitdirection, entnum, entposition, entangle, ghoul2);
			if (cg_goreDetail.integer>0)
			{
				CG_AddGrowGore(PGORE_KNIFE_SOAK, 4.5*1.35, 15000, 0.1f,
							hitloc, hitdirection, entnum, entposition, entangle, ghoul2);
			}
		}
		break;

	// Small guns
	case WP_MICRO_UZI_SUBMACHINEGUN:
		CG_AddGore(irand(PGORE_BULLET_E, PGORE_BULLET_G), flrand( 3.75f, 4.5f), 
						hitloc, hitdirection, entnum, entposition, entangle, ghoul2);
		if (cg_goreDetail.integer>0)
		{
			CG_AddGrowGore(PGORE_KNIFE_SOAK, 4.5f*1.35f, 15000, 0.1f,
						hitloc, hitdirection, entnum, entposition, entangle, ghoul2);
		}
		break;

	// Shotgun with whip altfire
	case WP_M590_SHOTGUN:
		if (attack==ATTACK_ALTERNATE)
		{	// Bond on de haid
			CG_AddGore(PGORE_BLOODY_SPLOTCH2, flrand(7.75, 11.25), 
						hitloc, hitdirection, entnum, entposition, entangle, ghoul2);
		}
		else
		{
			CG_AddGore(irand(PGORE_SHOTGUN, PGORE_SHOTGUNBIG), flrand( 8.25f, 11.25f), 
						hitloc, hitdirection, entnum, entposition, entangle, ghoul2);
			if (cg_goreDetail.integer>0)
			{
				CG_AddGrowGore(PGORE_KNIFE_SOAK, 11.25f*1.25f, 15000, 0.1f,
							hitloc, hitdirection, entnum, entposition, entangle, ghoul2);
				if (cg_goreDetail.integer>1)
				{
					CG_AddGore(PGORE_PELLETS, 8.25f, 
									hitloc, hitdirection, entnum, entposition, entangle, ghoul2);
				}
			}
		}
		break;

	// Medium guns
	case WP_M3A1_SUBMACHINEGUN:
		CG_AddGore(irand(PGORE_BULLET_E, PGORE_BULLET_G), flrand( 5.25f, 7.5f), 
						hitloc, hitdirection, entnum, entposition, entangle, ghoul2);
		if (cg_goreDetail.integer>0)
		{
			CG_AddGrowGore(PGORE_KNIFE_SOAK, 7.5f*1.3f, 15000, 0.1f,
						hitloc, hitdirection, entnum, entposition, entangle, ghoul2);
		}
		break;

	// Shotguns
	case WP_USAS_12_SHOTGUN:
		CG_AddGore(irand(PGORE_SHOTGUN, PGORE_SHOTGUNBIG), flrand( 8.25f, 11.25f), 
					hitloc, hitdirection, entnum, entposition, entangle, ghoul2);
		if (cg_goreDetail.integer>0)
		{
			CG_AddGrowGore(PGORE_KNIFE_SOAK, 11.25f*1.25f, 15000, 0.1,
						hitloc, hitdirection, entnum, entposition, entangle, ghoul2);
			if (cg_goreDetail.integer>1)
			{
				CG_AddGore(PGORE_PELLETS, 8.25f, 
								hitloc, hitdirection, entnum, entposition, entangle, ghoul2);
			}
		}
		break;

	// Assault rifle with grenade altfire
	case WP_M4_ASSAULT_RIFLE:
		if (attack==ATTACK_ALTERNATE)
		{
			CG_AddGore(PGORE_SHRAPNEL, flrand( 14.0f, 17.0f),  
							hitloc, hitdirection, entnum, entposition, entangle, ghoul2);
			if (cg_goreDetail.integer>1)
			{
				CG_AddGore(PGORE_PELLETS, 10.0, 
								hitloc, hitdirection, entnum, entposition, entangle, ghoul2);
			}
		}
		else
		{
			CG_AddGore(irand(PGORE_BULLET_E, PGORE_BULLET_G), flrand( 5.25f, 7.5f), 
							hitloc, hitdirection, entnum, entposition, entangle, ghoul2);
			if (cg_goreDetail.integer>0)
			{
				CG_AddGrowGore(PGORE_KNIFE_SOAK, 7.5f*1.3f, 15000, 0.1f,
							hitloc, hitdirection, entnum, entposition, entangle, ghoul2);
			}
		}
		break;

	// Assault rifle with bayonet altfire
	case WP_AK74_ASSAULT_RIFLE:
		if (attack==ATTACK_ALTERNATE)
		{
			CG_AddGore(PGORE_PUNCTURE, flrand(3.5f, 4.0f), 
						hitloc, hitdirection, entnum, entposition, entangle, ghoul2);
			if (cg_goreDetail.integer>0)
			{
				CG_AddGrowGore(PGORE_KNIFE_SOAK, 4.0f*1.4f, 15000, 0.1f,
							hitloc, hitdirection, entnum, entposition, entangle, ghoul2);
			}
		}
		else
		{
			CG_AddGore(irand(PGORE_BULLET_E, PGORE_BULLET_G), flrand( 5.25f, 7.5f), 
							hitloc, hitdirection, entnum, entposition, entangle, ghoul2);
			if (cg_goreDetail.integer>0)
			{
				CG_AddGrowGore(PGORE_KNIFE_SOAK, 7.5f*1.3f, 15000, 0.1f,
							hitloc, hitdirection, entnum, entposition, entangle, ghoul2);
			}
		}
		break;

	// Large-caliber bullets
	case WP_MSG90A1:
	case WP_M60_MACHINEGUN:
		CG_AddGore(irand(PGORE_BULLET_E, PGORE_BULLET_G), flrand( 6.0f, 9.0f),
						hitloc, hitdirection, entnum, entposition, entangle, ghoul2);
		if (cg_goreDetail.integer>0)
		{
			CG_AddGrowGore(PGORE_KNIFE_SOAK, 9.0f*1.25f, 15000, 0.1f,
						hitloc, hitdirection, entnum, entposition, entangle, ghoul2);
		}
		break;

	// Explosions
	case WP_MM1_GRENADE_LAUNCHER:
	case WP_RPG7_LAUNCHER:
	case WP_M67_GRENADE:
	case WP_F1_GRENADE:
	case WP_L2A2_GRENADE:
	case WP_MDN11_GRENADE:
		CG_AddGore(PGORE_SHRAPNEL, flrand( 14.0f, 17.0f),  
						hitloc, hitdirection, entnum, entposition, entangle, ghoul2);
		if (cg_goreDetail.integer>1)
		{
			CG_AddGore(PGORE_PELLETS, 10.0f, 
							hitloc, hitdirection, entnum, entposition, entangle, ghoul2);
		}
		break;

	// Stun/char
	case WP_M84_GRENADE:
	case WP_M15_GRENADE:
	case WP_SMOHG92_GRENADE:
		CG_AddGore(PGORE_BURN, flrand( 14.0f, 18.0f),  
						hitloc, hitdirection, entnum, entposition, entangle, ghoul2);
		break;

	// Fire
	case WP_ANM14_GRENADE:
		CG_AddTimedGore(PGORE_IMMOLATE, flrand( 18.0f, 22.0f), 4000, 
						hitloc, hitdirection, entnum, entposition, entangle, ghoul2);
		if (cg_goreDetail.integer>0)
		{
			CG_AddGore(PGORE_BURN, flrand( 18.0f, 22.0f),  
							hitloc, hitdirection, entnum, entposition, entangle, ghoul2);
		}
		break;
	}
}


void CG_PredictedProcGore ( int weaponnum, int attack, vec3_t start, vec3_t end, centity_t* cent )
{
	vec3_t direction;

	// if no blood then dont add the proc gore
	if ( cg_lockBlood.integer || !cent->ghoul2 || !cent->currentValid)
	{
		return;
	}
	
	VectorSubtract ( end, start, direction);
	VectorNormalize ( direction);

	CG_DoGoreFromWeapon(weaponnum, attack,				// Weaponnum and altattack
			end, direction,								// hitloc and hitdirection
			cent->currentState.number,					// victim entity number
			cent->lerpOrigin, cent->pe.ghoulLegsAngles[YAW],	// entity position, entity yaw
			cent->ghoul2);
}

/*
======================
CG_AddProcGore

Adds procedural gore to the player specified in the given cent.  The cent is a 
temp ent containing all the information about the shot.
======================
*/
void CG_AddProcGore(centity_t *cent)
{
	centity_t*	  source;
	attackType_t  attack;
	vec3_t		  direction;

	// Blood locked?
	if ( cg_lockBlood.integer || !cent->currentState.time )
	{
		return;
	}

	// No procedural gore on this shot.
	if ( cent->currentState.time & GORE_NONE )
	{
		return;
	}

	source = CG_GetEntity ( cent->currentState.otherEntityNum2 );
	if (!source->ghoul2 || !source->currentValid)
	{
		return;
	}

	// Extract the direction of fire
	ByteToDir( cent->currentState.eventParm, direction );
	attack = ((cent->currentState.time>>8)&0xFF);

	CG_UpdatePlayerModel ( source );

	CG_DoGoreFromWeapon(cent->currentState.time&0xFF, attack,					// Weaponnum and altattack
			cent->lerpOrigin, direction,								// hitloc and hitdirection
			cent->currentState.otherEntityNum2,									// victim entity number
			cent->currentState.angles, (cent->currentState.time>>16)&0x7FFF,				// entity position, entity yaw
			source->ghoul2);
}



#define MAX_GORE_POOL	20000

static char		gorePool[MAX_GORE_POOL];
static int		gorePoolSize = 0;

static char *AllocGorePool(int size)
{
	gorePoolSize = ((gorePoolSize + 0x00000003) & 0xfffffffc);

	if (gorePoolSize + size > MAX_GORE_POOL)
	{
		Com_Error( ERR_DROP, "AllocGorePool: buffer exceeded (%d > %d)", gorePoolSize + size, MAX_GORE_POOL);
		return 0;
	}

	gorePoolSize += size;

	return &gorePool[gorePoolSize-size];
}

static char *AllocMultiString(TGPValue field)
{
	TGPValue	value;
	int			size = 1;
	char		name[256];
	char		*output, *pos;

	if (!field)
	{
		return 0;
	}

	value = trap_GPV_GetList(field);
	while(value)
	{
		trap_GPV_GetName(value, name);
		size += strlen(name) + 1;
		value = trap_GPV_GetNext(value);
	}

	output = pos = AllocGorePool(size);
	value = trap_GPV_GetList(field);
	while(value)
	{
		trap_GPV_GetName(value, name);
		strcpy(pos, name);
		pos += strlen(name) + 1;
		value = trap_GPV_GetNext(value);
	}
	*pos = 0;

	return output;
}








#define	GORE_CHILD						0x00000001
#define	GORE_NO_CHILD_SURFACES_ON		0x00000002
#define GORE_NO_CHILD_FX				0x00000004
#define GORE_NO_CHILD_CHUNKS			0x00000008
#define GORE_NO_CHILD_BOLTONS			0x00000010

typedef enum
{
	GORE_SIDE_RIGHT = 0,
	GORE_SIDE_LEFT,
	GORE_SIDE_MAX
} EGoreSide;

typedef struct SGoreInfo
{
	const char	*mLongName;
	const char	*mShortName;
} TGoreInfo;

typedef struct SGoreLocation
{
	const char	*mPublicName;
	EGoreSide	mPrimarySide;
	EGoreSide	mOppositeSide;
} TGoreLocation;

typedef struct SGoreEffectType
{
	char			mName[MAX_QPATH];
	int				mFXID;

	struct SGoreEffectType	*mNext;
} TGoreEffectType;

typedef struct SGorePieceType
{
	char			mName[MAX_QPATH];
	char			mBolt[MAX_QPATH];

	TGhoul2			mG2Model;
	qhandle_t		mNormalModel;

	struct SGorePieceType	*mNext;
} TGorePieceType;

typedef struct SGoreEffect
{
	char			mName[MAX_QPATH];
	char			mBolt[MAX_QPATH];

	struct SGoreEffect		*mNext;
} TGoreEffect;

typedef struct SGoreBoltOn
{
	char			mName[MAX_QPATH];
	char			mBolt[MAX_QPATH];

	struct SGoreBoltOn		*mNext;
} TGoreBoltOn;

typedef struct SGoreChunk
{
	char			mRoot[MAX_QPATH];
	char			mBone[MAX_QPATH];
	char			*mSurfacesOn;
	char			*mChildrenOff;
	float			mMinForce;
	float			mMaxForce;

	struct SGoreChunk		*mNext;
} TGoreChunk;

typedef struct SGoreArea
{
	char			mLocation[MAX_QPATH];
	char			*mSurfacesOff;
	char			*mSurfacesOn;
	char			*mBoltsOff;
	char			*mChildren;
	unsigned		mFlags;

	struct SGoreEffect		*mFX;
	struct SGoreBoltOn		*mBoltOns;
	struct SGoreChunk		*mChunks;

	struct SGoreArea		*mNext;
} TGoreArea;











static TGoreEffectType	*GoreEffectTypes;
static TGorePieceType	*GorePieceTypes;
static TGoreArea		*GoreAreas;
static TGoreInfo		GoreInfo[GORE_SIDE_MAX] = 
{
	{ "right", "r" },
	{ "left", "l" }
};

static TGoreLocation	GoreLocations[] = 
{
	{ "none",		GORE_SIDE_RIGHT,	GORE_SIDE_LEFT	},

	{ "foot",		GORE_SIDE_RIGHT,	GORE_SIDE_LEFT	},
	{ "foot",		GORE_SIDE_LEFT,		GORE_SIDE_RIGHT	},
	{ "leg_upper",	GORE_SIDE_RIGHT,	GORE_SIDE_LEFT	},
	{ "leg_upper",	GORE_SIDE_LEFT,		GORE_SIDE_RIGHT	},
	{ "leg_lower",	GORE_SIDE_RIGHT,	GORE_SIDE_LEFT	},
	{ "leg_lower",	GORE_SIDE_LEFT,		GORE_SIDE_RIGHT	},
	
	{ "hand",		GORE_SIDE_RIGHT,	GORE_SIDE_LEFT	},
	{ "hand",		GORE_SIDE_LEFT,		GORE_SIDE_RIGHT	},
	{ "arm_lower",	GORE_SIDE_RIGHT,	GORE_SIDE_LEFT	},
	{ "arm_lower",	GORE_SIDE_LEFT,		GORE_SIDE_RIGHT	},

	{ "head",		GORE_SIDE_RIGHT,	GORE_SIDE_LEFT	},
	{ "torso",		GORE_SIDE_RIGHT,	GORE_SIDE_LEFT	},
	
	{ "torso",		GORE_SIDE_RIGHT,	GORE_SIDE_LEFT	},
	{ "torso",		GORE_SIDE_LEFT,		GORE_SIDE_RIGHT	},
	{ "torso",		GORE_SIDE_RIGHT,	GORE_SIDE_LEFT	},
	//{ "torso",		GORE_SIDE_RIGHT,	GORE_SIDE_LEFT	},
	{ "arm_upper",		GORE_SIDE_RIGHT,	GORE_SIDE_LEFT	},
	//{ "torso",		GORE_SIDE_LEFT,		GORE_SIDE_RIGHT	},
	{ "arm_upper",		GORE_SIDE_LEFT,		GORE_SIDE_RIGHT	},

	{ "torso",		GORE_SIDE_LEFT,		GORE_SIDE_RIGHT	},

	{ "",			GORE_SIDE_LEFT,		GORE_SIDE_RIGHT	},

	// DEBUG ONE
	{ "torso",		GORE_SIDE_RIGHT,	GORE_SIDE_LEFT	},

/*
	"hand_right",
	"arm_lower_right",
	"torso_right",
	"arm_upper_right",
	"leg_lower_right",
	"leg_upper_right",
	"hip_right",
	"head_back_lower_right",
	"head_back_upper_right",
	"head_front_lower_right",
	"head_front_mid_right",
	"head_front_upper_right",
	"head_side_right",
	"head_right",
*/
};














static const char *CreateFinalName(const char *Input, EGoreSide *Primary, EGoreSide *Opposite, qboolean SwapIfOpposite)
{
	static char	output[256];
	char		*outputPos;
	const char	*replace = "";
	EGoreSide	use = GORE_SIDE_RIGHT;
	qboolean	doSwap = qfalse;
	EGoreSide	save;
	const char	*origInput = Input;

	outputPos = output;
	while(*Input)
	{
		if ((*Input) == '<')
		{
			Input++;
			if ((*Input) == 'P')
			{	// default to primary
				use = *Primary;
			}
			else if ((*Input) == 'O')
			{	// use opposite
				use = *Opposite;
				doSwap = qtrue;
			}
			else
			{
				Com_Error(ERR_DROP, "CreateFinalName: bad input string: %s", origInput);
				break;
			}

			Input++;
			if ((*Input) == 'L')
			{	// default to long name
				replace = GoreInfo[use].mLongName;
			}
			else if ((*Input) == 'S')
			{	// use short name
				replace = GoreInfo[use].mShortName;
			}
			else
			{
				Com_Error(ERR_DROP, "CreateFinalName: bad input string: %s", origInput);
				break;
			}

			strcpy(outputPos, replace);
			outputPos += strlen(replace);

			Input++;

			if ((*Input) != '>')
			{
				Com_Error(ERR_DROP, "CreateFinalName: bad input string: %s", origInput);
				break;
			}
		}
		else
		{
			*outputPos++ = *Input;
		}

		Input++;
	}

	*outputPos = 0;
	if (SwapIfOpposite && doSwap)
	{
		save = *Primary;
		*Primary = *Opposite;
		*Opposite = save;
	}

	return output;
}

static TGoreArea *FindGoreZone(const char *Location, EGoreSide Primary, EGoreSide Opposite)
{
	TGoreArea		*gore = GoreAreas;

	while(gore)
	{
		if (Q_stricmp(CreateFinalName(gore->mLocation, &Primary, &Opposite, qfalse), Location) == 0)
		{
			return gore;
		}

		gore = gore->mNext;
	}

	return 0;
}

static TGoreEffectType *FindGoreEffectType(const char *Name)
{
	TGoreEffectType	*effect = GoreEffectTypes;

	while(effect)
	{
		if (Q_stricmp(effect->mName, Name) == 0)
		{
			return effect;
		}

		effect = effect->mNext;
	}

	return 0;
}

static TGorePieceType *FindGorePieceType(const char *Name)
{
	TGorePieceType	*piece = GorePieceTypes;

	while(piece)
	{
		if (Q_stricmp(piece->mName, Name) == 0)
		{
			return piece;
		}

		piece = piece->mNext;
	}

	return 0;
}

static void CG_ParseGoreEffect(TGPGroup group)
{
	TGoreEffectType	*effect;
	char			file[256];

	effect = (TGoreEffectType *)AllocGorePool(sizeof(*effect));
	memset(effect, 0, sizeof(*effect));

	effect->mNext = GoreEffectTypes;
	GoreEffectTypes = effect;

	trap_GPG_FindPairValue(group, "Name", "", effect->mName);
	trap_GPG_FindPairValue(group, "File", "", file);
	effect->mFXID = trap_FX_RegisterEffect(file);
}

static void CG_ParseGorePiece(TGPGroup group)
{
	TGorePieceType	*piece;
	char			file[256];

	piece = (TGorePieceType *)AllocGorePool(sizeof(*piece));
	memset(piece, 0, sizeof(*piece));

	piece->mNext = GorePieceTypes;
	GorePieceTypes = piece;

	trap_GPG_FindPairValue(group, "Name", "", piece->mName);
	trap_GPG_FindPairValue(group, "Bolt", "", piece->mBolt);
	trap_GPG_FindPairValue(group, "Model", "", file);

	if (trap_G2API_InitGhoul2Model(&piece->mG2Model, file, 0, 0, 0, 0, 0) == -1)
	{	// wasn't a g2 model, so try a regular one
		piece->mNormalModel = trap_R_RegisterModel(file);
	}
}

static TGoreEffect *CG_ParseFX(TGPGroup group)
{
	TGoreEffect		*effect;

	effect = (TGoreEffect *)AllocGorePool(sizeof(*effect));
	memset(effect, 0, sizeof(*effect));

	trap_GPG_FindPairValue(group, "Name", "", effect->mName);
	trap_GPG_FindPairValue(group, "Bolt", "", effect->mBolt);

	return effect;
}

static TGoreBoltOn *CG_ParseBoltOn(TGPGroup group)
{
	TGoreBoltOn		*bolt;

	bolt = (TGoreBoltOn *)AllocGorePool(sizeof(*bolt));
	memset(bolt, 0, sizeof(*bolt));

	trap_GPG_FindPairValue(group, "Name", "", bolt->mName);
	trap_GPG_FindPairValue(group, "Bolt", "", bolt->mBolt);

	return bolt;
}

static TGoreChunk *CG_ParseChunk(TGPGroup group)
{
	TGoreChunk		*chunk;
	char			temp[256];

	chunk = (TGoreChunk *)AllocGorePool(sizeof(*chunk));
	memset(chunk, 0, sizeof(*chunk));

	trap_GPG_FindPairValue(group, "Root", "", chunk->mRoot);
	trap_GPG_FindPairValue(group, "Bone", "", chunk->mBone);
	chunk->mSurfacesOn = AllocMultiString(trap_GPG_FindPair(group, "Surfaces_On"));
	chunk->mChildrenOff = AllocMultiString(trap_GPG_FindPair(group, "Children_Off"));

	trap_GPG_FindPairValue(group, "MinForce", "40", temp);
	chunk->mMinForce = atof(temp);
	trap_GPG_FindPairValue(group, "MaxForce", "80", temp);
	chunk->mMaxForce = atof(temp);

	return chunk;
}

static void CG_ParseGoreArea(TGPGroup group)
{
	TGoreArea		*gore;
	TGoreEffect		*effect;
	TGoreBoltOn		*bolt;
	TGoreChunk		*chunk;
	TGPValue		flags, value;
	TGPGroup		sub;
	char			name[256];

	gore = (TGoreArea *)AllocGorePool(sizeof(*gore));
	memset(gore, 0, sizeof(*gore));

	gore->mNext = GoreAreas;
	GoreAreas = gore;

	trap_GPG_FindPairValue(group, "Location", "", gore->mLocation);

	gore->mSurfacesOff = AllocMultiString(trap_GPG_FindPair(group, "Surfaces_Off"));
	gore->mSurfacesOn = AllocMultiString(trap_GPG_FindPair(group, "Surfaces_On"));
	gore->mBoltsOff = AllocMultiString(trap_GPG_FindPair(group, "Bolts_Off"));
	gore->mChildren = AllocMultiString(trap_GPG_FindPair(group, "Children"));

	flags = trap_GPG_FindPair(group, "Flags");
	if (flags)
	{
		value = trap_GPV_GetList(flags);
		while(value)
		{
			trap_GPV_GetName(value, name);
			if (Q_stricmp(name, "NoChildSurfacesOn") == 0)
			{
				gore->mFlags |= GORE_NO_CHILD_SURFACES_ON;
			}
			else if (Q_stricmp(name, "NoChildFX") == 0)
			{
				gore->mFlags |= GORE_NO_CHILD_FX;
			}
			else if (Q_stricmp(name, "NoChildChunks") == 0)
			{
				gore->mFlags |= GORE_NO_CHILD_CHUNKS;
			}			
			else if (Q_stricmp(name, "NoChildBoltOns") == 0)
			{
				gore->mFlags |= GORE_NO_CHILD_BOLTONS;
			}			
			
			value = trap_GPV_GetNext(value);
		}
	}

	sub = trap_GPG_GetSubGroups(group);
	while(sub)
	{
		trap_GPG_GetName(sub, name);
		if (Q_stricmp(name, "FX") == 0)
		{
			effect = CG_ParseFX(sub);
			effect->mNext = gore->mFX;
			gore->mFX = effect;
		}
		else if (Q_stricmp(name, "Chunk") == 0)
		{
			chunk = CG_ParseChunk(sub);
			chunk->mNext = gore->mChunks;
			gore->mChunks = chunk;
		}
		else if (Q_stricmp(name, "BoltOn") == 0)
		{
			bolt = CG_ParseBoltOn(sub);
			bolt->mNext = gore->mBoltOns;
			gore->mBoltOns = bolt;
		}

		sub = trap_GPG_GetNext(sub);
	}
}

qboolean CG_ParseGore(void)
{
	TGenericParser2	GP2;
	TGPGroup		topGroup, topSubs;
	char			name[256];

	GP2 = trap_GP_ParseFile("ext_data/sof2.gore", qtrue, qfalse);
	if (!GP2)
	{
		return qfalse;
	}

	gorePoolSize = 0;
	GoreEffectTypes = 0;
	GorePieceTypes = 0;
	GoreAreas = 0;

	topGroup = trap_GP_GetBaseParseGroup(GP2);
	topSubs = trap_GPG_GetSubGroups(topGroup);
	while(topSubs)
	{
		trap_GPG_GetName(topSubs, name);
		if (Q_stricmp(name, "gore_area") == 0)
		{
			CG_ParseGoreArea(topSubs);
		}
		else if (Q_stricmp(name, "gore_effect") == 0)
		{
			CG_ParseGoreEffect(topSubs);
		}
		else if (Q_stricmp(name, "gore_piece") == 0)
		{
			CG_ParseGorePiece(topSubs);
		}

		topSubs = trap_GPG_GetNext(topSubs);
	}

	trap_GP_Delete(&GP2);

	return qtrue;
}

















static void CG_ProcessSurfaceList(void *model, char *surfaceList, int flags,
								  EGoreSide Primary, EGoreSide Opposite)
{
	while(surfaceList && surfaceList[0])
	{
		if (!trap_G2API_SetSurfaceOnOff(model, 0, CreateFinalName(surfaceList, &Primary, &Opposite, qfalse), flags))
		{

#ifdef _DEBUG
//			Com_Printf("Missing surface '%s'\n", surfaceList);
#endif
		}
		surfaceList += strlen(surfaceList) + 1;
	}
}

static void CG_ProcessBoltList(void *model, char *boltList, EGoreSide Primary, EGoreSide Opposite)
{
	int		numModels;
	int		boltIndex;
	int		i;

	while(boltList && boltList[0])
	{
		numModels = trap_G2API_GetNumModels(model);
		boltIndex = trap_G2API_FindBoltIndex(model, 0, CreateFinalName(boltList, &Primary, &Opposite, qfalse));
		if (boltIndex != -1)
		{
			for(i=1;i<numModels;i++)
			{
				if (trap_G2API_GetBoltIndex(model, i) == boltIndex)
				{
					trap_G2API_DetachG2Model(model, i);
					trap_G2API_RemoveGhoul2Model(&model, i);
					numModels--;
					i--;
				}
			}
		}

		boltList += strlen(boltList) + 1;
	}
}

static void CG_ProcessChunkChild(void *Ghoul2, char *Name,
								 EGoreSide Primary, EGoreSide Opposite)
{
	TGoreArea		*gore;
	char			*child;
	char			finalName[256];

	strcpy(finalName, CreateFinalName(Name, &Primary, &Opposite, qtrue));
	gore = FindGoreZone(finalName, Primary, Opposite);
	if (!gore)
	{
		return;
	}

	CG_ProcessSurfaceList(Ghoul2, gore->mSurfacesOff, G2SURFACEFLAG_OFF, Primary, Opposite);

	child = gore->mChildren;
	while(child && child[0])
	{
		CG_ProcessChunkChild(Ghoul2, child, Primary, Opposite);

		child += strlen(child) + 1;
	}
}

static void CG_ProcessChunk(int clientNum, centity_t *cent, TGoreChunk *chunk, vec3_t Direction,
							EGoreSide Primary, EGoreSide Opposite)
{
	localEntity_t	*le;
	refEntity_t		*re;
	int				bolt;
	char			*child;
	animation_t		*anim;
	float			animSpeed;
	int				flags=BONE_ANIM_OVERRIDE_FREEZE;
	clientInfo_t	*ci;
	mdxaBone_t		matrix;
	qboolean		boltMatrixOK = qfalse;;

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_GIB;
	le->startTime = cg.time;
	le->endTime = le->startTime + BODY_SINK_DELAY + BODY_SINK_TIME;
	le->leFlags = LEF_TUMBLE;
	le->bounceFactor = 0.2f;
	re->radius = 50;
	re->renderfx = RF_MINLIGHT;

	VectorSet ( re->modelScale, 1, 1, 1 );

	AxisCopy( axisDefault, re->axis );

	le->pos.trType = TR_GRAVITY;
	VectorMA ( vec3_origin, irand ( chunk->mMinForce * 2, chunk->mMaxForce * 2), Direction, le->pos.trDelta );
	le->pos.trDelta[2] = flrand ( 100, 150 );
	le->pos.trTime = cg.time;

	le->angles.trType = TR_LINEAR_STOP;
	VectorClear(le->angles.trBase);
	le->angles.trBase[YAW] = crandom() * 15;
	le->angles.trDelta[0] = 0.0; // crandom();
	le->angles.trDelta[YAW] = crandom() * 15 - 7;
	le->angles.trDelta[2] = 0.0; // crandom();
	le->angles.trDuration = BODY_SINK_DELAY + BODY_SINK_TIME;
	le->angles.trTime = cg.time;

	le->zOffset = 26.0;

	// ghoul stuff to do limbs
	if (!cent->ghoul2)
	{
		Com_Error(ERR_DROP, "CG_ProcessChunk invalid g2 pointer for client %d\n", clientNum);
	}
	trap_G2API_DuplicateGhoul2Instance(cent->ghoul2, &re->ghoul2);
	trap_G2API_SetRootSurface(&re->ghoul2, 0, CreateFinalName(chunk->mRoot, &Primary, &Opposite, qfalse));
	if (!re->ghoul2)
	{	// whoa, that surface caused our model to go away???
		CG_FreeLocalEntity(le);
		return;
	}

	bolt = trap_G2API_AddBolt(cent->ghoul2, 0, CreateFinalName(chunk->mBone, &Primary, &Opposite, qfalse));
	if (bolt != -1)
	{
		boltMatrixOK = trap_G2API_GetBoltMatrix(cent->ghoul2, 0, bolt, &matrix, cent->lerpAngles, 
			cent->lerpOrigin, cg.time, cgs.gameModels, cent->modelScale);
	}

	if (bolt == -1 || !boltMatrixOK)
	{
		return;
	}

	matrix.matrix[0][3] = cent->lerpOrigin[0];
	matrix.matrix[1][3] = cent->lerpOrigin[1];
	matrix.matrix[2][3] = cent->lerpOrigin[2];

	le->pos.trBase[0] = re->origin[0] = matrix.matrix[0][3];
	le->pos.trBase[1] = re->origin[1] = matrix.matrix[1][3];
	le->pos.trBase[2] = re->origin[2] = matrix.matrix[2][3];

	bolt = trap_G2API_AddBolt(re->ghoul2, 0, CreateFinalName(chunk->mBone, &Primary, &Opposite, qfalse));
	trap_G2API_SetNewOrigin(re->ghoul2, 0, bolt);
	
	ci = &cgs.clientinfo[clientNum];
	anim = &ci->animations[cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT];
	animSpeed = 50.0f / anim->frameLerp;
//	trap_G2API_SetBoneAnim(re->ghoul2, 0, "model_root", 0, 0, flags, animSpeed, cg.time, -1, 0);
//	trap_G2API_SetBoneAnim(re->ghoul2, 0, "lower_lumbar", 0, 0, flags, animSpeed, cg.time, -1, 0);
	trap_G2API_SetBoneAnim(re->ghoul2, 0, "model_root", anim->firstFrame + anim->numFrames - 1, anim->firstFrame + anim->numFrames, flags, animSpeed, cg.time, -1, 0);
	trap_G2API_SetBoneAnim(re->ghoul2, 0, "lower_lumbar", anim->firstFrame + anim->numFrames - 1, anim->firstFrame + anim->numFrames, flags, animSpeed, cg.time, -1, 0);

	CG_ProcessSurfaceList(re->ghoul2, chunk->mSurfacesOn, 0, Primary, Opposite);

	child = chunk->mChildrenOff;
	while(child && child[0])
	{
		CG_ProcessChunkChild(re->ghoul2, child, Primary, Opposite);

		child += strlen(child) + 1;
	}
}

static void CG_ProcessBoltOn(int clientNum, centity_t *cent, TGoreBoltOn *bolton,
							EGoreSide Primary, EGoreSide Opposite)
{
	TGorePieceType	*piece = FindGorePieceType(bolton->mName);
	const char		*boltPosition;
	int				pieceIndex;
	int				boltIndex;

	if (!piece)
	{
		return;
	}

	if (piece->mG2Model)
	{
		boltPosition = bolton->mBolt;
		if (!boltPosition[0])
		{
			boltPosition = piece->mBolt;
		}

		pieceIndex = trap_G2API_CopySpecificGhoul2Model(piece->mG2Model, 0, cent->ghoul2, -1);
		if (pieceIndex != -1)
		{
			boltIndex = trap_G2API_AddBolt(cent->ghoul2, 0, CreateFinalName(boltPosition, &Primary, &Opposite, qfalse));
			if (boltIndex != -1)
			{
				trap_G2API_AttachG2Model(cent->ghoul2, pieceIndex, cent->ghoul2, boltIndex, 0);
			}
		}
	}
}

static void CG_ProcessGore(int clientNum, centity_t *cent, const char *Location, unsigned Flags,
						   vec3_t Direction, EGoreSide Primary, EGoreSide Opposite)
{
	TGoreArea		*gore;
	TGoreEffect		*fx;
	TGoreChunk		*chunk;
	TGoreBoltOn		*bolton;
	char			*children;
	int				bolt;
	mdxaBone_t		matrix;
	qboolean		boltMatrixOK;
	vec3_t			origin;
	TGoreEffectType *effect;
	char			finalName[256];

	strcpy(finalName, CreateFinalName(Location, &Primary, &Opposite, qtrue));
	gore = FindGoreZone(finalName, Primary, Opposite);
	if (!gore)
	{
		return;
	}

	if (!(Flags & GORE_CHILD) || !(Flags & GORE_NO_CHILD_FX) )
	{
		fx = gore->mFX;
		while(fx)
		{
			effect = FindGoreEffectType(fx->mName);
			if (effect)
			{
				bolt = trap_G2API_AddBolt(cent->ghoul2, 0, CreateFinalName(fx->mBolt, &Primary, &Opposite, qfalse));
				if (bolt != -1)
				{
					vec3_t axis[3];
					int boltInfo;

					boltMatrixOK = trap_G2API_GetBoltMatrix(cent->ghoul2, 0, bolt, &matrix, cent->lerpAngles, 
											cent->lerpOrigin, cg.time, cgs.gameModels, cent->modelScale);					
					axis[0][0] = matrix.matrix[0][0];
					axis[0][1] = matrix.matrix[1][0];
					axis[0][2] = matrix.matrix[2][0];
					axis[1][0] = matrix.matrix[0][1];
					axis[1][1] = matrix.matrix[1][1];
					axis[1][2] = matrix.matrix[2][1];
					axis[2][0] = matrix.matrix[0][2];
					axis[2][1] = matrix.matrix[1][2];
					axis[2][2] = matrix.matrix[2][2];
 					origin[0] = matrix.matrix[0][3];
 					origin[1] = matrix.matrix[1][3];
 					origin[2] = matrix.matrix[2][3];


					boltInfo    =  (( 0/*modelnum*/ & MODEL_AND  ) << MODEL_SHIFT  ); 
					boltInfo    |= (( bolt  & BOLT_AND   ) << BOLT_SHIFT   );
					boltInfo    |= (( cent->currentState.number   & ENTITY_AND ) << ENTITY_SHIFT );

					trap_FX_PlayEntityEffectID( effect->mFXID, origin, axis, boltInfo, -1, -1, -1);
/*					boltMatrixOK = trap_G2API_GetBoltMatrix(cent->ghoul2, 0, bolt, &matrix, cent->lerpAngles, 
						cent->lerpOrigin, cg.time, cgs.gameModels, cent->modelScale);
					if (boltMatrixOK)
					{
 						origin[0] = matrix.matrix[0][3];
 						origin[1] = matrix.matrix[1][3];
 						origin[2] = matrix.matrix[2][3];
						trap_FX_PlayEffectID(effect->mFXID, origin, Direction, -1, -1 );
					}
*/				}
			}

			fx = fx->mNext;
		}
	}

	if (!(Flags & GORE_CHILD) || !(Flags & GORE_NO_CHILD_CHUNKS) )
	{
		chunk = gore->mChunks;
		while(chunk)
		{
			CG_ProcessChunk(clientNum, cent, chunk, Direction, Primary, Opposite);

			chunk = chunk->mNext;
		}
	}

	if (!(Flags & GORE_CHILD) || !(Flags & GORE_NO_CHILD_BOLTONS) )
	{
		bolton = gore->mBoltOns;
		while(bolton)
		{
			CG_ProcessBoltOn(clientNum, cent, bolton, Primary, Opposite);

			bolton = bolton->mNext;
		}
	}

	CG_ProcessSurfaceList(cent->ghoul2, gore->mSurfacesOff, G2SURFACEFLAG_OFF, Primary, Opposite);
	if (!(Flags & GORE_CHILD) || !(Flags & GORE_NO_CHILD_SURFACES_ON) )
	{	// children with no gore from the parents shouldn't do this
		CG_ProcessSurfaceList(cent->ghoul2, gore->mSurfacesOn, 0, Primary, Opposite);
	}
	CG_ProcessBoltList(cent->ghoul2, gore->mBoltsOff, Primary, Opposite);

	children = gore->mChildren;
	while(children && children[0])
	{
		CG_ProcessGore(clientNum, cent, children, gore->mFlags | Flags | GORE_CHILD, Direction, Primary, Opposite);
		children += strlen(children) + 1;
	}
}


void CG_ApplyGore(int clientNum, centity_t *cent, int hitLocation, vec3_t Direction)
{
	TGoreLocation	*Location = 0;
	const char		*dg, *token;
	char			area[256];
	EGoreSide		side;
	int				i;

	// Gore locked?
	if ( cg_lockSever.integer )
	{
		return;
	}

	dg = cg_DebugGore.string;
	if (dg[0])
	{
		strcpy(area, COM_Parse(&dg));
		token = COM_Parse(&dg);
		for(side=0;side<GORE_SIDE_MAX;side++)
		{
			if (Q_stricmp(GoreInfo[side].mLongName, token) == 0 ||
				Q_stricmp(GoreInfo[side].mShortName, token) == 0)
			{
				break;
			}
		}
		for(i=0;i<HL_DEBUG;i++)
		{
			if (Q_stricmp(GoreLocations[i].mPublicName, area) == 0 &&
				GoreLocations[i].mPrimarySide == side)
			{
				Location = &GoreLocations[i];
				break;
			}
		}
	}

	if (!Location)
	{
		if (hitLocation < 0 || hitLocation >= (sizeof(GoreLocations) / sizeof(struct SGoreLocation)))
		{
			Com_Error( ERR_DROP, "CG_ApplyGore: invalid hit location %d\n", hitLocation);
			return;
		}
		Location = &GoreLocations[hitLocation];	
	}
	else
	{
		// just so that I don't have to keep restarting the game to change the file
		CG_ParseGore();

		Com_Printf ( "GORE:  Applying gore to location '%s'\n", Location->mPublicName );
	}

	CG_ProcessGore(clientNum, cent, Location->mPublicName, 0, Direction, Location->mPrimarySide, Location->mOppositeSide);
}

void CG_ShutdownGore(void)
{
	TGorePieceType	*piece = GorePieceTypes;

	while(piece)
	{
		if (piece->mG2Model)
		{
			trap_G2API_CleanGhoul2Models(&piece->mG2Model);
		}
		piece = piece->mNext;
	}

}



