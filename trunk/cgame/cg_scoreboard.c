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
// cg_scoreboard -- draw the scoreboard on top of the game screen
#include "cg_local.h"


#define SB_HEADER			86
#define SB_TOP				(SB_HEADER+32)

// Where the status bar starts, so we don't overwrite it
#define SB_STATUSBAR		420

#define SB_MAXCLIENTS_NORMAL  12
#define SB_MAXCLIENTS_MORE	  26
#define SB_MAXCLIENTS_ALOT	  32

// Used when interleaved
#define SB_HEADER_WIDTH		(580)
#define SB_HEADER_HEIGHT	30
#define SB_HEADER_X			((640-SB_HEADER_WIDTH)/2)
#define SB_HEADER_Y			86
#define SB_SCORELINE_X		(SB_HEADER_X+30)
#define SB_SCORELINE_Y		120
#define SB_SCORELINE_WIDTH	(SB_HEADER_WIDTH-60)
#define SB_NAME_X			(SB_SCORELINE_X)
#define SB_SCORE_X			(SB_SCORELINE_X+287)
#define SB_KILLS_X			(SB_SCORELINE_X+335)
#define SB_DEATHS_X			(SB_SCORELINE_X+384)
#define	SB_PING_X			(SB_SCORELINE_X+440)
#define SB_TIME_X			(SB_SCORELINE_X+489)

int		sb_lineHeight;
int		sb_maxClients;
float	sb_nameFontScale;
float	sb_numberFontScale;
float	sb_readyFontScale;

/*
=================
CG_DrawClientScore
=================
*/
static void CG_DrawClientScore( float x, float y, float w, score_t* score, float* color )
{
	clientInfo_t*	ci;
	vec4_t			dataColor;
	vec4_t			nameColor;
	const char*		s;
	float			f;

	nameColor[3] = dataColor[3] = 1.0f;
	
	// Validate the score
	if ( score->client < 0 || score->client >= cgs.maxclients ) 
	{
		Com_Printf( "Bad score->client: %i\n", score->client );
		return;
	}

	// Convienience	
	ci = &cgs.clientinfo[score->client];

	CG_DrawPic ( x - 5, y, w, sb_lineHeight, cgs.media.scoreboardLine );

	// highlight your position
	if ( score->client == cg.snap->ps.clientNum ) 
	{
		vec4_t hcolor;

		hcolor[0] = 1.0f;
		hcolor[1] = 1.0f;
		hcolor[2] = 1.0f;
		hcolor[3] = .10f;

		CG_FillRect( x - 5, y, w, sb_lineHeight, hcolor );

		VectorSet ( nameColor, 1.0f, 1.0f, 1.0f );
		VectorSet ( dataColor, 0.5f, 0.5f, 0.5f );
	}
	else if ( (cg.snap->ps.pm_type == PM_DEAD) && score->client == cg.snap->ps.persistant[PERS_ATTACKER] )
	{
		vec4_t hcolor;

		hcolor[0] = 1.0f;
		hcolor[1] = 1.0f;
		hcolor[2] = 1.0f;
		hcolor[3] = .10f;

		CG_FillRect( x - 5, y, w, sb_lineHeight, hcolor );

		VectorCopy ( color, nameColor );
		VectorSet ( dataColor, 0.5f, 0.5f, 0.5f );
	}
	else
	{
		VectorCopy ( color, nameColor );
		VectorSet ( dataColor, 0.3f, 0.3f, 0.3f );

		if ( ci->ghost )
		{
			VectorScale ( nameColor, 0.6f, nameColor );
		}
	}

	CG_DrawText( x, y, cgs.media.hudFont, sb_nameFontScale, nameColor, ci->name, 24, DT_OUTLINE );

	s = va("%i", score->score );
	f = trap_R_GetTextWidth ( s, cgs.media.hudFont, sb_nameFontScale, 0 );
	CG_DrawText( x + w - 10 - f, y, cgs.media.hudFont, sb_nameFontScale, nameColor, va("%i", score->score), 0, DT_OUTLINE );

	// Draw skull if dead and not in intermission
	if ( cg.snap->ps.pm_type == PM_INTERMISSION  )
	{
		if ( cg.snap->ps.stats[ STAT_CLIENTS_READY ] & ( 1 << score->client ) )
		{
			vec3_t deadColor;
			deadColor[0] = 0.60f;
			deadColor[1] = 0.60f;
			deadColor[2] = 0.60f;
			deadColor[3] = 1.0f;
			CG_DrawText( x + w - 70, y + 3, cgs.media.hudFont, sb_readyFontScale, deadColor, "READY", 0, DT_OUTLINE );
		}
	}
	else if ( ci->ghost )
	{
		CG_DrawPic ( x + w - 70, y + 1, sb_numberFontScale * 0.8f, sb_numberFontScale * 0.8f, cgs.media.deadShader );			
	}
	// Draw any gametype items the guy is carrying
	else
	{
		float xx = x + w - 70;
		int   i;
		for ( i = 0; i < MAX_GAMETYPE_ITEMS; i ++ )
		{
			centity_t* cent;

			cent = CG_GetEntity ( score->client);

			// No have item, no draw it
			if ( !(ci->gametypeitems & (1<<i) ) )
			{
				continue;
			}

			if ( !cg_items[MODELINDEX_GAMETYPE_ITEM+i].icon )
			{
				continue;
			}

			CG_DrawPic ( xx, y + 1, sb_numberFontScale * 0.8f, sb_numberFontScale * 0.8f, cg_items[MODELINDEX_GAMETYPE_ITEM+i].icon );

			xx += sb_numberFontScale;
		}
	}		


	s = va("%i/%i", score->kills, score->deaths);
	f = trap_R_GetTextWidth ( s, cgs.media.hudFont, sb_readyFontScale, 0 );
	CG_DrawText( x + w - 10 - f, y + sb_numberFontScale, cgs.media.hudFont, sb_readyFontScale, dataColor, s, 0, 0 );

	CG_DrawText( x, y + sb_numberFontScale, cgs.media.hudFont, sb_readyFontScale, dataColor, va("id: %i", score->client ), 0, 0 );
	CG_DrawText( x + 40, y + sb_numberFontScale, cgs.media.hudFont, sb_readyFontScale, dataColor, va("ping: %i", score->ping ), 0, 0 );
	CG_DrawText( x + 95, y + sb_numberFontScale, cgs.media.hudFont, sb_readyFontScale, dataColor, va("time: %i", score->time ), 0, 0 );

	if ( score->teamkillDamage )
	{
		CG_DrawText( x + 150, y + sb_numberFontScale, cgs.media.hudFont, sb_readyFontScale, dataColor, va("tk: %i%%", score->teamkillDamage ), 0, 0 );
	}
}

