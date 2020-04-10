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

#include "g_local.h"

/*
==============
OtherTeam
==============
*/
int OtherTeam(team_t team)
{
    if (team==TEAM_RED)
        return TEAM_BLUE;
    else if (team==TEAM_BLUE)
        return TEAM_RED;
    return team;
}

/*
==============
TeamName
==============
*/
const char *TeamName(team_t team)
{
    switch ( team )
    {
        case TEAM_RED:
            if(current_gametype.value == GT_HS)
                return "Hiders";
            else
                return "RED";
        case TEAM_BLUE:
            if(current_gametype.value == GT_HS)
                return "Seekers";
            else
                return "BLUE";
        case TEAM_FREE:
            return "FREE";

        case TEAM_SPECTATOR:
            return "SPECTATOR";
    }

    return "";
}

/*
==============
OtherTeamName
==============
*/
const char *OtherTeamName(team_t team)
{
    if (team==TEAM_RED)
        return TeamName ( TEAM_BLUE );
    else if (team==TEAM_BLUE)
        return TeamName ( TEAM_BLUE );

    return TeamName ( team );
}

/*
==============
TeamColorString
==============
*/
const char *TeamColorString(team_t team)
{
    if (team==TEAM_RED)
        return S_COLOR_RED;
    else if (team==TEAM_BLUE)
        return S_COLOR_BLUE;
    else if (team==TEAM_SPECTATOR)
        return S_COLOR_YELLOW;

    return S_COLOR_WHITE;
}

/*
==============
G_AddTeamScore

used for gametype > GT_TDM
for gametype GT_TDM the level.teamScores is updated in AddScore in g_combat.c
==============
*/
void G_AddTeamScore( team_t team, int score )
{
    // Dont allow negative scores to affect the team score.  The reason for this is
    // that negative scores come from the actions of one bad player and a single player
    // can cause a team to loose because he/she wants to just kill the rest of their team, or
    // continue to kill themselves.
    if ( score < 0 )
    {
        return;
    }

    level.teamScores[ team ] += score;
}

/*
==============
OnSameTeam
==============
*/
qboolean OnSameTeam( gentity_t *ent1, gentity_t *ent2 )
{
    if ( !ent1->client || !ent2->client )
    {
        return qfalse;
    }

    if ( !level.gametypeData->teams )
    {
        return qfalse;
    }

    if ( ent1->client->sess.team == ent2->client->sess.team )
    {
        return qtrue;
    }

    return qfalse;
}
/*
===========
Team_GetLocation

Report a location for the player. Uses placed nearby target_location entities
============
*/
gentity_t *Team_GetLocation(gentity_t *ent)
{
    gentity_t       *eloc, *best;
    float           bestlen, len;
    vec3_t          origin;

    best = NULL;
    bestlen = 3*8192.0*8192.0;

    VectorCopy( ent->r.currentOrigin, origin );

    for (eloc = level.locationHead; eloc; eloc = eloc->nextTrain)
    {
        len = ( origin[0] - eloc->r.currentOrigin[0] ) * ( origin[0] - eloc->r.currentOrigin[0] )
            + ( origin[1] - eloc->r.currentOrigin[1] ) * ( origin[1] - eloc->r.currentOrigin[1] )
            + ( origin[2] - eloc->r.currentOrigin[2] ) * ( origin[2] - eloc->r.currentOrigin[2] );

        if ( len > bestlen )
        {
            continue;
        }

        if ( !trap_InPVS( origin, eloc->r.currentOrigin ) )
        {
            continue;
        }

        bestlen = len;
        best = eloc;
    }

    return best;
}


/*
===========
Team_GetLocationMsg
============
*/
qboolean Team_GetLocationMsg ( gentity_t *ent, char *loc, int loclen )
{
    gentity_t *best;

    best = Team_GetLocation( ent );

    if (!best)
    {
        return qfalse;
    }

    Com_sprintf(loc, loclen, "%s", best->message);

    return qtrue;
}

