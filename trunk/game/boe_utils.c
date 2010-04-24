// Copyright (C) 2010 - Boe!Man.
//
// boe_utils.c - All the miscellaneous functions go here, e.g. tokens, motd, player menu, etc.

#include "g_local.h"
#include "boe_local.h"

/*
===================
Boe_Motd by boe
3/30/10 - 10:58 AM
===================
*/

void Boe_Motd (gentity_t *ent)
{
    char    gmotd[1024] = "\0";
	char	motd[1024] = "\0";
	char	*s = motd;
	char	*gs = gmotd;
	char	name[36];
	char	*header1 = va("@%s ^7%s ^7- %s\n", INF_VERSION_STRING_COLORED, INF_VERSION_STRING, INF_VERSION_DATE );
	char	*header2 = va("Developed by ^GBoe!Man ^7& ^6Henkie\nv1servers.com ^3| ^71fx.ipbfree.com\n\n");

	strcpy(name, ent->client->pers.netname);

	Com_sprintf(gmotd, 1024, "%s%s%s\n%s\n%s\n%s\n%s\n%s\n",
		header1,
		header2,
		server_motd1.string,
		server_motd2.string,
		server_motd3.string,
		server_motd4.string,
		server_motd5.string,
		server_motd6.string);
	
	gmotd[strlen(gmotd)+1] = '\0';

	while(*gs)
	{
		if(*gs == '#')
		{
			if(*++gs == 'u')
			{
				strcat(motd, name);
				strcat(motd, "^7");
				s += strlen(name) +2;
				gs++;
			}
			else
			{
				gs--;
			}
		}

		*s++ = *gs++;
	}

	*s = '\0';
	trap_SendServerCommand(ent-g_entities, va("cp \"%s\"", motd));
}

/*
===================
Boe_adminLog by boe
3/30/10 - 12:42 PM
===================
*/

void QDECL Boe_adminLog( const char *text, ... )
{
	char		string[1024] = "";
	va_list		argptr;
	qtime_t		q;
	fileHandle_t	f;

	trap_RealTime (&q);

	Com_sprintf( string, sizeof(string), "%02i/%02i/%i %02i:%02i - ", 1+q.tm_mon,q.tm_mday, q.tm_year+1900,q.tm_hour,q.tm_min);
	va_start( argptr, text );
	vsprintf( string + 19, text, argptr );
	va_end( argptr );
		
	trap_FS_FOpenFile("logs/adminlog.txt", &f, FS_APPEND_TEXT);
	
	if ( !f )
		return;

	trap_FS_Write( string, strlen( string ), f );
	trap_FS_Write( "\n", 1, f);
	trap_FS_FCloseFile(f);
}

// ==================================================================================================================================================================
// 'Old' entries from 1fx. Mod.
//
// Boe!Man 12/20/09
void ExitLevel( void );
gspawn_t* G_SelectRandomSafeSpawnPoint ( team_t team, float safeDistance );

chatSounds_t chatSounds[MAX_BOE_CHATS];

