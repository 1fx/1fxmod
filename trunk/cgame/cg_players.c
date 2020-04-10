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
// cg_players.c -- handle the media and animation for player entities

#include "cg_local.h"
#include "..\ghoul2\g2.h"
#include "..\game\inv.h"

typedef struct SCustomSound
{
	const char	*mGroup;
	const int	mIndex;
} TCustomSound;

TCustomSound CustomSounds[MAX_CUSTOM_SOUNDS] = 
{
	{ "Death", 0 },
	{ "Death", 1 },
	{ "Death", 2 },
	{ "Pain", 0 },
	{ "Pain", 1 },
	{ "Pain", 2 },
};

/*
================
CG_CustomSound
================
*/
sfxHandle_t	CG_CustomSound(int clientNum, const char *soundName) 
{
	return trap_S_RegisterSound( soundName );
}

/*
================
CG_CustomPlayerSound
================
*/
sfxHandle_t	CG_CustomPlayerSound(int clientNum, ECustomSounds sound)
{
	clientInfo_t	*ci;

	if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) 
	{
		clientNum = 0;
	}
	ci = &cgs.clientinfo[ clientNum ];

	return ci->sounds[sound];
}


/*
==========================
CG_RegisterClientIdentity
==========================
*/
static qboolean CG_RegisterClientIdentity ( clientInfo_t *ci, const char *identityName ) 
{
	char		afilename[MAX_QPATH];
	TIdentity*	identity;

	// Find the identity in question
	identity = BG_FindIdentity( identityName );
	if (!identity )
	{
		return qfalse;
	}

	// Register the identity
	ci->ghoul2Model = CG_RegisterIdentity( identity, afilename, &ci->gender );
	if (!ci->ghoul2Model)
	{
		return qfalse;
	}

	// find the bolt point for hanging world models of weapons on
	ci->boltWorldWeapon = trap_G2API_AddBolt(ci->ghoul2Model, 0, "rhang_tag_bone");	

	if ( !BG_ParseAnimationFile( afilename, ci->animations ) ) 
	{
		Com_Printf( "Failed to load animation file %s\n", afilename );
		trap_G2API_RemoveGhoul2Model(&ci->ghoul2Model, 0);
		return qfalse;
	}

	// Flag model as male or female.
	assert(afilename);
	ci->isMale=(strstr(afilename,"female"))?qfalse:qtrue;
	ci->identity = identity;

	return qtrue;
}

/*
====================
CG_ColorFromString
====================
*/
static void CG_ColorFromString( const char *v, vec3_t color ) {
	int val;

	VectorClear( color );

	val = atoi( v );

	if ( val < 1 || val > 7 ) {
		VectorSet( color, 1, 1, 1 );
		return;
	}

	if ( val & 1 ) {
		color[2] = 1.0f;
	}
	if ( val & 2 ) {
		color[1] = 1.0f;
	}
	if ( val & 4 ) {
		color[0] = 1.0f;
	}
}

/*
===================
CG_LoadClientInfo

Load it now, taking the disk hits.
This will usually be deferred to a safe time
===================
*/
static void CG_LoadClientInfo( clientInfo_t *ci ) 
{
	int		i;
	int		modelloaded;
	int		clientNum;

	modelloaded = qtrue;

	// Initialize all bolt positions
	ci->boltWorldWeapon = -1;
	ci->boltNightvision = -1;
	for ( i = 0; i < MAX_GAMETYPE_ITEMS; i ++ )
	{
		ci->boltGametypeItems[i] = -1;
	}

	// Register the client identity
	if ( !CG_RegisterClientIdentity ( ci, ci->identityName ) )
	{
		if ( !CG_RegisterClientIdentity ( ci, DEFAULT_IDENTITY ) )
		{
			Com_Error( ERR_FATAL, "DEFAULT_IDENTITY (%s) failed to register", DEFAULT_IDENTITY );
		}
	}

	// sounds
	for( i=0; i< MAX_CUSTOM_SOUNDS; i++ )
	{
		const char* sound;

		ci->sounds[i] = 0;
		sound = NULL;

		sound = BG_GetModelSound ( ci->identityName, CustomSounds[i].mGroup, CustomSounds[i].mIndex );

		// If there is a valid sound to load then load it
		if ( sound && sound[0] )
		{
			ci->sounds[i] = trap_S_RegisterSound( sound );
		}
	}

	ci->deferred = qfalse;

	// reset any existing players and bodies, because they might be in bad
	// frames for this new model
	clientNum = ci - cgs.clientinfo;
	for ( i = 0 ; i < MAX_GENTITIES ; i++ )
	{
		centity_t* cent;

		cent = CG_GetEntity ( i );

		if ( cent->currentState.clientNum == clientNum && 
			 cent->currentState.eType == ET_PLAYER ) 
		{
			CG_ResetPlayerEntity( cent );
		}
	}
}

/*
======================
CG_CopyClientInfoModel
======================
*/
static void CG_CopyClientInfoModel( clientInfo_t *from, clientInfo_t *to ) 
{
	int i;

	to->gender = from->gender;

	to->boltWorldWeapon = from->boltWorldWeapon;

	for ( i = 0; i < MAX_GAMETYPE_ITEMS; i ++ )
	{
		to->boltGametypeItems[i] = from->boltGametypeItems[i];
	}

	to->boltNightvision = from->boltNightvision;

	// Copy all the ghoul2 information too
	if (from->ghoul2Model)
	{
		trap_G2API_DuplicateGhoul2Instance( from->ghoul2Model, &to->ghoul2Model );
	}
	else
	{
		assert(0);
		Com_Error(ERR_DROP, "CG_CopyClientInfoModel invalid g2 pointer\n");
	}

	memcpy( to->animations, from->animations, sizeof( to->animations ) );
	memcpy( to->sounds, from->sounds, sizeof( to->sounds ) );
}

