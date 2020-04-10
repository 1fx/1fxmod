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
// cg_draw.c -- draw all of the graphical elements during
// active (after loading) gameplay

#include "cg_local.h"

#include "../ui/ui_shared.h"

// used for scoreboard
extern displayContextDef_t cgDC;

void CG_DrawRadar ( void );
void CG_DrawAutomap ( void );
void CG_DrawTimers ( void );

/*
================
CG_Draw3DModel
================
*/
void CG_Draw3DModel( float x, float y, float w, float h, qhandle_t model, qhandle_t skin, vec3_t origin, vec3_t angles ) 
{
	refdef_t		refdef;
	refEntity_t		ent;

	CG_AdjustFrom640( &x, &y, &w, &h );

	memset( &refdef, 0, sizeof( refdef ) );

	memset( &ent, 0, sizeof( ent ) );
	AnglesToAxis( angles, ent.axis );
	VectorCopy( origin, ent.origin );
	ent.hModel = model;
	ent.customSkin = skin;
	ent.renderfx = RF_NOSHADOW;		// no stencil shadows

	refdef.rdflags = RDF_NOWORLDMODEL;

	AxisClear( refdef.viewaxis );

	refdef.fov_x = 30;
	refdef.fov_y = 30;

	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	refdef.time = cg.time;

	trap_R_ClearScene();
	trap_R_AddRefEntityToScene( &ent );
	trap_R_RenderScene( &refdef );
}

void CG_Draw3DG2Model( float x, float y, float w, float h, void *ghoul2, qhandle_t skin, vec3_t origin, vec3_t angles ) 
{
	refdef_t		refdef;
	refEntity_t		ent;

	CG_AdjustFrom640( &x, &y, &w, &h );

	memset( &refdef, 0, sizeof( refdef ) );

	memset( &ent, 0, sizeof( ent ) );
	AnglesToAxis( angles, ent.axis );
	VectorCopy( origin, ent.origin );
	ent.ghoul2 = ghoul2;
	ent.customSkin = skin;
	ent.renderfx = RF_NOSHADOW;		// no stencil shadows

	refdef.rdflags = RDF_NOWORLDMODEL;

	AxisClear( refdef.viewaxis );

	refdef.fov_x = 30;
	refdef.fov_y = 30;

	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	refdef.time = cg.time;

	trap_R_ClearScene();
	trap_R_AddRefEntityToScene( &ent );
	trap_R_RenderScene( &refdef );
}

/*
==================
CG_DrawSnapshot
==================
*/
static float CG_DrawSnapshot( float y ) 
{
	char		*s;
	int			w;

	s = va( "time:%i snap:%i cmd:%i", cg.snap->serverTime, 
		cg.latestSnapshotNum, cgs.serverCommandSequence );

	w = trap_R_GetTextWidth ( s, cgs.media.hudFont, 0.53f, 0 );

	CG_DrawText ( 635 - w, y - 6, cgs.media.hudFont, 0.53f, colorWhite, s, 0, 0 );

	return 20;
}

/*
==================
CG_DrawFPS
==================
*/
#define	FPS_FRAMES	4
static float CG_DrawFPS( float y ) 
{
	char		*s;
	int			w;
	static int	previousTimes[FPS_FRAMES];
	static int	index;
	int		i, total;
	int		fps;
	static	int	previous;
	int		t, frameTime;

	// don't use serverTime, because that will be drifting to
	// correct for internet lag changes, timescales, timedemos, etc
	t = trap_Milliseconds();
	frameTime = t - previous;
	previous = t;

	previousTimes[index % FPS_FRAMES] = frameTime;
	index++;
	
	if ( index > FPS_FRAMES ) 
	{
		// average multiple frames together to smooth changes out a bit
		total = 0;
		for ( i = 0 ; i < FPS_FRAMES ; i++ ) {
			total += previousTimes[i];
		}
		if ( !total ) {
			total = 1;
		}
		fps = 1000 * FPS_FRAMES / total;

		s = va( "%i fps", fps );

		w = trap_R_GetTextWidth ( s, cgs.media.hudFont, 0.53f, 0 );

		CG_DrawText ( 635 - w, y, cgs.media.hudFont, 0.53f, g_color_table[ColorIndex(COLOR_GREEN)], s, 0,0 );
	}

	return y + 20;
}

/*
=====================
CG_DrawRadar
=====================
*/
#define RADAR_RANGE				2500
#define RADAR_RADIUS			60
#define RADAR_X					(580 - RADAR_RADIUS)
#define RADAR_Y					10
#define RADAR_CHAT_DURATION		6000

