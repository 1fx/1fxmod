#include "g_local.h"
#include "boe_local.h"
#ifdef _spMaps

///RxCxW - 02.26.06 - 07:53pm #sp map
/// Modified version from MANDOWN sdk
static fileHandle_t entFile;
char	*entBuffer;
int		len;

qboolean G_LoadEntFile(void)
{
	char entPath[128];
	vmCvar_t mapname;
	char alt[5];
	
	// Boe!Man 1/9/13: Memset the buffer, this fixes a crash under Linux.
	//memset(&entBuffer, 0, sizeof(entBuffer));
	// Boe!Man 1/18/13: Not needed anymore, using local calls.
	
	if(g_alternateMap.integer == 1){
		strcpy(alt, "alt/");
		trap_Cvar_Set( "g_alternateMap", "0");
		trap_Cvar_Update ( &g_alternateMap );
		level.altEnt = qtrue;
	}else{
		strcpy(alt, "");
	}
	trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM, 0.0, 0.0 );

	if(current_gametype.value == GT_HS)
		Com_sprintf(entPath, 128, "maps/%sh&s/%s.ent\0", alt, mapname.string);
	else if(current_gametype.value == GT_HZ)
		Com_sprintf(entPath, 128, "maps/%sh&z/%s.ent\0", alt, mapname.string);
	else
		Com_sprintf(entPath, 128, "maps/%s%s/%s.ent\0", alt, level.gametypeData->name, mapname.string, level.gametypeData->name);

	len = trap_FS_FOpenFile(entPath, &entFile, FS_READ);
	if (!entFile){ /// failing that, just try by map name
		if(current_gametype.value == GT_HS)
			Com_sprintf(entPath, 128, "maps/%sh&s/%s_h&s.ent\0", alt, mapname.string);
		else if(current_gametype.value == GT_HZ)
			Com_sprintf(entPath, 128, "maps/%sh&z/%s_h&s.ent\0", alt, mapname.string);
		else
			Com_sprintf(entPath, 128, "maps/%s%s/%s_%s.ent\0", alt, level.gametypeData->name, mapname.string, level.gametypeData->name);
		len = trap_FS_FOpenFile(entPath, &entFile, FS_READ);
		if (!entFile){
			if(current_gametype.value == GT_HS)
				Com_sprintf(entPath, 128, "maps/%s%s_h&s.ent\0",alt, mapname.string);
			else if(current_gametype.value == GT_HZ)
				Com_sprintf(entPath, 128, "maps/%s%s_h&z.ent\0",alt, mapname.string);
			else
				Com_sprintf(entPath, 128, "maps/%s%s_%s.ent\0", alt, mapname.string, level.gametypeData->name); 
			len = trap_FS_FOpenFile(entPath, &entFile, FS_READ);
			if (!entFile){
				Com_sprintf(entPath, 128, "maps/%s%s.ent\0", alt, mapname.string); 
				len = trap_FS_FOpenFile(entPath, &entFile, FS_READ);
				if (!entFile){
					Com_Printf("No ent data found at %s\n", entPath);
					if(strlen(alt) >= 2){
					Com_Printf("Loading default ent now\n");
					trap_Cvar_Set( "g_alternateMap", "0");
					trap_Cvar_Update ( &g_alternateMap );
					level.altEnt = qfalse;
					return G_LoadEntFile();
					}
					return qfalse;
				}
			}
		}
	}
	if(strlen(alt) >= 2)
		Com_Printf(S_COLOR_YELLOW "Loading alternative ent data from \"%s\"\n", entPath);
	else
		Com_Printf(S_COLOR_YELLOW "Loading ent data from \"%s\"\n", entPath);

	if (len >= 131072) {
		Com_Printf( S_COLOR_RED "File too large: %s is %i, max allowed is %i", entPath, len, 131072);
		trap_FS_FCloseFile(entFile);
		return qfalse;
	}
	
	// Boe!Man 1/9/13: We allocate the buffer ourselves. This fixes a nasty crash within the Linux build.
	entBuffer = (char *)trap_VM_LocalTempAlloc(len*sizeof(char));
	#ifdef _DEBUG
	if(entBuffer){
		Com_Printf("Allocated %i bytes memory, available for entity file.\n", len*sizeof(char));
	}
	#endif
	if(!entBuffer){
		Com_Printf("Error while allocating memory for entity file.\n");
		return qfalse;
	}
	
	trap_FS_Read(entBuffer, len, entFile);
	entBuffer[len] = 0;
	trap_FS_FCloseFile(entFile);

	return qtrue;
}
qboolean G_ReadingFromEntFile(qboolean inSubBSP)
{
	if (inSubBSP)
		return qfalse;

	if (RMG.integer)
		return qfalse;

	return (qboolean)(entFile != 0);
}
static char token[MAX_TOKEN_CHARS];

char *G_GetEntFileToken(void)
{
	qboolean hasNewLines = qfalse;
	const char *data;
	int c = 0, len2;
	
	data = entBuffer;
	len2 = 0;
	token[0] = 0;

	/// make sure incoming data is valid
	if (!data){
		// Boe!Man 1/9/13: Free the buffer.
		trap_VM_LocalTempFree(len*sizeof(char));
		entBuffer = NULL;
		len = 0;
		#ifdef _DEBUG
		Com_Printf("Freed entity file buffer.\n");
		#endif
		return NULL;
	}

	while (1){
		/// skip whitespace
		data = SkipWhitespace(entBuffer, &hasNewLines);
		if ( !data ){
			// Boe!Man 1/9/13: Free the buffer.
			trap_VM_LocalTempFree(len*sizeof(char));
			entBuffer = NULL;
			len = 0;
			#ifdef _DEBUG
			Com_Printf("Freed entity file buffer.\n");
			#endif
			return NULL; /// EOF
			///return token;
		}

		c = *data;

		/// skip double slash comments
		if (c == '/' && data[1] == '/') {
			data += 2;
			while (*data && *data != '\n'){
				data++;
			}
		}
		/// skip /* */ comments
		else if ( c=='/' && data[1] == '*') {
			data += 2;
			while (*data && (*data != '*' || data[1] != '/' )) {
				data++;
			}

			if (*data)
				data += 2;
		}
		else
			break;
	}
	///handle quoted strings
	if (c == '\"') {
		data++;
		while (1){
			c = *data++;
			if (c=='\"' || !c) {
				token[len2] = 0;
				entBuffer = ( char * ) data;
				return token;
			}
			if (len2 < MAX_TOKEN_CHARS) {
				token[len2] = c;
				len2++;
			}
		}
	}
	///parse a regular word
	do {
		if (len2 < MAX_TOKEN_CHARS) {
			token[len2] = c;
			len2++;
		}
		data++;
		c = *data;
	} while (c>32);

	if (len2 == MAX_TOKEN_CHARS) {
		///Com_Printf ("Token exceeded %i chars, discarded.\n", MAX_TOKEN_CHARS);
		len2 = 0;
	} 
	token[len2] = 0;
	entBuffer = (char *)data;

	if (token[0] == 0 || token[0] == ' '){
		return NULL; /// EOF
	} 
	return token;
}
#endif
