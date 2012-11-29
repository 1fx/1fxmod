// Copyright (C) 2001-2002 Raven Software.
// 
// string allocation/managment

#include "ui_shared.h"
#include "../game/bg_public.h"

#define SCROLL_TIME_START					500
#define SCROLL_TIME_ADJUST				150
#define SCROLL_TIME_ADJUSTOFFSET	40
#define SCROLL_TIME_FLOOR					20

typedef struct scrollInfo_s {
	int nextScrollTime;
	int nextAdjustTime;
	int adjustValue;
	int scrollKey;
	float xStart;
	float yStart;
	itemDef_t *item;
	qboolean scrollDir;
} scrollInfo_t;

static scrollInfo_t scrollInfo;

static void (*captureFunc) (void *p) = NULL;
static void *captureData = NULL;
static itemDef_t *itemCapture = NULL;		// item that has the mouse captured ( if any )
static itemDef_t *itemExclusive = NULL;		// item that has exclusive input (combo box)

displayContextDef_t *DC = NULL;

static qboolean g_waitingForKey = qfalse;
static qboolean g_editingField = qfalse;

static itemDef_t *g_bindItem = NULL;
static itemDef_t *g_editItem = NULL;

menuDef_t Menus[MAX_MENUS];      // defined menus
int menuCount = 0;               // how many

menuDef_t *menuStack[MAX_OPEN_MENUS];
int openMenuCount = 0;

static qboolean debugMode = qfalse;

#define DOUBLE_CLICK_DELAY 300
static int lastListBoxClickTime = 0;

void Item_RunScript(itemDef_t *item, const char *s);
void Item_SetupKeywordHash(void);
void Menu_SetupKeywordHash(void);
int BindingIDFromName(const char *name);
qboolean Item_Bind_HandleKey(itemDef_t *item, int key, qboolean down);
itemDef_t *Menu_SetPrevCursorItem(menuDef_t *menu);
itemDef_t *Menu_SetNextCursorItem(menuDef_t *menu);
static qboolean Menu_OverActiveItem(menuDef_t *menu, float x, float y);
static void Item_TextScroll_BuildLines ( itemDef_t* item );

typedef struct  itemFlagsDef_s 
{
	char	*string;
	int		value;

}	itemFlagsDef_t;

itemFlagsDef_t itemFlags [] = 
{
	"WINDOW_INACTIVE",		WINDOW_INACTIVE,
	NULL,					(int) NULL
};

char *styles [] = 
{
	"WINDOW_STYLE_EMPTY",
	"WINDOW_STYLE_FILLED",
	"WINDOW_STYLE_SHADER",
	"WINDOW_STYLE_CINEMATIC",
	NULL
};

char *alignment [] = 
{
	"ITEM_ALIGN_LEFT",
	"ITEM_ALIGN_CENTER",
	"ITEM_ALIGN_RIGHT",
	NULL
};

char *types [] = 
{
	"ITEM_TYPE_TEXT",
	"ITEM_TYPE_BUTTON",
	"ITEM_TYPE_RADIOBUTTON",
	"ITEM_TYPE_CHECKBOX",
	"ITEM_TYPE_EDITFIELD",
	"ITEM_TYPE_COMBO",
	"ITEM_TYPE_LISTBOX",
	"ITEM_TYPE_OWNERDRAW",
	"ITEM_TYPE_MODEL",
	"ITEM_TYPE_NUMERICFIELD",
	"ITEM_TYPE_SLIDER",
	"ITEM_TYPE_YESNO",
	"ITEM_TYPE_MULTI",
	"ITEM_TYPE_BIND",
	"ITEM_TYPE_PASSWORDFIELD",
	"ITEM_TYPE_COMBOBOX",
	"ITEM_TYPE_TEXTSCROLL",
	NULL
};

#if !(CGAME)
	extern vmCvar_t	ui_allowparental;
#endif

#define HASH_TABLE_SIZE 2048

/*
================
return a hash value for the string
================
*/
static long hashForString(const char *str) {
	int		i;
	long	hash;
	char	letter;

	hash = 0;
	i = 0;
	while (str[i] != '\0') {
		letter = tolower(str[i]);
		hash+=(long)(letter)*(i+119);
		i++;
	}
	hash &= (HASH_TABLE_SIZE-1);
	return hash;
}

typedef struct stringDef_s {
	struct stringDef_s *next;
	const char *str;
} stringDef_t;

static int strPoolIndex = 0;
static char strPool[STRING_POOL_SIZE];

static int strHandleCount = 0;
static stringDef_t *strHandle[HASH_TABLE_SIZE];


const char *String_Alloc(const char *p) {
	int len;
	long hash;
	stringDef_t *str, *last;
	static const char *staticNULL = "";

	if (p == NULL) {
		return NULL;
	}

	if (*p == 0) {
		return staticNULL;
	}

	hash = hashForString(p);

	str = strHandle[hash];
	while (str) {
		if (strcmp(p, str->str) == 0) {
			return str->str;
		}
		str = str->next;
	}

	len = strlen(p);
	if (len + strPoolIndex + 1 < STRING_POOL_SIZE) {
		int ph = strPoolIndex;
		strcpy(&strPool[strPoolIndex], p);
		strPoolIndex += len + 1;

		str = strHandle[hash];
		last = str;
		while (str && str->next) {
			last = str;
			str = str->next;
		}

		str  = trap_VM_LocalAlloc(sizeof(stringDef_t));
		str->next = NULL;
		str->str = &strPool[ph];
		if (last) {
			last->next = str;
		} else {
			strHandle[hash] = str;
		}
		return &strPool[ph];
	}
	return NULL;
}

void String_Report() {
	float f;
	Com_Printf("Memory/String Pool Info\n");
	Com_Printf("----------------\n");
	f = strPoolIndex;
	f /= STRING_POOL_SIZE;
	f *= 100;
	Com_Printf("String Pool is %.1f%% full, %i bytes out of %i used.\n", f, strPoolIndex, STRING_POOL_SIZE);
}

/*
=================
String_Init
=================
*/
void String_Init() {
	int i;
	for (i = 0; i < HASH_TABLE_SIZE; i++) {
		strHandle[i] = 0;
	}
	strHandleCount = 0;
	strPoolIndex = 0;
	menuCount = 0;
	openMenuCount = 0;
	Item_SetupKeywordHash();
	Menu_SetupKeywordHash();
	if (DC && DC->getBindingBuf) {
		Controls_GetConfig();
	}
}

/*
=================
PC_SourceWarning
=================
*/
void PC_SourceWarning(int handle, char *format, ...) 
{
	int line;
	char filename[128];
	va_list argptr;
	static char string[4096];

	va_start (argptr, format);
	vsprintf (string, format, argptr);
	va_end (argptr);

	filename[0] = '\0';
	line = 0;
	trap_PC_SourceFileAndLine(handle, filename, &line);

	Com_Printf(S_COLOR_YELLOW "WARNING: %s, line %d: %s\n", filename, line, string);
}

/*
=================
PC_SourceError
=================
*/
void PC_SourceError(int handle, char *format, ...) {
	int line;
	char filename[128];
	va_list argptr;
	static char string[4096];

	va_start (argptr, format);
	vsprintf (string, format, argptr);
	va_end (argptr);

	filename[0] = '\0';
	line = 0;
	trap_PC_SourceFileAndLine(handle, filename, &line);

	Com_Printf(S_COLOR_RED "ERROR: %s, line %d: %s\n", filename, line, string);
}

/*
=================
LerpColor
=================
*/
void LerpColor(vec4_t a, vec4_t b, vec4_t c, float t)
{
	int i;

	// lerp and clamp each component
	for (i=0; i<4; i++)
	{
		c[i] = a[i] + t*(b[i]-a[i]);
		if (c[i] < 0)
			c[i] = 0;
		else if (c[i] > 1.0)
			c[i] = 1.0;
	}
}

/*
=================
Float_Parse
=================
*/
qboolean Float_Parse(const char **p, float *f) {
	char	*token;
	token = COM_ParseExt(p, qfalse);
	if (token && token[0] != 0) {
		*f = atof(token);
		return qtrue;
	} else {
		return qfalse;
	}
}

/*
=================
PC_Float_Parse
=================
*/
qboolean PC_Float_Parse(int handle, float *f) {
	pc_token_t token;
	int negative = qfalse;

	if (!trap_PC_ReadToken(handle, &token))
		return qfalse;
	if (token.string[0] == '-') {
		if (!trap_PC_ReadToken(handle, &token))
			return qfalse;
		negative = qtrue;
	}
	if (token.type != TT_NUMBER) {
		PC_SourceError(handle, "expected float but found %s\n", token.string);
		return qfalse;
	}
	if (negative)
		*f = -token.floatvalue;
	else
		*f = token.floatvalue;
	return qtrue;
}

/*
=================
Color_Parse
=================
*/
qboolean Color_Parse(const char **p, vec4_t *c) {
	int i;
	float f;

	for (i = 0; i < 4; i++) {
		if (!Float_Parse(p, &f)) {
			return qfalse;
		}
		(*c)[i] = f;
	}
	return qtrue;
}

/*
=================
PC_Color_Parse
=================
*/
qboolean PC_Color_Parse(int handle, vec4_t *c) {
	int i;
	float f;

	for (i = 0; i < 4; i++) {
		if (!PC_Float_Parse(handle, &f)) {
			return qfalse;
		}
		(*c)[i] = f;
	}
	return qtrue;
}

/*
=================
Int_Parse
=================
*/
qboolean Int_Parse(const char **p, int *i) {
	char	*token;
	token = COM_ParseExt(p, qfalse);

	if (token && token[0] != 0) {
		*i = atoi(token);
		return qtrue;
	} else {
		return qfalse;
	}
}

/*
=================
PC_Int_Parse
=================
*/
qboolean PC_Int_Parse(int handle, int *i) {
	pc_token_t token;
	int negative = qfalse;

	if (!trap_PC_ReadToken(handle, &token))
		return qfalse;
	if (token.string[0] == '-') {
		if (!trap_PC_ReadToken(handle, &token))
			return qfalse;
		negative = qtrue;
	}
	if (token.type != TT_NUMBER) {
		PC_SourceError(handle, "expected integer but found %s\n", token.string);
		return qfalse;
	}
	*i = token.intvalue;
	if (negative)
		*i = - *i;
	return qtrue;
}

/*
=================
Rect_Parse
=================
*/
qboolean Rect_Parse(const char **p, rectDef_t *r) {
	if (Float_Parse(p, &r->x)) {
		if (Float_Parse(p, &r->y)) {
			if (Float_Parse(p, &r->w)) {
				if (Float_Parse(p, &r->h)) {
					return qtrue;
				}
			}
		}
	}
	return qfalse;
}

/*
=================
PC_Rect_Parse
=================
*/
qboolean PC_Rect_Parse(int handle, rectDef_t *r) {
	if (PC_Float_Parse(handle, &r->x)) {
		if (PC_Float_Parse(handle, &r->y)) {
			if (PC_Float_Parse(handle, &r->w)) {
				if (PC_Float_Parse(handle, &r->h)) {
					return qtrue;
				}
			}
		}
	}
	return qfalse;
}

/*
=================
String_Parse
=================
*/
qboolean String_Parse(const char **p, const char **out) {
	char *token;

	token = COM_ParseExt(p, qfalse);
	if (token && token[0] != 0) {
		*(out) = String_Alloc(token);
		return qtrue;
	}
	return qfalse;
}

/*
=================
PC_String_Parse
=================
*/
qboolean PC_String_Parse(int handle, const char **out) 
{
	pc_token_t token;

	if (!trap_PC_ReadToken(handle, &token))
	{
		return qfalse;
	}

	*(out) = String_Alloc(token.string);
	return qtrue;
}

/*
=================
PC_Script_Parse
=================
*/
qboolean PC_Script_Parse(int handle, const char **out) {
	char script[1024];
	pc_token_t token;

	memset(script, 0, sizeof(script));
	// scripts start with { and have ; separated command lists.. commands are command, arg.. 
	// basically we want everything between the { } as it will be interpreted at run time
  
	if (!trap_PC_ReadToken(handle, &token))
		return qfalse;
	if (Q_stricmp(token.string, "{") != 0) {
	    return qfalse;
	}

	while ( 1 ) {
		if (!trap_PC_ReadToken(handle, &token))
			return qfalse;

		if (Q_stricmp(token.string, "}") == 0) {
			*out = String_Alloc(script);
			return qtrue;
		}

		if (token.string[1] != '\0') {
			Q_strcat(script, 1024, va("\"%s\"", token.string));
		} else {
			Q_strcat(script, 1024, token.string);
		}
		Q_strcat(script, 1024, " ");
	}
	return qfalse; 	// bk001105 - LCC   missing return value
}

/*
==================
Init_Display

Initializes the display with a structure to all the drawing routines
 ==================
*/
void Init_Display(displayContextDef_t *dc) 
{
	DC = dc;
}

/*
==================
Window_Init

Initializes a window structure ( windowDef_t ) with defaults
==================
*/
void Window_Init(Window *w) {
	memset(w, 0, sizeof(windowDef_t));
	w->borderSize = 1;
	w->foreColor[0] = w->foreColor[1] = w->foreColor[2] = w->foreColor[3] = 1.0;
	w->cinematic = -1;
}

void Fade(int *flags, float *f, float clamp, int *nextTime, int offsetTime, qboolean bFlags, float fadeAmount) {
  if (*flags & (WINDOW_FADINGOUT | WINDOW_FADINGIN)) {
    if (DC->realTime > *nextTime) {
      *nextTime = DC->realTime + offsetTime;
      if (*flags & WINDOW_FADINGOUT) {
        *f -= fadeAmount;
        if (bFlags && *f <= 0.0) {
          *flags &= ~(WINDOW_FADINGOUT | WINDOW_VISIBLE);
        }
      } else {
        *f += fadeAmount;
        if (*f >= clamp) {
          *f = clamp;
          if (bFlags) {
            *flags &= ~WINDOW_FADINGIN;
          }
        }
      }
    }
  }
}


void Item_SetMouseOver(itemDef_t *item, qboolean focus) {
  if (item) {
    if (focus) {
      item->window.flags |= WINDOW_MOUSEOVER;
    } else {
      item->window.flags &= ~WINDOW_MOUSEOVER;
    }
  }

	// Save the item that the mouse is over
	if ( item->tooltip )
	{
		if ( focus )
		{
			DC->tooltipItem = item;
		}
		else if ( DC->tooltipItem == item )
		{
			DC->tooltipItem = NULL;
		}
	}
}

void Window_Paint(Window *w, float fadeAmount, float fadeClamp, float fadeCycle) 
{
	vec4_t		color;
	rectDef_t	fillRect;

	fillRect = w->rect;

	if (debugMode) 
	{
		color[0] = color[1] = color[2] = color[3] = 1;
		DC->drawRect(w->rect.x, w->rect.y, w->rect.w, w->rect.h, 1, color);
	}

	if (w == NULL || (w->style == 0 && w->border == 0)) 
	{
		return;
	}

	if (w->border != 0) 
	{
		fillRect.x += w->borderSize;
		fillRect.y += w->borderSize;
		fillRect.w -= w->borderSize + 1;
		fillRect.h -= w->borderSize + 1;
	}

	if (w->style == WINDOW_STYLE_FILLED) 
	{
		// box, but possible a shader that needs filled
		if (w->background) 
		{
			Fade(&w->flags, &w->backColor[3], fadeClamp, &w->nextTime, fadeCycle, qtrue, fadeAmount);
			DC->setColor(w->backColor);
			DC->drawHandlePic(fillRect.x, fillRect.y, fillRect.w, fillRect.h, w->background);
			DC->setColor(NULL);
		} 
		else 
		{
			DC->fillRect(fillRect.x, fillRect.y, fillRect.w, fillRect.h, w->backColor);
		}
	} 
	else if (w->style == WINDOW_STYLE_SHADER) 
	{
		if (w->flags & WINDOW_FORECOLORSET) 
		{
			DC->setColor(w->foreColor);
		}
		DC->drawHandlePic(fillRect.x, fillRect.y, fillRect.w, fillRect.h, w->background);
		DC->setColor(NULL);
	} 
	else if (w->style == WINDOW_STYLE_CINEMATIC) 
	{
		if (w->cinematic == -1) 
		{
			w->cinematic = DC->playCinematic(w->cinematicName, fillRect.x, fillRect.y, fillRect.w, fillRect.h);
			if (w->cinematic == -1) 
			{
				w->cinematic = -2;
			}
		} 
		if (w->cinematic >= 0) 
		{
			DC->runCinematicFrame(w->cinematic);
			DC->drawCinematic(w->cinematic, fillRect.x, fillRect.y, fillRect.w, fillRect.h);
		}
	}

	if (w->border == WINDOW_BORDER_FULL) 
	{
		DC->drawRect(w->rect.x, w->rect.y, w->rect.w, w->rect.h, w->borderSize, w->borderColor);
	} 
	else if (w->border == WINDOW_BORDER_HORZ) 
	{
		// top/bottom
		DC->setColor(w->borderColor);
		DC->drawTopBottom(w->rect.x, w->rect.y, w->rect.w, w->rect.h, w->borderSize);
		DC->setColor( NULL );
	} 
	else if (w->border == WINDOW_BORDER_VERT) 
	{
		// left right
		DC->setColor(w->borderColor);
		DC->drawSides(w->rect.x, w->rect.y, w->rect.w, w->rect.h, w->borderSize);
		DC->setColor( NULL );
	} 
}


void Item_SetScreenCoords(itemDef_t *item, float x, float y) 
{ 
	if (item == NULL) 
	{
	    return;
	}

	if (item->window.border != 0) 
	{
		x += item->window.borderSize;
		y += item->window.borderSize;
	}

	item->window.rect.x = x + item->window.rectClient.x;
	item->window.rect.y = y + item->window.rectClient.y;
	item->window.rect.w = item->window.rectClient.w;
	item->window.rect.h = item->window.rectClient.h;

	// force the text rects to recompute
	item->textRect.w = 0;
	item->textRect.h = 0;

	switch ( item->type )
	{
		case ITEM_TYPE_COMBOBOX:
		{
			listBoxDef_t* listPtr = (listBoxDef_t*) item->typeData;

			listPtr->comboRect = item->window.rect;
			listPtr->comboRect.h = listPtr->elementHeight;

			if ( !(item->window.flags & WINDOW_DROPPED ) )
			{
				item->window.rect.h = listPtr->elementHeight;
			}

			listPtr->listRect = item->window.rect;
			listPtr->listRect.y += (listPtr->elementHeight + item->window.border);
			listPtr->listRect.h -= (listPtr->elementHeight + item->window.border);
			
			break;
		}

		case ITEM_TYPE_LISTBOX:
		{
			listBoxDef_t* listPtr = (listBoxDef_t*) item->typeData;
			listPtr->listRect = item->window.rect;
			break;
		}

		case ITEM_TYPE_TEXTSCROLL:
		{
			textScrollDef_t *scrollPtr = (textScrollDef_t*)item->typeData;
			if ( scrollPtr )
			{
				scrollPtr->startPos = 0;
				scrollPtr->endPos = 0;
			}

			Item_TextScroll_BuildLines ( item );

			break;
		}
	}	
}

// FIXME: consolidate this with nearby stuff
void Item_UpdatePosition(itemDef_t *item) 
{
	float x, y;
	menuDef_t *menu;
  
	if (item == NULL || item->parent == NULL) 
	{
		return;
	}

	menu = item->parent;

	x = menu->window.rect.x;
	y = menu->window.rect.y;
  
	if (menu->window.border != 0) 
	{
		x += menu->window.borderSize;
		y += menu->window.borderSize;
	}

	Item_SetScreenCoords(item, x, y);
}

// menus
void Menu_UpdatePosition(menuDef_t *menu) 
{
	int	  i;
	float x;
	float y;

	if (menu == NULL) 
	{
		return;
	}
  
	x = menu->window.rect.x;
	y = menu->window.rect.y;
	
	if (menu->window.border != 0) 
	{
		x += menu->window.borderSize;
		y += menu->window.borderSize;
	}

	for (i = 0; i < menu->itemCount; i++) 
	{
		Item_SetScreenCoords(menu->items[i], x, y);
	}
}

void Menu_PostParse(menuDef_t *menu) 
{
	if (menu == NULL) 
	{
		return;
	}

	if (menu->fullScreen) 
	{
		menu->window.rect.x = 0;
		menu->window.rect.y = 0;
		menu->window.rect.w = 640;
		menu->window.rect.h = 480;
	}

	Menu_UpdatePosition(menu);
}

itemDef_t *Menu_ClearFocus(menuDef_t *menu) {
  int i;
  itemDef_t *ret = NULL;

  if (menu == NULL) {
    return NULL;
  }

  for (i = 0; i < menu->itemCount; i++) {
    if (menu->items[i]->window.flags & WINDOW_HASFOCUS) {
      ret = menu->items[i];
    } 
    menu->items[i]->window.flags &= ~WINDOW_HASFOCUS;
    if (menu->items[i]->leaveFocus) {
      Item_RunScript(menu->items[i], menu->items[i]->leaveFocus);
    }
  }
 
  return ret;
}

qboolean IsVisible(int flags) {
  return (flags & WINDOW_VISIBLE && !(flags & WINDOW_FADINGOUT));
}

qboolean Rect_ContainsPoint(rectDef_t *rect, float x, float y) {
  if (rect) {
    if (x > rect->x && x < rect->x + rect->w && y > rect->y && y < rect->y + rect->h) {
      return qtrue;
    }
  }
  return qfalse;
}

int Menu_ItemsMatchingGroup(menuDef_t *menu, const char *name) {
  int i;
  int count = 0;
  for (i = 0; i < menu->itemCount; i++) {
    if (Q_stricmp(menu->items[i]->window.name, name) == 0 || (menu->items[i]->window.group && Q_stricmp(menu->items[i]->window.group, name) == 0)) {
      count++;
    } 
  }
  return count;
}

itemDef_t *Menu_GetMatchingItemByNumber(menuDef_t *menu, int index, const char *name) {
  int i;
  int count = 0;
  for (i = 0; i < menu->itemCount; i++) {
    if (Q_stricmp(menu->items[i]->window.name, name) == 0 || (menu->items[i]->window.group && Q_stricmp(menu->items[i]->window.group, name) == 0)) {
      if (count == index) {
        return menu->items[i];
      }
      count++;
    } 
  }
  return NULL;
}



void Script_SetColor(itemDef_t *item, const char **args) {
  const char *name;
  int i;
  float f;
  vec4_t *out;
  // expecting type of color to set and 4 args for the color
  if (String_Parse(args, &name)) {
      out = NULL;
      if (Q_stricmp(name, "backcolor") == 0) {
        out = &item->window.backColor;
        item->window.flags |= WINDOW_BACKCOLORSET;
      } else if (Q_stricmp(name, "forecolor") == 0) {
        out = &item->window.foreColor;
        item->window.flags |= WINDOW_FORECOLORSET;
      } else if (Q_stricmp(name, "bordercolor") == 0) {
        out = &item->window.borderColor;
      }

      if (out) {
        for (i = 0; i < 4; i++) {
          if (!Float_Parse(args, &f)) {
            return;
          }
          (*out)[i] = f;
        }
      }
  }
}

void Script_SetAsset(itemDef_t *item, const char **args) {
  const char *name;
  // expecting name to set asset to
  if (String_Parse(args, &name)) {
    // check for a model 
    if (item->type == ITEM_TYPE_MODEL) {
    }
  }
}

void Script_SetMenuBackground ( itemDef_t* item, const char **args)
{
	const char* name;

	// expecting name to set asset to
	if (String_Parse(args, &name)) 
	{
		if ( item->parent )
		{
			menuDef_t* menu = (menuDef_t*)item->parent;
			menu->window.background = DC->registerShaderNoMip(name);
		}
	}
}


void Script_SetBackground(itemDef_t *item, const char **args) 
{
	const char *name;

	// expecting name to set asset to
	if (String_Parse(args, &name)) 
	{
		item->window.background = DC->registerShaderNoMip(name);
	}
}




itemDef_t *Menu_FindItemByName(menuDef_t *menu, const char *p) {
  int i;
  if (menu == NULL || p == NULL) {
    return NULL;
  }

  for (i = 0; i < menu->itemCount; i++) {
    if (Q_stricmp(p, menu->items[i]->window.name) == 0) {
      return menu->items[i];
    }
  }

  return NULL;
}

void Script_SetTeamColor(itemDef_t *item, const char **args) {
  if (DC->getTeamColor) {
    int i;
    vec4_t color;
    DC->getTeamColor(&color);
    for (i = 0; i < 4; i++) {
      item->window.backColor[i] = color[i];
    }
  }
}

void Script_SetItemColor(itemDef_t *item, const char **args) {
  const char *itemname;
  const char *name;
  vec4_t color;
  int i;
  vec4_t *out;
  // expecting type of color to set and 4 args for the color
  if (String_Parse(args, &itemname) && String_Parse(args, &name)) {
    itemDef_t *item2;
    int j;
    int count = Menu_ItemsMatchingGroup(item->parent, itemname);

    if (!Color_Parse(args, &color)) {
      return;
    }

    for (j = 0; j < count; j++) {
      item2 = Menu_GetMatchingItemByNumber(item->parent, j, itemname);
      if (item2 != NULL) {
        out = NULL;
        if (Q_stricmp(name, "backcolor") == 0) {
          out = &item2->window.backColor;
        } else if (Q_stricmp(name, "forecolor") == 0) {
          out = &item2->window.foreColor;
          item2->window.flags |= WINDOW_FORECOLORSET;
        } else if (Q_stricmp(name, "bordercolor") == 0) {
          out = &item2->window.borderColor;
        }

        if (out) {
          for (i = 0; i < 4; i++) {
            (*out)[i] = color[i];
          }
        }
      }
    }
  }
}


void Menu_ShowItemByName(menuDef_t *menu, const char *p, qboolean bShow) {
	itemDef_t *item;
	int i;
	int count = Menu_ItemsMatchingGroup(menu, p);
	for (i = 0; i < count; i++) {
		item = Menu_GetMatchingItemByNumber(menu, i, p);
		if (item != NULL) {
			if (bShow) {
				item->window.flags |= WINDOW_VISIBLE;
			} else {
				item->window.flags &= ~WINDOW_VISIBLE;
				// stop cinematics playing in the window
				if (item->window.cinematic >= 0) {
					DC->stopCinematic(item->window.cinematic);
					item->window.cinematic = -1;
				}

				if ( DC->tooltipItem == item )
					DC->tooltipItem = NULL;
			}
		}
	}
}

void Menu_FadeItemByName(menuDef_t *menu, const char *p, qboolean fadeOut) {
  itemDef_t *item;
  int i;
  int count = Menu_ItemsMatchingGroup(menu, p);
  for (i = 0; i < count; i++) {
    item = Menu_GetMatchingItemByNumber(menu, i, p);
    if (item != NULL) {
      if (fadeOut) {
        item->window.flags |= (WINDOW_FADINGOUT | WINDOW_VISIBLE);
        item->window.flags &= ~WINDOW_FADINGIN;
      } else {
        item->window.flags |= (WINDOW_VISIBLE | WINDOW_FADINGIN);
        item->window.flags &= ~WINDOW_FADINGOUT;
      }
    }
  }
}

menuDef_t *Menus_FindByName(const char *p) {
  int i;
  for (i = 0; i < menuCount; i++) {
    if (Q_stricmp(Menus[i].window.name, p) == 0) {
      return &Menus[i];
    } 
  }
  return NULL;
}

void Menus_ShowByName(const char *p) {
	menuDef_t *menu = Menus_FindByName(p);
	if (menu) {
		Menus_Activate(menu);
	}
}

void Menus_OpenByName(const char *p) {
  Menus_ActivateByName(p);
}

static void Menu_RunCloseScript(menuDef_t *menu) {
	if (menu && (menu->window.flags & WINDOW_VISIBLE) && menu->onClose) {
		itemDef_t item;
    item.parent = menu;
    Item_RunScript(&item, menu->onClose);
	}
}

