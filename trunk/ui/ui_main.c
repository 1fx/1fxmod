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
// Main user interface 
//

#include "ui_local.h"
#include "../game/bg_public.h"

uiInfo_t uiInfo;

static const char *MonthAbbrev[] = 
{
	"Jan","Feb","Mar",
	"Apr","May","Jun",
	"Jul","Aug","Sep",
	"Oct","Nov","Dec"
};


static const char *skillLevels[] = 
{
	"Amature",
	"Gun For Hire",
	"Consultant",
	"Soldier of Fortune",
};

static const char *netSources[] = 
{
	"Local",
	"Internet",
	"Favorites"
};

static const serverFilter_t serverFilters[] = 
{
	{"All", "" },
	{"Soldier of Fortune 2", "" },
};

static char* netnames[] = 
{
	"???",
	"UDP",
	"IPX",
	NULL
};

static const int numNetSources				= sizeof(netSources) / sizeof(const char*);
static const int numSkillLevels				= sizeof(skillLevels) / sizeof(const char*);
static const int numServerFilters			= sizeof(serverFilters) / sizeof(serverFilter_t);

static int gamecodetoui[] = {4,2,3,0,5,1,6};
static int uitogamecode[] = {4,6,2,3,1,5,7};

static playerInfo_t		playerInfo = { 0 };


static void			UI_StartServerRefresh(qboolean full);
static void			UI_StopServerRefresh( void );
static void			UI_DoServerRefresh( void );
static void			UI_FeederSelection(float feederID, int index);
static void			UI_BuildServerDisplayList(qboolean force);
static void			UI_BuildServerStatus(qboolean force);
static void			UI_BuildFindPlayerList(qboolean force);
static int QDECL	UI_ServersQsortCompare( const void *arg1, const void *arg2 );
static int			UI_MapCountByGameType(void);
static void			UI_ParseTeamInfo(const char *teamFile);
static const char*	UI_SelectedMap(int index, int *actual);
static const char*	UI_SelectedHead(int index, int *actual);
static int			UI_GetIndexFromSelection(int actual);
static void			UI_DrawVersionDownloadProgress(rectDef_t *rect, qhandle_t font, float scale, vec4_t color );
static int			UI_MapCountForVote ( void );

/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .qvm file
================
*/
vmCvar_t  ui_new;
vmCvar_t  ui_debug;
vmCvar_t  ui_initialized;
vmCvar_t  ui_sof2FirstRun;

void		_UI_Init		 ( qboolean );
void		_UI_Shutdown	 ( void );
qboolean	_UI_KeyEvent	 ( int key, qboolean down );
qboolean	_UI_MouseEvent	 ( int dx, int dy );
void		_UI_Refresh		 ( int realtime );

int vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11  ) 
{
	switch ( command ) 
	{
		case UI_GETAPIVERSION:
			return UI_API_VERSION;

		case UI_INIT:
			_UI_Init(arg0);
			return 0;

		case UI_SHUTDOWN:
			_UI_Shutdown();
			return 0;

		case UI_KEY_EVENT:
			return _UI_KeyEvent( arg0, arg1 );

		case UI_MOUSE_EVENT:
			return _UI_MouseEvent( arg0, arg1 );

		case UI_REFRESH:
			_UI_Refresh( arg0 );
			return 0;

		case UI_IS_FULLSCREEN:
			return Menus_AnyFullScreenVisible();

		case UI_SET_ACTIVE_MENU:
			UI_SetActiveMenu( arg0 );
			return 0;

		case UI_CLOSEALL:
			Menus_CloseAll ( );
			return 0;

		case UI_CONSOLE_COMMAND:
			return UI_ConsoleCommand(arg0);

		case UI_DRAW_CONNECT_SCREEN:
			UI_DrawConnectScreen( arg0 );
			return 0;

		case UI_DRAW_LOADING_SCREEN:
			UI_DrawLoadingScreen();
			return 0;

		case UI_HASUNIQUECDKEY: 
			return qfalse; 
	}

	return -1;
}

/*
================
AssetCache

PreCaches the assets for the main user interface
=================
*/
void AssetCache ( void ) 
{
	int n;

	// Scrollbar and slider assets
	uiInfo.uiDC.Assets.scrollBar			= trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR );
	uiInfo.uiDC.Assets.scrollBarHoriz		= trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_HORIZ );
	uiInfo.uiDC.Assets.scrollBarArrowDown	= trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWDOWN );
	uiInfo.uiDC.Assets.scrollBarArrowUp		= trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWUP );
	uiInfo.uiDC.Assets.scrollBarArrowLeft	= trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWLEFT );
	uiInfo.uiDC.Assets.scrollBarArrowRight	= trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWRIGHT );
	uiInfo.uiDC.Assets.scrollBarThumb		= trap_R_RegisterShaderNoMip( ASSET_SCROLL_THUMB );
	uiInfo.uiDC.Assets.sliderBar			= trap_R_RegisterShaderNoMip( ASSET_SLIDER_BAR );
	uiInfo.uiDC.Assets.sliderThumb			= trap_R_RegisterShaderNoMip( ASSET_SLIDER_THUMB );

	// Load the shaders for all of the supported crosshairs
	for( n = 0; n < NUM_CROSSHAIRS; n++ ) 
	{
		uiInfo.uiDC.Assets.crosshairShader[n] = trap_R_RegisterShaderNoMip( va("gfx/menus/crosshairs/ch%i", n) );
	}
}

/*
================
_UI_DrawRectLeftRight

Draws the left and right sides of the rectangle
=================
*/
void _UI_DrawRectLeftRight ( float x, float y, float w, float h, float size ) 
{
	UI_AdjustFrom640( &x, &y, &w, &h );
	size *= uiInfo.uiDC.xscale;
	trap_R_DrawStretchPic( x, y, size, h, 0, 0, 0, 0, NULL, uiInfo.uiDC.whiteShader );
	trap_R_DrawStretchPic( x + w - size, y, size, h, 0, 0, 0, 0, NULL, uiInfo.uiDC.whiteShader );
}

/*
================
_UI_DrawRectTopBottom

Draws the top and bottom sides of the rectangle
=================
*/
void _UI_DrawRectTopBottom ( float x, float y, float w, float h, float size ) 
{
	UI_AdjustFrom640( &x, &y, &w, &h );
	size *= uiInfo.uiDC.yscale;
	trap_R_DrawStretchPic( x, y, w, size, 0, 0, 0, 0, NULL, uiInfo.uiDC.whiteShader );
	trap_R_DrawStretchPic( x, y + h - size, w, size, 0, 0, 0, 0, NULL, uiInfo.uiDC.whiteShader );
}

/*
================
UI_DrawRect

Coordinates are 640*480 virtual values
=================
*/
void _UI_DrawRect( float x, float y, float width, float height, float size, const float *color ) 
{
	trap_R_SetColor( color );

	_UI_DrawRectTopBottom(x, y, width, height, size);
	_UI_DrawRectLeftRight(x, y, width, height, size);

	trap_R_SetColor( NULL );
}

/*
=================
UI_DrawCenteredPic
=================
*/
void UI_DrawCenteredPic(qhandle_t image, int w, int h) 
{
	int x;
	int	y;
  
	x = (SCREEN_WIDTH - w) / 2;
	y = (SCREEN_HEIGHT - h) / 2;

	UI_DrawHandlePic (x, y, w, h, image);
}


/*
=================
_UI_Refresh
=================
*/
#define	UI_FPS_FRAMES	4
void _UI_Refresh( int realtime )
{
	static int index;
	static int	previousTimes[UI_FPS_FRAMES];
	int			catcher;

	//if ( !( trap_Key_GetCatcher() & KEYCATCH_UI ) ) {
	//	return;
	//}

	uiInfo.uiDC.frameTime = realtime - uiInfo.uiDC.realTime;
	uiInfo.uiDC.realTime = realtime;

	previousTimes[index % UI_FPS_FRAMES] = uiInfo.uiDC.frameTime;
	index++;
	if ( index > UI_FPS_FRAMES ) 
	{
		int i, total;
		// average multiple frames together to smooth changes out a bit
		total = 0;
		for ( i = 0 ; i < UI_FPS_FRAMES ; i++ ) 
		{
			total += previousTimes[i];
		}
		if ( !total ) 
		{
			total = 1;
		}
		uiInfo.uiDC.FPS = 1000 * UI_FPS_FRAMES / total;
	}

	// Update all of the user interface cvars
	UI_UpdateCvars();

	// Render the menus if there are any
	if (Menu_Count() > 0) 
	{
		// paint all the menus
		Menu_PaintAll();

		// refresh server browser list
		UI_DoServerRefresh();

		// refresh server status
		UI_BuildServerStatus(qfalse);

		// refresh find player list
		UI_BuildFindPlayerList(qfalse);
	} 
	
	// draw cursor
	UI_SetColor( NULL );

	catcher = trap_Key_GetCatcher ( );
	if (Menu_Count() > 0 && (catcher&KEYCATCH_UI) && !(catcher & KEYCATCH_NUMBERSONLY) ) 
	{
		UI_DrawHandlePic( uiInfo.uiDC.cursorx - 16, uiInfo.uiDC.cursory - 16, 32, 32, uiInfo.uiDC.Assets.cursor);
	}
}

/*
=================
_UI_Shutdown
=================
*/
void _UI_Shutdown( void ) 
{
	if (playerInfo.playerG2Model)
	{
		trap_G2API_CleanGhoul2Models(&playerInfo.playerG2Model);
	}

	trap_LAN_SaveCachedServers();
}

char *defaultMenu = NULL;

char *GetMenuBuffer(const char *filename) {
	int	len;
	fileHandle_t	f;
	static char buf[MAX_MENUFILE];

	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( !f ) {
		trap_Print( va( S_COLOR_RED "menu file not found: %s, using default\n", filename ) );
		return defaultMenu;
	}
	if ( len >= MAX_MENUFILE ) {
		trap_Print( va( S_COLOR_RED "menu file too large: %s is %i, max allowed is %i", filename, len, MAX_MENUFILE ) );
		trap_FS_FCloseFile( f );
		return defaultMenu;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );
	//COM_Compress(buf);
  return buf;

}

qboolean Asset_Parse(int handle) {
	pc_token_t token;
	const char *tempStr;

	if (!trap_PC_ReadToken(handle, &token))
		return qfalse;
	if (Q_stricmp(token.string, "{") != 0) {
		return qfalse;
	}
    
	while ( 1 ) {

		memset(&token, 0, sizeof(pc_token_t));

		if (!trap_PC_ReadToken(handle, &token))
			return qfalse;

		if (Q_stricmp(token.string, "}") == 0) {
			return qtrue;
		}

		// font
		if (Q_stricmp(token.string, "defaultFont") == 0) 
		{
			if (!PC_String_Parse(handle, &tempStr)) 
			{
				return qfalse;
			}
			uiInfo.uiDC.Assets.defaultFont = trap_R_RegisterFont(tempStr );
			uiInfo.uiDC.Assets.fontRegistered = qtrue;
			continue;
		}

		if (Q_stricmp(token.string, "cursor") == 0) 
		{
			if (!PC_String_Parse(handle, &uiInfo.uiDC.Assets.cursorStr))
			{
				Com_Printf(S_COLOR_YELLOW,"Bad 1st parameter for keyword 'cursor'");
				return qfalse;
			}
			uiInfo.uiDC.Assets.cursor = trap_R_RegisterShaderNoMip( uiInfo.uiDC.Assets.cursorStr);
			continue;
		}

		// itemFocusSound
		if (Q_stricmp(token.string, "itemFocusSound") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
				return qfalse;
			}
			uiInfo.uiDC.Assets.itemFocusSound = trap_S_RegisterSound( tempStr );
			continue;
		}

		if (Q_stricmp(token.string, "fadeClamp") == 0) {
			if (!PC_Float_Parse(handle, &uiInfo.uiDC.Assets.fadeClamp)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "fadeCycle") == 0) {
			if (!PC_Int_Parse(handle, &uiInfo.uiDC.Assets.fadeCycle)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "fadeAmount") == 0) {
			if (!PC_Float_Parse(handle, &uiInfo.uiDC.Assets.fadeAmount)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "shadowX") == 0) {
			if (!PC_Float_Parse(handle, &uiInfo.uiDC.Assets.shadowX)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "shadowY") == 0) {
			if (!PC_Float_Parse(handle, &uiInfo.uiDC.Assets.shadowY)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "shadowColor") == 0) {
			if (!PC_Color_Parse(handle, &uiInfo.uiDC.Assets.shadowColor)) {
				return qfalse;
			}
			uiInfo.uiDC.Assets.shadowFadeClamp = uiInfo.uiDC.Assets.shadowColor[3];
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
	}
	return qfalse;
}

void UI_Report() 
{
	String_Report();
}

void UI_ParseMenu(const char *menuFile) 
{
	int			handle;
	pc_token_t	token;

	handle = trap_PC_LoadSource(menuFile);
	if (!handle) 
	{
		return;
	}

	while ( 1 ) 
	{
		memset(&token, 0, sizeof(pc_token_t));
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
			if (Asset_Parse(handle)) 
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
			Menu_New(handle);
		}
	}

	trap_PC_FreeSource(handle);
}

qboolean Load_Menu(int handle) 
{
	pc_token_t token;

	if (!trap_PC_ReadToken(handle, &token))
	{
		return qfalse;
	}

	if (token.string[0] != '{') 
	{
		return qfalse;
	}

	while ( 1 ) 
	{
		if (!trap_PC_ReadToken(handle, &token))
		{
			return qfalse;
		}
    
		if ( token.string[0] == 0 ) 
		{
			return qfalse;
		}

		if ( token.string[0] == '}' ) 
		{
			return qtrue;
		}

		UI_ParseMenu(token.string); 
	}

	return qfalse;
}

extern menuDef_t Menus[MAX_MENUS];      // defined menus
extern int menuCount;               // how many

/*
=================
UI_LoadMenus

Loads all menus from the given text file and optionally resets the 
existing menus
=================
*/
void UI_LoadMenus ( const char *menuFile, qboolean reset ) 
{
	pc_token_t	token;
	int			handle;
	int			start;

	start = trap_Milliseconds();

	trap_PC_LoadGlobalDefines ( "ui/menudef.h" );

	handle = trap_PC_LoadSource( menuFile );
	if (!handle) 
	{
		trap_Error( va( S_COLOR_RED "menu file not found: %s, unable to continue!\n", menuFile ) );
	}

	ui_new.integer = 1;

	if (reset) 
	{
		Menu_Reset();
	}

	while ( 1 ) 
	{
		if (!trap_PC_ReadToken(handle, &token))
		{
			break;
		}

		if( token.string[0] == 0 || token.string[0] == '}') 
		{
			break;
		}

		if ( token.string[0] == '}' ) 
		{
			break;
		}

		if (Q_stricmp(token.string, "loadmenu") == 0) 
		{
			if (Load_Menu(handle)) 
			{
				if (menuCount == 1 && Menus_FindByName("Loading"))
				{
					// handle double / tripple buffering...
					UI_UpdateScreen(qtrue);
					UI_UpdateScreen(qtrue);
					UI_UpdateScreen(qtrue);
				}
				continue;
			} 
			else 
			{
				break;
			}
		}
	}

	Com_Printf("UI total menu load time = %d ms\n", trap_Milliseconds() - start);

	trap_PC_FreeSource( handle );

	trap_PC_RemoveAllGlobalDefines ( );
}

/*
=================
UI_Reload

Reloads the 
=================
*/
void UI_Reload ( void ) 
{
	char		lastName[1024];
	menuDef_t	*menu;
	int			actual;

	// Retail the last active menu so it 
	// can be opened again
	menu = Menu_GetFocused();
	if (menu && menu->window.name) 
	{
		strcpy(lastName, menu->window.name);
	}

	// Reinitialize the strings
	String_Init();

	// Read in the gametype list for the various user interface screens
	BG_BuildGametypeList ( );

	// Load the list of arenas available for play
	UI_LoadArenas();

	UI_SelectedMap ( 0, &actual );
	trap_Cvar_Set("ui_mapIndex", "0");
	ui_currentNetMap.integer = actual;
	trap_Cvar_Set("ui_currentNetMap", va("%d",actual));
	trap_Cvar_Set("ui_currentNetMapName", uiInfo.mapList[actual].mapLoadName );

	// Load all of the menus files now
	UI_LoadMenus( uiInfo.menusFile, qtrue);

	// Close any open menus and open the last menu that was open before reloading
	Menus_CloseAll();
	Menus_ActivateByName(lastName);
}

void UI_DrawObjectivePhotos ( rectDef_t *rect, qhandle_t font, float scale, vec4_t color )
{
	int   gametype = (int) trap_Cvar_VariableValue ( "ui_about_gametype" );
	int   count;
	int   index;
	float width;
	float x;
	float spacing;
	float height;
	float fontheight;

	// Figure out how many photos there are
	for ( count = 0; bg_gametypeData[gametype].photos[count].name && count < MAX_GAMETYPE_PHOTOS; count ++ );

	// Images should be saved as 256 x 192
	width  = (rect->w / count) - ((count-1) * 10);
	height = width / 256 * 192;

	// If height is too big for the given rect, scale them down again
	if ( height > rect->h )
	{
		width = rect->h / height * width;
		height = rect->h;
	}
	
	// How much space is there between each photo horizontally?
	spacing = (rect->w - (width * count)) / (count-1);

	fontheight = trap_R_GetTextHeight ( "W", font, scale, 0 );

	for ( x = rect->x, index = 0; index < count; index ++ )
	{
		qhandle_t pic;

		pic = trap_R_RegisterShaderNoMip ( va("gfx/menus/levelshots/%s_%s_%s",
									  UI_Cvar_VariableString ( "ui_about_mapname" ),
									  bg_gametypeData[gametype].name,
									  bg_gametypeData[gametype].photos[index].name ) );

		if ( pic )
		{
			UI_DrawText ( x, rect->y - fontheight - 2, font, scale, color, bg_gametypeData[gametype].photos[index].displayName, 0, 0 );
			UI_DrawHandlePic ( x, rect->y, width, height, pic );
		}

		x += spacing;
		x += width;
	}	
}

// ui_gameType assumes gametype 0 is -1 ALL and will not show
static void UI_DrawGameType(rectDef_t *rect, qhandle_t font, float scale, vec4_t color ) 
{
	UI_DrawText (rect->x, rect->y, font, scale, color, bg_gametypeData[(int)trap_Cvar_VariableValue ( "ui_about_gametype" )].displayName, 0, 0 );
}

static void UI_DrawNetGameType(rectDef_t *rect, qhandle_t font, float scale, vec4_t color ) 
{
	if ( ui_netGameType.integer < 0 || ui_netGameType.integer >= bg_gametypeCount ) 
	{
		trap_Cvar_Set("ui_netGameType", "0");
		trap_Cvar_Set("ui_actualNetGameType", "0");

		trap_Cvar_Update ( &ui_netGameType );
		trap_Cvar_Update ( &ui_actualNetGameType );
	}

	UI_DrawText (rect->x, rect->y, font, scale, color, bg_gametypeData[ ui_netGameType.integer ].displayName, 0, 0 );
}

static void UI_DrawJoinGameType(rectDef_t *rect, qhandle_t font, float scale, vec4_t color ) 
{
 	if ( ui_joinGameType.integer < 0 || ui_joinGameType.integer > bg_gametypeCount ) 
	{
		trap_Cvar_Set("ui_joinGameType", "0");
	}

	if ( ui_joinGameType.integer == 0 )
	{
		UI_DrawText(rect->x, rect->y, font, scale, color, "All", 0, 0 );
	}
	else
	{
		UI_DrawText(rect->x, rect->y, font, scale, color, bg_gametypeData[ui_joinGameType.integer-1].displayName, 0, 0 );
	}
}

static void UI_DrawPreviewCinematic(rectDef_t *rect, float scale, vec4_t color) {
	if (uiInfo.previewMovie > -2) {
		uiInfo.previewMovie = trap_CIN_PlayCinematic(va("%s.roq", uiInfo.movieList[uiInfo.movieIndex]), 0, 0, 0, 0, (CIN_loop | CIN_silent) );
		if (uiInfo.previewMovie >= 0) {
		  trap_CIN_RunCinematic(uiInfo.previewMovie);
			trap_CIN_SetExtents(uiInfo.previewMovie, rect->x, rect->y, rect->w, rect->h);
 			trap_CIN_DrawCinematic(uiInfo.previewMovie);
		} else {
			uiInfo.previewMovie = -2;
		}
	} 

}

static void UI_DrawSkill(rectDef_t *rect, qhandle_t font, float scale, vec4_t color ) 
{
	int i;
	
	i = trap_Cvar_VariableValue( "g_botSkill" );
	
	if (i < 1 || i > numSkillLevels) 
	{
		i = 1;
	}

	UI_DrawText (rect->x, rect->y, font, scale, color, skillLevels[i-1],0, 0 );
}


static void UI_DrawTeamName(rectDef_t *rect, qhandle_t font, float scale, vec4_t color, qboolean blue ) 
{
	char name[MAX_QPATH];
	trap_Cvar_VariableStringBuffer ( va("ui_%steamname", blue?"blue":"red"), name, MAX_QPATH );

	UI_DrawText (rect->x, rect->y, font, scale, color, name,0, 0 );
}

static void UI_DrawTeamIdentity ( rectDef_t *rect, team_t team, int index )
{
	if ( uiInfo.identityTeams[team][index] && uiInfo.identityTeams[team][index]->mIcon )
	{
		UI_DrawHandlePic( rect->x + 2, rect->y + 2, rect->w - 4, rect->h - 4, uiInfo.identityTeams[team][index]->mIcon);
	}
	else
	{
		static vec4_t color = {0,0,0,0.25f};
		UI_DrawRect ( rect->x + 2, rect->y + 2, rect->w - 4, rect->h -4, color );
	}
}

static void UI_DrawMapPreview(rectDef_t *rect, float scale, vec4_t color, qboolean net) 
{
	int map = (net) ? ui_currentNetMap.integer : ui_currentMap.integer;
	if (map < 0 || map > uiInfo.mapCount) 
	{
		if (net) 
		{
			int actual;
			trap_Cvar_Set("ui_mapIndex", "0");
			UI_SelectedMap ( 0, &actual );
			ui_currentNetMap.integer = actual;
			trap_Cvar_Set("ui_currentNetMap", va("%d",actual));
			trap_Cvar_Set("ui_currentNetMapName", uiInfo.mapList[actual].mapLoadName );
		} 
		else 
		{
			ui_currentMap.integer = 0;
			trap_Cvar_Set("ui_currentMap", "0");
		}
		map = 0;
	}

	if (uiInfo.mapList[map].levelShot == -1) 
	{
		uiInfo.mapList[map].levelShot = trap_R_RegisterShaderNoMip(uiInfo.mapList[map].imageName);
	}

	if (uiInfo.mapList[map].levelShot > 0) {
		UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, uiInfo.mapList[map].levelShot);
	} else {
		UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, trap_R_RegisterShaderNoMip("gfx/menus/levelshots/unknownmap_mp"));
	}
}						 

