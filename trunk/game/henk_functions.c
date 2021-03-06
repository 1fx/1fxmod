/*
===========================================================================
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

#include "g_local.h"
#include "boe_local.h"
#ifdef __linux__
// Boe!Man 6/26/13: LinuxThreads implementation. I know, headers are always bad in seperate files, but we want to avoid declaring syscall twice..
#include <unistd.h>
#endif

// Country memory database
pthread_t           countryInit; // Boe!Man 6/25/13: The reference to the thread.
sqlite3             *countryDb;

int process_ddl_row(void * pData, int nColumns,
        char **values, char **columns)
{
        sqlite3     *db;

        if (nColumns != 1)
                return 1; // Error

        db = (sqlite3*)pData;
        sqlite3_exec(db, values[0], NULL, NULL, NULL);

        return 0;
}

int process_dml_row(void *pData, int nColumns,
        char **values, char **columns)
{
        sqlite3     *db;
        char *stmt;

        if (nColumns != 1)
                return 1; // Error

        db = (sqlite3*)pData;

        stmt = sqlite3_mprintf("insert into main.%q "
                "select * from country.%q", values[0], values[0]);
        sqlite3_exec(db, stmt, NULL, NULL, NULL);
        sqlite3_free(stmt);

        return 0;
}

qboolean G_isCountryDatabaseInitialized()
{
    qboolean result;
    int i;

    pthread_mutex_lock(&level.countryInitLock);
    result = level.countryInitialized;
    pthread_mutex_unlock(&level.countryInitLock);

    if(!level.countryPostProcessed && result){
        // Boe!Man 6/25/13: Parse the clients their country now.
        for (i = 0; i < level.numConnectedClients; i++){
            gentity_t* ent = &g_entities[level.sortedClients[i]];

            if (ent->client->pers.connected != CON_CONNECTED)
                continue;

            if (ent->r.svFlags & SVF_BOT)
                continue;

            HENK_COUNTRY(ent);
        }

        level.countryPostProcessed = qtrue;
    }

    return result;
}

void *Thread_countryInit(){
    int         rc, i;
    sqlite3     *db;
    char        fsGame[MAX_QPATH];
    qboolean    alt;

    // Boe!Man 12/6/12
    // The file can be on two locations. The DLL should always be in the fs_game folder, however, this could be misconfigured.
    // The Mod takes care of this problem and should load the file correctly, even if misplaced.
    rc = sqlite3_open_v2("./core/country.db", &db, SQLITE_OPEN_READONLY, NULL); // Boe!Man 12/5/12: *_v2 can make sure an empty database is NOT created. After all, the country db is READ ONLY.
    if(rc){
        trap_Cvar_VariableStringBuffer("fs_game", fsGame, sizeof(fsGame));
        rc = sqlite3_open_v2(va("./%s/core/country.db", fsGame), &db, SQLITE_OPEN_READONLY, NULL);
        if(rc){
            G_LogPrintf("^1Error: ^7Country database: %s\n", sqlite3_errmsg(db));
            return NULL;
        }else{
            alt = qtrue;
        }
    }else{
        alt = qfalse;
    }

    sqlite3_exec(db, "BEGIN", NULL, NULL, NULL);
    sqlite3_exec(db, "SELECT sql FROM sqlite_master WHERE sql NOT NULL", &process_ddl_row, countryDb, NULL);
    sqlite3_exec(db, "COMMIT", NULL, NULL, NULL);
    sqlite3_close(db);

    // Boe!Man 1/29/13: Do attach the proper database, based on where the library is located.
    if(!alt){
        sqlite3_exec(countryDb, "ATTACH DATABASE './core/country.db' as country", NULL, NULL, NULL);
    }else{
        sqlite3_exec(countryDb, va("ATTACH DATABASE './%s/core/country.db' as country", fsGame), NULL, NULL, NULL);
    }

    // Copy the data from the backup to the in memory
    sqlite3_exec(countryDb, "BEGIN", NULL, NULL, NULL);
    sqlite3_exec(countryDb, "SELECT name FROM country.sqlite_master WHERE type='table'", &process_dml_row, countryDb, NULL);
    sqlite3_exec(countryDb, "COMMIT", NULL, NULL, NULL);

    // Boe!Man 6/25/13: The game can use the country system now..
    pthread_mutex_lock(&level.countryInitLock);
    level.countryInitialized = qtrue;
    pthread_mutex_unlock(&level.countryInitLock);
}

//Henk 12/10/12 -> Copy country database to an in memory database.
// Select query from disk takes ~80ms, from memory 0ms.
void LoadCountries(){
    // Boe!Man 6/25/13: Initialize the in-memory database from the main thread.
    countryDb = NULL;

    sqlite3_open_v2(":memory:", &countryDb, SQLITE_OPEN_READWRITE, NULL);

    // Boe!Man 6/25/13: Try to init the thread.
    if(pthread_create(&countryInit, NULL, &Thread_countryInit, NULL) != 0){
        #ifdef _DEBUG
        Com_Error(ERR_FATAL_NOLOG, "Couldn't create Country initialization thread.");
        #else
        G_LogPrintf("Couldn't ininitalize Country database due to thread creation failing..\n"); // Don't throw a fatal error out of debug mode.
        trap_Cvar_Set("g_checkCountry", "0");
        trap_Cvar_Update(&g_checkCountry);
        G_ShutdownGame(1); // Restart the game module.
        #endif
    }
}

void UnloadCountries(){
    pthread_join(countryInit, NULL); // Boe!Man 6/25/13: Wait for the thread to exit.

    sqlite3_exec(countryDb, "DETACH DATABASE country", NULL, NULL, NULL);
    sqlite3_close(countryDb);

    Com_Printf("Unloaded country database.\n");
}

void trap_LinkEntity( gentity_t *ent ) {
    if(ent == NULL){
        G_LogPrintf("!=!=!=!=!=!=!=!=WARNING=!=!=!=!=!=!=!=!=!=! Trying to free a NULL ent, report at 1fxmod.org(Debug -> link)");
        return;
    }
    trap_LinkEntity1(ent);
}

void trap_UnlinkEntity( gentity_t *ent ) {
    if(ent == NULL){
        G_LogPrintf("!=!=!=!=!=!=!=!=WARNING=!=!=!=!=!=!=!=!=!=! Trying to free a NULL ent, report at 1fxmod.org(Debug -> unlink)");
        return;
    }
    trap_UnlinkEntity1(ent);
}

// sort of sscanf found here
// http://cboard.cprogramming.com/c-programming/18092-sscanf-source-code.html
int (readFromStr)( char *src, char *format, ... )
{
  va_list ap;
  float *f;
  int conv = 0, *i, index;
  char *a, *fp, *sp = src, buf[512] = {'\0'};

  va_start ( ap, format );
  for ( fp = format; *fp != '\0'; fp++ ) {
    for ( index = 0; *sp != '\0' && *sp != ' '; index++ )
      buf[index] = *sp++;
    while ( *sp == ' ' ) sp++;
    while ( *fp != '%' ) fp++;
    if ( *fp == '%' ) {
      switch ( *++fp ) {
      case 'i':
        i = va_arg ( ap, int * );
        *i = atoi ( buf );
        break;
      case 'f':
        f = va_arg ( ap, float * );
        *f = (float)atof ( buf );
        break;
      case 's':
        a = va_arg ( ap, char * );
        strncpy ( a, buf, strlen ( buf ) + 1 );
        break;
      }
      conv++;
    }
  }
  va_end ( ap );
  return conv;
}

void Henk_Tip(void){
    char buf[1024];
    struct test{
        char tip[128]; // Boe!Man 6/13/11: A size of 128 for a tip should be sufficient.
    } Tips[64];
    fileHandle_t f;
    int len, i, start, count;
    #ifdef WIN32
    // Boe!Man 9/19/13
    // The horror with Windows.. Sigh
    // If SoF2 on Windows finds a CRLF it is ALWAYS a simple \n rather than a \r\n. This IS NOT the case in Linux, it neatly detects this behaviour.
    // SoF2 however, obviously, messes this up. The file content length INCLUDES the \r, even though the buffer does not. They are appended at the end.
    // This boolean is used as the result of a rather nasty hack to detect if CRLF is actually present in the file or not.
    qboolean CRLF = qfalse;
    int linecount = 0;
    #endif

    len = trap_FS_FOpenFile( g_tipsFile.string, &f, FS_READ_TEXT);
    if (!f) {
        Com_Printf("Error opening tips file: %s\n", g_tipsFile.string);
        // Boe!Man 9/18/13: Fixed tips re-checking every x msec even though the file couldn't be opened due to errors.
        trap_Cvar_Set("server_enableTips", "0");
        trap_Cvar_Update(&server_enableTips);
        Com_Printf("Set server_enableTips to 0 due to error opening file.\n");
        //level.tipMsg = level.time+(server_msgInterval.integer*60000); // Boe!Man 6/17/11: If the tips file isn't found, do update the interval so it doesn't attempt to open the file every x msecs.
        return;
    }
    memset( buf, 0, sizeof(buf) );
    trap_FS_Read( buf, len, f );
    start = 0;
    count = 0;
    for(i=0;i<len;i++){
        if(buf[i] == '\n'){
            #ifdef WIN32
            if(i-start > 0){
            #elif defined(__linux__) || defined(MACOS_X)
            if(buf[i-1] == '\r' && i-start > 1 || buf[i-1] != '\r' && i-start > 0){
            #endif
                Q_strncpyz(Tips[count].tip, buf+start, i-start+1);
                count++;
            }
            start = i+1;

            #ifdef WIN32
            linecount++;
            #endif

            // Boe!Man 9/18/13: If there are already 64 tips loaded, break the process.
            if(count == 64)
                break;
        }
    }
    // Boe!Man 9/18/13: Fetch the remaining line.
    #ifdef WIN32
    // Boe!Man 9/19/13: Check for the CRLF on Windows.
    if(len >= 2 && buf[len-1] == 0){
        CRLF = qtrue;
    }

    if(CRLF ? (len > start+linecount && count != 64 && start != len) : (len > start && count != 64 && start != len)){
    #elif defined(__linux__) || defined(MACOS_X)
    if(len > start && count != 64){
    #endif
        if(!count){
            Q_strncpyz(Tips[count].tip, buf, sizeof(buf));
        }else{
            Q_strncpyz(Tips[count].tip, buf+start, len);
        }
        count++;
    }
    trap_FS_FCloseFile(f);

    if(count > 0){
        trap_SendServerCommand( -1, va("chat -1 \"%s Tip: %s\n\"", G_ColorizeMessage("Random"), Tips[irand(0, count-1)].tip ) );
        level.tipMsg = level.time+(server_msgInterval.integer*60000);
    }else{
        trap_Cvar_Set("server_enableTips", "0");
        trap_Cvar_Update(&server_enableTips);
        Com_Printf("Set server_enableTips to 0 due to no tips being present in file: %s.\n", g_tipsFile.string);
    }
    memset(Tips, 0, sizeof(Tips));
}

void Henk_Ignore(gentity_t *ent){
    char arg1[32];
    int i, numberofclients, idnum = -1, z, temparray[33], count = 0;
    char string[1024] = "\0", string1[64] = "\0";
    char cleanName[MAX_NETNAME];

    trap_Argv( 1, arg1, sizeof( arg1 ) );
    if(!arg1[0]){
        G_printInfoMessage(ent, "You haven't entered a valid player ID/player name.");
        return;
    }
    if(!henk_ischar(arg1[0])){
        idnum = atoi(arg1);
    }else if(henk_ischar(arg1[0])){
            memset(string, 0, sizeof(string));
            memset(string1, 0, sizeof(string1));
            numberofclients = 0;
            for(i=0;i<level.numConnectedClients;i++){
                Q_strncpyz(cleanName, g_entities[level.sortedClients[i]].client->pers.cleanName, sizeof(cleanName));
                if(strstr(Q_strlwr(cleanName), Q_strlwr(arg1))){
                    idnum = level.sortedClients[i];
                    numberofclients += 1;
                    Com_sprintf(string1, sizeof(string1), "^1[#%i] ^7%s, ",  idnum, g_entities[level.sortedClients[i]].client->pers.cleanName);
                    Q_strncpyz(string+strlen(string), string1, strlen(string1)+1);
                }
            }
            string[strlen(string)-2] = '\0';
            if(numberofclients > 1){
                G_printInfoMessage(ent, "Multiple names found with ^3%s^7: %s", arg1, string);
                return;
            }else if(numberofclients == 0)
                idnum = -1;
        }else
            idnum = -1;
        if ( idnum < 0 || idnum >= g_maxclients.integer )
        {
            G_printInfoMessage(ent, "You haven't entered a valid player ID/player name.");
            return;
        }

        if ( g_entities[idnum].client->pers.connected == CON_DISCONNECTED )
        {
            G_printInfoMessage(ent, "The player is not connected.");
            return;
        }
        if( !ent->client->sess.admin && g_entities[idnum].client->sess.admin >= 2 ){
            G_printInfoMessage(ent, "You can't ignore an Admin.");
            return;
        }
        // Check if he's already ignored then unignore him
        for(i=0;i<g_entities[idnum].client->sess.IgnoredClientCount;i++){
            if(g_entities[idnum].client->sess.IgnoredClients[i] == ent->s.number){
                g_entities[idnum].client->sess.IgnoredClients[i] = -1;
                // now sort the array again.
                for(z=0;z<g_entities[idnum].client->sess.IgnoredClientCount;z++){
                    if(g_entities[idnum].client->sess.IgnoredClients[z] != -1){
                        temparray[count] = g_entities[idnum].client->sess.IgnoredClients[z];
                        count++;
                    }
                }
                // temp array to original
                for(z=0;z<g_entities[idnum].client->sess.IgnoredClientCount;z++){
                    g_entities[idnum].client->sess.IgnoredClients[z] = temparray[z];
                }
                g_entities[idnum].client->sess.IgnoredClientCount--;
                G_printInfoMessage(ent, "You are not ignoring %s anymore.", g_entities[idnum].client->pers.cleanName);
                return;
            }
        }
        if(g_entities[idnum].client->sess.IgnoredClientCount >= 32){
            G_printInfoMessage(ent, "You can't ignore more than 32 players.");
            return;
        }
        g_entities[idnum].client->sess.IgnoredClients[g_entities[idnum].client->sess.IgnoredClientCount] = ent->s.number;
        g_entities[idnum].client->sess.IgnoredClientCount++;
        G_printInfoMessage(&g_entities[idnum], "%s has ignored you.", ent->client->pers.cleanName);
        G_printInfoMessage(ent, "You have ignored %s.", g_entities[idnum].client->pers.cleanName);
}

int TiedPlayers(void){
    gentity_t *ent;
    int i, highscore = 0, count = 0;
    int winners[MAX_CLIENTS];

    for(i=0;i<level.numConnectedClients;i++){
        ent = &g_entities[level.sortedClients[i]];
        if(ent->client->sess.team == TEAM_RED){
            if(ent->client->sess.kills == highscore){
                winners[count] = ent->s.number;
                count += 1;
                //highscore = ent->client->sess.kills; // Boe - useless.
            }else if(ent->client->sess.kills > highscore){
                winners[0] = ent->s.number;
                count = 1;
                highscore = ent->client->sess.kills;
            }
        }
    }

    if(count >= 2){ // Boe!Man 8/30/11: If there are tied players.
        for(i = 0; i < count; i++){
            g_entities[winners[i]].client->sess.cageFighter = qtrue;
        }
    }

    level.mapHighScore = highscore; // Boe!Man 8/30/11: Set the highest score (to make sure everybody gets in the cage if all have 0 points).
    return count;
}

void InitCagefight(void){
    int i, count = 0;

    vec3_t spawns[32];
    if(level.time < level.cagefighttimer){
        return;
    }
    level.startcage = qfalse;

    G_ResetGametype(qfalse, qtrue);
    for(i=0;i<=31;i++){ // Boe!Man 6/30/12: 32, regardless of the amount of players. The recommended max for any gametype is 32, above, unsupported. So just spread them more equally instead of adding even more spawns.
        VectorCopy(level.hideseek_cage, spawns[i]);
    }

    if(!level.hideseek_cageSize){ // 0 = regular.
        spawns[0][0] -= 105;
        spawns[0][1] -= 105;
        spawns[1][0] += 105;
        spawns[1][1] += 105;
        spawns[2][0] -= 105;
        spawns[2][1] += 105;
        spawns[3][0] += 105;
        spawns[3][1] -= 105;
        spawns[4][0] += 105; // fifth spawn
        //spawns[4][1] -= 0;
        spawns[5][0] -= 105;
        //spawns[5][1] += 0;
        //spawns[6][0] -= 0;
        spawns[6][1] += 105;
        //spawns[7][0] += 0;
        spawns[7][1] -= 105;
        spawns[8][0] += 105; // ninth spawn
        spawns[8][1] -= 50;
        spawns[9][0] -= 105;
        spawns[9][1] += 50;
        spawns[10][0] -= 50;
        spawns[10][1] += 105;
        spawns[11][0] += 50;
        spawns[11][1] -= 105;
        spawns[12][0] -= 50;
        spawns[12][1] -= 105;
        spawns[13][0] += 50;
        spawns[13][1] += 105;
        spawns[14][0] -= 105;
        spawns[14][1] -= 50;
        spawns[15][0] += 105;
        spawns[15][1] += 50;
        spawns[16][0] += 50;
        spawns[16][1] -= 50;
        spawns[17][0] += 50;
        spawns[17][1] += 50;
        spawns[18][0] -= 50;
        spawns[18][1] += 50;
        spawns[19][0] -= 50;
        spawns[19][1] -= 50;

        //spawns[20][0] += 0;
        spawns[20][1] -= 50;
        spawns[21][0] += 50;
        //spawns[21][1] += 0;
        //spawns[22][0] -= 0;
        spawns[22][1] += 50;
        spawns[23][0] -= 50;
        //spawns[23][1] -= 0;
        //spawns[24][0] -= 0;
        //spawns[24][1] -= 0;
    }else if(level.hideseek_cageSize == 2){ // Boe!Man 6/30/12: 2 = Big cage. So, add big cage spawnpoints.
        // The absolute corners.
        spawns[0][0] -= 231;
        spawns[0][1] -= 231;
        spawns[1][0] += 231;
        spawns[1][1] += 231;
        spawns[2][0] -= 231;
        spawns[2][1] += 231;
        spawns[3][0] += 231;
        spawns[3][1] -= 231;

        // The center between the two cages, still on the 'outer' line.
        //spawns[4][0] -= 0;
        spawns[4][1] -= 231;
        //spawns[5][0] += 0;
        spawns[5][1] += 231;
        spawns[6][0] -= 231;
        //spawns[6][1] -= 0;
        spawns[7][0] += 231;
        //spawns[7][1] += 0;

        // The quarter between the cage, again, on the outer line.
        spawns[8][0] -= 115.5;
        spawns[8][1] -= 231;
        spawns[9][0] += 115.5;
        spawns[9][1] += 231;
        spawns[10][0] -= 231;
        spawns[10][1] -= 115.5;
        spawns[11][0] += 231;
        spawns[11][1] += 115.5;

        // The same, except the opposite quarter.
        spawns[12][0] += 115.5;
        spawns[12][1] -= 231;
        spawns[13][0] -= 115.5;
        spawns[13][1] += 231;
        spawns[14][0] -= 231;
        spawns[14][1] += 115.5;
        spawns[15][0] += 231;
        spawns[15][1] -= 115.5;

        // Now we begin spawning them in the 'inner cage', the original spawnpoints that we maintain.
        spawns[16][0] -= 105;
        spawns[16][1] -= 105;
        spawns[17][0] += 105;
        spawns[17][1] += 105;
        spawns[18][0] -= 105;
        spawns[18][1] += 105;
        spawns[19][0] += 105;
        spawns[19][1] -= 105;

        // Center of the inner cage.
        //spawns[20][0] -= 0;
        spawns[20][1] -= 105;
        //spawns[21][0] += 0;
        spawns[21][1] += 105;
        spawns[22][0] -= 105;
        //spawns[22][1] += 0;
        spawns[23][0] += 105;
        //spawns[23][1] -= 0;

        // Spawn one fella in the absolute center.
        //spawns[24][0] -= 0;
        //spawns[24][1] -= 0;

        // Have the extra ability to spawn four more, do this between the absolutes of the inner cage and the outer cage ( (231 - 105) / 2 + 105)
        spawns[25][0] -= 168;
        spawns[25][1] -= 168;
        spawns[26][0] += 168;
        spawns[26][1] += 168;
        spawns[27][0] -= 168;
        spawns[27][1] += 168;
        spawns[28][0] += 168;
        spawns[28][1] -= 168;
    }

    level.cagefight = qtrue;
    level.messagedisplay = qtrue; // stop Seeker Released
    level.messagedisplay1 = qtrue; // stop RPG/M4 stuff
    if(level.hideseek_cageSize == 2){ // Spawn big cage.
        SpawnCage(level.hideseek_cage, NULL, qtrue, qtrue);
        SpawnCage(level.hideseek_cage, NULL, qtrue, qtrue); // 2 to be sure no parts are missing
    }else if(!level.hideseek_cageSize){
        SpawnCage(level.hideseek_cage, NULL, qtrue, qfalse);
        SpawnCage(level.hideseek_cage, NULL, qtrue, qfalse); // 2 to be sure no parts are missing
    }

    for(i = 0; i < level.numConnectedClients; i++){
        if(level.clients[level.sortedClients[i]].sess.team == TEAM_RED && (level.clients[level.sortedClients[i]].sess.cageFighter == qtrue || level.mapHighScore == 0)){
            //respawn ( &g_entities[level.sortedClients[i]] );
            if(level.hideseek_cageSize != 1){ // 1 = no cage at all, if this is the cage, don't teleport them to the 'cage'.
                TeleportPlayer(&g_entities[level.sortedClients[i]], spawns[count], level.clients[level.sortedClients[i]].ps.viewangles, qtrue);
                count += 1;
            }
            level.clients[level.sortedClients[i]].ps.stats[STAT_WEAPONS] = 0;
            memset ( level.clients[level.sortedClients[i]].ps.ammo, 0, sizeof(level.clients[level.sortedClients[i]].ps.ammo) );
            memset ( level.clients[level.sortedClients[i]].ps.clip, 0, sizeof(level.clients[level.sortedClients[i]].ps.clip) );
            //g_entities[level.lastalive[0]].client->ps.ammo[weaponData[WP_RPG7_LAUNCHER].attack[ATTACK_NORMAL].ammoIndex]=2;
            level.clients[level.sortedClients[i]].ps.stats[STAT_WEAPONS] |= ( 1 << WP_KNIFE );
            level.clients[level.sortedClients[i]].ps.clip[ATTACK_NORMAL][WP_KNIFE]=1;
            level.clients[level.sortedClients[i]].ps.stats[STAT_WEAPONS] |= ( 1 << WP_AK74_ASSAULT_RIFLE );
            //g_entities[level.lastalive[0]].client->ps.clip[ATTACK_NORMAL][WP_RPG7_LAUNCHER]=1;
            //g_entities[level.lastalive[0]].client->ps.firemode[WP_RPG7_LAUNCHER] = BG_FindFireMode ( WP_RPG7_LAUNCHER, ATTACK_NORMAL, WP_FIREMODE_AUTO );
            level.clients[level.sortedClients[i]].ps.weapon = WP_AK74_ASSAULT_RIFLE;
            level.clients[level.sortedClients[i]].ps.weaponstate = WEAPON_READY;
            level.clients[level.sortedClients[i]].ps.weaponTime = 0;
            level.clients[level.sortedClients[i]].ps.weaponAnimTime = 0;
            level.clients[level.sortedClients[i]].ps.stats[STAT_FROZEN] = 10000;
        }else if(level.clients[level.sortedClients[i]].sess.team != TEAM_SPECTATOR){ // Boe!Man 8/30/11: Means the client is a Red or Blue player that wasn't qualified to play in the cage fight. Pop them.
            G_Damage (&g_entities[level.sortedClients[i]], NULL, NULL, NULL, NULL, 10000, 0, MOD_POP, HL_NONE);
        } // Boe!Man 9/4/11: Else would be spectator only, no need to do anything.
    }
    // when it ends execute this:
    //Com_Printf("Updating scores..\n");
    //UpdateScores();
    //level.cagefight = qfalse;
    //LogExit( "Timelimit hit." );
}

static int weaponDropOdds[6][6] =
{
    { 80, 20, 0, 0, 0, 0 },         // 1-2 zombies
    { 35, 50, 15, 0, 0, 0 },        // 3-4 zombies
    { 5, 5, 50, 20, 20, 0 },        // 5-6 zombies
    { 5, 5, 25, 35, 20, 10 },       // 7-9 zombies
    { 5, 5, 15, 20, 40, 15 },       // 10-14 zombies
    { 5, 5, 5, 15, 40, 30 },        // >= 15 zombies
};

static int weaponGroups[6][3] =
{//  weapon1                       weapon2 (optional)       weapon3 (optional)
    { WP_M1911A1_PISTOL,            WP_USSOCOM_PISTOL,      0               , },   //group 1
    { WP_MICRO_UZI_SUBMACHINEGUN,   WP_M3A1_SUBMACHINEGUN,  0               , },   //group 2
    { WP_MSG90A1,                   WP_M60_MACHINEGUN,      0               , },   //group 3
    { WP_M4_ASSAULT_RIFLE,          WP_AK74_ASSAULT_RIFLE,  0               , },   //group 4
    { WP_USAS_12_SHOTGUN,           WP_SMOHG92_GRENADE,     0               , },   //group 5
    { WP_MM1_GRENADE_LAUNCHER,      WP_RPG7_LAUNCHER,       WP_L2A2_GRENADE , },   //group 6
};

void DropRandom( gentity_t *ent, int zombies){
    gentity_t   *dropped;
    gitem_t *item;
    int group, random = 0, i, DropGroup, start;
    int Odds[8][1];

    if(zombies >= 1 && zombies <= 2){
        DropGroup = 0;
    }else if(zombies >= 3 && zombies <= 4){
        DropGroup = 1;
    }else if(zombies >= 5 && zombies <= 6){
        DropGroup = 2;
    }else if(zombies >= 7 && zombies <= 9){
        DropGroup = 3;
    }else if(zombies >= 10 && zombies <= 14){
        DropGroup = 4;
    }else{ // >= 15
        DropGroup = 5;
    }

    random = irand(0, 100);
    start = 0;

    for (i = 0; i < 6; i++){
        Odds[i][0] = start;
        Odds[i][1] = (Odds[i][0]+weaponDropOdds[DropGroup][i])-1;
        if((random >= Odds[i][0] && random <= Odds[i][1])
            || (i == 0 && random <= Odds[i][1])
            || (i == 5)
            || (random >= Odds[i][1] && weaponDropOdds[DropGroup][i+1] == 0)){
                group = i;
                break;
        }
        start = Odds[i][0]+weaponDropOdds[DropGroup][i];
    }

    if (weaponGroups[group][1] != 0){
        if (weaponGroups[group][2] == 0){
            random = irand(0,1); // randomize between 2 weapons
        }else{
            random = irand(0, 2); // randomize between 3 weapons
        }
    }

    item = BG_FindWeaponItem ((weapon_t)weaponGroups[group][random]);
    item->addAllAmmo = qtrue;
    dropped = G_DropItem2(ent->r.currentOrigin, vec3_origin, item);
}


void CloneBody( gentity_t *ent, int number )
{
    gentity_t   *body;
    int         contents;
    int         parm;
    vec3_t      velo;

    int hitLocation = HL_NONE;
    vec3_t  direction;

    // Boe!Man 10/21/14: Turn off any kind of zooming when zombifying.
    #ifndef _DEMO
    if(ent->client->ps.pm_flags & PMF_ZOOMED)
    #else
    if(ent->client->ps.zoomFov)
    #endif // not _DEMO
    {
        ent->client->ps.zoomFov = 0;
        ent->client->ps.zoomTime = pm->ps->commandTime;
        ent->client->ps.pm_flags &= ~(PMF_ZOOM_FLAGS);
    }

    VectorCopy(ent->client->ps.viewangles, ent->client->sess.tempangles);

    #ifndef _DEMO
    Henk_CloseSound(ent->r.currentOrigin, G_SoundIndex("sound/enemy/dog/bark03.mp3", qtrue));
    Henk_CloseSound(ent->r.currentOrigin, G_SoundIndex("sound/enemy/dog/attack01.mp3", qtrue));
    #endif // not _DEMO

    // Boe!Man 1/8/12: Fix crash issues in DLL and possibily QVM by processing uninitialised vector.
    //VectorCopy(direction, vec3_origin);
    VectorCopy(vec3_origin, direction);
    trap_UnlinkEntity (ent);

    // if client is in a nodrop area, don't leave the body
    contents = trap_PointContents( ent->r.currentOrigin, -1 );
    if ( contents & CONTENTS_NODROP )
    {
        return;
    }

    // grab a body que and cycle to the next one
    body = level.bodyQue[ level.bodyQueIndex ];

    #ifndef _DEMO
    level.bodyQueIndex = (level.bodyQueIndex + 1) % level.bodyQueSize;
    #else
    level.bodyQueIndex = (level.bodyQueIndex + 1) % BODY_QUEUE_SIZE;
    #endif // not _DEMO

    trap_UnlinkEntity (body);

    body->s                 = ent->s;
    body->s.eType           = ET_BODY;
    body->s.eFlags          = EF_DEAD;
    body->s.gametypeitems   = 0;
    body->s.loopSound       = 0;
    body->s.number          = body - g_entities;
    body->timestamp         = level.time;
    body->physicsObject     = qtrue;
    body->physicsBounce     = 0;
    body->s.otherEntityNum  = ent->s.clientNum;
    g_entities[number].client->sess.zombie = qtrue;
    g_entities[number].client->sess.zombiebody = body->s.number;

    //if ( body->s.groundEntityNum == ENTITYNUM_NONE )
    VectorCopy(ent->client->ps.velocity, velo);
    AngleVectors( ent->client->ps.viewangles, velo, NULL, NULL );
    VectorScale( velo, 600, velo );
    body->s.pos.trType = TR_GRAVITY;
    body->s.pos.trTime = level.time;
    velo[2] = 200;
    VectorCopy( velo, body->s.pos.trDelta );

    body->s.event = 0;

    parm  = (DirToByte( direction )&0xFF);
    parm += (hitLocation<<8);

    G_AddEvent(body, EV_BODY_QUEUE_COPY, parm);

    body->r.svFlags = ent->r.svFlags | SVF_BROADCAST;
    VectorCopy (ent->r.mins, body->r.mins);
    VectorCopy (ent->r.maxs, body->r.maxs);
    VectorCopy (ent->r.absmin, body->r.absmin);
    VectorCopy (ent->r.absmax, body->r.absmax);

    body->s.torsoAnim = 45;

    body->clipmask = MASK_SHOT;
    body->r.contents = 0; // CONTENTS_CORPSE;
    body->r.ownerNum = ent->s.number;

    body->nextthink = level.time + 10000;
    body->think = G_FreeEntity;

    body->s.time2 = 0;

    body->die = body_die;
    body->takedamage = qtrue;

    body->s.apos.trBase[PITCH] = 0;

    body->s.pos.trBase[2] = ent->client->ps.origin[2];

    VectorCopy ( body->s.pos.trBase, body->r.currentOrigin );

    trap_LinkEntity (body);

}

void CheckEnts(gentity_t *ent){
    if(ent->model && ent->model != NULL && !strcmp(ent->model, "BLOCKED_TRIGGER"))
        {
            if(ent->count){
                ///Team Games
                if(level.gametypeData->teams){
                    if( ent->count <= (TeamCount( -1, TEAM_RED, NULL )) && ent->count <= (TeamCount( -1, TEAM_BLUE, NULL ))){
                        if (ent->r.linked)  {
                            trap_UnlinkEntity( ent );
                            if (ent->message != NULL)
                                G_Broadcast(ent->message, BROADCAST_GAME, NULL);
                        }
                    }
                    else if(!ent->r.linked) {
                        trap_LinkEntity( ent );
                        if(ent->message2 != NULL)
                            G_Broadcast(ent->message2, BROADCAST_GAME, NULL);
                    }
                }
                ///Non-Team Games
                else if(ent->count >= level.numPlayingClients){
                    if (ent->r.linked){
                        trap_UnlinkEntity( ent );
                        if(ent->message != NULL)
                            G_Broadcast(ent->message, BROADCAST_GAME, NULL);
                    }
                }
                else if(!ent->r.linked) {
                    trap_LinkEntity( ent );
                    if(ent->message2 != NULL)
                        G_Broadcast(ent->message2, BROADCAST_GAME, NULL);
                }
            }
            return;
        }
    if(level.numPlayingClients < ent->min_players && ent->min_players != 0){
        if(ent->r.linked){
            trap_UnlinkEntity(ent);
            g_entities[ent->effect_index].disabled = qtrue;
        }
    }else if(level.numPlayingClients >= ent->min_players && ent->min_players != 0){
        if(level.numPlayingClients <= ent->max_players){
            if(!ent->r.linked){
                trap_LinkEntity(ent);
                g_entities[ent->effect_index].disabled = qfalse;
            }
        }
    }
    if(level.numPlayingClients < ent->max_players && ent->max_players != 0){
        if(level.numPlayingClients > ent->min_players){
            if(!ent->r.linked){
                trap_LinkEntity(ent);
                g_entities[ent->effect_index].disabled = qfalse;
            }
        }
    }else if(level.numPlayingClients >= ent->max_players && ent->max_players != 0){
        if(ent->r.linked){
            trap_UnlinkEntity(ent);
            g_entities[ent->effect_index].disabled = qtrue;
        }
    }
}

qboolean IsClientMuted(gentity_t *ent, qboolean message){
    int i, remain, remainS;
    int counter = 0;

    // Boe!Man 7/18/13: Most of the time no-one is muted, so this really saves resources.
    if(!level.muteClientCount)
        return qfalse;

    for(i = 0; i < MAX_CLIENTS; i++){
        if(level.mutedClients[i].used == qtrue){
            counter++;
            if(!strcmp(level.mutedClients[i].ip, ent->client->pers.ip)){
                if(level.time < (level.mutedClients[i].startTime + level.mutedClients[i].time )){
                    if(message){
                        remain = ((level.mutedClients[i].startTime + level.mutedClients[i].time - level.time) / 1000) / 60;
                        remainS = ((level.mutedClients[i].startTime + level.mutedClients[i].time - level.time) / 1000);
                        G_printInfoMessage(ent, "You were muted for %d minutes, %d:%02d minutes remaining.", level.mutedClients[i].totalDuration, remain, remainS-(remain*60));
                    }
                    return qtrue;
                }else{
                    trap_SendServerCommand(-1, va("print \"^3[Auto action] ^7%s has been unmuted.\n\"", ent->client->pers.cleanName));
                    memset(&level.mutedClients[i], 0, sizeof(muted_t));
                    level.muteClientCount--;
                    ent->client->sess.mute = qfalse;
                }
            }

            if(level.muteClientCount == counter)
                break;
        }
    }
    return qfalse;
}

char    *ConcatArgs1( int start ) {
    int         i, c, tlen;
    static char line[MAX_STRING_CHARS];
    int         len;
    char        arg[MAX_STRING_CHARS];

    len = 0;
    c = trap_Argc();
    for ( i = start ; i < c ; i++ ) {
        trap_Argv( i, arg, sizeof( arg ) );
        tlen = strlen( arg );
        if ( len + tlen >= MAX_STRING_CHARS - 1 ) {
            break;
        }
        memcpy( line + len, arg, tlen );
        len += tlen;
        if ( i != c - 1 ) {
            line[len] = ' ';
            len++;
        }
    }

    line[len] = 0;

    return line;
}

qboolean AddMutedClient(gentity_t *ent, int time){
    int i;
    for(i = 0; i < MAX_CLIENTS; i++){
        if(level.mutedClients[i].used == qfalse){
            strcpy(level.mutedClients[i].ip, ent->client->pers.ip);
            level.mutedClients[i].used = qtrue;
            level.mutedClients[i].time = time * 60000;
            level.mutedClients[i].totalDuration = time;
            level.mutedClients[i].startTime = level.time;
            // Boe!Man 12/11/11: Handle this globally.
            ent->client->sess.mute = qtrue;
            level.muteClientCount++;
            return qtrue;
        }
    }
    return qfalse;
}

qboolean RemoveMutedClient(gentity_t *ent){
    int i;
    int counter = 0;
    qboolean unmuted = qfalse;
    for(i = 0; i < MAX_CLIENTS; i++){
        if(level.mutedClients[i].used == qtrue){
            counter++;
            if(!strcmp(level.mutedClients[i].ip, ent->client->pers.ip)){
                memset(&level.mutedClients[i], 0, sizeof(muted_t));
                ent->client->sess.mute = qfalse;
                level.muteClientCount--;
                unmuted = qtrue;
            }

            if(level.muteClientCount == counter)
                break;
        }
    }
    return unmuted;
}

char *GetReason(void) {
    static char reason[MAX_STRING_CHARS];
    char        *arg;
    int         argc, i;

    // Reset buffer.
    memset(reason, 0, sizeof(reason));

    // Get argument count.
    argc = G_GetChatArgumentCount();

    // Don't continue if we have no reason to fetch.
    if(argc < 2){
        Q_strncpyz(reason, ConcatArgs1(3), sizeof(reason));
    }else{
        // Append arguments to the buffer.
        for(i = 2; i <= argc; i++){
            // Fetch argument.
            arg = G_GetChatArgument(i);

            // Append to buffer.
            if(i != 2){
                strncat(reason, va(" %s", arg), sizeof(reason) - strlen(reason));
            }else{
                Q_strncpyz(reason, arg, sizeof(reason));
            }
        }
    }

    // Return buffer containing the reason.
    return reason;
}

int StartAfterCommand(char *param){
    int i;
    for(i=0;i<=strlen(param);i++){
        if(param[i] == ' '){
            return i+1;
        }
    }
    return 0;
}

unsigned int henk_atoi( const char *string ) {
    unsigned int        sign;
    unsigned int        value;
    unsigned int        c;

    while ( *string <= ' ' ) {
        if ( !*string ) {
            return 0;
        }
        string++;
    }

    switch ( *string ) {
    case '+':
        string++;
        sign = 1;
        break;
    case '-':
        string++;
        sign = -1;
        break;
    default:
        sign = 1;
        break;
    }

    value = 0;
    do {
        c = *string++;
        if ( c < '0' || c > '9' ) {
            break;
        }
        c -= '0';
        value = value * 10 + c;
    } while ( 1 );

    return value * sign;
}

void InitSpawn(int choice) // load bsp models before players loads a map(SOF2 clients cannot load a bsp model INGAME)
{
    // Boe!Man 5/14/11: Fix for random spawnpoint in DM/TDM gametypes.
    if(current_gametype.value == GT_DM || current_gametype.value == GT_TDM){
        return;
    }

    AddSpawnField("classname", "misc_bsp"); // blocker

    if(choice == 1){
    AddSpawnField("bspmodel",   "instances/Generic/fence01");
    }else if(choice == 2){
    AddSpawnField("bspmodel",   "instances/Colombia/npc_jump1");
    }else if(choice == 3){
    AddSpawnField("bspmodel",   "instances/Kamchatka/wall01");
    }else if(choice == 4){
        AddSpawnField("bspmodel",   "instances/Colombia/tree01");
    }else if(choice == 5){
        AddSpawnField("bspmodel",   "instances/Colombia/tree02");
    }else if(choice == 6){
        AddSpawnField("bspmodel",   "instances/Colombia/tree06");
    }

    AddSpawnField("origin",     "-4841 -4396 5000");
    AddSpawnField("angles",     "0 90 0");
    AddSpawnField("model",      "trigger_hurt"); //blocked_trigger
    AddSpawnField("count",       "1");

    G_SpawnGEntityFromSpawnVars(qfalse);
}

qboolean henk_isdigit(char c){ // by henk
    if(c >= 48 && c <= 57){
        return qtrue;
    }else{
        return qfalse;
    }
}


qboolean henk_ischar(char c){
    if(c >= 97 && c <= 122){ // a-z lowercase
        return qtrue;
    }else if(c >= 65 && c <= 90){ // A-Z uppercase
        return qtrue;
    }else if(c >= 33 && c <= 47 || c >= 58 && c <= 64 || c >= 91 && c <= 95 || c >= 123 && c <= 125){ // symbols
        return qtrue;
    }else{
        return qfalse;
    }
}

char *ChooseTeam(){
    int counts[3], seekers = 0, maxhiders = 0;

    counts[TEAM_BLUE] = TeamCount1(TEAM_BLUE);
    counts[TEAM_RED] = TeamCount1(TEAM_RED);

    if (level.customETHiderAmount[0]){
        int i;

        // The user put custom values here. Check them.
        for (i = 0; i < sizeof(level.customETHiderAmount) - 1; i++){
            if (level.customETHiderAmount[i + 1] == -1){
                // It seems the maximum of hiders specified is reached. Use that amount of seekers.
                seekers = i + 1;
                maxhiders = level.customETHiderAmount[i] + 1;
                break;
            }

            if (counts[TEAM_RED] >= level.customETHiderAmount[i] && counts[TEAM_RED] <= level.customETHiderAmount[i + 1]){
                seekers = i + 1;
                maxhiders = level.customETHiderAmount[i + 1];
                break;
            }
        }
    }else{
        seekers = (counts[TEAM_RED] + 1) / 5 + 1;

        if(counts[TEAM_BLUE] < 2){
            maxhiders = 6; // Henkie 24/02/10 -> Was 4
        }else{
            maxhiders = seekers * 5 - 2;
        }
    }

    if ( counts[TEAM_BLUE] >= seekers)  {
        return "r";
    }else if(counts[TEAM_RED] >= maxhiders){
        return "b";
    }else{
        return "r";
    }
}

void PrintCustom(int numb){
    void    *GP2, *group;
    char desc[256];
    char level[8];
    char name[32];
    char shortCmd[32];

    GP2 = trap_GP_ParseFile(g_customCommandsFile.string, qtrue, qfalse);
    if(!GP2){
        Com_Printf("Error while loading %s\n", g_customCommandsFile.string);
        return;
    }

    group = trap_GPG_GetSubGroups(GP2);
    while(group)
    {
        trap_GPG_FindPairValue(group, "Command", "none", name);
        if (strcmp(name, "none") != 0) {
            trap_GPG_FindPairValue(group, "Description", "No description defined", desc);
            trap_GPG_FindPairValue(group, "AdminLevel", "5", level);
            trap_GPG_FindPairValue(group, "ShortCommand", "", shortCmd);
            trap_SendServerCommand(numb, va("print \"[^3%-1.1s^7%-3s %-7.7s %-28.28s^7[^3%.35s^7]\n\"", level, "]", shortCmd, name, desc));
        }
        group = trap_GPG_GetNext(group);
    }
    trap_GP_Delete(&GP2);
    trap_SendServerCommand( numb, va("print \"\n\""));
}

int FormatDamage(int damage){
    char test[10];
    char *test1;
    strcpy(test, va("%i", damage));
    if(damage >= 100 && damage < 1000){
    test1 = va("%c00", test[0]);
    damage = atoi(test1);
    }else if(damage >= 1000 && damage < 10000){
    test1 = va("%c%c00", test[0], test[1]);
    damage = atoi(test1);
    }else if(damage >= 10000 && damage < 100000){
    test1 = va("%c%c%c00", test[0], test[1], test[2]);
    damage = atoi(test1);
    }
return damage/10;
}

/*
==============
G_IP2Integer
12/05/15 - 12:59 AM
Converts an IP address
to a numeric form.
==============
*/

