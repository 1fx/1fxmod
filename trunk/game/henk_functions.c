#include "g_local.h"
#include "boe_local.h"

char *GetCountry(char *ext);

void trap_LinkEntity( gentity_t *ent ) {
	if(ent == NULL){
		G_LogPrintf("!=!=!=!=!=!=!=!=WARNING=!=!=!=!=!=!=!=!=!=! Trying to free a NULL ent, report at 1fx.uk.to(Debug -> link)");
		return;
	}
	trap_LinkEntity1(ent);
}

void trap_UnlinkEntity( gentity_t *ent ) {
	if(ent == NULL){
		G_LogPrintf("!=!=!=!=!=!=!=!=WARNING=!=!=!=!=!=!=!=!=!=! Trying to free a NULL ent, report at 1fx.uk.to(Debug -> unlink)");
		return;
	}
	trap_UnlinkEntity1(ent);
}

qboolean IsValidCommand(char *cmd, char *string){
	int i, z;
	for(i=0;i<=strlen(string);i++){
		if(string[i] == '!'){
			for(z=1;z<=strlen(cmd)-1;z++){
				if(string[i+z] != cmd[z]){
					break; // invalid command
				}else if(z == strlen(cmd)-1){
					if(henk_isdigit(string[(i+z)+1]))
						return qtrue;
					else if(!string[(i+z)+1])
						return qtrue;
					else if(string[(i+z)+1] != ' ')
						return qfalse;
				}
			}
		}
	}
	return qtrue;
}

int TiedPlayers(void){
	int i, highscore = 0, count = 0;
	gentity_t *ent;
	for(i=0;i<level.numConnectedClients;i++){
		ent = &g_entities[level.sortedClients[i]];
		if(ent->client->sess.team == TEAM_RED){
			if(ent->client->sess.kills == highscore){
				count += 1;
				highscore = ent->client->sess.kills;
			}else if(ent->client->sess.kills > highscore){
				count = 1;
				highscore = ent->client->sess.kills;
			}
		}
	}
	return count;
}

void InitCagefight(void){
	int i, count = 0;
	vec3_t spawns[33];
	if(level.time < level.cagefighttimer){
		return;
	}
	level.startcage = qfalse;

	G_ResetGametype(qfalse, qtrue);
	for(i=0;i<=32;i++){
		VectorCopy(level.hideseek_cage, spawns[i]);
	}
	spawns[0][0] -= 105;
	spawns[0][1] -= 105;
	spawns[1][0] += 105;
	spawns[1][1] += 105;
	spawns[2][0] -= 105;
	spawns[2][1] += 105;
	spawns[3][0] += 105;
	spawns[3][1] -= 105;
	spawns[4][0] += 105; // fifth spawn
	spawns[4][1] -= 0;
	spawns[5][0] -= 105;
	spawns[5][1] += 0;
	spawns[6][0] -= 0;
	spawns[6][1] += 105;
	spawns[7][0] += 0;
	spawns[7][1] -= 105;
	spawns[8][0] += 105; // ninth spawn
	spawns[8][1] -= 50;
	spawns[9][0] -= 105;
	spawns[9][1] += 50;
	spawns[10][0] -= 50;
	spawns[10][1] += 105;
	spawns[11][0] += 50;
	spawns[11][1] -= 105;
	spawns[12][0] -= 50;
	spawns[12][1] -= 105;
	spawns[13][0] += 50;
	spawns[13][1] += 105;
	spawns[14][0] -= 105;
	spawns[14][1] -= 50;
	spawns[15][0] += 105;
	spawns[15][1] += 50;
	spawns[16][0] += 50;
	spawns[16][1] -= 50;
	spawns[17][0] += 50;
	spawns[17][1] += 50;
	spawns[18][0] -= 50;
	spawns[18][1] += 50;
	spawns[19][0] -= 50;
	spawns[19][1] -= 50;

	spawns[20][0] += 0;
	spawns[20][1] -= 50;
	spawns[21][0] += 50;
	spawns[21][1] += 0;
	spawns[22][0] -= 0;
	spawns[22][1] += 50;
	spawns[23][0] -= 50;
	spawns[23][1] -= 0;
	spawns[24][0] -= 0;
	spawns[24][1] -= 0;

	level.cagefight = qtrue;
	level.messagedisplay = qtrue; // stop Seeker Released
	level.messagedisplay1 = qtrue; // stop RPG/M4 stuff
	SpawnCage(level.hideseek_cage, NULL, qtrue);
	SpawnCage(level.hideseek_cage, NULL, qtrue); // 2 to be sure no parts are missing
	for(i=0;i<=level.numConnectedClients;i++){
		if(g_entities[level.sortedClients[i]].client->sess.team == TEAM_RED){
			//respawn ( &g_entities[level.sortedClients[i]] );
			TeleportPlayer(&g_entities[level.sortedClients[i]], spawns[count], g_entities[level.sortedClients[i]].client->ps.viewangles, qtrue);
			count += 1;
			g_entities[level.sortedClients[i]].client->ps.stats[STAT_WEAPONS] = 0;
			memset ( g_entities[level.sortedClients[i]].client->ps.ammo, 0, sizeof(g_entities[level.sortedClients[i]].client->ps.ammo) );
			memset ( g_entities[level.sortedClients[i]].client->ps.clip, 0, sizeof(g_entities[level.sortedClients[i]].client->ps.clip) );
			//g_entities[level.lastalive[0]].client->ps.ammo[weaponData[WP_RPG7_LAUNCHER].attack[ATTACK_NORMAL].ammoIndex]=2;
			g_entities[level.sortedClients[i]].client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_KNIFE );
			g_entities[level.sortedClients[i]].client->ps.clip[ATTACK_NORMAL][WP_KNIFE]=1;
			g_entities[level.sortedClients[i]].client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_AK74_ASSAULT_RIFLE );
			//g_entities[level.lastalive[0]].client->ps.clip[ATTACK_NORMAL][WP_RPG7_LAUNCHER]=1;
			//g_entities[level.lastalive[0]].client->ps.firemode[WP_RPG7_LAUNCHER] = BG_FindFireMode ( WP_RPG7_LAUNCHER, ATTACK_NORMAL, WP_FIREMODE_AUTO );
			g_entities[level.sortedClients[i]].client->ps.weapon = WP_AK74_ASSAULT_RIFLE;
			g_entities[level.sortedClients[i]].client->ps.weaponstate = WEAPON_READY;
			g_entities[level.sortedClients[i]].client->ps.weaponTime = 0;
			g_entities[level.sortedClients[i]].client->ps.weaponAnimTime = 0;
			g_entities[level.sortedClients[i]].client->ps.stats[STAT_FROZEN] = 10000;
		}else if(g_entities[level.sortedClients[i]].client->sess.team == TEAM_BLUE){
			G_Damage (&g_entities[level.sortedClients[i]], NULL, NULL, NULL, NULL, 10000, 0, MOD_POP, HL_HEAD|HL_FOOT_RT|HL_FOOT_LT|HL_LEG_UPPER_RT|HL_LEG_UPPER_LT|HL_HAND_RT|HL_HAND_LT|HL_WAIST|HL_CHEST|HL_NECK);
		}
	}
	// when it ends execute this:
	//Com_Printf("Updating scores..\n");
	//UpdateScores();
	//level.cagefight = qfalse;
	//LogExit( "Timelimit hit." );
}

