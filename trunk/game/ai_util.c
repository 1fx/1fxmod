// Copyright (C) 2001-2002 Raven Software.
//
// ai_util.c

#include "g_local.h"
#include "q_shared.h"
#include "botlib.h"
#include "ai_main.h"

#ifdef BOT_ZMALLOC
#define MAX_BALLOC 8192

void *BAllocList[MAX_BALLOC];
#endif

char gBotChatBuffer[MAX_CLIENTS][MAX_CHAT_BUFFER_SIZE];
//A total of 4194304 bytes. Not very nice at all, but we really
//want to have at least 65k for the total chat buffer just in
//case and we have no method of dynamic allocation here.

void *B_TempAlloc(int size)
{
	return trap_VM_LocalTempAlloc(size);
}

void B_TempFree(int size)
{
	trap_VM_LocalTempFree(size);
}

void *B_Alloc(int size)
{
#ifdef BOT_ZMALLOC
	void *ptr = NULL;
	int i = 0;

#ifdef BOTMEMTRACK
	int free = 0;
	int used = 0;

	while (i < MAX_BALLOC)
	{
		if (!BAllocList[i])
		{
			free++;
		}
		else
		{
			used++;
		}

		i++;
	}

	Com_Printf("Allocations used: %i\nFree allocation slots: %i\n", used, free);

	i = 0;
#endif

	ptr = trap_BotGetMemoryGame(size);

	while (i < MAX_BALLOC)
	{
		if (!BAllocList[i])
		{
			BAllocList[i] = ptr;
			break;
		}
		i++;
	}

	if (i == MAX_BALLOC)
	{
		//If this happens we'll have to rely on this chunk being freed manually with B_Free, which it hopefully will be
#ifdef DEBUG
		Com_Printf("WARNING: MAXIMUM B_ALLOC ALLOCATIONS EXCEEDED\n");
#endif
	}

	return ptr;
#else
	return trap_VM_LocalAlloc(size);
#endif
}

void B_Free(void *ptr)
{
#ifdef BOT_ZMALLOC
	int i = 0;

#ifdef BOTMEMTRACK
	int free = 0;
	int used = 0;

	while (i < MAX_BALLOC)
	{
		if (!BAllocList[i])
		{
			free++;
		}
		else
		{
			used++;
		}

		i++;
	}

	Com_Printf("Allocations used: %i\nFree allocation slots: %i\n", used, free);

	i = 0;
#endif

	while (i < MAX_BALLOC)
	{
		if (BAllocList[i] == ptr)
		{
			BAllocList[i] = NULL;
			break;
		}

		i++;
	}

	if (i == MAX_BALLOC)
	{
		//Likely because the limit was exceeded and we're now freeing the chunk manually as we hoped would happen
#ifdef DEBUG
		Com_Printf("WARNING: Freeing allocation which is not in the allocation structure\n");
#endif
	}

	trap_BotFreeMemoryGame(ptr);
#endif
}

void B_InitAlloc(void)
{
#ifdef BOT_ZMALLOC
	memset(BAllocList, 0, sizeof(BAllocList));
#endif

	memset(gWPArray, 0, sizeof(gWPArray));
}

void B_CleanupAlloc(void)
{
#ifdef BOT_ZMALLOC
	int i = 0;

	while (i < MAX_BALLOC)
	{
		if (BAllocList[i])
		{
			trap_BotFreeMemoryGame(BAllocList[i]);
			BAllocList[i] = NULL;
		}

		i++;
	}
#endif
}

int GetValueGroup(char *buf, char *group, char *outbuf)
{
	char *place, *placesecond;
	int iplace;
	int failure;
	int i;
	int startpoint, startletter;
	int subg = 0;

	i = 0;

	iplace = 0;

	place = strstr(buf, group);

	if (!place)
	{
		return 0;
	}

	startpoint = place - buf + strlen(group) + 1;
	startletter = (place - buf) - 1;

	failure = 0;

	while (buf[startpoint+1] != '{' || buf[startletter] != '\n')
	{
		placesecond = strstr(place+1, group);

		if (placesecond)
		{
			startpoint += (placesecond - place);
			startletter += (placesecond - place);
			place = placesecond;
		}
		else
		{
			failure = 1;
			break;
		}
	}

	if (failure)
	{
		return 0;
	}

	//we have found the proper group name if we made it here, so find the opening brace and read into the outbuf
	//until hitting the end brace

	while (buf[startpoint] != '{')
	{
		startpoint++;
	}

	startpoint++;

	while (buf[startpoint] != '}' || subg)
	{
		if (buf[startpoint] == '{')
		{
			subg++;
		}
		else if (buf[startpoint] == '}')
		{
			subg--;
		}
		outbuf[i] = buf[startpoint];
		i++;
		startpoint++;
	}
	outbuf[i] = '\0';

	return 1;
}

