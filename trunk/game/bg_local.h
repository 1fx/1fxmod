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
// bg_local.h -- local definitions for the bg (both games) files

#define MIN_WALK_NORMAL             0.7f        // can't walk on very steep slopes
#define MIN_WALK_NORMAL_TERRAIN     0.625f      // bit steeper for terrain

#define STEPSIZE        18

#define JUMP_VELOCITY   270

#define TIMER_LAND      130
#define TIMER_GESTURE   (34*66+50)

#define OVERCLIP        1.001f

// all of the locals will be zeroed before each
// pmove, just to make damn sure we don't have
// any differences when running on client or server
typedef struct 
{
    vec3_t      forward, right, up;
    float       frametime;

    int         msec;

    qboolean    walking;
    qboolean    groundPlane;
    trace_t     groundTrace;

    float       impactSpeed;

    vec3_t      previous_origin;
    vec3_t      previous_velocity;
    int         previous_waterlevel;
} pml_t;

extern  pml_t       pml;

// movement parameters
extern  float   pm_stopspeed;
extern  float   pm_duckScale;
extern  float   pm_swimScale;
extern  float   pm_wadeScale;

extern  float   pm_accelerate;
extern  float   pm_airaccelerate;
extern  float   pm_wateraccelerate;
extern  float   pm_flyaccelerate;

extern  float   pm_friction;
extern  float   pm_waterfriction;
extern  float   pm_flightfriction;

extern  int     c_pmove;

void PM_ClipVelocity( vec3_t in, vec3_t normal, vec3_t out, float overbounce );
void PM_AddTouchEnt( int entityNum );
void PM_AddEvent( int newEvent );
void PM_AddEventWithParm( int newEvent, int parm );

qboolean    PM_SlideMove( qboolean gravity );
void        PM_StepSlideMove( qboolean gravity );

void PM_StartTorsoAnim      ( playerState_t* ps, int anim, int time );
void PM_ContinueLegsAnim    ( playerState_t* ps, int anim );
void PM_ForceLegsAnim       ( playerState_t* ps, int anim );
void PM_TorsoAnimation      ( playerState_t* ps );
void PM_SetAnim             ( playerState_t* ps, int setAnimParts,int anim,int setAnimFlags, int blendTime);

