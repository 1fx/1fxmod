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
// cg_syscalls.c -- this file is only included when building a dll
// cg_syscalls.asm is included instead when building a qvm
#include "cg_local.h"

static int (QDECL *syscall)( int arg, ... ) = (int (QDECL *)( int, ...))-1;


void dllEntry( int (QDECL  *syscallptr)( int arg,... ) ) {
	syscall = syscallptr;
}


int PASSFLOAT( float x ) {
	float	floatTemp;
	floatTemp = x;
	return *(int *)&floatTemp;
}

void	trap_Print( const char *fmt ) {
	syscall( CG_PRINT, fmt );
}

void	trap_Error( const char *fmt ) {
	syscall( CG_ERROR, fmt );
}

int		trap_Milliseconds( void ) {
	return syscall( CG_MILLISECONDS ); 
}

void	trap_Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags, float MinValue, float MaxValue ) 
{
	syscall( CG_CVAR_REGISTER, vmCvar, varName, defaultValue, flags, PASSFLOAT(MinValue), PASSFLOAT(MaxValue) );
}

void	trap_Cvar_Update( vmCvar_t *vmCvar ) {
	syscall( CG_CVAR_UPDATE, vmCvar );
}

void	trap_Cvar_Set( const char *var_name, const char *value ) {
	syscall( CG_CVAR_SET, var_name, value );
}

void trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	syscall( CG_CVAR_VARIABLESTRINGBUFFER, var_name, buffer, bufsize );
}

int		trap_Argc( void ) {
	return syscall( CG_ARGC );
}

void	trap_Argv( int n, char *buffer, int bufferLength ) {
	syscall( CG_ARGV, n, buffer, bufferLength );
}

void	trap_Args( char *buffer, int bufferLength ) {
	syscall( CG_ARGS, buffer, bufferLength );
}

int		trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode ) {
	return syscall( CG_FS_FOPENFILE, qpath, f, mode );
}

void	trap_FS_Read( void *buffer, int len, fileHandle_t f ) {
	syscall( CG_FS_READ, buffer, len, f );
}

void	trap_FS_Write( const void *buffer, int len, fileHandle_t f ) {
	syscall( CG_FS_WRITE, buffer, len, f );
}

void	trap_FS_FCloseFile( fileHandle_t f ) {
	syscall( CG_FS_FCLOSEFILE, f );
}

void	trap_SendConsoleCommand( const char *text ) {
	syscall( CG_SENDCONSOLECOMMAND, text );
}

void	trap_AddCommand( const char *cmdName ) {
	syscall( CG_ADDCOMMAND, cmdName );
}

void	trap_RemoveCommand( const char *cmdName ) {
	syscall( CG_REMOVECOMMAND, cmdName );
}

void	trap_SendClientCommand( const char *s ) {
	syscall( CG_SENDCLIENTCOMMAND, s );
}

void	trap_UpdateScreen( void ) {
	syscall( CG_UPDATESCREEN );
}

void trap_RMG_Init(int terrainID, const char *terrainInfo)
{
	syscall(CG_RMG_INIT, terrainID, terrainInfo);
}

void	trap_CM_LoadMap( const char *mapname, qboolean SubBSP ) {
	syscall( CG_CM_LOADMAP, mapname, SubBSP );
}

int		trap_CM_NumInlineModels( void ) {
	return syscall( CG_CM_NUMINLINEMODELS );
}

clipHandle_t trap_CM_InlineModel( int index ) {
	return syscall( CG_CM_INLINEMODEL, index );
}

clipHandle_t trap_CM_TempBoxModel( const vec3_t mins, const vec3_t maxs ) {
	return syscall( CG_CM_TEMPBOXMODEL, mins, maxs );
}

clipHandle_t trap_CM_TempCapsuleModel( const vec3_t mins, const vec3_t maxs ) {
	return syscall( CG_CM_TEMPCAPSULEMODEL, mins, maxs );
}

int		trap_CM_PointContents( const vec3_t p, clipHandle_t model ) {
	return syscall( CG_CM_POINTCONTENTS, p, model );
}

int		trap_CM_TransformedPointContents( const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles ) {
	return syscall( CG_CM_TRANSFORMEDPOINTCONTENTS, p, model, origin, angles );
}

void	trap_CM_BoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
						  const vec3_t mins, const vec3_t maxs,
						  clipHandle_t model, int brushmask ) {
	syscall( CG_CM_BOXTRACE, results, start, end, mins, maxs, model, brushmask );
}

void	trap_CM_CapsuleTrace( trace_t *results, const vec3_t start, const vec3_t end,
						  const vec3_t mins, const vec3_t maxs,
						  clipHandle_t model, int brushmask ) {
	syscall( CG_CM_CAPSULETRACE, results, start, end, mins, maxs, model, brushmask );
}

void	trap_CM_TransformedBoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
						  const vec3_t mins, const vec3_t maxs,
						  clipHandle_t model, int brushmask,
						  const vec3_t origin, const vec3_t angles ) {
	syscall( CG_CM_TRANSFORMEDBOXTRACE, results, start, end, mins, maxs, model, brushmask, origin, angles );
}

