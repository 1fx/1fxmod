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

// sort of sscanf found here
// http://cboard.cprogramming.com/c-programming/18092-sscanf-source-code.html
int (readFromStr)( char *src, char *format, ... )
{
  va_list ap;
  float *f;
  int conv = 0, *i, index;
  char *a, *fp, *sp = src, buf[512] = {'\0'};

  va_start ( ap, format );
  for ( fp = format; *fp != '\0'; fp++ ) {
    for ( index = 0; *sp != '\0' && *sp != ' '; index++ )
      buf[index] = *sp++;
    while ( *sp == ' ' ) sp++;
    while ( *fp != '%' ) fp++;
    if ( *fp == '%' ) {
      switch ( *++fp ) {
      case 'i':
        i = va_arg ( ap, int * );
        *i = atoi ( buf );
        break;
      case 'f':
        f = va_arg ( ap, float * );
        *f = (float)atof ( buf );
        break;
      case 's':
        a = va_arg ( ap, char * );
        strncpy ( a, buf, strlen ( buf ) + 1 );
        break;
      }
      conv++;
    }
  }
  va_end ( ap );
  return conv;
}

void Henk_Tip(void){
	qboolean tip = qfalse;
	char buf[1024];
    struct test{
		char tip[256]; // Boe!Man 6/13/11: Ease down on those massive buffers.. The QVM compiler is chocking in them. A size of 255 for a tip should be sufficient (?).
	} Tips[64];
	fileHandle_t f;
	int len, i, start, count;

	len = trap_FS_FOpenFile( g_tipsFile.string, &f, FS_READ_TEXT); 
	if (!f) { 
		Com_Printf("Can't open %s\n", g_tipsFile.string);
		level.tipMsg = level.time+(server_msgInterval.integer*60000); // Boe!Man 6/17/11: If the tips file isn't found, do update the interval so it doesn't attempt to open the file every x msecs.
		return;
	}
	memset( buf, 0, sizeof(buf) );
	trap_FS_Read( buf, len, f );
	start = 0;
	count = 0;
	for(i=0;i<len;i++){
		if(buf[i] == '\n'){
			Q_strncpyz(Tips[count].tip, buf+start, i-start);
			count++;
			start = i+1;
		}
	}
	trap_FS_FCloseFile(f);

	trap_SendServerCommand( -1, va("chat -1 \"%sR%sa%sn%sd%so%sm Tip: %s\n\"", server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, Tips[irand(0, count-1)].tip ) );		
	level.tipMsg = level.time+(server_msgInterval.integer*60000);
}

