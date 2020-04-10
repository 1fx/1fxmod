/*
===========================================================================
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
Copyright (C) 2003 - 2007, ROCmod contributors
Copyright (C) 2015 - 2020, Ane-Jouke Schat (Boe!Man)
Copyright (C) 2015 - 2020, Jordi Epema (Henkie)

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
// rocmod_functions.c - Functions specific for ROCmod.

//==================================================================

#ifdef _GOLD
#include "g_local.h"
#include "boe_local.h"

/*
=============
ROCmod_verifyClient
=============
*/

void ROCmod_verifyClient(gentity_t *ent, int clientNum)
{
    char    buffer[MAX_TOKEN_CHARS];
    int     value;

    if (trap_Argc() == 1)
        return;

    trap_Argv(1, buffer, sizeof(buffer));
    value = atoi(buffer);

    if (value < 2012) // 2.1c
        return;

    #ifdef _DEBUG
    Com_Printf("ROCmod 2.1c verified on client #%d\n", clientNum);
    #endif // _DEBUG
    ent->client->sess.rocModClient = qtrue;
    strncpy(ent->client->sess.strClient, "2.1c", sizeof(ent->client->sess.strClient));
}

/*
==================
ROCmod_clientUpdate
==================
*/
void ROCmod_clientUpdate(gentity_t *ent, int clientNum)
{
    char    buffer[MAX_TOKEN_CHARS];

    if (trap_Argc() == 1)
        return;

    trap_Argv(1, buffer, sizeof(buffer));
    ent->client->sess.rocExtraFeatures = atoi(buffer);

    trap_SendServerCommand(ent - g_entities, va("efack %i", ent->client->sess.rocExtraFeatures));
    if (ent->client->sess.rocExtraFeatures & ROC_TEAMINFO)
    {
        ROCmod_sendExtraTeamInfo(ent);
    }
}

/*
==================
ROCmod_sendExtraTeamInfo
==================
*/
void ROCmod_sendExtraTeamInfo(gentity_t *ent)
{
    char        entry[64];
    char        red[1024];
    char        blue[1024];
    int         redlength;
    int         bluelength;
    int         entrylength;
    int         i, j;
    int         redcount;
    int         bluecount;
    gclient_t   *cl;
    gentity_t   *loc;

    // Don't update them if they don't want it
    if (ent && !(ent->client->sess.rocExtraFeatures & ROC_TEAMINFO))
        return;

    // send the latest information on all clients
    red[0] = 0;
    redlength = 0;
    redcount = 0;
    blue[0] = 0;
    bluelength = 0;
    bluecount = 0;

    for (i = 0; i < level.numConnectedClients; i++)
    {
        cl = &level.clients[level.sortedClients[i]];

        if (cl->pers.connected != CON_CONNECTED || cl->ps.stats[STAT_HEALTH] <= 0)
            continue;

        Com_sprintf(entry, sizeof(entry), " %i", level.sortedClients[i]);
        entrylength = strlen(entry);

        // Get location.
        loc = Team_GetLocation(&g_entities[level.sortedClients[i]]);

        // Add all extra information from the client.
        Com_sprintf(entry + entrylength, sizeof(entry) - entrylength, "h%i a%i w%i l%i g%i",
            cl->ps.stats[STAT_HEALTH],
            cl->ps.stats[STAT_ARMOR],
            g_entities[level.sortedClients[i]].s.weapon,
            (loc != NULL) ? loc->health : -1,
            cl->sess.ghost);
        entrylength = strlen(entry);

        j = strlen(entry);
        if (cl->sess.team == TEAM_RED)
        {
            if (redlength + j > 1022)
                break;

            strcpy(red + redlength, entry);
            redlength += j;
            redcount++;
        }
        else if (cl->sess.team == TEAM_BLUE)
        {
            if (bluelength + j > 1022)
                break;

            strcpy(blue + bluelength, entry);
            bluelength += j;
            bluecount++;
        }
    }

    if (ent)
    {
        if (ent->client->sess.team == TEAM_RED)
            trap_SendServerCommand(ent - g_entities, va("eti2 %i%s", redcount, red));
        else if (ent->client->sess.team == TEAM_BLUE)
            trap_SendServerCommand(ent - g_entities, va("eti2 %i%s", bluecount, blue));
    }
    else
    {
        for (i = 0; i < level.numConnectedClients; i++)
        {
            cl = &level.clients[level.sortedClients[i]];

            if (cl->pers.connected != CON_CONNECTED)
                continue;

            if (cl->sess.rocExtraFeatures & ROC_TEAMINFO) {
                if (cl->sess.team == TEAM_RED && redcount)
                    trap_SendServerCommand(level.sortedClients[i], va("eti2 %i%s", redcount, red));
                else if (cl->sess.team == TEAM_BLUE && bluecount)
                    trap_SendServerCommand(level.sortedClients[i], va("eti2 %i%s", bluecount, blue));
            }
        }
    }
}

/*
=============
ROCmod_sendBestPlayerStats
=============
*/

