// Copyright (C) 2001-2002 Raven Software
//
// bg_gametype.c -- dynamic gametype handling

#include "q_shared.h"
#include "bg_public.h"
#include "bg_local.h"

gametypeData_t	bg_gametypeData[MAX_GAMETYPES];
int				bg_gametypeCount;

/*
===============
BG_ParseGametypePhotos

Parses the photo information for objectives dialog
===============
*/
static qboolean BG_ParseGametypePhotos ( int gametypeIndex, TGPGroup group )
{
	TGPGroup photo;
	int		 index;
	char	 temp[MAX_TOKENLENGTH];

	// Convienience check
	if ( !group )
	{
		return qtrue;
	}

	index = 0;
	photo = trap_GPG_GetSubGroups ( group );

	while ( photo )
	{
		trap_GPG_GetName ( photo, temp );
#ifdef _TRUEMALLOC
		trap_TrueMalloc((void **)&bg_gametypeData[gametypeIndex].photos[index].name, sizeof(temp));
		if(bg_gametypeData[gametypeIndex].photos[index].name){
			strcpy((char *)bg_gametypeData[gametypeIndex].photos[index].name, temp);
		}
#else
		bg_gametypeData[gametypeIndex].photos[index].name = trap_VM_LocalStringAlloc ( temp );
#endif

		trap_GPG_FindPairValue ( photo, "displayname", "unknown", temp );
#ifdef _TRUEMALLOC
		trap_TrueMalloc((void **)&bg_gametypeData[gametypeIndex].photos[index].displayName, sizeof(temp));
		if(bg_gametypeData[gametypeIndex].photos[index].displayName){
			strcpy((char *)bg_gametypeData[gametypeIndex].photos[index].displayName, temp);
		}
#else
		bg_gametypeData[gametypeIndex].photos[index].displayName = trap_VM_LocalStringAlloc ( temp );
#endif

		index++;

		photo = trap_GPG_GetNext ( photo );
	}

	return qtrue;
}

/*
===============
BG_ParseGametypeInfo

Parses minimal information about the given gametype.  For the most part
this information is the name of the gametype and some of its parameters.
===============
*/
static qboolean BG_ParseGametypeInfo ( int gametypeIndex )
{
	gametypeData_t*		gametype;
	TGenericParser2		GP2;
	TGPGroup			topGroup;
	TGPGroup			gtGroup;
	char				temp[1024];

	// Get the pointer for the gametype data
	gametype = &bg_gametypeData[gametypeIndex];
	
	// Open the gametype's script file
	GP2 = trap_GP_ParseFile( (char*)gametype->script, qtrue, qfalse);
	if (!GP2)
	{
		return qfalse;
	}

	// Top group should only contain the "gametype" sub group
	topGroup = trap_GP_GetBaseParseGroup(GP2);
	if ( !topGroup )
	{
		return qfalse;
	}

	// Grab the gametype sub group
	gtGroup = trap_GPG_FindSubGroup ( topGroup, "gametype" );
	if ( !gtGroup )
	{
		return qfalse;
	}

	// Parse out the name of the gametype
	trap_GPG_FindPairValue ( gtGroup, "displayname", "", temp );
	if ( !temp[0] )
	{
		return qfalse;
	}
#ifdef _TRUEMALLOC
	trap_TrueMalloc((void **)&gametype->displayName, sizeof(temp));
	if(gametype->displayName){
		strcpy((char *)gametype->displayName, temp);
	}
#else
	gametype->displayName = trap_VM_LocalStringAlloc ( temp );
#endif

	// Description
	trap_GPG_FindPairValue ( gtGroup, "description", "", temp );
	if ( temp[0] )
	{
#ifdef _TRUEMALLOC
	trap_TrueMalloc((void **)&gametype->description, sizeof(temp));
	if(gametype->description){
		strcpy((char *)gametype->description, temp);
	}
#else
		gametype->description = trap_VM_LocalStringAlloc ( temp );
#endif
	}

	// Are pickups enabled?
	trap_GPG_FindPairValue ( gtGroup, "pickups", "yes", temp );
	if ( !Q_stricmp ( temp, "no" ) )
	{
		gametype->pickupsDisabled = qtrue;
	}

	// Are teams enabled?
	trap_GPG_FindPairValue ( gtGroup, "teams", "yes", temp );
	if ( !Q_stricmp ( temp, "yes" ) )
	{
		gametype->teams = qtrue;
	}

	// Display kills
	trap_GPG_FindPairValue ( gtGroup, "showkills", "no", temp );
	if ( !Q_stricmp ( temp, "yes" ) )
	{
		gametype->showKills = qtrue;
	}

	// Look for the respawn type
	trap_GPG_FindPairValue ( gtGroup, "respawn", "normal", temp );
	if ( !Q_stricmp ( temp, "none" ) )
	{
		gametype->respawnType = RT_NONE;
	}
	else if ( !Q_stricmp ( temp, "interval" ) )
	{
		gametype->respawnType = RT_INTERVAL;
	}
	else
	{
		gametype->respawnType = RT_NORMAL;
	}

	// Ryan Dec 5 2004
	// Stolen from gold edition code
	// A gametype can be based off another gametype which means it uses all the gametypes entities
	trap_GPG_FindPairValue ( gtGroup, "basegametype", "", temp );
	if ( temp[0] )
	{
#ifdef _TRUEMALLOC
		trap_TrueMalloc((void **)&gametype->basegametype, sizeof(temp));
		if(gametype->basegametype){
			strcpy((char *)gametype->basegametype, temp);
		}
#else
		gametype->basegametype = trap_VM_LocalStringAlloc ( temp );
#endif
	}
	// Ryan

	// What percentage doest he backpack replenish?
	trap_GPG_FindPairValue ( gtGroup, "backpack", "0", temp );
	gametype->backpack = atoi(temp);

	// Get the photo information for objectives dialog
	BG_ParseGametypePhotos ( gametypeIndex, trap_GPG_FindSubGroup ( gtGroup, "photos" ) );

	// Cleanup the generic parser
	trap_GP_Delete ( &GP2 );

	return qtrue;

}