void	trap_CM_TransformedCapsuleTrace( trace_t *results, const vec3_t start, const vec3_t end,
						  const vec3_t mins, const vec3_t maxs,
						  clipHandle_t model, int brushmask,
						  const vec3_t origin, const vec3_t angles ) {
	syscall( CG_CM_TRANSFORMEDCAPSULETRACE, results, start, end, mins, maxs, model, brushmask, origin, angles );
}

int		trap_CM_MarkFragments( int numPoints, const vec3_t *points, 
				const vec3_t projection,
				int maxPoints, vec3_t pointBuffer,
				int maxFragments, markFragment_t *fragmentBuffer ) {
	return syscall( CG_CM_MARKFRAGMENTS, numPoints, points, projection, maxPoints, pointBuffer, maxFragments, fragmentBuffer );
}

void trap_S_StopAllSounds ( void )
{
	syscall( CG_S_STOPALLSOUNDS );
}

void	trap_S_StartSound( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx, int volume, int radius ) 
{
	syscall( CG_S_STARTSOUND, origin, entityNum, entchannel, sfx, volume, radius );
}

void	trap_S_StartLocalSound( sfxHandle_t sfx, int channelNum ) {
	syscall( CG_S_STARTLOCALSOUND, sfx, channelNum );
}

void	trap_S_ClearLoopingSounds( qboolean killall ) {
	syscall( CG_S_CLEARLOOPINGSOUNDS, killall );
}

void	trap_S_AddLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, float radius, sfxHandle_t sfx ) 
{
	syscall( CG_S_ADDLOOPINGSOUND, entityNum, origin, velocity, PASSFLOAT(radius), sfx );
}

void	trap_S_AddRealLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, float radius, sfxHandle_t sfx ) 
{
	syscall( CG_S_ADDREALLOOPINGSOUND, entityNum, origin, velocity, PASSFLOAT(radius), sfx );
}

void	trap_S_StopLoopingSound( int entityNum ) {
	syscall( CG_S_STOPLOOPINGSOUND, entityNum );
}

void	trap_S_UpdateEntityPosition( int entityNum, const vec3_t origin ) {
	syscall( CG_S_UPDATEENTITYPOSITION, entityNum, origin );
}

void	trap_S_Respatialize( int entityNum, const vec3_t origin, vec3_t axis[3], int inwater ) {
	syscall( CG_S_RESPATIALIZE, entityNum, origin, axis, inwater );
}

sfxHandle_t	trap_S_RegisterSound( const char *sample ) {
	return syscall( CG_S_REGISTERSOUND, sample );
}

void	trap_S_StartBackgroundTrack( const char *intro, const char *loop, qboolean bReturnWithoutStarting  ) {
	syscall( CG_S_STARTBACKGROUNDTRACK, intro, loop, bReturnWithoutStarting  );
}

void	trap_AS_AddPrecacheEntry(const char *set)
{
	syscall( CG_AS_ADDPRECACHEENTRY, set );
}

void	trap_AS_ParseSets(void)
{
	syscall( CG_AS_PARSESETS );
}

void trap_AS_UpdateAmbientSet (const char *name, vec3_t origin)
{
	syscall( CG_AS_UPDATEAMBIENTSET, name, origin );
}

int	trap_AS_AddLocalSet(const char *name, vec3_t listener_origin, vec3_t origin, int entID, int time)
{
	return syscall( CG_AS_ADDLOCALSET, name, listener_origin, origin, entID, time );
}

sfxHandle_t	trap_AS_GetBModelSound(const char *name, int stage)
{
	return (sfxHandle_t)syscall( CG_AS_GETBMODELSOUND, name, stage );
}

void	trap_R_LoadWorldMap( const char *mapname ) {
	syscall( CG_R_LOADWORLDMAP, mapname );
}

qhandle_t trap_R_RegisterModel( const char *name ) {
	return syscall( CG_R_REGISTERMODEL, name );
}

qhandle_t trap_R_RegisterSkin( const char *name ) {
	return syscall( CG_R_REGISTERSKIN, name );
}

qhandle_t trap_R_RegisterShader( const char *name ) {
	return syscall( CG_R_REGISTERSHADER, name );
}

qhandle_t trap_R_RegisterShaderNoMip( const char *name ) {
	return syscall( CG_R_REGISTERSHADERNOMIP, name );
}

qhandle_t trap_R_RegisterFont(const char *fontName) 
{
	return (qhandle_t)syscall(CG_R_REGISTERFONT, fontName );
}

void	trap_R_ClearScene( void ) {
	syscall( CG_R_CLEARSCENE );
}

void trap_R_ClearDecals ( void )
{
	syscall ( CG_R_CLEARDECALS );
}

void	trap_R_AddRefEntityToScene( const refEntity_t *re ) {
	syscall( CG_R_ADDREFENTITYTOSCENE, re );
}

void	trap_R_AddPolyToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts ) {
	syscall( CG_R_ADDPOLYTOSCENE, hShader, numVerts, verts );
}