static void UI_DrawMapCinematic(rectDef_t *rect, float scale, vec4_t color, qboolean net) 
{
	int map = (net) ? ui_currentNetMap.integer : ui_currentMap.integer; 
	if (map < 0 || map > uiInfo.mapCount) 
	{
		if (net) 
		{
			int actual;
			trap_Cvar_Set("ui_mapIndex", "0");
			UI_SelectedMap ( 0, &actual );
			ui_currentNetMap.integer = actual;
			trap_Cvar_Set("ui_currentNetMap", va("%d",actual));
			trap_Cvar_Set("ui_currentNetMapName", uiInfo.mapList[actual].mapLoadName );
		} 
		else 
		{
			ui_currentMap.integer = 0;
			trap_Cvar_Set("ui_currentMap", "0");
		}
		
		map = 0;
	}

	if (uiInfo.mapList[map].cinematic >= -1) 
	{
		if (uiInfo.mapList[map].cinematic == -1) 
		{
			uiInfo.mapList[map].cinematic = trap_CIN_PlayCinematic(va("%s.roq", uiInfo.mapList[map].mapLoadName), 0, 0, 0, 0, (CIN_loop | CIN_silent) );
		}
		if (uiInfo.mapList[map].cinematic >= 0) 
		{
			trap_CIN_RunCinematic(uiInfo.mapList[map].cinematic);
			trap_CIN_SetExtents(uiInfo.mapList[map].cinematic, rect->x, rect->y, rect->w, rect->h);
			trap_CIN_DrawCinematic(uiInfo.mapList[map].cinematic);
		} 
		else 
		{
			uiInfo.mapList[map].cinematic = -2;
		}
	} 
	else 
	{
		UI_DrawMapPreview(rect, scale, color, net);
	}
}

static qboolean updateModel = qtrue;


/*
================
UI_DrawOutfittingBackground

Draws the slot background
================
*/
static void UI_DrawOutfittingBackground ( rectDef_t* rect, vec4_t color, int slot )
{
	vec4_t  col;

	VectorCopy4 ( color, col );

	if ( bg_outfittings[uiInfo.outfittingItemGroup].items[slot] == -1 )
	{
		col[3] *= 0.5f;
	}
	else
	{
		int item = bg_outfittingGroups[slot][bg_outfittings[uiInfo.outfittingItemGroup].items[slot]];

		if ( item <= 0 )
		{
			col[3] *= 0.5f;
		}
	}

	trap_R_SetColor ( col );
	UI_DrawHandlePic ( rect->x, rect->y, rect->w, rect->h, uiInfo.uiDC.whiteShader );
}

/*
================
UI_DrawOutfittingSlotName

Draw the name of the weapon
================
*/
static void UI_DrawOutfittingSlotName ( rectDef_t *rect, qhandle_t font, float scale, vec4_t color, int slot )
{
	const char* s;
	int			item;
	float		w;

	// Handle the case where there is nothing at all available for this group
	if ( bg_outfittings[uiInfo.outfittingItemGroup].items[slot] == -1 )
	{
		s = "NONE";
	}
	else
	{
		item = bg_outfittingGroups[slot][bg_outfittings[uiInfo.outfittingItemGroup].items[slot]];
		if ( item <= 0 )
		{
			s = "NONE";
		}
		else
		{
			s = bg_itemlist[item].pickup_name;
		}
	}

	w = trap_R_GetTextWidth ( s, font, scale, 0 );	

	UI_DrawText ( rect->x - w, rect->y, font, scale, color, s, 0, 0 );	
}

/*
================
UI_DrawOutfittingSlotRender

Draw the weapon render for the given outfitting slot
================
*/
static void UI_DrawOutfittingSlotRender ( rectDef_t *rect, int slot )
{
	qhandle_t	shader;
	const char*	name;

	// Nothing at all? Then draw a black square
	if ( bg_outfittings[uiInfo.outfittingItemGroup].items[slot] == -1 )
	{
		return;
	}
	
	name = bg_itemlist[bg_outfittingGroups[slot][bg_outfittings[uiInfo.outfittingItemGroup].items[slot]]].render;
	if ( !name )
	{
		return;
	}

	// This is a bit hacky, but any render with a * in the front of it
	// is only half size
	if ( *name == '*')
	{
		rect->x += rect->w / 4;
		rect->w /= 2;
		name ++;
	}

	shader = trap_R_RegisterShaderNoMip ( name );

	UI_DrawHandlePic ( rect->x, rect->y, rect->w, rect->h, shader );
}

static void UI_DrawPlayerModel(rectDef_t *rect) 
{
	vec3_t	viewangles;
	vec3_t	moveangles;
	char	identity[MAX_QPATH];

	if ( (int)trap_Cvar_VariableValue ( "ui_info_teamgame" ) )
	{
		strcpy(identity, UI_Cvar_VariableString("team_identity"));
	}
	else
	{
		strcpy(identity, UI_Cvar_VariableString("identity"));
	}

	if (updateModel) 
	{
		if (playerInfo.playerG2Model)
		{
			trap_G2API_CleanGhoul2Models(&playerInfo.playerG2Model);
		}

  		memset( &playerInfo, 0, sizeof(playerInfo_t) );
  		viewangles[YAW]   = 180 - 10;
  		viewangles[PITCH] = 0;
  		viewangles[ROLL]  = 0;
  		VectorClear( moveangles );
		UI_PlayerInfo_SetIdentity ( &playerInfo, identity );
		UI_PlayerInfo_SetInfo( &playerInfo, LEGS_WALK, TORSO_IDLE_PISTOL, viewangles, vec3_origin, WP_FIRST_RANGED_WEAPON, qfalse );
		updateModel = qfalse;
	}

	UI_DrawPlayer( rect->x, rect->y, rect->w, rect->h, &playerInfo, uiInfo.uiDC.realTime / 2);
}

static void UI_DrawNetSource(rectDef_t *rect, qhandle_t font, float scale, vec4_t color ) 
{
	if (ui_netSource.integer < 0 || ui_netSource.integer > numNetSources) 
	{
		ui_netSource.integer = 0;
	}

	UI_DrawText (rect->x, rect->y, font, scale, color, va("Source: %s", netSources[ui_netSource.integer]), 0, 0 );
}

static void UI_DrawNetMapPreview(rectDef_t *rect, float scale, vec4_t color) {

	if (uiInfo.serverStatus.currentServerPreview > 0) {
		UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, uiInfo.serverStatus.currentServerPreview);
	} else {
		UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, trap_R_RegisterShaderNoMip("gfx/menus/levelshots/unknownmap_mp"));
	}
}

static void UI_DrawNetMapCinematic(rectDef_t *rect, float scale, vec4_t color) 
{
	if (ui_currentNetMap.integer < 0 || ui_currentNetMap.integer > uiInfo.mapCount) 
	{
		int actual;
		trap_Cvar_Set("ui_mapIndex", "0");
		UI_SelectedMap ( 0, &actual );
		ui_currentNetMap.integer = actual;
		trap_Cvar_Set("ui_currentNetMap", va("%d",actual));
		trap_Cvar_Set("ui_currentNetMapName", uiInfo.mapList[actual].mapLoadName );
	}

	if (uiInfo.serverStatus.currentServerCinematic >= 0) 
	{
	  trap_CIN_RunCinematic(uiInfo.serverStatus.currentServerCinematic);
	  trap_CIN_SetExtents(uiInfo.serverStatus.currentServerCinematic, rect->x, rect->y, rect->w, rect->h);
 	  trap_CIN_DrawCinematic(uiInfo.serverStatus.currentServerCinematic);
	} 
	else 
	{
		UI_DrawNetMapPreview(rect, scale, color);
	}
}

static void UI_DrawNetFilter(rectDef_t *rect, qhandle_t font, float scale, vec4_t color ) 
{
	if (ui_serverFilterType.integer < 0 || ui_serverFilterType.integer > numServerFilters) 
	{
		ui_serverFilterType.integer = 0;
	}

	UI_DrawText (rect->x, rect->y, font, scale, color, va("Filter: %s", serverFilters[ui_serverFilterType.integer].description), 0, 0 );
}


static const char *UI_EnglishMapName(const char *map) {
	int i;
	for (i = 0; i < uiInfo.mapCount; i++) {
		if (Q_stricmp(map, uiInfo.mapList[i].mapLoadName) == 0) {
			return uiInfo.mapList[i].mapName;
		}
	}
	return "";
}

static void UI_DrawAllMapsSelection(rectDef_t *rect, qhandle_t font, float scale, vec4_t color, int textStyle, qboolean net) 
{
	int map = (net) ? ui_currentNetMap.integer : ui_currentMap.integer;
	
	if (map >= 0 && map < uiInfo.mapCount) 
	{
		UI_DrawText (rect->x, rect->y, font, scale, color, uiInfo.mapList[map].mapName, 0, 0 );
	}
}

static int UI_OwnerDrawWidth(int ownerDraw, qhandle_t font, float scale ) 
{
	const char	*s = NULL;

	switch (ownerDraw) 
	{
		case UI_GAMETYPE:
			s = bg_gametypeData[ui_gameType.integer].displayName;
			break;

		case UI_SKILL:
			{
				int	i;

				i = trap_Cvar_VariableValue( "g_botSkill" );
				if (i < 1 || i > numSkillLevels) 
				{
					i = 1;
				}
				s = skillLevels[i-1];
			}
			break;
   
		case UI_NETSOURCE:
			if (ui_netSource.integer < 0 || ui_netSource.integer >= bg_gametypeCount) 
			{
				ui_netSource.integer = 0;
			}
			s = va("Source: %s", netSources[ui_netSource.integer]);
			break;

		case UI_NETFILTER:
			if (ui_serverFilterType.integer < 0 || ui_serverFilterType.integer > numServerFilters) {
				ui_serverFilterType.integer = 0;
			}
			s = va("Filter: %s", serverFilters[ui_serverFilterType.integer].description );
			break;

		case UI_ALLMAPS_SELECTION:
			break;

		case UI_SERVERREFRESHDATE:
			s = UI_Cvar_VariableString(va("ui_lastServerRefresh_%i", ui_netSource.integer));
			break;
    
		default:
			break;
	}

	if (s) 
	{
		return trap_R_GetTextWidth (s, font, scale, 0 );
	}

	return 0;
}

static void UI_DrawBotName(rectDef_t *rect, qhandle_t font, float scale, vec4_t color ) 
{
	int			value = uiInfo.botIndex;
	const char	*text = "";

	if (value >= UI_GetNumBots()) 
	{
		value = 0;
	}

	text = UI_GetBotNameByNumber(value);

	UI_DrawText (rect->x, rect->y, font, scale, color, text, 0, 0 );
}

static void UI_DrawBotSkill(rectDef_t *rect, qhandle_t font, float scale, vec4_t color ) 
{
	if (uiInfo.skillIndex < 0 || uiInfo.skillIndex >= numSkillLevels) 
		return;

	UI_DrawText(rect->x, rect->y, font, scale, color, skillLevels[uiInfo.skillIndex], 0, 0 );
}

static void UI_DrawRedBlue(rectDef_t *rect, qhandle_t font, float scale, vec4_t color ) 
{
	UI_DrawText (rect->x, rect->y, font, scale, color, (uiInfo.redBlue == 0) ? "Red" : "Blue", 0, 0 );
}

static void UI_DrawCrosshair(rectDef_t *rect, float scale, vec4_t color) {
 	trap_R_SetColor( color );
	if (uiInfo.currentCrosshair < 0 || uiInfo.currentCrosshair >= NUM_CROSSHAIRS) {
		uiInfo.currentCrosshair = 0;
	}
	UI_DrawHandlePic( rect->x, rect->y, 24, 24, uiInfo.uiDC.Assets.crosshairShader[uiInfo.currentCrosshair]);
 	trap_R_SetColor( NULL );
}

/*
===============
UI_BuildPlayerList
===============
*/
static void UI_BuildPlayerList() 
{
	uiClientState_t	cs;
	int				n, count, team, team2, playerTeamNumber;
	char			info[MAX_INFO_STRING];

	trap_GetClientState( &cs );
	trap_GetConfigString( CS_PLAYERS + cs.clientNum, info, MAX_INFO_STRING );
	uiInfo.playerNumber = cs.clientNum;
	uiInfo.teamLeader = atoi(Info_ValueForKey(info, "tl"));
	team = atoi(Info_ValueForKey(info, "t"));
	trap_GetConfigString( CS_SERVERINFO, info, sizeof(info) );
	count = atoi( Info_ValueForKey( info, "sv_maxclients" ) );
	uiInfo.playerCount = 0;
	uiInfo.myTeamCount = 0;
	playerTeamNumber = 0;
	
	for( n = 0; n < count; n++ ) 
	{
		trap_GetConfigString( CS_PLAYERS + n, info, MAX_INFO_STRING );

		if (info[0]) 
		{
			Com_sprintf ( uiInfo.playerNames[uiInfo.playerCount], 4, "%2d", n );
			Q_strncpyz( uiInfo.playerNames[uiInfo.playerCount] + 2, Info_ValueForKey( info, "n" ), MAX_NAME_LENGTH );
			Q_CleanStr( uiInfo.playerNames[uiInfo.playerCount] + 2 );
			uiInfo.playerCount++;
			team2 = atoi(Info_ValueForKey(info, "t"));
			
			if (team2 == team) 
			{
				Q_strncpyz( uiInfo.teamNames[uiInfo.myTeamCount], Info_ValueForKey( info, "n" ), MAX_NAME_LENGTH );
				Q_CleanStr( uiInfo.teamNames[uiInfo.myTeamCount] );
				uiInfo.teamClientNums[uiInfo.myTeamCount] = n;

				if (uiInfo.playerNumber == n) 
				{
					playerTeamNumber = uiInfo.myTeamCount;
				}

				uiInfo.myTeamCount++;
			}
		}
	}

	if (!uiInfo.teamLeader) {
		trap_Cvar_Set("cg_selectedPlayer", va("%d", playerTeamNumber));
	}

	n = trap_Cvar_VariableValue("cg_selectedPlayer");
	if (n < 0 || n > uiInfo.myTeamCount) {
		n = 0;
	}
	if (n < uiInfo.myTeamCount) {
		trap_Cvar_Set("cg_selectedPlayerName", uiInfo.teamNames[n]);
	}
}


static void UI_DrawSelectedPlayer(rectDef_t *rect, qhandle_t font, float scale, vec4_t color ) 
{
	if (uiInfo.uiDC.realTime > uiInfo.playerRefresh) 
	{
		uiInfo.playerRefresh = uiInfo.uiDC.realTime + 3000;
		UI_BuildPlayerList();
	}

	UI_DrawText (rect->x, rect->y, font, scale, color, (uiInfo.teamLeader) ? UI_Cvar_VariableString("cg_selectedPlayerName") : UI_Cvar_VariableString("name") , 0, 0 );
}

static void UI_DrawServerRefreshDate(rectDef_t *rect, qhandle_t font, float scale, vec4_t color ) 
{
	if (uiInfo.serverStatus.refreshActive) 
	{
		vec4_t lowLight, newColor;
		lowLight[0] = 0.8 * color[0]; 
		lowLight[1] = 0.8 * color[1]; 
		lowLight[2] = 0.8 * color[2]; 
		lowLight[3] = 0.8 * color[3]; 
		LerpColor(color,lowLight,newColor,0.5+0.5*sin(uiInfo.uiDC.realTime / PULSE_DIVISOR));
		UI_DrawText (rect->x, rect->y, font, scale, newColor, va("Getting info for %d servers (ESC to cancel)", trap_LAN_GetServerCount(ui_netSource.integer)), 0, 0 );
	} 
	else 
	{
		char buff[64];
		Q_strncpyz(buff, UI_Cvar_VariableString(va("ui_lastServerRefresh_%i", ui_netSource.integer)), 64);
		UI_DrawText (rect->x, rect->y, font, scale, color, va("Refresh Time: %s", buff), 0, 0 );
	}
}

static void UI_DrawServerMOTD(rectDef_t *rect, float scale, vec4_t color) {
	if (uiInfo.serverStatus.motdLen) {
		float maxX;
	 
		if (uiInfo.serverStatus.motdWidth == -1) {
			uiInfo.serverStatus.motdWidth = 0;
			uiInfo.serverStatus.motdPaintX = rect->x + 1;
			uiInfo.serverStatus.motdPaintX2 = -1;
		}

		if (uiInfo.serverStatus.motdOffset > uiInfo.serverStatus.motdLen) {
			uiInfo.serverStatus.motdOffset = 0;
			uiInfo.serverStatus.motdPaintX = rect->x + 1;
			uiInfo.serverStatus.motdPaintX2 = -1;
		}

		if (uiInfo.uiDC.realTime > uiInfo.serverStatus.motdTime) 
		{
			uiInfo.serverStatus.motdTime = uiInfo.uiDC.realTime + 10;
			if (uiInfo.serverStatus.motdPaintX <= rect->x + 2) 
			{
				if (uiInfo.serverStatus.motdOffset < uiInfo.serverStatus.motdLen) 
				{
					uiInfo.serverStatus.motdPaintX += trap_R_GetTextWidth ( &uiInfo.serverStatus.motd[uiInfo.serverStatus.motdOffset], uiInfo.uiDC.Assets.defaultFont, scale, 1 ) - 1;
					uiInfo.serverStatus.motdOffset++;
				} 
				else 
				{
					uiInfo.serverStatus.motdOffset = 0;
					if (uiInfo.serverStatus.motdPaintX2 >= 0) 
					{
						uiInfo.serverStatus.motdPaintX = uiInfo.serverStatus.motdPaintX2;
					} 
					else 
					{
						uiInfo.serverStatus.motdPaintX = rect->x + rect->w - 2;
					}
					uiInfo.serverStatus.motdPaintX2 = -1;
				}
			} 
			else 
			{
				//serverStatus.motdPaintX--;
				uiInfo.serverStatus.motdPaintX -= 2;
				if (uiInfo.serverStatus.motdPaintX2 >= 0) {
					//serverStatus.motdPaintX2--;
					uiInfo.serverStatus.motdPaintX2 -= 2;
				}
			}
		}

		maxX = rect->x + rect->w - 2;
//		Text_Paint_Limit(&maxX, uiInfo.serverStatus.motdPaintX, rect->y + rect->h - 3, scale, color, &uiInfo.serverStatus.motd[uiInfo.serverStatus.motdOffset], 0, 0, NULL ); 
		if (uiInfo.serverStatus.motdPaintX2 >= 0) 
		{
//			float maxX2 = rect->x + rect->w - 2;
//			Text_Paint_Limit(&maxX2, uiInfo.serverStatus.motdPaintX2, rect->y + rect->h - 3, scale, color, uiInfo.serverStatus.motd, 0, uiInfo.serverStatus.motdOffset, NULL ); 
		}
		if (uiInfo.serverStatus.motdOffset && maxX > 0) {
			// if we have an offset ( we are skipping the first part of the string ) and we fit the string
			if (uiInfo.serverStatus.motdPaintX2 == -1) {
						uiInfo.serverStatus.motdPaintX2 = rect->x + rect->w - 2;
			}
		} else {
			uiInfo.serverStatus.motdPaintX2 = -1;
		}

	}
}

static void UI_DrawKeyBindStatus(rectDef_t *rect, qhandle_t font, float scale, vec4_t color ) 
{
	if (Display_KeyBindPending()) 
	{
		UI_DrawText (rect->x, rect->y, font, scale, color, "Waiting for new key... Press ESCAPE to cancel", 0, 0 );
	} 
	else 
	{
		UI_DrawText (rect->x, rect->y, font, scale, color, "Press ENTER or CLICK to change, Press BACKSPACE to clear", 0, 0 );
	}
}

static void UI_DrawGLInfo(rectDef_t *rect, qhandle_t font, float scale, vec4_t color ) 
{
	char * eptr;
	char buff[4096];
	const char *lines[64];
	int y, numLines, i;

	UI_DrawText (rect->x + 2, rect->y, font, scale, color, va("VENDOR: %s", uiInfo.uiDC.glconfig.vendor_string), 30, 0 );
	UI_DrawText (rect->x + 2, rect->y + 15, font, scale, color, va("VERSION: %s: %s", uiInfo.uiDC.glconfig.version_string,uiInfo.uiDC.glconfig.renderer_string), 30, 0 );
	UI_DrawText (rect->x + 2, rect->y + 30, font, scale, color, va ("PIXELFORMAT: color(%d-bits) Z(%d-bits) stencil(%d-bits)", uiInfo.uiDC.glconfig.colorBits, uiInfo.uiDC.glconfig.depthBits, uiInfo.uiDC.glconfig.stencilBits), 30, 0);

	// build null terminated extension strings
	Q_strncpyz(buff, uiInfo.uiDC.glconfig.extensions_string, 4096);
	eptr = buff;
	y = rect->y + 45;
	numLines = 0;
	while ( y < rect->y + rect->h && *eptr )
	{
		while ( *eptr && *eptr == ' ' )
			*eptr++ = '\0';

		// track start of valid string
		if (*eptr && *eptr != ' ') {
			lines[numLines++] = eptr;
		}

		while ( *eptr && *eptr != ' ' )
			eptr++;
	}

	i = 0;
	while (i < numLines) 
	{
		UI_DrawText (rect->x + 2, y, font, scale, color, lines[i++], 20, 0 );
		
		if (i < numLines) 
		{
			UI_DrawText(rect->x + rect->w / 2, y, font, scale, color, lines[i++], 20, 0 );
		
		}
		y += 10;

		if (y > rect->y + rect->h - 11) 
		{
			break;
		}
	}


}

/*
=================
UI_Version
=================
*/
static void UI_Version(rectDef_t *rect, float scale, vec4_t color) 
{
	int width;
	
	width = trap_R_GetTextWidth ( Q3_VERSION, uiInfo.uiDC.Assets.defaultFont, scale, 0 );

	UI_DrawText (rect->x - width, rect->y, uiInfo.uiDC.Assets.defaultFont, scale, color, Q3_VERSION, 0, 0 );
}

// FIXME: table drive
//
static void UI_OwnerDraw(float x, float y, float w, float h, float text_x, float text_y, int ownerDraw, int ownerDrawFlags, int align, float special, qhandle_t font, float scale, vec4_t color, qhandle_t shader, int textStyle, const char* param ) 
{
	rectDef_t rect;

	rect.x = x + text_x;
	rect.y = y + text_y;
	rect.w = w;
	rect.h = h;

	switch (ownerDraw) 
	{
		case UI_OUTFITTING_SLOT_RENDER:
			UI_DrawOutfittingSlotRender ( &rect, atoi ( param ));
			break;

		case UI_OUTFITTING_SLOT_NAME:
			UI_DrawOutfittingSlotName ( &rect, font, scale, color, atoi ( param ) );
			break;		

		case UI_OUTFITTING_SLOT_BACKGROUND:
			UI_DrawOutfittingBackground ( &rect, color, atoi ( param ) );
			break;

		case UI_PLAYERMODEL:
			UI_DrawPlayerModel(&rect);
			break;

		case UI_PREVIEWCINEMATIC:
			UI_DrawPreviewCinematic(&rect, scale, color);
			break;
		
		case UI_GAMETYPE:
			UI_DrawGameType(&rect, font, scale, color );
			break;

		case UI_OBJECTIVE_PHOTOS:	
			UI_DrawObjectivePhotos (&rect, font, scale, color );
			break;

		case UI_NETGAMETYPE:
			UI_DrawNetGameType(&rect, font, scale, color );
			break;

		case UI_JOINGAMETYPE:
			UI_DrawJoinGameType(&rect, font, scale, color );
			break;

		case UI_MAPPREVIEW:
			UI_DrawMapPreview(&rect, scale, color, qtrue);
			break;
	
		case UI_MAPCINEMATIC:
      UI_DrawMapCinematic(&rect, scale, color, qfalse);
      break;
    case UI_STARTMAPCINEMATIC:
      UI_DrawMapCinematic(&rect, scale, color, qtrue);
      break;
		
		case UI_SKILL:
			UI_DrawSkill(&rect, font, scale, color );
			break;

		case UI_REDTEAM_IDENTITY:
			UI_DrawTeamIdentity ( &rect, TEAM_RED, atoi(param) );
			break;

		case UI_BLUETEAM_IDENTITY:
			UI_DrawTeamIdentity ( &rect, TEAM_BLUE, atoi(param) );
			break;
		
		case UI_NETSOURCE:
			UI_DrawNetSource(&rect, font, scale, color );
			break;
		
    case UI_NETMAPPREVIEW:
      UI_DrawNetMapPreview(&rect, scale, color);
      break;
    case UI_NETMAPCINEMATIC:
      UI_DrawNetMapCinematic(&rect, scale, color);
      break;
		
		case UI_NETFILTER:
			UI_DrawNetFilter(&rect, font, scale, color );
			break;
		case UI_ALLMAPS_SELECTION:
			UI_DrawAllMapsSelection(&rect, font, scale, color, textStyle, qtrue);
			break;
		case UI_MAPS_SELECTION:
			UI_DrawAllMapsSelection(&rect, font, scale, color, textStyle, qfalse);
			break;
		case UI_BOTNAME:
			UI_DrawBotName(&rect, font, scale, color );
			break;
		case UI_BOTSKILL:
			UI_DrawBotSkill(&rect, font, scale, color );
			break;
		case UI_REDBLUE:
			UI_DrawRedBlue(&rect, font, scale, color );
			break;
		case UI_CROSSHAIR:
			UI_DrawCrosshair(&rect, scale, color);
			break;
		case UI_SELECTEDPLAYER:
			UI_DrawSelectedPlayer(&rect, font, scale, color );
			break;
		case UI_SERVERREFRESHDATE:
			UI_DrawServerRefreshDate(&rect, font, scale, color );
			break;
		case UI_SERVERMOTD:
			UI_DrawServerMOTD(&rect, scale, color);
			break;
		case UI_GLINFO:
			UI_DrawGLInfo(&rect, font, scale, color );
			break;
		case UI_VERSIONDOWNLOAD_PROGRESS:
			UI_DrawVersionDownloadProgress(&rect, font, scale, color );
			break;
		default:
			break;
	}
}