void ROCmod_sendBestPlayerStats(void)
{
    /*
    Structure:
    [0] - bestKills
    [1] - bestAccuracy
    [2] - bestHeadshots
    [3] - bestItemCaps
    [4] - bestRatio
    [5] - bestValue
    [6] - bestKillRate
    [7] - bestKillSpree
    [8] - bestNadeKills
    [9] - bestMeleeKills
    [10] - bestItemDefends
    */

    char            bestScoreNames[11][MAX_NETNAME];
    int             bestScores[11];
    int             bestKills = 0, bestHeadshots = 0, bestItemCaps = 0;
    int             bestValue = 0, bestKillRate = 0, bestKillSpree = 0, bestNadeKills = 0;
    int             bestMeleeKills = 0, bestItemDefends = 0;
    float           bestAccuracy = 0, bestRatio = 0;

    int             i, v;
    gentity_t       *ent;
    statinfo_t     *stat;

    // Initialize the array properly.
    for (i = 0; i < 11; i++) {
        bestScores[i] = -1;
    }

    // Calculate the scores.
    for (i = 0; i < level.maxclients; i++)
    {
        ent = g_entities + i;
        if (!ent->inuse)
        {
            continue;
        }

        if (ent->client->pers.connected != CON_CONNECTED)
        {
            continue;
        }

        stat = &ent->client->pers.statinfo;

        if (stat->kills > bestKills) {
            bestKills = stat->kills;
            bestScores[0] = i;
            Q_strncpyz(bestScoreNames[0], ent->client->pers.netname, MAX_NETNAME);
        }
        if (stat->accuracy > bestAccuracy) {
            bestAccuracy = stat->accuracy;
            bestScores[1] = i;
            Q_strncpyz(bestScoreNames[1], ent->client->pers.netname, MAX_NETNAME);
        }
        if (stat->headShotKills > bestHeadshots) {
            bestHeadshots = stat->headShotKills;
            bestScores[2] = i;
            Q_strncpyz(bestScoreNames[2], ent->client->pers.netname, MAX_NETNAME);
        }
        if (stat->itemCaptures > bestItemCaps) {
            bestItemCaps = stat->itemCaptures;
            bestScores[3] = i;
            Q_strncpyz(bestScoreNames[3], ent->client->pers.netname, MAX_NETNAME);
        }
        if (stat->ratio > bestRatio) {
            bestRatio = stat->ratio;
            bestScores[4] = i;
            Q_strncpyz(bestScoreNames[4], ent->client->pers.netname, MAX_NETNAME);
        }
        stat->overallScore = ent->client->sess.score + (int)(100 * (stat->accuracy + stat->ratio)) + (stat->damageDone - stat->damageTaken);
        if (stat->overallScore > bestValue) {
            bestValue = stat->overallScore;
            bestScores[5] = i;
            Q_strncpyz(bestScoreNames[5], ent->client->pers.netname, MAX_NETNAME);
        }

        v = (level.time - ent->client->pers.enterTime) / 60000;
        if (v && (stat->kills * (60 / v)) > bestKillRate)
        {
            bestKillRate = stat->kills * (60 / v);
            bestScores[6] = i;
            Q_strncpyz(bestScoreNames[6], ent->client->pers.netname, MAX_NETNAME);
        }

        // Check if the current killing spree is better than his old one.
        if (stat->killsinarow > stat->bestKillsInARow)
            stat->bestKillsInARow = stat->killsinarow;

        if (stat->bestKillsInARow > bestKillSpree) {
            bestKillSpree = stat->bestKillsInARow;
            bestScores[7] = i;
            Q_strncpyz(bestScoreNames[7], ent->client->pers.netname, MAX_NETNAME);
        }

        if (stat->explosiveKills > bestNadeKills) {
            bestNadeKills = stat->explosiveKills;
            bestScores[8] = i;
            Q_strncpyz(bestScoreNames[8], ent->client->pers.netname, MAX_NETNAME);
        }
        if (stat->knifeKills > bestMeleeKills) {
            bestMeleeKills = stat->knifeKills;
            bestScores[9] = i;
            Q_strncpyz(bestScoreNames[9], ent->client->pers.netname, MAX_NETNAME);
        }
        if (stat->itemDefends > bestItemDefends) {
            bestItemDefends = stat->itemDefends;
            bestScores[10] = i;
            Q_strncpyz(bestScoreNames[10], ent->client->pers.netname, MAX_NETNAME);
        }
    }

    // Send the scores.
    for (i = 0; i <  level.numConnectedClients; i++)
    {
        ent = &g_entities[level.sortedClients[i]];

        if (ent->r.svFlags & SVF_BOT)
            continue;

        if (ent->client->sess.rocModClient) {
            DeathmatchScoreboardMessage(ent);
            trap_SendServerCommand(ent - g_entities, va("playerstats %d %d %d %d %d %d %d %d %d %d %d", bestScores[0], bestScores[1], bestScores[2], bestScores[3], bestScores[4], bestScores[5], bestScores[6], bestScores[7], bestScores[8], bestScores[9], bestScores[10]));
        }
    }
}
#endif // _GOLD
