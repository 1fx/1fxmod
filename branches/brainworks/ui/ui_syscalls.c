// Copyright (C) 2001-2002 Raven Software
//
#include "ui_local.h"

// this file is only included when building a dll
// syscalls.asm is included instead when building a qvm

static int (QDECL *syscall)( int arg, ... ) = (int (QDECL *)( int, ...))-1;

void dllEntry( int (QDECL *syscallptr)( int arg,... ) ) {
	syscall = syscallptr;
}

int PASSFLOAT( float x ) {
	float	floatTemp;
	floatTemp = x;
	return *(int *)&floatTemp;
}

void trap_Print( const char *string ) {
	syscall( UI_PRINT, string );
}

void trap_Error( const char *string ) {
	syscall( UI_ERROR, string );
}

int trap_Milliseconds( void ) {
	return syscall( UI_MILLISECONDS ); 
}

void trap_Cvar_Register( vmCvar_t *cvar, const char *var_name, const char *value, int flags, float MinValue, float MaxValue ) {
	syscall( UI_CVAR_REGISTER, cvar, var_name, value, flags, PASSFLOAT(MinValue), PASSFLOAT(MaxValue) );
}

void trap_Cvar_Update( vmCvar_t *cvar ) {
	syscall( UI_CVAR_UPDATE, cvar );
}

void trap_Cvar_Set( const char *var_name, const char *value ) {
	syscall( UI_CVAR_SET, var_name, value );
}

float trap_Cvar_VariableValue( const char *var_name ) {
	int temp;
	temp = syscall( UI_CVAR_VARIABLEVALUE, var_name );
	return (*(float*)&temp);
}

void trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	syscall( UI_CVAR_VARIABLESTRINGBUFFER, var_name, buffer, bufsize );
}

void trap_Cvar_SetValue( const char *var_name, float value ) {
	syscall( UI_CVAR_SETVALUE, var_name, PASSFLOAT( value ) );
}

void trap_Cvar_Reset( const char *name ) {
	syscall( UI_CVAR_RESET, name ); 
}

void trap_Cvar_Create( const char *var_name, const char *var_value, int flags ) {
	syscall( UI_CVAR_CREATE, var_name, var_value, flags );
}

void trap_Cvar_InfoStringBuffer( int bit, char *buffer, int bufsize ) {
	syscall( UI_CVAR_INFOSTRINGBUFFER, bit, buffer, bufsize );
}

int trap_Argc( void ) {
	return syscall( UI_ARGC );
}

void trap_Argv( int n, char *buffer, int bufferLength ) {
	syscall( UI_ARGV, n, buffer, bufferLength );
}

void trap_Cmd_ExecuteText( int exec_when, const char *text ) {
	syscall( UI_CMD_EXECUTETEXT, exec_when, text );
}

int trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode ) {
	return syscall( UI_FS_FOPENFILE, qpath, f, mode );
}

void trap_FS_Read( void *buffer, int len, fileHandle_t f ) {
	syscall( UI_FS_READ, buffer, len, f );
}

void trap_FS_Write( const void *buffer, int len, fileHandle_t f ) {
	syscall( UI_FS_WRITE, buffer, len, f );
}

void trap_FS_FCloseFile( fileHandle_t f ) {
	syscall( UI_FS_FCLOSEFILE, f );
}

int trap_FS_GetFileList(  const char *path, const char *extension, char *listbuf, int bufsize ) {
	return syscall( UI_FS_GETFILELIST, path, extension, listbuf, bufsize );
}

qhandle_t trap_R_RegisterModel( const char *name ) {
	return syscall( UI_R_REGISTERMODEL, name );
}

qhandle_t trap_R_RegisterSkin( const char *name ) {
	return syscall( UI_R_REGISTERSKIN, name );
}

qhandle_t trap_R_RegisterFont(const char *fontName) {
	return (qhandle_t)syscall( UI_R_REGISTERFONT, fontName );
}