void Henk_Ignore(gentity_t *ent){
	char arg1[32];
	int i, numberofclients, idnum = -1, z, temparray[33], count = 0;
	char string[1024] = "\0", string1[64] = "\0";

	trap_Argv( 1, arg1, sizeof( arg1 ) );
	if(strlen(arg1) < 1){
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You haven't entered a valid player ID/player name.\n\""));
		return;
	}
	if(!henk_ischar(arg1[0])){
		idnum = atoi(arg1);
	}else if(henk_ischar(arg1[0])){
			memset(string, 0, sizeof(string));
			memset(string1, 0, sizeof(string1));
			numberofclients = 0;
			for(i=0;i<level.numConnectedClients;i++){
				//trap_SendServerCommand(-1, va("print\"^3[Debug] ^7%s comparing with %s.\n\"", g_entities[level.sortedClients[i]].client->pers.cleanName,numb));
				if(strstr(Q_strlwr(g_entities[level.sortedClients[i]].client->pers.cleanName), Q_strlwr(arg1))){
					idnum = level.sortedClients[i];
					numberofclients += 1;
					Com_sprintf(string1, sizeof(string1), "^1[#%i] ^7%s, ",  idnum, g_entities[level.sortedClients[i]].client->pers.cleanName);
					Q_strncpyz(string+strlen(string), string1, strlen(string1)+1);
				}
			}
			string[strlen(string)-2] = '\0';
			if(numberofclients > 1){
				trap_SendServerCommand(ent->s.number, va("print\"^3[Info] ^7Multiple names found with ^3%s^7: %s\n\"", arg1, string));
				return;
			}else if(numberofclients == 0)
				idnum = -1;
		}else
			idnum = -1;
		if ( idnum < 0 || idnum >= g_maxclients.integer )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You haven't entered a valid player ID/player name.\n\""));
			return;
		}
		
		if ( g_entities[idnum].client->pers.connected == CON_DISCONNECTED )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7The player is not connected.\n\""));
			return;
		}
		if( g_entities[idnum].client->sess.admin >= 2){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You can't ignore an admin.\n\""));
			return;
		}
		// Check if he's already ignored then unignore him
		for(i=0;i<g_entities[idnum].client->sess.IgnoredClientCount;i++){
			if(g_entities[idnum].client->sess.IgnoredClients[i] == ent->s.number){
				g_entities[idnum].client->sess.IgnoredClients[i] = -1;
				// now sort the array again.
				for(z=0;z<g_entities[idnum].client->sess.IgnoredClientCount;z++){
					if(g_entities[idnum].client->sess.IgnoredClients[z] != -1){
						temparray[count] = g_entities[idnum].client->sess.IgnoredClients[z];
						count++;
					}
				}
				// temp array to original
				for(z=0;z<g_entities[idnum].client->sess.IgnoredClientCount;z++){
					g_entities[idnum].client->sess.IgnoredClients[z] = temparray[z];
				}
				g_entities[idnum].client->sess.IgnoredClientCount--;
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You are not ignoring %s anymore.\n\"", g_entities[idnum].client->pers.netname));
				return;
			}
		}
		if(g_entities[idnum].client->sess.IgnoredClientCount >= 32){
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You can't ignore more than 32 people.\n\""));
			return;
		}
		g_entities[idnum].client->sess.IgnoredClients[g_entities[idnum].client->sess.IgnoredClientCount] = ent->s.number;
		g_entities[idnum].client->sess.IgnoredClientCount++;
		trap_SendServerCommand( g_entities[idnum].s.number, va("print \"^3[Info] ^7%s has ignored you.\n\"", ent->client->pers.netname));
		trap_SendServerCommand( ent->s.number, va("print \"^3[Info] ^7You have ignored %s.\n\"", g_entities[idnum].client->pers.netname));
}