void CG_DrawRadar ( void )
{
	vec4_t			color;
	vec4_t			teamColor;
	float			arrow_w;
	float			arrow_h;
	clientInfo_t	*cl;
	clientInfo_t	*local;
	int				i;
	float			scale;

	// Make sure the radar should be showing
	if ( cg.snap->ps.stats[STAT_HEALTH] <= 0 || cg.weaponMenuUp )
	{
		return;
	}

	if ( (cg.predictedPlayerState.pm_flags & PMF_GHOST) || cgs.clientinfo[cg.predictedPlayerState.clientNum].team == TEAM_SPECTATOR )
	{
		return;
	}

	local = &cgs.clientinfo[ cg.snap->ps.clientNum ];
	if ( !local->infoValid )
	{
		return;
	}

	// Draw the radar background image
	color[0] = color[1] = color[2] = 1.0f;
	color[3] = 0.6f;
	trap_R_SetColor ( color );
	CG_DrawPic( RADAR_X, RADAR_Y, RADAR_RADIUS*2, RADAR_RADIUS*2, cgs.media.radarShader );

	arrow_w = 16 * RADAR_RADIUS / 128;
	arrow_h = 16 * RADAR_RADIUS / 128;

	// determine the color of the arrows to draw
	switch(local->team)
	{
		default:
		case TEAM_RED:
			VectorCopy ( g_color_table[ColorIndex(COLOR_RED)], teamColor );
			break;

		case TEAM_BLUE:
			VectorCopy ( g_color_table[ColorIndex(COLOR_BLUE)], teamColor );
			break;
	}

	// Darken the color a tad
	VectorScale ( teamColor, 0.5f, teamColor );
	teamColor[3] = 1.0f;

	// Draw all of the radar entities.  Draw them backwards so players are drawn last
	for ( i = cg.radarEntityCount -1 ; i >= 0 ; i-- ) 
	{	
		vec3_t		dirLook;	
		vec3_t		dirPlayer;
		float		angleLook;
		float		anglePlayer;
		float		angle;
		float		distance;
		centity_t*	cent;

		cent = cg.radarEntities[i];

		// Get the distances first
		VectorSubtract ( cg.predictedPlayerState.origin, cent->lerpOrigin, dirPlayer );		
		dirPlayer[2] = 0;
		distance = VectorNormalize ( dirPlayer );

		if ( distance > RADAR_RANGE * 0.8f) 
		{
			continue;
		}

		distance  = distance / RADAR_RANGE;
		distance *= RADAR_RADIUS;

		AngleVectors ( cg.predictedPlayerState.viewangles, dirLook, NULL, NULL );

		dirLook[2] = 0;
		anglePlayer = atan2(dirPlayer[0],dirPlayer[1]);		
		VectorNormalize ( dirLook );
		angleLook = atan2(dirLook[0],dirLook[1]);
		angle = angleLook - anglePlayer;

		switch ( cent->currentState.eType )
		{
			case ET_ITEM:
				if ( cg_items[cent->currentState.modelindex].registered )
				{
					float  x;
					float  y;
		
					x = (float)RADAR_X + (float)RADAR_RADIUS + (float)sin (angle) * distance;
					y = (float)RADAR_Y + (float)RADAR_RADIUS + (float)cos (angle) * distance;

					trap_R_SetColor ( NULL );
					CG_DrawPic ( x - 4, y - 4, 9, 9, cg_items[cent->currentState.modelindex].icon );
				}
				break;

			case ET_PLAYER:
				cl = &cgs.clientinfo[ cent->currentState.number ];

				// not valid then dont draw it
				if ( !cl->infoValid ) 
				{	
					continue;
				}

				if (cl->mLastChatTime+RADAR_CHAT_DURATION > cg.time)
				{
					vec3_t finalColor;

					scale = ((cg.time - cl->mLastChatTime) / (float)RADAR_CHAT_DURATION);
					scale *= scale;

					finalColor[0] = (teamColor[0] * (scale)) + (colorWhite[0] * (1.0-scale));
					finalColor[1] = (teamColor[1] * (scale)) + (colorWhite[1] * (1.0-scale));
					finalColor[2] = (teamColor[2] * (scale)) + (colorWhite[2] * (1.0-scale));
					finalColor[3] = teamColor[3];
					trap_R_SetColor ( finalColor );
					scale += 1.0;
				}
				else
				{
					trap_R_SetColor ( teamColor );
					scale = 1.0;
				}

				CG_DrawRotatePic2( RADAR_X + RADAR_RADIUS + sin (angle) * distance,
								   RADAR_Y + RADAR_RADIUS + cos (angle) * distance, 
								   arrow_w, arrow_h, 
								   (360 - cent->lerpAngles[YAW]) + cg.predictedPlayerState.viewangles[YAW], cgs.media.mAutomapPlayerIcon );
				break;
		}
	}

	trap_R_SetColor ( colorWhite );
	CG_DrawRotatePic2( RADAR_X + RADAR_RADIUS, RADAR_Y + RADAR_RADIUS, arrow_w, arrow_h, 
					   0, cgs.media.mAutomapPlayerIcon );
}

/*
=====================
CG_DrawTeamScores
=====================
*/
static void CG_DrawTeamScores ( float y )
{
	char		scores[2][16];
	float		w;
	const char* s;

	if ( cgs.scores1 == SCORE_NOT_PRESENT ) 
	{
		Com_sprintf (scores[0], sizeof(scores[0]), "-");
	}
	else 
	{
		Com_sprintf (scores[0], sizeof(scores[0]), "%i", cgs.scores1);
	}

	if ( cgs.scores2 == SCORE_NOT_PRESENT ) 
	{
		Com_sprintf (scores[1], sizeof(scores[1]), "-");
	}
	else 
	{
		Com_sprintf (scores[1], sizeof(scores[1]), "%i", cgs.scores2);
	}

	s = va ( "Red: %s  Blue: %s", scores[0], scores[1] );

	w = trap_R_GetTextWidth ( s, cgs.media.hudFont, 0.35f, 0 );

	CG_DrawText ( RADAR_X + RADAR_RADIUS - w / 2, y, cgs.media.hudFont, 0.35f, g_color_table[ColorIndex(COLOR_GREEN)], s, 0, DT_OUTLINE );
}

/*
=====================
CG_DrawUpperRight
=====================
*/
static void CG_DrawUpperRight( void ) 
{
	float	y;

	y = 0;

	if ( cg.scoreBoardShowing )
	{
		return;
	}

	if ( cg_drawSnapshot.integer ) 
	{
		y = CG_DrawSnapshot( y );
	}
	
	switch ( cg_drawRadar.integer  )
	{
		// Off unless the key is pressed
		case 0:
			if ( cg.showAutomap )
			{
				// If in rmg default to the auto map and if in a non team game
				// the radar is useless so default to automap as well.
				if ( cg.mInRMG || !cgs.gametypeData->teams )
				{
					CG_DrawAutomap ( );
				}
				else
				{
					CG_DrawRadar ( );
				}
			}
			break;
		
		// Draw the radar unless the automap key is down
		case 1:
			if ( cg.showAutomap && cg.mInRMG )
			{
				CG_DrawAutomap ( );
			}
			// If its a team game allow radar
			else if ( cgs.gametypeData->teams )
			{
				CG_DrawRadar ( );
			}
			break;
		
		// Draw the automap only, but if the key is pressed show the radar
		case 2:
			if ( cgs.gametypeData->teams && (cg.showAutomap || !cg.mInRMG) )
			{
				CG_DrawRadar ( );
			}
			else
			{			
				CG_DrawAutomap ( );
			}
			break;
	}	

	if ( cg_drawFPS.integer ) 
	{
		y = CG_DrawFPS( y );
		y = 18;
	}
	else
	{
		y = 10;
	}

	if ( cg_drawTeamScores.integer && cgs.gametypeData->teams )
	{
		CG_DrawTeamScores ( y );		
	}

	CG_DrawTimers ( );
}

/*
===============================================================================

LAGOMETER

===============================================================================
*/

#define	LAG_SAMPLES		128


typedef struct {
	int		frameSamples[LAG_SAMPLES];
	int		frameCount;
	int		snapshotFlags[LAG_SAMPLES];
	int		snapshotSamples[LAG_SAMPLES];
	int		snapshotCount;
} lagometer_t;

lagometer_t		lagometer;

/*
==============
CG_AddLagometerFrameInfo

Adds the current interpolate / extrapolate bar for this frame
==============
*/
void CG_AddLagometerFrameInfo( void ) 
{
	int			offset;

	offset = cg.time - cg.latestSnapshotTime;
	lagometer.frameSamples[ lagometer.frameCount & ( LAG_SAMPLES - 1) ] = offset;
	lagometer.frameCount++;
}