qhandle_t trap_R_RegisterShaderNoMip( const char *name ) {
	return syscall( UI_R_REGISTERSHADERNOMIP, name );
}

void trap_R_ClearScene( void ) {
	syscall( UI_R_CLEARSCENE );
}

void trap_R_AddRefEntityToScene( const refEntity_t *re ) {
	syscall( UI_R_ADDREFENTITYTOSCENE, re );
}

void trap_R_AddPolyToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts ) {
	syscall( UI_R_ADDPOLYTOSCENE, hShader, numVerts, verts );
}

void trap_R_AddLightToScene( const vec3_t org, float intensity, float r, float g, float b ) {
	syscall( UI_R_ADDLIGHTTOSCENE, org, PASSFLOAT(intensity), PASSFLOAT(r), PASSFLOAT(g), PASSFLOAT(b) );
}

void trap_R_RenderScene( const refdef_t *fd ) {
	syscall( UI_R_RENDERSCENE, fd );
}

void trap_R_SetColor( const float *rgba ) {
	syscall( UI_R_SETCOLOR, rgba );
}

void trap_R_DrawStretchPic( float x, float y, float w, float h, float s1, float t1, float s2, float t2, const float* color, qhandle_t hShader ) {
	syscall( UI_R_DRAWSTRETCHPIC, PASSFLOAT(x), PASSFLOAT(y), PASSFLOAT(w), PASSFLOAT(h), PASSFLOAT(s1), PASSFLOAT(t1), PASSFLOAT(s2), PASSFLOAT(t2), color, hShader );
}

void	trap_R_ModelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs ) {
	syscall( UI_R_MODELBOUNDS, model, mins, maxs );
}

int		trap_R_GetTextWidth  ( const char* text, qhandle_t font, float scale, int limit )
{
	return syscall ( UI_R_GETTEXTWIDTH, text, font, PASSFLOAT(scale), limit );
}
	
int		trap_R_GetTextHeight ( const char* text, qhandle_t font, float scale, int limit )
{
	return syscall ( UI_R_GETTEXTHEIGHT, text, font, PASSFLOAT(scale), limit );
}

void	trap_R_DrawText ( int x, int y, qhandle_t font, float scale, vec4_t color, const char* text, int limit, int flags )
{
	syscall ( UI_R_DRAWTEXT, x, y, font, PASSFLOAT(scale), color, text, limit, flags );
}

void	trap_R_DrawTextWithCursor ( int x, int y, qhandle_t font, float scale, vec4_t color, const char* text, int limit, int flags, int cursorPos, char cursor )
{
	syscall ( UI_R_DRAWTEXTWITHCURSOR, x, y, font, PASSFLOAT(scale), color, text, limit, flags, cursorPos, cursor );
}

void trap_UpdateScreen( qboolean IsLoading ) {
	syscall( UI_UPDATESCREEN, IsLoading );
}

int trap_CM_LerpTag( orientation_t *tag, clipHandle_t mod, int startFrame, int endFrame, float frac, const char *tagName ) {
	return syscall( UI_CM_LERPTAG, tag, mod, startFrame, endFrame, PASSFLOAT(frac), tagName );
}

void trap_S_StartLocalSound( sfxHandle_t sfx, int channelNum ) {
	syscall( UI_S_STARTLOCALSOUND, sfx, channelNum );
}

sfxHandle_t	trap_S_RegisterSound( const char *sample ) {
	return syscall( UI_S_REGISTERSOUND, sample );
}

void trap_Key_KeynumToStringBuf( int keynum, char *buf, int buflen ) {
	syscall( UI_KEY_KEYNUMTOSTRINGBUF, keynum, buf, buflen );
}

void trap_Key_GetBindingBuf( int keynum, char *buf, int buflen ) {
	syscall( UI_KEY_GETBINDINGBUF, keynum, buf, buflen );
}

