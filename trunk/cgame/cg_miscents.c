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
// cg_miscents.h --

#include "cg_local.h"

#define MAX_MISC_ENTS	4000

static refEntity_t	*MiscEnts = 0;
static float		*Radius = 0;
static int			NumMiscEnts = 0;

void CG_MiscEnt(void)
{
	int			modelIndex;
	refEntity_t	*RefEnt;
	TCGMiscEnt	*data = (TCGMiscEnt *)cg.sharedBuffer;
	vec3_t		mins, maxs;
	float		*radius;

	if (NumMiscEnts >= MAX_MISC_ENTS)
	{
		return;
	}

	if (!MiscEnts)
	{
		MiscEnts = (refEntity_t *)trap_VM_LocalAlloc(sizeof(refEntity_t)*MAX_MISC_ENTS);
		Radius = (float *)trap_VM_LocalAlloc(sizeof(float)*MAX_MISC_ENTS);
	}
	
	radius = &Radius[NumMiscEnts];
	RefEnt = &MiscEnts[NumMiscEnts++];

	modelIndex = trap_R_RegisterModel(data->mModel);
	if (modelIndex == 0)
	{
		Com_Error(ERR_DROP, "client_model has invalid model definition");
		return;
	}

	memset(RefEnt, 0, sizeof(refEntity_t));
	RefEnt->reType = RT_MODEL;
	RefEnt->hModel = modelIndex;
	RefEnt->frame = 0;
	trap_R_ModelBounds(modelIndex, mins, maxs);
	VectorCopy(data->mScale, RefEnt->modelScale);
	VectorCopy(data->mOrigin, RefEnt->origin);

	VectorScaleVector(mins, data->mScale, mins);
	VectorScaleVector(maxs, data->mScale, maxs);
	*radius = Distance(mins, maxs);

	AnglesToAxis( data->mAngles, RefEnt->axis );
	CG_ScaleModelAxis(RefEnt);
}

void CG_DrawMiscEnts(void)
{
	int			i;
	refEntity_t	*RefEnt;
	float		*radius;
	vec3_t		difference;

	RefEnt = MiscEnts;
	radius = Radius;
	for(i=0;i<NumMiscEnts;i++)
	{
		VectorSubtract(RefEnt->origin, cg.refdef.vieworg, difference);
		if (VectorLength(difference)-(*radius) <= RMG_distancecull.value)
		{
			trap_R_AddRefEntityToScene(RefEnt);
		}
		RefEnt++;
		radius++;
	}
}