/*
=================
CG_TeamScoreboard
=================
*/
static int CG_TeamScoreboard( float x, float y, float w, team_t team ) 
{
	int				i;
	int				skipped;
	float			color[4];
	int				count;
	clientInfo_t	*ci;
	const char*		s;
	int				players;
	qboolean		drawnClient;

	// Do we make sure the current client is drawn?
	drawnClient = qtrue;
	if ( cg.scores [ cg.snap->ps.clientNum ].team == team )
	{
		drawnClient = qfalse;
	}

	// Determine the color for this team
	switch ( team )
	{
		case TEAM_RED:
			VectorCopy4 ( g_color_table[ColorIndex(COLOR_RED)], color );
			break;

		case TEAM_BLUE:
			VectorCopy4 ( g_color_table[ColorIndex(COLOR_BLUE)], color );
			break;

		case TEAM_FREE:
			VectorCopy4 ( g_color_table[ColorIndex(COLOR_GREEN)], color );
			break;

		case TEAM_SPECTATOR:
		default:
			VectorCopy4 ( colorWhite, color );
			break;
	}

	// Draw as many clients as we can for this team
	for ( skipped = -1, count = 0, i = 0 ; i < cg.numScores && count < sb_maxClients ; i++ ) 
	{
		score_t*	score;

		score = &cg.scores[i];
		ci    = &cgs.clientinfo[ score->client ];

		if ( team != score->team ) 
		{
			continue;
		}

		if ( count == sb_maxClients - 1 && !drawnClient )
		{
			if ( score->client != cg.snap->ps.clientNum )
			{
				skipped = i;
				continue;
			}
			
			drawnClient = qtrue; 
		}

		CG_DrawClientScore( x, y + SB_HEADER_HEIGHT + sb_lineHeight * count, w, score, color );

		count++;
	}

	if ( skipped != -1 &&  count < sb_maxClients )
	{
		CG_DrawClientScore( x, y + SB_HEADER_HEIGHT + sb_lineHeight * count, w, &cg.scores[skipped], color );

		count++;
	}

	s = "";
	switch ( team )
	{
		case TEAM_RED:
			s = "RED TEAM";
			players = ui_info_redcount.integer;
			break;

		case TEAM_BLUE:
			s = "BLUE TEAM";
			players = ui_info_bluecount.integer;
			break;

		case TEAM_FREE:
			s = "PLAYERS";
			players = ui_info_freecount.integer;
			break;

		default:
		case TEAM_SPECTATOR:
			s = "SPECTATORS";
			players = ui_info_speccount.integer;
			break;
	}

	// Use the same team color here, but alpha it a bit.
	color[3] = 0.6f;

	// Draw the header information for this team
	CG_DrawPic ( x - 5, y, w, 25, cgs.media.scoreboardHeader );
	CG_FillRect( x - 5, y, w, 25, color );
	CG_DrawText ( x, y, cgs.media.hudFont, 0.40f, colorWhite, va("%s", s), 0, 0 );
	CG_DrawText ( x, y + 13, cgs.media.hudFont, 0.30f, colorWhite, va("players: %d", players), 0, 0 );

	// Draw the totals if this is the red or blue team
	if ( (team == TEAM_RED || team == TEAM_BLUE))
	{
		const char* s;
		float		f;

		s = va("%d",(cg.teamScores[team-TEAM_RED]));
		f = trap_R_GetTextWidth ( s, cgs.media.hudFont, 0.43f, 0 );
		CG_DrawText ( x + w - 10 - f, y, cgs.media.hudFont, 0.43f, colorWhite, s, 0, DT_OUTLINE );
	}

	if ( count )
	{
		CG_DrawPic ( x - 5, y + SB_HEADER_HEIGHT + sb_lineHeight * count, w, sb_lineHeight, cgs.media.scoreboardFooter );
	}
	
	y = count * sb_lineHeight + y + 10;

	if ( y > cg.scoreBoardBottom )
	{
		cg.scoreBoardBottom = y;
	}

	return y;
}

