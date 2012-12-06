// Copyright (C) 2001-2002 Raven Software
//
// bg_misc.c -- both games misc functions, all completely stateless

#include "q_shared.h"
#include "bg_public.h"

#ifdef QAGAME
#include "g_local.h"
#include "boe_local.h"
#endif

#ifdef UI_EXPORTS
#include "../ui/ui_local.h"
#endif

#ifndef UI_EXPORTS
#ifndef QAGAME
#include "../cgame/cg_local.h"
#endif
#endif

/*QUAKED item_***** ( 0 0 0 ) (-16 -16 -16) (16 16 16) suspended
DO NOT USE THIS CLASS, IT JUST HOLDS GENERAL INFORMATION.
The suspended flag will allow items to hang in the air, otherwise they are dropped to the next surface.

If an item is the target of another entity, it will not spawn in until fired.

An item fires all of its targets when it is picked up.  If the toucher can't carry it, the targets won't be fired.

"notfree" if set to 1, don't spawn in free for all games
"notteam" if set to 1, don't spawn in team games
"notsingle" if set to 1, don't spawn in single player games
"wait"	override the default wait before respawning.  -1 = never respawn automatically, which can be used with targeted spawning.
"random" random number of plus or minus seconds varied from the respawn time
"count" override quantity or duration on most items.
*/