void trap_Key_SetBinding( int keynum, const char *binding ) {
	syscall( UI_KEY_SETBINDING, keynum, binding );
}

qboolean trap_Key_IsDown( int keynum ) {
	return syscall( UI_KEY_ISDOWN, keynum );
}

qboolean trap_Key_GetOverstrikeMode( void ) {
	return syscall( UI_KEY_GETOVERSTRIKEMODE );
}

void trap_Key_SetOverstrikeMode( qboolean state ) {
	syscall( UI_KEY_SETOVERSTRIKEMODE, state );
}

void trap_Key_ClearStates( void ) {
	syscall( UI_KEY_CLEARSTATES );
}

int trap_Key_GetCatcher( void ) {
	return syscall( UI_KEY_GETCATCHER );
}

void trap_Key_SetCatcher( int catcher ) {
	syscall( UI_KEY_SETCATCHER, catcher );
}

void trap_GetClipboardData( char *buf, int bufsize ) {
	syscall( UI_GETCLIPBOARDDATA, buf, bufsize );
}

void trap_GetClientState( uiClientState_t *state ) {
	syscall( UI_GETCLIENTSTATE, state );
}

void trap_GetGlconfig( glconfig_t *glconfig ) {
	syscall( UI_GETGLCONFIG, glconfig );
}

int trap_GetConfigString( int index, char* buff, int buffsize ) {
	return syscall( UI_GETCONFIGSTRING, index, buff, buffsize );
}

int	trap_LAN_GetServerCount( int source ) {
	return syscall( UI_LAN_GETSERVERCOUNT, source );
}

void trap_LAN_GetServerAddressString( int source, int n, char *buf, int buflen ) {
	syscall( UI_LAN_GETSERVERADDRESSSTRING, source, n, buf, buflen );
}

void trap_LAN_GetServerInfo( int source, int n, char *buf, int buflen ) {
	syscall( UI_LAN_GETSERVERINFO, source, n, buf, buflen );
}

int trap_LAN_GetServerPing( int source, int n ) {
	return syscall( UI_LAN_GETSERVERPING, source, n );
}

int trap_LAN_GetPingQueueCount( void ) {
	return syscall( UI_LAN_GETPINGQUEUECOUNT );
}

int trap_LAN_ServerStatus( const char *serverAddress, char *serverStatus, int maxLen ) {
	return syscall( UI_LAN_SERVERSTATUS, serverAddress, serverStatus, maxLen );
}

void trap_LAN_SaveCachedServers() {
	syscall( UI_LAN_SAVECACHEDSERVERS );
}

void trap_LAN_LoadCachedServers() {
	syscall( UI_LAN_LOADCACHEDSERVERS );
}

void trap_LAN_ResetPings(int n) {
	syscall( UI_LAN_RESETPINGS, n );
}

void trap_LAN_ClearPing( int n ) {
	syscall( UI_LAN_CLEARPING, n );
}

void trap_LAN_GetPing( int n, char *buf, int buflen, int *pingtime ) {
	syscall( UI_LAN_GETPING, n, buf, buflen, pingtime );
}

void trap_LAN_GetPingInfo( int n, char *buf, int buflen ) {
	syscall( UI_LAN_GETPINGINFO, n, buf, buflen );
}

void trap_LAN_MarkServerVisible( int source, int n, qboolean visible ) {
	syscall( UI_LAN_MARKSERVERVISIBLE, source, n, visible );
}

int trap_LAN_ServerIsVisible( int source, int n) {
	return syscall( UI_LAN_SERVERISVISIBLE, source, n );
}

qboolean trap_LAN_UpdateVisiblePings( int source ) {
	return syscall( UI_LAN_UPDATEVISIBLEPINGS, source );
}

int trap_LAN_AddServer(int source, const char *name, const char *addr) {
	return syscall( UI_LAN_ADDSERVER, source, name, addr );
}

