/*
===========================================================================
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
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

/*****************************************************************************
 * name:        ai_main.c
 *
 * desc:        Quake3 bot AI
 *
 * $Archive: /MissionPack/code/game/ai_main.c $
 * $Author: chris $
 * $Revision: 1.1.1.1 $
 * $Modtime: 6/06/01 1:11p $
 * $Date: 2004/12/29 14:26:27 $
 *
 *****************************************************************************/


#include "g_local.h"
#include "q_shared.h"
#include "botlib.h"     //bot lib interface
#include "be_aas.h"
#include "be_ea.h"
#include "be_ai_char.h"
#include "be_ai_chat.h"
#include "be_ai_gen.h"
#include "be_ai_goal.h"
#include "be_ai_move.h"
#include "be_ai_weap.h"
//
#include "ai_main.h"
//
#include "chars.h"
#include "inv.h"
#include "syn.h"

/*
#define BOT_CTF_DEBUG   1
*/

#ifndef MAX_PATH
#define MAX_PATH        144
#endif

#define BOT_THINK_TIME  0

//bot states
bot_state_t *botstates[MAX_CLIENTS];
//number of bots
int numbots;
//floating point time
float floattime;
//time to do a regular update
float regularupdate_time;
//

boteventtracker_t gBotEventTracker[MAX_CLIENTS];

//rww - new bot cvars..
#ifdef _DEBUG
vmCvar_t bot_debugmessages;
#endif

vmCvar_t bot_attachments;
vmCvar_t bot_camp;
vmCvar_t bot_pause;

vmCvar_t bot_wp_info;
vmCvar_t bot_wp_edit;
vmCvar_t bot_wp_clearweight;
vmCvar_t bot_wp_distconnect;
vmCvar_t bot_wp_visconnect;
//end rww

wpobject_t *flagRed;
wpobject_t *oFlagRed;
wpobject_t *flagBlue;
wpobject_t *oFlagBlue;

gentity_t *eFlagRed;
gentity_t *droppedRedFlag;
gentity_t *eFlagBlue;
gentity_t *droppedBlueFlag;

char *ctfStateNames[] = {
    "CTFSTATE_NONE",
    "CTFSTATE_ATTACKER",
    "CTFSTATE_DEFENDER",
    "CTFSTATE_RETRIEVAL",
    "CTFSTATE_GUARDCARRIER",
    "CTFSTATE_GETFLAGHOME",
    "CTFSTATE_MAXCTFSTATES"
};

char *ctfStateDescriptions[] = {
    "I'm not occupied",
    "I'm attacking the enemy's base",
    "I'm defending our base",
    "I'm getting our flag back",
    "I'm escorting our flag carrier",
    "I've got the enemy's flag"
};

char *teamplayStateDescriptions[] = {
    "I'm not occupied",
    "I'm following my squad commander",
    "I'm assisting my commanding",
    "I'm attempting to regroup and form a new squad"
};

void BotStraightTPOrderCheck(gentity_t *ent, int ordernum, bot_state_t *bs)
{
    switch (ordernum)
    {
    case 0:
        if (bs->squadLeader == ent)
        {
            bs->teamplayState = 0;
            bs->squadLeader = NULL;
        }
        break;
    case TEAMPLAYSTATE_FOLLOWING:
        bs->teamplayState = ordernum;
        bs->isSquadLeader = 0;
        bs->squadLeader = ent;
        bs->wpDestSwitchTime = 0;
        break;
    case TEAMPLAYSTATE_ASSISTING:
        bs->teamplayState = ordernum;
        bs->isSquadLeader = 0;
        bs->squadLeader = ent;
        bs->wpDestSwitchTime = 0;
        break;
    default:
        bs->teamplayState = ordernum;
        break;
    }
}

void BotReportStatus(bot_state_t *bs)
{
    if ( level.gametypeData->teams )
    {
        trap_EA_SayTeam(bs->client, teamplayStateDescriptions[bs->teamplayState]);
    }
}

void BotOrder(gentity_t *ent, int clientnum, int ordernum)
{
    int stateMin = 0;
    int stateMax = 0;
    int i = 0;

    if (!ent || !ent->client)
    {
        return;
    }

    if (clientnum != -1 && !botstates[clientnum])
    {
        return;
    }

    if (clientnum != -1 && !OnSameTeam(ent, &g_entities[clientnum]))
    {
        return;
    }

    if ( !level.gametypeData->teams )
    {
        return;
    }

/*
    if (level.gametype == GT_CTF)
    {
        stateMin = CTFSTATE_NONE;
        stateMax = CTFSTATE_MAXCTFSTATES;
    }
    else if (level.gametype == GT_TDM)
    {
        stateMin = TEAMPLAYSTATE_NONE;
        stateMax = TEAMPLAYSTATE_MAXTPSTATES;
    }
*/

    if ((ordernum < stateMin && ordernum != -1) || ordernum >= stateMax)
    {
        return;
    }

    if (clientnum != -1)
    {
        if (ordernum == -1)
        {
            BotReportStatus(botstates[clientnum]);
        }
        else
        {
            BotStraightTPOrderCheck(ent, ordernum, botstates[clientnum]);
            botstates[clientnum]->state_Forced = ordernum;
            botstates[clientnum]->chatObject = ent;
            botstates[clientnum]->chatAltObject = NULL;
            if (BotDoChat(botstates[clientnum], "OrderAccepted", 1))
            {
                botstates[clientnum]->chatTeam = 1;
            }
        }
    }
    else
    {
        while (i < MAX_CLIENTS)
        {
            if (botstates[i] && OnSameTeam(ent, &g_entities[i]))
            {
                if (ordernum == -1)
                {
                    BotReportStatus(botstates[i]);
                }
                else
                {
                    BotStraightTPOrderCheck(ent, ordernum, botstates[i]);
                    botstates[i]->state_Forced = ordernum;
                    botstates[i]->chatObject = ent;
                    botstates[i]->chatAltObject = NULL;
                    if (BotDoChat(botstates[i], "OrderAccepted", 0))
                    {
                        botstates[i]->chatTeam = 1;
                    }
                }
            }

            i++;
        }
    }
}

int BotGetWeaponRange(bot_state_t *bs);
int PassLovedOneCheck(bot_state_t *bs, gentity_t *ent);

void ExitLevel( void );

void QDECL BotAI_Print(int type, char *fmt, ...) { return; }

int IsTeamplay(void)
{
    return level.gametypeData->teams;
}

/*
==================
BotAI_GetClientState
==================
*/
int BotAI_GetClientState( int clientNum, playerState_t *state ) {
    gentity_t   *ent;

    ent = &g_entities[clientNum];
    if ( !ent->inuse ) {
        return qfalse;
    }
    if ( !ent->client ) {
        return qfalse;
    }

    memcpy( state, &ent->client->ps, sizeof(playerState_t) );
    return qtrue;
}

/*
==================
BotAI_GetEntityState
==================
*/
int BotAI_GetEntityState( int entityNum, entityState_t *state ) {
    gentity_t   *ent;

    ent = &g_entities[entityNum];
    memset( state, 0, sizeof(entityState_t) );
    if (!ent->inuse) return qfalse;
    if (!ent->r.linked) return qfalse;
    if (ent->r.svFlags & SVF_NOCLIENT) return qfalse;
    memcpy( state, &ent->s, sizeof(entityState_t) );
    return qtrue;
}

/*
==================
BotAI_GetSnapshotEntity
==================
*/
int BotAI_GetSnapshotEntity( int clientNum, int sequence, entityState_t *state ) {
    int     entNum;

    entNum = trap_BotGetSnapshotEntity( clientNum, sequence );
    if ( entNum == -1 ) {
        memset(state, 0, sizeof(entityState_t));
        return -1;
    }

    BotAI_GetEntityState( entNum, state );

    return sequence + 1;
}

/*
==============
BotEntityInfo
==============
*/
void BotEntityInfo(int entnum, aas_entityinfo_t *info) {
    trap_AAS_EntityInfo(entnum, info);
}

/*
==============
NumBots
==============
*/
int NumBots(void) {
    return numbots;
}

/*
==============
AngleDifference
==============
*/
float AngleDifference(float ang1, float ang2) {
    float diff;

    diff = ang1 - ang2;
    if (ang1 > ang2) {
        if (diff > 180.0) diff -= 360.0;
    }
    else {
        if (diff < -180.0) diff += 360.0;
    }
    return diff;
}

/*
==============
BotChangeViewAngle
==============
*/
float BotChangeViewAngle(float angle, float ideal_angle, float speed) {
    float move;

    angle = AngleMod(angle);
    ideal_angle = AngleMod(ideal_angle);
    if (angle == ideal_angle) return angle;
    move = ideal_angle - angle;
    if (ideal_angle > angle) {
        if (move > 180.0) move -= 360.0;
    }
    else {
        if (move < -180.0) move += 360.0;
    }
    if (move > 0) {
        if (move > speed) move = speed;
    }
    else {
        if (move < -speed) move = -speed;
    }
    return AngleMod(angle + move);
}

/*
==============
BotChangeViewAngles
==============
*/
void BotChangeViewAngles(bot_state_t *bs, float thinktime) {
    float diff, factor, maxchange, anglespeed, disired_speed;
    int i;

    if (bs->ideal_viewangles[PITCH] > 180) bs->ideal_viewangles[PITCH] -= 360;

    if (bs->currentEnemy && bs->frame_Enemy_Vis)
    {
        factor = bs->skills.turnspeed_combat*bs->settings.skill;
    }
    else
    {
        factor = bs->skills.turnspeed;
    }

    if (factor > 1)
    {
        factor = 1;
    }
    if (factor < 0.001)
    {
        factor = 0.001f;
    }

    maxchange = bs->skills.maxturn;

    //if (maxchange < 240) maxchange = 240;
    maxchange *= thinktime;
    for (i = 0; i < 2; i++) {
        bs->viewangles[i] = AngleMod(bs->viewangles[i]);
        bs->ideal_viewangles[i] = AngleMod(bs->ideal_viewangles[i]);
        diff = AngleDifference(bs->viewangles[i], bs->ideal_viewangles[i]);
        disired_speed = diff * factor;
        bs->viewanglespeed[i] += (bs->viewanglespeed[i] - disired_speed);
        if (bs->viewanglespeed[i] > 180) bs->viewanglespeed[i] = maxchange;
        if (bs->viewanglespeed[i] < -180) bs->viewanglespeed[i] = -maxchange;
        anglespeed = bs->viewanglespeed[i];
        if (anglespeed > maxchange) anglespeed = maxchange;
        if (anglespeed < -maxchange) anglespeed = -maxchange;
        bs->viewangles[i] += anglespeed;
        bs->viewangles[i] = AngleMod(bs->viewangles[i]);
        bs->viewanglespeed[i] *= 0.45 * (1 - factor);
    }
    if (bs->viewangles[PITCH] > 180) bs->viewangles[PITCH] -= 360;
    trap_EA_View(bs->client, bs->viewangles);
}

/*
==============
BotInputToUserCommand
==============
*/
void BotInputToUserCommand(bot_input_t *bi, usercmd_t *ucmd, int delta_angles[3], int time, int useTime) {
    vec3_t angles, forward, right;
    short temp;
    int j;

    //clear the whole structure
    memset(ucmd, 0, sizeof(usercmd_t));
    //
    //Com_Printf("dir = %f %f %f speed = %f\n", bi->dir[0], bi->dir[1], bi->dir[2], bi->speed);
    //the duration for the user command in milli seconds
    ucmd->serverTime = time;
    //
    if (bi->actionflags & ACTION_DELAYEDJUMP) {
        bi->actionflags |= ACTION_JUMP;
        bi->actionflags &= ~ACTION_DELAYEDJUMP;
    }
    //set the buttons
    if (bi->actionflags & ACTION_RESPAWN)
    {
        ucmd->buttons = BUTTON_ATTACK;
    }
    if (bi->actionflags & ACTION_ATTACK) ucmd->buttons |= BUTTON_ATTACK;
    if (bi->actionflags & ACTION_ALT_ATTACK) ucmd->buttons |= BUTTON_ALT_ATTACK;
//  if (bi->actionflags & ACTION_TALK) ucmd->buttons |= BUTTON_TALK;
//  if (bi->actionflags & ACTION_GESTURE) ucmd->buttons |= BUTTON_GESTURE;
#ifdef BOT_USE_HOLDABLE
    if (bi->actionflags & ACTION_USE) ucmd->buttons |= BUTTON_USE_HOLDABLE;
#endif
    if (bi->actionflags & ACTION_WALK) ucmd->buttons |= BUTTON_WALKING;

    if (useTime < level.time && Q_irand(1, 10) < 5)
    { //for now just hit use randomly in case there's something useable around
        ucmd->buttons |= BUTTON_USE;
    }
#if 0
// Here's an interesting bit.  The bots in TA used buttons to do additional gestures.
// I ripped them out because I didn't want too many buttons given the fact that I was already adding some for JK2.
// We can always add some back in if we want though.
    if (bi->actionflags & ACTION_AFFIRMATIVE) ucmd->buttons |= BUTTON_AFFIRMATIVE;
    if (bi->actionflags & ACTION_NEGATIVE) ucmd->buttons |= BUTTON_NEGATIVE;
    if (bi->actionflags & ACTION_GETFLAG) ucmd->buttons |= BUTTON_GETFLAG;
    if (bi->actionflags & ACTION_GUARDBASE) ucmd->buttons |= BUTTON_GUARDBASE;
    if (bi->actionflags & ACTION_PATROL) ucmd->buttons |= BUTTON_PATROL;
    if (bi->actionflags & ACTION_FOLLOWME) ucmd->buttons |= BUTTON_FOLLOWME;
#endif //0

    //
    ucmd->weapon = bi->weapon;
    //set the view angles
    //NOTE: the ucmd->angles are the angles WITHOUT the delta angles
    ucmd->angles[PITCH] = ANGLE2SHORT(bi->viewangles[PITCH]);
    ucmd->angles[YAW] = ANGLE2SHORT(bi->viewangles[YAW]);
    ucmd->angles[ROLL] = ANGLE2SHORT(bi->viewangles[ROLL]);
    //subtract the delta angles
    for (j = 0; j < 3; j++) {
        temp = ucmd->angles[j] - delta_angles[j];
        ucmd->angles[j] = temp;
    }
    //NOTE: movement is relative to the REAL view angles
    //get the horizontal forward and right vector
    //get the pitch in the range [-180, 180]
    if (bi->dir[2]) angles[PITCH] = bi->viewangles[PITCH];
    else angles[PITCH] = 0;
    angles[YAW] = bi->viewangles[YAW];
    angles[ROLL] = 0;
    AngleVectors(angles, forward, right, NULL);
    //bot input speed is in the range [0, 400]
    bi->speed = bi->speed * 127 / 400;
    //set the view independent movement
    ucmd->forwardmove = DotProduct(forward, bi->dir) * bi->speed;
    ucmd->rightmove = DotProduct(right, bi->dir) * bi->speed;
    ucmd->upmove = abs(forward[2]) * bi->dir[2] * bi->speed;
    //normal keyboard movement
    if (bi->actionflags & ACTION_MOVEFORWARD) ucmd->forwardmove += 127;
    if (bi->actionflags & ACTION_MOVEBACK) ucmd->forwardmove -= 127;
    if (bi->actionflags & ACTION_MOVELEFT) ucmd->rightmove -= 127;
    if (bi->actionflags & ACTION_MOVERIGHT) ucmd->rightmove += 127;
    //jump/moveup
    if (bi->actionflags & ACTION_JUMP) ucmd->upmove += 127;
    //crouch/movedown
    if (bi->actionflags & ACTION_CROUCH) ucmd->upmove -= 127;
    //
    //Com_Printf("forward = %d right = %d up = %d\n", ucmd.forwardmove, ucmd.rightmove, ucmd.upmove);
    //Com_Printf("ucmd->serverTime = %d\n", ucmd->serverTime);
}

/*
==============
BotUpdateInput
==============
*/
void BotUpdateInput(bot_state_t *bs, int time, int elapsed_time) {
    bot_input_t bi;
    int j;

    //add the delta angles to the bot's current view angles
    for (j = 0; j < 3; j++) {
        bs->viewangles[j] = AngleMod(bs->viewangles[j] + SHORT2ANGLE(bs->cur_ps.delta_angles[j]));
    }
    //change the bot view angles
    BotChangeViewAngles(bs, (float) elapsed_time / 1000);
    //retrieve the bot input
    trap_EA_GetInput(bs->client, (float) time / 1000, &bi);
    //respawn hack
    if (bi.actionflags & ACTION_RESPAWN)
    {
        // IF already trying to respawn or a ghost then cancel the respawn
        if ((bs->lastucmd.buttons & BUTTON_ATTACK) || (bs->cur_ps.pm_flags&PMF_GHOST))
        {
            bi.actionflags &= ~(ACTION_RESPAWN|ACTION_ATTACK);
        }
    }

    //convert the bot input to a usercmd
    BotInputToUserCommand(&bi, &bs->lastucmd, bs->cur_ps.delta_angles, time, bs->noUseTime);
    //subtract the delta angles
    for (j = 0; j < 3; j++) {
        bs->viewangles[j] = AngleMod(bs->viewangles[j] - SHORT2ANGLE(bs->cur_ps.delta_angles[j]));
    }
}

/*
==============
BotAIRegularUpdate
==============
*/
void BotAIRegularUpdate(void) {
    if (regularupdate_time < FloatTime()) {
        trap_BotUpdateEntityItems();
        regularupdate_time = FloatTime() + 0.3;
    }
}

/*
==============
BotAI
==============
*/
int BotAI(int client, float thinktime) {
    bot_state_t *bs;
    char buf[1024], *args;
    int j;
#ifdef _DEBUG
    int start = 0;
    int end = 0;
#endif

    trap_EA_ResetInput(client);
    //
    bs = botstates[client];
    if (!bs || !bs->inuse) {
        BotAI_Print(PRT_FATAL, "BotAI: client %d is not setup\n", client);
        return qfalse;
    }

    //retrieve the current client state
    BotAI_GetClientState( client, &bs->cur_ps );

    //retrieve any waiting server commands
    while( trap_BotGetServerCommand(client, buf, sizeof(buf)) ) {
        //have buf point to the command and args to the command arguments
        args = strchr( buf, ' ');
        if (!args) continue;
        *args++ = '\0';

        //remove color espace sequences from the arguments
        RemoveColorEscapeSequences( args );

        if (!Q_stricmp(buf, "cp "))
            { /*CenterPrintf*/ }
        else if (!Q_stricmp(buf, "cs"))
            { /*ConfigStringModified*/ }
        else if (!Q_stricmp(buf, "scores"))
            { /*FIXME: parse scores?*/ }
        else if (!Q_stricmp(buf, "clientLevelShot"))
            { /*ignore*/ }
    }
    //add the delta angles to the bot's current view angles
    for (j = 0; j < 3; j++) {
        bs->viewangles[j] = AngleMod(bs->viewangles[j] + SHORT2ANGLE(bs->cur_ps.delta_angles[j]));
    }
    //increase the local time of the bot
    bs->ltime += thinktime;
    //
    bs->thinktime = thinktime;
    //origin of the bot
    VectorCopy(bs->cur_ps.origin, bs->origin);
    //eye coordinates of the bot
    VectorCopy(bs->cur_ps.origin, bs->eye);
    bs->eye[2] += bs->cur_ps.viewheight;
    //get the area the bot is in

#ifdef _DEBUG
    start = trap_Milliseconds();
#endif
    StandardBotAI(bs, thinktime);
#ifdef _DEBUG
    end = trap_Milliseconds();

    trap_Cvar_Update(&bot_debugmessages);

    if (bot_debugmessages.integer)
    {
        Com_Printf("Single AI frametime: %i\n", (end - start));
    }
#endif

    //subtract the delta angles
    for (j = 0; j < 3; j++) {
        bs->viewangles[j] = AngleMod(bs->viewangles[j] - SHORT2ANGLE(bs->cur_ps.delta_angles[j]));
    }
    //everything was ok
    return qtrue;
}

