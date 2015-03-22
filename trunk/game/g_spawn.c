// Copyright (C) 2001-2002 Raven Software.
//

#include "g_local.h"
#include "boe_local.h"

qboolean G_SpawnString( const char *key, const char *defaultString, char **out ) 
{
	int		i;

	if ( !level.spawning ) 
	{
		*out = (char *)defaultString;
	}

	for ( i = 0 ; i < level.numSpawnVars ; i++ ) 
	{
		if ( !Q_stricmp( key, level.spawnVars[i][0] ) ) 
		{
			*out = level.spawnVars[i][1];
			return qtrue;
		}
	}

	*out = (char *)defaultString;
	return qfalse;
}

qboolean G_SpawnFloat( const char *key, const char *defaultString, float *out ) 
{
	char		*s;
	qboolean	present;

	present = G_SpawnString( key, defaultString, &s );
	*out = atof( s );
	return present;
}

qboolean G_SpawnInt( const char *key, const char *defaultString, int *out ) 
{
	char		*s;
	qboolean	present;

	present = G_SpawnString( key, defaultString, &s );
	*out = atoi( s );
	return present;
}

qboolean G_SpawnVector( const char *key, const char *defaultString, float *out ) 
{
	char		*s;
	qboolean	present;

	present = G_SpawnString( key, defaultString, &s );
	sscanf( s, "%f %f %f", &out[0], &out[1], &out[2] );
	return present;
}



//
// fields are needed for spawning from the entity string
//
typedef enum 
{
	F_INT, 
	F_FLOAT,
	F_LSTRING,			// string on disk, pointer in memory, TAG_LEVEL
	F_GSTRING,			// string on disk, pointer in memory, TAG_GAME
	F_VECTOR,
	F_ANGLEHACK,
	F_ENTITY,			// index on disk, pointer in memory
	F_ITEM,				// index on disk, pointer in memory
	F_CLIENT,			// index on disk, pointer in memory
	F_IGNORE

} fieldtype_t;

typedef struct
{
	char*		name;
	int			ofs;
	fieldtype_t	type;
	int			flags;

} field_t;

field_t fields[] = 
{
	{"classname",			FOFS(classname),			F_LSTRING},
	{"origin",				FOFS(s.origin),				F_VECTOR},
	{"model",				FOFS(model),				F_LSTRING},
	{"model2",				FOFS(model2),				F_LSTRING},
	{"spawnflags",			FOFS(spawnflags),			F_INT},
	{"speed",				FOFS(speed),				F_FLOAT},
	{"target",				FOFS(target),				F_LSTRING},
	{"targetname",			FOFS(targetname),			F_LSTRING},
	{"message",				FOFS(message),				F_LSTRING},
	{"team",				FOFS(team),					F_LSTRING},
	{"wait",				FOFS(wait),					F_FLOAT},
	{"random",				FOFS(random),				F_FLOAT},
	{"count",				FOFS(count),				F_INT},
	{"health",				FOFS(health),				F_INT},
	{"light",				0,							F_IGNORE},
	{"dmg",					FOFS(damage),				F_INT},
	{"angles",				FOFS(s.angles),				F_VECTOR},
	{"angle",				FOFS(s.angles),				F_ANGLEHACK},
	{"targetShaderName",	FOFS(targetShaderName),		F_LSTRING},
	{"targetShaderNewName", FOFS(targetShaderNewName),	F_LSTRING},
	{"bspmodel",			FOFS(bspmodel),				F_LSTRING},
	{"mins",				FOFS(r.mins),				F_VECTOR},
	{"maxs",				FOFS(r.maxs),				F_VECTOR},
	{"origin2",				FOFS(s.origin2),			F_VECTOR},
	///Henk 15/01/10 -> Door rotate
	// Boe!Man 1/24/10: This can now be used for door_sliding.
	
	// Henk 26/02/10 -> minimum hiders before teleport can be triggered
	{"minimumhiders",		FOFS(minimumhiders),		F_INT},
	{"apos1",				FOFS(apos1),				F_VECTOR},
	{"apos2",				FOFS(apos2),				F_VECTOR},
	{"distance",			FOFS(distance),				F_FLOAT},
	{"message2",			FOFS(message2),				F_LSTRING},
	{"up",					FOFS(up),					F_INT},
	{"forward",				FOFS(forward),				F_INT},
	{"origin_from",			FOFS(origin_from),			F_VECTOR},
	{"origin_to",			FOFS(origin_to),			F_VECTOR},
	{"angles_from",			FOFS(angles_from),			F_VECTOR},
	{"angles_to",			FOFS(angles_to),			F_VECTOR},
	{"both_sides",			FOFS(both_sides),			F_LSTRING},
	{"max_players",			FOFS(max_players),			F_INT},
	{"min_players",			FOFS(min_players),			F_INT},
	{"invisible",			FOFS(invisible),			F_LSTRING},
	{"auto",				FOFS(autoSection),			F_LSTRING},
	{"hideseek",			FOFS(hideseek),				F_INT},

	// Boe!Man 6/3/11: Parts for the reachable object (sun).
	{"endround",			FOFS(endround),				F_LSTRING},
	{"score",				FOFS(score),				F_INT},
	{"broadcast",			FOFS(broadcast),			F_LSTRING},
	{"effect_touch",		FOFS(effect_touch),			F_LSTRING},
	
	// Boe!Man 5/22/12: Sound set for some entities (like the booster).
	{"sound",				FOFS(sound),				F_LSTRING},
	
	// Boe!Man 6/30/12: Add 'size' -> This is for hideseek_cage.
	{"size",				FOFS(size),					F_LSTRING},
	
	{NULL}
};


typedef struct 
{
	char	*name;
	void	(*spawn)(gentity_t *ent);

} spawn_t;

void SP_info_player_deathmatch		(gentity_t *ent);
void SP_info_player_intermission	(gentity_t *ent);

void SP_func_plat					(gentity_t *ent);
void SP_func_static					(gentity_t *ent);
void SP_func_rotating				(gentity_t *ent);
void SP_func_bobbing				(gentity_t *ent);
void SP_func_pendulum				(gentity_t *ent);
void SP_func_button					(gentity_t *ent);
void SP_func_door					(gentity_t *ent);
void SP_func_train					(gentity_t *ent);
void SP_func_timer					(gentity_t *ent);
void SP_func_glass					(gentity_t *ent);
void SP_func_wall					(gentity_t *ent);

void SP_trigger_always				(gentity_t *ent);
void SP_trigger_multiple			(gentity_t *ent);
void SP_trigger_push				(gentity_t *ent);
void SP_trigger_teleport			(gentity_t *self);
void SP_1fx_teleport				(gentity_t *self);
void SP_trigger_hurt				(gentity_t *ent);
void SP_trigger_ladder				(gentity_t *ent);

