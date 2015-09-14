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

// extern   vmCvar_t    g_banIPs;
// extern   vmCvar_t    g_filterBan;


typedef struct ipFilter_s
{
    unsigned    mask;
    unsigned    compare;
} ipFilter_t;

#define MAX_IPFILTERS   1024

static ipFilter_t   ipFilters[MAX_IPFILTERS];
static int          numIPFilters;

char    *ConcatArgs( int start );

/*
=================
StringToFilter
=================
*/
/*
static qboolean StringToFilter (char *s, ipFilter_t *f)
{
    char    num[128];
    int     i, j;
    byte    b[4];
    byte    m[4];

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
*/

/*
=================
UpdateIPBans
=================

static void UpdateIPBans (void)
{
    byte    b[4];
    int     i;
    char    iplist[MAX_INFO_STRING];

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
    int     i;
    unsigned    in;
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
            return (qboolean)(g_filterBan.integer != 0);

    return (qboolean)(g_filterBan.integer == 0);
}

/*
=================
AddIP
=================

static void AddIP( char *str )
{
    int     i;

    for (i = 0 ; i < numIPFilters ; i++)
    {
        if (ipFilters[i].compare == 0xffffffff)
        {
            break;      // free spot
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
    char        str[MAX_TOKEN_CHARS];

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
Initial recode by Boe!Man - 10/31/11
SQLite backend update by Boe!Man - 2/26/13
Additions of Admin/Clans and partial recode by Boe!Man - 9/26/13

Usage: addip <list> <ip-subnet> <name> <reason>

Where list is:
    subnetbanlist, banlist, adminlist, passlist or clanlist.
=================
*/
void Svcmd_AddIP_f (void)
{
    char        arg[64];
    // General info.
    char        ip[MAX_IP];
    char        name[64];
    char        reason[64];
    // Boe!Man 2/26/13: Add SQLite support.
    sqlite3     *db;
    int         admLevel = 0;

    if ( trap_Argc() < 4 ) {
        Com_Printf("Usage:  addip <list> <IP/subnet> <clientname> <opt:reason/req:level>\n");
        return;
    }
    
    trap_Argv(1, arg, sizeof(arg));
    
    // Fetch the IP and store it.
    if(strstr(arg, "subnetban")){
        
        trap_Argv(2, arg, sizeof(arg));
        if(strlen(arg) <= 7){
            Q_strncpyz(ip, arg, sizeof(arg));
        }else if(strlen(arg) > 7){
            Q_strncpyz(ip, arg, 7);
        }
    }else{ // Another list, so store the full IP..
        trap_Argv(2, arg, sizeof(arg));
        Q_strncpyz(ip, arg, sizeof(ip));
    }
    Boe_convertNonSQLChars(ip);
    
    // Fetch the client name.
    trap_Argv(3, arg, sizeof(arg));
    Q_strncpyz(name, arg, sizeof(name));
    Boe_convertNonSQLChars(name);
    
    // Fetch the reason, if given.
    trap_Argv(4, arg, sizeof(arg));
    if(strlen(arg) > 0){
        Q_strncpyz(reason, arg, sizeof(reason));
        Boe_convertNonSQLChars(reason);
    }
    
    trap_Argv(1, arg, sizeof(arg));
    if(strstr(arg, "admin") || strstr(arg, "pass")){
        admLevel = atoi(reason);
        if(admLevel < 2 || admLevel > 4){
            Com_Printf("^1Error: ^7Invalid admin level: %i.\n", admLevel);
            return;
        }
    }
    
    // Boe!Man 9/26/13: We're going to add it to the database now.
    if(strstr(arg, "banlist")){
        db = bansDb;
        
        if(strstr(arg, "sub")){
            Q_strncpyz(arg, "subnetbans", sizeof(arg));
        }else{
            Q_strncpyz(arg, "bans", sizeof(arg));
        }
        
        if(sqlite3_exec(db, va("INSERT INTO %s (IP, name, by, reason) values ('%s', '%s', 'RCON', '%s')", arg, ip, name, reason), 0, 0, 0) != SQLITE_OK){
            Com_Printf("^1Error: ^7bans database: %s\n", sqlite3_errmsg(db));
        }else{
            Com_Printf(va("Success adding [%s] to the %s (name: %s, reason: %s).\n", ip, strstr(arg, "sub") ? "subnetbanlist" : "banlist", name, reason));
        }
    }else if(strstr(arg, "admin") || strstr(arg, "pass")){
        db = usersDb;
        
        if(strstr(arg, "pass")){
            Q_strncpyz(arg, "passadmins", sizeof(arg));
        }else{
            Q_strncpyz(arg, "admins", sizeof(arg));
        }
        
        if(sqlite3_exec(db, va("INSERT INTO %s (%sname, by, level) values (%s'%s', 'RCON', '%i')", arg, strstr(arg, "pass") ? "" : "ip, ", strstr(arg, "pass") ? "" : va("'%s', ", ip), name, admLevel), 0, 0, 0) != SQLITE_OK){
            Com_Printf("^1Error: ^7users database: %s\n", sqlite3_errmsg(db));
        }else{
            Com_Printf(va("Success adding [%s] to the %s (%slevel: %s).\n", ip, strstr(arg, "pass") ? "passlist" : "adminlist", va("name: %s, ", name), reason));
        }
    }else if(strstr(arg, "clan")){
        db = usersDb;
        
        if(sqlite3_exec(db, va("INSERT INTO clanmembers (IP, name, by) values ('%s', '%s', 'RCON')", ip, name), 0, 0, 0) != SQLITE_OK){
            Com_Printf("^1Error: ^7users database: %s\n", sqlite3_errmsg(db));
        }else{
            Com_Printf(va("Success adding [%s] to the clanlist (name: %s).\n", ip, name));
        }
    }else{
        Com_Printf("^3Info: ^7Invalid choice: %s. Valid choices are: subnetbanlist, banlist, adminlist, passlist, clanlist.\n", arg);   
    }
}

/*
=================
Svcmd_RemoveIP_f
=================

void Svcmd_RemoveIP_f (void)
{
    ipFilter_t  f;
    int         i;
    char        str[MAX_TOKEN_CHARS];

    if ( trap_Argc() < 2 ) {
        Com_Printf("Usage:  sv removeip <ip-mask>\n");
        return;
    }

    trap_Argv( 1, str, sizeof( str ) );

    if (!StringToFilter (str, &f))
        return;

    for (i=0 ; i<numIPFilters ; i++) {
        if (ipFilters[i].mask == f.mask &&
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
void    Svcmd_EntityList_f (void) {
    int         e;
    gentity_t       *check;

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
    int  time;

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

gclient_t   *ClientForString( const char *s ) {
    gclient_t   *cl;
    int         i;
    int         idnum;

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
// Boe!Man 4/15/13: Removed this old function - it's not used anymore.
===================
Svcmd_ForceTeam_f

forceteam <player> <team>
===================

void Svcmd_ForceTeam_f( void )
{
    gclient_t   *cl;
    char        str[MAX_TOKEN_CHARS];

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
*/

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
