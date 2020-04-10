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

#define ITEM_TYPE_TEXT 0                  // simple text
#define ITEM_TYPE_BUTTON 1                // button, basically text with a border 
#define ITEM_TYPE_RADIOBUTTON 2           // toggle button, may be grouped 
#define ITEM_TYPE_CHECKBOX 3              // check box
#define ITEM_TYPE_EDITFIELD 4             // editable text, associated with a cvar
#define ITEM_TYPE_COMBO 5                 // drop down list
#define ITEM_TYPE_LISTBOX 6               // scrollable list  
#define ITEM_TYPE_MODEL 7                 // model
#define ITEM_TYPE_OWNERDRAW 8             // owner draw, name specs what it is
#define ITEM_TYPE_NUMERICFIELD 9          // editable text, associated with a cvar
#define ITEM_TYPE_SLIDER 10               // mouse speed, volume, etc.
#define ITEM_TYPE_YESNO 11                // yes no cvar setting
#define ITEM_TYPE_MULTI 12                // multiple list setting, enumerated
#define ITEM_TYPE_BIND 13				  // multiple list setting, enumerated
#define ITEM_TYPE_PASSWORDFIELD		14	  // password field
#define ITEM_TYPE_COMBOBOX			15	  // Drop down combo box
#define ITEM_TYPE_TEXTSCROLL		16		  // scrolls text
    
#define ITEM_ALIGN_LEFT 0                 // left alignment
#define ITEM_ALIGN_CENTER 1               // center alignment
#define ITEM_ALIGN_RIGHT 2                // right alignment

#define ITEM_TEXTSTYLE_NORMAL 0           // normal text
#define ITEM_TEXTSTYLE_BLINK 1            // fast blinking
#define ITEM_TEXTSTYLE_PULSE 2            // slow pulsing
#define ITEM_TEXTSTYLE_SHADOWED 3         // drop shadow ( need a color for this )
#define ITEM_TEXTSTYLE_OUTLINED 4         // drop shadow ( need a color for this )
#define ITEM_TEXTSTYLE_OUTLINESHADOWED 5  // drop shadow ( need a color for this )
#define ITEM_TEXTSTYLE_SHADOWEDMORE 6         // drop shadow ( need a color for this )
                          
#define WINDOW_BORDER_NONE 0              // no border
#define WINDOW_BORDER_FULL 1              // full border based on border color ( single pixel )
#define WINDOW_BORDER_HORZ 2              // horizontal borders only
#define WINDOW_BORDER_VERT 3              // vertical borders only 
  
#define WINDOW_STYLE_EMPTY		0         // no background
#define WINDOW_STYLE_FILLED		1         // filled with background color
#define WINDOW_STYLE_SHADER		2         // Shaders
#define WINDOW_STYLE_CINEMATIC	3         // cinematic

#define MENU_TRUE 1                       // uh.. true
#define MENU_FALSE 0                      // and false

#define HUD_VERTICAL				0x00
#define HUD_HORIZONTAL				0x01

// list box element types
#define LISTBOX_TEXT  0x00
#define LISTBOX_IMAGE 0x01

// list feeders
#define FEEDER_HEADS						0			// model heads
#define FEEDER_MAPS							1			// text maps based on game type
#define FEEDER_SERVERS						2			// servers
#define FEEDER_CLANS						3			// clan names
#define FEEDER_ALLMAPS						4			// all maps available, in graphic format
#define FEEDER_VOTEMAPS						5			// all maps available, in graphic format
#define FEEDER_PLAYER_LIST					6			// players
#define FEEDER_TEAMIDENTITIES				7			// team members for team voting
#define FEEDER_MODS							8			// team members for team voting
#define FEEDER_DEMOS 						9			// team members for team voting
#define FEEDER_SCOREBOARD					10			// team members for team voting
#define FEEDER_IDENTITIES					11			// identities
#define FEEDER_SERVERSTATUS					12			// server status
#define FEEDER_FINDPLAYER					13			// find player
#define FEEDER_CINEMATICS					14			// cinematics
#define FEEDER_OUTFITTING_TEMPLATES			16				// templates for outfitting
#define FEEDER_NEWVERSION_WEBSITES			17			// Websites for the newest version of SOF2MP
#define FEEDER_NEWVERSION_TEXT				18			// Notes on the newest version of SOF2MP