void trap_R_AddDecalToScene ( qhandle_t shader, const vec3_t origin, const vec3_t dir, float orientation, float r, float g, float b, float a, qboolean alphaFade, float radius, qboolean temporary )
{
	syscall( CG_R_ADDDECALTOSCENE, shader, origin, dir, PASSFLOAT(orientation), PASSFLOAT(r), PASSFLOAT(g), PASSFLOAT(b), PASSFLOAT(a), alphaFade, PASSFLOAT(radius), temporary );
}

void	trap_R_AddPolysToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts, int num ) {
	syscall( CG_R_ADDPOLYSTOSCENE, hShader, numVerts, verts, num );
}

int		trap_R_LightForPoint( vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir ) {
	return syscall( CG_R_LIGHTFORPOINT, point, ambientLight, directedLight, lightDir );
}

void	trap_R_AddLightToScene( const vec3_t org, float intensity, float r, float g, float b ) {
	syscall( CG_R_ADDLIGHTTOSCENE, org, PASSFLOAT(intensity), PASSFLOAT(r), PASSFLOAT(g), PASSFLOAT(b) );
}

void	trap_R_AddAdditiveLightToScene( const vec3_t org, float intensity, float r, float g, float b ) {
	syscall( CG_R_ADDADDITIVELIGHTTOSCENE, org, PASSFLOAT(intensity), PASSFLOAT(r), PASSFLOAT(g), PASSFLOAT(b) );
}

void	trap_R_RenderScene( const refdef_t *fd ) {
	syscall( CG_R_RENDERSCENE, fd );
}

void trap_R_DrawVisualOverlay	( visual_t type, qboolean preProcess, float parm1, float parm2 )
{
	syscall( CG_R_DRAWVISUALOVERLAY, type, preProcess, PASSFLOAT(parm1), PASSFLOAT(parm2) );
}

void	trap_R_SetColor( const float *rgba ) {
	syscall( CG_R_SETCOLOR, rgba );
}

void	trap_R_DrawStretchPic( float x, float y, float w, float h, 
							   float s1, float t1, float s2, float t2, const float* color, qhandle_t hShader ) {
	syscall( CG_R_DRAWSTRETCHPIC, PASSFLOAT(x), PASSFLOAT(y), PASSFLOAT(w), PASSFLOAT(h), PASSFLOAT(s1), PASSFLOAT(t1), PASSFLOAT(s2), PASSFLOAT(t2), color, hShader );
}

void	trap_R_ModelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs ) {
	syscall( CG_R_MODELBOUNDS, model, mins, maxs );
}

int		trap_R_LerpTag( orientation_t *tag, clipHandle_t mod, int startFrame, int endFrame, 
					   float frac, const char *tagName ) {
	return syscall( CG_R_LERPTAG, tag, mod, startFrame, endFrame, PASSFLOAT(frac), tagName );
}

void	trap_R_DrawRotatePic( float x, float y, float w, float h, 
				   float s1, float t1, float s2, float t2,float a, qhandle_t hShader ) 
{
	syscall( CG_R_DRAWROTATEPIC, PASSFLOAT(x), PASSFLOAT(y), PASSFLOAT(w), PASSFLOAT(h), PASSFLOAT(s1), PASSFLOAT(t1), PASSFLOAT(s2), PASSFLOAT(t2), PASSFLOAT(a), hShader );
}

void	trap_R_DrawRotatePic2( float x, float y, float w, float h, 
				   float s1, float t1, float s2, float t2,float a, qhandle_t hShader ) 
{
	syscall( CG_R_DRAWROTATEPIC2, PASSFLOAT(x), PASSFLOAT(y), PASSFLOAT(w), PASSFLOAT(h), PASSFLOAT(s1), PASSFLOAT(t1), PASSFLOAT(s2), PASSFLOAT(t2), PASSFLOAT(a), hShader );
}

void	trap_R_RemapShader( const char *oldShader, const char *newShader, const char *timeOffset ) 
{
	syscall( CG_R_REMAP_SHADER, oldShader, newShader, timeOffset );
}

void	trap_R_GetLightStyle(int style, color4ub_t color)
{
	syscall( CG_R_GET_LIGHT_STYLE, style, color );
}

void	trap_R_SetLightStyle(int style, int color)
{
	syscall( CG_R_SET_LIGHT_STYLE, style, color );
}

int		trap_R_GetTextWidth  ( const char* text, qhandle_t font, float scale, int limit )
{
	return syscall ( CG_R_GETTEXTWIDTH, text, font, PASSFLOAT(scale), limit );
}
	
int		trap_R_GetTextHeight ( const char* text, qhandle_t font, float scale, int limit )
{
	return syscall ( CG_R_GETTEXTHEIGHT, text, font, PASSFLOAT(scale), limit );
}

void	trap_R_DrawText ( int x, int y, qhandle_t font, float scale, vec4_t color, const char* text, int limit, int flags )
{
	syscall ( CG_R_DRAWTEXT, x, y, font, PASSFLOAT(scale), color, text, limit, flags );
}

void trap_R_DrawTextWithCursor ( int x, int y, qhandle_t font, float scale, vec4_t color, const char* text, int limit, int flags, int cursorPos, char cursor )
{
	syscall ( CG_R_DRAWTEXTWITHCURSOR, x, y, font, PASSFLOAT(scale), color, text, limit, flags, cursorPos, cursor );
}

