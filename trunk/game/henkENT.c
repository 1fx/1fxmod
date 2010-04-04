
#include  "g_local.h"
#ifdef _spMaps

///RxCxW - 02.26.06 - 07:53pm #sp map
/// Modified version from MANDOWN sdk
static fileHandle_t entFile;
extern char *buffer;

qboolean G_LoadEntFile(void)
{
	char entPath[128];
	vmCvar_t mapname;
	int len;

	trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM, 0.0, 0.0 );
	/// first up, try finding an .ent file with the gametype extension
	//if (level.entExt[0] != 0)
	//	Com_sprintf(entPath, 128, "maps/%s%s.ent\0", mapname.string, level.entExt);
	//else
	Com_sprintf(entPath, 128, "maps/%s_%s.ent\0", mapname.string, level.gametypeData->name);

	len = trap_FS_FOpenFile(entPath, &entFile, FS_READ);

	if (!entFile){ /// failing that, just try by map name
		Com_sprintf(entPath, 128, "maps/%s.ent\0", mapname.string); 
		len = trap_FS_FOpenFile(entPath, &entFile, FS_READ);
		if (!entFile)
			return qfalse;
	}
	Com_Printf(S_COLOR_YELLOW "Loading ent data from \"%s\"\n", entPath);

	if (len >= 131072) {
		Com_Printf( S_COLOR_RED "file too large: %s is %i, max allowed is %i", entPath, len, 131072);
		trap_FS_FCloseFile(entFile);
		return qfalse;
	}

	trap_FS_Read(buffer, len, entFile);
	buffer[len] = 0;
	trap_FS_FCloseFile(entFile);

	return qtrue;
}
qboolean G_ReadingFromEntFile(qboolean inSubBSP)
{
	if (inSubBSP)
		return qfalse;

	if (RMG.integer)
		return qfalse;

	return (entFile != 0);
}
static char token[MAX_TOKEN_CHARS];

char *G_GetEntFileToken(void)
{
	qboolean hasNewLines = qfalse;
	const char *data;
	int c = 0, len;

	data = buffer;
	len = 0;
	token[0] = 0;

	/// make sure incoming data is valid
	if (!data){
		buffer = NULL;
		return NULL;
	}

	while (1){
		/// skip whitespace
		data = SkipWhitespace(buffer, &hasNewLines);
		if ( !data ){
			buffer = NULL;
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
				token[len] = 0;
				buffer = ( char * ) data;
				return token;
			}
			if (len < MAX_TOKEN_CHARS) {
				token[len] = c;
				len++;
			}
		}
	}
	///parse a regular word
	do {
		if (len < MAX_TOKEN_CHARS) {
			token[len] = c;
			len++;
		}
		data++;
		c = *data;
	} while (c>32);

	if (len == MAX_TOKEN_CHARS) {
		///Com_Printf ("Token exceeded %i chars, discarded.\n", MAX_TOKEN_CHARS);
		len = 0;
	} 
	token[len] = 0;
	buffer = (char *)data;

	if (token[0] == 0 || token[0] == ' '){
		return NULL; /// EOF
	} 
	return token;
}
#endif