/*
==============
CG_AddLagometerSnapshotInfo

Each time a snapshot is received, log its ping time and
the number of snapshots that were dropped before it.

Pass NULL for a dropped packet.
==============
*/
void CG_AddLagometerSnapshotInfo( snapshot_t *snap ) 
{
	// dropped packet
	if ( !snap ) 
	{
		lagometer.snapshotSamples[ lagometer.snapshotCount & ( LAG_SAMPLES - 1) ] = -1;
		lagometer.snapshotCount++;
		return;
	}

	// add this snapshot's info
	lagometer.snapshotSamples[ lagometer.snapshotCount & ( LAG_SAMPLES - 1) ] = snap->ping;
	lagometer.snapshotFlags[ lagometer.snapshotCount & ( LAG_SAMPLES - 1) ] = snap->snapFlags;
	lagometer.snapshotCount++;
}

/*
==============
CG_DrawDisconnect

Draws a little connection icon on the screen when connection to the 
server is lost
==============
*/
static void CG_DrawDisconnect ( void ) 
{
	float		x;
	float		y;
	int			cmdNum;
	usercmd_t	cmd;

	// draw the phone jack if we are completely past our buffers
	cmdNum = trap_GetCurrentCmdNumber() - CMD_BACKUP + 1;
	trap_GetUserCmd( cmdNum, &cmd );

	// special check for map_restart
	if ( cmd.serverTime <= cg.snap->ps.commandTime || cmd.serverTime > cg.time ) 
	{	
		return;
	}

	// blink the icon
	if ( ( cg.time >> 9 ) & 1 ) 
	{
		return;
	}

	x = 640 - 76;
	y = 480 - 165;

	CG_DrawPic( x - 3, y - 3, 56, 56, cgs.media.disconnectShader );
}


static vec4_t ChatColor =
{	// green
	0.0, 1.0, 0.0, 1.0
};

/*
==============
CG_DrawAutomap

Draws the rmg map with any radar items placed on it.
==============
*/
void CG_DrawAutomap ( void ) 
{
	TCGConvertPos	*pos = (TCGConvertPos *)cg.sharedBuffer;
	clientInfo_t	*cl, *local;
	int				i;
	qhandle_t		teamIcon;
	vec4_t			teamColor, finalColor;
	int				arrow_w;
	int				arrow_h;
	int				item_w;
	int				item_h;
	float			scale;

	// Only enabled in the RMG
	if (!cg.mInRMG || cg.weaponMenuUp )
	{
		return;
	}

	// invalid parms, so don't draw!
	if (cg_automap_x.integer < 0 || cg_automap_x.integer > 640-32 ||
		cg_automap_y.integer < 0 || cg_automap_y.integer > 480-32 ||
		cg_automap_w.integer < 32 || cg_automap_w.integer > 640 ||
		cg_automap_h.integer < 32 || cg_automap_h.integer > 480 ||
		cg_automap_x.integer + cg_automap_w.integer > 640 ||
		cg_automap_y.integer + cg_automap_h.integer > 480)
	{	
		Com_Printf("Automap drawing coordinates out of range!\n");
		return;
	}

	// Register the automap image if not already registerd
	if ( !cgs.media.mAutomap )
	{
		trap_CM_TM_Upload(0, 0);		
		cgs.media.mAutomap = trap_R_RegisterShader( "gfx/menus/rmg/automap" );
	}

	finalColor[0] = finalColor[1] = finalColor[2] = 1.0;
	finalColor[3] = cg_automap_a.value;
	if (finalColor[3] > 1.0)
	{
		finalColor[3] = 1.0;
	}

	if ( cgs.media.mAutomap)
	{
		trap_R_SetColor (finalColor);
		CG_DrawPic( cg_automap_x.integer, cg_automap_y.integer, cg_automap_w.integer, cg_automap_h.integer, cgs.media.mAutomap );
	}

	local = &cgs.clientinfo[ cg.snap->ps.clientNum ];
	if ( !local->infoValid)
	{
		return;
	}

	arrow_w = 16 * cg_automap_w.integer / 512;
	arrow_h = 16 * cg_automap_h.integer / 512;
	item_w  = 24 * cg_automap_w.integer / 512;
	item_h  = 24 * cg_automap_h.integer / 512;

	switch(local->team)
	{
		case TEAM_RED:
			teamColor[0] = 1.0;
			teamColor[1] = 0.0;
			teamColor[2] = 0.0;
			break;
		case TEAM_BLUE:
			teamColor[0] = 0.0;
			teamColor[1] = 0.0;
			teamColor[2] = 1.0;
			break;
		case TEAM_SPECTATOR:
		default:
			teamColor[0] = 1.0;
			teamColor[1] = 1.0;
			teamColor[2] = 1.0;
			break;
	}
	teamIcon = cgs.media.mAutomapPlayerIcon;
	teamColor[3] = cg_automap_a.value + 0.1;
	if (teamColor[3] > 1.0)
	{
		teamColor[3] = 1.0;
	}

	// Only team games show other people on the automap
	if ( cgs.gametypeData->teams )
	{
		// Draw all of the radar entities.  Draw them backwards so players are drawn last
		for ( i = cg.radarEntityCount -1 ; i >= 0 ; i-- ) 
		{	
			centity_t*	cent;

			cent = cg.radarEntities[i];

			switch ( cent->currentState.eType )
			{
				case ET_ITEM:
					if ( cg_items[cent->currentState.modelindex].registered )
					{
						VectorCopy( cent->lerpOrigin, pos->mOrigin);
						pos->mWidth = cg_automap_w.integer;
						pos->mHeight = cg_automap_h.integer;
						trap_CM_TM_ConvertPosition();
			
						trap_R_SetColor ( NULL );
						CG_DrawPic ( pos->mX - item_w / 2 + cg_automap_x.integer, 
									 pos->mY - item_h / 2 + cg_automap_y.integer, 
									 item_w, item_h, cg_items[cent->currentState.modelindex].icon );
					}
					break;

				case ET_PLAYER:

					cl = &cgs.clientinfo[ cent->currentState.number ];

					// not valid then dont draw it
					if ( !cl->infoValid ) 
					{	
						continue;
					}

					if (cl->mLastChatTime+RADAR_CHAT_DURATION > cg.time)
					{
						vec3_t finalColor;

						scale = ((cg.time - cl->mLastChatTime) / (float)RADAR_CHAT_DURATION);
						scale *= scale;

						finalColor[0] = (teamColor[0] * (scale)) + (colorWhite[0] * (1.0-scale));
						finalColor[1] = (teamColor[1] * (scale)) + (colorWhite[1] * (1.0-scale));
						finalColor[2] = (teamColor[2] * (scale)) + (colorWhite[2] * (1.0-scale));
						finalColor[3] = teamColor[3];
						trap_R_SetColor ( finalColor );
						scale += 1.0;
					}
					else
					{
						trap_R_SetColor ( teamColor );
						scale = 1.0;
					}

					VectorCopy( cent->lerpOrigin, pos->mOrigin);
					pos->mWidth = cg_automap_w.integer;
					pos->mHeight = cg_automap_h.integer;
					trap_CM_TM_ConvertPosition();

					CG_DrawRotatePic2( pos->mX + cg_automap_x.integer, 
									   pos->mY + cg_automap_y.integer, 
									   arrow_w*scale, arrow_h*scale, 
									   (360 - cent->lerpAngles[1]) - 90.0, teamIcon );
					break;
				}
		}
	}

	VectorCopy(cg.refdef.vieworg, pos->mOrigin);
	pos->mWidth = cg_automap_w.integer;
	pos->mHeight = cg_automap_h.integer;
	trap_CM_TM_ConvertPosition();

	teamColor[0] = teamColor[1] = teamColor[2] = 1.0;
	teamColor[3] = cg_automap_a.value + 0.2;
	if (teamColor[3] > 1.0)
	{
		teamColor[3] = 1.0;
	}
	trap_R_SetColor ( teamColor );
	scale = 1.0;

	CG_DrawRotatePic2( pos->mX + cg_automap_x.integer, pos->mY + cg_automap_y.integer, arrow_w*scale, arrow_h*scale, 
		(360 - cg.refdef.viewangles[1]) - 90.0, cgs.media.mAutomapPlayerIcon );
}

