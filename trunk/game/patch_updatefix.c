// Copyright (C) 2015 - Boe!Man, Henkie.
//
// patch_updatefix.c - "Update" exploit fix.

//==================================================================

#ifdef _WIN32
#include "patch_local.h"

// Local function definitions.
void        Patch_updateDetour0     ( void );
qboolean    Patch_updateDetour      ( const char *message, char *from, char *req );


/*
==================
Patch_updateDetour

The actual code of where we check if the request is valid.
==================
*/

qboolean Patch_updateDetour(const char *message, char *from, char *req)
{
    // Discard any update packets we receive.
    if(strcmp(req, "update") == 0){
        return qfalse;
    }

    // The rest of the traffic is valid.
    return qtrue;
}

/*
==================
Patch_updateDetour0

The detour for both GCC and VC++.
==================
*/

#ifdef __GNUC__
// Determine proper address.
#ifdef _GOLD
#define ADDRESS  "$0x004793a5"
#define ADDRESS2 "$0x00479528"
#elif _DEMO
#define ADDRESS  "$0x00474599"
#define ADDRESS2 "$0x00474715"
#else
#define ADDRESS  "$0x00476879"
#define ADDRESS2 "$0x004769f5"
#endif // _GOLD

__asm__(".globl Patch_updateDetour0 \n\t"
    "_Patch_updateDetour0: \n"
    "call _Patch_updateDetour \n"

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
#define ADDRESS  0x004793a5
#define ADDRESS2 0x00479528
#elif _DEMO
#define ADDRESS  0x00474599
#define ADDRESS2 0x00474715
#else
#define ADDRESS  0x00476879
#define ADDRESS2 0x004769f5
#endif // _GOLD, _DEMO or Full

__declspec(naked) void Patch_updateDetour0()
{
    __asm{
        call    Patch_updateDetour;
        cmp     eax, 0;
        je      notAllowed;
        push    ADDRESS;
        retn

    notAllowed:
        push    ADDRESS2;
        retn
    }
}
#endif // __GNUC__

#undef ADDRESS
#undef ADDRESS2

/*
==================
Patch_updateExploitProtection

Writes the jump to the detour.
==================
*/

void Patch_updateExploitProtection()
{
    #ifdef _GOLD
    Patch_detourAddress("update exploit protection", (long)&Patch_updateDetour0, 0x004793a5, qtrue);
    #elif _DEMO
    Patch_detourAddress("update exploit protection", (long)&Patch_updateDetour0, 0x00474599, qtrue);
    #else
    Patch_detourAddress("update exploit protection", (long)&Patch_updateDetour0, 0x00476879, qtrue);
    #endif // _GOLD, _DEMO or Full
}

#endif // _WIN32