void SP_target_give					(gentity_t *ent);
void SP_target_delay				(gentity_t *ent);
void SP_target_speaker				(gentity_t *ent);
void SP_target_print				(gentity_t *ent);
void SP_target_laser				(gentity_t *ent);
void SP_target_score				(gentity_t *ent);
void SP_target_teleporter			(gentity_t *ent);
void SP_target_relay				(gentity_t *ent);
void SP_target_kill					(gentity_t *ent);
void SP_target_position				(gentity_t *ent);
void SP_target_location				(gentity_t *ent);
void SP_target_push					(gentity_t *ent);
void SP_target_effect				(gentity_t *ent);

void SP_info_notnull				(gentity_t *ent);
void SP_info_camp					(gentity_t *ent);
void SP_path_corner					(gentity_t *ent);

void SP_misc_teleporter_dest		(gentity_t *ent);
void SP_misc_model					(gentity_t *ent);
void SP_misc_G2model				(gentity_t *ent);
void SP_misc_portal_camera			(gentity_t *ent);
void SP_misc_portal_surface			(gentity_t *ent);
void SP_misc_bsp					(gentity_t *ent);
void SP_func_door_rotating			(gentity_t *ent);
void SP_terrain						(gentity_t *ent);

void SP_model_static				(gentity_t* ent);
void NV_model						(gentity_t *ent );

void SP_gametype_item				(gentity_t* ent);
void SP_gametype_trigger			(gentity_t* ent);
void SP_gametype_player				(gentity_t* ent);
void SP_mission_player				(gentity_t* ent);
void SP_booster						(gentity_t* ent);
void SP_teleporter					(gentity_t* ent);
void SP_sun							(gentity_t* ent);
void SP_seekers						(gentity_t* ent);
void hideseek_cage					(gentity_t* ent);
void SP_accelerator					(gentity_t* ent);
									
void SP_fx_play_effect				(gentity_t* ent);
void nolower						(gentity_t* ent);
void noroof							(gentity_t *ent);
void nomiddle						(gentity_t *ent);
void nowhole						(gentity_t *ent);
void NV_blocked_trigger				(gentity_t *ent);
void NV_blocked_Teleport			(gentity_t *ent);
void NV_misc_bsp					(gentity_t *ent); 

#ifdef _3DServer
void SP_monkey_player				(gentity_t* ent);
#endif // _3DServer

spawn_t	spawns[] = 
{
	// info entities don't do anything at all, but provide positional
	// information for things controlled by other processes
	{"info_player_deathmatch",		SP_info_player_deathmatch},
	{"info_player_intermission",	SP_info_player_intermission},
	{"info_notnull",				SP_info_notnull},		// use target_position instead

	{"func_plat",					SP_func_plat},
	{"func_button",					SP_func_button},
	{"func_door",					SP_func_door},
	{"func_static",					SP_func_static},
	{"func_rotating",				SP_func_rotating},
	{"func_bobbing",				SP_func_bobbing},
	{"func_pendulum",				SP_func_pendulum},
	{"func_train",					SP_func_train},
	{"func_timer",					SP_func_timer},
	{"func_glass",					SP_func_glass},
	//{"func_wall",					SP_func_wall},

	// Triggers are brush objects that cause an effect when contacted
	// by a living player, usually involving firing targets.
	// While almost everything could be done with
	// a single trigger class and different targets, triggered effects
	// could not be client side predicted (push and teleport).
	{"trigger_always",				SP_trigger_always},
	{"trigger_multiple",			SP_trigger_multiple},
	{"trigger_push",				SP_trigger_push},
	{"trigger_teleport",			SP_trigger_teleport},
	{"1fx_teleport",				SP_1fx_teleport},
	{"trigger_hurt",				SP_trigger_hurt},
	{"trigger_ladder",				SP_trigger_ladder },

	// targets perform no action by themselves, but must be triggered
	// by another entity
	{"target_give",					SP_target_give},
	{"target_delay",				SP_target_delay},
	{"target_speaker",				SP_target_speaker},
	{"target_print",				SP_target_print},
	{"target_laser",				SP_target_laser},
	{"target_score",				SP_target_score},
	{"target_teleporter",			SP_target_teleporter},
	{"target_relay",				SP_target_relay},
	{"target_kill",					SP_target_kill},
	{"target_position",				SP_target_position},
	{"1fx_position",				SP_target_position},
	{"target_location",				SP_target_location},
	{"target_push",					SP_target_push},
	{"target_effect",				SP_target_effect},

	{"path_corner",					SP_path_corner},

	{"misc_teleporter_dest",		SP_misc_teleporter_dest},
	{"misc_model",					SP_misc_model},
	{"client_model",				SP_model_static},
	{"client_model1",				SP_model_static},
	{"misc_G2model",				SP_misc_G2model},
	{"misc_portal_surface",			SP_misc_portal_surface},
	{"misc_portal_camera",			SP_misc_portal_camera},
	{"misc_bsp",					SP_misc_bsp},
	{"terrain",						SP_terrain},
	{"func_door_rotating",			SP_func_door_rotating},
	{"door_rotating",				SP_func_door_rotating},
	{"door_sliding",				SP_func_door},

	{"model_static",				SP_model_static },
	{"nv_model",					NV_model },
	{"blocker",						NV_misc_bsp},

	{"gametype_item",				SP_gametype_item },
	{"gametype_trigger",			SP_gametype_trigger },
	{"gametype_player",				SP_gametype_player },
	{"mission_player",				SP_mission_player },

	// stuff from SP emulated
	{"func_breakable_brush",		SP_func_static},
	{"fx_play_effect",				SP_fx_play_effect},
	{"1fx_play_effect",				SP_fx_play_effect}, // internal use so we can clean it up
	{"nolower",						nolower},
	{"noroof",						noroof},
	{"nomiddle",					nomiddle},
	{"nowhole",						nowhole},
	{"blocked_trigger",				NV_blocked_trigger},
	{"blocked_teleporter",			NV_blocked_Teleport},
	{"booster",						SP_booster},
	{"teleporter",					SP_teleporter},
	{"seekers",						SP_seekers},
	{"reachable_object",			SP_sun},
	{"hideseek_cage",				hideseek_cage},
	{"accelerator",					SP_accelerator},
	// The following classnames are instantly removed when spawned.  The RMG 
	// shares instances with single player which is what causes these things
	// to attempt to spawn
	{"light",						0},
	{"func_group",					0},
	{"info_camp",					0},
	{"info_null",					0},
	{"emplaced_wpn",				0},
	{"func_wall",					0},
	{"info_NPC*",					0},
	{"info_player_start",			0},
	{"NPC_*",						0},
	{"ce_*",						0},
	{"pickup_ammo",					0},
	{"script_runner",				0},
	{"trigger_arioche_objective",	0},		

	#ifdef _3DServer
	// TODO: If we ever move the monkey code into the base, be sure to properly put it under gametype_player...
	{ "monkey_player",				SP_monkey_player },
	#endif // _3DServer

	{0, 0}
};