#define CG_SHOW_ANYTEAMGAME               0x00000004
#define CG_SHOW_LANPLAYONLY				  0x00000020
#define CG_SHOW_ANYNONTEAMGAME            0x00000100
#define CG_SHOW_2DONLY					  0x00000200

#define CG_SHOW_ONREDTEAM				  0x00000400
#define CG_SHOW_ONBLUETEAM				  0x00000800

#define CG_SHOW_HUD_HEALTH				  0x00020000
#define CG_SHOW_HUD_WEAPONINFO			  0x00040000
#define CG_SHOW_HUD_SNIPERSCOPE			  0x00080000
#define CG_SHOW_HUD_FUNDS				  0x00100000
#define CG_SHOW_HUD_TIMER				  0x00200000
#define CG_SHOW_HUD_NIGHTVISION			  0x00400000
#define CG_SHOW_HUD_THERMAL				  0x00800000
#define	CG_SHOW_PLAYER_ALT_WEAPONINFO	  0x20000000
#define CG_HIDE_PLAYER_ALT_WEAPONINFO	  0x40000000


#define UI_SHOW_LEADER				      0x00000001
#define UI_SHOW_NOTLEADER				  0x00000002
#define UI_SHOW_FAVORITESERVERS			  0x00000004
#define UI_SHOW_ANYNONTEAMGAME			  0x00000008
#define UI_SHOW_ANYTEAMGAME				  0x00000010
#define UI_SHOW_NEWHIGHSCORE			  0x00000020
#define UI_SHOW_DEMOAVAILABLE			  0x00000040
#define UI_SHOW_NEWBESTTIME				  0x00000080
#define UI_SHOW_NETANYNONTEAMGAME	 	  0x00000400
#define UI_SHOW_NETANYTEAMGAME		 	  0x00000800
#define UI_SHOW_NOTFAVORITESERVERS		  0x00001000

// owner draw types
// ideally these should be done outside of this file but
// this makes it much easier for the macro expansion to 
// convert them for the designers ( from the .menu files )
#define CG_OWNERDRAW_BASE				1
										
#define CG_PLAYER_SCORE					2
										
#define CG_BLUE_SCORE					3
#define CG_RED_SCORE					4
#define CG_RED_NAME						5
#define CG_BLUE_NAME					6
#define CG_PLAYER_LOCATION				8
#define CG_TEAM_COLOR					9                                       
#define CG_GAME_TYPE					10
#define CG_GAME_STATUS					11
#define CG_1STPLACE						13
#define CG_2NDPLACE						14
#define CG_WEAPON_LIST					15

#define	CG_PLAYER_HEALTH				16
#define CG_PLAYER_HEALTH_FADE			17
#define CG_PLAYER_ARMOR					18
#define CG_PLAYER_ARMOR_FADE			19
#define CG_PLAYER_WEAPON_NAME			20
#define CG_PLAYER_WEAPON_AMMO			21
#define CG_PLAYER_WEAPON_CLIP			22
#define CG_PLAYER_WEAPON_MODE			23
#define	CG_PLAYER_ALT_WEAPON_NAME		24
#define CG_PLAYER_ALT_WEAPON_AMMO		25
#define CG_PLAYER_ALT_WEAPON_AMMOICON	26

#define CG_TEAMINFO						27

#define CG_PLAYER_SNIPER_BULLET_1		28
#define CG_PLAYER_SNIPER_BULLET_2		29
#define CG_PLAYER_SNIPER_BULLET_3		30
#define CG_PLAYER_SNIPER_BULLET_4		31
#define CG_PLAYER_SNIPER_BULLET_5		32
#define CG_PLAYER_SNIPER_BULLET_6		33

#define	CG_USE_ICON						34
#define	CG_PLAYER_GAMETYPE_ITEMS		35
#define CG_PLAYER_SNIPER_MAGNIFICATION	36

#define UI_OWNERDRAW_BASE 200
#define UI_EFFECTS 201
#define UI_PLAYERMODEL 202
#define UI_GAMETYPE 205
#define UI_MAPPREVIEW 206
#define UI_SKILL 207
#define UI_NETSOURCE 220
#define UI_NETMAPPREVIEW 221
#define UI_NETFILTER 222
#define UI_ALLMAPS_SELECTION 236
#define UI_VOTE_KICK 238
#define UI_BOTNAME 239
#define UI_BOTSKILL 240
#define UI_REDBLUE 241
#define UI_CROSSHAIR 242
#define UI_SELECTEDPLAYER 243
#define UI_MAPCINEMATIC 244
#define UI_NETGAMETYPE 245
#define UI_NETMAPCINEMATIC 246
#define UI_SERVERREFRESHDATE 247
#define UI_SERVERMOTD 248
#define UI_GLINFO  249
#define UI_MAP_TIMETOBEAT 252
#define UI_JOINGAMETYPE 253
#define UI_PREVIEWCINEMATIC 254
#define UI_STARTMAPCINEMATIC 255
#define UI_MAPS_SELECTION 256
#define UI_VERSIONDOWNLOAD_PROGRESS 257