static void UI_DrawVersionDownloadProgress(rectDef_t *rect, qhandle_t font, float scale, vec4_t color )
{
	// Add code to show loading progress
	UI_DrawText (rect->x, rect->y, font, scale, color, va("Progress: %d %",90),0,0);
}

static qboolean UI_OwnerDrawDisabled ( int flags, const char* param )
{	
	return qfalse;
}

static qboolean UI_OwnerDrawVisible ( int flags, const char* param ) 
{
	qboolean vis = qtrue;

	while (vis && flags) 
	{
		if (flags & UI_SHOW_LEADER) 
		{
			// these need to show when this client can give orders to a player or a group
			if (!uiInfo.teamLeader) 
			{
				vis = qfalse;
			} 
			else 
			{
				// if showing yourself
				if (ui_selectedPlayer.integer < uiInfo.myTeamCount && uiInfo.teamClientNums[ui_selectedPlayer.integer] == uiInfo.playerNumber) 
				{				 
					vis = qfalse;
				}
			}
			flags &= ~UI_SHOW_LEADER;
		} 

		if (flags & UI_SHOW_NOTLEADER) 
		{
			// these need to show when this client is assigning their own status or they are NOT the leader
			if (uiInfo.teamLeader) {
				// if not showing yourself
				if (!(ui_selectedPlayer.integer < uiInfo.myTeamCount && uiInfo.teamClientNums[ui_selectedPlayer.integer] == uiInfo.playerNumber)) { 
					vis = qfalse;
				}
				// these need to show when this client can give orders to a player or a group
			}
			flags &= ~UI_SHOW_NOTLEADER;
		} 
		if (flags & UI_SHOW_FAVORITESERVERS) {
			// this assumes you only put this type of display flag on something showing in the proper context
			if (ui_netSource.integer != AS_FAVORITES) {
				vis = qfalse;
			}
			flags &= ~UI_SHOW_FAVORITESERVERS;
		} 
		if (flags & UI_SHOW_NOTFAVORITESERVERS) {
			// this assumes you only put this type of display flag on something showing in the proper context
			if (ui_netSource.integer == AS_FAVORITES) {
				vis = qfalse;
			}
			flags &= ~UI_SHOW_NOTFAVORITESERVERS;
		} 
		if (flags & UI_SHOW_ANYTEAMGAME) 
		{
			if ( !bg_gametypeData[ui_gameType.integer].teams ) 
			{
				vis = qfalse;
			}
			flags &= ~UI_SHOW_ANYTEAMGAME;
		} 
		
		if (flags & UI_SHOW_ANYNONTEAMGAME) 
		{
			if ( bg_gametypeData[ui_gameType.integer].teams ) 
			{
				vis = qfalse;
			}
			
			flags &= ~UI_SHOW_ANYNONTEAMGAME;
		} 
		
		if (flags & UI_SHOW_NETANYTEAMGAME) 
		{
			if ( !bg_gametypeData[ui_netGameType.integer].teams ) 
			{
				vis = qfalse;
			}

			flags &= ~UI_SHOW_NETANYTEAMGAME;
		} 
		
		if (flags & UI_SHOW_NETANYNONTEAMGAME) 
		{
			if ( bg_gametypeData[ui_netGameType.integer].teams ) 
			{
				vis = qfalse;
			}
			flags &= ~UI_SHOW_NETANYNONTEAMGAME;
		} 
		if (flags & UI_SHOW_NEWHIGHSCORE) {
			if (uiInfo.newHighScoreTime < uiInfo.uiDC.realTime) {
				vis = qfalse;
			} else {
				if (uiInfo.soundHighScore) {
					if (trap_Cvar_VariableValue("sv_killserver") == 0) {
						// wait on server to go down before playing sound
						uiInfo.soundHighScore = qfalse;
					}
				}
			}
			flags &= ~UI_SHOW_NEWHIGHSCORE;
		} 
		if (flags & UI_SHOW_NEWBESTTIME) {
			if (uiInfo.newBestTime < uiInfo.uiDC.realTime) {
				vis = qfalse;
			}
			flags &= ~UI_SHOW_NEWBESTTIME;
		} 
		if (flags & UI_SHOW_DEMOAVAILABLE) {
			if (!uiInfo.demoAvailable) {
				vis = qfalse;
			}
			flags &= ~UI_SHOW_DEMOAVAILABLE;
		} else {
			flags = 0;
		}
	}
  return vis;
}

static qboolean UI_GameType_HandleKey(int flags, float *special, int key, qboolean resetMap) 
{
	if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) 
	{
		int oldCount = UI_MapCountByGameType();

		// hard coded mess here
		if (key == K_MOUSE2) 
		{
			ui_gameType.integer--;
			if (ui_gameType.integer == 2) 
			{
				ui_gameType.integer = 1;
			} 
			else if (ui_gameType.integer < 2) 
			{
				ui_gameType.integer = bg_gametypeCount - 1;
			}
		} 
		else 
		{
			ui_gameType.integer++;
			if (ui_gameType.integer >= bg_gametypeCount ) 
			{
				ui_gameType.integer = 1;
			} 
			else if (ui_gameType.integer == 2) 
			{
				ui_gameType.integer = 3;
			}
		}
    
		trap_Cvar_Set("ui_gameType", va("%d", ui_gameType.integer));
		
		if (resetMap && oldCount != UI_MapCountByGameType()) 
		{
	  		trap_Cvar_Set( "ui_currentMap", "0");
			Menu_SetFeederSelection(NULL, FEEDER_MAPS, 0, NULL);
		}

		return qtrue;
	}
	
	return qfalse;
}

static qboolean UI_NetGameType_HandleKey(int flags, float *special, int key) 
{
	if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) 
	{
		int oldMapIndex;
		int actual;

		if (key == K_MOUSE2) 
		{
			ui_netGameType.integer--;
		} 
		else 
		{
			ui_netGameType.integer++;
		}

		if (ui_netGameType.integer < 0) 
		{
			ui_netGameType.integer = bg_gametypeCount - 1;
		} 
		else if (ui_netGameType.integer >= bg_gametypeCount) 
		{
			ui_netGameType.integer = 0;
		} 

		if( uiInfo.mapCount )
		{
			oldMapIndex = ui_currentNetMap.integer;
		}
		else
		{
			oldMapIndex = 0;
		}

  		trap_Cvar_Set( "ui_netGameType", va("%d", ui_netGameType.integer));
  		trap_Cvar_Set( "ui_actualnetGameType", va("%d", ui_netGameType.integer));
		trap_Cvar_Set( "ui_gtRespawnType", va("%d", bg_gametypeData[ui_netGameType.integer].respawnType ) );
		trap_Cvar_Set( "ui_gtPickupsDisabled", va("%d", bg_gametypeData[ui_netGameType.integer].pickupsDisabled ) );

		UI_SelectedMap ( 0, &actual );
		ui_currentNetMap.integer = actual;
		trap_Cvar_Set("ui_currentNetMap", va("%d",actual));
		trap_Cvar_Set("ui_currentNetMapName", uiInfo.mapList[actual].mapLoadName );

		UI_MapCountByGameType();
		Menu_SetFeederSelection(NULL, FEEDER_ALLMAPS, 0, NULL);

		if ( oldMapIndex > 0)
		{
			if ( uiInfo.mapList[oldMapIndex].active )
			{
				Menu_SetFeederSelection ( NULL, FEEDER_ALLMAPS, UI_GetIndexFromSelection(oldMapIndex), NULL );
			}
		}		
		
		return qtrue;
	}

	return qfalse;
}

static qboolean UI_JoinGameType_HandleKey(int flags, float *special, int key) 
{
	if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) 
	{
		if (key == K_MOUSE2) 
		{
			ui_joinGameType.integer--;
		} 
		else 
		{
			ui_joinGameType.integer++;
		}

		if (ui_joinGameType.integer < 0) 
		{
			ui_joinGameType.integer = bg_gametypeCount;
		} 
		else if (ui_joinGameType.integer > bg_gametypeCount ) 
		{
			ui_joinGameType.integer = 0;
		}

		trap_Cvar_Set( "ui_joinGameType", va("%d", ui_joinGameType.integer));
		UI_BuildServerDisplayList(qtrue);
		return qtrue;
	}
	return qfalse;
}



static qboolean UI_Skill_HandleKey(int flags, float *special, int key) 
{
	if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) 
	{
		int i = trap_Cvar_VariableValue( "g_botSkill" );

		if (key == K_MOUSE2) 
		{
			i--;
		} 
		else 
		{
			i++;
		}

		if (i < 1) 
		{
			i = numSkillLevels;
		} 
		else if (i > numSkillLevels) 
		{
			i = 1;
		}

		trap_Cvar_Set("g_botSkill", va("%i", i));
		
		return qtrue;
	}
	
	return qfalse;
}

static qboolean UI_NetSource_HandleKey(int flags, float *special, int key) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) {
		
		if (key == K_MOUSE2) {
			ui_netSource.integer--;
		} else {
			ui_netSource.integer++;
		}
    
		if (ui_netSource.integer >= numNetSources) {
      ui_netSource.integer = 0;
    } else if (ui_netSource.integer < 0) {
      ui_netSource.integer = numNetSources - 1;
		}

		UI_BuildServerDisplayList(qtrue);
		if (ui_netSource.integer != AS_GLOBAL) {
			UI_StartServerRefresh(qtrue);
		}
  	trap_Cvar_Set( "ui_netSource", va("%d", ui_netSource.integer));
    return qtrue;
  }
  return qfalse;
}

static qboolean UI_NetFilter_HandleKey(int flags, float *special, int key) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) {

		if (key == K_MOUSE2) {
			ui_serverFilterType.integer--;
		} else {
			ui_serverFilterType.integer++;
		}

    if (ui_serverFilterType.integer >= numServerFilters) {
      ui_serverFilterType.integer = 0;
    } else if (ui_serverFilterType.integer < 0) {
      ui_serverFilterType.integer = numServerFilters - 1;
		}
		UI_BuildServerDisplayList(qtrue);
    return qtrue;
  }
  return qfalse;
}

static qboolean UI_BotName_HandleKey(int flags, float *special, int key) 
{
	if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) 
	{
		int value = uiInfo.botIndex;

		if (key == K_MOUSE2) 
		{
			value--;
		} 
		else 
		{
			value++;
		}

		if (value >= UI_GetNumBots() ) 
		{
			value = 0;
		} 
		else if (value < 0) 
		{
			value = UI_GetNumBots() - 1;
		}
		
		uiInfo.botIndex = value;
		
		return qtrue;
	}
	
	return qfalse;
}

static qboolean UI_BotSkill_HandleKey(int flags, float *special, int key) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) {
		if (key == K_MOUSE2) {
			uiInfo.skillIndex--;
		} else {
			uiInfo.skillIndex++;
		}
		if (uiInfo.skillIndex >= numSkillLevels) {
			uiInfo.skillIndex = 0;
		} else if (uiInfo.skillIndex < 0) {
			uiInfo.skillIndex = numSkillLevels-1;
		}
    return qtrue;
  }
	return qfalse;
}

static qboolean UI_RedBlue_HandleKey(int flags, float *special, int key) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) {
		uiInfo.redBlue ^= 1;
		return qtrue;
	}
	return qfalse;
}

static qboolean UI_Crosshair_HandleKey(int flags, float *special, int key) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) {
		if (key == K_MOUSE2) {
			uiInfo.currentCrosshair--;
		} else {
			uiInfo.currentCrosshair++;
		}

		if (uiInfo.currentCrosshair >= NUM_CROSSHAIRS) {
			uiInfo.currentCrosshair = 0;
		} else if (uiInfo.currentCrosshair < 0) {
			uiInfo.currentCrosshair = NUM_CROSSHAIRS - 1;
		}
		trap_Cvar_Set("cg_drawCrosshair", va("%d", uiInfo.currentCrosshair)); 
		return qtrue;
	}
	return qfalse;
}

static qboolean UI_OutfittingSlot_HandleKey ( int slot, int key )
{
	weapon_t weapon;

	switch ( key )
	{
		case K_MOUSE1:
		case K_ENTER:
		case K_RIGHTARROW:

			// Nothing to cycle?
			if ( bg_outfittings[uiInfo.outfittingItemGroup].items[slot] == -1 )
			{
				break;
			}

			do
			{
				if ( uiInfo.outfittingItemGroup != 0 )
				{
					memcpy ( &bg_outfittings[0].items, &bg_outfittings[uiInfo.outfittingItemGroup].items, sizeof(bg_outfittings[0].items) );
					Menu_SetFeederSelection(NULL, FEEDER_OUTFITTING_TEMPLATES, 0, "ingame_outfitting");
				}

				bg_outfittings[uiInfo.outfittingItemGroup].items[slot]++;
				if ( bg_outfittingGroups[slot][bg_outfittings[uiInfo.outfittingItemGroup].items[slot]] == -1 )
				{
					bg_outfittings[uiInfo.outfittingItemGroup].items[slot] = 0;
				}

				if ( bg_itemlist[bg_outfittingGroups[slot][bg_outfittings[uiInfo.outfittingItemGroup].items[slot]]].giType != IT_WEAPON )
				{
					break;
				}

				weapon = bg_itemlist[bg_outfittingGroups[slot][bg_outfittings[uiInfo.outfittingItemGroup].items[slot]]].giTag;
			}
			while ( !BG_IsWeaponAvailableForOutfitting ( weapon, 2 ) );

			return qtrue;

		case K_MOUSE2:
		case K_SPACE:
		case K_LEFTARROW:

			// Nothing to cycle?
			if ( bg_outfittings[uiInfo.outfittingItemGroup].items[slot] == -1 )
			{
				break;
			}

			do
			{
				if ( uiInfo.outfittingItemGroup != 0 )
				{
					memcpy ( &bg_outfittings[0].items, &bg_outfittings[uiInfo.outfittingItemGroup].items, sizeof(bg_outfittings[0].items) );
					Menu_SetFeederSelection(NULL, FEEDER_OUTFITTING_TEMPLATES, 0, "ingame_outfitting");
				}

				bg_outfittings[uiInfo.outfittingItemGroup].items[slot]--;
				if ( bg_outfittings[uiInfo.outfittingItemGroup].items[slot] < 0 )
				{
					bg_outfittings[uiInfo.outfittingItemGroup].items[slot] = MAX_OUTFITTING_GROUPITEM-1;
					// Find the first valid one starting from the back
					while ( bg_outfittingGroups[slot][bg_outfittings[uiInfo.outfittingItemGroup].items[slot]] == -1 )
					{
						bg_outfittings[uiInfo.outfittingItemGroup].items[slot]--;
					}
				}

				if ( bg_itemlist[bg_outfittingGroups[slot][bg_outfittings[uiInfo.outfittingItemGroup].items[slot]]].giType != IT_WEAPON )
				{
					break;
				}

				weapon = bg_itemlist[bg_outfittingGroups[slot][bg_outfittings[uiInfo.outfittingItemGroup].items[slot]]].giTag;
			}
			while ( !BG_IsWeaponAvailableForOutfitting ( weapon, 2 ) );

			return qtrue;
	}

	return qfalse;
}

static qboolean UI_SelectedPlayer_HandleKey(int flags, float *special, int key) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) {
		int selected;

		UI_BuildPlayerList();
		if (!uiInfo.teamLeader) {
			return qfalse;
		}
		selected = trap_Cvar_VariableValue("cg_selectedPlayer");
		
		if (key == K_MOUSE2) {
			selected--;
		} else {
			selected++;
		}

		if (selected > uiInfo.myTeamCount) {
			selected = 0;
		} else if (selected < 0) {
			selected = uiInfo.myTeamCount;
		}

		if (selected == uiInfo.myTeamCount) {
		 	trap_Cvar_Set( "cg_selectedPlayerName", "Everyone");
		} else {
		 	trap_Cvar_Set( "cg_selectedPlayerName", uiInfo.teamNames[selected]);
		}
	 	trap_Cvar_Set( "cg_selectedPlayer", va("%d", selected));
	}
	return qfalse;
}


static qboolean UI_OwnerDrawHandleKey ( int ownerDraw, int flags, float *special, int key, const char* param ) 
{
	switch (ownerDraw) 
	{
		case UI_GAMETYPE:
			return UI_GameType_HandleKey(flags, special, key, qtrue);

		case UI_NETGAMETYPE:
			return UI_NetGameType_HandleKey(flags, special, key);

		case UI_JOINGAMETYPE:
			return UI_JoinGameType_HandleKey(flags, special, key);

		case UI_SKILL:
			return UI_Skill_HandleKey(flags, special, key);

		case UI_NETSOURCE:
			UI_NetSource_HandleKey(flags, special, key);
			break;
		
		case UI_NETFILTER:
			UI_NetFilter_HandleKey(flags, special, key);
			break;
		
	
		case UI_BOTNAME:
			return UI_BotName_HandleKey(flags, special, key);

		case UI_BOTSKILL:
			return UI_BotSkill_HandleKey(flags, special, key);

		case UI_REDBLUE:
			UI_RedBlue_HandleKey(flags, special, key);
			break;

		case UI_CROSSHAIR:
			UI_Crosshair_HandleKey(flags, special, key);
			break;

		case UI_SELECTEDPLAYER:
			UI_SelectedPlayer_HandleKey(flags, special, key);
			break;
		
		case UI_OUTFITTING_SLOT:
			UI_OutfittingSlot_HandleKey ( atoi(param), key );
			break;

		default:
			break;
	}

	return qfalse;
}


static float UI_GetValue(int ownerDraw) {
  return 0;
}

/*
=================
UI_ServersQsortCompare
=================
*/
static int QDECL UI_ServersQsortCompare( const void *arg1, const void *arg2 ) {
	return trap_LAN_CompareServers( ui_netSource.integer, uiInfo.serverStatus.sortKey, uiInfo.serverStatus.sortDir, *(int*)arg1, *(int*)arg2);
}


/*
=================
UI_ServersSort
=================
*/
void UI_ServersSort(int column, qboolean force) {

	if ( !force ) {
		if ( uiInfo.serverStatus.sortKey == column ) {
			return;
		}
	}

	uiInfo.serverStatus.sortKey = column;
	qsort( &uiInfo.serverStatus.displayServers[0], uiInfo.serverStatus.numDisplayServers, sizeof(int), UI_ServersQsortCompare);
}
/*
===============
UI_LoadMods
===============
*/
static void UI_LoadMods() {
	int		numdirs;
	char	dirlist[2048];
	char	*dirptr;
  char  *descptr;
	int		i;
	int		dirlen;

	uiInfo.modCount = 0;
	numdirs = trap_FS_GetFileList( "$modlist", "", dirlist, sizeof(dirlist) );
	dirptr  = dirlist;
	for( i = 0; i < numdirs; i++ ) {
		dirlen = strlen( dirptr ) + 1;
    descptr = dirptr + dirlen;
		uiInfo.modList[uiInfo.modCount].modName = String_Alloc(dirptr);
		uiInfo.modList[uiInfo.modCount].modDescr = String_Alloc(descptr);
    dirptr += dirlen + strlen(descptr) + 1;
		uiInfo.modCount++;
		if (uiInfo.modCount >= MAX_MODS) {
			break;
		}
	}

}

/*
===============
UI_LoadMovies
===============
*/
static void UI_LoadMovies() {
	char	movielist[4096];
	char	*moviename;
	int		i, len;

	uiInfo.movieCount = trap_FS_GetFileList( "video", "roq", movielist, 4096 );

	if (uiInfo.movieCount) {
		if (uiInfo.movieCount > MAX_MOVIES) {
			uiInfo.movieCount = MAX_MOVIES;
		}
		moviename = movielist;
		for ( i = 0; i < uiInfo.movieCount; i++ ) {
			len = strlen( moviename );
			if (!Q_stricmp(moviename +  len - 4,".roq")) {
				moviename[len-4] = '\0';
			}
			Q_strupr(moviename);
			uiInfo.movieList[i] = String_Alloc(moviename);
			moviename += len + 1;
		}
	}

}

/*
===============
UI_LoadDemos
===============
*/
static void UI_LoadDemos() 
{
	char	demolist[4096];
	char demoExt[32];
	char	*demoname;
	int		i, len;

	Com_sprintf(demoExt, sizeof(demoExt), "dm_%d", (int)trap_Cvar_VariableValue("protocol"));

	uiInfo.demoCount = trap_FS_GetFileList( "demos", demoExt, demolist, 4096 );

	Com_sprintf(demoExt, sizeof(demoExt), ".dm_%d", (int)trap_Cvar_VariableValue("protocol"));

	if (uiInfo.demoCount) {
		if (uiInfo.demoCount > MAX_DEMOS) {
			uiInfo.demoCount = MAX_DEMOS;
		}
		demoname = demolist;
		for ( i = 0; i < uiInfo.demoCount; i++ ) {
			len = strlen( demoname );
			if (!Q_stricmp(demoname +  len - strlen(demoExt), demoExt)) {
				demoname[len-strlen(demoExt)] = '\0';
			}
			Q_strupr(demoname);
			uiInfo.demoList[i] = String_Alloc(demoname);
			demoname += len + 1;
		}
	}

}


static qboolean UI_SetNextMap(int actual, int index) {
	int i;
	for (i = actual + 1; i < uiInfo.mapCount; i++) {
		if (uiInfo.mapList[i].active) {
			Menu_SetFeederSelection(NULL, FEEDER_MAPS, index + 1, "skirmish");
			return qtrue;
		}
	}
	return qfalse;
}