/*
==================
BotScheduleBotThink
==================
*/
void BotScheduleBotThink(void) {
    int i, botnum;

    botnum = 0;

    for( i = 0; i < MAX_CLIENTS; i++ ) {
        if( !botstates[i] || !botstates[i]->inuse ) {
            continue;
        }
        //initialize the bot think residual time
        botstates[i]->botthink_residual = BOT_THINK_TIME * botnum / numbots;
        botnum++;
    }
}

int PlayersInGame(void)
{
    int i = 0;
    gentity_t *ent;
    int pl = 0;

    while (i < MAX_CLIENTS)
    {
        ent = &g_entities[i];

        if (ent && ent->client && ent->client->pers.connected == CON_CONNECTED)
        {
            pl++;
        }

        i++;
    }

    return pl;
}

/*
==============
BotAISetupClient
==============
*/
int BotAISetupClient(int client, struct bot_settings_s *settings, qboolean restart) {
    bot_state_t *bs;

    if (!botstates[client]) botstates[client] = (bot_state_t *)B_Alloc(sizeof(bot_state_t)); //G_Alloc(sizeof(bot_state_t));
                                                                              //rww - G_Alloc bad! B_Alloc good.

    memset(botstates[client], 0, sizeof(bot_state_t));

    bs = botstates[client];

    if (bs && bs->inuse) {
        BotAI_Print(PRT_FATAL, "BotAISetupClient: client %d already setup\n", client);
        return qfalse;
    }

    memcpy(&bs->settings, settings, sizeof(bot_settings_t));

    bs->client = client; //need to know the client number before doing personality stuff

    //initialize weapon weight defaults..
    bs->botWeaponWeights[WP_NONE] = 0;
    bs->botWeaponWeights[WP_KNIFE] = 1;
    bs->botWeaponWeights[WP_M1911A1_PISTOL] = 3;
    bs->botWeaponWeights[WP_USSOCOM_PISTOL] = 2;
    bs->botWeaponWeights[WP_M4_ASSAULT_RIFLE] = 10;
    bs->botWeaponWeights[WP_AK74_ASSAULT_RIFLE] = 9;
    bs->botWeaponWeights[WP_M60_MACHINEGUN] = 11;
    bs->botWeaponWeights[WP_MICRO_UZI_SUBMACHINEGUN] = 8;
    bs->botWeaponWeights[WP_M3A1_SUBMACHINEGUN] = 7;
    bs->botWeaponWeights[WP_MSG90A1] = 11;
    bs->botWeaponWeights[WP_USAS_12_SHOTGUN] = 12;
    bs->botWeaponWeights[WP_M590_SHOTGUN] = 13;
    bs->botWeaponWeights[WP_MM1_GRENADE_LAUNCHER] = 8;
    bs->botWeaponWeights[WP_RPG7_LAUNCHER] = 16;
    bs->botWeaponWeights[WP_M84_GRENADE] = 6;
    bs->botWeaponWeights[WP_SMOHG92_GRENADE] = 2;
    bs->botWeaponWeights[WP_ANM14_GRENADE] = 2;
    bs->botWeaponWeights[WP_M15_GRENADE] = 2;
    #ifdef _GOLD
    bs->botWeaponWeights[WP_SILVER_TALON] = 4;
    bs->botWeaponWeights[WP_MP5] = 7;
    bs->botWeaponWeights[WP_SIG551] = 7;

    if(g_enforce1fxAdditions.integer){
    #endif // _GOLD
        bs->botWeaponWeights[WP_M67_GRENADE] = 6;
        bs->botWeaponWeights[WP_F1_GRENADE] = 6;
        bs->botWeaponWeights[WP_L2A2_GRENADE] = 5;
        bs->botWeaponWeights[WP_MDN11_GRENADE] = 5;
    #ifdef _GOLD
    }
    #endif // _GOLD

    BotUtilizePersonality(bs);

    //allocate a goal state
    bs->gs = trap_BotAllocGoalState(client);

    //allocate a weapon state
    bs->ws = trap_BotAllocWeaponState();

    bs->inuse = qtrue;
    bs->entitynum = client;
    bs->setupcount = 4;
    bs->entergame_time = FloatTime();
    bs->ms = trap_BotAllocMoveState();
    numbots++;

    //NOTE: reschedule the bot thinking
    BotScheduleBotThink();

    if (PlayersInGame())
    { //don't talk to yourself
        BotDoChat(bs, "GeneralGreetings", 0);
    }

    return qtrue;
}

/*
==============
BotAIShutdownClient
==============
*/
int BotAIShutdownClient(int client, qboolean restart) {
    bot_state_t *bs;

    bs = botstates[client];
    if (!bs || !bs->inuse) {
        //BotAI_Print(PRT_ERROR, "BotAIShutdownClient: client %d already shutdown\n", client);
        return qfalse;
    }

    trap_BotFreeMoveState(bs->ms);
    //free the goal state`
    trap_BotFreeGoalState(bs->gs);
    //free the weapon weights
    trap_BotFreeWeaponState(bs->ws);
    //
    //clear the bot state
    memset(bs, 0, sizeof(bot_state_t));
    //set the inuse flag to qfalse
    bs->inuse = qfalse;
    //there's one bot less
    numbots--;
    //everything went ok
    return qtrue;
}

/*
==============
BotResetState

called when a bot enters the intermission or observer mode and
when the level is changed
==============
*/
void BotResetState(bot_state_t *bs) {
    int client, entitynum, inuse;
    int movestate, goalstate, weaponstate;
    bot_settings_t settings;
    playerState_t ps;                           //current player state
    float entergame_time;

    //save some things that should not be reset here
    memcpy(&settings, &bs->settings, sizeof(bot_settings_t));
    memcpy(&ps, &bs->cur_ps, sizeof(playerState_t));
    inuse = bs->inuse;
    client = bs->client;
    entitynum = bs->entitynum;
    movestate = bs->ms;
    goalstate = bs->gs;
    weaponstate = bs->ws;
    entergame_time = bs->entergame_time;
    //reset the whole state
    memset(bs, 0, sizeof(bot_state_t));
    //copy back some state stuff that should not be reset
    bs->ms = movestate;
    bs->gs = goalstate;
    bs->ws = weaponstate;
    memcpy(&bs->cur_ps, &ps, sizeof(playerState_t));
    memcpy(&bs->settings, &settings, sizeof(bot_settings_t));
    bs->inuse = inuse;
    bs->client = client;
    bs->entitynum = entitynum;
    bs->entergame_time = entergame_time;
    //reset several states
    if (bs->ms) trap_BotResetMoveState(bs->ms);
    if (bs->gs) trap_BotResetGoalState(bs->gs);
    if (bs->ws) trap_BotResetWeaponState(bs->ws);
    if (bs->gs) trap_BotResetAvoidGoals(bs->gs);
    if (bs->ms) trap_BotResetAvoidReach(bs->ms);
}

/*
==============
BotAILoadMap
==============
*/
int BotAILoadMap( int restart ) {
    int         i;

    for (i = 0; i < MAX_CLIENTS; i++) {
        if (botstates[i] && botstates[i]->inuse) {
            BotResetState( botstates[i] );
            botstates[i]->setupcount = 4;
        }
    }

    return qtrue;
}

//rww - bot ai
int OrgVisible(vec3_t org1, vec3_t org2, int ignore)
{
    trace_t tr;

    trap_Trace(&tr, org1, NULL, NULL, org2, ignore, MASK_SOLID);

    if (tr.fraction == 1)
    {
        return 1;
    }

    return 0;
}

int WPOrgVisible(gentity_t *bot, vec3_t org1, vec3_t org2, int ignore)
{
    trace_t tr;

    trap_Trace(&tr, org1, NULL, NULL, org2, ignore, MASK_SOLID);

    if (tr.fraction == 1)
    {
        return 1;
    }

    return 0;
}

int OrgVisibleBox(vec3_t org1, vec3_t mins, vec3_t maxs, vec3_t org2, int ignore)
{
    trace_t tr;

    trap_Trace(&tr, org1, mins, maxs, org2, ignore, MASK_SOLID);

    if (tr.fraction == 1 && !tr.startsolid && !tr.allsolid)
    {
        return 1;
    }

    return 0;
}

int CheckForFunc(vec3_t org, int ignore)
{
    gentity_t *fent;
    vec3_t under;
    trace_t tr;

    VectorCopy(org, under);

    under[2] -= 64;

    trap_Trace(&tr, org, NULL, NULL, under, ignore, MASK_SOLID);

    if (tr.fraction == 1)
    {
        return 0;
    }

    fent = &g_entities[tr.entityNum];

    if (!fent)
    {
        return 0;
    }

    if (strstr(fent->classname, "func_"))
    {
        return 1; //there's a func brush here
    }

    return 0;
}

int GetNearestVisibleWP(vec3_t org, int ignore)
{
    int i;
    float bestdist;
    float flLen;
    int bestindex;
    vec3_t a, mins, maxs;

    i = 0;
    bestdist = 800;//99999;
               //don't trace over 800 units away to avoid GIANT HORRIBLE SPEED HITS ^_^
    bestindex = -1;

    mins[0] = -15;
    mins[1] = -15;
    mins[2] = -1;
    maxs[0] = 15;
    maxs[1] = 15;
    maxs[2] = 1;

    while (i < gWPNum)
    {
        if (gWPArray[i] && gWPArray[i]->inuse)
        {
            VectorSubtract(org, gWPArray[i]->origin, a);
            flLen = VectorLength(a);

            if (flLen < bestdist && trap_InPVS(org, gWPArray[i]->origin) && OrgVisibleBox(org, mins, maxs, gWPArray[i]->origin, ignore))
            {
                bestdist = flLen;
                bestindex = i;
            }
        }

        i++;
    }

    return bestindex;
}

//wpDirection
//0 == FORWARD
//1 == BACKWARD

int PassWayCheck(bot_state_t *bs, int windex)
{
    if (!gWPArray[windex] || !gWPArray[windex]->inuse)
    {
        return 0;
    }

    if (bs->wpDirection && (gWPArray[windex]->flags & WPFLAG_ONEWAY_FWD))
    {
        return 0;
    }
    else if (!bs->wpDirection && (gWPArray[windex]->flags & WPFLAG_ONEWAY_BACK))
    {
        return 0;
    }

    return 1;
}

float TotalTrailDistance(int start, int end, bot_state_t *bs)
{
    int beginat;
    int endat;
    float distancetotal;
    float gdif = 0;

    distancetotal = 0;

    if (start > end)
    {
        beginat = end;
        endat = start;
    }
    else
    {
        beginat = start;
        endat = end;
    }

    while (beginat < endat)
    {
        if (beginat >= gWPNum || !gWPArray[beginat] || !gWPArray[beginat]->inuse)
        {
            return -1; //error
        }

        if ((end > start && gWPArray[beginat]->flags & WPFLAG_ONEWAY_BACK) ||
            (start > end && gWPArray[beginat]->flags & WPFLAG_ONEWAY_FWD))
        {
            return -1;
        }

        if (gWPArray[beginat]->forceJumpTo)
        {
            if (gWPArray[beginat-1] && gWPArray[beginat-1]->origin[2]+64 < gWPArray[beginat]->origin[2])
            {
                gdif = gWPArray[beginat]->origin[2] - gWPArray[beginat-1]->origin[2];
            }

            if (gdif)
            {
            //  if (bs && bs->cur_ps.fd.forcePowerLevel[FP_LEVITATION] < gWPArray[beginat]->forceJumpTo)
            //  {
            //      return -1;
            //  }
            }
        }

    /*  if (bs->wpCurrent && gWPArray[windex]->forceJumpTo &&
            gWPArray[windex]->origin[2] > (bs->wpCurrent->origin[2]+64) &&
            bs->cur_ps.fd.forcePowerLevel[FP_LEVITATION] < gWPArray[windex]->forceJumpTo)
        {
            return -1;
        }*/

        distancetotal += gWPArray[beginat]->disttonext;

        beginat++;
    }

    return distancetotal;
}

void CheckForShorterRoutes(bot_state_t *bs, int newwpindex)
{
    float bestlen;
    float checklen;
    int bestindex;
    int i;

    i = 0;

    if (!bs->wpDestination)
    {
        return;
    }

    if (newwpindex < bs->wpDestination->index)
    {
        bs->wpDirection = 0;
    }
    else if (newwpindex > bs->wpDestination->index)
    {
        bs->wpDirection = 1;
    }

    if (bs->wpSwitchTime > level.time)
    {
        return;
    }

    if (!gWPArray[newwpindex]->neighbornum)
    {
        return;
    }

    bestindex = newwpindex;
    bestlen = TotalTrailDistance(newwpindex, bs->wpDestination->index, bs);

    while (i < gWPArray[newwpindex]->neighbornum)
    {
        checklen = TotalTrailDistance(gWPArray[newwpindex]->neighbors[i].num, bs->wpDestination->index, bs);

        if (checklen < bestlen-64 || bestlen == -1)
        {
            if (!gWPArray[newwpindex]->neighbors[i].forceJumpTo)
            {
                bestlen = checklen;
                bestindex = gWPArray[newwpindex]->neighbors[i].num;
            }
        }

        i++;
    }

    if (bestindex != newwpindex && bestindex != -1)
    {
        bs->wpCurrent = gWPArray[bestindex];
        bs->wpSwitchTime = level.time + 3000;
    }
}

void WPConstantRoutine(bot_state_t *bs)
{
    if (!bs->wpCurrent)
    {
        return;
    }

    if (bs->wpCurrent->flags & WPFLAG_DUCK)
    {
        bs->duckTime = level.time + 100;
    }
}

qboolean BotCTFGuardDuty(bot_state_t *bs)
{
/*
    if (level.gametype != GT_CTF)
    {
        return qfalse;
    }

    if (bs->ctfState == CTFSTATE_DEFENDER)
    {
        return qtrue;
    }
*/
    return qfalse;
}

void WPTouchRoutine(bot_state_t *bs)
{
    int lastNum;

    if (!bs->wpCurrent)
    {
        return;
    }

    bs->wpTravelTime = level.time + 10000;

    if (bs->wpCurrent->flags & WPFLAG_NOMOVEFUNC)
    {
        bs->noUseTime = level.time + 4000;
    }

#ifdef FORCEJUMP_INSTANTMETHOD
    if ((bs->wpCurrent->flags & WPFLAG_JUMP) && bs->wpCurrent->forceJumpTo)
    { //jump if we're flagged to but not if this indicates a force jump point. Force jumping is
      //handled elsewhere.
        bs->jumpTime = level.time + 100;
    }
#else
    if ((bs->wpCurrent->flags & WPFLAG_JUMP) && !bs->wpCurrent->forceJumpTo)
    { //jump if we're flagged to but not if this indicates a force jump point. Force jumping is
      //handled elsewhere.
        bs->jumpTime = level.time + 100;
    }
#endif

    trap_Cvar_Update(&bot_camp);

    if (bs->isCamper && bot_camp.integer && (BotIsAChickenWuss(bs) || BotCTFGuardDuty(bs) || bs->isCamper == 2) && ((bs->wpCurrent->flags & WPFLAG_SNIPEORCAMP) || (bs->wpCurrent->flags & WPFLAG_SNIPEORCAMPSTAND)) &&
        bs->cur_ps.weapon != WP_KNIFE)
    { //if we're a camper and a chicken then camp
        if (bs->wpDirection)
        {
            lastNum = bs->wpCurrent->index+1;
        }
        else
        {
            lastNum = bs->wpCurrent->index-1;
        }

        if (gWPArray[lastNum] && gWPArray[lastNum]->inuse && gWPArray[lastNum]->index && bs->isCamping < level.time)
        {
            bs->isCamping = level.time + rand()%15000 + 30000;
            bs->wpCamping = bs->wpCurrent;
            bs->wpCampingTo = gWPArray[lastNum];

            if (bs->wpCurrent->flags & WPFLAG_SNIPEORCAMPSTAND)
            {
                bs->campStanding = qtrue;
            }
            else
            {
                bs->campStanding = qfalse;
            }
        }

    }
    else if ((bs->cur_ps.weapon == WP_KNIFE) &&
        bs->isCamping > level.time)
    {
        bs->isCamping = 0;
        bs->wpCampingTo = NULL;
        bs->wpCamping = NULL;
    }

    if (bs->wpDestination)
    {
        if (bs->wpCurrent->index == bs->wpDestination->index)
        {
            bs->wpDestination = NULL;

            if (bs->runningLikeASissy)
            { //this obviously means we're scared and running, so we'll want to keep our navigational priorities less delayed
                bs->destinationGrabTime = level.time + 500;
            }
            else
            {
                bs->destinationGrabTime = level.time + 3500;
            }
        }
        else
        {
            CheckForShorterRoutes(bs, bs->wpCurrent->index);
        }
    }
}

void MoveTowardIdealAngles(bot_state_t *bs)
{
    VectorCopy(bs->goalAngles, bs->ideal_viewangles);
}

#define BOT_STRAFE_AVOIDANCE

#ifdef BOT_STRAFE_AVOIDANCE
#define STRAFEAROUND_RIGHT          1
#define STRAFEAROUND_LEFT           2

int BotTrace_Strafe(bot_state_t *bs, vec3_t traceto)
{
    vec3_t playerMins = {-15, -15, /*-24*/-8};
    vec3_t playerMaxs = {15, 15, 32};
    vec3_t from, to;
    vec3_t dirAng, dirDif;
    vec3_t forward, right;
    trace_t tr;

    if (bs->cur_ps.groundEntityNum == ENTITYNUM_NONE)
    { //don't do this in the air, it can be.. dangerous.
        return 0;
    }

    VectorSubtract(traceto, bs->origin, dirAng);
    VectorNormalize(dirAng);
    vectoangles(dirAng, dirAng);

    if (AngleDifference(bs->viewangles[YAW], dirAng[YAW]) > 60 ||
        AngleDifference(bs->viewangles[YAW], dirAng[YAW]) < -60)
    { //If we aren't facing the direction we're going here, then we've got enough excuse to be too stupid to strafe around anyway
        return 0;
    }

    VectorCopy(bs->origin, from);
    VectorCopy(traceto, to);

    VectorSubtract(to, from, dirDif);
    VectorNormalize(dirDif);
    vectoangles(dirDif, dirDif);

    AngleVectors(dirDif, forward, 0, 0);

    to[0] = from[0] + forward[0]*32;
    to[1] = from[1] + forward[1]*32;
    to[2] = from[2] + forward[2]*32;

    trap_Trace(&tr, from, playerMins, playerMaxs, to, bs->client, MASK_PLAYERSOLID);

    if (tr.fraction == 1)
    {
        return 0;
    }

    AngleVectors(dirAng, 0, right, 0);

    from[0] += right[0]*32;
    from[1] += right[1]*32;
    from[2] += right[2]*16;

    to[0] += right[0]*32;
    to[1] += right[1]*32;
    to[2] += right[2]*32;

    trap_Trace(&tr, from, playerMins, playerMaxs, to, bs->client, MASK_PLAYERSOLID);

    if (tr.fraction == 1)
    {
        return STRAFEAROUND_RIGHT;
    }

    from[0] -= right[0]*64;
    from[1] -= right[1]*64;
    from[2] -= right[2]*64;

    to[0] -= right[0]*64;
    to[1] -= right[1]*64;
    to[2] -= right[2]*64;

    trap_Trace(&tr, from, playerMins, playerMaxs, to, bs->client, MASK_PLAYERSOLID);

    if (tr.fraction == 1)
    {
        return STRAFEAROUND_LEFT;
    }

    return 0;
}
#endif

