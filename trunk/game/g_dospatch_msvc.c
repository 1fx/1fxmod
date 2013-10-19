// Copyright (C) 2013 - Boe!Man, Henkie.
//
// g_dospatch_msvc.c - MSVC version of Q3 getstatus DoS patch (WIN32 only).

//==================================================================

#if defined (WIN32) && !(__GNUC__)
#include "g_local.h" // we might need access to the print functions of sof
#include <windows.h>

static int requests = 0; // requests reset every
static unsigned long timer;
static bool validRequest = true;
static int requestsPerSecond = 50;
// engine declarations
typedef enum {
    NA_BOT,
    NA_BAD,                    // an address lookup failed
    NA_LOOPBACK,
    NA_BROADCAST,
    NA_IP,
    NA_IPX,
    NA_BROADCAST_IPX
} netadrtype_t;

typedef struct {
    netadrtype_t    type;

    unsigned char    ip[4];
    unsigned char    ipx[10];

    unsigned short    port;
} netadr_t;

typedef struct {
    qboolean    allowoverflow;    // if false, do a Com_Error
    qboolean    overflowed;        // set to true if the buffer size failed (with allowoverflow set)
    qboolean    oob;            // set to true if the buffer size failed (with allowoverflow set)
    unsigned char    *data;
    int        maxsize;
    int        cursize;
    int        readcount;
    int        bit;                // for bitwise reads and writes
} msg_t;

char *AlignDWORD(unsigned long input){
    char tempStr[9], output[9];
    int n;
    n = sprintf(tempStr, "%p", input);
    tempStr[n] = '\0';
    output[0] = tempStr[n-2];
    output[1] = tempStr[n-1];
    output[2] = tempStr[n-4];
    output[3] = tempStr[n-3];
    output[4] = tempStr[n-6];
    output[5] = tempStr[n-5];
    output[6] = tempStr[n-8];
    output[7] = tempStr[n-7];
    output[8] = '\0';
    return output;
}

__declspec(naked) void SV_ConnectionlessPacket_detour(netadr_t from, msg_t msg)
{
    if(requests >= requestsPerSecond)
        validRequest = false;
    else
        validRequest = true;
    if(GetTickCount() >= timer){
        timer = GetTickCount()+1000;
        if(requests >= requestsPerSecond){
            Com_Printf("Requests per second exceeded the limit of %i: %i requests/second\n", requestsPerSecond, requests);
            requests = 0;
        }
        requests = 0;
    }
    if(validRequest){
            requests++;
            // restore instructions which the jump replaced
        __asm{
             mov eax, 0x4760C0
             call eax // SVC_Status(from)
             push 0x4768A0
             retn

        }
    }else if(!validRequest){
        __asm{
            push 0x4768A0
            retn
        }
    }
}

void Patch_SV_ConnectionlessPacket(){
    unsigned char buf[8];
    int n;
    char addyAddr[10];
    char patch[24];
    char testAddr[10];
    BYTE buf1[12] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    int i;
    int currentByte = 0;
    char addr[10];
    DWORD TEST = 0x4768A0;//0x476805;
    //0047689B - E8 20F8FFFF                - call 004760C0
    strcpy(addr, AlignDWORD((int)&SV_ConnectionlessPacket_detour-TEST));
    n = sprintf(addyAddr, "%s", addr);
    addyAddr[n] = '\0';
    n = sprintf(patch, "E9%s", addyAddr);
    patch[n] = '\0';
    Com_Printf("Patch: %s\n", patch);
    n = sprintf(testAddr, "%p", &SV_ConnectionlessPacket_detour);
    testAddr[n] = '\0';
    //MessageBox(0, addyAddr, testAddr, MB_OK);

    // convert it to bytes
    for(i=0;i<strlen(patch);i+=2){
        char tempStr[3];
        tempStr[0] = patch[i];
        tempStr[1] = patch[i+1];
        tempStr[2] = '\0';
        buf1[currentByte] = (BYTE)strtol(tempStr, NULL, 16);
        currentByte++;
    }
    WriteProcessMemory(GetCurrentProcess(), (LPVOID)0x47689B, buf1, currentByte, NULL); // write the jump
    Com_Printf("Patched sv_connectionlesspacket\n");
}
#endif
