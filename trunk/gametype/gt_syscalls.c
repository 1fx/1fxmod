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

#include "../game/q_shared.h"
#include "gt_public.h"

// this file is only included when building a dll
// gt_syscalls.asm is included instead when building a qvm

static int (QDECL *syscall)( int arg, ... ) = (int (QDECL *)( int, ...))-1;

void dllEntry( int (QDECL *syscallptr)( int arg,... ) ) 
{
	syscall = syscallptr;
}

int PASSFLOAT( float x ) 
{
	float	floatTemp;
	floatTemp = x;
	return *(int *)&floatTemp;
}

void trap_Print( const char *string ) 
{
	syscall( GT_PRINT, string );
}

void trap_Error( const char *string ) {
	syscall( GT_ERROR, string );
}

int trap_Milliseconds( void ) 
{
	return syscall( GT_MILLISECONDS ); 
}

void trap_Cvar_Register ( vmCvar_t *cvar, const char *var_name, const char *value, int flags, float MinValue, float MaxValue ) 
{
	syscall( GT_CVAR_REGISTER, cvar, var_name, value, flags, PASSFLOAT(MinValue), PASSFLOAT(MaxValue) );
}

void trap_Cvar_Update( vmCvar_t *cvar ) 
{
	syscall( GT_CVAR_UPDATE, cvar );
}

void trap_Cvar_Set( const char *var_name, const char *value ) 
{
	syscall( GT_CVAR_SET, var_name, value );
}

int trap_Cvar_VariableIntegerValue( const char *var_name ) 
{
	return syscall( GT_CVAR_VARIABLE_INTEGER_VALUE, var_name );
}

void trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize ) 
{
	syscall( GT_CVAR_VARIABLE_STRING_BUFFER, var_name, buffer, bufsize );
}



void trap_Cmd_Restart ( int delay )
{
	syscall ( GT_RESTART, delay );
}

void trap_Cmd_TextMessage ( int client, const char* message )
{
	syscall ( GT_TEXTMESSAGE, client, message );
}

void trap_Cmd_RadioMessage ( int client, const char* message )
{
	syscall ( GT_RADIOMESSAGE, client, message );
}

int trap_Cmd_RegisterGlobalSound ( const char* sound )
{
	return syscall ( GT_REGISTERGLOBALSOUND, sound );
}

void trap_Cmd_StartGlobalSound ( int sound )
{
	syscall ( GT_STARTGLOBALSOUND, sound );
}

int trap_Cmd_RegisterEffect ( const char* effect )
{
	return syscall ( GT_REGISTEREFFECT, effect );
}

void trap_Cmd_PlayEffect ( int effect, vec3_t origin, vec3_t angles )
{
	syscall ( GT_PLAYEFFECT, effect, origin, angles );
}

void trap_Cmd_AddTeamScore ( team_t team, int score )
{
	syscall ( GT_ADDTEAMSCORE, team, score );
}

void trap_Cmd_AddClientScore ( int clientid, int score )
{
	syscall ( GT_ADDCLIENTSCORE, clientid, score );
}

qboolean trap_Cmd_RegisterItem ( int itemid, const char* name, gtItemDef_t* def )
{
	return (qboolean)syscall ( GT_REGISTERITEM, itemid, name, def );
}

qboolean trap_Cmd_RegisterTrigger ( int trigid, const char* name, gtTriggerDef_t* def )
{
	return (qboolean)syscall ( GT_REGISTERTRIGGER, trigid, name, def );
}

void trap_Cmd_ResetItem ( int itemid )
{
	syscall ( GT_RESETITEM, itemid );
}

void trap_Cmd_GetClientName ( int clientid, const char* buffer, int buffersize )
{
	syscall ( GT_GETCLIENTNAME, clientid, buffer, buffersize );
}

void trap_Cmd_GetClientItems ( int clientid, int* buffer, int buffersize )
{
	syscall ( GT_GETCLIENTITEMS, clientid, buffer, buffersize );
}

qboolean trap_Cmd_DoesClientHaveItem ( int clientid, int itemid )
{
	return (qboolean) syscall ( GT_DOESCLIENTHAVEITEM, clientid, itemid );
}