/*
======================
CG_ScanForExistingClientInfo
======================
*/
static qboolean CG_ScanForExistingClientInfo( clientInfo_t *ci ) 
{
	int				i;
	clientInfo_t	*match;

	for ( i = 0 ; i < cgs.maxclients ; i++ ) 
	{
		match = &cgs.clientinfo[ i ];
		
		if ( !match->infoValid ) 
		{
			continue;
		}
		if ( match->deferred ) 
		{
			continue;
		}

		// Identity name doesnt match
		if ( Q_stricmp ( ci->identityName, match->identityName ) )
		{
			continue;
		}

		if ( ci->team == TEAM_FREE      ||
		     ci->team == TEAM_SPECTATOR ||
			 ci->team == match->team        )
		{
			// this clientinfo is identical, so use it's handles
			ci->deferred = qfalse;

			CG_CopyClientInfoModel( match, ci );

			return qtrue;
		}
	}

	// nothing matches, so defer the load
	return qfalse;
}

/*
======================
CG_SetDeferredClientInfo

We aren't going to load it now, so grab some other
client's info to use until we have some spare time.
======================
*/
static void CG_SetDeferredClientInfo( clientInfo_t *ci ) 
{
	int				i;
	clientInfo_t	*match;

/*
	// If someone else is already the same models and skins we
	// can just load the client info
	for ( i = 0 ; i < cgs.maxclients ; i++ )
	{
		match = &cgs.clientinfo[ i ];

		// Cant steal the model from invalid or deferred clients
		if ( !match->infoValid || match->deferred ) 
		{
			continue;
		}

		// If the model, skin, and team match then we have a match
		if ( Q_stricmp( ci->identityName, match->identityName) ||
			 (ci->team != match->team)							  ) 
		{
			continue;
		}

		// just load the real info cause it uses the same models and skins
		CG_LoadClientInfo( ci );

		return;
	}
*/

	// Try to use any skin from the team they are joining
	if ( cgs.gametypeData->teams )
	{
		for ( i = 0; i < cgs.maxclients; i ++ )
		{
			match = &cgs.clientinfo[ i ];
			if ( !match->infoValid ) 
			{
				continue;
			}

			// Must be on the same team
			if ( match->team != ci->team )
			{
				continue;
			}

			ci->deferred = qtrue;
			CG_CopyClientInfoModel( match, ci );
			return;
		}
	}
	// find the first valid clientinfo and grab its stuff
	else
	{
		for ( i = 0 ; i < cgs.maxclients ; i++ ) 
		{
			match = &cgs.clientinfo[ i ];
			if ( !match->infoValid ) 
			{
				continue;
			}

			ci->deferred = qtrue;
			CG_CopyClientInfoModel( match, ci );
			return;
		}
	}

	CG_LoadClientInfo( ci );
}

/*
======================
CG_ResetClientIdentity
======================
*/
void CG_ResetClientIdentity ( int clientNum )
{
	clientInfo_t *ci;
	centity_t	 *cent;
	int			 i;

	ci = &cgs.clientinfo[clientNum];
	cent = CG_GetEntity ( clientNum );	

	// Clean up the raw ghoul model
	if ( ci->ghoul2Model )
	{
		trap_G2API_CleanGhoul2Models ( &ci->ghoul2Model );
		ci->ghoul2Model = NULL;
	}

	// Clean up the active ghoul model
	if ( cent->ghoul2 )
	{
		trap_G2API_CleanGhoul2Models ( &cent->ghoul2 );
		cent->pe.weaponModelSpot = 0;
		cent->pe.weapon = 0;
		cent->ghoul2 = NULL;
	}

	// Reset all bolt position
	ci->boltWorldWeapon = -1;
	ci->boltNightvision = -1;
	for ( i = 0; i < MAX_GAMETYPE_ITEMS; i ++ )
	{
		ci->boltGametypeItems[i] = -1;
	}
}

