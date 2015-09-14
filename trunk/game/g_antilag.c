// Copyright (C) 2000-2001 Raven Software, Inc.
//
// g_antilag.c -- handles server side anti-lag 

#include "g_local.h"


/*
================
G_UpdateClientAntiLag
================
*/
void G_UpdateClientAntiLag ( gentity_t* ent )
{
    int         head;
    int         newtime;

    head = ent->client->antilagHead;

    // If on a new frame snap the head up to the end of the last frame and 
    // add a new head
    if ( ent->client->antilag[head].leveltime < level.time )
    {
        ent->client->antilag[head].time = level.previousTime;

        // Move to the next position 
        if ( (++ent->client->antilagHead) > MAX_ANTILAG )
        {
            ent->client->antilagHead = 0;
        }

        head = ent->client->antilagHead;
    }

    // Bots only move once per frame
    if ( ent->r.svFlags & SVF_BOT ) 
    {
        newtime = level.time;
    } 
    else 
    {
        // calculate the actual server time     
        newtime = level.previousTime + trap_Milliseconds() - level.frameStartTime;
        
        if ( newtime > level.time ) 
        {
            newtime = level.time;
        } 
        else if ( newtime <= level.previousTime ) 
        {
            newtime = level.previousTime + 1;
        }
    }

    // Copy the clients current state into the antilag cache
    ent->client->antilag[head].leveltime = level.time;
    ent->client->antilag[head].time = newtime;
    VectorCopy ( ent->r.currentOrigin, ent->client->antilag[head].rOrigin );
    VectorCopy ( ent->r.currentAngles, ent->client->antilag[head].rAngles );
    VectorCopy ( ent->r.mins, ent->client->antilag[head].mins );
    VectorCopy ( ent->r.maxs, ent->client->antilag[head].maxs );

    VectorCopy ( ent->client->ghoulLegsAngles, ent->client->antilag[head].legsAngles );
    VectorCopy ( ent->client->ghoulLowerTorsoAngles, ent->client->antilag[head].lowerTorsoAngles );
    VectorCopy ( ent->client->ghoulUpperTorsoAngles, ent->client->antilag[head].upperTorsoAngles );
    VectorCopy ( ent->client->ghoulHeadAngles, ent->client->antilag[head].headAngles );

    ent->client->antilag[head].legsAnim  = ent->s.legsAnim;
    ent->client->antilag[head].torsoAnim = ent->s.torsoAnim;
    ent->client->antilag[head].pm_flags  = ent->client->ps.pm_flags;
    ent->client->antilag[head].leanTime  = ent->client->ps.leanTime;
}

/*
================
G_UndoClientAntiLag
================
*/
void G_UndoClientAntiLag ( gentity_t* ent )
{
    // If the client isnt already in the past then 
    // dont bother doing anything
    if ( ent->client->antilagUndo.leveltime != level.time  )
        return;

    // Move the client back into reality by moving over the undo information
    VectorCopy ( ent->client->antilagUndo.rOrigin, ent->r.currentOrigin );
    VectorCopy ( ent->client->antilagUndo.rAngles, ent->r.currentAngles );
    VectorCopy ( ent->client->antilagUndo.mins, ent->r.mins );
    VectorCopy ( ent->client->antilagUndo.maxs, ent->r.maxs );

    VectorCopy ( ent->client->antilagUndo.legsAngles, ent->client->ghoulLegsAngles );
    VectorCopy ( ent->client->antilagUndo.lowerTorsoAngles, ent->client->ghoulLowerTorsoAngles );
    VectorCopy ( ent->client->antilagUndo.upperTorsoAngles, ent->client->ghoulUpperTorsoAngles );
    VectorCopy ( ent->client->antilagUndo.headAngles, ent->client->ghoulHeadAngles );

    ent->s.legsAnim = ent->client->antilagUndo.legsAnim;
    ent->s.torsoAnim = ent->client->antilagUndo.torsoAnim;
    ent->client->ps.pm_flags = ent->client->antilagUndo.pm_flags;
    ent->client->ps.leanTime = ent->client->antilagUndo.leanTime;

    // Mark the undo information so it cant be used again
    ent->client->antilagUndo.leveltime = 0;
}