char* defaultChatSounds[][2] =
{
	{"Call for reinforcements",			"sound/enemy/english/male/call_for_reinforcements.mp3"	},
	{"Advance",							"sound/enemy/english/male/advance.mp3"					},
	{"Awaiting orders",					"sound/enemy/english/male/awaiting_orders.mp3"			},
	{"Check the perimiter",				"sound/enemy/english/male/check_peri.mp3"				},
	{"Go check that out",				"sound/enemy/english/male/check_that_out.mp3"			},
	{"He's dangerous",					"sound/enemy/english/male/dangerous.mp3"				},
	{"Did you hear that",				"sound/enemy/english/male/did_you_hear.mp3"				},
	{"He's disappeared",				"sound/enemy/english/male/disappeared.mp3"				},
	{"Drop your weapon",				"sound/enemy/english/male/drop_your_weapon.mp3"			},
	{"Eliminate target",				"sound/enemy/english/male/eliminate.mp3"				},
	{"GET HIM",							"sound/enemy/english/male/get_him.mp3"					},
	{"I got that bastard",				"sound/enemy/english/male/got_bastard.mp3"				},
	{"Hold here",						"sound/enemy/english/male/hold_here.mp3"				},
	{"Hurry",							"sound/enemy/english/male/hurry.mp3"					},
	{"I'm hit",							"sound/npc/col8/blakely/imhit.mp3"						},
	{"Investigate that area",			"sound/enemy/english/male/investigate_area.mp3"			},
	{"Keep looking",					"sound/enemy/english/male/keep_looking.mp3"				},
	{"KILL HIM",						"sound/enemy/english/male/kill_him"						},
	{"I killed him",					"sound/enemy/english/male/killed_him.mp3"				},
	{"I lost him",						"sound/enemy/english/male/lost_him.mp3"					},
	{"TAKING SERIOUS CASUALTIES",		"sound/enemy/english/male/man_down03.mp3"				},
	{"I need help here",				"sound/enemy/english/male/need_help.mp3"				},
	{"Medic Medic",						"sound/enemy/english/male/medic_medic.mp3"				},
	{"Open Fire",						"sound/enemy/english/male/open_fire.mp3"				},
	{"I'm out of Ammo",					"sound/enemy/english/fmale/out_ammo.mp3"				},
	{"He's over here",					"sound/enemy/english/male/over_here.mp3"				},
	{"Over there",						"sound/enemy/english/male/over_there.mp3"				},
	{"Plug him",						"sound/enemy/english/male/plug_him.mp3"					},
	{"Position reached",				"sound/enemy/english/male/pos_reached.mp3"				},
	{"Secure the area",					"sound/enemy/english/male/secure_the_area.mp3"			},
	{"We're getting slaughtered",		"sound/enemy/english/male/slaughtered.mp3"				},
	{"Somethings not right here",		"sound/enemy/english/male/something_not_right.mp3"		},
	{"Spread out",						"sound/enemy/english/male/spread_out.mp3"				},
	{"Surround him",					"sound/enemy/english/male/surround_him.mp3"				},
	{"Take cover",						"sound/enemy/english/male/take_cover2.mp3"				},
	{"Take him out",					"sound/enemy/english/male/take_him_out.mp3"				},
	{"Take Position",					"sound/enemy/english/male/take_position.mp3"			},
	{"Take them out",					"sound/enemy/english/male/take_them_out.mp3"			},
	{"Target has been eliminated",		"sound/enemy/english/male/target_eliminate.mp3"			},
	{"There he is",						"sound/enemy/english/male/there_he_is.mp3"				},
	{"Taking alot of fire",				"sound/enemy/english/male/underfire03.mp3"				},
	{"WATCH OUT",						"sound/enemy/english/male/watch_out.mp3"				},
	{"What are you doing here",			"sound/enemy/english/male/what_are_you_doing.mp3"		},
	{"What the...",						"sound/enemy/english/male/what_the.mp3"					},
	{"What was that",					"sound/enemy/english/male/what_was_that.mp3"			},
	{"Whats happening here",			"sound/enemy/english/male/whats_happening_here.mp3"		},
	{"Who are you",						"sound/enemy/english/male/who_are_you.mp3"				},
	{"You're not supposed to be here",	"sound/enemy/english/male/youre_not_supposed.mp3"		},
	{"Hey did you see that guy, He wet his pants ha haha ha ha..", "sound/npc/air1/guard01/01wetpants.mp3" },
	{"Hey honey..",						"sound/npc/air1/guard05/01honey.mp3"					},
	{"Huh I think i can help you",		"sound/npc/gladstone/arm1/03canhelp.mp3"				},
	{"If you look at me 1 more time, I swear I'll BLOW YOUR HEAD OFF", "sound/npc/air1/terrorists/blowheadoff.mp3" },
	{"How bout we see if you can dodge a BULLET", "sound/npc/air1/terrorists/dodgebullet.mp3"	},
	{"That was a close one",			"sound/npc/fritsch/closeone.mp3"						},
	{"What are you lookin at",			"sound/npc/air1/terrorists/lookingat.mp3"				},
	{"You scared?, You should be",		"sound/npc/air1/terrorists/scared02.mp3"				},
	{"Leave me alone",					"sound/npc/air1/woman01/01leaveme.mp3"					},
	{"Please just leave me be..",		"sound/npc/air1/woman01/02please.mp3"					},
	{"What? Please explain.. over",		"sound/npc/air4/hansen/04what.mp3"						},
	{"Attention",						"sound/npc/cem1/comm/01attention.mp3"					},
	{"Someone call 911",				"sound/npc/civ/english/male/call_911.mp3"				},
	{"Call the police",					"sound/npc/civ/english/male/callpolice.mp3"				},
	{"Comin thru..",					"sound/npc/civ/english/male/coming_thru.mp3"			},
	{"NICE SHOT",						"sound/npc/col8/washington/niceshot.mp3"				},
	{"Excuse me",						"sound/npc/civ/english/male/excuse_me.mp3"				},
	{"Don't hurt me",					"sound/npc/civ/english/male/dont_hurt.mp3"				},
	{"I don't understand",				"sound/npc/civ/english/male/dont_understand.mp3"		},
	{"I don't have any money",			"sound/npc/civ/english/male/no_money.mp3"				},
	{"Those men have guns",				"sound/npc/civ/english/male/thosemen.mp3"				},
	{"Take my wallet, Just don't hurt me", "sound/npc/civ/english/male/wallet.mp3"				},
	{"I'm unarmed",						"sound/npc/civ/english/male/unarmed.mp3"				},
	{"Watchout",						"sound/npc/civ/english/male/watchout.mp3"				},
	{"Pairup",							"sound/npc/col2/butch/pairup.mp3"						},
	{"Fanout",							"sound/npc/col2/butch/fanout.mp3"						},
	{"We got company, Take cover",		"sound/npc/col3/peterson/12company.mp3"					},
	{"Enemy has been neutralized, base camp is secure. over", "sound/npc/col3/radio/02enemy.mp3" },
	{"BANG! Your dead",					"sound/npc/col8/blakely/bang.mp3"						},
	{"DAMN! that was close",			"sound/npc/col8/blakely/close.mp3"						},
	{"GET DOWN",						"sound/npc/col8/blakely/getdown.mp3"					},
	{"Come get some",					"sound/npc/col8/blakely/getsome.mp3"					},
	{"Incoming",						"sound/npc/col8/blakely/incoming.mp3"					},
	{"Go cry to mama",					"sound/npc/col8/blakely/mama.mp3"						},
	{"We showed them",					"sound/npc/col8/blakely/showed.mp3"						},
	{"Take that",						"sound/npc/col8/blakely/takethat.mp3"					},
	{"That did it",						"sound/npc/col8/blakely/thatdidit.mp3"					},
	{"You want some of this",			"sound/npc/col8/blakely/wantsome.mp3"					},
	{"Yeah",							"sound/npc/col8/blakely/yeah.mp3"						},
	{"WOOOO",							"sound/npc/col8/blakely/woo.mp3"						},
	{"Don't worry girls it'll all be over soon", "sound/npc/col8/peterson/01girls.mp3"			},
	{"Cover my left",					"sound/npc/col8/peterson/coverleft.mp3"					},
	{"Cover my right",					"sound/npc/col8/peterson/coverright.mp3"				},
	{"Keep your eyes open boys",		"sound/npc/col8/peterson/eyesopen.mp3"					},
	{"Get moving",						"sound/npc/col8/peterson/getmoving02.mp3"				},
	{"Shudd-up man",					"sound/npc/col8/washington/02shuddup.mp3"				},
	{"Eat lead sukka",					"sound/npc/col8/washington/eatlead.mp3"					},
	{"Kickin ass, and takin names",		"sound/npc/col8/washington/takingnames.mp3"				},
	{"How'd you like that",				"sound/npc/col8/washington/youlike.mp3"					},
	{"WoOoOo",							"sound/npc/col8/washington/woo.mp3"						},
	{"O.K, lets do it",					"sound/npc/col9/pilot/03letsdoit.mp3"					},
	{"We can't take much more of this",	"sound/npc/col9/pilot/canttake.mp3"						},
	{NULL, NULL}
};

