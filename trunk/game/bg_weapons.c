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
// bg_weapons.c - weapon data loading

#include "q_shared.h"
#include "bg_public.h"
#include "bg_local.h"
#include "g_local.h"

char *bg_weaponNames[WP_MAX_WEAPONS] = {0};
char *bg_enumWeaponNames[WP_MAX_WEAPONS] = {0};
char *ammoNames[WP_MAX_AMMO] = {0};
char *modNames[WP_MAX_MODS] = {0};

weaponData_t weaponData[WP_MAX_WEAPONS];
ammoData_t ammoData[WP_MAX_AMMO];

void BG_InitializeWeaponsAndAmmo()
{
    #ifndef _GOLD
    bg_weaponNames[0] = "No Weapon";    // WP_NONE
    bg_weaponNames[1] = "Knife";        // WP_KNIFE
    bg_weaponNames[2] = "M1911A1";      // WP_M1911A1_PISTOL
    bg_weaponNames[3] = "US SOCOM";     // WP_US_SOCOM_PISTOL
    bg_weaponNames[4] = "M590";         // WP_M590_SHOTGUN
    bg_weaponNames[5] = "Micro Uzi";    // WP_MICRO_UZI_SUBMACHINEGUN
    bg_weaponNames[6] = "M3A1";         // WP_M3A1_SUBMACHINEGUN
    bg_weaponNames[7] = "USAS-12";      // WP_USAS_12_SHOTGUN
    bg_weaponNames[8] = "M4";           // WP_M4_ASSAULT_RIFLE
    bg_weaponNames[9] = "AK74";         // WP_AK74_ASSAULT_RIFLE
    bg_weaponNames[10] = "MSG90A1";     // WP_MSG90A1_SNIPER_RIFLE
    bg_weaponNames[11] = "M60";         // WP_M60_MACHINEGUN
    bg_weaponNames[12] = "MM1";         // WP_MM1_GRENADE_LAUNCHER
    bg_weaponNames[13] = "RPG7";        // WP_RPG7_LAUNCHER
    bg_weaponNames[14] = "M67";         // WP_M67_GRENADE
    bg_weaponNames[15] = "M84";         // WP_M84_GRENADE
    bg_weaponNames[16] = "F1";          // WP_F1_GRENADE
    bg_weaponNames[17] = "L2A2";        // WP_L2A2_GRENADE
    bg_weaponNames[18] = "MDN11";       // WP_MDN11_GRENADE
    bg_weaponNames[19] = "SMOHG92";     // WP_SMOHG92_GRENADE
    bg_weaponNames[20] = "ANM14";       // WP_ANM14_GRENADE
    bg_weaponNames[21] = "M15";         // WP_M15_GRENADE

    bg_enumWeaponNames[0] = "WP_NONE";
    bg_enumWeaponNames[1] = "WP_KNIFE";
    bg_enumWeaponNames[2] = "WP_M1911A1_PISTOL";
    bg_enumWeaponNames[3] = "WP_US_SOCOM_PISTOL";
    bg_enumWeaponNames[4] = "WP_M590_SHOTGUN";
    bg_enumWeaponNames[5] = "WP_MICRO_UZI_SUBMACHINEGUN";
    bg_enumWeaponNames[6] = "WP_M3A1_SUBMACHINEGUN";
    bg_enumWeaponNames[7] = "WP_USAS_12_SHOTGUN";
    bg_enumWeaponNames[8] = "WP_M4_ASSAULT_RIFLE";
    bg_enumWeaponNames[9] = "WP_AK74_ASSAULT_RIFLE";
    bg_enumWeaponNames[10] = "WP_MSG90A1_SNIPER_RIFLE";
    bg_enumWeaponNames[11] = "WP_M60_MACHINEGUN";
    bg_enumWeaponNames[12] = "WP_MM1_GRENADE_LAUNCHER";
    bg_enumWeaponNames[13] = "WP_RPG7_LAUNCHER";
    bg_enumWeaponNames[14] = "WP_M67_GRENADE";
    bg_enumWeaponNames[15] = "WP_M84_GRENADE";
    bg_enumWeaponNames[16] = "WP_F1_GRENADE";
    bg_enumWeaponNames[17] = "WP_L2A2_GRENADE";
    bg_enumWeaponNames[18] = "WP_MDN11_GRENADE";
    bg_enumWeaponNames[19] = "WP_SMOHG92_GRENADE";
    bg_enumWeaponNames[20] = "WP_ANM14_GRENADE";
    bg_enumWeaponNames[21] = "WP_M15_GRENADE";

    ammoNames[0] = "Knife";         //  AMMO_KNIFE
    ammoNames[1] = "0.45 ACP";      //  AMMO_045
    ammoNames[2] = "5.56mm";        //  AMMO_556
    ammoNames[3] = "9mm";           //  AMMO_9
    ammoNames[4] = "12 gauge";      //  AMMO_12
    ammoNames[5] = "7.62mm";        //  AMMO_762
    ammoNames[6] = "40mm grenade";  //  AMMO_40
    ammoNames[7] = "RPG7";          //  AMMO_RPG7
    ammoNames[8] = "M15";           //  AMMO_M15
    ammoNames[9] = "M67";           //  AMMO_M67
    ammoNames[10] = "M84";          //  AMMO_M84
    ammoNames[11] = "F1";           //  AMMO_F1
    ammoNames[12] = "L2A2";         //  AMMO_L2A2
    ammoNames[13] = "MDN11";        //  AMMO_MDN11
    ammoNames[14] = "SMOHG92";      //  AMMO_SMOHG92
    ammoNames[15] = "ANM14";        //  AMMO_ANM14

    modNames[0] = "Unknown";
    modNames[1] = "Knife";
    modNames[2] = "M1911A1";
    modNames[3] = "US SOCOM";
    modNames[4] = "M590";
    modNames[5] = "Micro Uzi";
    modNames[6] = "M3A1";
    modNames[7] = "USAS-12";
    modNames[8] = "M4";
    modNames[9] = "AK74";
    modNames[10] = "M1911A1";
    modNames[11] = "M60";
    modNames[12] = "MM1";
    modNames[13] = "RPG7";
    modNames[14] = "M67";
    modNames[15] = "M84";
    modNames[16] = "F1";
    modNames[17] = "L2A2";
    modNames[18] = "MDN11";
    modNames[19] = "SMOHG92";
    modNames[20] = "ANM14";
    modNames[21] = "M15";
    modNames[22] = "Water";
    modNames[23] = "Crushing entity";
    modNames[24] = "Telefrag";
    modNames[25] = "Falling";
    modNames[26] = "Sucicide";
    modNames[27] = "Team change";
    modNames[28] = "Target Laser";
    modNames[29] = "Trigger hurt entity";
    modNames[30] = "Trigger hurt entity (no suicide)";
    modNames[31] = "Car";
    modNames[32] = "Pop";
    modNames[33] = "Refresh";
    modNames[34] = "Plant up";
    modNames[35] = "Burn";

    #else
    bg_weaponNames[0] = "No Weapon";    // WP_NONE
    bg_weaponNames[1] = "Knife";        // WP_KNIFE
    bg_weaponNames[2] = "M1911A1";      // WP_M1911A1_PISTOL
    bg_weaponNames[3] = "US SOCOM";     // WP_US_SOCOM_PISTOL
    bg_weaponNames[4] = "Silver Talon"; // WP_SILVER_TALON
    bg_weaponNames[5] = "M590";         // WP_M590_SHOTGUN
    bg_weaponNames[6] = "Micro Uzi";    // WP_MICRO_UZI_SUBMACHINEGUN
    bg_weaponNames[7] = "M3A1";         // WP_M3A1_SUBMACHINEGUN
    bg_weaponNames[8] = "MP5";          // WP_MP5
    bg_weaponNames[9] = "USAS-12";      // WP_USAS_12_SHOTGUN
    bg_weaponNames[10] = "M4";          // WP_M4_ASSAULT_RIFLE
    bg_weaponNames[11] = "AK74";        // WP_AK74_ASSAULT_RIFLE
    bg_weaponNames[12] = "Sig 551";     // WP_SIG551
    bg_weaponNames[13] = "MSG90A1";     // WP_MSG90A1_SNIPER_RIFLE
    bg_weaponNames[14] = "M60";         // WP_M60_MACHINEGUN
    bg_weaponNames[15] = "MM1";         // WP_MM1_GRENADE_LAUNCHER
    bg_weaponNames[16] = "RPG7";        // WP_RPG7_LAUNCHER
    bg_weaponNames[17] = "M84";         // WP_M84_GRENADE
    bg_weaponNames[18] = "SMOHG92";     // WP_SMOHG92_GRENADE
    bg_weaponNames[19] = "ANM14";       // WP_ANM14_GRENADE
    bg_weaponNames[20] = "M15";         // WP_M15_GRENADE

    bg_enumWeaponNames[0] = "WP_NONE";
    bg_enumWeaponNames[1] = "WP_KNIFE";
    bg_enumWeaponNames[2] = "WP_M1911A1_PISTOL";
    bg_enumWeaponNames[3] = "WP_US_SOCOM_PISTOL";
    bg_enumWeaponNames[4] = "WP_SILVER_TALON";
    bg_enumWeaponNames[5] = "WP_M590_SHOTGUN";
    bg_enumWeaponNames[6] = "WP_MICRO_UZI_SUBMACHINEGUN";
    bg_enumWeaponNames[7] = "WP_M3A1_SUBMACHINEGUN";
    bg_enumWeaponNames[8] = "WP_MP5";
    bg_enumWeaponNames[9] = "WP_USAS_12_SHOTGUN";
    bg_enumWeaponNames[10] = "WP_M4_ASSAULT_RIFLE";
    bg_enumWeaponNames[11] = "WP_AK74_ASSAULT_RIFLE";
    bg_enumWeaponNames[12] = "WP_SIG551";
    bg_enumWeaponNames[13] = "WP_MSG90A1_SNIPER_RIFLE";
    bg_enumWeaponNames[14] = "WP_M60_MACHINEGUN";
    bg_enumWeaponNames[15] = "WP_MM1_GRENADE_LAUNCHER";
    bg_enumWeaponNames[16] = "WP_RPG7_LAUNCHER";
    bg_enumWeaponNames[17] = "WP_M84_GRENADE";
    bg_enumWeaponNames[18] = "WP_SMOHG92_GRENADE";
    bg_enumWeaponNames[19] = "WP_ANM14_GRENADE";
    bg_enumWeaponNames[20] = "WP_M15_GRENADE";

    ammoNames[0] = "Knife";         //  AMMO_KNIFE
    ammoNames[1] = "0.45 ACP";      //  AMMO_045
    ammoNames[2] = "5.56mm";        //  AMMO_556
    ammoNames[3] = "9mm";           //  AMMO_9
    ammoNames[4] = "12 gauge";      //  AMMO_12
    ammoNames[5] = "7.62mm";        //  AMMO_762
    ammoNames[6] = "40mm grenade";  //  AMMO_40
    ammoNames[7] = "RPG7";          //  AMMO_RPG7
    ammoNames[8] = "M15";           //  AMMO_M15
    ammoNames[9] = "M84";           //  AMMO_M84
    ammoNames[10] = "SMOHG92";      //  AMMO_SMOHG92
    ammoNames[11] = "ANM14";        //  AMMO_ANM14
    ammoNames[12] = "7.62mm belt";  //  AMMO_762_BELT
    ammoNames[13] = "9mm|mp5";      //  AMMO_9_MP5

    // Boe!Man 7/27/15: Check for 1fx. Client Additions, and if so, add the remaining weapons.
    if (g_enforce1fxAdditions.integer) {
        bg_weaponNames[21] = "M67";         // WP_M67_GRENADE
        bg_weaponNames[22] = "F1";          // WP_F1_GRENADE
        bg_weaponNames[23] = "L2A2";        // WP_L2A2_GRENADE
        bg_weaponNames[24] = "MDN11";       // WP_MDN11_GRENADE

        bg_enumWeaponNames[21] = "WP_M67_GRENADE";
        bg_enumWeaponNames[22] = "WP_F1_GRENADE";
        bg_enumWeaponNames[23] = "WP_L2A2_GRENADE";
        bg_enumWeaponNames[24] = "WP_MDN11_GRENADE";

        if(current_gametype.value == GT_HS){
            // Boe!Man 11/9/15: Don't cross any hard-defined bit fields for ammo types. Re-use some to stay under the hard limit of MAX_AMMO.
            ammoNames[12] = "F1";       //  AMMO_F1
            ammoNames[13] = "MDN11";    //  AMMO_MDN11
        }
        ammoNames[14] = "M67";          //  AMMO_M67
        ammoNames[15] = "L2A2";         //  AMMO_L2A2
    }

    modNames[0] = "Unknown";
    modNames[1] = "Knife";
    modNames[2] = "M1911A1";
    modNames[3] = "US SOCOM";
    modNames[4] = "Silver Talon";
    modNames[5] = "M590";
    modNames[6] = "Micro Uzi";
    modNames[7] = "M3A1";
    modNames[8] = "MP5";
    modNames[9] = "USAS-12";
    modNames[10] = "M4";
    modNames[11] = "AK74";
    modNames[12] = "Sig 551";
    modNames[13] = "M1911A1";
    modNames[14] = "M60";
    modNames[15] = "MM1";
    modNames[16] = "RPG7";
    modNames[17] = "M84";
    modNames[18] = "SMOHG92";
    modNames[19] = "ANM14";
    modNames[20] = "M15";
    modNames[21] = "M67";
    modNames[22] = "F1";
    modNames[23] = "L2A2";
    modNames[24] = "MDN11";
    modNames[25] = "Water";
    modNames[26] = "Crushing entity";
    modNames[27] = "Telefrag";
    modNames[28] = "Falling";
    modNames[29] = "Sucicide";
    modNames[30] = "Team change";
    modNames[31] = "Target Laser";
    modNames[32] = "Trigger hurt entity";
    modNames[33] = "Trigger hurt entity (no suicide)";
    modNames[34] = "Car";
    modNames[35] = "Pop";
    modNames[36] = "Refresh";
    modNames[37] = "Plant up";
    modNames[38] = "Burn";
    #endif // not _GOLD
}

