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
// cg_lights.h --

#pragma once
#if !defined(CG_LIGHTS_H_INC)
#define CG_LIGHTS_H_INC

typedef struct
{
	int				length;
	color4ub_t		value;
	color4ub_t		map[MAX_QPATH];
} clightstyle_t;

void	CG_ClearLightStyles (void);
void	CG_RunLightStyles (void);
void	CG_SetLightstyle (int i);

#endif // CG_LIGHTS_H_INC
