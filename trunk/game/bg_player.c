// Copyright (C) 2001-2002 Raven Software.
//
// BG_Player.c

#include "q_shared.h"
#include "bg_public.h"
#include "bg_local.h"

#include "../ghoul2/g2.h"

#include "../cgame/animtable.h"

#ifdef QAGAME
#include "g_local.h"
#endif

#ifdef UI_EXPORTS
#include "../ui/ui_local.h"
#endif

#ifndef UI_EXPORTS
#ifndef QAGAME
#include "../cgame/cg_local.h"
#endif
#endif

TCharacterTemplate      *bg_characterTemplates = NULL;
TItemTemplate           *bg_itemTemplates      = NULL;
int                     bg_identityCount       = 0;
TIdentity               bg_identities[MAX_IDENTITIES];

goutfitting_t           bg_outfittings[MAX_OUTFITTINGS];
int                     bg_outfittingCount     = 0;

char                    bg_availableOutfitting[WP_MAX_WEAPONS] = {-1};

#ifndef _GOLD
int bg_outfittingGroups[OUTFITTING_GROUP_MAX][MAX_OUTFITTING_GROUPITEM] =
{
    { MODELINDEX_WEAPON_AK74,       MODELINDEX_WEAPON_M4,       MODELINDEX_WEAPON_USAS12, MODELINDEX_WEAPON_MSG90A1,    MODELINDEX_WEAPON_M60,      MODELINDEX_WEAPON_RPG7,  MODELINDEX_WEAPON_MM1, -1, -1, -1 },
    { MODELINDEX_WEAPON_M590,       MODELINDEX_WEAPON_MICROUZI, MODELINDEX_WEAPON_M3A1,     -1, -1, -1, -1, -1, -1, - 1 },
    { MODELINDEX_WEAPON_M19,        MODELINDEX_WEAPON_SOCOM,    -1,                         -1,                         -1, -1, -1, -1, -1, -1 },
    { MODELINDEX_WEAPON_SMOHG92,    MODELINDEX_WEAPON_M84,      MODELINDEX_WEAPON_M15,      MODELINDEX_WEAPON_ANM14,    -1, -1, -1, -1, -1, -1 },
    { MODELINDEX_ARMOR,             MODELINDEX_NIGHTVISION,     MODELINDEX_THERMAL,         -1,                         -1, -1, -1, -1, -1, -1 },
};
#else
int bg_outfittingGroups[OUTFITTING_GROUP_MAX][MAX_OUTFITTING_GROUPITEM] =
{
    { MODELINDEX_WEAPON_AK74,       MODELINDEX_WEAPON_M4,       MODELINDEX_WEAPON_SIG551,   MODELINDEX_WEAPON_USAS12, MODELINDEX_WEAPON_MSG90A1,    MODELINDEX_WEAPON_M60,  MODELINDEX_WEAPON_MP5,  MODELINDEX_WEAPON_RPG7,  MODELINDEX_WEAPON_MM1, -1, -1, -1 },
    { MODELINDEX_WEAPON_M590,       MODELINDEX_WEAPON_MICROUZI, MODELINDEX_WEAPON_M3A1,     -1, -1, -1, -1, -1, -1, -1, -1, -1 },
    { MODELINDEX_WEAPON_M19,        MODELINDEX_WEAPON_SOCOM,    MODELINDEX_WEAPON_SILVERTALON,  -1,                         -1, -1, -1, -1, -1, -1, -1, -1 },
    { MODELINDEX_WEAPON_SMOHG92,    MODELINDEX_WEAPON_M84,      MODELINDEX_WEAPON_M15,      MODELINDEX_WEAPON_ANM14,    -1, -1, -1, -1, -1, -1, -1, -1 },
    { MODELINDEX_ARMOR,             MODELINDEX_NIGHTVISION,     MODELINDEX_THERMAL,         -1,                         -1, -1, -1, -1, -1, -1, -1, -1 },
};
#endif // not _GOLD

