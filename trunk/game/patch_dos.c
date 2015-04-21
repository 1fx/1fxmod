// Copyright (C) 2014 - Boe!Man, Henkie.
//
// patch_dos.c - Q3 getstatus DoS patch.

//==================================================================

#include "patch_local.h"

typedef enum {
	GETSTATUS,
	GETINFO,

	MAX_REQUEST
} request_t;

// Local variable definitions.
#ifdef _DEBUG
static const char		nameReq[MAX_REQUEST][10]	= { "getstatus", "getinfo" };
#endif // _DEBUG

static int				numRequests[MAX_REQUEST]	= { 0 };	// Requests made. Reset every second.
static int				requestTimer				= 0;		// To keep track how many requests have been made in a period of time.
static const int		requestsPerSecond			= 20;		// Number of allowed requests per second.

// Local function definitions.
#ifdef _WIN32
void		Patch_dosDetour0	(void);
qboolean	Patch_dosDetour		(const char *message, char *from, char *request);
#elif __linux__
void		Patch_dosDetour		(const char *message, char *from, char *request);
#endif // _WIN32


/*
==================
Patch_dosDetour

The actual code of where we check if the request exceeded the number of allowed requests.
==================
*/

#ifdef _WIN32
qboolean Patch_dosDetour(const char *message, char *from, char *request)
#elif __linux__
void Patch_dosDetour(const char *message, char *from, char *request)
#endif // _WIN32
{
	int req = -1;
	#ifdef _DEBUG
	int i;
	#endif // _DEBUG

	if (strcmp(request, "getstatus") == 0)
		req = GETSTATUS;
	else if (strcmp(request, "getinfo") == 0)
		req = GETINFO;

	if (level.time >= requestTimer){
		requestTimer = level.time + 1000;

		#ifdef _DEBUG
		for (i = 0; i < MAX_REQUEST; i++){
			if (numRequests[i] >= requestsPerSecond)
				Com_Printf("DoS protection: Too many %s requests per second: %d (%d allowed)\n", nameReq[i], numRequests[i], requestsPerSecond);
		}
		#endif // _DEBUG

		memset(numRequests, 0, sizeof(numRequests));
	}

	if (req != -1){
		numRequests[req]++;

		if (numRequests[req] >= requestsPerSecond){
			#ifdef __linux__
			#ifdef _GOLD
			__asm__("add $0x2c, %esp \n\t"
					"pop %ebx \n\t"
					"pop %esi \n\t"
					"pop %edi \n\t"
					"pop %ebp \n\t"
					"push $0x08058d26 \n\t"
					"ret \n");
			#else
			__asm__("lea 0xffffffd8(%ebp), %esp \n\t"
				"pop %ebx \n\t"
				"pop %esi \n\t"
				"pop %edi \n\t"
				"leave \n"
				"push $0x08055263 \n\t"
				"ret \n");
			#endif // _GOLD
			#elif _WIN32
			return qfalse;
			#endif // __linux__
		}
	}

	#ifdef _WIN32
	return qtrue;
	#endif // _WIN32
}

/*
==================
Patch_dosDetour0

The detour for both GCC and VC++.
==================
*/

#ifdef _WIN32
#ifdef __GNUC__
// Determine proper address.
#ifdef _GOLD
#define ADDRESS  "$0x004793A5"
#define ADDRESS2 "$0x00479528"
#else
#define ADDRESS  "$0x00476879"
#define ADDRESS2 "$0x004769F5"
#endif // _GOLD

__asm__(".globl Patch_dosDetour0 \n\t"
	"_Patch_dosDetour0: \n"
	"call _Patch_dosDetour \n"

	"cmp $0, %eax \n\t"
	"je notAllowed \n\t"
	"push " ADDRESS " \n\t"
	"ret \n"

	"notAllowed: \n\t"
	"push " ADDRESS2 " \n\t"
	"ret \n"
	);
#elif _MSC_VER
// Determine proper address.
#ifdef _GOLD
#define ADDRESS	 0x4793A5
#define ADDRESS2 0x479528
#else
#define ADDRESS  0x476879
#define ADDRESS2 0x4769F5
#endif // _GOLD

__declspec(naked) void Patch_dosDetour0()
{
	__asm{
		call	Patch_dosDetour;
		cmp		eax, 0;
		je		notAllowed;
		push	ADDRESS;
		retn

	notAllowed:
		push	ADDRESS2;
		retn
	}
}
#endif // __GNUC__

#undef ADDRESS
#undef ADDRESS2
#endif // _WIN32

/*
==================
Patch_dosProtection

Writes the jump to the detour.
==================
*/

void Patch_dosProtection()
{
	#ifdef _WIN32
	#ifdef _GOLD
	Patch_detourAddress("DoS protection", (long)&Patch_dosDetour0, 0x4793A5);
	#else
	Patch_detourAddress("DoS protection", (long)&Patch_dosDetour0, 0x476879);
	#endif // _GOLD
	#endif // _WIN32
	
	#ifdef __linux__
	#ifdef _GOLD
	Patch_detourAddress("DoS protection", (long)&Patch_dosDetour, 0x08058b1f);
	#else
	Patch_detourAddress("DoS protection", (long)&Patch_dosDetour, 0x0805505c);
	#endif // _GOLD
	#endif // __linux__
}
