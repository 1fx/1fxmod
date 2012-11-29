// Copyright (C) 2001-2002 Raven Software
//

// this file holds commands that can be executed by the server console, but not remote clients

#include "g_local.h"
#include "boe_local.h"

/*
==============================================================================

PACKET FILTERING


You can add or remove addresses from the filter list with:

addip <ip>
removeip <ip>

The ip address is specified in dot format, and any unspecified digits will match any value, so you can specify an entire class C network with "addip 192.246.40".

Removeip will only remove an address specified exactly the same way.  You cannot addip a subnet, then removeip a single host.

listip
Prints the current list of filters.

g_filterban <0 or 1>

If 1 (the default), then ip addresses matching the current list will be prohibited from entering the game.  This is the default setting.

If 0, then only addresses matching the list will be allowed.  This lets you easily set up a private game, or a game that only allows players from your local network.


==============================================================================
*/

// extern	vmCvar_t	g_banIPs;
// extern	vmCvar_t	g_filterBan;


typedef struct ipFilter_s
{
	unsigned	mask;
	unsigned	compare;
} ipFilter_t;

#define	MAX_IPFILTERS	1024

static ipFilter_t	ipFilters[MAX_IPFILTERS];
static int			numIPFilters;

char	*ConcatArgs( int start );

/*
=================
StringToFilter
=================
*/
static qboolean StringToFilter (char *s, ipFilter_t *f)
{
	char	num[128];
	int		i, j;
	byte	b[4];
	byte	m[4];

	for (i=0 ; i<4 ; i++)
	{
		b[i] = 0;
		m[i] = 0;
	}

	for (i=0 ; i<4 ; i++)
	{
		if (*s < '0' || *s > '9')
		{
			Com_Printf( "Bad filter address: %s\n", s );
			return qfalse;
		}

		j = 0;
		while (*s >= '0' && *s <= '9')
		{
			num[j++] = *s++;
		}
		num[j] = 0;
		b[i] = atoi(num);
		if (b[i] != 0)
			m[i] = 255;

		if (!*s)
			break;
		s++;
	}

	f->mask = *(unsigned *)m;
	f->compare = *(unsigned *)b;

	return qtrue;
}

/*
=================
UpdateIPBans
=================

static void UpdateIPBans (void)
{
	byte	b[4];
	int		i;
	char	iplist[MAX_INFO_STRING];

	*iplist = 0;
	for (i = 0 ; i < numIPFilters ; i++)
	{
		if (ipFilters[i].compare == 0xffffffff)
			continue;

		*(unsigned *)b = ipFilters[i].compare;
		Com_sprintf( iplist + strlen(iplist), sizeof(iplist) - strlen(iplist),
			"%i.%i.%i.%i ", b[0], b[1], b[2], b[3]);
	}

	trap_Cvar_Set( "g_banIPs", iplist );
}
*/

/*
=================
G_FilterPacket
=================
*/
qboolean G_FilterPacket (char *from)
{
	int		i;
	unsigned	in;
	byte m[4];
	char *p;

	m[0] = m[1] = m[2] = m[3] = 0;

	i = 0;
	p = from;
	while (*p && i < 4) {
		while (*p >= '0' && *p <= '9') {
			m[i] = m[i]*10 + (*p - '0');
			p++;
		}
		if (!*p || *p == ':')
			break;
		i++, p++;
	}

	in = *(unsigned *)m;

	for (i=0 ; i<numIPFilters ; i++)
		if ( (in & ipFilters[i].mask) == ipFilters[i].compare)
			return g_filterBan.integer != 0;

	return g_filterBan.integer == 0;
}

/*
=================
AddIP
=================

static void AddIP( char *str )
{
	int		i;

	for (i = 0 ; i < numIPFilters ; i++)
	{
		if (ipFilters[i].compare == 0xffffffff)
		{
			break;		// free spot
		}
	}

	if (i == numIPFilters)
	{
		if (numIPFilters == MAX_IPFILTERS)
		{
			Com_Printf ("IP filter list is full\n");
			return;
		}
		numIPFilters++;
	}

	if (!StringToFilter (str, &ipFilters[i]))
	{
		ipFilters[i].compare = 0xffffffffu;
	}

	UpdateIPBans();
}
*/

/*
=================
G_ProcessIPBans
=================

void G_ProcessIPBans(void)
{
	char *s, *t;
	char		str[MAX_TOKEN_CHARS];

	Q_strncpyz( str, g_banIPs.string, sizeof(str) );

	for (t = s = g_banIPs.string; *t; /* */ /* ) { // extra comment start - boe 11/04/11
		s = strchr(s, ' ');
		if (!s)
			break;
		while (*s == ' ')
			*s++ = 0;
		if (*t)
			AddIP( t );
		t = s;
	}
}
*/