/*
==============
Boe_Tokens
==============
*/
void Boe_Tokens(gentity_t *ent, char *chatText, int mode)
{
	int  i = 0, n = 0;
	qboolean playedSound = qfalse, ghost = qfalse;
	char location[64] = "\0";
	char newText[MAX_SAY_TEXT] = "\0";
	char *newTextp;
	char *chatTextS;
	gentity_t *tent;
	qboolean text = qtrue;
	gitem_t* item;		
	/*
	if(ent->client->sess.mute)
	{
		return;
	}
	*/
	newTextp = newText;
	chatTextS = chatText;
	while(*chatText && newTextp < newText + MAX_SAY_TEXT )
	{
		if (*chatText == '#')
		{
			chatText++;

			switch(*chatText)
			{
				case 'h':
					Q_strcat(newText, MAX_SAY_TEXT, va("%s%d^2",Boe_StatColor(ent->health), ent->health));
					chatText++;
					continue;
				case 'H':
					Q_strcat(newText, MAX_SAY_TEXT, va("%d", ent->health));
					chatText++;
					continue;
				case 'a':
					Q_strcat(newText, MAX_SAY_TEXT, va("%s%d^2",Boe_StatColor(ent->client->ps.stats[STAT_ARMOR]), ent->client->ps.stats[STAT_ARMOR]));
					chatText++;
					continue;
				case 'A':
					Q_strcat(newText, MAX_SAY_TEXT, va("%d", ent->client->ps.stats[STAT_ARMOR]));
					chatText++;
					continue;
				case 'd':
				case 'D':
					Q_strcat(newText, MAX_SAY_TEXT, va("%s", g_entities[ent->client->pers.statinfo.lasthurtby].client->pers.netname));
					chatText++;
					continue;
				case 't':
				case 'T':
					Q_strcat(newText, MAX_SAY_TEXT, va("%s", g_entities[ent->client->pers.statinfo.lastclient_hurt].client->pers.netname));
					chatText++;
					continue;
				case 'n':
				case 'N':
						Q_strcat(newText, MAX_SAY_TEXT, va("%s", g_motd.string));
					chatText++;
					continue;
				case 'f':
				case 'F':
					i = Boe_FindClosestTeammate(ent, qfalse);
					if(i > -1)
					{
						Q_strcat(newText, MAX_SAY_TEXT, va("%s", g_entities[i].client->pers.netname));
					}
					chatText++;
					continue;
				case 'l':
				case 'L':
					if(Team_GetLocationMsg(ent, location, sizeof(location)))
					{
						Q_strcat(newText, MAX_SAY_TEXT, va("%s", location));
					}
					chatText++;
					continue;
				case 'E':
				case 'e':
					i = Boe_FindClosestEnemy(ent, qfalse);
					if(i > -1)
					{
						Q_strcat(newText, MAX_SAY_TEXT, va("%s", g_entities[i].client->pers.netname));
					}
					chatText++;
					continue;
				case 'Z':
				case 'z':
					Q_strcat(newText, MAX_SAY_TEXT, g_entities[level.lastConnectedClient].client->pers.netname );
					chatText++;
					continue;
				default:
					if(*chatText >= '0' && *chatText <= '9')
					{
						n = atoi(chatText);
						if(n > -1 && n < MAX_CLIENTS)
						{
							tent = g_entities + n;
							if(tent && tent->client)
							{
								Q_strcat(newText, MAX_SAY_TEXT, tent->client->pers.netname);
							}
							else
							{
								Q_strcat(newText, MAX_SAY_TEXT, "none");
							}
							if ( n > 9)
							{
								chatText++;
							}
							chatText++;
							continue;
						}
					}
					chatText--;
			}
		}
		if((*chatText == '@'/* || *chatText == '!'*/) && !playedSound) {
			if ( g_voiceFloodCount.integer ) {
				if ( ent->client->voiceFloodPenalty ) {
					if ( ent->client->voiceFloodPenalty > level.time ) {
						playedSound = qtrue;
						continue;
					}
					ent->client->voiceFloodPenalty = 0;
				}
				if ( ent->client->voiceFloodCount >= g_voiceFloodCount.integer ) {
					ent->client->voiceFloodCount = 0;
					ent->client->voiceFloodTimer = 0;
					ent->client->voiceFloodPenalty = level.time + g_voiceFloodPenalty.integer * 1000;
					// Boe!Man 12/20/09 - Update 12/22/09 [Yellow color instead of Red].
					trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Voice chat flooded, you will be able use voice chats again in %d seconds.\n\"", g_voiceFloodPenalty.integer ) );
					playedSound = qtrue;
					continue;
				}
			}
			//if(*chatText == '!') {
			//	text = qfalse;
			//}
			chatText++;
			i = 0;
			n = atoi(chatText) - 1;
			if(n > -1 && n < MAX_BOE_CHATS && chatSounds[n].sound && *chatSounds[n].text) {
				i = chatSounds[n].sound;
				if(text)
					Q_strcat(newText, MAX_SAY_TEXT, chatSounds[n].text);
				chatText++;
			}
			else
				return;
			if(i) {
				if( level.gametypeData->respawnType == RT_NONE)	{
					if ( G_IsClientDead ( ent->client ) )
							ghost = qtrue;
				}
				if(mode == SAY_TEAM) {
					for (n = 0; n < level.maxclients; n++) {
						tent = &g_entities[n];

						if (!tent || !tent->inuse || !tent->client)
							continue;
						if ( !OnSameTeam(ent, tent) )
							continue;
						if (!G_IsClientDead ( tent->client ) && !G_IsClientSpectating( tent->client) && ghost)
							continue;
						Boe_ClientSound(tent, i);
					}
					ent->client->voiceFloodCount++;
					playedSound = qtrue;
				}
				else if(mode != SAY_TELL) {
						if(ghost) {
							for (n = 0; n < level.maxclients; n++) {
								tent = &g_entities[n];
								if (!tent || !tent->inuse || !tent->client)
									continue;
								if (!G_IsClientDead ( tent->client ) && !G_IsClientSpectating( tent->client))
									continue;
								Boe_ClientSound(tent, i);
							}
						}
						else
						Boe_GlobalSound(i);
						ent->client->voiceFloodCount++;
						playedSound = qtrue;
				}
			}
				else {
					return;
				} 
				while (*chatText >= '0' && *chatText <= '9')
					chatText++;
			}
		while(*newTextp)
		{newTextp++;}

		if(*chatText)
		{
			*newTextp++ = *chatText++;
			
		}
		*newTextp = '\0';
	}
		chatText = chatTextS;
		Q_strncpyz (chatText, newText, MAX_SAY_TEXT);
		return;
}

