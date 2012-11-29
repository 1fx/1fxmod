// Copyright (C) 2001-2002 Raven Software.
//

// gt_public.h -- game type module 

#define	GAMETYPE_API_VERSION	1

typedef enum 
{
	//============== general Quake services ==================

	GT_PRINT,						// ( const char *string );
	GT_ERROR,						// ( const char *string );
	GT_MILLISECONDS,				// ( void );

	GT_CVAR_REGISTER,				// ( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags );
	GT_CVAR_UPDATE,					// ( vmCvar_t *vmCvar );
	GT_CVAR_SET,					// ( const char *var_name, const char *value );
	GT_CVAR_VARIABLE_INTEGER_VALUE,	// ( const char *var_name );
	GT_CVAR_VARIABLE_STRING_BUFFER,	// ( const char *var_name, char *buffer, int bufsize );

	GT_MEMSET = 100,
	GT_MEMCPY,
	GT_STRNCPY,
	GT_SIN,
	GT_COS,
	GT_ATAN2,
	GT_SQRT,
	GT_ANGLEVECTORS,
	GT_PERPENDICULARVECTOR,
	GT_FLOOR,
	GT_CEIL,
	GT_TESTPRINTINT,
	GT_TESTPRINTFLOAT,
	GT_ACOS,
	GT_ASIN,
	GT_MATRIXMULTIPLY,

	
	GT_TEXTMESSAGE,					// void ( int clientid, const char* message );
	GT_RESETITEM,					// void ( int itemid );
	GT_GETCLIENTNAME,				// void ( int clientid, const char* buffer, int buffersize );
									
	GT_REGISTERGLOBALSOUND,			// int  ( const char* filename );
	GT_STARTGLOBALSOUND,			// void ( int soundid );
									
	GT_REGISTERITEM,				// bool ( int itemid, const char* name, gtItemDef_t* def );
	GT_RADIOMESSAGE,				// void ( int clientid, const char* message );
	GT_REGISTERTRIGGER,				// bool ( int trigid, const char* name, gtTriggerDef_t* def );

	GT_GETCLIENTITEMS,				// void ( int clientid, int* buffer, int buffersize );
	GT_DOESCLIENTHAVEITEM,			// bool ( int clientid, int itemid );

	GT_ADDTEAMSCORE,				// void ( team_t team, int score );
	GT_ADDCLIENTSCORE,				// void ( int clientid, int score );

	GT_RESTART,						// void ( int delay );

	GT_REGISTEREFFECT,				// int	( const char* name );
	GT_PLAYEFFECT,					// void	( int effect, vec3_t origin, vec3_t angles );

} gametypeImport_t;


typedef enum
{
	GAMETYPE_INIT,
	GAMETYPE_START,
	GAMETYPE_RUN_FRAME,
	GAMETYPE_EVENT,

} gametypeExport_t;


typedef enum
{
	GTCMD_TEXTMESSAGE,				// void ( int client, const char* message );
	GTCMD_RESETITEM,				// void ( const char* itemName );
	GTCMD_GETCLIENTNAME,			// void ( int clientid, char* buffer, int buffersize );
									
	GTCMD_REGISTERGLOBALSOUND,		// int  ( const char* soundFile );
	GTCMD_STARTGLOBALSOUND,			// void ( int soundid );
									
	GTCMD_REGISTERITEM,				// int  ( const char* name, gtItemDef_t* def );

	GTCMD_RADIOMESSAGE,				// void ( int clientid, const char* message );
	GTCMD_REGISTERTRIGGER,			// bool ( int triggerid, const char* message, gtTriggerDef_t* def );

	GTCMD_DOESCLIENTHAVEITEM,		// bool ( int clientid, int itemid );

	GTCMD_ADDTEAMSCORE,				// void ( team_t team, int score );
	GTCMD_ADDCLIENTSCORE,			// void ( int clientid, int score );

	GTCMD_RESTART,					// void ( int delay );

	GTCMD_REGISTEREFFECT,			// int	( const char* name );
	GTCMD_PLAYEFFECT,				// void ( int effect, vec3_t origin, vec3_t angles );

} gametypeCommand_t;



typedef enum
{
	GTEV_ITEM_DROPPED,			// void ( int itemID );
	GTEV_ITEM_TOUCHED,			// int  ( int itemID, int clientID, int clientTeam );

	GTEV_TRIGGER_TOUCHED,		// int  ( int trigID, int clientID, int clientTeam );

	GTEV_TEAM_ELIMINATED,		// void ( team_t team );
	GTEV_TIME_EXPIRED,			// void ( void );
	
	GTEV_ITEM_STUCK,			// void ( int itemID );

	GTEV_ITEM_DEFEND,			// void ( int itemID, int clientID, int clientTeam );

	GTEV_CLIENT_DEATH,			// void ( int clientID, int clientTeam, int killerID, int killerTeam );

	GTEV_MAX,
	
	GTEV_PAUSE					// Boe!Man 4/22/12: Send this when the game's paused. Only CTF uses this at the moment.

} gametypeEvent_t;


typedef struct gtItemDef_s
{
	qboolean	use;			// whether or not the item needs to be used
	int			useTime;		// If the item needs to be used, this is the time it takes to use it

} gtItemDef_t;

typedef struct gtTriggerDef_s
{
	qboolean	use;			// Whether or not the trigger needs to be used
	int			useTime;		// If the trigger needs to be used, this is the time it takes to use it

} gtTriggerDef_t;

