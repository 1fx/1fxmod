// Copyright (C) 2001-2002 Raven Software
//
// cg_gametype.c -- dynamic gametype handling

#include "cg_local.h"

/*
===============
CG_ParseGametypeItems
===============
*/
qboolean CG_ParseGametypeItems ( TGPGroup itemsGroup )
{
	TGPGroup  itemGroup;
	int		  itemCount;
	char	  temp[MAX_QPATH];

	// Handle NULL for convienience
	if ( !itemsGroup )
	{
		return qfalse;
	}

	// Loop over all the items and add each 
	itemGroup = trap_GPG_GetSubGroups ( itemsGroup );
	itemCount = 0;

	while ( itemGroup )
	{
		// Parse icon file
		trap_GPG_FindPairValue ( itemGroup, "icon", "", temp );
		bg_itemlist[ MODELINDEX_GAMETYPE_ITEM + itemCount ].icon = (char *)trap_VM_LocalStringAlloc ( temp );

		// Parse display name file
		trap_GPG_FindPairValue ( itemGroup, "displayname", "", temp );
		bg_itemlist[ MODELINDEX_GAMETYPE_ITEM + itemCount ].pickup_name = (char *)trap_VM_LocalStringAlloc ( temp );

		// Parse world model file
		trap_GPG_FindPairValue ( itemGroup, "model", "", temp );
		bg_itemlist[ MODELINDEX_GAMETYPE_ITEM + itemCount ].world_model[0] = (char *)trap_VM_LocalStringAlloc ( temp );

		// Parse bolt model file
		trap_GPG_FindPairValue ( itemGroup, "boltmodel", "", temp );
		trap_G2API_InitGhoul2Model(&cg_items[MODELINDEX_GAMETYPE_ITEM+itemCount].boltModel, temp, 0 , 0, 0, 0, 0);
		cg_items[MODELINDEX_GAMETYPE_ITEM+itemCount].radius[0] = 60;

		CG_RegisterItemVisuals ( MODELINDEX_GAMETYPE_ITEM+itemCount );

		itemCount++;

		// Next sub group
		itemGroup = trap_GPG_GetNext(itemGroup);
	}

	return qtrue;
}

/*
===============
CG_ParseGametypeFile
===============
*/
qboolean CG_ParseGametypeFile ( void )
{
	TGenericParser2 GP2;
	TGPGroup		topGroup;
	TGPGroup		gametypeGroup;

	// Open the given gametype file
	GP2 = trap_GP_ParseFile ( (char*)cgs.gametypeData->script, qtrue, qfalse );
	if (!GP2)
	{
		return qfalse;
	}

	// Grab the top group and the list of sub groups
	topGroup = trap_GP_GetBaseParseGroup(GP2);
	gametypeGroup = trap_GPG_FindSubGroup(topGroup, "gametype" );
	if ( !gametypeGroup )
	{
		trap_GP_Delete(&GP2);
		return qfalse;
	}
	
	// Mainly interested in the items within the file
	CG_ParseGametypeItems ( trap_GPG_FindSubGroup ( gametypeGroup, "items" ) );

	// Free up the parser
	trap_GP_Delete(&GP2);

	// Defaults
	trap_Cvar_Set ( "ui_blueteamname", "Blue Team" );
	trap_Cvar_Set ( "ui_redteamname", "Red Team" );

	return qtrue;
}

