// Copyright (C) 2001-2002 Raven Software
//

int				OtherTeam				( team_t team );
const char*		TeamName				( team_t team );
const char*		TeamSkins				( team_t team );
const char*		OtherTeamName			( team_t team );
const char*		TeamColorString			( team_t team );
void			G_AddTeamScore			( team_t team, int score );

gentity_t*		Team_GetLocation		( gentity_t *ent );
qboolean		Team_GetLocationMsg		( gentity_t *ent, char *loc, int loclen );


