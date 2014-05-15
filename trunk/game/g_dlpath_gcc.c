// Copyright (C) 2014 - Boe!Man, Henkie.
//
// g_dlpatch_gcc.c - MinGW (GCC) version of Q3 "dirtrav" auto download exploit patch (WIN32 only).

//==================================================================

#if defined (__GNUC__) && (WIN32)
#include "g_local.h" // we might need access to the print functions of sof
#include <windows.h>

void autoDownloadDetour0();
void autoDownloadDetour(char *message, int *clientNum, char *mapString)
{
    if(!strstr(mapString, ".pk3")){
        G_LogPrintf("Auto D/L protection: ^1HACK ATTEMPT: ^7client %d tries to download \"%s\"\n", clientNum, mapString);
        *mapString = '\0';
    }
    #ifdef _DEBUG
    else{
        Com_Printf("Auto D/L: Allowed client %d to download \"%s\"\n", clientNum, mapString);
    }
    #endif
}

__asm__(".globl autoDownloadDetour0 \n\t"
        "_autoDownloadDetour0: \n"
        "call _autoDownloadDetour \n"

        "push $0x0046EF20 \n\t"
        "ret\n"
        );

void Patch_AutoDownloadExploit(){
    char patch[24];
    unsigned char buf[12] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    int i;
    int currentByte = 0;
    unsigned long addr = 0x46EF20;

    Com_Printf("Applying Auto D/L exploit protection... ");

    sprintf(patch, va("E9%s", AlignDWORD((int)&autoDownloadDetour0-addr)));

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
        WriteProcessMemory(GetCurrentProcess(), (void *)0x46EF1B, buf, currentByte, NULL); // write the jump
        Com_Printf("done!\n");
    }
}
#endif