void Menus_CloseByName(const char *p) 
{
	menuDef_t *menu = Menus_FindByName(p);
	
	// If the menu wasnt found just exit
	if (menu == NULL) 
	{
		return;
	}

	// Run the close script for the menu
	Menu_RunCloseScript(menu);

	// If this window had the focus then take it away
	if ( menu->window.flags & WINDOW_HASFOCUS )
	{	
		// If there is something still in the open menu list then
		// set it to have focus now
		if ( openMenuCount )
		{
			// Subtract one from the open menu count to prepare to
			// remove the top menu from the list
			openMenuCount -= 1;

			// Set the top menu to have focus now
			menuStack[openMenuCount]->window.flags |= WINDOW_HASFOCUS;

			// Remove the top menu from the list
			menuStack[openMenuCount] = NULL;
		}
	}

	// Window is now invisible and doenst have focus
	menu->window.flags &= ~(WINDOW_VISIBLE | WINDOW_HASFOCUS);
}

void Menus_CloseAll() 
{
	int i;

	itemExclusive = NULL;
	itemCapture = NULL;

	g_waitingForKey = qfalse;

	for (i = 0; i < menuCount; i++) 
	{
		Menu_RunCloseScript ( &Menus[i] );
		Menus[i].window.flags &= ~(WINDOW_HASFOCUS | WINDOW_VISIBLE);
	}

	// Clear the menu stack
	openMenuCount = 0;
}

void Script_Show(itemDef_t *item, const char **args) {
  const char *name;
  if (String_Parse(args, &name)) {
    Menu_ShowItemByName(item->parent, name, qtrue);
  }
}

void Script_Hide(itemDef_t *item, const char **args) {
  const char *name;
  if (String_Parse(args, &name)) {
    Menu_ShowItemByName(item->parent, name, qfalse);
  }
}

void Script_FadeIn(itemDef_t *item, const char **args) {
  const char *name;
  if (String_Parse(args, &name)) {
    Menu_FadeItemByName(item->parent, name, qfalse);
  }
}

void Script_FadeOut(itemDef_t *item, const char **args) {
  const char *name;
  if (String_Parse(args, &name)) {
    Menu_FadeItemByName(item->parent, name, qtrue);
  }
}



void Script_Open(itemDef_t *item, const char **args) {
  const char *name;
  if (String_Parse(args, &name)) {
    Menus_OpenByName(name);
  }
}

void Script_Close(itemDef_t *item, const char **args) {
  const char *name;
  if (String_Parse(args, &name)) {
    Menus_CloseByName(name);
  }
}

void Menu_TransitionItemByName(menuDef_t *menu, const char *p, rectDef_t rectFrom, rectDef_t rectTo, int time, float amt) {
  itemDef_t *item;
  int i;
  int count = Menu_ItemsMatchingGroup(menu, p);
  for (i = 0; i < count; i++) {
    item = Menu_GetMatchingItemByNumber(menu, i, p);
    if (item != NULL) {
      item->window.flags |= (WINDOW_INTRANSITION | WINDOW_VISIBLE);
      item->window.offsetTime = time;
			memcpy(&item->window.rectClient, &rectFrom, sizeof(rectDef_t));
			memcpy(&item->window.rectEffects, &rectTo, sizeof(rectDef_t));
			item->window.rectEffects2.x = abs(rectTo.x - rectFrom.x) / amt;
			item->window.rectEffects2.y = abs(rectTo.y - rectFrom.y) / amt;
			item->window.rectEffects2.w = abs(rectTo.w - rectFrom.w) / amt;
			item->window.rectEffects2.h = abs(rectTo.h - rectFrom.h) / amt;
      Item_UpdatePosition(item);
    }
  }
}


void Script_Transition(itemDef_t *item, const char **args) {
  const char *name;
	rectDef_t rectFrom, rectTo;
  int time;
	float amt;

  if (String_Parse(args, &name)) {
    if ( Rect_Parse(args, &rectFrom) && Rect_Parse(args, &rectTo) && Int_Parse(args, &time) && Float_Parse(args, &amt)) {
      Menu_TransitionItemByName(item->parent, name, rectFrom, rectTo, time, amt);
    }
  }
}


void Menu_OrbitItemByName(menuDef_t *menu, const char *p, float x, float y, float cx, float cy, int time) {
  itemDef_t *item;
  int i;
  int count = Menu_ItemsMatchingGroup(menu, p);
  for (i = 0; i < count; i++) {
    item = Menu_GetMatchingItemByNumber(menu, i, p);
    if (item != NULL) {
      item->window.flags |= (WINDOW_ORBITING | WINDOW_VISIBLE);
      item->window.offsetTime = time;
      item->window.rectEffects.x = cx;
      item->window.rectEffects.y = cy;
      item->window.rectClient.x = x;
      item->window.rectClient.y = y;
      Item_UpdatePosition(item);
    }
  }
}


void Script_Orbit(itemDef_t *item, const char **args) {
  const char *name;
  float cx, cy, x, y;
  int time;

  if (String_Parse(args, &name)) {
    if ( Float_Parse(args, &x) && Float_Parse(args, &y) && Float_Parse(args, &cx) && Float_Parse(args, &cy) && Int_Parse(args, &time) ) {
      Menu_OrbitItemByName(item->parent, name, x, y, cx, cy, time);
    }
  }
}



void Script_SetFocus(itemDef_t *item, const char **args) 
{
	const char	*name;
	itemDef_t	*focusItem;

	if (String_Parse(args, &name)) 
	{
		focusItem = Menu_FindItemByName(item->parent, name);
		if (focusItem && !(focusItem->window.flags & WINDOW_DECORATION) && !(focusItem->window.flags & WINDOW_HASFOCUS)) 
		{
			Menu_ClearFocus(item->parent);
			focusItem->window.flags |= WINDOW_HASFOCUS;

			if ( focusItem->type == ITEM_TYPE_EDITFIELD ||
				 focusItem->type == ITEM_TYPE_PASSWORDFIELD )
			{
				focusItem->cursorPos = 0;
				g_editingField = qtrue;
				g_editItem = focusItem;
				DC->setOverstrikeMode(qtrue);
			}

			if (focusItem->onFocus) 
			{
				Item_RunScript(focusItem, focusItem->onFocus);
			}
			
			if (DC->Assets.itemFocusSound) 
			{
				DC->startLocalSound( DC->Assets.itemFocusSound, CHAN_LOCAL_SOUND );
			}
		}
	}
}

void Script_SetPlayerModel(itemDef_t *item, const char **args) {
  const char *name;
  if (String_Parse(args, &name)) {
    DC->setCVar("team_model", name);
  }
}

void Script_SetPlayerSkin(itemDef_t *item, const char **args) {
  const char *name;
  if (String_Parse(args, &name)) {
    DC->setCVar("team_skin", name);
  }
}

void Script_SetCvar(itemDef_t *item, const char **args) {
	const char *cvar, *val;
	if (String_Parse(args, &cvar) && String_Parse(args, &val)) {
		DC->setCVar(cvar, val);
	}
	
}

void Script_Exec(itemDef_t *item, const char **args) {
	const char *val;
	if (String_Parse(args, &val)) {
		DC->executeText(EXEC_APPEND, va("%s ; ", val));
	}
}

void Script_Play(itemDef_t *item, const char **args) {
	const char *val;
	if (String_Parse(args, &val)) {
		DC->startLocalSound(DC->registerSound(val), CHAN_LOCAL_SOUND);
	}
}

void Script_playLooped(itemDef_t *item, const char **args) {
	const char *val;
	if (String_Parse(args, &val)) {
		DC->stopBackgroundTrack();
		DC->startBackgroundTrack(val, val, qfalse);
	}
}


commandDef_t commandList[] =
{
  {"fadein", &Script_FadeIn},                   // group/name
  {"fadeout", &Script_FadeOut},                 // group/name
  {"show", &Script_Show},                       // group/name
  {"hide", &Script_Hide},                       // group/name
  {"setcolor", &Script_SetColor},               // works on this
  {"open", &Script_Open},                       // nenu
  {"close", &Script_Close},                     // menu
  {"setmenubackground", &Script_SetMenuBackground },	//menu
  {"setasset", &Script_SetAsset},               // works on this
  {"setbackground", &Script_SetBackground},     // works on this
  {"setitemcolor", &Script_SetItemColor},       // group/name
  {"setteamcolor", &Script_SetTeamColor},       // sets this background color to team color
  {"setfocus", &Script_SetFocus},               // sets this background color to team color
  {"setplayermodel", &Script_SetPlayerModel},   // sets this background color to team color
  {"setplayerskin", &Script_SetPlayerSkin},     // sets this background color to team color
  {"transition", &Script_Transition},           // group/name
  {"setcvar", &Script_SetCvar},					// group/name
  {"exec", &Script_Exec},						// group/name
  {"play", &Script_Play},						// group/name
  {"playlooped", &Script_playLooped},           // group/name
  {"orbit", &Script_Orbit}                      // group/name
};

int scriptCommandCount = sizeof(commandList) / sizeof(commandDef_t);


void Item_RunScript(itemDef_t *item, const char *s) {
  char script[1024];
  const char *p;
  int i;
  qboolean bRan;
  memset(script, 0, sizeof(script));
  if (item && s && s[0]) {
    Q_strcat(script, 1024, s);
    p = script;
    while (1) {
      const char *command;
      // expect command then arguments, ; ends command, NULL ends script
      if (!String_Parse(&p, &command)) {
        return;
      }

      if (command[0] == ';' && command[1] == '\0') {
        continue;
      }

      bRan = qfalse;
      for (i = 0; i < scriptCommandCount; i++) {
        if (Q_stricmp(command, commandList[i].name) == 0) {
          (commandList[i].handler(item, &p));
          bRan = qtrue;
          break;
        }
      }
      // not in our auto list, pass to handler
      if (!bRan) {
        DC->runScript(&p);
      }
    }
  }
}


qboolean Item_EnableShowViaCvar(itemDef_t *item, int flag) 
{
	char		script[1024];
	const char	*p;
  
	memset(script, 0, sizeof(script));

	if (item && item->enableCvar && *item->enableCvar && item->cvarTest && *item->cvarTest) 
	{
		char buff[1024];
	
		DC->getCVarString(item->cvarTest, buff, sizeof(buff));

		Q_strcat(script, 1024, item->enableCvar);
		
		p = script;
		
		while (1) 
		{
			const char *val;
      
			// expect value then ; or NULL, NULL ends list
			if (!String_Parse(&p, &val)) 
			{
				return (item->cvarFlags & flag) ? qfalse : qtrue;
			}

			if (val[0] == ';' && val[1] == '\0') 
			{
				continue;
			}

			// enable it if any of the values are true
			if (item->cvarFlags & flag) 
			{
				if (Q_stricmp(buff, val) == 0) 
				{
					return qtrue;
				}
			} 
			else 
			{
				// disable it if any of the values are true
				if (Q_stricmp(buff, val) == 0) 
				{
					return qfalse;
				}
			}
		}

		return (item->cvarFlags & flag) ? qfalse : qtrue;
	}
	
	return qtrue;
}


// will optionaly set focus to this item 
qboolean Item_SetFocus(itemDef_t *item, float x, float y) 
{
	int i;
	itemDef_t *oldFocus;
	sfxHandle_t *sfx = &DC->Assets.itemFocusSound;
	qboolean playSound = qfalse;
	menuDef_t *parent; // bk001206: = (menuDef_t*)item->parent;

	// sanity check, non-null, not a decoration and does not already have the focus
	if (item == NULL || item->window.flags & WINDOW_DECORATION || item->window.flags & WINDOW_HASFOCUS || !(item->window.flags & WINDOW_VISIBLE) || (item->window.flags & WINDOW_DISABLED)) 
	{
		return qfalse;
	}

	// bk001206 - this can be NULL.
	parent = (menuDef_t*)item->parent; 
      
	// items can be enabled and disabled based on cvars
	if (item->cvarFlags & (CVAR_ENABLE | CVAR_DISABLE) && !Item_EnableShowViaCvar(item, CVAR_ENABLE)) {
		return qfalse;
	}

	if (item->cvarFlags & (CVAR_SHOW | CVAR_HIDE) && !Item_EnableShowViaCvar(item, CVAR_SHOW)) {
		return qfalse;
	}

	oldFocus = Menu_ClearFocus(item->parent);

	if (item->type == ITEM_TYPE_TEXT) {
		rectDef_t r;
		r = item->textRect;
		r.y -= r.h;
		if (Rect_ContainsPoint(&r, x, y)) {
			item->window.flags |= WINDOW_HASFOCUS;
			if (item->focusSound) {
				sfx = &item->focusSound;
			}
			playSound = qtrue;
		} else {
			if (oldFocus) {
				oldFocus->window.flags |= WINDOW_HASFOCUS;
				if (oldFocus->onFocus) {
					Item_RunScript(oldFocus, oldFocus->onFocus);
				}
			}
		}
	} else {
	    item->window.flags |= WINDOW_HASFOCUS;
		if (item->onFocus) {
			Item_RunScript(item, item->onFocus);
		}
		if (item->focusSound) {
			sfx = &item->focusSound;
		}
		playSound = qtrue;
	}

	if (playSound && sfx) {
		DC->startLocalSound( *sfx, CHAN_AUTO );
	}

	for (i = 0; i < parent->itemCount; i++) {
		if (parent->items[i] == item) {
			parent->cursorItem = i;
			break;
		}
	}

	return qtrue;
}

int Item_TextScroll_MaxScroll ( itemDef_t *item ) 
{
	textScrollDef_t *scrollPtr = (textScrollDef_t*)item->typeData;
	
	int count = scrollPtr->lineCount;
	int max   = count - (int)(item->window.rect.h / scrollPtr->lineHeight) + 1;

	if (max < 0) 
	{
		return 0;
	}

	return max;
}

int Item_TextScroll_ThumbPosition ( itemDef_t *item ) 
{
	float max, pos, size;
	textScrollDef_t *scrollPtr = (textScrollDef_t*)item->typeData;

	max  = Item_TextScroll_MaxScroll ( item );
	size = item->window.rect.h - (SCROLLBAR_SIZE * 2) - 2;

	if (max > 0) 
	{
		pos = (size-SCROLLBAR_SIZE) / (float) max;
	} 
	else 
	{
		pos = 0;
	}
	
	pos *= scrollPtr->startPos;
	
	return item->window.rect.y + 1 + SCROLLBAR_SIZE + pos;
}

int Item_TextScroll_ThumbDrawPosition ( itemDef_t *item ) 
{
	int min, max;

	if (itemCapture == item) 
	{
		min = item->window.rect.y + SCROLLBAR_SIZE + 1;
		max = item->window.rect.y + item->window.rect.h - 2*SCROLLBAR_SIZE - 1;

		if (DC->cursory >= min + SCROLLBAR_SIZE/2 && DC->cursory <= max + SCROLLBAR_SIZE/2) 
		{
			return DC->cursory - SCROLLBAR_SIZE/2;
		}

		return Item_TextScroll_ThumbPosition(item);
	}

	return Item_TextScroll_ThumbPosition(item);
}

int Item_TextScroll_OverLB ( itemDef_t *item, float x, float y ) 
{
	rectDef_t		r;
	textScrollDef_t *scrollPtr;
	int				thumbstart;
	int				count;

	scrollPtr = (textScrollDef_t*)item->typeData;
	count     = scrollPtr->lineCount;

	r.x = item->window.rect.x + item->window.rect.w - SCROLLBAR_SIZE;
	r.y = item->window.rect.y;
	r.h = r.w = SCROLLBAR_SIZE;
	if (Rect_ContainsPoint(&r, x, y)) 
	{
		return WINDOW_LB_LEFTARROW;
	}

	r.y = item->window.rect.y + item->window.rect.h - SCROLLBAR_SIZE;
	if (Rect_ContainsPoint(&r, x, y)) 
	{
		return WINDOW_LB_RIGHTARROW;
	}

	thumbstart = Item_TextScroll_ThumbPosition(item);
	r.y = thumbstart;
	if (Rect_ContainsPoint(&r, x, y)) 
	{
		return WINDOW_LB_THUMB;
	}

	r.y = item->window.rect.y + SCROLLBAR_SIZE;
	r.h = thumbstart - r.y;
	if (Rect_ContainsPoint(&r, x, y)) 
	{
		return WINDOW_LB_PGUP;
	}

	r.y = thumbstart + SCROLLBAR_SIZE;
	r.h = item->window.rect.y + item->window.rect.h - SCROLLBAR_SIZE;
	if (Rect_ContainsPoint(&r, x, y)) 
	{
		return WINDOW_LB_PGDN;
	}

	return 0;
}

void Item_TextScroll_MouseEnter (itemDef_t *item, float x, float y) 
{
	item->window.flags &= ~(WINDOW_LB_LEFTARROW | WINDOW_LB_RIGHTARROW | WINDOW_LB_THUMB | WINDOW_LB_PGUP | WINDOW_LB_PGDN);
	item->window.flags |= Item_TextScroll_OverLB(item, x, y);
}

qboolean Item_TextScroll_HandleKey ( itemDef_t *item, int key, qboolean down, qboolean force) 
{
	textScrollDef_t *scrollPtr = (textScrollDef_t*)item->typeData;
	int				max;
	int				viewmax;

	if (force || (Rect_ContainsPoint(&item->window.rect, DC->cursorx, DC->cursory) && item->window.flags & WINDOW_HASFOCUS)) 
	{
		max = Item_TextScroll_MaxScroll(item);

		viewmax = (item->window.rect.h / scrollPtr->lineHeight);
		if ( key == K_UPARROW || key == K_KP_UPARROW ) 
		{
			scrollPtr->startPos--;
			if (scrollPtr->startPos < 0)
			{
				scrollPtr->startPos = 0;
			}
			return qtrue;
		}

		if ( key == K_DOWNARROW || key == K_KP_DOWNARROW ) 
		{
			scrollPtr->startPos++;
			if (scrollPtr->startPos > max)
			{
				scrollPtr->startPos = max;
			}

			return qtrue;
		}

		// mouse hit
		if (key == K_MOUSE1 || key == K_MOUSE2) 
		{
			if (item->window.flags & WINDOW_LB_LEFTARROW) 
			{
				scrollPtr->startPos--;
				if (scrollPtr->startPos < 0) 
				{
					scrollPtr->startPos = 0;
				}
			} 
			else if (item->window.flags & WINDOW_LB_RIGHTARROW) 
			{
				// one down
				scrollPtr->startPos++;
				if (scrollPtr->startPos > max) 
				{
					scrollPtr->startPos = max;
				}
			} 
			else if (item->window.flags & WINDOW_LB_PGUP) 
			{
				// page up
				scrollPtr->startPos -= viewmax;
				if (scrollPtr->startPos < 0) 
				{
					scrollPtr->startPos = 0;
				}
			} 
			else if (item->window.flags & WINDOW_LB_PGDN) 
			{
				// page down
				scrollPtr->startPos += viewmax;
				if (scrollPtr->startPos > max) 
				{
					scrollPtr->startPos = max;
				}
			} 
			else if (item->window.flags & WINDOW_LB_THUMB) 
			{
				// Display_SetCaptureItem(item);
			} 

			return qtrue;
		}

		if ( key == K_HOME || key == K_KP_HOME) 
		{
			// home
			scrollPtr->startPos = 0;
			return qtrue;
		}
		if ( key == K_END || key == K_KP_END) 
		{
			// end
			scrollPtr->startPos = max;
			return qtrue;
		}
		if (key == K_PGUP || key == K_KP_PGUP ) 
		{
			scrollPtr->startPos -= viewmax;
			if (scrollPtr->startPos < 0) 
			{
					scrollPtr->startPos = 0;
			}

			return qtrue;
		}
		if ( key == K_PGDN || key == K_KP_PGDN ) 
		{
			scrollPtr->startPos += viewmax;
			if (scrollPtr->startPos > max) 
			{
				scrollPtr->startPos = max;
			}
			return qtrue;
		}
	}

	return qfalse;
}

int Item_ListBox_MaxScroll(itemDef_t *item) {
	listBoxDef_t *listPtr = (listBoxDef_t*)item->typeData;
	int count = DC->feederCount(item->special);
	int max;

	if (item->window.flags & WINDOW_HORIZONTAL) 
	{
		max = count - (item->window.rect.w / listPtr->elementWidth) + 1;
	}
	else 
	{
		float h;

		h = item->window.rect.h;
		if ( item->type == ITEM_TYPE_COMBOBOX  )
		{
			h -= (item->window.border + listPtr->elementHeight);
		}

		max = count - (h / listPtr->elementHeight) + 1;
	}

	if (max < 0) 
	{
		return 0;
	}

	return max;
}

int Item_ListBox_ThumbPosition(itemDef_t *item) 
{
	float		 max;
	float		 pos;
	float		 size;
	listBoxDef_t *listPtr = (listBoxDef_t*)item->typeData;

	max = Item_ListBox_MaxScroll(item);

	if (item->window.flags & WINDOW_HORIZONTAL) 
	{
		size = item->window.rect.w - (SCROLLBAR_SIZE * 2) - 2;
		if (max > 0) 
		{
			pos = (size-SCROLLBAR_SIZE) / (float) max;
		} 
		else 
		{
			pos = 0;
		}
		
		pos *= listPtr->startPos;
		
		return item->window.rect.x + 1 + SCROLLBAR_SIZE + pos;
	}
	else 
	{
		float y;

		size = item->window.rect.h - (SCROLLBAR_SIZE * 2) - 2;
	
		if (max > 0) 
		{
			pos = (size-SCROLLBAR_SIZE) / (float) max;
		} 
		else 
		{
			pos = 0;
		}
		
		y = item->window.rect.y;
		if ( item->type == ITEM_TYPE_COMBOBOX  )
		{
			y += (item->window.border + listPtr->elementHeight);
		}

		pos *= listPtr->startPos;
		return y + 1 + SCROLLBAR_SIZE + pos;
	}
}

int Item_ListBox_ThumbDrawPosition(itemDef_t *item) 
{
	int min, max;
	listBoxDef_t *listPtr = (listBoxDef_t*)item->typeData;

	if (itemCapture == item) 
	{
		if (item->window.flags & WINDOW_HORIZONTAL) 
		{
			min = item->window.rect.x + SCROLLBAR_SIZE + 1;
			max = item->window.rect.x + item->window.rect.w - 2*SCROLLBAR_SIZE - 1;
			if (DC->cursorx >= min + SCROLLBAR_SIZE/2 && DC->cursorx <= max + SCROLLBAR_SIZE/2) 
			{
				return DC->cursorx - SCROLLBAR_SIZE/2;
			}

			return Item_ListBox_ThumbPosition(item);
		}
		else 
		{
			float y;
			float h;

			y = item->window.rect.y;
			h = item->window.rect.h;
			if ( item->type == ITEM_TYPE_COMBOBOX  )
			{
				y += (item->window.border + listPtr->elementHeight);
				h -= (item->window.border + listPtr->elementHeight);
			}

			min = y + SCROLLBAR_SIZE + 1;
			max = y + h - 2 * SCROLLBAR_SIZE - 1;
			if (DC->cursory >= min + SCROLLBAR_SIZE/2 && DC->cursory <= max + SCROLLBAR_SIZE/2) 
			{
				return DC->cursory - SCROLLBAR_SIZE/2;
			}

			return Item_ListBox_ThumbPosition(item);
		}
	}

	return Item_ListBox_ThumbPosition(item);
}

float Item_Slider_ThumbPosition ( itemDef_t *item ) 
{
	float			value;
	float			range;
	float			x;
	editFieldDef_t	*editDef = item->typeData;

	if (item->text && *item->text) 
	{
		x = item->textRect.x + item->textRect.w + 8;
	} 
	else 
	{
		x = item->window.rect.x; //  + (SLIDER_THUMB_WIDTH/2);
	}

	if (editDef == NULL && item->cvar) 
	{
		return x;
	}

	value = DC->getCVarValue(item->cvar);

	if (value < editDef->minVal) 
	{
		value = editDef->minVal;
	} 
	else if (value > editDef->maxVal) 
	{
		value = editDef->maxVal;
	}

	range = editDef->maxVal - editDef->minVal;
	value -= editDef->minVal;
	value /= range;
	value *= (SLIDER_WIDTH - SLIDER_THUMB_WIDTH);
	x += value;

	return x;
}

int Item_Slider_OverSlider(itemDef_t *item, float x, float y) 
{
	rectDef_t r;

	r.x = Item_Slider_ThumbPosition(item);
	r.y = item->window.rect.y;
	r.w = SLIDER_THUMB_WIDTH;
	r.h = SLIDER_THUMB_HEIGHT;

	if (Rect_ContainsPoint(&r, x, y)) 
	{
		return WINDOW_LB_THUMB;
	}

	return 0;
}

int Item_ListBox_OverLB(itemDef_t *item, float x, float y) 
{
	rectDef_t		r;
	listBoxDef_t	*listPtr;
	int				thumbstart;
	int				count;

	count = DC->feederCount(item->special);
	listPtr = (listBoxDef_t*)item->typeData;

	if (item->window.flags & WINDOW_HORIZONTAL) {
		// check if on left arrow
		r.x = listPtr->listRect.x;
		r.y = listPtr->listRect.y + listPtr->listRect.h - SCROLLBAR_SIZE;
		r.h = r.w = SCROLLBAR_SIZE;
		if (Rect_ContainsPoint(&r, x, y)) {
			return WINDOW_LB_LEFTARROW;
		}
		// check if on right arrow
		r.x = listPtr->listRect.x + listPtr->listRect.w - SCROLLBAR_SIZE;
		if (Rect_ContainsPoint(&r, x, y)) {
			return WINDOW_LB_RIGHTARROW;
		}
		// check if on thumb
		thumbstart = Item_ListBox_ThumbPosition(item);
		r.x = thumbstart;
		if (Rect_ContainsPoint(&r, x, y)) {
			return WINDOW_LB_THUMB;
		}
		r.x = listPtr->listRect.x + SCROLLBAR_SIZE;
		r.w = thumbstart - r.x;
		if (Rect_ContainsPoint(&r, x, y)) {
			return WINDOW_LB_PGUP;
		}
		r.x = thumbstart + SCROLLBAR_SIZE;
		r.w = listPtr->listRect.x + listPtr->listRect.w - SCROLLBAR_SIZE;
		if (Rect_ContainsPoint(&r, x, y)) {
			return WINDOW_LB_PGDN;
		}
	}
	else 
	{
		if ( item->type == ITEM_TYPE_COMBOBOX )
		{
			if ( !(item->window.flags & WINDOW_DROPPED ) )
			{
				return 0;
			}
		}
			
		r.x = listPtr->listRect.x + listPtr->listRect.w - SCROLLBAR_SIZE;
		r.y = listPtr->listRect.y;
		r.h = r.w = SCROLLBAR_SIZE;
		if (Rect_ContainsPoint(&r, x, y)) {
			return WINDOW_LB_LEFTARROW;
		}
		r.y = listPtr->listRect.y + listPtr->listRect.h - SCROLLBAR_SIZE;
		if (Rect_ContainsPoint(&r, x, y)) {
			return WINDOW_LB_RIGHTARROW;
		}
		thumbstart = Item_ListBox_ThumbPosition(item);
		r.y = thumbstart;
		if (Rect_ContainsPoint(&r, x, y)) {
			return WINDOW_LB_THUMB;
		}
		r.y = listPtr->listRect.y + SCROLLBAR_SIZE;
		r.h = thumbstart - r.y;
		if (Rect_ContainsPoint(&r, x, y)) {
			return WINDOW_LB_PGUP;
		}
		r.y = thumbstart + SCROLLBAR_SIZE;
		r.h = listPtr->listRect.y + listPtr->listRect.h - SCROLLBAR_SIZE;
		if (Rect_ContainsPoint(&r, x, y)) {
			return WINDOW_LB_PGDN;
		}
	}
	return 0;
}


