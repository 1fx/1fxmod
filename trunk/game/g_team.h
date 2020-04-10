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

int             OtherTeam               ( team_t team );
const char*     TeamName                ( team_t team );
const char*     TeamSkins               ( team_t team );
const char*     OtherTeamName           ( team_t team );
const char*     TeamColorString         ( team_t team );
void            G_AddTeamScore          ( team_t team, int score );

gentity_t*      Team_GetLocation        ( gentity_t *ent );
qboolean        Team_GetLocationMsg     ( gentity_t *ent, char *loc, int loclen );