/*
================
G_ApplyClientAntiLag
================
*/
void G_ApplyClientAntiLag ( gentity_t* ent, int time )
{
    float   lerp;
    int     from;
    int     to;

    // Find the two pieces of history information that sandwitch the
    // time we are looking for
    from = ent->client->antilagHead;
    to   = ent->client->antilagHead;
    do
    {
        if ( ent->client->antilag[from].time <= time )
        {
            break;
        }

        to = from;
        from--;

        if ( from < 0 )
        {
            from = MAX_ANTILAG - 1;
        }
    }
    while ( from != ent->client->antilagHead );

    // If the from is equal to the to then there wasnt even
    // one piece of information worth using so just use the current time frame
    if ( from == to )
    {
        return;
    }

    // Save the undo information if its not already saved
    if ( ent->client->antilagUndo.leveltime != level.time )
    {
        // Save the undo information
        ent->client->antilagUndo.leveltime = level.time;

        VectorCopy ( ent->r.currentOrigin, ent->client->antilagUndo.rOrigin );
        VectorCopy ( ent->r.currentAngles, ent->client->antilagUndo.rAngles );
        VectorCopy ( ent->r.mins, ent->client->antilagUndo.mins );
        VectorCopy ( ent->r.maxs, ent->client->antilagUndo.maxs );

        VectorCopy ( ent->client->ghoulLegsAngles, ent->client->antilagUndo.legsAngles );
        VectorCopy ( ent->client->ghoulLowerTorsoAngles, ent->client->antilagUndo.lowerTorsoAngles );
        VectorCopy ( ent->client->ghoulUpperTorsoAngles, ent->client->antilagUndo.upperTorsoAngles );
        VectorCopy ( ent->client->ghoulHeadAngles, ent->client->antilagUndo.headAngles );

        ent->client->antilagUndo.legsAnim  = ent->s.legsAnim;
        ent->client->antilagUndo.torsoAnim = ent->s.torsoAnim;
        ent->client->antilagUndo.pm_flags  = ent->client->ps.pm_flags;
        ent->client->antilagUndo.leanTime  = ent->client->ps.leanTime;
    }

    // If the best history found was the last in the list then
    // dont lerp, just use the last one
    if ( from == ent->client->antilagHead )
    {
        VectorCopy ( ent->client->antilag[to].rOrigin, ent->r.currentOrigin );
        VectorCopy ( ent->client->antilag[to].rAngles, ent->r.currentAngles );
        VectorCopy ( ent->client->antilag[to].mins, ent->r.maxs );
        VectorCopy ( ent->client->antilag[to].maxs, ent->r.mins );

        VectorCopy ( ent->client->antilag[to].legsAngles, ent->client->ghoulLegsAngles );
        VectorCopy ( ent->client->antilag[to].lowerTorsoAngles, ent->client->ghoulLowerTorsoAngles );
        VectorCopy ( ent->client->antilag[to].upperTorsoAngles, ent->client->ghoulUpperTorsoAngles );
        VectorCopy ( ent->client->antilag[to].headAngles, ent->client->ghoulHeadAngles );

        ent->s.legsAnim  = ent->client->antilag[to].legsAnim;
        ent->s.torsoAnim = ent->client->antilag[to].torsoAnim;
        ent->client->ps.pm_flags = ent->client->antilag[to].pm_flags;
        ent->client->ps.leanTime = ent->client->antilag[to].leanTime;
    }
    else
    {
        // Calculate the lerp value to use for the vectors
        lerp = (float)(time - ent->client->antilag[from].time) / (float)(ent->client->antilag[to].time - ent->client->antilag[from].time);

        // Lerp all the vectors between the before and after history information
        LerpVector ( ent->client->antilag[from].rOrigin, ent->client->antilag[to].rOrigin, lerp, ent->r.currentOrigin );
        LerpVector ( ent->client->antilag[from].rAngles, ent->client->antilag[to].rAngles, lerp, ent->r.currentAngles );
        LerpVector ( ent->client->antilag[from].maxs, ent->client->antilag[to].maxs, lerp, ent->r.maxs );
        LerpVector ( ent->client->antilag[from].mins, ent->client->antilag[to].mins, lerp, ent->r.mins );

        LerpVector ( ent->client->antilag[from].legsAngles, ent->client->antilag[to].legsAngles, lerp, ent->client->ghoulLegsAngles );
        LerpVector ( ent->client->antilag[from].lowerTorsoAngles, ent->client->antilag[to].lowerTorsoAngles, lerp, ent->client->ghoulLowerTorsoAngles );
        LerpVector ( ent->client->antilag[from].upperTorsoAngles, ent->client->antilag[to].upperTorsoAngles, lerp, ent->client->ghoulUpperTorsoAngles );
        LerpVector ( ent->client->antilag[from].headAngles, ent->client->antilag[to].headAngles, lerp, ent->client->ghoulHeadAngles );

        ent->client->ps.leanTime = ent->client->antilag[from].leanTime + (ent->client->antilag[from].leanTime-ent->client->antilag[to].leanTime) * lerp;

        ent->s.legsAnim  = ent->client->antilag[to].legsAnim;
        ent->s.torsoAnim = ent->client->antilag[to].torsoAnim;
        ent->client->ps.pm_flags = ent->client->antilag[to].pm_flags;
    }
}