void trap_LAN_RemoveServer(int source, const char *addr) {
	syscall( UI_LAN_REMOVESERVER, source, addr );
}

int trap_LAN_CompareServers( int source, int sortKey, int sortDir, int s1, int s2 ) {
	return syscall( UI_LAN_COMPARESERVERS, source, sortKey, sortDir, s1, s2 );
}

int trap_MemoryRemaining( void ) {
	return syscall( UI_MEMORY_REMAINING );
}

void trap_GetCDKey( char *buf, int buflen ) {
	syscall( UI_GET_CDKEY, buf, buflen );
}

void trap_SetCDKey( char *buf ) {
	syscall( UI_SET_CDKEY, buf );
}

int trap_PC_AddGlobalDefine( char *define ) {
	return syscall( UI_PC_ADD_GLOBAL_DEFINE, define );
}

int trap_PC_LoadSource( const char *filename ) {
	return syscall( UI_PC_LOAD_SOURCE, filename );
}

int trap_PC_FreeSource( int handle ) {
	return syscall( UI_PC_FREE_SOURCE, handle );
}

int trap_PC_ReadToken( int handle, pc_token_t *pc_token ) {
	return syscall( UI_PC_READ_TOKEN, handle, pc_token );
}

int trap_PC_SourceFileAndLine( int handle, char *filename, int *line ) {
	return syscall( UI_PC_SOURCE_FILE_AND_LINE, handle, filename, line );
}

int trap_PC_LoadGlobalDefines ( const char* filename )
{
	return syscall ( UI_PC_LOAD_GLOBAL_DEFINES, filename );
}

void trap_PC_RemoveAllGlobalDefines ( void )
{
	syscall ( UI_PC_REMOVE_ALL_GLOBAL_DEFINES );
}

void trap_S_StopBackgroundTrack( void ) {
	syscall( UI_S_STOPBACKGROUNDTRACK );
}

void trap_S_StartBackgroundTrack( const char *intro, const char *loop, qboolean bReturnWithoutStarting) {
	syscall( UI_S_STARTBACKGROUNDTRACK, intro, loop, bReturnWithoutStarting );
}

int trap_RealTime(qtime_t *qtime) {
	return syscall( UI_REAL_TIME, qtime );
}

// this returns a handle.  arg0 is the name in the format "idlogo.roq", set arg1 to NULL, alteredstates to qfalse (do not alter gamestate)
int trap_CIN_PlayCinematic( const char *arg0, int xpos, int ypos, int width, int height, int bits) {
  return syscall(UI_CIN_PLAYCINEMATIC, arg0, xpos, ypos, width, height, bits);
}
 
// stops playing the cinematic and ends it.  should always return FMV_EOF
// cinematics must be stopped in reverse order of when they are started
e_status trap_CIN_StopCinematic(int handle) {
  return syscall(UI_CIN_STOPCINEMATIC, handle);
}


// will run a frame of the cinematic but will not draw it.  Will return FMV_EOF if the end of the cinematic has been reached.
e_status trap_CIN_RunCinematic (int handle) {
  return syscall(UI_CIN_RUNCINEMATIC, handle);
}
 

// draws the current frame
void trap_CIN_DrawCinematic (int handle) {
  syscall(UI_CIN_DRAWCINEMATIC, handle);
}
 

// allows you to resize the animation dynamically
void trap_CIN_SetExtents (int handle, int x, int y, int w, int h) {
  syscall(UI_CIN_SETEXTENTS, handle, x, y, w, h);
}


void trap_R_RemapShader( const char *oldShader, const char *newShader, const char *timeOffset ) 
{
	syscall( UI_R_REMAP_SHADER, oldShader, newShader, timeOffset );
}

qboolean trap_VerifyCDKey( const char *key ) 
{
	return syscall( UI_VERIFY_CDKEY, key );
}