static void UI_Update ( const char *name ) 
{
	int	val = trap_Cvar_VariableValue(name);

 	if (Q_stricmp(name, "ui_SetName") == 0) 
	{
		trap_Cvar_Set( "name", UI_Cvar_VariableString("ui_Name"));
 	} 
	else if (Q_stricmp(name, "ui_setRate") == 0) 
	{
		float rate = trap_Cvar_VariableValue("rate");
		if (rate >= 5000) 
		{
			trap_Cvar_Set("cl_maxpackets", "30");
			trap_Cvar_Set("cl_packetdup", "1");
		} 
		else if (rate >= 4000) 
		{
			trap_Cvar_Set("cl_maxpackets", "15");
			trap_Cvar_Set("cl_packetdup", "2");		// favor less prediction errors when there's packet loss
		} 
		else 
		{
			trap_Cvar_Set("cl_maxpackets", "15");
			trap_Cvar_Set("cl_packetdup", "1");		// favor lower bandwidth
		}
 	} 
	else if (Q_stricmp(name, "ui_GetName") == 0) 
	{
		trap_Cvar_Set( "ui_Name", UI_Cvar_VariableString("name"));
 	} 
	else if (Q_stricmp(name, "r_colorbits") == 0) 
	{
		switch (val) 
		{
			case 0:
				trap_Cvar_SetValue( "r_depthbits", 0 );
				trap_Cvar_SetValue( "r_stencilbits", 0 );
			break;
			case 16:
				trap_Cvar_SetValue( "r_depthbits", 16 );
				trap_Cvar_SetValue( "r_stencilbits", 0 );
			break;
			case 32:
				trap_Cvar_SetValue( "r_depthbits", 24 );
			break;
		}
	} 
	else if (Q_stricmp(name, "r_lodbias") == 0) 
	{
		switch (val) 
		{
			case 0:
				trap_Cvar_SetValue( "r_subdivisions", 4 );
				break;
			
			case 1:
				trap_Cvar_SetValue( "r_subdivisions", 12 );
				break;
			
			case 2:
				trap_Cvar_SetValue( "r_subdivisions", 20 );
				break;
		}
	} 
	else if (Q_stricmp(name, "ui_glCustom") == 0) 
	{
		switch (val) 
		{
			case 0:	// high quality
				trap_Cvar_SetValue( "r_fullScreen", 1 );
				trap_Cvar_SetValue( "r_subdivisions", 4 );
				trap_Cvar_SetValue( "r_lodbias", 0 );
				trap_Cvar_SetValue( "r_colorbits", 32 );
				trap_Cvar_SetValue( "r_depthbits", 24 );
				trap_Cvar_SetValue( "r_picmip", 1 );
				trap_Cvar_SetValue( "r_mode", 4 );
				trap_Cvar_SetValue( "r_texturebits", 32 );
				trap_Cvar_SetValue( "r_fastSky", 0 );
				trap_Cvar_SetValue( "r_inGameVideo", 1 );
				trap_Cvar_SetValue( "cg_shadows", 1 );
				trap_Cvar_Set( "r_texturemode", "GL_LINEAR_MIPMAP_LINEAR" );
				break;

			case 1: // normal 
				trap_Cvar_SetValue( "r_fullScreen", 1 );
				trap_Cvar_SetValue( "r_subdivisions", 12 );
				trap_Cvar_SetValue( "r_lodbias", 0 );
				trap_Cvar_SetValue( "r_colorbits", 0 );
				trap_Cvar_SetValue( "r_depthbits", 24 );
				trap_Cvar_SetValue( "r_picmip", 2 );
				trap_Cvar_SetValue( "r_mode", 3 );
				trap_Cvar_SetValue( "r_texturebits", 0 );
				trap_Cvar_SetValue( "r_fastSky", 0 );
				trap_Cvar_SetValue( "r_inGameVideo", 1 );
				trap_Cvar_Set( "r_texturemode", "GL_LINEAR_MIPMAP_LINEAR" );
				trap_Cvar_SetValue( "cg_shadows", 0 );
				break;

			case 2: // fast
				trap_Cvar_SetValue( "r_fullScreen", 1 );
				trap_Cvar_SetValue( "r_subdivisions", 8 );
				trap_Cvar_SetValue( "r_lodbias", 1 );
				trap_Cvar_SetValue( "r_colorbits", 0 );
				trap_Cvar_SetValue( "r_depthbits", 0 );
				trap_Cvar_SetValue( "r_picmip", 3 );
				trap_Cvar_SetValue( "r_mode", 3 );
				trap_Cvar_SetValue( "r_texturebits", 0 );
				trap_Cvar_SetValue( "cg_shadows", 0 );
				trap_Cvar_SetValue( "r_fastSky", 1 );
				trap_Cvar_SetValue( "r_inGameVideo", 0 );
				trap_Cvar_Set( "r_texturemode", "GL_LINEAR_MIPMAP_NEAREST" );
				break;

			case 3: // fastest
				trap_Cvar_SetValue( "r_fullScreen", 1 );
				trap_Cvar_SetValue( "r_subdivisions", 20 );
				trap_Cvar_SetValue( "r_lodbias", 2 );
				trap_Cvar_SetValue( "r_colorbits", 16 );
				trap_Cvar_SetValue( "r_depthbits", 16 );
				trap_Cvar_SetValue( "r_mode", 3 );
				trap_Cvar_SetValue( "r_picmip", 3 );
				trap_Cvar_SetValue( "r_texturebits", 16 );
				trap_Cvar_SetValue( "cg_shadows", 0 );
				trap_Cvar_SetValue( "r_fastSky", 1 );
				trap_Cvar_SetValue( "r_inGameVideo", 0 );
				trap_Cvar_Set( "r_texturemode", "GL_LINEAR_MIPMAP_NEAREST" );
				break;
		}
	} 
	else if (Q_stricmp(name, "ui_mousePitch") == 0) 
	{
		if (val == 0) 
		{
			trap_Cvar_SetValue( "m_pitch", 0.022f );
		} 
		else 
		{
			trap_Cvar_SetValue( "m_pitch", -0.022f );
		}
	}
}

/*
===============
UI_VerifyNetwork

Verifies that there is a network connection and if not dumps a com_error
===============
*/
qboolean UI_VerifyNetwork ( void )
{
	if ( !trap_NET_Available ( ) )
	{
		if ( !(int)trap_Cvar_VariableValue ( "com_ignoreverifynetwork" ) )
		{
			Menus_ActivateByName("verifynetwork_popmenu");
			return qfalse;
		}
	}

	return qtrue;
}

/*
===============
UI_RunMenuScript

Runs scripts from the menu
===============
*/
static void UI_RunMenuScript(const char **args) 
{
	const char *name, *name2;
	char buff[1024];

	if (String_Parse(args, &name)) 
	{		
		if ( Q_stricmp ( name, "LoadOutfittings" ) == 0 )
		{
			BG_SetAvailableOutfitting ( UI_Cvar_VariableString ( "ui_info_availableweapons" ) );
			
			BG_ParseOutfittingTemplates ( qfalse );
		}
		else if ( Q_stricmp ( name, "DecompressOutfitting" ) == 0 )
		{
			int				index;
			goutfitting_t	outfitting;

			BG_DecompressOutfitting ( UI_Cvar_VariableString ( "outfitting" ), &outfitting );

			index = BG_FindOutfitting ( &outfitting );
			if ( index < 0 )
			{
				memcpy ( &bg_outfittings[0].items[0], &outfitting.items[0], sizeof(outfitting.items) );
				Menu_SetFeederSelection(NULL, FEEDER_OUTFITTING_TEMPLATES, 0, "main");
			}
			else
			{
				Menu_SetFeederSelection(NULL, FEEDER_OUTFITTING_TEMPLATES, index, "main");
			}
		}
		else if ( Q_stricmp ( name, "CompressOutfitting" ) == 0 )
		{
			char compressed[OUTFITTING_GROUP_MAX + 1];
			BG_CompressOutfitting ( &bg_outfittings[uiInfo.outfittingItemGroup], compressed, OUTFITTING_GROUP_MAX );
			trap_Cvar_Set ( "outfitting", compressed );
		}
		// Verify that the user is connected and if not through a com_error
		else if ( Q_stricmp ( name, "VerifyNet" ) == 0 )
		{
			UI_VerifyNetwork ( );
		}
		else if ( Q_stricmp ( name, "autoTeamJoin" ) == 0 )
		{
			qboolean joinRed = qfalse;

			if ( !String_Parse(args, &name) || !name )
			{
				return;
			}

			if ( !String_Parse(args, &name2) || !name2 )
			{
				return;
			}

			// If the teams have the same number of players then join the 
			// team that has less points
			if ( ui_info_bluecount.integer == ui_info_redcount.integer )
			{
				// If the blue team has more points then join red
				if ( ui_info_bluescore.integer > ui_info_redscore.integer )
				{
					joinRed = qtrue;
				}
			}
			// If the blue team has more players then join red
			else if ( ui_info_bluecount.integer > ui_info_redcount.integer )
			{
				joinRed = qtrue;
			}
					
			if ( joinRed )
			{
				Menus_OpenByName ( name );
			}
			else 
			{
				Menus_OpenByName ( name2 );
			}
		}
		else if ( Q_stricmp ( name, "JoinTeam" ) == 0 )
		{
			int		index;
			team_t	team;

			if ( !String_Parse(args, &name) || !name )
			{
				return;
			}

			if ( !Int_Parse ( args, &index ) )
			{
				return;
			}

			if ( Q_stricmp ( name, "red" ) == 0 )
			{
				team = TEAM_RED;
			}
			else if ( Q_stricmp ( name, "blue" ) == 0 )
			{
				team = TEAM_BLUE;
			}
			else
			{	
				return;
			}

			if ( index < 0 || index > 4 )
			{
				index = 0;
			}

			if ( !uiInfo.identityTeams[team][index] )
			{
				return;
			}

			trap_Cmd_ExecuteText( EXEC_APPEND, va("cmd team %s %s\n", name, uiInfo.identityTeams[team][index]->mName ));
		}
		else if ( Q_stricmp ( name, "UpdateModel" ) == 0 )
		{
			updateModel = qtrue;
		}
		else if ( Q_stricmp ( name, "UpdateParental" ) == 0 )
		{
			char password[MAX_QPATH];
			trap_Parental_GetPassword ( password, MAX_QPATH );

			// As long as the password is correct, let them by
			if ( Q_stricmp ( password, uiInfo.parentalPassword ) == 0 )
			{
				trap_Parental_Update ( );
			}
		}
		else if ( Q_stricmp ( name, "OpenParentalPasswordMenu" ) == 0 )
		{
				trap_Cvar_Set ( "ui_lock_password2", "" );				
		}
		else if ( Q_stricmp ( name, "UpdateParentalPassword" ) == 0 )
		{
			char password[MAX_QPATH],password2[MAX_QPATH];

			trap_Cvar_VariableStringBuffer ( "ui_lock_password", password, MAX_QPATH );
			trap_Cvar_VariableStringBuffer ( "ui_lock_password2", password2, MAX_QPATH );

			if ( !Q_stricmp ( password, password2  ) )
			{
				trap_Cvar_VariableStringBuffer ( "ui_lock_password", uiInfo.parentalPassword , MAX_QPATH );
				trap_Parental_SetPassword ( uiInfo.parentalPassword );	

				trap_Cvar_Set ( "ui_lock_password", "" );

				Menus_CloseByName("violence_setpassword_popmenu");
			}
			else
			{
				trap_Cvar_Set ( "setpassword_error", "Password and Re-Enter Password must match." );
			}
		}
		else if (Q_stricmp(name,"violence_options") == 0 ) 
		{
			char password[MAX_QPATH];
			trap_Parental_GetPassword ( password, MAX_QPATH );
			trap_Cvar_VariableStringBuffer ( "ui_lock_password", uiInfo.parentalPassword, MAX_QPATH );

			if ( !Q_stricmp ( password, uiInfo.parentalPassword  ) )
			{
				Menus_CloseByName ( "violence_menu" );
				Menus_OpenByName ( "violence_options_menu" );
				trap_Cvar_Set ( "parental_password_error", " " );
			}
			else
			{
				trap_Cvar_Set ( "parental_password_error", "Incorrect password." );
			}
			trap_Cvar_Set ( "ui_lock_password", "" );
		} 
		else if (Q_stricmp(name, "SetConnectPassword") == 0)
		{
			char password[MAX_QPATH];
			uiInfo.connectPasswordRequest = qfalse;
			trap_Cvar_VariableStringBuffer ( "ui_connect_password", password, MAX_QPATH );
			trap_Cvar_Set ( "password", password );
			trap_Cvar_Set ( "ui_connect_password", "" );
		}
		else if (Q_stricmp(name, "StartServer") == 0) 
		{
#ifdef _SOF2_BOTS
			int		clients;
			int		oldclients;
			float	skill;
#endif

			trap_Cvar_Set("cg_thirdPerson", "0");
			trap_Cvar_Set("cg_cameraOrbit", "0");
			trap_Cvar_SetValue( "dedicated", Com_Clamp( 0, 2, ui_dedicated.integer ) );
			trap_Cvar_Set ( "g_gametype", bg_gametypeData[ui_netGameType.integer].name );

			// Must be connected
			if ( ui_dedicated.integer > 0 )
			{
				if ( !UI_VerifyNetwork ( ) )
				{
					return;
				}
			}

			// Handle randomly generated maps specially
			if ( !Q_stricmp ( uiInfo.mapList[ui_currentNetMap.integer].mapLoadName, "*random" ) )
			{
				char	rmgConfig[MAX_QPATH];
				char	rmgSize[MAX_QPATH];
				char	rmgTime[MAX_QPATH];
				char	rmgSeed[MAX_QPATH];


				trap_Cvar_VariableStringBuffer ( "ui_rmg_size", rmgSize, MAX_QPATH );
				trap_Cvar_VariableStringBuffer ( "ui_rmg_config", rmgConfig, MAX_QPATH );
				trap_Cvar_VariableStringBuffer ( "ui_rmg_time", rmgTime, MAX_QPATH );
				trap_Cvar_VariableStringBuffer ( "ui_rmg_seed", rmgSeed, MAX_QPATH );

				trap_Cmd_ExecuteText ( EXEC_APPEND, va( "wait ; wait ; rmgmap 1 \"%s\" 2 \"%s\" 3 \"%s\" 4 \"%s\" 0\n", 
					rmgSize, rmgConfig, rmgTime, rmgSeed  ) );
			}
			else
			{
				trap_Cmd_ExecuteText ( EXEC_APPEND, va( "wait ; wait ; map %s\n", uiInfo.mapList[ui_currentNetMap.integer].mapLoadName ) );
			}
#ifdef _SOF2_BOTS

			skill = trap_Cvar_VariableValue( "g_botSkill" );	
	
			// set max clients based on spots
			oldclients = trap_Cvar_VariableValue( "sv_maxClients" );
			clients    = 0;

			if (clients == 0) 
			{
				clients = 8;
			}
			
			if (oldclients > clients) 
			{
				clients = oldclients;
			}

			trap_Cvar_Set("sv_maxClients", va("%d",clients));
#endif
		} 
		else if (Q_stricmp(name, "resetDefaults") == 0) 
		{
			trap_Cmd_ExecuteText( EXEC_APPEND, "exec sof2mp_default.cfg\n");
			trap_Cmd_ExecuteText( EXEC_APPEND, "cvar_restart\n");
			Controls_SetDefaults();
			trap_Cmd_ExecuteText( EXEC_APPEND, "vid_restart\n" );
		} 
		else if (Q_stricmp(name, "getCDKey") == 0) {
			char out[17];
			trap_GetCDKey(buff, 17);
			trap_Cvar_Set("cdkey1", "");
			trap_Cvar_Set("cdkey2", "");
			trap_Cvar_Set("cdkey3", "");
			trap_Cvar_Set("cdkey4", "");
			trap_Cvar_Set("cdkeychecksum", "");
			trap_Cvar_Set("cdkey_error", "");
			
			if (strlen(buff) == CDKEY_LEN) 
			{
				Q_strncpyz(out, buff, 5);
				trap_Cvar_Set("cdkey1", out);
				Q_strncpyz(out, buff + 4, 5);
				trap_Cvar_Set("cdkey2", out);
				Q_strncpyz(out, buff + 8, 5);
				trap_Cvar_Set("cdkey3", out);
				Q_strncpyz(out, buff + 12, 5);
				trap_Cvar_Set("cdkey4", out);
				Q_strncpyz(out, buff + 16, 2);
				trap_Cvar_Set("cdkeychecksum", out);
			}

		} else if (Q_stricmp(name, "verifyCDKey") == 0) {
			buff[0] = '\0';
			Q_strcat(buff, 1024, UI_Cvar_VariableString("cdkey1")); 
			Q_strcat(buff, 1024, UI_Cvar_VariableString("cdkey2")); 
			Q_strcat(buff, 1024, UI_Cvar_VariableString("cdkey3")); 
			Q_strcat(buff, 1024, UI_Cvar_VariableString("cdkey4")); 
			Q_strcat(buff, 1024, UI_Cvar_VariableString("cdkeychecksum")); 
			trap_Cvar_Set("cdkey", buff);

			if (trap_VerifyCDKey(buff)) 
			{
				trap_Cvar_Set("cdkey_error", "Valid CD Key.");
				trap_SetCDKey(buff);
				Menus_CloseByName ( "cdkey_popmenu" );
			} 
			else 
			{
				trap_Cvar_Set("cdkey_error", "Invalid CD Key.");
			}			
		} 
		else if (Q_stricmp(name, "loadArenas") == 0) 
		{
			int actual;

			UI_LoadArenas();
			UI_MapCountByGameType();
			Menu_SetFeederSelection(NULL, FEEDER_ALLMAPS, 0, "createserver_menu");

			trap_Cvar_Set("ui_mapIndex", "0");
			ui_mapIndex.integer = 0;
			UI_SelectedMap ( 0, &actual );
			ui_currentNetMap.integer = actual;
			trap_Cvar_Set("ui_currentNetMap", va("%d",actual));
			trap_Cvar_Set("ui_currentNetMapName", uiInfo.mapList[actual].mapLoadName );
		} 
		else if (Q_stricmp(name, "loadVoteArenas") == 0) 
		{
			int actual;

			UI_LoadArenas();
			UI_MapCountForVote();
			Menu_SetFeederSelection(NULL, FEEDER_VOTEMAPS, 0, "ingame_callvote");

			trap_Cvar_Set("ui_mapIndex", "0");
			ui_mapIndex.integer = 0;
			UI_SelectedMap ( 0, &actual );
			ui_currentNetMap.integer = actual;
			trap_Cvar_Set("ui_currentNetMap", va("%d",actual));
			trap_Cvar_Set("ui_currentNetMapName", uiInfo.mapList[actual].mapLoadName );
		} 
		else if (Q_stricmp(name, "saveControls") == 0) {
			Controls_SetConfig(qtrue);
		} else if (Q_stricmp(name, "loadControls") == 0) {
			Controls_GetConfig();
		} else if (Q_stricmp(name, "clearError") == 0) {
			trap_Cvar_Set("com_errorMessage", "");
		} 
		else if (Q_stricmp(name, "loadGameInfo") == 0) 
		{
			BG_BuildGametypeList ( );				
		} else if (Q_stricmp(name, "RefreshServers") == 0) {
			UI_StartServerRefresh(qtrue);
			UI_BuildServerDisplayList(qtrue);
		} else if (Q_stricmp(name, "RefreshFilter") == 0) {
			UI_StartServerRefresh(qfalse);
			UI_BuildServerDisplayList(qtrue);
		} else if (Q_stricmp(name, "RunSPDemo") == 0) {
			if (uiInfo.demoAvailable) {
			  trap_Cmd_ExecuteText( EXEC_APPEND, va("demo %s_%s\n", uiInfo.mapList[ui_currentMap.integer].mapLoadName, bg_gametypeData[ui_gameType.integer].name));
			}
		} else if (Q_stricmp(name, "LoadDemos") == 0) {
			UI_LoadDemos();
		} else if (Q_stricmp(name, "LoadMovies") == 0) {
			UI_LoadMovies();
		} else if (Q_stricmp(name, "LoadMods") == 0) {
			UI_LoadMods();
		} else if (Q_stricmp(name, "playMovie") == 0) {
			if (uiInfo.previewMovie >= 0) {
			  trap_CIN_StopCinematic(uiInfo.previewMovie);
			}
			trap_Cmd_ExecuteText( EXEC_APPEND, va("cinematic %s.roq 2\n", uiInfo.movieList[uiInfo.movieIndex]));
		} else if (Q_stricmp(name, "RunMod") == 0) {
			trap_Cvar_Set( "fs_game", uiInfo.modList[uiInfo.modIndex].modName);
			trap_Cmd_ExecuteText( EXEC_APPEND, "vid_restart;" );
		} else if (Q_stricmp(name, "RunDemo") == 0) {
			trap_Cmd_ExecuteText( EXEC_APPEND, va("demo \"%s\"\n", uiInfo.demoList[uiInfo.demoIndex]));
		} else if (Q_stricmp(name, "Quake3") == 0) {
			trap_Cvar_Set( "fs_game", "");
			trap_Cmd_ExecuteText( EXEC_APPEND, "vid_restart;" );
		} else if (Q_stricmp(name, "closeJoin") == 0) {
			if (uiInfo.serverStatus.refreshActive) {
				UI_StopServerRefresh();
				uiInfo.serverStatus.nextDisplayRefresh = 0;
				uiInfo.nextServerStatusRefresh = 0;
				uiInfo.nextFindPlayerRefresh = 0;
				UI_BuildServerDisplayList(qtrue);
			} else {
				Menus_CloseByName("joinserver");
				Menus_OpenByName("main");
			}
		} else if (Q_stricmp(name, "StopRefresh") == 0) {
			UI_StopServerRefresh();
			uiInfo.serverStatus.nextDisplayRefresh = 0;
			uiInfo.nextServerStatusRefresh = 0;
			uiInfo.nextFindPlayerRefresh = 0;
		} else if (Q_stricmp(name, "UpdateFilter") == 0) {
			if (ui_netSource.integer == AS_LOCAL) {
				UI_StartServerRefresh(qtrue);
			}
			UI_BuildServerDisplayList(qtrue);
			UI_FeederSelection(FEEDER_SERVERS, 0);
		} else if (Q_stricmp(name, "ServerStatus") == 0) {
			trap_LAN_GetServerAddressString(ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.serverStatus.currentServer], uiInfo.serverStatusAddress, sizeof(uiInfo.serverStatusAddress));
			UI_BuildServerStatus(qtrue);
		} else if (Q_stricmp(name, "FoundPlayerServerStatus") == 0) {
			Q_strncpyz(uiInfo.serverStatusAddress, uiInfo.foundPlayerServerAddresses[uiInfo.currentFoundPlayerServer], sizeof(uiInfo.serverStatusAddress));
			UI_BuildServerStatus(qtrue);
			Menu_SetFeederSelection(NULL, FEEDER_FINDPLAYER, 0, NULL);
		} else if (Q_stricmp(name, "FindPlayer") == 0) {
			UI_BuildFindPlayerList(qtrue);
			// clear the displayed server status info
			uiInfo.serverStatusInfo.numLines = 0;
			Menu_SetFeederSelection(NULL, FEEDER_FINDPLAYER, 0, NULL);
		} else if (Q_stricmp(name, "JoinServer") == 0) {
			trap_Cvar_Set("cg_thirdPerson", "0");
			trap_Cvar_Set("cg_cameraOrbit", "0");
			if (uiInfo.serverStatus.currentServer >= 0 && uiInfo.serverStatus.currentServer < uiInfo.serverStatus.numDisplayServers) {
				trap_LAN_GetServerAddressString(ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.serverStatus.currentServer], buff, 1024);
				trap_Cmd_ExecuteText( EXEC_APPEND, va( "connect %s\n", buff ) );
			}
		} else if (Q_stricmp(name, "FoundPlayerJoinServer") == 0) {
			if (uiInfo.currentFoundPlayerServer >= 0 && uiInfo.currentFoundPlayerServer < uiInfo.numFoundPlayerServers) {
				trap_Cmd_ExecuteText( EXEC_APPEND, va( "connect %s\n", uiInfo.foundPlayerServerAddresses[uiInfo.currentFoundPlayerServer] ) );
			}
		} 
		else if (Q_stricmp(name, "Quit") == 0) {
			trap_Cmd_ExecuteText( EXEC_NOW, "quit");
		} else if (Q_stricmp(name, "Controls") == 0) {
		  trap_Cvar_Set( "cl_paused", "1" );
			trap_Key_SetCatcher( KEYCATCH_UI );
			Menus_CloseAll();
			Menus_ActivateByName("setup_menu2");
		} else if (Q_stricmp(name, "Leave") == 0) 
		{
			uiInfo.connectPasswordRequest = qfalse;
			trap_Cmd_ExecuteText( EXEC_APPEND, "disconnect\n" );
			trap_Key_SetCatcher( KEYCATCH_UI );
			Menus_CloseAll();
			Menus_ActivateByName("main");
		} else if (Q_stricmp(name, "ServerSort") == 0) {
			int sortColumn;
			if (Int_Parse(args, &sortColumn)) {
				// if same column we're already sorting on then flip the direction
				if (sortColumn == uiInfo.serverStatus.sortKey) {
					uiInfo.serverStatus.sortDir = !uiInfo.serverStatus.sortDir;
				}
				// make sure we sort again
				UI_ServersSort(sortColumn, qtrue);
			}
		} else if (Q_stricmp(name, "closeingame") == 0) {
			trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_UI );
			trap_Key_ClearStates();
			trap_Cvar_Set( "cl_paused", "0" );
			Menus_CloseAll();
		} 
		else if (Q_stricmp(name, "voteMap") == 0) 
		{
			if (ui_currentNetMap.integer >=0 && ui_currentNetMap.integer < uiInfo.mapCount) 
			{
				trap_Cmd_ExecuteText( EXEC_APPEND, va("callvote map %s\n",uiInfo.mapList[ui_currentNetMap.integer].mapLoadName) );
			}
		} 
		else if (Q_stricmp(name, "voteKick") == 0) 
		{
			if (uiInfo.playerIndex >= 0 && uiInfo.playerIndex < uiInfo.playerCount) 
			{
				char clientnum[3];
				
				// The client number is hidden in front of the string
				strncpy ( clientnum, uiInfo.playerNames[uiInfo.playerIndex], 2 );
				clientnum[2] = 0;

				trap_Cmd_ExecuteText( EXEC_APPEND, va("callvote clientkick \"%s\"\n",clientnum) );
			}
		} 
		else if (Q_stricmp(name, "voteGame") == 0) 
		{
			if (ui_netGameType.integer >= 0 && ui_netGameType.integer < bg_gametypeCount ) 
			{
				trap_Cmd_ExecuteText( EXEC_APPEND, va("callvote g_gametype %s\n",bg_gametypeData[ui_netGameType.integer].name) );
			}
		}
		else if ( Q_stricmp ( name, "voteRMG" ) == 0 )
		{
			char	rmgConfig[MAX_QPATH];
			char	rmgSize[MAX_QPATH];
			char	rmgTime[MAX_QPATH];
			char	rmgSeed[MAX_QPATH];

			trap_Cvar_VariableStringBuffer ( "ui_rmg_size", rmgSize, MAX_QPATH );
			trap_Cvar_VariableStringBuffer ( "ui_rmg_config", rmgConfig, MAX_QPATH );
			trap_Cvar_VariableStringBuffer ( "ui_rmg_time", rmgTime, MAX_QPATH );
			trap_Cvar_VariableStringBuffer ( "ui_rmg_seed", rmgSeed, MAX_QPATH );

			trap_Cmd_ExecuteText ( EXEC_APPEND, va( "callvote rmgmap \"%s\" \"%s\" \"%s\" \"%s\"\n", 
								   rmgSize, rmgConfig, rmgTime, rmgSeed  ) );			
		} 
		else if (Q_stricmp(name, "loadNewVersionFromBrowser") == 0) 
		{
			trap_Version_Download(VD_BROWSER_DOWNLOAD);
		} 
		else if (Q_stricmp(name, "ignoreCurrentVersion") == 0) 
		{
			Menus_CloseByName("invalid_version_ignore_menu");
			Menus_CloseByName("invalid_version_menu");
			trap_Version_Download(VD_IGNORE);
		}
		else if (Q_stricmp(name, "loadNewVersion") == 0) 
		{
			Menus_ActivateByName("patch_download");
			trap_Version_Download(uiInfo.versionIndex);
		} 
		else if (Q_stricmp(name, "cancelDownload") == 0) 
		{
			trap_Version_Download(VD_CANCEL);
		}
		else if (Q_stricmp(name, "addBot") == 0) 
		{
			trap_Cmd_ExecuteText( EXEC_APPEND, va("addbot %s %i %s\n", UI_GetBotNameByNumber(uiInfo.botIndex), uiInfo.skillIndex+1, ui_botteam.string ) );
		} 
		else if (Q_stricmp(name, "addFavorite") == 0) 
		{
			if (ui_netSource.integer != AS_FAVORITES) {
				char name[MAX_NAME_LENGTH];
				char addr[MAX_NAME_LENGTH];
				int res;

				trap_LAN_GetServerInfo(ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.serverStatus.currentServer], buff, MAX_STRING_CHARS);
				name[0] = addr[0] = '\0';
				Q_strncpyz(name, 	Info_ValueForKey(buff, "hostname"), MAX_NAME_LENGTH);
				Q_strncpyz(addr, 	Info_ValueForKey(buff, "addr"), MAX_NAME_LENGTH);
				if (strlen(name) > 0 && strlen(addr) > 0) {
					res = trap_LAN_AddServer(AS_FAVORITES, name, addr);
					if (res == 0) {
						// server already in the list
						Com_Printf("Favorite already in list\n");
					}
					else if (res == -1) {
						// list full
						Com_Printf("Favorite list full\n");
					}
					else {
						// successfully added
						Com_Printf("Added favorite server %s\n", addr);
					}
				}
			}
		} 
		else if (Q_stricmp(name, "deleteFavorite") == 0) 
		{
			if (ui_netSource.integer == AS_FAVORITES) 
			{
				char addr[MAX_NAME_LENGTH];

				trap_LAN_GetServerInfo(ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.serverStatus.currentServer], buff, MAX_STRING_CHARS);
				addr[0] = '\0';
				Q_strncpyz(addr, 	Info_ValueForKey(buff, "addr"), MAX_NAME_LENGTH);
				if (strlen(addr) > 0) {
					trap_LAN_RemoveServer(AS_FAVORITES, addr);
				}

				UI_BuildServerDisplayList(qtrue);
			}
		} 
		else if (Q_stricmp(name, "CancelCreateFavorite") == 0) 
		{
			trap_Cvar_Set( "fave_ip_error", " " );				
			trap_Cvar_Set( "ui_favoriteAddress", " " );				
		}
		else if (Q_stricmp(name, "createFavorite") == 0) 
		{
			char	name[MAX_NAME_LENGTH];
			char	addr[MAX_NAME_LENGTH];
			int		res;

			name[0] = addr[0] = '\0';
			Q_strncpyz(name, 	UI_Cvar_VariableString("ui_favoriteAddress"), MAX_NAME_LENGTH);
			Q_strncpyz(addr, 	UI_Cvar_VariableString("ui_favoriteAddress"), MAX_NAME_LENGTH);

			if ( strlen(name) > 0 && strlen(addr) > 0) 
			{
				res = trap_LAN_AddServer(AS_FAVORITES, name, addr);
				if (res == 0) 
				{
					// server already in the list
					Com_Printf("Favorite already in list\n");
				}
				else if (res == -1) 
				{
					// list full
					Com_Printf("Favorite list full\n");
				}
				else if (res == -2) 
				{
					trap_Cvar_Set( "fave_ip_error", "Invalid address." );				
				}
				else 
				{
					// successfully added
					Com_Printf("Added favorite server %s\n", addr);
					trap_Cvar_Set( "fave_ip_error", " " );				
					trap_Cvar_Set( "ui_favoriteAddress", " " );				
					Menus_CloseByName("createfavorite_popmenu");
				}

				UI_BuildServerDisplayList(qtrue);
			}
		} 
		else if (Q_stricmp(name, "display_settings_store") == 0)
		{	
			trap_Cvar_SetValue( "ui_glCustom_store", trap_Cvar_VariableValue("ui_glCustom") );
			trap_Cvar_SetValue( "r_fullscreen_store",trap_Cvar_VariableValue("r_fullscreen") );
			trap_Cvar_SetValue( "r_allowExtensions_store", trap_Cvar_VariableValue("r_allowExtensions") );
			trap_Cvar_SetValue( "r_mode_store", trap_Cvar_VariableValue("r_mode") );
			trap_Cvar_SetValue( "r_colorbits_store",trap_Cvar_VariableValue("r_colorbits") );
			trap_Cvar_SetValue( "r_lodbias_store", trap_Cvar_VariableValue("r_lodbias") );
			trap_Cvar_SetValue( "r_picmip_store", trap_Cvar_VariableValue("r_picmip") );
			trap_Cvar_SetValue( "r_texturebits_store", trap_Cvar_VariableValue("r_texturebits") );
			trap_Cvar_SetValue( "r_texturemode_store", trap_Cvar_VariableValue("r_texturemode") );
			trap_Cvar_SetValue( "r_ext_compressed_textures_store", trap_Cvar_VariableValue("r_ext_compressed_textures") ); 
		}
		else if (Q_stricmp(name, "display_settings_recall") == 0)
		{
			trap_Cvar_SetValue( "ui_glCustom", trap_Cvar_VariableValue("ui_glCustom_store") );
			trap_Cvar_SetValue( "r_fullscreen",trap_Cvar_VariableValue("r_fullscreen_store") );
			trap_Cvar_SetValue( "r_allowExtensions", trap_Cvar_VariableValue("r_allowExtensions_store") );
			trap_Cvar_SetValue( "r_mode", trap_Cvar_VariableValue("r_mode_store") );
			trap_Cvar_SetValue( "r_colorbits",trap_Cvar_VariableValue("r_colorbits_store") );
			trap_Cvar_SetValue( "r_lodbias", trap_Cvar_VariableValue("r_lodbias_store") );
			trap_Cvar_SetValue( "r_picmip", trap_Cvar_VariableValue("r_picmip_store") );
			trap_Cvar_SetValue( "r_texturebits", trap_Cvar_VariableValue("r_texturebits_store") );
			trap_Cvar_SetValue( "r_texturemode", trap_Cvar_VariableValue("r_texturemode_store") );
			trap_Cvar_SetValue( "r_ext_compressed_textures", trap_Cvar_VariableValue("r_ext_compressed_textures_store") ); 
		}
		else if (Q_stricmp(name, "orders") == 0) {
			const char *orders;
			if (String_Parse(args, &orders)) {
				int selectedPlayer = trap_Cvar_VariableValue("cg_selectedPlayer");
				if (selectedPlayer < uiInfo.myTeamCount) {
					strcpy(buff, orders);
					trap_Cmd_ExecuteText( EXEC_APPEND, va(buff, uiInfo.teamClientNums[selectedPlayer]) );
					trap_Cmd_ExecuteText( EXEC_APPEND, "\n" );
				} else {
					int i;
					for (i = 0; i < uiInfo.myTeamCount; i++) {
						if (Q_stricmp(UI_Cvar_VariableString("name"), uiInfo.teamNames[i]) == 0) {
							continue;
						}
						strcpy(buff, orders);
						trap_Cmd_ExecuteText( EXEC_APPEND, va(buff, uiInfo.teamNames[i]) );
						trap_Cmd_ExecuteText( EXEC_APPEND, "\n" );
					}
				}
				trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_UI );
				trap_Key_ClearStates();
				trap_Cvar_Set( "cl_paused", "0" );
				Menus_CloseAll();
			}
		}
		else if (Q_stricmp(name, "voiceOrdersTeam") == 0) {
			const char *orders;
			if (String_Parse(args, &orders)) {
				int selectedPlayer = trap_Cvar_VariableValue("cg_selectedPlayer");
				if (selectedPlayer == uiInfo.myTeamCount) {
					trap_Cmd_ExecuteText( EXEC_APPEND, orders );
					trap_Cmd_ExecuteText( EXEC_APPEND, "\n" );
				}
				trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_UI );
				trap_Key_ClearStates();
				trap_Cvar_Set( "cl_paused", "0" );
				Menus_CloseAll();
			}
		} else if (Q_stricmp(name, "voiceOrders") == 0) {
			const char *orders;
			if (String_Parse(args, &orders)) {
				int selectedPlayer = trap_Cvar_VariableValue("cg_selectedPlayer");
				if (selectedPlayer < uiInfo.myTeamCount) {
					strcpy(buff, orders);
					trap_Cmd_ExecuteText( EXEC_APPEND, va(buff, uiInfo.teamClientNums[selectedPlayer]) );
					trap_Cmd_ExecuteText( EXEC_APPEND, "\n" );
				}
				trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_UI );
				trap_Key_ClearStates();
				trap_Cvar_Set( "cl_paused", "0" );
				Menus_CloseAll();
			}
		}
		else if (Q_stricmp(name, "RMGRandomSettings") == 0) 
		{
			trap_Cvar_Set( "ui_rmg_config", "?" );
			trap_Cvar_Set( "ui_rmg_time", "?" );				
			trap_Cvar_Set( "ui_rmg_seed", "?" );				
		} else if (Q_stricmp(name, "glCustom") == 0) {
			trap_Cvar_Set("ui_glCustom", "4");
		} else if (Q_stricmp(name, "update") == 0) {
			if (String_Parse(args, &name2)) {
				UI_Update(name2);
		}
		else {
			Com_Printf("unknown UI script %s\n", name);
			}
		}
	}
}

