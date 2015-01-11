// Copyright (C) 2014 - Boe!Man, Henkie.
//
// patch_local.h - Memory patch definitions are stored here.

//==================================================================

#ifndef _PATCH_LOCAL
#define _PATCH_LOCAL

#include "g_local.h"
#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <linux/user.h>

// Boe!Man 1/11/15: Required information for memory patching in another thread.
struct patchArgs {
	pid_t	pid;				// PID of main thread/process.
	char	*buf;				// Bytedata of the modified call (4 bytes).
	long	address;			// Address to patch.
};

typedef enum {
	NA_BOT,
	NA_BAD,
	NA_LOOPBACK,
	NA_BROADCAST,
	NA_IP,
	NA_IPX,
	NA_BROADCAST_IPX
} netadrtype_t;

typedef struct {
	netadrtype_t	type;

	unsigned char	ip[4];
	unsigned char	ipx[10];

	unsigned short	port;
} netadr_t;
#endif // _WIN32

// Function definitions.
char	*AlignDWORD					(long input);
void	Patch_detourAddress			(char *genericName, long func, long offset);
void	Patch_autoDownloadExploit	(void);
void	Patch_dosProtection			(void);

#endif // _PATCH_LOCAL