int GetPairedValue(char *buf, char *key, char *outbuf)
{
	char *place, *placesecond;
	int startpoint, startletter;
	int i, found;

	if (!buf || !key || !outbuf)
	{
		return 0;
	}

	i = 0;

	while (buf[i] && buf[i] != '\0')
	{
		if (buf[i] == '/')
		{
			if (buf[i+1] && buf[i+1] != '\0' && buf[i+1] == '/')
			{
				while (buf[i] != '\n')
				{
					buf[i] = '/';
					i++;
				}
			}
		}
		i++;
	}

	place = strstr(buf, key);

	if (!place)
	{
		return 0;
	}
	//tab == 9
	startpoint = place - buf + strlen(key);
	startletter = (place - buf) - 1;

	found = 0;

	while (!found)
	{
		if (startletter == 0 || !buf[startletter] || buf[startletter] == '\0' || buf[startletter] == 9 || buf[startletter] == ' ' || buf[startletter] == '\n')
		{
			if (buf[startpoint] == '\0' || buf[startpoint] == 9 || buf[startpoint] == ' ' || buf[startpoint] == '\n')
			{
				found = 1;
				break;
			}
		}

		placesecond = strstr(place+1, key);

		if (placesecond)
		{
			startpoint += placesecond - place;
			startletter += placesecond - place;
			place = placesecond;
		}
		else
		{
			place = NULL;
			break;
		}

	}

	if (!found || !place || !buf[startpoint] || buf[startpoint] == '\0')
	{
		return 0;
	}

	while (buf[startpoint] == ' ' || buf[startpoint] == 9 || buf[startpoint] == '\n')
	{
		startpoint++;
	}

	i = 0;

	while (buf[startpoint] && buf[startpoint] != '\0' && buf[startpoint] != '\n')
	{
		outbuf[i] = buf[startpoint];
		i++;
		startpoint++;
	}

	outbuf[i] = '\0';

	return 1;
}

int BotDoChat(bot_state_t *bs, char *section, int always)
{
	char *chatgroup;
	int rVal;
	int inc_1;
	int inc_2;
	int inc_n;
	int lines;
	int checkedline;
	int getthisline;
	gentity_t *cobject;

	if (!bs->canChat)
	{
		return 0;
	}

	if (bs->doChat)
	{ //already have a chat scheduled
		return 0;
	}

	if (Q_irand(1, 10) > bs->chatFrequency && !always)
	{
		return 0;
	}

	bs->chatTeam = 0;

	chatgroup = (char *)B_TempAlloc(MAX_CHAT_BUFFER_SIZE);

	rVal = GetValueGroup(gBotChatBuffer[bs->client], section, chatgroup);

	if (!rVal) //the bot has no group defined for the specified chat event
	{
		B_TempFree(MAX_CHAT_BUFFER_SIZE); //chatgroup
		return 0;
	}

	inc_1 = 0;
	inc_2 = 2;

	while (chatgroup[inc_2] && chatgroup[inc_2] != '\0')
	{
		if (chatgroup[inc_2] != 13 && chatgroup[inc_2] != 9)
		{
			chatgroup[inc_1] = chatgroup[inc_2];
			inc_1++;
		}
		inc_2++;
	}
	chatgroup[inc_1] = '\0';

	inc_1 = 0;

	lines = 0;

	while (chatgroup[inc_1] && chatgroup[inc_1] != '\0')
	{
		if (chatgroup[inc_1] == '\n')
		{
			lines++;
		}
		inc_1++;
	}

	if (!lines)
	{
		B_TempFree(MAX_CHAT_BUFFER_SIZE); //chatgroup
		return 0;
	}

	getthisline = Q_irand(0, (lines+1));

	if (getthisline < 1)
	{
		getthisline = 1;
	}
	if (getthisline > lines)
	{
		getthisline = lines;
	}

	checkedline = 1;

	inc_1 = 0;

	while (checkedline != getthisline)
	{
		if (chatgroup[inc_1] && chatgroup[inc_1] != '\0')
		{
			if (chatgroup[inc_1] == '\n')
			{
				inc_1++;
				checkedline++;
			}
		}

		if (checkedline == getthisline)
		{
			break;
		}

		inc_1++;
	}

	//we're at the starting position of the desired line here
	inc_2 = 0;

	while (chatgroup[inc_1] != '\n')
	{
		chatgroup[inc_2] = chatgroup[inc_1];
		inc_2++;
		inc_1++;
	}
	chatgroup[inc_2] = '\0';

	//trap_EA_Say(bs->client, chatgroup);
	inc_1 = 0;
	inc_2 = 0;

	if (strlen(chatgroup) > MAX_CHAT_LINE_SIZE)
	{
		B_TempFree(MAX_CHAT_BUFFER_SIZE); //chatgroup
		return 0;
	}

	while (chatgroup[inc_1])
	{
		if (chatgroup[inc_1] == '%' && chatgroup[inc_1+1] != '%')
		{
			inc_1++;

			if (chatgroup[inc_1] == 's' && bs->chatObject)
			{
				cobject = bs->chatObject;
			}
			else if (chatgroup[inc_1] == 'a' && bs->chatAltObject)
			{
				cobject = bs->chatAltObject;
			}
			else
			{
				cobject = NULL;
			}

			if (cobject && cobject->client)
			{
				inc_n = 0;

				while (cobject->client->pers.netname[inc_n])
				{
					bs->currentChat[inc_2] = cobject->client->pers.netname[inc_n];
					inc_2++;
					inc_n++;
				}
				inc_2--; //to make up for the auto-increment below
			}
		}
		else
		{
			bs->currentChat[inc_2] = chatgroup[inc_1];
		}
		inc_2++;
		inc_1++;
	}
	bs->currentChat[inc_2] = '\0';

	if (strcmp(section, "GeneralGreetings") == 0)
	{
		bs->doChat = 2;
	}
	else
	{
		bs->doChat = 1;
	}
	bs->chatTime_stored = (strlen(bs->currentChat)*45)+Q_irand(1300, 1500);
	bs->chatTime = level.time + bs->chatTime_stored;

	B_TempFree(MAX_CHAT_BUFFER_SIZE); //chatgroup
	return 1;
}

