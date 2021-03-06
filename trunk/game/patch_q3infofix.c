/*
===========================================================================
Copyright (C) 2015 - 2020, Ane-Jouke Schat (Boe!Man)
Copyright (C) 2015 - 2020, Jordi Epema (Henkie)

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
// patch_q3infofix.c - "q3infofix" memory patch.

//==================================================================

#include "patch_local.h"

// Local function definitions.
void    Patch_connectionlessPacketDetour    (char *packet);

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
#define ADDRESS "$0x004459d0"
#elif _DEMO
#define ADDRESS "$0x00443560"
#else
#define ADDRESS "$0x00444b00"
#endif // _GOLD, _DEMO or Full
#endif // __linux__
#elif _MSC_VER
#ifdef _GOLD
#define ADDRESS 0x004459d0
#elif _DEMO
#define ADDRESS 0x00443560
#else
#define ADDRESS 0x00444b00
#endif // _GOLD, _DEMO or Full
#endif // __GNUC__

void Patch_connectionlessPacketDetour(char *packet)
{
    int packetLength = strlen(packet);

    // Check length of packet.
    if (packetLength > 320){
        // Only warn people when a genuine attack happens.
        if(packetLength > 512){
            Com_Printf("WARNING: q3infoboom exploit attack.\n");
        }
        packet[320] = '\0'; // Truncate it.
    }

    // Push the parameter back and call the original function.
    #if defined(__GNUC__) && !defined(MACOS_X) // FIXME: Mac OS X implementation later.
    __asm__("push %0 \n\t"
            "movl " ADDRESS ", %%eax \n\t"
            "call *%%eax \n"
            "popl %%eax \n\t"
            :
            : "a" (packet));
    #elif _MSC_VER
    __asm {
        push    packet;
        mov     eax, ADDRESS;
        call    eax;
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
    #elif _DEMO
    Patch_detourAddress("q3infoboom/q3msgboom protection", (long)&Patch_connectionlessPacketDetour, 0x00474569, qfalse);
    #else
    Patch_detourAddress("q3infoboom/q3msgboom protection", (long)&Patch_connectionlessPacketDetour, 0x00476849, qfalse);
    #endif // _GOLD, _DEMO or Full
    #elif __linux__
    #ifdef _GOLD
    Patch_detourAddress("q3infoboom/q3msgboom protection", (long)&Patch_connectionlessPacketDetour, 0x08058ad1, qfalse);
    #else
    Patch_detourAddress("q3infoboom/q3msgboom protection", (long)&Patch_connectionlessPacketDetour, 0x0805500e, qfalse);
    #endif // _GOLD
    #endif // _WIN32
}