int BotTrace_Jump(bot_state_t *bs, vec3_t traceto)
{
    vec3_t mins, maxs, a, fwd, traceto_mod, tracefrom_mod;
    trace_t tr;
    int orTr;

    VectorSubtract(traceto, bs->origin, a);
    vectoangles(a, a);

    AngleVectors(a, fwd, NULL, NULL);

    traceto_mod[0] = bs->origin[0] + fwd[0]*4;
    traceto_mod[1] = bs->origin[1] + fwd[1]*4;
    traceto_mod[2] = bs->origin[2] + fwd[2]*4;

    mins[0] = -15;
    mins[1] = -15;
    mins[2] = -15;
    maxs[0] = 15;
    maxs[1] = 15;
    maxs[2] = 32;

    trap_Trace(&tr, bs->origin, mins, maxs, traceto_mod, bs->client, MASK_PLAYERSOLID);

    if (tr.fraction == 1)
    {
        return 0;
    }

    orTr = tr.entityNum;

    VectorCopy(bs->origin, tracefrom_mod);

    tracefrom_mod[2] += 41;
    traceto_mod[2] += 41;

    mins[0] = -15;
    mins[1] = -15;
    mins[2] = 0;
    maxs[0] = 15;
    maxs[1] = 15;
    maxs[2] = 8;

    trap_Trace(&tr, tracefrom_mod, mins, maxs, traceto_mod, bs->client, MASK_PLAYERSOLID);

    if (tr.fraction == 1)
    {
        if (orTr >= 0 && orTr < MAX_CLIENTS && botstates[orTr] && botstates[orTr]->jumpTime > level.time)
        {
            return 0; //so bots don't try to jump over each other at the same time
        }

        if (bs->currentEnemy && bs->currentEnemy->s.number == orTr && (BotGetWeaponRange(bs) == BWEAPONRANGE_SABER || BotGetWeaponRange(bs) == BWEAPONRANGE_MELEE))
        {
            return 0;
        }

        return 1;
    }

    return 0;
}

int BotTrace_Duck(bot_state_t *bs, vec3_t traceto)
{
    vec3_t mins, maxs, a, fwd, traceto_mod, tracefrom_mod;
    trace_t tr;

    VectorSubtract(traceto, bs->origin, a);
    vectoangles(a, a);

    AngleVectors(a, fwd, NULL, NULL);

    traceto_mod[0] = bs->origin[0] + fwd[0]*4;
    traceto_mod[1] = bs->origin[1] + fwd[1]*4;
    traceto_mod[2] = bs->origin[2] + fwd[2]*4;

    mins[0] = -15;
    mins[1] = -15;
    mins[2] = -23;
    maxs[0] = 15;
    maxs[1] = 15;
    maxs[2] = 8;

    trap_Trace(&tr, bs->origin, mins, maxs, traceto_mod, bs->client, MASK_PLAYERSOLID);

    if (tr.fraction != 1)
    {
        return 0;
    }

    VectorCopy(bs->origin, tracefrom_mod);

    tracefrom_mod[2] += 31;//33;
    traceto_mod[2] += 31;//33;

    mins[0] = -15;
    mins[1] = -15;
    mins[2] = 0;
    maxs[0] = 15;
    maxs[1] = 15;
    maxs[2] = 32;

    trap_Trace(&tr, tracefrom_mod, mins, maxs, traceto_mod, bs->client, MASK_PLAYERSOLID);

    if (tr.fraction != 1)
    {
        return 1;
    }

    return 0;
}

int PassStandardEnemyChecks(bot_state_t *bs, gentity_t *en)
{
    if (!bs || !en)
    {
        return 0;
    }

    if (!en->client)
    {
        return 0;
    }

    if (en->health < 1)
    {
        return 0;
    }

    if (!en->takedamage)
    {
        return 0;
    }

    if (en->client)
    {
        if (en->client->ps.pm_type != PM_NORMAL )
        {
            return 0;
        }

        if ( G_IsClientSpectating ( en->client ) )
        {
            return 0;
        }
    }

    if (!en->s.solid)
    {
        return 0;
    }

    if (bs->client == en->s.number)
    {
        return 0;
    }

    if (OnSameTeam(&g_entities[bs->client], en))
    {
        return 0;
    }

    /*
    if (en->client && en->client->pers.connected != CON_CONNECTED)
    {
        return 0;
    }
    */

    return 1;
}

void BotDamageNotification(gclient_t *bot, gentity_t *attacker)
{
    bot_state_t *bs;
    bot_state_t *bs_a;
    int i;

    if (!bot || !attacker || !attacker->client)
    {
        return;
    }

    bs_a = botstates[attacker->s.number];

    if (bs_a)
    {
        bs_a->lastAttacked = &g_entities[bot->ps.clientNum];
        i = 0;

        while (i < MAX_CLIENTS)
        {
            if (botstates[i] &&
                i != bs_a->client &&
                botstates[i]->lastAttacked == &g_entities[bot->ps.clientNum])
            {
                botstates[i]->lastAttacked = NULL;
            }

            i++;
        }
    }
    else //got attacked by a real client, so no one gets rights to lastAttacked
    {
        i = 0;

        while (i < MAX_CLIENTS)
        {
            if (botstates[i] &&
                botstates[i]->lastAttacked == &g_entities[bot->ps.clientNum])
            {
                botstates[i]->lastAttacked = NULL;
            }

            i++;
        }
    }

    bs = botstates[bot->ps.clientNum];

    if (!bs)
    {
        return;
    }

    bs->lastHurt = attacker;

    if (bs->currentEnemy)
    {
        return;
    }

    if (!PassStandardEnemyChecks(bs, attacker))
    {
        return;
    }

    if (PassLovedOneCheck(bs, attacker))
    {
        bs->currentEnemy = attacker;
        bs->enemySeenTime = level.time + ENEMY_FORGET_MS;
    }
}

int BotCanHear(bot_state_t *bs, gentity_t *en, float endist)
{
    float minlen;

    if (!en || !en->client)
    {
        return 0;
    }

    /*
    if (en && en->client && en->client->ps.otherSoundTime > level.time)
    {
        minlen = en->client->ps.otherSoundLen;
        goto checkStep;
    }
    */

    /*
    if (en && en->client && en->client->ps.footstepTime > level.time)
    {
        minlen = 256;
        goto checkStep;
    }
    */

    if (gBotEventTracker[en->s.number].eventTime < level.time)
    {
        return 0;
    }

    switch(gBotEventTracker[en->s.number].events[gBotEventTracker[en->s.number].eventSequence & (MAX_PS_EVENTS-1)])
    {
    case EV_GLOBAL_SOUND:
        minlen = 256;
        break;
    case EV_FIRE_WEAPON:
    case EV_ALT_FIRE:
        minlen = 512;
        break;
    case EV_STEP_4:
    case EV_STEP_8:
    case EV_STEP_12:
    case EV_STEP_16:
    case EV_FOOTSTEP:
    case EV_FOOTWADE:
        minlen = 256;
        break;
    case EV_JUMP:
        minlen = 256;
        break;
    default:
        minlen = 999999;
        break;
    }

    if (endist <= minlen)
    {
        return 1;
    }

    return 0;
}

void UpdateEventTracker(void)
{
    int i;

    i = 0;

    while (i < MAX_CLIENTS)
    {
        if (gBotEventTracker[i].eventSequence != level.clients[i].ps.eventSequence)
        { //updated event
            gBotEventTracker[i].eventSequence = level.clients[i].ps.eventSequence;
            gBotEventTracker[i].events[0] = level.clients[i].ps.events[0];
            gBotEventTracker[i].events[1] = level.clients[i].ps.events[1];
            gBotEventTracker[i].eventTime = level.time + 0.5;
        }

        i++;
    }
}

int InFieldOfVision(vec3_t viewangles, float fov, vec3_t angles)
{
    int i;
    float diff, angle;

    for (i = 0; i < 2; i++)
    {
        angle = AngleMod(viewangles[i]);
        angles[i] = AngleMod(angles[i]);
        diff = angles[i] - angle;
        if (angles[i] > angle)
        {
            if (diff > 180.0)
            {
                diff -= 360.0;
            }
        }
        else
        {
            if (diff < -180.0)
            {
                diff += 360.0;
            }
        }
        if (diff > 0)
        {
            if (diff > fov * 0.5)
            {
                return 0;
            }
        }
        else
        {
            if (diff < -fov * 0.5)
            {
                return 0;
            }
        }
    }
    return 1;
}

int PassLovedOneCheck(bot_state_t *bs, gentity_t *ent)
{
    int i;
    bot_state_t *loved;

    if (!bs->lovednum)
    {
        return 1;
    }

    i = 0;

    if (!botstates[ent->s.number])
    { //not a bot
        return 1;
    }

    trap_Cvar_Update(&bot_attachments);

    if (!bot_attachments.integer)
    {
        return 1;
    }

    loved = botstates[ent->s.number];

    while (i < bs->lovednum)
    {
        if (strcmp(level.clients[loved->client].pers.netname, bs->loved[i].name) == 0)
        {
            if (!IsTeamplay() && bs->loved[i].level < 2)
            { //if FFA and level of love is not greater than 1, just don't care
                return 1;
            }
            else if (IsTeamplay() && !OnSameTeam(&g_entities[bs->client], &g_entities[loved->client]) && bs->loved[i].level < 2)
            { //is teamplay, but not on same team and level < 2
                return 1;
            }
            else
            {
                return 0;
            }
        }

        i++;
    }

    return 1;
}

int ScanForEnemies(bot_state_t *bs)
{
    vec3_t a;
    float distcheck;
    float closest;
    int bestindex;
    int i;
    float hasEnemyDist = 0;

    closest = 999999;
    i = 0;
    bestindex = -1;

    if (bs->currentEnemy)
    {
        hasEnemyDist = bs->frame_Enemy_Len;
    }

    while (i <= MAX_CLIENTS)
    {
        if (i != bs->client && g_entities[i].client && !OnSameTeam(&g_entities[bs->client], &g_entities[i]) && PassStandardEnemyChecks(bs, &g_entities[i]) && trap_InPVS(g_entities[i].client->ps.origin, bs->eye) && PassLovedOneCheck(bs, &g_entities[i]))
        {
            VectorSubtract(g_entities[i].client->ps.origin, bs->eye, a);
            distcheck = VectorLength(a);
            vectoangles(a, a);

            if (distcheck < closest && ((InFieldOfVision(bs->viewangles, 90, a) /*&& !BotMindTricked(bs->client, i)*/) || BotCanHear(bs, &g_entities[i], distcheck)) && OrgVisible(bs->eye, g_entities[i].client->ps.origin, -1))
            {
                if (!hasEnemyDist || distcheck < (hasEnemyDist - 128))
                { //if we have an enemy, only switch to closer if he is 128+ closer to avoid flipping out
                    closest = distcheck;
                    bestindex = i;
                }
            }
        }
        i++;
    }

    return bestindex;
}

int WaitingForNow(bot_state_t *bs, vec3_t goalpos)
{ //checks if the bot is doing something along the lines of waiting for an elevator to raise up
    vec3_t xybot, xywp, a;

    if (!bs->wpCurrent)
    {
        return 0;
    }

    if ((int)goalpos[0] != (int)bs->wpCurrent->origin[0] ||
        (int)goalpos[1] != (int)bs->wpCurrent->origin[1] ||
        (int)goalpos[2] != (int)bs->wpCurrent->origin[2])
    {
        return 0;
    }

    VectorCopy(bs->origin, xybot);
    VectorCopy(bs->wpCurrent->origin, xywp);

    xybot[2] = 0;
    xywp[2] = 0;

    VectorSubtract(xybot, xywp, a);

    if (VectorLength(a) < 16 && bs->frame_Waypoint_Len > 100)
    {
        if (CheckForFunc(bs->origin, bs->client))
        {
            return 1; //we're probably standing on an elevator and riding up/down. Or at least we hope so.
        }
    }
    else if (VectorLength(a) < 64 && bs->frame_Waypoint_Len > 64 &&
        CheckForFunc(bs->origin, bs->client))
    {
        bs->noUseTime = level.time + 2000;
    }

    return 0;
}

int BotGetWeaponRange(bot_state_t *bs)
{
    switch (weaponData[bs->cur_ps.weapon].category)
    {
    case CAT_KNIFE:
        return BWEAPONRANGE_MELEE;
    case CAT_PISTOL:
        return BWEAPONRANGE_MID; //short
    case CAT_SHOTGUN:
        return BWEAPONRANGE_MID; //short
    case CAT_SUB:
        return BWEAPONRANGE_MID;
    case CAT_ASSAULT:
        return BWEAPONRANGE_MID;
    case CAT_SNIPER:
        return BWEAPONRANGE_LONG;
    case CAT_HEAVY:
        return BWEAPONRANGE_LONG;
    case CAT_GRENADE:
        return BWEAPONRANGE_MID; //short
    default:
        return BWEAPONRANGE_MID;
    }
}

int BotIsAChickenWuss(bot_state_t *bs)
{
    int bWRange;

    if (bs->chickenWussCalculationTime > level.time)
    {
        return 2; //don't want to keep going between two points...
    }

    bs->chickenWussCalculationTime = level.time + MAX_CHICKENWUSS_TIME;

    if (g_entities[bs->client].health < BOT_RUN_HEALTH)
    {
        return 1;
    }

    bWRange = BotGetWeaponRange(bs);

    if (bWRange == BWEAPONRANGE_MELEE)
    {
        if (!bs->meleeSpecialist)
        {
            return 1;
        }
    }

    if (bs->cur_ps.weapon < WP_USAS_12_SHOTGUN)
    { //the bryar is a weak weapon, so just try to find a new one if it's what you're having to use
        return 1;
    }

    if (bs->currentEnemy && bs->currentEnemy->client &&
        bs->currentEnemy->client->ps.weapon == WP_KNIFE &&
        bs->frame_Enemy_Len < 512 && bs->cur_ps.weapon != WP_KNIFE)
    { //if close to an enemy with a knife and not using a knife, then try to back off
        return 1;
    }

    //didn't run, reset the timer
    bs->chickenWussCalculationTime = 0;

    return 0;
}

gentity_t *GetNearestBadThing(bot_state_t *bs)
{
    int i = 0;
    float glen;
    vec3_t hold;
    int bestindex = 0;
    float bestdist = 800; //if not within a radius of 800, it's no threat anyway
    int foundindex = 0;
    float factor = 0;
    gentity_t *ent;
    trace_t tr;

    while (i < MAX_GENTITIES)
    {
        ent = &g_entities[i];

        if ( (ent &&
            !ent->client &&
            ent->inuse &&
            ent->damage &&
            /*(ent->s.weapon == WP_THERMAL || ent->s.weapon == WP_FLECHETTE)*/
            ent->s.weapon &&
            ent->splashDamage) )
        { //try to escape from anything with a non-0 s.weapon and non-0 damage. This hopefully only means dangerous projectiles.
          //Or a sentry gun if bolt_Head == 1000. This is a terrible hack, yes.
            VectorSubtract(bs->origin, ent->r.currentOrigin, hold);
            glen = VectorLength(hold);

            //if (ent->s.weapon != WP_THERMAL && ent->s.weapon != WP_FLECHETTE &&
            //  ent->s.weapon != WP_DET_PACK && ent->s.weapon != WP_TRIP_MINE)
            if (weaponData[ent->s.weapon].category != CAT_GRENADE)
            {
                factor = 0.5;
            }
            else
            {
                factor = 1;
            }

            if (ent->s.weapon == WP_RPG7_LAUNCHER &&
                (ent->r.ownerNum == bs->client ||
                (ent->r.ownerNum > 0 && ent->r.ownerNum < MAX_CLIENTS &&
                g_entities[ent->r.ownerNum].client && OnSameTeam(&g_entities[bs->client], &g_entities[ent->r.ownerNum]))) )
            { //don't be afraid of your own rockets or your teammates' rockets
                factor = 0;
            }

            if (glen < bestdist*factor && trap_InPVS(bs->origin, ent->s.pos.trBase))
            {
                trap_Trace(&tr, bs->origin, NULL, NULL, ent->s.pos.trBase, bs->client, MASK_SOLID);

                if (tr.fraction == 1 || tr.entityNum == ent->s.number)
                {
                    bestindex = i;
                    bestdist = glen;
                    foundindex = 1;
                }
            }
        }

        i++;
    }

    if (foundindex)
    {
        bs->dontGoBack = level.time + 1500;
        return &g_entities[bestindex];
    }
    else
    {
        return NULL;
    }
}

int BotDefendFlag(bot_state_t *bs)
{
    wpobject_t *flagPoint;
    vec3_t a;

    if (level.clients[bs->client].sess.team == TEAM_RED)
    {
        flagPoint = flagRed;
    }
    else if (level.clients[bs->client].sess.team == TEAM_BLUE)
    {
        flagPoint = flagBlue;
    }
    else
    {
        return 0;
    }

    if (!flagPoint)
    {
        return 0;
    }

    VectorSubtract(bs->origin, flagPoint->origin, a);

    if (VectorLength(a) > BASE_GUARD_DISTANCE)
    {
        bs->wpDestination = flagPoint;
    }

    return 1;
}

int BotGetEnemyFlag(bot_state_t *bs)
{
    wpobject_t *flagPoint;
    vec3_t a;

    if (level.clients[bs->client].sess.team == TEAM_RED)
    {
        flagPoint = flagBlue;
    }
    else if (level.clients[bs->client].sess.team == TEAM_BLUE)
    {
        flagPoint = flagRed;
    }
    else
    {
        return 0;
    }

    if (!flagPoint)
    {
        return 0;
    }

    VectorSubtract(bs->origin, flagPoint->origin, a);

    if (VectorLength(a) > BASE_GETENEMYFLAG_DISTANCE)
    {
        bs->wpDestination = flagPoint;
    }

    return 1;
}

int BotGetFlagBack(bot_state_t *bs)
{
#ifdef BOT_KNOW_CTF
    int i = 0;
    int myFlag = 0;
    int foundCarrier = 0;
    int tempInt = 0;
    gentity_t *ent = NULL;
    vec3_t usethisvec;

    if (level.clients[bs->client].sess.sessionTeam == TEAM_RED)
    {
        myFlag = PW_REDFLAG;
    }
    else
    {
        myFlag = PW_BLUEFLAG;
    }

    while (i < MAX_CLIENTS)
    {
        ent = &g_entities[i];

        if (ent && ent->client && ent->client->ps.powerups[myFlag] && !OnSameTeam(&g_entities[bs->client], ent))
        {
            foundCarrier = 1;
            break;
        }

        i++;
    }

    if (!foundCarrier)
    {
        return 0;
    }

    if (!ent)
    {
        return 0;
    }

    if (bs->wpDestSwitchTime < level.time)
    {
        if (ent->client)
        {
            VectorCopy(ent->client->ps.origin, usethisvec);
        }
        else
        {
            VectorCopy(ent->s.origin, usethisvec);
        }

        tempInt = GetNearestVisibleWP(usethisvec, 0);

        if (tempInt != -1 && TotalTrailDistance(bs->wpCurrent->index, tempInt, bs) != -1)
        {
            bs->wpDestination = gWPArray[tempInt];
            bs->wpDestSwitchTime = level.time + Q_irand(1000, 5000);
        }
    }

    return 1;
#else
    return 0;
#endif
}