/*
=================
CG_DrawNormalScoreboard

Draws a scoreboard that has no teams
=================
*/
qboolean CG_DrawNormalScoreboard ( float y )
{
	cg.scoreBoardBottom = 0;

	// DRaw the game timer and the game type
	CG_DrawText ( 385,  y - 14, cgs.media.hudFont, 0.38f, colorLtGrey, "Game Time:", 0, DT_OUTLINE );
	CG_DrawTimer ( 455,  y - 14, cgs.media.hudFont, 0.38f, colorLtGrey, DT_OUTLINE, cg.time - cgs.levelStartTime );
	CG_DrawText ( 150,  y - 14, cgs.media.hudFont, 0.38f, colorLtGrey, cgs.gametypeData->displayName, 0, DT_OUTLINE );

	if ( ui_info_speccount.integer )
	{
		CG_FillRect ( 0, 470, 640, 10, colorBlack );
		CG_DrawText ( 5, 470, cgs.media.hudFont, 0.30f, colorWhite, va("SPECTATORS:   %s", cg.scoreBoardSpectators), 0, 0 );
	}

	if ( ui_info_freecount.integer > 10 ) 
	{
		sb_maxClients	   = 16;
		sb_lineHeight	   = 20;
		sb_nameFontScale   = 0.35f;
		sb_readyFontScale  = 0.30f;
		sb_numberFontScale = trap_R_GetTextHeight ( "W", cgs.media.hudFont, sb_nameFontScale, 0 );
	} 
	else
	{
		sb_maxClients	   = 10;
		sb_lineHeight	   = 30;
		sb_nameFontScale   = 0.43f;
		sb_readyFontScale  = 0.30f;
		sb_numberFontScale = trap_R_GetTextHeight ( "W", cgs.media.hudFont, sb_nameFontScale, 0 ) + 4;
	}

	CG_TeamScoreboard ( 150, y, 350, TEAM_FREE );

	return qtrue;
}

/*
=================
CG_DrawTeamScoreboard

Draw the normal in-game scoreboard
=================
*/
qboolean CG_DrawTeamScoreboard( float y ) 
{
	qboolean redFirst = qfalse;

	cg.scoreBoardBottom = 0;

	// DRaw the game timer and the game type
	CG_DrawText ( 470,  y - 14, cgs.media.hudFont, 0.38f, colorLtGrey, "Game Time:", 0, DT_OUTLINE );
	CG_DrawTimer ( 540,  y - 14, cgs.media.hudFont, 0.38f, colorLtGrey, DT_OUTLINE, cg.time - cgs.levelStartTime );
	CG_DrawText ( 60,  y - 14, cgs.media.hudFont, 0.38f, colorLtGrey, cgs.gametypeData->displayName, 0, DT_OUTLINE );

	if ( ui_info_speccount.integer )
	{
		CG_FillRect ( 0, 470, 640, 10, colorBlack );
		CG_DrawText ( 5, 470, cgs.media.hudFont, 0.30f, colorWhite, va("SPECTATORS:   %s", cg.scoreBoardSpectators), 0, 0 );
	}

	if ( ui_info_redcount.integer > 10 || ui_info_bluecount.integer > 10 ) 
	{
		sb_maxClients	   = 16;
		sb_lineHeight	   = 20;
		sb_nameFontScale   = 0.35f;
		sb_readyFontScale  = 0.30f;
		sb_numberFontScale = trap_R_GetTextHeight ( "W", cgs.media.hudFont, sb_nameFontScale, 0 );
	} 
	else
	{
		sb_maxClients	   = 10;
		sb_lineHeight	   = 30;
		sb_nameFontScale   = 0.43f;
		sb_readyFontScale  = 0.30f;
		sb_numberFontScale = trap_R_GetTextHeight ( "W", cgs.media.hudFont, sb_nameFontScale, 0 ) + 4;
	}

	// If there are more scores than the scoreboard can show then show the 
	// players team first rather than the winning team
	if ( cgs.clientinfo[cg.clientNum].team == TEAM_SPECTATOR )
	{
		if ( cg.teamScores[0] >= cg.teamScores[1] )
		{
			redFirst = qtrue;
		}
	}
	else if ( cgs.clientinfo[cg.clientNum].team == TEAM_RED )
	{
		redFirst = qtrue;
	}

	if ( redFirst ) 
	{
		CG_TeamScoreboard( 50, y, 265, TEAM_RED );
		CG_TeamScoreboard( 330, y, 265, TEAM_BLUE );
	}
	else
	{
		CG_TeamScoreboard( 330, y, 265, TEAM_RED );
		CG_TeamScoreboard( 50, y, 265, TEAM_BLUE );
	}

	return qtrue;
}