void Item_ListBox_MouseEnter(itemDef_t *item, float x, float y) 
{
	rectDef_t r;
	listBoxDef_t *listPtr = (listBoxDef_t*)item->typeData;
        
	item->window.flags &= ~(WINDOW_LB_LEFTARROW | WINDOW_LB_RIGHTARROW | WINDOW_LB_THUMB | WINDOW_LB_PGUP | WINDOW_LB_PGDN);
	item->window.flags |= Item_ListBox_OverLB(item, x, y);

	if (item->window.flags & WINDOW_HORIZONTAL) 
	{
		if (!(item->window.flags & (WINDOW_LB_LEFTARROW | WINDOW_LB_RIGHTARROW | WINDOW_LB_THUMB | WINDOW_LB_PGUP | WINDOW_LB_PGDN))) 
		{
			// check for selection hit as we have exausted buttons and thumb
			if (listPtr->elementStyle == LISTBOX_IMAGE) 
			{
				r.x = listPtr->listRect.x;
				r.y = listPtr->listRect.y;
				r.h = listPtr->listRect.h - SCROLLBAR_SIZE;
				r.w = listPtr->listRect.w - listPtr->drawPadding;
				if (Rect_ContainsPoint(&r, x, y)) 
				{
					listPtr->cursorPos =  (int)((x - r.x) / listPtr->elementWidth)  + listPtr->startPos;
					if (listPtr->cursorPos >= listPtr->endPos) 
					{
						listPtr->cursorPos = listPtr->endPos - 1;
					}
				}
			} 
			else 
			{
				// text hit.. 
			}
		}
	} 
	else if (!(item->window.flags & (WINDOW_LB_LEFTARROW | WINDOW_LB_RIGHTARROW | WINDOW_LB_THUMB | WINDOW_LB_PGUP | WINDOW_LB_PGDN))) 
	{
		r.x = listPtr->listRect.x;
		r.y = listPtr->listRect.y;
		r.w = listPtr->listRect.w - SCROLLBAR_SIZE;
		r.h = listPtr->listRect.h; //  - listPtr->drawPadding;
		if (Rect_ContainsPoint(&r, x, y)) 
		{			
			listPtr->cursorPos =  (int)((y - 2 - r.y) / listPtr->elementHeight)  + listPtr->startPos;
			if (listPtr->cursorPos >= listPtr->endPos) 
			{
				listPtr->cursorPos = listPtr->endPos - 1;
			}
		}
	}
}

void Item_MouseEnter(itemDef_t *item, float x, float y) 
{
	rectDef_t r;
	if (!item) 
	{
		return;
	}

	r = item->textRect;
	r.y -= r.h;
	// in the text rect?

	if ( item->window.flags & WINDOW_DISABLED )
	{
		return;
	}

	// items can be enabled and disabled based on cvars
	if (item->cvarFlags & (CVAR_ENABLE | CVAR_DISABLE) && !Item_EnableShowViaCvar(item, CVAR_ENABLE)) 
	{
		return;
	}

	if (item->cvarFlags & (CVAR_SHOW | CVAR_HIDE) && !Item_EnableShowViaCvar(item, CVAR_SHOW)) 
	{
		return;
	}

	if (Rect_ContainsPoint(&r, x, y)) 
	{
		if (!(item->window.flags & WINDOW_MOUSEOVERTEXT)) 
		{
			Item_RunScript(item, item->mouseEnterText);
			item->window.flags |= WINDOW_MOUSEOVERTEXT;
		}

		if (!(item->window.flags & WINDOW_MOUSEOVER)) 
		{
			Item_RunScript(item, item->mouseEnter);
			Item_SetMouseOver ( item, qtrue );
		}
	} 
	else 
	{
		// not in the text rect
		if (item->window.flags & WINDOW_MOUSEOVERTEXT) 
		{
			// if we were
			Item_RunScript(item, item->mouseExitText);
			item->window.flags &= ~WINDOW_MOUSEOVERTEXT;
		}
		
		if (!(item->window.flags & WINDOW_MOUSEOVER)) 
		{
			Item_RunScript(item, item->mouseEnter);
			Item_SetMouseOver ( item, qtrue );
//			item->window.flags |= WINDOW_MOUSEOVER;
		}

		switch ( item->type )
		{
			case ITEM_TYPE_COMBOBOX:
			case ITEM_TYPE_LISTBOX:
				Item_ListBox_MouseEnter(item, x, y);
				break;
			case ITEM_TYPE_TEXTSCROLL:
				Item_TextScroll_MouseEnter ( item, x, y );
				break;
		}
	}
}

void Item_MouseLeave(itemDef_t *item) 
{
	if (!item) 
	{
		return;
	}

    if (item->window.flags & WINDOW_MOUSEOVERTEXT) 
	{
		Item_RunScript(item, item->mouseExitText);
		item->window.flags &= ~WINDOW_MOUSEOVERTEXT;
    }
    
	Item_RunScript(item, item->mouseExit);
    item->window.flags &= ~(WINDOW_LB_RIGHTARROW | WINDOW_LB_LEFTARROW);
}

itemDef_t *Menu_HitTest(menuDef_t *menu, float x, float y) {
  int i;
  for (i = 0; i < menu->itemCount; i++) {
    if (Rect_ContainsPoint(&menu->items[i]->window.rect, x, y)) {
      return menu->items[i];
    }
  }
  return NULL;
}

qboolean Item_OwnerDraw_HandleKey(itemDef_t *item, int key) 
{
	if (item && DC->ownerDrawHandleKey) 
	{
		if ( key == K_MOUSE1 || key == K_MOUSE2 )
		{
			if ( !Rect_ContainsPoint(&item->window.rect, DC->cursorx, DC->cursory) ) 
			{
				return qfalse;
			}
		}

		return DC->ownerDrawHandleKey(item->window.ownerDraw, item->window.ownerDrawFlags, &item->special, key, item->window.ownerDrawParam );
	}
	
	return qfalse;
}

void Item_ListBox_FeederSelection ( itemDef_t* item, qboolean force )
{
	// Only run selection when the click on an item or hit enter
	if ( !force && item->type == ITEM_TYPE_COMBOBOX )
	{
		return;
	}

	DC->feederSelection(item->special, item->cursorPos);

	// Undrop the window
	if ( item->window.flags & WINDOW_DROPPED )
	{
		item->window.flags &= (~WINDOW_DROPPED);
		Item_UpdatePosition ( item );

		itemExclusive = NULL;

		Menu_HandleMouseMove ( (menuDef_t*)item->parent, DC->cursorx, DC->cursory );
	}
}

qboolean Item_ListBox_HandleKey(itemDef_t *item, int key, qboolean down, qboolean force) 
{
	listBoxDef_t *listPtr = (listBoxDef_t*)item->typeData;
	int			 count = DC->feederCount(item->special);
	int			 max;
	int			 viewmax;

	// Combo boxes have an extra button at the top to drop down the list
	if ( item->type == ITEM_TYPE_COMBOBOX && !itemCapture )
	{			
		// If its on the button then mark the combo box as dropped
		if ( Rect_ContainsPoint(&listPtr->comboRect, DC->cursorx, DC->cursory) )
		{
			itemExclusive = (item->window.flags&WINDOW_DROPPED)?NULL:item;
			item->window.flags ^= WINDOW_DROPPED;
			Item_UpdatePosition ( item );			
			return qtrue;
		}

		// If the combo box isnt droped then ignore the key
		if ( !(item->window.flags & WINDOW_DROPPED ) )
		{
			return qfalse;
		}
	}

	if ( force || (Rect_ContainsPoint(&listPtr->listRect, DC->cursorx, DC->cursory) && (item->window.flags & WINDOW_HASFOCUS))) 
	{
		max = Item_ListBox_MaxScroll(item);
		if (item->window.flags & WINDOW_HORIZONTAL) {
			viewmax = (listPtr->listRect.w / listPtr->elementWidth);
			if ( key == K_LEFTARROW || key == K_KP_LEFTARROW ) 
			{
				if (!listPtr->notselectable) {
					listPtr->cursorPos--;
					if (listPtr->cursorPos < 0) {
						listPtr->cursorPos = 0;
					}
					if (listPtr->cursorPos < listPtr->startPos) {
						listPtr->startPos = listPtr->cursorPos;
					}
					if (listPtr->cursorPos >= listPtr->startPos + viewmax) {
						listPtr->startPos = listPtr->cursorPos - viewmax + 1;
					}
					item->cursorPos = listPtr->cursorPos;
					Item_ListBox_FeederSelection ( item, qfalse );
				}
				else {
					listPtr->startPos--;
					if (listPtr->startPos < 0)
						listPtr->startPos = 0;
				}
				return qtrue;
			}
			if ( key == K_RIGHTARROW || key == K_KP_RIGHTARROW ) 
			{
				if (!listPtr->notselectable) {
					listPtr->cursorPos++;
					if (listPtr->cursorPos < listPtr->startPos) {
						listPtr->startPos = listPtr->cursorPos;
					}
					if (listPtr->cursorPos >= count) {
						listPtr->cursorPos = count-1;
					}
					if (listPtr->cursorPos >= listPtr->startPos + viewmax) {
						listPtr->startPos = listPtr->cursorPos - viewmax + 1;
					}
					item->cursorPos = listPtr->cursorPos;
					Item_ListBox_FeederSelection ( item, qfalse );
				}
				else {
					listPtr->startPos++;
					if (listPtr->startPos >= count)
						listPtr->startPos = count-1;
				}
				return qtrue;
			}
		}
		else {
			viewmax = (listPtr->listRect.h / listPtr->elementHeight);
			if ( key == K_UPARROW || key == K_KP_UPARROW ) 
			{
				if (!listPtr->notselectable) {
					listPtr->cursorPos--;
					if (listPtr->cursorPos < 0) {
						listPtr->cursorPos = 0;
					}
					if (listPtr->cursorPos < listPtr->startPos) {
						listPtr->startPos = listPtr->cursorPos;
					}
					if (listPtr->cursorPos >= listPtr->startPos + viewmax) {
						listPtr->startPos = listPtr->cursorPos - viewmax + 1;
					}
					item->cursorPos = listPtr->cursorPos;
					Item_ListBox_FeederSelection ( item, qfalse );
				}
				else {
					listPtr->startPos--;
					if (listPtr->startPos < 0)
						listPtr->startPos = 0;
				}
				return qtrue;
			}
			if ( key == K_DOWNARROW || key == K_KP_DOWNARROW ) 
			{
				if (!listPtr->notselectable) {
					listPtr->cursorPos++;
					if (listPtr->cursorPos < listPtr->startPos) {
						listPtr->startPos = listPtr->cursorPos;
					}
					if (listPtr->cursorPos >= count) {
						listPtr->cursorPos = count-1;
					}
					if (listPtr->cursorPos >= listPtr->startPos + viewmax) {
						listPtr->startPos = listPtr->cursorPos - viewmax + 1;
					}
					item->cursorPos = listPtr->cursorPos;
					Item_ListBox_FeederSelection ( item, qfalse );
				}
				else {
					listPtr->startPos++;
					if (listPtr->startPos > max)
						listPtr->startPos = max;
				}
				return qtrue;
			}
		}

		switch ( key )
		{
			case K_ENTER:
			case K_SPACE:
				if ( item->type == ITEM_TYPE_COMBOBOX )
				{
					if ( !(item->window.flags & WINDOW_DROPPED ) )
					{
						itemExclusive = item;
						item->window.flags |= WINDOW_DROPPED;
						Item_UpdatePosition ( item );
					}
					else
					{
						Item_ListBox_FeederSelection ( item, qtrue );
					}

					return qtrue;
				}
				break;

			case K_MOUSE1:
			case K_MOUSE2:
			{
				if (item->window.flags & WINDOW_LB_LEFTARROW) 
				{
					listPtr->startPos--;
					if (listPtr->startPos < 0) 
					{
						listPtr->startPos = 0;
					}
				} 
				else if (item->window.flags & WINDOW_LB_RIGHTARROW) 
				{
					// one down
					listPtr->startPos++;
					if (listPtr->startPos > max) {
						listPtr->startPos = max;
					}
				} 
				else if (item->window.flags & WINDOW_LB_PGUP) 
				{
					// page up
					listPtr->startPos -= viewmax;
					if (listPtr->startPos < 0) 
					{
						listPtr->startPos = 0;
					}
				} 
				else if (item->window.flags & WINDOW_LB_PGDN) 
				{
					// page down
					listPtr->startPos += viewmax;
					if (listPtr->startPos > max) 
					{
						listPtr->startPos = max;
					}
				} 
				else if (item->window.flags & WINDOW_LB_THUMB) 
				{
					// Display_SetCaptureItem(item);
				} 
				else 
				{
					// select an item
					if (DC->realTime < lastListBoxClickTime && listPtr->doubleClick) 
					{
						Item_RunScript(item, listPtr->doubleClick);
					}

					lastListBoxClickTime = DC->realTime + DOUBLE_CLICK_DELAY;

					if (item->cursorPos != listPtr->cursorPos || item->type == ITEM_TYPE_COMBOBOX ) 
					{
						item->cursorPos = listPtr->cursorPos;
						Item_ListBox_FeederSelection ( item, qtrue );
					}
				}
				return qtrue;
			}

			case K_HOME:
			case K_KP_HOME:
				// home
				listPtr->startPos = 0;
				return qtrue;
		
			case K_END:
			case K_KP_END:
				// end
				listPtr->startPos = max;
				return qtrue;

			case K_PGUP:
			case K_KP_PGUP:
				// page up
				if (!listPtr->notselectable) 
				{
					listPtr->cursorPos -= viewmax;
					if (listPtr->cursorPos < 0) 
					{
						listPtr->cursorPos = 0;
					}
					if (listPtr->cursorPos < listPtr->startPos) 
					{
						listPtr->startPos = listPtr->cursorPos;
					}
					if (listPtr->cursorPos >= listPtr->startPos + viewmax) 
					{
						listPtr->startPos = listPtr->cursorPos - viewmax + 1;
					}
					item->cursorPos = listPtr->cursorPos;
					Item_ListBox_FeederSelection ( item, qfalse );
				}
				else 
				{
					listPtr->startPos -= viewmax;
					if (listPtr->startPos < 0) 
					{
						listPtr->startPos = 0;
					}
				}
			
				return qtrue;

			case K_PGDN:
			case K_KP_PGDN:
				// page down
				if (!listPtr->notselectable) 
				{
					listPtr->cursorPos += viewmax;
					if (listPtr->cursorPos < listPtr->startPos) {
						listPtr->startPos = listPtr->cursorPos;
					}
					if (listPtr->cursorPos >= count) {
						listPtr->cursorPos = count-1;
					}
					if (listPtr->cursorPos >= listPtr->startPos + viewmax) {
						listPtr->startPos = listPtr->cursorPos - viewmax + 1;
					}
					item->cursorPos = listPtr->cursorPos;
					Item_ListBox_FeederSelection ( item, qfalse );
				}
				else {
					listPtr->startPos += viewmax;
					if (listPtr->startPos > max) {
						listPtr->startPos = max;
					}
				}
				return qtrue;	
		}
	}
	else if ( !itemCapture && item->type == ITEM_TYPE_COMBOBOX && (item->window.flags & WINDOW_DROPPED) )
	{
		if ( key == K_MOUSE1 || key == K_MOUSE2 )
		{
			item->window.flags &= ~WINDOW_DROPPED;
			Item_UpdatePosition ( item );

			itemExclusive = NULL;

			Menu_HandleMouseMove ( (menuDef_t*)item->parent, DC->cursorx, DC->cursory );

			return qtrue;
		}
	}

	return qfalse;
}

qboolean Item_YesNo_HandleKey(itemDef_t *item, int key) {

  if (Rect_ContainsPoint(&item->window.rect, DC->cursorx, DC->cursory) && item->window.flags & WINDOW_HASFOCUS && item->cvar) {
		if (key == K_MOUSE1 || key == K_ENTER || key == K_MOUSE2 || key == K_MOUSE3) {
	    DC->setCVar(item->cvar, va("%i", !DC->getCVarValue(item->cvar)));
		  return qtrue;
		}
  }

  return qfalse;

}

int Item_Multi_CountSettings(itemDef_t *item) {
	multiDef_t *multiPtr = (multiDef_t*)item->typeData;
	if (multiPtr == NULL) {
		return 0;
	}
	return multiPtr->count;
}

int Item_Multi_FindCvarByValue(itemDef_t *item) {
	char buff[1024];
	float value = 0;
	int i;
	multiDef_t *multiPtr = (multiDef_t*)item->typeData;
	if (multiPtr) {
		if (multiPtr->strDef) {
	    DC->getCVarString(item->cvar, buff, sizeof(buff));
		} else {
			value = DC->getCVarValue(item->cvar);
		}
		for (i = 0; i < multiPtr->count; i++) {
			if (multiPtr->strDef) {
				if (Q_stricmp(buff, multiPtr->cvarStr[i]) == 0) {
					return i;
				}
			} else {
 				if (multiPtr->cvarValue[i] == value) {
 					return i;
 				}
 			}
 		}
	}
	return 0;
}

const char *Item_Multi_Setting(itemDef_t *item) {
	char buff[1024];
	float value = 0;
	int i;
	multiDef_t *multiPtr = (multiDef_t*)item->typeData;
	if (multiPtr) {
		if (multiPtr->strDef) {
	    DC->getCVarString(item->cvar, buff, sizeof(buff));
		} else {
			value = DC->getCVarValue(item->cvar);
		}
		for (i = 0; i < multiPtr->count; i++) {
			if (multiPtr->strDef) {
				if (Q_stricmp(buff, multiPtr->cvarStr[i]) == 0) {
					return multiPtr->cvarList[i];
				}
			} else {
 				if (multiPtr->cvarValue[i] == value) {
					return multiPtr->cvarList[i];
 				}
 			}
 		}
	}
	return "Custom";
}

qboolean Item_Multi_HandleKey(itemDef_t *item, int key) {
	multiDef_t *multiPtr = (multiDef_t*)item->typeData;
	if (multiPtr) {
	  if (Rect_ContainsPoint(&item->window.rect, DC->cursorx, DC->cursory) && item->window.flags & WINDOW_HASFOCUS && item->cvar) {
			if (key == K_MOUSE1 || key == K_ENTER || key == K_MOUSE2 || key == K_MOUSE3) {
				int current = Item_Multi_FindCvarByValue(item) + 1;
				int max = Item_Multi_CountSettings(item);
				if ( current < 0 || current >= max ) {
					current = 0;
				}
				if (multiPtr->strDef) {
					DC->setCVar(item->cvar, multiPtr->cvarStr[current]);
				} else {
					float value = multiPtr->cvarValue[current];
					if (((float)((int) value)) == value) {
						DC->setCVar(item->cvar, va("%i", (int) value ));
					}
					else {
						DC->setCVar(item->cvar, va("%f", value ));
					}
				}
				return qtrue;
			}
		}
	}
  return qfalse;
}

qboolean Item_TextField_HandleKey(itemDef_t *item, int key) {
	char buff[1024];
	int len;
	editFieldDef_t *editPtr = (editFieldDef_t*)item->typeData;

	if (item->cvar) {

		memset(buff, 0, sizeof(buff));
		DC->getCVarString(item->cvar, buff, sizeof(buff));
		len = strlen(buff);
		if (editPtr->maxChars && len > editPtr->maxChars) {
			len = editPtr->maxChars;
		}
		if ( key & K_CHAR_FLAG ) {
			key &= ~K_CHAR_FLAG;


			if (key == 'h' - 'a' + 1 )	{	// ctrl-h is backspace
				if ( item->cursorPos > 0 ) {
					memmove( &buff[item->cursorPos - 1], &buff[item->cursorPos], len + 1 - item->cursorPos);
					item->cursorPos--;
					if (item->cursorPos < editPtr->paintOffset) {
						editPtr->paintOffset--;
					}
				}
				DC->setCVar(item->cvar, buff);
	    		return qtrue;
			}


			//
			// ignore any non printable chars
			//
			if ( key < 32 || !item->cvar) {
			    return qtrue;
		    }

			if (item->type == ITEM_TYPE_NUMERICFIELD) {
				if (key < '0' || key > '9') {
					return qfalse;
				}
			}

			if (!DC->getOverstrikeMode()) {
				if (( len == MAX_EDITFIELD - 1 ) || (editPtr->maxChars && len >= editPtr->maxChars)) {
					return qtrue;
				}
				memmove( &buff[item->cursorPos + 1], &buff[item->cursorPos], len + 1 - item->cursorPos );
			} else {
				if (editPtr->maxChars && item->cursorPos >= editPtr->maxChars) {
					return qtrue;
				}
			}

			buff[item->cursorPos] = key;

			DC->setCVar(item->cvar, buff);

			if (item->cursorPos < len + 1) {
				item->cursorPos++;
				if (editPtr->maxPaintChars && item->cursorPos > editPtr->maxPaintChars) {
					editPtr->paintOffset++;
				}
			}

		} else {

			if ( key == K_DEL || key == K_KP_DEL ) {
				if ( item->cursorPos < len ) {
					memmove( buff + item->cursorPos, buff + item->cursorPos + 1, len - item->cursorPos);
					DC->setCVar(item->cvar, buff);
				}
				return qtrue;
			}

			if ( key == K_RIGHTARROW || key == K_KP_RIGHTARROW ) 
			{
				if (editPtr->maxPaintChars && item->cursorPos >= editPtr->maxPaintChars && item->cursorPos < len) {
					item->cursorPos++;
					editPtr->paintOffset++;
					return qtrue;
				}
				if (item->cursorPos < len) {
					item->cursorPos++;
				} 
				return qtrue;
			}

			if ( key == K_LEFTARROW || key == K_KP_LEFTARROW ) 
			{
				if ( item->cursorPos > 0 ) {
					item->cursorPos--;
				}
				if (item->cursorPos < editPtr->paintOffset) {
					editPtr->paintOffset--;
				}
				return qtrue;
			}

			if ( key == K_HOME || key == K_KP_HOME) {// || ( tolower(key) == 'a' && trap_Key_IsDown( K_CTRL ) ) ) {
				item->cursorPos = 0;
				editPtr->paintOffset = 0;
				return qtrue;
			}

			if ( key == K_END || key == K_KP_END)  {// ( tolower(key) == 'e' && trap_Key_IsDown( K_CTRL ) ) ) {
				item->cursorPos = len;
				if(item->cursorPos > editPtr->maxPaintChars) {
					editPtr->paintOffset = len - editPtr->maxPaintChars;
				}
				return qtrue;
			}

			if ( key == K_INS || key == K_KP_INS ) {
				DC->setOverstrikeMode(!DC->getOverstrikeMode());
				return qtrue;
			}
		}

		if (key == K_TAB || key == K_DOWNARROW || key == K_KP_DOWNARROW) 
		{
			g_editingField = qfalse;
			g_editItem = NULL;
		}

		if (key == K_UPARROW || key == K_KP_UPARROW) 
		{
			g_editingField = qfalse;
			g_editItem = NULL;
		}

		if ( key == K_ENTER || key == K_KP_ENTER || key == K_ESCAPE)  {
			return qfalse;
		}

		return qtrue;
	}
	return qfalse;

}

static void Scroll_TextScroll_AutoFunc (void *p) 
{
	scrollInfo_t *si = (scrollInfo_t*)p;

	if (DC->realTime > si->nextScrollTime) 
	{ 
		// need to scroll which is done by simulating a click to the item
		// this is done a bit sideways as the autoscroll "knows" that the item is a listbox
		// so it calls it directly
		Item_TextScroll_HandleKey(si->item, si->scrollKey, qtrue, qfalse);
		si->nextScrollTime = DC->realTime + si->adjustValue; 
	}

	if (DC->realTime > si->nextAdjustTime) 
	{
		si->nextAdjustTime = DC->realTime + SCROLL_TIME_ADJUST;
		if (si->adjustValue > SCROLL_TIME_FLOOR) 
		{
			si->adjustValue -= SCROLL_TIME_ADJUSTOFFSET;
		}
	}
}

static void Scroll_TextScroll_ThumbFunc(void *p) 
{
	scrollInfo_t *si = (scrollInfo_t*)p;
	rectDef_t	 r;
	int			 pos;
	int			 max;

	textScrollDef_t *scrollPtr = (textScrollDef_t*)si->item->typeData;

	if (DC->cursory != si->yStart) 
	{
		r.x = si->item->window.rect.x + si->item->window.rect.w - SCROLLBAR_SIZE - 1;
		r.y = si->item->window.rect.y + SCROLLBAR_SIZE + 1;
		r.h = si->item->window.rect.h - (SCROLLBAR_SIZE*2) - 2;
		r.w = SCROLLBAR_SIZE;
		max = Item_TextScroll_MaxScroll(si->item);
		//
		pos = (DC->cursory - r.y - SCROLLBAR_SIZE/2) * max / (r.h - SCROLLBAR_SIZE);
		if (pos < 0) 
		{
			pos = 0;
		}
		else if (pos > max) 
		{
			pos = max;
		}

		scrollPtr->startPos = pos;
		si->yStart = DC->cursory;
	}

	if (DC->realTime > si->nextScrollTime) 
	{ 
		// need to scroll which is done by simulating a click to the item
		// this is done a bit sideways as the autoscroll "knows" that the item is a listbox
		// so it calls it directly
		Item_TextScroll_HandleKey(si->item, si->scrollKey, qtrue, qfalse);
		si->nextScrollTime = DC->realTime + si->adjustValue; 
	}

	if (DC->realTime > si->nextAdjustTime) 
	{
		si->nextAdjustTime = DC->realTime + SCROLL_TIME_ADJUST;
		if (si->adjustValue > SCROLL_TIME_FLOOR) 
		{
			si->adjustValue -= SCROLL_TIME_ADJUSTOFFSET;
		}
	}
}

static void Scroll_ListBox_AutoFunc(void *p) {
	scrollInfo_t *si = (scrollInfo_t*)p;
	if (DC->realTime > si->nextScrollTime) { 
		// need to scroll which is done by simulating a click to the item
		// this is done a bit sideways as the autoscroll "knows" that the item is a listbox
		// so it calls it directly
		Item_ListBox_HandleKey(si->item, si->scrollKey, qtrue, qfalse);
		si->nextScrollTime = DC->realTime + si->adjustValue; 
	}

	if (DC->realTime > si->nextAdjustTime) {
		si->nextAdjustTime = DC->realTime + SCROLL_TIME_ADJUST;
		if (si->adjustValue > SCROLL_TIME_FLOOR) {
			si->adjustValue -= SCROLL_TIME_ADJUSTOFFSET;
		}
	}
}

static void Scroll_ListBox_ThumbFunc(void *p) {
	scrollInfo_t *si = (scrollInfo_t*)p;
	rectDef_t r;
	int pos, max;

	listBoxDef_t *listPtr = (listBoxDef_t*)si->item->typeData;
	if (si->item->window.flags & WINDOW_HORIZONTAL) {
		if (DC->cursorx == si->xStart) {
			return;
		}
		r.x = si->item->window.rect.x + SCROLLBAR_SIZE + 1;
		r.y = si->item->window.rect.y + si->item->window.rect.h - SCROLLBAR_SIZE - 1;
		r.h = SCROLLBAR_SIZE;
		r.w = si->item->window.rect.w - (SCROLLBAR_SIZE*2) - 2;
		max = Item_ListBox_MaxScroll(si->item);
		//
		pos = (DC->cursorx - r.x - SCROLLBAR_SIZE/2) * max / (r.w - SCROLLBAR_SIZE);
		if (pos < 0) {
			pos = 0;
		}
		else if (pos > max) {
			pos = max;
		}
		listPtr->startPos = pos;
		si->xStart = DC->cursorx;
	}
	else if (DC->cursory != si->yStart) {

		r.x = si->item->window.rect.x + si->item->window.rect.w - SCROLLBAR_SIZE - 1;
		r.y = si->item->window.rect.y + SCROLLBAR_SIZE + 1;
		r.h = si->item->window.rect.h - (SCROLLBAR_SIZE*2) - 2;
		r.w = SCROLLBAR_SIZE;
		max = Item_ListBox_MaxScroll(si->item);
		//
		pos = (DC->cursory - r.y - SCROLLBAR_SIZE/2) * max / (r.h - SCROLLBAR_SIZE);
		if (pos < 0) {
			pos = 0;
		}
		else if (pos > max) {
			pos = max;
		}
		listPtr->startPos = pos;
		si->yStart = DC->cursory;
	}

	if (DC->realTime > si->nextScrollTime) { 
		// need to scroll which is done by simulating a click to the item
		// this is done a bit sideways as the autoscroll "knows" that the item is a listbox
		// so it calls it directly
		Item_ListBox_HandleKey(si->item, si->scrollKey, qtrue, qfalse);
		si->nextScrollTime = DC->realTime + si->adjustValue; 
	}

	if (DC->realTime > si->nextAdjustTime) {
		si->nextAdjustTime = DC->realTime + SCROLL_TIME_ADJUST;
		if (si->adjustValue > SCROLL_TIME_FLOOR) {
			si->adjustValue -= SCROLL_TIME_ADJUSTOFFSET;
		}
	}
}