unsigned int G_IP2Integer(const char *originalIP){
    char            *octet;
    char            ip[MAX_IP];
    int             octetCount = 3;
    unsigned int    ipNum = 0;

    // Make a copy from the original so strtok never modifies the original data.
    strncpy(ip, originalIP, sizeof(ip));
    octet = strtok(ip, ".");

    // Loop through octects while they are available.
    while(octet != NULL){
        ipNum += (unsigned int)(atoi(octet) * pow(256, octetCount--));
        octet = strtok(NULL, ".");
    }

    // Return result.
    return ipNum;
}

void HENK_COUNTRY(gentity_t *ent){
    int     rc, start;
    unsigned int        IPnum;
    sqlite3_stmt        *stmt;
    int                 tableNum = -1;

    if(sql_timeBench.integer){
        start = trap_Milliseconds();
    }

    IPnum = G_IP2Integer(ent->client->pers.ip);
    rc = sqlite3_prepare(countryDb, va("SELECT table_index from country_index WHERE %u BETWEEN begin_ip AND end_ip", IPnum), -1, &stmt, 0);
    if(rc != SQLITE_OK){
        Com_Printf("^1Error: ^7Country database: %s\n", sqlite3_errmsg(countryDb));
        sqlite3_finalize(stmt);
        return;
    }else while((rc = sqlite3_step(stmt)) != SQLITE_DONE){
        tableNum = sqlite3_column_int(stmt, 0);
        break;
    }
    sqlite3_finalize(stmt);

    // Boe!Man 6/28/13: Check if the table number has been found.
    if(tableNum == -1)
        return;


    rc = sqlite3_prepare(countryDb, va("SELECT country,ext from db%i WHERE %u BETWEEN begin_ip AND end_ip", tableNum, IPnum), -1, &stmt, 0);
    if(rc != SQLITE_OK){
        Com_Printf("^1Error: ^7Country database: %s\n", sqlite3_errmsg(countryDb));
        sqlite3_finalize(stmt);
        return;
    }else while((rc = sqlite3_step(stmt)) != SQLITE_DONE){
        if(rc == SQLITE_ROW){
            strcpy(ent->client->sess.country,(char *)sqlite3_column_text(stmt, 0));
            strcpy(ent->client->sess.countryext, (char *)sqlite3_column_text(stmt, 1));
            break;
        }
    }
    sqlite3_finalize(stmt);

    if(sql_timeBench.integer){
        Com_Printf(va("IP2Country took %d ms\n", trap_Milliseconds()-start));
    }
    return;
}
