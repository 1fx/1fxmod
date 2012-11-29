// Copyright (C) 2002-2012 - Raven Software, Boe!Man, Henkie.
//
// 1fx_gt.h - Local definitions from the gametypes are stored here.

//==================================================================

typedef struct gametypeLocals_s
{
	int		time;

	int		caseTakenSound;		// INF
	int		caseCaptureSound;	// INF
	int		caseReturnSound;	// INF

} gametypeLocals_t;

extern	gametypeLocals_t	gametype;