#define	MAX_LAGOMETER_PING	900
#define	MAX_LAGOMETER_RANGE	300

/*
==============
CG_DrawLagometer
==============
*/
static void CG_DrawLagometer( void ) 
{
	int		a, x, y, i;
	float	v;
	float	ax, ay, aw, ah, mid, range;
	int		color;
	float	vscale;

	trap_R_SetColor( NULL );

	if ( !cg_lagometer.integer || cgs.localServer ) 
	{
		CG_DrawDisconnect();
		return;
	}

	// draw the graph
	x = 640 - 76;
	y = 480 - 165;

	CG_DrawPic( x - 3, y - 3, 56, 56, cgs.media.lagometerShader );

	ax = x;
	ay = y;
	aw = 48;
	ah = 48;
	CG_AdjustFrom640( &ax, &ay, &aw, &ah );

	color = -1;
	range = ah / 3;
	mid = ay + range;

	vscale = range / MAX_LAGOMETER_RANGE;

	// draw the frame interpoalte / extrapolate graph
	for ( a = 0 ; a < aw ; a++ ) {
		i = ( lagometer.frameCount - 1 - a ) & (LAG_SAMPLES - 1);
		v = lagometer.frameSamples[i];
		v *= vscale;
		if ( v > 0 ) {
			if ( color != 1 ) {
				color = 1;
				trap_R_SetColor( g_color_table[ColorIndex(COLOR_YELLOW)] );
			}
			if ( v > range ) {
				v = range;
			}
			trap_R_DrawStretchPic ( ax + aw - a, mid - v, 1, v, 0, 0, 0, 0, NULL, cgs.media.whiteShader );
		} else if ( v < 0 ) {
			if ( color != 2 ) {
				color = 2;
				trap_R_SetColor( g_color_table[ColorIndex(COLOR_BLUE)] );
			}
			v = -v;
			if ( v > range ) {
				v = range;
			}
			trap_R_DrawStretchPic( ax + aw - a, mid, 1, v, 0, 0, 0, 0, NULL, cgs.media.whiteShader );
		}
	}

	// draw the snapshot latency / drop graph
	range = ah / 2;
	vscale = range / MAX_LAGOMETER_PING;

	for ( a = 0 ; a < aw ; a++ ) {
		i = ( lagometer.snapshotCount - 1 - a ) & (LAG_SAMPLES - 1);
		v = lagometer.snapshotSamples[i];
		if ( v > 0 ) {
			if ( lagometer.snapshotFlags[i] & SNAPFLAG_RATE_DELAYED ) {
				if ( color != 5 ) {
					color = 5;	// YELLOW for rate delay
					trap_R_SetColor( g_color_table[ColorIndex(COLOR_YELLOW)] );
				}
			} else {
				if ( color != 3 ) {
					color = 3;
					trap_R_SetColor( g_color_table[ColorIndex(COLOR_GREEN)] );
				}
			}
			v = v * vscale;
			if ( v > range ) {
				v = range;
			}
			trap_R_DrawStretchPic( ax + aw - a, ay + ah - v, 1, v, 0, 0, 0, 0, NULL, cgs.media.whiteShader );
		} else if ( v < 0 ) {
			if ( color != 4 ) {
				color = 4;		// RED for dropped snapshots
				trap_R_SetColor( g_color_table[ColorIndex(COLOR_RED)] );
			}
			trap_R_DrawStretchPic( ax + aw - a, ay + ah - range, 1, range, 0, 0, 0, 0, NULL, cgs.media.whiteShader );
		}
	}

	trap_R_SetColor( NULL );

	CG_DrawDisconnect();
}

/*
===============================================================================

CENTER PRINTING

===============================================================================
*/


/*
==============
CG_CenterPrint

Called for important messages that should stay in the center of the screen
for a few moments
==============
*/
void CG_CenterPrint( const char *str, float scale ) 
{
	char	*s;

	Q_strncpyz( cg.centerPrint, str, sizeof(cg.centerPrint) );

	cg.centerPrintTime = cg.time;
	cg.centerPrintScale = scale;

	// count the number of lines for centering
	cg.centerPrintLines = 1;
	s = cg.centerPrint;
	while( *s ) 
	{
		if (*s == '\n')
		{
			cg.centerPrintLines++;
		}

		s++;
	}
}