/*
================
G_UndoAntiLag
================
*/
void G_UndoAntiLag ( void )
{
    int i;

    // Undo all history
    for ( i = 0; i < level.numConnectedClients; i ++ )
    {
        gentity_t* other = &g_entities[level.sortedClients[i]];
        
        if ( other->client->pers.connected != CON_CONNECTED )
        {
            continue;
        }

        // Skip clients that are spectating
        if ( G_IsClientSpectating ( other->client ) || G_IsClientDead ( other->client ) )
        {
            continue;
        }

        //Ryan april 6 2004 9:05pm
        //Taken from gold code, for fixing leaning player hits
        if ( other->r.svFlags & SVF_DOUBLED_BBOX )
        {
            // Put the hitbox back the way it was
            other->r.maxs[0] = other->client->maxSave[0];
            other->r.maxs[1] = other->client->maxSave[1];
            other->r.maxs[2] = other->client->maxSave[2];

            other->r.mins[0] = other->client->minSave[0];
            other->r.mins[1] = other->client->minSave[1];
            other->r.mins[2] = other->client->minSave[2];

            other->r.svFlags &= (~SVF_DOUBLED_BBOX);
        }
        
        /*if ( other->r.svFlags & SVF_DOUBLED_BBOX )
        {
            // Put the hitbox back the way it was
            other->r.maxs[0] /= 2;
            other->r.maxs[1] /= 2;
            other->r.mins[0] /= 2;
            other->r.mins[1] /= 2;

            other->r.svFlags &= (~SVF_DOUBLED_BBOX);
        }*/
        //Ryan

        G_UndoClientAntiLag ( other );

        // Relink the entity into the world
        trap_LinkEntity ( other );
    }
}

/*
================
G_ApplyAntiLag
================
*/
void G_ApplyAntiLag ( gentity_t* ref, qboolean enlargeHitBox )
{
    int i;
    int reftime;

    // Figure out the reference time based on the reference clients server time
    reftime = ref->client->pers.cmd.serverTime;
    if ( reftime > level.time ) 
    {
        reftime = level.time;
    }

    // Move all the clients back into the reference clients time frame.
    for ( i = 0; i < level.numConnectedClients; i ++ )
    {
        gentity_t* other = &g_entities[level.sortedClients[i]];

        if ( other->client->pers.connected != CON_CONNECTED )
        {
            continue;
        }

        // Skip the reference client
        if ( other == ref )
        {
            continue;
        }

        // Skip entities not in use
        if ( !other->inuse )
        {
            continue;
        }

        // Skip clients that are spectating
        if ( G_IsClientSpectating ( other->client ) || G_IsClientDead ( other->client ) )
        {
            continue;
        }

        // Dont bring them back in time unless requested
        if ( !(ref->r.svFlags & SVF_BOT) & ref->client->pers.antiLag )
        {
            // Apply the antilag to this player
            G_ApplyClientAntiLag ( other, reftime );
        }

        //Ryan april 6 2004 9:01pm   
        //took this from gold code, its much better for hitting leaning players
        if ( enlargeHitBox )
        {
            other->client->minSave[0] = other->r.mins[0];
            other->client->minSave[1] = other->r.mins[1];
            other->client->minSave[2] = other->r.mins[2];

            other->client->maxSave[0] = other->r.maxs[0];
            other->client->maxSave[1] = other->r.maxs[1];
            other->client->maxSave[2] = other->r.maxs[2];

            if ( other->client->ps.pm_flags & PMF_DUCKED )
            {
                other->r.maxs[2] += 10;
            }

            // Adjust the hit box to account for hands and such 
            // that are sticking out of the normal bounding box

            if ( other->client->ps.pm_flags & PMF_LEANING )
            {
                other->r.maxs[0] *= 3.0f;
                other->r.maxs[1] *= 3.0f;
                other->r.mins[0] *= 3.0f;
                other->r.mins[1] *= 3.0f;
            }
            else
            {
                other->r.maxs[0] *= 2.0f;
                other->r.maxs[1] *= 2.0f;
                other->r.mins[0] *= 2.0f;
                other->r.mins[1] *= 2.0f;
            }

            other->r.svFlags |= SVF_DOUBLED_BBOX;
        }
        /*if ( enlargeHitBox )
        {
            // Adjust the hit box to account for hands and such 
            // that are sticking out of the normal bounding box
            other->r.maxs[0] *= 2;
            other->r.maxs[1] *= 2;
            other->r.mins[0] *= 2;
            other->r.mins[1] *= 2;
            other->r.svFlags |= SVF_DOUBLED_BBOX;
        }*/
        //Ryan

        // Relink the entity into the world
        trap_LinkEntity ( other );
    }   
}