void CheckEnts(gentity_t *ent){
	if(ent->model && ent->model != NULL && !strcmp(ent->model, "BLOCKED_TRIGGER"))
		{
			if(ent->count){
				///Team Games
				if(level.gametypeData->teams){
					if(	ent->count <= (TeamCount( -1, TEAM_RED, NULL )) && ent->count <= (TeamCount( -1, TEAM_BLUE, NULL ))){
						if (ent->r.linked)	{
							trap_UnlinkEntity( ent );
							if(ent->message != NULL)
								trap_SendServerCommand(-1, va("cp \"%s\n\"", ent->message));
						}
					}
					else if(!ent->r.linked)	{
						trap_LinkEntity( ent );
						if(ent->message2 != NULL)
							trap_SendServerCommand(-1, va("cp \"%s\n\"", ent->message2));
					}
				}
				///Non-Team Games
				else if(ent->count >= level.numPlayingClients){
					if (ent->r.linked){
						trap_UnlinkEntity( ent );
						if(ent->message != NULL)
							trap_SendServerCommand(-1, va("cp \"%s\n\"", ent->message));
					}
				}
				else if(!ent->r.linked)	{
					trap_LinkEntity( ent );
					if(ent->message2 != NULL)
						trap_SendServerCommand(-1, va("cp \"%s\n\"", ent->message2));
				}
			}
			return;
		}
	if(level.numPlayingClients < ent->min_players && ent->min_players != 0){
		if(ent->r.linked){
			trap_UnlinkEntity(ent);
			g_entities[ent->effect_index].disabled = qtrue;
		}
	}else if(level.numPlayingClients >= ent->min_players && ent->min_players != 0){
		if(level.numPlayingClients <= ent->max_players){
			if(!ent->r.linked){
				trap_LinkEntity(ent);
				g_entities[ent->effect_index].disabled = qfalse;
			}
		}
	}
	if(level.numPlayingClients < ent->max_players && ent->max_players != 0){
		if(level.numPlayingClients > ent->min_players){
			if(!ent->r.linked){
				trap_LinkEntity(ent);
				g_entities[ent->effect_index].disabled = qfalse;
			}
		}
	}else if(level.numPlayingClients >= ent->max_players && ent->max_players != 0){
		if(ent->r.linked){
			trap_UnlinkEntity(ent);
			g_entities[ent->effect_index].disabled = qtrue;
		}
	}
}

qboolean IsClientMuted(gentity_t *ent, qboolean message){
	int i, remain, remainS;
	for(i=0;i<=20;i++){
		if(level.mutedClients[i].used == qtrue){
			if(strstr(level.mutedClients[i].ip, ent->client->pers.ip)){
				if(level.time < (level.mutedClients[i].startTime+((level.mutedClients[i].time*60)*1000))){
					remain = ((level.mutedClients[i].startTime+((level.mutedClients[i].time*60)*1000)-level.time)/1000)/60;
					remainS = ((level.mutedClients[i].startTime+((level.mutedClients[i].time*60)*1000)-level.time)/1000);
					if(message)
					trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You were muted for %i minutes, %i:%02i minutes remaining.\n\"", level.mutedClients[i].time, remain, remainS-(remain*60)) );
					ent->client->sess.mute = qtrue;
					return qtrue;
				}else{
					trap_SendServerCommand(-1, va("print \"^3[Auto action] ^7%s has been unmuted.\n\"", ent->client->pers.netname) );
					level.mutedClients[i].used = qfalse;
					level.mutedClients[i].time = 0;
					memset(level.mutedClients[i].ip, 0, sizeof(level.mutedClients[i]));
					ent->client->sess.mute = qfalse;
				}
			}
		}
	}
	return qfalse;
}

char	*ConcatArgs1( int start ) {
	int		i, c, tlen;
	char	line[MAX_STRING_CHARS];
	int		len;
	char	arg[MAX_STRING_CHARS];

	len = 0;
	c = trap_Argc();
	for ( i = start ; i < c ; i++ ) {
		trap_Argv( i, arg, sizeof( arg ) );
		tlen = strlen( arg );
		if ( len + tlen >= MAX_STRING_CHARS - 1 ) {
			break;
		}
		memcpy( line + len, arg, tlen );
		len += tlen;
		if ( i != c - 1 ) {
			line[len] = ' ';
			len++;
		}
	}

	line[len] = 0;

	return line;
}

qboolean AddMutedClient(gentity_t *ent, int time){
	int i;
	for(i=0;i<=20;i++){
		if(level.mutedClients[i].used == qfalse){
			strcpy(level.mutedClients[i].ip, ent->client->pers.ip);
			level.mutedClients[i].used = qtrue;
			level.mutedClients[i].time = time;
			level.mutedClients[i].startTime = level.time;
			return qtrue;
		}
	}
	return qfalse;
}

qboolean RemoveMutedClient(gentity_t *ent){
	int i;
	for(i=0;i<=20;i++){
		if(level.mutedClients[i].used == qtrue){
			if(strstr(level.mutedClients[i].ip, ent->client->pers.ip)){
				level.mutedClients[i].used = qfalse;
				level.mutedClients[i].time = 0;
				memset(level.mutedClients[i].ip, 0, sizeof(level.mutedClients[i]));
				ent->client->sess.mute = qfalse;
				return qtrue;
			}
		}
	}
	return qfalse;
}


char *GetReason(void) {
	char	arg[256] = "\0"; // increase buffer so we can process more commands
	char	*reason = "";
	int i, z;
	qboolean Do = qfalse;
	trap_Argv( 1, arg, sizeof( arg ) );
	for(i=0;i<256;i++){
		if(arg[i] == ' '){
			if(Do == qtrue){ // second space so its the reason
				for(z=i+1;z<256-i;z++){
					if(arg[z] == ' '){
						arg[z] = '_';
					}
					reason = va("%s%c", reason, arg[z]);
				}
				if(strlen(reason) < 1){
					strcpy(arg ,ConcatArgs1(3));
					//trap_Argv( 3, arg, sizeof( arg ) );
					return arg;
				}
				return reason;
			}
		Do = qtrue;
		}
	}
	if(strlen(reason) < 1){
		strcpy(arg ,ConcatArgs1(3));
		//trap_Argv( 3, arg, sizeof( arg ) );
		return arg;
	}
	return "";
}

// Boe!Man 1/10/10
// This function is called alot, better optimize this -.-''
int CheckAdmin(gentity_t *ent, char *buffer, qboolean otheradmins) {
char		*numb;
int			id;
int			i;
for(i=0;i<=g_maxclients.integer;i++){
	numb = va("%i", i);
	if(strstr(buffer, numb)){
		id = atoi(numb);
		if(i>=10){
			break;
		}
	}
}
	memset(&numb, 0, sizeof(numb)); // clean buffer
	if(id >= 0 && id <= g_maxclients.integer){
		if(g_entities[id].client->sess.admin && otheradmins == qfalse){
			trap_SendServerCommand(ent->s.number, va("print\"^3[Info] ^7You cannot use this command on other Admins.\n\""));
			return -1;
		}
			else{
				if ( g_entities[id].client->pers.connected != CON_DISCONNECTED )
				{
					if(ent && ent->client)
					{
						return id;
					}else{
						return -1;
					}
				}else{
					return -1;
				}
			}
	}else{
	return -1;
	}
}

int StartAfterCommand(char *param){
	int i;
	for(i=0;i<=strlen(param);i++){
		if(param[i] == ' '){
			return i+1;
		}
	}
	return 0;
}

/*
====================
IP2Country by Henkie
4/1/10 - 12:04 AM
====================
*/

void AddIPList(char ip1[24], char country[128], char ext[6])
{
	int				len;
	fileHandle_t	f;
	char			string[1024];
	char	octet[4][4], octetx[4][4];
	int		i, z, countx[4], loops = 0, count = 0;
	char *IP;
	//G_LogPrintf("Checking ip..\n");
	IP = va("%s", ip1);
	//Com_Printf("IP is: %s\n", ent->client->pers.ip);
	// Set countx to zero, when you do not set the variable you get weird ass results.
	countx[0] = 0;
	countx[1] = 0;
	countx[2] = 0;
	countx[3] = 0;
	// End
	while(*IP){
		if(*IP == '.')
		{
			for(i=0;i<count;i++){
				if(loops == 0){
				octet[0][i] = *--IP;
				countx[0] += 1;
				}else if(loops == 1){
				octet[1][i] = *--IP;
				countx[1] += 1;
				}else if(loops == 2){
				octet[2][i] = *--IP;
				countx[2] += 1;
				}
			}
			for(i=0;i<count;i++){
				*++IP; // ignore this error, compiler says it does not have an effect
			}
			loops += 1;
			count = 0;
		}else{
		count += 1;
		}
		IP++;
		if(*IP == '\0'){
			for(i=0;i<3;i++){
				if(*--IP != '.'){
				octet[3][i] = *IP;
				countx[3] += 1;
				}
			}
			break;
		}
	}
	for(i=0;i<=3;i++){ // 4 octets
		for(z=0;z<countx[i];z++){
		octetx[i][z] = octet[i][countx[i]-(z+1)];
		}
		octetx[i][countx[i]] = '\0';
	}
	Com_Printf("Adding ip to list...\n");
	len = trap_FS_FOpenFile( va("country\\known\\IP.%s", octetx[0]), &f, FS_APPEND_TEXT );
	if (!f)
	{
		Com_Printf("^1Error opening File\n");
		return;
	}
	strcpy(string, va("1\n{\nip \"%s\"\ncountry \"%s\"\next \"%s\"\n}\n", ip1, country, ext));

	trap_FS_Write(string, strlen(string), f);
	trap_FS_Write("\n", 1, f);
	trap_FS_FCloseFile(f);

}