static qboolean BG_ParseAmmoStats(ammo_t ammoNum, void *group)
{
    char        tmpStr[256];
    ammoData_t *ammo;       // ammo

    ammo = &ammoData[ammoNum];
    memset(ammo, 0, sizeof(ammoData_t));

    ammo->name = (char*)trap_VM_LocalStringAlloc ( ammoNames[ammoNum] );
    Q_strlwr ( ammo->name );

    // Get the scale of the gore for this bullet
    trap_GPG_FindPairValue(group, "mp_goreScale||goreScale", "1", tmpStr );
    ammo->goreScale = atof ( tmpStr );

    // Max ammo will be filled in by the weapon parsing

    return qtrue;
}

qboolean BG_InitAmmoStats(void)
{
    void        *GP2, *topGroup, *topSubs;
    char        name[256];
    int         i;

    ammoData[AMMO_NONE].goreScale = 0.0f;
    ammoData[AMMO_NONE].name = "none";

    GP2 = trap_GP_ParseFile("ext_data/sof2.ammo", qtrue, qfalse);
    if (!GP2)
    {
        return qfalse;
    }

    topGroup = trap_GP_GetBaseParseGroup(GP2);
    topSubs = trap_GPG_GetSubGroups(topGroup);
    while(topSubs)
    {
        trap_GPG_GetName(topSubs, name);
        if (Q_stricmp(name, "ammo") == 0)
        {
            trap_GPG_FindPairValue(topSubs, "name", "", name);
            for(i=0;i<level.ammoMax;i++)
            {
                if (Q_stricmp(ammoNames[i], name) == 0)
                {
                    BG_ParseAmmoStats((ammo_t)i, topSubs);
                    break;
                }
            }

#ifdef _DEBUG
            if (i == level.ammoMax)
            {
                Com_Printf("BG_InitAmmoStats: Unknown ammo: %s\n", name);
            }
#endif
        }
        topSubs = trap_GPG_GetNext(topSubs);
    }

    trap_GP_Delete(&GP2);

    return qtrue;
}