static void Scroll_Slider_ThumbFunc ( void *p ) 
{
	float			x;
	float			value;
	float			cursorx;
	scrollInfo_t	*si = (scrollInfo_t*)p;
	editFieldDef_t	*editDef = si->item->typeData;

	if (si->item->text && *si->item->text) 
	{
		x = si->item->textRect.x + si->item->textRect.w + 8;
	} 
	else 
	{
		x = si->item->window.rect.x;
		x += (SLIDER_THUMB_WIDTH / 2);
	}

	cursorx = DC->cursorx;

	if (cursorx < x) 
	{
		cursorx = x;
	} 
	else if (cursorx > x + SLIDER_WIDTH - SLIDER_THUMB_WIDTH) 
	{
		cursorx = x + SLIDER_WIDTH - SLIDER_THUMB_WIDTH;
	}
	
	value = cursorx - x;
	value /= (SLIDER_WIDTH - SLIDER_THUMB_WIDTH);
	value *= (editDef->maxVal - editDef->minVal);
	value += editDef->minVal;

	DC->setCVar(si->item->cvar, va("%f", value));
}

void Item_StartCapture(itemDef_t *item, int key) {
	int flags;
	switch (item->type) {
    case ITEM_TYPE_EDITFIELD:
	case ITEM_TYPE_PASSWORDFIELD:
    case ITEM_TYPE_NUMERICFIELD:

		case ITEM_TYPE_COMBOBOX:
		case ITEM_TYPE_LISTBOX:
		{
			flags = Item_ListBox_OverLB(item, DC->cursorx, DC->cursory);
			if (flags & (WINDOW_LB_LEFTARROW | WINDOW_LB_RIGHTARROW)) {
				scrollInfo.nextScrollTime = DC->realTime + SCROLL_TIME_START;
				scrollInfo.nextAdjustTime = DC->realTime + SCROLL_TIME_ADJUST;
				scrollInfo.adjustValue = SCROLL_TIME_START;
				scrollInfo.scrollKey = key;
				scrollInfo.scrollDir = (flags & WINDOW_LB_LEFTARROW) ? qtrue : qfalse;
				scrollInfo.item = item;
				captureData = &scrollInfo;
				captureFunc = &Scroll_ListBox_AutoFunc;
				itemCapture = item;
			} else if (flags & WINDOW_LB_THUMB) {
				scrollInfo.scrollKey = key;
				scrollInfo.item = item;
				scrollInfo.xStart = DC->cursorx;
				scrollInfo.yStart = DC->cursory;
				captureData = &scrollInfo;
				captureFunc = &Scroll_ListBox_ThumbFunc;
				itemCapture = item;
			}
			break;
		}
		case ITEM_TYPE_TEXTSCROLL:
			flags = Item_TextScroll_OverLB (item, DC->cursorx, DC->cursory);
			if (flags & (WINDOW_LB_LEFTARROW | WINDOW_LB_RIGHTARROW)) 
			{
				scrollInfo.nextScrollTime = DC->realTime + SCROLL_TIME_START;
				scrollInfo.nextAdjustTime = DC->realTime + SCROLL_TIME_ADJUST;
				scrollInfo.adjustValue = SCROLL_TIME_START;
				scrollInfo.scrollKey = key;
				scrollInfo.scrollDir = (flags & WINDOW_LB_LEFTARROW) ? qtrue : qfalse;
				scrollInfo.item = item;
				captureData = &scrollInfo;
				captureFunc = &Scroll_TextScroll_AutoFunc;
				itemCapture = item;
			} 
			else if (flags & WINDOW_LB_THUMB) 
			{
				scrollInfo.scrollKey = key;
				scrollInfo.item = item;
				scrollInfo.xStart = DC->cursorx;
				scrollInfo.yStart = DC->cursory;
				captureData = &scrollInfo;
				captureFunc = &Scroll_TextScroll_ThumbFunc;
				itemCapture = item;
			}
			break;

		case ITEM_TYPE_SLIDER:
		{
			flags = Item_Slider_OverSlider(item, DC->cursorx, DC->cursory);
			if (flags & WINDOW_LB_THUMB) {
				scrollInfo.scrollKey = key;
				scrollInfo.item = item;
				scrollInfo.xStart = DC->cursorx;
				scrollInfo.yStart = DC->cursory;
				captureData = &scrollInfo;
				captureFunc = &Scroll_Slider_ThumbFunc;
				itemCapture = item;
			}
			break;
		}
	}
}

void Item_StopCapture(itemDef_t *item) {

	Item_RunScript ( item, item->action );

}

qboolean Item_Slider_HandleKey ( itemDef_t *item, int key, qboolean down )
{
	float x, value, width, work;

	if (item->window.flags & WINDOW_HASFOCUS && item->cvar && Rect_ContainsPoint(&item->window.rect, DC->cursorx, DC->cursory)) 
	{
		if (key == K_MOUSE1 || key == K_ENTER || key == K_MOUSE2 || key == K_MOUSE3) 
		{
			editFieldDef_t *editDef = item->typeData;
			if (editDef) 
			{
				width = (SLIDER_WIDTH - SLIDER_THUMB_WIDTH);
				
				if (item->text && *item->text) 
				{
					x = item->textRect.x + item->textRect.w + 8;
				} 
				else 
				{
					x = item->window.rect.x + (SLIDER_THUMB_WIDTH/2);
				}

				if ( DC->cursorx <= x )
				{
					value = editDef->minVal;
				}
				else if ( DC->cursorx >= x + width )
				{
					value = editDef->maxVal;
				}
				else
				{
					work = DC->cursorx - x + 1;
					value = work / width;
					value *= (editDef->maxVal - editDef->minVal);
					value += editDef->minVal;
				}
					
				DC->setCVar(item->cvar, va("%f", value));
				return qtrue;
			}
		}
	}

	return qfalse;
}


qboolean Item_HandleKey ( itemDef_t *item, int key, qboolean down )
{
	if (itemCapture) 
	{
		Item_StopCapture(itemCapture);
		itemCapture = NULL;
		captureFunc = NULL;
		captureData = NULL;
	} 
	else 
	{
	  // bk001206 - parentheses
		if ( down && ( key == K_MOUSE1 || key == K_MOUSE2 || key == K_MOUSE3 ) ) 
		{
			Item_StartCapture(item, key);
		}
	}

	if (!down) 
	{
		return qfalse;
	}

	switch (item->type) 
	{
		case ITEM_TYPE_BUTTON:
			if(item->hotKey && item->hotKey == key )
			{
				return qfalse;
			}
			return qfalse;
			break;

		case ITEM_TYPE_RADIOBUTTON:
			return qfalse;

		case ITEM_TYPE_CHECKBOX:
			return qfalse;

		case ITEM_TYPE_PASSWORDFIELD:
		case ITEM_TYPE_EDITFIELD:
		case ITEM_TYPE_NUMERICFIELD:
			//return Item_TextField_HandleKey(item, key);
			return qfalse;

		case ITEM_TYPE_COMBO:
			return qfalse;
		
		case ITEM_TYPE_COMBOBOX:
		case ITEM_TYPE_LISTBOX:
			return Item_ListBox_HandleKey(item, key, down, qfalse);

		case ITEM_TYPE_TEXTSCROLL:
		  return Item_TextScroll_HandleKey(item, key, down, qfalse);
		  break;

		case ITEM_TYPE_YESNO:
			return Item_YesNo_HandleKey(item, key);

		case ITEM_TYPE_MULTI:
			return Item_Multi_HandleKey(item, key);

		case ITEM_TYPE_OWNERDRAW:
			return Item_OwnerDraw_HandleKey(item, key);

		case ITEM_TYPE_BIND:
			return Item_Bind_HandleKey(item, key, down);
      
		case ITEM_TYPE_SLIDER:
			return Item_Slider_HandleKey(item, key, down);
	}

	return qfalse;
}

void Item_Action(itemDef_t *item) 
{
	if (item) 
	{
		Item_RunScript(item, item->action);
	}
}

itemDef_t *Menu_SetPrevCursorItem(menuDef_t *menu) {
  qboolean wrapped = qfalse;
	int oldCursor = menu->cursorItem;
  
  if (menu->cursorItem < 0) {
    menu->cursorItem = menu->itemCount-1;
    wrapped = qtrue;
  } 

  while (menu->cursorItem > -1) {
    
    menu->cursorItem--;
    if (menu->cursorItem < 0 && !wrapped) {
      wrapped = qtrue;
      menu->cursorItem = menu->itemCount -1;
    }

		if (Item_SetFocus(menu->items[menu->cursorItem], DC->cursorx, DC->cursory)) {
			Menu_HandleMouseMove(menu, menu->items[menu->cursorItem]->window.rect.x + 1, menu->items[menu->cursorItem]->window.rect.y + 1);
      return menu->items[menu->cursorItem];
    }
  }
	menu->cursorItem = oldCursor;
	return NULL;

}

itemDef_t *Menu_SetNextCursorItem(menuDef_t *menu) {

  qboolean wrapped = qfalse;
	int oldCursor = menu->cursorItem;


  if (menu->cursorItem == -1) {
    menu->cursorItem = 0;
    wrapped = qtrue;
  }

  while (menu->cursorItem < menu->itemCount) {

    menu->cursorItem++;
    if (menu->cursorItem >= menu->itemCount && !wrapped) {
      wrapped = qtrue;
      menu->cursorItem = 0;
    }
		if (Item_SetFocus(menu->items[menu->cursorItem], DC->cursorx, DC->cursory)) {
			Menu_HandleMouseMove(menu, menu->items[menu->cursorItem]->window.rect.x + 1, menu->items[menu->cursorItem]->window.rect.y + 1);
      return menu->items[menu->cursorItem];
    }
    
  }

	menu->cursorItem = oldCursor;
	return NULL;
}

static void Window_CloseCinematic(windowDef_t *window) {
	if (window->style == WINDOW_STYLE_CINEMATIC && window->cinematic >= 0) {
		DC->stopCinematic(window->cinematic);
		window->cinematic = -1;
	}
}

static void Menu_CloseCinematics(menuDef_t *menu) {
	if (menu) {
		int i;
		Window_CloseCinematic(&menu->window);
	  for (i = 0; i < menu->itemCount; i++) {
		  Window_CloseCinematic(&menu->items[i]->window);
			if (menu->items[i]->type == ITEM_TYPE_OWNERDRAW) {
				DC->stopCinematic(0-menu->items[i]->window.ownerDraw);
			}
	  }
	}
}

static void Display_CloseCinematics() {
	int i;
	for (i = 0; i < menuCount; i++) {
		Menu_CloseCinematics(&Menus[i]);
	}
}

void  Menus_Activate(menuDef_t *menu) 
{
	int i;

	menu->window.flags |= (WINDOW_HASFOCUS | WINDOW_VISIBLE);

	// Run the onOpen script if there is one
	if (menu->onOpen) 
	{
		itemDef_t item;
		item.parent = menu;
		Item_RunScript(&item, menu->onOpen);
	}

	if (menu->soundName && *menu->soundName) {
//		DC->stopBackgroundTrack();					// you don't want to do this since it will reset s_rawend
		DC->startBackgroundTrack(menu->soundName, menu->soundName, qfalse);
	}

	menu->appearanceTime = 0;
	Display_CloseCinematics();

	// Close any combo boxes
	for ( i = 0; i < menu->itemCount ; i ++ )
	{
		if ( menu->items[i]->window.flags & WINDOW_DROPPED )
		{
			menu->items[i]->window.flags &= (~WINDOW_DROPPED);
			Item_UpdatePosition ( menu->items[i] );
		}
	}
}

int Display_VisibleMenuCount() {
	int i, count;
	count = 0;
	for (i = 0; i < menuCount; i++) {
		if (Menus[i].window.flags & (WINDOW_FORCED | WINDOW_VISIBLE)) {
			count++;
		}
	}
	return count;
}

void Menus_HandleOOBClick(menuDef_t *menu, int key, qboolean down) {
	if (menu) {
		int i;
		// basically the behaviour we are looking for is if there are windows in the stack.. see if 
		// the cursor is within any of them.. if not close them otherwise activate them and pass the 
		// key on.. force a mouse move to activate focus and script stuff 
		if (down && menu->window.flags & WINDOW_OOB_CLICK) {
			Menu_RunCloseScript(menu);
			menu->window.flags &= ~(WINDOW_HASFOCUS | WINDOW_VISIBLE);
		}

		for (i = 0; i < menuCount; i++) {
			if (Menu_OverActiveItem(&Menus[i], DC->cursorx, DC->cursory)) {
//				Menu_RunCloseScript(menu);
//				menu->window.flags &= ~(WINDOW_HASFOCUS | WINDOW_VISIBLE);
//				menu->window.flags &= ~(WINDOW_HASFOCUS);
//				Menus_Activate(&Menus[i]);
				Menu_HandleMouseMove(&Menus[i], DC->cursorx, DC->cursory);
				Menu_HandleKey(&Menus[i], key, down);
			}
		}

		if (Display_VisibleMenuCount() == 0) {
			if (DC->Pause) {
				DC->Pause(qfalse);
			}
		}
		Display_CloseCinematics();
	}
}

static rectDef_t *Item_CorrectedTextRect(itemDef_t *item) {
	static rectDef_t rect;
	memset(&rect, 0, sizeof(rectDef_t));
	if (item) {
		rect = item->textRect;
		if (rect.w) {
			rect.y -= rect.h;
		}
	}
	return &rect;
}

qboolean Menu_HandleKey(menuDef_t *menu, int key, qboolean down) 
{
	int			i;
	itemDef_t	*item = NULL;
	qboolean	inHandler = qfalse;

	if (inHandler) 
	{
		return qfalse;
	}

	inHandler = qtrue;
	if (g_waitingForKey && down) 
	{
		qboolean handled = Item_Bind_HandleKey(g_bindItem, key, down);
		inHandler = qfalse;
		return handled;
	}

	if (g_editingField && down) 
	{
		if (!Item_TextField_HandleKey(g_editItem, key)) 
		{
			g_editingField = qfalse;
			g_editItem = NULL;
			inHandler = qfalse;
			return qfalse;
		} 
		else if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_MOUSE3) 
		{
			g_editingField = qfalse;
			g_editItem = NULL;
			Display_MouseMove(NULL, DC->cursorx, DC->cursory);
		} 
		else if (key == K_TAB || key == K_UPARROW || key == K_DOWNARROW) 
		{
			return qfalse;
		}
	}

	if (menu == NULL) 
	{
		inHandler = qfalse;
		return qfalse;
	}

		// see if the mouse is within the window bounds and if so is this a mouse click
	if (down && !(menu->window.flags & WINDOW_POPUP) && !Rect_ContainsPoint(&menu->window.rect, DC->cursorx, DC->cursory)) 
	{
		static qboolean inHandleKey = qfalse;
		// bk001206 - parentheses
		if (!inHandleKey && ( key == K_MOUSE1 || key == K_MOUSE2 || key == K_MOUSE3 ) ) 
		{
			inHandleKey = qtrue;
			Menus_HandleOOBClick(menu, key, down);
			inHandleKey = qfalse;
			inHandler = qfalse;
			return qfalse;
		}
	}

	// get the item with focus
	if ( itemExclusive )
	{
		item = itemExclusive;
	}
	else
	{
		for (i = 0; i < menu->itemCount; i++) 
		{
			if (menu->items[i]->window.flags & WINDOW_HASFOCUS) 
			{
				item = menu->items[i];
			}
		}
	}

	if (item != NULL) 
	{
		if (Item_HandleKey(item, key, down)) 
		{
			Item_Action(item);
			inHandler = qfalse;
			return qtrue;
		}
	}

	if (!down) 
	{
		inHandler = qfalse;
		return qfalse;
	}

	// default handling
	switch ( key ) {

		case K_F11:
			if (DC->getCVarValue("developer")) {
				debugMode ^= 1;
			}
			break;

		case K_F12:
			if (DC->getCVarValue("developer")) {
				DC->executeText(EXEC_APPEND, "screenshot\n");
			}
			break;
		case K_LEFTARROW:
		case K_KP_LEFTARROW:
		case K_KP_UPARROW:
		case K_UPARROW:
			if ( !g_editingField )
			{
				Menu_SetPrevCursorItem(menu);
			}
			break;

		case K_ESCAPE:
			if (!g_waitingForKey && menu->onESC) {
				itemDef_t it;
		    it.parent = menu;
		    Item_RunScript(&it, menu->onESC);
			}
			break;
		case K_TAB:
		case K_RIGHTARROW:
		case K_KP_RIGHTARROW:
		case K_KP_DOWNARROW:
		case K_DOWNARROW:
			if ( !g_editingField )
			{
				Menu_SetNextCursorItem(menu);
			}
			break;

		case K_MOUSE1:
		case K_MOUSE2:

			if (item) 
			{
				if (item->type == ITEM_TYPE_TEXT) {
					if (Rect_ContainsPoint(Item_CorrectedTextRect(item), DC->cursorx, DC->cursory)) {
						Item_Action(item);
					}
				} else if (item->type == ITEM_TYPE_EDITFIELD || item->type == ITEM_TYPE_NUMERICFIELD || item->type == ITEM_TYPE_PASSWORDFIELD) {
					if (Rect_ContainsPoint(&item->window.rect, DC->cursorx, DC->cursory)) {
						item->cursorPos = 0;
						g_editingField = qtrue;
						g_editItem = item;
						DC->setOverstrikeMode(qtrue);
					}
				} else {
					if (Rect_ContainsPoint(&item->window.rect, DC->cursorx, DC->cursory)) {
						Item_Action(item);
					}
				}
			}
			break;

		case K_JOY1:
		case K_JOY2:
		case K_JOY3:
		case K_JOY4:
		case K_AUX1:
		case K_AUX2:
		case K_AUX3:
		case K_AUX4:
		case K_AUX5:
		case K_AUX6:
		case K_AUX7:
		case K_AUX8:
		case K_AUX9:
		case K_AUX10:
		case K_AUX11:
		case K_AUX12:
		case K_AUX13:
		case K_AUX14:
		case K_AUX15:
		case K_AUX16:
			break;
		case K_KP_ENTER:
		case K_ENTER:
			if (item) {
				if (item->type == ITEM_TYPE_EDITFIELD || item->type == ITEM_TYPE_NUMERICFIELD || item->type == ITEM_TYPE_PASSWORDFIELD ) {
					item->cursorPos = 0;
					g_editingField = qtrue;
					g_editItem = item;
					DC->setOverstrikeMode(qtrue);
				} else {
						Item_Action(item);
				}
			}
			break;

		default:

			// See if a hotkey was pressed.
			for (i = 0; i < menu->itemCount; i++) 
			{
				if ( menu->items[i]->window.flags & WINDOW_DISABLED )
				{
					continue;
				}

				if ((menu->items[i]->cvarFlags & (CVAR_ENABLE | CVAR_DISABLE)) && !Item_EnableShowViaCvar(menu->items[i], CVAR_ENABLE) )
				{
					continue;
				}

				if ( !(menu->items[i]->window.flags & WINDOW_VISIBLE) )
				{
					continue;
				}

				if (menu->items[i]->hotKey == key ) 
				{
					Item_OwnerDraw_HandleKey ( menu->items[i], K_ENTER );
					Item_Action(menu->items[i]);
					inHandler = qfalse;
					return qtrue;
				}
			}
			break;

	}
	inHandler = qfalse;

	return qfalse;
}

void ToWindowCoords(float *x, float *y, windowDef_t *window) {
	if (window->border != 0) {
		*x += window->borderSize;
		*y += window->borderSize;
	} 
	*x += window->rect.x;
	*y += window->rect.y;
}

void Rect_ToWindowCoords(rectDef_t *rect, windowDef_t *window) {
	ToWindowCoords(&rect->x, &rect->y, window);
}

void Item_SetTextExtents(itemDef_t *item, int *width, int *height, const char *text) 
{
	const char *textPtr = (text) ? text : item->text;

	if (textPtr == NULL ) {
		return;
	}

	*width = item->textRect.w;
	*height = item->textRect.h;

	// keeps us from computing the widths and heights more than once
	if (*width == 0 || (item->type == ITEM_TYPE_OWNERDRAW && item->textalignment == ITEM_ALIGN_CENTER)) {
		int originalWidth = DC->getTextWidth(textPtr, item->textFont, item->textscale, 0);

		if (item->type == ITEM_TYPE_OWNERDRAW && (item->textalignment == ITEM_ALIGN_CENTER || item->textalignment == ITEM_ALIGN_RIGHT)) {
			originalWidth += DC->ownerDrawWidth(item->window.ownerDraw, item->textscale, item->textFont );
		} else if ((item->type == ITEM_TYPE_EDITFIELD || item->type == ITEM_TYPE_PASSWORDFIELD) && item->textalignment == ITEM_ALIGN_CENTER && item->cvar) {
			char buff[256];
			DC->getCVarString(item->cvar, buff, 256);
			originalWidth += DC->getTextWidth(buff, item->textFont, item->textscale, 0);
		}

		*width = DC->getTextWidth(textPtr, item->textFont, item->textscale, 0 );
		*height = DC->getTextHeight(textPtr, item->textFont, item->textscale, 0 );
		item->textRect.w = *width;
		item->textRect.h = *height;
		item->textRect.x = item->textalignx;
		item->textRect.y = item->textaligny;
		if (item->textalignment == ITEM_ALIGN_RIGHT) {
			item->textRect.x = item->textalignx - originalWidth;
		} else if (item->textalignment == ITEM_ALIGN_CENTER) {
			item->textRect.x = item->textalignx - originalWidth / 2;
		}

		ToWindowCoords(&item->textRect.x, &item->textRect.y, &item->window);
	}
}

void Item_TextColor(itemDef_t *item, vec4_t *newColor) {
	vec4_t lowLight;
	menuDef_t *parent = (menuDef_t*)item->parent;

	Fade(&item->window.flags, &item->window.foreColor[3], parent->fadeClamp, &item->window.nextTime, parent->fadeCycle, qtrue, parent->fadeAmount);

/* no blinking for now
	if (item->window.flags & WINDOW_HASFOCUS) {
		lowLight[0] = 0.8 * parent->focusColor[0]; 
		lowLight[1] = 0.8 * parent->focusColor[1]; 
		lowLight[2] = 0.8 * parent->focusColor[2]; 
		lowLight[3] = 0.8 * parent->focusColor[3]; 
		LerpColor(parent->focusColor,lowLight,*newColor,0.5+0.5*sin(DC->realTime / PULSE_DIVISOR));
	} else 
*/
	if (item->textStyle == ITEM_TEXTSTYLE_BLINK && !((DC->realTime/BLINK_DIVISOR) & 1)) {
		lowLight[0] = 0.8 * item->window.foreColor[0]; 
		lowLight[1] = 0.8 * item->window.foreColor[1]; 
		lowLight[2] = 0.8 * item->window.foreColor[2]; 
		lowLight[3] = 0.8 * item->window.foreColor[3]; 
		LerpColor(item->window.foreColor,lowLight,*newColor,0.5+0.5*sin(DC->realTime / PULSE_DIVISOR));
	} else {
		memcpy(newColor, &item->window.foreColor, sizeof(vec4_t));
		// items can be enabled and disabled based on cvars
	}

	if ( item->window.flags & WINDOW_DISABLED )
	{
		memcpy(newColor, &parent->disableColor, sizeof(vec4_t));
	}
	else if (item->enableCvar && *item->enableCvar && item->cvarTest && *item->cvarTest) 
	{
		if (item->cvarFlags & (CVAR_ENABLE | CVAR_DISABLE) && !Item_EnableShowViaCvar(item, CVAR_ENABLE)) 
		{
			memcpy(newColor, &parent->disableColor, sizeof(vec4_t));
		}
	}
}