static void UI_GetTeamColor(vec4_t *color) {
}

/*
==================
UI_MapCountByGameType

returns the number of available maps for the netgametype as well as
marks the active maps active
==================
*/
static int UI_MapCountByGameType( void ) 
{
	int i, c, game;
	c = 0;
	game = ui_netGameType.integer;

	for (i = 0; i < uiInfo.mapCount; i++) 
	{
		uiInfo.mapList[i].active = qfalse;
		if ( uiInfo.mapList[i].typeBits & (1 << game)) 
		{
			c++;
			uiInfo.mapList[i].active = qtrue;
		}
	}
	return c;
}

/*
==================
UI_MapCountForVote

returns the number of maps available for a vote
==================
*/
static int UI_MapCountForVote ( void )
{
	int i;
	int	c;

	c = 0;

	for (i = 0; i < uiInfo.mapCount; i++) 
	{
		uiInfo.mapList[i].active = qfalse;

		// The random map is prefaced by a *
		if ( uiInfo.mapList[i].mapLoadName[0] != '*' ) 
		{
			c++;
			uiInfo.mapList[i].active = qtrue;
		}
	}

	return c;
}

/*
==================
UI_InsertServerIntoDisplayList
==================
*/
static void UI_InsertServerIntoDisplayList(int num, int position) 
{
	int i;

	if (position < 0 || position > uiInfo.serverStatus.numDisplayServers ) 
	{
		return;
	}
	
	uiInfo.serverStatus.numDisplayServers++;
	for (i = uiInfo.serverStatus.numDisplayServers; i > position; i--) 
	{
		uiInfo.serverStatus.displayServers[i] = uiInfo.serverStatus.displayServers[i-1];
	}

	uiInfo.serverStatus.displayServers[position] = num;
}

/*
==================
UI_RemoveServerFromDisplayList
==================
*/
static void UI_RemoveServerFromDisplayList(int num) {
	int i, j;

	for (i = 0; i < uiInfo.serverStatus.numDisplayServers; i++) {
		if (uiInfo.serverStatus.displayServers[i] == num) {
			uiInfo.serverStatus.numDisplayServers--;
			for (j = i; j < uiInfo.serverStatus.numDisplayServers; j++) {
				uiInfo.serverStatus.displayServers[j] = uiInfo.serverStatus.displayServers[j+1];
			}
			return;
		}
	}
}

/*
==================
UI_BinaryServerInsertion
==================
*/
static void UI_BinaryServerInsertion(int num) {
	int mid, offset, res, len;

	// use binary search to insert server
	len = uiInfo.serverStatus.numDisplayServers;
	mid = len;
	offset = 0;
	res = 0;
	while(mid > 0) {
		mid = len >> 1;
		//
		res = trap_LAN_CompareServers( ui_netSource.integer, uiInfo.serverStatus.sortKey,
					uiInfo.serverStatus.sortDir, num, uiInfo.serverStatus.displayServers[offset+mid]);
		// if equal
		if (res == 0) {
			UI_InsertServerIntoDisplayList(num, offset+mid);
			return;
		}
		// if larger
		else if (res == 1) {
			offset += mid;
			len -= mid;
		}
		// if smaller
		else {
			len -= mid;
		}
	}
	if (res == 1) {
		offset++;
	}
	UI_InsertServerIntoDisplayList(num, offset);
}

/*
==================
UI_BuildServerDisplayList
==================
*/
static void UI_BuildServerDisplayList(qboolean force) {
	int i, count, clients, maxClients, ping, game, len, visible;
	char info[MAX_STRING_CHARS];
//	qboolean startRefresh = qtrue; TTimo: unused
	static int numinvisible;

	if (!(force || uiInfo.uiDC.realTime > uiInfo.serverStatus.nextDisplayRefresh)) {
		return;
	}
	// if we shouldn't reset
	if ( force == 2 ) {
		force = 0;
	}

	// do motd updates here too
	trap_Cvar_VariableStringBuffer( "cl_motdString", uiInfo.serverStatus.motd, sizeof(uiInfo.serverStatus.motd) );
	len = strlen(uiInfo.serverStatus.motd);
	if (len == 0) {
		strcpy(uiInfo.serverStatus.motd, "Welcome to Team Arena!");
		len = strlen(uiInfo.serverStatus.motd);
	} 
	if (len != uiInfo.serverStatus.motdLen) {
		uiInfo.serverStatus.motdLen = len;
		uiInfo.serverStatus.motdWidth = -1;
	} 

	if (force) {
		numinvisible = 0;
		// clear number of displayed servers
		uiInfo.serverStatus.numDisplayServers = 0;
		uiInfo.serverStatus.numPlayersOnServers = 0;
		// set list box index to zero
		Menu_SetFeederSelection(NULL, FEEDER_SERVERS, 0, NULL);
		// mark all servers as visible so we store ping updates for them
		trap_LAN_MarkServerVisible(ui_netSource.integer, -1, qtrue);
	}

	// get the server count (comes from the master)
	count = trap_LAN_GetServerCount(ui_netSource.integer);
	if (count == -1 || (ui_netSource.integer == AS_LOCAL && count == 0) ) {
		// still waiting on a response from the master
		uiInfo.serverStatus.numDisplayServers = 0;
		uiInfo.serverStatus.numPlayersOnServers = 0;
		uiInfo.serverStatus.nextDisplayRefresh = uiInfo.uiDC.realTime + 500;
		return;
	}

	visible = qfalse;
	for (i = 0; i < count; i++) {
		// if we already got info for this server
		if (!trap_LAN_ServerIsVisible(ui_netSource.integer, i)) {
			continue;
		}
		visible = qtrue;
		// get the ping for this server
		ping = trap_LAN_GetServerPing(ui_netSource.integer, i);
		if (ping > 0 || ui_netSource.integer == AS_FAVORITES) {

			trap_LAN_GetServerInfo(ui_netSource.integer, i, info, MAX_STRING_CHARS);

			clients = atoi(Info_ValueForKey(info, "clients"));
			uiInfo.serverStatus.numPlayersOnServers += clients;

			if (ui_browserShowEmpty.integer == 0) {
				if (clients == 0) {
					trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
					continue;
				}
			}

			if (ui_browserShowFull.integer == 0) {
				maxClients = atoi(Info_ValueForKey(info, "sv_maxclients"));
				if (clients == maxClients) {
					trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
					continue;
				}
			}

			if ( ui_joinGameType.integer != 0) 
			{
				game = BG_FindGametype(Info_ValueForKey(info, "gametype"));
				
				if (game != ui_joinGameType.integer - 1 ) 
				{
					trap_LAN_MarkServerVisible( ui_netSource.integer, i, qfalse);
					continue;
				}
			}
				
			if (ui_serverFilterType.integer > 0) {
				if (Q_stricmp(Info_ValueForKey(info, "game"), serverFilters[ui_serverFilterType.integer].basedir) != 0) {
					trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
					continue;
				}
			}
			// make sure we never add a favorite server twice
			if (ui_netSource.integer == AS_FAVORITES) {
				UI_RemoveServerFromDisplayList(i);
			}
			// insert the server into the list
			UI_BinaryServerInsertion(i);
			// done with this server
			if (ping > 0) {
				trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
				numinvisible++;
			}
		}
	}

	uiInfo.serverStatus.refreshtime = uiInfo.uiDC.realTime;

	// if there were no servers visible for ping updates
	if (!visible) {
//		UI_StopServerRefresh();
//		uiInfo.serverStatus.nextDisplayRefresh = 0;
	}
}

typedef struct
{
	char	*name;
	char	*altName;

} serverStatusCvar_t;

serverStatusCvar_t serverStatusCvars[] = 
{
	{"sv_hostname",		"Name"		},
	{"Address",			""			},
	{"gamename",		"Game name"	},
	{"g_gametype",		"Game type"	},
	{"mapname",			"Map"		},
	{"version",			""			},
	{"protocol",		""			},
	{"timelimit",		""			},
	{"scorelimit",		""			},
	{NULL, NULL}
};

/*
==================
UI_SortServerStatusInfo
==================
*/
static void UI_SortServerStatusInfo( serverStatusInfo_t *info ) 
{
	int i, j, index;
	char *tmp1, *tmp2;

	index = 0;
	for (i = 0; serverStatusCvars[i].name; i++) 
	{
		for (j = 0; j < info->numLines; j++) 
		{
			if ( !info->lines[j][1] || info->lines[j][1][0] ) 
			{
				continue;
			}

			if ( !Q_stricmp(serverStatusCvars[i].name, info->lines[j][0]) ) 
			{
				// swap lines
				tmp1 = info->lines[index][0];
				tmp2 = info->lines[index][3];
				info->lines[index][0] = info->lines[j][0];
				info->lines[index][3] = info->lines[j][3];
				info->lines[j][0] = tmp1;
				info->lines[j][3] = tmp2;
				
				//
				if ( strlen(serverStatusCvars[i].altName) ) 
				{
					info->lines[index][0] = serverStatusCvars[i].altName;
				}
				index++;
			}
		}
	}
}