static qboolean BG_ParseAttackStats ( int weaponNum, attackData_t* attack, void *attacksub )
{
    void*   sub;
    char    tmpStr[256];
    int     i;
    float   recoilRatio, inaccuracyRatio;

    // No group is success.  This is to allow NULL to be passed
    if ( NULL == attacksub )
    {
        return qtrue;
    }

    #ifdef _GOLD
    // Zoom information
    trap_GPG_FindPairValue(attacksub, "action", "", tmpStr);
    if (!Q_stricmp(tmpStr, "toggleZoom"))
    {
        weaponData_t *weapon;
        void         *value;
        int          zoomlvl;

        weapon = &weaponData[weaponNum];

        sub = trap_GPG_FindSubGroup(attacksub, "zoomFactors");
        if (!sub)
        {
            return qfalse;
        }

        value = trap_GPG_GetPairs(sub);
        zoomlvl = 0;
        while (value)
        {
            trap_GPV_GetName(value, weapon->zoom[zoomlvl].name);
            trap_GPV_GetTopValue(value, tmpStr);

            weapon->zoom[zoomlvl].fov = atoi(tmpStr);

            value = trap_GPV_GetNext(value);
            zoomlvl++;
        }

        return qtrue;
    }
    #endif // _GOLD

    // Assign a melee attribute if there is one
    trap_GPG_FindPairValue(attacksub, "mp_melee||melee", "none", tmpStr );
    if ( Q_stricmp ( tmpStr, "none" ) )
    {
        Q_strlwr ( tmpStr );
        attack->melee = trap_VM_LocalStringAlloc ( tmpStr );
    }

    trap_GPG_FindPairValue(attacksub, "name", "NONE", attack->name);
    trap_GPG_FindPairValue(attacksub, "hudIcon", "NONE", attack->icon);
    #ifdef _GOLD
    if (level.pickupsDisabled)
    {
        trap_GPG_FindPairValue(attacksub, "mp_ammoType_outfitting", "", tmpStr);
        if (!tmpStr[0])
        {
            trap_GPG_FindPairValue(attacksub, "mp_ammoType||ammoType", "none", tmpStr);
        }
    }
    else
    #endif // _GOLD
    trap_GPG_FindPairValue(attacksub, "mp_ammoType||ammoType", "none", tmpStr);


    attack->ammoIndex = AMMO_NONE;
    for (i = 0; i < level.ammoMax; i++)
    {
        if (0 == Q_stricmp(tmpStr, ammoNames[i]))
        {
            attack->ammoIndex = i;
            break;
        }
    }

#ifdef _DEBUG
    if (level.ammoMax == i)
    {
        Com_Printf("BG_ParseWeaponStats: Unknown ammo: %s\n", tmpStr);
    }
#endif

    // Determine recoil and inaccuracy ratio.
    if(weaponNum == WP_M590_SHOTGUN){
        inaccuracyRatio = 1.0f;
    }else{
        inaccuracyRatio = g_inaccuracyRatio.value;
    }
    recoilRatio = g_recoilRatio.value;

    // Parse the weapon animations
    trap_GPG_FindPairValue( attacksub, "mp_animFire", "TORSO_ATTACK_PISTOL", tmpStr );
    attack->animFire = GetIDForString ( bg_animTable, tmpStr );
    trap_GPG_FindPairValue( attacksub, "mp_animFireZoomed", "", tmpStr );
    attack->animFireZoomed = GetIDForString ( bg_animTable, tmpStr );

    trap_GPG_FindPairValue(attacksub, "mp_range||range", "8192", tmpStr);
    attack->rV.range = atoi(tmpStr);
    trap_GPG_FindPairValue(attacksub, "mp_fireDelay||fireDelay", "0", tmpStr);
    attack->fireDelay = atoi(tmpStr);
    trap_GPG_FindPairValue(attacksub, "mp_fireAmount||fireAmount", "1", tmpStr);
    attack->fireAmount = atoi(tmpStr);
    trap_GPG_FindPairValue(attacksub, "mp_fireFromClip||fireFromClip", "1", tmpStr);
    attack->fireFromClip = atoi(tmpStr);
    trap_GPG_FindPairValue(attacksub, "mp_inaccuracy||inaccuracy", "0", tmpStr);
    attack->inaccuracy = (int)(atof(tmpStr) * inaccuracyRatio * 1000.0f);
    #ifdef _GOLD
    trap_GPG_FindPairValue(attacksub, "mp_zoominaccuracy", "0", tmpStr);
    attack->zoomInaccuracy = (int)(atof(tmpStr)*1000.0f);
    #endif // _GOLD
    trap_GPG_FindPairValue(attacksub, "mp_maxInaccuracy||maxInaccuracy", "0", tmpStr);
    attack->maxInaccuracy = (int)(atof(tmpStr)*1000.0f);
    trap_GPG_FindPairValue(attacksub, "mp_gore||gore", "YES", tmpStr);
    attack->gore = (Q_stricmp ( tmpStr, "YES" )?qfalse:qtrue);
    trap_GPG_FindPairValue(attacksub, "mp_clipSize||clipSize", "0", tmpStr);
    attack->clipSize = atoi(tmpStr);
    trap_GPG_FindPairValue(attacksub, "mp_damage||damage", "0", tmpStr);
    attack->damage = atoi(tmpStr);
    trap_GPG_FindPairValue(attacksub,"mp_extraClips", "0", tmpStr );
    attack->extraClips = atoi ( tmpStr );
    trap_GPG_FindPairValue(attacksub, "mp_radius||radius", "0", tmpStr);
    attack->splashRadius = atoi(tmpStr);

    // max ammo is the combination of all guns that share the ammo
    ammoData[attack->ammoIndex].max += attack->clipSize * attack->extraClips;

    //Ryan april 24 2004 8:48pm
    //save the default values so we can revert back to them later if needed
    attack->defaultClipSize = attack->clipSize;
    attack->defaultExtraClips = attack->extraClips;
    attack->defaultDamage = attack->damage;
    ammoData[attack->ammoIndex].defaultMax = ammoData[attack->ammoIndex].max;

    trap_GPG_FindPairValue(attacksub,"mp_kickAngles||kickAngles", "0 0 0 0 0 0", tmpStr);
    sscanf( tmpStr, "%f %f %f %f %f %f",
            &attack->minKickAngles[0],
            &attack->maxKickAngles[0],
            &attack->minKickAngles[1],
            &attack->maxKickAngles[1],
            &attack->minKickAngles[2],
            &attack->maxKickAngles[2]);

    if (0 == attack->inaccuracy)
    {
        trap_GPG_FindPairValue(attacksub, "mp_spread||spread", "0", tmpStr);
        attack->inaccuracy = atof(tmpStr);
    }

    for(i = 0; i < 3; i++){
        attack->minKickAngles[i] *= recoilRatio;
        attack->maxKickAngles[i] *= recoilRatio;
    }

    trap_GPG_FindPairValue(attacksub, "mp_pellets||pellets", "1", tmpStr);
    attack->pellets = atof(tmpStr);
    attack->mod = (meansOfDeath_t)weaponNum;

    trap_GPG_FindPairValue(attacksub, "mp_lockFlashToBarrel||lockFlashToBarrel", "true", tmpStr);
    if (0 == Q_stricmp(tmpStr, "false"))
    {
        attack->weaponFlags |= UNLOCK_MUZZLEFLASH;
    }
    // load effects, sounds
    trap_GPG_FindPairValue(attacksub, "muzzleFlash", "", attack->muzzleEffect);
    trap_GPG_FindPairValue(attacksub, "3rdPersonMuzzleFlash", "", attack->muzzleEffectInWorld);
    trap_GPG_FindPairValue(attacksub, "EjectBone", "", attack->ejectBone);
    trap_GPG_FindPairValue(attacksub, "ShellCasingEject", "", attack->shellEject);
    trap_GPG_FindPairValue(attacksub, "TracerEffect", "", attack->tracerEffect);

    // Some alt attacks have special bones they need their muzzle flashes attached to
    trap_GPG_FindPairValue ( attacksub, "mp_muzzleFlashBone", "", attack->muzzleEffectBone );

    sub = trap_GPG_FindSubGroup(attacksub, "fireModes");
    if (sub)
    {
        int i;

        for ( i = 0; i < 5; i ++ )
        {
            trap_GPG_FindPairValue ( sub, va("mp_mode%i||mode%i", i, i ), "", tmpStr );
            if ( !tmpStr[0] )
            {
                continue;
            }

            if (0 == Q_stricmp("single", tmpStr))
                attack->weaponFlags |= (1<<WP_FIREMODE_SINGLE);
            else if (0 == Q_stricmp("auto", tmpStr))
                attack->weaponFlags |= (1<<WP_FIREMODE_AUTO);
            else if (0 == Q_stricmp("burst", tmpStr))
                attack->weaponFlags |= (1<<WP_FIREMODE_BURST);
            else
                attack->weaponFlags |= (1<<WP_FIREMODE_SINGLE);
        }
    }
    else
    {
        attack->weaponFlags |= (1<<WP_FIREMODE_SINGLE);
    }

    sub = trap_GPG_FindSubGroup(attacksub, "projectile");
    if (sub)
    {
        attack->weaponFlags |= PROJECTILE_FIRE;

        trap_GPG_FindPairValue(sub, "gravity", "1", tmpStr);
        if (0 < atof(tmpStr))
            attack->weaponFlags |= PROJECTILE_GRAVITY;

        trap_GPG_FindPairValue(sub, "detonation", "0", tmpStr);
        if (0 == Q_stricmp(tmpStr,"timer"))
            attack->weaponFlags |= PROJECTILE_TIMED;

        #ifndef _DEMO
        trap_GPG_FindPairValue(sub, "mp_bounce||bounce", "0", tmpStr );
        attack->bounceScale = atof ( tmpStr );
        #else
        // In demo, this cannot be set. It is always fixed at 0.45f.
        attack->bounceScale = 0.45f;
        #endif // not _DEMO

        switch ( weaponNum )
        {
            case WP_ANM14_GRENADE:
                // incediary grenade
                attack->weaponFlags |= PROJECTILE_DAMAGE_AREA;
                break;

            case WP_KNIFE:
                #ifndef _DEMO
                if ( attack->weaponFlags & PROJECTILE_GRAVITY )
                {
                    attack->weaponFlags &= ~PROJECTILE_GRAVITY;
                    attack->weaponFlags |= PROJECTILE_LIGHTGRAVITY;
                }
                #else
                attack->weaponFlags |= PROJECTILE_GRAVITY;
                #endif // not _DEMO
                break;
        }
        trap_GPG_FindPairValue(sub, "mp_speed||speed", "0", tmpStr);
        attack->rV.velocity = atoi(tmpStr);
        #ifndef _DEMO
        trap_GPG_FindPairValue(sub, "mp_timer||timer", "10", tmpStr);
        #else
        trap_GPG_FindPairValue(sub, "timer", "10", tmpStr);
        #endif // not _DEMO
        attack->projectileLifetime = (int)(atof(tmpStr) * 1000);

        // 'trail' effect
        trap_GPG_FindPairValue(sub, "mp_effect||effect", "", attack->tracerEffect);
        trap_GPG_FindPairValue(sub, "model", "", attack->missileG2Model);
        trap_GPG_FindPairValue(sub, "mp_explosionEffect||explosionEffect", "", attack->explosionEffect);

        trap_GPG_FindPairValue(sub, "mp_explosionSound||explosionSound", "", attack->explosionSound);
    }

    return qtrue;
}