int BotGuardFlagCarrier(bot_state_t *bs)
{
#ifdef BOT_KNOW_CTF
    int i = 0;
    int enemyFlag = 0;
    int foundCarrier = 0;
    int tempInt = 0;
    gentity_t *ent = NULL;
    vec3_t usethisvec;

    if (level.clients[bs->client].sess.sessionTeam == TEAM_RED)
    {
        enemyFlag = PW_BLUEFLAG;
    }
    else
    {
        enemyFlag = PW_REDFLAG;
    }

    while (i < MAX_CLIENTS)
    {
        ent = &g_entities[i];

        if (ent && ent->client && ent->client->ps.powerups[enemyFlag] && OnSameTeam(&g_entities[bs->client], ent))
        {
            foundCarrier = 1;
            break;
        }

        i++;
    }

    if (!foundCarrier)
    {
        return 0;
    }

    if (!ent)
    {
        return 0;
    }

    if (bs->wpDestSwitchTime < level.time)
    {
        if (ent->client)
        {
            VectorCopy(ent->client->ps.origin, usethisvec);
        }
        else
        {
            VectorCopy(ent->s.origin, usethisvec);
        }

        tempInt = GetNearestVisibleWP(usethisvec, 0);

        if (tempInt != -1 && TotalTrailDistance(bs->wpCurrent->index, tempInt, bs) != -1)
        {
            bs->wpDestination = gWPArray[tempInt];
            bs->wpDestSwitchTime = level.time + Q_irand(1000, 5000);
        }
    }

    return 1;
#else
    return 0;
#endif
}

int BotGetFlagHome(bot_state_t *bs)
{
#ifdef BOT_KNOW_CTF
    wpobject_t *flagPoint;
    vec3_t a;

    if (level.clients[bs->client].sess.sessionTeam == TEAM_RED)
    {
        flagPoint = flagRed;
    }
    else if (level.clients[bs->client].sess.sessionTeam == TEAM_BLUE)
    {
        flagPoint = flagBlue;
    }
    else
    {
        return 0;
    }

    if (!flagPoint)
    {
        return 0;
    }

    VectorSubtract(bs->origin, flagPoint->origin, a);

    if (VectorLength(a) > BASE_FLAGWAIT_DISTANCE)
    {
        bs->wpDestination = flagPoint;
    }

    return 1;
#else
    return 0;
#endif
}

void GetNewFlagPoint(wpobject_t *wp, gentity_t *flagEnt, int team)
{ //get the nearest possible waypoint to the flag since it's not in its original position
#ifdef BOT_KNOW_CTF
    int i = 0;
    vec3_t a, mins, maxs;
    float bestdist;
    float testdist;
    int bestindex = 0;
    int foundindex = 0;
    trace_t tr;

    mins[0] = -15;
    mins[1] = -15;
    mins[2] = -5;
    maxs[0] = 15;
    maxs[1] = 15;
    maxs[2] = 5;

    VectorSubtract(wp->origin, flagEnt->s.pos.trBase, a);

    bestdist = VectorLength(a);

    if (bestdist <= WP_KEEP_FLAG_DIST)
    {
        trap_Trace(&tr, wp->origin, mins, maxs, flagEnt->s.pos.trBase, flagEnt->s.number, MASK_SOLID);

        if (tr.fraction == 1)
        { //this point is good
            return;
        }
    }

    while (i < gWPNum)
    {
        VectorSubtract(gWPArray[i]->origin, flagEnt->s.pos.trBase, a);
        testdist = VectorLength(a);

        if (testdist < bestdist)
        {
            trap_Trace(&tr, gWPArray[i]->origin, mins, maxs, flagEnt->s.pos.trBase, flagEnt->s.number, MASK_SOLID);

            if (tr.fraction == 1)
            {
                foundindex = 1;
                bestindex = i;
                bestdist = testdist;
            }
        }

        i++;
    }

    if (foundindex)
    {
        if (team == TEAM_RED)
        {
            flagRed = gWPArray[bestindex];
        }
        else
        {
            flagBlue = gWPArray[bestindex];
        }
    }
#endif
}

int CTFTakesPriority(bot_state_t *bs)
{
/*
#ifdef BOT_KNOW_CTF
    gentity_t *ent = NULL;
    int enemyFlag = 0;
    int myFlag = 0;
    int enemyHasOurFlag = 0;
    int weHaveEnemyFlag = 0;
    int numOnMyTeam = 0;
    int numOnEnemyTeam = 0;
    int numAttackers = 0;
    int numDefenders = 0;
    int i = 0;
    int idleWP;
    int dosw = 0;
    wpobject_t *dest_sw = NULL;
#ifdef BOT_CTF_DEBUG
    vec3_t t;

    G_Printf("CTFSTATE: %s\n", ctfStateNames[bs->ctfState]);
#endif

    if (level.gametype != GT_CTF)
    {
        return 0;
    }

    if (bs->cur_ps.weapon == WP_BRYAR_PISTOL &&
        (level.time - bs->lastDeadTime) < BOT_MAX_WEAPON_GATHER_TIME)
    { //get the nearest weapon laying around base before heading off for battle
        idleWP = GetBestIdleGoal(bs);

        if (idleWP != -1 && gWPArray[idleWP] && gWPArray[idleWP]->inuse)
        {
            if (bs->wpDestSwitchTime < level.time)
            {
                bs->wpDestination = gWPArray[idleWP];
            }
            return 1;
        }
    }
    else if (bs->cur_ps.weapon == WP_BRYAR_PISTOL &&
        (level.time - bs->lastDeadTime) < BOT_MAX_WEAPON_CHASE_CTF &&
        bs->wpDestination && bs->wpDestination->weight)
    {
        dest_sw = bs->wpDestination;
        dosw = 1;
    }

    if (level.clients[bs->client].sess.sessionTeam == TEAM_RED)
    {
        myFlag = PW_REDFLAG;
    }
    else
    {
        myFlag = PW_BLUEFLAG;
    }

    if (level.clients[bs->client].sess.sessionTeam == TEAM_RED)
    {
        enemyFlag = PW_BLUEFLAG;
    }
    else
    {
        enemyFlag = PW_REDFLAG;
    }

    if (!flagRed || !flagBlue ||
        !flagRed->inuse || !flagBlue->inuse ||
        !eFlagRed || !eFlagBlue)
    {
        return 0;
    }

#ifdef BOT_CTF_DEBUG
    VectorCopy(flagRed->origin, t);
    t[2] += 128;
    G_TestLine(flagRed->origin, t, 0x0000ff, 500);

    VectorCopy(flagBlue->origin, t);
    t[2] += 128;
    G_TestLine(flagBlue->origin, t, 0x0000ff, 500);
#endif

    if (droppedRedFlag && (droppedRedFlag->flags & FL_DROPPED_ITEM))
    {
        GetNewFlagPoint(flagRed, droppedRedFlag, TEAM_RED);
    }
    else
    {
        flagRed = oFlagRed;
    }

    if (droppedBlueFlag && (droppedBlueFlag->flags & FL_DROPPED_ITEM))
    {
        GetNewFlagPoint(flagBlue, droppedBlueFlag, TEAM_BLUE);
    }
    else
    {
        flagBlue = oFlagBlue;
    }

    if (!bs->ctfState)
    {
        return 0;
    }

    i = 0;

    while (i < MAX_CLIENTS)
    {
        ent = &g_entities[i];

        if (ent && ent->client)
        {
            if (ent->client->ps.powerups[enemyFlag] && OnSameTeam(&g_entities[bs->client], ent))
            {
                weHaveEnemyFlag = 1;
            }
            else if (ent->client->ps.powerups[myFlag] && !OnSameTeam(&g_entities[bs->client], ent))
            {
                enemyHasOurFlag = 1;
            }

            if (OnSameTeam(&g_entities[bs->client], ent))
            {
                numOnMyTeam++;
            }
            else
            {
                numOnEnemyTeam++;
            }

            if (botstates[ent->s.number])
            {
                if (botstates[ent->s.number]->ctfState == CTFSTATE_ATTACKER ||
                    botstates[ent->s.number]->ctfState == CTFSTATE_RETRIEVAL)
                {
                    numAttackers++;
                }
                else
                {
                    numDefenders++;
                }
            }
            else
            { //assume real players to be attackers in our logic
                numAttackers++;
            }
        }
        i++;
    }

    if (bs->cur_ps.powerups[enemyFlag])
    {
        if ((numOnMyTeam < 2 || !numAttackers) && enemyHasOurFlag)
        {
            bs->ctfState = CTFSTATE_RETRIEVAL;
        }
        else
        {
            bs->ctfState = CTFSTATE_GETFLAGHOME;
        }
    }
    else if (bs->ctfState == CTFSTATE_GETFLAGHOME)
    {
        bs->ctfState = 0;
    }

    if (bs->state_Forced)
    {
        bs->ctfState = bs->state_Forced;
    }

    if (bs->ctfState == CTFSTATE_DEFENDER)
    {
        if (BotDefendFlag(bs))
        {
            goto success;
        }
    }

    if (bs->ctfState == CTFSTATE_ATTACKER)
    {
        if (BotGetEnemyFlag(bs))
        {
            goto success;
        }
    }

    if (bs->ctfState == CTFSTATE_RETRIEVAL)
    {
        if (BotGetFlagBack(bs))
        {
            goto success;
        }
        else
        { //can't find anyone on another team being a carrier, so ignore this priority
            bs->ctfState = 0;
        }
    }

    if (bs->ctfState == CTFSTATE_GUARDCARRIER)
    {
        if (BotGuardFlagCarrier(bs))
        {
            goto success;
        }
        else
        { //can't find anyone on our team being a carrier, so ignore this priority
            bs->ctfState = 0;
        }
    }

    if (bs->ctfState == CTFSTATE_GETFLAGHOME)
    {
        if (BotGetFlagHome(bs))
        {
            goto success;
        }
    }

    return 0;

success:
    if (dosw)
    { //allow ctf code to run, but if after a particular item then keep going after it
        bs->wpDestination = dest_sw;
    }

    return 1;
#else
    return 0;
#endif
*/
    return 0;
}

int EntityVisibleBox(vec3_t org1, vec3_t mins, vec3_t maxs, vec3_t org2, int ignore, int ignore2)
{
    trace_t tr;

    trap_Trace(&tr, org1, mins, maxs, org2, ignore, MASK_SOLID);

    if (tr.fraction == 1 && !tr.startsolid && !tr.allsolid)
    {
        return 1;
    }
    else if (tr.entityNum != ENTITYNUM_NONE && tr.entityNum == ignore2)
    {
        return 1;
    }

    return 0;
}

int BotHasAssociated(bot_state_t *bs, wpobject_t *wp)
{
    gentity_t *as;

    if (wp->associated_entity == ENTITYNUM_NONE)
    { //make it think this is an item we have so we don't go after nothing
        return 1;
    }

    as = &g_entities[wp->associated_entity];

    if (!as || !as->item)
    {
        return 0;
    }

    if (as->item->giType == IT_WEAPON)
    {
        if (bs->cur_ps.stats[STAT_WEAPONS] & (1 << as->item->giTag))
        {
            return 1;
        }

        return 0;
    }
#ifdef BOT_USE_HOLDABLE
    else if (as->item->giType == IT_HOLDABLE)
    {
        if (bs->cur_ps.stats[STAT_HOLDABLE_ITEMS] & (1 << as->item->giTag))
        {
            return 1;
        }

        return 0;
    }
#endif
    else if (as->item->giType == IT_AMMO)
    {
        if (bs->cur_ps.ammo[as->item->giTag] > 10) //hack
        {
            return 1;
        }

        return 0;
    }

    return 0;
}

int GetBestIdleGoal(bot_state_t *bs)
{
    int i = 0;
    int highestweight = 0;
    int desiredindex = -1;
    int dist_to_weight = 0;
    int traildist;

    if (!bs->wpCurrent)
    {
        return -1;
    }

    if (bs->isCamper != 2)
    {
        if (bs->randomNavTime < level.time)
        {
            if (Q_irand(1, 10) < 5)
            {
                bs->randomNav = 1;
            }
            else
            {
                bs->randomNav = 0;
            }

            bs->randomNavTime = level.time + Q_irand(5000, 15000);
        }
    }

    if (bs->randomNav)
    { //stop looking for items and/or camping on them
        return -1;
    }

    while (i < gWPNum)
    {
        if (gWPArray[i] &&
            gWPArray[i]->inuse &&
            (gWPArray[i]->flags & WPFLAG_GOALPOINT) &&
            gWPArray[i]->weight > highestweight &&
            !BotHasAssociated(bs, gWPArray[i]))
        {
            traildist = TotalTrailDistance(bs->wpCurrent->index, i, bs);

            if (traildist != -1)
            {
                dist_to_weight = (int)traildist/10000;
                dist_to_weight = (gWPArray[i]->weight)-dist_to_weight;

                if (dist_to_weight > highestweight)
                {
                    highestweight = dist_to_weight;
                    desiredindex = i;
                }
            }
        }

        i++;
    }

    return desiredindex;
}

void GetIdealDestination(bot_state_t *bs)
{
    int tempInt, cWPIndex, bChicken, idleWP;
    float distChange, plusLen, minusLen;
    vec3_t usethisvec, a;
    gentity_t *badthing;

    if (!bs->wpCurrent)
    {
        return;
    }

    if ((level.time - bs->escapeDirTime) > 4000)
    {
        badthing = GetNearestBadThing(bs);
    }
    else
    {
        badthing = NULL;
    }

    if (badthing && badthing->inuse &&
        badthing->health > 0 && badthing->takedamage)
    {
        bs->dangerousObject = badthing;
    }
    else
    {
        bs->dangerousObject = NULL;
    }

    if (!badthing && bs->wpDestIgnoreTime > level.time)
    {
        return;
    }

    if (!badthing && bs->dontGoBack > level.time)
    {
        if (bs->wpDestination)
        {
            bs->wpStoreDest = bs->wpDestination;
        }
        bs->wpDestination = NULL;
        return;
    }
    else if (!badthing && bs->wpStoreDest)
    { //after we finish running away, switch back to our original destination
        bs->wpDestination = bs->wpStoreDest;
        bs->wpStoreDest = NULL;
    }

    if (badthing && bs->wpCamping)
    {
        bs->wpCamping = NULL;
    }

    if (bs->wpCamping)
    {
        bs->wpDestination = bs->wpCamping;
        return;
    }

    if (!badthing && CTFTakesPriority(bs))
    {
        if (bs->ctfState)
        {
            bs->runningToEscapeThreat = 1;
        }
        return;
    }

    if (badthing)
    {
        bs->runningLikeASissy = level.time + 100;

        if (bs->wpDestination)
        {
            bs->wpStoreDest = bs->wpDestination;
        }
        bs->wpDestination = NULL;

        if (bs->wpDirection)
        {
            tempInt = bs->wpCurrent->index+1;
        }
        else
        {
            tempInt = bs->wpCurrent->index-1;
        }

        if (gWPArray[tempInt] && gWPArray[tempInt]->inuse && bs->escapeDirTime < level.time)
        {
            VectorSubtract(badthing->s.pos.trBase, bs->wpCurrent->origin, a);
            plusLen = VectorLength(a);
            VectorSubtract(badthing->s.pos.trBase, gWPArray[tempInt]->origin, a);
            minusLen = VectorLength(a);

            if (plusLen < minusLen)
            {
                if (bs->wpDirection)
                {
                    bs->wpDirection = 0;
                }
                else
                {
                    bs->wpDirection = 1;
                }

                bs->wpCurrent = gWPArray[tempInt];

                bs->escapeDirTime = level.time + Q_irand(500, 1000);//Q_irand(1000, 1400);

                //G_Printf("Escaping from scary bad thing [%s]\n", badthing->classname);
            }
        }
        //G_Printf("Run away run away run away!\n");
        return;
    }

    distChange = 0; //keep the compiler from complaining

    tempInt = BotGetWeaponRange(bs);

    if (tempInt == BWEAPONRANGE_MELEE)
    {
        distChange = 1;
    }
    else if (tempInt == BWEAPONRANGE_SABER)
    {
        distChange = 1;
    }
    else if (tempInt == BWEAPONRANGE_MID)
    {
        distChange = 128;
    }
    else if (tempInt == BWEAPONRANGE_LONG)
    {
        distChange = 300;
    }

    if (bs->revengeEnemy && bs->revengeEnemy->health > 0 &&
        bs->revengeEnemy->client && (bs->revengeEnemy->client->pers.connected == CA_ACTIVE || bs->revengeEnemy->client->pers.connected == CA_AUTHORIZING))
    { //if we hate someone, always try to get to them
        if (bs->wpDestSwitchTime < level.time)
        {
            if (bs->revengeEnemy->client)
            {
                VectorCopy(bs->revengeEnemy->client->ps.origin, usethisvec);
            }
            else
            {
                VectorCopy(bs->revengeEnemy->s.origin, usethisvec);
            }

            tempInt = GetNearestVisibleWP(usethisvec, 0);

            if (tempInt != -1 && TotalTrailDistance(bs->wpCurrent->index, tempInt, bs) != -1)
            {
                bs->wpDestination = gWPArray[tempInt];
                bs->wpDestSwitchTime = level.time + Q_irand(5000, 10000);
            }
        }
    }
    else if (bs->squadLeader && bs->squadLeader->health > 0 &&
        bs->squadLeader->client && (bs->squadLeader->client->pers.connected == CA_ACTIVE || bs->squadLeader->client->pers.connected == CA_AUTHORIZING))
    {
        if (bs->wpDestSwitchTime < level.time)
        {
            if (bs->squadLeader->client)
            {
                VectorCopy(bs->squadLeader->client->ps.origin, usethisvec);
            }
            else
            {
                VectorCopy(bs->squadLeader->s.origin, usethisvec);
            }

            tempInt = GetNearestVisibleWP(usethisvec, 0);

            if (tempInt != -1 && TotalTrailDistance(bs->wpCurrent->index, tempInt, bs) != -1)
            {
                bs->wpDestination = gWPArray[tempInt];
                bs->wpDestSwitchTime = level.time + Q_irand(5000, 10000);
            }
        }
    }
    else if (bs->currentEnemy)
    {
        if (bs->currentEnemy->client)
        {
            VectorCopy(bs->currentEnemy->client->ps.origin, usethisvec);
        }
        else
        {
            VectorCopy(bs->currentEnemy->s.origin, usethisvec);
        }

        bChicken = BotIsAChickenWuss(bs);
        bs->runningToEscapeThreat = bChicken;

        if (bs->frame_Enemy_Len < distChange || (bChicken && bChicken != 2))
        {
            cWPIndex = bs->wpCurrent->index;

            if (bs->frame_Enemy_Len > 400)
            { //good distance away, start running toward a good place for an item or powerup or whatever
                idleWP = GetBestIdleGoal(bs);

                if (idleWP != -1 && gWPArray[idleWP] && gWPArray[idleWP]->inuse)
                {
                    bs->wpDestination = gWPArray[idleWP];
                }
            }
            else if (gWPArray[cWPIndex-1] && gWPArray[cWPIndex-1]->inuse &&
                gWPArray[cWPIndex+1] && gWPArray[cWPIndex+1]->inuse)
            {
                VectorSubtract(gWPArray[cWPIndex+1]->origin, usethisvec, a);
                plusLen = VectorLength(a);
                VectorSubtract(gWPArray[cWPIndex-1]->origin, usethisvec, a);
                minusLen = VectorLength(a);

                if (minusLen > plusLen)
                {
                    bs->wpDestination = gWPArray[cWPIndex-1];
                }
                else
                {
                    bs->wpDestination = gWPArray[cWPIndex+1];
                }
            }
        }
        else if (bChicken != 2 && bs->wpDestSwitchTime < level.time)
        {
            tempInt = GetNearestVisibleWP(usethisvec, 0);

            if (tempInt != -1 && TotalTrailDistance(bs->wpCurrent->index, tempInt, bs) != -1)
            {
                bs->wpDestination = gWPArray[tempInt];
                bs->wpDestSwitchTime = level.time + Q_irand(1000, 5000);
            }
        }
    }

    if (!bs->wpDestination && bs->wpDestSwitchTime < level.time)
    {
        //G_Printf("I need something to do\n");
        idleWP = GetBestIdleGoal(bs);

        if (idleWP != -1 && gWPArray[idleWP] && gWPArray[idleWP]->inuse)
        {
            bs->wpDestination = gWPArray[idleWP];
        }
    }
}

