// Copyright (C) 2001-2002 Raven Software.
//
// cg_drawtools.c -- helper functions called by cg_draw, cg_scoreboard, cg_info, etc
#include "cg_local.h"

/*
================
CG_AdjustFrom640

Adjusted for resolution and screen aspect ratio
================
*/
void CG_AdjustFrom640( float *x, float *y, float *w, float *h ) {
#if 0
	// adjust for wide screens
	if ( cgs.glconfig.vidWidth * 480 > cgs.glconfig.vidHeight * 640 ) {
		*x += 0.5 * ( cgs.glconfig.vidWidth - ( cgs.glconfig.vidHeight * 640 / 480 ) );
	}
#endif
	// scale for screen sizes
	*x *= cgs.screenXScale;
	*y *= cgs.screenYScale;
	*w *= cgs.screenXScale;
	*h *= cgs.screenYScale;
}

/*
================
CG_FillRect

Coordinates are 640*480 virtual values
=================
*/
void CG_FillRect( float x, float y, float width, float height, const float *color ) {
	trap_R_SetColor( color );

	CG_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, 0, 0, 0, 0, NULL, cgs.media.whiteShader );

	trap_R_SetColor( NULL );
}

/*
================
CG_DrawSides

Coords are virtual 640x480
================
*/
void CG_DrawSides(float x, float y, float w, float h, float size) {
	CG_AdjustFrom640( &x, &y, &w, &h );
	size *= cgs.screenXScale;
	trap_R_DrawStretchPic( x, y, size, h, 0, 0, 0, 0, NULL, cgs.media.whiteShader );
	trap_R_DrawStretchPic( x + w - size, y, size, h, 0, 0, 0, 0, NULL, cgs.media.whiteShader );
}

void CG_DrawTopBottom(float x, float y, float w, float h, float size) {
	CG_AdjustFrom640( &x, &y, &w, &h );
	size *= cgs.screenYScale;
	trap_R_DrawStretchPic( x, y, w, size, 0, 0, 0, 0, NULL, cgs.media.whiteShader );
	trap_R_DrawStretchPic( x, y + h - size, w, size, 0, 0, 0, 0, NULL, cgs.media.whiteShader );
}
/*
================
UI_DrawRect

Coordinates are 640*480 virtual values
=================
*/
void CG_DrawRect( float x, float y, float width, float height, float size, const float *color ) {
	trap_R_SetColor( color );

  CG_DrawTopBottom(x, y, width, height, size);
  CG_DrawSides(x, y, width, height, size);

	trap_R_SetColor( NULL );
}



/*
================
CG_DrawPic

Coordinates are 640*480 virtual values
=================
*/
void CG_DrawPic( float x, float y, float width, float height, qhandle_t hShader ) {
	CG_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, NULL, hShader );
}


/*
================
CG_DrawStretchPic

Coordinates are 640*480 virtual values
=================
*/
void CG_DrawStretchPic( 
	float x, 
	float y, 
	float width, 
	float height, 
	float sx, 
	float sy, 
	float sw, 
	float sh, 
	const float* color,
	qhandle_t hShader 
	) 
{
	CG_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, sx, sy, sw, sh, NULL, hShader );
}

/*
================
CG_DrawRotatePic

Coordinates are 640*480 virtual values
A width of 0 will draw with the original image width
rotates around the upper right corner of the passed in point
=================
*/
void CG_DrawRotatePic( float x, float y, float width, float height,float angle, qhandle_t hShader ) {
	CG_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawRotatePic( x, y, width, height, 0, 0, 1, 1, angle, hShader );
}

/*
================
CG_DrawRotatePic2

Coordinates are 640*480 virtual values
A width of 0 will draw with the original image width
Actually rotates around the center point of the passed in coordinates
=================
*/
void CG_DrawRotatePic2( float x, float y, float width, float height,float angle, qhandle_t hShader ) {
	CG_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawRotatePic2( x, y, width, height, 0, 0, 1, 1, angle, hShader );
}

/*
=================
CG_DrawStrlen

Returns character count, skiping color escape codes
=================
*/
int CG_DrawStrlen( const char *str ) {
	const char *s = str;
	int count = 0;

	while ( *s ) {
		if ( Q_IsColorString( s ) ) {
			s += 2;
		} else {
			count++;
			s++;
		}
	}

	return count;
}

/*
=============
CG_TileClearBox

This repeats a 64*64 tile graphic to fill the screen around a sized down
refresh window.
=============
*/
static void CG_TileClearBox( int x, int y, int w, int h, qhandle_t hShader ) {
	float	s1, t1, s2, t2;

	s1 = x/64.0;
	t1 = y/64.0;
	s2 = (x+w)/64.0;
	t2 = (y+h)/64.0;
	trap_R_DrawStretchPic( x, y, w, h, s1, t1, s2, t2, NULL, hShader );
}