void	trap_FX_AddLine( const vec3_t start, const vec3_t end, float size1, float size2, float sizeParm,
									float alpha1, float alpha2, float alphaParm,
									const vec3_t sRGB, const vec3_t eRGB, float rgbParm,
									int killTime, qhandle_t shader, int flags)
{
	syscall( CG_FX_ADDLINE, start, end, PASSFLOAT(size1), PASSFLOAT(size2), PASSFLOAT(sizeParm),
									PASSFLOAT(alpha1), PASSFLOAT(alpha2), PASSFLOAT(alphaParm),
									sRGB, eRGB, PASSFLOAT(rgbParm),
									killTime, shader, flags);
}

void		trap_GetGlconfig( glconfig_t *glconfig ) {
	syscall( CG_GETGLCONFIG, glconfig );
}

void		trap_GetGameState( gameState_t *gamestate ) {
	syscall( CG_GETGAMESTATE, gamestate );
}

void		trap_GetCurrentSnapshotNumber( int *snapshotNumber, int *serverTime ) {
	syscall( CG_GETCURRENTSNAPSHOTNUMBER, snapshotNumber, serverTime );
}

qboolean	trap_GetSnapshot( int snapshotNumber, snapshot_t *snapshot ) {
	return syscall( CG_GETSNAPSHOT, snapshotNumber, snapshot );
}

qboolean	trap_GetDefaultState(int entityIndex, entityState_t *state )
{
	return syscall( CG_GETDEFAULTSTATE, entityIndex, state );
}

qboolean	trap_GetServerCommand( int serverCommandNumber ) {
	return syscall( CG_GETSERVERCOMMAND, serverCommandNumber );
}

int			trap_GetCurrentCmdNumber( void ) {
	return syscall( CG_GETCURRENTCMDNUMBER );
}

qboolean	trap_GetUserCmd( int cmdNumber, usercmd_t *ucmd ) {
	return syscall( CG_GETUSERCMD, cmdNumber, ucmd );
}

void trap_SetUserCmdValue( int stateValue, float sensitivityScale ) {
	syscall( CG_SETUSERCMDVALUE, stateValue, PASSFLOAT(sensitivityScale) );
}

void trap_RW_SetTeam(int team, qboolean dead )
{
	syscall( CG_RW_SETTEAM, team, dead );
}

void trap_ResetAutorun ( void )
{
	syscall ( CG_RESETAUTORUN );
}

void		testPrintInt( char *string, int i ) {
	syscall( CG_TESTPRINTINT, string, i );
}

void		testPrintFloat( char *string, float f ) {
	syscall( CG_TESTPRINTFLOAT, string, PASSFLOAT(f) );
}

int trap_MemoryRemaining( void ) {
	return syscall( CG_MEMORY_REMAINING );
}

qboolean trap_Key_IsDown( int keynum ) {
	return syscall( CG_KEY_ISDOWN, keynum );
}

int trap_Key_GetCatcher( void ) {
	return syscall( CG_KEY_GETCATCHER );
}

void trap_Key_SetCatcher( int catcher ) {
	syscall( CG_KEY_SETCATCHER, catcher );
}

int trap_Key_GetKey( const char *binding ) {
	return syscall( CG_KEY_GETKEY, binding );
}

int trap_PC_AddGlobalDefine( char *define ) {
	return syscall( CG_PC_ADD_GLOBAL_DEFINE, define );
}

int trap_PC_LoadSource( const char *filename ) {
	return syscall( CG_PC_LOAD_SOURCE, filename );
}

int trap_PC_FreeSource( int handle ) {
	return syscall( CG_PC_FREE_SOURCE, handle );
}

int trap_PC_ReadToken( int handle, pc_token_t *pc_token ) {
	return syscall( CG_PC_READ_TOKEN, handle, pc_token );
}

int trap_PC_SourceFileAndLine( int handle, char *filename, int *line ) {
	return syscall( CG_PC_SOURCE_FILE_AND_LINE, handle, filename, line );
}

int trap_PC_LoadGlobalDefines ( const char* filename )
{
	return syscall ( CG_PC_LOAD_GLOBAL_DEFINES, filename );
}

void trap_PC_RemoveAllGlobalDefines ( void )
{
	syscall ( CG_PC_REMOVE_ALL_GLOBAL_DEFINES );
}

void	trap_S_StopBackgroundTrack( void ) {
	syscall( CG_S_STOPBACKGROUNDTRACK );
}

int trap_RealTime(qtime_t *qtime) {
	return syscall( CG_REAL_TIME, qtime );
}

void trap_SnapVector( float *v ) {
	syscall( CG_SNAPVECTOR, v );
}

// this returns a handle.  arg0 is the name in the format "idlogo.roq", set arg1 to NULL, alteredstates to qfalse (do not alter gamestate)
int trap_CIN_PlayCinematic( const char *arg0, int xpos, int ypos, int width, int height, int bits) {
  return syscall(CG_CIN_PLAYCINEMATIC, arg0, xpos, ypos, width, height, bits);
}
 