void CommanderBotCTFAI(bot_state_t *bs)
{
#ifdef BOT_KNOW_CTF
    int i = 0;
    gentity_t *ent;
    int squadmates = 0;
    gentity_t *squad[MAX_CLIENTS];
    int defendAttackPriority = 0; //0 == attack, 1 == defend
    int guardDefendPriority = 0; //0 == defend, 1 == guard
    int attackRetrievePriority = 0; //0 == retrieve, 1 == attack
    int myFlag = 0;
    int enemyFlag = 0;
    int enemyHasOurFlag = 0;
    int weHaveEnemyFlag = 0;
    int numOnMyTeam = 0;
    int numOnEnemyTeam = 0;
    int numAttackers = 0;
    int numDefenders = 0;

    if (level.clients[bs->client].sess.sessionTeam == TEAM_RED)
    {
        myFlag = PW_REDFLAG;
    }
    else
    {
        myFlag = PW_BLUEFLAG;
    }

    if (level.clients[bs->client].sess.sessionTeam == TEAM_RED)
    {
        enemyFlag = PW_BLUEFLAG;
    }
    else
    {
        enemyFlag = PW_REDFLAG;
    }

    while (i < MAX_CLIENTS)
    {
        ent = &g_entities[i];

        if (ent && ent->client)
        {
            if (ent->client->ps.powerups[enemyFlag] && OnSameTeam(&g_entities[bs->client], ent))
            {
                weHaveEnemyFlag = 1;
            }
            else if (ent->client->ps.powerups[myFlag] && !OnSameTeam(&g_entities[bs->client], ent))
            {
                enemyHasOurFlag = 1;
            }

            if (OnSameTeam(&g_entities[bs->client], ent))
            {
                numOnMyTeam++;
            }
            else
            {
                numOnEnemyTeam++;
            }

            if (botstates[ent->s.number])
            {
                if (botstates[ent->s.number]->ctfState == CTFSTATE_ATTACKER ||
                    botstates[ent->s.number]->ctfState == CTFSTATE_RETRIEVAL)
                {
                    numAttackers++;
                }
                else
                {
                    numDefenders++;
                }
            }
            else
            { //assume real players to be attackers in our logic
                numAttackers++;
            }
        }
        i++;
    }

    i = 0;

    while (i < MAX_CLIENTS)
    {
        ent = &g_entities[i];

        if (ent && ent->client && botstates[i] && botstates[i]->squadLeader && botstates[i]->squadLeader->s.number == bs->client && i != bs->client)
        {
            squad[squadmates] = ent;
            squadmates++;
        }

        i++;
    }

    squad[squadmates] = &g_entities[bs->client];
    squadmates++;

    i = 0;

    if (enemyHasOurFlag && !weHaveEnemyFlag)
    { //start off with an attacker instead of a retriever if we don't have the enemy flag yet so that they can't capture it first.
      //after that we focus on getting our flag back.
        attackRetrievePriority = 1;
    }

    while (i < squadmates)
    {
        if (squad[i] && squad[i]->client && botstates[squad[i]->s.number])
        {
            if (botstates[squad[i]->s.number]->ctfState != CTFSTATE_GETFLAGHOME)
            { //never tell a bot to stop trying to bring the flag to the base
                if (defendAttackPriority)
                {
                    if (weHaveEnemyFlag)
                    {
                        if (guardDefendPriority)
                        {
                            botstates[squad[i]->s.number]->ctfState = CTFSTATE_GUARDCARRIER;
                            guardDefendPriority = 0;
                        }
                        else
                        {
                            botstates[squad[i]->s.number]->ctfState = CTFSTATE_DEFENDER;
                            guardDefendPriority = 1;
                        }
                    }
                    else
                    {
                        botstates[squad[i]->s.number]->ctfState = CTFSTATE_DEFENDER;
                    }
                    defendAttackPriority = 0;
                }
                else
                {
                    if (enemyHasOurFlag)
                    {
                        if (attackRetrievePriority)
                        {
                            botstates[squad[i]->s.number]->ctfState = CTFSTATE_ATTACKER;
                            attackRetrievePriority = 0;
                        }
                        else
                        {
                            botstates[squad[i]->s.number]->ctfState = CTFSTATE_RETRIEVAL;
                            attackRetrievePriority = 1;
                        }
                    }
                    else
                    {
                        botstates[squad[i]->s.number]->ctfState = CTFSTATE_ATTACKER;
                    }
                    defendAttackPriority = 1;
                }
            }
            else if ((numOnMyTeam < 2 || !numAttackers) && enemyHasOurFlag)
            { //I'm the only one on my team who will attack and the enemy has my flag, I have to go after him
                botstates[squad[i]->s.number]->ctfState = CTFSTATE_RETRIEVAL;
            }
        }

        i++;
    }
#endif
}

void BotDoTeamplayAI(bot_state_t *bs)
{
    if (bs->state_Forced)
    {
        bs->teamplayState = bs->state_Forced;
    }

    if (bs->teamplayState == TEAMPLAYSTATE_REGROUP)
    { //force to find a new leader
        bs->squadLeader = NULL;
        bs->isSquadLeader = 0;
    }
}

void CommanderBotTeamplayAI(bot_state_t *bs)
{
    int i = 0;
    int squadmates = 0;
    int teammates = 0;
    int teammate_indanger = -1;
    int teammate_helped = 0;
    int foundsquadleader = 0;
    int worsthealth = 50;
    gentity_t *squad[MAX_CLIENTS];
    gentity_t *ent;
    bot_state_t *bst;

    while (i < MAX_CLIENTS)
    {
        ent = &g_entities[i];

        if (ent && ent->client && OnSameTeam(&g_entities[bs->client], ent) && botstates[ent->s.number])
        {
            bst = botstates[ent->s.number];

            if (foundsquadleader && bst && bst->isSquadLeader)
            { //never more than one squad leader
                bst->isSquadLeader = 0;
            }

            if (bst && !bst->isSquadLeader)
            {
                squad[squadmates] = ent;
                squadmates++;
            }
            else if (bst)
            {
                foundsquadleader = 1;
            }
        }

        if (ent && ent->client && OnSameTeam(&g_entities[bs->client], ent))
        {
            teammates++;

            if (ent->health < worsthealth)
            {
                teammate_indanger = ent->s.number;
                worsthealth = ent->health;
            }
        }

        i++;
    }

    if (!squadmates)
    {
        return;
    }

    i = 0;

    while (i < squadmates && squad[i])
    {
        bst = botstates[squad[i]->s.number];

        if (bst && !bst->state_Forced)
        { //only order if this guy is not being ordered directly by the real player team leader
            if (teammate_indanger >= 0 && !teammate_helped)
            { //send someone out to help whoever needs help most at the moment
                bst->teamplayState = TEAMPLAYSTATE_ASSISTING;
                bst->squadLeader = &g_entities[teammate_indanger];
                teammate_helped = 1;
            }
            else if ((teammate_indanger == -1 || teammate_helped) && bst->teamplayState == TEAMPLAYSTATE_ASSISTING)
            { //no teammates need help badly, but this guy is trying to help them anyway, so stop
                bst->teamplayState = TEAMPLAYSTATE_FOLLOWING;
                bst->squadLeader = &g_entities[bs->client];
            }

            if (bs->squadRegroupInterval < level.time && Q_irand(1, 10) < 5)
            { //every so often tell the squad to regroup for the sake of variation
                if (bst->teamplayState == TEAMPLAYSTATE_FOLLOWING)
                {
                    bst->teamplayState = TEAMPLAYSTATE_REGROUP;
                }

                bs->isSquadLeader = 0;
                bs->squadCannotLead = level.time + 500;
                bs->squadRegroupInterval = level.time + Q_irand(45000, 65000);
            }
        }

        i++;
    }
}

void CommanderBotAI(bot_state_t *bs)
{
/*
    if (level.gametype == GT_CTF)
    {
        CommanderBotCTFAI(bs);
    }
    else if (level.gametype == GT_TDM)
    {
        CommanderBotTeamplayAI(bs);
    }
*/
}


void MeleeCombatHandling(bot_state_t *bs)
{
    vec3_t usethisvec;
    vec3_t downvec;
    vec3_t midorg;
    vec3_t a;
    vec3_t fwd;
    vec3_t mins, maxs;
    trace_t tr;
    int en_down;
    int me_down;
    int mid_down;

    if (!bs->currentEnemy)
    {
        return;
    }

    if (bs->currentEnemy->client)
    {
        VectorCopy(bs->currentEnemy->client->ps.origin, usethisvec);
    }
    else
    {
        VectorCopy(bs->currentEnemy->s.origin, usethisvec);
    }

    if (bs->meleeStrafeTime < level.time)
    {
        if (bs->meleeStrafeDir)
        {
            bs->meleeStrafeDir = 0;
        }
        else
        {
            bs->meleeStrafeDir = 1;
        }

        bs->meleeStrafeTime = level.time + Q_irand(500, 1800);
    }

    mins[0] = -15;
    mins[1] = -15;
    mins[2] = -24;
    maxs[0] = 15;
    maxs[1] = 15;
    maxs[2] = 32;

    VectorCopy(usethisvec, downvec);
    downvec[2] -= 4096;

    trap_Trace(&tr, usethisvec, mins, maxs, downvec, -1, MASK_SOLID);

    en_down = (int)tr.endpos[2];

    VectorCopy(bs->origin, downvec);
    downvec[2] -= 4096;

    trap_Trace(&tr, bs->origin, mins, maxs, downvec, -1, MASK_SOLID);

    me_down = (int)tr.endpos[2];

    VectorSubtract(usethisvec, bs->origin, a);
    vectoangles(a, a);
    AngleVectors(a, fwd, NULL, NULL);

    midorg[0] = bs->origin[0] + fwd[0]*bs->frame_Enemy_Len/2;
    midorg[1] = bs->origin[1] + fwd[1]*bs->frame_Enemy_Len/2;
    midorg[2] = bs->origin[2] + fwd[2]*bs->frame_Enemy_Len/2;

    VectorCopy(midorg, downvec);
    downvec[2] -= 4096;

    trap_Trace(&tr, midorg, mins, maxs, downvec, -1, MASK_SOLID);

    mid_down = (int)tr.endpos[2];

    if (me_down == en_down &&
        en_down == mid_down)
    {
        VectorCopy(usethisvec, bs->goalPosition);
    }
}

#if 0
void SaberCombatHandling(bot_state_t *bs)
{
    vec3_t usethisvec;
    vec3_t downvec;
    vec3_t midorg;
    vec3_t a;
    vec3_t fwd;
    vec3_t mins, maxs;
    trace_t tr;
    int en_down;
    int me_down;
    int mid_down;

    if (!bs->currentEnemy)
    {
        return;
    }

    if (bs->currentEnemy->client)
    {
        VectorCopy(bs->currentEnemy->client->ps.origin, usethisvec);
    }
    else
    {
        VectorCopy(bs->currentEnemy->s.origin, usethisvec);
    }

    if (bs->meleeStrafeTime < level.time)
    {
        if (bs->meleeStrafeDir)
        {
            bs->meleeStrafeDir = 0;
        }
        else
        {
            bs->meleeStrafeDir = 1;
        }

        bs->meleeStrafeTime = level.time + Q_irand(500, 1800);
    }

    mins[0] = -15;
    mins[1] = -15;
    mins[2] = -24;
    maxs[0] = 15;
    maxs[1] = 15;
    maxs[2] = 32;

    VectorCopy(usethisvec, downvec);
    downvec[2] -= 4096;

    trap_Trace(&tr, usethisvec, mins, maxs, downvec, -1, MASK_SOLID);

    en_down = (int)tr.endpos[2];

    if (tr.startsolid || tr.allsolid)
    {
        en_down = 1;
        me_down = 2;
    }
    else
    {
        VectorCopy(bs->origin, downvec);
        downvec[2] -= 4096;

        trap_Trace(&tr, bs->origin, mins, maxs, downvec, -1, MASK_SOLID);

        me_down = (int)tr.endpos[2];

        if (tr.startsolid || tr.allsolid)
        {
            en_down = 1;
            me_down = 2;
        }
    }

    VectorSubtract(usethisvec, bs->origin, a);
    vectoangles(a, a);
    AngleVectors(a, fwd, NULL, NULL);

    midorg[0] = bs->origin[0] + fwd[0]*bs->frame_Enemy_Len/2;
    midorg[1] = bs->origin[1] + fwd[1]*bs->frame_Enemy_Len/2;
    midorg[2] = bs->origin[2] + fwd[2]*bs->frame_Enemy_Len/2;

    VectorCopy(midorg, downvec);
    downvec[2] -= 4096;

    trap_Trace(&tr, midorg, mins, maxs, downvec, -1, MASK_SOLID);

    mid_down = (int)tr.endpos[2];

    if (me_down == en_down &&
        en_down == mid_down)
    {
        if (usethisvec[2] > (bs->origin[2]+32) &&
            bs->currentEnemy->client &&
            bs->currentEnemy->client->ps.groundEntityNum == ENTITYNUM_NONE)
        {
            bs->jumpTime = level.time + 100;
        }

        if (bs->frame_Enemy_Len > 128)
        { //be ready to attack
            bs->saberDefending = 0;
            bs->saberDefendDecideTime = level.time + Q_irand(1000, 2000);
        }
        else
        {
            if (bs->saberDefendDecideTime < level.time)
            {
                if (bs->saberDefending)
                {
                    bs->saberDefending = 0;
                }
                else
                {
                    bs->saberDefending = 1;
                }

                bs->saberDefendDecideTime = level.time + Q_irand(500, 2000);
            }
        }

        if (bs->frame_Enemy_Len < 54)
        {
            VectorCopy(bs->origin, bs->goalPosition);
            bs->saberBFTime = 0;
        }
        else
        {
            VectorCopy(usethisvec, bs->goalPosition);
        }

        if (bs->frame_Enemy_Len > 90 && bs->saberBFTime > level.time && bs->saberBTime > level.time && bs->beStill < level.time && bs->saberSTime < level.time)
        {
            bs->beStill = level.time + Q_irand(500, 1000);
            bs->saberSTime = level.time + Q_irand(1200, 1800);
        }
        else if (bs->currentEnemy->client->ps.weapon == WP_SABER && bs->frame_Enemy_Len < 80 && (Q_irand(1, 10) < 8 && bs->saberBFTime < level.time) || bs->saberBTime > level.time)
        {
            vec3_t vs;
            vec3_t groundcheck;

            VectorSubtract(bs->origin, usethisvec, vs);
            VectorNormalize(vs);

            bs->goalPosition[0] = bs->origin[0] + vs[0]*64;
            bs->goalPosition[1] = bs->origin[1] + vs[1]*64;
            bs->goalPosition[2] = bs->origin[2] + vs[2]*64;

            if (bs->saberBTime < level.time)
            {
                bs->saberBFTime = level.time + Q_irand(900, 1300);
                bs->saberBTime = level.time + Q_irand(300, 700);
            }

            VectorCopy(bs->goalPosition, groundcheck);

            groundcheck[2] -= 64;

            trap_Trace(&tr, bs->goalPosition, NULL, NULL, groundcheck, bs->client, MASK_SOLID);

            if (tr.fraction == 1.0)
            { //don't back off of a ledge
                VectorCopy(usethisvec, bs->goalPosition);
            }
        }
        else if (bs->currentEnemy->client->ps.weapon == WP_SABER && bs->frame_Enemy_Len >= 75)
        {
            bs->saberBFTime = level.time + Q_irand(700, 1300);
            bs->saberBTime = 0;
        }

        /*AngleVectors(bs->viewangles, NULL, fwd, NULL);

        if (bs->meleeStrafeDir)
        {
            bs->goalPosition[0] += fwd[0]*16;
            bs->goalPosition[1] += fwd[1]*16;
            bs->goalPosition[2] += fwd[2]*16;
        }
        else
        {
            bs->goalPosition[0] -= fwd[0]*16;
            bs->goalPosition[1] -= fwd[1]*16;
            bs->goalPosition[2] -= fwd[2]*16;
        }*/
    }
    else if (bs->frame_Enemy_Len <= 56)
    {
        bs->doAttack = 1;
        bs->saberDefending = 0;
    }
}
#endif

float BotWeaponCanLead(bot_state_t *bs)
{
    switch (bs->cur_ps.weapon)
    {
    default:
        return 0;
    case WP_KNIFE:
        return 0.5;

    // no leading needed for any bullet weapons
    case WP_M1911A1_PISTOL:
    case WP_USSOCOM_PISTOL:
    case WP_M4_ASSAULT_RIFLE:
    case WP_AK74_ASSAULT_RIFLE:
    case WP_M60_MACHINEGUN:
    case WP_MICRO_UZI_SUBMACHINEGUN:
    case WP_M3A1_SUBMACHINEGUN:
    case WP_MSG90A1:
    case WP_USAS_12_SHOTGUN:
    case WP_M590_SHOTGUN:
        return 0;
    // projectile weapons lead
    case WP_MM1_GRENADE_LAUNCHER:
        return 0.5;
    case WP_RPG7_LAUNCHER:
        return 0.5;
    case WP_M67_GRENADE:
    case WP_M84_GRENADE:
    case WP_F1_GRENADE:
    case WP_L2A2_GRENADE:
    case WP_MDN11_GRENADE:
    case WP_SMOHG92_GRENADE:
    case WP_ANM14_GRENADE:
    case WP_M15_GRENADE:
        return 0.7;
    }
}

//  Calculate proper angle for ballistic weapon
/*static float CalcWeaponAngle(float vel, float gravity, float targetRange)
{
    float angle = 0;
    float val =  (gravity * targetRange) / (vel * vel);

    if (val >= -1 && val <= 1)
        angle = RAD2DEG(asin(val)/2);
    else
        angle = 90;

    return angle;
}*/