/*
=================
Svcmd_AddIP_f
Updated by Boe!Man - 10/31/11

Usage: addip <ip-subnet> <name> <reason>

arg 1: If three digits are found in the IP, and the length isn't > 15 or < 7, the IP is considered valid. If there are less dots found, it's considered a subnet (though len >= 7).
arg 2: No name specified will result in N/A.
arg 3: No reason specified will result in N/A.

By will always (!) be "RCON".
=================
*/
void Svcmd_AddIP_f (void)
{
	char		arg[64];
	char		banentry[512];
	int			i = 0;
	qboolean	subnet = qfalse;
	// General info.
	char		ip[MAX_IP];
	char		name[64];
	char		reason[64];
	// Locals for arg char loop.
	int			dots = 0;
	// Locals for arg reason loop.
	char *temp = "";
	

	if ( trap_Argc() < 2 ) {
		Com_Printf("Usage:  addip <IP/Subnet> <banned IP (client) name> <reason>\n");
		return;
	}

	trap_Argv( 1, arg, sizeof(arg)); // IP or Subnet.
	
	// Boe!Man 10/31/11: Basic routine check on arg.
	if(strlen(arg) < 7 || strlen(arg) > 16){
		Com_Printf("Error: IP or Subnet doesn't seem to be valid. Length should be 7-16.\n");
		return;
	}
	
	for(i = 0; i<strlen(arg);i++){
		if(!henk_isdigit(arg[i]) && arg[i] != 46){
			Com_Printf("Error: IP or Subnet can only contain digits and separators (dots).\n");
			return;
		}else if(arg[i] == 46){ // 46 = dot.
			dots++;
			if(dots > 3){
				Com_Printf("Error: Not a valid IP!\n");
				return;
			}
		}
	}
	
	// Boe!Man 10/31/11: Passed the check, now checking if we're dealing with a part of an IP (which we treat as subnet).
	if(dots < 3 && dots >= 1){ // Dealing with subnet.
		// Boe!Man 11/04/11: Minimum size should be 7 or else it'll be useless.
		if(strlen(arg) < 7){
			Com_Printf("Error: Minimum IP size for adding a subnet is 7.\n");
			return;
		}
		Q_strncpyz(ip, arg, 7); // Subnet only takes 7.
		subnet = qtrue;
	}else if(dots != 3){ // Huh? This definitely isn't a valid IP.
		Com_Printf("Error: Not a valid IP!\n");
		return;
	}else{ // Definitely an IP.
		Q_strncpyz(ip, arg, sizeof(ip));
	}
	
	// Boe!Man 10/31/11: Check if they supplied a name and reason, if not, fill them out with "N/A".
	memset(arg, 0, sizeof(arg)); // Clean buffer.
	trap_Argv( 2, arg, sizeof(arg)); // Name.
	if(strlen(arg) > 0){
		Q_strncpyz(name, arg, sizeof(name));
	}else{
		Q_strncpyz(name, "N/A", sizeof(name));
	}
	
	memset(arg, 0, sizeof(arg)); // Clean buffer.
	trap_Argv( 3, arg, sizeof(arg)); // Reason arg 0.
	
	// Boe!Man 11/05/11: Only go through with this if there's actually a reason, no need to waste useful resources.
	if(strlen(arg) > 0){
		temp = va("%s", arg);
		for(i=0;i<=25;i++){
			trap_Argv( 4+i, arg, sizeof(arg)); // Reason arg 0 + 1.
			if(strlen(arg) > 0){
				temp = va("%s %s", temp, arg); // we fill this array up with 25 arguments
			}
		}
	}
	
	if(strlen(temp) > 0){ // Boe!Man 11/05/11
		Q_strncpyz(reason, temp, sizeof(reason));
	}else{
		Q_strncpyz(reason, "N/A", sizeof(reason));
	}
	
	// Boe!Man 10/31/11: Write to buffer with sprintf.
	Com_sprintf (banentry, sizeof(banentry), "%s\\%s//%s||%s", ip, name, "RCON", reason);
	
	// Boe!Man 10/31/11: We're going to add it to the list now.
	if(!subnet){ // Regular ban.
		if(Boe_AddToList(banentry, g_banfile.string, "Ban", NULL)){
			Com_Printf(va("Success adding [%s] to the banlist (name: %s, reason: %s).\n", ip, name, reason));
		}else{
			Com_Printf(va("Couldn't open file for writing while trying to add %s.\n", ip));
		}
	}else{
		if(Boe_AddToList(banentry, "users/subnetbans.txt", "Subnet ban", NULL)){
			Com_Printf(va("Success adding [%s] to the subnetbanlist (name: %s, reason: %s).\n", ip, name, reason));
		}else{
			Com_Printf(va("Couldn't open file for writing while trying to add %s.\n", ip));
		}
	}
	
	return;
//	AddIP( str );

}

