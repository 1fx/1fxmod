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

// Henk 20/02/10 -> Add UpdateScores()
void ShowScores(void)
{
    char    winner[64];

    Com_sprintf(winner, sizeof(winner), "%s ^7won the round!", level.cagewinner);

    // Boe!Man 9/2/12: Advanced H&S statistics.
    if(hideSeek_ExtendedRoundStats.integer && level.time > level.awardTime + 8000 && level.awardTime){
        G_Broadcast(va("^3%s\n\n^3Statistics for this map:\n"
                "^_Rounds survived: ^3%i ^_by ^3%s\n"
                "^_MM1 hits taken: ^3%i ^_by ^3%s\n"
                "^_RPG boosts: ^3%i ^_by ^3%s\n"
                "^_Taken RPG: ^3%i ^_by ^3%s\n"
                "^_Taken M4: ^3%i ^_by ^3%s\n"
                "^_Stun attacks: ^3%i ^_by ^3%s\n"
                "^_Seekers caged: ^3%i ^_by ^3%s\n"
                "^_Weapons stolen: ^3%i ^_by ^3%s\n\n"
                "^yPoints: ^3%i ^yby ^3%s\n"
                "^yTaken MM1: ^3%i ^yby ^3%s\n"
                "^yStunned: ^3%i ^yby ^3%s\n"
                "^yTrapped in cage: ^3%i ^yby ^3%s",
                g_motd.string,
                level.advancedHsScores[0].score, level.advancedHsScores[0].name, level.advancedHsScores[1].score, level.advancedHsScores[1].name, level.advancedHsScores[2].score, level.advancedHsScores[2].name, level.advancedHsScores[3].score, level.advancedHsScores[3].name,
                level.advancedHsScores[4].score, level.advancedHsScores[4].name, level.advancedHsScores[5].score, level.advancedHsScores[5].name, level.advancedHsScores[6].score, level.advancedHsScores[6].name, level.advancedHsScores[7].score, level.advancedHsScores[7].name,
                level.advancedHsScores[8].score, level.advancedHsScores[8].name, level.advancedHsScores[9].score, level.advancedHsScores[9].name, level.advancedHsScores[10].score, level.advancedHsScores[10].name, level.advancedHsScores[11].score, level.advancedHsScores[11].name
        ), BROADCAST_AWARDS, NULL);
    }else{
        G_Broadcast(va("^3%s\n\n%s\n\n^_ THE 3 BEST HIDERS IN THIS MAP ARE:\n^31st ^7%s with ^3%i ^7wins.\n^+2nd ^7%s with ^+%i ^7wins.\n^@3rd ^7%s with ^@%i ^7wins.\n\n"
                "^y THE 3 BEST SEEKERS IN THIS MAP ARE:\n^31st ^7%s with ^3%i ^7kills.\n^+2nd ^7%s with ^+%i ^7kills.\n^@3rd ^7%s with ^@%i ^7kills.",
                g_motd.string, winner,
                level.top3Hiders[0].name, level.top3Hiders[0].score, level.top3Hiders[1].name, level.top3Hiders[1].score, level.top3Hiders[2].name, level.top3Hiders[2].score,
                level.top3Seekers[0].name, level.top3Seekers[0].score, level.top3Seekers[1].name, level.top3Seekers[1].score, level.top3Seekers[2].name, level.top3Seekers[2].score
        ), BROADCAST_AWARDS, NULL);
    }
}

/*
================
Henk_GetScore
Recode by boe - 2/25/13 - 6:25 PM
Get scores for the end-scoreboard (H&S).
================
*/
int Henk_GetScore (qboolean seekers)
{
    char            mapname[64]; // Stores the current map name (used for table name).
    int             rc, spotsTaken;
    sqlite3         *db;
    sqlite3_stmt    *stmt;
    qboolean        dbOkay;

    if(!level.altPath){
        rc = sqlite3_open_v2("./users/scores.db", &db, SQLITE_OPEN_READONLY, NULL);
    }else{
        rc = sqlite3_open_v2(va("%s/users/scores.db", level.altString), &db, SQLITE_OPEN_READONLY, NULL);
    }
    if(rc){
        G_LogPrintf("^1Error: ^7scores database: %s\n", sqlite3_errmsg(db));
        dbOkay = qfalse;
    }else{
        dbOkay = qtrue;
    }

    // Boe!Man 2/25/13: Only do this if the connection to the database is successfull.
    if(dbOkay){
        spotsTaken = 0;
        trap_Cvar_VariableStringBuffer("mapname", mapname, MAX_QPATH);
        if(seekers){
            sqlite3_prepare(db, va("SELECT name,score FROM %s WHERE team='%i' ORDER BY score DESC LIMIT 3", mapname, TEAM_BLUE), -1, &stmt, 0);
            while(sqlite3_step(stmt) == SQLITE_ROW && spotsTaken < 3){
                strncpy(level.top3Seekers[spotsTaken].name, (const char *)sqlite3_column_text(stmt, 0), sizeof(level.top3Seekers[spotsTaken].name));
                level.top3Seekers[spotsTaken].score = sqlite3_column_int(stmt, 1);
                spotsTaken++;
            }
            sqlite3_finalize(stmt);
            if(spotsTaken < 3){ // If there are less then 3 seekers found, fill up with none-data.
                while(spotsTaken < 3){
                    strcpy(level.top3Seekers[spotsTaken].name, "none");
                    level.top3Seekers[spotsTaken].score = 0;
                    spotsTaken++;
                }
            }
        }else{ // Hiders.
            sqlite3_prepare(db, va("SELECT name,count(*) as b FROM %s WHERE team='%i' GROUP BY name ORDER BY b DESC LIMIT 3", mapname, TEAM_RED), -1, &stmt, 0);

            while(sqlite3_step(stmt) == SQLITE_ROW && spotsTaken < 3){
                strncpy(level.top3Hiders[spotsTaken].name, (const char *)sqlite3_column_text(stmt, 0), sizeof(level.top3Hiders[spotsTaken].name));
                level.top3Hiders[spotsTaken].score = sqlite3_column_int(stmt, 1);
                spotsTaken++;
            }
            sqlite3_finalize(stmt);
            if(spotsTaken < 3){ // If there are less then 3 seekers found, fill up with none-data.
                while(spotsTaken < 3){
                    strcpy(level.top3Hiders[spotsTaken].name, "none");
                    level.top3Hiders[spotsTaken].score = 0;
                    spotsTaken++;
                }
            }
        }
        sqlite3_close(db);
    }

    return 0;
}