/*
===============
G_CallSpawn

Finds the spawn function for the entity and calls it,
returning qfalse if not found
===============
*/
qboolean G_CallSpawn( gentity_t *ent ) 
{
	spawn_t	*s;
	gitem_t	*item;

	if ( !ent->classname ) 
	{
		Com_Printf ("G_CallSpawn: NULL classname\n");
		return qfalse;
	}

	// check item spawn functions
	for ( item=bg_itemlist+1 ; item->classname ; item++ ) 
	{
		if ( !strcmp(item->classname, ent->classname) ) 
		{
			// If this is a backpack then handle it specially
			if ( item->giType == IT_BACKPACK )
			{
				if ( !level.gametypeData->backpack )
				{
					return qfalse;
				}

				G_SpawnItem ( ent, item );
				return qtrue;
			}

			// Make sure pickups arent disabled
			if ( !level.pickupsDisabled )
			{
				G_SpawnItem( ent, item );
				return qtrue;
			}
			else
			{	// Pickups dont spawn when disabled - this avoids the "doesn't have a spawn function" message
				return qfalse;
			}
		}
	}

	// check normal spawn functions
	for ( s=spawns ; s->name ; s++ ) 
	{
		char* wildcard = strchr ( s->name, '*' );
		int   result;
		
		if ( wildcard )
		{
			result = Q_strncmp ( s->name, ent->classname, wildcard - s->name );
		}
		else
		{
			result = strcmp(s->name, ent->classname);
		}

		if ( !result ) 
		{
			if (s->spawn)
			{	// found it
				s->spawn(ent);
				return qtrue;
			}
			else
			{
				return qfalse;
			}
		}
	}
	
	Com_Printf ("%s doesn't have a spawn function\n", ent->classname);
	return qfalse;
}

/*
=============
G_NewString

Builds a copy of the string, translating \n to real linefeeds
so message texts can be multi-line
=============
*/
char *G_NewString( const char *string ) 
{
	char	*newb, *new_p;
	int		i,l;
	
	l = strlen(string) + 1;
#ifdef _TRUEMALLOC
	trap_TrueMalloc((void **)&newb, l);
#else
	newb = (char *)trap_VM_LocalAlloc( l );
#endif

	new_p = newb;

	// turn \n into a real linefeed
	for ( i=0 ; i< l ; i++ ) {
		if (string[i] == '\\' && i < l-1) {
			i++;
			if (string[i] == 'n') {
				*new_p++ = '\n';
			} else {
				*new_p++ = '\\';
			}
		} else {
			*new_p++ = string[i];
		}
	}
	
	return newb;
}

/*
===============
G_ParseField

Takes a key/value pair and sets the binary values
in a gentity
===============
*/
void G_ParseField( const char *key, const char *value, gentity_t *ent ) {
	field_t	*f;
	byte	*b;
	float	v;
	vec3_t	vec;

	for ( f=fields ; f->name ; f++ ) {
		if ( !Q_stricmp(f->name, key) ) {
			// found it
			b = (byte *)ent;

			switch( f->type ) {
			case F_LSTRING:
				*(char **)(b+f->ofs) = G_NewString (value);
				break;
			case F_VECTOR:
				sscanf (value, "%f %f %f", &vec[0], &vec[1], &vec[2]);
				((float *)(b+f->ofs))[0] = vec[0];
				((float *)(b+f->ofs))[1] = vec[1];
				((float *)(b+f->ofs))[2] = vec[2];
				break;
			case F_INT:
				*(int *)(b+f->ofs) = atoi(value);
				break;
			case F_FLOAT:
				*(float *)(b+f->ofs) = atof(value);
				break;
			case F_ANGLEHACK:
				v = atof(value);
				((float *)(b+f->ofs))[0] = 0;
				((float *)(b+f->ofs))[1] = v;
				((float *)(b+f->ofs))[2] = 0;
				break;
			default:
			case F_IGNORE:
				break;
			}
			return;
		}
	}
}

// Ryan Dec 5 2004
// Added from gold fro gametype fixing
/*
===================
G_IsGametypeInList

Determines if the given gametype is in the given list.
===================
*/
qboolean G_IsGametypeInList ( const char* gametype, const char* list )
{
	const char* buf = (char*) list;
	char* token;

	while ( 1 )
	{
		token = COM_Parse ( &buf );
		if ( !token || !token[0] )
		{
			break;
		}

		if ( Q_stricmp ( token, gametype ) == 0 )
		{
			return qtrue;
		}
	}

	return qfalse;
}	
// Ryan