static qboolean BG_ParseWeaponStats(weapon_t weaponNum, void *group)
{
    char         tmpStr[256];
    weaponData_t *weapon;

    weapon = &weaponData[weaponNum];

    memset(weapon, 0, sizeof(weaponData_t));

    weapon->classname = bg_weaponNames[weaponNum];
    trap_GPG_FindPairValue(group, "category", "0", tmpStr);
    weapon->category = (ECategory)atoi(tmpStr);

    trap_GPG_FindPairValue(group, "safe", "false", tmpStr);
    weapon->safe = (qboolean)!Q_stricmp(tmpStr, "true");

    trap_GPG_FindPairValue(group, "model", "", weapon->worldModel);

    trap_GPG_FindPairValue(group, "menuImage", "", weapon->menuImage);

    // Grab the animations
    trap_GPG_FindPairValue( group, "mp_animRaise", "TORSO_RAISE", tmpStr );
    weapon->animRaise = GetIDForString ( bg_animTable, tmpStr );
    trap_GPG_FindPairValue( group, "mp_animDrop", "TORSO_DROP", tmpStr );
    weapon->animDrop = GetIDForString ( bg_animTable, tmpStr );
    trap_GPG_FindPairValue( group, "mp_animIdle", "TORSO_IDLE_PISTOL", tmpStr );
    weapon->animIdle = GetIDForString ( bg_animTable, tmpStr );
    trap_GPG_FindPairValue( group, "mp_animIdleZoomed", "", tmpStr );
    weapon->animIdleZoomed = GetIDForString ( bg_animTable, tmpStr );
    trap_GPG_FindPairValue( group, "mp_animReload", "", tmpStr );
    weapon->animReload = GetIDForString ( bg_animTable, tmpStr );
    trap_GPG_FindPairValue( group, "mp_animReloadStart", "", tmpStr );
    weapon->animReloadStart = GetIDForString ( bg_animTable, tmpStr );
    trap_GPG_FindPairValue( group, "mp_animReloadEnd", "", tmpStr );
    weapon->animReloadEnd = GetIDForString ( bg_animTable, tmpStr );

    // primary attack
    BG_ParseAttackStats ( weaponNum, &weapon->attack[ATTACK_NORMAL], trap_GPG_FindSubGroup(group, "attack"));

    // alternate attack
    BG_ParseAttackStats ( weaponNum, &weapon->attack[ATTACK_ALTERNATE], trap_GPG_FindSubGroup(group, "altattack"));

    return qtrue;
}