#define UI_REDTEAM_IDENTITY				260
#define UI_BLUETEAM_IDENTITY			261

#define UI_OUTFITTING_SLOT				270
#define UI_OUTFITTING_SLOT_RENDER		271
#define UI_OUTFITTING_SLOT_NAME			272
#define UI_OUTFITTING_SLOT_COST			273
#define UI_OUTFITTING_SLOT_BACKGROUND	274

#define UI_OBJECTIVE_PHOTOS				275

#define VOICECHAT_GETFLAG			"getflag"				// command someone to get the flag
#define VOICECHAT_OFFENSE			"offense"				// command someone to go on offense
#define VOICECHAT_DEFEND			"defend"				// command someone to go on defense
#define VOICECHAT_DEFENDFLAG		"defendflag"			// command someone to defend the flag
#define VOICECHAT_PATROL			"patrol"				// command someone to go on patrol (roam)
#define VOICECHAT_CAMP				"camp"					// command someone to camp (we don't have sounds for this one)
#define VOICECHAT_FOLLOWME			"followme"				// command someone to follow you
#define VOICECHAT_RETURNFLAG		"returnflag"			// command someone to return our flag
#define VOICECHAT_FOLLOWFLAGCARRIER	"followflagcarrier"		// command someone to follow the flag carrier
#define VOICECHAT_YES				"yes"					// yes, affirmative, etc.
#define VOICECHAT_NO				"no"					// no, negative, etc.
#define VOICECHAT_ONGETFLAG			"ongetflag"				// I'm getting the flag
#define VOICECHAT_ONOFFENSE			"onoffense"				// I'm on offense
#define VOICECHAT_ONDEFENSE			"ondefense"				// I'm on defense
#define VOICECHAT_ONPATROL			"onpatrol"				// I'm on patrol (roaming)
#define VOICECHAT_ONCAMPING			"oncamp"				// I'm camping somewhere
#define VOICECHAT_ONFOLLOW			"onfollow"				// I'm following
#define VOICECHAT_ONFOLLOWCARRIER	"onfollowcarrier"		// I'm following the flag carrier
#define VOICECHAT_ONRETURNFLAG		"onreturnflag"			// I'm returning our flag
#define VOICECHAT_INPOSITION		"inposition"			// I'm in position
#define VOICECHAT_IHAVEFLAG			"ihaveflag"				// I have the flag
#define VOICECHAT_BASEATTACK		"baseattack"			// the base is under attack
#define VOICECHAT_ENEMYHASFLAG		"enemyhasflag"			// the enemy has our flag (CTF)
#define VOICECHAT_STARTLEADER		"startleader"			// I'm the leader
#define VOICECHAT_STOPLEADER		"stopleader"			// I resign leadership
#define VOICECHAT_TRASH				"trash"					// lots of trash talk
#define VOICECHAT_WHOISLEADER		"whoisleader"			// who is the team leader
#define VOICECHAT_WANTONDEFENSE		"wantondefense"			// I want to be on defense
#define VOICECHAT_WANTONOFFENSE		"wantonoffense"			// I want to be on offense
#define VOICECHAT_KILLINSULT		"kill_insult"			// I just killed you
#define VOICECHAT_TAUNT				"taunt"					// I want to taunt you
#define VOICECHAT_DEATHINSULT		"death_insult"			// you just killed me
#define VOICECHAT_KILLGAUNTLET		"kill_gauntlet"			// I just killed you with the gauntlet
#define VOICECHAT_PRAISE			"praise"				// you did something good

#define OUTFITTING_GROUP_PRIMARY	0
#define OUTFITTING_GROUP_SECONDARY	1
#define OUTFITTING_GROUP_PISTOL		2
#define OUTFITTING_GROUP_GRENADE	3
#define OUTFITTING_GROUP_ACCESSORY	4
#define OUTFITTING_GROUP_MAX		5
#define OUTFITTING_GROUP_KNIFE		6