void ParseEmotionalAttachments(bot_state_t *bs, char *buf)
{
	int i = 0;
	int i_c = 0;
	char tbuf[16];

	while (buf[i] && buf[i] != '}')
	{
		while (buf[i] == ' ' || buf[i] == '{' || buf[i] == 9 || buf[i] == 13 || buf[i] == '\n')
		{
			i++;
		}

		if (buf[i] && buf[i] != '}')
		{
			i_c = 0;
			while (buf[i] != '{' && buf[i] != 9 && buf[i] != 13 && buf[i] != '\n')
			{
				bs->loved[bs->lovednum].name[i_c] = buf[i];
				i_c++;
				i++;
			}
			bs->loved[bs->lovednum].name[i_c] = '\0';

			while (buf[i] == ' ' || buf[i] == '{' || buf[i] == 9 || buf[i] == 13 || buf[i] == '\n')
			{
				i++;
			}

			i_c = 0;

			while (buf[i] != '{' && buf[i] != 9 && buf[i] != 13 && buf[i] != '\n')
			{
				tbuf[i_c] = buf[i];
				i_c++;
				i++;
			}
			tbuf[i_c] = '\0';

			bs->loved[bs->lovednum].level = atoi(tbuf);

			bs->lovednum++;
		}
		else
		{
			break;
		}

		if (bs->lovednum >= MAX_LOVED_ONES)
		{
			return;
		}

		i++;
	}
}

int ReadChatGroups(bot_state_t *bs, char *buf)
{
	char *cgroupbegin;
	int cgbplace;
	int i;

	cgroupbegin = strstr(buf, "BEGIN_CHAT_GROUPS");

	if (!cgroupbegin)
	{
		return 0;
	}

	if (strlen(cgroupbegin) >= MAX_CHAT_BUFFER_SIZE)
	{
		Com_Printf(S_COLOR_RED "Error: Personality chat section exceeds max size\n");
		return 0;
	}

	cgbplace = cgroupbegin - buf+1;

	while (buf[cgbplace] != '\n')
	{
		cgbplace++;
	}

	i = 0;

	while (buf[cgbplace] && buf[cgbplace] != '\0')
	{
		gBotChatBuffer[bs->client][i] = buf[cgbplace];
		i++;
		cgbplace++;
	}

	gBotChatBuffer[bs->client][i] = '\0';

	return 1;
}

char personalityBuffer[131072];
// Henk 1/14/10
char *buffer = personalityBuffer;