/*
===================
CG_DrawCenterString
===================
*/
static void CG_DrawCenterString( void ) 
{
	char	*start;
	int		l;
	int		x, y, w;
	int		h;
	float	*color;

	if ( !cg_centertime.value )
	{
		return;
	}

	if ( !cg.centerPrintTime ) 
	{
		return;
	}

	color = CG_FadeColor( cg.centerPrintTime, 1000 * cg_centertime.value );
	if ( !color ) 
	{
		return;
	}

	trap_R_SetColor( color );

	start = cg.centerPrint;

	y = cg_centerY.integer;

	while ( 1 ) 
	{
		char linebuffer[1024];

		for ( l = 0; l < 50; l++ ) 
		{
			if ( !start[l] || start[l] == '\n' ) 
			{
				break;
			}
			
			linebuffer[l] = start[l];
		}
		linebuffer[l] = 0;

		w = trap_R_GetTextWidth(linebuffer, cgs.media.hudFont, cg.centerPrintScale, 0 );
		h = trap_R_GetTextHeight(linebuffer, cgs.media.hudFont, cg.centerPrintScale, 0 );
		x = (SCREEN_WIDTH - w) / 2;
		CG_DrawText (x, y + h, cgs.media.hudFont, cg.centerPrintScale, color, linebuffer, 0, DT_OUTLINE );
		y += h + 6;

		while ( *start && ( *start != '\n' ) ) 
		{
			start++;
		}
		
		if ( !*start ) 
		{
			break;
		}
		start++;
	}

	trap_R_SetColor( NULL );
}

/*
=================
CG_DrawCenterText
=================
*/
static void CG_DrawCenterText ( void )
{
	int w;

	if ( cgs.gametypeMessageTime < cg.time )
	{
		CG_DrawCenterString ( );
		return;
	}

	w = trap_R_GetTextWidth( cgs.gametypeMessage, cgs.media.hudFont, 0.43f, 0 );
	CG_DrawText ( (SCREEN_WIDTH - w) / 2, cg_centerY.integer, cgs.media.hudFont, 0.43f, colorWhite, cgs.gametypeMessage, 0, DT_OUTLINE );
}


/*
=================
CG_DrawCrosshair
=================
*/
static void CG_DrawCrosshair(void) 
{
	float		w;
	float		h;
	qhandle_t	hShader;
	float		x;
	float		y;
	float		scale;
	int			ca;
	qboolean	zoomed;

	if ( !cg_drawCrosshair.integer ) 
	{
		return;
	}

	// If zoomed or unzoomed with the sniper rifle dont draw the standard crosshair
	zoomed = (cg.predictedPlayerState.pm_flags&PMF_ZOOMED);
	if ( zoomed || (cg.predictedPlayerState.weapon==WP_MSG90A1 && !zoomed) )
	{
		return;
	}

	if ( cg.snap->ps.pm_type == PM_SPECTATOR ) 
	{
		return;
	}

	if ( cg.renderingThirdPerson ) 
	{
		return;
	}

	// Default crosshair color
	trap_R_SetColor( cg.crosshairRGBA );

	// Change the crosshair color when its on friendly targets
	if ( cg.crosshairColorClientNum != -1 && cgs.gametypeData->teams )	
	{
		// Is the crosshair over a friendly target?
		if ( cgs.clientinfo[ cg.crosshairColorClientNum ].team == cgs.clientinfo[ cg.snap->ps.clientNum ].team )
		{
			trap_R_SetColor( cg.crosshairFriendRGBA );
		}
	}

	w = h = cg_crosshairSize.value;

	// Determine the 
	scale = ((float)cg.predictedPlayerState.inaccuracy / ((float)weaponData[cg.predictedPlayerState.weapon].attack[ATTACK_NORMAL].maxInaccuracy+1));
	scale = 1 + scale * 1; 

	w = w * scale;
	h = h * scale;

	{
		x = cg_crosshairX.integer;
		y = cg_crosshairY.integer;
	}

	CG_AdjustFrom640( &x, &y, &w, &h );

	ca = cg_drawCrosshair.integer;
	if (ca < 0) 
	{
		ca = 0;
	}

	hShader = cgs.media.crosshairShader[ ca % NUM_CROSSHAIRS ];

	trap_R_DrawStretchPic( x + cg.refdef.x + 0.5 * (cg.refdef.width - w), 
		y + cg.refdef.y + 0.5 * (cg.refdef.height - h), 
		w, h, 0, 0, 1, 1, NULL, hShader );
}

qboolean CG_WorldCoordToScreenCoordFloat(vec3_t worldCoord, float *x, float *y)
{
	int	xcenter, ycenter;
	vec3_t	local, transformed;
	vec3_t	vfwd;
	vec3_t	vright;
	vec3_t	vup;
	float xzi;
	float yzi;

//	xcenter = cg.refdef.width / 2;//gives screen coords adjusted for resolution
//	ycenter = cg.refdef.height / 2;//gives screen coords adjusted for resolution
	
	//NOTE: did it this way because most draw functions expect virtual 640x480 coords
	//	and adjust them for current resolution
	xcenter = 640 / 2;//gives screen coords in virtual 640x480, to be adjusted when drawn
	ycenter = 480 / 2;//gives screen coords in virtual 640x480, to be adjusted when drawn

	AngleVectors (cg.refdef.viewangles, vfwd, vright, vup);

	VectorSubtract (worldCoord, cg.refdef.vieworg, local);

	transformed[0] = DotProduct(local,vright);
	transformed[1] = DotProduct(local,vup);
	transformed[2] = DotProduct(local,vfwd);		

	// Make sure Z is not negative.
	if(transformed[2] < 0.01)
	{
		return qfalse;
	}

	xzi = xcenter / transformed[2] * (90.0/cg.refdef.fov_x);
	yzi = ycenter / transformed[2] * (90.0/cg.refdef.fov_y);

	*x = xcenter + xzi * transformed[0];
	*y = ycenter - yzi * transformed[1];

	return qtrue;
}

qboolean CG_WorldCoordToScreenCoord( vec3_t worldCoord, int *x, int *y )
{
	float	xF, yF;
	qboolean retVal = CG_WorldCoordToScreenCoordFloat( worldCoord, &xF, &yF );
	*x = (int)xF;
	*y = (int)yF;
	return retVal;
}

/*
=================
CG_ScanForCrosshairEntity
=================
*/
static void CG_ScanForCrosshairEntity( void ) 
{
	trace_t		trace;
	vec3_t		start;
	vec3_t		end;
	int			content;

	VectorCopy( cg.refdef.vieworg, start );
	VectorMA( start, 131072, cg.refdef.viewaxis[0], end );

	cg.crosshairColorClientNum = -1;

	CG_Trace( &trace, start, vec3_origin, vec3_origin, end, 
		cg.snap->ps.clientNum, MASK_SHOT );
	if ( trace.entityNum >= MAX_CLIENTS ) {
		return;
	}

	// if the player is in fog, don't show it
	content = trap_CM_PointContents( trace.endpos, 0 );
	if ( content & CONTENTS_FOG ) {
		return;
	}

	// People playing a team game cant see the name of people not on their team, unless
	// they are spectating.
	if ( cgs.gametypeData->teams )
	{
		if ( cg.predictedPlayerState.pm_type != PM_SPECTATOR && !(cg.predictedPlayerState.pm_flags&PMF_GHOST) && !(cg.predictedPlayerState.pm_flags&PMF_FOLLOW) )
		{
			if ( cgs.clientinfo[ trace.entityNum ].team != cg.predictedPlayerState.persistant[PERS_TEAM] )
			{
				return;
			}
		}
	}

	// update the fade timer
	cg.crosshairClientNum = trace.entityNum;
	cg.crosshairClientTime = cg.time;
	cg.crosshairColorClientNum = trace.entityNum;
}