void Item_Text_AutoWrapped_Paint(itemDef_t *item) 
{
	char		text[1024];
	char		buff[1024];
	const char*	p;
	const char*	textPtr;
	const char*	newLinePtr;
	int			width;
	int			height;
	int			len;
	int			textWidth;
	int			newLine;
	int			newLineWidth;
	float		y;
	vec4_t		color;

	if (item->text == NULL) 
	{
		if (item->cvar == NULL) 
		{
			return;
		}
		else 
		{
			DC->getCVarString(item->cvar, text, sizeof(text));
			textPtr = text;
		}
	}
	else 
	{
		textPtr = item->text;
	}

	if (*textPtr == '\0') 
	{
		return;
	}

	Item_TextColor(item, &color);
	Item_SetTextExtents(item, &width, &height, textPtr);

	y = item->textaligny;
	len = 0;
	buff[0] = '\0';
	textWidth = 0;
	newLine = 0;
	newLinePtr = NULL;
	newLineWidth = 0;
	p = textPtr;
	while (p) 
	{
		if (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\0') 
		{
			newLine = len;
			newLinePtr = p+1;
			newLineWidth = textWidth;
		}

		textWidth = DC->getTextWidth(buff, item->textFont, item->textscale, 0);
		if ( (newLine && textWidth > item->window.rect.w - item->textalignx * 2) || *p == '\n' || *p == '\0') 
		{
			if (len) 
			{
				if (item->textalignment == ITEM_ALIGN_LEFT) 
				{
					item->textRect.x = item->textalignx;
				} 
				else if (item->textalignment == ITEM_ALIGN_RIGHT) 
				{
					item->textRect.x = item->textalignx - newLineWidth;
				} 
				else if (item->textalignment == ITEM_ALIGN_CENTER) 
				{
					item->textRect.x = item->textalignx - newLineWidth / 2;
				}
				item->textRect.y = y;
				ToWindowCoords(&item->textRect.x, &item->textRect.y, &item->window);
				//
				buff[newLine] = '\0';
				DC->drawText(item->textRect.x, item->textRect.y, item->textFont, item->textscale, color, buff, 0, 0 );
			}
			
			if (*p == '\0') 
			{
				break;
			}
			//
			y += height + 5;
			p = newLinePtr;
			len = 0;
			newLine = 0;
			newLineWidth = 0;
			continue;
		}
		buff[len++] = *p++;
		buff[len] = '\0';
	}
}

void Item_Text_Wrapped_Paint(itemDef_t *item) {
	char text[1024];
	const char *p, *start, *textPtr;
	char buff[1024];
	int width, height;
	float x, y;
	vec4_t color;

	// now paint the text and/or any optional images
	// default to left

	if (item->text == NULL) {
		if (item->cvar == NULL) {
			return;
		}
		else {
			DC->getCVarString(item->cvar, text, sizeof(text));
			textPtr = text;
		}
	}
	else {
		textPtr = item->text;
	}
	if (*textPtr == '\0') {
		return;
	}

	Item_TextColor(item, &color);
	Item_SetTextExtents(item, &width, &height, textPtr);

	x = item->textRect.x;
	y = item->textRect.y;
	start = textPtr;
	p = strchr(textPtr, '\r');
	while (p && *p) {
		strncpy(buff, start, p-start+1);
		buff[p-start] = '\0';
		DC->drawText(x, y, item->textFont, item->textscale, color, buff, 0, 0 );
		y += height + 5;
		start += p - start + 1;
		p = strchr(p+1, '\r');
	}
	DC->drawText(x, y, item->textFont, item->textscale, color, start, 0, 0 );
}

void Item_Text_Paint(itemDef_t *item) {
	char text[1024];
	const char *textPtr;
	int height, width;
	vec4_t color;
	int		dFlags = 0;

	if (item->window.flags & WINDOW_WRAPPED) {
		Item_Text_Wrapped_Paint(item);
		return;
	}
	if (item->window.flags & WINDOW_AUTOWRAPPED) {
		Item_Text_AutoWrapped_Paint(item);
		return;
	}

	if (item->text == NULL || !*item->text ) 
	{
		if (item->cvar == NULL) 
		{
			return;
		}
		else 
		{
			DC->getCVarString(item->cvar, text, sizeof(text));
			textPtr = text;
			
			item->textRect.w = 0;
			item->textRect.h = 0;
		}
	}
	else 
	{
		textPtr = item->text;
	}

	// this needs to go here as it sets extents for cvar types as well
	Item_SetTextExtents(item, &width, &height, textPtr);

	if (*textPtr == '\0') {
		return;
	}


	Item_TextColor(item, &color);

	//FIXME: this is a fucking mess
/*
	adjust = 0;
	if (item->textStyle == ITEM_TEXTSTYLE_OUTLINED || item->textStyle == ITEM_TEXTSTYLE_OUTLINESHADOWED) {
		adjust = 0.5;
	}

	if (item->textStyle == ITEM_TEXTSTYLE_SHADOWED || item->textStyle == ITEM_TEXTSTYLE_OUTLINESHADOWED) {
		Fade(&item->window.flags, &DC->Assets.shadowColor[3], DC->Assets.fadeClamp, &item->window.nextTime, DC->Assets.fadeCycle, qfalse);
		DC->drawText(item->textRect.x + DC->Assets.shadowX, item->textRect.y + DC->Assets.shadowY, item->textscale, DC->Assets.shadowColor, textPtr, adjust);
	}
*/

	if (item->textStyle == ITEM_TEXTSTYLE_OUTLINED )
	{
		dFlags |= DT_OUTLINE;
	}

//	if (item->textStyle == ITEM_TEXTSTYLE_OUTLINED || item->textStyle == ITEM_TEXTSTYLE_OUTLINESHADOWED) {
//		Fade(&item->window.flags, &item->window.outlineColor[3], DC->Assets.fadeClamp, &item->window.nextTime, DC->Assets.fadeCycle, qfalse);
//		/*
//		Text_Paint(item->textRect.x-1, item->textRect.y-1, item->textscale, item->window.foreColor, textPtr, adjust);
//		Text_Paint(item->textRect.x, item->textRect.y-1, item->textscale, item->window.foreColor, textPtr, adjust);
//		Text_Paint(item->textRect.x+1, item->textRect.y-1, item->textscale, item->window.foreColor, textPtr, adjust);
//		Text_Paint(item->textRect.x-1, item->textRect.y, item->textscale, item->window.foreColor, textPtr, adjust);
//		Text_Paint(item->textRect.x+1, item->textRect.y, item->textscale, item->window.foreColor, textPtr, adjust);
//		Text_Paint(item->textRect.x-1, item->textRect.y+1, item->textscale, item->window.foreColor, textPtr, adjust);
//		Text_Paint(item->textRect.x, item->textRect.y+1, item->textscale, item->window.foreColor, textPtr, adjust);
//		Text_Paint(item->textRect.x+1, item->textRect.y+1, item->textscale, item->window.foreColor, textPtr, adjust);
//		*/
//		DC->drawText(item->textRect.x - 1, item->textRect.y + 1, item->textscale * 1.02, item->window.outlineColor, textPtr, adjust);
//	}

	DC->drawText(item->textRect.x, item->textRect.y, item->textFont, item->textscale, color, textPtr, 0, dFlags );


	if (item->text2)	// Is there a second line of text?
	{
		Item_TextColor(item, &color);
		DC->drawText(item->textRect.x + item->text2alignx, item->textRect.y + item->text2aligny, item->textFont, item->textscale, color, item->text2, 0, dFlags );
	}

}



//float			trap_Cvar_VariableValue( const char *var_name );
//void			trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );

void Item_TextField_Paint(itemDef_t *item) 
{
	char			buff[1024];
	vec4_t			newColor;
	int				offset;
	menuDef_t		*parent = (menuDef_t*)item->parent;
	editFieldDef_t	*editPtr = (editFieldDef_t*)item->typeData;

	buff[0] = '\0';

	if (item->cvar) 
	{
		DC->getCVarString(item->cvar, buff, sizeof(buff));
	} 

	if ( item->text && *item->text )
	{
		Item_Text_Paint(item);
	}
	else
	{
		item->textRect.w = 0;
		item->textRect.h = 0;
		item->textRect.x = item->window.rect.x + item->textalignx;
		item->textRect.y = item->window.rect.y + item->textaligny;
	}

	// If this is a password field then we need to render *'s instead of the real chars
	if ( item->type == ITEM_TYPE_PASSWORDFIELD )
	{
		// Fill the string with stars
		for ( offset = 0; buff[offset]; offset++ )
			buff[offset] = '*';
	}

	parent = (menuDef_t*)item->parent;

/* No more focus blinking
	if (item->window.flags & WINDOW_HASFOCUS) {
		lowLight[0] = 0.8 * parent->focusColor[0]; 
		lowLight[1] = 0.8 * parent->focusColor[1]; 
		lowLight[2] = 0.8 * parent->focusColor[2]; 
		lowLight[3] = 0.8 * parent->focusColor[3]; 
		LerpColor(parent->focusColor,lowLight,newColor,0.5+0.5*sin(DC->realTime / PULSE_DIVISOR));
	} else {
		memcpy(&newColor, &item->window.foreColor, sizeof(vec4_t));
	}
*/
	memcpy(&newColor, &item->window.foreColor, sizeof(vec4_t));

	offset = (item->text && *item->text) ? 8 : 0;

	if ((item->window.flags & WINDOW_HASFOCUS) && g_editingField) 
	{
		char cursor = DC->getOverstrikeMode() ? '_' : '|';
		DC->drawTextWithCursor(item->textRect.x + item->textRect.w + offset, item->textRect.y, item->textFont, item->textscale, newColor, buff + editPtr->paintOffset, editPtr->maxPaintChars, 0, item->cursorPos - editPtr->paintOffset, cursor );
	} 
	else 
	{
		DC->drawText(item->textRect.x + item->textRect.w + offset, item->textRect.y, item->textFont, item->textscale, newColor, buff + editPtr->paintOffset, editPtr->maxPaintChars, 0 );
	}
}

void Item_YesNo_Paint(itemDef_t *item) 
{
	vec4_t	newColor;
	float	value;

	value = (item->cvar) ? DC->getCVarValue(item->cvar) : 0;

	memcpy(&newColor, &item->window.foreColor, sizeof(vec4_t));

	if (item->text) 
	{
		Item_Text_Paint(item);
		DC->drawText(item->textRect.x + item->textRect.w + 8, item->textRect.y, item->textFont, item->textscale, newColor, (value != 0) ? "Yes" : "No", 0, 0);
	} 
	else 
	{
		DC->drawText(item->textRect.x, item->textRect.y, item->textFont, item->textscale, newColor, (value != 0) ? "Yes" : "No", 0, 0);
	}
}

void Item_Multi_Paint(itemDef_t *item) 
{
	vec4_t newColor;
	const char *text = "";

 	memcpy(&newColor, &item->window.foreColor, sizeof(vec4_t));

	text = Item_Multi_Setting(item);

	if (item->text) 
	{
		Item_Text_Paint(item);
		DC->drawText(item->textRect.x + item->textRect.w + 8, item->textRect.y, item->textFont, item->textscale, newColor, text, 0, 0);
	} 
	else 
	{
		DC->drawText(item->textRect.x, item->textRect.y, item->textFont, item->textscale, newColor, text, 0, 0 );
	}
}


typedef struct {
	char	*command;
	int		id;
	int		defaultbind1;
	int		defaultbind2;
	int		bind1;
	int		bind2;
} bind_t;

typedef struct
{
	char*	name;
	float	defaultvalue;
	float	value;	
} configcvar_t;


static bind_t g_bindings[] = 
{
	{"+scores",			 K_TAB,				-1,		-1, -1},
	{"+speed", 			 K_SHIFT,			-1,		-1,	-1},
	{"+forward", 		 K_UPARROW,			-1,		-1, -1},
	{"+back", 			 K_DOWNARROW,		-1,		-1, -1},
	{"+moveleft",		 ',',				-1,		-1, -1},
	{"+moveright", 		 '.',				-1,		-1, -1},
	{"+moveup",			 K_SPACE,			-1,		-1, -1},
	{"+movedown",		 'c',				-1,		-1, -1},
	{"+left", 			 K_LEFTARROW,		-1,		-1, -1},
	{"+right", 			 K_RIGHTARROW,		-1,		-1, -1},
	{"+strafe", 		 K_ALT,				-1,		-1, -1},
	{"+lookup", 		 K_PGDN,			-1,		-1, -1},
	{"+lookdown",	 	 K_DEL,				-1,		-1, -1},
	{"+mlook", 			 '/',				-1,		-1, -1},
	{"centerview",		 K_END,				-1,		-1, -1},
	{"weapon 1",		 '1',				-1,		-1, -1},
	{"weapon 2",		 '2',				-1,		-1, -1},
	{"weapon 3",		 '3',				-1,		-1, -1},
	{"weapon 4",		 '4',				-1,		-1, -1},
	{"weapon 5",		 '5',				-1,		-1, -1},
	{"weapon 6",		 '6',				-1,		-1, -1},
	{"weapon 7",		 '7',				-1,		-1, -1},
	{"weapon 8",		 '8',				-1,		-1, -1},
	{"weapon 9",		 '9',				-1,		-1, -1},
	{"weapon 10",		 '0',				-1,		-1, -1},
	{"weapon 11",		 -1,				-1,		-1, -1},
	{"weapon 12",		 -1,				-1,		-1, -1},
	{"weapon 13",		 -1,				-1,		-1, -1},
	{"+attack", 		 K_CTRL,			-1,		-1, -1},
	{"+altattack", 		K_MOUSE2,			-1,		-1,	-1},
	{"weapprev",		 '[',				-1,		-1, -1},
	{"weapnext", 		 ']',				-1,		-1, -1},
	{"weaplast", 		 -1,				-1,		-1, -1},
	{"messagemode",		't',				-1,		-1, -1},
	{"messagemode2",	'y',				-1,		-1, -1},
	{"messagemode3",	-1,					-1,		-1, -1},
	{"messagemode4",	-1,					-1,		-1, -1},
	{"+reload",			K_MOUSE3,			-1,		-1,	-1},
	{"+zoomin",			'=',				-1,		-1,	-1},
	{"+zoomout",		'-',				-1,		-1,	-1},
	{"+firemode",		'r',				-1,		-1,	-1},
	{"ui_objectives",	'o',				-1,		-1, -1},
	{"ui_radio",		'u',				-1,		-1,	-1},
	{"ui_outfitting",	'b',				-1,		-1, -1},
	{"ui_team",			'n',				-1,		-1, -1},
	{"+automap",		'm',				-1,		-1,	-1},
	{"+lean",			-1,					-1,		-1,	-1},
	{"+leanleft",		'q',				-1,		-1,	-1},
	{"+leanright",		'e',				-1,		-1,	-1},
	{"+autorun",		K_KP_NUMLOCK,		-1,		-1,	-1},
	{"drop",			'f',				-1,		-1, -1},
	{"+goggles",		K_ENTER,			-1,		-1,	-1},
	{"vote yes",		K_F1,				-1,		-1,	-1},
	{"vote no",			K_F2,				-1,		-1,	-1},
};


static const int g_bindCount = sizeof(g_bindings) / sizeof(bind_t);

/*
=================
Controls_GetKeyAssignment
=================
*/
static void Controls_GetKeyAssignment (char *command, int *twokeys)
{
	int		count;
	int		j;
	char	b[256];

	twokeys[0] = twokeys[1] = -1;
	count = 0;

	for ( j = 0; j < 256; j++ )
	{
		DC->getBindingBuf( j, b, 256 );
		if ( *b == 0 ) {
			continue;
		}
		if ( !Q_stricmp( b, command ) ) {
			twokeys[count] = j;
			count++;
			if (count == 2) {
				break;
			}
		}
	}
}

/*
=================
Controls_GetConfig
=================
*/
void Controls_GetConfig( void )
{
	int		i;
	int		twokeys[2];

	// iterate each command, get its numeric binding
	for (i=0; i < g_bindCount; i++)
	{

		Controls_GetKeyAssignment(g_bindings[i].command, twokeys);

		g_bindings[i].bind1 = twokeys[0];
		g_bindings[i].bind2 = twokeys[1];
	}
}

/*
=================
Controls_SetConfig
=================
*/
void Controls_SetConfig(qboolean restart)
{
	int		i;

	// iterate each command, get its numeric binding
	for (i=0; i < g_bindCount; i++)
	{

		if (g_bindings[i].bind1 != -1)
		{	
			DC->setBinding( g_bindings[i].bind1, g_bindings[i].command );

			if (g_bindings[i].bind2 != -1)
				DC->setBinding( g_bindings[i].bind2, g_bindings[i].command );
		}
	}

	//if ( s_controls.invertmouse.curvalue )
	//	DC->setCVar("m_pitch", va("%f),-fabs( DC->getCVarValue( "m_pitch" ) ) );
	//else
	//	trap_Cvar_SetValue( "m_pitch", fabs( trap_Cvar_VariableValue( "m_pitch" ) ) );

	//trap_Cvar_SetValue( "m_filter", s_controls.smoothmouse.curvalue );
	//trap_Cvar_SetValue( "cl_run", s_controls.alwaysrun.curvalue );
	//trap_Cvar_SetValue( "cg_autoswitch", s_controls.autoswitch.curvalue );
	//trap_Cvar_SetValue( "sensitivity", s_controls.sensitivity.curvalue );
	//trap_Cvar_SetValue( "in_joystick", s_controls.joyenable.curvalue );
	//trap_Cvar_SetValue( "joy_threshold", s_controls.joythreshold.curvalue );
	//trap_Cvar_SetValue( "cl_freelook", s_controls.freelook.curvalue );
	if (restart)
	{
		DC->executeText(EXEC_APPEND, "in_restart\n");
	}

}

/*
=================
Controls_SetDefaults
=================
*/
void Controls_SetDefaults( void )
{
	int	i;

	// iterate each command, set its default binding
  for (i=0; i < g_bindCount; i++)
	{
		g_bindings[i].bind1 = g_bindings[i].defaultbind1;
		g_bindings[i].bind2 = g_bindings[i].defaultbind2;
	}

	//s_controls.invertmouse.curvalue  = Controls_GetCvarDefault( "m_pitch" ) < 0;
	//s_controls.smoothmouse.curvalue  = Controls_GetCvarDefault( "m_filter" );
	//s_controls.alwaysrun.curvalue    = Controls_GetCvarDefault( "cl_run" );
	//s_controls.autoswitch.curvalue   = Controls_GetCvarDefault( "cg_autoswitch" );
	//s_controls.sensitivity.curvalue  = Controls_GetCvarDefault( "sensitivity" );
	//s_controls.joyenable.curvalue    = Controls_GetCvarDefault( "in_joystick" );
	//s_controls.joythreshold.curvalue = Controls_GetCvarDefault( "joy_threshold" );
	//s_controls.freelook.curvalue     = Controls_GetCvarDefault( "cl_freelook" );
}

int BindingIDFromName(const char *name) {
	int i;
  for (i=0; i < g_bindCount; i++)
	{
		if (Q_stricmp(name, g_bindings[i].command) == 0) {
			return i;
		}
	}
	return -1;
}

char g_nameBind1[32];
char g_nameBind2[32];

void BindingFromName(const char *cvar) {
	int	i, b1, b2;

	// iterate each command, set its default binding
	for (i=0; i < g_bindCount; i++)
	{
		if (Q_stricmp(cvar, g_bindings[i].command) == 0) {
			b1 = g_bindings[i].bind1;
			if (b1 == -1) {
				break;
			}
				DC->keynumToStringBuf( b1, g_nameBind1, 32 );
				Q_strupr(g_nameBind1);

				b2 = g_bindings[i].bind2;
				if (b2 != -1)
				{
					DC->keynumToStringBuf( b2, g_nameBind2, 32 );
					Q_strupr(g_nameBind2);
					strcat( g_nameBind1, " or " );
					strcat( g_nameBind1, g_nameBind2 );
				}
			return;
		}
	}
	strcpy(g_nameBind1, "???");
}

void Item_Slider_Paint(itemDef_t *item) 
{
	vec4_t newColor;
	float x, y, value;

	value = (item->cvar) ? DC->getCVarValue(item->cvar) : 0;

	memcpy(&newColor, &item->window.foreColor, sizeof(vec4_t));

	y = item->window.rect.y;
	if (item->text && *item->text ) 
	{
		Item_Text_Paint(item);
		x = item->textRect.x + item->textRect.w + 8;
	} 
	else 
	{
		x = item->window.rect.x;
	}

	DC->setColor(newColor);
	DC->drawHandlePic( x, y, SLIDER_WIDTH, SLIDER_HEIGHT, DC->Assets.sliderBar );

	x = Item_Slider_ThumbPosition(item);
	DC->drawHandlePic( x, y, SLIDER_THUMB_WIDTH, SLIDER_THUMB_HEIGHT, DC->Assets.sliderThumb );

	DC->setColor ( NULL );
}

void Item_Bind_Paint(itemDef_t *item) {
	vec4_t newColor, lowLight;
	float value;
	int maxChars = 0;
	menuDef_t *parent = (menuDef_t*)item->parent;
	editFieldDef_t *editPtr = (editFieldDef_t*)item->typeData;
	if (editPtr) {
		maxChars = editPtr->maxPaintChars;
	}

	value = (item->cvar) ? DC->getCVarValue(item->cvar) : 0;

	if (item->window.flags & WINDOW_HASFOCUS) {
		if (g_bindItem == item && Display_KeyBindPending ( ) ) {
			lowLight[0] = 0.8f * 1.0f;
			lowLight[1] = 0.8f * 0.0f;
			lowLight[2] = 0.8f * 0.0f;
			lowLight[3] = 0.8f * 1.0f;

			LerpColor(parent->focusColor,lowLight,newColor,0.5+0.5*sin(DC->realTime / PULSE_DIVISOR));
		} else {

			/* no more blinking
			lowLight[0] = 0.8f * parent->focusColor[0]; 
			lowLight[1] = 0.8f * parent->focusColor[1]; 
			lowLight[2] = 0.8f * parent->focusColor[2]; 
			lowLight[3] = 0.8f * parent->focusColor[3]; 
			*/

			memcpy(&newColor, &item->window.foreColor, sizeof(vec4_t));
		}
	} else {
		memcpy(&newColor, &item->window.foreColor, sizeof(vec4_t));
	}

	if (item->text) {
		Item_Text_Paint(item);
		BindingFromName(item->cvar);
		DC->drawText(item->textRect.x + item->textRect.w + 8, item->textRect.y, item->textFont, item->textscale, newColor, g_nameBind1, maxChars, 0 );
	} else {
		DC->drawText(item->textRect.x, item->textRect.y, item->textFont, item->textscale, newColor, (value != 0) ? "FIXME" : "FIXME", maxChars, 0 );
	}
}

qboolean Display_KeyBindPending() {
	return g_waitingForKey;
}

qboolean Item_Bind_HandleKey(itemDef_t *item, int key, qboolean down) {
	int			id;
	int			i;

	if (Rect_ContainsPoint(&item->window.rect, DC->cursorx, DC->cursory) && !g_waitingForKey)
	{
		if (down)
		{
			if (key == K_MOUSE1 || key == K_ENTER) 
			{
				g_waitingForKey = qtrue;
				g_bindItem = item;
				return qtrue;
			}
			else if (key == K_BACKSPACE || key == K_DEL)
			{
				id = BindingIDFromName(item->cvar);
				if (id != -1) 
				{
					if ( g_bindings[id].bind1 != -1 )
					{
						DC->setBinding ( g_bindings[id].bind1, "" );
					}
					
					if ( g_bindings[id].bind2 != -1 )
					{
						DC->setBinding ( g_bindings[id].bind2, "" );
					}

					g_bindings[id].bind1 = -1;
					g_bindings[id].bind2 = -1;
				}
				Controls_SetConfig(qfalse);
				g_waitingForKey = qfalse;
				g_bindItem = NULL;
				return qtrue;
			}
		}
		return qfalse;
	}
	else
	{
		if (!g_waitingForKey || g_bindItem == NULL) {
//			return qtrue;
			return qfalse;
		}

		if (key & K_CHAR_FLAG) {
			return qtrue;
		}

		switch (key)
		{
			case K_ESCAPE:
				g_waitingForKey = qfalse;
				return qtrue;
	
			case '`':
				return qtrue;
		}
	}

	if (key != -1)
	{

		for (i=0; i < g_bindCount; i++)
		{

			if (g_bindings[i].bind2 == key) {
				g_bindings[i].bind2 = -1;
			}

			if (g_bindings[i].bind1 == key)
			{
				g_bindings[i].bind1 = g_bindings[i].bind2;
				g_bindings[i].bind2 = -1;
			}
		}
	}


	id = BindingIDFromName(item->cvar);

	if (id != -1) {
		if (key == -1) {
			if( g_bindings[id].bind1 != -1 ) {
				DC->setBinding( g_bindings[id].bind1, "" );
				g_bindings[id].bind1 = -1;
			}
			if( g_bindings[id].bind2 != -1 ) {
				DC->setBinding( g_bindings[id].bind2, "" );
				g_bindings[id].bind2 = -1;
			}
		}
		else if (g_bindings[id].bind1 == -1) {
			g_bindings[id].bind1 = key;
		}
		else if (g_bindings[id].bind1 != key && g_bindings[id].bind2 == -1) {
			g_bindings[id].bind2 = key;
		}
		else {
			DC->setBinding( g_bindings[id].bind1, "" );
			DC->setBinding( g_bindings[id].bind2, "" );
			g_bindings[id].bind1 = key;
			g_bindings[id].bind2 = -1;
		}						
	}

	Controls_SetConfig(qfalse);	
	g_waitingForKey = qfalse;

	return qtrue;
}



void AdjustFrom640(float *x, float *y, float *w, float *h) {
	//*x = *x * DC->scale + DC->bias;
	*x *= DC->xscale;
	*y *= DC->yscale;
	*w *= DC->xscale;
	*h *= DC->yscale;
}

void Item_Model_Paint(itemDef_t *item) {
	float x, y, w, h;
	refdef_t refdef;
	refEntity_t		ent;
	vec3_t			mins, maxs, origin;
	vec3_t			angles;
	modelDef_t *modelPtr = (modelDef_t*)item->typeData;

	if (modelPtr == NULL) {
		return;
	}

	// setup the refdef
	memset( &refdef, 0, sizeof( refdef ) );
	refdef.rdflags = RDF_NOWORLDMODEL;
	AxisClear( refdef.viewaxis );
	x = item->window.rect.x+1;
	y = item->window.rect.y+1;
	w = item->window.rect.w-2;
	h = item->window.rect.h-2;

	AdjustFrom640( &x, &y, &w, &h );

	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	DC->modelBounds( item->asset, mins, maxs );

	origin[2] = -0.5 * ( mins[2] + maxs[2] );
	origin[1] = 0.5 * ( mins[1] + maxs[1] );

	// calculate distance so the model nearly fills the box
	if (qtrue) {
		float len = 0.5 * ( maxs[2] - mins[2] );		
		origin[0] = len / 0.268;	// len / tan( fov/2 )
		//origin[0] = len / tan(w/2);
	} else {
		origin[0] = item->textscale;
	}
	refdef.fov_x = (modelPtr->fov_x) ? modelPtr->fov_x : w;
	refdef.fov_y = (modelPtr->fov_y) ? modelPtr->fov_y : h;

	//refdef.fov_x = (int)((float)refdef.width / 640.0f * 90.0f);
	//xx = refdef.width / tan( refdef.fov_x / 360 * M_PI );
	//refdef.fov_y = atan2( refdef.height, xx );
	//refdef.fov_y *= ( 360 / M_PI );

	DC->clearScene();

	refdef.time = DC->realTime;

	// add the model

	memset( &ent, 0, sizeof(ent) );

	//adjust = 5.0 * sin( (float)uis.realtime / 500 );
	//adjust = 360 % (int)((float)uis.realtime / 1000);
	//VectorSet( angles, 0, 0, 1 );

	// use item storage to track
	if (modelPtr->rotationSpeed) {
		if (DC->realTime > item->window.nextTime) {
			item->window.nextTime = DC->realTime + modelPtr->rotationSpeed;
			modelPtr->angle = (int)(modelPtr->angle + 1) % 360;
		}
	}
	VectorSet( angles, 0, modelPtr->angle, 0 );
	AnglesToAxis( angles, ent.axis );

	ent.hModel = item->asset;
	VectorCopy( origin, ent.origin );
	VectorCopy( origin, ent.lightingOrigin );
	ent.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
	VectorCopy( ent.origin, ent.oldorigin );

	DC->addRefEntityToScene( &ent );
	DC->renderScene( &refdef );

}


void Item_Image_Paint(itemDef_t *item) {
	if (item == NULL) {
		return;
	}
	DC->drawHandlePic(item->window.rect.x+1, item->window.rect.y+1, item->window.rect.w-2, item->window.rect.h-2, item->asset);
}

void Item_TextScroll_Paint(itemDef_t *item) 
{
	float x, y, size, count, thumb;
	int	  i;
	qhandle_t		optionalImage;
	textScrollDef_t *scrollPtr;

	scrollPtr = (textScrollDef_t*)item->typeData;

	if (item->special && !scrollPtr->lineCount )
	{
		item->text = (char *) DC->feederItemText(item->special, 0, 0, &optionalImage);

		scrollPtr->startPos = 0;
		scrollPtr->endPos = 0;

		Item_TextScroll_BuildLines ( item );
	}

	count = scrollPtr->lineCount;

	// draw scrollbar to right side of the window
	x = item->window.rect.x + item->window.rect.w - SCROLLBAR_SIZE - 1;
	y = item->window.rect.y + 1;
	DC->drawHandlePic(x, y, SCROLLBAR_SIZE, SCROLLBAR_SIZE, DC->Assets.scrollBarArrowUp);
	y += SCROLLBAR_SIZE - 1;

	scrollPtr->endPos = scrollPtr->startPos;
	size = item->window.rect.h - (SCROLLBAR_SIZE * 2);
	DC->drawHandlePic(x, y, SCROLLBAR_SIZE, size+1, DC->Assets.scrollBar);
	y += size - 1;
	DC->drawHandlePic(x, y, SCROLLBAR_SIZE, SCROLLBAR_SIZE, DC->Assets.scrollBarArrowDown);
	
	// thumb
	thumb = Item_TextScroll_ThumbDrawPosition(item);
	if (thumb > y - SCROLLBAR_SIZE - 1) 
	{
		thumb = y - SCROLLBAR_SIZE - 1;
	}
	DC->drawHandlePic(x, thumb, SCROLLBAR_SIZE, SCROLLBAR_SIZE, DC->Assets.scrollBarThumb);

	// adjust size for item painting
	size = item->window.rect.h - 2;
	x	 = item->window.rect.x + 1;
	y	 = item->window.rect.y + 1;

	for (i = scrollPtr->startPos; i < count; i++) 
	{
		char *text;

		text = scrollPtr->lines[i];
		if (!text) 
		{
			continue;
		}

		DC->drawText( 
			x, 
			y + item->textaligny, 
			item->textFont, 
			item->textscale, 
			item->window.foreColor, 
			text, 
			0, 0 );

		size -= scrollPtr->lineHeight;
		if (size < scrollPtr->lineHeight) 
		{
			scrollPtr->drawPadding = scrollPtr->lineHeight - size;
			break;
		}

		scrollPtr->endPos++;
		y += scrollPtr->lineHeight;
	}
}

void Item_ListBox_Paint(itemDef_t *item) 
{
	float			x, y, size, count, i, thumb;
	qhandle_t		image;
	qhandle_t		optionalImage;
	listBoxDef_t	*listPtr = (listBoxDef_t*)item->typeData;
	rectDef_t		rect;
	const char		*text;

	rect = item->window.rect;

	if ( item->type == ITEM_TYPE_COMBOBOX )
	{
			// default is vertical if horizontal flag is not here
		// draw scrollbar to right side of the window
		x = listPtr->comboRect.x + listPtr->comboRect.w - listPtr->comboRect.h - 1;
		y = listPtr->comboRect.y;
		DC->drawHandlePic(x, y, listPtr->comboRect.h, listPtr->comboRect.h, DC->Assets.scrollBarArrowDown);
		y += listPtr->comboRect.h - 1;

		// Draw the current text in the combo box
		x = listPtr->comboRect.x + 4 + item->textalignx;
		text = DC->feederItemText ( item->special, item->cursorPos, 0, &optionalImage );
		DC->drawText( x, y + item->textaligny, item->textFont, 
					  item->textscale, item->window.foreColor, text, 0, 0 );

		// If the combo box isnt dropped down then enough is rendered
		if ( !(item->window.flags & WINDOW_DROPPED ) )
		{
			return;
		}

		DC->drawRect ( rect.x, rect.y + listPtr->elementHeight, rect.w, item->window.border, 1, item->window.borderColor );

		rect.y += (listPtr->elementHeight + item->window.border);
		rect.h -= (listPtr->elementHeight + item->window.border);
	}

	// the listbox is horizontal or vertical and has a fixed size scroll bar going either direction
	// elements are enumerated from the DC and either text or image handles are acquired from the DC as well
	// textscale is used to size the text, textalignx and textaligny are used to size image elements
	// there is no clipping available so only the last completely visible item is painted
	count = DC->feederCount(item->special);
	
	// default is vertical if horizontal flag is not here
	if (item->window.flags & WINDOW_HORIZONTAL) 
	{
		// draw scrollbar in bottom of the window
		// bar
		x = rect.x + 1;
		y = rect.y + rect.h - SCROLLBAR_SIZE - 1;
		DC->drawHandlePic(x, y, SCROLLBAR_SIZE, SCROLLBAR_SIZE, DC->Assets.scrollBarArrowLeft);
		x += SCROLLBAR_SIZE - 1;
		size = rect.w - (SCROLLBAR_SIZE * 2);
		DC->drawHandlePic(x, y, size+1, SCROLLBAR_SIZE, DC->Assets.scrollBarHoriz);
		x += size - 1;
		DC->drawHandlePic(x, y, SCROLLBAR_SIZE, SCROLLBAR_SIZE, DC->Assets.scrollBarArrowRight);
		// thumb
		thumb = Item_ListBox_ThumbDrawPosition(item);//Item_ListBox_ThumbPosition(item);
		if (thumb > x - SCROLLBAR_SIZE - 1) {
			thumb = x - SCROLLBAR_SIZE - 1;
		}
		DC->drawHandlePic(thumb, y, SCROLLBAR_SIZE, SCROLLBAR_SIZE, DC->Assets.scrollBarThumb);
		//
		listPtr->endPos = listPtr->startPos;
		size = rect.w - 2;
		// items
		// size contains max available space
		if (listPtr->elementStyle == LISTBOX_IMAGE) {
			// fit = 0;
			x = rect.x + 1;
			y = rect.y + 1;
			for (i = listPtr->startPos; i < count; i++) {
				// always draw at least one
				// which may overdraw the box if it is too small for the element
				image = DC->feederItemImage(item->special, i);
				if (image) {
					DC->drawHandlePic(x+1, y+1, listPtr->elementWidth - 2, listPtr->elementHeight - 2, image);
				}

				if (i == item->cursorPos) {
					DC->drawRect(x, y, listPtr->elementWidth-1, listPtr->elementHeight-1, item->window.borderSize, item->window.outlineColor);
				}

				size -= listPtr->elementWidth;
				if (size < listPtr->elementWidth) {
					listPtr->drawPadding = size; //listPtr->elementWidth - size;
					break;
				}
				x += listPtr->elementWidth;
				listPtr->endPos++;
				// fit++;
			}
		} else {
			//
		}

		// Endpos is one past the end
		listPtr->endPos++;
	} 
	else 
	{
		DC->setColor ( colorWhite );

		// draw scrollbar to right side of the window
		x = rect.x + rect.w - SCROLLBAR_SIZE - 1;
		y = rect.y + 1;
		DC->drawHandlePic(x, y, SCROLLBAR_SIZE, SCROLLBAR_SIZE, DC->Assets.scrollBarArrowUp);
		y += SCROLLBAR_SIZE - 1;

		listPtr->endPos = listPtr->startPos;
		size = rect.h - (SCROLLBAR_SIZE * 2);
		DC->drawHandlePic(x, y, SCROLLBAR_SIZE, size+1, DC->Assets.scrollBar);
		y += size - 1;
		DC->drawHandlePic(x, y, SCROLLBAR_SIZE, SCROLLBAR_SIZE, DC->Assets.scrollBarArrowDown);
		// thumb
		thumb = Item_ListBox_ThumbDrawPosition(item);//Item_ListBox_ThumbPosition(item);
		if (thumb > y - SCROLLBAR_SIZE - 1) {
			thumb = y - SCROLLBAR_SIZE - 1;
		}
		DC->drawHandlePic(x, thumb, SCROLLBAR_SIZE, SCROLLBAR_SIZE, DC->Assets.scrollBarThumb);

		// adjust size for item painting
		size = rect.h - 2;
		if (listPtr->elementStyle == LISTBOX_IMAGE) {
			// fit = 0;
			x = rect.x + 1;
			y = rect.y + 1;
			for (i = listPtr->startPos; i < count; i++) {
				// always draw at least one
				// which may overdraw the box if it is too small for the element
				image = DC->feederItemImage(item->special, i);
				if (image) {
					DC->drawHandlePic(x+1, y+1, listPtr->elementWidth - 2, listPtr->elementHeight - 2, image);
				}

				if ( item->type == ITEM_TYPE_COMBOBOX )
				{
					if ( i == listPtr->cursorPos )
					{
						DC->drawRect(x, y, listPtr->elementWidth - 1, listPtr->elementHeight - 1, item->window.borderSize, item->window.outlineColor);
					}
				}
				else if (i == item->cursorPos) 
				{
					DC->drawRect(x, y, listPtr->elementWidth - 1, listPtr->elementHeight - 1, item->window.borderSize, item->window.outlineColor);
				}

				listPtr->endPos++;
				size -= listPtr->elementWidth;
				if (size < listPtr->elementHeight) {
					listPtr->drawPadding = listPtr->elementHeight - size;
					break;
				}
				y += listPtr->elementHeight;
				// fit++;
			}
		} else {
			
			x = rect.x + 1;
			y = rect.y + 1;
			
			for (i = listPtr->startPos; i < count; i++) 
			{
				// always draw at least one
				// which may overdraw the box if it is too small for the element

				if ( item->type == ITEM_TYPE_COMBOBOX )
				{
					if ( i == listPtr->cursorPos )
					{
						DC->fillRect(x + 2, y + 2, rect.w - SCROLLBAR_SIZE - 4, listPtr->elementHeight, item->window.outlineColor);
					}
				}
				else if (i == item->cursorPos) 
				{
					DC->fillRect(x + 2, y + 2, rect.w - SCROLLBAR_SIZE - 4, listPtr->elementHeight, item->window.outlineColor);
				}

				if ( listPtr->numColumns > 0 ) 
				{
					int j;
					for (j = 0; j < listPtr->numColumns; j++) 
					{
						text = DC->feederItemText(item->special, i, j, &optionalImage);
						if (optionalImage >= 0) 
						{
							DC->drawHandlePic(x + 4 + listPtr->columnInfo[j].pos, y - 1 + listPtr->elementHeight / 2, listPtr->columnInfo[j].width, listPtr->columnInfo[j].width, optionalImage);
						} 
						else if (text) 
						{
							DC->drawText(x + 4 + listPtr->columnInfo[j].pos + item->textalignx, y + listPtr->elementHeight + item->textaligny, item->textFont, item->textscale, item->window.foreColor, text, listPtr->columnInfo[j].maxChars, 0 );
						}
					}
				} 
				else 
				{
					text = DC->feederItemText(item->special, i, 0, &optionalImage);
					if (text) 
					{
						DC->drawText(x + 4, y + listPtr->elementHeight + item->textaligny, item->textFont, item->textscale, item->window.foreColor, text, 0, 0 );
					}
				}

				listPtr->endPos++;

				size -= listPtr->elementHeight;
				if (size < listPtr->elementHeight) 
				{
					listPtr->drawPadding = listPtr->elementHeight - size;
					break;
				}
				y += listPtr->elementHeight;
				// fit++;
			}
		}
		// Endpos is one past the end
//		listPtr->endPos++;
	}
}

void Item_OwnerDraw_Paint(itemDef_t *item) 
{
	menuDef_t	*parent;
	vec4_t		color;
	vec4_t		lowLight;

	if (item == NULL) 
	{
		return;
	}
  
	parent = (menuDef_t*)item->parent;

	if (!DC->ownerDrawItem) 
	{
		return;
	}

	Fade(&item->window.flags, &item->window.foreColor[3], parent->fadeClamp, &item->window.nextTime, parent->fadeCycle, qtrue, parent->fadeAmount);
	
	memcpy(&color, &item->window.foreColor, sizeof(color));
	
	if (item->numColors > 0 && DC->getValue) 
	{
		// if the value is within one of the ranges then set color to that, otherwise leave at default
		int i;
		float f = DC->getValue(item->window.ownerDraw);
	
		for (i = 0; i < item->numColors; i++) 
		{
			if (f >= item->colorRanges[i].low && f <= item->colorRanges[i].high) 
			{
				memcpy(&color, &item->colorRanges[i].color, sizeof(color));
				break;
			}
		}
	}

	if (item->textStyle == ITEM_TEXTSTYLE_BLINK && !((DC->realTime/BLINK_DIVISOR) & 1)) 
	{
		lowLight[0] = 0.8 * item->window.foreColor[0]; 
		lowLight[1] = 0.8 * item->window.foreColor[1]; 
		lowLight[2] = 0.8 * item->window.foreColor[2]; 
		lowLight[3] = 0.8 * item->window.foreColor[3]; 
		LerpColor(item->window.foreColor,lowLight,color,0.5+0.5*sin(DC->realTime / PULSE_DIVISOR));
	}

	if ( item->window.flags & WINDOW_DISABLED )
	{
		memcpy(color, parent->disableColor, sizeof(vec4_t)); // bk001207 - FIXME: Com_Memcpy
	}
	else if (item->cvarFlags & (CVAR_ENABLE | CVAR_DISABLE) && !Item_EnableShowViaCvar(item, CVAR_ENABLE)) 
	{
		memcpy(color, parent->disableColor, sizeof(vec4_t)); // bk001207 - FIXME: Com_Memcpy
	}
	
	if (item->text) 
	{
		Item_Text_Paint(item);

		if (item->text[0]) 
		{
			// +8 is an offset kludge to properly align owner draw items that have text combined with them
			DC->ownerDrawItem(item->textRect.x + item->textRect.w + 8, item->window.rect.y, item->window.rect.w, item->window.rect.h, 0, item->textaligny, item->window.ownerDraw, item->window.ownerDrawFlags, item->alignment, item->special, item->textFont, item->textscale, color, item->window.background, item->textStyle, item->window.ownerDrawParam );
		} 
		else 
		{
			DC->ownerDrawItem(item->textRect.x + item->textRect.w, item->window.rect.y, item->window.rect.w, item->window.rect.h, 0, item->textaligny, item->window.ownerDraw, item->window.ownerDrawFlags, item->alignment, item->special, item->textFont, item->textscale, color, item->window.background, item->textStyle, item->window.ownerDrawParam );
		}
	} 
	else 
	{
		DC->ownerDrawItem(item->window.rect.x, item->window.rect.y, item->window.rect.w, item->window.rect.h, item->textalignx, item->textaligny, item->window.ownerDraw, item->window.ownerDrawFlags, item->alignment, item->special, item->textFont, item->textscale, color, item->window.background, item->textStyle, item->window.ownerDrawParam );
	}
}


void Item_Paint(itemDef_t *item) 
{
	vec4_t		red;
	menuDef_t *parent = (menuDef_t*)item->parent;
	int			xPos,textWidth;
	vec4_t		color = {1, 1, 1, 1};
	float		origWidth;

	red[0] = red[3] = 1;
	red[1] = red[2] = 0;

	if (item == NULL) 
	{
		return;
	}

	if (item->window.flags & WINDOW_MOUSEOVER)
	{
		if (item->descText)
		{
			textWidth = DC->getTextWidth (item->descText, item->textFont, (float) 0.2f, 0 );

			if (parent->descAlignment == ITEM_ALIGN_RIGHT)
			{
				xPos = parent->descX - textWidth;	// Right justify
			}
			else if (parent->descAlignment == ITEM_ALIGN_CENTER)
			{
				xPos = parent->descX - (textWidth/2);	// Center justify
			}
			else										// Left justify	
			{
				xPos = parent->descX;
			}

			Item_TextColor(item, &color);
			DC->drawText(xPos, parent->descY, item->textFont, (float) 0.2f, parent->descColor, item->descText, 0, 0 );
		}
	}

	if (item->window.flags & WINDOW_ORBITING) 
	{
		if (DC->realTime > item->window.nextTime) 
		{
			float rx, ry, a, c, s, w, h;
      
			item->window.nextTime = DC->realTime + item->window.offsetTime;
			// translate
			w = item->window.rectClient.w / 2;
			h = item->window.rectClient.h / 2;
			rx = item->window.rectClient.x + w - item->window.rectEffects.x;
			ry = item->window.rectClient.y + h - item->window.rectEffects.y;
			a = 3 * M_PI / 180;
			c = cos(a);
			s = sin(a);
			item->window.rectClient.x = (rx * c - ry * s) + item->window.rectEffects.x - w;
			item->window.rectClient.y = (rx * s + ry * c) + item->window.rectEffects.y - h;
			Item_UpdatePosition(item);
		}
	}


  if (item->window.flags & WINDOW_INTRANSITION) {
    if (DC->realTime > item->window.nextTime) {
      int done = 0;
      item->window.nextTime = DC->realTime + item->window.offsetTime;
			// transition the x,y
			if (item->window.rectClient.x == item->window.rectEffects.x) {
				done++;
			} else {
				if (item->window.rectClient.x < item->window.rectEffects.x) {
					item->window.rectClient.x += item->window.rectEffects2.x;
					if (item->window.rectClient.x > item->window.rectEffects.x) {
						item->window.rectClient.x = item->window.rectEffects.x;
						done++;
					}
				} else {
					item->window.rectClient.x -= item->window.rectEffects2.x;
					if (item->window.rectClient.x < item->window.rectEffects.x) {
						item->window.rectClient.x = item->window.rectEffects.x;
						done++;
					}
				}
			}
			if (item->window.rectClient.y == item->window.rectEffects.y) {
				done++;
			} else {
				if (item->window.rectClient.y < item->window.rectEffects.y) {
					item->window.rectClient.y += item->window.rectEffects2.y;
					if (item->window.rectClient.y > item->window.rectEffects.y) {
						item->window.rectClient.y = item->window.rectEffects.y;
						done++;
					}
				} else {
					item->window.rectClient.y -= item->window.rectEffects2.y;
					if (item->window.rectClient.y < item->window.rectEffects.y) {
						item->window.rectClient.y = item->window.rectEffects.y;
						done++;
					}
				}
			}
			if (item->window.rectClient.w == item->window.rectEffects.w) {
				done++;
			} else {
				if (item->window.rectClient.w < item->window.rectEffects.w) {
					item->window.rectClient.w += item->window.rectEffects2.w;
					if (item->window.rectClient.w > item->window.rectEffects.w) {
						item->window.rectClient.w = item->window.rectEffects.w;
						done++;
					}
				} else {
					item->window.rectClient.w -= item->window.rectEffects2.w;
					if (item->window.rectClient.w < item->window.rectEffects.w) {
						item->window.rectClient.w = item->window.rectEffects.w;
						done++;
					}
				}
			}
			if (item->window.rectClient.h == item->window.rectEffects.h) {
				done++;
			} else {
				if (item->window.rectClient.h < item->window.rectEffects.h) {
					item->window.rectClient.h += item->window.rectEffects2.h;
					if (item->window.rectClient.h > item->window.rectEffects.h) {
						item->window.rectClient.h = item->window.rectEffects.h;
						done++;
					}
				} else {
					item->window.rectClient.h -= item->window.rectEffects2.h;
					if (item->window.rectClient.h < item->window.rectEffects.h) {
						item->window.rectClient.h = item->window.rectEffects.h;
						done++;
					}
				}
			}

      Item_UpdatePosition(item);

      if (done == 4) {
        item->window.flags &= ~WINDOW_INTRANSITION;
      }

    }
  }

	if (item->window.ownerDrawFlags && DC->ownerDrawVisible) 
	{
		int owner = DC->ownerDrawVisible(item->window.ownerDrawFlags, item->window.ownerDrawParam);
		if ( owner != -1 )
		{
			if (!owner) 
			{
				item->window.flags &= ~WINDOW_VISIBLE;
			} 
			else 
			{
				item->window.flags |= WINDOW_VISIBLE;
			}
		}
	}

	if ( item->window.flags & WINDOW_VISIBLE )
	{
		if (item->window.ownerDrawFlags && DC->ownerDrawDisabled)
		{
			if (!DC->ownerDrawDisabled(item->window.ownerDrawFlags, item->window.ownerDrawParam))
			{
				item->window.flags &= ~WINDOW_DISABLED;
			}
			else
			{
				item->window.flags |= WINDOW_DISABLED;
			}
		}
	}

	if (item->cvarFlags & (CVAR_SHOW | CVAR_HIDE)) 
	{
		if (!Item_EnableShowViaCvar(item, CVAR_SHOW)) 
		{
			return;
		}
	}

	if (!(item->window.flags & WINDOW_VISIBLE)) 
	{
	    return;
	}

	origWidth = item->window.rect.w;

	// Alter the width of the window if this window is ownerwidth
	if ( item->window.ownerWidth )
	{
		float scale = DC->getValue ( item->window.ownerWidth );

		// Dont draw the window if the width is zero
		if ( scale == 0.0f )
		{
			return;
		}

		// Adjust the width by the percentage returned from getvalue
		item->window.rect.w *= scale;
	}

	// paint the rect first.. 
	Window_Paint(&item->window, parent->fadeAmount , parent->fadeClamp, parent->fadeCycle);

  if (debugMode) {
		vec4_t color;
		rectDef_t *r = Item_CorrectedTextRect(item);
    color[1] = color[3] = 1;
    color[0] = color[2] = 0;
    DC->drawRect(r->x, r->y, r->w, r->h, 1, color);
  }

  //DC->drawRect(item->window.rect.x, item->window.rect.y, item->window.rect.w, item->window.rect.h, 1, red);

	switch (item->type) 
	{
		case ITEM_TYPE_OWNERDRAW:
			Item_OwnerDraw_Paint(item);
			break;
    
		case ITEM_TYPE_TEXT:
		case ITEM_TYPE_BUTTON:
			Item_Text_Paint(item);
			break;
    
		case ITEM_TYPE_RADIOBUTTON:
			break;
			
		case ITEM_TYPE_CHECKBOX:
			break;
	
		case ITEM_TYPE_PASSWORDFIELD:
		case ITEM_TYPE_EDITFIELD:
		case ITEM_TYPE_NUMERICFIELD:
			Item_TextField_Paint(item);
			break;
    
		case ITEM_TYPE_COMBO:
			break;

		case ITEM_TYPE_COMBOBOX:
		case ITEM_TYPE_LISTBOX:
			Item_ListBox_Paint(item);
			break;

		case ITEM_TYPE_TEXTSCROLL:
			Item_TextScroll_Paint ( item );
			break;

		case ITEM_TYPE_MODEL:
			Item_Model_Paint(item);
			break;
    
		case ITEM_TYPE_YESNO:
			Item_YesNo_Paint(item);
			break;
		
		case ITEM_TYPE_MULTI:
			Item_Multi_Paint(item);
			break;
    
		case ITEM_TYPE_BIND:
			Item_Bind_Paint(item);
			break;
    
		case ITEM_TYPE_SLIDER:
			Item_Slider_Paint(item);
			break;
    
		default:
			break;
  }

	// Restore the original width of the window incase an owner width window
	// messed around with it
	item->window.rect.w = origWidth;
}

void Menu_Init(menuDef_t *menu) 
{
	memset(menu, 0, sizeof(menuDef_t));
	menu->cursorItem = -1;
	menu->fadeAmount = DC->Assets.fadeAmount;
	menu->fadeClamp = DC->Assets.fadeClamp;
	menu->fadeCycle = DC->Assets.fadeCycle;

	// Initialize the tooltip background color
	menu->tooltipBackColor[0] = 1;
	menu->tooltipBackColor[1] = 1;
	menu->tooltipBackColor[2] = 1;
	menu->tooltipBackColor[3] = 1;

	// Initialize the tooltip foreground color
	menu->tooltipForeColor[0] = 0;
	menu->tooltipForeColor[1] = 0;
	menu->tooltipForeColor[2] = 0;
	menu->tooltipForeColor[3] = 1;

	menu->tooltipScale = 1;
	menu->tooltipDelay = 500;

	Window_Init(&menu->window);
}

itemDef_t* Menu_GetItemByName ( menuDef_t* menu, const char* name )
{
	int i;

	if ( !menu )
	{
		return NULL;
	}

    for (i = 0; i < menu->itemCount; i++) 
	{
		if ( !Q_stricmp ( menu->items[i]->window.name, name ) )
		{
			return menu->items[i];
		}
    }

	return NULL;
}

itemDef_t *Menu_GetFocusedItem(menuDef_t *menu) {
  int i;
  if (menu) {
    for (i = 0; i < menu->itemCount; i++) {
      if (menu->items[i]->window.flags & WINDOW_HASFOCUS) {
        return menu->items[i];
      }
    }
  }
  return NULL;
}

menuDef_t *Menu_GetFocused() {
  int i;
  for (i = 0; i < menuCount; i++) {
    if (Menus[i].window.flags & WINDOW_HASFOCUS && Menus[i].window.flags & WINDOW_VISIBLE) {
      return &Menus[i];
    }
  }
  return NULL;
}

void Menu_ScrollFeeder(menuDef_t *menu, int feeder, qboolean down) {
	if (menu) {
		int i;
    for (i = 0; i < menu->itemCount; i++) {
			if (menu->items[i]->special == feeder) {
				Item_ListBox_HandleKey(menu->items[i], (down) ? K_DOWNARROW : K_UPARROW, qtrue, qtrue);
				return;
			}
		}
	}
}



void Menu_SetFeederSelection(menuDef_t *menu, int feeder, int index, const char *name) 
{
	if (menu == NULL) 
	{
		if (name == NULL) 
		{
			menu = Menu_GetFocused();
		} 
		else 
		{
			menu = Menus_FindByName(name);
		}
	}

	if (menu) 
	{
		int i;

		for (i = 0; i < menu->itemCount; i++) 
		{
			if (menu->items[i]->special == feeder) 
			{
				if ( index == 0) 
				{
					listBoxDef_t *listPtr = (listBoxDef_t*)menu->items[i]->typeData;
					listPtr->cursorPos = 0;
					listPtr->startPos = 0;
				}
				menu->items[i]->cursorPos = index;
				DC->feederSelection(menu->items[i]->special, menu->items[i]->cursorPos);
				return;
			}
		}
	}
}

qboolean Menus_AnyFullScreenVisible() {
  int i;
  for (i = 0; i < menuCount; i++) {
    if (Menus[i].window.flags & WINDOW_VISIBLE && Menus[i].fullScreen) {
			return qtrue;
    }
  }
  return qfalse;
}

menuDef_t *Menus_ActivateByName(const char *p) 
{
	int		  i;
	menuDef_t *m = NULL;
	menuDef_t *focus = Menu_GetFocused();

	// If we find a match for the window then save the pointer, otherwise
	// clear the focus of the window since the window we are looking 
	// for will get the focus eventually
	for (i = 0; i < menuCount; i++) 
	{
		if (Q_stricmp(Menus[i].window.name, p) == 0) 
		{
			m = &Menus[i];
		}
		else 
		{
			Menus[i].window.flags &= ~WINDOW_HASFOCUS;
		}
	}

	// make sure the menu was found in the list
	if ( !m )
	{
		return NULL;
	}

	// Adjust the menu stack if there was a window with focus already
	if (openMenuCount < MAX_OPEN_MENUS && focus != NULL ) 
	{
		int j;

		// make sure the menu with focus doenst get in there twice
		for ( j = 0; j < openMenuCount; j ++ )
		{
			if ( menuStack[j] == focus )
			{
				break;
			}
		}

		// If the window isnt already in there somewhere then
		// add it to the end
		if ( j >= openMenuCount )
		{
			menuStack[openMenuCount++] = focus;
		}
	}
	
	// Activate the menu
	Menus_Activate(m);
		
	Display_CloseCinematics();
	
	return m;
}


void Item_Init(itemDef_t *item) {
	memset(item, 0, sizeof(itemDef_t));
	item->textscale = 1;
	Window_Init(&item->window);
}

void Menu_HandleMouseMove ( menuDef_t *menu, float x, float y ) 
{
	int			i;
	int			pass;
	qboolean	focusSet = qfalse;

	itemDef_t *overItem;
	if (menu == NULL) 
	{
		return;
	}

	if (!(menu->window.flags & (WINDOW_VISIBLE | WINDOW_FORCED))) 
	{
		return;
	}

	if (itemCapture) 
	{
		//Item_MouseMove(itemCapture, x, y);
		return;
	}

	if (g_waitingForKey || g_editingField) 
	{
		return;
	}

	// FIXME: this is the whole issue of focus vs. mouse over.. 
	// need a better overall solution as i don't like going through everything twice
	for (pass = 0; pass < 2; pass++) 
	{
		for (i = menu->itemCount - 1; i >= 0 ; i-- ) 
		{
			// turn off focus each item
			// menu->items[i].window.flags &= ~WINDOW_HASFOCUS;

			if (!(menu->items[i]->window.flags & (WINDOW_VISIBLE | WINDOW_FORCED))) 
			{
				continue;
			}

			if ( menu->items[i]->window.flags & WINDOW_DISABLED )
			{
				continue;
			}

			// items can be enabled and disabled based on cvars
			if (menu->items[i]->cvarFlags & (CVAR_ENABLE | CVAR_DISABLE) && !Item_EnableShowViaCvar(menu->items[i], CVAR_ENABLE)) {
				continue;
			}

			if (menu->items[i]->cvarFlags & (CVAR_SHOW | CVAR_HIDE) && !Item_EnableShowViaCvar(menu->items[i], CVAR_SHOW)) {
				continue;
			}

			if ( (!itemExclusive || itemExclusive == menu->items[i]) && Rect_ContainsPoint(&menu->items[i]->window.rect, x, y)) 
			{
				if (pass == 1) 
				{
					overItem = menu->items[i];
					if (overItem->type == ITEM_TYPE_TEXT && overItem->text) 
					{
						if (!Rect_ContainsPoint(Item_CorrectedTextRect(overItem), x, y)) 
						{
							continue;
						}
					}

					// if we are over an item
					if (IsVisible(overItem->window.flags) && !(overItem->window.flags&WINDOW_DECORATION) ) 
					{
						// different one
						Item_MouseEnter(overItem, x, y);
						// Item_SetMouseOver(overItem, qtrue);

						// if item is not a decoration see if it can take focus
						if (!focusSet) 
						{
							focusSet = Item_SetFocus(overItem, x, y);

							// Only need one focus item
							i = -1;
							continue;
						}
					}
				}
				else 
				{
					if ( !(menu->items[i]->window.flags & WINDOW_MOUSEOVER) ) 
					{
						Item_MouseEnter(menu->items[i], x, y);
					}
				}
			} 
			else if (menu->items[i]->window.flags & WINDOW_MOUSEOVER) 
			{
				Item_MouseLeave(menu->items[i]);
				Item_SetMouseOver(menu->items[i], qfalse);
			}
		}
	}
}

void Menu_Paint(menuDef_t *menu, qboolean forcePaint) 
{
	int i;

	if (menu == NULL) {
		return;
	}

	if (!(menu->window.flags & WINDOW_VISIBLE) &&  !forcePaint) {
		return;
	}

/*
	if (menu->window.ownerDrawFlags && DC->ownerDrawVisible && !DC->ownerDrawVisible(menu->window.ownerDrawFlags, menu->window.ownerDrawParam)) 
	{
		return;
	}
*/
	
	if (forcePaint) {
		menu->window.flags |= WINDOW_FORCED;
	}

	// draw the background if necessary
	if (menu->fullScreen) {
		// implies a background shader
		// FIXME: make sure we have a default shader if fullscreen is set with no background
		DC->drawHandlePic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, menu->window.background );
	} else if (menu->window.background) {
		// this allows a background shader without being full screen
		//UI_DrawHandlePic(menu->window.rect.x, menu->window.rect.y, menu->window.rect.w, menu->window.rect.h, menu->backgroundShader);
	}

	// paint the background and or border
	Window_Paint(&menu->window, menu->fadeAmount, menu->fadeClamp, menu->fadeCycle );

	// Loop through all items for the menu and paint them
	for (i = 0; i < menu->itemCount; i++) 
	{
		if (!menu->items[i]->appearanceSlot)
		{
			Item_Paint(menu->items[i]);
		}
		else // Timed order of appearance
		{
			if (menu->appearanceTime < DC->realTime)	// Time to show another item
			{
				menu->appearanceTime = DC->realTime + menu->appearanceIncrement;
				menu->appearanceCnt++;
			}

			if (menu->items[i]->appearanceSlot<=menu->appearanceCnt)
			{
				Item_Paint(menu->items[i]);
			}
		}
	}

	if (debugMode) {
		vec4_t color;
		color[0] = color[2] = color[3] = 1;
		color[1] = 0;
		DC->drawRect(menu->window.rect.x, menu->window.rect.y, menu->window.rect.w, menu->window.rect.h, 1, color);
	}
}

