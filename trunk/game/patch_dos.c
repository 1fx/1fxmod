// Copyright (C) 2014 - Boe!Man, Henkie.
//
// patch_dos.c - Q3 getstatus DoS patch (WIN32 only).

//==================================================================

#ifdef _WIN32
#include "patch_local.h"

// Local variable definitions.
static int				numRequests			= 0;		// Requests made. Reset every second.
static unsigned long	requestTimer;					// To keep track how many requests have been made in a period of time.
static qboolean			validRequest		= qtrue;	// If we exceeded our requestsPerSecond value, this will be set qfalse.
static int				requestsPerSecond	= 20;		// Number of allowed requests per second.

// Local function definitions.
void	Patch_dosDetour0	(void);
int		Patch_dosDetour		(void);

/*
==================
Patch_dosDetour

The actual code of where we check if the request exceeded the number of allowed requests.
==================
*/

int Patch_dosDetour()
{
	if (numRequests >= requestsPerSecond){
		validRequest = qfalse;
	}else{
		validRequest = qtrue;
	}

	if (level.time >= requestTimer){
		requestTimer = level.time + 1000;

#ifdef _DEBUG
		if (numRequests >= requestsPerSecond){
			Com_Printf("DoS protection: Too many getstatus requests per second: %i (%i allowed)\n", numRequests, requestsPerSecond);
		}
#endif

		numRequests = 0;
	}

	numRequests++;

	return (int)validRequest;
}

/*
==================
Patch_dosDetour0

The detour for both GCC and VC++.
==================
*/

#ifdef __GNUC__
__asm__(".globl Patch_dosDetour0 \n\t"
	"_Patch_dosDetour0: \n"
	"call _Patch_dosDetour \n"

	"cmp $0, %eax \n\t"
	"je exitFunc \n\t"
	// Call SVC_Status if we're below our allowed limit.
	"movl $0x004760C0, %eax \n\t"
	"call *%eax \n"

	"exitFunc: \n\t"
	"push $0x004768A0 \n\t"
	"ret\n"
	);
#elif _MSC_VER
__declspec(naked) void Patch_dosDetour0()
{
	if (Patch_dosDetour() != 0){
		// Call SVC_Status if we're below our allowed limit.
		__asm{
			mov		eax, 0x4760C0;
			call	eax;
		}
	}

	__asm{
		push	0x4768A0;
		retn
	}
}
#endif // __GNUC__

/*
==================
Patch_dosProtection

Writes the jump to the detour.
==================
*/

void Patch_dosProtection()
{
	Patch_detourAddress("DoS protection", (unsigned long)&Patch_dosDetour0, 0x4768A0);
}

#endif // _WIN32
