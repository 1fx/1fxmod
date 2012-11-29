// Copyright (C) 2001-2002 Raven Software.
//
// cg_info.c -- display information while data is being loading

#include "cg_local.h"

#define MAX_LOADING_PLAYER_ICONS	16
#define MAX_LOADING_ITEM_ICONS		26

static int			loadingPlayerIconCount;
static int			loadingItemIconCount;
static qhandle_t	loadingPlayerIcons[MAX_LOADING_PLAYER_ICONS];
static qhandle_t	loadingItemIcons[MAX_LOADING_ITEM_ICONS];

void CG_LoadBar(void);

/*
======================
CG_LoadingStage
======================
*/
void CG_LoadingStage ( int stage )
{
	if ( !cg.loading )
	{
		return;
	}

	cg.loadStage = stage;

	if ( cg.loadStage )
	{
		trap_UpdateScreen();
	}
}

/*
======================
CG_LoadingString
======================
*/
void CG_LoadingString( const char *s ) 
{
	if ( !cg.loading )
	{
		return;
	}

	Q_strncpyz( cg.infoScreenText, s, sizeof( cg.infoScreenText ) );

	trap_UpdateScreen();
}

/*
===================
CG_LoadingItem
===================
*/
void CG_LoadingItem( int itemNum ) 
{
	gitem_t		*item;

	if ( !cg.loading )
	{
		return;
	}

	item = &bg_itemlist[itemNum];
	
	if ( item->icon && loadingItemIconCount < MAX_LOADING_ITEM_ICONS ) {
		loadingItemIcons[loadingItemIconCount++] = trap_R_RegisterShaderNoMip( item->icon );
	}

	CG_LoadingString( item->pickup_name );
}

/*
===================
CG_LoadingClient
===================
*/
void CG_LoadingClient( int clientNum ) 
{
	const char		*info;
	char			personality[MAX_QPATH];

	if ( !cg.loading )
	{
		return;
	}

	info = CG_ConfigString( CS_PLAYERS + clientNum );

	Q_strncpyz( personality, Info_ValueForKey( info, "n" ), sizeof(personality) );
	Q_CleanStr( personality );

	CG_LoadingString( personality );
}