qboolean IsValidCommand(char *cmd, char *string){
	int i, z;
	qboolean space = qfalse;
	for(i=0;i<=strlen(string);i++){
		if(string[i] == ' ')
			space = qtrue;
		if(string[i] == '!'){
			for(z=1;z<=strlen(cmd)-1;z++){
				if(string[i+z] != cmd[z]){
					break; // invalid command
				}else if(space){ // Henkie 25/09/2012 -> Fix issue with putting the short command in the middle of a sentence
					return qfalse;
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
	gentity_t *ent;
	int i, highscore = 0, count = 0;
	int winners[32];

	for(i=0;i<level.numConnectedClients;i++){
		ent = &g_entities[level.sortedClients[i]];
		if(ent->client->sess.team == TEAM_RED){
			if(ent->client->sess.kills == highscore){
				winners[count] = ent->s.number;
				count += 1;
				//highscore = ent->client->sess.kills; // Boe - useless.
			}else if(ent->client->sess.kills > highscore){
				winners[0] = ent->s.number;
				count = 1;
				highscore = ent->client->sess.kills;
			}
		}
	}

	if(count >= 2){ // Boe!Man 8/30/11: If there are tied players.
		for(i = 0; i < count; i++){
			g_entities[winners[i]].client->sess.cageFighter = qtrue;
		}
	}

	level.mapHighScore = highscore; // Boe!Man 8/30/11: Set the highest score (to make sure everybody gets in the cage if all have 0 points).
	return count;
}

void InitCagefight(void){
	int i, count = 0;

	vec3_t spawns[32];
	if(level.time < level.cagefighttimer){
		return;
	}
	level.startcage = qfalse;

	G_ResetGametype(qfalse, qtrue);
	for(i=0;i<=31;i++){ // Boe!Man 6/30/12: 32, regardless of the amount of players. The recommended max for any gametype is 32, above, unsupported. So just spread them more equally instead of adding even more spawns.
		VectorCopy(level.hideseek_cage, spawns[i]);
	}
	
	if(!level.hideseek_cageSize){ // 0 = regular.
		spawns[0][0] -= 105;
		spawns[0][1] -= 105;
		spawns[1][0] += 105;
		spawns[1][1] += 105;
		spawns[2][0] -= 105;
		spawns[2][1] += 105;
		spawns[3][0] += 105;
		spawns[3][1] -= 105;
		spawns[4][0] += 105; // fifth spawn
		//spawns[4][1] -= 0;
		spawns[5][0] -= 105;
		//spawns[5][1] += 0;
		//spawns[6][0] -= 0;
		spawns[6][1] += 105;
		//spawns[7][0] += 0;
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

		//spawns[20][0] += 0;
		spawns[20][1] -= 50;
		spawns[21][0] += 50;
		//spawns[21][1] += 0;
		//spawns[22][0] -= 0;
		spawns[22][1] += 50;
		spawns[23][0] -= 50;
		//spawns[23][1] -= 0;
		//spawns[24][0] -= 0;
		//spawns[24][1] -= 0;
	}else if(level.hideseek_cageSize == 2){ // Boe!Man 6/30/12: 2 = Big cage. So, add big cage spawnpoints.
		// The absolute corners.
		spawns[0][0] -= 231;
		spawns[0][1] -= 231;
		spawns[1][0] += 231;
		spawns[1][1] += 231;
		spawns[2][0] -= 231;
		spawns[2][1] += 231;
		spawns[3][0] += 231;
		spawns[3][1] -= 231;
		
		// The center between the two cages, still on the 'outer' line.
		//spawns[4][0] -= 0;
		spawns[4][1] -= 231;
		//spawns[5][0] += 0;
		spawns[5][1] += 231;
		spawns[6][0] -= 231;
		//spawns[6][1] -= 0;
		spawns[7][0] += 231;
		//spawns[7][1] += 0;
		
		// The quarter between the cage, again, on the outer line.
		spawns[8][0] -= 115.5;
		spawns[8][1] -= 231;
		spawns[9][0] += 115.5;
		spawns[9][1] += 231;
		spawns[10][0] -= 231;
		spawns[10][1] -= 115.5;
		spawns[11][0] += 231;
		spawns[11][1] += 115.5;
		
		// The same, except the opposite quarter.
		spawns[12][0] += 115.5;
		spawns[12][1] -= 231;
		spawns[13][0] -= 115.5;
		spawns[13][1] += 231;
		spawns[14][0] -= 231;
		spawns[14][1] += 115.5;
		spawns[15][0] += 231;
		spawns[15][1] -= 115.5;
		
		// Now we begin spawning them in the 'inner cage', the original spawnpoints that we maintain.
		spawns[16][0] -= 105;
		spawns[16][1] -= 105;
		spawns[17][0] += 105;
		spawns[17][1] += 105;
		spawns[18][0] -= 105;
		spawns[18][1] += 105;
		spawns[19][0] += 105;
		spawns[19][1] -= 105;
		
		// Center of the inner cage.
		//spawns[20][0] -= 0;
		spawns[20][1] -= 105;
		//spawns[21][0] += 0;
		spawns[21][1] += 105;
		spawns[22][0] -= 105;
		//spawns[22][1] += 0;
		spawns[23][0] += 105;
		//spawns[23][1] -= 0;
		
		// Spawn one fella in the absolute center.
		//spawns[24][0] -= 0;
		//spawns[24][1] -= 0;
		
		// Have the extra ability to spawn four more, do this between the absolutes of the inner cage and the outer cage ( (231 - 105) / 2 + 105)
		spawns[25][0] -= 168;
		spawns[25][1] -= 168;
		spawns[26][0] += 168;
		spawns[26][1] += 168;
		spawns[27][0] -= 168;
		spawns[27][1] += 168;
		spawns[28][0] += 168;
		spawns[28][1] -= 168;
	}

	level.cagefight = qtrue;
	level.messagedisplay = qtrue; // stop Seeker Released
	level.messagedisplay1 = qtrue; // stop RPG/M4 stuff
	if(level.hideseek_cageSize == 2){ // Spawn big cage.
		SpawnCage(level.hideseek_cage, NULL, qtrue, qtrue);
		SpawnCage(level.hideseek_cage, NULL, qtrue, qtrue); // 2 to be sure no parts are missing
	}else if(!level.hideseek_cageSize){
		SpawnCage(level.hideseek_cage, NULL, qtrue, qfalse);
		SpawnCage(level.hideseek_cage, NULL, qtrue, qfalse); // 2 to be sure no parts are missing
	}

	for(i = 0; i < level.numConnectedClients; i++){
		if(level.clients[level.sortedClients[i]].pers.connected != CON_CONNECTED)
		{
			continue;
		}

		if(level.clients[level.sortedClients[i]].sess.team == TEAM_RED && (level.clients[level.sortedClients[i]].sess.cageFighter == qtrue || level.mapHighScore == 0)){
			//respawn ( &g_entities[level.sortedClients[i]] );
			if(level.hideseek_cageSize != 1){ // 1 = no cage at all, if this is the cage, don't teleport them to the 'cage'.
				TeleportPlayer(&g_entities[level.sortedClients[i]], spawns[count], level.clients[level.sortedClients[i]].ps.viewangles, qtrue);
				count += 1;
			}
			level.clients[level.sortedClients[i]].ps.stats[STAT_WEAPONS] = 0;
			memset ( level.clients[level.sortedClients[i]].ps.ammo, 0, sizeof(level.clients[level.sortedClients[i]].ps.ammo) );
			memset ( level.clients[level.sortedClients[i]].ps.clip, 0, sizeof(level.clients[level.sortedClients[i]].ps.clip) );
			//g_entities[level.lastalive[0]].client->ps.ammo[weaponData[WP_RPG7_LAUNCHER].attack[ATTACK_NORMAL].ammoIndex]=2;
			level.clients[level.sortedClients[i]].ps.stats[STAT_WEAPONS] |= ( 1 << WP_KNIFE );
			level.clients[level.sortedClients[i]].ps.clip[ATTACK_NORMAL][WP_KNIFE]=1;
			level.clients[level.sortedClients[i]].ps.stats[STAT_WEAPONS] |= ( 1 << WP_AK74_ASSAULT_RIFLE );
			//g_entities[level.lastalive[0]].client->ps.clip[ATTACK_NORMAL][WP_RPG7_LAUNCHER]=1;
			//g_entities[level.lastalive[0]].client->ps.firemode[WP_RPG7_LAUNCHER] = BG_FindFireMode ( WP_RPG7_LAUNCHER, ATTACK_NORMAL, WP_FIREMODE_AUTO );
			level.clients[level.sortedClients[i]].ps.weapon = WP_AK74_ASSAULT_RIFLE;
			level.clients[level.sortedClients[i]].ps.weaponstate = WEAPON_READY;
			level.clients[level.sortedClients[i]].ps.weaponTime = 0;
			level.clients[level.sortedClients[i]].ps.weaponAnimTime = 0;
			level.clients[level.sortedClients[i]].ps.stats[STAT_FROZEN] = 10000;
		}else if(level.clients[level.sortedClients[i]].sess.team == TEAM_BLUE){
			G_Damage (&g_entities[level.sortedClients[i]], NULL, NULL, NULL, NULL, 10000, 0, MOD_POP, HL_HEAD|HL_FOOT_RT|HL_FOOT_LT|HL_LEG_UPPER_RT|HL_LEG_UPPER_LT|HL_HAND_RT|HL_HAND_LT|HL_WAIST|HL_CHEST|HL_NECK);
		}else if(level.clients[level.sortedClients[i]].sess.team != TEAM_SPECTATOR){ // Boe!Man 8/30/11: Means the client is a Red player that wasn't qualified to play in the cage fight. Pop as well.
			G_Damage (&g_entities[level.sortedClients[i]], NULL, NULL, NULL, NULL, 10000, 0, MOD_POP, HL_HEAD|HL_FOOT_RT|HL_FOOT_LT|HL_LEG_UPPER_RT|HL_LEG_UPPER_LT|HL_HAND_RT|HL_HAND_LT|HL_WAIST|HL_CHEST|HL_NECK);
		} // Boe!Man 9/4/11: Else would be spectator only, no need to do anything.
	}
	// when it ends execute this:
	//Com_Printf("Updating scores..\n");
	//UpdateScores();
	//level.cagefight = qfalse;
	//LogExit( "Timelimit hit." );
}

void DropRandom( gentity_t *ent, int zombies){
	gentity_t	*dropped;
	gitem_t *item;
	int group, random, i, DropGroup, start;
	int Odds[8][1];
	int weaponDropOdds[8][7] = 
   {//      group1   group2   group3   group4   group5   group6
      {   80,      20,      0,      0,      0,      0,      0,   },      //1-2 zombies
      {   20,      40,      40,      0,      0,      0,      0,   },      //3-4 zombies
      {   5,      5,      30,      30,      20,      10,      0,   },      //5-6 zombies
      {   5,      5,      10,      20,      40,      20,      0,   },      //7-8 zombies
      {   5,      5,      10,      20,      30,      30,      0,   },      //9-10 zombies
	  {   5,      5,      5,      10,      40,      30,      5,   } ,     //11-13 zombies
	  {   0,      5,      5,      10,      30,      40,      10,   } ,     //14-16 zombies
	  {   0,      5,      5,      10,      20,      40,      20,   }      //17-20 zombies
   };

   int weaponGroups[6][3] =
   {//      weapon1                  weapon2               weapon3 (optional)
      {   WP_M1911A1_PISTOL,         WP_USSOCOM_PISTOL,      0,         },   //group 1
      {   WP_MICRO_UZI_SUBMACHINEGUN,   WP_M3A1_SUBMACHINEGUN,   0,         },   //group 2
      {   WP_L2A2_GRENADE,         WP_L2A2_GRENADE,      0,         },   //group 3
      {   WP_M4_ASSAULT_RIFLE,      WP_AK74_ASSAULT_RIFLE,   WP_MSG90A1,   },   //group 4
      {   WP_USAS_12_SHOTGUN,         WP_M60_MACHINEGUN,      0,         },   //group 5
      {   WP_MM1_GRENADE_LAUNCHER,   WP_RPG7_LAUNCHER,      0,         },   //group 6
   };

	//trap_SendServerCommand(-1, va("print \"^3[Debug] ^7Spawning random weapon with %i zombies.\n\"", zombies) );

   if(zombies >= 1 && zombies <= 2){
	   DropGroup = 0;
   }else if(zombies >= 3 && zombies <= 4){
	   DropGroup = 1;
   }else if(zombies >= 5 && zombies <= 6){
	   DropGroup = 2;
   }else if(zombies >= 7 && zombies <= 8){
	   DropGroup = 3;
   }else if(zombies >= 9 && zombies <= 10){
	   DropGroup = 4;
   }else if(zombies >= 11 && zombies <= 13){
	   DropGroup = 5;
   }else if(zombies >= 14 && zombies <= 16){
	   DropGroup = 6;
   }else if(zombies >= 17 && zombies <= 20){
	   DropGroup = 7;
   }else
	   DropGroup = -1;

   if(DropGroup == -1){
	   Com_Printf("Error, too much zombies: %i\n", zombies);
	   return;
   }
   random = irand(0, 99);
   start = 0;
   for(i=0;i<8;i++){
	   // 80, 20, 0 , 0
	   // 20, 40, 40 , 0
		if(weaponDropOdds[DropGroup][i] != 0){
			Odds[i][0] = start;
			Odds[i][1] = (Odds[i][0]+weaponDropOdds[DropGroup][i])-1;
			if(random >= Odds[i][0] && random <= Odds[i][1]){ // we have found our group.
				group = i;
				break;
			}
			start = Odds[i][0]+weaponDropOdds[DropGroup][i];
		}
   }
   #ifdef _DEBUG
   Com_Printf("Group: %i\n", group);
   #endif
   if(weaponGroups[group][2] == 0) 
	   random = irand(0,1); // randomize between 2 weapons
   else
	  random = irand(0, 2); // randomize between 3 weapons 

   //Com_Printf("Random: %i\n", random);

	item = BG_FindWeaponItem ((weapon_t)weaponGroups[group][random]);
	dropped = G_DropItem2(ent->r.currentOrigin, vec3_origin, item);
	//dropped->count  = 1&0xFF;
	//dropped->count += ((2<<8) & 0xFF00);
	//dropped->count += ((1 << 16) & 0xFF0000 );
	//dropped->count += ((2 << 24) & 0xFF000000 );
	//trap_SendServerCommand(-1, va("print \"^3[Debug] ^7%s spawned.\n\"", item->pickup_name) );
}


void CloneBody( gentity_t *ent, int number )
{
	gentity_t	*body;
	int			contents;
	int			parm;
	vec3_t		velo;

	int hitLocation = HL_NONE;
	vec3_t	direction;

	VectorCopy(ent->client->ps.viewangles, ent->client->sess.tempangles);
	Henk_CloseSound(ent->r.currentOrigin, G_SoundIndex("sound/enemy/dog/bark03.mp3"));
	Henk_CloseSound(ent->r.currentOrigin, G_SoundIndex("sound/enemy/dog/attack01.mp3"));
	// Boe!Man 1/8/12: Fix crash issues in DLL and possibily QVM by processing uninitialised vector.
	//VectorCopy(direction, vec3_origin);
	VectorCopy(vec3_origin, direction);
	trap_UnlinkEntity (ent);

	// if client is in a nodrop area, don't leave the body
	contents = trap_PointContents( ent->r.currentOrigin, -1 );
	if ( contents & CONTENTS_NODROP )
	{
		return;
	}

	// grab a body que and cycle to the next one
	body = level.bodyQue[ level.bodyQueIndex ];

	level.bodyQueIndex = (level.bodyQueIndex + 1) % level.bodyQueSize;

	trap_UnlinkEntity (body);

	body->s					= ent->s;
	body->s.eType			= ET_BODY;
	body->s.eFlags			= EF_DEAD;
	body->s.gametypeitems	= 0;
	body->s.loopSound		= 0;
	body->s.number			= body - g_entities;
	body->timestamp			= level.time;
	body->physicsObject		= qtrue;
	body->physicsBounce		= 0;
	body->s.otherEntityNum  = ent->s.clientNum;
	g_entities[number].client->sess.zombie = qtrue;
	g_entities[number].client->sess.zombiebody = body->s.number;
	
	//if ( body->s.groundEntityNum == ENTITYNUM_NONE )
	VectorCopy(ent->client->ps.velocity, velo);
	AngleVectors( ent->client->ps.viewangles, velo, NULL, NULL );
	VectorScale( velo, 600, velo );
	body->s.pos.trType = TR_GRAVITY;
	body->s.pos.trTime = level.time;
	velo[2] = 200;
	VectorCopy( velo, body->s.pos.trDelta );

	body->s.event = 0;

	parm  = (DirToByte( direction )&0xFF);
	parm += (hitLocation<<8);
	
	G_AddEvent(body, EV_BODY_QUEUE_COPY, parm);

	body->r.svFlags = ent->r.svFlags | SVF_BROADCAST;
	VectorCopy (ent->r.mins, body->r.mins);
	VectorCopy (ent->r.maxs, body->r.maxs);
	VectorCopy (ent->r.absmin, body->r.absmin);
	VectorCopy (ent->r.absmax, body->r.absmax);

	body->s.torsoAnim = 45;
	
	body->clipmask = CONTENTS_SOLID | CONTENTS_PLAYERCLIP;
	body->r.contents = 0; // CONTENTS_CORPSE;
	body->r.ownerNum = ent->s.number;

	body->nextthink = level.time + 10000;
	body->think = G_FreeEntity;

	body->s.time2 = 0;

	body->die = body_die;
	body->takedamage = qtrue;

	body->s.apos.trBase[PITCH] = 0;

	body->s.pos.trBase[2] = ent->client->ps.origin[2];

	VectorCopy ( body->s.pos.trBase, body->r.currentOrigin );

	trap_LinkEntity (body);

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
					//ent->client->sess.mute = qtrue;
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
			// Boe!Man 12/11/11: Handle this globally.
			ent->client->sess.mute = qtrue;
			return qtrue;
		}
	}
	return qfalse;
}

qboolean RemoveMutedClient(gentity_t *ent){
	int i;
	qboolean unmuted = qfalse;
	for(i=0;i<=20;i++){
		if(level.mutedClients[i].used == qtrue){
			if(strstr(level.mutedClients[i].ip, ent->client->pers.ip)){
				level.mutedClients[i].used = qfalse;
				level.mutedClients[i].time = 0;
				memset(level.mutedClients[i].ip, 0, sizeof(level.mutedClients[i]));
				ent->client->sess.mute = qfalse;
				unmuted = qtrue;
			}
		}
	}
	return unmuted;
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

int StartAfterCommand(char *param){
	int i;
	for(i=0;i<=strlen(param);i++){
		if(param[i] == ' '){
			return i+1;
		}
	}
	return 0;
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
	// Boe!Man 5/14/11: Fix for random spawnpoint in DM/TDM gametypes.
	if(current_gametype.value == GT_DM || current_gametype.value == GT_TDM){
		return;
	}

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
				// Boe!Man 5/27/11: Is the Admin level allowed to spec the opposite team?
				if (g_adminSpec.integer <= ent->client->sess.admin && g_adminSpec.integer != 0 && cm_enabled.integer < 2){
					ent->client->sess.adminspec = qtrue;
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
	}else if(c >= 33 && c <= 47 || c >= 58 && c <= 64 || c >= 91 && c <= 95 || c >= 123 && c <= 125){ // symbols
		return qtrue;
	}else{
		return qfalse;
	}
}

char *ChooseTeam(){
	int counts[3], seekers = 0, maxhiders = 0;
	
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
		for(i=0;i<28-strlen(name);i++){
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

	int rc;

	int table_index;
	sqlite3 * db;
	sqlite3_stmt *stmt;
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
	rc = sqlite3_open("country.db", &db);
	//Com_Printf("Db status: %s\n", sqlite3_errmsg(db));
	if(rc){
		Com_Printf("Database error: %s\n", sqlite3_errmsg(db));
		return;
	}
	//SELECT table_index FROM country_index where 1 BETWEEN begin_ip AND end_ip
	//SELECT ext,country FROM db6 where 1 BETWEEN begin_ip AND end_ip
	rc = sqlite3_prepare(db, va("select table_index from country_index where %u BETWEEN begin_ip AND end_ip", IPnum), -1, &stmt, 0);
	Com_Printf("Query: %s\n", va("select table_index from country_index where %u BETWEEN begin_ip AND end_ip", IPnum));
	if(rc!=SQLITE_OK){
		Com_Printf("Database SQL error: %s\n", sqlite3_errmsg(db));
		return;
	}else while((rc = sqlite3_step(stmt)) != SQLITE_DONE){
		if(rc == SQLITE_ROW){
			table_index = sqlite3_column_int(stmt, 0);
			break;
		}
	}
	rc = sqlite3_prepare(db, va("select country,ext from db%i where %u BETWEEN begin_ip AND end_ip", table_index, IPnum), -1, &stmt, 0);
	Com_Printf("Query: %s\n", va("select country,ext from db%i where %u BETWEEN begin_ip AND end_ip", table_index, IPnum));
	if(rc!=SQLITE_OK){
		Com_Printf("Database SQL error: %s\n", sqlite3_errmsg(db));
		return;
	}else while((rc = sqlite3_step(stmt)) != SQLITE_DONE){
		if(rc == SQLITE_ROW){
			strcpy(ent->client->sess.country, va("%s", sqlite3_column_text(stmt, 0)));
			strcpy(ent->client->sess.countryext, va("%s", sqlite3_column_text(stmt, 1)));
		}
	}
	sqlite3_finalize(stmt);
	sqlite3_close(db);	
	return;
}