qboolean CheckIP(gentity_t *ent){ // Henk Update 12/05/10 -> Lag spike when file reaches 200kb, fix by splitting files
	void	*GP2, *group;
	char	*filePtr, *file, Files[1024];
	char	ip1[24], country[128], ext[6];
	int		fileCount;
	char	octet[4][4], octetx[4][4];
	int		i, z, countx[4], loops = 0, count = 0;
	char *IP;

	IP = va("%s", ent->client->pers.ip);
	countx[0] = 0;
	countx[1] = 0;
	countx[2] = 0;
	countx[3] = 0;
	// End
	while(*IP){
		if(*IP == '.')
		{
			for(i=0;i<count;i++){
				if(loops == 0){
				octet[0][i] = *--IP;
				countx[0] += 1;
				}else if(loops == 1){
				octet[1][i] = *--IP;
				countx[1] += 1;
				}else if(loops == 2){
				octet[2][i] = *--IP;
				countx[2] += 1;
				}
			}
			for(i=0;i<count;i++){
				*++IP; // ignore this error, compiler says it does not have an effect
			}
			loops += 1;
			count = 0;
		}else{
		count += 1;
		}
		IP++;
		if(*IP == '\0'){
			for(i=0;i<3;i++){
				if(*--IP != '.'){
				octet[3][i] = *IP;
				countx[3] += 1;
				}
			}
			break;
		}
	}
	for(i=0;i<=3;i++){ // 4 octets
		for(z=0;z<countx[i];z++){
		octetx[i][z] = octet[i][countx[i]-(z+1)];
		}
		octetx[i][countx[i]] = '\0';
	}
	fileCount = trap_FS_GetFileList( "country\\", va("IP.%s.%s", octetx[0], octetx[1]), Files, 1024 );
	filePtr = Files;
	file = va("country\\%s", filePtr);
	GP2 = trap_GP_ParseFile(file, qtrue, qfalse);
	if (!GP2)
	{
		// Boe!Man 11/12/10: Ignore file warnings!
		//G_LogPrintf("Error in file: \"%s\" or file not found.\n", file);
		return qfalse;
	}
		group = trap_GPG_GetSubGroups(GP2);

		while(group)
		{
			trap_GPG_FindPairValue(group, "ip", "0", ip1);
			if(strstr(ip1, ent->client->pers.ip)){
				trap_GPG_FindPairValue(group, "country", "", country);
				trap_GPG_FindPairValue(group, "ext", "", ext);
				trap_GP_Delete(&GP2);
				strcpy(ent->client->sess.country, country);
				strcpy(ent->client->sess.countryext, ext);
				//Com_Printf("Found in known list\n");
				return qtrue;
				break; // stop searching
			}
			group = trap_GPG_GetNext(group);
		}
		strcpy(ent->client->sess.countryext, "??");
		trap_GP_Delete(&GP2);
		//Com_Printf("Not found in known list\n");
		return qfalse;
}

unsigned int henk_atoi( const char *string ) {
	unsigned int		sign;
	unsigned int		value;
	unsigned int		c;

	while ( *string <= ' ' ) {
		if ( !*string ) {
			return 0;
		}
		string++;
	}

	switch ( *string ) {
	case '+':
		string++;
		sign = 1;
		break;
	case '-':
		string++;
		sign = -1;
		break;
	default:
		sign = 1;
		break;
	}

	value = 0;
	do {
		c = *string++;
		if ( c < '0' || c > '9' ) {
			break;
		}
		c -= '0';
		value = value * 10 + c;
	} while ( 1 );

	return value * sign;
}

void InitSpawn(int choice) // load bsp models before players loads a map(SOF2 clients cannot load a bsp model INGAME)
{
	AddSpawnField("classname", "misc_bsp"); // blocker
	if(choice == 1){
	AddSpawnField("bspmodel",	"instances/Generic/fence01");
	}else if(choice == 2){
	AddSpawnField("bspmodel",	"instances/Colombia/npc_jump1");
	}else{
	AddSpawnField("bspmodel",	"instances/Kamchatka/wall01");
	}
	AddSpawnField("origin",		"-4841 -4396 5000");
	AddSpawnField("angles",		"0 90 0");
	AddSpawnField("model",		"trigger_hurt"); //blocked_trigger
	AddSpawnField("count",		 "1");

	G_SpawnGEntityFromSpawnVars(qfalse);
}	

void AddToPasswordList(gentity_t *ent, int lvl){
	int len;
	fileHandle_t f;
	char str[512], octet[4];
	char levelstr[32];
	len = trap_FS_FOpenFile(g_adminPassFile.string, &f, FS_APPEND_TEXT);
	if(!f){
		G_LogPrintf("Error while opening %s\n", g_adminPassFile.string);
		return;
	}
	strcpy(octet, va("%c%c%c", ent->client->pers.ip[0], ent->client->pers.ip[1], ent->client->pers.ip[2]));
	strcpy(str, va("%s\\%s:%i\n", ent->client->pers.cleanName, octet, lvl));
	str[strlen(str)] = '\0';
	trap_FS_Write(str, strlen(str), f);
	trap_FS_FCloseFile(f);
	if(lvl == 2)
		strcpy(levelstr, "B-Admin");
	else if(lvl == 3)
		strcpy(levelstr, "Admin");
	else if(lvl == 4)
		strcpy(levelstr, "S-Admin");

	trap_SendServerCommand( -1, va("print \"^3[Info] ^7%s has been added to the %s list.\n\"", ent->client->pers.netname, levelstr));

}

qboolean CheckPasswordList(gentity_t *ent, char *pass){
	fileHandle_t f;
	int len, i, start, end;
	char lvl, passlvl;
	char buf[1024], octet[5], name[64], myoctet[5];

	// sort out what admin lvl pass he entered
	if(!Q_stricmp(pass, g_badminPass.string))
		passlvl = '2';
	else if(!Q_stricmp(pass, g_adminPass.string))
		passlvl = '3';
	else if(!Q_stricmp(pass, g_sadminPass.string))
		passlvl = '4';

	// set our first octet
	strcpy(myoctet, va("%c%c%c", ent->client->pers.ip[0], ent->client->pers.ip[1], ent->client->pers.ip[2]));

	len = trap_FS_FOpenFile(g_adminPassFile.string, &f, FS_READ_TEXT);
	if(!f){
		G_LogPrintf("Error while opening %s\n", g_adminPassFile.string);
		return qfalse;
	}
	trap_FS_Read(buf, len, f);
	for(i=0;i<=len;i++){
		if(i==0)
			start = 0;
		if(buf[i] == '\n'){
			end = i;
			lvl = buf[i-1];
			strcpy(octet, va("%c%c%c", buf[i-5], buf[i-4], buf[i-3]));
			memset(name, 0, sizeof(name));
			strncpy(name, buf+start, (i-6)-start);
			//Com_Printf("Name: %s(%i - %i)\nLevel: %c\nOctet: %s\n\n", name, i, len, lvl, octet);
			if(!Q_stricmp(octet, myoctet) && !Q_stricmp(ent->client->pers.cleanName, name) && lvl == passlvl){ // found octet
				if(lvl == '2'){
					ent->client->sess.admin = 2;
					trap_SendServerCommand( -1, va("print \"^3[Info] ^7%s has been granted B-Admin.\n\"", ent->client->pers.netname));
				}else if(lvl == '3'){
					ent->client->sess.admin = 3;
					trap_SendServerCommand( -1, va("print \"^3[Info] ^7%s has been granted Admin.\n\"", ent->client->pers.netname));
				}else if(lvl == '4'){
					ent->client->sess.admin = 4;
					trap_SendServerCommand( -1, va("print \"^3[Info] ^7%s has been granted S-Admin.\n\"", ent->client->pers.netname));
				}
				trap_FS_FCloseFile(f);
				return qtrue;
			}
			start = i+1;
		}
	}

	trap_FS_FCloseFile(f);
	// this guy has the password but didn't match our file.
	// This could mean:
	// - He changed name
	// - Password was leaked to some idiot who's not on the list.
	// We are going to log this.
	return qfalse;
}