/*
====================
CG_DrawInformation

Draw all the status / pacifier stuff during level loading
====================
*/
void CG_DrawInformation( void ) 
{
	static qhandle_t	levelshot = 0;
	static char			levelshotShader[MAX_QPATH] = "";

	const char	*s;
	const char	*info;
	const char	*sysInfo;
	char		shader[MAX_QPATH];
	int			y;
	int			value;
	qhandle_t	overlay;
	char		buf[1024];

	// As long as the map change flag is set we draw the map change screen
	if ( cg.mMapChange )
	{
		CG_DrawMapChange ( );
		return;
	}

	info = CG_ConfigString( CS_SERVERINFO );
	sysInfo = CG_ConfigString( CS_SYSTEMINFO );

	if ( cg.mInRMG )
	{
		const char* terrainInfo;

		terrainInfo = CG_ConfigString( CS_TERRAINS + 1 );
		if ( terrainInfo )
		{
			s = Info_ValueForKey ( terrainInfo, "terraindef" );
			Com_sprintf ( shader, sizeof(shader), "gfx/menus/levelshots/mp_rmg_%s", s );
		}
		else
		{
			Com_sprintf ( shader, sizeof(shader), "gfx/menus/levelshots/unknownmap_mp" );
		}
	}
	else
	{
		s = Info_ValueForKey( info, "mapname" );
		Com_sprintf ( shader, sizeof(shader), "gfx/menus/levelshots/%s", s );
	}

	if ( Q_stricmp ( levelshotShader, shader ) )
	{
		levelshot = trap_R_RegisterShaderNoMip( shader );
		Com_sprintf ( levelshotShader, sizeof(levelshotShader), shader );
	}

	overlay = trap_R_RegisterShaderNoMip( "gfx/menus/levelshots/unknownmap_mp" );	

	// Draw the level shot
	trap_R_SetColor( NULL );

	if ( levelshot )
	{
		vec4_t fade;

		fade[0] = 1.0f;
		fade[1] = 1.0f;
		fade[2] = 1.0f;
		fade[3] = 1.0f - ((float)cg.loadStage / 15.0f);

		CG_DrawPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, levelshot );
		
		trap_R_SetColor ( fade );

		CG_DrawPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, overlay );

		trap_R_SetColor ( NULL );
	}
	else
	{
		CG_DrawPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, overlay );
	}

	// Draw the progress bar
	CG_LoadBar();			   

	// Determine the string to print
	if ( cg.infoScreenText[0] ) 
	{
		s = va("Loading... %s", cg.infoScreenText);
	}
	else
	{
		s = "Awaiting snapshot...";
	}

	// Render the string 
	CG_DrawText( 320 - trap_R_GetTextWidth ( s, cgs.media.hudFont, 0.53f, 0 ) / 2, 198 - 32,
				 cgs.media.hudFont, 0.53f, colorWhite, s, 0, 0 );

	// draw info string information

	y = 250-32;

	// don't print server lines if playing a local game
	trap_Cvar_VariableStringBuffer( "sv_running", buf, sizeof( buf ) );
	if ( !atoi( buf ) ) 
	{
		// server hostname
		Q_strncpyz(buf, Info_ValueForKey( info, "sv_hostname" ), 1024);
		Q_CleanStr(buf);

		CG_DrawText ( 320 - trap_R_GetTextWidth ( buf, cgs.media.hudFont, 0.53f, 0 ) / 2, y,
						  cgs.media.hudFont, 0.53f, colorWhite, buf, 0, 0 );

		y += PROP_HEIGHT;

		// pure server
		s = Info_ValueForKey( sysInfo, "sv_pure" );
		if ( atoi(s) ) 
		{
			s = "Pure Server";
			CG_DrawText( 320 - trap_R_GetTextWidth ( s, cgs.media.hudFont, 0.53f, 0 ) / 2, y,
							  cgs.media.hudFont, 0.53f, colorWhite, s, 0, 0 );

			y += PROP_HEIGHT;
		}

		// server-specific message of the day
		s = CG_ConfigString( CS_MOTD );
		if ( s[0] ) 		
		{
			CG_DrawText ( 320 - trap_R_GetTextWidth ( s,  cgs.media.hudFont, 0.53f, 0 ) / 2, y,
							  cgs.media.hudFont, 0.53f, colorWhite, s, 0, 0 );

			y += PROP_HEIGHT;
		}

		// some extra space after hostname and motd
		y += 10;
	}

	// map-specific message (long map name)
	s = CG_ConfigString( CS_MESSAGE );
	if ( s[0] ) 
	{
		CG_DrawText ( 320 - trap_R_GetTextWidth ( s, cgs.media.hudFont, 0.53f, 0 ) / 2, y,
						  cgs.media.hudFont, 0.53f, colorWhite, s, 0, 0 );

		y += PROP_HEIGHT;
	}

	// cheats warning
	s = Info_ValueForKey( sysInfo, "sv_cheats" );
	if ( s[0] == '1' ) 
	{
		s = "CHEATS ARE ENABLED";
		CG_DrawText ( 320 - trap_R_GetTextWidth ( s, cgs.media.hudFont, 0.53f, 0 ) / 2, y,
						  cgs.media.hudFont, 0.53f, colorWhite, s, 0, 0 );

		y += PROP_HEIGHT;

		cg.cheats = qtrue;
	}
	else
		cg.cheats = qfalse;

	s = cgs.gametypeData->displayName;
	CG_DrawText ( 320 - trap_R_GetTextWidth ( s, cgs.media.hudFont, 0.53f, 0 ) / 2, y, 
					  cgs.media.hudFont, 0.53f, colorWhite, s, 0, 0 );
	y += PROP_HEIGHT;
		
	value = atoi( Info_ValueForKey( info, "timelimit" ) );
	if ( value ) 
	{
		s = va( "Timelimit %i", value );
		CG_DrawText ( 320 - trap_R_GetTextWidth( s, cgs.media.hudFont, 0.53f, 0 ) / 2, y,
						  cgs.media.hudFont, 0.53f, colorWhite, s, 0, 0 );
		y += PROP_HEIGHT;
	}

	value = atoi( Info_ValueForKey( info, "scorelimit" ) );
	if ( value ) 
	{
		s = va( "Scorelimit %i", value );
		CG_DrawText ( 320 - trap_R_GetTextWidth ( s, cgs.media.hudFont, 0.53f, 0 ) / 2, y, 
					  cgs.media.hudFont, 0.53f, colorWhite, s, 0, 0 );
		y += PROP_HEIGHT;
	}
}

/*
===================
CG_LoadBar
===================
*/

#define LOADBAR_CLIP_WIDTH		256
#define LOADBAR_CLIP_HEIGHT		64
#define LOADBAR_BULLET_WIDTH	16
#define LOADBAR_BULLET_HEIGHT	64

void CG_LoadBar(void)
{
	int			x,y,i;

	y = 50;
	x = (640 - LOADBAR_CLIP_WIDTH) / 2;

	for (i=0;i < cg.loadStage; i++ )
	{
		CG_DrawPic(x + (i*LOADBAR_BULLET_WIDTH), y, LOADBAR_BULLET_WIDTH, LOADBAR_BULLET_HEIGHT, cgs.media.loadBulletShader );
	}

	CG_DrawPic ( x, y, LOADBAR_CLIP_WIDTH, LOADBAR_CLIP_HEIGHT, cgs.media.loadClipShader );
}