/*
===============
Item_ValidateTypeData
===============
*/
void Item_ValidateTypeData(itemDef_t *item) 
{
	if (item->typeData) 
	{
		return;
	}

	switch ( item->type )
	{
		case ITEM_TYPE_LISTBOX:
		case ITEM_TYPE_COMBOBOX:
			item->typeData = trap_VM_LocalAlloc(sizeof(listBoxDef_t));
			memset(item->typeData, 0, sizeof(listBoxDef_t));
			break;

		case ITEM_TYPE_EDITFIELD:
		case ITEM_TYPE_NUMERICFIELD:
		case ITEM_TYPE_YESNO:
		case ITEM_TYPE_BIND:
		case ITEM_TYPE_SLIDER:
		case ITEM_TYPE_TEXT:
		case ITEM_TYPE_PASSWORDFIELD:
			item->typeData = trap_VM_LocalAlloc(sizeof(editFieldDef_t));
			memset(item->typeData, 0, sizeof(editFieldDef_t));
			if (item->type == ITEM_TYPE_EDITFIELD || item->type == ITEM_TYPE_PASSWORDFIELD) 
			{
				if (!((editFieldDef_t *) item->typeData)->maxPaintChars) 
				{
					((editFieldDef_t *) item->typeData)->maxPaintChars = MAX_EDITFIELD;
				}
			}
			break;

		case ITEM_TYPE_MULTI:
			item->typeData = trap_VM_LocalAlloc(sizeof(multiDef_t));
			break;

		case ITEM_TYPE_MODEL:
			item->typeData = trap_VM_LocalAlloc(sizeof(modelDef_t));
			break;

		case ITEM_TYPE_TEXTSCROLL:
			item->typeData = trap_VM_LocalAlloc(sizeof(textScrollDef_t));
			break;
	}
}

