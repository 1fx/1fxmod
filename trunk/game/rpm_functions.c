// Copyright (C) 2009-2015 - Boe!Man, Henkie.
//
// rpm_functions.c - Functions specific for RPM.

//==================================================================

#ifndef _GOLD
#include "g_local.h"
#include "boe_local.h"

// Henk 07/04/10 -> Copied from RPM to make scoreboard from RPM client working
/*
=================
RPM_UpdateTMI

//Used to only send teammate info hence "TMI" but I have decided to
//send all player infos so specs can see health etc.. of both teams
//and its also kinda cool in deathmatches to see the health of the enemy
=================
*/
void RPM_UpdateTMI(void)
{
    int         i, j;
    char        entry[1024];
    char        infoString[2048];
    int         infoStringLength;
    gclient_t   *cl;
    int         numAdded, location;
    gentity_t   *bestLoc;
    int         adm = 0;
    char        *s;
    char        userinfo[MAX_INFO_STRING];
    int         damage;
    char        *string;
    int         UpdateTime;
/*  if (!level.gametypeData->teams)
    {   // only bother if a league game with teams
        return;
    }
*/
    // if we are waiting for the level to restart, do nothing, even if forced
    if (level.restarted){
        return;
    }

    if(level.pause)
        return;

    if(level.numConnectedClients >= 0 && level.numConnectedClients <= 10)
        UpdateTime = 2000;
    else if(level.numConnectedClients >= 11 && level.numConnectedClients <= 20)
        UpdateTime = 3000;
    else if(level.numConnectedClients > 20)
        UpdateTime = 4000;
    else
        UpdateTime = 2000;

    if (level.time - level.lastTMIupdate < UpdateTime) // Henk 06/04/10 -> Increase to reduce lagg
    {
        return;
    }

    level.lastTMIupdate = level.time;

    entry[0] = 0;
    infoString[0] = 0;
    infoStringLength = 0;
    numAdded = 0;

    for (i = 0; i < level.numConnectedClients; i++)
    {
        cl = &level.clients[level.sortedClients[i]];

        //IsClientMuted(&g_entities[cl->ps.clientNum], qfalse);
        IsClientMuted(&g_entities[level.sortedClients[i]], qfalse);
        if (G_IsClientSpectating(cl))// || G_IsClientDead (cl))
        {
            continue;
        }

        bestLoc = Team_GetLocation(&g_entities[level.sortedClients[i]]);

        location = 0;
        if (bestLoc)
        {
            location = bestLoc->health;
        }
        // Henk 06/04/10 -> Check what view person is in
        trap_GetUserinfo( g_entities[level.sortedClients[i]].s.number, userinfo, sizeof( userinfo ) );
        if(g_entities[level.sortedClients[i]].client->sess.rpmClient > 0.7){
        s = Info_ValueForKey( userinfo, "cg_thirdperson" );
        cl->sess.thirdperson = atoi(s);
        if(cl->sess.thirdperson >= 1){
            cl->sess.thirdperson = 1;
        }else{
            cl->sess.thirdperson = 0;
        }
        }else{
            cl->sess.thirdperson = 2;
        }
            adm = cl->sess.admin;
            damage = cl->pers.statinfo.damageDone;
            if(damage < 100){
                string = va("%i", cl->ps.weapon);
            }else{
                damage = FormatDamage(damage);
                if(cl->ps.weapon >= 10){ // fix for large weapon string = damage *10 on scoreboard
                    damage = damage/10;
                }
                string = va("%i%i", damage, cl->ps.weapon);
            }

            //if(cl->sess.rpmClient > 0.6){
            Com_sprintf (entry, sizeof(entry),
                " %i %i %i %i %i %i %s %i %i",
                level.sortedClients[i],
                cl->ps.stats[STAT_HEALTH],
                cl->ps.stats[STAT_ARMOR],
                location,
                cl->sess.thirdperson, // 1 = third | 0 = first | 2 = n/a, aka no client
                adm,
                string,
                //cl->ps.weapon, //cl->pers.statinfo.damageDone
                cl->sess.mute,      // Confirmed
                cl->sess.clanMember // Confirmed
                );
            /*}else if(cl->sess.proClient > 4.0){
            Com_sprintf (entry, sizeof(entry),
                " %i %i %i %i %i %i %s %i %i",
                level.sortedClients[i],
                cl->ps.stats[STAT_HEALTH],
                cl->ps.stats[STAT_ARMOR],
                cl->ps.weapon,
                location,
                adm
                );
            }*/

        j = strlen(entry);

        if (infoStringLength + j > 2046)
        {
            break;
        }
        strcpy (infoString + infoStringLength, entry);
        infoStringLength += j;
        numAdded++;
    }
    for (i = 0 ; i < level.numConnectedClients; i++)
    {
        cl = &level.clients[level.sortedClients[i]];

        // Boe!Man 6/5/13: Don't send if the player is timing out..
        if(cl->ps.ping >= 999)
        {
            continue;
        }

//RxCxW - 1.20.2005 - #scoreboard #Version compatiblity
        if(cl->sess.rpmClient > 0.6)
            trap_SendServerCommand(level.sortedClients[i], va("tmi %i%s", numAdded, infoString));
        else if(cl->sess.proClient > 4.0)
            trap_SendServerCommand(level.sortedClients[i], va("tmi %i%s", numAdded, infoString));
    }
}