/*
================
UpdateScores
Recode by boe - 2/25/13 - 5:47 PM
Update the scores in the database (H&S).
================
*/
void UpdateScores(void)
{
    char            mapname[64]; // Stores the current map name (used for table name).
    int             rc, i;
    int             highestHider = 0, highestScore = 0;
    sqlite3         *db;
    qboolean        dbOkay;
    char            clientName[MAX_NETNAME]; // Query compatible-name.

    if(!level.altPath){
        rc = sqlite3_open_v2("./users/scores.db", &db, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL);
    }else{
        rc = sqlite3_open_v2(va("%s/users/scores.db", level.altString), &db, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL);
    }
    if(rc){
        G_LogPrintf("^1Error: ^7scores database: %s\n", sqlite3_errmsg(db));
        dbOkay = qfalse;
    }else{
        dbOkay = qtrue;
        sqlite3_exec(db, "PRAGMA synchronous = OFF", NULL, NULL, NULL);
        sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL);
    }

    // Boe!Man 2/25/13: Check if a table for the mapname exists yet.
    // Start by fetching the mapname.
    trap_Cvar_VariableStringBuffer("mapname", mapname, MAX_QPATH);
    if(sqlite3_exec(db, va("CREATE TABLE IF NOT EXISTS %s('name' VARCHAR(36), 'team' INTEGER NOT NULL, 'score' INTEGER)", mapname), 0, 0, 0) != SQLITE_OK){
        G_LogPrintf("^1Error: ^7scores database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        dbOkay = qfalse;
    }

    // Next loop the clients, check for scores of red + blue.
    for ( i = 0; i < level.numConnectedClients; i ++ )
    {
        gentity_t* ent = &g_entities[level.sortedClients[i]];

        // Boe!Man 9/2/12: This is shitty, check for the advanced H&S scores. Kinda a performance hit.
        // MM1 Hits.
        if(ent->client->sess.MM1HitsTaken > level.advancedHsScores[1].score){
            level.advancedHsScores[1].score = ent->client->sess.MM1HitsTaken;
            strcpy(level.advancedHsScores[1].name, ent->client->pers.cleanName);
        }
        // RPG boosts.
        if(ent->client->sess.RPGBoosts > level.advancedHsScores[2].score){
            level.advancedHsScores[2].score = ent->client->sess.RPGBoosts;
            strcpy(level.advancedHsScores[2].name, ent->client->pers.cleanName);
        }
        // Taken RPG.
        if(ent->client->sess.takenRPG > level.advancedHsScores[3].score){
            level.advancedHsScores[3].score = ent->client->sess.takenRPG;
            strcpy(level.advancedHsScores[3].name, ent->client->pers.cleanName);
        }
        // Taken M4.
        if(ent->client->sess.takenM4 > level.advancedHsScores[4].score){
            level.advancedHsScores[4].score = ent->client->sess.takenM4;
            strcpy(level.advancedHsScores[4].name, ent->client->pers.cleanName);
        }
        // Stun attacks.
        if(ent->client->sess.stunAttacks > level.advancedHsScores[5].score){
            level.advancedHsScores[5].score = ent->client->sess.stunAttacks;
            strcpy(level.advancedHsScores[5].name, ent->client->pers.cleanName);
        }
        // Seekers caged.
        if(ent->client->sess.seekersCaged > level.advancedHsScores[6].score){
            level.advancedHsScores[6].score = ent->client->sess.seekersCaged;
            strcpy(level.advancedHsScores[6].name, ent->client->pers.cleanName);
        }
        // Weapons stolen.
        if(ent->client->sess.weaponsStolen > level.advancedHsScores[7].score){
            level.advancedHsScores[7].score = ent->client->sess.weaponsStolen;
            strcpy(level.advancedHsScores[7].name, ent->client->pers.cleanName);
        }
        // Taken MM1.
        if(ent->client->sess.takenMM1 > level.advancedHsScores[9].score){
            level.advancedHsScores[9].score = ent->client->sess.takenMM1;
            strcpy(level.advancedHsScores[9].name, ent->client->pers.cleanName);
        }
        // Stunned.
        if(ent->client->sess.stunned > level.advancedHsScores[10].score){
            level.advancedHsScores[10].score = ent->client->sess.stunned;
            strcpy(level.advancedHsScores[10].name, ent->client->pers.cleanName);
        }
        // Trapped in cage.
        if(ent->client->sess.trappedInCage > level.advancedHsScores[11].score){
            level.advancedHsScores[11].score = ent->client->sess.trappedInCage;
            strcpy(level.advancedHsScores[11].name, ent->client->pers.cleanName);
        }

        // Boe!Man 9/2/12: Don't continue just yet on these two, we need those advanced stats as well.
        if(ent->client->sess.kills == 0){
            continue;
        }
        if(ent->client->sess.team == TEAM_SPECTATOR){ // Boe!Man 8/7/12: Continue on spec.
            continue;
        }

        if(ent->client->sess.team == TEAM_BLUE){
            // Boe!Man 2/25/13: Add to table.
            if(dbOkay && ent->client->sess.kills){ // No need to do this for a player with zero points.
                Q_strncpyz(clientName, ent->client->pers.cleanName, sizeof(clientName));
                Boe_convertNonSQLChars(clientName);

                if(sqlite3_exec(db, va("INSERT INTO %s VALUES('%s', %i, %i)", mapname, clientName, TEAM_BLUE, ent->client->sess.kills), 0, 0, 0) != SQLITE_OK){
                    G_LogPrintf("^1Error: ^7scores database: %s\n", sqlite3_errmsg(db));
                }
            }
            // Boe!Man 9/2/12: Also check for the highest blue player.
            if(ent->client->sess.kills > level.advancedHsScores[8].score){
                level.advancedHsScores[8].score = ent->client->sess.kills;
                strcpy(level.advancedHsScores[8].name, ent->client->pers.cleanName);
            }
        }else{ // Red team.
            if(ent->client->sess.kills > highestScore){
                highestHider = ent->s.number;
                highestScore = ent->client->sess.kills;
            }
        }
    }

    // Boe!Man 9/2/12: For the advanced H&S scoretable, check for things that aren't achieved by anyone and fill those slots with data.
    for(i = 1; i < 12; i++){
        if(!level.advancedHsScores[i].score){
            strcpy(level.advancedHsScores[i].name, "none");
        }
    }

    // Write highest hider score.
    if(highestScore){
        Q_strncpyz(clientName, g_entities[highestHider].client->pers.cleanName, sizeof(clientName));
        Boe_convertNonSQLChars(clientName);

        if(dbOkay){
            if(sqlite3_exec(db, va("INSERT INTO %s VALUES('%s', %i, ?)", mapname, clientName, TEAM_RED), 0, 0, 0) != SQLITE_OK){
                G_LogPrintf("^1Error: ^7scores database: %s\n", sqlite3_errmsg(db));
            }
        }

        // Boe!Man 9/2/12: Also add his score to the advanced table.
        level.advancedHsScores[0].score = g_entities[highestHider].client->sess.kills;
        strcpy(level.advancedHsScores[0].name, g_entities[highestHider].client->pers.cleanName);
        strcpy(level.cagewinner, g_entities[highestHider].client->pers.netname);
    }else{ // highestScore wasn't filled.. Seekers won this time.
        strncpy(level.cagewinner, server_seekerteamprefix.string, sizeof(level.cagewinner));
        strcpy(level.advancedHsScores[0].name, "none");
    }

    // Boe!Man 12/30/12: Close the scores database.
    if(dbOkay){
        sqlite3_exec(db, "COMMIT", NULL, NULL, NULL);
        sqlite3_close(db);
    }

    Henk_GetScore(qfalse); // For the hiders.
    Henk_GetScore(qtrue);  // For the seekers.
}