gitem_t	bg_itemlist[] = 
{
	{
		NULL,				// classname	
		NULL,				// pickup_sound
		{	NULL,			// world_model[0]
			NULL,			// world_model[1]
			0, 0} ,			// world_model[2],[3]
/* icon */		NULL,		// icon
				NULL,
				NULL,
/* pickup */	NULL,		// pickup_name
		0,					// quantity
		(itemType_t)0,					// giType (IT_*)
		0,					// giTag
/* precache */ "",			// precaches
/* sounds */ ""				// sounds
	},	// leave index 0 alone

	//
	// Pickups
	//

/*QUAKED pickup_armor_big (0 .6 .6) (-15 -15 -15) (15 15 15) 
*/
	{
		"pickup_armor_big", 
		"sound/player/pickup/armour.wav",
        { "models/pick_ups/armor_large.md3",
		0, 0, 0},
/* icon */		"gfx/menus/hud/weapon_icons/armor_large_icon",
				"",
				"some",
/* pickup */	"Heavy Armor",
		75,
		IT_ARMOR,
		0,
/* precache */ "",
/* sounds */ "",

	},

/*QUAKED pickup_armor_medium (0 .6 .6) (-15 -15 -15) (15 15 15) 
*/
	{
		"pickup_armor_medium", 
		"sound/player/pickup/armour.wav",
        { "models/pick_ups/armor_medium.md3",
		0, 0, 0},
/* icon */		"gfx/menus/hud/weapon_icons/armor_medium_icon",
				"",
				"some",
/* pickup */	"Medium Armor",
		50,
		IT_ARMOR,
		0,
/* precache */ "",
/* sounds */ "",

	},

/*QUAKED pickup_armor_small (0 .6 .6) (-15 -15 -15) (15 15 15) 
*/	
	{
		"pickup_armor_small", 
		"sound/player/pickup/armour.wav",
        { "models/pick_ups/armor_small.md3",
		0, 0, 0},
/* icon */		"gfx/menus/hud/weapon_icons/armor_small_icon",
				"",
				"some",
/* pickup */	"Small Armor",
		25,
		IT_ARMOR,
		0,
/* precache */ "",
/* sounds */ "",

	},

/*QUAKED pickup_health_big (0 .6 .6) (-15 -15 -15) (15 15 15) 
*/
	{
		"pickup_health_big",
		"sound/player/pickup/health.wav",
        { "models/pick_ups/health_lrg.md3", 
		0, 0, 0 },
/* icon */		"gfx/menus/hud/weapon_icons/health_large_icon",
				"",
				"a",
/* pickup */	"Large Health",
		100,
		IT_HEALTH,
		0,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED pickup_health_small (0 .6 .6) (-15 -15 -15) (15 15 15) 
*/
	{
		"pickup_health_small",
		"sound/player/pickup/health.wav",
        { "models/pick_ups/health_smll.md3", 
		0, 0, 0 },
/* icon */		"gfx/menus/hud/weapon_icons/health_small_icon",
				"",
				"a",
/* pickup */	"Small Health",
		25,
		IT_HEALTH,
		0,
/* precache */ "",
/* sounds */ ""
	},

	//
	// ITEMS
	//


	//
	// WEAPONS 
	//

/*Q U A K E D weapon_knife (.3 .3 1) (-15 -15 -15) (15 15 15) suspended
Don't place this
*/
	{
		"weapon_knife", 
		"sound/weapons/knife/ready.wav",
        { "models/weapons/knife/world/knifeworld.glm",
		0, 0, 0},
/* icon */		"gfx/menus/hud/weapon_icons/knife_icon",
				"*gfx/menus/weapon_renders/knife",
				"a",
/* pickup */	"Knife",
		-1,
		IT_WEAPON,
		WP_KNIFE,
/* precache */ "",
/* sounds */ "",

		OUTFITTING_GROUP_KNIFE
	},

/*QUAKED pickup_weapon_US_SOCOM (0 .6 .6) (-15 -15 -15) (15 15 15) 
Pistol, uses 45 rounds
*/
	{
		"pickup_weapon_US_SOCOM", 
		"sound/player/pickup/weapon.wav",
        { "models/weapons/ussocom/world/ussocomworld.glm", 
		0, 0, 0},
/* icon */		"gfx/menus/hud/weapon_icons/ussocom_icon",
				"*gfx/menus/weapon_renders/ussocom_mp",
				"a",
/* pickup */	"US SOCOM",
		10,
		IT_WEAPON,
		WP_USSOCOM_PISTOL,
/* precache */ "",
/* sounds */ "",

		OUTFITTING_GROUP_PISTOL,
	},

/*QUAKED pickup_weapon_M19 (0 .6 .6) (-15 -15 -15) (15 15 15) 
Pistol, uses 45 rounds
*/
	{
		"pickup_weapon_M19", 
		"sound/player/pickup/weapon.wav",
        { "models/weapons/m1911a1/world/m1911a1world.glm", 
		0, 0, 0},
/* icon */		"gfx/menus/hud/weapon_icons/m1911a1_icon",
				"*gfx/menus/weapon_renders/m1911a1",
				"a",
/* pickup */	"M1911A1",
		12,
		IT_WEAPON,
		WP_M1911A1_PISTOL,
/* precache */ "",
/* sounds */ "",

		OUTFITTING_GROUP_PISTOL,
	},

/*QUAKED pickup_weapon_microuzi (0 .6 .6) (-15 -15 -15) (15 15 15) 
Sub-Machinegun, uses 9mm rounds
*/
	{
		"pickup_weapon_microuzi", 
		"sound/player/pickup/weapon.wav",
        { "models/weapons/microuzi/world/microuziworld.glm", 
		0, 0, 0},
/* icon */		"gfx/menus/hud/weapon_icons/microuzi_icon",
				"*gfx/menus/weapon_renders/microuzi",
				"a",
/* pickup */	"MicroUzi",
		30,
		IT_WEAPON,
		WP_MICRO_UZI_SUBMACHINEGUN,
/* precache */ "",
/* sounds */ "",

		OUTFITTING_GROUP_SECONDARY,
	},

/*QUAKED pickup_weapon_M3A1 (0 .6 .6) (-15 -15 -15) (15 15 15) 
Sub-Machinegun, uses 45 rounds
*/
	{
		"pickup_weapon_M3A1", 
		"sound/player/pickup/weapon.wav",
        { "models/weapons/m3a1/world/m3a1world.glm", 
		0, 0, 0},
/* icon */		"gfx/menus/hud/weapon_icons/m3a1_icon",
				"gfx/menus/weapon_renders/m3a1",
				"a",
/* pickup */	"M3A1 Sub-machinegun",
		30,
		IT_WEAPON,
		WP_M3A1_SUBMACHINEGUN,
/* precache */ "",
/* sounds */ "",

		OUTFITTING_GROUP_SECONDARY,
	},

/*QUAKED pickup_weapon_USAS_12 (0 .6 .6) (-15 -15 -15) (15 15 15) 
Shotgun, uses 12-gauge rounds
ammo ---------- amount of ammo (defaults to 10)
*/
	{
		"pickup_weapon_USAS_12", 
		"sound/player/pickup/weapon.wav",
        { "models/weapons/usas12/world/usas12world.glm", 
		0, 0, 0},
/* icon */		"gfx/menus/hud/weapon_icons/usas12_icon",
				"gfx/menus/weapon_renders/usas12",
				"a",
/* pickup */	"USAS12 SHOTGUN",
		30,
		IT_WEAPON,
		WP_USAS_12_SHOTGUN,
/* precache */ "",
/* sounds */ "",
		
		OUTFITTING_GROUP_PRIMARY,
	},

/*QUAKED pickup_weapon_M590 (0 .6 .6) (-15 -15 -15) (15 15 15) 
Shotgun, uses 12-gauge rounds
*/
	{
		"pickup_weapon_M590", 
		"sound/player/pickup/weapon.wav",
        { "models/weapons/m590/world/m590world.glm", 
		0, 0, 0},
/* icon */		"gfx/menus/hud/weapon_icons/m590_icon",
				"gfx/menus/weapon_renders/m590",
				"a",
/* pickup */	"M590",
		30,
		IT_WEAPON,
		WP_M590_SHOTGUN,
/* precache */ "",
/* sounds */ "",

		OUTFITTING_GROUP_SECONDARY,
	},

/*QUAKED pickup_weapon_MSG90A1 (0 .6 .6) (-15 -15 -15) (15 15 15) 
Sniper Rifle, uses 7.62 rounds
*/
	{
		"pickup_weapon_MSG90A1", 
		"sound/player/pickup/weapon.wav",
        { "models/weapons/msg90a1/world/msg90a1world.glm", 
		0, 0, 0},
/* icon */		"gfx/menus/hud/weapon_icons/msg90a1_icon",
				"gfx/menus/weapon_renders/msg90a1",
				"a",
/* pickup */	"MSG90A1 Sniper",
		30,
		IT_WEAPON,
		WP_MSG90A1,
/* precache */ "",
/* sounds */ "",

		OUTFITTING_GROUP_PRIMARY,
	},

/*QUAKED pickup_weapon_M4 (0 .6 .6) (-15 -15 -15) (15 15 15) 
Assault Rifle, uses 5.56 rounds and 40mm grenades
*/
	{
		"pickup_weapon_M4", 
		"sound/player/pickup/weapon.wav",
        { "models/weapons/m4/world/m4world.glm", 
		0, 0, 0},
/* icon */		"gfx/menus/hud/weapon_icons/m4_icon",
				"gfx/menus/weapon_renders/m4_m203",
				"a",
/* pickup */	"M4 Assault",
		30,
		IT_WEAPON,
		WP_M4_ASSAULT_RIFLE,
/* precache */ "",
/* sounds */ "",

		OUTFITTING_GROUP_PRIMARY,
	},

/*QUAKED pickup_weapon_AK_74 (0 .6 .6) (-15 -15 -15) (15 15 15) 
Assault Rifle, uses 5.56 rounds
*/
	{
		"pickup_weapon_AK_74", 
		"sound/player/pickup/weapon.wav",
        { "models/weapons/ak74/world/ak74world.glm", 
		0, 0, 0},
/* icon */		"gfx/menus/hud/weapon_icons/ak74_icon",
				"gfx/menus/weapon_renders/ak74",
				"an",
/* pickup */	"AK74 Assault",
		30,
		IT_WEAPON,
		WP_AK74_ASSAULT_RIFLE,
/* precache */ "",
/* sounds */ "",

		OUTFITTING_GROUP_PRIMARY,
	},

/*QUAKED pickup_weapon_M60 (0 .6 .6) (-15 -15 -15) (15 15 15) 
Machinegun, uses 7.62 rounds
*/
	{
		"pickup_weapon_M60", 
		"sound/player/pickup/weapon.wav",
        { "models/weapons/m60/world/m60world.glm", 
		0, 0, 0},
/* icon */		"gfx/menus/hud/weapon_icons/m60_icon",
				"gfx/menus/weapon_renders/m60",
				"a",
/* pickup */	"M60 Machinegun",
		30,
		IT_WEAPON,
		WP_M60_MACHINEGUN,
/* precache */ "",
/* sounds */ "",

		OUTFITTING_GROUP_PRIMARY,
	},

/*QUAKED pickup_weapon_RPG_7 (0 .6 .6) (-15 -15 -15) (15 15 15) 
RPG, uses 40mm rounds
*/
	{
		"pickup_weapon_RPG_7", 
		"sound/player/pickup/weapon.wav",
        { "models/weapons/rpg7/world/rpg7world.glm", 
		0, 0, 0},
/* icon */		"gfx/menus/hud/weapon_icons/rpg7_icon",
				"gfx/menus/weapon_renders/rpg7",
				"a",
/* pickup */	"RPG-7",
		10,
		IT_WEAPON,
		WP_RPG7_LAUNCHER,
/* precache */ "",
/* sounds */ "",
		
		OUTFITTING_GROUP_PRIMARY,
	},

/*QUAKED pickup_weapon_MM_1 (0 .6 .6) (-15 -15 -15) (15 15 15) 
Grenade Launcher, uses 40mm rounds
*/
	{
		"pickup_weapon_MM_1", 
		"sound/player/pickup/weapon.wav",
        { "models/weapons/mm1/world/mm1world.glm", 
		0, 0, 0},
/* icon */		"gfx/menus/hud/weapon_icons/mm1_icon",
				"gfx/menus/weapon_renders/mm1",
				"a",
/* pickup */	"MM1 Grenade Launcher",
		10,
		IT_WEAPON,
		WP_MM1_GRENADE_LAUNCHER,
/* precache */ "",
/* sounds */ "",

		OUTFITTING_GROUP_PRIMARY,
	},

/*QUAKED pickup_weapon_M67 (0 .6 .6) (-15 -15 -15) (15 15 15) 
Grenade
*/
	{
		"pickup_weapon_M67", 
		"sound/player/pickup/weapon.wav",
        { "models/weapons/m67/world/m67world.glm", 
		0, 0, 0},
/* icon */		"gfx/menus/hud/weapon_icons/m67_icon",
				"*gfx/menus/weapon_renders/m67",
				"a",
/* pickup */	"M67 grenade",
		5,
		IT_WEAPON,
		WP_M67_GRENADE,
/* precache */ "",
/* sounds */ "",

		OUTFITTING_GROUP_GRENADE,
	},

/*QUAKED pickup_weapon_M84 (0 .6 .6) (-15 -15 -15) (15 15 15) 
Grenade
*/
	{
		"pickup_weapon_M84", 
		"sound/player/pickup/weapon.wav",
        { "models/weapons/m84/world/m84world.glm", 
		0, 0, 0},
/* icon */		"gfx/menus/hud/weapon_icons/m84_icon",
				"*gfx/menus/weapon_renders/m84",
				"a",
/* pickup */	"M84 Flash",
		5,
		IT_WEAPON,
		WP_M84_GRENADE,
/* precache */ "",
/* sounds */ "",

		OUTFITTING_GROUP_GRENADE,
	},

/*QUAKED pickup_weapon_F1 (0 .6 .6) (-15 -15 -15) (15 15 15) 
Grenade
*/
	{
		"pickup_weapon_F1", 
		"sound/player/pickup/weapon.wav",
        { "models/weapons/f1/world/f1world.glm", 
		0, 0, 0},
/* icon */		"gfx/menus/hud/weapon_icons/f1_icon",
				"*gfx/menus/weapon_renders/m84",
				"a",
/* pickup */	"F1 Frag",
		5,
		IT_WEAPON,
		WP_F1_GRENADE,
/* precache */ "",
/* sounds */ "",

		OUTFITTING_GROUP_GRENADE,
	},

/*QUAKED pickup_weapon_L2A2 (0 .6 .6) (-15 -15 -15) (15 15 15) 
Grenade
*/
	{
		"pickup_weapon_L2A2", 
		"sound/player/pickup/weapon.wav",
        { "models/weapons/l2a2/world/l2a2world.glm", 
		0, 0, 0},
/* icon */		"gfx/menus/hud/weapon_icons/l2a2_icon",
				"*gfx/menus/weapon_renders/m84",
				"a",
/* pickup */	"L2A2 grenade",
		5,
		IT_WEAPON,
		WP_L2A2_GRENADE,
/* precache */ "",
/* sounds */ "",

		OUTFITTING_GROUP_GRENADE,
	},

/*QUAKED pickup_weapon_MDN11 (0 .6 .6) (-15 -15 -15) (15 15 15) 
Grenade
*/
	{
		"pickup_weapon_MDN11", 
		"sound/player/pickup/weapon.wav",
        { "models/weapons/mdn11/world/mdn11world.glm", 
		0, 0, 0},
/* icon */		"gfx/menus/hud/weapon_icons/mdn11_icon",
				"*gfx/menus/weapon_renders/mdn11",
				"a",
/* pickup */	"MDN11 grenade",
		5,
		IT_WEAPON,
		WP_MDN11_GRENADE,
/* precache */ "",
/* sounds */ "",

		OUTFITTING_GROUP_GRENADE,
	},

/*QUAKED pickup_weapon_SMOHG92 (0 .6 .6) (-15 -15 -15) (15 15 15) 
Grenade
*/
	{
		"pickup_weapon_SMOHG92", 
		"sound/player/pickup/weapon.wav",
        { "models/weapons/smohg92/world/smohg92world.glm", 
		0, 0, 0},
/* icon */		"gfx/menus/hud/weapon_icons/smohg92_icon",
				"*gfx/menus/weapon_renders/smohg92",
				"a",
/* pickup */	"SMOHG92 Frag",
		5,
		IT_WEAPON,
		WP_SMOHG92_GRENADE,
/* precache */ "",
/* sounds */ "",

		OUTFITTING_GROUP_GRENADE,
	},

/*QUAKED pickup_weapon_AN_M14 (0 .6 .6) (-15 -15 -15) (15 15 15) 
Incendiary Grenade
*/
	{
		"pickup_weapon_AN_M14", 
		"sound/player/pickup/weapon.wav",
        { "models/weapons/anm14/world/anm14world.glm", 
		0, 0, 0},
/* icon */		"gfx/menus/hud/weapon_icons/anm14_icon",
				"*gfx/menus/weapon_renders/anm14",
				"an",
/* pickup */	"ANM14 Incendiary",
		5,
		IT_WEAPON,
		WP_ANM14_GRENADE,
/* precache */ "",
/* sounds */ "",

		OUTFITTING_GROUP_GRENADE,
	},

/*QUAKED pickup_weapon_M15 (0 .6 .6) (-15 -15 -15) (15 15 15) 
White Phosphorus Grenade
*/
	{
		"pickup_weapon_M15", 
		"sound/player/pickup/weapon.wav",
        { "models/weapons/m15/world/m15world.glm", 
		0, 0, 0},
/* icon */		"gfx/menus/hud/weapon_icons/m15_icon",
				"*gfx/menus/weapon_renders/m15",
				"a",
/* pickup */	"M15 Smoke",
		5,
		IT_WEAPON,
		WP_M15_GRENADE,
/* precache */ "",
/* sounds */ "",

		OUTFITTING_GROUP_GRENADE,
	},

	//
	// AMMO ITEMS
	//

/*QUAKED pickup_ammo_45 (0 .6 .6) (-15 -15 -15) (15 15 15) 
*/
	{
		"pickup_ammo_45",
		"sound/player/pickup/ammo.wav",
        { "models/pick_ups/ammo_45_smll.md3", 
		0, 0, 0},
/* icon */		"gfx/menus/hud/weapon_icons/ammo_45_icon",
				"",
				"some",
/* pickup */	"0.45 ACP Ammo",
		30,
		IT_AMMO,
		AMMO_045,
/* precache */ "",
/* sounds */ "",
	},

/*QUAKED pickup_ammo_9mm (0 .6 .6) (-15 -15 -15) (15 15 15) 
*/
	{
		"pickup_ammo_9mm",
		"sound/player/pickup/ammo.wav",
        { "models/pick_ups/ammo_9mm_smll.md3", 
		0, 0, 0},
/* icon */		"gfx/menus/hud/weapon_icons/ammo_9mm_icon",
				"",
				"some",
/* pickup */	"9mm Ammo",
		30,
		IT_AMMO,
		AMMO_9,
/* precache */ "",
/* sounds */ "",
	},

/*QUAKED pickup_ammo_12gauge (0 .6 .6) (-15 -15 -15) (15 15 15) 
*/
	{
		"pickup_ammo_12gauge",
		"sound/player/pickup/ammo.wav",
        { "models/pick_ups/ammo_shotgun_smll.md3", 
		0, 0, 0},
/* icon */		"gfx/menus/hud/weapon_icons/ammo_shotgun_icon",
				"",
				"some",
/* pickup */	"Shotgun Ammo",
		10,
		IT_AMMO,
		AMMO_12,
/* precache */ "",
/* sounds */ "",
	},

/*QUAKED pickup_ammo_762 (0 .6 .6) (-15 -15 -15) (15 15 15) 
*/
	{
		"pickup_ammo_762",
		"sound/player/pickup/ammo.wav",
        { "models/pick_ups/ammo_762_smll.md3", 
		0, 0, 0},
/* icon */		"gfx/menus/hud/weapon_icons/ammo_762_icon",
				"",
				"some",
/* pickup */	"7.62mm Ammo",
		30,
		IT_AMMO,
		AMMO_762,
/* precache */ "",
/* sounds */ "",
	},

/*QUAKED pickup_ammo_556 (0 .6 .6) (-15 -15 -15) (15 15 15) 
*/
	{
		"pickup_ammo_556",
		"sound/player/pickup/ammo.wav",
        { "models/pick_ups/ammo_556_smll.md3", 
		0, 0, 0},
/* icon */		"gfx/menus/hud/weapon_icons/ammo_556_icon",
				"",
				"some",
/* pickup */	"5.56mm Ammo",
		30,
		IT_AMMO,
		AMMO_556,
/* precache */ "",
/* sounds */ "",
	},

/*QUAKED pickup_ammo_40mm (0 .6 .6) (-15 -15 -15) (15 15 15) 
*/
	{
		"pickup_ammo_40mm",
		"sound/player/pickup/ammo.wav",
        { "models/pick_ups/ammo_40_smll.md3", 
		0, 0, 0},
/* icon */		"gfx/menus/hud/weapon_icons/ammo_40mm_icon",
				"",
				"some",
/* pickup */	"40mm Grenade Ammo",
		5,
		IT_AMMO,
		AMMO_40,
/* precache */ "",
/* sounds */ "",
	},


/*QUAKED pickup_ammo_rpg7 (0 .6 .6) (-15 -15 -15) (15 15 15) 
*/
	{
		"pickup_ammo_rpg7",
		"sound/player/pickup/ammo.wav",
        { "models/pick_ups/ammo_rpg7_smll.md3", 
		0, 0, 0},
/* icon */		"gfx/menus/hud/weapon_icons/ammo_rpg_icon",
				"",
				"some",
/* pickup */	"RPG7 Ammo",
		5,
		IT_AMMO,
		AMMO_RPG7,
/* precache */ "",
/* sounds */ "",
	},


/*QUAKED pickup_backpack (0 .6 .6) (-15 -15 -15) (15 15 15) 
*/
	{	// just a temp place holder
		"pickup_backpack",
		"sound/player/pickup/health.wav",
        { "models/pick_ups/mp_universal_pickup.md3", 
		0, 0, 0},
/* icon */		"gfx/menus/hud/weapon_icons/mp_universal_pickup",
				"",
				"a ",
/* pickup */	"Backpack",
		0,
		IT_BACKPACK,
		0,
/* precache */ "",
/* sounds */ "",
	},

	{
		"gametype_item_1",
		NULL,
		{ 0 },
		"",
		"",
		"the",
		"",
		0,
		IT_GAMETYPE,
		0,
		"",
		""
	},

	{
		"gametype_item_2",
		NULL,
		{ 0 },
		"",
		"",
		"the",
		"",
		0,
		IT_GAMETYPE,
		1,
		"",
		""
	},

	{
		"gametype_item_3",
		NULL,
		{ 0 },
		"",
		"",
		"the",
		"",
		0,
		IT_GAMETYPE,
		2,
		"",
		""
	},

	{
		"gametype_item_4",
		NULL,
		{ 0 },
		"",
		"",
		"the",
		"",
		0,
		IT_GAMETYPE,
		3,
		"",
		""
	},

	{
		"gametype_item_5",
		NULL,
		{ 0 },
		"",
		"",
		"the",
		"",
		0,
		IT_GAMETYPE,
		4,
		"",
		""
	},

	{
		"armor",
		"sound/player/pickup/ammo.wav",
        { 0 },
		"gfx/menus/hud/weapon_icons/armor_icon",
		"*gfx/menus/weapon_renders/armor",
		"some",
		"Armor",
		5,
		IT_PASSIVE,
		0,
		"",
		"",
		
		OUTFITTING_GROUP_ACCESSORY
	},

	{
		"venhance_night_vision",
		"sound/player/pickup/ammo.wav",
        { 0 },
		"gfx/menus/hud/weapon_icons/nightvision_icon",
		"*gfx/menus/weapon_renders/nightvision",
		"some",
		"NV. Goggles",
		5,
		IT_PASSIVE,
		0,
		"",
		"",

		OUTFITTING_GROUP_ACCESSORY
	},

	{
		"venhance_thermal",
		"sound/player/pickup/ammo.wav",
        { 0 },
		"gfx/menus/hud/weapon_icons/thermal_icon",
		"*gfx/menus/weapon_renders/thermal",
		"some",
		"Thermal Goggles",
		5,
		IT_PASSIVE,
		0,
		"",
		"",

		OUTFITTING_GROUP_ACCESSORY
	},

	// end of list marker
	{NULL}
};

int		bg_numItems = sizeof(bg_itemlist) / sizeof(bg_itemlist[0]) - 1;


/*
===============
BG_FindWeaponItem
===============
*/
gitem_t	*BG_FindWeaponItem ( weapon_t weapon ) 
{
	gitem_t	*it;
	
	for ( it = bg_itemlist + 1 ; it->classname ; it++) 
	{
		if ( it->giType == IT_WEAPON && it->giTag == weapon ) 
		{
			return it;
		}
	}

	Com_Error( ERR_DROP, "Couldn't find item for weapon %i", weapon);
	return NULL;
}

/*
===============
BG_FindItem
===============
*/
gitem_t	*BG_FindItem( const char *pickupName ) 
{
	gitem_t	*it;
	
	for ( it = bg_itemlist + 1 ; it->classname ; it++ ) 
	{
		if ( !Q_stricmp( it->pickup_name, pickupName ) )
		{
			return it;
		}
	}

	return NULL;
}
/*
===============
BG_FindClassnameItem
===============
*/
gitem_t	*BG_FindClassnameItem ( const char *classname ) 
{
	gitem_t	*it;
	
	for ( it = bg_itemlist + 1 ; it->classname ; it++ ) 
	{
		if ( !Q_stricmp( it->classname, classname ) )
		{
			return it;
		}
	}

	return NULL;
}

/*
============
BG_PlayerTouchesItem

Items can be picked up without actually touching their physical bounds to make
grabbing them easier
============
*/
qboolean BG_PlayerTouchesItem( playerState_t *ps, entityState_t *item, int atTime ) 
{
	vec3_t	origin;

	BG_EvaluateTrajectory( &item->pos, atTime, origin );

	// we are ignoring ducked differences here
	if ( ps->origin[0] - origin[0] > 36   || 
		 ps->origin[0] - origin[0] < -36  ||
		 ps->origin[1] - origin[1] > 36   ||
		 ps->origin[1] - origin[1] < -36  ||
		 ps->origin[2] - origin[2] > 55   ||
		 ps->origin[2] - origin[2] < -50     ) 
	{
		return qfalse;
	}

	return qtrue;
}

/*
================
BG_CanItemBeGrabbed

Returns false if the item should not be picked up.
This needs to be the same for client side prediction and server use.
================
*/
qboolean BG_CanItemBeGrabbed( int gametype, const entityState_t *ent, const playerState_t *ps ) 
{
	gitem_t	*item;

	if ( ent->modelindex < 1 || ent->modelindex >= bg_numItems ) 
	{
		Com_Error( ERR_DROP, "BG_CanItemBeGrabbed: index out of range" );
	}

	item = &bg_itemlist[ent->modelindex];

	// Can the item be picked up yet?
	if ( ent->eFlags & EF_NOPICKUP )
	{
		return qfalse;
	}

	switch( item->giType ) 
	{
		case IT_WEAPON:

			// See if this player is under limited inventory restrictions.  The truth is that
			// all players on the server will be under the same restrictions, but by doing it this
			// way its easy to get info to the bg-code.
			if ( ps->pm_flags & PMF_LIMITED_INVENTORY && current_gametype.value != GT_HZ )
			{
				int			primary   = 0;
				int			secondary = 0;
				int	weapon;

				switch ( item->outfittingGroup )
				{
					case OUTFITTING_GROUP_PRIMARY:
						primary++;
						break;

					case OUTFITTING_GROUP_SECONDARY:
						secondary++;
						break;

					// We only have restrictions on primary and secondary items
					default:
						return qtrue;
				}

				for ( weapon = WP_KNIFE + 1; weapon < WP_NUM_WEAPONS; weapon ++ )
				{
					gitem_t* witem;

					if ( !( ps->stats[STAT_WEAPONS] & (1<<weapon) ) )
					{
						continue;
					}

					witem = BG_FindWeaponItem ( (weapon_t)weapon );
					if ( !witem )
					{
						continue;
					}
					
					switch ( witem->outfittingGroup )
					{
						case OUTFITTING_GROUP_PRIMARY:
							primary++;
							break;

						case OUTFITTING_GROUP_SECONDARY:
							secondary++;
							break;
					}
				}

				// Cant hold 2 of either primary or secondary
				if ( primary > 1 || secondary > 1 )
				{
					return qfalse;
				}

				return qtrue;
			}

			// If you dont have it then pick it up
			if ( !(ps->stats[STAT_WEAPONS] & (1<<item->giTag) ) )
			{
				return qtrue;			
			}

			// If you already have the weapon and have full ammo, then dont pick it up
			if ( ps->ammo[weaponData[item->giTag].attack[ATTACK_NORMAL].ammoIndex] >= ammoData[weaponData[item->giTag].attack[ATTACK_NORMAL].ammoIndex].max) 
			{
				if ( BG_WeaponHasAlternateAmmo ( item->giTag ) )
				{
					if ( ps->ammo[weaponData[item->giTag].attack[ATTACK_ALTERNATE].ammoIndex] >= ammoData[weaponData[item->giTag].attack[ATTACK_ALTERNATE].ammoIndex].max )
					{
						return qfalse;
					}
				}
				else
				{
					return qfalse;		
				}
			}

			return qtrue;

		case IT_AMMO:

			if ( ps->ammo[item->giTag] >= ammoData[item->giTag].max) 
			{
				return qfalse;		// can't hold any more
			}
			return qtrue;

		case IT_ARMOR:

			if ( ps->stats[STAT_ARMOR] >= MAX_ARMOR ) 
			{
				return qfalse;
			}

			return qtrue;

		case IT_HEALTH:

			if ( item->quantity == 5 || item->quantity == 100 ) 
			{
				if ( ps->stats[STAT_HEALTH] >= MAX_HEALTH ) 
				{
					return qfalse;
				}

				return qtrue;
			}

			if ( ps->stats[STAT_HEALTH] >= MAX_HEALTH ) 
			{
				return qfalse;
			}

			return qtrue;
		
		case IT_GAMETYPE:
			
			// Can only have one of a given item.  This is for when there are 
			// multiple items floating around with the same name
			if ( ps->stats[STAT_GAMETYPE_ITEMS] & (1<<item->giTag) )
			{
				return qfalse;
			}

			// If both teams are checked then its ok to pick it up
			if ( (ent->eFlags & EF_BLUETEAM) && (ent->eFlags & EF_REDTEAM) )
			{
				return qtrue;
			}

			if ( (ent->eFlags & EF_BLUETEAM) && !(ps->persistant[PERS_TEAM]==TEAM_BLUE) )
			{
				return qfalse;
			}

			if ( (ent->eFlags & EF_REDTEAM) && !(ps->persistant[PERS_TEAM]==TEAM_RED) )
			{
				return qfalse;
			}

			return qtrue;

		case IT_BAD:
			Com_Error( ERR_DROP, "BG_CanItemBeGrabbed: IT_BAD" );
			break;

		case IT_BACKPACK:
		{
			int i;

			// Not full health then pick it up
			if ( ps->stats[STAT_HEALTH] != MAX_HEALTH )
			{
				return qtrue;
			}

			// Not full armor and dont have goggles then pick it up
			if ( !ps->stats[STAT_GOGGLES] && ps->stats[STAT_ARMOR] != MAX_HEALTH )
			{
				return qtrue;
			}
			
			// Boe!Man 11/13/12: New code for ammo checking regarding no nades.
			if(!g_disableNades.integer && level.nadesFound){ // If nades are set to be used and are actually found..
				for ( i = 0; i < MAX_AMMO; i ++ )
				{
					int maxammo;

					maxammo = BG_GetMaxAmmo ( ps, i );

					if ( !maxammo || ps->ammo[i] >= maxammo )
					{
						continue;
					}

					return qtrue;
				}
			}else{
				for ( i = 0; i < AMMO_M15; i ++ )
				{
					int maxammo;

					maxammo = BG_GetMaxAmmo ( ps, i );

					if ( !maxammo || ps->ammo[i] >= maxammo )
					{
						continue;
					}

					return qtrue;
				}
			}
			
			// Grenades are a special case because they can be depleted by just throwing them.  
			// Therefore we need to check to see if they have any of the grenades indicated in 
			// their outfitting
			if ( ps->pm_flags & PMF_LIMITED_INVENTORY && (!g_disableNades.integer && level.nadesFound))
			{
				// No grenades, then let the pick up the backpack
				if ( !(ps->stats[STAT_WEAPONS] & (1<<ps->stats[STAT_OUTFIT_GRENADE])) )
				{
					return qtrue;
				}
			}

			return qfalse;
		}

        default:
#ifndef Q3_VM
#ifndef NDEBUG 
		      Com_Printf("BG_CanItemBeGrabbed: unknown enum %d\n", item->giType );
#endif
#endif
			 break;
	}

	return qfalse;
}

//======================================================================

/*
================
BG_EvaluateTrajectory

================
*/
void BG_EvaluateTrajectory( const trajectory_t *tr, int atTime, vec3_t result ) {
	float		deltaTime;
	float		phase;

	switch( tr->trType ) {
	case TR_STATIONARY:
	case TR_INTERPOLATE:
		VectorCopy( tr->trBase, result );
		break;
	case TR_LINEAR:
		deltaTime = ( atTime - tr->trTime ) * 0.001;	// milliseconds to seconds
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		break;
	case TR_SINE:
		deltaTime = ( atTime - tr->trTime ) / (float) tr->trDuration;
		phase = sin( deltaTime * M_PI * 2 );
		VectorMA( tr->trBase, phase, tr->trDelta, result );
		break;
	case TR_LINEAR_STOP:
		if ( atTime > tr->trTime + tr->trDuration ) {
			atTime = tr->trTime + tr->trDuration;
		}
		deltaTime = ( atTime - tr->trTime ) * 0.001;	// milliseconds to seconds
		if ( deltaTime < 0 ) {
			deltaTime = 0;
		}
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		break;
	case TR_LIGHTGRAVITY:
		deltaTime = ( atTime - tr->trTime ) * 0.001;	// milliseconds to seconds
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		result[2] -= 0.3f * DEFAULT_GRAVITY * deltaTime * deltaTime;		// FIXME: local gravity...
		break;
	case TR_GRAVITY:
		deltaTime = ( atTime - tr->trTime ) * 0.001;	// milliseconds to seconds
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		result[2] -= 0.5 * DEFAULT_GRAVITY * deltaTime * deltaTime;		// FIXME: local gravity...
		break;
	case TR_HEAVYGRAVITY:
		deltaTime = ( atTime - tr->trTime ) * 0.001;	// milliseconds to seconds
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		result[2] -= 1.0 * DEFAULT_GRAVITY * deltaTime * deltaTime;		// FIXME: local gravity...
		break;
	default:
		Com_Error( ERR_DROP, "BG_EvaluateTrajectory: unknown trType: %i", tr->trTime );
		break;
	}
}

/*
================
BG_EvaluateTrajectoryDelta

For determining velocity at a given time
================
*/
void BG_EvaluateTrajectoryDelta( const trajectory_t *tr, int atTime, vec3_t result ) {
	float	deltaTime;
	float	phase;

	switch( tr->trType ) {
	case TR_STATIONARY:
	case TR_INTERPOLATE:
		VectorClear( result );
		break;
	case TR_LINEAR:
		VectorCopy( tr->trDelta, result );
		break;
	case TR_SINE:
		deltaTime = ( atTime - tr->trTime ) / (float) tr->trDuration;
		phase = cos( deltaTime * M_PI * 2 );	// derivative of sin = cos
		phase *= 0.5;
		VectorScale( tr->trDelta, phase, result );
		break;
	case TR_LINEAR_STOP:
		if ( atTime > tr->trTime + tr->trDuration ) {
			VectorClear( result );
			return;
		}
		VectorCopy( tr->trDelta, result );
		break;
	case TR_LIGHTGRAVITY:
		deltaTime = ( atTime - tr->trTime ) * 0.001;	// milliseconds to seconds
		VectorCopy( tr->trDelta, result );
		result[2] -= 0.6f * DEFAULT_GRAVITY * deltaTime;		// FIXME: local gravity...
		break;
	case TR_GRAVITY:
		deltaTime = ( atTime - tr->trTime ) * 0.001;	// milliseconds to seconds
		VectorCopy( tr->trDelta, result );
		result[2] -= DEFAULT_GRAVITY * deltaTime;		// FIXME: local gravity...
		break;
	case TR_HEAVYGRAVITY:
		deltaTime = ( atTime - tr->trTime ) * 0.001;	// milliseconds to seconds
		VectorCopy( tr->trDelta, result );
		result[2] -= 2.0f * DEFAULT_GRAVITY * deltaTime;		// FIXME: local gravity...
		break;
	default:
		Com_Error( ERR_DROP, "BG_EvaluateTrajectoryDelta: unknown trType: %i", tr->trTime );
		break;
	}
}

char *eventnames[] = {
	"EV_NONE",

	"EV_FOOTSTEP",
	"EV_FOOTWADE",
	"EV_SWIM",

	"EV_STEP_4",
	"EV_STEP_8",
	"EV_STEP_12",
	"EV_STEP_16",

	"EV_FALL_SHORT",
	"EV_FALL_MEDIUM",
	"EV_FALL_FAR",

	"EV_JUMP",
	"EV_WATER_FOOTSTEP",
	"EV_WATER_TOUCH",	// foot touches

	"EV_ITEM_PICKUP",			// normal item pickups are predictable

	"EV_NOAMMO",
	"EV_CHANGE_WEAPON",
	"EV_FIRE_WEAPON",
	"EV_ALT_FIRE",

	"EV_USE",			// +Use key

	"EV_ITEM_RESPAWN",
	"EV_ITEM_POP",
	"EV_PLAYER_TELEPORT_IN",
	"EV_PLAYER_TELEPORT_OUT",

	"EV_GRENADE_BOUNCE",		// eventParm will be the soundindex

	"EV_PLAY_EFFECT",

	"EV_GENERAL_SOUND",
	"EV_GLOBAL_SOUND",		// no attenuation
	"EV_ENTITY_SOUND",

	"EV_GLASS_SHATTER",

	"EV_MISSILE_HIT",
	"EV_MISSILE_MISS",
	"EV_BULLET",				// otherEntity is the shooter

	"EV_PAIN",
	"EV_OBITUARY",

	"EV_DESTROY_GHOUL2_INSTANCE",

	"EV_WEAPON_CHARGE",
	"EV_WEAPON_CHARGE_ALT",

	"EV_DEBUG_LINE",
	"EV_TESTLINE",
	"EV_STOPLOOPINGSOUND",
};

/*
===============
BG_AddPredictableEventToPlayerstate

Handles the sequence numbers
===============
*/

void	trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );

void BG_AddPredictableEventToPlayerstate( int newEvent, int eventParm, playerState_t *ps ) {

#ifdef _DEBUG
	{
		char buf[256];
		trap_Cvar_VariableStringBuffer("showevents", buf, sizeof(buf));
		if ( atof(buf) != 0 ) {
#ifdef QAGAME
			Com_Printf(" game event svt %5d -> %5d: num = %20s parm %d\n", ps->pmove_framecount/*ps->commandTime*/, ps->eventSequence, eventnames[newEvent], eventParm);
#else
			Com_Printf("Cgame event svt %5d -> %5d: num = %20s parm %d\n", ps->pmove_framecount/*ps->commandTime*/, ps->eventSequence, eventnames[newEvent], eventParm);
#endif
		}
	}
#endif
	ps->events[ps->eventSequence & (MAX_PS_EVENTS-1)] = newEvent;
	ps->eventParms[ps->eventSequence & (MAX_PS_EVENTS-1)] = eventParm;
	ps->eventSequence++;
}

/*
========================
BG_PlayerStateToEntityState

This is done after each set of usercmd_t on the server,
and after local prediction on the client
========================
*/
void BG_PlayerStateToEntityState( playerState_t *ps, entityState_t *s, qboolean snap ) 
{
	if ( ps->pm_type == PM_INTERMISSION || ps->pm_type == PM_SPECTATOR || g_entities[s->clientNum].client->sess.invisibleGoggles) // Boe!Man 9/16/12: Henkie's flag isn't working, just check the sess info.
	{
		s->eType = ET_INVISIBLE;
	} 
	else 
	{
		s->eType = ET_PLAYER;
	}

	s->number = ps->clientNum;

	s->pos.trType = TR_INTERPOLATE;

	VectorCopy( ps->origin, s->pos.trBase );
	if ( snap ) 
	{
		SnapVector( s->pos.trBase );
	}
	// set the trDelta for flag direction
	VectorCopy( ps->velocity, s->pos.trDelta );

	s->apos.trType = TR_INTERPOLATE;
	VectorCopy( ps->viewangles, s->apos.trBase );
	if ( snap ) 
	{
		SnapVector( s->apos.trBase );
	}

	s->angles2[YAW] = ps->movementDir;
	s->legsAnim = ps->legsAnim;
	s->torsoAnim = ps->torsoAnim;
	s->torsoTimer = ps->torsoTimer;
	s->clientNum = ps->clientNum;		// ET_PLAYER looks here instead of at number
										// so corpses can also reference the proper config
	s->eFlags = ps->eFlags;

	if ( ps->stats[STAT_HEALTH] <= 0 || ps->pm_type == PM_DEAD ) 
	{
		s->eFlags |= EF_DEAD;
	} 
	else 
	{
		s->eFlags &= ~EF_DEAD;
	}

	if ( ps->pm_flags & PMF_GOGGLES_ON )
	{
		s->eFlags |= EF_GOGGLES;
	}
	else
	{
		s->eFlags &= (~EF_GOGGLES);
	}

	if ( ps->pm_flags & PMF_DUCKED)
	{
		s->eFlags |= EF_DUCKED;
	}
	else
	{
		s->eFlags &= (~EF_DUCKED);
	}

	if ( ps->externalEvent ) 
	{
		s->event = ps->externalEvent;
		s->eventParm = ps->externalEventParm;
	} 
	else if ( ps->entityEventSequence < ps->eventSequence ) 
	{
		int		seq;

		if ( ps->entityEventSequence < ps->eventSequence - MAX_PS_EVENTS) 
		{
			ps->entityEventSequence = ps->eventSequence - MAX_PS_EVENTS;
		}
		
		seq = ps->entityEventSequence & (MAX_PS_EVENTS-1);
		s->event = ps->events[ seq ] | ( ( ps->entityEventSequence & 3 ) << 8 );
		s->eventParm = ps->eventParms[ seq ];
		ps->entityEventSequence++;
	}

	s->weapon		   = ps->weapon;
	s->groundEntityNum = ps->groundEntityNum;
	s->gametypeitems   = ps->stats[STAT_GAMETYPE_ITEMS];
	s->loopSound	   = ps->loopSound;
	s->generic1		   = ps->generic1;
	s->leanOffset	   = (int) BG_CalculateLeanOffset ( ps->leanTime ) + LEAN_OFFSET;
	s->time			   = ps->persistant[PERS_SPAWN_COUNT];
}