/*
==================
UI_GetServerStatusInfo
==================
*/
static int UI_GetServerStatusInfo( const char *serverAddress, serverStatusInfo_t *info ) {
	char *p, *score, *ping, *name;
	int i, len;

	if (!info) {
		trap_LAN_ServerStatus( serverAddress, NULL, 0);
		return qfalse;
	}
	memset(info, 0, sizeof(*info));
	if ( trap_LAN_ServerStatus( serverAddress, info->text, sizeof(info->text)) ) {
		Q_strncpyz(info->address, serverAddress, sizeof(info->address));
		p = info->text;
		info->numLines = 0;
		info->lines[info->numLines][0] = "Address";
		info->lines[info->numLines][1] = "";
		info->lines[info->numLines][2] = "";
		info->lines[info->numLines][3] = info->address;
		info->numLines++;
		// get the cvars
		while (p && *p) {
			p = strchr(p, '\\');
			if (!p) break;
			*p++ = '\0';
			if (*p == '\\')
				break;
			info->lines[info->numLines][0] = p;
			info->lines[info->numLines][1] = "";
			info->lines[info->numLines][2] = "";
			p = strchr(p, '\\');
			if (!p) break;
			*p++ = '\0';
			info->lines[info->numLines][3] = p;

			info->numLines++;
			if (info->numLines >= MAX_SERVERSTATUS_LINES)
				break;
		}
		// get the player list
		if (info->numLines < MAX_SERVERSTATUS_LINES-3) {
			// empty line
			info->lines[info->numLines][0] = "";
			info->lines[info->numLines][1] = "";
			info->lines[info->numLines][2] = "";
			info->lines[info->numLines][3] = "";
			info->numLines++;
			// header
			info->lines[info->numLines][0] = "num";
			info->lines[info->numLines][1] = "score";
			info->lines[info->numLines][2] = "ping";
			info->lines[info->numLines][3] = "name";
			info->numLines++;
			// parse players
			i = 0;
			len = 0;
			while (p && *p) {
				if (*p == '\\')
					*p++ = '\0';
				if (!p)
					break;
				score = p;
				p = strchr(p, ' ');
				if (!p)
					break;
				*p++ = '\0';
				ping = p;
				p = strchr(p, ' ');
				if (!p)
					break;
				*p++ = '\0';
				name = p;
				Com_sprintf(&info->pings[len], sizeof(info->pings)-len, "%d", i);
				info->lines[info->numLines][0] = &info->pings[len];
				len += strlen(&info->pings[len]) + 1;
				info->lines[info->numLines][1] = score;
				info->lines[info->numLines][2] = ping;
				info->lines[info->numLines][3] = name;
				info->numLines++;
				if (info->numLines >= MAX_SERVERSTATUS_LINES)
					break;
				p = strchr(p, '\\');
				if (!p)
					break;
				*p++ = '\0';
				//
				i++;
			}
		}
		UI_SortServerStatusInfo( info );
		return qtrue;
	}
	return qfalse;
}

/*
==================
stristr
==================
*/
static char *stristr(char *str, char *charset) {
	int i;

	while(*str) {
		for (i = 0; charset[i] && str[i]; i++) {
			if (toupper(charset[i]) != toupper(str[i])) break;
		}
		if (!charset[i]) return str;
		str++;
	}
	return NULL;
}

/*
==================
UI_BuildFindPlayerList
==================
*/
static void UI_BuildFindPlayerList(qboolean force) {
	static int numFound, numTimeOuts;
	int i, j, resend;
	serverStatusInfo_t info;
	char name[MAX_NAME_LENGTH+2];
	char infoString[MAX_STRING_CHARS];

	if (!force) {
		if (!uiInfo.nextFindPlayerRefresh || uiInfo.nextFindPlayerRefresh > uiInfo.uiDC.realTime) {
			return;
		}
	}
	else {
		memset(&uiInfo.pendingServerStatus, 0, sizeof(uiInfo.pendingServerStatus));
		uiInfo.numFoundPlayerServers = 0;
		uiInfo.currentFoundPlayerServer = 0;
		trap_Cvar_VariableStringBuffer( "ui_findPlayer", uiInfo.findPlayerName, sizeof(uiInfo.findPlayerName));
		Q_CleanStr(uiInfo.findPlayerName);
		// should have a string of some length
		if (!strlen(uiInfo.findPlayerName)) {
			uiInfo.nextFindPlayerRefresh = 0;
			return;
		}
		// set resend time
		resend = ui_serverStatusTimeOut.integer / 2 - 10;
		if (resend < 50) {
			resend = 50;
		}
		trap_Cvar_Set("cl_serverStatusResendTime", va("%d", resend));
		// reset all server status requests
		trap_LAN_ServerStatus( NULL, NULL, 0);
		//
		uiInfo.numFoundPlayerServers = 1;
		Com_sprintf(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers-1],
						sizeof(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers-1]),
							"searching %d...", uiInfo.pendingServerStatus.num);
		numFound = 0;
		numTimeOuts++;
	}
	for (i = 0; i < MAX_SERVERSTATUSREQUESTS; i++) {
		// if this pending server is valid
		if (uiInfo.pendingServerStatus.server[i].valid) {
			// try to get the server status for this server
			if (UI_GetServerStatusInfo( uiInfo.pendingServerStatus.server[i].adrstr, &info ) ) {
				//
				numFound++;
				// parse through the server status lines
				for (j = 0; j < info.numLines; j++) {
					// should have ping info
					if ( !info.lines[j][2] || !info.lines[j][2][0] ) {
						continue;
					}
					// clean string first
					Q_strncpyz(name, info.lines[j][3], sizeof(name));
					Q_CleanStr(name);
					// if the player name is a substring
					if (stristr(name, uiInfo.findPlayerName)) {
						// add to found server list if we have space (always leave space for a line with the number found)
						if (uiInfo.numFoundPlayerServers < MAX_FOUNDPLAYER_SERVERS-1) {
							//
							Q_strncpyz(uiInfo.foundPlayerServerAddresses[uiInfo.numFoundPlayerServers-1],
										uiInfo.pendingServerStatus.server[i].adrstr,
											sizeof(uiInfo.foundPlayerServerAddresses[0]));
							Q_strncpyz(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers-1],
										uiInfo.pendingServerStatus.server[i].name,
											sizeof(uiInfo.foundPlayerServerNames[0]));
							uiInfo.numFoundPlayerServers++;
						}
						else {
							// can't add any more so we're done
							uiInfo.pendingServerStatus.num = uiInfo.serverStatus.numDisplayServers;
						}
					}
				}
				Com_sprintf(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers-1],
								sizeof(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers-1]),
									"searching %d/%d...", uiInfo.pendingServerStatus.num, numFound);
				// retrieved the server status so reuse this spot
				uiInfo.pendingServerStatus.server[i].valid = qfalse;
			}
		}
		// if empty pending slot or timed out
		if (!uiInfo.pendingServerStatus.server[i].valid ||
			uiInfo.pendingServerStatus.server[i].startTime < uiInfo.uiDC.realTime - ui_serverStatusTimeOut.integer) {
			if (uiInfo.pendingServerStatus.server[i].valid) {
				numTimeOuts++;
			}
			// reset server status request for this address
			UI_GetServerStatusInfo( uiInfo.pendingServerStatus.server[i].adrstr, NULL );
			// reuse pending slot
			uiInfo.pendingServerStatus.server[i].valid = qfalse;
			// if we didn't try to get the status of all servers in the main browser yet
			if (uiInfo.pendingServerStatus.num < uiInfo.serverStatus.numDisplayServers) {
				uiInfo.pendingServerStatus.server[i].startTime = uiInfo.uiDC.realTime;
				trap_LAN_GetServerAddressString(ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.pendingServerStatus.num],
							uiInfo.pendingServerStatus.server[i].adrstr, sizeof(uiInfo.pendingServerStatus.server[i].adrstr));
				trap_LAN_GetServerInfo(ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.pendingServerStatus.num], infoString, sizeof(infoString));
				Q_strncpyz(uiInfo.pendingServerStatus.server[i].name, Info_ValueForKey(infoString, "hostname"), sizeof(uiInfo.pendingServerStatus.server[0].name));
				uiInfo.pendingServerStatus.server[i].valid = qtrue;
				uiInfo.pendingServerStatus.num++;
				Com_sprintf(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers-1],
								sizeof(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers-1]),
									"searching %d/%d...", uiInfo.pendingServerStatus.num, numFound);
			}
		}
	}
	for (i = 0; i < MAX_SERVERSTATUSREQUESTS; i++) {
		if (uiInfo.pendingServerStatus.server[i].valid) {
			break;
		}
	}
	// if still trying to retrieve server status info
	if (i < MAX_SERVERSTATUSREQUESTS) {
		uiInfo.nextFindPlayerRefresh = uiInfo.uiDC.realTime + 25;
	}
	else {
		// add a line that shows the number of servers found
		if (!uiInfo.numFoundPlayerServers) {
			Com_sprintf(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers-1], sizeof(uiInfo.foundPlayerServerAddresses[0]), "no servers found");
		}
		else {
			Com_sprintf(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers-1], sizeof(uiInfo.foundPlayerServerAddresses[0]),
						"%d server%s found for player", uiInfo.numFoundPlayerServers-1,
						uiInfo.numFoundPlayerServers == 2 ? "":"s");
		}
		uiInfo.nextFindPlayerRefresh = 0;
		// show the server status info for the selected server
		UI_FeederSelection(FEEDER_FINDPLAYER, uiInfo.currentFoundPlayerServer);
	}
}

/*
==================
UI_BuildServerStatus
==================
*/
static void UI_BuildServerStatus(qboolean force) {

	if (uiInfo.nextFindPlayerRefresh) {
		return;
	}
	if (!force) {
		if (!uiInfo.nextServerStatusRefresh || uiInfo.nextServerStatusRefresh > uiInfo.uiDC.realTime) {
			return;
		}
	}
	else {
		Menu_SetFeederSelection(NULL, FEEDER_SERVERSTATUS, 0, NULL);
		uiInfo.serverStatusInfo.numLines = 0;
		// reset all server status requests
		trap_LAN_ServerStatus( NULL, NULL, 0);
	}
	if (uiInfo.serverStatus.currentServer < 0 || uiInfo.serverStatus.currentServer > uiInfo.serverStatus.numDisplayServers || uiInfo.serverStatus.numDisplayServers == 0) {
		return;
	}
	if (UI_GetServerStatusInfo( uiInfo.serverStatusAddress, &uiInfo.serverStatusInfo ) ) {
		uiInfo.nextServerStatusRefresh = 0;
		UI_GetServerStatusInfo( uiInfo.serverStatusAddress, NULL );
	}
	else {
		uiInfo.nextServerStatusRefresh = uiInfo.uiDC.realTime + 500;
	}
}

/*
==================
UI_FeederCount
==================
*/
static int UI_FeederCount(float feederID) 
{
	switch ( (int)feederID )
	{
		case FEEDER_IDENTITIES:
			return bg_identityCount;

		case FEEDER_TEAMIDENTITIES:
		{
			int		count;
			team_t	team;

			team = ui_info_team.integer;
			if ( team != TEAM_RED && team != TEAM_BLUE )
			{	
				return 0;
			}

			for ( count = 0; count < MAX_TEAMIDENTITIES; count ++ )
			{
				if ( !uiInfo.identityTeams[team][count] )
					break;
			}

			return count;
		}

		case FEEDER_CINEMATICS:
			return uiInfo.movieCount;

		case FEEDER_MAPS:
		case FEEDER_ALLMAPS:
			return UI_MapCountByGameType ();

		case FEEDER_VOTEMAPS:
			return UI_MapCountForVote ( );

		case FEEDER_OUTFITTING_TEMPLATES:
			return bg_outfittingCount;

		case FEEDER_SERVERS:
			return uiInfo.serverStatus.numDisplayServers;

		case FEEDER_SERVERSTATUS:
			return uiInfo.serverStatusInfo.numLines;

		case FEEDER_FINDPLAYER:
			return uiInfo.numFoundPlayerServers;

		case FEEDER_NEWVERSION_WEBSITES:
			return trap_Version_GetNumSites();

		case FEEDER_PLAYER_LIST:
			if (uiInfo.uiDC.realTime > uiInfo.playerRefresh) 
			{
				uiInfo.playerRefresh = uiInfo.uiDC.realTime + 3000;
				UI_BuildPlayerList();
			}
			return uiInfo.playerCount;

		case FEEDER_MODS:
			return uiInfo.modCount;

		case FEEDER_DEMOS:
			return uiInfo.demoCount;
	}
	
	return 0;
}

static const char *UI_SelectedMap(int index, int *actual) 
{
	int i, c;
	c = 0;
	*actual = 0;

	for (i = 0; i < uiInfo.mapCount; i++) 
	{
		if (uiInfo.mapList[i].active) 
		{
			if (c == index) 
			{
				*actual = i;
				return uiInfo.mapList[i].mapName;
			} 
			else 
			{
				c++;
			}
		}
	}
	
	return "";
}

static int UI_GetIndexFromSelection ( int actual ) 
{
	int i;
	int	c;

	for ( c = 0, i = 0; i < uiInfo.mapCount; i++) 
	{
		if (uiInfo.mapList[i].active) 
		{
			if (i == actual) 
			{
				return c;
			}
			
			c++;
		}
	}

	return 0;
}

static void UI_UpdatePendingPings() { 
	trap_LAN_ResetPings(ui_netSource.integer);
	uiInfo.serverStatus.refreshActive = qtrue;
	uiInfo.serverStatus.refreshtime = uiInfo.uiDC.realTime + 1000;

}


static const char *UI_FeederItemText(float feederID, int index, int column, qhandle_t *handle) 
{
	static char info[MAX_STRING_CHARS];
	static char info2[MAX_STRING_CHARS];
	static char hostname[MAX_STRING_CHARS];
	static char needpass[2];
	static char clientBuff[32];
	static int	lastColumn = -1;
	static int	lastTime = 0;

	*handle = -1;

	switch ( (int)feederID )
	{
		case FEEDER_IDENTITIES:
			if (index >= 0 && index < bg_identityCount) 
			{
				return bg_identities[index].mName;
			}
			break;

		case FEEDER_TEAMIDENTITIES:
			if ( index >= 0 && index < MAX_TEAMIDENTITIES )
			{
				return uiInfo.identityTeams[ui_info_team.integer][index]->mName;
			}
			break;

		case FEEDER_MAPS:
		case FEEDER_ALLMAPS:
		case FEEDER_VOTEMAPS:
		{
			int actual;
			return UI_SelectedMap(index, &actual);
		}

		case FEEDER_OUTFITTING_TEMPLATES:
			return bg_outfittings[index].name;

		case FEEDER_NEWVERSION_TEXT:
			trap_Version_GetDescription(info2, sizeof(info2));
			return info2;

		case FEEDER_NEWVERSION_WEBSITES:
			trap_Version_GetSite(index, info, sizeof(info));
			return info;

		case FEEDER_SERVERS:
			if (index >= 0 && index < uiInfo.serverStatus.numDisplayServers) 
			{
				int ping;
				int game;
				
				if (lastColumn != column || lastTime > uiInfo.uiDC.realTime + 5000) 
				{
					trap_LAN_GetServerInfo(ui_netSource.integer, uiInfo.serverStatus.displayServers[index], info, MAX_STRING_CHARS);
					lastColumn = column;
					lastTime = uiInfo.uiDC.realTime;
				}
				
				ping = atoi(Info_ValueForKey(info, "ping"));
				if (ping == -1) 
				{
					// if we ever see a ping that is out of date, do a server refresh
					// UI_UpdatePendingPings();
				}
			
				switch (column) 
				{
					case SORT_NEEDPASS:
						needpass[0] = atoi(Info_ValueForKey(info, "needpass"))?'*':' ';
						needpass[1] = 0;
						return needpass;

					case SORT_HOST : 
						if (ping <= 0) 
						{
							return Info_ValueForKey(info, "addr");
						} 
						else 
						{
							if ( ui_netSource.integer == AS_LOCAL ) 
							{
								Com_sprintf( hostname, sizeof(hostname), "[%s] %s",
											 netnames[atoi(Info_ValueForKey(info, "nettype"))],
											 Info_ValueForKey(info, "hostname") );
								return hostname;
							}
							else
							{
								// anonymous server
								if (atoi(Info_ValueForKey(info, "sv_allowAnonymous")) != 0)
								{				
									Com_sprintf( hostname, sizeof(hostname), "(A) %s",
												 Info_ValueForKey(info, "hostname"));
								} 
								else 
								{
									Com_sprintf( hostname, sizeof(hostname), "%s",
												 Info_ValueForKey(info, "hostname"));
								}
							}
						
							return hostname;
						}

						break;

					case SORT_MAP : 
						return Info_ValueForKey(info, "mapname");

					case SORT_CLIENTS : 
						Com_sprintf( clientBuff, sizeof(clientBuff), "%s (%s)", Info_ValueForKey(info, "clients"), Info_ValueForKey(info, "sv_maxclients"));
						return clientBuff;
				
					case SORT_GAME :
						game = BG_FindGametype ( Info_ValueForKey(info, "gametype") );
						if (ping <= 0)
						{
							return "Inactive";	
						}
						else if (game < 0) 
						{
							return "Unknown";	
						} 

						return bg_gametypeData[game].name;

					case SORT_PING : 
						if (ping <= 0) 
						{
							return "...";
						} 

						return Info_ValueForKey(info, "ping");
				}
			}

			break;

		case FEEDER_SERVERSTATUS:

			if ( index >= 0 && index < uiInfo.serverStatusInfo.numLines ) 
			{
				if ( column >= 0 && column < 4 ) 
				{
					return uiInfo.serverStatusInfo.lines[index][column];
				}
			}
			break;

		case FEEDER_FINDPLAYER:
			if ( index >= 0 && index < uiInfo.numFoundPlayerServers ) 
			{
				return uiInfo.foundPlayerServerNames[index];
			}
			break;

		case FEEDER_PLAYER_LIST:
			if (index >= 0 && index < uiInfo.playerCount) 
			{
				return uiInfo.playerNames[index] + 2;
			}
			break;
	
		case FEEDER_MODS:
			if (index >= 0 && index < uiInfo.modCount) 
			{
				if (uiInfo.modList[index].modDescr && *uiInfo.modList[index].modDescr) 
				{
					return uiInfo.modList[index].modDescr;
				} 
				else 
				{
					return uiInfo.modList[index].modName;
				}
			}
			break;

		case FEEDER_CINEMATICS:
			if (index >= 0 && index < uiInfo.movieCount) 
			{
				return uiInfo.movieList[index];
			}
			break;
	
		case FEEDER_DEMOS:
			if (index >= 0 && index < uiInfo.demoCount) 
			{
				return uiInfo.demoList[index];
			}
			break;
	}
	
	return "";
}

static qhandle_t UI_FeederItemImage ( float feederID, int index ) 
{
	switch ( (int)feederID )
	{
		case FEEDER_IDENTITIES:
			if (index >= 0 && index < bg_identityCount) 
			{
				return bg_identities[index].mIcon;
			}
			break;

		case FEEDER_TEAMIDENTITIES:
			if (index >= 0 && index < MAX_TEAMIDENTITIES) 
			{
				return uiInfo.identityTeams[ui_info_team.integer][index]->mIcon;
			}
			break;

		case FEEDER_ALLMAPS:
		case FEEDER_MAPS:
		case FEEDER_VOTEMAPS:
		{
			int actual;
			
			UI_SelectedMap(index, &actual);
			index = actual;
			
			if (index >= 0 && index < uiInfo.mapCount) 
			{
				if (uiInfo.mapList[index].levelShot == -1) 
				{
					uiInfo.mapList[index].levelShot = trap_R_RegisterShaderNoMip(uiInfo.mapList[index].imageName);
				}
				return uiInfo.mapList[index].levelShot;
			}

			break;
		}
	}
  
	return 0;
}

static void UI_FeederSelection(float feederID, int index) 
{
	static char info[MAX_STRING_CHARS];
	
	switch ( (int)feederID )
	{
		case FEEDER_OUTFITTING_TEMPLATES:
			uiInfo.outfittingItemGroup = index;
			break;
	
		case FEEDER_IDENTITIES:
			if (index >= 0 && index < bg_identityCount) 
			{
				trap_Cvar_Set( "identity", bg_identities[index].mName);
				updateModel = qtrue;
			}
			break;

		case FEEDER_TEAMIDENTITIES:
			if (index >= 0 && index < MAX_TEAMIDENTITIES) 
			{
				trap_Cvar_Set( "team_identity", uiInfo.identityTeams[ui_info_team.integer][index]->mName );
				updateModel = qtrue;
			}
			break;

		case FEEDER_MAPS:
		case FEEDER_ALLMAPS:
		case FEEDER_VOTEMAPS:
		{
			int actual, map;
			map = (feederID == FEEDER_ALLMAPS) ? ui_currentNetMap.integer : ui_currentMap.integer;
			if (uiInfo.mapList[map].cinematic >= 0) 
			{
				trap_CIN_StopCinematic(uiInfo.mapList[map].cinematic);
				uiInfo.mapList[map].cinematic = -1;
			}
			
			if ( *UI_SelectedMap(index, &actual) )
			{
				trap_Cvar_Set("ui_mapIndex", va("%d", index));
				ui_mapIndex.integer = index;

				if (feederID == FEEDER_MAPS) 
				{
					ui_currentMap.integer = actual;
					trap_Cvar_Set("ui_currentMap", va("%d", actual));
	  				uiInfo.mapList[ui_currentMap.integer].cinematic = trap_CIN_PlayCinematic(va("%s.roq", uiInfo.mapList[ui_currentMap.integer].mapLoadName), 0, 0, 0, 0, (CIN_loop | CIN_silent) );
					trap_Cvar_Set("ui_opponentModel", uiInfo.mapList[ui_currentMap.integer].opponentName);
				} 
				else 
				{
					ui_currentNetMap.integer = actual;
					trap_Cvar_Set("ui_currentNetMap", va("%d", actual));
					trap_Cvar_Set("ui_currentNetMapName", uiInfo.mapList[actual].mapLoadName );
	  				uiInfo.mapList[ui_currentNetMap.integer].cinematic = trap_CIN_PlayCinematic(va("%s.roq", uiInfo.mapList[ui_currentNetMap.integer].mapLoadName), 0, 0, 0, 0, (CIN_loop | CIN_silent) );
				}
			}
			break;
		}

		case FEEDER_NEWVERSION_WEBSITES:
			uiInfo.versionIndex = index;
			break;

		case FEEDER_SERVERS:
		{
			const char *mapName = NULL;
			uiInfo.serverStatus.currentServer = index;
			trap_LAN_GetServerInfo(ui_netSource.integer, uiInfo.serverStatus.displayServers[index], info, MAX_STRING_CHARS);
			uiInfo.serverStatus.currentServerPreview = trap_R_RegisterShaderNoMip(va("gfx/menus/levelshots/%s", Info_ValueForKey(info, "mapname")));
			if (uiInfo.serverStatus.currentServerCinematic >= 0) 
			{
				trap_CIN_StopCinematic( uiInfo.serverStatus.currentServerCinematic);
				uiInfo.serverStatus.currentServerCinematic = -1;
			}
			mapName = Info_ValueForKey(info, "mapname");
			if (mapName && *mapName) 
			{
				uiInfo.serverStatus.currentServerCinematic = trap_CIN_PlayCinematic(va("%s.roq", mapName), 0, 0, 0, 0, (CIN_loop | CIN_silent) );
			}
			break;
		}

		case FEEDER_FINDPLAYER:
			uiInfo.currentFoundPlayerServer = index;

			if ( index < uiInfo.numFoundPlayerServers-1) 
			{
				// build a new server status for this server
				Q_strncpyz(uiInfo.serverStatusAddress, uiInfo.foundPlayerServerAddresses[uiInfo.currentFoundPlayerServer], sizeof(uiInfo.serverStatusAddress));
				Menu_SetFeederSelection(NULL, FEEDER_SERVERSTATUS, 0, NULL);
				UI_BuildServerStatus(qtrue);
			}
			break;

		case FEEDER_PLAYER_LIST:
			uiInfo.playerIndex = index;
			break;

		case FEEDER_MODS:
			uiInfo.modIndex = index;
			break;

		case FEEDER_CINEMATICS:
			uiInfo.movieIndex = index;
			if (uiInfo.previewMovie >= 0) 
			{
				trap_CIN_StopCinematic(uiInfo.previewMovie);
			}
			uiInfo.previewMovie = -1;
			break;

		case FEEDER_DEMOS:
			uiInfo.demoIndex = index;
			break;
	}
}