// 14/01/10: Custom commands by Henk
void SpawnCage(vec3_t org, gentity_t *ent, qboolean autoremove, qboolean big) // Spawn a cage
{
    // Boe!Man 6/22/12: Addition for big cage.
    char            *origin;
    int part, numb;

    for(part=1;part<=4;part++){
        AddSpawnField("classname", "misc_bsp"); // blocker
        AddSpawnField("bspmodel",   "instances/Generic/fence01");
        if(part == 1){
            if(!big){
                origin = va("%.0f %.0f %.0f",
                    org[0],
                    org[1]+126,
                org[2] - 50);
            }else{
                origin = va("%.0f %.0f %.0f",
                    org[0]+126,
                    org[1]+252,
                org[2] - 50);
            }
            AddSpawnField("origin",     origin);
            AddSpawnField("angles",     "0 360 0");
        }else if(part == 2){
            if(!big){
                origin = va("%.0f %.0f %.0f",
                    org[0],
                    org[1]-126,
                org[2] - 50);
            }else{
                origin = va("%.0f %.0f %.0f",
                    org[0]-126,
                    org[1]-252,
                org[2] - 50);
            }
            AddSpawnField("origin",     origin);
            AddSpawnField("angles",     "0 180 0");
        }else if(part == 3){
            if(!big){
                origin = va("%.0f %.0f %.0f",
                    org[0]-126,
                    org[1],
                org[2] - 50);
            }else{
                origin = va("%.0f %.0f %.0f",
                    org[0]-252,
                    org[1]-126,
                org[2] - 50);
            }
            AddSpawnField("origin",     origin);
            AddSpawnField("angles",     "0 -270 0");
        }else if(part == 4){
            if(!big){
                origin = va("%.0f %.0f %.0f",
                    org[0]+126,
                    org[1],
                org[2] - 50);
            }else{
                origin = va("%.0f %.0f %.0f",
                    org[0]+252,
                    org[1]+126,
                org[2] - 50);
            }
            AddSpawnField("origin",     origin);
            AddSpawnField("angles",     "0 -90 0");
        }
        AddSpawnField("model",      "trigger_hurt"); //blocked_trigger
        AddSpawnField("count",       "1");
        AddSpawnField("hideseek",    "1");
        numb = G_SpawnGEntityFromSpawnVars(qtrue);
        if(numb != -1 && !autoremove){
            g_entities[numb].think = G_FreeEntity;
            g_entities[numb].nextthink = level.time+12000;
        }

        level.numSpawnVars = 0;
        level.numSpawnVarChars = 0;

        if(big){ // Boe!Man 6/22/12: Spawn the additional fences.
            AddSpawnField("classname", "misc_bsp"); // blocker
            AddSpawnField("bspmodel",   "instances/Generic/fence01");
            if(part == 1){
                origin = va("%.0f %.0f %.0f",
                    org[0]-126,
                    org[1]+252,
                org[2] - 50);
                AddSpawnField("origin",     origin);
                AddSpawnField("angles",     "0 360 0");
            }else if(part == 2){
                origin = va("%.0f %.0f %.0f",
                    org[0]+126,
                    org[1]-252,
                org[2] - 50);
                AddSpawnField("origin",     origin);
                AddSpawnField("angles",     "0 180 0");
            }else if(part == 3){
                origin = va("%.0f %.0f %.0f",
                    org[0]-252,
                    org[1]+126,
                org[2] - 50);
                AddSpawnField("origin",     origin);
                AddSpawnField("angles",     "0 -270 0");
            }else if(part == 4){
                origin = va("%.0f %.0f %.0f",
                    org[0]+252,
                    org[1]-126,
                org[2] - 50);
                AddSpawnField("origin",     origin);
                AddSpawnField("angles",     "0 -90 0");
            }
            AddSpawnField("model",      "trigger_hurt"); //blocked_trigger
            AddSpawnField("count",       "1");
            AddSpawnField("hideseek",    "1");
            numb = G_SpawnGEntityFromSpawnVars(qtrue);
            if(numb != -1 && !autoremove){
                g_entities[numb].think = G_FreeEntity;
                g_entities[numb].nextthink = level.time+12000;
            }

            level.numSpawnVars = 0;
            level.numSpawnVarChars = 0;
        }
    }
}