qboolean BG_InitWeaponStats(qboolean init)
{
    void        *GP2, *topGroup, *topSubs;
    char        name[256];
    int         i;
    char WpnFile[64];

    strcpy(WpnFile, "ext_data/SOF2.wpn");

    Com_Printf("Reading weapon file from: %s\n", WpnFile);
    GP2 = trap_GP_ParseFile(WpnFile, qtrue, qfalse);
    if (!GP2)
    {
        Com_Printf("Error reading from weapon file: %s\n", WpnFile);
        return qfalse;
    }

    topGroup = trap_GP_GetBaseParseGroup(GP2);
    topSubs = trap_GPG_GetSubGroups(topGroup);
    while(topSubs)
    {
        trap_GPG_GetName(topSubs, name);
            if (Q_stricmp(name, "weapon") == 0)
            {
                trap_GPG_FindPairValue(topSubs, "name", "", name);
                for(i=0;i<level.wpNumWeapons;i++)
                {
                    if (Q_stricmp(bg_weaponNames[i], name) == 0)
                    {
                        BG_ParseWeaponStats((weapon_t)i, topSubs);
                        break;
                    }
                }

            #ifdef _DEBUG
                if (i == level.wpNumWeapons)
                {
                    Com_Printf("BG_InitWeaponStats: Unknown weapon: %s\n", name);
                }
            #endif
            }else if(init == qfalse){
                trap_GPG_FindPairValue(topSubs, "name", "", name);

                for(i=0;i<level.wpNumWeapons;i++)
                {
                    if (Q_stricmp(bg_weaponNames[i], name) == 0)
                    {
                        BG_ParseWeaponStats((weapon_t)i, topSubs);
                        break;
                    }
                }
            }
        topSubs = trap_GPG_GetNext(topSubs);
    }

    trap_GP_Delete(&GP2);

    return qtrue;
}

////////////////////////////////////////////////////////////////////////////////////

TWeaponParseInfo    weaponParseInfo[WP_MAX_WEAPONS];
char                weaponLeftHand[MAX_QPATH];
char                weaponRightHand[MAX_QPATH];

#define     MAX_WEAPON_FILES    10

static void *weaponFrames[MAX_WEAPON_FILES];
static void *frameGroup[MAX_WEAPON_FILES];
static int  numWeaponFiles = 0;
static int  numInitialFiles = 0;

static qboolean BG_OpenWeaponFrames(const char *name)
{
    weaponFrames[numWeaponFiles] = trap_GP_ParseFile((char *)name, qtrue, qfalse);

    if (!weaponFrames)
    {
        return qfalse;
    }

    frameGroup[numWeaponFiles] = trap_GP_GetBaseParseGroup(weaponFrames[numWeaponFiles]);
    numWeaponFiles++;

    return qtrue;
}

static TNoteTrack *BG_FindNoteTracks(void *group)
{
    void        *sub;
    char        name[256];
    TNoteTrack  *head, *last, *current, *insert;

    head = last = insert = 0;

    sub = trap_GPG_GetSubGroups(group);
    while(sub)
    {
        trap_GPG_GetName(sub, name);
        if (Q_stricmp(name, "notetrack") == 0)
        {
            current = (TNoteTrack *)trap_VM_LocalAlloc(sizeof(*current));
            memset(current, 0, sizeof(*current));

            // last character is automatically 0 cuz of the memset
            trap_GPG_FindPairValue(sub, "note", "", current->mNote);
            trap_GPG_FindPairValue(sub, "frame", "-1", name);
            current->mFrame = atoi(name);

            last=insert=head;
            while(insert)
            {
                if(current->mFrame<insert->mFrame)
                {
                    break;
                }
                last=insert;
                insert=insert->mNext;
            }
            if(insert==head)
            {
                head=current;
            }
            else
            {
                last->mNext=current;
            }
            current->mNext=insert;
        }

        sub = trap_GPG_GetNext(sub);
    }

    return head;
}