/*
===================
G_SpawnGEntityFromSpawnVars

Spawn an entity and fill in all of the level fields from
level.spawnVars[], then call the class specfic spawn function
===================
*/
int G_SpawnGEntityFromSpawnVars( qboolean inSubBSP ) 
{
	int			i;
	gentity_t	*ent;
	char		*value;

	if (inSubBSP)
	{	
		// filter out the unwanted entities
		G_SpawnString("filter", "", &value);
		if (value[0] && Q_stricmp(level.mFilter, value))
		{	
			// we are not matching up to the filter, so no spawney
			return -1;
		}
	}

	// get the next free entity
	ent = G_Spawn();
	for ( i = 0 ; i < level.numSpawnVars ; i++ ) 
	{
		if(current_gametype.value == GT_HS){
			if(strstr(level.spawnVars[i][1], "gametype_item")){
				ent->think = G_FreeEntity;
				ent->nextthink = level.time+100;
			}else if(strstr(level.spawnVars[i][0], "effect")){
				if(strstr(level.spawnVars[i][1], "flare_blue")){
					level.MM1Flare = ent->s.number;
				}
			}
			if(strstr(level.spawnVars[i][0], "rpg")){
				if(strstr(level.spawnVars[i][1], "true")){
					level.RPGFlare = ent->s.number;
				}else{
					level.M4Flare = ent->s.number;
				}
			}
		}
		if(strstr(level.spawnVars[i][0], "tempent")){
				ent->think = G_FreeEntity;
				ent->nextthink = level.time+100;
				level.tempent = ent->s.number;
		}

		if(G_ReadingFromEntFile(inSubBSP) && strstr(level.spawnVars[i][1], "flare") && current_gametype.value == GT_HS){
			Com_Printf("Ignoring flares in .ent\n");
			G_FreeEntity(ent);
		}else
		G_ParseField( level.spawnVars[i][0], level.spawnVars[i][1], ent );
	}

	// check for "notteam" flag (GT_DM)
	if ( level.gametypeData->teams ) 
	{
		G_SpawnInt( "notteam", "0", &i );
		if ( i ) 
		{
			G_FreeEntity( ent );
			return -1;
		}
	} 
	else 
	{
		G_SpawnInt( "notfree", "0", &i );
		if ( i ) 
		{
			G_FreeEntity( ent );
			return -1;
		}
	}

	// Only spawn this entity in the specified gametype
	// Ryan Dec 5 2004
	// More code from gold to fix gametypes
	if( G_SpawnString( "gametype", NULL, &value ) && value ) 
	{
		if ( !G_IsGametypeInList ( level.gametypeData->name, value ) )
		{
			if ( level.gametypeData->basegametype )
			{
				if ( !G_IsGametypeInList ( level.gametypeData->basegametype, value ) )
				{
					G_FreeEntity ( ent );
					return -1;
				}
			}
			else
			{
				G_FreeEntity ( ent );
				return -1;
			}
		} 
	}
/*	if( G_SpawnString( "gametype", NULL, &value ) ) 
	{
		// Has to be a case match
		if ( value && !strstr ( value, level.gametypeData->name ) )
		{
			G_FreeEntity ( ent );
			return;
		}
	}
*/
	// Ryan

	// move editor origin to pos
	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );

	// if we didn't get a classname, don't bother spawning anything
		if ( !G_CallSpawn( ent ) ) 
		{
			G_FreeEntity( ent );
		}
		level.numSpawnVars = 0;
		level.numSpawnVarChars = 0;
		return ent->s.number;
}



/*
====================
G_AddSpawnVarToken
====================
*/
char *G_AddSpawnVarToken( const char *string ) 
{
	int		l;
	char	*dest;

	l = strlen( string );
	if ( level.numSpawnVarChars + l + 1 > MAX_SPAWN_VARS_CHARS ) 
	{
		Com_Error( ERR_FATAL, "G_AddSpawnVarToken: MAX_SPAWN_CHARS" );
	}

	dest = level.spawnVarChars + level.numSpawnVarChars;
	memcpy( dest, string, l+1 );

	level.numSpawnVarChars += l + 1;

	return dest;
}

void AddSpawnField(char *field, char *value)
{
	int	i;

	for(i=0;i<level.numSpawnVars;i++)
	{
		if (Q_stricmp(level.spawnVars[i][0], field) == 0)
		{
			level.spawnVars[ i ][1] = G_AddSpawnVarToken( value );
			return;
		}
	}

	level.spawnVars[ level.numSpawnVars ][0] = G_AddSpawnVarToken( field );
	level.spawnVars[ level.numSpawnVars ][1] = G_AddSpawnVarToken( value );
	level.numSpawnVars++;
}

#define NOVALUE "novalue"

static void HandleEntityAdjustment(void)
{
	char		*value;
	vec3_t		origin, newOrigin, angles;
	char		temp[MAX_QPATH];
	float		rotation;

	G_SpawnString("origin", NOVALUE, &value);
	if (Q_stricmp(value, NOVALUE) != 0)
	{
		sscanf( value, "%f %f %f", &origin[0], &origin[1], &origin[2] );
	}
	else
	{
		origin[0] = origin[1] = origin[2] = 0.0;
	}

	rotation = DEG2RAD(level.mRotationAdjust);
	newOrigin[0] = origin[0]*cos(rotation) - origin[1]*sin(rotation);
	newOrigin[1] = origin[0]*sin(rotation) + origin[1]*cos(rotation);
	newOrigin[2] = origin[2];
	VectorAdd(newOrigin, level.mOriginAdjust, newOrigin);
	// damn VMs don't handle outputing a float that is compatible with sscanf in all cases
	Com_sprintf(temp, MAX_QPATH, "%0.0f %0.0f %0.0f", newOrigin[0], newOrigin[1], newOrigin[2]);
	AddSpawnField("origin", temp);

	G_SpawnString("angles", NOVALUE, &value);
	if (Q_stricmp(value, NOVALUE) != 0)
	{
		sscanf( value, "%f %f %f", &angles[0], &angles[1], &angles[2] );

		angles[1] = fmod((float)angles[1] + level.mRotationAdjust, (float)360.0);
		// damn VMs don't handle outputing a float that is compatible with sscanf in all cases
		Com_sprintf(temp, MAX_QPATH, "%0.0f %0.0f %0.0f", angles[0], angles[1], angles[2]);
		AddSpawnField("angles", temp);
	}
	else
	{
		G_SpawnString("angle", NOVALUE, &value);
		if (Q_stricmp(value, NOVALUE) != 0)
		{
			sscanf( value, "%f", &angles[1] );
		}
		else
		{
			angles[1] = 0.0;
		}
		angles[1] = fmod((float)angles[1] + level.mRotationAdjust, (float)360.0);
		Com_sprintf(temp, MAX_QPATH, "%0.0f", angles[1]);
		AddSpawnField("angle", temp);
	}

	// RJR experimental code for handling "direction" field of breakable brushes
	// though direction is rarely ever used.
	G_SpawnString("direction", NOVALUE, &value);
	if (Q_stricmp(value, NOVALUE) != 0)
	{
		sscanf( value, "%f %f %f", &angles[0], &angles[1], &angles[2] );
	}
	else
	{
		angles[0] = angles[1] = angles[2] = 0.0;
	}
	angles[1] = fmod((float)angles[1] + level.mRotationAdjust, (float)360.0);
	Com_sprintf(temp, MAX_QPATH, "%0.0f %0.0f %0.0f", angles[0], angles[1], angles[2]);
	AddSpawnField("direction", temp);


	AddSpawnField("BSPInstanceID", level.mTargetAdjust);

	G_SpawnString("targetname", NOVALUE, &value);
	if (Q_stricmp(value, NOVALUE) != 0)
	{
		Com_sprintf(temp, MAX_QPATH, "%s%s", level.mTargetAdjust, value);
		AddSpawnField("targetname", temp);
	}

	G_SpawnString("target", NOVALUE, &value);
	if (Q_stricmp(value, NOVALUE) != 0)
	{
		Com_sprintf(temp, MAX_QPATH, "%s%s", level.mTargetAdjust, value);
		AddSpawnField("target", temp);
	}

	G_SpawnString("killtarget", NOVALUE, &value);
	if (Q_stricmp(value, NOVALUE) != 0)
	{
		Com_sprintf(temp, MAX_QPATH, "%s%s", level.mTargetAdjust, value);
		AddSpawnField("killtarget", temp);
	}

	G_SpawnString("brushparent", NOVALUE, &value);
	if (Q_stricmp(value, NOVALUE) != 0)
	{
		Com_sprintf(temp, MAX_QPATH, "%s%s", level.mTargetAdjust, value);
		AddSpawnField("brushparent", temp);
	}

	G_SpawnString("brushchild", NOVALUE, &value);
	if (Q_stricmp(value, NOVALUE) != 0)
	{
		Com_sprintf(temp, MAX_QPATH, "%s%s", level.mTargetAdjust, value);
		AddSpawnField("brushchild", temp);
	}

	G_SpawnString("enemy", NOVALUE, &value);
	if (Q_stricmp(value, NOVALUE) != 0)
	{
		Com_sprintf(temp, MAX_QPATH, "%s%s", level.mTargetAdjust, value);
		AddSpawnField("enemy", temp);
	}

	G_SpawnString("ICARUSname", NOVALUE, &value);
	if (Q_stricmp(value, NOVALUE) != 0)
	{
		Com_sprintf(temp, MAX_QPATH, "%s%s", level.mTargetAdjust, value);
		AddSpawnField("ICARUSname", temp);
	}
}