void BotAimLeading(bot_state_t *bs, vec3_t headlevel, float leadAmount)
{
    int x;
    vec3_t predictedSpot;
    vec3_t movementVector;
    vec3_t a, ang;
    float vtotal;

    if (!bs->currentEnemy ||
        !bs->currentEnemy->client)
    {
        return;
    }

    if (!bs->frame_Enemy_Len)
    {
        return;
    }

    vtotal = 0;

    if (bs->currentEnemy->client->ps.velocity[0] < 0)
    {
        vtotal += -bs->currentEnemy->client->ps.velocity[0];
    }
    else
    {
        vtotal += bs->currentEnemy->client->ps.velocity[0];
    }

    if (bs->currentEnemy->client->ps.velocity[1] < 0)
    {
        vtotal += -bs->currentEnemy->client->ps.velocity[1];
    }
    else
    {
        vtotal += bs->currentEnemy->client->ps.velocity[1];
    }

    if (bs->currentEnemy->client->ps.velocity[2] < 0)
    {
        vtotal += -bs->currentEnemy->client->ps.velocity[2];
    }
    else
    {
        vtotal += bs->currentEnemy->client->ps.velocity[2];
    }

    //G_Printf("Leadin target with a velocity total of %f\n", vtotal);

    VectorCopy(bs->currentEnemy->client->ps.velocity, movementVector);

    VectorNormalize(movementVector);

    x = bs->frame_Enemy_Len*leadAmount; //hardly calculated with an exact science, but it works

    if (vtotal > 400)
    {
        vtotal = 400;
    }

    if (vtotal)
    {
        x = (bs->frame_Enemy_Len*0.9)*leadAmount*(vtotal*0.0012); //hardly calculated with an exact science, but it works
    }
    else
    {
        x = (bs->frame_Enemy_Len*0.9)*leadAmount; //hardly calculated with an exact science, but it works
    }

    predictedSpot[0] = headlevel[0] + (movementVector[0]*x);
    predictedSpot[1] = headlevel[1] + (movementVector[1]*x);
    predictedSpot[2] = headlevel[2] + (movementVector[2]*x);

    VectorSubtract(predictedSpot, bs->eye, a);
    vectoangles(a, ang);
    VectorCopy(ang, bs->goalAngles);
}

void BotAimOffsetGoalAngles(bot_state_t *bs)
{
    int i;
    float accVal;
    i = 0;

    if (bs->skills.perfectaim)
    {
        return;
    }

    if (bs->aimOffsetTime > level.time)
    {
        if (bs->aimOffsetAmtYaw)
        {
            bs->goalAngles[YAW] += bs->aimOffsetAmtYaw;
        }

        if (bs->aimOffsetAmtPitch)
        {
            bs->goalAngles[PITCH] += bs->aimOffsetAmtPitch;
        }

        while (i <= 2)
        {
            if (bs->goalAngles[i] > 360)
            {
                bs->goalAngles[i] -= 360;
            }

            if (bs->goalAngles[i] < 0)
            {
                bs->goalAngles[i] += 360;
            }

            i++;
        }
        return;
    }

    accVal = bs->skills.accuracy/bs->settings.skill;

    if (bs->revengeEnemy && bs->revengeHateLevel &&
        bs->currentEnemy == bs->revengeEnemy)
    { //bot becomes more skilled as anger level raises
        accVal = accVal/bs->revengeHateLevel;
    }

    if (bs->currentEnemy && bs->frame_Enemy_Vis)
    { //assume our goal is aiming at the enemy, seeing as he's visible and all
        if (!bs->currentEnemy->s.pos.trDelta[0] &&
            !bs->currentEnemy->s.pos.trDelta[1] &&
            !bs->currentEnemy->s.pos.trDelta[2])
        {
            accVal = 0; //he's not even moving, so he shouldn't really be hard to hit.
        }
        else
        {
            accVal += accVal*0.25; //if he's moving he's this much harder to hit
        }

        if (g_entities[bs->client].s.pos.trDelta[0] ||
            g_entities[bs->client].s.pos.trDelta[1] ||
            g_entities[bs->client].s.pos.trDelta[2])
        {
            accVal += accVal*0.15; //make it somewhat harder to aim if we're moving also
        }
    }

    if (accVal > 90)
    {
        accVal = 90;
    }
    if (accVal < 1)
    {
        accVal = 0;
    }

    if (!accVal)
    {
        bs->aimOffsetAmtYaw = 0;
        bs->aimOffsetAmtPitch = 0;
        return;
    }

    if (rand()%10 <= 5)
    {
        bs->aimOffsetAmtYaw = rand()%(int)accVal;
    }
    else
    {
        bs->aimOffsetAmtYaw = -(rand()%(int)accVal);
    }

    if (rand()%10 <= 5)
    {
        bs->aimOffsetAmtPitch = rand()%(int)accVal;
    }
    else
    {
        bs->aimOffsetAmtPitch = -(rand()%(int)accVal);
    }

    bs->aimOffsetTime = level.time + rand()%500 + 200;
}

int ShouldSecondaryFire(bot_state_t *bs, vec3_t eorg, vec3_t dir)
{
    weaponData_t *weapon = &weaponData[bs->cur_ps.weapon];
    attackData_t *attack = &weapon->attack[ATTACK_ALTERNATE];

    if ((bs->cur_ps.ammo[attack->ammoIndex] < 1) || 0 == attack->damage)
    {   // no ammo for alt fire
        return 0;
    }

#ifdef BOT_LAUNCH_ANGLES
    if ((attack->weaponFlags & PROJECTILE_FIRE) && bs->frame_Enemy_Len > MAX_PROJECTILE_DISTANCE) //don't forget to make sure the bot doesn't shoot it off in his own face!
    {
        // if alt-fire is projectile
        if (bs->frame_Enemy_Len < attack->rV.velocity * attack->projectileLifetime * 0.001)
        {
            // in range
            // 2D range
            float range2D = max(0, SQRTFAST(dir[0]*dir[0] + dir[1]*dir[1]) - attack->splashRadius);
            if (attack->weaponFlags & PROJECTILE_TIMED)
            {   // if timed projectile, let bounce for 1 sec.
                range2D = max(0, range2D - attack->rV.velocity);
            }
            // calculate projectile launch angle
            bs->launchAngle = CalcWeaponAngle( attack->rV.velocity, DEFAULT_GRAVITY, range2D);
            if (bs->launchAngle < 90)
            {
                if (!OrgVisible(bs->eye, eorg, bs->client))
                    bs->launchAngle = 90 - bs->launchAngle;
                return 1;
            }
        }
    }
    else if (bs->frame_Enemy_Len < attack->rV.range)
    {   // alt-fire is bullet
        return 1;
    }
#else
    if ((attack->weaponFlags & PROJECTILE_FIRE) && bs->frame_Enemy_Len > MAX_PROJECTILE_DISTANCE) //don't forget to make sure the bot doesn't shoot it off in his own face!
    {
        return 1;
    }
    else if (bs->frame_Enemy_Len < attack->rV.range)
    {   // alt-fire is bullet
        return 1;
    }
#endif
    return 0;
}

int CombatBotAI(bot_state_t *bs, float thinktime)
{
    vec3_t eorg, a, dir;
    int secFire;
    float fovcheck;

    if (!bs->currentEnemy)
    {
        return 0;
    }

    if (bs->currentEnemy->client)
    {
        VectorCopy(bs->currentEnemy->client->ps.origin, eorg);
    }
    else
    {
        VectorCopy(bs->currentEnemy->s.origin, eorg);
    }

    VectorSubtract(eorg, bs->eye, dir);
    vectoangles(dir, a);

    if (BotGetWeaponRange(bs) == BWEAPONRANGE_SABER)
    {
        if (bs->frame_Enemy_Len <= SABER_ATTACK_RANGE)
        {
            bs->doAttack = 1;
        }
    }
    else if (BotGetWeaponRange(bs) == BWEAPONRANGE_MELEE)
    {
        if (bs->frame_Enemy_Len <= MELEE_ATTACK_RANGE)
        {
            bs->doAttack = 1;
        }
    }
    else
    {
        if (bs->cur_ps.weapon == WP_MSG90A1 || bs->cur_ps.weapon == WP_RPG7_LAUNCHER)
        { //be careful with the hurty weapons
            fovcheck = 10;
        }
        else
        {
            fovcheck = 60;
        }

        if (bs->frame_Enemy_Len < 128)
        {
            fovcheck *= 2;
        }

        if (InFieldOfVision(bs->viewangles, fovcheck, a))
        {
#ifdef BOT_LAUNCH_ANGLES
            weaponData_t *weapon = &weaponData[bs->cur_ps.weapon];
            if (CAT_GRENADE == weapon->category || WP_MM1_GRENADE_LAUNCHER == bs->cur_ps.weapon)
            {
                attackData_t* attack;
                float         range2D;

                // are we using a grenade type weapon?
                if (bs->frame_Enemy_Len < weapon->attack[ATTACK_NORMAL].rV.velocity * weapon->attack[ATTACK_NORMAL].projectileLifetime * 0.001)
                {
                    attack = &weapon->attack[ATTACK_NORMAL];
                }
                else
                {
                    attack = &weapon->attack[ATTACK_ALTERNATE];
                }

                // only 2D range since Z is for the ballistic path only
                range2D = max(0, SQRTFAST(dir[0]*dir[0] + dir[1]*dir[1]) - attack->splashRadius);
                if (attack->weaponFlags & PROJECTILE_TIMED)
                {
                    // if timed projectile, let bounce for 1 sec.
                    range2D = max(0, range2D - attack->rV.velocity);
                }

                bs->launchAngle = CalcWeaponAngle(attack->rV.velocity, DEFAULT_GRAVITY, range2D);
                if (bs->launchAngle < 90)
                {
                    if (!OrgVisible(bs->eye, eorg, bs->client))
                    {
                        bs->launchAngle = 90 - bs->launchAngle;
                    }

                    bs->doAttack = 1;
                }
            }
            else
#endif
            {
                secFire = ShouldSecondaryFire(bs, eorg, dir);

                if (bs->cur_ps.weaponstate != WEAPON_CHARGING_ALT)
                {
                    bs->altChargeTime = Q_irand(500, 1000);
                }

                if (secFire == 1)
                {
                    bs->doAltAttack = 1;
                }
                else if (!secFire)
                {
                    bs->doAttack = 1;
                }

                if (secFire == 2)
                { //released a charge
                    return 1;
                }
            }
        }
    }

    return 0;
}

int BotFallbackNavigation(bot_state_t *bs)
{
    vec3_t b_angle, fwd, trto, mins, maxs;
    trace_t tr;

    if (bs->currentEnemy && bs->frame_Enemy_Vis)
    {
        return 2; //we're busy
    }

    mins[0] = -15;
    mins[1] = -15;
    mins[2] = 0;
    maxs[0] = 15;
    maxs[1] = 15;
    maxs[2] = 32;

    bs->goalAngles[PITCH] = 0;
    bs->goalAngles[ROLL] = 0;

    VectorCopy(bs->goalAngles, b_angle);

    AngleVectors(b_angle, fwd, NULL, NULL);

    trto[0] = bs->origin[0] + fwd[0]*16;
    trto[1] = bs->origin[1] + fwd[1]*16;
    trto[2] = bs->origin[2] + fwd[2]*16;

    trap_Trace(&tr, bs->origin, mins, maxs, trto, -1, MASK_SOLID);

    if (tr.fraction == 1)
    {
        VectorCopy(trto, bs->goalPosition);
        return 1; //success!
    }
    else
    {
        bs->goalAngles[YAW] = rand()%360;
    }

    return 0;
}

int BotTryAnotherWeapon(bot_state_t *bs)
{ //out of ammo, resort to the first weapon we come across that has ammo
    int i;

    i = 0;

    while (i < level.wpNumWeapons)
    {
        if ((bs->cur_ps.ammo[weaponData[i].attack[ATTACK_NORMAL].ammoIndex] > 1 || bs->cur_ps.clip[ATTACK_NORMAL][i] > 1) &&
            (bs->cur_ps.stats[STAT_WEAPONS] & (1 << i)))
        {
            bs->virtualWeapon = i;
            trap_EA_SelectWeapon(bs->client, i);
            //bs->cur_ps.weapon = i;
            //level.clients[bs->client].ps.weapon = i;
            return 1;
        }

        i++;
    }

    if (bs->cur_ps.weapon != 1 && bs->virtualWeapon != 1)
    { //should always have this.. shouldn't we?
        bs->virtualWeapon = 1;
        trap_EA_SelectWeapon(bs->client, 1);
        //bs->cur_ps.weapon = 1;
        //level.clients[bs->client].ps.weapon = 1;
        return 1;
    }

    return 0;
}

int BotSelectIdealWeapon(bot_state_t *bs)
{
    int i;
    int bestweight = -1;
    int bestweapon = 0;

    i = 0;

    while (i < level.wpNumWeapons)
    {
        if ((bs->cur_ps.ammo[weaponData[i].attack[ATTACK_NORMAL].ammoIndex] > 1 || bs->cur_ps.clip[ATTACK_NORMAL][i] > 1) &&
            (bs->botWeaponWeights[i] > bestweight) &&
            (bs->cur_ps.stats[STAT_WEAPONS] & (1 << i)))
        {
            bestweight = bs->botWeaponWeights[i];
            bestweapon = i;
        }

        i++;
    }

    if (bestweight != -1 && bs->cur_ps.weapon != bestweapon && bs->virtualWeapon != bestweapon)
    {
        bs->virtualWeapon = bestweapon;
        trap_EA_SelectWeapon(bs->client, bestweapon);
        //bs->cur_ps.weapon = bestweapon;
        //level.clients[bs->client].ps.weapon = bestweapon;
        return 1;
    }

    return 0;
}

int BotSelectChoiceWeapon(bot_state_t *bs, int weapon, int doselection)
{ //if !doselection then bot will only check if he has the specified weapon and return 1 (yes) or 0 (no)
    int i;
    int hasit = 0;

    i = 0;

    while (i < level.wpNumWeapons)
    {
        if ((bs->cur_ps.ammo[weaponData[i].attack[ATTACK_NORMAL].ammoIndex] > 1 || bs->cur_ps.clip[ATTACK_NORMAL][i] > 1) &&
            i == weapon &&
            (bs->cur_ps.stats[STAT_WEAPONS] & (1 << i)))
        {
            hasit = 1;
            break;
        }

        i++;
    }

    if (hasit && bs->cur_ps.weapon != weapon && doselection && bs->virtualWeapon != weapon)
    {
        bs->virtualWeapon = weapon;
        trap_EA_SelectWeapon(bs->client, weapon);
        //bs->cur_ps.weapon = weapon;
        //level.clients[bs->client].ps.weapon = weapon;
        return 2;
    }

    if (hasit)
    {
        return 1;
    }

    return 0;
}

int BotSelectMelee(bot_state_t *bs)
{
    if (bs->cur_ps.weapon != 1 && bs->virtualWeapon != 1)
    {
        bs->virtualWeapon = 1;
        trap_EA_SelectWeapon(bs->client, 1);
        //bs->cur_ps.weapon = 1;
        //level.clients[bs->client].ps.weapon = 1;
        return 1;
    }

    return 0;
}

int GetLoveLevel(bot_state_t *bs, bot_state_t *love)
{
    int i = 0;
    const char *lname = NULL;

    if (!bs || !love || !g_entities[love->client].client)
    {
        return 0;
    }

    if (!bs->lovednum)
    {
        return 0;
    }

    trap_Cvar_Update(&bot_attachments);

    if (!bot_attachments.integer)
    {
        return 0;
    }

    lname = g_entities[love->client].client->pers.netname;

    if (!lname)
    {
        return 0;
    }

    while (i < bs->lovednum)
    {
        if (strcmp(bs->loved[i].name, lname) == 0)
        {
            return bs->loved[i].level;
        }

        i++;
    }

    return 0;
}

void BotLovedOneDied(bot_state_t *bs, bot_state_t *loved, int lovelevel)
{
    if (!loved->lastHurt || !loved->lastHurt->client ||
        loved->lastHurt->s.number == loved->client)
    {
        return;
    }

    if (!IsTeamplay())
    {
        if (lovelevel < 2)
        {
            return;
        }
    }
    else if (OnSameTeam(&g_entities[bs->client], loved->lastHurt))
    { //don't hate teammates no matter what
        return;
    }

    if (loved->client == loved->lastHurt->s.number)
    {
        return;
    }

    if (bs->client == loved->lastHurt->s.number)
    { //oops!
        return;
    }

    trap_Cvar_Update(&bot_attachments);

    if (!bot_attachments.integer)
    {
        return;
    }

    if (!PassLovedOneCheck(bs, loved->lastHurt))
    { //a loved one killed a loved one.. you cannot hate them
        bs->chatObject = loved->lastHurt;
        bs->chatAltObject = &g_entities[loved->client];
        BotDoChat(bs, "LovedOneKilledLovedOne", 0);
        return;
    }

    if (bs->revengeEnemy == loved->lastHurt)
    {
        if (bs->revengeHateLevel < bs->loved_death_thresh)
        {
            bs->revengeHateLevel++;

            if (bs->revengeHateLevel == bs->loved_death_thresh)
            {
                //broke into the highest anger level
                //CHAT: Hatred section
                bs->chatObject = loved->lastHurt;
                bs->chatAltObject = NULL;
                BotDoChat(bs, "Hatred", 1);
            }
        }
    }
    else if (bs->revengeHateLevel < bs->loved_death_thresh-1)
    { //only switch hatred if we don't hate the existing revenge-enemy too much
        //CHAT: BelovedKilled section
        bs->chatObject = &g_entities[loved->client];
        bs->chatAltObject = loved->lastHurt;
        BotDoChat(bs, "BelovedKilled", 0);
        bs->revengeHateLevel = 0;
        bs->revengeEnemy = loved->lastHurt;
    }
}

void BotDeathNotify(bot_state_t *bs)
{ //in case someone has an emotional attachment to us, we'll notify them
    int i = 0;
    int ltest = 0;

    while (i < MAX_CLIENTS)
    {
        if (botstates[i] && botstates[i]->lovednum)
        {
            ltest = 0;
            while (ltest < botstates[i]->lovednum)
            {
                if (strcmp(level.clients[bs->client].pers.netname, botstates[i]->loved[ltest].name) == 0)
                {
                    BotLovedOneDied(botstates[i], bs, botstates[i]->loved[ltest].level);
                    break;
                }

                ltest++;
            }
        }

        i++;
    }
}

void StrafeTracing(bot_state_t *bs)
{
    vec3_t mins, maxs;
    vec3_t right, rorg, drorg;
    trace_t tr;

    mins[0] = -15;
    mins[1] = -15;
    //mins[2] = -24;
    mins[2] = -22;
    maxs[0] = 15;
    maxs[1] = 15;
    maxs[2] = 32;

    AngleVectors(bs->viewangles, NULL, right, NULL);

    if (bs->meleeStrafeDir)
    {
        rorg[0] = bs->origin[0] - right[0]*32;
        rorg[1] = bs->origin[1] - right[1]*32;
        rorg[2] = bs->origin[2] - right[2]*32;
    }
    else
    {
        rorg[0] = bs->origin[0] + right[0]*32;
        rorg[1] = bs->origin[1] + right[1]*32;
        rorg[2] = bs->origin[2] + right[2]*32;
    }

    trap_Trace(&tr, bs->origin, mins, maxs, rorg, bs->client, MASK_SOLID);

    if (tr.fraction != 1)
    {
        bs->meleeStrafeDisable = level.time + Q_irand(500, 1500);
    }

    VectorCopy(rorg, drorg);

    drorg[2] -= 32;

    trap_Trace(&tr, rorg, NULL, NULL, drorg, bs->client, MASK_SOLID);

    if (tr.fraction == 1)
    { //this may be a dangerous ledge, so don't strafe over it just in case
        bs->meleeStrafeDisable = level.time + Q_irand(500, 1500);
    }
}