void SpawnBox        (vec3_t org)
{
    char            *origin;
    int             numb;
    origin = va("%.0f %.0f %.0f", org[0], org[1], org[2]);// - (ent->client->ps.viewheight));

    AddSpawnField("classname", "misc_bsp"); // blocker
    AddSpawnField("bspmodel",   "instances/Colombia/npc_jump1");
    AddSpawnField("origin",     origin);
    AddSpawnField("angles",     "0 90 0");
    AddSpawnField("model",      "trigger_hurt"); //blocked_trigger
    AddSpawnField("count",       "1");
    AddSpawnField("hideseek",    "1");

    numb = G_SpawnGEntityFromSpawnVars(qtrue);
    if(numb != -1){
    g_entities[numb].think = G_FreeEntity;
    g_entities[numb].nextthink = level.time+10000;
    }
    level.numSpawnVars = 0;
    level.numSpawnVarChars = 0;
}

int SpawnBoxEx(vec3_t org, vec3_t ang)
{
        char            *origin;
        char            *angles;
    origin = va("%.0f %.0f %.0f", org[0], org[1], org[2]);
    angles = va("%.0f %.0f %.0f", ang[0], ang[1], ang[2]);

    AddSpawnField("classname", "misc_bsp"); // blocker
    AddSpawnField("bspmodel",   "instances/Colombia/npc_jump1");
    AddSpawnField("origin",     origin);
    AddSpawnField("angles",     angles);
    AddSpawnField("model",      "trigger_hurt"); //blocked_trigger
    AddSpawnField("count",       "1");

    return G_SpawnGEntityFromSpawnVars(qtrue);
}

void Effect (vec3_t org, char * name, qboolean rpg)
{
    char            *origin;
    origin = va("%.0f %.0f %.0f", org[0], org[1], org[2]);
    AddSpawnField("classname", "1fx_play_effect");
    if(strstr(name, "flare_red")){
    AddSpawnField("effect", name);
    if(rpg == qtrue){
        AddSpawnField("rpg", "true");
    }else{
        AddSpawnField("rpg", "false");
    }
    }else{
    AddSpawnField("effect", name);
    }
    AddSpawnField("origin",     origin);
    AddSpawnField("wait",       "3");
    AddSpawnField("count",       "-1");

    G_SpawnGEntityFromSpawnVars(qtrue);

    level.numSpawnVars = 0;
    level.numSpawnVarChars = 0;
}

void DoTeleport(gentity_t *ent, vec3_t origin){
    vec3_t angles;
    char *origin1;
    VectorCopy(ent->pos1, angles);
    origin[2] += 50;
    TeleportPlayer(ent, origin, ent->client->ps.viewangles, qtrue);

#ifdef _3DServer
    if (!(current_gametype.value == GT_ELIM && boe_fragWars.integer)){
#endif // _3DServer
    origin1 = va("%.0f %.0f %.0f", origin[0], origin[1], origin[2]);
    //G_PlayEffect ( G_EffectIndex("effects/explosions/col9_boat_explosion"),origin, angles);
    AddSpawnField("classname", "1fx_play_effect");
    AddSpawnField("effect", "effects/explosions/col9_boat_explosion");
    AddSpawnField("origin", origin1);
    AddSpawnField("wait", "1");
    AddSpawnField("count", "1");

    G_SpawnGEntityFromSpawnVars(qtrue);
#ifdef _3DServer
    }
#endif // _3DServer
}