/*
====================
G_ParseSpawnVars

Parses a brace bounded set of key / value pairs out of the
level's entity strings into level.spawnVars[]

This does not actually spawn an entity.
====================
*/
qboolean G_ParseSpawnVars( qboolean inSubBSP ) 
{
	char	keyname[MAX_TOKEN_CHARS];
	char	com_token[MAX_TOKEN_CHARS];

#ifdef _spMaps
	char *token;
#endif

	level.numSpawnVars = 0;
	level.numSpawnVarChars = 0;

#ifdef _spMaps
	if (G_ReadingFromEntFile(inSubBSP)){
		char *token;
		token = G_GetEntFileToken();
		if (!token)
			return qfalse;
		Com_sprintf(com_token, sizeof(com_token), "%s", token);
	}
	else
#endif

	// parse the opening brace
	if ( !trap_GetEntityToken( com_token, sizeof( com_token ) ) ) 
	{
		// end of spawn string
		return qfalse;
	}
	
	if ( com_token[0] != '{' ) 
	{
		Com_Error( ERR_FATAL, "G_ParseSpawnVars: found %s when expecting {",com_token );
	}

	// go through all the key / value pairs
	while ( 1 ) 
	{
#ifdef _spMaps
		if (G_ReadingFromEntFile(inSubBSP)) {
			token = G_GetEntFileToken();
			if (!token)
				Com_Error( ERR_FATAL, "G_ParseSpawnVars: EOF without closing brace" );
			Com_sprintf(keyname, sizeof(keyname), "%s", token);
		}
		else
#endif
		// parse key
		if ( !trap_GetEntityToken( keyname, sizeof( keyname ) ) ) 
		{
			Com_Error( ERR_FATAL, "G_ParseSpawnVars: EOF without closing brace" );
		}

		if ( keyname[0] == '}' ) 
		{
			break;
		}

#ifdef _spMaps
		if (G_ReadingFromEntFile(inSubBSP)) {
			token = G_GetEntFileToken();
			if (!token)
				Com_Error( ERR_FATAL, "G_ParseSpawnVars: EOF without closing brace" );
			Com_sprintf(com_token, sizeof(com_token), "%s", token);
		}
		else
#endif

		// parse value	
		if ( !trap_GetEntityToken( com_token, sizeof( com_token ) ) ) 
		{
			Com_Error( ERR_FATAL, "G_ParseSpawnVars: EOF without closing brace" );
		}

		if ( com_token[0] == '}' ) 
		{
			Com_Error( ERR_FATAL, "G_ParseSpawnVars: closing brace without data" );
		}
		
		if ( level.numSpawnVars == MAX_SPAWN_VARS ) 
		{
			Com_Error( ERR_FATAL, "G_ParseSpawnVars: MAX_SPAWN_VARS" );
		}
		
		AddSpawnField(keyname, com_token);
	}

	if (inSubBSP)
	{
		HandleEntityAdjustment();
	}

	return qtrue;
}

static char *defaultStyles[32][3] = 
{
	{	// 0 normal
		"z",
		"z",
		"z"
	},
	{	// 1 FLICKER (first variety)
		"mmnmmommommnonmmonqnmmo",
		"mmnmmommommnonmmonqnmmo",
		"mmnmmommommnonmmonqnmmo"
	},
	{	// 2 SLOW STRONG PULSE
		"abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcb",
		"abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcb",
		"abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcb"
	},
	{	// 3 CANDLE (first variety)
		"mmmmmaaaaammmmmaaaaaabcdefgabcdefg",
		"mmmmmaaaaammmmmaaaaaabcdefgabcdefg",
		"mmmmmaaaaammmmmaaaaaabcdefgabcdefg"
	},
	{	// 4 FAST STROBE
		"mamamamamama",
		"mamamamamama",
		"mamamamamama"
	},
	{	// 5 GENTLE PULSE 1
		"jklmnopqrstuvwxyzyxwvutsrqponmlkj",
		"jklmnopqrstuvwxyzyxwvutsrqponmlkj",
		"jklmnopqrstuvwxyzyxwvutsrqponmlkj"
	},
	{	// 6 FLICKER (second variety)
		"nmonqnmomnmomomno",
		"nmonqnmomnmomomno",
		"nmonqnmomnmomomno"
	},
	{	// 7 CANDLE (second variety)
		"mmmaaaabcdefgmmmmaaaammmaamm",
		"mmmaaaabcdefgmmmmaaaammmaamm",
		"mmmaaaabcdefgmmmmaaaammmaamm"
	},
	{	// 8 CANDLE (third variety)
		"mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa",
		"mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa",
		"mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa"
	},
	{	// 9 SLOW STROBE (fourth variety)
		"aaaaaaaazzzzzzzz",
		"aaaaaaaazzzzzzzz",
		"aaaaaaaazzzzzzzz"
	},
	{	// 10 FLUORESCENT FLICKER
		"mmamammmmammamamaaamammma",
		"mmamammmmammamamaaamammma",
		"mmamammmmammamamaaamammma"
	},
	{	// 11 SLOW PULSE NOT FADE TO BLACK
		"abcdefghijklmnopqrrqponmlkjihgfedcba",
		"abcdefghijklmnopqrrqponmlkjihgfedcba",
		"abcdefghijklmnopqrrqponmlkjihgfedcba"
	},
	{	// 12 FAST PULSE FOR JEREMY
		"mkigegik",
		"mkigegik",
		"mkigegik"
	},
	{	// 13 Test Blending
		"abcdefghijklmqrstuvwxyz",
		"zyxwvutsrqmlkjihgfedcba",
		"aammbbzzccllcckkffyyggp"
	},
	{	// 14
		"",
		"",
		""
	},
	{	// 15
		"",
		"",
		""
	},
	{	// 16
		"",
		"",
		""
	},
	{	// 17
		"",
		"",
		""
	},
	{	// 18
		"",
		"",
		""
	},
	{	// 19
		"",
		"",
		""
	},
	{	// 20
		"",
		"",
		""
	},
	{	// 21
		"",
		"",
		""
	},
	{	// 22
		"",
		"",
		""
	},
	{	// 23
		"",
		"",
		""
	},
	{	// 24
		"",
		"",
		""
	},
	{	// 25
		"",
		"",
		""
	},
	{	// 26
		"",
		"",
		""
	},
	{	// 27
		"",
		"",
		""
	},
	{	// 28
		"",
		"",
		""
	},
	{	// 29
		"",
		"",
		""
	},
	{	// 30
		"",
		"",
		""
	},
	{	// 31
		"",
		"",
		""
	}
};