/*
===============
Boe_StatColor
===============
*/
char *Boe_StatColor(int stat)
{
	char *color;

	if(stat>=80)
	{
		color = "^g";
	}
	else if(stat>=70)
	{
		color = "^&";
	}
	else if(stat>=60)
	{
		color = "^3";
	}
	else if(stat>=50)
	{
		color = "^b";
	}
	else if(stat>=40)
	{
		color = "^d";
	}
	else if(stat>=30)
	{
		color = "^@";
	}
	else if(stat>=20)
	{
		color = "^6";
	}
	else
	{
		color = "^1";
	}

	return color;
}

/*
=====================
Boe_FindClosestTeammate
=====================
*/
int	Boe_FindClosestTeammate(gentity_t *ent, qboolean bot)
{
	int			i;
	int			client = -1;
	gentity_t*  other;
	float		dist, bestDist = 99999;
	vec3_t	    diff;

	for ( i = 0; i < level.numConnectedClients; i ++ )
	{
		other = &g_entities[ level.sortedClients[i] ];

		if ( other->client->pers.connected != CON_CONNECTED )
		{
			continue;
		}

		if ( other == ent)
		{
			continue;
		}

		if ( G_IsClientSpectating ( other->client ) || G_IsClientDead ( other->client ) )
		{
			continue;
		}

		if ( !OnSameTeam(ent, other) )
		{
			continue;
		}

		if(bot)
		{
			if (!other->r.svFlags & SVF_BOT)
			{
				continue;
			}
		}

		VectorSubtract ( other->r.currentOrigin, ent->r.currentOrigin, diff );
		dist = VectorLength(diff);

		if ( dist < bestDist)
		{
			client = other->s.number;
			bestDist = VectorLength(diff);
		}
	}

	return client;
}

/*
=====================
Boe_FindClosestEnemy
=====================
*/
int	Boe_FindClosestEnemy(gentity_t *ent, qboolean bot)
{
	int			i;
	int			client = -1;
	gentity_t*  other;
	float		dist, bestDist = 99999;
	vec3_t	    diff;

	for ( i = 0; i < level.numConnectedClients; i ++ )
	{
		other = &g_entities[ level.sortedClients[i] ];

		if ( other->client->pers.connected != CON_CONNECTED )
		{
			continue;
		}

		if ( other == ent)
		{
			continue;
		}

		if ( G_IsClientSpectating ( other->client ) || G_IsClientDead ( other->client ) )
		{
			continue;
		}

		if (OnSameTeam(ent, other) )
		{
			continue;
		}

		if(bot)
		{
			if (!other->r.svFlags & SVF_BOT)
			{
				continue;
			}
		}

		VectorSubtract ( other->r.currentOrigin, ent->r.currentOrigin, diff );
		dist = VectorLength(diff);

		if ( dist < bestDist)
		{
			client = other->s.number;
			bestDist = VectorLength(diff);
		}
	}

	return client;
}

/*
===============
Boe_ParseChatSounds
===============
*/

void Boe_ParseChatSounds (void)
{
	int			fileCount, filelen, i, numSounds, number;
	char		chatFiles[1024];
	char		*filePtr;
	char		*file;
	char		text[MAX_SAY_TEXT];
	char		sound[MAX_QPATH];
	char		numString[8];
	void		*GP2, *group;

	G_LogPrintf("Loading Default chat Sounds\n");
	for( numSounds = 0; defaultChatSounds[numSounds][0]; numSounds++)
	{
		Q_strncpyz(chatSounds[numSounds].text, defaultChatSounds[numSounds][0], MAX_SAY_TEXT);
		chatSounds[numSounds].sound = G_SoundIndex(va("%s", defaultChatSounds[numSounds][1]));
	}
	
	fileCount = trap_FS_GetFileList( "", ".vchat", chatFiles, 1024 );
	filePtr = chatFiles;

	G_LogPrintf("Number of Chat sound files to Parse: %d\n", fileCount);

	for( i = 0; i < fileCount && numSounds < MAX_BOE_CHATS; i++, filePtr += filelen+1 )
	{
		filelen = strlen(filePtr);
		file = va("%s", filePtr);

		GP2 = trap_GP_ParseFile(file, qtrue, qfalse);
		if (!GP2)
		{
			G_LogPrintf("Error in file: \"%s\" or file not found.\n", file);
			continue;
		}

		G_LogPrintf("Parsing chat file: %s\n", file);
		group = trap_GPG_GetSubGroups(GP2);

		while(group)
		{
			trap_GPG_FindPairValue(group, "number", "0", numString);
			trap_GPG_FindPairValue(group, "text", "", text);
			trap_GPG_FindPairValue(group, "sound", "", sound);

			number = atoi(numString) -1;

			if(number < 0 || number > MAX_BOE_CHATS - 1)
			{
				G_LogPrintf("Error in chat file: %s\nNumber %d out of range, must be 101 - 200.\n", file, number);
				group = trap_GPG_GetNext(group);
				continue;
			}
			if(*chatSounds[number].text || chatSounds[number].sound)
			{
				group = trap_GPG_GetNext(group);
				continue;
			}

			if(*text && *sound)
			{
				Q_strncpyz(chatSounds[number].text, text, MAX_SAY_TEXT - 1);
				chatSounds[number].sound = G_SoundIndex(va("%s", sound));
				numSounds++;
			}
			if(numSounds == MAX_BOE_CHATS)
			{
				break;
			}

			group = trap_GPG_GetNext(group);
		}

		trap_GP_Delete(&GP2);
	}

	G_LogPrintf("Loaded %d sounds.\n", numSounds);
}

