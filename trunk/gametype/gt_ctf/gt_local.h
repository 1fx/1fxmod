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
// gt_local.h -- local definitions for gametype module

#include "../../game/q_shared.h"
#include "../gt_public.h"
#include "../gt_syscalls.h"

typedef struct gametypeLocals_s
{
	int		time;

	int		redFlagDropTime;
	int		blueFlagDropTime;

	int		flagReturnSound;
	int		flagTakenSound;
	int		flagCaptureSound;

	int		redCaptureEffect;
	int		blueCaptureEffect;
	
	// Boe!Man 4/22/12: We use this for pause stuff.
	int			pauseTime;		// Restore time.
	int			paused;			// 1 when paused.
	

} gametypeLocals_t;

extern	gametypeLocals_t	gametype;