qboolean SP_bsp_worldspawn ( void )
{
	return qtrue;
}

/*QUAKED worldspawn (0 0 0) ?

Every map should have exactly one worldspawn.
"music"			music wav file
"soundSet"		soundset name to use (do not combine with 'noise', ignores all other flags)
"gravity"		800 is default gravity
"message"		Text to print during connection process
"mission"		Indicates which mission script file should be used to find the scripts for mission mode
*/
void SP_worldspawn( void ) 
{
	char		*text, temp[32];
	int			i;
	int			lengthRed, lengthBlue, lengthGreen;

	G_SpawnString( "classname", "", &text );
	if ( Q_stricmp( text, "worldspawn" ) ) 
	{
		Com_Error( ERR_FATAL, "SP_worldspawn: The first entity isn't 'worldspawn'" );
	}

	// make some data visible to connecting client
	trap_SetConfigstring( CS_GAME_VERSION, GAME_VERSION );

	trap_SetConfigstring( CS_LEVEL_START_TIME, va("%i", level.startTime ) );

	G_SpawnString( "music", "", &text );
	trap_SetConfigstring( CS_MUSIC, text );

	if (G_SpawnString( "soundSet", "", &text ) )
	{
		trap_SetConfigstring(CS_AMBIENT_SOUNDSETS, text );
	}

	if ( level.gametypeData->teams )
	{
		G_SpawnString( "redteam", "", &text );
		if ( text && *text )
		{
#ifdef _TRUEMALLOC
			trap_TrueMalloc((void **)&level.gametypeTeam[TEAM_RED], sizeof(text));
			if(level.gametypeTeam[TEAM_RED]){
				strcpy((char *)level.gametypeTeam[TEAM_RED], text);
			}
#else
			level.gametypeTeam[TEAM_RED] = trap_VM_LocalStringAlloc ( text );
#endif
		}

		G_SpawnString( "blueteam", "", &text );
		if ( text && *text )
		{
#ifdef _TRUEMALLOC
			trap_TrueMalloc((void **)&level.gametypeTeam[TEAM_BLUE], sizeof(text));
			if(level.gametypeTeam[TEAM_BLUE]){
				strcpy((char *)level.gametypeTeam[TEAM_BLUE], text);
			}
#else
			level.gametypeTeam[TEAM_BLUE] = trap_VM_LocalStringAlloc ( text );
#endif			
		}

		if ( !level.gametypeTeam[TEAM_RED]  ||
			 !level.gametypeTeam[TEAM_BLUE]    )
		{
			level.gametypeTeam[TEAM_RED] = "marine";
			level.gametypeTeam[TEAM_BLUE] = "thug";
		}

		trap_SetConfigstring( CS_GAMETYPE_REDTEAM, level.gametypeTeam[TEAM_RED] );
		trap_SetConfigstring( CS_GAMETYPE_BLUETEAM, level.gametypeTeam[TEAM_BLUE] );
	}

	/*
	G_SpawnString( "message", "", &text );
	trap_SetConfigstring( CS_MESSAGE, text );				// map specific message
*/

	RPM_UpdateLoadScreenMessage();
	trap_SetConfigstring( CS_MOTD, g_motd.string );		// message of the day

	// Boe!Man 9/6/12: Fix gravity resetting every round.
	//G_SpawnString( "gravity", "800", &text );
	G_SpawnString( "gravity", va("%d", g_gravity.integer), &text );
	trap_Cvar_Set( "g_gravity", text );

	// Handle all the worldspawn stuff common to both main bsp and sub bsp
	SP_bsp_worldspawn ( );

	g_entities[ENTITYNUM_WORLD].s.number = ENTITYNUM_WORLD;
	g_entities[ENTITYNUM_WORLD].classname = "worldspawn";

	// see if we want a warmup time
	trap_SetConfigstring( CS_WARMUP, "" );
	if ( g_restarted.integer ) 
	{
		trap_Cvar_Set( "g_restarted", "0" );
		level.warmupTime = 0;
	} 
	else if ( g_doWarmup.integer ) 
	{ 
		// Turn it on
		level.warmupTime = -1;
		trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );
		G_LogPrintf( "Warmup:\n" );
	}
	//Ryan if in competition mode auto set the spectators to locked
	if(g_compMode.integer)
	{
		level.specsLocked = 1;
	}
	//Ryan

	trap_SetConfigstring(CS_LIGHT_STYLES+(LS_STYLES_START*3)+0, defaultStyles[0][0]);
	trap_SetConfigstring(CS_LIGHT_STYLES+(LS_STYLES_START*3)+1, defaultStyles[0][1]);
	trap_SetConfigstring(CS_LIGHT_STYLES+(LS_STYLES_START*3)+2, defaultStyles[0][2]);
	
	for(i=1;i<LS_NUM_STYLES;i++)
	{
		Com_sprintf(temp, sizeof(temp), "ls_%dr", i);
		G_SpawnString(temp, defaultStyles[i][0], &text);
		lengthRed = strlen(text);
		trap_SetConfigstring(CS_LIGHT_STYLES+((i+LS_STYLES_START)*3)+0, text);

		Com_sprintf(temp, sizeof(temp), "ls_%dg", i);
		G_SpawnString(temp, defaultStyles[i][1], &text);
		lengthGreen = strlen(text);
		trap_SetConfigstring(CS_LIGHT_STYLES+((i+LS_STYLES_START)*3)+1, text);

		Com_sprintf(temp, sizeof(temp), "ls_%db", i);
		G_SpawnString(temp, defaultStyles[i][2], &text);
		lengthBlue = strlen(text);
		trap_SetConfigstring(CS_LIGHT_STYLES+((i+LS_STYLES_START)*3)+2, text);

		if (lengthRed != lengthGreen || lengthGreen != lengthBlue)
		{
			Com_Error(ERR_DROP, "Style %d has inconsistent lengths: R %d, G %d, B %d", 
				i, lengthRed, lengthGreen, lengthBlue);
		}
	}
	
	// Boe!Man 10/14/12: New check for the CTB minigame.
	trap_Cvar_VariableStringBuffer ( "mapname", level.mapname, MAX_QPATH );
	G_SpawnString( "minigame", "", &text );
	if(strstr(level.mapname, "col9") && current_gametype.value == GT_HS && strstr(text, "ctb")){
		level.crossTheBridge = qtrue;
		Com_Printf("Cross The Bridge entity file, reloading weapon file.\n");
		RPM_WeaponMod();
	}
	// Boe!Man 9/11/12: Every lvl has a worldspawn. If effects are loaded before the preloaded effects, the effectindex will mess up. Avoid this by spawning them DIRECTLY after the worldspawn.
	Preload_Effects();
}