/*
Ghoul2 Insert Start
*/
// CG Specific API calls
void trap_G2_ListModelSurfaces(void *ghlInfo)
{
	syscall( UI_G2_LISTSURFACES, ghlInfo);
}

void trap_G2_ListModelBones(void *ghlInfo, int frame)
{
	syscall( UI_G2_LISTBONES, ghlInfo, frame);
}

void trap_G2_SetGhoul2ModelIndexes(void *ghoul2, qhandle_t *modelList, qhandle_t *skinList)
{
	syscall( UI_G2_SETMODELS, ghoul2, modelList, skinList);
}

int	trap_G2API_AddBolt(void *ghoul2, const int modelIndex, const char *boneName)
{
	return (int) (syscall(UI_G2_ADDBOLT, ghoul2, modelIndex, boneName));
}

qboolean trap_G2API_RemoveBolt(void *ghlInfo, const int modelIndex, const int index)
{
	return (qboolean)(syscall(UI_G2_REMOVEBOLT, ghlInfo, modelIndex, index));
}

qboolean trap_G2API_AttachG2Model(void *ghoul2From, int modelFrom, void *ghoul2To, int toBoltIndex, int toModel)
{
	return (qboolean)(syscall(UI_G2_ATTACHG2MODEL, ghoul2From, modelFrom, ghoul2To, toBoltIndex, toModel));
}

qboolean trap_G2_HaveWeGhoul2Models(	void *ghoul2)
{
	return (qboolean)(syscall(UI_G2_HAVEWEGHOULMODELS, ghoul2));
}

qboolean trap_G2API_GetBoltMatrix(void *ghoul2, const int modelIndex, const int boltIndex, mdxaBone_t *matrix,
								const vec3_t angles, const vec3_t position, const int frameNum, qhandle_t *modelList, vec3_t scale)
{
	return (qboolean)(syscall(UI_G2_GETBOLT, ghoul2, modelIndex, boltIndex, matrix, angles, position, frameNum, modelList, scale));
}

int trap_G2API_InitGhoul2Model(void **ghoul2Ptr, const char *fileName, int modelIndex, qhandle_t customSkin,
						  qhandle_t customShader, int modelFlags, int lodBias)
{
	return syscall(UI_G2_INITGHOUL2MODEL, ghoul2Ptr, fileName, modelIndex, customSkin, customShader, modelFlags, lodBias);
}

void trap_G2API_CleanGhoul2Models(void **ghoul2Ptr)
{
	syscall(UI_G2_CLEANMODELS, ghoul2Ptr);
}

qboolean trap_G2API_SetBoneAngles(void *ghoul2, int modelIndex, const char *boneName, const vec3_t angles, const int flags,
								const int up, const int right, const int forward, qhandle_t *modelList,
								int blendTime , int currentTime )
{
	return (syscall(UI_G2_ANGLEOVERRIDE, ghoul2, modelIndex, boneName, angles, flags, up, right, forward, modelList, blendTime, currentTime));
}

qboolean trap_G2API_SetBoneAnim(void *ghoul2, const int modelIndex, const char *boneName, const int startFrame, const int endFrame,
							  const int flags, const float animSpeed, const int currentTime, const float setFrame , const int blendTime )
{
	return syscall(UI_G2_PLAYANIM, ghoul2, modelIndex, boneName, startFrame, endFrame, flags, PASSFLOAT(animSpeed), currentTime, PASSFLOAT(setFrame), blendTime);
}

char *trap_G2API_GetGLAName(void *ghoul2, int modelIndex)
{
	return (char *)syscall(UI_G2_GETGLANAME, ghoul2, modelIndex);
}

int trap_G2API_CopyGhoul2Instance(void *g2From, void *g2To, int modelIndex)
{
	return syscall(UI_G2_COPYGHOUL2INSTANCE, g2From, g2To, modelIndex);
}