/*
===============
Item Hash
===============
*/

typedef struct itemHash_s
{
	keywordHash_t	keyword;
	qboolean		(*func)(itemDef_t *item, int handle);

} itemHash_t;

/*
===============
Item Keyword Parse functions
===============
*/

// name <string>
qboolean ItemParse_name( itemDef_t *item, int handle ) {
	if (!PC_String_Parse(handle, &item->window.name)) {
		return qfalse;
	}
	return qtrue;
}

// name <string>
qboolean ItemParse_focusSound( itemDef_t *item, int handle ) {
	const char *temp;
	if (!PC_String_Parse(handle, &temp)) {
		return qfalse;
	}
	item->focusSound = DC->registerSound(temp);
	return qtrue;
}


// text <string>
qboolean ItemParse_text( itemDef_t *item, int handle ) {
	if (!PC_String_Parse(handle, &item->text)) {
		return qfalse;
	}
	return qtrue;
}

/*
===============
ItemParse_descText 
	text <string>
===============
*/
qboolean ItemParse_descText( itemDef_t *item, int handle) 
{

	if (!PC_String_Parse(handle, &item->descText))
	{
		return qfalse;
	}

	return qtrue;

}


/*
===============
ItemParse_text 
	text <string>
===============
*/
qboolean ItemParse_text2( itemDef_t *item, int handle) 
{

	if (!PC_String_Parse(handle, &item->text2))
	{
		return qfalse;
	}

	return qtrue;

}

/*
===============
ItemParse_text2alignx 
===============
*/
qboolean ItemParse_text2alignx( itemDef_t *item, int handle) 
{
	if (!PC_Float_Parse(handle, &item->text2alignx)) 
	{
		return qfalse;
	}
	return qtrue;
}

/*
===============
ItemParse_text2aligny 
===============
*/
qboolean ItemParse_text2aligny( itemDef_t *item, int handle) 
{
	if (!PC_Float_Parse(handle, &item->text2aligny)) 
	{
		return qfalse;
	}
	return qtrue;
}

// group <string>
qboolean ItemParse_group( itemDef_t *item, int handle ) {
	if (!PC_String_Parse(handle, &item->window.group)) {
		return qfalse;
	}
	return qtrue;
}

// asset_model <string>
qboolean ItemParse_asset_model( itemDef_t *item, int handle ) {
	const char *temp;
	modelDef_t *modelPtr;
	Item_ValidateTypeData(item);
	modelPtr = (modelDef_t*)item->typeData;

	if (!PC_String_Parse(handle, &temp)) {
		return qfalse;
	}
	item->asset = DC->registerModel(temp);
	modelPtr->angle = rand() % 360;
	return qtrue;
}

// asset_shader <string>
qboolean ItemParse_asset_shader( itemDef_t *item, int handle ) {
	const char *temp;

	if (!PC_String_Parse(handle, &temp)) {
		return qfalse;
	}
	item->asset = DC->registerShaderNoMip(temp);
	return qtrue;
}

// model_origin <number> <number> <number>
qboolean ItemParse_model_origin( itemDef_t *item, int handle ) {
	modelDef_t *modelPtr;
	Item_ValidateTypeData(item);
	modelPtr = (modelDef_t*)item->typeData;

	if (PC_Float_Parse(handle, &modelPtr->origin[0])) {
		if (PC_Float_Parse(handle, &modelPtr->origin[1])) {
			if (PC_Float_Parse(handle, &modelPtr->origin[2])) {
				return qtrue;
			}
		}
	}
	return qfalse;
}

// model_fovx <number>
qboolean ItemParse_model_fovx( itemDef_t *item, int handle ) {
	modelDef_t *modelPtr;
	Item_ValidateTypeData(item);
	modelPtr = (modelDef_t*)item->typeData;

	if (!PC_Float_Parse(handle, &modelPtr->fov_x)) {
		return qfalse;
	}
	return qtrue;
}

// model_fovy <number>
qboolean ItemParse_model_fovy( itemDef_t *item, int handle ) {
	modelDef_t *modelPtr;
	Item_ValidateTypeData(item);
	modelPtr = (modelDef_t*)item->typeData;

	if (!PC_Float_Parse(handle, &modelPtr->fov_y)) {
		return qfalse;
	}
	return qtrue;
}

// model_rotation <integer>
qboolean ItemParse_model_rotation( itemDef_t *item, int handle ) {
	modelDef_t *modelPtr;
	Item_ValidateTypeData(item);
	modelPtr = (modelDef_t*)item->typeData;

	if (!PC_Int_Parse(handle, &modelPtr->rotationSpeed)) {
		return qfalse;
	}
	return qtrue;
}

// model_angle <integer>
qboolean ItemParse_model_angle( itemDef_t *item, int handle ) {
	modelDef_t *modelPtr;
	Item_ValidateTypeData(item);
	modelPtr = (modelDef_t*)item->typeData;

	if (!PC_Int_Parse(handle, &modelPtr->angle)) {
		return qfalse;
	}
	return qtrue;
}

// rect <rectangle>
qboolean ItemParse_rect( itemDef_t *item, int handle ) {
	if (!PC_Rect_Parse(handle, &item->window.rectClient)) {
		return qfalse;
	}
	return qtrue;
}

/*
===============
ItemParse_flag
	style <integer>
===============
*/
qboolean ItemParse_flag( itemDef_t *item, int handle) 
{
	int		i;
	const char	*tempStr;

	if (!PC_String_Parse(handle, &tempStr))
	{
		return qfalse;
	}

	i=0;
	while (styles[i])
	{
		if (Q_stricmp(tempStr,itemFlags[i].string)==0)
		{
			item->window.flags |= itemFlags[i].value;
			break;
		}
		i++;
	}

	if (itemFlags[i].string == NULL)
	{
		Com_Printf(va(S_COLOR_YELLOW "Unknown item style value '%s'",tempStr));
	}

	return qtrue;
}

/*
===============
ItemParse_style 
	style <integer>
===============
*/
qboolean ItemParse_style( itemDef_t *item, int handle) 
{
	if (!PC_Int_Parse(handle, &item->window.style))
	{
		Com_Printf(S_COLOR_YELLOW "Unknown item style value");
		return qfalse;
	}

	return qtrue;
}


// decoration
qboolean ItemParse_decoration( itemDef_t *item, int handle ) {
	item->window.flags |= WINDOW_DECORATION;
	return qtrue;
}

// notselectable
qboolean ItemParse_notselectable( itemDef_t *item, int handle ) {
	listBoxDef_t *listPtr;
	Item_ValidateTypeData(item);
	listPtr = (listBoxDef_t*)item->typeData;
	if (item->type == ITEM_TYPE_LISTBOX && listPtr) {
		listPtr->notselectable = qtrue;
	}
	return qtrue;
}

// manually wrapped
qboolean ItemParse_wrapped( itemDef_t *item, int handle ) {
	item->window.flags |= WINDOW_WRAPPED;
	return qtrue;
}

// auto wrapped
qboolean ItemParse_autowrapped( itemDef_t *item, int handle ) {
	item->window.flags |= WINDOW_AUTOWRAPPED;
	return qtrue;
}


// horizontalscroll
qboolean ItemParse_horizontalscroll( itemDef_t *item, int handle ) {
	item->window.flags |= WINDOW_HORIZONTAL;
	return qtrue;
}

/*
===============
ItemParse_type 
	type <integer>
===============
*/
qboolean ItemParse_type( itemDef_t *item, int handle  ) 
{
//	int		i,holdInt;

	if (!PC_Int_Parse(handle, &item->type)) 
	{
		return qfalse;
	}
	Item_ValidateTypeData(item);
	return qtrue;
}

// elementwidth, used for listbox image elements
// uses textalignx for storage
qboolean ItemParse_elementwidth( itemDef_t *item, int handle ) {
	listBoxDef_t *listPtr;

	Item_ValidateTypeData(item);
	listPtr = (listBoxDef_t*)item->typeData;
	if (!PC_Float_Parse(handle, &listPtr->elementWidth)) {
		return qfalse;
	}
	return qtrue;
}

// elementheight, used for listbox image elements
// uses textaligny for storage
qboolean ItemParse_elementheight( itemDef_t *item, int handle ) {
	listBoxDef_t *listPtr;

	Item_ValidateTypeData(item);
	listPtr = (listBoxDef_t*)item->typeData;
	if (!PC_Float_Parse(handle, &listPtr->elementHeight)) {
		return qfalse;
	}
	return qtrue;
}

// feeder <float>
qboolean ItemParse_feeder( itemDef_t *item, int handle ) {
	if (!PC_Float_Parse(handle, &item->special)) {
		return qfalse;
	}
	return qtrue;
}

// elementtype, used to specify what type of elements a listbox contains
// uses textstyle for storage
qboolean ItemParse_elementtype( itemDef_t *item, int handle ) {
	listBoxDef_t *listPtr;

	Item_ValidateTypeData(item);
	if (!item->typeData)
		return qfalse;
	listPtr = (listBoxDef_t*)item->typeData;
	if (!PC_Int_Parse(handle, &listPtr->elementStyle)) {
		return qfalse;
	}
	return qtrue;
}

