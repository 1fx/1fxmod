// Copyright (C) 2013 - Boe!Man, Henkie.
//
// g_dospatch_gcc.c - MinGW (GCC) version of Q3 getstatus DoS patch (WIN32 only).

//==================================================================

#if defined (__GNUC__) && (WIN32)
#include "g_local.h" // we might need access to the print functions of sof
#include <windows.h>

void SV_ConnectionlessPacket_detour0();

static int requests = 0; // requests, reset every second
static unsigned long timer;
static qboolean validRequest = qtrue;
static int requestsPerSecond = 20; // Boe!Man 10/17/13: 20 per second should be more then sufficient.

char *AlignDWORD(unsigned long input){
	static char output[9];
    char tempStr[9];
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

int SV_ConnectionlessPacket_detour()
{
    if(requests >= requestsPerSecond){
        validRequest = qfalse;
    }else{
        validRequest = qtrue;
    }
    
    if(level.time >= timer){
        timer = level.time + 1000;
        if(requests >= requestsPerSecond){
            #ifdef _DEBUG
            Com_Printf("Info: Too many requests per second: %i (%i allowed)\n", requests, requestsPerSecond);
            #endif
        }
        requests = 0;
    }
    
    requests++;
    
    return (int)validRequest;
}

__asm__(".globl SV_ConnectionlessPacket_detour0 \n\t"
        "_SV_ConnectionlessPacket_detour0: \n"
        "call _SV_ConnectionlessPacket_detour \n"
        
        "cmp $0, %eax \n\t"
        "je exit \n\t"
        // Call SVC_Status if we're below our allowed limit.
        "movl $0x004760C0, %eax \n\t"
        "call *%eax \n"
        
        "exit: \n\t"
        "push $0x004768A0 \n\t"
        "ret\n"
        );

void Patch_SV_ConnectionlessPacket(){
    char patch[24];
    unsigned char buf[12] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    int i;
    int currentByte = 0;
    unsigned long addr = 0x4768A0;
    
    Com_Printf("Applying DoS protection... ");
    
    sprintf(patch, va("E9%s", AlignDWORD((int)&SV_ConnectionlessPacket_detour0-addr)));

    // convert it to bytes
    for(i=0;i<strlen(patch);i+=2){
        char tempStr[3];
        tempStr[0] = patch[i];
        tempStr[1] = patch[i+1];
        tempStr[2] = '\0';
        buf[currentByte] = (unsigned char)strtol(tempStr, NULL, 16);
        currentByte++;
    }
    
    if(currentByte != 5){ // Must be 5 bytes long.
        Com_Printf("fail!\n");
    }else{
        WriteProcessMemory(GetCurrentProcess(), (void *)0x47689B, buf, currentByte, NULL); // write the jump
        Com_Printf("done!\n");
    }
}
#endif