int PrimFiring(bot_state_t *bs)
{
    if (bs->cur_ps.weaponstate != WEAPON_CHARGING &&
        bs->doAttack)
    {
        return 1;
    }

    if (bs->cur_ps.weaponstate == WEAPON_CHARGING &&
        !bs->doAttack)
    {
        return 1;
    }

    return 0;
}

int KeepPrimFromFiring(bot_state_t *bs)
{
    if (bs->cur_ps.weaponstate != WEAPON_CHARGING &&
        bs->doAttack)
    {
        bs->doAttack = 0;
    }

    if (bs->cur_ps.weaponstate == WEAPON_CHARGING &&
        !bs->doAttack)
    {
        bs->doAttack = 1;
    }

    return 0;
}

int AltFiring(bot_state_t *bs)
{
    if (bs->cur_ps.weaponstate != WEAPON_CHARGING_ALT &&
        bs->doAltAttack)
    {
        return 1;
    }

    if (bs->cur_ps.weaponstate == WEAPON_CHARGING_ALT &&
        !bs->doAltAttack)
    {
        return 1;
    }

    return 0;
}

int KeepAltFromFiring(bot_state_t *bs)
{
    if (bs->cur_ps.weaponstate != WEAPON_CHARGING_ALT &&
        bs->doAltAttack)
    {
        bs->doAltAttack = 0;
    }

    if (bs->cur_ps.weaponstate == WEAPON_CHARGING_ALT &&
        !bs->doAltAttack)
    {
        bs->doAltAttack = 1;
    }

    return 0;
}

gentity_t *CheckForFriendInLOF(bot_state_t *bs)
{
    vec3_t fwd;
    vec3_t trfrom, trto;
    vec3_t mins, maxs;
    gentity_t *trent;
    trace_t tr;

    mins[0] = -3;
    mins[1] = -3;
    mins[2] = -3;

    maxs[0] = 3;
    maxs[1] = 3;
    maxs[2] = 3;

    AngleVectors(bs->viewangles, fwd, NULL, NULL);

    VectorCopy(bs->eye, trfrom);

    trto[0] = trfrom[0] + fwd[0]*2048;
    trto[1] = trfrom[1] + fwd[1]*2048;
    trto[2] = trfrom[2] + fwd[2]*2048;

    trap_Trace(&tr, trfrom, mins, maxs, trto, bs->client, MASK_PLAYERSOLID);

    if (tr.fraction != 1 && tr.entityNum <= MAX_CLIENTS)
    {
        trent = &g_entities[tr.entityNum];

        if (trent && trent->client)
        {
            if (IsTeamplay() && OnSameTeam(&g_entities[bs->client], trent))
            {
                return trent;
            }

            if (botstates[trent->s.number] && GetLoveLevel(bs, botstates[trent->s.number]) > 1)
            {
                return trent;
            }
        }
    }

    return NULL;
}

void BotScanForLeader(bot_state_t *bs)
{ //bots will only automatically obtain a leader if it's another bot using this method.
    int i = 0;
    gentity_t *ent;

    if (bs->isSquadLeader)
    {
        return;
    }

    while (i < MAX_CLIENTS)
    {
        ent = &g_entities[i];

        if (ent && ent->client && botstates[i] && botstates[i]->isSquadLeader && bs->client != i)
        {
            if (OnSameTeam(&g_entities[bs->client], ent))
            {
                bs->squadLeader = ent;
                break;
            }
            if (GetLoveLevel(bs, botstates[i]) > 1 && !IsTeamplay())
            { //ignore love status regarding squad leaders if we're in teamplay
                bs->squadLeader = ent;
                break;
            }
        }

        i++;
    }
}

void BotReplyGreetings(bot_state_t *bs)
{
    int i = 0;
    int numhello = 0;

    while (i < MAX_CLIENTS)
    {
        if (botstates[i] &&
            botstates[i]->canChat &&
            i != bs->client)
        {
            botstates[i]->chatObject = &g_entities[bs->client];
            botstates[i]->chatAltObject = NULL;
            if (BotDoChat(botstates[i], "ResponseGreetings", 0))
            {
                numhello++;
            }
        }

        if (numhello > 3)
        { //don't let more than 4 bots say hello at once
            return;
        }

        i++;
    }
}

void CTFFlagMovement(bot_state_t *bs)
{
    int diddrop = 0;
    gentity_t *desiredDrop = NULL;
    vec3_t a, mins, maxs;
    trace_t tr;

    mins[0] = -15;
    mins[1] = -15;
    mins[2] = -7;
    maxs[0] = 15;
    maxs[1] = 15;
    maxs[2] = 7;

    if (bs->wantFlag && (bs->wantFlag->flags & FL_DROPPED_ITEM))
    {
        if (bs->staticFlagSpot[0] == bs->wantFlag->s.pos.trBase[0] &&
            bs->staticFlagSpot[1] == bs->wantFlag->s.pos.trBase[1] &&
            bs->staticFlagSpot[2] == bs->wantFlag->s.pos.trBase[2])
        {
            VectorSubtract(bs->origin, bs->wantFlag->s.pos.trBase, a);

            if (VectorLength(a) <= BOT_FLAG_GET_DISTANCE)
            {
                VectorCopy(bs->wantFlag->s.pos.trBase, bs->goalPosition);
                return;
            }
            else
            {
                bs->wantFlag = NULL;
            }
        }
        else
        {
            bs->wantFlag = NULL;
        }
    }
    else if (bs->wantFlag)
    {
        bs->wantFlag = NULL;
    }

    if (flagRed && flagBlue)
    {
        if (bs->wpDestination == flagRed ||
            bs->wpDestination == flagBlue)
        {
            if (bs->wpDestination == flagRed && droppedRedFlag && (droppedRedFlag->flags & FL_DROPPED_ITEM) && droppedRedFlag->classname && strcmp(droppedRedFlag->classname, "freed") != 0)
            {
                desiredDrop = droppedRedFlag;
                diddrop = 1;
            }
            if (bs->wpDestination == flagBlue && droppedBlueFlag && (droppedBlueFlag->flags & FL_DROPPED_ITEM) && droppedBlueFlag->classname && strcmp(droppedBlueFlag->classname, "freed") != 0)
            {
                desiredDrop = droppedBlueFlag;
                diddrop = 1;
            }

            if (diddrop && desiredDrop)
            {
                VectorSubtract(bs->origin, desiredDrop->s.pos.trBase, a);

                if (VectorLength(a) <= BOT_FLAG_GET_DISTANCE)
                {
                    trap_Trace(&tr, bs->origin, mins, maxs, desiredDrop->s.pos.trBase, bs->client, MASK_SOLID);

                    if (tr.fraction == 1 || tr.entityNum == desiredDrop->s.number)
                    {
                        VectorCopy(desiredDrop->s.pos.trBase, bs->goalPosition);
                        VectorCopy(desiredDrop->s.pos.trBase, bs->staticFlagSpot);
                        return;
                    }
                }
            }
        }
    }
}

int BotUseInventoryItem(bot_state_t *bs)
{
#ifdef BOT_USE_HOLDABLE
    if (bs->cur_ps.stats[STAT_HOLDABLE_ITEMS] & (1 << HI_MEDPAC))
    {
        if (g_entities[bs->client].health <= 50)
        {
            bs->cur_ps.stats[STAT_HOLDABLE_ITEM] = BG_GetItemIndexByTag(HI_MEDPAC, IT_HOLDABLE);
            goto wantuseitem;
        }
    }
    if (bs->cur_ps.stats[STAT_HOLDABLE_ITEMS] & (1 << HI_SEEKER))
    {
        if (bs->currentEnemy && bs->frame_Enemy_Vis)
        {
            bs->cur_ps.stats[STAT_HOLDABLE_ITEM] = BG_GetItemIndexByTag(HI_SEEKER, IT_HOLDABLE);
            goto wantuseitem;
        }
    }
    if (bs->cur_ps.stats[STAT_HOLDABLE_ITEMS] & (1 << HI_SENTRY_GUN))
    {
        if (bs->currentEnemy && bs->frame_Enemy_Vis)
        {
            bs->cur_ps.stats[STAT_HOLDABLE_ITEM] = BG_GetItemIndexByTag(HI_SENTRY_GUN, IT_HOLDABLE);
            goto wantuseitem;
        }
    }
    if (bs->cur_ps.stats[STAT_HOLDABLE_ITEMS] & (1 << HI_SHIELD))
    {
        if (bs->currentEnemy && bs->frame_Enemy_Vis && bs->runningToEscapeThreat)
        { //this will (hopefully) result in the bot placing the shield down while facing
          //the enemy and running away
            bs->cur_ps.stats[STAT_HOLDABLE_ITEM] = BG_GetItemIndexByTag(HI_SHIELD, IT_HOLDABLE);
            goto wantuseitem;
        }
    }

    return 0;

wantuseitem:
    level.clients[bs->client].ps.stats[STAT_HOLDABLE_ITEM] = bs->cur_ps.stats[STAT_HOLDABLE_ITEM];

    return 1;
#else
    return 0;
#endif
}

int BotSurfaceNear(bot_state_t *bs)
{
    trace_t tr;
    vec3_t fwd;

    AngleVectors(bs->viewangles, fwd, NULL, NULL);

    fwd[0] = bs->origin[0]+(fwd[0]*64);
    fwd[1] = bs->origin[1]+(fwd[1]*64);
    fwd[2] = bs->origin[2]+(fwd[2]*64);

    trap_Trace(&tr, bs->origin, NULL, NULL, fwd, bs->client, MASK_SOLID);

    if (tr.fraction != 1)
    {
        return 1;
    }

    return 0;
}

wpobject_t *GetLastWP(bot_state_t *bs, int wpIndex)
{
    int goalWPIndex = 0;

    //reverse because we want the last wp to the current, not the next
    if (bs->wpDirection)
    {
        goalWPIndex = bs->wpCurrent->index+1;
    }
    else
    {
        goalWPIndex = bs->wpCurrent->index-1;
    }

    if (goalWPIndex < 0)
    {
        return NULL;
    }

    if (goalWPIndex >= gWPNum)
    {
        return NULL;
    }

    return gWPArray[goalWPIndex];
}