/*
=============
Boe_ClientSound
=============
*/
void Boe_ClientSound (gentity_t *ent, int soundIndex)
{
	gentity_t *tent;

	tent = G_TempEntity( ent->r.currentOrigin, EV_GLOBAL_SOUND);
	tent->s.eventParm = soundIndex;
	tent->r.svFlags |= SVF_SINGLECLIENT;
	tent->r.singleClient = ent->s.number;
}

/*
=============
Boe_GlobalSound
=============
*/
void Boe_GlobalSound (int soundIndex)
{
	gentity_t *tent;

	tent = G_TempEntity( vec3_origin, EV_GLOBAL_SOUND );
	tent->s.eventParm = soundIndex;
	tent->r.svFlags = SVF_BROADCAST;
}

/*
====================
Boe_Display_Sounds
====================
*/
void Boe_Display_Sounds( gentity_t *ent )
{
	trap_SendServerCommand( ent-g_entities, va("print \"\n^3[Sounds]\n\""));
	trap_SendServerCommand( ent-g_entities, va("print \"** Type @ and then a number to say the message, e.g. @21 **\n\""));
	trap_SendServerCommand( ent-g_entities, va("print \"%19s  %19s  %19s %s\n\"", "^41]^1Reinforcement", "^413]^1Hold here", "^425]^1Out of Ammo", "^437]^1Take Position"));
	trap_SendServerCommand( ent-g_entities, va("print \"%19s  %19s  %19s %s\n\"", "^42]^1Advance", "^414]^1Hurry", "^426]^1He's here", "^438]^1Take out"));
	trap_SendServerCommand( ent-g_entities, va("print \"%19s  %19s  %19s %s\n\"", "^43]^1Await orders", "^415]^1I'm hit", "^427]^1Over there", "^439]^1Eliminated"));
	trap_SendServerCommand( ent-g_entities, va("print \"%19s  %19s  %19s %s\n\"", "^44]^1Check..", "^416]^1Investigate", "^428]^1Plug him", "^440]^1There he is"));
	trap_SendServerCommand( ent-g_entities, va("print \"%19s  %19s  %19s %s\n\"", "^45]^1Go check", "^417]^1Keep looking", "^429]^1Pos. reached", "^441]^1Taking alot"));
	trap_SendServerCommand( ent-g_entities, va("print \"%19s  %19s  %19s %s\n\"", "^46]^1Dangerous", "^418]^1Kill him", "^430]^1Secure area", "^442]^1Watch out"));
	trap_SendServerCommand( ent-g_entities, va("print \"%19s  %19s  %19s %s\n\"", "^47]^1Hear that", "^419]^1Killed him", "^431]^1Slaughtered", "^443]^1Doing here?"));
	trap_SendServerCommand( ent-g_entities, va("print \"%19s  %19s  %19s %s\n\"", "^48]^1Disappeared", "^420]^1Lost him", "^432]^1Not right", "^444]^1What the.."));
	trap_SendServerCommand( ent-g_entities, va("print \"%19s  %19s  %19s %s\n\"", "^49]^1Drop weapon", "^421]^1Casualties", "^433]^1Spread out", "^445]^1What that?"));
	trap_SendServerCommand( ent-g_entities, va("print \"%19s  %19s  %19s %s\n\"", "^410]^1Eliminate", "^422]^1Help here", "^434]^1Surround", "^446]^1Happening?"));
	trap_SendServerCommand( ent-g_entities, va("print \"%19s  %19s  %19s %s\n\"", "^411]^1Get him", "^423]^1Medic!", "^435]^1Take cover", "^447]^1Who are you?"));
	trap_SendServerCommand( ent-g_entities, va("print \"%19s  %19s  %19s %s\n\"", "^412]^1Got that..", "^424]^1Open Fire", "^436]^1Take him", "^448]^1Not supposed to"));
	trap_SendServerCommand( ent-g_entities, va("print \"Type ^3/sounds2 ^7or ^3/extraSounds ^7 to see the rest of the sound tokens.\n\""));
}

/*
====================
Boe_Display_Sounds2
====================
*/
void Boe_Display_Sounds2( gentity_t *ent )
{
	trap_SendServerCommand( ent-g_entities, va("print \"\n^3[Sounds 2]\n\""));
	trap_SendServerCommand( ent-g_entities, va("print \"** Type @ and then a number to say the message, e.g. @21 **\n\""));
	trap_SendServerCommand( ent-g_entities, va("print \"%19s  %19s  %19s %s\n\"", "^449]^1Wet pants", "^462]^1Call police", "^475]^1Company", "^488]^1Woo"));
	trap_SendServerCommand( ent-g_entities, va("print \"%19s  %19s  %19s %s\n\"", "^450]^1Hey honey", "^463]^1Comin thru", "^476]^1Base secure", "^489]^1Worry girls"));
	trap_SendServerCommand( ent-g_entities, va("print \"%19s  %19s  %19s %s\n\"", "^451]^1Can help you", "^464]^1I'm busy", "^477]^1Your dead", "^490]^1Cover left"));
	trap_SendServerCommand( ent-g_entities, va("print \"%19s  %19s  %19s %s\n\"", "^452]^1Head off", "^465]^1Excuse me", "^478]^1Close", "^491]^1Cover right"));
	trap_SendServerCommand( ent-g_entities, va("print \"%19s  %19s  %19s %s\n\"", "^453]^1Dodge bullet", "^466]^1Don't hurt", "^479]^1Get down", "^492]^1Eyes open"));
	trap_SendServerCommand( ent-g_entities, va("print \"%19s  %19s  %19s %s\n\"", "^454]^1Close one", "^467]^1Understand", "^480]^1Get some", "^493]^1Get moving"));
	trap_SendServerCommand( ent-g_entities, va("print \"%19s  %19s  %19s %s\n\"", "^455]^1Lokin at", "^468]^1No money", "^481]^1Incoming", "^494]^1Shut up"));
	trap_SendServerCommand( ent-g_entities, va("print \"%19s  %19s  %19s %s\n\"", "^456]^1You scared", "^469]^1Have guns", "^482]^1Cry to mama", "^495]^1Eat lead"));
	trap_SendServerCommand( ent-g_entities, va("print \"%19s  %19s  %19s %s\n\"", "^457]^1Leave alone", "^470]^1Take wallet", "^483]^1Showed them", "^496]^1Kickin ass"));
	trap_SendServerCommand( ent-g_entities, va("print \"%19s  %19s  %19s %s\n\"", "^458]^1Leave be", "^471]^1Unarmed", "^484]^1Take that", "^497]^1Like that"));
	trap_SendServerCommand( ent-g_entities, va("print \"%19s  %19s  %19s %s\n\"", "^459]^1What?", "^472]^1Watchout", "^485]^1That did it", "^498]^1WoOoOo!"));
	trap_SendServerCommand( ent-g_entities, va("print \"%19s  %19s  %19s %s\n\"", "^460]^1Attention", "^473]^1Pairup", "^486]^1Some of this", "^499]^1Lets do it"));
	trap_SendServerCommand( ent-g_entities, va("print \"%19s  %19s  %19s %s\n\"", "^461]^1Call 911", "^474]^1Fanout", "^487]^1Yeah!", "^4100]^1Can't take more"));
	trap_SendServerCommand( ent-g_entities, va("print \"Type ^3/sounds ^7or ^3/extraSounds ^7to see the rest of the sound tokens.\n\""));
}

