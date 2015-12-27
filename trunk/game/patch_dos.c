// Copyright (C) 2015 - Boe!Man, Henkie.
//
// patch_dos.c - Q3 connectionless packet DoS patch.

//==================================================================

#include "patch_local.h"

typedef struct request_s {
    int             numRequests;
    unsigned int    ip;
    #ifdef _DEBUG
    qboolean        exceededMessage;
    #endif // _DEBUG
}request_t;

#define DOS_REQUESTS_IP     64  // Maximum clients allowed to make requests simultaniously.
#define DOS_REQUESTS_SEC    5   // Maximum requests from one client per second.

// Local variable definitions.
static request_t        requestsPerIP[DOS_REQUESTS_IP]  = { 0 };    // Requests made per IP. Resets every second.
static int              requestTimer                    = 0;        // To keep track how many requests have been made in a period of time.
static qboolean         blockAllTraffic                 = qfalse;   // This is true if all clients slots are fully flooded.

// Local function definitions.
void        Patch_dosDetour0    ( void );
qboolean    Patch_dosDetour     ( const char *message, char *from, char *req );


/*
==================
Patch_dosDetour

The actual code of where we check if the request exceeded the number of allowed requests.
==================
*/

qboolean Patch_dosDetour(const char *message, char *from, char *req)
{
    int         i, ipNum;
    char        ip[MAX_IP];
    request_t   *request = NULL;

    if(blockAllTraffic && level.time < requestTimer){
        return qfalse;
    }

    if(level.time > requestTimer){
        // Second has passed, we can reset our state.
        requestTimer = level.time + 1000;
        blockAllTraffic = qfalse;

        // Check if we can flush the client table.
        memset(requestsPerIP, 0, sizeof(requestsPerIP));

        // Safe to assume we can take the first slot.
        request = &requestsPerIP[0];
    }

    // Get the IP number.
    Q_strncpyz(ip, from, sizeof(ip));
    if(ip[0] == 0|| strtok(ip, ":") == NULL || (ipNum = (G_IP2Integer(ip))) == 0){
        // Invalid IP specified, don't allow this packet.
        // We don't print a warning message unless in local debug mode,
        // so this can never be abused on live servers (print spam).
        #ifdef _NIGHTLY
        Com_Printf("DoS protection: invalid IP: %s -> %u\n", ip, ipNum);
        #endif // _NIGHTLY
        return qfalse;
    }

    // Only find an empty slot when we didn't get a client number assigned yet.
    // Most of the time only one or perhaps two requests are made every second.
    // This check significantly speeds up the search for a free slot.
    if(request == NULL){
        // Find the client number.
        for(i = 0; i < DOS_REQUESTS_IP; i++){
            if(requestsPerIP[i].ip == ipNum){
                request = &requestsPerIP[i];
                break;
            }
        }

        // Find an empty slot if we didn't encounter this IP yet.
        if(request == NULL){
            for(i = 0; i < DOS_REQUESTS_IP; i++){
                if(requestsPerIP[i].ip == 0){
                    request = &requestsPerIP[i];
                    request->ip = ipNum;
                    break;
                }
            }
        }
    }

    // There is no free slot left or an existing one found.
    // This means someone is flooding us, from different IPs..
    if(request == NULL){
        Com_Printf("DoS protection: no more free IP slots for incoming traffic for another %d msec.\n", requestTimer - level.time);

        blockAllTraffic = qtrue;
        return qfalse;
    }

    // Check if this client sent too many requests already.
    if (request->numRequests > DOS_REQUESTS_SEC){
        // Too many requests for IP.
        #ifdef _DEBUG
        if(!request->exceededMessage){
            Com_Printf("DoS protection: Too many requests per second from IP: %s, blocking for %d msec.\n", ip, requestTimer - level.time);
            request->exceededMessage = qtrue;
        }
        #endif // _DEBUG

        return qfalse;
    }
    request->numRequests++;

    return qtrue;
}

/*
==================
Patch_dosDetour0

The detour for both GCC and VC++.
==================
*/

#ifdef __GNUC__
// Determine proper address.
#ifdef _GOLD
#ifdef __linux__
#define ADDRESS  "$0x08058b1f"
#define ADDRESS2 "$0x08058d26"
#elif _WIN32
#define ADDRESS  "$0x004793a5"
#define ADDRESS2 "$0x00479528"
#endif // __linux__
#else
#ifdef __linux__
#define ADDRESS  "$0x0805505c"
#define ADDRESS2 "$0x08055263"
#elif _WIN32
#define ADDRESS  "$0x00476879"
#define ADDRESS2 "$0x004769f5"
#endif // __linux__
#endif // _GOLD

__asm__(".globl Patch_dosDetour0 \n\t"
    #ifdef _WIN32
    "_Patch_dosDetour0: \n"
    "call _Patch_dosDetour \n"
    #elif __linux__
    "Patch_dosDetour0: \n"
    // Push the first parameter to the local stack frame.
    // We retrieve it later when returning.
    "movl 8(%ebp), %ecx \n\t"
    "movl %ecx, -4(%ebp) \n\t"
    "pop 8(%ebp) \n\t"
    "call Patch_dosDetour \n"
    #endif // _WIN32

    "cmp $0, %eax \n\t"
    "je notAllowed \n\t"
    #ifdef __linux__
    "movl -4(%ebp), %ecx \n\t"
    "movl %ecx, 8(%ebp) \n\t"
    "pop %ecx \n\t"
    "pop -4(%ebp) \n\t"
    #endif // __linux__
    "push " ADDRESS " \n\t"
    "ret \n"

    "notAllowed: \n\t"
    #ifdef __linux__
    "movl -4(%ebp), %ecx \n\t"
    "movl %ecx, 8(%ebp) \n\t"
    "pop %ecx \n\t"
    "pop -4(%ebp) \n\t"
    #endif // __linux__
    "push " ADDRESS2 " \n\t"
    "ret \n"
    );

#elif _MSC_VER
// Determine proper address.
#ifdef _GOLD
#define ADDRESS  0x004793a5
#define ADDRESS2 0x00479528
#else
#define ADDRESS  0x00476879
#define ADDRESS2 0x004769f5
#endif // _GOLD

__declspec(naked) void Patch_dosDetour0()
{
    __asm{
        call    Patch_dosDetour;
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
Patch_dosProtection

Writes the jump to the detour.
==================
*/

void Patch_dosProtection()
{
    #ifdef _WIN32
    #ifdef _GOLD
    Patch_detourAddress("DoS protection", (long)&Patch_dosDetour0, 0x004793a5, qtrue);
    #else
    Patch_detourAddress("DoS protection", (long)&Patch_dosDetour0, 0x00476879, qtrue);
    #endif // _GOLD
    #endif // _WIN32

    #ifdef __linux__
    #ifdef _GOLD
    Patch_detourAddress("DoS protection", (long)&Patch_dosDetour0, 0x08058b1f, qfalse);
    #else
    Patch_detourAddress("DoS protection", (long)&Patch_dosDetour0, 0x0805505c, qfalse);
    #endif // _GOLD
    #endif // __linux__
}
