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

	//for(i=0;i<=level.numConnectedClients;i++){
	for(i = 0; i <= MAX_CLIENTS; i++){
		if(level.clients[i].pers.connected != CON_CONNECTED)
		{
			continue;
		}

		if(level.clients[i].sess.team == TEAM_RED && (level.clients[i].sess.cageFighter == qtrue || level.mapHighScore == 0)){
			//respawn ( &g_entities[level.sortedClients[i]] );
			if(level.hideseek_cageSize != 1){ // 1 = no cage at all, if this is the cage, don't teleport them to the 'cage'.
				TeleportPlayer(&g_entities[i], spawns[count], level.clients[i].ps.viewangles, qtrue);
				count += 1;
			}
			level.clients[i].ps.stats[STAT_WEAPONS] = 0;
			memset ( level.clients[i].ps.ammo, 0, sizeof(level.clients[i].ps.ammo) );
			memset ( level.clients[i].ps.clip, 0, sizeof(level.clients[i].ps.clip) );
			//g_entities[level.lastalive[0]].client->ps.ammo[weaponData[WP_RPG7_LAUNCHER].attack[ATTACK_NORMAL].ammoIndex]=2;
			level.clients[i].ps.stats[STAT_WEAPONS] |= ( 1 << WP_KNIFE );
			level.clients[i].ps.clip[ATTACK_NORMAL][WP_KNIFE]=1;
			level.clients[i].ps.stats[STAT_WEAPONS] |= ( 1 << WP_AK74_ASSAULT_RIFLE );
			//g_entities[level.lastalive[0]].client->ps.clip[ATTACK_NORMAL][WP_RPG7_LAUNCHER]=1;
			//g_entities[level.lastalive[0]].client->ps.firemode[WP_RPG7_LAUNCHER] = BG_FindFireMode ( WP_RPG7_LAUNCHER, ATTACK_NORMAL, WP_FIREMODE_AUTO );
			level.clients[i].ps.weapon = WP_AK74_ASSAULT_RIFLE;
			level.clients[i].ps.weaponstate = WEAPON_READY;
			level.clients[i].ps.weaponTime = 0;
			level.clients[i].ps.weaponAnimTime = 0;
			level.clients[i].ps.stats[STAT_FROZEN] = 10000;
		}else if(level.clients[i].sess.team == TEAM_BLUE){
			G_Damage (&g_entities[i], NULL, NULL, NULL, NULL, 10000, 0, MOD_POP, HL_HEAD|HL_FOOT_RT|HL_FOOT_LT|HL_LEG_UPPER_RT|HL_LEG_UPPER_LT|HL_HAND_RT|HL_HAND_LT|HL_WAIST|HL_CHEST|HL_NECK);
		}else if(level.clients[i].sess.team != TEAM_SPECTATOR){ // Boe!Man 8/30/11: Means the client is a Red player that wasn't qualified to play in the cage fight. Pop as well.
			G_Damage (&g_entities[i], NULL, NULL, NULL, NULL, 10000, 0, MOD_POP, HL_HEAD|HL_FOOT_RT|HL_FOOT_LT|HL_LEG_UPPER_RT|HL_LEG_UPPER_LT|HL_HAND_RT|HL_HAND_LT|HL_WAIST|HL_CHEST|HL_NECK);
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
	#ifdef _DEBUG
	Com_Printf("Adding ip to list...\n");
	#endif
	len = trap_FS_FOpenFile( va("country\\known\\IP.%s", octetx[0]), &f, FS_APPEND_TEXT );
	if (!f)
	{
		#ifdef _DEBUG
		Com_Printf("^1Error opening File\n");
		#endif
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
	// Boe!Man 11/11/12: Dynamic code, generated by ip2country updater.
	// Last update date: 11/11/12
	if(IPnum >= 16777216 && IPnum <= 387784703){
	part = 0;
	}else if(IPnum >= 387784704 && IPnum <= 523014143){
	part = 1;
	}else if(IPnum >= 523014144 && IPnum <= 630132735){
	part = 2;
	}else if(IPnum >= 630132736 && IPnum <= 702544895){
	part = 3;
	}else if(IPnum >= 702544896 && IPnum <= 786653183){
	part = 4;
	}else if(IPnum >= 786653184 && IPnum <= 1024131071){
	part = 5;
	}else if(IPnum >= 1024131072 && IPnum <= 1065852927){
	part = 6;
	}else if(IPnum >= 1065852928 && IPnum <= 1090207743){
	part = 7;
	}else if(IPnum >= 1090207744 && IPnum <= 1120788479){
	part = 8;
	}else if(IPnum >= 1120788480 && IPnum <= 1157947391){
	part = 9;
	}else if(IPnum >= 1157947392 && IPnum <= 1209884671){
	part = 10;
	}else if(IPnum >= 1209884672 && IPnum <= 1266139135){
	part = 11;
	}else if(IPnum >= 1266139136 && IPnum <= 1315905535){
	part = 12;
	}else if(IPnum >= 1315905536 && IPnum <= 1350369279){
	part = 13;
	}else if(IPnum >= 1350369280 && IPnum <= 1383120895){
	part = 14;
	}else if(IPnum >= 1383120896 && IPnum <= 1425063935){
	part = 15;
	}else if(IPnum >= 1425063936 && IPnum <= 1476165631){
	part = 16;
	}else if(IPnum >= 1476165632 && IPnum <= 1533689855){
	part = 17;
	}else if(IPnum >= 1533689856 && IPnum <= 1539748351){
	part = 18;
	}else if(IPnum >= 1539748352 && IPnum <= 1540365311){
	part = 19;
	}else if(IPnum >= 1540365312 && IPnum <= 1540722431){
	part = 20;
	}else if(IPnum >= 1540722432 && IPnum <= 1541194239){
	part = 21;
	}else if(IPnum >= 1541194240 && IPnum <= 1541696511){
	part = 22;
	}else if(IPnum >= 1541696512 && IPnum <= 1542175743){
	part = 23;
	}else if(IPnum >= 1542175744 && IPnum <= 1543241727){
	part = 24;
	}else if(IPnum >= 1543241728 && IPnum <= 1583763455){
	part = 25;
	}else if(IPnum >= 1583763456 && IPnum <= 1605533695){
	part = 26;
	}else if(IPnum >= 1605533696 && IPnum <= 1728512511){
	part = 27;
	}else if(IPnum >= 1728512512 && IPnum <= 1729586175){
	part = 28;
	}else if(IPnum >= 1729586176 && IPnum <= 1836957695){
	part = 29;
	}else if(IPnum >= 1836957696 && IPnum <= 1897176063){
	part = 30;
	}else if(IPnum >= 1897176064 && IPnum <= 1986461695){
	part = 31;
	}else if(IPnum >= 1986461696 && IPnum <= 2070679551){
	part = 32;
	}else if(IPnum >= 2070679552 && IPnum <= 2172518399){
	part = 33;
	}else if(IPnum >= 2172518400 && IPnum <= 2254079999){
	part = 34;
	}else if(IPnum >= 2254080000 && IPnum <= 2343632895){
	part = 35;
	}else if(IPnum >= 2343632896 && IPnum <= 2411724799){
	part = 36;
	}else if(IPnum >= 2411724800 && IPnum <= 2486370303){
	part = 37;
	}else if(IPnum >= 2486370304 && IPnum <= 2556231679){
	part = 38;
	}else if(IPnum >= 2556231680 && IPnum <= 2660368383){
	part = 39;
	}else if(IPnum >= 2660368384 && IPnum <= 2723414015){
	part = 40;
	}else if(IPnum >= 2723414016 && IPnum <= 2822832127){
	part = 41;
	}else if(IPnum >= 2822832128 && IPnum <= 2928459775){
	part = 42;
	}else if(IPnum >= 2928459776 && IPnum <= 2987479039){
	part = 43;
	}else if(IPnum >= 2987479040 && IPnum <= 3033661439){
	part = 44;
	}else if(IPnum >= 3033661440 && IPnum <= 3104376831){
	part = 45;
	}else if(IPnum >= 3104376832 && IPnum <= 3188211711){
	part = 46;
	}else if(IPnum >= 3188211712 && IPnum <= 3222036991){
	part = 47;
	}else if(IPnum >= 3222036992 && IPnum <= 3223581695){
	part = 48;
	}else if(IPnum >= 3223581696 && IPnum <= 3224779007){
	part = 49;
	}else if(IPnum >= 3224779008 && IPnum <= 3225860607){
	part = 50;
	}else if(IPnum >= 3225860608 && IPnum <= 3226751487){
	part = 51;
	}else if(IPnum >= 3226751488 && IPnum <= 3227555839){
	part = 52;
	}else if(IPnum >= 3227555840 && IPnum <= 3228375295){
	part = 53;
	}else if(IPnum >= 3228375296 && IPnum <= 3229955327){
	part = 54;
	}else if(IPnum >= 3229955328 && IPnum <= 3230921727){
	part = 55;
	}else if(IPnum >= 3230921728 && IPnum <= 3231469823){
	part = 56;
	}else if(IPnum >= 3231469824 && IPnum <= 3233487359){
	part = 57;
	}else if(IPnum >= 3233488896 && IPnum <= 3234197247){
	part = 58;
	}else if(IPnum >= 3234197248 && IPnum <= 3236410879){
	part = 59;
	}else if(IPnum >= 3236410880 && IPnum <= 3238598399){
	part = 60;
	}else if(IPnum >= 3238598400 && IPnum <= 3239938815){
	part = 61;
	}else if(IPnum >= 3239938816 && IPnum <= 3240758527){
	part = 62;
	}else if(IPnum >= 3240758528 && IPnum <= 3243514623){
	part = 63;
	}else if(IPnum >= 3243514624 && IPnum <= 3245181951){
	part = 64;
	}else if(IPnum >= 3245181952 && IPnum <= 3248775167){
	part = 65;
	}else if(IPnum >= 3248775168 && IPnum <= 3251194879){
	part = 66;
	}else if(IPnum >= 3251194880 && IPnum <= 3254704639){
	part = 67;
	}else if(IPnum >= 3254704640 && IPnum <= 3256416255){
	part = 68;
	}else if(IPnum >= 3256416256 && IPnum <= 3258013695){
	part = 69;
	}else if(IPnum >= 3258013696 && IPnum <= 3260293119){
	part = 70;
	}else if(IPnum >= 3260293120 && IPnum <= 3263134207){
	part = 71;
	}else if(IPnum >= 3263134208 && IPnum <= 3266343935){
	part = 72;
	}else if(IPnum >= 3266343936 && IPnum <= 3272921599){
	part = 73;
	}else if(IPnum >= 3272921600 && IPnum <= 3276677119){
	part = 74;
	}else if(IPnum >= 3276677120 && IPnum <= 3281190911){
	part = 75;
	}else if(IPnum >= 3281190912 && IPnum <= 3285721087){
	part = 76;
	}else if(IPnum >= 3285721088 && IPnum <= 3288572671){
	part = 77;
	}else if(IPnum >= 3288572672 && IPnum <= 3323054847){
	part = 78;
	}else if(IPnum >= 3323054848 && IPnum <= 3325998079){
	part = 79;
	}else if(IPnum >= 3325998080 && IPnum <= 3330713343){
	part = 80;
	}else if(IPnum >= 3330713344 && IPnum <= 3331997695){
	part = 81;
	}else if(IPnum >= 3331997696 && IPnum <= 3333682431){
	part = 82;
	}else if(IPnum >= 3333682432 && IPnum <= 3335134975){
	part = 83;
	}else if(IPnum >= 3335134976 && IPnum <= 3338469119){
	part = 84;
	}else if(IPnum >= 3338469120 && IPnum <= 3341484287){
	part = 85;
	}else if(IPnum >= 3341484288 && IPnum <= 3344536319){
	part = 86;
	}else if(IPnum >= 3344536320 && IPnum <= 3349688319){
	part = 87;
	}else if(IPnum >= 3349688320 && IPnum <= 3351888639){
	part = 88;
	}else if(IPnum >= 3351888640 && IPnum <= 3354976511){
	part = 89;
	}else if(IPnum >= 3354976512 && IPnum <= 3356157951){
	part = 90;
	}else if(IPnum >= 3356157952 && IPnum <= 3360808959){
	part = 91;
	}else if(IPnum >= 3360808960 && IPnum <= 3389465343){
	part = 92;
	}else if(IPnum >= 3389465344 && IPnum <= 3391537151){
	part = 93;
	}else if(IPnum >= 3391537152 && IPnum <= 3393541119){
	part = 94;
	}else if(IPnum >= 3393541120 && IPnum <= 3397648383){
	part = 95;
	}else if(IPnum >= 3397648384 && IPnum <= 3405950975){
	part = 96;
	}else if(IPnum >= 3405950976 && IPnum <= 3406864895){
	part = 97;
	}else if(IPnum >= 3406864896 && IPnum <= 3407371775){
	part = 98;
	}else if(IPnum >= 3407371776 && IPnum <= 3407804927){
	part = 99;
	}else if(IPnum >= 3407804928 && IPnum <= 3409537023){
	part = 100;
	}else if(IPnum >= 3409537024 && IPnum <= 3415495679){
	part = 101;
	}else if(IPnum >= 3415495680 && IPnum <= 3423439871){
	part = 102;
	}else if(IPnum >= 3423439872 && IPnum <= 3427033087){
	part = 103;
	}else if(IPnum >= 3427033088 && IPnum <= 3429584383){
	part = 104;
	}else if(IPnum >= 3429584384 && IPnum <= 3432099839){
	part = 105;
	}else if(IPnum >= 3432099840 && IPnum <= 3438178559){
	part = 106;
	}else if(IPnum >= 3438178560 && IPnum <= 3450232831){
	part = 107;
	}else if(IPnum >= 3450232832 && IPnum <= 3453159935){
	part = 108;
	}else if(IPnum >= 3453159936 && IPnum <= 3464687871){
	part = 109;
	}else if(IPnum >= 3464687872 && IPnum <= 3475996671){
	part = 110;
	}else if(IPnum >= 3475996672 && IPnum <= 3494387711){
	part = 111;
	}else if(IPnum >= 3494387712 && IPnum <= 3495778303){
	part = 112;
	}else if(IPnum >= 3495778304 && IPnum <= 3519889407){
	part = 113;
	}else if(IPnum >= 3519889408 && IPnum <= 3558375423){
	part = 114;
	}else if(IPnum >= 3558375424 && IPnum <= 3573022719){
	part = 115;
	}else if(IPnum >= 3573022720 && IPnum <= 3589079039){
	part = 116;
	}else if(IPnum >= 3589079040 && IPnum <= 3635437567){
	part = 117;
	}else if(IPnum >= 3635437568 && IPnum <= 3645390847){
	part = 118;
	}else if(IPnum >= 3645390848 && IPnum <= 3697590271){
	part = 119;
	}else{
	part = 120;
	}
	// End Boe!Man 11/11/12
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
				#ifdef _DEBUG
				Com_Printf("Found country\n");
				#endif
				strcpy(ent->client->sess.country, GetCountry(ext));
				strcpy(ent->client->sess.countryext, ext);
				return;
				break; // stop searching
			}
			group = trap_GPG_GetNext(group);
		}
	}else{
		//trap_SendServerCommand( -1, va("print \"^3[Debug]^7 File not found\n\"") );
		#ifdef _DEBUG
		G_LogPrintf("IP2Country Debug: File not found\n"); // Boe 12/23/10: Append to console & log to file rather than sending debug information to all clients.
		#endif
		
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