/*
==============
G_SpawnEntitiesFromString

Parses textual entity definitions out of an entstring and spawns gentities.
==============
*/
void G_SpawnEntitiesFromString( qboolean inSubBSP ) 
{
	// allow calls to G_Spawn*()
	level.spawning = qtrue;
	level.numSpawnVars = 0;

	// the worldspawn is not an actual entity, but it still
	// has a "spawn" function to perform any global setup
	// needed by a level (setting configstrings or cvars, etc)

	//RxCxW - 04.20.06 - 03:47pm #spmaps
#ifdef _spMaps
	if (!inSubBSP)
		G_LoadEntFile();
#endif
	//End  - 04.20.06 - 03:48pm

	if ( !G_ParseSpawnVars(inSubBSP) ) 
	{
		Com_Error( ERR_FATAL, "SpawnEntities: no entities" );
	}
	
	if (!inSubBSP)
	{
		SP_worldspawn();
	}
	else
	{
		// Skip this guy if its worldspawn fails
		if ( !SP_bsp_worldspawn() )
		{
			return;
		}
	}

	// parse ents
	while( G_ParseSpawnVars(inSubBSP) ) 
	{
		G_SpawnGEntityFromSpawnVars(inSubBSP);
	}	

	if (!inSubBSP)
	{
		level.spawning = qfalse;			// any future calls to G_Spawn*() will be errors
	}
}

void NV_model( gentity_t *ent ) 
{
	ent->s.modelindex = G_ModelIndex( ent->model );
	VectorSet (ent->r.mins, -16, -16, -16);
	VectorSet (ent->r.maxs, 16, 16, 16);
	trap_LinkEntity (ent);

	G_SetOrigin( ent, ent->s.origin );
	VectorCopy( ent->s.angles, ent->s.apos.trBase );
}

/*QUAKED model_static (1 0 0) (-16 -16 -16) (16 16 16) NO_MP
"model"		arbitrary .md3 file to display
*/
void SP_model_static ( gentity_t* ent )
{
	if (ent->spawnflags & 1)
	{	// NO_MULTIPLAYER
		G_FreeEntity( ent );
	}

	G_SetOrigin( ent, ent->s.origin );
	
	VectorCopy(ent->s.angles, ent->r.currentAngles);
	VectorCopy(ent->s.angles, ent->s.apos.trBase );

	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );

	ent->s.modelindex = G_ModelIndex( ent->model );
	ent->s.pos.trType = TR_STATIONARY;
	ent->s.apos.trTime = level.time;

	if (level.mBSPInstanceDepth)
	{	// this means that this guy will never be updated, moved, changed, etc.
		ent->s.eFlags = EF_PERMANENT;
	}

	trap_LinkEntity ( ent );
}

void NV_misc_bsp(gentity_t *ent) 
{
	char	temp[MAX_QPATH];
	char	*out;
	vec3_t	newAngle;
	int		tempint;
	vec3_t	mins, maxs;
	//int		newBsp = 0;

	// Boe!Man 5/24/12: Using a float isn't foolproof, use a vector instead.
	if(G_SpawnVector("angles", "0 0 0", newAngle)){
		ent->s.angles[0] = newAngle[0];
		ent->s.angles[1] = newAngle[1];
		ent->s.angles[2] = newAngle[2];
#ifdef _SPMAPS	
	VectorCopy( ent->s.angles, ent->savedAngles );
#endif

	}
	// don't support rotation any other way
	//ent->s.angles[0] = 0.0;
	//ent->s.angles[2] = 0.0;
	
	G_SpawnString("bspmodel", "", &out);

	//ent->s.eFlags = /*EF_PERMANENT*/ EF_TELEPORT_BIT;

	// Mainly for debugging
	G_SpawnInt( "spacing", "0", &tempint);
	ent->s.time2 = tempint;
	G_SpawnInt( "flatten", "0", &tempint);
	ent->s.time = tempint;
	Com_sprintf(temp, MAX_QPATH, "#%s", out);

	trap_SetBrushModel( ent, temp );  // SV_SetBrushModel -- sets mins and maxs
	G_BSPIndex(temp);

	if(G_SpawnVector( "maxs", "0 0 0", maxs )){
		//VectorCopy(ent->r.maxs, maxs);
		ent->r.maxs[0] = (int)maxs[0];
		ent->r.maxs[1] = (int)maxs[1];
		ent->r.maxs[2] = (int)maxs[2];
	}
	
	if(G_SpawnVector( "mins", "0 0 0", mins )){
		//VectorCopy(ent->r.mins, mins);
		ent->r.mins[0] = (int)mins[0];
		ent->r.mins[1] = (int)mins[1];
		ent->r.mins[2] = (int)mins[2];
	}
	level.mNumBSPInstances++;
	Com_sprintf(temp, MAX_QPATH, "%d-", level.mNumBSPInstances);
	VectorCopy(ent->s.origin, level.mOriginAdjust);
	level.mRotationAdjust = ent->s.angles[1];
	level.mTargetAdjust = temp;
	level.hasBspInstances = qtrue;
	level.mBSPInstanceDepth++;
	G_SpawnString("filter", "", &out);
	strcpy(level.mFilter, out);
	G_SpawnString("teamfilter", "", &out);
	strcpy(level.mTeamFilter, out);

	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );
	VectorCopy( ent->s.angles, ent->s.apos.trBase );
	VectorCopy( ent->s.angles, ent->r.currentAngles );

	ent->s.eType = ET_MOVER;
	///ent->s.eType = ET_WALL;

	trap_LinkEntity (ent);

	trap_SetActiveSubBSP(ent->s.modelindex);
	G_SpawnEntitiesFromString(qtrue);
	trap_SetActiveSubBSP(-1);

	level.mBSPInstanceDepth--;
	level.mFilter[0] = level.mTeamFilter[0] = 0;

	if ( g_debugRMG.integer )
	{
		G_SpawnDebugCylinder ( ent->s.origin, ent->s.time2, &g_entities[0], 2000, COLOR_WHITE );

		if ( ent->s.time )
		{
			G_SpawnDebugCylinder ( ent->s.origin, ent->s.time, &g_entities[0], 2000, COLOR_RED );
		}
	}
	
}

