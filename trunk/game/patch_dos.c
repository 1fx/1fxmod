// Copyright (C) 2014 - Boe!Man, Henkie.
//
// patch_dos.c - Q3 getstatus DoS patch.

//==================================================================

#include "patch_local.h"

// Local variable definitions.
static int				numRequests			= 0;		// Requests made. Reset every second.
static int				requestTimer		= 0;		// To keep track how many requests have been made in a period of time.
static qboolean			validRequest		= qtrue;	// If we exceeded our requestsPerSecond value, this will be set qfalse.
static int				requestsPerSecond	= 20;		// Number of allowed requests per second.

// Local function definitions.
#ifdef _WIN32
void	Patch_dosDetour0	(void);
#elif __linux__
void	Patch_dosDetour0	(netadr_t from);
#endif // _WIN32
int		Patch_dosDetour		(void);

/*
==================
Patch_dosDetour

The actual code of where we check if the request exceeded the number of allowed requests.
==================
*/

int Patch_dosDetour()
{
	if (level.time >= requestTimer){
		requestTimer = level.time + 1000;

		#ifdef _DEBUG
		if (numRequests >= requestsPerSecond){
			Com_Printf("DoS protection: Too many getstatus requests per second: %i (%i allowed)\n", numRequests, requestsPerSecond);
		}
		#endif // _DEBUG

		numRequests = 0;
	}
	
	if (numRequests >= requestsPerSecond){
		validRequest = qfalse;
	}else{
		validRequest = qtrue;
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
#ifdef _WIN32
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
#elif __linux__
void Patch_dosDetour0(netadr_t from){
	if(Patch_dosDetour() != 0){
		// Call SVC_Status if we're below our allowed limit, after restoring the stack.
		__asm__(
			"add $0xfffffff4,%esp \n\t"
			"add $0xffffffec,%esp \n\t"
			"mov %esp,%edi \n\t"
			"lea 0x8(%ebp),%esi \n\t"
			"cld \n"
			"mov $0x5, %ecx \n\t"
			"repz movsl %ds:(%esi),%es:(%edi) \n\t"
			"movl $0x080546a8, %eax \n\t"
			"call *%eax \n"
			);
	}
}
#endif // _WIN32
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
	#ifdef _WIN32
	Patch_detourAddress("DoS protection", (long)&Patch_dosDetour0, 0x4768A0);
	#elif __linux__
	Patch_detourAddress("DoS protection", (long)&Patch_dosDetour0, 0x8055099);
	#endif // _WIN32
}