// Henk 31/03/11 -> New M4 give away code
int GetM4Winner(int rpgwinner){
    int i;
    int winners[MAX_CLIENTS];
    int winnercount = 0;
    int deaths[MAX_CLIENTS];
    int deathcount = 0; // id
    int timeofdeath = 0; // time

    if(TeamCount1(TEAM_RED) < 2)
        return 100;

    for ( i = 0; i < level.numConnectedClients; i ++ )
    {
        gentity_t* ent = &g_entities[level.sortedClients[i]];
        if(ent->client->sess.team != TEAM_RED)
            continue;
        if(G_IsClientDead(ent->client))
            continue;
        if(ent->client->pers.connected != CON_CONNECTED)
            continue;

        if(ent->client->sess.timeOfDeath == 1 && ent->s.number != rpgwinner){ // this guy really won the round
            winners[winnercount] = ent->s.number;
            winnercount += 1;
        }else if(ent->client->sess.timeOfDeath == 0){ // weird case didn't die and won
            continue;
        }else if(ent->client->sess.timeOfDeath > 1 && ent->client->sess.timeOfDeath < level.gametypeStartTime){ // this is the time at which he died
            if(ent->client->sess.timeOfDeath > timeofdeath && ent->s.number != rpgwinner){
                timeofdeath = ent->client->sess.timeOfDeath;
                deaths[deathcount] = ent->s.number;
                deathcount += 1;
            }
        }
    }
    if(winnercount == 1){
        return winners[0];
    }else if(winnercount == 0 && deathcount >= 1){
        //return clients[irand(0, deaths-1)];
        return deaths[deathcount-1]; // Boe!Man 8/18/11: return the last person that got written to the integer, i.e. the one that lasted the longest.
    }else if(winnercount > 1){
        return winners[irand(0, winnercount-1)];
    }

    return -1; // fail
}

// Henk 30/03/11 -> New RPG give away code
int GetRpgWinner(void){
    int i;
    int winners[MAX_CLIENTS];
    int winnercount = 0;
    int deaths[MAX_CLIENTS];
    int deathcount = 0; // id
    int timeofdeath = 0; // time
    if(TeamCount1(TEAM_RED) < 1)
        return 100;
    for ( i = 0; i < level.numConnectedClients; i ++ )
    {
        gentity_t* ent = &g_entities[level.sortedClients[i]];
        if(ent->client->sess.team != TEAM_RED)
            continue;
        if(G_IsClientDead(ent->client))
            continue;
        if(ent->client->pers.connected != CON_CONNECTED)
            continue;

        if(ent->client->sess.timeOfDeath == 1){ // this guy really won the round
            winners[winnercount] = ent->s.number; // Boe!Man 8/18/11: Write it to a separate integer, or else it'll overwrite something from the death integer or vice versa..
            winnercount += 1;
        }else if(ent->client->sess.timeOfDeath == 0){ // weird case didn't die and won
            continue;
        }else if(ent->client->sess.timeOfDeath > 1 && ent->client->sess.timeOfDeath < level.gametypeStartTime){ // this is the time at which he died
            if(ent->client->sess.timeOfDeath > timeofdeath){
                timeofdeath = ent->client->sess.timeOfDeath;
                deaths[deathcount] = ent->s.number;
                deathcount += 1;
            }
        }
    }

    if(winnercount == 1){
        return winners[0];
    }else if(winnercount == 0 && deathcount >= 1){
        // Boe!Man 8/18/11: No no no no! Don't return a random value, we need to sort out which client lasted the longest!
        //return clients[irand(0, deaths-1)];
        return deaths[deathcount-1]; // So return the last person that got written to the integer, i.e. the one that lasted the longest.
    }else if(winnercount > 1){
        return winners[irand(0, winnercount-1)];
    }
    return -1; // fail
}

// Henk 23/01/10 -> Will strip hiders and put weapon to WP_NONE(puts their hands up).
void StripHiders(void)
{
    int i;
    char location[64];

    for ( i = 0; i < level.numConnectedClients; i ++ )
    {
        gentity_t* ent = &g_entities[level.sortedClients[i]];
        // Henk 26/01/10 -> Don't strip seekers..
        if(ent->client->sess.team != TEAM_RED) // better then TEAM_BLUE because specs are now also excluded
            continue;

        // Henk 26/01/10 -> Dead clients dun have to be stripped
        if(G_IsClientDead(ent->client))
            continue;

        if(level.crossTheBridge){
            Team_GetLocationMsg(ent, location, sizeof(location)); // Get the location..
            // Boe!Man 11/13/12: Only hiders that actually crossed the bridge should get a point.
            if (!strstr(location, "Safe")){
                continue;
            }
        }

        ent->client->sess.timeOfDeath = 1;
        ent->client->ps.zoomFov = 0;    ///if they are looking through a scope go to normal view
        ent->client->ps.pm_flags &= ~(PMF_GOGGLES_ON|PMF_ZOOM_FLAGS);
        ent->client->ps.stats[STAT_WEAPONS] = 0;
        ent->client->ps.stats[STAT_GOGGLES] = GOGGLES_NONE;
        memset ( ent->client->ps.ammo, 0, sizeof(ent->client->ps.ammo) );
        memset ( ent->client->ps.clip, 0, sizeof(ent->client->ps.clip) );
        ent->client->ps.weapon = WP_NONE;
        ent->client->ps.weaponstate = WEAPON_READY;
        ent->client->ps.weaponTime = 0;
        ent->client->ps.weaponAnimTime = 0;
        ent->client->sess.score += 1;
        ent->client->sess.kills += 1;
    }
}

