// Copyright (C) 2013 - Boe!Man.

// Some portions:
// Copyright (C) 2001-2002 Raven Software
// Copyright (C) 2002-2007 Ted Vessenes

// ai_main.h - Main AI definitions.

#include "g_local.h"
#include "botlib.h"

//==============================================
// Setup.
//==============================================
// Predeclare and typedef the bot state for structures that must have
// a pointer to the bot state, such as function pointers elements.
typedef struct bot_state_s bot_state_t;

//==============================================
// Engine defined values.
//==============================================
#define MAX_CHARACTERISTIC_PATH		144