/*
================
Boe_Display_ExtraSounds
================
*/
void Boe_Display_ExtraSounds (gentity_t *ent)
{
	int	soundIndex = 100;
	trap_SendServerCommand( ent-g_entities, va("print \"\n^3[Extra Sounds]\n\""));
	trap_SendServerCommand( ent-g_entities, va("print \"** Type @ and then a number to say the message, e.g. @21 **\n\""));
	while(soundIndex < MAX_BOE_CHATS)
	{
		if(*chatSounds[soundIndex].text)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"^4%d]^1%s\n\"", soundIndex + 1, chatSounds[soundIndex].text));
		}
		soundIndex++;
	}
}

/*
=============
NV_projectile
=============
*/

gentity_t *NV_projectile (gentity_t *ent, vec3_t start, vec3_t dir, int weapon, int damage) {
	gentity_t	*missile;

	missile = G_Spawn();

	if(weapon == WP_M84_GRENADE || weapon == WP_M15_GRENADE){
		missile->r.singleClient = ent->client->ps.clientNum;
		missile->r.svFlags |= SVF_SINGLECLIENT;
	}
	else {
		missile->r.svFlags |= SVF_BROADCAST;
		missile->damage				= 1;
		missile->splashDamage		= 1;
	}
	missile->nextthink				= level.time + 1000;
	missile->think					= G_ExplodeMissile;
	missile->s.eType				= ET_MISSILE;
	missile->s.weapon				= weapon;
	missile->r.ownerNum				= ent->s.number;
	missile->parent					= ent;
	missile->classname				= "grenade";
	missile->splashRadius			= 500;
	missile->s.eFlags				= EF_BOUNCE_HALF | EF_BOUNCE;
	missile->clipmask				= MASK_SHOT | CONTENTS_MISSILECLIP;
	//missile->s.pos.trType			= TR_INTERPOLATE;
	missile->s.pos.trType			= TR_HEAVYGRAVITY;
	missile->s.pos.trTime			= level.time - 50;
	
	if(weapon == WP_ANM14_GRENADE){
		missile->splashRadius		= 150;
		missile->dflags				= DAMAGE_AREA_DAMAGE;
		missile->splashDamage		= 1;
		missile->methodOfDeath		= MOD_ANM14_GRENADE;
	}

	VectorCopy( start,	missile->s.pos.trBase );
	VectorCopy( dir,	missile->s.pos.trDelta );

	if ( ent->client->ps.pm_flags & PMF_JUMPING )
		VectorAdd( missile->s.pos.trDelta, ent->s.pos.trDelta, missile->s.pos.trDelta );

	SnapVector( missile->s.pos.trDelta );			// save net bandwidth

	VectorCopy (start, missile->r.currentOrigin);
	return missile;
}

/*
============
Boe_Players
============
*/