static void BG_FindWeaponFrames(TAnimInfoWeapon *animInfo, int choice)
{
    void    *group;
    int     i;

    if (!numWeaponFiles || !animInfo->mAnim[choice])
    {
        animInfo->mNumFrames[choice] = -1;
        return;
    }

    for(i=0;i<numWeaponFiles;i++)
    {
        char  temp[256];

        group = trap_GPG_GetSubGroups ( frameGroup[i] );
        while ( group )
        {
            char name[256];
            // Get the name and break it down to just the filename without
            // and extension
            trap_GPG_GetName ( group, temp );
            Q_strncpyz(name, COM_SkipPath(temp), sizeof(name));
            COM_StripExtension ( name, temp );
            if ( Q_stricmp ( temp, animInfo->mAnim[choice] ) == 0 )
            {
                break;
            }

            group = trap_GPG_GetNext ( group );
        }

        if (group)
        {
            trap_GPG_FindPairValue(group, "startframe", "0", temp);
            animInfo->mStartFrame[choice] = atoi(temp);
            trap_GPG_FindPairValue(group, "duration", "0", temp);
            animInfo->mNumFrames[choice] = atoi(temp);
            trap_GPG_FindPairValue(group, "fps", "0", temp);
            animInfo->mFPS[choice] = atoi(temp);
            animInfo->mNoteTracks[choice] = BG_FindNoteTracks(group);
            return;
        }
    }
    animInfo->mNumFrames[choice] = -1;
}

static void BG_CloseWeaponFrames(int upTo)
{
    int     i;

    for(i=upTo; i < numWeaponFiles; i++)
    {
        if (weaponFrames[i])
        {
            trap_GP_Delete(&weaponFrames[i]);
        }
    }

    // Always flush structs for the v1.00 Linux Woody build.
    if(upTo == 0){
        memset(weaponFrames, 0, sizeof(weaponFrames));
    }

    numWeaponFiles = upTo;
}


static qboolean BG_ParseAnimGroup(weapon_t weapon, sqlite3 * db)
{
    TAnimWeapon     *anim;
    TAnimInfoWeapon *info;
    char            value[256];
    int             k;
    sqlite3_stmt *stmt, *stmtInfo;
    int rc;
    qboolean succes = qfalse;
    char query[128];

    strncpy(query, "SELECT * FROM weapon_anim WHERE WEAPON_ID=?",
        sizeof(query));

    // Prepare statement.
    rc = sqlite3_prepare(db, query, -1, &stmt, 0);
    sqlite3_bind_int(stmt, 1, weapon);

    if(rc!=SQLITE_OK){
        G_LogPrintf("^1Error: ^7Inview database: %s\n", sqlite3_errmsg(db));
        return qfalse;
    }

    // Iterate through the anim group.
    while((rc = sqlite3_step(stmt)) != SQLITE_DONE){
        if(rc == SQLITE_ROW){
            // Allocate memory.
            anim = (TAnimWeapon *)trap_VM_LocalAlloc(sizeof(*anim));
            memset(anim, 0, sizeof(TAnimWeapon));

            // Parse base info.
            anim->mNext = weaponParseInfo[weapon].mAnimList;
            weaponParseInfo[weapon].mAnimList = anim;
            strcpy(anim->mName, (char *)sqlite3_column_text(stmt, 2));

            // Parse anim info.
            strncpy(query, "SELECT * FROM weapon_anim_info WHERE ANIM_ID=?",
                sizeof(query));

            // Prepare statement.
            rc = sqlite3_prepare(db, query, -1, &stmtInfo, 0);
            sqlite3_bind_int(stmtInfo, 1, sqlite3_column_int(stmt, 1));

            if(rc !=SQLITE_OK){
                G_LogPrintf("^1Error: ^7Inview database: %s\n",
                    sqlite3_errmsg(db));
                return qfalse;
            }

            // Iterate through the associated anim info.
            while((rc = sqlite3_step(stmtInfo)) != SQLITE_DONE){
                if(rc == SQLITE_ROW){
                    // Allocate memory.
                    info = (TAnimInfoWeapon *)
                        trap_VM_LocalAlloc(sizeof(TAnimInfoWeapon));
                    memset(info, 0, sizeof(TAnimInfoWeapon));

                    // Get base info.
                    info->mNext = anim->mInfos;
                    anim->mInfos = info;
                    info->mNumChoices = 0;
                    strncpy(info->mName, sqlite3_column_text(stmtInfo, 2),
                        sizeof(info->mName));
                    strncpy(info->mType, sqlite3_column_text(stmtInfo, 1),
                        sizeof(info->mType));

                    if(!Q_stricmp(info->mType, "weaponmodel"))
                    {
                        anim->mWeaponModelInfo = info;
                    }

                    // We first look for a multiplayer specific speed. If we
                    // don't find a valid speed, use the single player speed
                    // instead.
                    if(sqlite3_column_double(stmtInfo, 16)){ // mp_speed
                        info->mSpeed = sqlite3_column_double(stmtInfo, 16);
                    }else if(sqlite3_column_double(stmtInfo, 15)){ // speed
                        info->mSpeed = sqlite3_column_double(stmtInfo, 15);
                    }else{
                        // Default value.
                        info->mSpeed = 1.0f;
                    }


                    if(sqlite3_column_int(stmtInfo, 17)){
                        info->mLODBias = sqlite3_column_int(stmtInfo, 17);
                    }else{
                        info->mLODBias = 0;
                    }

                    // Anim.
                    if(sqlite3_column_text(stmtInfo, 3)){
                        strncpy(value, sqlite3_column_text(stmtInfo, 3),
                            sizeof(value));
                        info->mAnim[info->mNumChoices] =
                            trap_VM_LocalAlloc(strlen(value)+1);
                        strncpy(info->mAnim[info->mNumChoices], value,
                            strlen(value));

                        info->mNumChoices++;
                    }

                    for(k = 1; k <= MAX_WEAPON_ANIM_CHOICES; k++){
                        succes = qfalse;

                        if(sqlite3_column_text(stmtInfo, 3+k)){
                            // anim* keys.
                            strncpy(value, sqlite3_column_text(stmtInfo, 3+k),
                                sizeof(value));
                            info->mAnim[info->mNumChoices] =
                                trap_VM_LocalAlloc(strlen(value)+1);
                            strncpy(info->mAnim[info->mNumChoices], value,
                                strlen(value));

                            succes = qtrue;
                        }else if(sqlite3_column_text(stmtInfo, 7+k)){
                            // animNoLerp* keys.
                            strncpy(value, sqlite3_column_text(stmtInfo, 7+k),
                                sizeof(value));
                            info->mAnim[info->mNumChoices] =
                                trap_VM_LocalAlloc(strlen(value)+1);
                            strncpy(info->mAnim[info->mNumChoices], value,
                                strlen(value));

                            succes = qtrue;
                        }

                        if(succes){
                            // transition* keys.
                            if(sqlite3_column_text(stmtInfo, 18+k)){
                                strncpy(value,
                                    sqlite3_column_text(stmtInfo, 18+k),
                                    sizeof(value));
                                info->mTransition[info->mNumChoices] =
                                    trap_VM_LocalAlloc(strlen(value)+1);
                                strncpy(info->mTransition[info->mNumChoices],
                                    value, strlen(value));
                            }

                            // end* keys.
                            if(sqlite3_column_text(stmtInfo, 21+k)){
                                strncpy(value,
                                    sqlite3_column_text(stmtInfo, 21+k),
                                    sizeof(value));
                                info->mEnd[info->mNumChoices] =
                                    trap_VM_LocalAlloc(strlen(value)+1);
                                strncpy(info->mEnd[info->mNumChoices], value,
                                    strlen(value));
                            }
                            info->mNumChoices++;
                        }
                    }
                }
            }

            // Free allocated memory.
            sqlite3_finalize(stmtInfo);
        }
    }
    // Free allocated memory.
    sqlite3_finalize(stmt);

    return qtrue;
}