/*
=====================
CG_DrawCrosshairNames
=====================
*/
static void CG_DrawCrosshairNames( void ) 
{
	float		*color;
	char		*name;
	float		w;
	int			y;

	if ( !cg_drawCrosshair.integer ) 
	{
		return;
	}
	
	if ( !cg_drawCrosshairNames.integer ) 
	{
		return;
	}
	
	if ( cg.renderingThirdPerson ) 
	{
		return;
	}

	// If the player isnt spectating then make sure he cant see enemies names
	if ( cgs.gametypeData->teams )
	{
		if ( cg.predictedPlayerState.pm_type != PM_SPECTATOR && !(cg.predictedPlayerState.pm_flags&PMF_GHOST) && !(cg.predictedPlayerState.pm_flags&PMF_FOLLOW) )
		{
			if ( cgs.clientinfo[ cg.crosshairClientNum ].team != cg.predictedPlayerState.persistant[PERS_TEAM] )
			{
				return;
			}
		}
	}

	// draw the name of the player being looked at
	color = CG_FadeColor( cg.crosshairClientTime, 1000 );
	if ( !color ) {
		trap_R_SetColor( NULL );
		return;
	}

	name = cgs.clientinfo[ cg.crosshairClientNum ].name;

	if ( cg_drawCrosshairNames.integer == 2 )  // Just below the crosshair
	{
		y = (SCREEN_HEIGHT / 2) + 20 ;
	}
	else
	{
		y = 465 ;
	}


	color[3] *= 0.5f;
	w = trap_R_GetTextWidth(name, cgs.media.hudFont, 0.43f, 0);
	CG_DrawText( 320 - w / 2, y, cgs.media.hudFont, 0.43f, color, name, 0, 0 );

	trap_R_SetColor( NULL );
}


//==============================================================================

/*
=================
CG_DrawSpectator
=================
*/
static void CG_DrawSpectator(void) 
{
	const char* s;
	float		y;

	if ( cg.scoreBoardShowing )
	{
		return;
	}

	y = 415;

	// Need to be a ghost or someone following who isnt a spectator to see the respawn time
	if ( (cg.predictedPlayerState.pm_flags & PMF_GHOST) ||
	     ((cg.snap->ps.pm_flags & PMF_FOLLOW) && cgs.clientinfo[cg.clientNum].team != TEAM_SPECTATOR) )
	{
		if ( cg.predictedPlayerState.respawnTimer )
		{
			int time;
			time = cg.predictedPlayerState.respawnTimer - cg.time;
			time /= 1000;
			if ( time < 1 )
			{
				time = 1;
			}
			s = va("RESPAWN IN %i SECONDS", time );
		}
		else
		{
			s = "GHOST";
		}
	}
	else
		s = "SPECTATOR";

	CG_DrawText ( 320 - trap_R_GetTextWidth ( s, cgs.media.hudFont, 0.45f, 0 ) / 2,
					y, cgs.media.hudFont, 0.45f, colorWhite, s, 0, DT_OUTLINE );
	
	// Draw some instructions on the screen for joining the game when the client is a spectator
	if ( cgs.clientinfo[cg.clientNum].team == TEAM_SPECTATOR )
	{
		// Make sure they arent following someone and arent a ghost 
		if ( !(cg.predictedPlayerState.pm_flags & (PMF_GHOST|PMF_FOLLOW)) ) 
		{
			s = "press ESC and use the PLAYER menu to play";
			CG_DrawText( 320 - trap_R_GetTextWidth ( s, cgs.media.hudFont, 0.43f, 0 ) / 2,
							y + 15, cgs.media.hudFont, 0.43f, colorWhite, s, 0, DT_OUTLINE );
		}
	}
}

/*
=================
CG_DrawVote
=================
*/
static void CG_DrawVote(void) 
{
	char	*s;
	int		sec;

	if ( !cgs.voteTime ) 
	{
		return;
	}

	// play a talk beep whenever it is modified
	if ( cgs.voteModified ) 
	{
		cgs.voteModified = qfalse;
		trap_S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
	}

	sec = ( cgs.voteDuration - ( cg.time - cgs.voteTime ) ) / 1000;
	
	if ( sec < 0 ) 
	{
		sec = 0;
	}

	s = va("VOTE(%i):%s", sec, cgs.voteString );
	CG_DrawText ( 10, 58, cgs.media.hudFont, 0.40f, colorLtGrey, s, 0, DT_OUTLINE );

	s = va("needed:%i yes:%i no:%i", cgs.voteNeeded, cgs.voteYes, cgs.voteNo);
	CG_DrawText ( 10, 70, cgs.media.hudFont, 0.40f, colorLtGrey, s, 0, DT_OUTLINE );
}

/*
=================
CG_DrawIntermission
=================
*/
static void CG_DrawIntermission( void ) 
{
	cg.scoreFadeTime = cg.time;
	cg.scoreBoardShowing = CG_DrawScoreboard();
}

/*
=================
CG_DrawFollow
=================
*/
static qboolean CG_DrawFollow( void ) 
{
	const char *s;

	if ( cg.scoreBoardShowing )
	{
		return qfalse;
	}

	if ( !(cg.snap->ps.pm_flags & PMF_FOLLOW) ) 
	{
		return qfalse;
	}

	s = va("following %s", cgs.clientinfo[ cg.snap->ps.clientNum ].name );
	CG_DrawText ( 320 - trap_R_GetTextWidth ( s, cgs.media.hudFont, 0.63f, 0 ) / 2,
					40, cgs.media.hudFont, 0.63f, colorWhite, s, 0, DT_OUTLINE );

	CG_DrawSpectator ( );

	return qtrue;
}