/*
======================
CG_NewClientInfo
======================
*/
void CG_NewClientInfo( int clientNum ) 
{
	clientInfo_t *ci;
	clientInfo_t newInfo;
	centity_t*	 cent;
	const char	*configstring;
	const char	*v;

	ci = &cgs.clientinfo[clientNum];

	cent = CG_GetEntity ( clientNum );	

	configstring = CG_ConfigString( clientNum + CS_PLAYERS );
	if ( !configstring[0] ) 
	{
		memset( ci, 0, sizeof( *ci ) );
		return;
	}

	// Clear the client models / etc
	CG_ResetClientIdentity ( clientNum );

	ci->infoValid = qfalse;

	// build into a temp buffer so the defer checks can use
	// the old value
	memset( &newInfo, 0, sizeof( newInfo ) );

	// isolate the player's name
	v = Info_ValueForKey(configstring, "n");
	Q_strncpyz( newInfo.name, v, sizeof( newInfo.name ) );

	// bot skill
	v = Info_ValueForKey( configstring, "skill" );
	newInfo.botSkill = atoi( v );

	// handicap
	v = Info_ValueForKey( configstring, "hc" );
	newInfo.handicap = atoi( v );

	// wins
	v = Info_ValueForKey( configstring, "w" );
	newInfo.wins = atoi( v );

	// losses
	v = Info_ValueForKey( configstring, "l" );
	newInfo.losses = atoi( v );

	// team
	v = Info_ValueForKey( configstring, "t" );
	newInfo.team = atoi( v );

	// team task
	v = Info_ValueForKey( configstring, "tt" );
	newInfo.teamTask = atoi(v);

	// team leader
	v = Info_ValueForKey( configstring, "tl" );
	newInfo.teamLeader = atoi(v);

	// identity
	v = Info_ValueForKey( configstring, "identity" );
	Q_strncpyz ( newInfo.identityName, v, sizeof(newInfo.identityName) );

	// Inform the ui of the team switch
	if ( clientNum == cg.clientNum )
	{
		char		temp[MAX_QPATH];
		const char* identityCvar;

		trap_Cvar_Set ( "ui_info_team", va("%i",newInfo.team ) );

		if (cgs.gametypeData->teams )
		{
			identityCvar = "team_identity";
		}
		else
		{
			identityCvar = "identity";
		}

		trap_Cvar_VariableStringBuffer ( identityCvar, temp, sizeof(temp) );

		if ( Q_stricmp ( temp, newInfo.identityName ) )
		{
			trap_Cvar_Set ( identityCvar, newInfo.identityName );
		}
	}

	// scan for an existing clientinfo that matches this modelname
	// so we can avoid loading checks if possible
	if ( !CG_ScanForExistingClientInfo( &newInfo ) ) 
	{
		qboolean	forceDefer;

		forceDefer = trap_MemoryRemaining() < 4000000;

		// if we are defering loads, just have it pick the first valid
		if ( forceDefer || (cg_deferPlayers.integer && !cg_buildScript.integer && !cg.loading ) ) 
		{
			// keep whatever they had if it won't violate team skins
			CG_SetDeferredClientInfo( &newInfo );

			// if we are low on memory, leave them with this model
			if ( forceDefer ) 
			{
				Com_Printf( "Memory is low.  Using deferred model.\n" );
				newInfo.deferred = qfalse;
			}
		} 
		else 
		{
			CG_LoadClientInfo( &newInfo );
		}
	}

	// replace whatever was there with the new one
	newInfo.infoValid = qtrue;
	*ci = newInfo;
}

/*
======================
CG_LoadDeferredPlayers

Called each frame when a player is dead
and the scoreboard is up
so deferred players can be loaded
======================
*/
void CG_LoadDeferredPlayers( void ) 
{
	int				i;
	clientInfo_t	*ci;

	// scan for a deferred player to load
	for ( i = 0, ci = cgs.clientinfo ; i < cgs.maxclients ; i++, ci++ ) 
	{
		if ( ci->infoValid && ci->deferred ) 
		{
			// if we are low on memory, leave it deferred
			if ( trap_MemoryRemaining() < 4000000 ) 
			{
				Com_Printf( "Memory is low.  Using deferred model.\n" );
				ci->deferred = qfalse;
				continue;
			}

			// Clear the client models / etc
			CG_ResetClientIdentity ( i );

			CG_LoadClientInfo( ci );
		}
	}
}

/*
=================
CG_AddPainTwitch
=================
*/
static void CG_AddPainTwitch( centity_t *cent, vec3_t torsoAngles ) 
{
	int		t;
	float	f;

	t = cg.time - cent->pe.painTime;
	if ( t >= PAIN_TWITCH_TIME ) {
		return;
	}

	f = 1.0 - (float)t / PAIN_TWITCH_TIME;

	if ( cent->pe.painDirection ) {
		torsoAngles[ROLL] += 20 * f;
	} else {
		torsoAngles[ROLL] -= 20 * f;
	}
}

/*
===============
CG_PlayerFloatSprite

Float a sprite over the player's head
===============
*/
static void CG_PlayerFloatSprite( centity_t *cent, qhandle_t shader ) 
{
	int				rf;
	refEntity_t		ent;

	if ( cent->currentState.number == cg.snap->ps.clientNum && !cg.renderingThirdPerson ) 
	{
		// only show in mirrors
		rf = RF_THIRD_PERSON;		
	} 
	else 
	{
		rf = 0;
	}

	memset( &ent, 0, sizeof( ent ) );
	VectorCopy( cent->lerpOrigin, ent.origin );
	ent.origin[2] += 58;
	ent.reType = RT_SPRITE;
	ent.customShader = shader;
	ent.radius = 10;
	ent.renderfx = rf;
	ent.shaderRGBA[0] = 255;
	ent.shaderRGBA[1] = 255;
	ent.shaderRGBA[2] = 255;
	ent.shaderRGBA[3] = 255;
	trap_R_AddRefEntityToScene( &ent );
}

/*
===============
CG_PlayerSprites

Float sprites over the player's head
===============
*/
static void CG_PlayerSprites( centity_t *cent ) 
{
	int		team;

	team = cgs.clientinfo[ cent->currentState.clientNum ].team;
	if ( !(cent->currentState.number == cg.snap->ps.clientNum) &&
	     !(cent->currentState.eFlags & EF_DEAD)     && 
		  cg.snap->ps.persistant[PERS_TEAM] == team &&
		  cgs.gametypeData->teams ) 
	{
		if (cg_drawFriend.integer) 
		{
			CG_PlayerFloatSprite( cent, team==TEAM_RED?cgs.media.redFriendShader:cgs.media.blueFriendShader );
		}

		return;
	}
}

