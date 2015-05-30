// Copyright (C) 2002-2012 - Raven Software, Boe!Man, Henkie.
//
// 1fx_gt.h - Local definitions from the gametypes are stored here.

//==================================================================

// Static declarations go here.
// INF
#define	ITEM_BRIEFCASE			100				
#define TRIGGER_EXTRACTION		200

// CTF
#define REDFLAG					0
#define BLUEFLAG				1

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

	// INF (and partly H&S/H&Z)
	int		caseTakenSound;
	int		caseCaptureSound; // Also used in H&S and H&Z.
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
	
	// Boe!Man 10/30/13: We use this to determine if the flag is taken.
	qboolean	flagTaken[2];
	
	// ELIM
	int		captureSound;
	
} gametypeLocals_t;

extern	gametypeLocals_t	gametype;