/*
=================
Svcmd_RemoveIP_f
=================

void Svcmd_RemoveIP_f (void)
{
	ipFilter_t	f;
	int			i;
	char		str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 2 ) {
		Com_Printf("Usage:  sv removeip <ip-mask>\n");
		return;
	}

	trap_Argv( 1, str, sizeof( str ) );

	if (!StringToFilter (str, &f))
		return;

	for (i=0 ; i<numIPFilters ; i++) {
		if (ipFilters[i].mask == f.mask	&&
			ipFilters[i].compare == f.compare) {
			ipFilters[i].compare = 0xffffffffu;
			Com_Printf ("Removed.\n");

			UpdateIPBans();
			return;
		}
	}

	Com_Printf ( "Didn't find %s.\n", str );
}
*/

/*
===================
Svcmd_EntityList_f
===================
*/
void	Svcmd_EntityList_f (void) {
	int			e;
	gentity_t		*check;

	check = g_entities+1;
	for (e = 1; e < level.num_entities ; e++, check++) {
		if ( !check->inuse ) {
			continue;
		}
		Com_Printf("%3i:", e);
		switch ( check->s.eType ) {
		case ET_GENERAL:
			Com_Printf("ET_GENERAL          ");
			break;
		case ET_PLAYER:
			Com_Printf("ET_PLAYER           ");
			break;
		case ET_ITEM:
			Com_Printf("ET_ITEM             ");
			break;
		case ET_MISSILE:
			Com_Printf("ET_MISSILE          ");
			break;
		case ET_MOVER:
			Com_Printf("ET_MOVER            ");
			break;
		case ET_BEAM:
			Com_Printf("ET_BEAM             ");
			break;
		case ET_PORTAL:
			Com_Printf("ET_PORTAL           ");
			break;
		case ET_SPEAKER:
			Com_Printf("ET_SPEAKER          ");
			break;
		case ET_PUSH_TRIGGER:
			Com_Printf("ET_PUSH_TRIGGER     ");
			break;
		case ET_TELEPORT_TRIGGER:
			Com_Printf("ET_TELEPORT_TRIGGER ");
			break;
		case ET_INVISIBLE:
			Com_Printf("ET_INVISIBLE        ");
			break;
		case ET_GRAPPLE:
			Com_Printf("ET_GRAPPLE          ");
			break;
		default:
			Com_Printf("%3i                 ", check->s.eType);
			break;
		}

		if ( check->classname ) {
			Com_Printf("%s", check->classname);
		}
		Com_Printf("\n");
	}
}


void Svcmd_ExtendTime_f (void)
{
	char str[MAX_TOKEN_CHARS];
	int	 time;

	if ( trap_Argc() < 2 )
	{
		Com_Printf("Usage:  extendtime <minutes>\n");
		return;
	}

	trap_Argv( 1, str, sizeof( str ) );

	time = atoi(str);
	level.timeExtension += time;

	G_LogPrintf ( "timelimit extended by %d minutes\n", time );

	trap_SendServerCommand( -1, va("print \"timelimit extended by %d minutes\n\"", time) );
}

gclient_t	*ClientForString( const char *s ) {
	gclient_t	*cl;
	int			i;
	int			idnum;

	// numeric values are just slot numbers
	if ( s[0] >= '0' && s[0] <= '9' ) {
		idnum = atoi( s );
		if ( idnum < 0 || idnum >= level.maxclients ) {
			Com_Printf( "Bad client slot: %i\n", idnum );
			return NULL;
		}

		cl = &level.clients[idnum];
		if ( cl->pers.connected == CON_DISCONNECTED ) {
			Com_Printf( "Client %i is not connected\n", idnum );
			return NULL;
		}
		return cl;
	}

	// check for a name match
	for ( i=0 ; i < level.maxclients ; i++ ) {
		cl = &level.clients[i];
		if ( cl->pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( !Q_stricmp( cl->pers.netname, s ) ) {
			return cl;
		}
	}

	Com_Printf( "User %s is not on the server\n", s );

	return NULL;
}

/*
===================
Svcmd_ForceTeam_f

forceteam <player> <team>
===================
*/
void Svcmd_ForceTeam_f( void )
{
	gclient_t	*cl;
	char		str[MAX_TOKEN_CHARS];

	// find the player
	trap_Argv( 1, str, sizeof( str ) );
	cl = ClientForString( str );
	if ( !cl )
	{
		return;
	}

	// set the team
	trap_Argv( 2, str, sizeof( str ) );
	SetTeam( &g_entities[cl - level.clients], str, NULL, qfalse );
}

/*
===================
Svcmd_CancelVote_f

cancels the vote in progress
===================

void Svcmd_CancelVote_f ( void )
{
	level.voteTime = 0;

	trap_SetConfigstring( CS_VOTE_TIME, "" );

	trap_SendServerCommand( -1, "print \"Vote cancelled by admin.\n\"" );
}
*/