/*
=============
RPM_Awards
=============
*/
void RPM_Awards(void)
{
    static int overallScore = 0, headshots = 0, damage = 0, explosiveKills = 0, knifeKills = 0;
    static float  accuracy = 0, ratio = 0;
    unsigned int i, numPlayers = 0;
    //I changed this to an integer so players who connect
    //faster than the rest don't bump up the avg. time
    //float avgtime = 0.0, playerTime = 0.0;
    int avgtime = 0, playerTime = 0;
    statinfo_t     *stat;
    // Boe!Man 2/20/13: No need to be static again, the struct already is.
    gentity_t *bestOverall = NULL, *headshooter = NULL, *killer = NULL;
    gentity_t *accurate = NULL, *bestRatio = NULL, *explosive = NULL, *knifer = NULL;
    gentity_t *ent;

    /*
    Boe!Man 2/20/13: Oh noooess this is crappy. ;_; But there's no other fix for dynlib it seems.
    Structure:
    [0] - bestOverall
    [1] - headshooter
    [2] - killer
    [3] - accurate
    [4] - bestRatio
    [5] - explosive
    [6] - knifer
    */
    typedef struct {
        char name[MAX_NETNAME];
        int  number;
    }scores_t;
    static scores_t     bestScores[7];

    if(!level.awardTime)
    {
        //find the average time player by all connected clients
        for (i=0; i < level.maxclients ; i++)
        {
            ent = g_entities + i;
            if (!ent->inuse)
            {
                continue;
            }
            numPlayers++;
            avgtime += ((level.time - ent->client->pers.enterTime) - ent->client->sess.totalSpectatorTime) / 60000;
        }
        //incase timelimit runs out with nobody in the server
        //I'll use an "if" here
        if(numPlayers)
        {
            avgtime /= numPlayers;
            //Com_Printf("^3Averagetime %d\n", avgtime);
        }

        for (i = 0; i < level.maxclients ; i++)
        {
            ent = g_entities + i;
            if (!ent->inuse)
            {
                continue;
            }

            stat = &ent->client->pers.statinfo;

            playerTime = ((level.time - ent->client->pers.enterTime) - ent->client->sess.totalSpectatorTime) / 60000;
            //Com_Printf("%s playtime %d, Level time: %d, Enter Time: %d SPECTIME: %d\n", ent->client->pers.netname, playerTime, level.time, ent->client->pers.enterTime, ent->client->sess.totalSpectatorTime);

            //RxCxW - 1.20.2005 - #Version 0.5 compatible. Right?
            if(ent->client->sess.rpmClient >= 0.5)
//          if(ent->client->sess.rpmClient >= RPM_VERSION)
            {
                trap_SendServerCommand( i, va("stats %i %i %i %i %.2f %i %i %i %i",
                playerTime,
                (((level.time - ent->client->pers.enterTime) - ent->client->sess.totalSpectatorTime) % 60000) / 1000,
                stat->damageDone,
                stat->damageTaken,
                stat->ratio,
                stat->shotcount,
                stat->hitcount,
                stat->explosiveKills,
                stat->knifeKills));
            }

            //this will remove the SPECTATOR Press ESC etc.. from the specs screen
            if(ent->client->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR)
            {
                ent->client->ps.persistant[PERS_TEAM] = TEAM_FREE;
            }

            //Make sure they can't move
            ent->client->ps.pm_type = PM_FREEZE;

            //This will remove the HUD icons etc.. from the players screen
            ent->client->ps.stats[STAT_HEALTH] = -1;

            stat->overallScore = ent->client->sess.score + (int)(100 * (stat->accuracy + stat->ratio)) + (stat->damageDone - stat->damageTaken);

            if(stat->overallScore > overallScore)
            {
                overallScore = stat->overallScore;
                bestOverall = ent;
            }
            else if(stat->overallScore == overallScore && overallScore)
            {
                //if they got the same amout of points in less time
                //make them mvp
                if(ent->client->pers.enterTime > bestOverall->client->pers.enterTime)
                {
                    overallScore = stat->overallScore;
                    bestOverall = ent;
                }
            }

            if(stat->headShotKills > headshots)
            {
                headshots = stat->headShotKills;
                headshooter = ent;
            }
            else if(stat->headShotKills == headshots && headshots)
            {
                if(ent->client->pers.enterTime > headshooter->client->pers.enterTime)
                {
                    headshots = stat->headShotKills;
                    headshooter = ent;
                }
            }

            if(stat->damageDone > damage)
            {
                damage = stat->damageDone;
                killer = ent;
            }
            else if(stat->damageDone == damage && damage)
            {
                if(ent->client->pers.enterTime > killer->client->pers.enterTime)
                {
                    damage = stat->damageDone;
                    killer = ent;
                }
            }

            if(stat->explosiveKills > explosiveKills)
            {
                explosiveKills = stat->explosiveKills;
                explosive = ent;
            }
            else if(stat->explosiveKills == explosiveKills && explosiveKills)
            {
                if(ent->client->pers.enterTime > explosive->client->pers.enterTime)
                {
                    explosiveKills = stat->explosiveKills;
                    explosive = ent;
                }
            }

            if(stat->knifeKills > knifeKills )
            {
                knifeKills = stat->knifeKills;
                knifer = ent;
            }
            else if(stat->knifeKills == knifeKills && knifeKills)
            {
                if(ent->client->pers.enterTime > knifer->client->pers.enterTime)
                {
                    knifeKills = stat->knifeKills;
                    knifer = ent;
                }
            }

            //The awards below here are time sensitive so players
            //with 1/0 for ratio etc.. dont get the award
            if(playerTime < avgtime)
            {
                continue;
            }

            if(stat->accuracy > accuracy)
            {
                accuracy = stat->accuracy;
                accurate = ent;
            }
            else if(stat->accuracy == accuracy && accuracy)
            {
                //if this player held the accuracy longer give him the award
                if(ent->client->pers.enterTime < accurate->client->pers.enterTime)
                {
                    accuracy = stat->accuracy;
                    accurate = ent;
                }
            }

            if(stat->ratio > ratio)
            {
                ratio = stat->ratio;
                bestRatio = ent;
            }
            else if(stat->ratio == ratio && ratio)
            {
                if(ent->client->pers.enterTime < bestRatio->client->pers.enterTime)
                {
                    ratio = stat->ratio;
                    bestRatio = ent;
                }
            }
        }

        // Boe!Man 2/20/13: Put data in struct. Nasty piece of code but effective.
        if(bestOverall){
            Q_strncpyz(bestScores[0].name, g_entities[bestOverall->s.number].client->pers.netname, sizeof(bestScores[0].name));
            bestScores[0].number = bestOverall->s.number;
        }else{
            strcpy(bestScores[0].name, "None");
            bestScores[0].number = -1;
        }
        if(headshooter){
            Q_strncpyz(bestScores[1].name, g_entities[headshooter->s.number].client->pers.netname, sizeof(bestScores[1].name));
            bestScores[1].number = headshooter->s.number;
        }else{
            strcpy(bestScores[1].name, "None");
            bestScores[1].number = -1;
        }
        if(killer){
            Q_strncpyz(bestScores[2].name, g_entities[killer->s.number].client->pers.netname, sizeof(bestScores[2].name));
            bestScores[2].number = killer->s.number;
        }else{
            strcpy(bestScores[2].name, "None");
            bestScores[2].number = -1;
        }
        if(accurate){
            Q_strncpyz(bestScores[3].name, g_entities[accurate->s.number].client->pers.netname, sizeof(bestScores[3].name));
            bestScores[3].number = accurate->s.number;
        }else{
            strcpy(bestScores[3].name, "None");
            bestScores[3].number = -1;
        }
        if(bestRatio){
            Q_strncpyz(bestScores[4].name, g_entities[bestRatio->s.number].client->pers.netname, sizeof(bestScores[4].name));
            bestScores[4].number = bestRatio->s.number;
        }else{
            strcpy(bestScores[4].name, "None");
            bestScores[4].number = -1;
        }
        if(explosive){
            Q_strncpyz(bestScores[5].name, g_entities[explosive->s.number].client->pers.netname, sizeof(bestScores[5].name));
            bestScores[5].number = explosive->s.number;
        }else{
            strcpy(bestScores[5].name, "None");
            bestScores[5].number = -1;
        }
        if(knifer){
            Q_strncpyz(bestScores[6].name, g_entities[knifer->s.number].client->pers.netname, sizeof(bestScores[6].name));
            bestScores[6].number = knifer->s.number;
        }else{
            strcpy(bestScores[6].name, "None");
            bestScores[6].number = -1;
        }
    }

    for (i = 0; i < level.maxclients ; i++)
    {
        ent = g_entities + i;

        if (!ent->inuse)
        {
            continue;
        }

        ///RxCxW - 01.12.06 - 03:03pm - dont send to bots
        if (ent->r.svFlags & SVF_BOT)
            continue;
        ///End  - 01.12.06 - 03:03pm

        if(ent->client->sess.rpmClient < 0.5)
        {
            G_Broadcast(va("^1Best Overall: ^7%s - %i\n^1Headshots: ^7%s - %i\n^1Killer: ^7%s - %i\n^1Accurate: ^7%s - %.2f percent\n^1Best Ratio: ^7%s - %.2f\n^1Nades: ^7%s - %i detonated\n^1Knifer: ^7%s - %i shanked",
                bestScores[0].name,
                overallScore,
                bestScores[1].name,
                headshots,
                bestScores[2].name,
                damage,
                bestScores[3].name,
                accuracy,
                bestScores[4].name,
                ratio,
                bestScores[5].name,
                explosiveKills,
                bestScores[6].name,
                knifeKills ), BROADCAST_AWARDS, ent);
            continue;
        }
        if(!level.awardTime)
        {
            trap_SendServerCommand( i, va("awards %i %i %i %i %i %i %i %.2f %i %.2f %i %i %i %i",
                bestScores[0].number,
                overallScore,
                bestScores[1].number,
                headshots,
                bestScores[2].number,
                damage,
                bestScores[3].number,
                accuracy,
                bestScores[4].number,
                ratio,
                bestScores[5].number,
                explosiveKills,
                bestScores[6].number,
                knifeKills
                ));
        }
    }
}

#endif // not _GOLD