void BotUtilizePersonality(bot_state_t *bs)
{
	fileHandle_t f;
	int len, rlen;
	int failed;
	int i;
	char *readbuf, *group;

	len = trap_FS_FOpenFile(bs->settings.personalityfile, &f, FS_READ);

	failed = 0;

	if (!f)
	{
		Com_Printf(S_COLOR_RED "Error: Specified personality not found\n");
		return;
	}

	if (len >= 131072)
	{
		Com_Printf(S_COLOR_RED "Personality file exceeds maximum length\n");
		///RxCxW - 08.26.06 - 12:34am - #File handle - Close this beotch!
		trap_FS_FCloseFile(f);
		///End  - 08.26.06 - 12:34am
		return;
	}

	trap_FS_Read(personalityBuffer, len, f);

	rlen = len;

	while (len < 131072)
	{ //kill all characters after the file length, since sometimes FS_Read doesn't do that entirely (or so it seems)
		personalityBuffer[len] = '\0';
		len++;
	}

	len = rlen;

	readbuf = (char *)trap_VM_LocalTempAlloc(1024);
	group = (char *)trap_VM_LocalTempAlloc(65536);

	if (!GetValueGroup(personalityBuffer, "GeneralBotInfo", group))
	{
		Com_Printf(S_COLOR_RED "Personality file contains no GeneralBotInfo group\n");
		failed = 1; //set failed so we know to set everything to default values
	}

	if (!failed && GetPairedValue(group, "reflex", readbuf))
	{
		bs->skills.reflex = atoi(readbuf);
	}
	else
	{
		bs->skills.reflex = 100; //default
	}

	if (!failed && GetPairedValue(group, "accuracy", readbuf))
	{
		bs->skills.accuracy = atof(readbuf);
	}
	else
	{
		bs->skills.accuracy = 10; //default
	}

	if (!failed && GetPairedValue(group, "turnspeed", readbuf))
	{
		bs->skills.turnspeed = atof(readbuf);
	}
	else
	{
		bs->skills.turnspeed = 0.01f; //default
	}

	if (!failed && GetPairedValue(group, "turnspeed_combat", readbuf))
	{
		bs->skills.turnspeed_combat = atof(readbuf);
	}
	else
	{
		bs->skills.turnspeed_combat = 0.05f; //default
	}

	if (!failed && GetPairedValue(group, "maxturn", readbuf))
	{
		bs->skills.maxturn = atof(readbuf);
	}
	else
	{
		bs->skills.maxturn = 360; //default
	}

	if (!failed && GetPairedValue(group, "perfectaim", readbuf))
	{
		bs->skills.perfectaim = atoi(readbuf);
	}
	else
	{
		bs->skills.perfectaim = 0; //default
	}

	if (!failed && GetPairedValue(group, "chatability", readbuf))
	{
		bs->canChat = atoi(readbuf);
	}
	else
	{
		bs->canChat = 0; //default
	}

	if (!failed && GetPairedValue(group, "chatfrequency", readbuf))
	{
		bs->chatFrequency = atoi(readbuf);
	}
	else
	{
		bs->chatFrequency = 5; //default
	}

	if (!failed && GetPairedValue(group, "hatelevel", readbuf))
	{
		bs->loved_death_thresh = atoi(readbuf);
	}
	else
	{
		bs->loved_death_thresh = 3; //default
	}

	if (!failed && GetPairedValue(group, "camper", readbuf))
	{
		bs->isCamper = atoi(readbuf);
	}
	else
	{
		bs->isCamper = 0; //default
	}

	if (!failed && GetPairedValue(group, "meleeSpecialist", readbuf))
	{
		bs->meleeSpecialist = atoi(readbuf);
	}
	else
	{
		bs->meleeSpecialist = 0; //default
	}

	i = 0;

	while (i < MAX_CHAT_BUFFER_SIZE)
	{ //clear out the chat buffer for this bot
		gBotChatBuffer[bs->client][i] = '\0';
		i++;
	}

	if (bs->canChat)
	{
		if (!ReadChatGroups(bs, personalityBuffer))
		{
			bs->canChat = 0;
		}
	}

	if (GetValueGroup(personalityBuffer, "BotWeaponWeights", group))
	{
		for (i=0; i < level.wpNumWeapons; i++)
		{
			if (GetPairedValue(group, bg_weaponNames[i], readbuf) || GetPairedValue(group, bg_enumWeaponNames[i], readbuf))
			{
				bs->botWeaponWeights[i] = atoi(readbuf);
			}
		}
	}

	bs->lovednum = 0;

	if (GetValueGroup(personalityBuffer, "EmotionalAttachments", group))
	{
		ParseEmotionalAttachments(bs, group);
	}

	trap_VM_LocalTempFree(65536);
	trap_VM_LocalTempFree(1024);
	trap_FS_FCloseFile(f);
}
