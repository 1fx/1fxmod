#include "g_local.h"
#include "boe_local.h"

char *GetCountry(char *ext);

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

	G_ResetGametype(qfalse);
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
	spawns[4][0] += 105;
	spawns[4][1] -= 85;
	level.cagefight = qtrue;
	level.messagedisplay = qtrue; // stop Seeker Released
	level.messagedisplay1 = qtrue; // stop RPG/M4 stuff
	SpawnCage(level.hideseek_cage, NULL, qtrue);
	for(i=0;i<level.numConnectedClients;i++){
		if(g_entities[level.sortedClients[i]].client->sess.team == TEAM_RED){
			//respawn ( &g_entities[level.sortedClients[i]] );
			count += 1;
			TeleportPlayer(&g_entities[level.sortedClients[i]], spawns[count], g_entities[level.sortedClients[i]].client->ps.viewangles, qtrue);
			g_entities[level.sortedClients[i]].client->ps.stats[STAT_WEAPONS] = 0;
			memset ( g_entities[level.sortedClients[i]].client->ps.ammo, 0, sizeof(g_entities[level.sortedClients[i]].client->ps.ammo) );
			memset ( g_entities[level.sortedClients[i]].client->ps.clip, 0, sizeof(g_entities[level.sortedClients[i]].client->ps.clip) );
			//g_entities[level.lastalive[0]].client->ps.ammo[weaponData[WP_RPG7_LAUNCHER].attack[ATTACK_NORMAL].ammoIndex]=2;
			g_entities[level.sortedClients[i]].client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_KNIFE );
			g_entities[level.sortedClients[i]].client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_AK74_ASSAULT_RIFLE );
			//g_entities[level.lastalive[0]].client->ps.clip[ATTACK_NORMAL][WP_RPG7_LAUNCHER]=1;
			//g_entities[level.lastalive[0]].client->ps.firemode[WP_RPG7_LAUNCHER] = BG_FindFireMode ( WP_RPG7_LAUNCHER, ATTACK_NORMAL, WP_FIREMODE_AUTO );
			g_entities[level.sortedClients[i]].client->ps.weapon = WP_AK74_ASSAULT_RIFLE;
			g_entities[level.sortedClients[i]].client->ps.weaponstate = WEAPON_READY;
			g_entities[level.sortedClients[i]].client->ps.weaponTime = 0;
			g_entities[level.sortedClients[i]].client->ps.weaponAnimTime = 0;
			g_entities[level.sortedClients[i]].client->ps.stats[STAT_FROZEN] = 10000;
		}else{
			player_die(&g_entities[level.sortedClients[i]], NULL, NULL, 10000, MOD_POP, 0, 0);
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


	// skip whitespace
	while ( *string <= ' ' ) {
		if ( !*string ) {
			return 0;
		}
		string++;
	}

	// check sign
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

	// read digits
	value = 0;
	do {
		c = *string++;
		if ( c < '0' || c > '9' ) {
			break;
		}
		c -= '0';
		value = value * 10 + c;
	} while ( 1 );

	// not handling 10e10 notation...

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

	level.numSpawnVars = 0;
	level.numSpawnVarChars = 0;
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
	char	begin_ip[24], end_ip[24], country[128], ext[6];
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
if(IPnum >= 16777216 && IPnum <= 460945407){
part = 0;
}else if(IPnum >= 460947456 && IPnum <= 773140479){
part = 1;
}else if(IPnum >= 773140480 && IPnum <= 786634751){
part = 2;
}else if(IPnum >= 786634752 && IPnum <= 1041705695){
part = 3;
}else if(IPnum >= 1041705696 && IPnum <= 1042894355){
part = 4;
}else if(IPnum >= 1042894356 && IPnum <= 1044938523){
part = 5;
}else if(IPnum >= 1044938524 && IPnum <= 1047568251){
part = 6;
}else if(IPnum >= 1047568252 && IPnum <= 1049762303){
part = 7;
}else if(IPnum >= 1049762304 && IPnum <= 1052361663){
part = 8;
}else if(IPnum >= 1052361664 && IPnum <= 1054672775){
part = 9;
}else if(IPnum >= 1054672776 && IPnum <= 1074680623){
part = 10;
}else if(IPnum >= 1074680624 && IPnum <= 1076309151){
part = 11;
}else if(IPnum >= 1076309152 && IPnum <= 1078437567){
part = 12;
}else if(IPnum >= 1078437568 && IPnum <= 1083417791){
part = 13;
}else if(IPnum >= 1083417792 && IPnum <= 1093740191){
part = 14;
}else if(IPnum >= 1093740192 && IPnum <= 1106547759){
part = 15;
}else if(IPnum >= 1106547760 && IPnum <= 1117985447){
part = 16;
}else if(IPnum >= 1117985448 && IPnum <= 1120493087){
part = 17;
}else if(IPnum >= 1120493088 && IPnum <= 1125455399){
part = 18;
}else if(IPnum >= 1125455400 && IPnum <= 1137524735){
part = 19;
}else if(IPnum >= 1137524736 && IPnum <= 1158496506){
part = 20;
}else if(IPnum >= 1158496507 && IPnum <= 1160810751){
part = 21;
}else if(IPnum >= 1160810752 && IPnum <= 1163588608){
part = 22;
}else if(IPnum >= 1163588609 && IPnum <= 1208518244){
part = 23;
}else if(IPnum >= 1208518245 && IPnum <= 1210097791){
part = 24;
}else if(IPnum >= 1210097792 && IPnum <= 1254959151){
part = 25;
}else if(IPnum >= 1254959152 && IPnum <= 1279975423){
part = 26;
}else if(IPnum >= 1279975424 && IPnum <= 1307578367){
part = 27;
}else if(IPnum >= 1307578368 && IPnum <= 1317668791){
part = 28;
}else if(IPnum >= 1317668792 && IPnum <= 1334595583){
part = 29;
}else if(IPnum >= 1334595584 && IPnum <= 1347977607){
part = 30;
}else if(IPnum >= 1347977608 && IPnum <= 1357899279){
part = 31;
}else if(IPnum >= 1357899280 && IPnum <= 1360629247){
part = 32;
}else if(IPnum >= 1360629248 && IPnum <= 1365220855){
part = 33;
}else if(IPnum >= 1365220856 && IPnum <= 1386676223){
part = 34;
}else if(IPnum >= 1386676224 && IPnum <= 1401563135){
part = 35;
}else if(IPnum >= 1401563136 && IPnum <= 1407524767){
part = 36;
}else if(IPnum >= 1407524768 && IPnum <= 1410818047){
part = 37;
}else if(IPnum >= 1410818048 && IPnum <= 1426967567){
part = 38;
}else if(IPnum >= 1426967568 && IPnum <= 1441603583){
part = 39;
}else if(IPnum >= 1441603584 && IPnum <= 1446911367){
part = 40;
}else if(IPnum >= 1446911368 && IPnum <= 1485275135){
part = 41;
}else if(IPnum >= 1485275136 && IPnum <= 1505456895){
part = 42;
}else if(IPnum >= 1505456896 && IPnum <= 1534714287){
part = 43;
}else if(IPnum >= 1534714288 && IPnum <= 1539480063){
part = 44;
}else if(IPnum >= 1539480064 && IPnum <= 1540152319){
part = 45;
}else if(IPnum >= 1540152320 && IPnum <= 1540664575){
part = 46;
}else if(IPnum >= 1540664576 && IPnum <= 1541165823){
part = 47;
}else if(IPnum >= 1541165824 && IPnum <= 1566400639){
part = 48;
}else if(IPnum >= 1566400640 && IPnum <= 1578594239){
part = 49;
}else if(IPnum >= 1578594240 && IPnum <= 1593209163){
part = 50;
}else if(IPnum >= 1593209164 && IPnum <= 1652643223){
part = 51;
}else if(IPnum >= 1652643224 && IPnum <= 1836777471){
part = 52;
}else if(IPnum >= 1836777472 && IPnum <= 1908670463){
part = 53;
}else if(IPnum >= 1908670464 && IPnum <= 2018004991){
part = 54;
}else if(IPnum >= 2018004992 && IPnum <= 2173239295){
part = 55;
}else if(IPnum >= 2173239296 && IPnum <= 2337013759){
part = 56;
}else if(IPnum >= 2337013760 && IPnum <= 2545483775){
part = 57;
}else if(IPnum >= 2545483776 && IPnum <= 2756509695){
part = 58;
}else if(IPnum >= 2756509696 && IPnum <= 2917168479){
part = 59;
}else if(IPnum >= 2917168480 && IPnum <= 2928655991){
part = 60;
}else if(IPnum >= 2928655992 && IPnum <= 2988465647){
part = 61;
}else if(IPnum >= 2988465648 && IPnum <= 2988543015){
part = 62;
}else if(IPnum >= 2988543016 && IPnum <= 2997003199){
part = 63;
}else if(IPnum >= 2997003200 && IPnum <= 3075388415){
part = 64;
}else if(IPnum >= 3075389440 && IPnum <= 3164950735){
part = 65;
}else if(IPnum >= 3164950736 && IPnum <= 3195645951){
part = 66;
}else if(IPnum >= 3195650048 && IPnum <= 3226481407){
part = 67;
}else if(IPnum >= 3226481408 && IPnum <= 3229969663){
part = 68;
}else if(IPnum >= 3229969664 && IPnum <= 3233654271){
part = 69;
}else if(IPnum >= 3233654272 && IPnum <= 3239306239){
part = 70;
}else if(IPnum >= 3239306240 && IPnum <= 3240858623){
part = 71;
}else if(IPnum >= 3240858624 && IPnum <= 3245029375){
part = 72;
}else if(IPnum >= 3245029376 && IPnum <= 3247816703){
part = 73;
}else if(IPnum >= 3247816704 && IPnum <= 3251245567){
part = 74;
}else if(IPnum >= 3251245568 && IPnum <= 3253890559){
part = 75;
}else if(IPnum >= 3253890560 && IPnum <= 3256165375){
part = 76;
}else if(IPnum >= 3256165376 && IPnum <= 3257785183){
part = 77;
}else if(IPnum >= 3257785184 && IPnum <= 3261784063){
part = 78;
}else if(IPnum >= 3261784064 && IPnum <= 3262476299){
part = 79;
}else if(IPnum >= 3262476300 && IPnum <= 3262479059){
part = 80;
}else if(IPnum >= 3262479060 && IPnum <= 3262753791){
part = 81;
}else if(IPnum >= 3262753792 && IPnum <= 3265605631){
part = 82;
}else if(IPnum >= 3265605632 && IPnum <= 3268268799){
part = 83;
}else if(IPnum >= 3268268800 && IPnum <= 3272084735){
part = 84;
}else if(IPnum >= 3272084736 && IPnum <= 3273345279){
part = 85;
}else if(IPnum >= 3273345280 && IPnum <= 3275446271){
part = 86;
}else if(IPnum >= 3275446272 && IPnum <= 3276045455){
part = 87;
}else if(IPnum >= 3276045456 && IPnum <= 3276371239){
part = 88;
}else if(IPnum >= 3276371240 && IPnum <= 3276401759){
part = 89;
}else if(IPnum >= 3276401760 && IPnum <= 3276790127){
part = 90;
}else if(IPnum >= 3276790128 && IPnum <= 3278939335){
part = 91;
}else if(IPnum >= 3278939336 && IPnum <= 3278943296){
part = 92;
}else if(IPnum >= 3278943297 && IPnum <= 3278945271){
part = 93;
}else if(IPnum >= 3278945272 && IPnum <= 3280930959){
part = 94;
}else if(IPnum >= 3280930960 && IPnum <= 3280986365){
part = 95;
}else if(IPnum >= 3280986366 && IPnum <= 3283585023){
part = 96;
}else if(IPnum >= 3283585024 && IPnum <= 3285471103){
part = 97;
}else if(IPnum >= 3285471104 && IPnum <= 3285575679){
part = 98;
}else if(IPnum >= 3285575680 && IPnum <= 3287704063){
part = 99;
}else if(IPnum >= 3287704064 && IPnum <= 3326713343){
part = 100;
}else if(IPnum >= 3326713344 && IPnum <= 3355478271){
part = 101;
}else if(IPnum >= 3355478272 && IPnum <= 3360694079){
part = 102;
}else if(IPnum >= 3360694080 && IPnum <= 3392339967){
part = 103;
}else if(IPnum >= 3392339968 && IPnum <= 3397781503){
part = 104;
}else if(IPnum >= 3397781504 && IPnum <= 3406670847){
part = 105;
}else if(IPnum >= 3406670848 && IPnum <= 3416328191){
part = 106;
}else if(IPnum >= 3416328192 && IPnum <= 3420367503){
part = 107;
}else if(IPnum >= 3420367504 && IPnum <= 3429396223){
part = 108;
}else if(IPnum >= 3429396224 && IPnum <= 3451824127){
part = 109;
}else if(IPnum >= 3451824128 && IPnum <= 3460917135){
part = 110;
}else if(IPnum >= 3460917136 && IPnum <= 3468632295){
part = 111;
}else if(IPnum >= 3468632296 && IPnum <= 3476420863){
part = 112;
}else if(IPnum >= 3476420864 && IPnum <= 3488072703){
part = 113;
}else if(IPnum >= 3488072704 && IPnum <= 3495741439){
part = 114;
}else if(IPnum >= 3495741440 && IPnum <= 3508303871){
part = 115;
}else if(IPnum >= 3508303872 && IPnum <= 3511419951){
part = 116;
}else if(IPnum >= 3511419952 && IPnum <= 3517424639){
part = 117;
}else if(IPnum >= 3517424640 && IPnum <= 3522120447){
part = 118;
}else if(IPnum >= 3522120448 && IPnum <= 3558288063){
part = 119;
}else if(IPnum >= 3558288064 && IPnum <= 3560833023){
part = 120;
}else if(IPnum >= 3560833024 && IPnum <= 3560943451){
part = 121;
}else if(IPnum >= 3560943452 && IPnum <= 3560951119){
part = 122;
}else if(IPnum >= 3560951120 && IPnum <= 3563854175){
part = 123;
}else if(IPnum >= 3563854176 && IPnum <= 3565493759){
part = 124;
}else if(IPnum >= 3565493760 && IPnum <= 3569740047){
part = 125;
}else if(IPnum >= 3569740048 && IPnum <= 3575625039){
part = 126;
}else if(IPnum >= 3575625040 && IPnum <= 3576257167){
part = 127;
}else if(IPnum >= 3576257168 && IPnum <= 3582192303){
part = 128;
}else if(IPnum >= 3582192304 && IPnum <= 3584196607){
part = 129;
}else if(IPnum >= 3584196608 && IPnum <= 3587287387){
part = 130;
}else if(IPnum >= 3587287388 && IPnum <= 3590311231){
part = 131;
}else if(IPnum >= 3590311232 && IPnum <= 3626382121){
part = 132;
}else if(IPnum >= 3626382122 && IPnum <= 3630074543){
part = 133;
}else if(IPnum >= 3630074544 && IPnum <= 3633036607){
part = 134;
}else if(IPnum >= 3633036608 && IPnum <= 3640112575){
part = 135;
}else if(IPnum >= 3640112576 && IPnum <= 3644961343){
part = 136;
}else if(IPnum >= 3644961344 && IPnum <= 3645764561){
part = 137;
}else if(IPnum >= 3645764562 && IPnum <= 3651223663){
part = 138;
}else if(IPnum >= 3651223664 && IPnum <= 3740925951){
part = 139;
}else{
part = 140;
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