void RemoveFence(void){
	int i;
	for (i = 0; i < MAX_GENTITIES; i++)
	{
		if(g_entities[i].classname != NULL)
		{
			if(!strcmp(g_entities[i].classname, "misc_bsp") && g_entities[i].nolower == 1 )
				G_FreeEntity( &g_entities[i] );
		}
	}
}

qboolean henk_isdigit(char c){ // by henk
	if(c >= 48 && c <= 57){
		return qtrue;
	}else{
		return qfalse;
	}
}


qboolean henk_ischar(char c){
	if(c >= 97 && c <= 122){ // a-z lowercase
		return qtrue;
	}else if(c >= 65 && c <= 90){ // A-Z uppercase
		return qtrue;
	}else{
		return qfalse;
	}
}

char *ChooseTeam(){
	int counts[2], seekers, maxhiders;
	counts[TEAM_BLUE] = TeamCount1(TEAM_BLUE);
	counts[TEAM_RED] = TeamCount1(TEAM_RED);
	if(counts[TEAM_RED] >= 4 && counts[TEAM_RED] <= 8){
		seekers = 2;
	}else if(counts[TEAM_RED] >= 9 && counts[TEAM_RED] <= 13){
		seekers = 3;
	}else if(counts[TEAM_RED] >= 14 && counts[TEAM_RED] <= 18){
		seekers = 4;
	}else if(counts[TEAM_RED] >= 19){
		seekers = 5;
	}else{
		seekers = 1;
	}
	if(counts[TEAM_BLUE] == 2){
		maxhiders = 8;
	}else if(counts[TEAM_BLUE] == 3){
		maxhiders = 13;
	}else if(counts[TEAM_BLUE] == 4){
		maxhiders = 18;
	}else if(counts[TEAM_BLUE] == 5){
		maxhiders = 24;
	}else{
		maxhiders = 6; // Henkie 24/02/10 -> Was 4
	}
	if ( counts[TEAM_BLUE] >= seekers)	{
		return "r";
	}else if(counts[TEAM_RED] >= maxhiders){
		return "b";
	}else{
		return "r";
	}
}

void PrintCustom(int numb){
	void	*GP2, *group;
	char desc[256];
	char level[2];
	char spaces[30]; // 27 - strlen
	char name[10];
	int i;
	GP2 = trap_GP_ParseFile(g_customCommandsFile.string, qtrue, qfalse);
	if(!GP2){
		Com_Printf("Error while loading %s\n", g_customCommandsFile.string);
		return;
	}
	group = trap_GPG_GetSubGroups(GP2);
	while(group)
	{
		trap_GPG_FindPairValue(group, "Command", "none", name);
		for(i=0;i<27-strlen(name);i++){
			spaces[i] = ' ';
		}
		spaces[i] = '\0';
		trap_GPG_FindPairValue(group, "Description", "No description defined", desc);
		trap_GPG_FindPairValue(group, "AdminLevel", "5", level);
		trap_SendServerCommand( numb, va("print \"[^3%s^7]       %s%s^7[^3%s^7]\n\"", level, name, spaces, desc));
		group = trap_GPG_GetNext(group);
	}
	trap_GP_Delete(&GP2);
	trap_SendServerCommand( numb, va("print \"\n\""));
}

void SpawnFence(int choice) // big cage
{
	int result;
	AddSpawnField("classname", "misc_bsp"); // blocker
	if (choice == 1){
	AddSpawnField("bspmodel",	"instances/Generic/fence01");
	AddSpawnField("origin",		"-346 -309 -275");
	AddSpawnField("angles",		"0 270 0");
	}else if(choice == 2){
	AddSpawnField("bspmodel",	"instances/Generic/fence01");
	AddSpawnField("origin",		"-4073 -710 -275");
	AddSpawnField("angles",		"0 90 0");
	// Boe!Man 5/3/10: The solid walls.
	}else if (choice == 3){
	AddSpawnField("bspmodel",	"instances/Kamchatka/wall01");
	AddSpawnField("origin",		"-1813 -210 -10");
	AddSpawnField("angles",		"0 0 0");
	}else if(choice == 4){
	AddSpawnField("bspmodel",	"instances/Kamchatka/wall01");
	AddSpawnField("origin",		"-2607 -825 -10");
	AddSpawnField("angles",		"0 180 0");
	}
	AddSpawnField("model",		"trigger_hurt"); //blocked_trigger
	AddSpawnField("count",		 "1");
	AddSpawnField("nolower",	"1");

	result = G_SpawnGEntityFromSpawnVars(qtrue);
	//Com_Printf("Spawning %i result: %i\n", choice, result);

	level.numSpawnVars = 0;
	level.numSpawnVarChars = 0;
}

int FormatDamage(int damage){
	char test[10];
	char *test1;
	strcpy(test, va("%i", damage));
	if(damage >= 100 && damage < 1000){
	test1 = va("%c00", test[0]);
	damage = atoi(test1);
	}else if(damage >= 1000 && damage < 10000){
	test1 = va("%c%c00", test[0], test[1]);
	damage = atoi(test1);
	}else if(damage >= 10000 && damage < 100000){
	test1 = va("%c%c%c00", test[0], test[1], test[2]);
	damage = atoi(test1);
	}
return damage/10;
}