// stops playing the cinematic and ends it.  should always return FMV_EOF
// cinematics must be stopped in reverse order of when they are started
e_status trap_CIN_StopCinematic(int handle) {
  return syscall(CG_CIN_STOPCINEMATIC, handle);
}


// will run a frame of the cinematic but will not draw it.  Will return FMV_EOF if the end of the cinematic has been reached.
e_status trap_CIN_RunCinematic (int handle) {
  return syscall(CG_CIN_RUNCINEMATIC, handle);
}
 

// draws the current frame
void trap_CIN_DrawCinematic (int handle) {
  syscall(CG_CIN_DRAWCINEMATIC, handle);
}
 

// allows you to resize the animation dynamically
void trap_CIN_SetExtents (int handle, int x, int y, int w, int h) {
  syscall(CG_CIN_SETEXTENTS, handle, x, y, w, h);
}

qboolean trap_GetEntityToken( char *buffer, int bufferSize ) {
	return syscall( CG_GET_ENTITY_TOKEN, buffer, bufferSize );
}

qboolean trap_R_inPVS( const vec3_t p1, const vec3_t p2 ) {
	return syscall( CG_R_INPVS, p1, p2 );
}

int	trap_FX_RegisterEffect(const char *file)
{
	return syscall( CG_FX_REGISTER_EFFECT, file);
}

void trap_FX_PlaySimpleEffect( const char *file, vec3_t org, int vol, int rad )
{
	syscall( CG_FX_PLAY_SIMPLE_EFFECT, file, org, vol, rad );
}

void trap_FX_PlayEffect( const char *file, vec3_t org, vec3_t fwd, int vol, int rad )
{
	syscall( CG_FX_PLAY_EFFECT, file, org, fwd, vol, rad);
}

void trap_FX_PlayEntityEffect( const char *file, vec3_t org, 
						vec3_t axis[3], const int boltInfo, const int entNum, int vol, int rad )
{
	syscall( CG_FX_PLAY_ENTITY_EFFECT, file, org, axis, boltInfo, entNum, vol, rad );
}

void trap_FX_PlaySimpleEffectID( int id, vec3_t org, int vol, int rad )
{
	syscall( CG_FX_PLAY_SIMPLE_EFFECT_ID, id, org, vol, rad );
}

void trap_FX_PlayEffectID( int id, vec3_t org, vec3_t fwd, int vol, int rad )
{
	syscall( CG_FX_PLAY_EFFECT_ID, id, org, fwd, vol, rad );
}

void trap_FX_PlayEntityEffectID( int id, vec3_t org, 
						vec3_t axis[3], const int boltInfo, const int entNum, int vol, int rad )
{
	syscall( CG_FX_PLAY_ENTITY_EFFECT_ID, id, org, axis, boltInfo, entNum, vol, rad );
}

void trap_FX_PlayBoltedEffectID(int id,CFxBoltInterface *obj, int vol, int rad )
{
	syscall( CG_FX_PLAY_BOLTED_EFFECT_ID, id, obj, vol, rad);
}

void trap_FX_AddScheduledEffects( void )
{
	syscall( CG_FX_ADD_SCHEDULED_EFFECTS );
}

void trap_FX_Draw2DEffects ( float screenXScale, float screenYScale )
{
	syscall( CG_FX_DRAW_2D_EFFECTS, PASSFLOAT(screenXScale), PASSFLOAT(screenYScale) );
}	

int	trap_FX_InitSystem( refdef_t* refdef )
{
	return syscall( CG_FX_INIT_SYSTEM, refdef );
}

qboolean trap_FX_FreeSystem( void )
{
	return syscall( CG_FX_FREE_SYSTEM );
}

void trap_FX_Reset ( void )
{
	syscall ( CG_FX_RESET );
}

void trap_FX_AdjustTime( int time )
{
	syscall( CG_FX_ADJUST_TIME, time );
}

/*
Ghoul2 Insert Start
*/
// CG Specific API calls
void trap_G2_ListModelSurfaces(void *ghlInfo)
{
	syscall( CG_G2_LISTSURFACES, ghlInfo);
}

void trap_G2_ListModelBones(void *ghlInfo, int frame)
{
	syscall( CG_G2_LISTBONES, ghlInfo, frame);
}

void trap_G2_SetGhoul2ModelIndexes(void *ghoul2, qhandle_t *modelList, qhandle_t *skinList)
{
	syscall( CG_G2_SETMODELS, ghoul2, modelList, skinList);
}

void trap_G2API_CollisionDetect ( 
	CollisionRecord_t *collRecMap, 
	void* ghoul2, 
	const vec3_t angles, 
	const vec3_t position,
	int frameNumber, 
	int entNum, 
	const vec3_t rayStart, 
	const vec3_t rayEnd, 
	const vec3_t scale, 
	int traceFlags, 
	int useLod
	)
{
	syscall ( CG_G2_COLLISIONDETECT, collRecMap, ghoul2, angles, position, frameNumber, entNum, rayStart, rayEnd, scale, traceFlags, useLod );
}

int	trap_G2API_AddBolt(void *ghoul2, const int modelIndex, const char *boneName)
{
	return (int) (syscall(CG_G2_ADDBOLT, ghoul2, modelIndex, boneName));
}

