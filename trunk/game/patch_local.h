// Copyright (C) 2014 - Boe!Man, Henkie.
//
// patch_local.h - Memory patch definitions are stored here.

//==================================================================

#ifndef _PATCH_LOCAL
#define _PATCH_LOCAL

#include "g_local.h"
#ifdef _WIN32
#include <windows.h>
#endif // _WIN32

// Function definitions.
char	*AlignDWORD					(unsigned long input);
void	Patch_detourAddress			(char *genericName, unsigned long func, unsigned long offset);
void	Patch_autoDownloadExploit	(void);
void	Patch_dosProtection			(void);

#endif // _PATCH_LOCAL