void HENK_COUNTRY(gentity_t *ent){
	void	*GP2, *group;
	char	*filePtr, *file, Files[1024];
	int		fileCount;
	char	*IP;
	int		count = 0;
	int		i, z, countx[4], loops = 0;
	char	begin_ip[24], end_ip[24], ext[6];
	unsigned int		begin_ipi, end_ipi;

	char	octet[4][4], octetx[4][4];
	int		RealOctet[4];
	unsigned int		IPnum, part;
	//Com_sprintf(IP, 24, ent->client->pers.ip);
	IP = va("%s", ent->client->pers.ip);
	//Com_Printf("IP is: %s\n", ent->client->pers.ip);
	// Set countx to zero, when you do not set the variable you get weird ass results.
	countx[0] = 0;
	countx[1] = 0;
	countx[2] = 0;
	countx[3] = 0;
	// End
	while(*IP){
		if(*IP == '.')
		{
			for(i=0;i<count;i++){
				if(loops == 0){
				octet[0][i] = *--IP;
				countx[0] += 1;
				}else if(loops == 1){
				octet[1][i] = *--IP;
				countx[1] += 1;
				}else if(loops == 2){
				octet[2][i] = *--IP;
				countx[2] += 1;
				}
			}
			for(i=0;i<count;i++){
				*++IP; // ignore this error, compiler says it does not have an effect
			}
			loops += 1;
			count = 0;
		}else{
		count += 1;
		}
		IP++;
		if(*IP == '\0'){
			for(i=0;i<3;i++){
				if(*--IP != '.'){
				octet[3][i] = *IP;
				countx[3] += 1;
				}
			}
			break;
		}
	}
	for(i=0;i<=3;i++){ // 4 octets
		for(z=0;z<countx[i];z++){
		octetx[i][z] = octet[i][countx[i]-(z+1)];
		}
		octetx[i][countx[i]] = '\0';
	}

	RealOctet[0] = atoi(octetx[0]);
	RealOctet[1] = atoi(octetx[1]);
	RealOctet[2] = atoi(octetx[2]);
	RealOctet[3] = atoi(octetx[3]);

	IPnum = (RealOctet[0] * 16777216) + (RealOctet[1] * 65536) + (RealOctet[2] * 256) + (RealOctet[3]);
if(IPnum >= 16777216 && IPnum <= 460602367){
part = 0;
}else if(IPnum >= 460602368 && IPnum <= 772902911){
part = 1;
}else if(IPnum >= 772902912 && IPnum <= 781320191){
part = 2;
}else if(IPnum >= 781320192 && IPnum <= 1024329343){
part = 3;
}else if(IPnum >= 1024329344 && IPnum <= 1041737015){
part = 4;
}else if(IPnum >= 1041737016 && IPnum <= 1044020359){
part = 5;
}else if(IPnum >= 1044020360 && IPnum <= 1046497671){
part = 6;
}else if(IPnum >= 1046497672 && IPnum <= 1048882151){
part = 7;
}else if(IPnum >= 1048882152 && IPnum <= 1052016447){
part = 8;
}else if(IPnum >= 1052016448 && IPnum <= 1053297151){
part = 9;
}else if(IPnum >= 1053297152 && IPnum <= 1067625983){
part = 10;
}else if(IPnum >= 1067625984 && IPnum <= 1076183071){
part = 11;
}else if(IPnum >= 1076183072 && IPnum <= 1077936433){
part = 12;
}else if(IPnum >= 1077936434 && IPnum <= 1081385215){
part = 13;
}else if(IPnum >= 1081385216 && IPnum <= 1091802871){
part = 14;
}else if(IPnum >= 1091802872 && IPnum <= 1101041407){
part = 15;
}else if(IPnum >= 1101041408 && IPnum <= 1115698047){
part = 16;
}else if(IPnum >= 1115698048 && IPnum <= 1119166855){
part = 17;
}else if(IPnum >= 1119166856 && IPnum <= 1122126249){
part = 18;
}else if(IPnum >= 1122126250 && IPnum <= 1136689743){
part = 19;
}else if(IPnum >= 1136689744 && IPnum <= 1138661417){
part = 20;
}else if(IPnum >= 1138661418 && IPnum <= 1159522071){
part = 21;
}else if(IPnum >= 1159522072 && IPnum <= 1163396199){
part = 22;
}else if(IPnum >= 1163396200 && IPnum <= 1177164415){
part = 23;
}else if(IPnum >= 1177164416 && IPnum <= 1210061327){
part = 24;
}else if(IPnum >= 1210061328 && IPnum <= 1249027503){
part = 25;
}else if(IPnum >= 1249027504 && IPnum <= 1275875367){
part = 26;
}else if(IPnum >= 1275875368 && IPnum <= 1296746495){
part = 27;
}else if(IPnum >= 1296746496 && IPnum <= 1315504127){
part = 28;
}else if(IPnum >= 1315504128 && IPnum <= 1317835007){
part = 29;
}else if(IPnum >= 1317835008 && IPnum <= 1347239935){
part = 30;
}else if(IPnum >= 1347239936 && IPnum <= 1353950399){
part = 31;
}else if(IPnum >= 1353950400 && IPnum <= 1358899471){
part = 32;
}else if(IPnum >= 1358899472 && IPnum <= 1364586495){
part = 33;
}else if(IPnum >= 1364586496 && IPnum <= 1383226095){
part = 34;
}else if(IPnum >= 1383226096 && IPnum <= 1389266559){
part = 35;
}else if(IPnum >= 1389266560 && IPnum <= 1406751967){
part = 36;
}else if(IPnum >= 1406751968 && IPnum <= 1408270335){
part = 37;
}else if(IPnum >= 1408270336 && IPnum <= 1424618495){
part = 38;
}else if(IPnum >= 1424618496 && IPnum <= 1433890815){
part = 39;
}else if(IPnum >= 1433890816 && IPnum <= 1446907371){
part = 40;
}else if(IPnum >= 1446907372 && IPnum <= 1475131391){
part = 41;
}else if(IPnum >= 1475131392 && IPnum <= 1500094463){
part = 42;
}else if(IPnum >= 1500094464 && IPnum <= 1508639231){
part = 43;
}else if(IPnum >= 1508639232 && IPnum <= 1534990335){
part = 44;
}else if(IPnum >= 1534990336 && IPnum <= 1539754495){
part = 45;
}else if(IPnum >= 1539754496 && IPnum <= 1540405759){
part = 46;
}else if(IPnum >= 1540405760 && IPnum <= 1540897279){
part = 47;
}else if(IPnum >= 1540897280 && IPnum <= 1541375487){
part = 48;
}else if(IPnum >= 1541375488 && IPnum <= 1570660543){
part = 49;
}else if(IPnum >= 1570660544 && IPnum <= 1578612903){
part = 50;
}else if(IPnum >= 1578612904 && IPnum <= 1602316287){
part = 51;
}else if(IPnum >= 1602316288 && IPnum <= 1652657447){
part = 52;
}else if(IPnum >= 1652657448 && IPnum <= 1842047371){
part = 53;
}else if(IPnum >= 1842047372 && IPnum <= 1925642239){
part = 54;
}else if(IPnum >= 1925642240 && IPnum <= 2036703231){
part = 55;
}else if(IPnum >= 2036703232 && IPnum <= 2190868479){
part = 56;
}else if(IPnum >= 2190868480 && IPnum <= 2359885823){
part = 57;
}else if(IPnum >= 2359885824 && IPnum <= 2604138495){
part = 58;
}else if(IPnum >= 2604138496 && IPnum <= 2780561407){
part = 59;
}else if(IPnum >= 2780561408 && IPnum <= 2918534943){
part = 60;
}else if(IPnum >= 2918534944 && IPnum <= 2928635127){
part = 61;
}else if(IPnum >= 2928635128 && IPnum <= 2988463679){
part = 62;
}else if(IPnum >= 2988463680 && IPnum <= 2988524623){
part = 63;
}else if(IPnum >= 2988524624 && IPnum <= 2991128575){
part = 64;
}else if(IPnum >= 2991128576 && IPnum <= 3029602303){
part = 65;
}else if(IPnum >= 3029604352 && IPnum <= 3161948159){
part = 66;
}else if(IPnum >= 3161948160 && IPnum <= 3168161791){
part = 67;
}else if(IPnum >= 3168161792 && IPnum <= 3224822015){
part = 68;
}else if(IPnum >= 3224822016 && IPnum <= 3228084479){
part = 69;
}else if(IPnum >= 3228084480 && IPnum <= 3231296511){
part = 70;
}else if(IPnum >= 3231296512 && IPnum <= 3237319679){
part = 71;
}else if(IPnum >= 3237319680 && IPnum <= 3240215551){
part = 72;
}else if(IPnum >= 3240215552 && IPnum <= 3244855295){
part = 73;
}else if(IPnum >= 3244855296 && IPnum <= 3247069951){
part = 74;
}else if(IPnum >= 3247069952 && IPnum <= 3250601983){
part = 75;
}else if(IPnum >= 3250601984 && IPnum <= 3252452503){
part = 76;
}else if(IPnum >= 3252452504 && IPnum <= 3255246623){
part = 77;
}else if(IPnum >= 3255246624 && IPnum <= 3257757831){
part = 78;
}else if(IPnum >= 3257757832 && IPnum <= 3259319407){
part = 79;
}else if(IPnum >= 3259319408 && IPnum <= 3262474004){
part = 80;
}else if(IPnum >= 3262474005 && IPnum <= 3262478482){
part = 81;
}else if(IPnum >= 3262478483 && IPnum <= 3262479912){
part = 82;
}else if(IPnum >= 3262479913 && IPnum <= 3264668159){
part = 83;
}else if(IPnum >= 3264668160 && IPnum <= 3267745615){
part = 84;
}else if(IPnum >= 3267745616 && IPnum <= 3271398047){
part = 85;
}else if(IPnum >= 3271398048 && IPnum <= 3272976991){
part = 86;
}else if(IPnum >= 3272976992 && IPnum <= 3274902423){
part = 87;
}else if(IPnum >= 3274902424 && IPnum <= 3276029583){
part = 88;
}else if(IPnum >= 3276029584 && IPnum <= 3276362079){
part = 89;
}else if(IPnum >= 3276362080 && IPnum <= 3276395047){
part = 90;
}else if(IPnum >= 3276395048 && IPnum <= 3276744191){
part = 91;
}else if(IPnum >= 3276744192 && IPnum <= 3278786943){
part = 92;
}else if(IPnum >= 3278786944 && IPnum <= 3278942953){
part = 93;
}else if(IPnum >= 3278942954 && IPnum <= 3278944222){
part = 94;
}else if(IPnum >= 3278944223 && IPnum <= 3280590847){
part = 95;
}else if(IPnum >= 3280590848 && IPnum <= 3280979975){
part = 96;
}else if(IPnum >= 3280979976 && IPnum <= 3283493119){
part = 97;
}else if(IPnum >= 3283493120 && IPnum <= 3285331967){
part = 98;
}else if(IPnum >= 3285331968 && IPnum <= 3285548959){
part = 99;
}else if(IPnum >= 3285548960 && IPnum <= 3287218175){
part = 100;
}else if(IPnum >= 3287218176 && IPnum <= 3323068415){
part = 101;
}else if(IPnum >= 3323201536 && IPnum <= 3349446911){
part = 102;
}else if(IPnum >= 3349446912 && IPnum <= 3358766591){
part = 103;
}else if(IPnum >= 3358766592 && IPnum <= 3389979647){
part = 104;
}else if(IPnum >= 3389979648 && IPnum <= 3395133439){
part = 105;
}else if(IPnum >= 3395133440 && IPnum <= 3400437663){
part = 106;
}else if(IPnum >= 3400437664 && IPnum <= 3412803583){
part = 107;
}else if(IPnum >= 3412819968 && IPnum <= 3418511439){
part = 108;
}else if(IPnum >= 3418511440 && IPnum <= 3427770623){
part = 109;
}else if(IPnum >= 3427770624 && IPnum <= 3448382351){
part = 110;
}else if(IPnum >= 3448382352 && IPnum <= 3459277823){
part = 111;
}else if(IPnum >= 3459277824 && IPnum <= 3468048127){
part = 112;
}else if(IPnum >= 3468048128 && IPnum <= 3470646807){
part = 113;
}else if(IPnum >= 3470646808 && IPnum <= 3484777695){
part = 114;
}else if(IPnum >= 3484777696 && IPnum <= 3494764343){
part = 115;
}else if(IPnum >= 3494764344 && IPnum <= 3506198007){
part = 116;
}else if(IPnum >= 3506198008 && IPnum <= 3510469831){
part = 117;
}else if(IPnum >= 3510469832 && IPnum <= 3513368575){
part = 118;
}else if(IPnum >= 3513368576 && IPnum <= 3520034791){
part = 119;
}else if(IPnum >= 3520034792 && IPnum <= 3557364607){
part = 120;
}else if(IPnum >= 3557364608 && IPnum <= 3559491455){
part = 121;
}else if(IPnum >= 3559491456 && IPnum <= 3560943007){
part = 122;
}else if(IPnum >= 3560943008 && IPnum <= 3560945867){
part = 123;
}else if(IPnum >= 3560945868 && IPnum <= 3562856447){
part = 124;
}else if(IPnum >= 3562856448 && IPnum <= 3564893759){
part = 125;
}else if(IPnum >= 3564893760 && IPnum <= 3568304127){
part = 126;
}else if(IPnum >= 3568304128 && IPnum <= 3572580351){
part = 127;
}else if(IPnum >= 3572580352 && IPnum <= 3576103679){
part = 128;
}else if(IPnum >= 3576103680 && IPnum <= 3579486831){
part = 129;
}else if(IPnum >= 3579486832 && IPnum <= 3583707775){
part = 130;
}else if(IPnum >= 3583707776 && IPnum <= 3586906735){
part = 131;
}else if(IPnum >= 3586906736 && IPnum <= 3590033759){
part = 132;
}else if(IPnum >= 3590033760 && IPnum <= 3625243215){
part = 133;
}else if(IPnum >= 3625243216 && IPnum <= 3628772095){
part = 134;
}else if(IPnum >= 3628772096 && IPnum <= 3632489127){
part = 135;
}else if(IPnum >= 3632489128 && IPnum <= 3638530047){
part = 136;
}else if(IPnum >= 3638530048 && IPnum <= 3642554675){
part = 137;
}else if(IPnum >= 3642554676 && IPnum <= 3645764300){
part = 138;
}else if(IPnum >= 3645764301 && IPnum <= 3650433023){
part = 139;
}else if(IPnum >= 3650433024 && IPnum <= 3671326719){
part = 140;
}else{
part = 141;
}
	fileCount = trap_FS_GetFileList( "country", va(".%i", part), Files, 1024 );
	filePtr = Files;
	file = va("country\\%s", filePtr);
	GP2 = trap_GP_ParseFile(file, qtrue, qfalse);
	if(GP2){ // henk check if group is correct
		group = trap_GPG_GetSubGroups(GP2);
		while(group)
		{
			trap_GPG_FindPairValue(group, "l", "0", begin_ip);
			begin_ipi = henk_atoi(begin_ip);
			trap_GPG_FindPairValue(group, "h", "", end_ip);
			end_ipi = henk_atoi(end_ip);
			//trap_SendServerCommand( -1, va("print \"^3[Debug]^7 %i with %i\n\"", IPnum, begin_ipi) );
			if(IPnum >= begin_ipi && IPnum <= end_ipi){
				//trap_GPG_FindPairValue(group, "c", "", country);
				trap_GPG_FindPairValue(group, "e", "??", ext); // add ?? as default variable
				trap_GP_Delete(&GP2);
				strcpy(ent->client->sess.country, GetCountry(ext));
				strcpy(ent->client->sess.countryext, ext);
				return;
				break; // stop searching
			}
			group = trap_GPG_GetNext(group);
		}
	}else{
		//trap_SendServerCommand( -1, va("print \"^3[Debug]^7 File not found\n\"") );
		G_LogPrintf("IP2Country Debug: File not found\n"); // Boe 12/23/10: Append to console & log to file rather than sending debug information to all clients.
		
	}
		// End other file
		strcpy(ent->client->sess.countryext, "??");
		trap_GP_Delete(&GP2);
		return;
}