/*
===============
CG_PlayerShadow

Returns the Z component of the surface being shadowed

  should it return a full plane instead of a Z?
===============
*/
#define	SHADOW_DISTANCE		256
qboolean CG_PlayerShadow( centity_t *cent, float *shadowPlane ) 
{
	vec3_t		end, mins = {-15, -15, 0}, maxs = {15, 15, 2};
	trace_t		trace;
	float		alpha;

	*shadowPlane = 0;

	if ( cg_shadows.integer == 0 ) {
		return qfalse;
	}

	// send a trace down from the player to the ground
	VectorCopy( cent->lerpOrigin, end );
	end[2] -= SHADOW_DISTANCE;

	trap_CM_BoxTrace( &trace, cent->lerpOrigin, end, mins, maxs, 0, MASK_PLAYERSOLID );

	// no shadow if too high
	if ( trace.fraction == 1.0 || trace.startsolid || trace.allsolid ) {
		return qfalse;
	}

	*shadowPlane = trace.endpos[2] + 1;

	if ( cg_shadows.integer != 1 ) {	// no mark for stencil or projection shadows
		return qtrue;
	}

	// fade the shadow out with height
	alpha = 1.0 - trace.fraction;
	alpha = 1.0;
	trap_R_AddDecalToScene ( cgs.media.shadowMarkShader, trace.endpos, trace.plane.normal, 
							 cent->currentState.apos.trBase[YAW], 1.0,0.0,1.0,1.0, qfalse, 24, qtrue );

	return qtrue;
}


/*
===============
CG_PlayerSplash

Draw a mark at the water surface
===============
*/
static void CG_PlayerSplash( centity_t *cent ) {
	vec3_t		start, end;
	trace_t		trace;
	int			contents;
	polyVert_t	verts[4];

	if ( !cg_shadows.integer ) {
		return;
	}

	VectorCopy( cent->lerpOrigin, end );
	end[2] -= 24;

	// if the feet aren't in liquid, don't make a mark
	// this won't handle moving water brushes, but they wouldn't draw right anyway...
	contents = trap_CM_PointContents( end, 0 );
	if ( !( contents & ( CONTENTS_WATER | CONTENTS_LAVA ) ) ) {
		return;
	}

	VectorCopy( cent->lerpOrigin, start );
	start[2] += 32;

	// if the head isn't out of liquid, don't make a mark
	contents = trap_CM_PointContents( start, 0 );
	if ( contents & ( CONTENTS_SOLID | CONTENTS_WATER | CONTENTS_LAVA ) ) {
		return;
	}

	// trace down to find the surface
	trap_CM_BoxTrace( &trace, start, end, NULL, NULL, 0, ( CONTENTS_WATER | CONTENTS_LAVA ) );

	if ( trace.fraction == 1.0 ) {
		return;
	}

	// create a mark polygon
	VectorCopy( trace.endpos, verts[0].xyz );
	verts[0].xyz[0] -= 32;
	verts[0].xyz[1] -= 32;
	verts[0].st[0] = 0;
	verts[0].st[1] = 0;
	verts[0].modulate[0] = 255;
	verts[0].modulate[1] = 255;
	verts[0].modulate[2] = 255;
	verts[0].modulate[3] = 255;

	VectorCopy( trace.endpos, verts[1].xyz );
	verts[1].xyz[0] -= 32;
	verts[1].xyz[1] += 32;
	verts[1].st[0] = 0;
	verts[1].st[1] = 1;
	verts[1].modulate[0] = 255;
	verts[1].modulate[1] = 255;
	verts[1].modulate[2] = 255;
	verts[1].modulate[3] = 255;

	VectorCopy( trace.endpos, verts[2].xyz );
	verts[2].xyz[0] += 32;
	verts[2].xyz[1] += 32;
	verts[2].st[0] = 1;
	verts[2].st[1] = 1;
	verts[2].modulate[0] = 255;
	verts[2].modulate[1] = 255;
	verts[2].modulate[2] = 255;
	verts[2].modulate[3] = 255;

	VectorCopy( trace.endpos, verts[3].xyz );
	verts[3].xyz[0] += 32;
	verts[3].xyz[1] -= 32;
	verts[3].st[0] = 1;
	verts[3].st[1] = 0;
	verts[3].modulate[0] = 255;
	verts[3].modulate[1] = 255;
	verts[3].modulate[2] = 255;
	verts[3].modulate[3] = 255;

	trap_R_AddPolyToScene( cgs.media.wakeMarkShader, 4, verts );
}