/*
===============
BG_BuildGametypeList

Builds a list of the gametypes that are available and parses minimal
information about those gametypes.
===============
*/
qboolean BG_BuildGametypeList ( void )
{
	char		filename[MAX_QPATH];
	char		filelist[4096];
	char*		fileptr;
	char*		s;
	int			filelen;
	int			filecount;
	int			i;

	bg_gametypeCount = 0;

	// Retrieve the list of gametype files.  The returned list is a 
	// null separated list with the number of entries returned by the call
	filecount = trap_FS_GetFileList("scripts", ".gametype", filelist, 4096 );
	fileptr   = filelist;
	
	for ( i = 0; i < filecount; i++, fileptr += filelen+1) 
	{
		// Grab the length so we can skip this file later
		filelen = strlen(fileptr);

		// Build the full filename
		strcpy(filename, "scripts/");
		strcat(filename, fileptr );

		// Fill in what we know so far
#ifdef _TRUEMALLOC
		trap_TrueMalloc((void **)&bg_gametypeData[bg_gametypeCount].script, sizeof(filename));
		if(bg_gametypeData[bg_gametypeCount].script){
			strcpy((char *)bg_gametypeData[bg_gametypeCount].script, filename);
		}
#else
		bg_gametypeData[bg_gametypeCount].script = trap_VM_LocalStringAlloc ( filename );
#endif

		// Kill the dot so we can use the filename as the short name
		s  = strchr ( fileptr, '.' );
		*s = '\0';
#ifdef _TRUEMALLOC
		trap_TrueMalloc((void **)&bg_gametypeData[bg_gametypeCount].name, sizeof(fileptr));
		if(bg_gametypeData[bg_gametypeCount].name){
			strcpy((char *)bg_gametypeData[bg_gametypeCount].name, fileptr);
		}
#else
		bg_gametypeData[bg_gametypeCount].name   = trap_VM_LocalStringAlloc ( fileptr );
#endif
		
		// TODO: Parse the gametype file
		BG_ParseGametypeInfo ( bg_gametypeCount++ );
	}

	return qtrue;
}

/*
===============
BG_FindGametype

Returns the gametype index using the given name. If the gametype isnt found
then -1 will be returned (and this is bad)
===============
*/
int BG_FindGametype ( const char* name )
{
	int i;

	// Loop through the known gametypes and compare their names to 
	// the name given
	for ( i = 0; i < bg_gametypeCount; i ++ )
	{
		// Do the names match?
		if ( !Q_stricmp ( bg_gametypeData[i].name, name )  )
		{
			return i;
		}
	}

	return -1;
}

/*
==============
BG_FindGametypeItem

Search through the item list for the gametype item with
the given index.
==============
*/
gitem_t	*BG_FindGametypeItem ( int index ) 
{
	return &bg_itemlist[index + MODELINDEX_GAMETYPE_ITEM];
}

/*
==============
BG_FindGametypeItemByID

Gametype will assign ids to the gametype items for them for future reference, the 
id is crammed into the quantity field of the gametype item.  This function will
find the gametype item with the given item id.
==============
*/
gitem_t *BG_FindGametypeItemByID ( int itemid )
{
	int i;
	
	for ( i = 0; i < MAX_GAMETYPE_ITEMS; i ++ )
	{
		if ( bg_itemlist[i + MODELINDEX_GAMETYPE_ITEM].quantity == itemid )
		{
			return &bg_itemlist[i + MODELINDEX_GAMETYPE_ITEM];
		}
	}

	return NULL;
}
