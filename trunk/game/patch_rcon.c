// Copyright (C) 2014 - Boe!Man, Henkie.
//
// patch_rcon.c - Memory patch that enables logging from RCON.

//==================================================================

#include "patch_local.h"

// Local function definitions.
#ifdef _WIN32
void Patch_rconLogging0(void);
#endif // _WIN32
void Patch_rconLogging(const char *message, char *ip, char *command);

/*
==================
Patch_rconLogging

The actual code of where we check what command the client tries to execute.
==================
*/

void Patch_rconLogging(const char *message, char *ip, char *command)
{
	char		ip2[MAX_IP];
	char		*p;
	qboolean	validAttempt = qtrue;

	// Check if the attempt was valid.
	if (strstr(message, "Bad rcon from")){
		validAttempt = qfalse;
	}

	if (validAttempt && g_enableRconLog.integer){
		p = strstr(ip, ":");
		strncpy(ip2, ip, (int)(p - ip));

		G_LogRcon("%-40sby %s", command, ip);
	}

	Com_Printf(message, ip, command);
}

/*
==================
Patch_rconLog

The detour for both GCC and VC++ on Windows.
The Linux detour skips this step (call directly to rconLogging).
==================
*/

#ifdef _WIN32
#ifdef __GNUC__
// Determine proper address.
#ifdef _GOLD
#define ADDRESS "$0x0047908A"
#else
#define ADDRESS "$0x0047666A"
#endif // _GOLD

__asm__(".globl Patch_rconLogging0 \n\t"
	"_Patch_rconLogging0: \n"
	"call _Patch_rconLogging \n"

	"push " ADDRESS " \n\t"
	"ret\n"
	);
#elif _MSC_VER
// Determine proper address.
#ifdef _GOLD
#define ADDRESS 0x047908A
#else
#define ADDRESS 0x047666A
#endif // _GOLD

__declspec(naked) void Patch_rconLogging0()
{
	__asm {
		call	Patch_rconLogging;
		push	ADDRESS;
		retn
	}
}
#endif // __GNUC__

#undef ADDRESS
#endif // _WIN32

/*
==================
Patch_rconLog

Writes the jump to the detour.
==================
*/

void Patch_rconLog()
{
	#ifdef _WIN32
	#ifdef _GOLD
	Patch_detourAddress("RCON log patch", (long)&Patch_rconLogging0, 0x047908A);
	#else
	Patch_detourAddress("RCON log patch", (long)&Patch_rconLogging0, 0x047666A);
	#endif // _GOLD
	#endif // _WIN32
	
	#ifdef __linux__
	#ifdef _GOLD
	Patch_detourAddress("RCON log patch (1/2)", (long)&Patch_rconLogging, 0x0805876b);
	Patch_detourAddress("RCON log patch (2/2)", (long)&Patch_rconLogging, 0x08058719);
	#else
	Patch_detourAddress("RCON log patch (1/2)", (long)&Patch_rconLogging, 0x08054def);
	Patch_detourAddress("RCON log patch (2/2)", (long)&Patch_rconLogging, 0x08054d9d);
	#endif // _GOLD
	#endif // __linux__
}