/*
===============
CG_UpdatePlayerAnimations
===============
*/
static void CG_UpdatePlayerAnimations (	centity_t* cent	) 
{
	clientInfo_t *ci;

	// Make sure its a player
	assert ( cent->currentState.eType == ET_PLAYER );

	// Retrieve the client info pointer for the given client
	ci = &cgs.clientinfo[ cent->currentState.clientNum ];

	// Update the leg animation
	if ( cent->pe.legs.anim != cent->currentState.legsAnim && cent->currentState.legsAnim != -1 )
	{
		animation_t	*anim;
		int			flags;
		float		speed;

		anim = &ci->animations[ (cent->currentState.legsAnim & (~ANIM_TOGGLEBIT)) ];

		if ( anim->loopFrames == -1 )
		{
			flags = BONE_ANIM_OVERRIDE|BONE_ANIM_OVERRIDE_FREEZE;
		}
		else
		{
			flags = BONE_ANIM_OVERRIDE_LOOP;
		}

		if (cg_animBlend.integer)
		{
			flags |= BONE_ANIM_BLEND;
		}

		speed = 1.0f;

		trap_G2API_SetBoneAnim(cent->ghoul2, 0, "model_root", anim->firstFrame, anim->firstFrame + anim->numFrames, flags, 50.0f / anim->frameLerp * speed, cg.time, -1, 150);

		cent->pe.legs.anim = cent->currentState.legsAnim;
		cent->pe.legs.animTime = cg.time;
	}

	// Update the torso animation
	if ( cent->pe.torso.anim != cent->currentState.torsoAnim && cent->currentState.torsoAnim != -1 )
	{
		animation_t	*anim;
		int			flags;
		float		time;
		anim = &ci->animations[ (cent->currentState.torsoAnim & (~ANIM_TOGGLEBIT)) ];

		if ( anim->loopFrames == -1 )
		{
			flags = BONE_ANIM_OVERRIDE|BONE_ANIM_OVERRIDE_FREEZE;
		}
		else
		{
			flags = BONE_ANIM_OVERRIDE_LOOP;
		}

		if (cg_animBlend.integer)
		{
			flags |= BONE_ANIM_BLEND;
		}

		if ( cent->currentState.torsoTimer > 0 )
		{
			time = cent->currentState.torsoTimer;
			time /= anim->numFrames;
		}
		else
		{
			time = anim->frameLerp;
		}

		// Default to 20 FPS.
		if ( time <= 0 )
		{
			time = 1000.0f / 20.0f;
		}

		trap_G2API_SetBoneAnim(cent->ghoul2, 0, "lower_lumbar", anim->firstFrame, anim->firstFrame + anim->numFrames, flags, 50.0f / time, cg.time, -1, 150);

		cent->pe.torso.anim = cent->currentState.torsoAnim ;
		cent->pe.torso.animTime = cg.time;
	}
}

/*
===============
CG_PlayerGametypeItems
===============
*/
void CG_PlayerGametypeItems ( centity_t* cent, vec3_t angles )
{
	clientInfo_t	*ci = &cgs.clientinfo[cent->currentState.clientNum];
	int				i;
	int				rf;

	if ( cent->currentState.number == cg.snap->ps.clientNum && !cg.renderingThirdPerson ) 
	{
		// only show in mirrors
		rf = RF_THIRD_PERSON;		
	} 
	else 
	{
		rf = 0;
	}

	// If the player is wearing goggles then draw them on their head
	if ( cent->currentState.eFlags & EF_GOGGLES )
	{
		refEntity_t	item;

		if ( ci->boltNightvision == -1 )
		{
			ci->boltNightvision = trap_G2API_AddBolt( cent->ghoul2, 0, "*head_t" );
		}

		memset ( &item, 0, sizeof(item) );
		item.renderfx = rf | RF_MINLIGHT;
		item.ghoul2 = cgs.media.nightVisionModel;
		G2_PositionEntityOnBolt ( &item, cent->ghoul2, 0, ci->boltNightvision, cent->lerpOrigin, angles, cent->modelScale );
		trap_R_AddRefEntityToScene(&item);
	}
		

	// Loop through the gametye items  and handle each separately
	for ( i = 0; i < MAX_GAMETYPE_ITEMS; i ++ )
	{
		// If they dont have the item then skip it 
		if ( !(cent->currentState.gametypeitems & (1<<i) ))
		{
			continue;
		}

		// Need to add bolt to model
		if ( cg_items[ MODELINDEX_GAMETYPE_ITEM + i ].boltModel )
		{
			refEntity_t	item;

			if ( ci->boltGametypeItems[i] == -1 )
			{
				ci->boltGametypeItems[i] = trap_G2API_AddBolt( cent->ghoul2, 0, "*back" );	
				if ( ci->boltGametypeItems[i] == -1 )
				{
					continue;
				}

				// Remove all identity items on the back
				CG_RemoveIdentityItemsOnBack ( cent );
			}

			memset ( &item, 0, sizeof(item) );
			item.renderfx = rf | RF_MINLIGHT;
			item.ghoul2 = cg_items[ MODELINDEX_GAMETYPE_ITEM + i ].boltModel;
			G2_PositionEntityOnBolt ( &item, cent->ghoul2, 0, ci->boltGametypeItems[i], cent->lerpOrigin, angles, cent->modelScale );
			trap_R_AddRefEntityToScene(&item);
		}
	}	
}

