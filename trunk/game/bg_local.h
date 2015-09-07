// Copyright (C) 2001-2002 Raven Software.
//
// bg_local.h -- local definitions for the bg (both games) files

#define	MIN_WALK_NORMAL				0.7f		// can't walk on very steep slopes
#define MIN_WALK_NORMAL_TERRAIN		0.625f		// bit steeper for terrain

#define	STEPSIZE		18

#define	JUMP_VELOCITY	270

#define	TIMER_LAND		130
#define	TIMER_GESTURE	(34*66+50)

#define	OVERCLIP		1.001f

// all of the locals will be zeroed before each
// pmove, just to make damn sure we don't have
// any differences when running on client or server
typedef struct 
{
	vec3_t		forward, right, up;
	float		frametime;

	int			msec;

	qboolean	walking;
	qboolean	groundPlane;
	trace_t		groundTrace;

	float		impactSpeed;

	vec3_t		previous_origin;
	vec3_t		previous_velocity;
	int			previous_waterlevel;
} pml_t;

extern	pml_t		pml;

// movement parameters
extern	float	pm_stopspeed;
extern	float	pm_duckScale;
extern	float	pm_swimScale;
extern	float	pm_wadeScale;

extern	float	pm_accelerate;
extern	float	pm_airaccelerate;
extern	float	pm_wateraccelerate;
extern	float	pm_flyaccelerate;

extern	float	pm_friction;
extern	float	pm_waterfriction;
extern	float	pm_flightfriction;

extern	int		c_pmove;

void PM_ClipVelocity( vec3_t in, vec3_t normal, vec3_t out, float overbounce );
void PM_AddTouchEnt( int entityNum );
void PM_AddEvent( int newEvent );
void PM_AddEventWithParm( int newEvent, int parm );

qboolean	PM_SlideMove( qboolean gravity );
void		PM_StepSlideMove( qboolean gravity );

void PM_StartTorsoAnim		( playerState_t* ps, int anim, int time );
void PM_ContinueLegsAnim	( playerState_t* ps, int anim );
void PM_ForceLegsAnim		( playerState_t* ps, int anim );
void PM_TorsoAnimation		( playerState_t* ps );
void PM_SetAnim				( playerState_t* ps, int setAnimParts,int anim,int setAnimFlags, int blendTime);

