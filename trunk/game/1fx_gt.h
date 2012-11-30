// Copyright (C) 2002-2012 - Raven Software, Boe!Man, Henkie.
//
// 1fx_gt.h - Local definitions from the gametypes are stored here.

//==================================================================

// Static declarations go here.
// INF
#define	ITEM_BRIEFCASE			100				
#define TRIGGER_EXTRACTION		200

// CTF
#define	ITEM_REDFLAG			100
#define ITEM_BLUEFLAG			101
								
#define TRIGGER_REDCAPTURE		200
#define TRIGGER_BLUECAPTURE		201

// CVAR internal declarations.
vmCvar_t	gt_simpleScoring;
vmCvar_t	gt_flagReturnTime;

typedef struct gametypeLocals_s
{
	int		time;

	// INF
	int		caseTakenSound;
	int		caseCaptureSound;
	int		caseReturnSound;

	// CTF
	int		redFlagDropTime;
	int		blueFlagDropTime;
	
	int		flagReturnSound;
	int		flagTakenSound;
	int		flagCaptureSound;
	
	int		redCaptureEffect;
	int		blueCaptureEffect;
	
	// Boe!Man 4/22/12: We use this for pause stuff.
	int			pauseTime;		// Restore time.
	
	// ELIM
	int		captureSound;
	
} gametypeLocals_t;

extern	gametypeLocals_t	gametype;