/*
===================
PM_StartLegsAnim

Starts a new leg animation for the given playerstate
===================
*/
static void PM_StartLegsAnim( playerState_t* ps, int anim )
{
    if ( ps->pm_type >= PM_DEAD )
    {
        return;
    }

    ps->legsAnim = ( ( ps->legsAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;
}

/*
===================
PM_ContinueLegsAnim

Continues running the given leg animation, if its a new animation then it is started
===================
*/
void PM_ContinueLegsAnim( playerState_t* ps, int anim )
{
    if ( ( ps->legsAnim & ~ANIM_TOGGLEBIT ) == anim )
    {
        return;
    }

    PM_StartLegsAnim( ps, anim );
}

/*
===================
PM_ForceLegsAnim
===================
*/
void PM_ForceLegsAnim( playerState_t* ps, int anim)
{
    PM_StartLegsAnim( ps, anim );
}

/*
===================
PM_StartTorsoAnim
===================
*/
void PM_StartTorsoAnim( playerState_t* ps, int anim, int time )
{
    if ( anim == -1 )
    {
        return;
    }

    if ( ps->pm_type >= PM_DEAD )
    {
        return;
    }

    ps->torsoAnim  = ( ( ps->torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;
    ps->torsoTimer = time;
}

/*
===================
PM_ContinueTorsoAnim

Continues running the given torso animation, if its a new animation then it is started
===================
*/
static void PM_ContinueTorsoAnim( playerState_t* ps, int anim )
{
    if ( anim == -1 )
    {
        return;
    }

    if ( ( ps->torsoAnim & ~ANIM_TOGGLEBIT ) == anim )
    {
        return;
    }

    PM_StartTorsoAnim( ps, anim, 0 );
}

/*
==============
PM_TorsoAnimation

Sets the current torso animation based on the given playerstate
==============
*/
void PM_TorsoAnimation( playerState_t* ps )
{
    switch ( ps->weaponstate )
    {
        case WEAPON_SPAWNING:
        case WEAPON_READY:
            #ifdef _GOLD
            if (ps->stats[STAT_USEWEAPONDROP])
            {
                PM_ContinueTorsoAnim(ps, TORSO_USE);
            }
            else if ((ps->pm_flags & PMF_ZOOMED) && weaponData[ps->weapon].animIdleZoomed)
            #else
            if ( (ps->pm_flags & PMF_ZOOMED) && weaponData[ps->weapon].animIdleZoomed )
            #endif // _GOLD
            {
                PM_ContinueTorsoAnim ( ps, weaponData[ps->weapon].animIdleZoomed );
            }
            else
            {
                PM_ContinueTorsoAnim ( ps, weaponData[ps->weapon].animIdle );
            }

            break;
    }
}

/*
=================
BG_ParseInventory

Parses the inventory items indicated in the given group and returns
a linked list containing the results
=================
*/
TInventoryTemplate *BG_ParseInventory ( TGPGroup group )
{
    TInventoryTemplate  *top, *inv;
    TGPGroup            subGroup;
    char                temp[1024];

    // Convienience for handling no items
    if (!group)
    {
        return 0;
    }

    top = NULL;

    // Parse each of the inventory items
    subGroup = trap_GPG_GetSubGroups( group );
    while(subGroup)
    {
        trap_GPG_GetName ( subGroup, temp );

        // If the group name isnt item then 'Item' then its not an inventory item
        if ( Q_stricmp ( temp, "Item") == 0)
        {
            inv = (TInventoryTemplate *)trap_VM_LocalAlloc ( sizeof(*inv));

            // Name of the item
            trap_GPG_FindPairValue ( subGroup, "Name||Name1", "", temp );
            inv->mName = trap_VM_LocalStringAlloc ( temp );

            // Bolt for the item
            trap_GPG_FindPairValue ( subGroup, "Bolt", "", temp );
            inv->mBolt = trap_VM_LocalStringAlloc ( temp );

            trap_GPG_FindPairValue ( subGroup, "mp_onback||onback", "no", temp );
            if ( !Q_stricmp ( temp, "yes" ) )
            {
                inv->mOnBack = qtrue;
            }

            inv->mNext = top;
            top = inv;
        }

        // Move to the next group
        subGroup = trap_GPG_GetNext ( subGroup );
    }

    return top;
}

/*
=================
BG_ParseSkins

Parses the skins contained in the given group and returns a linked
list with the results
=================
*/
TSkinTemplate *BG_ParseSkins( TCharacterTemplate* character, TGPGroup group )
{
    TSkinTemplate   *skin;
    TIdentity       *identity;
    TGPGroup        subGroup;
    char            temp[1024];
    fileHandle_t    f;
#ifndef SPECIAL_PRE_CACHE
    int             len;
#endif
    qboolean        validSkin;

    character->mSkins = NULL;

    // Parse the skin file first
    trap_GPG_FindPairValue ( group, "SkinFile", "", temp );
    if ( temp[0] )
    {
        skin = (TSkinTemplate *) trap_VM_LocalAlloc ( sizeof(*skin) );
        skin->mSkin = trap_VM_LocalStringAlloc ( temp );
        skin->mNext = character->mSkins;
        character->mSkins = skin;
    }

    // Now parse all the skin groups
    subGroup = trap_GPG_GetSubGroups ( group );
    while(subGroup)
    {
        trap_GPG_GetName ( subGroup, temp );

        // If the groups name isnt 'Skin' then skip it
        if ( Q_stricmp( temp, "Skin") == 0 )
        {
            // Allocate memory for the skin & grab the skin filename
            trap_GPG_FindPairValue ( subGroup, "File", "", temp );
            skin = (TSkinTemplate *) trap_VM_LocalAlloc ( sizeof(*skin) );
            skin->mSkin = trap_VM_LocalStringAlloc ( temp );

#ifdef SPECIAL_PRE_CACHE
            f = 0;
            validSkin = qtrue;
#else
            Com_sprintf( temp, sizeof(temp), "models/characters/skins/%s.g2skin", skin->mSkin );
            len = trap_FS_FOpenFile( temp, &f, FS_READ );
            if (f != 0)
            {
                trap_FS_FCloseFile(f);
                validSkin = qtrue;
            }
            else
            {
                validSkin = qfalse;
            }
#endif

            // Parse the inventory for the skin
            skin->mInventory = BG_ParseInventory( trap_GPG_FindSubGroup ( subGroup, "Inventory") );

            // Link the skin into the skin linked list
            skin->mNext = character->mSkins;
            character->mSkins = skin;

            // If the character isnt deathmatch then dont add it to the
            // identity list
            if ( character->mDeathmatch && validSkin)
            {
                // Allocate a new identity
                identity = &bg_identities[bg_identityCount++];

                identity->mCharacter = character;
                identity->mSkin      = skin;

                trap_GPG_FindPairValue ( subGroup, "mp_identity", "", temp );
                if ( !temp[0] )
                {
                    identity->mName = trap_VM_LocalStringAlloc ( va("%s/%s", character->mName, skin->mSkin ) );
                }
                else
                {
                    identity->mName = trap_VM_LocalStringAlloc ( temp );
                }

                // Team name?
                trap_GPG_FindPairValue ( subGroup, "mp_team", "", temp );
                if ( temp[0] )
                {
                    identity->mTeam = trap_VM_LocalStringAlloc ( temp );
                }
                else
                {
                    identity->mTeam = "";
                }
            }
        }

        // Move to the next sub group in the parsers list
        subGroup = trap_GPG_GetNext ( subGroup );
    }

    return character->mSkins;
}

/*
=================
BG_ParseModelSounds

Parses the model sounds for the given group and returns a linked
list with the results
=================
*/
TModelSounds *BG_ParseModelSounds( TGPGroup group )
{
    TModelSounds    *top;
    TModelSounds    *sounds;
    TGPGroup        pairs;
    char            temp[1024];

    // Convienience
    if ( !group )
    {
        return NULL;
    }

    top = NULL;

    // Now parse all the skin groups
    pairs = trap_GPG_GetPairs ( group );
    while(pairs)
    {
        // Allocate memory for the sounds
        sounds = (TModelSounds*) trap_VM_LocalAlloc ( sizeof(*sounds) );
        sounds->mNext = top;
        top = sounds;

        // Grab the sounds name
        trap_GPV_GetName ( pairs, temp );
        sounds->mName = trap_VM_LocalStringAlloc ( temp );

        // Start with no sounds
        sounds->mCount = 0;

        // Should be a list
        if ( trap_GPV_IsList ( pairs ) )
        {
            TGPValue list;

            // Run through the list
            list = trap_GPV_GetList ( pairs );
            while ( list && sounds->mCount < MAX_MODEL_SOUNDS )
            {
                // Add the sound to the list
                trap_GPV_GetName ( list, temp );
                sounds->mCount++;
                sounds->mSounds[sounds->mCount] = trap_VM_LocalStringAlloc ( temp );

                list = trap_GPV_GetNext ( list );
            }
        }

        // Move to the next sound set in the parsers list
        pairs = trap_GPV_GetNext ( pairs );
    }

    return top;
}

/*
=================
BG_ParseItemFile

Parses the item file.  The item file contains a list of all of the
items that can be used as inventory items for a given skin
=================
*/
qboolean BG_ParseItemFile ( void )
{
    TGPGroup        *baseGroup, *subGroup;
    TGPValue        *pairs;
    TItemTemplate   *item;
    TSurfaceList    *surf;
    char            temp[1024];
    TGenericParser2 ItemFile;

    // Create the generic parser so the item file can be parsed
    ItemFile = trap_GP_ParseFile( "ext_data/sof2.item", qtrue, qfalse );
    if ( !ItemFile )
    {
        return qfalse;
    }

    // Boe!Man 9/27/12: Native on newer Linux-based systems fix.
#ifdef __linux__
    bg_itemTemplates = NULL;
#endif

    baseGroup = (TGPGroup *)trap_GP_GetBaseParseGroup ( ItemFile );
    subGroup = (TGPGroup *)trap_GPG_GetSubGroups ( baseGroup );
    while(subGroup)
    {
        trap_GPG_GetName ( subGroup, temp );

        if (Q_stricmp( temp, "item") == 0)
        {
            // Is this item used for deathmatch?
            trap_GPG_FindPairValue ( subGroup, "Deathmatch", "yes", temp );
            if (Q_stricmp( temp, "no") == 0)
            {
                subGroup = (TGPGroup *)trap_GPG_GetNext ( subGroup );
                continue;
            }

            // Allocate the item template and link it up to the item list
            item = (TItemTemplate *) trap_VM_LocalAlloc ( sizeof(*item) );
            item->mNext   = bg_itemTemplates;
            bg_itemTemplates = item;

            // Name of the item
            trap_GPG_FindPairValue ( subGroup, "Name", "", temp );
            item->mName = trap_VM_LocalStringAlloc ( temp );

            // Model for the item
            trap_GPG_FindPairValue ( subGroup, "Model", "", temp );
            if ( temp[0] )
            {
                item->mModel = trap_VM_LocalStringAlloc ( temp );
            }

            pairs = (TGPValue *)trap_GPG_GetPairs ( subGroup );
            while(pairs)
            {
                trap_GPV_GetName ( pairs, temp );

                // Surface off?
                if ( Q_stricmpn ( temp, "offsurf", 7) == 0)
                {
                    // Allocate the surface structure and link it into the list
                    surf = (TSurfaceList *) trap_VM_LocalAlloc ( sizeof(*surf) );
                    surf->mNext = item->mOffList;
                    item->mOffList = surf;

                    // Name of the surface to turn off
                    trap_GPV_GetTopValue ( pairs, temp );
                    surf->mName = trap_VM_LocalStringAlloc ( temp );
                }
                // Surface on?
                else if ( Q_stricmpn( temp, "onsurf", 6) == 0)
                {
                    // Allocate the surface structure and link it into the list
                    surf = (TSurfaceList *) trap_VM_LocalAlloc(sizeof(*surf));
                    surf->mNext = item->mOnList;
                    item->mOnList = surf;

                    // Name of the surface to turn off
                    trap_GPV_GetTopValue ( pairs, temp );
                    surf->mName = trap_VM_LocalStringAlloc ( temp );
                }

                // Next pairs
                pairs = (TGPValue *)trap_GPV_GetNext ( pairs );
            }
        }

        // Next group
        subGroup = (TGPGroup *)trap_GPG_GetNext ( subGroup );
    }

    trap_GP_Delete(&ItemFile);

    return qtrue;
}

/*
=================
BG_FindCharacterTemplate

Finds a character template the matches the given name or NULL if
a match could not be found
=================
*/
TCharacterTemplate *BG_FindCharacterTemplate (const char *name)
{
    TCharacterTemplate  *current;

    // Convienience
    if (!name)
    {
        return NULL;
    }

    // Linear search through all of the parsed templates
    current = bg_characterTemplates;
    while(current)
    {
        if (Q_stricmp(name, current->mName) == 0)
        {
            return current;
        }

        current = current->mNext;
    }

    // None found
    return NULL;
}

/*
=================
BG_FindItemTemplate

Finds an item template the matches the given name or NULL if a match
could not be found
=================
*/
TItemTemplate *BG_FindItemTemplate(const char *name)
{
    TItemTemplate       *current;

    // Convienience
    if (!name)
    {
        return NULL;
    }

    // Linear search through all of the parsed items
    current = bg_itemTemplates;
    while(current)
    {
        if (Q_stricmp(name, current->mName) == 0)
        {
            return current;
        }

        current = current->mNext;
    }

    return NULL;
}

/*
=================
BG_LinkTemplates

Cross links the various templates
=================
*/
static void BG_LinkTemplates(void)
{
    TCharacterTemplate  *current;
    TInventoryTemplate  *inv;
    TSkinTemplate       *skin;

    current = bg_characterTemplates;
    while(current)
    {
        // If this template has a parent then find it and link it up as
        // its parent.  Ensure that the parent doesnt link back to itself
        current->mParent = BG_FindCharacterTemplate(current->mParentName);
        if (current->mParent == current)
        {
            current->mParent = NULL;
        }
        // Bring over any parent items
        else if ( current->mParent )
        {
            // No model, bring over the parents.
            if ( !current->mModel )
            {
                current->mModel = current->mParent->mModel;
            }
        }

        // Link up all the inventory for this character
        inv = current->mInventory;
        while(inv)
        {
            inv->mItem = BG_FindItemTemplate(inv->mName);
            inv = inv->mNext;
        }

        // Link up all the skins for this character
        skin = current->mSkins;
        while(skin)
        {
            // Link up all the inventory items specific to the skins
            inv = skin->mInventory;
            while(inv)
            {
                inv->mItem = BG_FindItemTemplate(inv->mName);

                inv = inv->mNext;
            }

            skin = skin->mNext;
        }

        // Move on to the next character template
        current = current->mNext;
    }
}

/*
=================
BG_ParseNPCFiles

Parses all the the .npc files in the npc directory and
stores their info into global lists.
=================
*/
qboolean BG_ParseNPCFiles ( void )
{
    int                 i, numNPCFiles, filelen;
    TGPGroup            baseGroup, subGroup;
    TGenericParser2     NPCFile;
    const char          *currentParent = 0;
    TCharacterTemplate  *newTemplate;
    char                fileName[MAX_QPATH];
    char                NPCFiles[4096];
    char                temp[1024];
    char                *fileptr;

    // Clear the current list
    bg_characterTemplates = NULL;

    // Grab the list of NPC files
    numNPCFiles = trap_FS_GetFileList("NPCs", ".npc", NPCFiles, 4096 );
    if ( !numNPCFiles )
    {
        return qfalse;
    }

    // Parse each of the NPC files
    fileptr = NPCFiles;
    for( i=0; i<numNPCFiles; i++, fileptr += filelen+1 )
    {
        // Grab the length so we can skip this file later
        filelen = strlen(fileptr);

        Com_sprintf(fileName, sizeof(fileName),"NPCs/%s", fileptr );

        // Create the generic parser so the item file can be parsed
        NPCFile = trap_GP_ParseFile( fileName, qtrue, qfalse );
        if ( !NPCFile )
        {
            continue;
        }

        baseGroup = trap_GP_GetBaseParseGroup ( NPCFile );
        subGroup = trap_GPG_GetSubGroups ( baseGroup );

        while(subGroup)
        {
            trap_GPG_GetName ( subGroup, temp );

            // Look for a parent template if this is the group info
            if ( Q_stricmp( temp, "GroupInfo") == 0)
            {
                currentParent = NULL;

                // Is there a parent template?
                trap_GPG_FindPairValue ( subGroup, "ParentTemplate", "", temp );
                if ( temp[0] )
                {
                    currentParent = trap_VM_LocalStringAlloc ( temp );
                }
            }
            // A new character template
            else if ( Q_stricmp( temp, "CharacterTemplate") == 0)
            {
                // Allocate the new template and link it into the global list.
                newTemplate = (TCharacterTemplate *)trap_VM_LocalAlloc (sizeof(*newTemplate));
                newTemplate->mNext = bg_characterTemplates;
                bg_characterTemplates = newTemplate;

                // Exclude from deathmatch?
                trap_GPG_FindPairValue ( subGroup, "DeathMatch", "yes", temp );
                if ( Q_stricmp( temp, "no") == 0)
                {
                    newTemplate->mDeathmatch = qfalse;
                }
                else
                {
                    newTemplate->mDeathmatch = qtrue;
                }

                // Template name
                trap_GPG_FindPairValue ( subGroup, "Name", "", temp );
                if ( temp[0] )
                {
                    newTemplate->mName = trap_VM_LocalStringAlloc ( temp );
                }

                // Template formal name
                trap_GPG_FindPairValue ( subGroup, "FormalName", "", temp );
                if ( temp[0] )
                {
                    newTemplate->mFormalName = trap_VM_LocalStringAlloc ( temp );
                }

                // Template model
                trap_GPG_FindPairValue ( subGroup, "Model", "", temp );
                if ( temp[0] )
                {
                    newTemplate->mModel = trap_VM_LocalStringAlloc ( temp );
                }

                // Use the current parent
                newTemplate->mParentName = currentParent;

                // Parse inventory for this character template
                newTemplate->mInventory = BG_ParseInventory( trap_GPG_FindSubGroup( subGroup, "Inventory" ));

                // Parse the skins for this character template
                BG_ParseSkins( newTemplate, subGroup);

                // Parse the sounds for this character template
                newTemplate->mSounds = BG_ParseModelSounds ( trap_GPG_FindSubGroup ( subGroup, "MPSounds" ) );
            }

            // Move to the next group
            subGroup = trap_GPG_GetNext ( subGroup );
        }

        trap_GP_Delete(&NPCFile);
    }

    // Parse the item file
    BG_ParseItemFile();

    // Link up all the templates
    BG_LinkTemplates();

    return qtrue;
}

/*
=================
BG_GetModelSoundsGroup

Returns the group of sounds for the given model and sound group combination, if the
sound group couldnt be found NULL is returned
=================
*/
TModelSounds* BG_GetModelSoundsGroup ( const char* Identity, const char* SoundGroup )
{
    TIdentity           *identity;
    TCharacterTemplate  *character;
    TModelSounds        *sounds;

    // Grab the identity in question
    identity = BG_FindIdentity (Identity );
    if ( !identity )
    {
        return NULL;
    }

    character = identity->mCharacter;

    while ( character )
    {
        // Run through the sounds and look for the match
        sounds = character->mSounds;
        while ( sounds )
        {
            // Match?
            if ( !Q_stricmp ( sounds->mName, SoundGroup ) )
            {
                return sounds;
            }

            sounds = sounds->mNext;
        }

        character = character->mParent;
    }

    // Not found
    return NULL;
}

/*
=================
BG_GetModelSoundCount

Return the number of sounds for the given model
=================
*/
int BG_GetModelSoundCount ( const char *Identity, const char *SoundGroup )
{
    TModelSounds* sounds;

    // Grab the sounds
    sounds = BG_GetModelSoundsGroup( Identity, SoundGroup );
    if ( !sounds )
    {
        return 0;
    }

    // Return the sound count
    return sounds->mCount;
}

/*
=================
BG_GetModelSound

Returns the model sound for the given sound group and index.  If the sound
could not be found then NULL is returned.
=================
*/
const char *BG_GetModelSound ( const char *Identity, const char *SoundGroup, int index )
{
    TModelSounds    *sounds;

    // Grab the sounds
    sounds = BG_GetModelSoundsGroup( Identity, SoundGroup );
    if ( !sounds )
    {
        return NULL;
    }

    // Invalid index?
    if ( index >= sounds->mCount )
    {
        return "";
    }

    // Run through the sounds and look for the match
    return sounds->mSounds[index];
}

/*
=================
BG_FindIdentity

Returns the identity with the given name
=================
*/
TIdentity *BG_FindIdentity ( const char *identityName )
{
    int i;

    // Convienience
    if (!identityName)
    {
        return NULL;
    }

    // Linear search through all of the parsed identities
    for ( i = 0; i < bg_identityCount; i ++ )
    {
        if (Q_stricmp(identityName, bg_identities[i].mName) == 0)
        {
            return &bg_identities[i];
        }
    }

    // None found
    return NULL;
}

/*
=================
BG_FindTeamIdentity

returns the first identity which matches the given team name
=================
*/
#define MAX_TEAM_IDENTS 5
TIdentity* BG_FindTeamIdentity ( const char* teamName, int index )
{
    TIdentity* idents[MAX_TEAM_IDENTS];
    int        count;
    int        i;

    // Convienience
    if ( !teamName )
    {
        return NULL;
    }

    // Linear search through all of the parsed identities
    for ( i = 0, count = 0; i < bg_identityCount && count < MAX_TEAM_IDENTS; i ++ )
    {
        if (Q_stricmp(teamName, bg_identities[i].mTeam) == 0)
        {
            idents[count++] = &bg_identities[i];
        }
    }

    if ( !count )
    {
        return NULL;
    }

    if ( index != -1 )
    {
        if ( index >= count )
        {
            index = count - 1;
        }

        return idents[index];
    }

    // None found
    return idents[rand()%count];
}


/*
=================
BG_ParseSkin

Reads a skin file into a null terminated list and returns the number found
=================
*/
int BG_ParseSkin ( const char* filename, char* pairs, int pairsSize )
{
    TGenericParser2 skinFile;
    TGPGroup        *basegroup;
    TGPGroup        *group;
    TGPGroup        *sub;
    char            name[MAX_QPATH];
    char*           end;
    int             numPairs;

    // Open the skin file
    skinFile = trap_GP_ParseFile( (char*)filename, qtrue, qfalse );
    if ( !skinFile )
    {
        return 0;
    }

    numPairs  = 0;
    end       = pairs;
    *end      = 0;
    basegroup = (TGPGroup *)trap_GP_GetBaseParseGroup ( skinFile );
    group     = (TGPGroup *)trap_GPG_GetSubGroups ( basegroup );

    while(group)
    {
        trap_GPG_GetName ( group, name );

        // Parse the material
        if ( Q_stricmp ( name, "material") == 0)
        {
            char    matName[MAX_QPATH];
            char    shaderName[MAX_QPATH];

            trap_GPG_FindPairValue ( group, "name", "", matName );

            sub = (TGPGroup *)trap_GPG_FindSubGroup ( group, "group");
            if (sub)
            {
                trap_GPG_FindPairValue ( sub, "shader1", "", shaderName );
                if (!shaderName[0])
                {
                    trap_GPG_FindPairValue ( sub, "texture1", "", shaderName );
                }
            }

            if (matName[0] && shaderName[0])
            {
                int size;

                size = Com_sprintf(end, pairsSize, "%s %s ", matName, shaderName);
                end += size;
                pairsSize -= size;
                numPairs++;
            }
        }

        group = (TGPGroup *)trap_GPG_GetNext ( group );
    }

    trap_GP_Delete(&skinFile);

    return numPairs;
}

/*
==================
BG_SwingAngles
==================
*/
static void BG_SwingAngles (
    float       destination,
    float       swingTolerance,
    float       clampTolerance,
    float       speed,
    float       *angle,
    qboolean    *swinging,
    int         frameTime
    )
{
    float   swing;
    float   move;
    float   scale;

    if ( !*swinging )
    {
        // see if a swing should be started
        swing = AngleSubtract( *angle, destination );
        if ( swing > swingTolerance || swing < -swingTolerance )
        {
            *swinging = qtrue;
        }
    }

    if ( !*swinging )
    {
        return;
    }

    // modify the speed depending on the delta
    // so it doesn't seem so linear
    swing = AngleSubtract( destination, *angle );
    scale = fabs( swing );
    if ( scale < swingTolerance * 0.5 )
    {
        scale = 0.5;
    }
    else if ( scale < swingTolerance )
    {
        scale = 1.0;
    }
    else
    {
        scale = 2.0;
    }

    // swing towards the destination angle
    if ( swing >= 0 )
    {
        move = frameTime * scale * speed;
        if ( move >= swing )
        {
            move = swing;
            *swinging = qfalse;
        }
        *angle = AngleMod( *angle + move );
    }
    else if ( swing < 0 )
    {
        move = frameTime * scale * -speed;
        if ( move <= swing )
        {
            move = swing;
            *swinging = qfalse;
        }
        *angle = AngleMod( *angle + move );
    }

    // clamp to no more than tolerance
    swing = AngleSubtract( destination, *angle );
    if ( swing > clampTolerance )
    {
        *angle = AngleMod( destination - (clampTolerance - 1) );
    }
    else if ( swing < -clampTolerance )
    {
        *angle = AngleMod( destination + (clampTolerance - 1) );
    }
}

/*
=================
CG_AddPainTwitch
=================
*/
#define PAIN_TWITCH_TIME    200
static void BG_AddPainTwitch( int painTime, int painDirection, int currentTime,  vec3_t torsoAngles ) {
    int     t;
    float   f;

    t = currentTime - painTime;
    if ( t >= PAIN_TWITCH_TIME ) {
        return;
    }

    f = 1.0 - (float)t / PAIN_TWITCH_TIME;

    if ( painDirection ) {
        torsoAngles[ROLL] += 20 * f;
    } else {
        torsoAngles[ROLL] -= 20 * f;
    }
}

/*
=================
BG_CalculateLeanOffset
=================
*/
float BG_CalculateLeanOffset ( int leanTime )
{
    return ((float)(leanTime - LEAN_TIME) / LEAN_TIME * LEAN_OFFSET);
}

/*
=================
BG_PlayerAngles
=================
*/
void BG_PlayerAngles (

    vec3_t      startAngles,
    vec3_t      legs[3],

    vec3_t      legsAngles,             // out
    vec3_t      lowerTorsoAngles,
    vec3_t      upperTorsoAngles,
    vec3_t      headAngles,

    int         leanOffset,

    int         painTime,
    int         painDirection,
    int         currentTime,

    animInfo_t* torsoInfo,
    animInfo_t* legsInfo,

    int         frameTime,
    vec3_t      realvelocity,
    qboolean    dead,
    float       movementDir,
    void*       ghoul2
    )
{
    float       dest;
    static  int movementOffsets[8] = { 0, 22, 45, -22, 0, 22, -45, -22 };
    float       speed;
    int         dir;
    vec3_t      velocity;

    VectorCopy( startAngles, headAngles );
    VectorCopy ( realvelocity, velocity );
    headAngles[YAW] = AngleMod( headAngles[YAW] );
    VectorClear( legsAngles );
    VectorClear( lowerTorsoAngles );
    VectorClear( upperTorsoAngles );

    // --------- yaw -------------

    // allow yaw to drift a bit
    if ( ( legsInfo->anim & ~ANIM_TOGGLEBIT ) != TORSO_IDLE_PISTOL  )
    {
        // if not standing still, always point all in the same direction
        torsoInfo->yawing   = qtrue;
        torsoInfo->pitching = qtrue;
        legsInfo->yawing    = qtrue;
    }

    speed = VectorNormalize( velocity );

    // adjust legs for movement dir
    if (dead)
    {
        dir = 0;
    }
    else
    {
        dir = movementDir;

        if ( leanOffset && !speed )
        {
            dir = 0;
        }
    }

//  legsAngles[YAW]   = headAngles[YAW] + 2 * movementOffsets[ dir ];
//  torsoAngles[YAW]  = headAngles[YAW] + 2 * movementOffsets[ dir ];
    legsAngles[YAW]   = headAngles[YAW] + 2 * movementOffsets[ dir ];
    lowerTorsoAngles[YAW]  = headAngles[YAW] + 2 * movementOffsets[ dir ];

    // torso
    BG_SwingAngles( lowerTorsoAngles[YAW], 25, 90, 0.3f, &torsoInfo->yawAngle, &torsoInfo->yawing, frameTime );
    BG_SwingAngles( legsAngles[YAW],  40, 90, 0.3f, &legsInfo->yawAngle,  &legsInfo->yawing,  frameTime );

    if ( leanOffset )
    {
        legsAngles[YAW] = headAngles[YAW];
    }
    else
    {
        legsAngles[YAW] = legsInfo->yawAngle;
    }

    lowerTorsoAngles[YAW] = Com_Clampf ( -90, 90, AngleDelta (headAngles[YAW], legsAngles[YAW] ) );
    upperTorsoAngles[YAW] = lowerTorsoAngles[YAW] / 2;
    lowerTorsoAngles[YAW]  = legsAngles[YAW] + lowerTorsoAngles[YAW] / 2;

    headAngles[YAW] -= upperTorsoAngles[YAW];


    // --------- pitch -------------

    // only show a fraction of the pitch angle in the torso
    if ( headAngles[PITCH] > 180 ) {
        dest = (-360 + headAngles[PITCH]) * 0.75;
    } else {
        dest = headAngles[PITCH] * 0.75;
    }

    lowerTorsoAngles[PITCH] = dest;

    // --------- roll -------------

    // lean towards the direction of travel

    // Add in leanoffset
    if ( leanOffset )
    {
        lowerTorsoAngles[ROLL] -= ((float)leanOffset * 1.25f);
        lowerTorsoAngles[YAW] -= 1.25f * ((float)leanOffset/LEAN_OFFSET) * dest;
        headAngles[YAW] -= ((float)leanOffset/LEAN_OFFSET) * dest;
        headAngles[ROLL] -= ((float)leanOffset * 1.25f);
    }
    else if ( speed )
    {
        vec3_t  axis[3];
        float   side;

        speed *= 0.025;

        AnglesToAxis( legsAngles, axis );
        side = speed * DotProduct( velocity, axis[1] );
        legsAngles[ROLL] -= side;
    }

    // pain twitch
    BG_AddPainTwitch( painTime, painDirection, currentTime, lowerTorsoAngles );

    // pull the angles back out of the hierarchial chain
    AnglesSubtract( headAngles, lowerTorsoAngles, headAngles );
    AnglesSubtract( lowerTorsoAngles, legsAngles, lowerTorsoAngles );

    if ( legs && ghoul2 )
    {
        AnglesToAxis( legsAngles, legs );

        // Apply the rotations
        trap_G2API_SetBoneAngles(ghoul2, 0, "upper_lumbar", upperTorsoAngles, BONE_ANGLES_POSTMULT, POSITIVE_X, NEGATIVE_Y, NEGATIVE_Z, 0, 0, currentTime);

        trap_G2API_SetBoneAngles(ghoul2, 0, "lower_lumbar", lowerTorsoAngles, BONE_ANGLES_POSTMULT, POSITIVE_X, NEGATIVE_Y, NEGATIVE_Z, 0, 0, currentTime);
        trap_G2API_SetBoneAngles(ghoul2, 0, "cranium", headAngles, BONE_ANGLES_POSTMULT, POSITIVE_Z, NEGATIVE_Y, POSITIVE_X, 0,0, currentTime);
    }
}

/*
======================
BG_ParseAnimationFile

Read a configuration file containing animation counts and rates
models/players/visor/animation.cfg, etc
======================
*/
qboolean BG_ParseAnimationFile ( const char *filename, animation_t* animations )
{
    const char      *text_p;
    int             len;
    int             i;
    char            *token;
    float           fps;
    int             skip;
    char            text[20000];
    fileHandle_t    f;
    int             animNum;

    // load the file
    len = trap_FS_FOpenFile( filename, &f, FS_READ );

    ///RxCxW - 09.10.06 - 12:21am #fileHandles
    if(!f)
        return qfalse;
    ///End  - 09.10.06 - 12:21am

    if ( len <= 0 || len >= sizeof( text ) - 1 )
    {
        ///RxCxW - 09.10.06 - 12:21am #fileHandles
        trap_FS_FCloseFile( f );
        ///End  - 09.10.06 - 12:21am
        return qfalse;
    }

    trap_FS_Read( text, len, f );
    trap_FS_FCloseFile( f );

    // parse the text
    text[len] = 0;
    text_p    = text;
    skip      = 0;

    //initialize anim array so that from 0 to MAX_ANIMATIONS, set default values of 0 1 0 100
    for(i = 0; i < MAX_ANIMATIONS; i++)
    {
        animations[i].firstFrame = 0;
        animations[i].numFrames = 0;
        animations[i].loopFrames = -1;
        animations[i].frameLerp = 100;
        animations[i].initialLerp = 100;
    }

    // read information for each frame
    while(1)
    {
        token = COM_Parse( &text_p );

        if ( !token || !token[0])
        {
            break;
        }

        animNum = GetIDForString(bg_animTable, token);
        if(animNum == -1)
        {
//#ifndef FINAL_BUILD
#ifdef _DEBUG
            Com_Printf(S_COLOR_RED"WARNING: Unknown token %s in %s\n", token, filename);
#endif
            continue;
        }

        token = COM_Parse( &text_p );
        if ( !token )
        {
            break;
        }
        animations[animNum].firstFrame = atoi( token );

        token = COM_Parse( &text_p );
        if ( !token )
        {
            break;
        }
        animations[animNum].numFrames = atoi( token );

        token = COM_Parse( &text_p );
        if ( !token )
        {
            break;
        }
        animations[animNum].loopFrames = atoi( token );

        token = COM_Parse( &text_p );
        if ( !token )
        {
            break;
        }
        fps = atof( token );
        if ( fps == 0 )
        {
            fps = 1;//Don't allow divide by zero error
        }
        if ( fps < 0 )
        {//backwards
            animations[animNum].frameLerp = floor(1000.0f / fps);
        }
        else
        {
            animations[animNum].frameLerp = ceil(1000.0f / fps);
        }

        animations[animNum].initialLerp = ceil(1000.0f / fabs(fps));
    }

    return qtrue;
}

/*
========================
BG_SetAvailableOutfitting

Set the current availability table
========================
*/
void BG_SetAvailableOutfitting ( const char* available )
{
    int     len;

    len = strlen ( available );
    if ( len > level.wpNumWeapons )
    {
        len = level.wpNumWeapons;
    }

    // IF the availability has changed force a reload of the outfitting groups
    if ( Q_strncmp ( available, bg_availableOutfitting, minimum(sizeof(bg_availableOutfitting),len) ) )
    {
        bg_outfittingCount = 0;
    }

    // Initialize it to all on.
    memset ( &bg_availableOutfitting[0], '2', sizeof(bg_availableOutfitting) );
    memcpy ( &bg_availableOutfitting[0], available, len );
}

/*
========================
BG_IsWeaponAvailableForOutfitting

Is the given weapon available for outfitting?
========================
*/
qboolean BG_IsWeaponAvailableForOutfitting ( weapon_t weapon, int level )
{
    if ( bg_availableOutfitting[0] == -1 )
    {
        return qtrue;
    }
    if ( bg_availableOutfitting[weapon-1] - '0' >= level )
    {
        return qtrue;
    }

    return qfalse;
}

/*
========================
BG_DecompressOutfitting

Decompresses the given outfitting string into the outfitting structure
========================
*/
void BG_DecompressOutfitting ( const char* compressed, goutfitting_t* outfitting)
{
    int group;
    int origitem;

    memset ( outfitting->items, 0, sizeof(outfitting->items) );

    for ( group = 0; group < OUTFITTING_GROUP_MAX; group ++ )
    {
        int item;

        if ( !*compressed )
        {
            item = -1;
        }
        else
        {
            item = ((*compressed++) - 'A');
        }

        // Valid item number?
        if ( item < 0 || item >= 10 )
        {
            item = 0;
        }

        // Valid slot for the group ?
        if ( bg_outfittingGroups[group][item] == -1 )
        {
            item = 0;
            continue;
        }

        // No initialized
        if ( bg_availableOutfitting[0] == -1 )
        {
            item = 0;
            continue;
        }
        // Is it available?
        if ( bg_itemlist[bg_outfittingGroups[group][item]].giType == IT_WEAPON )
        {
            origitem = item;
            while ( !BG_IsWeaponAvailableForOutfitting ( (weapon_t)bg_itemlist[bg_outfittingGroups[group][item]].giTag, 2 ) ) // Fix for nades with /outfitting AAFFA
            {
                item++;
                if ( bg_outfittingGroups[group][item] == -1 )
                {
                    item = 0;
                }

                if ( item == origitem )
                {
                    //Com_Error ( ERR_FATAL, "ERROR: There must be at least one weapon available in each category" );
                    item = -1;
                    break;
                }
            }
        }

        // Ok to set the item now
        outfitting->items[group] = item;
    }
}

/*
========================
BG_CompressOutfitting

Compresses the given outfitting structure into the given string
========================
*/
void BG_CompressOutfitting ( goutfitting_t* outfitting, char* compressed, int size )
{
    int i;

    for ( i = 0; i < OUTFITTING_GROUP_MAX && size; i ++, size-- )
    {
        *compressed++ = outfitting->items[i] + 'A';
    }

    *compressed = '\0';
}

/*
========================
BG_FindOutfitting

Finds the real outfitting that matches the given outfitting
========================
*/
int BG_FindOutfitting ( goutfitting_t* outfitting )
{
    int i;
    int l;

    // Loop through all the outfittings linearly
    for ( i = 0; i < bg_outfittingCount; i ++ )
    {
        for ( l = 0; l < OUTFITTING_GROUP_MAX; l ++ )
        {
            if ( bg_outfittings[i].items[l] != outfitting->items[l] )
            {
                break;
            }
        }

        // Both iterators at the end signifies a match
        if ( l == OUTFITTING_GROUP_MAX )
        {
            return i;
        }
    }

    return -1;
}

/*
========================
BG_ParseOutfittingTemplate

Parses a single outfitting template
========================
*/
qboolean BG_ParseOutfittingTemplate ( const char* fileName, goutfitting_t* outfitting )
{
    TGPGroup            baseGroup;
    TGPGroup            subGroup;
    TGenericParser2     file;
    TGPGroup            pairs;
    char                temp[MAX_OUTFITTING_NAME];

    // Initialize the outfitting
    memset ( outfitting, 0, sizeof(goutfitting_t) );

    // Create the generic parser so the item file can be parsed
    file = trap_GP_ParseFile( (char*)fileName, qtrue, qfalse );
    if ( !file )
    {
        return qfalse;
    }

    // Start at the top with the "outfitting" group
    baseGroup = trap_GP_GetBaseParseGroup ( file );
    subGroup  = trap_GPG_FindSubGroup ( baseGroup, "outfitting" );
    if ( !subGroup )
    {
        trap_GP_Delete(&file);
        return qfalse;
    }

    // Get the name of the template
    trap_GPG_FindPairValue ( subGroup, "displayName", fileName, outfitting->name );

    // Sub group named "items"
    pairs = trap_GPG_FindPair ( subGroup, "items" );
    if ( pairs )
    {
        TGPValue list;

        // Run through the list
        list = trap_GPV_GetList ( pairs );
        while ( list )
        {
            gitem_t*    item;

            trap_GPV_GetName ( list, temp );

            item = BG_FindItem ( temp );
            if ( item )
            {
                int index;
                for ( index=0; bg_outfittingGroups[item->outfittingGroup][index] != -1; index ++ )
                {
                    if ( bg_outfittingGroups[item->outfittingGroup][index] == (item - &bg_itemlist[0]) )
                    {
                        outfitting->items[item->outfittingGroup] = index;
                        break;
                    }
                }
            }

            list = trap_GPV_GetNext ( list );
        }
    }

    // Sub group named "weapons"
    pairs = trap_GPG_FindPair ( subGroup, "weapons" );

    // Run through the weapons
    if ( pairs )
    {
        TGPValue list;

        // Run through the list
        list = trap_GPV_GetList ( pairs );
        while ( list )
        {
            gitem_t*    item;
            int         i;

            trap_GPV_GetName ( list, temp );

            // Lookup the weapon number
            for( i = WP_NONE + 1, item=NULL; i < level.wpNumWeapons; i++ )
            {
                if ( Q_stricmp(bg_weaponNames[i], temp ) == 0)
                {
                    // translate the weapon index into an item index.
                    item = BG_FindWeaponItem ( (weapon_t) i );
                    break;
                }
            }

            // If the weapon translated into an item ok then drop it
            // in its appropriate slot.
            if ( item )
            {
                int index;

                // Make sure outfitting groups that have weapons that are not available
                // do not show up
                if ( !BG_IsWeaponAvailableForOutfitting ( (weapon_t)item->giTag, 2 ) )
                {
                    trap_GP_Delete(&file);
                    return qfalse;
                }

                for ( index=0; bg_outfittingGroups[item->outfittingGroup][index] != -1; index ++ )
                {
                    if ( bg_outfittingGroups[item->outfittingGroup][index] == (item - &bg_itemlist[0]) )
                    {
                        outfitting->items[item->outfittingGroup] = index;
                        break;
                    }
                }
            }

            list = trap_GPV_GetNext ( list );
        }
    }

    trap_GP_Delete(&file);

    return qtrue;
}

/*
========================
BG_ParseOutfittingTemplates

Parses the available outfitting templates
========================
*/
int BG_ParseOutfittingTemplates ( qboolean force )
{
    int     i;
    int     numOutfittingFiles;
    int     filelen;
    char    fileName[MAX_QPATH];
    char    outfittingFiles[4096];
    char    *fileptr;

    // Dont reload unless forced
    if ( bg_outfittingCount && !force )
    {
        return bg_outfittingCount;
    }

    // Clear the current list
    bg_outfittingCount = 1;
    strcpy ( bg_outfittings[0].name, "CUSTOM" );

    // Grab the list of NPC files
    numOutfittingFiles = trap_FS_GetFileList("scripts", ".outfitting", outfittingFiles, 4096 );
    if ( !numOutfittingFiles )
    {
        return 0;
    }

    // Parse each of the NPC files
    fileptr = outfittingFiles;
    for( i = 0; i < numOutfittingFiles; i++, fileptr += filelen+1 )
    {
        // Grab the length so we can skip this file later
        filelen = strlen(fileptr);

        Com_sprintf ( fileName, sizeof(fileName), "scripts/%s", fileptr );

        // Parse the outfitting template
        if ( BG_ParseOutfittingTemplate ( fileName, &bg_outfittings[bg_outfittingCount] ) )
        {
            bg_outfittingCount++;
        }
    }

    return bg_outfittingCount;
}

#ifdef _GOLD

/*
========================
BG_ApplyLeanOffset

Applies the given lean offset to the origin
========================
*/

void BG_ApplyLeanOffset(playerState_t* ps, vec3_t origin)
{
    float   leanOffset;
    vec3_t  up;
    vec3_t  right;

    leanOffset = (float)(ps->leanTime - LEAN_TIME) / LEAN_TIME * LEAN_OFFSET;
    AngleVectors(ps->viewangles, NULL, right, up);
    VectorMA(origin, leanOffset, right, origin);
    VectorMA(origin, Q_fabs(leanOffset) * -0.20f, up, origin);
}

#endif // _GOLD