int trap_G2API_CopySpecifiUIhoul2Model(void *g2From, int modelFrom, void *g2To, int modelTo)
{
	return syscall(UI_G2_COPYSPECIFICGHOUL2MODEL, g2From, modelFrom, g2To, modelTo);
}

void trap_G2API_DuplicateGhoul2Instance(void *g2From, void **g2To)
{
	syscall(UI_G2_DUPLICATEGHOUL2INSTANCE, g2From, g2To);
}

qboolean trap_G2API_RemoveGhoul2Model(void **ghlInfo, int modelIndex)
{
	return syscall(UI_G2_REMOVEGHOUL2MODEL, ghlInfo, modelIndex);
}

qboolean trap_G2API_SetSurfaceOnOff(void *ghoul2, const int modelIndex, const char *surfaceName, const int flags)
{
	return syscall(UI_G2_SETSURFACEONOFF, ghoul2, modelIndex, surfaceName, flags);
}

qboolean trap_G2API_GetAnimFileNameIndex ( TGhoul2 ghoul2, qhandle_t modelIndex, const char* filename )
{
	return syscall(UI_G2_GETANIMFILENAMEINDEX, ghoul2, modelIndex, filename );
}

qhandle_t trap_G2API_RegisterSkin ( const char *skinName, int numPairs, const char *skinPairs)
{
	return syscall(UI_G2_REGISTERSKIN, skinName, numPairs, skinPairs );
}

qboolean trap_G2API_SetSkin ( TGhoul2 ghoul2, int modelIndex, qhandle_t customSkin)
{
	return syscall(UI_G2_SETSKIN, ghoul2, modelIndex, customSkin );
}

// CGenericParser2 (void *) routines
TGenericParser2 trap_GP_Parse(char **dataPtr, qboolean cleanFirst, qboolean writeable)
{
	return (TGenericParser2)syscall(UI_GP_PARSE, dataPtr, cleanFirst, writeable);
}

TGenericParser2 trap_GP_ParseFile(char *fileName, qboolean cleanFirst, qboolean writeable)
{
	return (TGenericParser2)syscall(UI_GP_PARSE_FILE, fileName, cleanFirst, writeable);
}

void trap_GP_Clean(TGenericParser2 GP2)
{
	syscall(UI_GP_CLEAN, GP2);
}

void trap_GP_Delete(TGenericParser2 *GP2)
{
	syscall(UI_GP_DELETE, GP2);
}

TGPGroup trap_GP_GetBaseParseGroup(TGenericParser2 GP2)
{
	return (TGPGroup)syscall(UI_GP_GET_BASE_PARSE_GROUP, GP2);
}


// CGPGroup (void *) routines
qboolean trap_GPG_GetName(TGPGroup GPG, char *Value)
{
	return (qboolean)syscall(UI_GPG_GET_NAME, GPG, Value);
}

TGPGroup trap_GPG_GetNext(TGPGroup GPG)
{
	return (TGPGroup)syscall(UI_GPG_GET_NEXT, GPG);
}

TGPGroup trap_GPG_GetInOrderNext(TGPGroup GPG)
{
	return (TGPGroup)syscall(UI_GPG_GET_INORDER_NEXT, GPG);
}

TGPGroup trap_GPG_GetInOrderPrevious(TGPGroup GPG)
{
	return (TGPGroup)syscall(UI_GPG_GET_INORDER_PREVIOUS, GPG);
}

TGPGroup trap_GPG_GetPairs(TGPGroup GPG)
{
	return (TGPGroup)syscall(UI_GPG_GET_PAIRS, GPG);
}

TGPGroup trap_GPG_GetInOrderPairs(TGPGroup GPG)
{
	return (TGPGroup)syscall(UI_GPG_GET_INORDER_PAIRS, GPG);
}

TGPGroup trap_GPG_GetSubGroups(TGPGroup GPG)
{
	return (TGPGroup)syscall(UI_GPG_GET_SUBGROUPS, GPG);
}