static qboolean Alias_Parse(const char **p) {
  char *token;

  token = COM_ParseExt(p, qtrue);

  if (token[0] != '{') {
    return qfalse;
  }

  while ( 1 ) {
    token = COM_ParseExt(p, qtrue);

    if (Q_stricmp(token, "}") == 0) {
      return qtrue;
    }

    if ( !token || token[0] == 0 ) {
      return qfalse;
    }

    if (token[0] == '{') {
      // three tokens per line, character name, bot alias, and preferred action a - all purpose, d - defense, o - offense
      if (!String_Parse(p, &uiInfo.aliasList[uiInfo.aliasCount].name) || !String_Parse(p, &uiInfo.aliasList[uiInfo.aliasCount].ai) || !String_Parse(p, &uiInfo.aliasList[uiInfo.aliasCount].action)) {
        return qfalse;
      }
    
      Com_Printf("Loaded character alias %s using character ai %s.\n", uiInfo.aliasList[uiInfo.aliasCount].name, uiInfo.aliasList[uiInfo.aliasCount].ai);
      if (uiInfo.aliasCount < MAX_ALIASES) {
        uiInfo.aliasCount++;
      } else {
        Com_Printf("Too many aliases, last alias replaced!\n");
      }
     
      token = COM_ParseExt(p, qtrue);
      if (token[0] != '}') {
        return qfalse;
      }
    }
  }

  return qfalse;
}

static void UI_Pause(qboolean b) 
{
	if (b) 
	{
		// pause the game and set the ui keycatcher
		trap_Cvar_Set( "cl_paused", "1" );
		trap_Key_SetCatcher( KEYCATCH_UI );
	} 
	else 
	{
		// unpause the game and clear the ui keycatcher
		trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_UI );
		trap_Key_ClearStates();
		trap_Cvar_Set( "cl_paused", "0" );
	}
}

static int UI_PlayCinematic(const char *name, float x, float y, float w, float h) 
{
	return trap_CIN_PlayCinematic(name, x, y, w, h, (CIN_loop | CIN_silent));
}

static void UI_StopCinematic(int handle) {
	if (handle >= 0) {
	  trap_CIN_StopCinematic(handle);
	} else {
		handle = abs(handle);
		if (handle == UI_MAPCINEMATIC) {
			if (uiInfo.mapList[ui_currentMap.integer].cinematic >= 0) {
			  trap_CIN_StopCinematic(uiInfo.mapList[ui_currentMap.integer].cinematic);
			  uiInfo.mapList[ui_currentMap.integer].cinematic = -1;
			}
		} else if (handle == UI_NETMAPCINEMATIC) {
			if (uiInfo.serverStatus.currentServerCinematic >= 0) {
			  trap_CIN_StopCinematic(uiInfo.serverStatus.currentServerCinematic);
				uiInfo.serverStatus.currentServerCinematic = -1;
			}
		} 
	}
}

static void UI_DrawCinematic(int handle, float x, float y, float w, float h) {
	trap_CIN_SetExtents(handle, x, y, w, h);
  trap_CIN_DrawCinematic(handle);
}

static void UI_RunCinematicFrame(int handle) {
  trap_CIN_RunCinematic(handle);
}

/*
=================
UI_Init
=================
*/
void _UI_Init( qboolean inGameLoad ) 
{
	int			start;
	int			i;
	char		identity[256];

	UI_RegisterCvars();

	// cache redundant calulations
	trap_GetGlconfig( &uiInfo.uiDC.glconfig );

	// for 640x480 virtualized screen
	uiInfo.uiDC.yscale = uiInfo.uiDC.glconfig.vidHeight * (1.0/480.0);
	uiInfo.uiDC.xscale = uiInfo.uiDC.glconfig.vidWidth * (1.0/640.0);
	if ( uiInfo.uiDC.glconfig.vidWidth * 480 > uiInfo.uiDC.glconfig.vidHeight * 640 ) 
	{
		// wide screen
		uiInfo.uiDC.bias = 0.5 * ( uiInfo.uiDC.glconfig.vidWidth - ( uiInfo.uiDC.glconfig.vidHeight * (640.0/480.0) ) );
	}
	else 
	{
		// no wide screen
		uiInfo.uiDC.bias = 0;
	}

	uiInfo.uiDC.registerShaderNoMip = &trap_R_RegisterShaderNoMip;
	uiInfo.uiDC.setColor = &UI_SetColor;
	uiInfo.uiDC.drawHandlePic = &UI_DrawHandlePic;
	uiInfo.uiDC.drawStretchPic = &trap_R_DrawStretchPic;
	uiInfo.uiDC.drawText = &UI_DrawText;
	uiInfo.uiDC.drawTextWithCursor = &UI_DrawTextWithCursor;
	uiInfo.uiDC.getTextWidth = &trap_R_GetTextWidth;
	uiInfo.uiDC.getTextHeight = &trap_R_GetTextHeight;
	uiInfo.uiDC.registerModel = &trap_R_RegisterModel;
	uiInfo.uiDC.modelBounds = &trap_R_ModelBounds;
	uiInfo.uiDC.fillRect = &UI_FillRect;
	uiInfo.uiDC.drawRect = &_UI_DrawRect;
	uiInfo.uiDC.drawSides = &_UI_DrawRectLeftRight;
	uiInfo.uiDC.drawTopBottom = &_UI_DrawRectTopBottom;
	uiInfo.uiDC.clearScene = &trap_R_ClearScene;
	uiInfo.uiDC.addRefEntityToScene = &trap_R_AddRefEntityToScene;
	uiInfo.uiDC.renderScene = &trap_R_RenderScene;
	uiInfo.uiDC.registerFont = &trap_R_RegisterFont;
	uiInfo.uiDC.ownerDrawItem = &UI_OwnerDraw;
	uiInfo.uiDC.getValue = &UI_GetValue;
	uiInfo.uiDC.ownerDrawVisible = &UI_OwnerDrawVisible;
	uiInfo.uiDC.ownerDrawDisabled = &UI_OwnerDrawDisabled;
	uiInfo.uiDC.runScript = &UI_RunMenuScript;
	uiInfo.uiDC.getTeamColor = &UI_GetTeamColor;
	uiInfo.uiDC.setCVar = trap_Cvar_Set;
	uiInfo.uiDC.getCVarString = trap_Cvar_VariableStringBuffer;
	uiInfo.uiDC.getCVarValue = trap_Cvar_VariableValue;
	uiInfo.uiDC.setOverstrikeMode = &trap_Key_SetOverstrikeMode;
	uiInfo.uiDC.getOverstrikeMode = &trap_Key_GetOverstrikeMode;
	uiInfo.uiDC.startLocalSound = &trap_S_StartLocalSound;
	uiInfo.uiDC.ownerDrawHandleKey = &UI_OwnerDrawHandleKey;
	uiInfo.uiDC.feederCount = &UI_FeederCount;
	uiInfo.uiDC.feederItemImage = &UI_FeederItemImage;
	uiInfo.uiDC.feederItemText = &UI_FeederItemText;
	uiInfo.uiDC.feederSelection = &UI_FeederSelection;
	uiInfo.uiDC.setBinding = &trap_Key_SetBinding;
	uiInfo.uiDC.getBindingBuf = &trap_Key_GetBindingBuf;
	uiInfo.uiDC.keynumToStringBuf = &trap_Key_KeynumToStringBuf;
	uiInfo.uiDC.executeText = &trap_Cmd_ExecuteText;
	uiInfo.uiDC.Error = &Com_Error; 
	uiInfo.uiDC.Print = &Com_Printf; 
	uiInfo.uiDC.Pause = &UI_Pause;
	uiInfo.uiDC.ownerDrawWidth = &UI_OwnerDrawWidth;
	uiInfo.uiDC.registerSound = &trap_S_RegisterSound;
	uiInfo.uiDC.startBackgroundTrack = &trap_S_StartBackgroundTrack;
	uiInfo.uiDC.stopBackgroundTrack = &trap_S_StopBackgroundTrack;
	uiInfo.uiDC.playCinematic = &UI_PlayCinematic;
	uiInfo.uiDC.stopCinematic = &UI_StopCinematic;
	uiInfo.uiDC.drawCinematic = &UI_DrawCinematic;
	uiInfo.uiDC.runCinematicFrame = &UI_RunCinematicFrame;

	Init_Display(&uiInfo.uiDC);

	String_Init();
  
	uiInfo.uiDC.whiteShader = trap_R_RegisterShaderNoMip( "white" );

	AssetCache();

	start = trap_Milliseconds();

	uiInfo.aliasCount = 0;

	// Read in the gametype list for the various user interface screens
	BG_BuildGametypeList ( );
	BG_ParseNPCFiles ( );

	// Load all the menus
	if ( inGameLoad )
	{
		uiInfo.menusFile = "ui/ingame.txt";
	}
	else
	{
		uiInfo.menusFile = "ui/menus.txt";
	}

	UI_LoadMenus( uiInfo.menusFile, qtrue);
	
	Menus_CloseAll();

	trap_LAN_LoadCachedServers();

	UI_LoadIdentityIcons ( );

	// Initialize the team cvars
	trap_Cvar_Set ( "ui_info_redteam", "" );
	trap_Cvar_Set ( "ui_info_blueteam", "" );
	trap_Cvar_Set ( "ui_info_gametype", "0" );
	trap_Cvar_Set ( "ui_info_team", "0" );

#ifdef _SOF2_BOTS
	UI_LoadBots();
#endif

	// sets defaults for ui temp cvars
	uiInfo.currentCrosshair = (int)trap_Cvar_VariableValue("cg_drawCrosshair");
	trap_Cvar_Set("ui_mousePitch", (trap_Cvar_VariableValue("m_pitch") >= 0) ? "0" : "1");

	uiInfo.serverStatus.currentServerCinematic = -1;
	uiInfo.previewMovie = -1;

	if (trap_Cvar_VariableValue("ui_sof2FirstRun") == 0) 
	{
		trap_Cvar_Set("s_volume", "0.8");
		trap_Cvar_Set("s_musicvolume", "0.5");
		trap_Cvar_Set("s_attenuate", "0.00045" );
		trap_Cvar_Set("ui_sof2FirstRun", "1");
	}

	trap_Cvar_Register(NULL, "debug_protocol", "", 0, 0.0, 0.0 );

	trap_Cvar_Set("ui_actualNetGameType", va("%d", ui_netGameType.integer));
	trap_Cvar_Set("ui_gtRespawnType", va("%d", bg_gametypeData[ui_netGameType.integer].respawnType));
	trap_Cvar_Set("ui_gtPickupsDisabled", va("%d", bg_gametypeData[ui_netGameType.integer].pickupsDisabled));

	trap_Cvar_VariableStringBuffer("identity", identity, sizeof(identity));
	for(i=0;i<bg_identityCount;i++)
	{
		if (Q_stricmp(bg_identities[i].mName, identity) == 0)
		{
			Menu_SetFeederSelection(NULL, FEEDER_IDENTITIES, i, "player_menu");
			break;
		}
	}
}

/*
=================
UI_KeyEvent
=================
*/
qboolean _UI_KeyEvent( int key, qboolean down ) 
{
	menuDef_t *menu;

	if (Menu_Count() <= 0) 
	{
		return qfalse;
	}

    menu = Menu_GetFocused();
	
	if (menu) 
	{
		if (key == K_ESCAPE && down && !Menus_AnyFullScreenVisible()) 
		{
			Menus_CloseAll();
		} 
		else 
		{
			// Passthrough has no mouse input
			if ( (key < '0' || key > '9') && (trap_Key_GetCatcher ( ) & KEYCATCH_NUMBERSONLY) )
			{
				return qfalse;
			}

			return Menu_HandleKey(menu, key, down );
		}
	} 
	else 
	{
		int catcher = trap_Key_GetCatcher ( );

		if ( catcher & KEYCATCH_NUMBERSONLY )
		{
			trap_Key_SetCatcher( trap_Key_GetCatcher() & ~(KEYCATCH_UI|KEYCATCH_NUMBERSONLY) );
		}
		else
		{
			trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_UI );
			trap_Key_ClearStates();
			trap_Cvar_Set( "cl_paused", "0" );
		}
	}

	return qfalse;
}

/*
=================
UI_MouseEvent
=================
*/
qboolean _UI_MouseEvent( int dx, int dy )
{
	// update mouse screen position
	uiInfo.uiDC.cursorx += dx;
	if (uiInfo.uiDC.cursorx < 0)
		uiInfo.uiDC.cursorx = 0;
	else if (uiInfo.uiDC.cursorx > SCREEN_WIDTH)
		uiInfo.uiDC.cursorx = SCREEN_WIDTH;

	uiInfo.uiDC.cursory += dy;
	if (uiInfo.uiDC.cursory < 0)
		uiInfo.uiDC.cursory = 0;
	else if (uiInfo.uiDC.cursory > SCREEN_HEIGHT)
		uiInfo.uiDC.cursory = SCREEN_HEIGHT;

	// If the cursor has moved then reset the tooltip location and time.
	if ( uiInfo.uiDC.tooltipx != uiInfo.uiDC.cursorx ||
	     uiInfo.uiDC.tooltipy != uiInfo.uiDC.cursory    )
	{
		uiInfo.uiDC.tooltipx = uiInfo.uiDC.cursorx;
		uiInfo.uiDC.tooltipy = uiInfo.uiDC.cursory;
		uiInfo.uiDC.tooltiptime = uiInfo.uiDC.realTime;
	}

	if (Menu_Count() > 0) 
	{
		//menuDef_t *menu = Menu_GetFocused();
		//Menu_HandleMouseMove(menu, uiInfo.uiDC.cursorx, uiInfo.uiDC.cursory);
		Display_MouseMove(NULL, uiInfo.uiDC.cursorx, uiInfo.uiDC.cursory);
	}

	return qtrue;
}

void UI_LoadNonIngame() 
{
	UI_LoadMenus("ui/menus.txt", qfalse);
	uiInfo.inGameLoad = qfalse;
}

void UI_SetActiveMenu( uiMenuCommand_t menu ) 
{
	char	buf[256];

	// this should be the ONLY way the menu system is brought up
	// enusure minumum menu data is cached
	if (Menu_Count() <= 0) 
	{	
		return;
	}
	
	switch ( menu ) 
	{
		case UIMENU_NONE:
			trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_UI );
			trap_Key_ClearStates();
			trap_Cvar_Set( "cl_paused", "0" );
			Menus_CloseAll();
			return;

		case UIMENU_MAIN:
		{
			qboolean   active = qfalse;
			menuDef_t* menudef;
			itemDef_t* item;

			// Make sure the player setup alwasy shows non team game models
			trap_Cvar_Set ( "ui_info_teamgame", "0" );
	
			trap_Key_SetCatcher( KEYCATCH_UI );

			if (uiInfo.inGameLoad) 
			{
				UI_LoadNonIngame();
			}
			
			Menus_CloseAll();
			menudef = Menus_ActivateByName("main");

			// the ui_joinserver cvar tells us whether or not we should jump
			// straight to the join server menu
			if ( ui_joinserver.integer )
			{
				item = Menu_GetItemByName ( menudef, "play_button" );
			}
			else
			{
				item = NULL;
			}

			trap_Cvar_Set ( "ui_joinserver", "0" );

			if ( item )
			{
				Item_Action ( item );

				trap_Cvar_VariableStringBuffer("com_errorMessage", buf, sizeof(buf));
				if (strlen(buf)) 
				{
					Menus_ActivateByName("error_popmenu");
					active = qtrue;
				}
			}
			else
			{
				trap_Cvar_VariableStringBuffer("com_errorMessage", buf, sizeof(buf));
				if (strlen(buf)) 
				{
					Menus_ActivateByName("error_popmenu");
					active = qtrue;
				}
				
				if (!active && ui_invalidversion.integer) 
				{
					Menus_ActivateByName("patch_info");
					active = qtrue;
				}
				
				if ( !active && !ui_cdkeychecked.integer )
				{
					char key[20];

					trap_GetCDKey( key, sizeof(key) );
					if( !trap_VerifyCDKey( key ) ) 
					{
						Menus_ActivateByName("cdkey_popmenu");
						active = qtrue;
					}
				}
				
				if ( !active && (int)trap_Cvar_VariableValue ( "com_othertasks" ) )
				{
					trap_Cvar_Set("com_othertasks", "0");
					if ( !(int)trap_Cvar_VariableValue ( "com_ignoreothertasks" ) )
					{
						Menus_ActivateByName("backgroundtask_popmenu");
						active = qtrue;
					}
				}
			}

			return;
		}

		case UIMENU_VERSION:
			trap_Key_SetCatcher( KEYCATCH_UI );
			Menus_ActivateByName("patch_info");
			return;

		case UIMENU_BAD_CD_KEY:
			// no cd check in TA
			//trap_Key_SetCatcher( KEYCATCH_UI );
			//Menus_ActivateByName("badcd");
			//UI_ConfirmMenu( "Bad CD Key", NULL, NeedCDKeyAction );
			return;

		 case UIMENU_INGAME:
			trap_Cvar_Set( "cl_paused", "1" );
			trap_Key_SetCatcher( KEYCATCH_UI );
			UI_BuildPlayerList();
			Menus_CloseAll();
			Menus_ActivateByName("ingame");
			return;

		case UIMENU_OBJECTIVES:
			trap_Key_SetCatcher( KEYCATCH_UI );
			Menus_CloseAll();
			Menus_ActivateByName("ingame_objectives");				
			return;

		case UIMENU_RADIO:
			trap_Key_SetCatcher( KEYCATCH_UI|KEYCATCH_NUMBERSONLY );
			Menus_CloseAll();
			Menus_ActivateByName("ingame_radio");				
			return;

		case UIMENU_OUTFITTING:
//			trap_Cvar_Set( "cl_paused", "1" );
			trap_Key_SetCatcher( KEYCATCH_UI );
			Menus_CloseAll();
			Menus_ActivateByName("ingame_outfitting");
			return;

		case UIMENU_TEAM:
//			trap_Cvar_Set( "cl_paused", "1" );
			trap_Key_SetCatcher( KEYCATCH_UI );
			Menus_CloseAll();
			Menus_ActivateByName("ingame_team");
			return;
	}
}

static connstate_t	lastConnState;
static char			lastLoadingText[MAX_INFO_VALUE];

static void UI_ReadableSize ( char *buf, int bufsize, int value )
{
	if (value > 1024*1024*1024 ) { // gigs
		Com_sprintf( buf, bufsize, "%d", value / (1024*1024*1024) );
		Com_sprintf( buf+strlen(buf), bufsize-strlen(buf), ".%02d GB", 
			(value % (1024*1024*1024))*100 / (1024*1024*1024) );
	} else if (value > 1024*1024 ) { // megs
		Com_sprintf( buf, bufsize, "%d", value / (1024*1024) );
		Com_sprintf( buf+strlen(buf), bufsize-strlen(buf), ".%02d MB", 
			(value % (1024*1024))*100 / (1024*1024) );
	} else if (value > 1024 ) { // kilos
		Com_sprintf( buf, bufsize, "%d KB", value / 1024 );
	} else { // bytes
		Com_sprintf( buf, bufsize, "%d bytes", value );
	}
}

// Assumes time is in msec
static void UI_PrintTime ( char *buf, int bufsize, int time ) {
	time /= 1000;  // change to seconds

	if (time > 3600) { // in the hours range
		Com_sprintf( buf, bufsize, "%d hr %d min", time / 3600, (time % 3600) / 60 );
	} else if (time > 60) { // mins
		Com_sprintf( buf, bufsize, "%d min %d sec", time / 60, time % 60 );
	} else  { // secs
		Com_sprintf( buf, bufsize, "%d sec", time );
	}
}

void Text_PaintCenter(float x, float y, qhandle_t font, float scale, vec4_t color, const char *text, float adjust ) 
{
	int len = trap_R_GetTextWidth (text, font, scale, 0 );
	
	UI_DrawText ( x - len / 2, y, font, scale, color, text, 0, 0 );
}


static void UI_DisplayDownloadInfo( const char *downloadName, float centerPoint, float yStart, float scale ) 
{
	static char dlText[]	= "Downloading:";
	static char etaText[]	= "Estimated time left:";
	static char xferText[]	= "Transfer rate:";

	int downloadSize, downloadCount, downloadTime;
	char dlSizeBuf[64], totalSizeBuf[64], xferRateBuf[64], dlTimeBuf[64];
	int xferRate;
	int leftWidth;
	const char *s;

	downloadSize = trap_Cvar_VariableValue( "cl_downloadSize" );
	downloadCount = trap_Cvar_VariableValue( "cl_downloadCount" );
	downloadTime = trap_Cvar_VariableValue( "cl_downloadTime" );

	leftWidth = 320;

	UI_SetColor(colorWhite);
	Text_PaintCenter(centerPoint, yStart + 112, uiInfo.uiDC.Assets.defaultFont, scale, colorWhite, dlText, 0 );
	Text_PaintCenter(centerPoint, yStart + 192, uiInfo.uiDC.Assets.defaultFont, scale, colorWhite, etaText, 0 );
	Text_PaintCenter(centerPoint, yStart + 248, uiInfo.uiDC.Assets.defaultFont, scale, colorWhite, xferText, 0 );

	if (downloadSize > 0) {
		s = va( "%s (%d%%)", downloadName, downloadCount * 100 / downloadSize );
	} else {
		s = downloadName;
	}

	Text_PaintCenter(centerPoint, yStart+136, uiInfo.uiDC.Assets.defaultFont, scale, colorWhite, s, 0 );

	UI_ReadableSize( dlSizeBuf,		sizeof dlSizeBuf,		downloadCount );
	UI_ReadableSize( totalSizeBuf,	sizeof totalSizeBuf,	downloadSize );

	if (downloadCount < 4096 || !downloadTime) {
		Text_PaintCenter(leftWidth, yStart+216, uiInfo.uiDC.Assets.defaultFont, scale, colorWhite, "estimating", 0 );
		Text_PaintCenter(leftWidth, yStart+160, uiInfo.uiDC.Assets.defaultFont, scale, colorWhite, va("(%s of %s copied)", dlSizeBuf, totalSizeBuf), 0 );
	} else {
		if ((uiInfo.uiDC.realTime - downloadTime) / 1000) {
			xferRate = downloadCount / ((uiInfo.uiDC.realTime - downloadTime) / 1000);
		} else {
			xferRate = 0;
		}
		UI_ReadableSize( xferRateBuf, sizeof xferRateBuf, xferRate );

		// Extrapolate estimated completion time
		if (downloadSize && xferRate) {
			int n = downloadSize / xferRate; // estimated time for entire d/l in secs

			// We do it in K (/1024) because we'd overflow around 4MB
			UI_PrintTime ( dlTimeBuf, sizeof dlTimeBuf, 
				(n - (((downloadCount/1024) * n) / (downloadSize/1024))) * 1000);

			Text_PaintCenter(leftWidth, yStart+216, uiInfo.uiDC.Assets.defaultFont, scale, colorWhite, dlTimeBuf, 0 );
			Text_PaintCenter(leftWidth, yStart+160, uiInfo.uiDC.Assets.defaultFont, scale, colorWhite, va("(%s of %s copied)", dlSizeBuf, totalSizeBuf), 0 );
		} else {
			Text_PaintCenter(leftWidth, yStart+216, uiInfo.uiDC.Assets.defaultFont, scale, colorWhite, "estimating", 0 );
			if (downloadSize) {
				Text_PaintCenter(leftWidth, yStart+160, uiInfo.uiDC.Assets.defaultFont, scale, colorWhite, va("(%s of %s copied)", dlSizeBuf, totalSizeBuf), 0 );
			} else {
				Text_PaintCenter(leftWidth, yStart+160, uiInfo.uiDC.Assets.defaultFont, scale, colorWhite, va("(%s copied)", dlSizeBuf), 0 );
			}
		}

		if (xferRate) {
			Text_PaintCenter(leftWidth, yStart+272, uiInfo.uiDC.Assets.defaultFont, scale, colorWhite, va("%s/Sec", xferRateBuf), 0 );
		}
	}
}