/*
==============
G_TransformPlayerToObject

Transforms a player into an object.
==============
*/

typedef struct 
{
	char	*modelName;
	char	*angles;
	int		originOffset;
	qboolean isModel; // True if it's a model (md3), false if it's a solid object (bsp).
	char	*mins;
	char	*maxs;
} transformObject_t;

static transformObject_t TransformObjects[] = 
{
	{"instances/Colombia/tree01",						"90 0 0",	20,		qfalse,		NULL,			NULL},
	{"instances/Colombia/tree02",						"0 0 0",	-35,	qfalse,		NULL,			NULL},
	{"instances/Colombia/tree06",						"0 0 0",	-35,	qfalse,		NULL,			NULL},
	{"instances/Colombia/npc_jump1",					"0 0 0",	-35,	qfalse,		NULL,			NULL},
	{"models/objects/common/trash_can_lid_1.md3",		"0 180 0",	-5,		qtrue,		"-17 -17 0",	"17 17 70"},
	{"models/objects/common/yugo.md3",					"0 180 0",	-43,	qtrue,		"-50 -90 0",	"50 80 140"},
	{"models/objects/common/toilet_1.md3",				"0 180 0",	-39,	qtrue,		"-17 -17 0",	"17 17 125"},
	{"models/objects/Airport/box_cart.md3",				"0 180 0",	-39,	qtrue,		"-40 -70 0",	"40 55 150"},
	{"models/objects/Prague/misc/pra3_chandelier.md3",	"0 180 0",	-39,	qtrue,		"-75 -75 0",	"75 75 175"}
};

void TransformPlayerBack(gentity_t *self, gentity_t *other, trace_t *trace)
{
	if ((other->client && other->client->sess.team != TEAM_BLUE) || self->hideseek < 256){
		return;
	}

	self->hideseek -= 256;
	
	if(!g_entities[self->hideseek].client || g_entities[self->hideseek].client->pers.connected != CON_CONNECTED || G_IsClientSpectating(g_entities[self->hideseek].client)){
		trap_SendServerCommand(other-g_entities, "print \"^3[H&S] ^7Woops, nothing here!\n\"");
		G_FreeEntity(self);
		return;
	}
	
	// First we make sure he can walk again.
	g_entities[self->hideseek].client->sess.freeze = qfalse;
	g_entities[self->hideseek].client->ps.pm_type = PM_NORMAL;
	g_entities[self->hideseek].client->sess.invisibleGoggles = qfalse; // And that others can see him again as well.
	
	// Good, now we can free the entities spawned.
	if(g_entities[self->hideseek].client->sess.transformedEntity2){
		G_FreeEntity(&g_entities[g_entities[self->hideseek].client->sess.transformedEntity]);
		g_entities[self->hideseek].client->sess.transformedEntity = 0;
	}
	
	trap_SendServerCommand(-1, va("print \"^3[H&S] ^7%s scared %s back to %s original form!\n\"", other->client->pers.cleanName, g_entities[self->hideseek].client->pers.cleanName, (strstr(g_entities[self->hideseek].client->pers.identity->mCharacter->mModel, "female") ? "her" : "his")));
	
	strncpy(level.RandomNadeLoc, "Disappeared", sizeof(level.RandomNadeLoc));
	G_FreeEntity(self);
}

void G_TransformPlayerToObject(gentity_t *ent)
{
	int object;
	
	// Boe!Man 2/5/14: First pick a random object.
	object = irand(0, sizeof(TransformObjects) / sizeof(TransformObjects[0]) - 1);
	
	// Do this so the server knows where the player is.
	VectorCopy( ent->client->ps.origin, ent->s.origin );
	
	// Put the object on the client their position.
	if(!TransformObjects[object].isModel){
		AddSpawnField("classname", "misc_bsp");
		AddSpawnField("bspmodel", TransformObjects[object].modelName);
	}else{
		AddSpawnField("classname", "nv_model");
		AddSpawnField("model", TransformObjects[object].modelName);
	}
	AddSpawnField("origin", va("%0.f %0.f %0.f", ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] + TransformObjects[object].originOffset));
	AddSpawnField("angles", TransformObjects[object].angles);
	
	// Take care of the player.
	ent->client->sess.invisibleGoggles = qtrue; // Make sure he's invisible,
	ent->client->sess.freeze = qtrue; // .. and that he can't move.
	
	// Reset the transformed entity if there is one.
	if(ent->client->sess.transformedEntity){
		G_FreeEntity(&g_entities[ent->client->sess.transformedEntity]);
		ent->client->sess.transformedEntity = 0;
	}
	if(ent->client->sess.transformedEntity2){
		G_FreeEntity(&g_entities[ent->client->sess.transformedEntity2]);
		ent->client->sess.transformedEntity2 = 0;
	}
	// And store the entity number for later usage.
	ent->client->sess.transformedEntity = G_SpawnGEntityFromSpawnVars(qfalse);
	
	// Don't forget a blocked trigger for models.
	if(TransformObjects[object].isModel){
		AddSpawnField("classname", "blocked_trigger");
		AddSpawnField("model", "BLOCKED_TRIGGER");
		AddSpawnField("origin", va("%0.f %0.f %0.f", ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] + TransformObjects[object].originOffset));
		AddSpawnField("mins", TransformObjects[object].mins);
		AddSpawnField("maxs", TransformObjects[object].maxs);
		ent->client->sess.transformedEntity2 = G_SpawnGEntityFromSpawnVars(qfalse);
		
		// Make sure the seeker can pop the hider out.
		g_entities[ent->client->sess.transformedEntity2].touch = TransformPlayerBack;
		g_entities[ent->client->sess.transformedEntity2].hideseek = ent->s.number + 256;
	}else{
		// Apply the use action on the bsp.
		g_entities[ent->client->sess.transformedEntity].touch = TransformPlayerBack;
		g_entities[ent->client->sess.transformedEntity].hideseek = ent->s.number + 256;
	}
}