// columns sets a number of columns and an x pos and width per.. 
qboolean ItemParse_columns( itemDef_t *item, int handle ) {
	int num, i;
	listBoxDef_t *listPtr;

	Item_ValidateTypeData(item);
	if (!item->typeData)
		return qfalse;
	listPtr = (listBoxDef_t*)item->typeData;
	if (PC_Int_Parse(handle, &num)) {
		if (num > MAX_LB_COLUMNS) {
			num = MAX_LB_COLUMNS;
		}
		listPtr->numColumns = num;
		for (i = 0; i < num; i++) {
			int pos, width, maxChars;

			if (PC_Int_Parse(handle, &pos) && PC_Int_Parse(handle, &width) && PC_Int_Parse(handle, &maxChars)) {
				listPtr->columnInfo[i].pos = pos;
				listPtr->columnInfo[i].width = width;
				listPtr->columnInfo[i].maxChars = maxChars;
			} else {
				return qfalse;
			}
		}
	} else {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_border( itemDef_t *item, int handle ) {
	if (!PC_Int_Parse(handle, &item->window.border)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_bordersize( itemDef_t *item, int handle ) {
	if (!PC_Float_Parse(handle, &item->window.borderSize)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_visible( itemDef_t *item, int handle ) {
	int i;

	if (!PC_Int_Parse(handle, &i)) {
		return qfalse;
	}
	if (i) {
		item->window.flags |= WINDOW_VISIBLE;
	}
	return qtrue;
}

qboolean ItemParse_ownerdraw( itemDef_t *item, int handle ) {
	if (!PC_Int_Parse(handle, &item->window.ownerDraw)) {
		return qfalse;
	}
	item->type = ITEM_TYPE_OWNERDRAW;
	return qtrue;
}

qboolean ItemParse_ownerwidth( itemDef_t *item, int handle ) 
{
	if (!PC_Int_Parse(handle, &item->window.ownerWidth)) 
	{
		return qfalse;
	}

	return qtrue;
}

qboolean ItemParse_align( itemDef_t *item, int handle ) {
	if (!PC_Int_Parse(handle, &item->alignment)) {
		return qfalse;
	}
	return qtrue;
}

/*
===============
ItemParse_textalign 
===============
*/
qboolean ItemParse_textalign( itemDef_t *item, int handle ) 
{
	if (!PC_Int_Parse(handle, &item->textalignment)) 
	{
		Com_Printf(S_COLOR_YELLOW "Unknown text alignment value");
		return qfalse;
	}

	return qtrue;

}

qboolean ItemParse_textalignx( itemDef_t *item, int handle ) {
	if (!PC_Float_Parse(handle, &item->textalignx)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_textaligny( itemDef_t *item, int handle ) {
	if (!PC_Float_Parse(handle, &item->textaligny)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_textscale( itemDef_t *item, int handle ) {
	if (!PC_Float_Parse(handle, &item->textscale)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_textstyle( itemDef_t *item, int handle ) {
	if (!PC_Int_Parse(handle, &item->textStyle)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_textfont( itemDef_t *item, int handle ) 
{
	const char* fontName;

	if (!PC_String_Parse(handle, &fontName) )
	{
		return qfalse;
	}

	item->textFont = DC->registerFont ( fontName );

	return qtrue;
}

qboolean ItemParse_backcolor( itemDef_t *item, int handle ) {
	int i;
	float f;

	for (i = 0; i < 4; i++) {
		if (!PC_Float_Parse(handle, &f)) {
			return qfalse;
		}
		item->window.backColor[i]  = f;
	}
	return qtrue;
}

qboolean ItemParse_forecolor( itemDef_t *item, int handle ) {
	int i;
	float f;

	for (i = 0; i < 4; i++) {
		if (!PC_Float_Parse(handle, &f)) {
			return qfalse;
		}
		item->window.foreColor[i]  = f;
		item->window.flags |= WINDOW_FORECOLORSET;
	}
	return qtrue;
}

qboolean ItemParse_bordercolor( itemDef_t *item, int handle ) {
	int i;
	float f;

	for (i = 0; i < 4; i++) {
		if (!PC_Float_Parse(handle, &f)) {
			return qfalse;
		}
		item->window.borderColor[i]  = f;
	}
	return qtrue;
}

qboolean ItemParse_outlinecolor( itemDef_t *item, int handle ) {
	if (!PC_Color_Parse(handle, &item->window.outlineColor)){
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_background( itemDef_t *item, int handle ) {
	const char *temp;

	if (!PC_String_Parse(handle, &temp)) {
		return qfalse;
	}
	item->window.background = DC->registerShaderNoMip(temp);
	return qtrue;
}

qboolean ItemParse_cinematic( itemDef_t *item, int handle ) {
	if (!PC_String_Parse(handle, &item->window.cinematicName)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_doubleClick( itemDef_t *item, int handle ) {
	listBoxDef_t *listPtr;

	Item_ValidateTypeData(item);
	if (!item->typeData) {
		return qfalse;
	}

	listPtr = (listBoxDef_t*)item->typeData;

	if (!PC_Script_Parse(handle, &listPtr->doubleClick)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_hotKey ( itemDef_t* item, int handle ) {
	const char *temp;
	if (!PC_String_Parse(handle, &temp)) {
		return qfalse;
	}

	item->hotKey = temp[0];

	return qtrue;
}

qboolean ItemParse_onFocus( itemDef_t *item, int handle ) {
	if (!PC_Script_Parse(handle, &item->onFocus)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_leaveFocus( itemDef_t *item, int handle ) {
	if (!PC_Script_Parse(handle, &item->leaveFocus)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_mouseEnter( itemDef_t *item, int handle ) {
	if (!PC_Script_Parse(handle, &item->mouseEnter)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_mouseExit( itemDef_t *item, int handle ) {
	if (!PC_Script_Parse(handle, &item->mouseExit)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_mouseEnterText( itemDef_t *item, int handle ) {
	if (!PC_Script_Parse(handle, &item->mouseEnterText)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_mouseExitText( itemDef_t *item, int handle ) {
	if (!PC_Script_Parse(handle, &item->mouseExitText)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_action( itemDef_t *item, int handle ) {
	if (!PC_Script_Parse(handle, &item->action)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_special( itemDef_t *item, int handle ) {
	if (!PC_Float_Parse(handle, &item->special)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_cvarTest( itemDef_t *item, int handle ) {
	if (!PC_String_Parse(handle, &item->cvarTest)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_cvar( itemDef_t *item, int handle ) {
	editFieldDef_t *editPtr;

	Item_ValidateTypeData(item);
	if (!PC_String_Parse(handle, &item->cvar)) {
		return qfalse;
	}
	if (item->typeData) {
		editPtr = (editFieldDef_t*)item->typeData;
		editPtr->minVal = -1;
		editPtr->maxVal = -1;
		editPtr->defVal = -1;
	}
	return qtrue;
}

qboolean ItemParse_maxChars( itemDef_t *item, int handle ) {
	editFieldDef_t *editPtr;
	int maxChars;

	Item_ValidateTypeData(item);
	if (!item->typeData)
		return qfalse;

	if (!PC_Int_Parse(handle, &maxChars)) {
		return qfalse;
	}
	editPtr = (editFieldDef_t*)item->typeData;
	editPtr->maxChars = maxChars;
	return qtrue;
}

qboolean ItemParse_maxPaintChars( itemDef_t *item, int handle ) {
	editFieldDef_t *editPtr;
	int maxChars;

	Item_ValidateTypeData(item);
	if (!item->typeData)
		return qfalse;

	if (!PC_Int_Parse(handle, &maxChars)) {
		return qfalse;
	}
	editPtr = (editFieldDef_t*)item->typeData;
	editPtr->maxPaintChars = maxChars;
	return qtrue;
}



qboolean ItemParse_maxLineChars( itemDef_t *item, int handle ) 
{
	textScrollDef_t *scrollPtr;
	int				maxChars;

	Item_ValidateTypeData(item);
	if (!item->typeData)
		return qfalse;

	if (!PC_Int_Parse(handle, &maxChars)) 
	{
		return qfalse;
	}

	scrollPtr = (textScrollDef_t*)item->typeData;
	scrollPtr->maxLineChars = maxChars;

	return qtrue;
}

qboolean ItemParse_lineHeight( itemDef_t *item, int handle ) 
{
	textScrollDef_t *scrollPtr;
	int				height;

	Item_ValidateTypeData(item);
	if (!item->typeData)
		return qfalse;

	if (!PC_Int_Parse(handle, &height)) 
	{
		return qfalse;
	}

	scrollPtr = (textScrollDef_t*)item->typeData;
	scrollPtr->lineHeight = height;

	return qtrue;
}

qboolean ItemParse_cvarFloat( itemDef_t *item, int handle ) {
	editFieldDef_t *editPtr;

	Item_ValidateTypeData(item);
	if (!item->typeData)
		return qfalse;
	editPtr = (editFieldDef_t*)item->typeData;
	if (PC_String_Parse(handle, &item->cvar) &&
		PC_Float_Parse(handle, &editPtr->defVal) &&
		PC_Float_Parse(handle, &editPtr->minVal) &&
		PC_Float_Parse(handle, &editPtr->maxVal)) {
		return qtrue;
	}
	return qfalse;
}

qboolean ItemParse_cvarStrList( itemDef_t *item, int handle ) {
	pc_token_t token;
	multiDef_t *multiPtr;
	int pass;
	
	Item_ValidateTypeData(item);
	if (!item->typeData)
		return qfalse;
	multiPtr = (multiDef_t*)item->typeData;
	multiPtr->count = 0;
	multiPtr->strDef = qtrue;

	if (!trap_PC_ReadToken(handle, &token))
		return qfalse;
	if (*token.string != '{') {
		return qfalse;
	}

	pass = 0;
	while ( 1 ) {
		if (!trap_PC_ReadToken(handle, &token)) {
			PC_SourceError(handle, "end of file inside menu item\n");
			return qfalse;
		}

		if (*token.string == '}') {
			return qtrue;
		}

		if (*token.string == ',' || *token.string == ';') {
			continue;
		}

		if (pass == 0) {
			multiPtr->cvarList[multiPtr->count] = String_Alloc(token.string);
			pass = 1;
		} else {
			multiPtr->cvarStr[multiPtr->count] = String_Alloc(token.string);
			pass = 0;
			multiPtr->count++;
			if (multiPtr->count >= MAX_MULTI_CVARS) {
				return qfalse;
			}
		}

	}
	return qfalse; 	// bk001205 - LCC missing return value
}

qboolean ItemParse_cvarFloatList( itemDef_t *item, int handle ) {
	pc_token_t token;
	multiDef_t *multiPtr;
	
	Item_ValidateTypeData(item);
	if (!item->typeData)
		return qfalse;
	multiPtr = (multiDef_t*)item->typeData;
	multiPtr->count = 0;
	multiPtr->strDef = qfalse;

	if (!trap_PC_ReadToken(handle, &token))
		return qfalse;
	if (*token.string != '{') {
		return qfalse;
	}

	while ( 1 ) {
		if (!trap_PC_ReadToken(handle, &token)) {
			PC_SourceError(handle, "end of file inside menu item\n");
			return qfalse;
		}

		if (*token.string == '}') {
			return qtrue;
		}

		if (*token.string == ',' || *token.string == ';') {
			continue;
		}

		multiPtr->cvarList[multiPtr->count] = String_Alloc(token.string);
		if (!PC_Float_Parse(handle, &multiPtr->cvarValue[multiPtr->count])) {
			return qfalse;
		}

		multiPtr->count++;
		if (multiPtr->count >= MAX_MULTI_CVARS) {
			return qfalse;
		}

	}
	return qfalse; 	// bk001205 - LCC missing return value
}



qboolean ItemParse_addColorRange( itemDef_t *item, int handle ) {
	colorRangeDef_t color;

	if (PC_Float_Parse(handle, &color.low) &&
		PC_Float_Parse(handle, &color.high) &&
		PC_Color_Parse(handle, &color.color) ) {
		if (item->numColors < MAX_COLOR_RANGES) {
			memcpy(&item->colorRanges[item->numColors], &color, sizeof(color));
			item->numColors++;
		}
		return qtrue;
	}
	return qfalse;
}

qboolean ItemParse_ownerdrawFlag( itemDef_t *item, int handle ) 
{
	int i;
	if (!PC_Int_Parse(handle, &i)) 
	{
		return qfalse;
	}
	item->window.ownerDrawFlags |= i;
	return qtrue;
}

qboolean ItemParse_ownerdrawParam ( itemDef_t* item, int handle )
{
	if ( !PC_String_Parse ( handle, &item->window.ownerDrawParam ) )
	{
		return qfalse;
	}

	return qtrue;
}

qboolean ItemParse_enableCvar( itemDef_t *item, int handle ) {
	if (PC_Script_Parse(handle, &item->enableCvar)) {
		item->cvarFlags = CVAR_ENABLE;
		return qtrue;
	}
	return qfalse;
}

qboolean ItemParse_disableCvar( itemDef_t *item, int handle ) {
	if (PC_Script_Parse(handle, &item->enableCvar)) {
		item->cvarFlags = CVAR_DISABLE;
		return qtrue;
	}
	return qfalse;
}

qboolean ItemParse_showCvar( itemDef_t *item, int handle ) {
	if (PC_Script_Parse(handle, &item->enableCvar)) {
		item->cvarFlags = CVAR_SHOW;
		return qtrue;
	}
	return qfalse;
}

qboolean ItemParse_hideCvar( itemDef_t *item, int handle ) {
	if (PC_Script_Parse(handle, &item->enableCvar)) {
		item->cvarFlags = CVAR_HIDE;
		return qtrue;
	}
	return qfalse;
}

/*
===============
ItemParse_align 
===============
*/
qboolean ItemParse_Appearance_slot( itemDef_t *item, int handle ) 
{
	if (!PC_Int_Parse(handle, &item->appearanceSlot))
	{
		return qfalse;
	}
	return qtrue;
}


/*
===============
ItemParse_tooltip 
===============
*/
qboolean ItemParse_tooltip ( itemDef_t *item, int handle ) 
{
	if (!PC_String_Parse(handle, &item->tooltip))
	{
		return qfalse;
	}

	return qtrue;
}

itemHash_t itemParseKeywords[] = {
	{{"action",				NULL },		ItemParse_action			},
	{{"addColorRange",		NULL },		ItemParse_addColorRange		},
	{{"align",				NULL },		ItemParse_align				},
	{{"autowrapped",		NULL },		ItemParse_autowrapped		},
	{{"appearance_slot",	NULL },		ItemParse_Appearance_slot	},
	{{"asset_model",		NULL },		ItemParse_asset_model		},
	{{"asset_shader",		NULL },		ItemParse_asset_shader		},
	{{"backcolor",			NULL },		ItemParse_backcolor			},
	{{"background",			NULL },		ItemParse_background		},
	{{"border",				NULL },		ItemParse_border			},
	{{"bordercolor",		NULL },		ItemParse_bordercolor		},
	{{"bordersize",			NULL },		ItemParse_bordersize		},
	{{"cinematic",			NULL },		ItemParse_cinematic			},
	{{"columns",			NULL },		ItemParse_columns			},
	{{"cvar",				NULL },		ItemParse_cvar				},
	{{"cvarFloat",			NULL },		ItemParse_cvarFloat			},
	{{"cvarFloatList",		NULL },		ItemParse_cvarFloatList		},
	{{"cvarStrList",		NULL },		ItemParse_cvarStrList		},
	{{"cvarTest",			NULL },		ItemParse_cvarTest			},
	{{"desctext",			NULL },		ItemParse_descText			},
	{{"decoration",			NULL },		ItemParse_decoration		},
	{{"disableCvar",		NULL },		ItemParse_disableCvar		},
	{{"doubleclick",		NULL },		ItemParse_doubleClick		},
	{{"elementheight",		NULL },		ItemParse_elementheight		},
	{{"elementtype",		NULL },		ItemParse_elementtype		},
	{{"elementwidth",		NULL },		ItemParse_elementwidth		},
	{{"enableCvar",			NULL },		ItemParse_enableCvar		},
	{{"feeder",				NULL },		ItemParse_feeder			},
	{{"flag",				NULL },		ItemParse_flag				},
	{{"focusSound",			NULL },		ItemParse_focusSound		},
	{{"forecolor",			NULL },		ItemParse_forecolor			},
	{{"group",				NULL },		ItemParse_group				},
	{{"hideCvar",			NULL },		ItemParse_hideCvar			},
	{{"horizontalscroll",	NULL },		ItemParse_horizontalscroll	},
	{{"leaveFocus",			NULL },		ItemParse_leaveFocus		},
	{{"maxChars",			NULL },		ItemParse_maxChars			},
	{{"maxPaintChars",		NULL },		ItemParse_maxPaintChars		},
	{{"model_angle",		NULL },		ItemParse_model_angle		},
	{{"model_fovx",			NULL },		ItemParse_model_fovx		},
	{{"model_fovy",			NULL },		ItemParse_model_fovy		},
	{{"model_origin",		NULL },		ItemParse_model_origin		},
	{{"model_rotation",		NULL },		ItemParse_model_rotation	},
	{{"mouseEnter",			NULL },		ItemParse_mouseEnter		},
	{{"mouseEnterText",		NULL },		ItemParse_mouseEnterText	},
	{{"mouseExit",			NULL },		ItemParse_mouseExit			},
	{{"mouseExitText",		NULL },		ItemParse_mouseExitText		},
	{{"name",				NULL },		ItemParse_name				},
	{{"notselectable",		NULL },		ItemParse_notselectable		},
	{{"onFocus",			NULL },		ItemParse_onFocus			},
	{{"outlinecolor",		NULL },		ItemParse_outlinecolor		},
	{{"ownerdraw",			NULL },		ItemParse_ownerdraw			},
	{{"ownerdrawFlag",		NULL },		ItemParse_ownerdrawFlag		},
	{{"ownerdrawParam",		NULL },		ItemParse_ownerdrawParam	},
	{{"ownerwidth",			NULL },		ItemParse_ownerwidth		},
	{{"rect",				NULL },		ItemParse_rect				},
	{{"showCvar",			NULL },		ItemParse_showCvar			},
	{{"special",			NULL },		ItemParse_special			},
	{{"style",				NULL },		ItemParse_style				},
	{{"text",				NULL },		ItemParse_text				},
	{{"textalign",			NULL },		ItemParse_textalign			},
	{{"textalignx",			NULL },		ItemParse_textalignx		},
	{{"textaligny",			NULL },		ItemParse_textaligny		},
	{{"textscale",			NULL },		ItemParse_textscale			},
	{{"textstyle",			NULL },		ItemParse_textstyle			},
	{{"textfont",			NULL },		ItemParse_textfont			},
	{{"text2",				NULL },		ItemParse_text2				},
	{{"text2alignx",		NULL },		ItemParse_text2alignx		},
	{{"text2aligny",		NULL },		ItemParse_text2aligny		},
	{{"type",				NULL },		ItemParse_type				},
	{{"visible",			NULL },		ItemParse_visible			},
	{{"wrapped",			NULL },		ItemParse_wrapped			},
	{{"tooltip",			NULL },		ItemParse_tooltip			},
	{{"hotkey",				NULL },		ItemParse_hotKey			},
	// Text scroll specific
	{{"maxLineChars",		NULL },		ItemParse_maxLineChars		},
	{{"lineHeight",			NULL },		ItemParse_lineHeight		},

	{{ NULL,				NULL },		NULL						}
};

keywordHash_t *itemParseKeywordHash[KEYWORDHASH_SIZE];

/*
===============
Item_SetupKeywordHash
===============
*/
void Item_SetupKeywordHash(void) 
{
	int i;

	memset(itemParseKeywordHash, 0, sizeof(itemParseKeywordHash));
	for (i = 0; itemParseKeywords[i].keyword.name; i++) 
	{
		// Initialize the keyword collision list
		itemParseKeywords[i].keyword.next = NULL;

		// Add the keyword
		KeywordHash_Add( itemParseKeywordHash, (keywordHash_t*)&itemParseKeywords[i]);
	}
}

/*
===============
Item_Parse
===============
*/
qboolean Item_Parse(int handle, itemDef_t *item) 
{
	pc_token_t	token;
	itemHash_t*	key;

	if (!trap_PC_ReadToken(handle, &token))
		return qfalse;
	if (*token.string != '{') {
		return qfalse;
	}
	while ( 1 ) {
		if (!trap_PC_ReadToken(handle, &token)) {
			PC_SourceError(handle, "end of file inside menu item\n");
			return qfalse;
		}

		if (*token.string == '}') {
			return qtrue;
		}

		key = (itemHash_t*)KeywordHash_Find(itemParseKeywordHash, token.string);
		if (!key) {
			PC_SourceError(handle, "unknown menu item keyword %s", token.string);
			continue;
		}
		
		if ( !key->func(item, handle) ) {
			PC_SourceError(handle, "couldn't parse menu item keyword %s", token.string);
			return qfalse;
		}
	}
	return qfalse; 	// bk001205 - LCC missing return value
}


static void Item_TextScroll_BuildLines ( itemDef_t* item )
{
	textScrollDef_t* scrollPtr = (textScrollDef_t*) item->typeData;
	int				 width;
	char*			 lineStart;
	char*			 lineEnd;
	float			 w;
	float			 cw;

	scrollPtr->lineCount = 0;

	if (!item->text || *item->text == '\0' )
	{
		return;
	}

	scrollPtr->lineCount = 0;
	width = scrollPtr->maxLineChars;

	lineStart = (char*)item->text;

	lineEnd   = lineStart;
	w		  = 0;

	// Keep going as long as there are more lines
	while ( scrollPtr->lineCount < MAX_TEXTSCROLL_LINES )
	{
		// End of the road
		if ( *lineEnd == '\0')
		{
			if ( lineStart < lineEnd )
			{
				scrollPtr->lines[ scrollPtr->lineCount++ ] = lineStart;
			}

			break;
		}

		// Force a line end if its a '\n'
		else if ( *lineEnd == '\n' )
		{
			*lineEnd = '\0';
			scrollPtr->lines[ scrollPtr->lineCount++ ] = lineStart;
			lineStart = lineEnd + 1;
			lineEnd   = lineStart;
			w = 0;
			continue;
		}

		// Get the current character width 
		cw = DC->getTextWidth ( va("%c", *lineEnd), item->textFont,item->textscale, 0 );

		// Past the end of the boundary?
		if ( w + cw > (item->window.rect.w - SCROLLBAR_SIZE - 10) )
		{
			// Past the end so backtrack to the word boundary
			while ( *lineEnd != ' ' && *lineEnd != '\t' && lineEnd > lineStart )
			{
				lineEnd--;
			}					

			*lineEnd = '\0';
			scrollPtr->lines[ scrollPtr->lineCount++ ] = lineStart;

			// Skip any whitespaces
			lineEnd++;
			while ( (*lineEnd == ' ' || *lineEnd == '\t') && *lineEnd )
			{
				lineEnd++;
			}

			lineStart = lineEnd;
			w = 0;
		}
		else
		{
			w += cw;
			lineEnd++;
		}
	}
}

// Item_InitControls
// init's special control types
void Item_InitControls(itemDef_t *item) 
{
	if (item == NULL) 
	{
		return;
	}

	switch ( item->type )
	{
		case ITEM_TYPE_LISTBOX:
		case ITEM_TYPE_COMBOBOX:
		{
			listBoxDef_t *listPtr = (listBoxDef_t*)item->typeData;
			item->cursorPos = 0;
			if (listPtr) 
			{
				listPtr->cursorPos = 0;
				listPtr->startPos = 0;
				listPtr->endPos = 0;
				listPtr->cursorPos = 0;
			}

			break;
		}
	}
}

/*
===============
Menu Keyword Parse functions
===============
*/

qboolean MenuParse_name( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	if (!PC_String_Parse(handle, &menu->window.name)) {
		return qfalse;
	}
	if (Q_stricmp(menu->window.name, "main") == 0) {
		// default main as having focus
		//menu->window.flags |= WINDOW_HASFOCUS;
	}
	return qtrue;
}

qboolean MenuParse_fullscreen( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	if (!PC_Int_Parse(handle, (int*) &menu->fullScreen)) { // bk001206 - cast qboolean
		return qfalse;
	}
	return qtrue;
}

qboolean MenuParse_rect( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	if (!PC_Rect_Parse(handle, &menu->window.rect)) {
		return qfalse;
	}
	return qtrue;
}

/*
=================
MenuParse_style
=================
*/
qboolean MenuParse_style( itemDef_t *item, int handle) 
{
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_Int_Parse(handle, &menu->window.style))
	{
		Com_Printf(S_COLOR_YELLOW "Unknown menu style value");
		return qfalse;
	}

	return qtrue;
}

qboolean MenuParse_visible( itemDef_t *item, int handle ) {
	int i;
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_Int_Parse(handle, &i)) {
		return qfalse;
	}
	if (i) {
		menu->window.flags |= WINDOW_VISIBLE;
	}
	return qtrue;
}

qboolean MenuParse_onOpen( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	if (!PC_Script_Parse(handle, &menu->onOpen)) {
		return qfalse;
	}
	return qtrue;
}

qboolean MenuParse_onClose( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	if (!PC_Script_Parse(handle, &menu->onClose)) {
		return qfalse;
	}
	return qtrue;
}

qboolean MenuParse_onESC( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	if (!PC_Script_Parse(handle, &menu->onESC)) {
		return qfalse;
	}
	return qtrue;
}



qboolean MenuParse_border( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	if (!PC_Int_Parse(handle, &menu->window.border)) {
		return qfalse;
	}
	return qtrue;
}

qboolean MenuParse_borderSize( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	if (!PC_Float_Parse(handle, &menu->window.borderSize)) {
		return qfalse;
	}
	return qtrue;
}

qboolean MenuParse_backcolor( itemDef_t *item, int handle ) {
	int i;
	float f;
	menuDef_t *menu = (menuDef_t*)item;

	for (i = 0; i < 4; i++) {
		if (!PC_Float_Parse(handle, &f)) {
			return qfalse;
		}
		menu->window.backColor[i]  = f;
	}
	return qtrue;
}

/*
=================
MenuParse_descAlignment
=================
*/
qboolean MenuParse_descAlignment( itemDef_t *item, int handle ) 
{
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_Int_Parse(handle, &menu->descAlignment)) 
	{
		Com_Printf(S_COLOR_YELLOW "Unknown desc alignment value");
		return qfalse;
	}

	return qtrue;
}

/*
=================
MenuParse_descX
=================
*/
qboolean MenuParse_descX( itemDef_t *item, int handle ) 
{
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_Int_Parse(handle, &menu->descX))
	{
		return qfalse;
	}
	return qtrue;
}

/*
=================
MenuParse_descY
=================
*/
qboolean MenuParse_descY( itemDef_t *item, int handle ) 
{
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_Int_Parse(handle, &menu->descY))
	{
		return qfalse;
	}
	return qtrue;
}
/*
=================
MenuParse_descColor
=================
*/
qboolean MenuParse_descColor( itemDef_t *item, int handle) 
{
	int i;
	float f;
	menuDef_t *menu = (menuDef_t*)item;

	for (i = 0; i < 4; i++) 
	{
		if (!PC_Float_Parse(handle, &f)) 
		{
			return qfalse;
		}
		menu->descColor[i]  = f;
	}
	return qtrue;
}

qboolean MenuParse_forecolor( itemDef_t *item, int handle ) {
	int i;
	float f;
	menuDef_t *menu = (menuDef_t*)item;

	for (i = 0; i < 4; i++) {
		if (!PC_Float_Parse(handle, &f)) {
			return qfalse;
		}
		menu->window.foreColor[i]  = f;
		menu->window.flags |= WINDOW_FORECOLORSET;
	}
	return qtrue;
}

qboolean MenuParse_bordercolor( itemDef_t *item, int handle ) {
	int i;
	float f;
	menuDef_t *menu = (menuDef_t*)item;

	for (i = 0; i < 4; i++) {
		if (!PC_Float_Parse(handle, &f)) {
			return qfalse;
		}
		menu->window.borderColor[i]  = f;
	}
	return qtrue;
}

qboolean MenuParse_focuscolor( itemDef_t *item, int handle ) {
	int i;
	float f;
	menuDef_t *menu = (menuDef_t*)item;

	for (i = 0; i < 4; i++) {
		if (!PC_Float_Parse(handle, &f)) {
			return qfalse;
		}
		menu->focusColor[i]  = f;
	}
	return qtrue;
}

qboolean MenuParse_disablecolor( itemDef_t *item, int handle ) {
	int i;
	float f;
	menuDef_t *menu = (menuDef_t*)item;
	for (i = 0; i < 4; i++) {
		if (!PC_Float_Parse(handle, &f)) {
			return qfalse;
		}
		menu->disableColor[i]  = f;
	}
	return qtrue;
}


qboolean MenuParse_outlinecolor( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	if (!PC_Color_Parse(handle, &menu->window.outlineColor)){
		return qfalse;
	}
	return qtrue;
}

qboolean MenuParse_background( itemDef_t *item, int handle ) {
	const char *buff;
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_String_Parse(handle, &buff)) {
		return qfalse;
	}
	menu->window.background = DC->registerShaderNoMip(buff);
	return qtrue;
}

qboolean MenuParse_cinematic( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_String_Parse(handle, &menu->window.cinematicName)) {
		return qfalse;
	}
	return qtrue;
}

qboolean MenuParse_ownerdrawFlag( itemDef_t *item, int handle ) {
	int i;
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_Int_Parse(handle, &i)) {
		return qfalse;
	}
	menu->window.ownerDrawFlags |= i;
	return qtrue;
}

qboolean MenuParse_ownerdraw( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_Int_Parse(handle, &menu->window.ownerDraw)) {
		return qfalse;
	}
	return qtrue;
}


// decoration
qboolean MenuParse_popup( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	menu->window.flags |= WINDOW_POPUP;
	return qtrue;
}


qboolean MenuParse_outOfBounds( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;

	menu->window.flags |= WINDOW_OOB_CLICK;
	return qtrue;
}

qboolean MenuParse_soundLoop( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_String_Parse(handle, &menu->soundName)) {
		return qfalse;
	}
	return qtrue;
}

qboolean MenuParse_fadeClamp( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_Float_Parse(handle, &menu->fadeClamp)) {
		return qfalse;
	}
	return qtrue;
}

qboolean MenuParse_fadeAmount( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_Float_Parse(handle, &menu->fadeAmount)) {
		return qfalse;
	}
	return qtrue;
}


qboolean MenuParse_fadeCycle( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_Int_Parse(handle, &menu->fadeCycle)) {
		return qfalse;
	}
	return qtrue;
}

#define VIOLENCE_NAME	"violence_"


qboolean MenuParse_itemDef( itemDef_t *item, int handle ) 
{
#if !(CGAME)
	char	temp[64];
#endif

	menuDef_t *menu = (menuDef_t*)item;
	if (menu->itemCount < MAX_MENUITEMS) {
		menu->items[menu->itemCount] = trap_VM_LocalAlloc(sizeof(itemDef_t));
		Item_Init(menu->items[menu->itemCount]);
		if (!Item_Parse(handle, menu->items[menu->itemCount])) {
			return qfalse;
		}

#if !(CGAME)
		if (!ui_allowparental.integer && menu->items[menu->itemCount]->window.name)
		{
			strncpy(temp, menu->items[menu->itemCount]->window.name, strlen(VIOLENCE_NAME));
			temp[strlen(VIOLENCE_NAME)] = 0;
			if (Q_stricmp(temp, VIOLENCE_NAME) == 0)
			{
				return qtrue;
			}
		}
#endif
		Item_InitControls(menu->items[menu->itemCount]);
		menu->items[menu->itemCount++]->parent = menu;
	}
	return qtrue;
}
/*
=================
MenuParse_focuscolor
=================
*/
qboolean MenuParse_appearanceIncrement( itemDef_t *item, int handle ) 
{
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_Float_Parse(handle, &menu->appearanceIncrement))
	{
		return qfalse;
	}
	return qtrue;
}

/*
=================
MenuParse_tooltipforecolor
=================
*/
qboolean MenuParse_tooltipforecolor ( itemDef_t *item, int handle ) 
{
	int i;
	float f;
	menuDef_t *menu = (menuDef_t*)item;

	for (i = 0; i < 4; i++) 
	{
		if (!PC_Float_Parse(handle, &f)) 
		{
			return qfalse;
		}

		menu->tooltipForeColor[i]  = f;
	}

	return qtrue;
}

/*
=================
MenuParse_tooltipbackcolor
=================
*/
qboolean MenuParse_tooltipbackcolor ( itemDef_t *item, int handle ) 
{
	int i;
	float f;
	menuDef_t *menu = (menuDef_t*)item;

	for (i = 0; i < 4; i++) 
	{
		if (!PC_Float_Parse(handle, &f)) 
		{
			return qfalse;
		}

		menu->tooltipBackColor[i]  = f;
	}

	return qtrue;
}

/*
=================
MenuParse_tooltipscale
=================
*/
qboolean MenuParse_tooltipscale ( itemDef_t *item, int handle ) 
{
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_Float_Parse(handle, &menu->tooltipScale)) 
	{
		return qfalse;
	}

	return qtrue;
}

/*
=================
MenuParse_tooltipdelay
=================
*/
qboolean MenuParse_tooltipdelay ( itemDef_t *item, int handle ) 
{
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_Int_Parse(handle, &menu->tooltipDelay)) 
	{
		return qfalse;
	}

	return qtrue;
}

/*
=================
MenuParse_tooltipfont
=================
*/
qboolean MenuParse_tooltipfont( itemDef_t *item, int handle ) 
{
	const char *fontName;
	menuDef_t  *menu = (menuDef_t*)item;

	if (!PC_String_Parse(handle, &fontName) )
	{
		return qfalse;
	}

	menu->tooltipFont = DC->registerFont ( fontName );

	return qtrue;
}


itemHash_t menuParseKeywords[] = 
{
	{{"appearanceIncrement"		}, MenuParse_appearanceIncrement	},
	{{"backcolor"				}, MenuParse_backcolor				},
	{{"background"				}, MenuParse_background				},
	{{"border"					}, MenuParse_border					},
	{{"bordercolor"				}, MenuParse_bordercolor			},
	{{"borderSize"				}, MenuParse_borderSize				},
	{{"cinematic"				}, MenuParse_cinematic				},
	{{"descAlignment"			}, MenuParse_descAlignment			},
	{{"desccolor"				}, MenuParse_descColor				},
	{{"descX"					}, MenuParse_descX					},
	{{"descY"					}, MenuParse_descY					},
	{{"disablecolor"			}, MenuParse_disablecolor			},
	{{"fadeAmount"				}, MenuParse_fadeAmount				},
	{{"fadeClamp"				}, MenuParse_fadeClamp				},
	{{"fadeCycle"				}, MenuParse_fadeCycle				},
	{{"focuscolor"				}, MenuParse_focuscolor				},
	{{"forecolor"				}, MenuParse_forecolor				},
	{{"fullscreen"				}, MenuParse_fullscreen				},
	{{"itemDef"					}, MenuParse_itemDef				},
	{{"name"					}, MenuParse_name					},
	{{"onClose"					}, MenuParse_onClose				},
	{{"onESC"					}, MenuParse_onESC					},
	{{"outOfBoundsClick"		}, MenuParse_outOfBounds			},
	{{"onOpen"					}, MenuParse_onOpen					},
	{{"outlinecolor"			}, MenuParse_outlinecolor			},
	{{"ownerdraw"				}, MenuParse_ownerdraw				},
	{{"ownerdrawFlag"			}, MenuParse_ownerdrawFlag			},
	{{"popup"					}, MenuParse_popup					},
	{{"rect"					}, MenuParse_rect					},
	{{"soundLoop"				}, MenuParse_soundLoop				},
	{{"style"					}, MenuParse_style					},
	{{"visible"					}, MenuParse_visible				},
	{{"tooltipforecolor"		}, MenuParse_tooltipforecolor		},
	{{"tooltipbackcolor"		}, MenuParse_tooltipbackcolor		},
	{{"tooltipscale"			}, MenuParse_tooltipscale			},
	{{"tooltipdelay"			}, MenuParse_tooltipdelay			},
	{{"tooltipfont"				}, MenuParse_tooltipfont			},
	{{ NULL,					}, NULL								}
};

keywordHash_t *menuParseKeywordHash[KEYWORDHASH_SIZE];

/*
===============
Menu_SetupKeywordHash
===============
*/
void Menu_SetupKeywordHash(void) 
{
	int i;

	memset(menuParseKeywordHash, 0, sizeof(menuParseKeywordHash));
	for (i = 0; menuParseKeywords[i].keyword.name; i++) 
	{
		// Initialize the collision list
		menuParseKeywords[i].keyword.next = NULL;

		// Add the keyword
		KeywordHash_Add(menuParseKeywordHash, (keywordHash_t*)&menuParseKeywords[i]);
	}
}

/*
===============
Menu_Parse
===============
*/
qboolean Menu_Parse(int handle, menuDef_t *menu) 
{
	pc_token_t		token;
	itemHash_t*		key;

	if (!trap_PC_ReadToken(handle, &token))
	{
		return qfalse;
	}
	
	if (*token.string != '{') 
	{
		return qfalse;
	}
    
	while ( 1 ) 
	{
		memset(&token, 0, sizeof(pc_token_t));

		if (!trap_PC_ReadToken(handle, &token)) 
		{
			PC_SourceError(handle, "end of file inside menu\n");
			return qfalse;
		}

		if (*token.string == '}') 
		{
			return qtrue;
		}

		key = (itemHash_t*)KeywordHash_Find(menuParseKeywordHash, token.string);
		
		if (!key) 
		{
			PC_SourceError(handle, "unknown menu keyword %s", token.string);
			continue;
		}
		
		if ( !key->func((itemDef_t*)menu, handle) ) 
		{
			PC_SourceError(handle, "couldn't parse menu keyword %s", token.string);
			return qfalse;
		}
	}

	// bk001205 - LCC missing return value
	return qfalse; 	
}

/*
===============
Menu_New
===============
*/
void Menu_New(int handle) 
{
	menuDef_t *menu = &Menus[menuCount];

	if (menuCount < MAX_MENUS) 
	{
		Menu_Init(menu);
		
		if (Menu_Parse(handle, menu)) 
		{
			Menu_PostParse(menu);
			menuCount++;
		}
	}
}

int Menu_Count() {
	return menuCount;
}

void Menu_PaintAll() {
	int i;
	if (captureFunc) {
		captureFunc(captureData);
	}

	// Make sure the color is cleared first
	DC->setColor ( NULL );

	for (i = 0; i < Menu_Count(); i++) {
		Menu_Paint(&Menus[i], qfalse);
	}

	if ( DC->tooltipItem )
	{
		Tooltip_Paint ( DC->tooltipItem );
	}

	if (debugMode) {
		vec4_t v = {1, 1, 1, 1};
		DC->drawText(5, 25, DC->Assets.defaultFont, 0.5f, v, va("fps: %f", DC->FPS), 0, 0 );
		DC->drawText(5, 45, DC->Assets.defaultFont, 0.5f, v, va("x: %d  y:%d", DC->cursorx,DC->cursory), 0, 0 );
	}
}

void Menu_Reset() {
	menuCount = 0;
}

displayContextDef_t *Display_GetContext() {
	return DC;
}
 
#ifndef MISSIONPACK // bk001206
static float captureX;
static float captureY;
#endif

void *Display_CaptureItem(int x, int y) {
	int i;

	for (i = 0; i < menuCount; i++) {
		// turn off focus each item
		// menu->items[i].window.flags &= ~WINDOW_HASFOCUS;
		if (Rect_ContainsPoint(&Menus[i].window.rect, x, y)) {
			return &Menus[i];
		}
	}
	return NULL;
}


// FIXME: 
qboolean Display_MouseMove(void *p, int x, int y) {
	int i;
	menuDef_t *menu = p;

	if (menu == NULL) {
    menu = Menu_GetFocused();
		if (menu) {
			if (menu->window.flags & WINDOW_POPUP) {
				Menu_HandleMouseMove(menu, x, y);
				return qtrue;
			}
		}
		for (i = 0; i < menuCount; i++) {
			Menu_HandleMouseMove(&Menus[i], x, y);
		}
	} else {
		menu->window.rect.x += x;
		menu->window.rect.y += y;
		Menu_UpdatePosition(menu);
	}
 	return qtrue;

}

int Display_CursorType(int x, int y) {
	int i;
	for (i = 0; i < menuCount; i++) {
		rectDef_t r2;
		r2.x = Menus[i].window.rect.x - 3;
		r2.y = Menus[i].window.rect.y - 3;
		r2.w = r2.h = 7;
		if (Rect_ContainsPoint(&r2, x, y)) {
			return CURSOR_SIZER;
		}
	}
	return CURSOR_ARROW;
}


void Display_HandleKey(int key, qboolean down, int x, int y) {
	menuDef_t *menu = Display_CaptureItem(x, y);
	if (menu == NULL) {  
		menu = Menu_GetFocused();
	}
	if (menu) 
	{
		Menu_HandleKey(menu, key, down );
	}
}

static void Window_CacheContents(windowDef_t *window) {
	if (window) {
		if (window->cinematicName) {
			int cin = DC->playCinematic(window->cinematicName, 0, 0, 0, 0);
			DC->stopCinematic(cin);
		}
	}
}


static void Item_CacheContents(itemDef_t *item) {
	if (item) {
		Window_CacheContents(&item->window);
	}

}

static void Menu_CacheContents(menuDef_t *menu) {
	if (menu) {
		int i;
		Window_CacheContents(&menu->window);
		for (i = 0; i < menu->itemCount; i++) {
			Item_CacheContents(menu->items[i]);
		}

		if (menu->soundName && *menu->soundName) {
			DC->registerSound(menu->soundName);
		}
	}

}

void Display_CacheAll() {
	int i;
	for (i = 0; i < menuCount; i++) {
		Menu_CacheContents(&Menus[i]);
	}
}


static qboolean Menu_OverActiveItem(menuDef_t *menu, float x, float y) {
 	if (menu && menu->window.flags & (WINDOW_VISIBLE | WINDOW_FORCED)) {
		if (Rect_ContainsPoint(&menu->window.rect, x, y)) {
			int i;
			for (i = 0; i < menu->itemCount; i++) {
				// turn off focus each item
				// menu->items[i].window.flags &= ~WINDOW_HASFOCUS;

				if (!(menu->items[i]->window.flags & (WINDOW_VISIBLE | WINDOW_FORCED))) {
					continue;
				}

				if (menu->items[i]->window.flags & WINDOW_DECORATION) {
					continue;
				}

				if (Rect_ContainsPoint(&menu->items[i]->window.rect, x, y)) {
					itemDef_t *overItem = menu->items[i];
					if (overItem->type == ITEM_TYPE_TEXT && overItem->text) {
						if (Rect_ContainsPoint(Item_CorrectedTextRect(overItem), x, y)) {
							return qtrue;
						} else {
							continue;
						}
					} else {
						return qtrue;
					}
				}
			}

		}
	}
	return qfalse;
}

void Tooltip_Paint ( itemDef_t *item )
{
	float		w;
	float		h;
	float		x;
	float		y;
	menuDef_t	*menu;

	x = DC->tooltipx;
	y = DC->tooltipy;

	// If the item doesnt have a tooltip then dont render one
	if ( NULL == item->tooltip )
	{
		return;
	}

	menu = item->parent;

	// Only render the tooltip if the cursor has been stopped for at 
	// at least tooltipDelay time
	if ( DC->realTime - DC->tooltiptime < menu->tooltipDelay )
		return;

	// TODO: Determine the width and height of the text and adjust the
	//       orientation of the tooltip to ensure it fits on the screen

	w = DC->getTextWidth ( item->tooltip, menu->tooltipFont, menu->tooltipScale, 0 );
	h = DC->getTextHeight ( item->tooltip, menu->tooltipFont, menu->tooltipScale, 0 );

	// Draw the background and border for the tooltip
	DC->fillRect( x, y, w + 6, h + 6, menu->tooltipBackColor );
	DC->drawRect( x, y, w + 6, h + 6, 1, menu->tooltipForeColor );

	// Draw the tooltip text
	DC->drawText( x + 3, y + 2, menu->tooltipFont, menu->tooltipScale, menu->tooltipForeColor, item->tooltip, 0, 0 );
}