/*
========================
UI_DrawConnectScreen

This will also be overlaid on the cgame info screen during loading
to prevent it from blinking away too rapidly on local or lan games.
========================
*/
void UI_DrawConnectScreen( qboolean overlay ) {
	char			*s;
	uiClientState_t	cstate;
	char			info[MAX_INFO_VALUE];
	char text[256];
	float centerPoint, yStart, scale;
	
	menuDef_t *menu = Menus_FindByName("Connect");

	if ( !ui_joinserver.integer )
	{
		trap_Cvar_Set ( "ui_joinserver", "1" );
	}

	if ( !overlay && menu ) {
		Menu_Paint(menu, qtrue);
	}

	if (!overlay) {
		centerPoint = 320;
		yStart = 340;
		scale = 0.53f;
	} else {
		centerPoint = 340;
		yStart = 380;
		scale = 0.53f;
		return;
	}

	// see what information we should display
	trap_GetClientState( &cstate );
		
	info[0] = '\0';
	if( trap_GetConfigString( CS_SERVERINFO, info, sizeof(info) ) ) 
	{
		Text_PaintCenter(centerPoint, yStart, uiInfo.uiDC.Assets.defaultFont, scale, colorWhite, va( "Loading %s", Info_ValueForKey( info, "mapname" )), 0 );
	}

	if (!Q_stricmp(cstate.servername,"localhost")) 
	{
		Text_PaintCenter(centerPoint, yStart + 48, uiInfo.uiDC.Assets.defaultFont, scale, colorWhite, va("Loading..."), 0 );
	} 
	else 
	{
		strcpy(text, va("Connecting to %s", cstate.servername));
		Text_PaintCenter(centerPoint, yStart + 48, uiInfo.uiDC.Assets.defaultFont, scale, colorWhite,text , 0 );
	}

	// display global MOTD at bottom
	Text_PaintCenter(centerPoint, 600, uiInfo.uiDC.Assets.defaultFont, scale, colorWhite, Info_ValueForKey( cstate.updateInfoString, "motd" ), 0 );
	
	// print any server info (server full, bad version, etc)
	if ( cstate.connState < CA_CONNECTED ) 
	{
		// Display the password dialog
		if ( strstr ( cstate.messageString, "password" ) )
		{
			if ( !uiInfo.connectPasswordRequest )
			{
				char password[MAX_QPATH];
				trap_Cvar_VariableStringBuffer ( "password", password, MAX_QPATH );

				// Dont bring it up again until the server says that password was bad too
				if ( strstr ( cstate.messageString, password ) )
				{
					trap_Key_SetCatcher( KEYCATCH_UI );
					Menus_CloseAll ( );
					Menus_ActivateByName("password_popmenu");
					uiInfo.connectPasswordRequest = qtrue;
				}
			}
		}
		
		Text_PaintCenter(centerPoint, yStart + 112, uiInfo.uiDC.Assets.defaultFont, scale, colorRed, cstate.messageString, 0 );
	}

	if ( lastConnState > cstate.connState ) {
		lastLoadingText[0] = '\0';
	}
	lastConnState = cstate.connState;

	switch ( cstate.connState ) {
	case CA_CONNECTING:
		s = va("Awaiting connection...%i", cstate.connectPacketCount);
		break;
	case CA_CHALLENGING:
		s = va("Awaiting challenge...%i", cstate.connectPacketCount);
		break;
	case CA_CONNECTED: {
		char downloadName[MAX_INFO_VALUE];

			trap_Cvar_VariableStringBuffer( "cl_downloadName", downloadName, sizeof(downloadName) );
			if (*downloadName) {
				UI_DisplayDownloadInfo( downloadName, centerPoint, yStart, scale );
				return;
			}
		}
		s = "Awaiting gamestate";
		break;
	case CA_LOADING:
		return;
	case CA_PRIMED:
		return;
	default:
		return;
	}


	if (Q_stricmp(cstate.servername,"localhost")) {
		Text_PaintCenter(centerPoint, yStart + 80, uiInfo.uiDC.Assets.defaultFont, scale, colorWhite, s, 0 );
	}

	// password required / connection rejected information goes here
}



/*
========================
UI_DrawConnectScreen

This will also be overlaid on the cgame info screen during loading
to prevent it from blinking away too rapidly on local or lan games.
========================
*/
void UI_DrawLoadingScreen( void ) 
{
	float centerPoint, yStart, scale;
	
	menuDef_t *menu = Menus_FindByName("Loading");

	if ( menu ) 
	{
		Menu_Paint(menu, qtrue);
	}

	centerPoint = 320;
	yStart = 388;
	scale = 0.53f;

	Text_PaintCenter(centerPoint, yStart, uiInfo.uiDC.Assets.defaultFont, scale, colorWhite, "Loading...", 0 );
}


/*
================
cvars
================
*/

typedef struct 
{
	vmCvar_t	*vmCvar;
	char		*cvarName;
	char		*defaultString;
	int			cvarFlags;
	float		mMinValue, mMaxValue;
} cvarTable_t;

vmCvar_t	ui_arenasFile;
vmCvar_t	ui_botsFile;
vmCvar_t	ui_botSkill;

vmCvar_t	ui_browserMaster;
vmCvar_t	ui_browserGameType;
vmCvar_t	ui_browserSortKey;
vmCvar_t	ui_browserShowFull;
vmCvar_t	ui_browserShowEmpty;

vmCvar_t	ui_drawCrosshair;
vmCvar_t	ui_drawCrosshairNames;

vmCvar_t	ui_server1;
vmCvar_t	ui_server2;
vmCvar_t	ui_server3;
vmCvar_t	ui_server4;
vmCvar_t	ui_server5;
vmCvar_t	ui_server6;
vmCvar_t	ui_server7;
vmCvar_t	ui_server8;
vmCvar_t	ui_server9;
vmCvar_t	ui_server10;
vmCvar_t	ui_server11;
vmCvar_t	ui_server12;
vmCvar_t	ui_server13;
vmCvar_t	ui_server14;
vmCvar_t	ui_server15;
vmCvar_t	ui_server16;

vmCvar_t	ui_cdkeychecked;
vmCvar_t	ui_invalidversion;
vmCvar_t	ui_downloadsize;
vmCvar_t	ui_downloadsite;
vmCvar_t	ui_downloadstatus;

vmCvar_t	ui_dedicated;
vmCvar_t	ui_gameType;
vmCvar_t	ui_netGameType;
vmCvar_t	ui_actualNetGameType;
vmCvar_t	ui_gtRespawnType;
vmCvar_t	ui_gtPickupsDisabled;
vmCvar_t	ui_joinGameType;
vmCvar_t	ui_netSource;
vmCvar_t	ui_serverFilterType;
vmCvar_t	ui_currentMap;
vmCvar_t	ui_currentNetMap;
vmCvar_t	ui_mapIndex;
vmCvar_t	ui_selectedPlayer;
vmCvar_t	ui_selectedPlayerName;
vmCvar_t	ui_lastServerRefresh_0;
vmCvar_t	ui_lastServerRefresh_1;
vmCvar_t	ui_lastServerRefresh_2;
vmCvar_t	ui_lastServerRefresh_3;
vmCvar_t	ui_scoreLimit;
vmCvar_t	ui_findPlayer;
vmCvar_t	ui_hudFiles;
vmCvar_t	ui_realWarmUp;
vmCvar_t	ui_serverStatusTimeOut;
vmCvar_t	ui_glCustom;
vmCvar_t	ui_botteam;

vmCvar_t	ui_rmg_config;
vmCvar_t	ui_rmg_size;
vmCvar_t	ui_rmg_time;
vmCvar_t	ui_rmg_seed;

// Readonly cvars used to transfer information from cgame to ui
vmCvar_t	ui_info_redteam;
vmCvar_t	ui_info_blueteam;
vmCvar_t	ui_info_redcount;
vmCvar_t	ui_info_bluecount;
vmCvar_t	ui_info_objectives;
vmCvar_t	ui_info_gametype;
vmCvar_t	ui_info_team;
vmCvar_t	ui_info_teamgame;
vmCvar_t	ui_info_redscore;
vmCvar_t	ui_info_bluescore;
vmCvar_t	ui_joinserver;

vmCvar_t	ui_allowparental;

static cvarTable_t cvarTable[] = 
{
	{ &ui_botteam, "ui_botteam", "auto", 0 },
	{ &ui_glCustom, "ui_glCustom", "0", CVAR_ARCHIVE },

	{ &ui_botsFile, "g_botsFile", "", CVAR_INIT|CVAR_ROM },
	{ &ui_botSkill, "g_botSkill", "2", CVAR_ARCHIVE },

	{ &ui_browserMaster, "ui_browserMaster", "0", CVAR_ARCHIVE },
	{ &ui_browserGameType, "ui_browserGameType", "0", CVAR_ARCHIVE },
	{ &ui_browserSortKey, "ui_browserSortKey", "4", CVAR_ARCHIVE },
	{ &ui_browserShowFull, "ui_browserShowFull", "1", CVAR_ARCHIVE },
	{ &ui_browserShowEmpty, "ui_browserShowEmpty", "1", CVAR_ARCHIVE },

	{ &ui_drawCrosshair, "cg_drawCrosshair", "4", CVAR_ARCHIVE },
	{ &ui_drawCrosshairNames, "cg_drawCrosshairNames", "1", CVAR_ARCHIVE },

	{ &ui_server1, "server1", "", CVAR_ARCHIVE },
	{ &ui_server2, "server2", "", CVAR_ARCHIVE },
	{ &ui_server3, "server3", "", CVAR_ARCHIVE },
	{ &ui_server4, "server4", "", CVAR_ARCHIVE },
	{ &ui_server5, "server5", "", CVAR_ARCHIVE },
	{ &ui_server6, "server6", "", CVAR_ARCHIVE },
	{ &ui_server7, "server7", "", CVAR_ARCHIVE },
	{ &ui_server8, "server8", "", CVAR_ARCHIVE },
	{ &ui_server9, "server9", "", CVAR_ARCHIVE },
	{ &ui_server10, "server10", "", CVAR_ARCHIVE },
	{ &ui_server11, "server11", "", CVAR_ARCHIVE },
	{ &ui_server12, "server12", "", CVAR_ARCHIVE },
	{ &ui_server13, "server13", "", CVAR_ARCHIVE },
	{ &ui_server14, "server14", "", CVAR_ARCHIVE },
	{ &ui_server15, "server15", "", CVAR_ARCHIVE },
	{ &ui_server16, "server16", "", CVAR_ARCHIVE },
	{ &ui_cdkeychecked, "ui_cdkeychecked", "0", CVAR_ROM },
	{ &ui_invalidversion, "ui_invalidversion", "0", CVAR_TEMP },
	{ &ui_downloadsize, "ui_downloadsize", "0", CVAR_TEMP },
	{ &ui_downloadsite, "ui_downloadsite", "0", CVAR_TEMP },
	{ &ui_downloadstatus, "ui_downloadstatus", "0", CVAR_TEMP },
	{ &ui_new, "ui_new", "0", CVAR_TEMP },
	{ &ui_debug, "ui_debug", "0", CVAR_TEMP },
	{ &ui_initialized, "ui_initialized", "0", CVAR_TEMP },
	{ &ui_dedicated, "ui_dedicated", "0", CVAR_ARCHIVE },
	{ &ui_gameType, "ui_gametype", "3", CVAR_ARCHIVE },
	{ &ui_joinGameType, "ui_joinGametype", "0", CVAR_ARCHIVE },
	{ &ui_netGameType, "ui_netGametype", "3", CVAR_ARCHIVE | CVAR_INTERNAL | CVAR_ROM },
	{ &ui_actualNetGameType, "ui_actualNetGametype", "3", CVAR_ARCHIVE | CVAR_INTERNAL | CVAR_ROM },
	{ &ui_gtRespawnType, "ui_gtRespawnType", "0", CVAR_ARCHIVE | CVAR_INTERNAL | CVAR_ROM },
	{ &ui_gtPickupsDisabled, "ui_gtPickupsDisabled", "0", CVAR_ARCHIVE | CVAR_INTERNAL | CVAR_ROM }, 
	{ &ui_netSource, "ui_netSource", "0", CVAR_ARCHIVE },
	{ &ui_currentMap, "ui_currentMap", "0", CVAR_ARCHIVE },
	{ &ui_currentNetMap, "ui_currentNetMap", "0", CVAR_ARCHIVE },
	{ &ui_mapIndex, "ui_mapIndex", "0", CVAR_ARCHIVE },
	{ &ui_selectedPlayer, "cg_selectedPlayer", "0", CVAR_ARCHIVE},
	{ &ui_selectedPlayerName, "cg_selectedPlayerName", "", CVAR_ARCHIVE},
	{ &ui_lastServerRefresh_0, "ui_lastServerRefresh_0", "", CVAR_ARCHIVE},
	{ &ui_lastServerRefresh_1, "ui_lastServerRefresh_1", "", CVAR_ARCHIVE},
	{ &ui_lastServerRefresh_2, "ui_lastServerRefresh_2", "", CVAR_ARCHIVE},
	{ &ui_lastServerRefresh_3, "ui_lastServerRefresh_3", "", CVAR_ARCHIVE},
	{ &ui_scoreLimit, "ui_scoreLimit", "10", 0},
	{ &ui_findPlayer, "ui_findPlayer", "unknown", CVAR_ARCHIVE},
	{ &ui_hudFiles, "cg_hudFiles", "ui/hud.txt", CVAR_ARCHIVE},
	{ &ui_sof2FirstRun, "ui_sof2FirstRun", "0", CVAR_ARCHIVE},
	{ &ui_realWarmUp, "g_warmup", "20", CVAR_ARCHIVE},
	{ &ui_serverStatusTimeOut, "ui_serverStatusTimeOut", "7000", CVAR_ARCHIVE},

	{ &ui_rmg_config,		"ui_rmg_config",		"desert",						CVAR_ARCHIVE },
	{ &ui_rmg_size,			"ui_rmg_size",			"small",						CVAR_ARCHIVE },
	{ &ui_rmg_time,			"ui_rmg_time",			"day",							CVAR_ARCHIVE },
	{ &ui_rmg_seed,			"ui_rmg_seed",			"0",							CVAR_ARCHIVE },

	// Readonly cvars used to transfer information from cgame to ui
	{ &ui_info_redteam,		"ui_info_redteam",		"0", CVAR_ROM|CVAR_INTERNAL },
	{ &ui_info_blueteam,	"ui_info_blueteam",		"0", CVAR_ROM|CVAR_INTERNAL },
	{ &ui_info_redcount,	"ui_info_redcount",		"0", CVAR_ROM|CVAR_INTERNAL },
	{ &ui_info_bluecount,	"ui_info_bluecount",	"0", CVAR_ROM|CVAR_INTERNAL },
	{ &ui_info_redscore,	"ui_info_redscore",		"0", CVAR_ROM|CVAR_INTERNAL },
	{ &ui_info_bluescore,	"ui_info_bluescore",	"0", CVAR_ROM|CVAR_INTERNAL },
	{ &ui_info_objectives,	"ui_info_objectives",	"0", CVAR_ROM|CVAR_INTERNAL },
	{ &ui_info_gametype,	"ui_info_gametype",		"0", CVAR_ROM|CVAR_INTERNAL },
	{ &ui_info_team,		"ui_info_team",			"0", CVAR_ROM|CVAR_INTERNAL },
	{ &ui_info_teamgame,	"ui_info_teamgame",		"0", CVAR_ROM|CVAR_INTERNAL },

	{ &ui_joinserver,		"ui_joinserver",		"0", CVAR_ROM|CVAR_INTERNAL },

	{ &ui_allowparental,	"ui_allowparental",		"1", CVAR_ROM | CVAR_INTERNAL | CVAR_PARENTAL },
};

static int	cvarTableSize = sizeof(cvarTable) / sizeof(cvarTable[0]);

/*
=================
UI_RegisterCvars
=================
*/
void UI_RegisterCvars( void ) 
{
	int			i;
	cvarTable_t	*cv;

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) 
	{
		trap_Cvar_Register( cv->vmCvar, cv->cvarName, cv->defaultString, cv->cvarFlags, cv->mMinValue, cv->mMaxValue );
	}

	trap_Cvar_Register(NULL, "lock_password",	"", CVAR_INTERNAL, 0.0, 0.0 );
	trap_Cvar_Register(NULL, "lock_blood",		"0", CVAR_INTERNAL, 0.0, 0.0 );
	trap_Cvar_Register(NULL, "lock_deaths",		"0", CVAR_INTERNAL, 0.0, 0.0 );
	trap_Cvar_Register(NULL, "lock_sever",		"0", CVAR_INTERNAL, 0.0, 0.0 );
}

/*
=================
UI_UpdateCvars
=================
*/
void UI_UpdateCvars( void ) 
{
	int			i;
	cvarTable_t	*cv;

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) 
	{
		int modificationCount = cv->vmCvar->modificationCount;

		trap_Cvar_Update( cv->vmCvar );

		if ( cv->vmCvar->modificationCount != modificationCount )
		{
			if ( Q_stricmp ( cv->cvarName, "ui_info_redteam" ) == 0 )
			{
				int	 count;
				int	 i;

				memset ( &uiInfo.identityTeams[TEAM_RED], 0, sizeof(TIdentity*) * MAX_TEAMIDENTITIES );

				if ( cv->vmCvar->string[0] )
				{
					for ( i = 0, count = 0; i < bg_identityCount && count < MAX_TEAMIDENTITIES; i ++ )
					{
						if ( Q_stricmp ( bg_identities[i].mTeam, cv->vmCvar->string ) == 0 )
						{
							uiInfo.identityTeams[TEAM_RED][count++] = &bg_identities[i];
						}
					}
				}
			}
			else if ( Q_stricmp ( cv->cvarName, "ui_info_blueteam" ) == 0 )
			{
				int	 count;
				int	 i;

				memset ( &uiInfo.identityTeams[TEAM_BLUE], 0, sizeof(TIdentity*) * MAX_TEAMIDENTITIES );

				if ( cv->vmCvar->string[0] )
				{
					for ( i = 0, count = 0; i < bg_identityCount && count < MAX_TEAMIDENTITIES; i ++ )
					{
						if ( Q_stricmp ( bg_identities[i].mTeam, cv->vmCvar->string ) == 0 )
						{
							uiInfo.identityTeams[TEAM_BLUE][count++] = &bg_identities[i];
						}
					}
				}
			}
		}
	}
}


/*
=================
ArenaServers_StopRefresh
=================
*/
static void UI_StopServerRefresh( void )
{
	int count;

	if (!uiInfo.serverStatus.refreshActive) {
		// not currently refreshing
		return;
	}
	uiInfo.serverStatus.refreshActive = qfalse;
	Com_Printf("%d servers listed in browser with %d players.\n",
					uiInfo.serverStatus.numDisplayServers,
					uiInfo.serverStatus.numPlayersOnServers);
	count = trap_LAN_GetServerCount(ui_netSource.integer);
	if (count - uiInfo.serverStatus.numDisplayServers > 0) {
		Com_Printf("%d servers not listed due to packet loss or pings higher than %d\n",
						count - uiInfo.serverStatus.numDisplayServers,
						(int) trap_Cvar_VariableValue("cl_maxPing"));
	}

}

/*
=================
ArenaServers_MaxPing
=================
*/
#ifndef MISSIONPACK // bk001206
static int ArenaServers_MaxPing( void ) {
	int		maxPing;

	maxPing = (int)trap_Cvar_VariableValue( "cl_maxPing" );
	if( maxPing < 100 ) {
		maxPing = 100;
	}
	return maxPing;
}
#endif

/*
=================
UI_DoServerRefresh
=================
*/
static void UI_DoServerRefresh( void )
{
	qboolean wait = qfalse;

	if (!uiInfo.serverStatus.refreshActive) {
		return;
	}
	if (ui_netSource.integer != AS_FAVORITES) {
		if (ui_netSource.integer == AS_LOCAL) {
			if (!trap_LAN_GetServerCount(ui_netSource.integer)) {
				wait = qtrue;
			}
		} else {
			if (trap_LAN_GetServerCount(ui_netSource.integer) < 0) {
				wait = qtrue;
			}
		}
	}

	if (uiInfo.uiDC.realTime < uiInfo.serverStatus.refreshtime) {
		if (wait) {
			return;
		}
	}

	// if still trying to retrieve pings
	if (trap_LAN_UpdateVisiblePings(ui_netSource.integer)) {
		uiInfo.serverStatus.refreshtime = uiInfo.uiDC.realTime + 1000;
	} else if (!wait) {
		// get the last servers in the list
		UI_BuildServerDisplayList(2);
		// stop the refresh
		UI_StopServerRefresh();
	}
	//
	UI_BuildServerDisplayList(qfalse);
}

/*
=================
UI_StartServerRefresh
=================
*/
static void UI_StartServerRefresh ( qboolean full )
{
	char*	ptr;
	qtime_t q;

	trap_RealTime(&q);
 	trap_Cvar_Set( va("ui_lastServerRefresh_%i", ui_netSource.integer), va("%s-%i, %i at %i:%i", MonthAbbrev[q.tm_mon],q.tm_mday, 1900+q.tm_year,q.tm_hour,q.tm_min));

	if (!full) 
	{
		UI_UpdatePendingPings();
		return;
	}

	uiInfo.serverStatus.refreshActive = qtrue;
	uiInfo.serverStatus.nextDisplayRefresh = uiInfo.uiDC.realTime + 1000;

	// clear number of displayed servers
	uiInfo.serverStatus.numDisplayServers = 0;
	uiInfo.serverStatus.numPlayersOnServers = 0;

	// mark all servers as visible so we store ping updates for them
	trap_LAN_MarkServerVisible(ui_netSource.integer, -1, qtrue);
	
	// reset all the pings
	trap_LAN_ResetPings(ui_netSource.integer);

	if( ui_netSource.integer == AS_LOCAL ) 
	{
		trap_Cmd_ExecuteText( EXEC_NOW, "localservers\n" );
		uiInfo.serverStatus.refreshtime = uiInfo.uiDC.realTime + 1000;
		return;
	}

	uiInfo.serverStatus.refreshtime = uiInfo.uiDC.realTime + 5000;
	if( ui_netSource.integer == AS_GLOBAL  ) 
	{
		ptr = UI_Cvar_VariableString("debug_protocol");
		if (strlen(ptr)) 
		{
			trap_Cmd_ExecuteText( EXEC_NOW, va( "globalservers 0 %s\n", ptr));
		}
		else 
		{
			trap_Cmd_ExecuteText( EXEC_NOW, va( "globalservers 0 %d\n", (int)trap_Cvar_VariableValue( "protocol" ) ) );
		}
	}
}