/*
===============
CG_UpdatePlayerWeaponModel
===============
*/
void CG_UpdatePlayerWeaponModel ( centity_t* cent, qboolean force )
{
	clientInfo_t	*ci = &cgs.clientinfo[cent->currentState.clientNum];

	// If the weapon model hasnt changed then dont update it
	if ( !force && cent->currentState.weapon == cent->pe.weapon )
	{
		return;
	}

	// Ensure the weapon is registered
	CG_RegisterWeapon ( cent->currentState.weapon );

	// If we have a ghoul model then we can attach it to the right hand of the player model
	if (cg_weapons[cent->currentState.weapon].weaponG2Model)
	{
		if ( cent->pe.weaponModelSpot )
		{
			trap_G2API_DetachG2Model ( cent->ghoul2, cent->pe.weaponModelSpot );
			trap_G2API_RemoveGhoul2Model( &cent->ghoul2, cent->pe.weaponModelSpot );
			cent->pe.weaponModelSpot = 0;
		}

		cent->pe.weaponModelSpot = trap_G2API_CopySpecificGhoul2Model(cg_weapons[cent->currentState.weapon].weaponG2Model, 0, cent->ghoul2, -1 ); 
//		trap_G2API_CopySpecificGhoul2Model(cg_weapons[cent->currentState.weapon].weaponG2Model, 0, cent->ghoul2, cent->pe.weaponModelSpot ); 
		trap_G2API_SetBoltInfo(cent->ghoul2, cent->pe.weaponModelSpot, ci->boltWorldWeapon );
		trap_G2API_AttachG2Model(cent->ghoul2, cent->pe.weaponModelSpot, cent->ghoul2, ci->boltWorldWeapon, 0);

		// Special case to make sure the bayonet shows up for the ak74
		switch ( cent->currentState.weapon )
		{
			case WP_AK74_ASSAULT_RIFLE:
				trap_G2API_SetSurfaceOnOff( cent->ghoul2, cent->pe.weaponModelSpot, "bayonet_off", 0 );
				break;

			case WP_M4_ASSAULT_RIFLE:
				trap_G2API_SetSurfaceOnOff( cent->ghoul2, cent->pe.weaponModelSpot, "m203_off", 0 );
				break;
		}
	}

	// Update the weapon in the player entity
	cent->pe.weapon = cent->currentState.weapon;
}

/*
===============
CG_UpdatePlayerModel
===============
*/
void CG_UpdatePlayerModel ( centity_t* cent )
{
	if ((cent->ghoul2 == NULL) && trap_G2_HaveWeGhoul2Models(cgs.clientinfo[cent->currentState.clientNum].ghoul2Model))
	{
		CG_ResetPlayerEntity ( cent );

		if (!cgs.clientinfo[cent->currentState.clientNum].ghoul2Model)
		{
			Com_Error(ERR_DROP, "CG_UpdatePlayerModel invalid g2 pointer for client\n");
		}
		trap_G2API_DuplicateGhoul2Instance(cgs.clientinfo[cent->currentState.clientNum].ghoul2Model, &cent->ghoul2);
	
		// Force the players weapon model to be updated since their main model changed
		CG_UpdatePlayerWeaponModel ( cent, qtrue );
	}
}

/*
===============
CG_Player
===============
*/
void CG_Player( centity_t *cent ) 
{
	clientInfo_t	*ci;
	refEntity_t		player;
	int				clientNum;
	int				renderfx;
	qboolean		shadow;
	float			shadowPlane;
	int				iwantout = 0;
	vec3_t			save;

	// Dont draw any player models when the round has ended
	if ( cg.snap->ps.pm_type == PM_INTERMISSION )
	{
		return;
	}

	// the client number is stored in clientNum.  It can't be derived
	// from the entity number, because a single client may have
	// multiple corpses on the level using the same clientinfo
	clientNum = cent->currentState.clientNum;
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) 
	{
		Com_Error( ERR_FATAL, "Bad clientNum on player entity");
	}

	ci = &cgs.clientinfo[ clientNum ];

	// it is possible to see corpses from disconnected players that may
	// not have valid clientinfo
	if ( !ci->infoValid ) 
	{
		return;
	}

	// don't draw the death sequences, as the body queue should be doing it for us
	if ( cent->currentState.eFlags & EF_DEAD )
	{	
		return;
	}

	// Add the player to the radar if on the same team and its a team game
	if ( cgs.gametypeData->teams )
	{
		if ( cg.snap->ps.clientNum != cent->currentState.number && ci->team == cgs.clientinfo[ cg.snap->ps.clientNum ].team )
		{
			cg.radarEntities[cg.radarEntityCount++] = cent;
		}
	}

	// get the player model information
	renderfx = 0;
	if ( cent->currentState.number == cg.snap->ps.clientNum) 
	{
		// If rendering third person then we shouldnt draw the player if 
		// the view origin is too close to the player (ie, inside it)
		if ( cg.renderingThirdPerson )
		{
			vec3_t diff;
			vec3_t a;
			vec3_t b;
			float f;

			VectorCopy ( cg.refdef.vieworg, a );
			VectorCopy ( cent->lerpOrigin, b );
//			a[2] = 0;
//			b[2] = 0;

			VectorSubtract ( a, b, diff );

			f = VectorLengthSquared ( diff );
			if ( VectorLengthSquared ( diff ) < 1800 )
			{
				renderfx = RF_THIRD_PERSON;
			}
		}
		// only draw in mirrors
		else
		{
			renderfx = RF_THIRD_PERSON;			
		} 
	}

	// Initialize the ref entity
	memset (&player, 0, sizeof(player));

	// NOTENOTE Temporary
	VectorSet(player.modelScale, 1,1,1);
	player.radius = 64;
	VectorClear(player.angles);

	// add the shadow
	shadow = CG_PlayerShadow( cent, &shadowPlane );

	if (iwantout)
	{
		return;
	}

	// add a water splash if partially in and out of water