// Henk 28/01/10 -> Add Eventeams
void EvenTeams_HS (gentity_t *adm, qboolean aet)
{
    int     counts[TEAM_NUM_TEAMS];
    int     diff = 0;
    int     highTeam, i;
    gentity_t *lastConnected;
    int seekers, maxhiders, totalplayers;

    if(level.intermissiontime || level.changemap)
        return;

    // Boe!Man 1/19/11: If teams are locked don't proceed with evening the teams.
    if(level.blueLocked || level.redLocked){
        if((adm && adm->client) || aet == qfalse){
            G_printInfoMessage(adm, "Teams are locked.");
        }
        return;
    }

    counts[TEAM_BLUE] = TeamCount1(TEAM_BLUE);
    counts[TEAM_RED] = TeamCount1(TEAM_RED);
    totalplayers = counts[TEAM_BLUE]+counts[TEAM_RED];
    // Henk 29/01/10 -> New player balance code
    if(totalplayers <= 2){
        if((adm && adm->client) || aet == qfalse){
            G_printInfoMessage(adm, "You need at least 3 players to use eventeams.");
        }
        return;
    }

    if(level.customETHiderAmount[0]){
        // The user put custom values here. Check them.
        for(i = 0; i < sizeof(level.customETHiderAmount)-1; i++){
            if(level.customETHiderAmount[i+1] == -1){
                // It seems the maximum of hiders specified is reached. Use that amount of seekers.
                seekers = i+1;
                break;
            }

            if(totalplayers >= level.customETHiderAmount[i] && totalplayers <= level.customETHiderAmount[i+1]){
                seekers = i+1;
                break;
            }
        }
    }else{
        if(counts[TEAM_RED] < 4){
            seekers = 1;
        }else{
            seekers = (totalplayers + 1) / 5;
        }
    }

    maxhiders = totalplayers-seekers;

    if(counts[TEAM_BLUE] < seekers){ // too few seekers
        highTeam = TEAM_RED;
        diff = (seekers - counts[TEAM_BLUE]); // move diff to seekers team
    }else if(counts[TEAM_BLUE] > seekers){ // too many seekers
        highTeam = TEAM_BLUE;
        diff = (counts[TEAM_BLUE] - seekers); // move diff to hiders team
    }
    else {
        if((adm && adm->client) || aet == qfalse){
            G_printInfoMessage(adm, "Teams are as even as possible.");
        }
        return;
    }
    // if less than 2 players difference, you cant make it any more even
    if(diff < 0){
        if((adm && adm->client) || aet == qfalse){
            G_printInfoMessage(adm, "Teams are as even as possible.");
        }
        return;
    }

    for(i = 0; i < diff; i++){
         lastConnected = findLastEnteredPlayer(highTeam, qfalse);

        // Boe!Man 7/13/12: Fix crash issue with auto eventeams too soon before entering the map.
        if(lastConnected == NULL){
            lastConnected = findLastEnteredPlayer(highTeam, qtrue); // Try to search for it again but also try to even the teams regardless of players with scores.

            if(lastConnected == NULL){
                if((adm && adm->client) || aet == qfalse){
                    G_printInfoMessage(adm, "You cannot even the teams this fast.");
                }
                return;
            }
        }

        if(!G_IsClientDead ( lastConnected->client )){
            TossClientItems( lastConnected ); // Henk 19/01/11 -> Fixed items not dropping with !et
        }

        lastConnected->client->ps.stats[STAT_WEAPONS] = 0;
        G_StartGhosting( lastConnected );

        if(highTeam == TEAM_RED){
            lastConnected->client->sess.team = TEAM_BLUE;
        }else{
            lastConnected->client->sess.team = TEAM_RED;
        }

        // Boe!Man 8/3/16: Ensure the last team is also set when forcefully switched.
        lastConnected->client->sess.lastTeam = lastConnected->client->sess.team;

        if (lastConnected->r.svFlags & SVF_BOT) {
            char userinfo[MAX_INFO_STRING];
            trap_GetUserinfo( lastConnected->s.number, userinfo, sizeof( userinfo ) );
            Info_SetValueForKey( userinfo, "team", lastConnected->client->sess.team == TEAM_RED?"red":"blue");
            trap_SetUserinfo( lastConnected->s.number, userinfo );
        }

        // Boe!Man 9/20/12: Also fix the scores (reset when switching teams using !et).
        lastConnected->client->sess.score = 0;
        lastConnected->client->sess.kills = 0;
        lastConnected->client->sess.killsAsZombie = 0;
        lastConnected->client->sess.deaths = 0;
        lastConnected->client->sess.timeOfDeath = 0; // Boe!Man 8/29/11: Also reset this when switching team (so seekers that won won't get RPG for example).

        lastConnected->client->pers.identity = NULL;
        ClientUserinfoChanged( lastConnected->s.number );
        CalculateRanks();

        G_StopFollowing( lastConnected );
        G_StopGhosting( lastConnected );
        trap_UnlinkEntity ( lastConnected );
        ClientSpawn( lastConnected);
    }

    #ifndef _DEMO
    Boe_GlobalSound (G_SoundIndex("sound/misc/events/tut_lift02.mp3", qtrue));
    #endif // not _DEMO

    if(adm && adm->client) {
        trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Eventeams by %s.\n\"", adm->client->pers.cleanName));
    } else  {
        if(aet == qfalse)
        trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7Eventeams.\n\""));
        else
        trap_SendServerCommand(-1, va("print\"^3[Auto Action] ^7Eventeams.\n\""));
    }
}