void StandardBotAI(bot_state_t *bs, float thinktime)
{
    int wp, enemy;
    int desiredIndex;
    int goalWPIndex;
    int doingFallback;
    vec3_t a, ang, headlevel, eorg, dif;
    float reaction;
    float bLeadAmount;
    int meleestrafe = 0;
    int cBAI = 0;
    gentity_t *friendInLOF = 0;
    int visResult = 0;
    int selResult = 0;

    trap_Cvar_Update(&bot_pause);

    if (gDeactivated || bot_pause.integer /*|| g_botLean.integer*/)
    {
        bs->wpCurrent = NULL;
        bs->currentEnemy = NULL;
        bs->wpDestination = NULL;
        bs->wpDirection = 0;
        return;
    }

    if (g_entities[bs->client].inuse &&
        g_entities[bs->client].client &&
        G_IsClientSpectating ( g_entities[bs->client].client ) )
    {
        bs->wpCurrent = NULL;
        bs->currentEnemy = NULL;
        bs->wpDestination = NULL;
        bs->wpDirection = 0;
        return;
    }


    if (!bs->lastDeadTime)
    { //just spawned in?
        bs->lastDeadTime = level.time;
    }

    if (g_entities[bs->client].health < 1)
    {
        bs->lastDeadTime = level.time;

        if (!bs->deathActivitiesDone && bs->lastHurt && bs->lastHurt->client && bs->lastHurt->s.number != bs->client)
        {
            BotDeathNotify(bs);
            if (PassLovedOneCheck(bs, bs->lastHurt))
            {
                //CHAT: Died
                bs->chatObject = bs->lastHurt;
                bs->chatAltObject = NULL;
                BotDoChat(bs, "Died", 0);
            }
            else if (!PassLovedOneCheck(bs, bs->lastHurt) &&
                botstates[bs->lastHurt->s.number] &&
                PassLovedOneCheck(botstates[bs->lastHurt->s.number], &g_entities[bs->client]))
            { //killed by a bot that I love, but that does not love me
                bs->chatObject = bs->lastHurt;
                bs->chatAltObject = NULL;
                BotDoChat(bs, "KilledOnPurposeByLove", 0);
            }

            bs->deathActivitiesDone = 1;
        }

        bs->wpCurrent = NULL;
        bs->currentEnemy = NULL;
        bs->wpDestination = NULL;
        bs->wpCamping = NULL;
        bs->wpCampingTo = NULL;
        bs->wpStoreDest = NULL;
        bs->wpDestIgnoreTime = 0;
        bs->wpDestSwitchTime = 0;
        bs->wpSeenTime = 0;
        bs->wpDirection = 0;

        if (rand()%10 < 5 &&
            (!bs->doChat || bs->chatTime < level.time))
        {
            trap_EA_Attack(bs->client);
        }

        return;
    }

    bs->doAttack = 0;
    bs->doAltAttack = 0;
    //reset the attack states

    if (bs->isSquadLeader)
    {
        CommanderBotAI(bs);
    }
    else
    {
        BotDoTeamplayAI(bs);
    }

    if (!bs->currentEnemy)
    {
        bs->frame_Enemy_Vis = 0;
    }

    if (bs->revengeEnemy && bs->revengeEnemy->client &&
        bs->revengeEnemy->client->pers.connected != CA_ACTIVE && bs->revengeEnemy->client->pers.connected != CA_AUTHORIZING)
    {
        bs->revengeEnemy = NULL;
        bs->revengeHateLevel = 0;
    }

    if (bs->currentEnemy && bs->currentEnemy->client &&
        bs->currentEnemy->client->pers.connected != CA_ACTIVE && bs->currentEnemy->client->pers.connected != CA_AUTHORIZING)
    {
        bs->currentEnemy = NULL;
    }

    doingFallback = 0;

    bs->deathActivitiesDone = 0;

    if (BotUseInventoryItem(bs))
    {
        if (rand()%10 < 5)
        {
            trap_EA_Use(bs->client);
        }
    }

    if ((bs->cur_ps.ammo[weaponData[bs->cur_ps.weapon].attack[ATTACK_NORMAL].ammoIndex] < 1 && bs->cur_ps.clip[ATTACK_NORMAL][bs->cur_ps.weapon] < 1))
    {
        if (BotTryAnotherWeapon(bs))
        {
            return;
        }
    }
    else
    {
        if (bs->currentEnemy && bs->lastVisibleEnemyIndex == bs->currentEnemy->s.number &&
            bs->frame_Enemy_Vis && bs->forceWeaponSelect /*&& bs->plantContinue < level.time*/)
        {
            bs->forceWeaponSelect = 0;
        }

        if (bs->forceWeaponSelect)
        {
            selResult = BotSelectChoiceWeapon(bs, bs->forceWeaponSelect, 1);
        }

        if (selResult)
        {
            if (selResult == 2)
            { //newly selected
                return;
            }
        }
        else if (BotSelectIdealWeapon(bs))
        {
            return;
        }
    }
    /*if (BotSelectMelee(bs))
    {
        return;
    }*/

    reaction = bs->skills.reflex/bs->settings.skill;

    if (reaction < 0)
    {
        reaction = 0;
    }
    if (reaction > 2000)
    {
        reaction = 2000;
    }

    if (!bs->currentEnemy)
    {
        bs->timeToReact = level.time + reaction;
    }

    if (bs->wpCamping)
    {
        if (bs->isCamping < level.time)
        {
            bs->wpCamping = NULL;
            bs->isCamping = 0;
        }

        if (bs->currentEnemy && bs->frame_Enemy_Vis)
        {
            bs->wpCamping = NULL;
            bs->isCamping = 0;
        }
    }

    if (bs->wpCurrent &&
        (bs->wpSeenTime < level.time || bs->wpTravelTime < level.time))
    {
        bs->wpCurrent = NULL;
    }

    if (bs->currentEnemy)
    {
        if (bs->enemySeenTime < level.time ||
            !PassStandardEnemyChecks(bs, bs->currentEnemy))
        {
            if (bs->revengeEnemy == bs->currentEnemy &&
                bs->currentEnemy->health < 1 &&
                bs->lastAttacked && bs->lastAttacked == bs->currentEnemy)
            {
                //CHAT: Destroyed hated one [KilledHatedOne section]
                bs->chatObject = bs->revengeEnemy;
                bs->chatAltObject = NULL;
                BotDoChat(bs, "KilledHatedOne", 1);
                bs->revengeEnemy = NULL;
                bs->revengeHateLevel = 0;
            }
            else if (bs->currentEnemy->health < 1 && PassLovedOneCheck(bs, bs->currentEnemy) &&
                bs->lastAttacked && bs->lastAttacked == bs->currentEnemy)
            {
                //CHAT: Killed
                bs->chatObject = bs->currentEnemy;
                bs->chatAltObject = NULL;
                BotDoChat(bs, "Killed", 0);
            }

            bs->currentEnemy = NULL;
        }
    }

    if (!bs->wpCurrent)
    {
        wp = GetNearestVisibleWP(bs->origin, bs->client);

        if (wp != -1)
        {
            bs->wpCurrent = gWPArray[wp];
            bs->wpSeenTime = level.time + 1500;
            bs->wpTravelTime = level.time + 10000; //never take more than 10 seconds to travel to a waypoint
        }
    }

    if (bs->enemySeenTime < level.time || !bs->frame_Enemy_Vis || !bs->currentEnemy ||
        (bs->currentEnemy /*&& bs->cur_ps.weapon == WP_SABER && bs->frame_Enemy_Len > 300*/))
    {
        enemy = ScanForEnemies(bs);

        if (enemy != -1)
        {
            bs->currentEnemy = &g_entities[enemy];
            bs->enemySeenTime = level.time + ENEMY_FORGET_MS;
        }
    }

    if (!bs->squadLeader && !bs->isSquadLeader)
    {
        BotScanForLeader(bs);
    }

    if (!bs->squadLeader && bs->squadCannotLead < level.time)
    { //if still no leader after scanning, then become a squad leader
        bs->isSquadLeader = 1;
    }

    if (bs->isSquadLeader && bs->squadLeader)
    { //we don't follow anyone if we are a leader
        bs->squadLeader = NULL;
    }

    //ESTABLISH VISIBILITIES AND DISTANCES FOR THE WHOLE FRAME HERE
    if (bs->wpCurrent)
    {
        VectorSubtract(bs->wpCurrent->origin, bs->origin, a);
        bs->frame_Waypoint_Len = VectorLength(a);

        visResult = WPOrgVisible(&g_entities[bs->client], bs->origin, bs->wpCurrent->origin, bs->client);

        if (visResult == 2)
        {
            bs->frame_Waypoint_Vis = 0;
            bs->wpSeenTime = 0;
            bs->wpDestination = NULL;
            bs->wpDestIgnoreTime = level.time + 5000;

            if (bs->wpDirection)
            {
                bs->wpDirection = 0;
            }
            else
            {
                bs->wpDirection = 1;
            }
        }
        else if (visResult)
        {
            bs->frame_Waypoint_Vis = 1;
        }
        else
        {
            bs->frame_Waypoint_Vis = 0;
        }
    }

    if (bs->currentEnemy)
    {
        if (bs->currentEnemy->client)
        {
            VectorCopy(bs->currentEnemy->client->ps.origin, eorg);
            eorg[2] += bs->currentEnemy->client->ps.viewheight;
        }
        else
        {
            VectorCopy(bs->currentEnemy->s.origin, eorg);
        }

        VectorSubtract(eorg, bs->eye, a);
        bs->frame_Enemy_Len = VectorLength(a);

        if (OrgVisible(bs->eye, eorg, bs->client))
        {
            bs->frame_Enemy_Vis = 1;
            VectorCopy(eorg, bs->lastEnemySpotted);
            VectorCopy(bs->origin, bs->hereWhenSpotted);
            bs->lastVisibleEnemyIndex = bs->currentEnemy->s.number;
            //VectorCopy(bs->eye, bs->lastEnemySpotted);
            bs->hitSpotted = 0;
        }
        else
        {
            bs->frame_Enemy_Vis = 0;
        }
    }
    else
    {
        bs->lastVisibleEnemyIndex = ENTITYNUM_NONE;
    }
    //END

    if (bs->frame_Enemy_Vis)
    {
        bs->enemySeenTime = level.time + ENEMY_FORGET_MS;
    }

    if (bs->wpCurrent)
    {
        WPConstantRoutine(bs);

        if (!bs->wpCurrent)
        { //WPConstantRoutine has the ability to nullify the waypoint if it fails certain checks, so..
            return;
        }

        if (bs->wpCurrent->flags & WPFLAG_WAITFORFUNC)
        {
            if (!CheckForFunc(bs->wpCurrent->origin, -1))
            {
                bs->beStill = level.time + 500; //no func brush under.. wait
            }
        }
        if (bs->wpCurrent->flags & WPFLAG_NOMOVEFUNC)
        {
            if (CheckForFunc(bs->wpCurrent->origin, -1))
            {
                bs->beStill = level.time + 500; //func brush under.. wait
            }
        }

        if (bs->frame_Waypoint_Vis || (bs->wpCurrent->flags & WPFLAG_NOVIS))
        {
            bs->wpSeenTime = level.time + 1500; //if we lose sight of the point, we have 1.5 seconds to regain it before we drop it
        }
        VectorCopy(bs->wpCurrent->origin, bs->goalPosition);
        if (bs->wpDirection)
        {
            goalWPIndex = bs->wpCurrent->index-1;
        }
        else
        {
            goalWPIndex = bs->wpCurrent->index+1;
        }

        if (bs->wpCamping)
        {
            VectorSubtract(bs->wpCampingTo->origin, bs->origin, a);
            vectoangles(a, ang);
            VectorCopy(ang, bs->goalAngles);

            VectorSubtract(bs->origin, bs->wpCamping->origin, a);
            if (VectorLength(a) < 64)
            {
                VectorCopy(bs->wpCamping->origin, bs->goalPosition);
                bs->beStill = level.time + 1000;

                if (!bs->campStanding)
                {
                    bs->duckTime = level.time + 1000;
                }
            }
        }
        /*
        else if (gWPArray[goalWPIndex] && gWPArray[goalWPIndex]->inuse &&
            !(bs->cur_ps.pm_flags & PMF_LADDER))
        { //don't look at one ahead if on a ladder..
            if (bs->cur_ps.pm_flags & PMF_LADDER)
            {
                vec3_t ladderMod;
                wpobject_t *prevWP;

                prevWP = GetLastWP(bs, gWPArray[goalWPIndex]->index);

                VectorCopy(gWPArray[goalWPIndex]->origin, ladderMod);

                if (prevWP && prevWP->origin[2] < gWPArray[goalWPIndex]->origin[2])
                { //We're going up a ladder, as opposed to down
                    ladderMod[2] += 400;
                }
                VectorSubtract(ladderMod, bs->origin, a);
                vectoangles(a, ang);
                VectorCopy(ang, bs->goalAngles);

                //bs->jumpTime = level.time + 100;
            }
            else
            {
                VectorSubtract(gWPArray[goalWPIndex]->origin, bs->origin, a);
                vectoangles(a, ang);
                VectorCopy(ang, bs->goalAngles);
            }
        }
        */
        //This code usually causes horrible issues in SoF2 because the levels are more cramped and tight.
        //So I guess for now they'll just have snappier viewangles.
        else
        {
            if (bs->cur_ps.pm_flags & PMF_LADDER)
            {
                vec3_t ladderMod;
                wpobject_t *prevWP;

                prevWP = GetLastWP(bs, bs->wpCurrent->index);

                VectorCopy(bs->wpCurrent->origin, ladderMod);

                if (prevWP && prevWP->origin[2] < bs->wpCurrent->origin[2])
                { //We're going up a ladder, as opposed to down
                    ladderMod[2] += 400;
                }
                VectorSubtract(ladderMod, bs->origin, a);
                vectoangles(a, ang);
                VectorCopy(ang, bs->goalAngles);

                //bs->jumpTime = level.time + 100;
            }
            else
            {
                VectorSubtract(bs->wpCurrent->origin, bs->origin, a);
                vectoangles(a, ang);
                VectorCopy(ang, bs->goalAngles);
            }
        }

        if (bs->destinationGrabTime < level.time /*&& (!bs->wpDestination || (bs->currentEnemy && bs->frame_Enemy_Vis))*/)
        {
            GetIdealDestination(bs);
        }

        if (bs->wpCurrent && bs->wpDestination)
        {
            if (TotalTrailDistance(bs->wpCurrent->index, bs->wpDestination->index, bs) == -1)
            {
                bs->wpDestination = NULL;
                bs->destinationGrabTime = level.time + 10000;
            }
        }

        if (bs->frame_Waypoint_Len < BOT_WPTOUCH_DISTANCE)
        {
            WPTouchRoutine(bs);

            if (!bs->wpDirection)
            {
                desiredIndex = bs->wpCurrent->index+1;
            }
            else
            {
                desiredIndex = bs->wpCurrent->index-1;
            }

            if (gWPArray[desiredIndex] &&
                gWPArray[desiredIndex]->inuse &&
                desiredIndex < gWPNum &&
                desiredIndex >= 0 &&
                PassWayCheck(bs, desiredIndex))
            {
                bs->wpCurrent = gWPArray[desiredIndex];
            }
            else
            {
                if (bs->wpDestination)
                {
                    bs->wpDestination = NULL;
                    bs->destinationGrabTime = level.time + 10000;
                }

                if (bs->wpDirection)
                {
                    bs->wpDirection = 0;
                }
                else
                {
                    bs->wpDirection = 1;
                }
            }
        }
    }
    else //We can't find a waypoint, going to need a fallback routine.
    {
        doingFallback = BotFallbackNavigation(bs);
    }

    if (bs->timeToReact < level.time && bs->currentEnemy && bs->enemySeenTime > level.time + (ENEMY_FORGET_MS - (ENEMY_FORGET_MS*0.2)))
    {
        if (bs->frame_Enemy_Vis)
        {
            cBAI = CombatBotAI(bs, thinktime);
        }
        else if (bs->cur_ps.weaponstate == WEAPON_CHARGING_ALT)
        { //keep charging in case we see him again before we lose track of him
            bs->doAltAttack = 1;
        }

        if (bs->destinationGrabTime > level.time + 100)
        {
            bs->destinationGrabTime = level.time + 100; //assures that we will continue staying within a general area of where we want to be in a combat situation
        }

        if (bs->cur_ps.pm_flags & PMF_LADDER)
        { //keep navigating the ladder but if anyone happens in front of us on it, shoot them up good.
            goto skipviewchecks;
        }

        if (bs->currentEnemy->client)
        {
            VectorCopy(bs->currentEnemy->client->ps.origin, headlevel);
            headlevel[2] += bs->currentEnemy->client->ps.viewheight;
        }
        else
        {
            VectorCopy(bs->currentEnemy->client->ps.origin, headlevel);
        }

        bLeadAmount = BotWeaponCanLead(bs);
        if ((bs->skills.accuracy/bs->settings.skill) <= 8 &&
            bLeadAmount)
        {
            BotAimLeading(bs, headlevel, bLeadAmount);
        }
        else
        {
            VectorSubtract(headlevel, bs->eye, a);
            vectoangles(a, ang);
            VectorCopy(ang, bs->goalAngles);
        }

#ifdef BOT_LAUNCH_ANGLES
        if ((CAT_GRENADE == weapon->category ||
             WP_MM1_GRENADE_LAUNCHER == bs->cur_ps.weapon) && bs->launchAngle < 90)
        {
            bs->goalAngles[PITCH] = bs->goalAngles[PITCH]-bs->launchAngle;
        }
#endif

        BotAimOffsetGoalAngles(bs);
    }
skipviewchecks:
    if (bs->currentEnemy)
    {
        if (BotGetWeaponRange(bs) == BWEAPONRANGE_MELEE)
        {
            if (bs->frame_Enemy_Len <= MELEE_ATTACK_RANGE)
            {
                MeleeCombatHandling(bs);
                meleestrafe = 1;
            }
        }
    }

    if (doingFallback && bs->currentEnemy) //just stand and fire if we have no idea where we are
    {
        VectorCopy(bs->origin, bs->goalPosition);
    }

    if (bs->doChat && bs->chatTime > level.time && (!bs->currentEnemy || !bs->frame_Enemy_Vis))
    {
        return;
    }
    else if (bs->doChat && bs->currentEnemy && bs->frame_Enemy_Vis)
    {
        //bs->chatTime = level.time + bs->chatTime_stored;
        bs->doChat = 0; //do we want to keep the bot waiting to chat until after the enemy is gone?
        bs->chatTeam = 0;
    }
    else if (bs->doChat && bs->chatTime <= level.time)
    {
        if (bs->chatTeam)
        {
            trap_EA_SayTeam(bs->client, bs->currentChat);
            bs->chatTeam = 0;
        }
        else
        {
            trap_EA_Say(bs->client, bs->currentChat);
        }
        if (bs->doChat == 2)
        {
            BotReplyGreetings(bs);
        }
        bs->doChat = 0;
    }

    CTFFlagMovement(bs);

    if (/*bs->wpDestination &&*/ bs->shootGoal &&
        /*bs->wpDestination->associated_entity == bs->shootGoal->s.number &&*/
        bs->shootGoal->health > 0 && bs->shootGoal->takedamage)
    {
        dif[0] = (bs->shootGoal->r.absmax[0]+bs->shootGoal->r.absmin[0])/2;
        dif[1] = (bs->shootGoal->r.absmax[1]+bs->shootGoal->r.absmin[1])/2;
        dif[2] = (bs->shootGoal->r.absmax[2]+bs->shootGoal->r.absmin[2])/2;

        if (!bs->currentEnemy || bs->frame_Enemy_Len > 256)
        { //if someone is close then don't stop shooting them for this
            VectorSubtract(dif, bs->eye, a);
            vectoangles(a, a);
            VectorCopy(a, bs->goalAngles);

            if (InFieldOfVision(bs->viewangles, 30, a) &&
                EntityVisibleBox(bs->origin, NULL, NULL, dif, bs->client, bs->shootGoal->s.number))
            {
                bs->doAttack = 1;
            }
        }
    }

    if (bs->beStill < level.time && !WaitingForNow(bs, bs->goalPosition))
    {
        VectorSubtract(bs->goalPosition, bs->origin, bs->goalMovedir);
        VectorNormalize(bs->goalMovedir);

        if (bs->jumpTime > level.time && bs->jDelay < level.time &&
            level.clients[bs->client].pers.cmd.upmove > 0)
        {
        //  trap_EA_Move(bs->client, bs->origin, 5000);
            bs->beStill = level.time + 200;
        }
        else
        {
            trap_EA_Move(bs->client, bs->goalMovedir, 5000);
        }

        if (meleestrafe)
        {
            StrafeTracing(bs);
        }

        if (bs->meleeStrafeDir && meleestrafe && bs->meleeStrafeDisable < level.time)
        {
            trap_EA_MoveRight(bs->client);
        }
        else if (meleestrafe && bs->meleeStrafeDisable < level.time)
        {
            trap_EA_MoveLeft(bs->client);
        }

        if (BotTrace_Jump(bs, bs->goalPosition))
        {
            bs->jumpTime = level.time + 100;
        }
        else if (BotTrace_Duck(bs, bs->goalPosition))
        {
            bs->duckTime = level.time + 100;
        }
#ifdef BOT_STRAFE_AVOIDANCE
        else
        {
            int strafeAround = BotTrace_Strafe(bs, bs->goalPosition);

            if (strafeAround == STRAFEAROUND_RIGHT)
            {
                trap_EA_MoveRight(bs->client);
            }
            else if (strafeAround == STRAFEAROUND_LEFT)
            {
                trap_EA_MoveLeft(bs->client);
            }
        }
#endif
    }

    if (bs->jumpTime > level.time && bs->jDelay < level.time)
    {
        if (!(bs->cur_ps.pm_debounce & PMD_JUMP ))
        {
            trap_EA_Jump(bs->client);
        }
    }

    if (bs->duckTime > level.time)
    {
        trap_EA_Crouch(bs->client);
    }

    if ( bs->dangerousObject && bs->dangerousObject->inuse && bs->dangerousObject->health > 0 &&
        bs->dangerousObject->takedamage && (!bs->frame_Enemy_Vis || !bs->currentEnemy) &&
        (BotGetWeaponRange(bs) == BWEAPONRANGE_MID || BotGetWeaponRange(bs) == BWEAPONRANGE_LONG) &&
//      bs->cur_ps.weapon != WP_DET_PACK && bs->cur_ps.weapon != WP_TRIP_MINE &&
        !bs->shootGoal )
    {
        float danLen;

        VectorSubtract(bs->dangerousObject->r.currentOrigin, bs->eye, a);

        danLen = VectorLength(a);

        if (danLen > 256)
        {
            vectoangles(a, a);
            VectorCopy(a, bs->goalAngles);

            if (Q_irand(1, 10) < 5)
            {
                bs->goalAngles[YAW] += Q_irand(0, 3);
                bs->goalAngles[PITCH] += Q_irand(0, 3);
            }
            else
            {
                bs->goalAngles[YAW] -= Q_irand(0, 3);
                bs->goalAngles[PITCH] -= Q_irand(0, 3);
            }

            if (InFieldOfVision(bs->viewangles, 30, a) &&
                EntityVisibleBox(bs->origin, NULL, NULL, bs->dangerousObject->r.currentOrigin, bs->client, bs->dangerousObject->s.number))
            {
                bs->doAttack = 1;
            }
        }
    }

    if (PrimFiring(bs) ||
        AltFiring(bs))
    {
        friendInLOF = CheckForFriendInLOF(bs);

        if (friendInLOF)
        {
            if (PrimFiring(bs))
            {
                KeepPrimFromFiring(bs);
            }
            if (AltFiring(bs))
            {
                KeepAltFromFiring(bs);
            }
        }
    }

    if (bs->doAttack)
    {
        trap_EA_Attack(bs->client);
    }
    else if (bs->doAltAttack)
    {
        trap_EA_Alt_Attack(bs->client);
    }

    MoveTowardIdealAngles(bs);
}
//end rww

/*
==================
BotAIStartFrame
==================
*/
int BotAIStartFrame(int time)
{
#ifdef _SOF2_BOTS
    int i;
    int elapsed_time, thinktime;
    static int local_time;
    //static int botlib_residual;
    static int lastbotthink_time;

    G_CheckBotSpawn();

    //rww - addl bot frame functions
    if (gBotEdit)
    {
        trap_Cvar_Update(&bot_wp_info);
        BotWaypointRender();
    }

    UpdateEventTracker();
    //end rww

    //cap the bot think time
    //if the bot think time changed we should reschedule the bots
    if (BOT_THINK_TIME != lastbotthink_time) {
        lastbotthink_time = BOT_THINK_TIME;
        BotScheduleBotThink();
    }

    elapsed_time = time - local_time;
    local_time = time;

    if (elapsed_time > BOT_THINK_TIME) thinktime = elapsed_time;
    else thinktime = BOT_THINK_TIME;

    // execute scheduled bot AI
    for( i = 0; i < MAX_CLIENTS; i++ ) {
        if( !botstates[i] || !botstates[i]->inuse ) {
            continue;
        }
        //
        botstates[i]->botthink_residual += elapsed_time;
        //
        if ( botstates[i]->botthink_residual >= thinktime ) {
            botstates[i]->botthink_residual -= thinktime;

            if (g_entities[i].client->pers.connected == CON_CONNECTED) {
                BotAI(i, (float) thinktime / 1000);
            }
        }
    }

    // execute bot user commands every frame
    for( i = 0; i < MAX_CLIENTS; i++ ) {
        if( !botstates[i] || !botstates[i]->inuse ) {
            continue;
        }
        if( g_entities[i].client->pers.connected != CON_CONNECTED ) {
            continue;
        }

        BotUpdateInput(botstates[i], time, elapsed_time);
        trap_BotUserCommand(botstates[i]->client, &botstates[i]->lastucmd);
    }

    return qtrue;
#else
    return qfalse;
#endif
}

/*
==============
BotAISetup
==============
*/
int BotAISetup( int restart ) {
    //rww - new bot cvars..
#ifdef _DEBUG
    trap_Cvar_Register(&bot_debugmessages, "bot_debugmessages", "0", CVAR_CHEAT, 0.0, 0.0);
#endif

    trap_Cvar_Register(&bot_attachments, "bot_attachments", "1", 0, 0.0, 0.0);
    trap_Cvar_Register(&bot_camp, "bot_camp", "1", 0, 0.0, 0.0);
    trap_Cvar_Register(&bot_pause, "bot_pause", "0", 0, 0.0, 0.0);

    trap_Cvar_Register(&bot_wp_info, "bot_wp_info", "1", 0, 0.0, 0.0);
    trap_Cvar_Register(&bot_wp_edit, "bot_wp_edit", "0", CVAR_CHEAT, 0.0, 0.0);
    trap_Cvar_Register(&bot_wp_clearweight, "bot_wp_clearweight", "1", 0, 0.0, 0.0);
    trap_Cvar_Register(&bot_wp_distconnect, "bot_wp_distconnect", "1", 0, 0.0, 0.0);
    trap_Cvar_Register(&bot_wp_visconnect, "bot_wp_visconnect", "1", 0, 0.0, 0.0);

    //end rww

    //if the game is restarted for a tournament
    if (restart) {
        return qtrue;
    }

    //initialize the bot states
    memset( botstates, 0, sizeof(botstates) );

    if (!trap_BotLibSetup())
    {
        return qfalse; //wts?!
    }

    return qtrue;
}

/*
==============
BotAIShutdown
==============
*/
int BotAIShutdown( int restart ) {

    int i;

    //if the game is restarted for a tournament
    if ( restart ) {
        //shutdown all the bots in the botlib
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (botstates[i] && botstates[i]->inuse) {
                BotAIShutdownClient(botstates[i]->client, (qboolean)restart);
            }
        }
        //don't shutdown the bot library
    }
    else {
        trap_BotLibShutdown();
    }
    return qtrue;
}