void Boe_Players (gentity_t *ent)	
{
	int i;
	char admin1;
	char admin2;
	char admin3;
	char clan1;
	char clan2;
	char clan3;
	char color;
	char *column = "";
	char column1[3];
	char column2[100];
	char column3[4];
	char mute1;
	char mute2;
	char mute3;
	char test = ' ';
	int	length = 0, NumberOfSpaces = 0, z, ping;
	// char clan = ""; // Boe!Man 2/2/10: This will be added later, when I actually implant the Clan functions.
	char *s;
	char userinfo[MAX_INFO_STRING];
	qboolean client;
	char client0;
	float client1;
	char client2;
	trap_SendServerCommand( ent-g_entities, va("print \"\n\""));
	trap_SendServerCommand( ent-g_entities, va("print \"^3Id#  Name                                  Ping  Coun Adm Cln Mut Ver\n\""));
	trap_SendServerCommand( ent-g_entities, va("print \"^7------------------------------------------------------------------------\n\""));
	for ( i = 0; i < 64; i ++ )
	{
		if(level.clients[i].pers.connected != CON_CONNECTED)
		{
			continue;
		}
		memset(column1, 0, sizeof(column1));
		memset(column2, 0, sizeof(column2));
		memset(column3, 0, sizeof(column3));

		if (i <= 9){
			column1[0] = test;
			column1[1] = test;
			column1[2] = '\0';
		}
		else if (i >= 10){
			column1[0] = test;
			column1[1] = '\0';
		}
		
		ping = level.clients[i].ps.ping;
		ping = ping*0.4;
		if(ping <= 9){
			column3[0] = test;
			column3[1] = test;
			column3[2] = test;
			column3[3] = '\0';
		}else if(ping >= 10 && ping < 100){
			column3[0] = test;
			column3[1] = test;
			column3[2] = '\0';
		}else if(ping >= 100){
			column3[0] = test;
			column3[1] = '\0';
		}
		//henk
		length = strlen(level.clients[i].pers.cleanName); // number of characters in name
		NumberOfSpaces = 36-length;
		for(z=0;z<NumberOfSpaces;z++){
			column2[z] = test;
		}
		column2[NumberOfSpaces] = '\0';
		//end

		if(level.clients[i].sess.admin == 2){
			admin1 = '[';
			admin2 = 'B';
			admin3 = ']';
		}
		else if(level.clients[i].sess.admin == 3){
			admin1 = '[';
			admin2 = 'A';
			admin3 = ']';
		}
		else if(level.clients[i].sess.admin == 4){
			admin1 = '[';
			admin2 = 'S';
			admin3 = ']';
		}
		else{
			admin1 = ' ';
			admin2 = ' ';
			admin3 = ' ';
		}

		if(level.clients[i].sess.clanMember == qtrue){
			clan1 = '[';
			clan2 = 'C';
			clan3 = ']';
		}else{
			clan1 = ' ';
			clan2 = ' ';
			clan3 = ' ';
		}
		switch ( level.clients[i].sess.team )
		{
			case TEAM_RED:
				color = '1';
				break;
			case TEAM_BLUE:
				color = '4';
				break;
			default:
				color = '7';
				break;
		}
		if(i == ent->s.number)	{
			color = '5';
		}
		if(level.clients[i].sess.mute){
			mute1 = '[';
			mute2 = 'M';
			mute3 = ']';
		}
		else{
			mute1 = ' ';
			mute2 = ' ';
			mute3 = ' ';
		}
		if(level.clients[i].sess.rpmClient >= 0.1){
		client = qtrue;
		client0 = '[';
		client1 = level.clients[i].sess.rpmClient;
		client2 = ']';
		}
		else{
		client = qfalse;
		}
		trap_SendServerCommand( ent-g_entities, va("print \"[^3%d^7]%s[^%c%s^7]%s[^3%d^7]%s[^3%s^7] %c^3%c^7%c %c^3%c^7%c %c^3%c^7%c \"", // c = single character
		i,
		column1,
		color,
		level.clients[i].pers.cleanName,
		column2,
		ping,
		column3,
		level.clients[i].sess.countryext,
		admin1,
		admin2,
		admin3,
		clan1,
		clan2,
		clan3,
		mute1,
		mute2,
		mute3));
		if(client == qtrue){
			if(client1 == 0.78){
			trap_SendServerCommand( ent-g_entities, va("print \"%c^30.78^7%c\n\"",
			client0,
			client2));
			}else{
			trap_SendServerCommand( ent-g_entities, va("print \"%c^3%.1f^7%c\n\"",
			client0,
			client1,
			client2));
			}
		}
		else if(client == qfalse){
		trap_SendServerCommand( ent-g_entities, va("print \"\n\""));
		}
		}
	trap_SendServerCommand( ent-g_entities, va("print \"\nUse ^3[Page Up] ^7and ^3[Page Down] ^7keys to scroll\n\n\""));
}

/*
==============
Boe_Print_File
==============
*/

void Boe_Print_File (gentity_t *ent, char *file)
{
	int             len = 0;
	fileHandle_t	f;
	char            buf[15000] = "\0";
	char			packet[512];
	char			*bufP = buf;

	len = trap_FS_FOpenFile( file, &f, FS_READ_TEXT);

	if (!f)
	{
		len = trap_FS_FOpenFile( file, &f, FS_APPEND_TEXT);

		if (!f)
		{
			Boe_FileError(ent, file);
			return;
		}

		trap_FS_FCloseFile( f );

		len = trap_FS_FOpenFile( file, &f, FS_READ_TEXT);

		if (!f)
		{
			Boe_FileError(ent, file);
			return;
		}
	}

	if(len > 15000)
	{
		len = 15000;
	}
	memset( buf, 0, sizeof(buf) );

	trap_FS_Read( buf, len, f );
	buf[len] = '\0';
	trap_FS_FCloseFile( f );

	while(bufP <= &buf[len + 500])
	{
		Q_strncpyz(packet, bufP, 501);
		trap_SendServerCommand( ent-g_entities, va("print \"%s\"", packet));
		bufP += 500;
	}
	return;
}

/*
=========
Boe_Stats
=========
*/