static qboolean BG_ParseWeaponGroup(TWeaponModel *weapon, weapon_t weaponID, sqlite3 * db)
{
    char            frames[256], query[128];
    sqlite3_stmt    *stmt;
    int             rc;

    strncpy(query, "SELECT * FROM weaponmodel where WEAPON_ID=?",
        sizeof(query));

    // Prepare the statement.
    rc = sqlite3_prepare(db, query, -1, &stmt, 0);
    sqlite3_bind_int(stmt, 1, weaponID);

    if(rc != SQLITE_OK){
        G_LogPrintf("^1Error: ^7Inview database: %s\n", sqlite3_errmsg(db));
        return qfalse;
    }

    // Iterate through the weapon models.
    while((rc = sqlite3_step(stmt)) != SQLITE_DONE){
        if(rc == SQLITE_ROW){
            strncpy(weapon->mName, sqlite3_column_text(stmt, 1),
                sizeof(weapon->mName));
            strncpy(weapon->mModel, sqlite3_column_text(stmt, 2),
                sizeof(weapon->mModel));

            strncpy(frames, sqlite3_column_text(stmt, 3), sizeof(frames));
            BG_OpenWeaponFrames(frames);
        }
    }

    sqlite3_finalize(stmt);

    return qtrue;
}

static qboolean BG_ParseWeapon(weapon_t weapon, sqlite3 * db)
{
    int             i = 0;
    TAnimWeapon     *anims;
    TAnimInfoWeapon *infos;

    // Clear some structures. This primarily fixes unexpected behavior on our
    // Woody v1.00 build, namely across map restarts.
    memset(&weaponParseInfo[weapon], 0, sizeof(TWeaponParseInfo));
    weaponParseInfo[weapon].mName = bg_weaponNames[weapon];

    // Parse weapon models.
    BG_ParseWeaponGroup(&weaponParseInfo[weapon].mWeaponModel, weapon, db);

    // Parse weapon animations.
    BG_ParseAnimGroup(weapon, db);

    // Iterate through fetched animations.
    anims = weaponParseInfo[weapon].mAnimList;
    while(anims){
        infos = anims->mInfos;
        while(infos){
            for(i = 0; i < infos->mNumChoices; i++){
                BG_FindWeaponFrames(infos, i);
            }
            infos = infos->mNext;
        }

        anims = anims->mNext;
    }

    BG_CloseWeaponFrames(numInitialFiles);
    return qtrue;
}

qboolean BG_ParseInviewFile(void)
{
    int         i;
    sqlite3     *db;
    int
        rc;
    // Boe!Man 12/5/12
    // The file can be on two locations. The DLL should always be in the
    // fs_game folder, however, this could be misconfigured.
    // The Mod takes care of this problem and should load the file correctly,
    // even if misplaced.

    // We use the *_v2 routine so the database isn't created if it isn't found.
    rc = sqlite3_open_v2(va("./%s", g_inviewDb.string), &db,
        SQLITE_OPEN_READONLY, NULL);
    if(rc){
        // Alternate location, probably next to the binary.
        char fsGame[MAX_QPATH];

        trap_Cvar_VariableStringBuffer("fs_game", fsGame, sizeof(fsGame));
        rc = sqlite3_open_v2(va("./%s/%s", fsGame, g_inviewDb.string), &db,
            SQLITE_OPEN_READONLY, NULL);

        if(rc){
            G_LogPrintf("^1Error: ^7Inview database: %s\n", sqlite3_errmsg(db));
            Com_Error(ERR_FATAL_NOLOG, "Failed to load inview database: %s",
                sqlite3_errmsg(db));
        }else{
            level.altPath = qtrue;
            Q_strncpyz(level.altString, va("./%s", fsGame),
                sizeof(level.altString));
        }
    }

    // Make a copy of the availableWeapons string, and update CVARs accordingly.
    Q_strncpyz(availableWeapons.string, availableWeapons.string,
        level.wpNumWeapons);
    trap_Cvar_Update(&availableWeapons);

    // Make sure we clear everything when initializing.
    // The Linux v1.00 Woody build does not always do this properly.
    memset(frameGroup, 0, sizeof(frameGroup));
    memset(weaponFrames, 0, sizeof(weaponFrames));
    numWeaponFiles = 0;
    numInitialFiles = 0;

    weaponLeftHand[0] = 0;
    weaponRightHand[0] = 0;

    strcpy(weaponLeftHand, "models/weapons/lhand/lhand.glm");
    if( BG_OpenWeaponFrames("skeletons/weapons/lhand/lhand.frames"))
        numInitialFiles++;
    strcpy(weaponRightHand, "models/weapons/rhand/rhand.glm");
    if( BG_OpenWeaponFrames("skeletons/weapons/rhand/rhand.frames"))
        numInitialFiles++;

    // Iterate through the weapons.
    for(i = 1; i < level.wpNumWeapons; i++){
        BG_ParseWeapon((weapon_t)i, db);
    }

    #ifdef _DEBUG
    if (i == level.wpNumWeapons)
    {
        Com_Printf("BG_InitWeapons: Done\n");
    }
    #endif
    BG_CloseWeaponFrames(0);

    BG_InitAmmoStats();
    sqlite3_close(db);
    return BG_InitWeaponStats(qtrue);
}