void EvenTeams_HZ(gentity_t *adm, qboolean aet){
    int i;
    gentity_t *ent, *lastConnected;

    if (level.intermissiontime || level.changemap)
        return;

    // Boe!Man 1/19/11: If teams are locked don't proceed with evening the teams.
    if (level.blueLocked || level.redLocked){
        if((adm && adm->client) || aet == qfalse){
            G_printInfoMessage(adm, "Teams are locked.");
        }
        return;
    }

    if (TeamCount1(TEAM_BLUE) == 1){
        if((adm && adm->client) || aet == qfalse){
            G_printInfoMessage(adm, "Teams are as even as possible.");
        }
        return;
    }

    if (level.nextZombie != -1){
        lastConnected = &g_entities[level.nextZombie];
        level.nextZombie = -1;

        if (!lastConnected || !lastConnected->client || lastConnected->client->sess.team == TEAM_SPECTATOR){
            lastConnected = NULL;
        }
    }

    for (i = 0; i < level.numConnectedClients; i++){
        ent = &g_entities[level.sortedClients[i]];

        if (ent->client->sess.team == TEAM_SPECTATOR){
            continue;
        }

        if (!aet && !G_IsClientDead(ent->client)){
            TossClientItems(ent); // Henk 19/01/11 -> Fixed items not dropping with !et
        }

        ent->client->ps.stats[STAT_WEAPONS] = 0;
        G_StartGhosting(ent);

        if (ent == lastConnected){
            ent->client->sess.team = TEAM_BLUE;
        }else{
            ent->client->sess.team = TEAM_RED;
        }

        if (ent->r.svFlags & SVF_BOT)   {
            char userinfo[MAX_INFO_STRING];
            trap_GetUserinfo(ent->s.number, userinfo, sizeof(userinfo));
            Info_SetValueForKey(userinfo, "team", ent->client->sess.team == TEAM_RED ? "red" : "blue");
            trap_SetUserinfo(ent->s.number, userinfo);
        }

        ent->client->pers.identity = NULL;
        ClientUserinfoChanged(ent->s.number);
        CalculateRanks();

        G_StopFollowing(ent);
        G_StopGhosting(ent);
        trap_UnlinkEntity(ent);
        ClientSpawn(ent);
    }

    #ifndef _DEMO
    Boe_GlobalSound(G_SoundIndex("sound/misc/events/tut_lift02.mp3", qtrue));
    #endif // not _DEMO

    if (adm && adm->client) {
        trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Eventeams by %s.\n\"", adm->client->pers.cleanName));
    }else{
        if (aet == qfalse)
            trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7Eventeams.\n\""));
        else
            trap_SendServerCommand(-1, va("print\"^3[Auto Action] ^7Eventeams.\n\""));
    }
}

/*
================
HZ_clayMore

Claymore think.
================
*/

void HZ_Claymore(gentity_t *ent)
{
    vec3_t          mins, maxs;
    gentity_t       *tent;
    int             entityList[MAX_GENTITIES];
    int             clientsClose[MAX_CLIENTS];
    int             numListedEntities, i, distance;
    int             closestClient = 500, count = 0;

    // Set the boundary of the claymore.
    // Zombies within this boundary can hear the beep of this specific claymore.
    VectorCopy(ent->r.currentOrigin, mins);
    VectorCopy(ent->r.currentOrigin, maxs);
    for (i = 0; i < 3; i++){
        mins[i] -= 300;
        maxs[i] += 300;
    }

    numListedEntities = trap_EntitiesInBox(mins, maxs, entityList, MAX_GENTITIES);
    // Loop through all entities caught in the radius.
    for (i = 0; i < numListedEntities; i++){
        tent = &g_entities[entityList[i]];

        // Make sure the client coming near is a zombie.
        if (tent && tent->client && tent->client->sess.team == TEAM_BLUE){
            distance = (int)(DistanceSquared(ent->r.currentOrigin, tent->r.currentOrigin) / 100);

            // Keep track of the closest client.
            if (distance < closestClient){
                closestClient = distance;
            }
            clientsClose[count++] = tent->s.number;
        }
    }

    if (closestClient < 100){
        // Explode when a zombie gets too near.
        HZ_claymoreExplode(ent);
    }else{
        // Check if we need to let zombies hear the grenade, and let all players see the flashing red dot.
        if (level.time >= ent->speed || (closestClient + closestClient / 2 < ent->up)){
            VectorCopy(ent->r.currentOrigin, mins);
            mins[2] += 5;

            // Play the effect.
            G_PlayEffect(G_EffectIndex("red_dot"), mins, ent->r.currentAngles);
            ent->speed = level.time + closestClient; // When the next effect/sound check is.
            ent->up = closestClient; // We register the previous closest distance this way.

            // Broadcast the sound to players.
            for (i = 0; i < count; i++){
                #ifndef _DEMO
                Boe_ClientSound(&g_entities[clientsClose[i]], G_SoundIndex("sound/misc/events/micro_ding.mp3", qtrue));
                #endif // not _DEMO
            }
        }

        ent->nextthink = level.time + (closestClient < 200) ? closestClient : 200;
    }
}

/*
================
HZ_ClaymoreShoot

Function that gets called when the grenade takes damage.
================
*/

void HZ_ClaymoreShoot(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod, int hitLocation, vec3_t hitDir)
{
    // When someone shoots the grenade, or it's caught in a grende radius, it's safe to blow up.
    if (inflictor && inflictor->s.weapon == WP_L2A2_GRENADE){
        // Just don't blow up if we're throwing a L2A2 on another L2A2..
        return;
    }

    HZ_claymoreExplode(self);
}

/*
================
HZ_ClaymoreShoot

Explode routine.
================
*/

void HZ_claymoreExplode(gentity_t *ent)
{
    vec3_t      dir = { 0, 0, 1 };

    // Toggle the take damage flag, there's no need to inflict further damage on the pickup.
    ent->takedamage = qfalse;

    // Toggle the flags, and make sure the claymore represents the proper grenade.
    ent->s.eFlags |= EF_EXPLODE;
    ent->s.weapon = WP_L2A2_GRENADE;

    // Do the damage.
    G_RadiusDamage(ent->r.currentOrigin, ent->parent, ent->splashDamage, ent->splashRadius, ent->parent, 1, ent->splashMethodOfDeath);
    G_AddEvent(ent, EV_MISSILE_MISS, (DirToByte(dir) << MATERIAL_BITS) | MATERIAL_NONE);

    // And free it after 250msec (and thus after the blast).
    ent->think = G_FreeEntity;
    ent->nextthink = level.time + 250;
}