void trap_G2API_SetBoltInfo(void *ghoul2, int modelIndex, int boltInfo)
{
	syscall(CG_G2_SETBOLTON, ghoul2, modelIndex, boltInfo);
}

qboolean trap_G2API_RemoveBolt(void *ghlInfo, const int modelIndex, const int index)
{
	return (qboolean)(syscall(CG_G2_REMOVEBOLT, ghlInfo, modelIndex, index));
}

qboolean trap_G2API_AttachG2Model(void *ghoul2From, int modelFrom, void *ghoul2To, int toBoltIndex, int toModel)
{
	return (qboolean)(syscall(CG_G2_ATTACHG2MODEL, ghoul2From, modelFrom, ghoul2To, toBoltIndex, toModel));
}

qboolean	trap_G2API_DetachG2Model(void *ghoul2, int modelIndex)
{
	return (qboolean)(syscall(CG_G2_DETACHG2MODEL, ghoul2, modelIndex));
}

qboolean trap_G2_HaveWeGhoul2Models(	void *ghoul2)
{
	return (qboolean)(syscall(CG_G2_HAVEWEGHOULMODELS, ghoul2));
}

qboolean trap_G2API_GetBoltMatrix(void *ghoul2, const int modelIndex, const int boltIndex, mdxaBone_t *matrix,
								const vec3_t angles, const vec3_t position, const int frameNum, qhandle_t *modelList, vec3_t scale)
{
	return (qboolean)(syscall(CG_G2_GETBOLT, ghoul2, modelIndex, boltIndex, matrix, angles, position, frameNum, modelList, scale));
}

int trap_G2API_InitGhoul2Model(void **ghoul2Ptr, const char *fileName, int modelIndex, qhandle_t customSkin,
						  qhandle_t customShader, int modelFlags, int lodBias)
{
	return syscall(CG_G2_INITGHOUL2MODEL, ghoul2Ptr, fileName, modelIndex, customSkin, customShader, modelFlags, lodBias);
}

qboolean trap_G2API_GetAnimFileNameIndex ( TGhoul2 ghoul2, qhandle_t modelIndex, const char* filename )
{
	return syscall(CG_G2_GETANIMFILENAMEINDEX, ghoul2, modelIndex, filename );
}

qhandle_t trap_G2API_RegisterSkin ( const char *skinName, int numPairs, const char *skinPairs)
{
	return syscall(CG_G2_REGISTERSKIN, skinName, numPairs, skinPairs );
}

qboolean trap_G2API_SetSkin ( TGhoul2 ghoul2, int modelIndex, qhandle_t customSkin)
{
	return syscall(CG_G2_SETSKIN, ghoul2, modelIndex, customSkin );
}

void trap_G2API_CleanGhoul2Models(void **ghoul2Ptr)
{
	syscall(CG_G2_CLEANMODELS, ghoul2Ptr);
}

qboolean trap_G2API_SetBoneAngles(void *ghoul2, int modelIndex, const char *boneName, const vec3_t angles, const int flags,
								const int up, const int right, const int forward, qhandle_t *modelList,
								int blendTime , int currentTime )
{
	return (syscall(CG_G2_ANGLEOVERRIDE, ghoul2, modelIndex, boneName, angles, flags, up, right, forward, modelList, blendTime, currentTime));
}

qboolean trap_G2API_SetBoneAnim(void *ghoul2, const int modelIndex, const char *boneName, const int startFrame, const int endFrame,
							  const int flags, const float animSpeed, const int currentTime, const float setFrame , const int blendTime )
{
	return syscall(CG_G2_PLAYANIM, ghoul2, modelIndex, boneName, startFrame, endFrame, flags, PASSFLOAT(animSpeed), currentTime, PASSFLOAT(setFrame), blendTime);
}

qboolean trap_G2API_GetBoneAnim( void *ghoul2, const int modelIndex, const char *boneName, const int currentTime, float* frame )
{
	return syscall(CG_G2_GETANIM, ghoul2, modelIndex, boneName, currentTime, frame );
}

qboolean trap_G2API_SetSurfaceOnOff(void *ghoul2, const int modelIndex, const char *surfaceName, const int flags)
{
	return syscall(CG_G2_SETSURFACEONOFF, ghoul2, modelIndex, surfaceName, flags);
}

qboolean trap_G2API_SetRootSurface(void **ghoul2, const int modelIndex, const char *surfaceName)
{
	return syscall(CG_G2_SETROOTSURFACE, ghoul2, modelIndex, surfaceName);
}

qboolean trap_G2API_SetNewOrigin(void *ghoul2, const int modelIndex, const int boltIndex)
{
	return syscall(CG_G2_SETNEWORIGIN, ghoul2, modelIndex, boltIndex);
}

char *trap_G2API_GetGLAName(void *ghoul2, int modelIndex)
{
	return (char *)syscall(CG_G2_GETGLANAME, ghoul2, modelIndex);
}

int trap_G2API_CopyGhoul2Instance(void *g2From, void *g2To, int modelIndex)
{
	return syscall(CG_G2_COPYGHOUL2INSTANCE, g2From, g2To, modelIndex);
}