/*
========================
BG_PlayerStateToEntityStateExtraPolate

This is done after each set of usercmd_t on the server,
and after local prediction on the client
========================
*/
void BG_PlayerStateToEntityStateExtraPolate( playerState_t *ps, entityState_t *s, int time, qboolean snap ) 
{
	if ( ps->pm_type == PM_INTERMISSION || ps->pm_type == PM_SPECTATOR || g_entities[s->clientNum].client->sess.invisibleGoggles) // Boe!Man 9/16/12: Henkie's flag isn't working, just check the sess info.
	{
		s->eType = ET_INVISIBLE;
	} 
	else 
	{
		s->eType = ET_PLAYER;
	}

	s->number = ps->clientNum;

	s->pos.trType = TR_LINEAR_STOP;

	VectorCopy( ps->origin, s->pos.trBase );
	if ( snap ) 
	{
		SnapVector( s->pos.trBase );
	}
	
	// set the trDelta for flag direction and linear prediction
	VectorCopy( ps->velocity, s->pos.trDelta );
	// set the time for linear prediction
	s->pos.trTime = time;
	// set maximum extra polation time
	s->pos.trDuration = 50; // 1000 / sv_fps (default = 20)

	s->apos.trType = TR_INTERPOLATE;
	VectorCopy( ps->viewangles, s->apos.trBase );
	if ( snap ) 
	{
		SnapVector( s->apos.trBase );
	}

	s->angles2[YAW] = ps->movementDir;
	s->legsAnim = ps->legsAnim;
	s->torsoAnim = ps->torsoAnim;
	s->torsoTimer = ps->torsoTimer;
	s->clientNum = ps->clientNum;		// ET_PLAYER looks here instead of at number
										// so corpses can also reference the proper config
	s->eFlags = ps->eFlags;

	if ( ps->stats[STAT_HEALTH] <= 0 || ps->pm_type == PM_DEAD ) 
	{
		s->eFlags |= EF_DEAD;
	} 
	else 
	{
		s->eFlags &= ~EF_DEAD;
	}

	// Boe!Man 9/16/12: New invisibility goggle system.
	if ( ps->pm_flags & PMF_GOGGLES_ON )
	{
		if(ps->stats[STAT_GOGGLES] == GOGGLES_NIGHTVISION && ps->persistant[PERS_TEAM] == TEAM_BLUE && current_gametype.value == GT_HS && hideSeek_Extra.string[GOGGLES] == '1' && !g_entities[s->clientNum].client->sess.invisibleGoggles){
			if(level.time >= g_entities[s->clientNum].client->sess.invisibleCoolDown + 3000){
				g_entities[s->clientNum].client->sess.invisibleGoggles = qtrue;
				s->eFlags |= EF_GOGGLES;
			}else{
				if(level.time >= g_entities[s->clientNum].client->sess.invisibleCoolDown + 2000){
					trap_SendServerCommand(s->clientNum, va("print\"^3[H&S] ^7You can use the invisibility goggles again in a second.\n\""));
				}else if(level.time >= g_entities[s->clientNum].client->sess.invisibleCoolDown + 1000){
					trap_SendServerCommand(s->clientNum, va("print\"^3[H&S] ^7You can use the invisibility goggles again in 2 seconds.\n\""));
				}else{
					trap_SendServerCommand(s->clientNum, va("print\"^3[H&S] ^7You can use the invisibility goggles again in 3 seconds.\n\""));
				}
				ps->pm_flags &= ~(PMF_GOGGLES_ON);
			}
		}else{
			s->eFlags |= EF_GOGGLES;
		}
		
		if(g_entities[s->clientNum].client->sess.invisibleGoggles){
			if(level.time >= g_entities[s->clientNum].client->sess.invisibletime){
				if(g_entities[s->clientNum].client->ps.stats[STAT_ARMOR] <= 0){
					trap_SendServerCommand(s->clientNum, va("print\"^3[H&S] ^7Invisibility goggles wore off.\n\""));
					ps->pm_flags &= ~(PMF_GOGGLES_ON);
					ps->stats[STAT_GOGGLES] = GOGGLES_NONE;
					g_entities[s->clientNum].client->sess.invisibleGoggles = qfalse;
				}else{
					// Boe!Man 9/16/12: Show effect.
					if(level.time >= g_entities[s->clientNum].client->sess.invisibleFxTime){
						AddSpawnField("classname", "1fx_play_effect");
						AddSpawnField("effect", "levels/air4_toxic_smoke");
						AddSpawnField("origin", va("%.0f %.0f %.0f", g_entities[s->clientNum].r.currentOrigin[0], g_entities[s->clientNum].r.currentOrigin[1], g_entities[s->clientNum].r.currentOrigin[2]+25));
						AddSpawnField("count", "1");
						G_SpawnGEntityFromSpawnVars (qtrue);
						g_entities[s->clientNum].client->sess.invisibleFxTime = level.time+500; // Next update.	
					}
					// Boe!Man 9/16/12: Decrease armor.
					ps->stats[STAT_ARMOR] -= 1;
					g_entities[s->clientNum].client->sess.invisibletime = level.time+100; // Next update.
				}
			}
		}
	}
	else
	{
		s->eFlags &= (~EF_GOGGLES);
		if(g_entities[s->clientNum].client->sess.invisibleGoggles && ps->stats[STAT_GOGGLES] == GOGGLES_NIGHTVISION && ps->persistant[PERS_TEAM] == TEAM_BLUE && current_gametype.value == GT_HS && hideSeek_Extra.string[GOGGLES] == '1'){
			// Boe!Man 9/16/12: His last status was invisible, so set the invisibleCoolDown now..
			g_entities[s->clientNum].client->sess.invisibleGoggles = qfalse;
			g_entities[s->clientNum].client->sess.invisibleCoolDown = level.time;
		}
	}

	if ( ps->pm_flags & PMF_DUCKED)
	{
		s->eFlags |= EF_DUCKED;
	}
	else
	{
		s->eFlags &= (~EF_DUCKED);
	}

	if ( ps->externalEvent ) 
	{
		s->event = ps->externalEvent;
		s->eventParm = ps->externalEventParm;
	} 
	else if ( ps->entityEventSequence < ps->eventSequence ) 
	{
		int	seq;

		if ( ps->entityEventSequence < ps->eventSequence - MAX_PS_EVENTS) 
		{
			ps->entityEventSequence = ps->eventSequence - MAX_PS_EVENTS;
		}
		seq = ps->entityEventSequence & (MAX_PS_EVENTS-1);
		s->event = ps->events[ seq ] | ( ( ps->entityEventSequence & 3 ) << 8 );
		s->eventParm = ps->eventParms[ seq ];
		ps->entityEventSequence++;
	}

	// Direct copies
	s->weapon		   = ps->weapon;
	s->groundEntityNum = ps->groundEntityNum;
	s->gametypeitems   = ps->stats[STAT_GAMETYPE_ITEMS];
	s->loopSound	   = ps->loopSound;
	s->generic1		   = ps->generic1;
	s->leanOffset	   = (int) BG_CalculateLeanOffset ( ps->leanTime ) + LEAN_OFFSET;
	s->time			   = ps->persistant[PERS_SPAWN_COUNT];
}