gentity_t* findLastEnteredPlayer(int highTeam, qboolean scoresAllowed)
{
    gentity_t *ent;
    gentity_t *lastConnected = NULL;
    clientSession_t *sess;
    int lastConnectedTime, i;

    lastConnectedTime = 0;
    for (i = 0; i < level.numConnectedClients; i++ )    {
        ent = &g_entities[level.sortedClients[i]];
        sess = &ent->client->sess;

        if (ent->client->pers.connected != CON_CONNECTED)
            continue;
        if(sess->team != TEAM_RED && sess->team != TEAM_BLUE)
            continue;
        if(sess->team != highTeam)
            continue;
        if(!scoresAllowed && sess->score)
            continue;

        if(ent->client->pers.enterTime > lastConnectedTime) {
            lastConnectedTime = ent->client->pers.enterTime;
            lastConnected = ent;
        }
    }

    return lastConnected;
}

// Boe!Man 9/11/12: Function to preload effects in H&S/H&Z.
void Preload_Effects(void)
{
    // Boe!Man 9/11/12: Handle H&S/H&Z stuff.
    if(current_gametype.value == GT_HS || current_gametype.value == GT_HZ){
        // We'll have to preload the non-map effects in order to use them.
        AddSpawnField("classname", "fx_play_effect");
        AddSpawnField("effect", "flare_blue");
        AddSpawnField("tempent", "1");
        G_SpawnGEntityFromSpawnVars(qtrue);
        G_FreeEntity(&g_entities[level.tempent]);

        AddSpawnField("classname", "fx_play_effect");
        AddSpawnField("effect", "flare_red");
        AddSpawnField("tempent", "1");
        G_SpawnGEntityFromSpawnVars(qtrue);
        G_FreeEntity(&g_entities[level.tempent]);

        AddSpawnField("classname", "fx_play_effect");
        AddSpawnField("effect", "arm2smallsmoke");
        AddSpawnField("tempent", "1");
        G_SpawnGEntityFromSpawnVars(qtrue);
        G_FreeEntity(&g_entities[level.tempent]);

        AddSpawnField("classname", "fx_play_effect");
        AddSpawnField("effect", "levels/air4_toxic_smoke");
        AddSpawnField("tempent", "1");
        G_SpawnGEntityFromSpawnVars(qtrue);
        G_FreeEntity(&g_entities[level.tempent]);

        AddSpawnField("classname", "fx_play_effect");
        AddSpawnField("effect", "misc/exclaimation");
        AddSpawnField("tempent", "1");
        G_SpawnGEntityFromSpawnVars(qtrue);
        G_FreeEntity(&g_entities[level.tempent]);

        AddSpawnField("classname", "fx_play_effect");
        AddSpawnField("effect", "red_dot");
        AddSpawnField("tempent", "1");
        G_SpawnGEntityFromSpawnVars(qtrue);
        G_FreeEntity(&g_entities[level.tempent]);
    }

    AddSpawnField("classname", "fx_play_effect");
    AddSpawnField("effect", "misc/electrical");
    AddSpawnField("tempent", "1");
    G_SpawnGEntityFromSpawnVars(qtrue);
    G_FreeEntity(&g_entities[level.tempent]);

    AddSpawnField("classname", "fx_play_effect");
    AddSpawnField("effect", "misc/electrical");
    AddSpawnField("tempent", "1");
    G_SpawnGEntityFromSpawnVars(qtrue);
    G_FreeEntity(&g_entities[level.tempent]);

    AddSpawnField("classname", "fx_play_effect");
    AddSpawnField("effect", "gen_tendril1");
    AddSpawnField("tempent", "1");
    G_SpawnGEntityFromSpawnVars(qtrue);
    G_FreeEntity(&g_entities[level.tempent]);

    AddSpawnField("classname", "fx_play_effect");
    AddSpawnField("effect", "levels/shop7_toxiic_explosion");
    AddSpawnField("tempent", "1");
    G_SpawnGEntityFromSpawnVars(qtrue);
    G_FreeEntity(&g_entities[level.tempent]);

    AddSpawnField("classname", "fx_play_effect");
    AddSpawnField("effect", "effects/explosions/col9_boat_explosion");
    AddSpawnField("tempent", "1");
    G_SpawnGEntityFromSpawnVars(qtrue);
    G_FreeEntity(&g_entities[level.tempent]);

    AddSpawnField("classname", "fx_play_effect");
    AddSpawnField("effect", "jon_sam_trail");
    AddSpawnField("tempent", "1");
    G_SpawnGEntityFromSpawnVars(qtrue);
    G_FreeEntity(&g_entities[level.tempent]);

    AddSpawnField("classname", "fx_play_effect");
    AddSpawnField("effect", "effects/levels/hk6_spark_shower");
    AddSpawnField("tempent", "1");
    G_SpawnGEntityFromSpawnVars(qtrue);
    G_FreeEntity(&g_entities[level.tempent]);

    AddSpawnField("classname", "fx_play_effect");
    AddSpawnField("effect", "fire/blue_target_flame");
    AddSpawnField("tempent", "1");
    G_SpawnGEntityFromSpawnVars(qtrue);
    G_FreeEntity(&g_entities[level.tempent]);

    // Boe!Man 4/15/13: Effect for the accelerator.
    AddSpawnField("classname", "fx_play_effect");
    AddSpawnField("effect", "explosions/phosphorus_trail");
    AddSpawnField("tempent", "1");
    G_SpawnGEntityFromSpawnVars(qtrue);
    G_FreeEntity(&g_entities[level.tempent]);

    level.MM1Flare = -1;
    level.M4Flare = -1;
    level.RPGFlare = -1;
}
// End