//	CG_PlayerSplash( cent );

	if ( cg_shadows.integer == 3 && shadow ) 
	{
		renderfx |= RF_SHADOW_PLANE;
	}

	// use the same origin for all
	renderfx |= RF_LIGHTING_ORIGIN | RF_MINLIGHT;			

	VectorCopy( cent->lerpOrigin, player.origin );
	VectorCopy( cent->lerpOrigin, player.lightingOrigin );

	player.shadowPlane = shadowPlane;
	player.renderfx = renderfx;
	
	// don't positionally lerp at all
	VectorCopy (player.origin, player.oldorigin);	

	// get the animation state (after rotation, to allow feet shuffle)
	CG_UpdatePlayerModel ( cent );
	CG_UpdatePlayerAnimations ( cent );
	CG_UpdatePlayerWeaponModel ( cent, qfalse );

	if ( (cent->currentState.eFlags & EF_DEAD ) )
	{
		AnglesToAxis( cent->lerpAngles, player.axis );
	}
	else
	{
		// Force the legs and torso to stay aligned for now to ensure the client
		// and server are in sync with the angles.  
		// TODO: Come up with a way to keep these in sync on both client and server
		cent->pe.torso.yawing = qtrue;
		cent->pe.torso.pitching = qtrue;
		cent->pe.legs.yawing= qtrue;

		BG_PlayerAngles( cent->lerpAngles, 
 					     player.axis, 
						 
						 cent->pe.ghoulLegsAngles, 
						 cent->pe.ghoulLowerTorsoAngles,
						 cent->pe.ghoulUpperTorsoAngles,
						 cent->pe.ghoulHeadAngles,

						 cent->lerpLeanOffset,
	
						 cent->pe.painTime, 
						 cent->pe.painDirection, 
						 cg.time,

						 &cent->pe.torso,
						 &cent->pe.legs,

					     cg.frametime, 
						 cent->lerpVelocity,
						 (cent->currentState.eFlags & EF_DEAD), 
						 cent->currentState.angles2[YAW],
						 cent->ghoul2 );
	}

	VectorCopy ( cent->lerpOrigin, save );
	VectorCopy ( player.origin, cent->lerpOrigin );

	CG_ScaleModelAxis(&player);

	// Copy the ghoul 2 info into the ref entity
	CG_SetGhoul2Info(&player, cent);

	// Now add the player to the scene
	trap_R_AddRefEntityToScene(&player);
	
	//
	// add the gun / barrel / flash
	//
	// need the angle AFTER the lean is added
//	vectoangles( player.axis[0], cent->pe.ghoulRootAngles ); 	

	// Render any weapon related effects
	CG_PlayerWeaponEffects ( &player, cent, ci->team, cent->pe.ghoulLegsAngles ); 
	
	// Render any of the floating sprites above the players head
	CG_PlayerSprites ( cent );

	CG_PlayerGametypeItems ( cent, cent->pe.ghoulLegsAngles );

	VectorCopy ( save, cent->lerpOrigin );
}

/*
===============
CG_ResetPlayerEntity

A player just came into view or teleported, so reset all animation info
===============
*/
void CG_ResetPlayerEntity( centity_t *cent ) 
{
	cent->errorTime = -99999;		// guarantee no error decay added
	cent->extrapolated = qfalse;	

	BG_EvaluateTrajectory( &cent->currentState.pos, cg.time, cent->lerpOrigin );
	BG_EvaluateTrajectory( &cent->currentState.apos, cg.time, cent->lerpAngles );

	VectorCopy( cent->lerpOrigin, cent->rawOrigin );
	VectorCopy( cent->lerpAngles, cent->rawAngles );

	cent->pe.legs.anim = -1;
	cent->pe.torso.anim = -1;

	memset( &cent->pe.legs, 0, sizeof( cent->pe.legs ) );
	cent->pe.legs.yawAngle = cent->rawAngles[YAW];
	cent->pe.legs.yawing = qfalse;
	cent->pe.legs.pitchAngle = 0;
	cent->pe.legs.pitching = qfalse;

	memset( &cent->pe.torso, 0, sizeof( cent->pe.legs ) );
	cent->pe.torso.yawAngle = cent->rawAngles[YAW];
	cent->pe.torso.yawing = qfalse;
	cent->pe.torso.pitchAngle = cent->rawAngles[PITCH];
	cent->pe.torso.pitching = qfalse;
}

/*
=================
CG_ProcessIdentityItems

Attaches all the items for a character skin
=================
*/
static void CG_ProcessIdentityItems ( TGhoul2 ghoul2, TInventoryTemplate *items )
{
	TSurfaceList	*surf;

	while ( items )
	{
		if ( items->mItem )
		{
			if ( items->mItem->mModel && items->mBolt )
			{
				items->mModelIndex = trap_G2API_InitGhoul2Model(&ghoul2, items->mItem->mModel, 0, 0, 0, 0, 0);
				if (items->mModelIndex != -1)
				{
					items->mBoltIndex = trap_G2API_AddBolt ( ghoul2, 0, items->mBolt);
					if (items->mBoltIndex != -1)
					{
						trap_G2API_AttachG2Model( ghoul2, items->mModelIndex, ghoul2, items->mBoltIndex, 0);
					}
				}
			}

			surf = items->mItem->mOffList;
			while(surf)
			{
				trap_G2API_SetSurfaceOnOff( ghoul2, 0, surf->mName, G2SURFACEFLAG_OFF);

				surf = surf->mNext;
			}

			surf = items->mItem->mOnList;
			while(surf)
			{
				trap_G2API_SetSurfaceOnOff( ghoul2, 0, surf->mName, 0);

				surf = surf->mNext;
			}
		}

		items = items->mNext;
	}
}