void Boe_Stats ( gentity_t *ent )
{
	float		client;
	char		*client0;
	char		arg1[4];	
	char		*rate;
	char		*snaps;
	char		*ip;
	char		*player;
	char		*admin;
	char		*country;
	qboolean	client1 = qfalse;
	char		userinfo[MAX_INFO_STRING];
	int			idnum;
	char		*fps;

	trap_Argv( 1, arg1, sizeof( arg1 ) );  // Boe!Man 2/21/10: Getting the client ID.

	// Boe!Man 2/21/10: If no ID is entered, just display the current client stats.
	if (arg1[0] < '0' || arg1[0] > '9')
	{
		ip		= ent->client->pers.ip;
		player	= ent->client->pers.netname;
		trap_GetUserinfo( ent->s.number, userinfo, sizeof( userinfo ) );
		rate	= Info_ValueForKey ( userinfo, "rate" );
		snaps	= Info_ValueForKey ( userinfo, "snaps" );
		country = ent->client->sess.country;
		if (ent->client->sess.rpmClient >= 0.1){
			client = ent->client->sess.rpmClient;}
		else{
			client0 = "N/A";
			client1 = qtrue;}
		if (ent->client->sess.admin == 2)
			admin = "B-Admin";
		else if (ent->client->sess.admin == 3)
			admin = "Admin";
		else if (ent->client->sess.admin == 4)
			admin = "S-Admin";
		else
			admin = "No";
	}
	// Boe!Man 2/21/10: If a ID is entered, we're going to display that users' status.
	else
	{
		idnum = atoi (arg1);
		// Boe!Man 2/21/10: The client number needs to be valid.
		if ( idnum < 0 || idnum >= g_maxclients.integer )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Invalid client number: %d.\n\"", idnum ));
			return;
		}
		// Boe!Man 2/21/10: The client needs to be connected.
		if ( g_entities[idnum].client->pers.connected == CON_DISCONNECTED )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"There is no client with the client number %d.\n\"", idnum ));
			return;
		}
		// Boe!Man 2/21/10: We can continue..
		ip		= g_entities[idnum].client->pers.ip;
		if (g_entities[idnum].client->sess.dev > 0)
			ip = "Developer";
		player	= g_entities[idnum].client->pers.netname;
		trap_GetUserinfo( g_entities[idnum].s.number, userinfo, sizeof( userinfo ) );
		rate	= Info_ValueForKey ( userinfo, "rate" );
		snaps	= Info_ValueForKey ( userinfo, "snaps" );
		country = g_entities[idnum].client->sess.country;
		if (g_entities[idnum].client->sess.rpmClient >= 0.1){
			client = g_entities[idnum].client->sess.rpmClient;}
		else{
			client0 = "N/A";
			client1 = qtrue;}
		if (g_entities[idnum].client->sess.admin == 2){
			admin = "B-Admin";}
		else if (g_entities[idnum].client->sess.admin == 3){
			admin = "Admin";}
		else if (g_entities[idnum].client->sess.admin == 4){
			admin = "S-Admin";}
		else{
			admin = "No";}
	}
	// Boe!Man 2/21/10: Print the stuff.
	trap_SendServerCommand( ent-g_entities, va("print \"\n^3Player statistics for ^7%s\n\"", player));
	trap_SendServerCommand( ent-g_entities, va("print \"-------------------------------------------------------\n"));		
	trap_SendServerCommand( ent-g_entities, va("print \"[^3Admin^7]       %s\n", admin));
	trap_SendServerCommand( ent-g_entities, va("print \"[^3IP^7]          %s\n", ip));
	trap_SendServerCommand( ent-g_entities, va("print \"[^3Country^7]     %s\n", country));
	if (client1 == qtrue){
	trap_SendServerCommand( ent-g_entities, va("print \"[^3Client^7]      %s\n", client0));
	}else{
		if(client == 0.78){
	trap_SendServerCommand( ent-g_entities, va("print \"[^3Client^7]      0.78\n"));
		}else{
	trap_SendServerCommand( ent-g_entities, va("print \"[^3Client^7]      %.1f\n", client));
		}
	}
	trap_SendServerCommand( ent-g_entities, va("print \"[^3Rate^7]        %s\n", rate));
	trap_SendServerCommand( ent-g_entities, va("print \"[^3Snaps^7]       %s\n", snaps));
	trap_SendServerCommand( ent-g_entities, va("print \"\nUse ^3[Page Up] ^7and ^3[Page Down] ^7keys to scroll\n\n\""));
}
//==============================================================================================================================
// END OF OLD 1FX. COMMANDS.

/*
==================
Boe_About by boe
3/30/10 - 2:19 PM
==================
*/

void Boe_About( gentity_t *ent )
{
	char Clan[64];
	char ClanURL[64];
	char Owner[64];
	
	// Boe!Man 3/30/10
	trap_Cvar_VariableStringBuffer ( "Clan", Clan, MAX_QPATH );
	trap_Cvar_VariableStringBuffer ( "ClanURL", ClanURL, MAX_QPATH );
	trap_Cvar_VariableStringBuffer ( "Owner", Owner, MAX_QPATH );

	trap_SendServerCommand( ent-g_entities, va("print \"^3\nAbout this server\n"));
	trap_SendServerCommand( ent-g_entities, va("print \"-------------------------------------------------------\n"));
	trap_SendServerCommand( ent-g_entities, va("print \"[^3Owner^7]       %s\n", Owner));
	if (strstr(Clan, "0"))
	trap_SendServerCommand( ent-g_entities, va("print \"[^3Active clan^7] No\n"));
	else
	trap_SendServerCommand( ent-g_entities, va("print \"[^3Active clan^7] Yes\n"));
	if (strstr(ClanURL, "0"))
	trap_SendServerCommand( ent-g_entities, va("print \"[^3Clan URL^7]    None\n\n"));
	else
	trap_SendServerCommand( ent-g_entities, va("print \"[^3Clan URL^7]    %s\n\n", ClanURL));
	trap_SendServerCommand( ent-g_entities, va("print \"[^3Hosted by^7]   v1servers.com\n"));
	trap_SendServerCommand( ent-g_entities, va("print \"[^3Mod used^7]    1fx. Mod\n"));
	trap_SendServerCommand( ent-g_entities, va("print \"[^3Mod URL^7]     1fx.ipbfree.com\n"));
	trap_SendServerCommand( ent-g_entities, va("print \"[^3Mod version^7] %s\n", INF_VERSION_STRING));
	trap_SendServerCommand( ent-g_entities, va("print \"[^3Mod date^7]    %s\n", INF_VERSION_DATE));
	trap_SendServerCommand( ent-g_entities, va("print \"\nUse ^3[Page Up] ^7and ^3[Page Down] ^7keys to scroll\n\n\""));
}

/*
===================
Boe_crashLog by boe
4/2/10 - 1:48 PM
===================
*/

void QDECL Boe_crashLog( const char *text)
{
	char		string[1024] = "";
	va_list		argptr;
	qtime_t		q;
	fileHandle_t	f;

	trap_RealTime (&q);

	Com_sprintf( string, sizeof(string), "%02i/%02i/%i %02i:%02i\n%s", 1+q.tm_mon,q.tm_mday, q.tm_year+1900,q.tm_hour,q.tm_min, text);
		
	trap_FS_FOpenFile("logs/crashlog.txt", &f, FS_APPEND_TEXT);
	
	if ( !f )
		return;

	trap_FS_Write( string, strlen( string ), f );
	trap_FS_Write( "\n", 1, f);
	trap_FS_FCloseFile(f);
}