/*
==============
CG_TileClear

Clear around a sized down screen
==============
*/
void CG_TileClear( void ) 
{
	int		top, bottom, left, right;
	int		w, h;

	w = cgs.glconfig.vidWidth;
	h = cgs.glconfig.vidHeight;

	if ( cg.refdef.x == 0 && cg.refdef.y == 0 && 
		cg.refdef.width == w && cg.refdef.height == h ) {
		return;		// full screen rendering
	}

	top = cg.refdef.y;
	bottom = top + cg.refdef.height-1;
	left = cg.refdef.x;
	right = left + cg.refdef.width-1;

	// clear above view screen
	CG_TileClearBox( 0, 0, w, top, cgs.media.backTileShader );

	// clear below view screen
	CG_TileClearBox( 0, bottom, w, h - bottom, cgs.media.backTileShader );

	// clear left of view screen
	CG_TileClearBox( 0, top, left, bottom - top + 1, cgs.media.backTileShader );

	// clear right of view screen
	CG_TileClearBox( right, top, w - right, bottom - top + 1, cgs.media.backTileShader );
}



/*
================
CG_FadeColor
================
*/
float *CG_FadeColor( int startMsec, int totalMsec ) {
	static vec4_t		color;
	int			t;

	if ( startMsec == 0 ) {
		return NULL;
	}

	t = cg.time - startMsec;

	if ( t >= totalMsec ) {
		return NULL;
	}

	// fade out
	if ( totalMsec - t < FADE_TIME ) {
		color[3] = ( totalMsec - t ) * 1.0/FADE_TIME;
	} else {
		color[3] = 1.0;
	}
	color[0] = color[1] = color[2] = 1;

	return color;
}


/*
================
CG_TeamColor
================
*/
float *CG_TeamColor( int team ) {
	static vec4_t	red = {1, 0.2f, 0.2f, 1};
	static vec4_t	blue = {0.2f, 0.2f, 1, 1};
	static vec4_t	other = {1, 1, 1, 1};
	static vec4_t	spectator = {0.7f, 0.7f, 0.7f, 1};

	switch ( team ) {
	case TEAM_RED:
		return red;
	case TEAM_BLUE:
		return blue;
	case TEAM_SPECTATOR:
		return spectator;
	default:
		return other;
	}
}


/*
=================
CG_GetColorForHealth
=================
*/
void CG_GetColorForHealth( vec4_t color, int health, int armor ) 
{
	int				count;
	int				max;

	VectorCopy ( colorWhite, color );
	
	color[3] = 1.0f;

	// calculate the total points of damage that can
	// be sustained at the current health / armor level
	if ( health <= 0 ) 
	{
		VectorCopy ( colorBlack, color );
		return;
	}

	count = armor;
	max = health * ARMOR_PROTECTION / ( 1.0 - ARMOR_PROTECTION );

	if ( max < count ) 
	{
		count = max;
	}

	health += count;

	// set the color based on health
	color[0] = 1.0f;
	if ( health >= 100 ) 
	{
		color[2] = 1.0f;
	} 
	else if ( health < 66 ) 
	{
		color[2] = 0;
	} 
	else 
	{
		color[2] = ( health - 66 ) / 33.0;
	}

	if ( health > 60 ) 
	{
		color[1] = 1.0f;
	} 
	else if ( health < 30 ) 
	{
		color[1] = 0;
	} 
	else 
	{
		color[1] = ( health - 30 ) / 30.0;
	}
}

/*
=====================
CG_DrawText

Renders text on the screen
=====================
*/
void CG_DrawText ( float x, float y, qhandle_t font, float scale, vec4_t color, const char* text, int limit, int flags )
{
	x	  *= cgs.screenXScale;
	y	  *= cgs.screenYScale;
	scale *= (cgs.screenXScale);

	trap_R_DrawText ( (int)x, (int)y, font, scale, color, text, limit, flags );
}

/*
=====================
CG_DrawTimer

Draws a timer on the screen with the given parameters
=====================
*/
void CG_DrawTimer ( float x, float y, qhandle_t font, float scale, vec4_t color, int flags, int msec )
{
	const char* s;

	if ( msec )
	{
		int		 mins;
		int		 seconds;
		int		 tens;
		qboolean neg;

		// Remember the milliseconds were negative
		// before making them positive again
		if ( msec < 0 )
		{
			msec *= -1;
			neg  = qtrue;
		}
		else
		{
			neg = qfalse;
		}

		seconds = msec / 1000;
		mins = seconds / 60;
		seconds -= mins * 60;
		tens = seconds / 10;
		seconds -= tens * 10;

		s = va( "%s%i:%i%i", neg?"-":"",mins, tens, seconds );
	}
	else
	{
		s = "------";
	}

	CG_DrawText ( x, y, font, scale, color, s, 0, flags );
}

/*
=====================
CG_DrawTextWithCursor

Renders text on the screen with a blinking cursor
=====================
*/
void CG_DrawTextWithCursor ( float x, float y, qhandle_t font, float scale, vec4_t color, const char* text, int limit, int flags, int cursorPos, char cursor )
{
	x	  *= cgs.screenXScale;
	y	  *= cgs.screenYScale;
	scale *= (cgs.screenXScale);

	trap_R_DrawTextWithCursor ( (int)x, (int)y, font, scale, color, text, limit, flags, cursorPos, cursor );
}