/*
=================
CG_RemoveIdentityItemsOnBack

Removes all identity items on the back of the given entity. There is
currently no way to get the identity items back after removing them
without killing off the player.
=================
*/
void CG_RemoveIdentityItemsOnBack ( centity_t* cent )
{
	TInventoryTemplate*	items[2];
	TIdentity*			identity;
	int					pass;

	// Grab the identity structure for this entity
	identity = cgs.clientinfo[cent->currentState.number].identity;
	if ( !identity )
	{
		return;
	}

	// Takes two passes to remove em all since the item lists are in two places
	items[0] = identity->mCharacter->mInventory;
	items[1] = identity->mSkin->mInventory;

	for ( pass = 0; pass < 2; pass ++ )
	{
		while ( items[pass] )
		{
			if ( items[pass]->mOnBack && items[pass]->mItem )
			{
				TSurfaceList	*surf;

				// Bolt on?
				if ( items[pass]->mBolt && *items[pass]->mBolt)
				{
					trap_G2API_DetachG2Model ( cent->ghoul2, items[pass]->mModelIndex );
					trap_G2API_RemoveGhoul2Model ( &cent->ghoul2, items[pass]->mModelIndex );
				}

				// Surface list?  Reversed from what process does
				surf = items[pass]->mItem->mOffList;
				while(surf)
				{
					trap_G2API_SetSurfaceOnOff( cent->ghoul2, 0, surf->mName, 0);

					surf = surf->mNext;
				}

				surf = items[pass]->mItem->mOnList;
				while(surf)
				{
					trap_G2API_SetSurfaceOnOff( cent->ghoul2, 0, surf->mName, G2SURFACEFLAG_OFF);

					surf = surf->mNext;
				}
			}

			items[pass] = items[pass]->mNext;
		}
	}
}

/*
=================
CG_RegisterIdentity

Registers an identity
=================
*/
TGhoul2 CG_RegisterIdentity ( TIdentity* identity, char *animationFile, gender_t* gender )
{
	char				name[MAX_QPATH];
	TGenericParser2		skinFile;
	TGPGroup			*basegroup, *group, *sub;
	char				temp[20480], *end;
	int					numPairs;
	TGhoul2				ghoul2Ptr;

	numPairs   = 0;
	end		   = temp;
	*end	   = 0;
	ghoul2Ptr  = NULL;

	*animationFile = 0;

	if (!identity )
	{
		return NULL;
	}

	if (trap_G2API_InitGhoul2Model( &ghoul2Ptr, identity->mCharacter->mModel, 0, 0, 0, (1<<4), 0) == -1)
	{
		return NULL;
	}
	
	trap_G2API_GetAnimFileNameIndex( ghoul2Ptr, 0, name );
	Com_sprintf(animationFile, MAX_QPATH, "%s_mp.cfg", name );

	if ( identity->mCharacter->mParent)
	{
		CG_ProcessIdentityItems( ghoul2Ptr, identity->mCharacter->mParent->mInventory );
	}

	CG_ProcessIdentityItems( ghoul2Ptr, identity->mCharacter->mInventory);
	CG_ProcessIdentityItems( ghoul2Ptr, identity->mSkin->mInventory);

	// don't need the mouth
	trap_G2API_SetSurfaceOnOff( ghoul2Ptr, 0, "mouth_r", G2SURFACEFLAG_OFF|G2SURFACEFLAG_NODESCENDANTS);
	trap_G2API_SetSurfaceOnOff( ghoul2Ptr, 0, "mouth_l", G2SURFACEFLAG_OFF|G2SURFACEFLAG_NODESCENDANTS);

	// Parse the g2skin file
	Com_sprintf( name, sizeof(name), "models/characters/skins/%s.g2skin", identity->mSkin->mSkin );
	skinFile = trap_GP_ParseFile( name, qtrue, qfalse );
	if ( !skinFile )
	{
		trap_G2API_CleanGhoul2Models( &ghoul2Ptr);
		return NULL;
	}

	basegroup = trap_GP_GetBaseParseGroup ( skinFile );
	group = trap_GPG_GetSubGroups ( basegroup );

	while(group)
	{
		trap_GPG_GetName ( group, name );

		// Parse the material
		if ( Q_stricmp ( name, "material") == 0)
		{
			char	matName[MAX_QPATH];
			char	shaderName[MAX_QPATH];

			trap_GPG_FindPairValue ( group, "name", "", matName );

			sub = trap_GPG_FindSubGroup ( group, "group");
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
				end += Com_sprintf(end, sizeof(temp) - (end-temp+1), "%s %s ", matName, shaderName);
				numPairs++;
			}
		}

		group = trap_GPG_GetNext ( group );
	}

	trap_GP_Delete(&skinFile);

	if (numPairs)
	{
		qhandle_t	handle;

		handle = trap_G2API_RegisterSkin( identity->mName, numPairs, temp);
		trap_G2API_SetSkin( ghoul2Ptr, 0, handle);
	}

	// If the gender was requested then look for female in the model
	// name as the indicator
	if ( gender )
	{
		Q_strlwr ( (char*)identity->mCharacter->mModel );
		
		*gender = GENDER_MALE;
		if ( strstr ( identity->mCharacter->mModel, "female" ) )
		{
			*gender = GENDER_FEMALE;
		}
	}

	return ghoul2Ptr;
}