char *GetCountry(char *ext){
	char result[128];
if(strstr(ext, "AU"))
strcpy(result, "Australia");
else if(strstr(ext, "MY"))
strcpy(result, "Malaysia");
else if(strstr(ext, "KR"))
strcpy(result, "Kore");
else if(strstr(ext, "CN"))
strcpy(result, "China");
else if(strstr(ext, "JP"))
strcpy(result, "Japan");
else if(strstr(ext, "IN"))
strcpy(result, "India");
else if(strstr(ext, "TW"))
strcpy(result, "Taiwan");
else if(strstr(ext, "HK"))
strcpy(result, "Hong Kong");
else if(strstr(ext, "TH"))
strcpy(result, "Thailand");
else if(strstr(ext, "VN"))
strcpy(result, "Vietnam");
else if(strstr(ext, "FR"))
strcpy(result, "France");
else if(strstr(ext, "EU"))
strcpy(result, "Europe");
else if(strstr(ext, "SE"))
strcpy(result, "Sweden");
else if(strstr(ext, "IT"))
strcpy(result, "Italy");
else if(strstr(ext, "DE"))
strcpy(result, "Germany");
else if(strstr(ext, "GB"))
strcpy(result, "United Kingdom");
else if(strstr(ext, "AE"))
strcpy(result, "United Arab Emirates");
else if(strstr(ext, "IL"))
strcpy(result, "Israel");
else if(strstr(ext, "UA"))
strcpy(result, "Ukraine");
else if(strstr(ext, "CZ"))
strcpy(result, "Czech Republic");
else if(strstr(ext, "RU"))
strcpy(result, "Russian Federation");
else if(strstr(ext, "KZ"))
strcpy(result, "Kazakhstan");
else if(strstr(ext, "PT"))
strcpy(result, "Portugal");
else if(strstr(ext, "GR"))
strcpy(result, "Greece");
else if(strstr(ext, "SA"))
strcpy(result, "Saudi Arabia");
else if(strstr(ext, "DK"))
strcpy(result, "Denmark");
else if(strstr(ext, "ES"))
strcpy(result, "Spain");
else if(strstr(ext, "IR"))
strcpy(result, "Ira");
else if(strstr(ext, "NO"))
strcpy(result, "Norway");
else if(strstr(ext, "US"))
strcpy(result, "United States");
else if(strstr(ext, "CA"))
strcpy(result, "Canada");
else if(strstr(ext, "MX"))
strcpy(result, "Mexico");
else if(strstr(ext, "BM"))
strcpy(result, "Bermuda");
else if(strstr(ext, "PR"))
strcpy(result, "Puerto Rico");
else if(strstr(ext, "VI"))
strcpy(result, "Virgin Island");
else if(strstr(ext, "BO"))
strcpy(result, "Bolivia");
else if(strstr(ext, "NL"))
strcpy(result, "Netherlands");
else if(strstr(ext, "NZ"))
strcpy(result, "New Zealand");
else if(strstr(ext, "SG"))
strcpy(result, "Singapore");
else if(strstr(ext, "ID"))
strcpy(result, "Indonesia");
else if(strstr(ext, "NP"))
strcpy(result, "Nepal");
else if(strstr(ext, "PH"))
strcpy(result, "Philippines");
else if(strstr(ext, "PG"))
strcpy(result, "Papua New Guinea");
else if(strstr(ext, "PK"))
strcpy(result, "Pakistan");
else if(strstr(ext, "BS"))
strcpy(result, "Bahamas");
else if(strstr(ext, "VC"))
strcpy(result, "Saint Vincent and the Grenadines");
else if(strstr(ext, "CL"))
strcpy(result, "Chile");
else if(strstr(ext, "NC"))
strcpy(result, "New Caledonia");
else if(strstr(ext, "AR"))
strcpy(result, "Argentina");
else if(strstr(ext, "DM"))
strcpy(result, "Dominica");
else if(strstr(ext, "TK"))
strcpy(result, "Tokelau");
else if(strstr(ext, "KH"))
strcpy(result, "Cambodia");
else if(strstr(ext, "MO"))
strcpy(result, "Macau");
else if(strstr(ext, "MV"))
strcpy(result, "Maldives");
else if(strstr(ext, "AF"))
strcpy(result, "Afghanistan");
else if(strstr(ext, "FJ"))
strcpy(result, "Fiji");
else if(strstr(ext, "MN"))
strcpy(result, "Mongolia");
else if(strstr(ext, "WF"))
strcpy(result, "Wallis and Futuna");
else if(strstr(ext, "BD"))
strcpy(result, "Bangladesh");
else if(strstr(ext, "IE"))
strcpy(result, "Ireland");
else if(strstr(ext, "AN"))
strcpy(result, "Netherlands Antilles");
else if(strstr(ext, "BE"))
strcpy(result, "Belgium");
else if(strstr(ext, "BZ"))
strcpy(result, "Belize");
else if(strstr(ext, "BR"))
strcpy(result, "Brazil");
else if(strstr(ext, "CH"))
strcpy(result, "Switzerland");
else if(strstr(ext, "ZA"))
strcpy(result, "South Africa");
else if(strstr(ext, "EG"))
strcpy(result, "Egypt");
else if(strstr(ext, "NG"))
strcpy(result, "Nigeria");
else if(strstr(ext, "TZ"))
strcpy(result, "Tanzani");
else if(strstr(ext, "ZM"))
strcpy(result, "Zambia");
else if(strstr(ext, "SN"))
strcpy(result, "Senegal");
else if(strstr(ext, "MG"))
strcpy(result, "Madagascar");
else if(strstr(ext, "AO"))
strcpy(result, "Angola");
else if(strstr(ext, "NA"))
strcpy(result, "Namibia");
else if(strstr(ext, "CI"))
strcpy(result, "Cote D'Ivoire");
else if(strstr(ext, "GH"))
strcpy(result, "Ghana");
else if(strstr(ext, "SD"))
strcpy(result, "Sudan");
else if(strstr(ext, "CM"))
strcpy(result, "Cameroon");
else if(strstr(ext, "MW"))
strcpy(result, "Malawi");
else if(strstr(ext, "KE"))
strcpy(result, "Kenya");
else if(strstr(ext, "GA"))
strcpy(result, "Gabon");
else if(strstr(ext, "ML"))
strcpy(result, "Mali");
else if(strstr(ext, "BJ"))
strcpy(result, "Benin");
else if(strstr(ext, "TD"))
strcpy(result, "Chad");
else if(strstr(ext, "BW"))
strcpy(result, "Botswana");
else if(strstr(ext, "LY"))
strcpy(result, "Libyan Arab Jamahiriya");
else if(strstr(ext, "CV"))
strcpy(result, "Cape Verde");
else if(strstr(ext, "RW"))
strcpy(result, "Rwanda");
else if(strstr(ext, "CG"))
strcpy(result, "Congo");
else if(strstr(ext, "UG"))
strcpy(result, "Uganda");
else if(strstr(ext, "MZ"))
strcpy(result, "Mozambique");
else if(strstr(ext, "GM"))
strcpy(result, "Gambia");
else if(strstr(ext, "LS"))
strcpy(result, "Lesotho");
else if(strstr(ext, "MU"))
strcpy(result, "Mauritius");
else if(strstr(ext, "ZW"))
strcpy(result, "Zimbabwe");
else if(strstr(ext, "BF"))
strcpy(result, "Burkina Faso");
else if(strstr(ext, "SL"))
strcpy(result, "Sierra Leone");
else if(strstr(ext, "SO"))
strcpy(result, "Somalia");
else if(strstr(ext, "CD"))
strcpy(result, "Cong");
else if(strstr(ext, "NE"))
strcpy(result, "Niger");
else if(strstr(ext, "CF"))
strcpy(result, "Central African Republic");
else if(strstr(ext, "SZ"))
strcpy(result, "Swaziland");
else if(strstr(ext, "TG"))
strcpy(result, "Togo");
else if(strstr(ext, "GN"))
strcpy(result, "Guinea");
else if(strstr(ext, "LR"))
strcpy(result, "Liberia");
else if(strstr(ext, "SC"))
strcpy(result, "Seychelles");
else if(strstr(ext, "MA"))
strcpy(result, "Morocco");
else if(strstr(ext, "DZ"))
strcpy(result, "Algeria");
else if(strstr(ext, "MR"))
strcpy(result, "Mauritania");
else if(strstr(ext, "DJ"))
strcpy(result, "Djibouti");
else if(strstr(ext, "A2"))
strcpy(result, "Satellite Provider");
else if(strstr(ext, "KM"))
strcpy(result, "Comoros");
else if(strstr(ext, "RE"))
strcpy(result, "Reunion");
else if(strstr(ext, "GQ"))
strcpy(result, "Equatorial Guinea");
else if(strstr(ext, "TN"))
strcpy(result, "Tunisia");
else if(strstr(ext, "TR"))
strcpy(result, "Turkey");
else if(strstr(ext, "BG"))
strcpy(result, "Bulgaria");
else if(strstr(ext, "MT"))
strcpy(result, "Malta");
else if(strstr(ext, "NI"))
strcpy(result, "Nicaragua");
else if(strstr(ext, "RS"))
strcpy(result, "Serbia");
else if(strstr(ext, "EE"))
strcpy(result, "Estonia");
else if(strstr(ext, "CY"))
strcpy(result, "Cyprus");
else if(strstr(ext, "LB"))
strcpy(result, "Lebanon");
else if(strstr(ext, "PL"))
strcpy(result, "Poland");
else if(strstr(ext, "LU"))
strcpy(result, "Luxembourg");
else if(strstr(ext, "FI"))
strcpy(result, "Finland");
else if(strstr(ext, "AT"))
strcpy(result, "Austria");
else if(strstr(ext, "PS"))
strcpy(result, "Palestinian Territor");
else if(strstr(ext, "AZ"))
strcpy(result, "Azerbaijan");
else if(strstr(ext, "SK"))
strcpy(result, "Slovakia");
else if(strstr(ext, "SI"))
strcpy(result, "Slovenia");
else if(strstr(ext, "AM"))
strcpy(result, "Armenia");
else if(strstr(ext, "RO"))
strcpy(result, "Romania");
else if(strstr(ext, "LV"))
strcpy(result, "Latvia");
else if(strstr(ext, "AL"))
strcpy(result, "Albania");
else if(strstr(ext, "TJ"))
strcpy(result, "Tajikistan");
else if(strstr(ext, "IS"))
strcpy(result, "Iceland");
else if(strstr(ext, "JO"))
strcpy(result, "Jordan");
else if(strstr(ext, "BY"))
strcpy(result, "Belarus");
else if(strstr(ext, "HU"))
strcpy(result, "Hungary");
else if(strstr(ext, "MK"))
strcpy(result, "Macedonia");
else if(strstr(ext, "IQ"))
strcpy(result, "Iraq");
else if(strstr(ext, "ME"))
strcpy(result, "Montenegro");
else if(strstr(ext, "HR"))
strcpy(result, "Croatia");
else if(strstr(ext, "GI"))
strcpy(result, "Gibraltar");
else if(strstr(ext, "YE"))
strcpy(result, "Yemen");
else if(strstr(ext, "BA"))
strcpy(result, "Bosnia and Herzegovina");
else if(strstr(ext, "LT"))
strcpy(result, "Lithuania");
else if(strstr(ext, "VA"))
strcpy(result, "Holy See (Vatican City State)");
else if(strstr(ext, "LI"))
strcpy(result, "Liechtenstein");
else if(strstr(ext, "IM"))
strcpy(result, "Isle of Man");
else if(strstr(ext, "CR"))
strcpy(result, "Costa Rica");
else if(strstr(ext, "PA"))
strcpy(result, "Panama");
else if(strstr(ext, "CU"))
strcpy(result, "Cuba");
else if(strstr(ext, "KY"))
strcpy(result, "Cayman Islands");
else if(strstr(ext, "JM"))
strcpy(result, "Jamaica");
else if(strstr(ext, "CO"))
strcpy(result, "Colombia");
else if(strstr(ext, "GT"))
strcpy(result, "Guatemala");
else if(strstr(ext, "MH"))
strcpy(result, "Marshall Islands");
else if(strstr(ext, "PE"))
strcpy(result, "Peru");
else if(strstr(ext, "AQ"))
strcpy(result, "Antarctica");
else if(strstr(ext, "QA"))
strcpy(result, "Qatar");
else if(strstr(ext, "BB"))
strcpy(result, "Barbados");
else if(strstr(ext, "AW"))
strcpy(result, "Aruba");
else if(strstr(ext, "MC"))
strcpy(result, "Monaco");
else if(strstr(ext, "KW"))
strcpy(result, "Kuwait");
else if(strstr(ext, "OM"))
strcpy(result, "Oman");
else if(strstr(ext, "AI"))
strcpy(result, "Anguilla");
else if(strstr(ext, "KN"))
strcpy(result, "Saint Kitts and Nevis");
else if(strstr(ext, "GD"))
strcpy(result, "Grenada");
else if(strstr(ext, "DO"))
strcpy(result, "Dominican Republic");
else if(strstr(ext, "PY"))
strcpy(result, "Paraguay");
else if(strstr(ext, "LC"))
strcpy(result, "Saint Lucia");
else if(strstr(ext, "GL"))
strcpy(result, "Greenland");
else if(strstr(ext, "UZ"))
strcpy(result, "Uzbekistan");
else if(strstr(ext, "VE"))
strcpy(result, "Venezuela");
else if(strstr(ext, "TC"))
strcpy(result, "Turks and Caicos Islands");
else if(strstr(ext, "AG"))
strcpy(result, "Antigua and Barbuda");
else if(strstr(ext, "BH"))
strcpy(result, "Bahrain");
else if(strstr(ext, "JE"))
strcpy(result, "Jersey");
else if(strstr(ext, "TV"))
strcpy(result, "Tuvalu");
else if(strstr(ext, "PF"))
strcpy(result, "French Polynesia");
else if(strstr(ext, "SB"))
strcpy(result, "Solomon Islands");
else if(strstr(ext, "VU"))
strcpy(result, "Vanuatu");
else if(strstr(ext, "GE"))
strcpy(result, "Georgia");
else if(strstr(ext, "LK"))
strcpy(result, "Sri Lanka");
else if(strstr(ext, "SY"))
strcpy(result, "Syrian Arab Republic");
else if(strstr(ext, "MD"))
strcpy(result, "Moldov");
else if(strstr(ext, "AX"))
strcpy(result, "Aland Islands");
else if(strstr(ext, "AS"))
strcpy(result, "American Samoa");
else if(strstr(ext, "AD"))
strcpy(result, "Andorra");
else if(strstr(ext, "FO"))
strcpy(result, "Faroe Islands");
else if(strstr(ext, "GG"))
strcpy(result, "Guernsey");
else if(strstr(ext, "KG"))
strcpy(result, "Kyrgyzstan");
else if(strstr(ext, "SM"))
strcpy(result, "San Marino");
else if(strstr(ext, "SJ"))
strcpy(result, "Svalbard and Jan Mayen");
else if(strstr(ext, "TM"))
strcpy(result, "Turkmenistan");
else if(strstr(ext, "GU"))
strcpy(result, "Guam");
else if(strstr(ext, "NU"))
strcpy(result, "Niue");
else if(strstr(ext, "AP"))
strcpy(result, "Asia/Pacific Region");
else if(strstr(ext, "MM"))
strcpy(result, "Myanmar");
else if(strstr(ext, "BN"))
strcpy(result, "Brunei Darussalam");
else if(strstr(ext, "FK"))
strcpy(result, "Falkland Islands (Malvinas)");
else if(strstr(ext, "HT"))
strcpy(result, "Haiti");
else if(strstr(ext, "UM"))
strcpy(result, "United States Minor Outlying Islands");
else if(strstr(ext, "FM"))
strcpy(result, "Micronesi");
else if(strstr(ext, "EC"))
strcpy(result, "Ecuador");
else if(strstr(ext, "HN"))
strcpy(result, "Honduras");
else if(strstr(ext, "SV"))
strcpy(result, "El Salvador");
else if(strstr(ext, "TT"))
strcpy(result, "Trinidad and Tobago");
else if(strstr(ext, "BV"))
strcpy(result, "Bouvet Island");
else if(strstr(ext, "CK"))
strcpy(result, "Cook Islands");
else if(strstr(ext, "A1"))
strcpy(result, "Anonymous Proxy");
else if(strstr(ext, "WS"))
strcpy(result, "Samoa");
else if(strstr(ext, "UY"))
strcpy(result, "Uruguay");
else if(strstr(ext, "MP"))
strcpy(result, "Northern Mariana Islands");
else if(strstr(ext, "PW"))
strcpy(result, "Palau");
else if(strstr(ext, "LA"))
strcpy(result, "Lao People's Democratic Republic");
else if(strstr(ext, "GY"))
strcpy(result, "Guyana");
else if(strstr(ext, "SR"))
strcpy(result, "Suriname");
else if(strstr(ext, "MS"))
strcpy(result, "Montserrat");
else if(strstr(ext, "PM"))
strcpy(result, "Saint Pierre and Miquelon");
else if(strstr(ext, "GP"))
strcpy(result, "Guadeloupe");
else if(strstr(ext, "MF"))
strcpy(result, "Saint Martin");
else if(strstr(ext, "BI"))
strcpy(result, "Burundi");
else if(strstr(ext, "ET"))
strcpy(result, "Ethiopia");
else if(strstr(ext, "BT"))
strcpy(result, "Bhutan");
else if(strstr(ext, "MQ"))
strcpy(result, "Martinique");
else if(strstr(ext, "IO"))
strcpy(result, "British Indian Ocean Territory");
else if(strstr(ext, "ER"))
strcpy(result, "Eritrea");
else if(strstr(ext, "GW"))
strcpy(result, "Guinea-Bissau");
else if(strstr(ext, "TO"))
strcpy(result, "Tonga");
else if(strstr(ext, "TL"))
strcpy(result, "Timor-Leste");
else if(strstr(ext, "ST"))
strcpy(result, "Sao Tome and Principe");
else if(strstr(ext, "GF"))
strcpy(result, "French Guiana");
else if(strstr(ext, "KI"))
strcpy(result, "Kiribati");
else if(strstr(ext, "NF"))
strcpy(result, "Norfolk Island");
else if(strstr(ext, "NR"))
strcpy(result, "Nauru");
else if(strstr(ext, "YT"))
strcpy(result, "Mayotte");

return result;
}