/*
=================
CG_DrawWarmup
=================
*/
static void CG_DrawWarmup( void ) 
{
	int				w;
	int				sec;
	const char		*s;

	sec = cg.warmup;
	if ( !sec ) 
	{
		return;
	}

	if ( sec < 0 ) 
	{
		s = "Waiting for players";		

		w = trap_R_GetTextWidth ( s, cgs.media.hudFont, 0.53f, 0 );
		CG_DrawText ( 320 - w / 2, 24, cgs.media.hudFont, 0.53f, colorWhite, s, 0, DT_OUTLINE );

		cg.warmupCount = 0;
		return;
	}

	sec = ( sec - cg.time ) / 1000;
	if ( sec < 0 ) 
	{
		cg.warmup = 0;
		sec = 0;
	}

	s = va( "map restart in: %i", sec + 1 );
	if ( sec != cg.warmupCount ) 
	{
		cg.warmupCount = sec;
	}

	w = trap_R_GetTextWidth(s, cgs.media.hudFont, 0.53f, 0 );
	CG_DrawText (320 - w / 2, 105, cgs.media.hudFont, 0.53f, colorWhite, s, 0, DT_OUTLINE );
}

/* 
=================
CG_DrawChat
=================
*/
static void CG_DrawChat ( void ) 
{
	float	w;
	int		h;
	int		i;
	int		chatHeight;
	float	y;
	float	x;
	
	// Grab the users text height but dont let it get bigger than the define
	chatHeight = cg_chatHeight.integer;
	if (chatHeight > CHAT_HEIGHT )
	{
		chatHeight = CHAT_HEIGHT;
	}

	// Is the chat enabled right now?
	if ( chatHeight <= 0 )
	{
		return; 
	}

	// Nothing to draw 
	if (cgs.chatLastPos == cgs.chatPos ) 
	{
		return;
	}

	// Is it time to stop drawing the current chat message?
	if ( cg.time - cgs.chatTime[cgs.chatLastPos % chatHeight] > cg_chatTime.integer) 
	{
		cgs.chatLastPos++;
	}

	// Determine how tall the entire chat block is
	h = (cgs.chatPos - cgs.chatLastPos) * 15;

	if ( cg.scoreBoardShowing )
	{
		y = 395;
		x = 50;

		if ( cg.scoreBoardBottom + 10 > y - h )
		{
			y = cg.scoreBoardBottom + 10 + h;
		}

		if ( y > 480 )
		{
			y = 475;
		}
	}
	else
	{
		y = 395;
		x = 35;
	}

	// Find the greatest width out of the strings rendered
	for (w = 0, i = cgs.chatLastPos; i < cgs.chatPos; i++) 
	{
		float tw = trap_R_GetTextWidth ( cgs.chatText[i % chatHeight], cgs.media.hudFont, 0.43f, 0 );

		if (tw > w)
		{
			w = tw;
		}
	}

	for (i = cgs.chatPos - 1; i >= cgs.chatLastPos ; i--) 
	{
		qhandle_t font = cgs.media.hudFont;
		float     scale = 0.38f;

		CG_DrawText ( x, y - (cgs.chatPos - i - 1) * 15,
					  font, scale, colorWhite, cgs.chatText[i % chatHeight], 0, DT_OUTLINE );
	}
}

/* 
=================
CG_DrawTimedMenus
=================
*/
void CG_DrawTimedMenus() 
{
	if (cg.voiceTime) 
	{
		int t = cg.time - cg.voiceTime;
		if ( t > 2500 ) 
		{
			Menus_CloseByName("voiceMenu");
			trap_Cvar_Set("cl_conXOffset", "0");
			cg.voiceTime = 0;
		}
	}
}

/*
=================
CG_DrawMapChange
=================
*/
void CG_DrawMapChange ( void )
{
	const char	*s;
	int			w;
	float		x;

	// Draw a nice background image
	CG_DrawStretchPic ( 0, 0, 640, 480, 0, 0, 1, 1, colorWhite, 
						trap_R_RegisterShaderNoMip ( "gfx/menus/backdrop/pra1_sof2_logo" ) );

	s = "Server Changing Maps";
	w = trap_R_GetTextWidth(s, cgs.media.hudFont, 0.53f, 0 );
	x = (SCREEN_WIDTH - w) / 2;
	CG_DrawText (x, 360, cgs.media.hudFont, 0.53f, colorWhite, s, 0, 0 );

	s = "Please wait";
	w = trap_R_GetTextWidth(s, cgs.media.hudFont, 0.53f, 0 );
	x = (SCREEN_WIDTH - w) / 2;
	CG_DrawText (x, 400, cgs.media.hudFont, 0.53f, colorWhite, s, 0, 0 );
}

/*
==================
CG_DrawTimers

Draws the round, total, and frozer timers
==================
*/
void CG_DrawTimers ( void )
{
	int y;
	int x;

	if ( !cg_drawTimer.integer )
	{
		return;
	}

	y = 435;
	x = 120;

	if ( cg.predictedPlayerState.stats[STAT_FROZEN] )
	{
		CG_DrawTimer ( x, y, cgs.media.hudFont, 0.53f, colorGreen, DT_OUTLINE, -cg.predictedPlayerState.stats[STAT_FROZEN] );
	}
	else if ( cgs.gametypeTimerTime != 0  )
	{		
		if ( cgs.gametypeTimerTime < cg.time )
		{
			return;
		}	

		CG_DrawTimer ( x, y, cgs.media.hudFont, 0.53f, colorGreen, DT_OUTLINE, cgs.gametypeTimerTime - cg.time );
	}
}

/*
==================
CG_DrawFlashBang

Renders and handles the progression of the flashgrenade.  The flash grenade is just a white 
overlay on the whole screen which fades out over time
==================
*/
static void CG_DrawFlashBang ( void )
{
	vec4_t color;

	// Is there an active flash bang?
	if ( cg.flashbangTime + cg.flashbangFadeTime <= cg.time )
	{
		return;
	}

	// Spectators and dead people dont need to see a flash so it can stop here
	if ( (cg.predictedPlayerState.pm_flags & (PMF_GHOST|PMF_FOLLOW)) || cg.predictedPlayerState.pm_type != PM_NORMAL )
	{
		cg.flashbangTime = 0;
		return;
	}
	
	VectorCopy ( colorWhite, color );
	color[3] = cg.flashbangAlpha * (1.0f - ((float)(cg.time - cg.flashbangTime) / (float)cg.flashbangFadeTime));

	color[3] *=  2.0;
	if (color[3]<0)
	{
		color[3]=0;
	}
	
	if (color[3]>1) 
	{
		color[3]=1;
	}

	CG_FillRect ( 0, 0, 640, 480, color );
}


