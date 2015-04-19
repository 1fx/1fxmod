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

char *AlignDWORD(long input)
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
Patch_linuxPatchAddress

Logic to patch an address in another thread using ptrace (Linux).
==================
*/

#ifdef __linux__
void *Patch_linuxPatchAddress(void *arguments)
{
	char *address;
	union u{
		long val;
		char chars[4];
	}data;
	struct patchArgs *args = arguments;

	#if (defined(__GNUC__) && __GNUC__ < 3)
	usleep(10000); // Boe!Man 3/8/15: Sleep for 10msec, this fixes a very strange problem of LinuxThreads sending a SIGSTOP while attaching.
	#else
	pid_t pID = fork();

	if(pID < 0){
		perror("fail!\nForking process failed");
	}else if(pID != 0){
		// Wait for the child.
		wait(NULL);

		return NULL;
	}
	#endif // GNUC < 3

	if (ptrace(PTRACE_ATTACH, args->pid, NULL, NULL) == -1){
		perror("fail!\nAttach failed");
		return NULL;
	}

	if (waitpid(args->pid, NULL, WUNTRACED) != args->pid){
		perror("fail!\nCouldn't wait for PID");
		return NULL;
	}

	address = args->buf;
	memcpy(data.chars, address, 4);
	if(ptrace(PTRACE_POKEDATA, args->pid, args->address, data.val) == -1){
		perror("fail!\nWriting modified data failed");
		return NULL;
	}

	if (ptrace(PTRACE_DETACH, args->pid, NULL, NULL) == -1){
		perror("fail!\nDetach failed");
		return NULL;
	}

	printf("done!\n");
	#if (defined (__GNUC__) && __GNUC__ > 2)
	_Exit(EXIT_SUCCESS);
	#endif // GNUC > 2
}
#endif // __linux__

/*
==================
Patch_detourAddress

Main logic of detouring to an address.
==================
*/

void Patch_detourAddress(char *genericName, long func, long offset)
{
	char patch[9];
	#ifdef _WIN32
	unsigned char buf[5];
	#elif __linux__
	pthread_t thread;
	struct patchArgs args;

	unsigned char buf[4];
	#endif // _WIN32
	int i;
	int currentByte = 0;
	long addr = offset;

	Com_Printf("Applying %s... ", genericName);
	#ifdef _WIN32
	sprintf(patch, va("E9%s", AlignDWORD(func - addr)));
	#elif __linux__
	strncpy(patch, AlignDWORD(func - addr), sizeof(patch));
	#endif // _WIN32

	// Convert it to bytes.
	for(i = 0; i < strlen(patch); i += 2){
		char tempStr[3];
		tempStr[0] = patch[i];
		tempStr[1] = patch[i + 1];
		tempStr[2] = '\0';
		buf[currentByte] = (unsigned char)strtol(tempStr, NULL, 16);
		currentByte++;
	}

#ifdef _WIN32
	if(WriteProcessMemory(GetCurrentProcess(), (void *)(offset - 5), buf, 5, NULL) == 0){ // Write the jump.
		Com_Printf("fail!\n");
	}else{
		Com_Printf("done!\n");
	}
#elif __linux__
	#if (defined(__GNUC__) && __GNUC__ > 2)
	// We don't need to get the thread return status on modern Linux systems, so ignore the signal so they don't turn defunct.
	signal(SIGCHLD, SIG_IGN);
	#endif // GNUC > 2

	// Fill args with essential stuff, like PID and addresses.
	args.pid = getpid();
	args.buf = (char *)&buf;
	args.address = offset - 4;

	// Create the thread that patches the address with the modified data.
	if (pthread_create(&thread, NULL, &Patch_linuxPatchAddress, (void *)&args) != 0){
		perror("fail!\nCreate thread failed");
		return;
	}

	// Wait for the thread to finish.
	pthread_join(thread, NULL);
#endif // _WIN32
}

/*
==================
Patch_Main

Main function called from SOF2, in order to apply all memory patches.
==================
*/

void Patch_Main()
{
	int i = trap_Milliseconds();

	Com_Printf("------------------------------------------\n");
	Com_Printf("Applying memory runtime modifications...\n");

	if(g_dosPatch.integer){
		Patch_dosProtection();
	}

	#ifndef _GOLD
	// Only patch the auto download exploit on v1.00, not on v1.03.
	Patch_autoDownloadExploit();
	#endif // not _GOLD
	Patch_rconLog();

	Com_Printf("Patching took %d milliseconds.\n", trap_Milliseconds() - i);
}