/*
=================
CG_DrawScoreboard

Draws either a team or a normal scoreboard
=================
*/
qboolean CG_DrawScoreboard ( void )
{
	float y;
	float w;

	// don't draw amuthing if the menu or console is up
	if ( cg_paused.integer ) 
	{
		cg.deferredPlayerLoading = 0;
		return qfalse;
	}

	// don't draw scoreboard during death while warmup up
	if ( cg.warmup && !cg.showScores ) 
	{
		return qfalse;
	}

	if ( !cg.showScores										&& 
		 cg.predictedPlayerState.pm_type != PM_DEAD			&&
		 cg.predictedPlayerState.pm_type != PM_INTERMISSION    ) 
	{
		return qfalse;
	}

	// scoreboard
	y = 45;

	if ( cgs.gametypeData->teams )
	{
		if ( ui_info_redcount.integer < 10 && ui_info_bluecount.integer < 10 )
		{
			y += 50;
		}
	}
	else if ( ui_info_freecount.integer < 10 )
	{
		y += 50;
	}

	// Draw any gameover text
	if ( cgs.gameover[0] )
	{
		w = trap_R_GetTextWidth ( cgs.gameover, cgs.media.hudFont, 0.48f, 0 );
		CG_DrawText ( 320 - w / 2, y - 30, cgs.media.hudFont, 0.48f, colorWhite, cgs.gameover, 0, DT_OUTLINE );
	}
	else if ( cgs.gametypeMessageTime > cg.time )
	{
		w = trap_R_GetTextWidth ( cgs.gametypeMessage, cgs.media.hudFont, 0.48f, 0 );
		CG_DrawText ( 320 - w / 2, y - 30, cgs.media.hudFont, 0.48f, colorWhite, cgs.gametypeMessage, 0, DT_OUTLINE );
	}
	// Should we draw who killed you?
	else if ( cg.snap->ps.pm_type == PM_DEAD && 
			  cg.snap->ps.persistant[PERS_ATTACKER] < MAX_CLIENTS &&
			  cg.snap->ps.persistant[PERS_ATTACKER] != cg.snap->ps.clientNum )
	{
		const char* s;
		s = va("Killed by %s", cgs.clientinfo[cg.snap->ps.persistant[PERS_ATTACKER]].name );
		w = trap_R_GetTextWidth ( s, cgs.media.hudFont, 0.48f, 0 );
		CG_DrawText ( 320 - w / 2, y - 30, cgs.media.hudFont, 0.48f, colorWhite, s, 0, DT_OUTLINE );
	}
	else if ( cgs.gametypeData->teams )
	{
		const char* s;
		if ( cg.teamScores[0] == cg.teamScores[1] )
		{
			s = va ( "Game Tied at %d", cg.teamScores[0] );
		}
		else if ( cg.teamScores[0] > cg.teamScores[1] )
		{
			s = va ( "Red leads Blue by %d", cg.teamScores[0] - cg.teamScores[1] );
		}
		else
		{
			s = va ( "Blue leads Red by %d", cg.teamScores[1] - cg.teamScores[0] );
		}

		w = trap_R_GetTextWidth ( s, cgs.media.hudFont, 0.48f, 0 );
		CG_DrawText ( 320 - w / 2, y - 30, cgs.media.hudFont, 0.48f, colorWhite, s, 0, DT_OUTLINE );
	}		

	// load any models that have been deferred
	cg.deferredPlayerLoading++;

	if ( cgs.gametypeData->teams )
	{
		return CG_DrawTeamScoreboard ( y );
	}

	return CG_DrawNormalScoreboard ( y ); 
}