int trap_G2API_CopySpecificGhoul2Model(void *g2From, int modelFrom, void *g2To, int modelTo)
{
	return syscall(CG_G2_COPYSPECIFICGHOUL2MODEL, g2From, modelFrom, g2To, modelTo);
}

void trap_G2API_DuplicateGhoul2Instance(void *g2From, void **g2To)
{
	syscall(CG_G2_DUPLICATEGHOUL2INSTANCE, g2From, g2To);
}

qboolean trap_G2API_RemoveGhoul2Model(void **ghlInfo, int modelIndex)
{
	return syscall(CG_G2_REMOVEGHOUL2MODEL, ghlInfo, modelIndex);
}

void trap_G2API_AddSkinGore(void *ghlInfo,SSkinGoreData *gore)
{
	syscall(CG_G2_ADDSKINGORE, ghlInfo, gore);
}

void trap_G2API_ClearSkinGore ( void* ghlInfo )
{
	syscall(CG_G2_CLEARSKINGORE, ghlInfo );
}

qboolean trap_G2API_SetGhoul2ModelFlags(void *ghlInfo,int flags)
{
	return(syscall(CG_G2_SETGHOUL2MODELFLAGS,ghlInfo,flags));
}

int trap_G2API_GetGhoul2ModelFlags(void *ghlInfo)
{
	return(syscall(CG_G2_GETGHOUL2MODELFLAGS,ghlInfo));
}

qboolean trap_G2API_SetGhoul2ModelFlagsByIndex(void *ghoul2,int index,int flags)
{
	return(syscall(CG_G2_SETGHOUL2MODELFLAGSBYINDEX,ghoul2,index,flags));
}

int trap_G2API_GetGhoul2ModelFlagsByIndex(void *ghoul2,int index)
{
	return(syscall(CG_G2_GETGHOUL2MODELFLAGSBYINDEX,ghoul2,index));
}

int trap_G2API_GetNumModels(TGhoul2 ghoul2)
{
	return syscall(CG_G2_GETNUMMODELS, ghoul2);
}

int trap_G2API_FindBoltIndex(TGhoul2 ghoul2, const int modelIndex, const char *boneName)
{
	return syscall(CG_G2_FINDBOLTINDEX, ghoul2, modelIndex, boneName);
}

int trap_G2API_GetBoltIndex(TGhoul2 ghoul2, const int modelIndex)
{
	return syscall(CG_G2_GETBOLTINDEX, ghoul2, modelIndex);
}

void trap_MAT_Init(void)
{
	syscall(CG_MAT_CACHE);
}

void trap_MAT_Reset(void)
{
	syscall(CG_MAT_RESET);
}

sfxHandle_t trap_MAT_GetSound(char *key, int material)
{
	return (sfxHandle_t)syscall(CG_MAT_GET_SOUND, key, material);
}

qhandle_t trap_MAT_GetDecal(char *key, int material)
{
	return (sfxHandle_t)syscall(CG_MAT_GET_DECAL, key, material);
}

const float trap_MAT_GetDecalScale(char *key, int material)
{
	return (const float)syscall(CG_MAT_GET_DECAL_SCALE, key, material);
}

qhandle_t trap_MAT_GetEffect(char *key, int material)
{
	return (sfxHandle_t)syscall(CG_MAT_GET_EFFECT, key, material);
}

qhandle_t trap_MAT_GetDebris(char *key, int material)
{
	return (sfxHandle_t)syscall(CG_MAT_GET_DEBRIS, key, material);
}

const float trap_MAT_GetDebrisScale(char *key, int material)
{
	return (const float)syscall(CG_MAT_GET_DEBRIS_SCALE, key, material);
}

// CGenericParser2 (void *) routines
TGenericParser2 trap_GP_Parse(char **dataPtr, qboolean cleanFirst, qboolean writeable)
{
	return (TGenericParser2)syscall(GP_PARSE, dataPtr, cleanFirst, writeable);
}

TGenericParser2 trap_GP_ParseFile(char *fileName, qboolean cleanFirst, qboolean writeable)
{
	return (TGenericParser2)syscall(GP_PARSE_FILE, fileName, cleanFirst, writeable);
}

void trap_GP_Clean(TGenericParser2 GP2)
{
	syscall(GP_CLEAN, GP2);
}

void trap_GP_Delete(TGenericParser2 *GP2)
{
	syscall(GP_DELETE, GP2);
}

TGPGroup trap_GP_GetBaseParseGroup(TGenericParser2 GP2)
{
	return (TGPGroup)syscall(GP_GET_BASE_PARSE_GROUP, GP2);
}


// CGPGroup (void *) routines
qboolean trap_GPG_GetName(TGPGroup GPG, char *Value)
{
	return (qboolean)syscall(GPG_GET_NAME, GPG, Value);
}

TGPGroup trap_GPG_GetNext(TGPGroup GPG)
{
	return (TGPGroup)syscall(GPG_GET_NEXT, GPG);
}

TGPGroup trap_GPG_GetInOrderNext(TGPGroup GPG)
{
	return (TGPGroup)syscall(GPG_GET_INORDER_NEXT, GPG);
}

