/*
===========================================================================
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
Copyright (C) 2010 - 2020, Ane-Jouke Schat (Boe!Man)
Copyright (C) 2010 - 2020, Jordi Epema (Henkie)

This file is part of the 1fx. Mod source code.

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/

#ifdef _MSC_VER
#pragma once
#endif // _MSC_VER
#if !defined(G2_H_INC)
#define G2_H_INC


#define BONE_ANGLES_PREMULT			0x0001
#define BONE_ANGLES_POSTMULT		0x0002
#define BONE_ANGLES_REPLACE			0x0004
#define	BONE_ANGLES_REPLACE_TO_ANIM	0x0400
#define	BONE_ANGLES_RAGDOLL			0x0800

#define BONE_ANGLES_TOTAL			(BONE_ANGLES_RAGDOLL | BONE_ANGLES_PREMULT | BONE_ANGLES_POSTMULT | BONE_ANGLES_REPLACE | BONE_ANGLES_REPLACE_TO_ANIM )
#define BONE_ANIM_OVERRIDE			0x0008
#define BONE_ANIM_OVERRIDE_LOOP		0x0010
#define BONE_ANIM_OVERRIDE_DEFAULT	( 0x0020 + BONE_ANIM_OVERRIDE )
#define BONE_ANIM_OVERRIDE_FREEZE	( 0x0040 + BONE_ANIM_OVERRIDE )
#define BONE_ANIM_BLEND				0x0080
#define BONE_ANIM_BLEND_FROM_PARENT	0x0100
#define BONE_ANIM_BLEND_TO_PARENT	0x0200
#define BONE_ANIM_TOTAL				( BONE_ANIM_OVERRIDE | BONE_ANIM_OVERRIDE_LOOP | BONE_ANIM_OVERRIDE_DEFAULT | BONE_ANIM_OVERRIDE_FREEZE | BONE_ANIM_BLEND	| BONE_ANIM_BLEND_TO_PARENT | BONE_ANIM_BLEND_FROM_PARENT )


// defines to setup the
#define		ENTITY_WIDTH 12
#define		MODEL_WIDTH	10
#define		BOLT_WIDTH	10
 
#define		MODEL_AND	((1<<MODEL_WIDTH)-1)
#define		BOLT_AND	((1<<BOLT_WIDTH)-1)
#define		ENTITY_AND	((1<<ENTITY_WIDTH)-1)

#define		BOLT_SHIFT	0
#define		MODEL_SHIFT	(BOLT_SHIFT + BOLT_WIDTH)
#define		ENTITY_SHIFT (MODEL_SHIFT + MODEL_WIDTH)


#endif // G2_H_INC