/*
=================
CG_Draw2D
=================
*/
static void CG_Draw2D( void ) 
{
	static qboolean oldScoreBoardShowing = qfalse;

	// if we are taking a levelshot for the menu, don't draw anything
	if ( cg.levelShot ) 
	{
		return;
	}

	if (cg.mMapChange)
	{
		CG_DrawMapChange ( );
		return;
	}

	if ( cg_draw2D.integer == 0 ) 
	{
		CG_DrawFlashBang ( );
		return;
	}

	// Handle the diabling of the console messages when in the scoreboard.  enough
	// clutter already on the scoreboard without seeing those too.
	if ( oldScoreBoardShowing != cg.scoreBoardShowing )
	{
		trap_Cvar_Set ( "con_draw", cg.scoreBoardShowing?"0":"1" );
		oldScoreBoardShowing = cg.scoreBoardShowing;
	}

	if ( cg.snap->ps.pm_type == PM_INTERMISSION ) 
	{
		CG_DrawIntermission();
		CG_DrawChat ( );
		return;
	}

	CG_DrawFlashBang ( );

	// scan the known entities to see if the crosshair is sighted on one
	CG_ScanForCrosshairEntity();

	if ( cg.snap->ps.pm_type == PM_SPECTATOR || cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR ) 
	{
		CG_DrawSpectator();
		CG_DrawCrosshair();
		CG_DrawCrosshairNames();
	} 
	else 
	{
		// don't draw any status if dead or the scoreboard is being explicitly shown
		if ( cg.snap->ps.stats[STAT_HEALTH] > 0 ) 
		{
			Menu_PaintAll();
			
			if ( !cg.showScores )
			{
				CG_DrawTimedMenus();   
				CG_DrawCrosshair();
				CG_DrawCrosshairNames();
			}
		} 
	}

	CG_DrawVote();

	CG_DrawLagometer();

	if (!cg_paused.integer) 
	{
		CG_DrawUpperRight();
	}

	if ( !CG_DrawFollow() ) 
	{
		CG_DrawWarmup();
	}

	// don't draw center string if scoreboard is up
	cg.scoreBoardShowing = CG_DrawScoreboard();
	if ( !cg.scoreBoardShowing) 
	{
		CG_DrawCenterText();
	}

	// Always Draw chat
	CG_DrawChat ( );
}

/*
=====================
CG_DrawActive

Perform all drawing needed to completely fill the screen
=====================
*/
void CG_DrawActive( stereoFrame_t stereoView ) 
{
	float		separation;
	vec3_t		baseOrg;
	float		parm1, parm2;

	// optionally draw the info screen instead
	if ( !cg.snap ) 
	{
		CG_DrawInformation();
		return;
	}

	if (cg.mMapChange)
	{
		CG_DrawMapChange ( );
		return;
	}

	// Handle the start of an inf match
	if ( cg.predictedPlayerState.persistant[PERS_TEAM] != TEAM_SPECTATOR &&
		 !(cg.predictedPlayerState.pm_flags & PMF_GHOST) && 
	     !(cg.predictedPlayerState.pm_flags & PMF_FOLLOW) )
	{
		if ( cgs.gametypeTimerTime >= cg.time && !cg.predictedPlayerState.stats[STAT_FROZEN] && cgs.gametypeTimerTime != 0 && !cg.gametypeStarted )
		{
			CG_CenterPrint( "GO!", 1.1f );
			cg.gametypeStarted = qtrue;

			trap_S_StartLocalSound ( cgs.media.goSound, CHAN_AUTO );
		}
	}

	// Popup the objectives scren if we need to
	if( cg.popupObjectives )
	{
		char temp[MAX_INFO_STRING];
		char lastobjectives[MAX_INFO_STRING];

		Com_sprintf ( lastobjectives, MAX_INFO_STRING, "%s_%s_%d", cgs.mapname, cgs.gametypeData->name, cgs.gameID );
		trap_Cvar_VariableStringBuffer ( "cg_lastobjectives", temp, MAX_INFO_STRING );

		if ( Q_stricmp ( temp, lastobjectives ) )
		{
			if ( !cgs.gametypeData->description )
			{
				// If the client isnt on a team yet and this is a team game, bring up the team dialog
				if ( cgs.gametypeData->teams && cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_SPECTATOR )
				{
					trap_SendConsoleCommand ( "ui_team;" );
				}
				// Providing outfitting is available bring up the outfitting dialog
				else if ( cgs.pickupsDisabled )
				{
					trap_SendConsoleCommand ( "ui_outfitting;" );
				}
			}
			else
			{
				// If already on a team then no need to choose a team
				if ( cgs.clientinfo[cg.clientNum].team != TEAM_SPECTATOR )
				{
					trap_Cvar_Set ( "ui_info_seenobjectives", "1" );
				}
		
				trap_SendConsoleCommand ( "ui_objectives;" );
			}

			trap_Cvar_Set ( "cg_lastobjectives", lastobjectives );
		}

		cg.popupObjectives = qfalse;
	}

	switch ( stereoView ) 
	{
		case STEREO_CENTER:
			separation = 0;
			break;
		case STEREO_LEFT:
			separation = -cg_stereoSeparation.value / 2;
			break;
		case STEREO_RIGHT:
			separation = cg_stereoSeparation.value / 2;
			break;
		default:
			separation = 0;
			Com_Error( ERR_FATAL, "CG_DrawActive: Undefined stereoView" );
	}

	// clear around the rendered view if sized down
	CG_TileClear();

	// offset vieworg appropriately if we're doing stereo separation
	VectorCopy( cg.refdef.vieworg, baseOrg );
	if ( separation != 0 ) {
		VectorMA( cg.refdef.vieworg, -separation, cg.refdef.viewaxis[1], cg.refdef.vieworg );
	}

	// When goggles are on there are a few passes to generate the effect
	if ( cg.predictedPlayerState.pm_flags & PMF_GOGGLES_ON )
	{
		switch(cg.predictedPlayerState.stats[STAT_GOGGLES])
		{
			case GOGGLES_NIGHTVISION:
				if (cg.mInRMG)
				{
					parm1 = RMG_distancecull.value;
					parm2 = 0.0;
				}
				else
				{
					parm1 = 2500;
					parm2 = 0.0;
				}
				break;
			case GOGGLES_INFRARED:
				parm1 = cgs.mIRSeeThrough;
				parm2 = cgs.mIRDist;
				break;
			default:
				parm1 = parm2 = 0.0;
				break;
		}

		trap_R_DrawVisualOverlay ( cg.predictedPlayerState.stats[STAT_GOGGLES], qtrue, parm1, parm2);

		// draw 3D view
		trap_R_RenderScene( &cg.refdef );

		trap_R_DrawVisualOverlay( cg.predictedPlayerState.stats[STAT_GOGGLES], qfalse, parm1, parm2);
	}
	// Normal rendering
	else
	{
		trap_R_RenderScene( &cg.refdef );
	}

	// restore original viewpoint if running stereo
	if ( separation != 0 ) 
	{
		VectorCopy( baseOrg, cg.refdef.vieworg );
	}

	// draw status bar and other floating elements
 	CG_Draw2D();
}

