// Copyright (C) 2015 - Boe!Man, Henkie.
//
// patch_q3infofix.c - "q3infofix" memory patch.

//==================================================================

#include "patch_local.h"

// Local function definitions.
void	Patch_connectionlessPacketDetour	(char *packet);

/*
==================
Patch_connectionlessPacketDetour

The code of where we check the packet length, and if too big, truncate it.
==================
*/

#ifdef __GNUC__
#ifdef __linux__
#ifdef _GOLD
#define ADDRESS "$0x0807edd4"
#else
#define ADDRESS "$0x0807add8"
#endif // _GOLD
#elif _WIN32
#ifdef _GOLD
#define ADDRESS "$0x004459D0"
#else
#define ADDRESS "$0x00444B00"
#endif // _GOLD
#endif // __linux__
#elif _MSC_VER
#ifdef _GOLD
#define ADDRESS 0x004459D0
#else
#define ADDRESS 0x00444B00
#endif // _GOLD
#endif // __GNUC__

void Patch_connectionlessPacketDetour(char *packet)
{
	// Check length of packet.
	if (strlen(packet) > 320){
		Com_Printf("WARNING: q3infoboom exploit attack.\n");
		packet[320] = '\0'; // Truncate it.
	}

	// Push the parameter back and call the original function.
	#ifdef __GNUC__
	__asm__("push %0 \n\t"
			"movl " ADDRESS ", %%eax \n\t"
			"call *%%eax \n"
			"popl %%eax \n\t"
			:
            : "a" (packet));
	#elif _MSC_VER
	__asm {
		push	packet;
		mov		eax, ADDRESS;
		call	eax;
	}
	#endif // __GNUC__
}

#undef ADDRESS

/*
==================
Patch_autoDownloadExploit

Writes the jump to the detour.
==================
*/

void Patch_q3infofix()
{
	#ifdef _WIN32
	#ifdef _GOLD
	Patch_detourAddress("q3infoboom/q3msgboom protection", (long)&Patch_connectionlessPacketDetour, 0x00479375, qfalse);
	#else
	Patch_detourAddress("q3infoboom/q3msgboom protection", (long)&Patch_connectionlessPacketDetour, 0x00476849, qfalse);
	#endif // _GOLD
	#elif __linux__
	#ifdef _GOLD
	Patch_detourAddress("q3infoboom/q3msgboom protection", (long)&Patch_connectionlessPacketDetour, 0x08058ad1, qfalse);
	#else
	Patch_detourAddress("q3infoboom/q3msgboom protection", (long)&Patch_connectionlessPacketDetour, 0x0805500e, qfalse);
	#endif // _GOLD
	#endif // _WIN32
}