TGPGroup trap_GPG_GetInOrderSubGroups(TGPGroup GPG)
{
	return (TGPGroup)syscall(UI_GPG_GET_INORDER_SUBGROUPS, GPG);
}

TGPValue trap_GPG_FindSubGroup(TGPGroup GPG, const char *name)
{
	return (TGPValue)syscall(UI_GPG_FIND_SUBGROUP, GPG, name);
}

TGPValue trap_GPG_FindPair(TGPGroup GPG, const char *key)
{
	return (TGPValue)syscall(UI_GPG_FIND_PAIR, GPG, key);
}

qboolean trap_GPG_FindPairValue(TGPGroup GPG, const char *key, const char *defaultVal, char *Value)
{
	return (qboolean)syscall(UI_GPG_FIND_PAIRVALUE, GPG, key, defaultVal, Value);
}


// CGPValue (void *) routines
qboolean trap_GPV_GetName(TGPValue GPV, char *Value)
{
	return (qboolean)syscall(UI_GPV_GET_NAME, GPV, Value);
}

TGPValue trap_GPV_GetNext(TGPValue GPV)
{
	return (TGPValue)syscall(UI_GPV_GET_NEXT, GPV);
}

TGPValue trap_GPV_GetInOrderNext(TGPValue GPV)
{
	return (TGPValue)syscall(UI_GPV_GET_INORDER_NEXT, GPV);
}

TGPValue trap_GPV_GetInOrderPrevious(TGPValue GPV)
{
	return (TGPValue)syscall(UI_GPV_GET_INORDER_PREVIOUS, GPV);
}

qboolean trap_GPV_IsList(TGPValue GPV)
{
	return (qboolean)syscall(UI_GPV_IS_LIST, GPV);
}

qboolean trap_GPV_GetTopValue(TGPValue GPV, char *Value)
{
	return (qboolean)syscall(UI_GPV_GET_TOP_VALUE, GPV, Value);
}

TGPValue trap_GPV_GetList(TGPValue GPV)
{
	return (TGPValue)syscall(UI_GPV_GET_LIST, GPV);
}

void trap_Parental_Update ( void )
{
	syscall ( UI_PARENTAL_UPDATE );
}

void trap_Parental_SetPassword ( const char* password )
{
	syscall ( UI_PARENTAL_SET_PASSWORD, password );
}

void trap_Parental_GetPassword ( char* password, int buffersize )
{
	syscall ( UI_PARENTAL_GET_PASSWORD, password, buffersize );
}

void *trap_VM_LocalAlloc ( int size )
{
	return (void *)syscall ( UI_VM_LOCALALLOC, size );
}

void *trap_VM_LocalAllocUnaligned ( int size )
{
	return (void *)syscall ( UI_VM_LOCALALLOCUNALIGNED, size );
}

void *trap_VM_LocalTempAlloc( int size )
{
	return (void *)syscall ( UI_VM_LOCALTEMPALLOC, size );
}

void trap_VM_LocalTempFree( int size )
{
	syscall ( UI_VM_LOCALTEMPFREE, size );
}

const char *trap_VM_LocalStringAlloc ( const char *source )
{
	return (const char *)syscall ( UI_VM_LOCALSTRINGALLOC, source );
}

qboolean trap_NET_Available(void)
{
	return (qboolean)syscall ( UI_NET_AVAILABLE );
}

void trap_Version_GetDescription(char *description, int length)
{
	syscall (UI_VERSION_GET_DESCRIPTION, description, length);
}

int trap_Version_GetNumSites(void)
{
	return (int)syscall (UI_VERSION_GET_NUM_SITES);
}

void trap_Version_GetSite(int index, char *site, int length)
{
	syscall(UI_VERSION_GET_SITE, index, site, length);
}

void trap_Version_Download(int index)
{
	syscall(UI_VERSION_DOWNLOAD, index);
}