TAnimWeapon *BG_GetInviewAnim(int weaponIdx,const char *animKey,int *animIndex)
{
    TAnimWeapon         *animWeapon;

    (*animIndex)=0;
    animWeapon=weaponParseInfo[weaponIdx].mAnimList;
    while((animWeapon!=0)&&(Q_stricmp(animWeapon->mName,animKey)))
    {
        animWeapon=animWeapon->mNext;
        (*animIndex)++;
    }
    if(!animWeapon)
    {
        return(0);
    }
    return(animWeapon);
}

TAnimWeapon *BG_GetInviewAnimFromIndex(int weaponIdx,int animIndex)
{
    TAnimWeapon     *animWeapon;
    int             i=0;

    animWeapon=weaponParseInfo[weaponIdx].mAnimList;
    while((animWeapon!=0)&&(i!=animIndex))
    {
        animWeapon=animWeapon->mNext;
        i++;
    }
    if(!animWeapon)
    {
        return(0);
    }
    return(animWeapon);
}

TAnimInfoWeapon *BG_GetInviewModelAnim(int weaponIdx,const char *modelKey,const char *animKey)
{
    TAnimWeapon         *animWeapon;
    TAnimInfoWeapon     *animInfoWeapon;
    animWeapon=weaponParseInfo[weaponIdx].mAnimList;
    while((animWeapon!=0)&&(Q_stricmp(animWeapon->mName,animKey)))
    {
        animWeapon=animWeapon->mNext;
    }
    if(!animWeapon)
    {
        return(0);
    }
    animInfoWeapon=animWeapon->mInfos;
    while((animInfoWeapon!=0)&&(Q_stricmp(animInfoWeapon->mType,modelKey)))
    {
        animInfoWeapon=animInfoWeapon->mNext;
    }
    if(!animInfoWeapon)
    {
        return(0);
    }
    return(animInfoWeapon);
}

/*
===============
BG_WeaponHasAlternateAmmo

Returns qtrue if the given weapon has ammo for its alternate attack
===============
*/
qboolean BG_WeaponHasAlternateAmmo ( int weapon )
{
    // No valid ammo index means no alternate ammo
    if ( weaponData[weapon].attack[ATTACK_ALTERNATE].ammoIndex == AMMO_NONE )
    {
        return qfalse;
    }

    // If the alternate attack doesnt deplete ammo then it doesnt use it
    if ( !weaponData[weapon].attack[ATTACK_ALTERNATE].fireAmount )
    {
        return qfalse;
    }

    // If the alternate ammo is the same as the primary ammo then
    // the primary is good enough
    if ( weaponData[weapon].attack[ATTACK_ALTERNATE].ammoIndex ==
         weaponData[weapon].attack[ATTACK_NORMAL].ammoIndex )
    {
        return qfalse;
    }

    // Yup, alternates have ammo
    return qtrue;
}

/*
===============
BG_FindFireMode

Finds the firemode for the given weapon using the given default
===============
*/
int BG_FindFireMode ( weapon_t weapon, attackType_t attack, int firemode )
{
    int i;

    if ( !weapon )
    {
        return WP_FIREMODE_NONE;
    }

    for ( i=0; i <= WP_FIREMODE_SINGLE; i++ )
    {
        if( firemode >= WP_FIREMODE_MAX )
        {
            firemode = WP_FIREMODE_NONE + 1;
        }

        if( weaponData[weapon].attack[ATTACK_NORMAL].weaponFlags&(1<<firemode))
        {
            break;
        }
        else
        {
            firemode++;
        }
    }

    assert ( firemode < WP_FIREMODE_MAX );

    return firemode;
}

/*
===============
BG_CalculateBulletEndpoint

Calculates the end point of a bullet based on the given inaccuracy and range
===============
*/
void BG_CalculateBulletEndpoint ( vec3_t muzzlePoint, vec3_t fireAngs, float inaccuracy, float range, vec3_t end, int *seed )
{
    float   fGaussianX = 0;
    float   fGaussianY = 0;
    vec3_t  dir;
    vec3_t  fwd;
    vec3_t  up;
    vec3_t  right;

    AngleVectors ( fireAngs, fwd, right, up );

    // No inaccuracy so just extend it forward by the range
    if ( inaccuracy <= 0.0f )
    {
        VectorMA (muzzlePoint, range, fwd, end);
        return;
    }

    // Gaussian spread should keep it a bit less random looking
    while ( 1 )
    {
        float fGaussian;
        float f1;
        float f2;

        f1 = (float)(Q_rand ( seed ) % 15000) / 15000.0f;
        f2 = (float)(Q_rand ( seed ) % 15000) / 15000.0f;
        fGaussianX = (f1-0.5f) + (f2-0.5f);

        f1 = (float)(Q_rand ( seed ) % 15000) / 15000.0f;
        f2 = (float)(Q_rand ( seed ) % 15000) / 15000.0f;
        fGaussianY = (f1-0.5f) + (f2-0.5f);

        fGaussian = fGaussianX * fGaussianX + fGaussianY * fGaussianY;

        if ( fGaussian < 1 )
        {
            break;
        }
    }

    VectorMA ( fwd, 0.05f * inaccuracy * fGaussianX, right, dir );
    VectorMA ( dir, 0.05f * inaccuracy * fGaussianY, up, dir );

    VectorMA (muzzlePoint, range, dir, end);
}

/*
===============
BG_GetMaxAmmo

Returns the max ammo a client can hold for the given ammo index
===============
*/
int BG_GetMaxAmmo ( const playerState_t* ps, int ammoIndex )
{
    int         ammo;
    int weapon;

    if ( ammoIndex == AMMO_NONE )
    {
        return 0;
    }

    for ( ammo = 0, weapon = WP_KNIFE; weapon < level.wpNumWeapons; weapon ++ )
    {
        if ( !(ps->stats[STAT_WEAPONS] & (1<<weapon)) )
        {
            if ( weapon != ps->stats[STAT_OUTFIT_GRENADE] )
            {
                continue;
            }
        }

        if ( weaponData[weapon].attack[ATTACK_NORMAL].ammoIndex == ammoIndex )
        {
            ammo += (weaponData[weapon].attack[ATTACK_NORMAL].extraClips + 1) * weaponData[weapon].attack[ATTACK_NORMAL].clipSize;
            ammo -= ps->clip[ATTACK_NORMAL][weapon];
        }

        if ( BG_WeaponHasAlternateAmmo ( weapon ) )
        {
            if ( weaponData[weapon].attack[ATTACK_ALTERNATE].ammoIndex == ammoIndex )
            {
                ammo += (weaponData[weapon].attack[ATTACK_ALTERNATE].extraClips + 1) * weaponData[weapon].attack[ATTACK_ALTERNATE].clipSize;
                ammo -= ps->clip[ATTACK_ALTERNATE][weapon];
            }
        }
    }

    return ammo;
}