TGPGroup trap_GPG_GetInOrderPrevious(TGPGroup GPG)
{
	return (TGPGroup)syscall(GPG_GET_INORDER_PREVIOUS, GPG);
}

TGPGroup trap_GPG_GetPairs(TGPGroup GPG)
{
	return (TGPGroup)syscall(GPG_GET_PAIRS, GPG);
}

TGPGroup trap_GPG_GetInOrderPairs(TGPGroup GPG)
{
	return (TGPGroup)syscall(GPG_GET_INORDER_PAIRS, GPG);
}

TGPGroup trap_GPG_GetSubGroups(TGPGroup GPG)
{
	return (TGPGroup)syscall(GPG_GET_SUBGROUPS, GPG);
}

TGPGroup trap_GPG_GetInOrderSubGroups(TGPGroup GPG)
{
	return (TGPGroup)syscall(GPG_GET_INORDER_SUBGROUPS, GPG);
}

TGPValue trap_GPG_FindSubGroup(TGPGroup GPG, const char *name)
{
	return (TGPValue)syscall(GPG_FIND_SUBGROUP, GPG, name);
}

TGPValue trap_GPG_FindPair(TGPGroup GPG, const char *key)
{
	return (TGPValue)syscall(GPG_FIND_PAIR, GPG, key);
}

qboolean trap_GPG_FindPairValue(TGPGroup GPG, const char *key, const char *defaultVal, char *Value)
{
	return (qboolean)syscall(GPG_FIND_PAIRVALUE, GPG, key, defaultVal, Value);
}


// CGPValue (void *) routines
qboolean trap_GPV_GetName(TGPValue GPV, char *Value)
{
	return (qboolean)syscall(GPV_GET_NAME, GPV, Value);
}

TGPValue trap_GPV_GetNext(TGPValue GPV)
{
	return (TGPValue)syscall(GPV_GET_NEXT, GPV);
}

TGPValue trap_GPV_GetInOrderNext(TGPValue GPV)
{
	return (TGPValue)syscall(GPV_GET_INORDER_NEXT, GPV);
}

TGPValue trap_GPV_GetInOrderPrevious(TGPValue GPV)
{
	return (TGPValue)syscall(GPV_GET_INORDER_PREVIOUS, GPV);
}

qboolean trap_GPV_IsList(TGPValue GPV)
{
	return (qboolean)syscall(GPV_IS_LIST, GPV);
}

qboolean trap_GPV_GetTopValue(TGPValue GPV, char *Value)
{
	return (qboolean)syscall(GPV_GET_TOP_VALUE, GPV, Value);
}

TGPValue trap_GPV_GetList(TGPValue GPV)
{
	return (TGPValue)syscall(GPV_GET_LIST, GPV);
}

void trap_CM_TM_Create(int terrainID)
{
	syscall(CG_CM_TM_CREATE, terrainID);
}

void trap_CM_TM_AddBuilding(int x, int y, int side)
{
	syscall(CG_CM_TM_ADDBUILDING, x, y, side);
}

void trap_CM_TM_AddSpot(int x, int y)
{
	syscall(CG_CM_TM_ADDSPOT, x, y);
}

void trap_CM_TM_AddTarget(int x, int y, float radius)
{
	syscall(CG_CM_TM_ADDTARGET, x, y, PASSFLOAT(radius));
}

void trap_CM_TM_Upload(const vec3_t origin, const vec3_t angle)
{
	syscall(CG_CM_TM_UPLOAD, origin, angle);
}

void trap_CM_TM_ConvertPosition(void)
{
	syscall(CG_CM_TM_CONVERT_POS);
}

int	trap_CM_RegisterTerrain(const char *config)
{
	return syscall(CG_CM_REGISTER_TERRAIN, config);
}

void trap_RE_InitRendererTerrain( const char *info )
{
	syscall(CG_RE_INIT_RENDERER_TERRAIN, info);
}

void trap_CG_RegisterSharedMemory(char *memory)
{
	syscall(CG_SET_SHARED_BUFFER, memory);
}

int trap_FS_GetFileList(  const char *path, const char *extension, char *listbuf, int bufsize ) 
{
	return syscall( CG_FS_GETFILELIST, path, extension, listbuf, bufsize );
}

void *trap_VM_LocalAlloc ( int size )
{
	return (void *)syscall ( CG_VM_LOCALALLOC, size );
}

void *trap_VM_LocalAllocUnaligned ( int size )
{
	return (void *)syscall ( CG_VM_LOCALALLOCUNALIGNED, size );
}

void *trap_VM_LocalTempAlloc( int size )
{
	return (void *)syscall ( CG_VM_LOCALTEMPALLOC, size );
}

void trap_VM_LocalTempFree( int size )
{
	syscall ( CG_VM_LOCALTEMPFREE, size );
}

const char *trap_VM_LocalStringAlloc ( const char *source )
{
	return (const char *)syscall ( CG_VM_LOCALSTRINGALLOC, source );
}

void trap_UI_CloseAll ( void )
{
	syscall ( CG_UI_CLOSEALL );
}
	
void trap_UI_SetActiveMenu ( int menu )
{
	syscall ( CG_UI_SETACTIVEMENU, menu );
}

