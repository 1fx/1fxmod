// Copyright (C) 2014 - Boe!Man, Henkie.
//
// patch_main.c - Functions used by more than one memory patch.

//==================================================================

#include "patch_local.h"

/*
==================
AlignDWORD

Aligns DWORD value from ulong to C string.
==================
*/

char *AlignDWORD(unsigned long input)
{
	static char output[9];
	char tempStr[9];
	int n;

	n = sprintf(tempStr, "%p", input);
	tempStr[n] = '\0';
	output[0] = tempStr[n - 2];
	output[1] = tempStr[n - 1];
	output[2] = tempStr[n - 4];
	output[3] = tempStr[n - 3];
	output[4] = tempStr[n - 6];
	output[5] = tempStr[n - 5];
	output[6] = tempStr[n - 8];
	output[7] = tempStr[n - 7];
	output[8] = '\0';

	return output;
}

/*
==================
Patch_detourAddress

Main logic of detouring to an address.
==================
*/

#ifdef _WIN32
void Patch_detourAddress(char *genericName, unsigned long func, unsigned long offset)
{
	char patch[24];
	unsigned char buf[12] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	int i;
	int currentByte = 0;
	unsigned long addr = offset;

	Com_Printf("Applying %s... ", genericName);
	sprintf(patch, va("E9%s", AlignDWORD((int)func - addr)));

	// Convert it to bytes.
	for(i = 0; i < strlen(patch); i += 2){
		char tempStr[3];
		tempStr[0] = patch[i];
		tempStr[1] = patch[i + 1];
		tempStr[2] = '\0';
		buf[currentByte] = (unsigned char)strtol(tempStr, NULL, 16);
		currentByte++;
	}

	if (currentByte != 5){ // Must be 5 bytes long.
		Com_Printf("fail!\n");
	}else{
		WriteProcessMemory(GetCurrentProcess(), (void *)(offset - 5), buf, currentByte, NULL); // Write the jump.
		Com_Printf("done!\n");
	}
}
#endif // _WIN32

/*
==================
Patch_Main

Main function called from SOF2, in order to apply all memory patches.
==================
*/

void Patch_Main()
{
	#ifdef _WIN32
	if(g_dosPatch.integer){
		Patch_dosProtection();
	}

	Patch_autoDownloadExploit();
	#endif
}
