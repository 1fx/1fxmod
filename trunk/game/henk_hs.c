#include "g_local.h"
#include "boe_local.h"

// Henk 20/02/10 -> Add UpdateScores()
void ShowScores(void)
{
	char winner[128];
	int i;

	if(strstr(level.cagewinner, va("%s", server_seekerteamprefix.string))){ // Boe!Man 8/7/12: Meaning the seekers won, display msg.
		Com_sprintf(winner, sizeof(winner), "%s\n\n", level.cagewinner);
	}else if(!strstr(level.cagewinner, "none")){
		Com_sprintf(winner, sizeof(winner), "%s ^7won the round!\n\n", level.cagewinner);
	}else{ // This shouldn't happen anymore.
		memset(winner, 0, sizeof(winner));
	}

	for ( i = 0; i < level.numConnectedClients; i ++ )
	{
		// Boe!Man 9/2/12: Advanced H&S statistics.
		if(hideSeek_ExtendedRoundStats.integer && level.time > level.awardTime + 8000 && level.awardTime){
			trap_SendServerCommand( g_entities[level.sortedClients[i]].s.number, va("cp \"@^3%s\n\n^3Statistics for this map:\n\n"
					"^_Rounds survived: ^3%i ^_by ^3%s\n"
					"^_MM1 hits taken: ^3%i ^_by ^3%s\n"
					"^_RPG boosts: ^3%i ^_by ^3%s\n"
					"^_Taken RPG: ^3%i ^_by ^3%s\n"
					"^_Taken M4: ^3%i ^_by ^3%s\n"
					"^_Stun attacks: ^3%i ^_by ^3%s\n"
					"^_Seekers caged: ^3%i ^_by ^3%s\n"
					"^_Weapons stolen: ^3%i ^_by ^3%s\n\n"
					"^yPoints: ^3%i ^yby ^3%s\n"
					"^yTaken MM1: ^3%i ^yby ^3%s\n"
					"^yStunned: ^3%i ^yby ^3%s\n"
					"^yTrapped in cage: ^3%i ^yby ^3%s\n\"",
					g_motd.string,
					level.advancedHsScores[0].score, level.advancedHsScores[0].name, level.advancedHsScores[1].score, level.advancedHsScores[1].name, level.advancedHsScores[2].score, level.advancedHsScores[2].name, level.advancedHsScores[3].score, level.advancedHsScores[3].name,
					level.advancedHsScores[4].score, level.advancedHsScores[4].name, level.advancedHsScores[5].score, level.advancedHsScores[5].name, level.advancedHsScores[6].score, level.advancedHsScores[6].name, level.advancedHsScores[7].score, level.advancedHsScores[7].name,
					level.advancedHsScores[8].score, level.advancedHsScores[8].name, level.advancedHsScores[9].score, level.advancedHsScores[9].name, level.advancedHsScores[10].score, level.advancedHsScores[10].name, level.advancedHsScores[11].score, level.advancedHsScores[11].name
			));
		}else{
			trap_SendServerCommand( g_entities[level.sortedClients[i]].s.number, va("cp \"@^3%s\n\n%s^_ THE 3 BEST HIDERS IN THIS MAP ARE:\n\n^31st ^7%s with ^3%i ^7wins.\n^+2nd ^7%s with ^+%i ^7wins.\n^@3rd ^7%s with ^@%i ^7wins.\n\n"
					"^y THE 3 BEST SEEKERS IN THIS MAP ARE:\n\n^31st ^7%s with ^3%i ^7kills.\n^+2nd ^7%s with ^+%i ^7kills.\n^@3rd ^7%s with ^@%i ^7kills.\n\"",
					g_motd.string, winner,
					level.firstname, level.firstscore, level.secondname, level.secondscore, level.thirdname, level.thirdscore,
					level.Sfirstname, level.Sfirstscore, level.Ssecondname, level.Ssecondscore, level.Sthirdname, level.Sthirdscore));
		}
	}
}

int Henk_GetScore (qboolean seekers)
{
	int             len;
	fileHandle_t	f;
	char            buf[15000];
	char			listName[64];
	char			*listP = listName;
	char			*bufP = buf;
	char			*file;
	int number = 0, highestscore = 0;
	char			mapname[64];
	char			strScore[12];
	int i;
	struct {
		char	name[64];
		int		score;
	} Scores[128];
	int count;
	int firstnum = 0, secondnum = 0;
	
	trap_Cvar_VariableStringBuffer ( "mapname", mapname, MAX_QPATH );
	if(seekers)
		file = va("scores/s_%s.scores", mapname);
	else
		file = va("scores/h_%s.scores", mapname);
	len = trap_FS_FOpenFile( file, &f, FS_READ_TEXT); 

	if (!f) { 
		len = trap_FS_FOpenFile( file, &f, FS_APPEND_TEXT);   
		if (!f) {  
			Boe_FileError(NULL, file);
			return -1;
		}
		trap_FS_FCloseFile( f ); 
		len = trap_FS_FOpenFile( file, &f, FS_READ_TEXT); 

		if (!f) { 
			Boe_FileError(NULL, file);
			return -1;
		}
	}
	if(f >= 5){
		Com_Printf("%i of 64 max handles used\n", f);
		G_LogPrintf( "!=!=!=!=!=!=!=!=WARNING=!=!=!=!=!=!=!=!=!=! File handles are not closing properly  [handle count: ( %i )]\n", f );
	}

	if(len > 15000)	{
		len = 15000;
	}
	memset( buf, 0, sizeof(buf) );
	trap_FS_Read( buf, len, f );
	buf[len] = '\0';
	trap_FS_FCloseFile( f );

	while( *bufP != '\0') {
		while(*bufP != '\n' && *bufP != '\0') {
			*listP++ = *bufP++;
		}

		*listP = '\0';
		listP = listName;			
		Com_Printf("listName: %s\n", listName);
		for(i=0;i<strlen(listName);i++){
			if(listName[i] == ':'){
				Q_strncpyz(Scores[number].name, listName, i+1);
				Q_strncpyz(strScore, (listName+i)+1, strlen(listName)-i);
				//Com_Printf("%s - %i\n", strScore, strlen(listName)-i);
			}
		}
		Scores[number].score = atoi(strScore);
		Com_Printf("Found score -> name: %s -> score -> %i\n", Scores[number].name, Scores[number].score);	
		number += 1;
			
		while(*bufP == '\n') {
			bufP++;
		}
	}	
	// start parsing top 3 hiders
	for(count=0;count<number;count++){
		if(Scores[count].score >= highestscore){
			if(seekers){
				strcpy(level.Sfirstname, Scores[count].name);
				level.Sfirstscore = Scores[count].score;
				firstnum = count;
			}else{
				strcpy(level.firstname, Scores[count].name);
				level.firstscore = Scores[count].score;
			}
			highestscore = Scores[count].score;
		}
	}
	highestscore = 0;
	for(count=0;count<number;count++){
		if(seekers){
			if(Scores[count].score > highestscore && count != firstnum){
				strcpy(level.Ssecondname, Scores[count].name);
				level.Ssecondscore = Scores[count].score;
				highestscore = Scores[count].score;
				secondnum = count;
			}
		}else{
			if(Scores[count].score >= highestscore && !strstr(Scores[count].name, level.firstname)){
				strcpy(level.secondname, Scores[count].name);
				level.secondscore = Scores[count].score;
				highestscore = Scores[count].score;
			}
		}
	}
	highestscore = 0;
	for(count=0;count<number;count++){
		if(seekers){
			if(Scores[count].score > highestscore && count != firstnum && count != secondnum){
				strcpy(level.Sthirdname, Scores[count].name);
				level.Sthirdscore = Scores[count].score;
				highestscore = Scores[count].score;
			}
		}else{
			if(Scores[count].score >= highestscore && !strstr(Scores[count].name, level.secondname) && !strstr(Scores[count].name, level.firstname)){
				strcpy(level.thirdname, Scores[count].name);
				level.thirdscore = Scores[count].score;
				highestscore = Scores[count].score;
			}
		}
	}
	if(strlen(level.firstname) < 1){
		strcpy(level.firstname, "none");
	}
	if(strlen(level.secondname) < 1){
		strcpy(level.secondname, "none");
	}
	if(strlen(level.thirdname) < 1){
		strcpy(level.thirdname, "none");
	}
	if(strlen(level.Sfirstname) < 1){
		strcpy(level.Sfirstname, "none");
	}
	if(strlen(level.Ssecondname) < 1){
		strcpy(level.Ssecondname, "none");
	}
	if(strlen(level.Sthirdname) < 1){
		strcpy(level.Sthirdname, "none");
	}
	Com_Printf("Top 3 Hiders\n%s\n%s\n%s\n", level.firstname, level.secondname, level.thirdname);
	Com_Printf("Top 3 Seekers\n%s\n%s\n%s\n", level.Sfirstname, level.Ssecondname, level.Sthirdname);
	// end

	return 0;
}

// Boe!Man 8/7/12: Small recode of UpdateScores.
// Only seekers should be added they way they're added now, with full scores, in the files. 
// The winning hider should only be written to the file, do this later on (after we're done with the seekers etc., this also gives us time to sort the scores).
// Boe!Man 9/2/12: The advanced score table is also added in this function as of now.
void UpdateScores(void)
{
	int i;
	char mapname[64];
	char filename[128];
	int oldscore = 0;
	char *SearchStr;
	// Boe!Man 8/7/12: Hider stuff.
	int highestScore = 0;
	int highestHider; // level.sortedClients[i]
	// Boe!Man 8/7/12: Stuff for the file buffer, to check if there's a duplicate.
	int len = 0;
	fileHandle_t f;
	char buf[5000];
	
	// Boe!Man 8/7/12: Get the filename.
	trap_Cvar_VariableStringBuffer ( "mapname", mapname, MAX_QPATH );
	strcpy(filename, va("scores/s_%s.scores", mapname));
	
	// Boe!Man 8/7/12: Read file to buffer.
	len = trap_FS_FOpenFile( filename, &f, FS_READ_TEXT);
	
	if (!f) { 
		Boe_FileError(NULL, filename);
		Henk_GetScore(qfalse);
		Henk_GetScore(qtrue);
		return;
	}
	
	if(len > 5000){
		len = 5000;
	}
	
	// Read to buffer.
	memset(buf, 0, sizeof(buf));
	trap_FS_Read(buf, len, f);
	buf[len] = '\0';
	trap_FS_FCloseFile(f);
	
	// Next loop the clients, check for scores of red + blue.
	for ( i = 0; i < level.numConnectedClients; i ++ )
	{
		gentity_t* ent = &g_entities[level.sortedClients[i]];
		
		// Boe!Man 9/2/12: This is shitty, check for the advanced H&S scores. Kinda a performance hit.
		// MM1 Hits.
		if(ent->client->sess.MM1HitsTaken > level.advancedHsScores[1].score){
			level.advancedHsScores[1].score = ent->client->sess.MM1HitsTaken;
			strcpy(level.advancedHsScores[1].name, Q_strlwr(ent->client->pers.cleanName));
		}
		// RPG boosts.
		if(ent->client->sess.RPGBoosts > level.advancedHsScores[2].score){
			level.advancedHsScores[2].score = ent->client->sess.RPGBoosts;
			strcpy(level.advancedHsScores[2].name, Q_strlwr(ent->client->pers.cleanName));
		}
		// Taken RPG.
		if(ent->client->sess.takenRPG > level.advancedHsScores[3].score){
			level.advancedHsScores[3].score = ent->client->sess.takenRPG;
			strcpy(level.advancedHsScores[3].name, Q_strlwr(ent->client->pers.cleanName));
		}
		// Taken M4.
		if(ent->client->sess.takenM4 > level.advancedHsScores[4].score){
			level.advancedHsScores[4].score = ent->client->sess.takenM4;
			strcpy(level.advancedHsScores[4].name, Q_strlwr(ent->client->pers.cleanName));
		}
		// Stun attacks.
		if(ent->client->sess.stunAttacks > level.advancedHsScores[5].score){
			level.advancedHsScores[5].score = ent->client->sess.stunAttacks;
			strcpy(level.advancedHsScores[5].name, Q_strlwr(ent->client->pers.cleanName));
		}
		// Seekers caged.
		if(ent->client->sess.seekersCaged > level.advancedHsScores[6].score){
			level.advancedHsScores[6].score = ent->client->sess.seekersCaged;
			strcpy(level.advancedHsScores[6].name, Q_strlwr(ent->client->pers.cleanName));
		}
		// Weapons stolen.
		if(ent->client->sess.weaponsStolen > level.advancedHsScores[7].score){
			level.advancedHsScores[7].score = ent->client->sess.weaponsStolen;
			strcpy(level.advancedHsScores[7].name, Q_strlwr(ent->client->pers.cleanName));
		}
		// Taken MM1.
		if(ent->client->sess.takenMM1 > level.advancedHsScores[9].score){
			level.advancedHsScores[9].score = ent->client->sess.takenMM1;
			strcpy(level.advancedHsScores[9].name, Q_strlwr(ent->client->pers.cleanName));
		}
		// Stunned.
		if(ent->client->sess.stunned > level.advancedHsScores[10].score){
			level.advancedHsScores[10].score = ent->client->sess.stunned;
			strcpy(level.advancedHsScores[10].name, Q_strlwr(ent->client->pers.cleanName));
		}
		// Trapped in cage.
		if(ent->client->sess.trappedInCage > level.advancedHsScores[11].score){
			level.advancedHsScores[11].score = ent->client->sess.trappedInCage;
			strcpy(level.advancedHsScores[11].name, Q_strlwr(ent->client->pers.cleanName));
		}
		
		// Boe!Man 9/2/12: Don't continue just yet on these two, we need those advanced stats as well.
		if(ent->client->sess.kills == 0){
			continue;
		}
		if(ent->client->sess.team == TEAM_SPECTATOR){ // Boe!Man 8/7/12: Continue on spec.
			continue;
		}
		
		if(ent->client->sess.team == TEAM_BLUE){
			oldscore = Boe_NameListCheck ( 0, Q_strlwr(ent->client->pers.cleanName), filename, NULL, qfalse, qfalse, qfalse, qtrue, qfalse);
			SearchStr = va("%s:%i", Q_strlwr(ent->client->pers.cleanName), ent->client->sess.kills);
			if(!strstr(buf, SearchStr)){ // To avoid duplicates.
				Boe_AddToList(SearchStr, filename, "Score", NULL);
			}
			// Boe!Man 9/2/12: Also check for the highest blue player.
			if(ent->client->sess.kills > level.advancedHsScores[8].score){
				level.advancedHsScores[8].score = ent->client->sess.kills;
				strcpy(level.advancedHsScores[8].name, Q_strlwr(ent->client->pers.cleanName));
			}
		}else{ // Red team.
			if(ent->client->sess.kills > highestScore){
				highestHider = i;
				highestScore = ent->client->sess.kills;
			}
		}
	}
	
	// Boe!Man 9/2/12: For the advanced H&S scoretable, check for things that aren't achieved by no-one and fill those slots with data.
	for(i = 1; i < 12; i++){
		if(!level.advancedHsScores[i].score){
			strcpy(level.advancedHsScores[i].name, "none");
		}
	}
	
	// Write highest hider score.
	if(highestScore){
		strcpy(filename, va("scores/h_%s.scores", mapname));
		
		oldscore = Boe_NameListCheck ( 0, Q_strlwr(g_entities[level.sortedClients[highestHider]].client->pers.cleanName), filename, NULL, qfalse, qfalse, qfalse, qtrue, qfalse);
		Boe_Remove_from_list(Q_strlwr(g_entities[level.sortedClients[highestHider]].client->pers.cleanName), filename, "Score", NULL, qfalse, qfalse, qtrue);
		SearchStr = va("%s:%i", Q_strlwr(g_entities[level.sortedClients[highestHider]].client->pers.cleanName), oldscore+1); // Add one, instead of his score.
		Boe_AddToList(SearchStr, filename, "Score", NULL);
		// Boe!Man 9/2/12: Also add his score to the advanced table.
		level.advancedHsScores[0].score = g_entities[level.sortedClients[highestHider]].client->sess.kills;
		strcpy(level.advancedHsScores[0].name, Q_strlwr(g_entities[level.sortedClients[highestHider]].client->pers.cleanName));
	}else{ // highestScore wasn't filled.. Seekers won this time.
		strcpy(level.cagewinner, va("%s ^7won the round!", server_seekerteamprefix.string));
		strcpy(level.advancedHsScores[0].name, "none");
	}
		
	Henk_GetScore(qfalse);
	Henk_GetScore(qtrue);
}

// 14/01/10: Custom commands by Henk
void SpawnCage(vec3_t org, gentity_t *ent, qboolean autoremove, qboolean big) // Spawn a cage
{
	// Boe!Man 6/22/12: Addition for big cage.
	char			*origin;
	int part, numb;
	
	for(part=1;part<=4;part++){
		AddSpawnField("classname", "misc_bsp"); // blocker
		AddSpawnField("bspmodel",	"instances/Generic/fence01");
		if(part == 1){
			if(!big){
				origin = va("%.0f %.0f %.0f",
					org[0], 
					org[1]+126, 
				org[2] - 50);
			}else{
				origin = va("%.0f %.0f %.0f",
					org[0]+126, 
					org[1]+252, 
				org[2] - 50);
			}
			AddSpawnField("origin",		origin);
			AddSpawnField("angles",		"0 360 0");
		}else if(part == 2){
			if(!big){
				origin = va("%.0f %.0f %.0f",
					org[0], 
					org[1]-126, 
				org[2] - 50);
			}else{
				origin = va("%.0f %.0f %.0f",
					org[0]-126, 
					org[1]-252, 
				org[2] - 50);
			}
			AddSpawnField("origin",		origin);
			AddSpawnField("angles",		"0 180 0");
		}else if(part == 3){
			if(!big){
				origin = va("%.0f %.0f %.0f",
					org[0]-126, 
					org[1], 
				org[2] - 50);
			}else{
				origin = va("%.0f %.0f %.0f",
					org[0]-252, 
					org[1]-126, 
				org[2] - 50);
			}
			AddSpawnField("origin",		origin);
			AddSpawnField("angles",		"0 -270 0");
		}else if(part == 4){
			if(!big){
				origin = va("%.0f %.0f %.0f",
					org[0]+126, 
					org[1], 
				org[2] - 50);
			}else{
				origin = va("%.0f %.0f %.0f",
					org[0]+252, 
					org[1]+126, 
				org[2] - 50);
			}
			AddSpawnField("origin",		origin);
			AddSpawnField("angles",		"0 -90 0");
		}
		AddSpawnField("model",		"trigger_hurt"); //blocked_trigger
		AddSpawnField("count",		 "1");
		AddSpawnField("hideseek",	 "1");
		numb = G_SpawnGEntityFromSpawnVars(qtrue);
		if(numb != -1 && !autoremove){
			g_entities[numb].think = G_FreeEntity;
			g_entities[numb].nextthink = level.time+12000;
		}
		
		level.numSpawnVars = 0;
		level.numSpawnVarChars = 0;
		
		if(big){ // Boe!Man 6/22/12: Spawn the additional fences.
			AddSpawnField("classname", "misc_bsp"); // blocker
			AddSpawnField("bspmodel",	"instances/Generic/fence01");
			if(part == 1){
				origin = va("%.0f %.0f %.0f",
					org[0]-126, 
					org[1]+252,
				org[2] - 50);
				AddSpawnField("origin",		origin);
				AddSpawnField("angles",		"0 360 0");
			}else if(part == 2){
				origin = va("%.0f %.0f %.0f",
					org[0]+126, 
					org[1]-252,
				org[2] - 50);
				AddSpawnField("origin",		origin);
				AddSpawnField("angles",		"0 180 0");
			}else if(part == 3){
				origin = va("%.0f %.0f %.0f",
					org[0]-252, 
					org[1]+126, 
				org[2] - 50);
				AddSpawnField("origin",		origin);
				AddSpawnField("angles",		"0 -270 0");
			}else if(part == 4){
				origin = va("%.0f %.0f %.0f",
					org[0]+252, 
					org[1]-126, 
				org[2] - 50);
				AddSpawnField("origin",		origin);
				AddSpawnField("angles",		"0 -90 0");
			}
			AddSpawnField("model",		"trigger_hurt"); //blocked_trigger
			AddSpawnField("count",		 "1");
			AddSpawnField("hideseek",	 "1");
			numb = G_SpawnGEntityFromSpawnVars(qtrue);
			if(numb != -1 && !autoremove){
				g_entities[numb].think = G_FreeEntity;
				g_entities[numb].nextthink = level.time+12000;
			}
			
			level.numSpawnVars = 0;
			level.numSpawnVarChars = 0;
		}
	}
}

void SpawnBox		 (vec3_t org)
{
	char			*origin;
	int				numb;
	origin = va("%.0f %.0f %.0f", org[0], org[1], org[2]);// - (ent->client->ps.viewheight));

	AddSpawnField("classname", "misc_bsp"); // blocker
	AddSpawnField("bspmodel",	"instances/Colombia/npc_jump1");
	AddSpawnField("origin",		origin);
	AddSpawnField("angles",		"0 98 0");
	AddSpawnField("model",		"trigger_hurt"); //blocked_trigger
	AddSpawnField("count",		 "1");
	AddSpawnField("hideseek",	 "1");

	numb = G_SpawnGEntityFromSpawnVars(qtrue);
	if(numb != -1){
	g_entities[numb].think = G_FreeEntity;
	g_entities[numb].nextthink = level.time+10000;
	}
	level.numSpawnVars = 0;
	level.numSpawnVarChars = 0;
}	

int SpawnBoxEx(vec3_t org, vec3_t ang)
{
		char			*origin;
		char			*angles;
	origin = va("%.0f %.0f %.0f", org[0], org[1], org[2]);
	angles = va("%.0f %.0f %.0f", ang[0], ang[1], ang[2]);

	AddSpawnField("classname", "misc_bsp"); // blocker
	AddSpawnField("bspmodel",	"instances/Colombia/npc_jump1");
	AddSpawnField("origin",		origin);
	AddSpawnField("angles",		angles);
	AddSpawnField("model",		"trigger_hurt"); //blocked_trigger
	AddSpawnField("count",		 "1");

	return G_SpawnGEntityFromSpawnVars(qtrue);
}

void Effect (vec3_t org, char * name, qboolean rpg)
{
	char			*origin;
	origin = va("%.0f %.0f %.0f", org[0], org[1], org[2]);
	AddSpawnField("classname", "1fx_play_effect");
	if(strstr(name, "flare_red")){
	AddSpawnField("effect",	name);
	if(rpg == qtrue){
		AddSpawnField("rpg", "true");
	}else{
		AddSpawnField("rpg", "false");
	}
	}else{
	AddSpawnField("effect",	name);
	}
	AddSpawnField("origin",		origin);
	AddSpawnField("wait",		"3");
	AddSpawnField("count",		 "-1");

	G_SpawnGEntityFromSpawnVars(qtrue);

	level.numSpawnVars = 0;
	level.numSpawnVarChars = 0;
}	

void DoTeleport(gentity_t *ent, vec3_t origin){
	vec3_t angles;
	char *origin1;
	VectorCopy(ent->pos1, angles);
	origin[2] += 50;
	TeleportPlayer(ent, origin, ent->client->ps.viewangles, qtrue);
	origin1 = va("%.0f %.0f %.0f", origin[0], origin[1], origin[2]);
	//G_PlayEffect ( G_EffectIndex("effects/explosions/col9_boat_explosion"),origin, angles);
	AddSpawnField("classname", "1fx_play_effect");
	AddSpawnField("effect",	"effects/explosions/col9_boat_explosion");
	AddSpawnField("origin", origin1);
	AddSpawnField("wait", "1");
	AddSpawnField("count", "1");

	G_SpawnGEntityFromSpawnVars(qtrue);
}

// Henk 31/03/11 -> New M4 give away code
int GetM4Winner(int rpgwinner){
	int i;
	int winners[32];
	int winnercount = 0;
	int deaths[32];
	int deathcount = 0; // id
	int timeofdeath = 0; // time

	if(TeamCount1(TEAM_RED) < 2)
		return 100;

	for ( i = 0; i < level.numConnectedClients; i ++ )
	{
		gentity_t* ent = &g_entities[level.sortedClients[i]];
		if(ent->client->sess.team != TEAM_RED)
			continue;
		if(G_IsClientDead(ent->client))
			continue;
		if(ent->client->pers.connected != CON_CONNECTED)
			continue;

		if(ent->client->sess.timeOfDeath == 1 && ent->s.number != rpgwinner){ // this guy really won the round
			winners[winnercount] = ent->s.number;
			winnercount += 1;
		}else if(ent->client->sess.timeOfDeath == 0){ // weird case didn't die and won
			continue;
		}else if(ent->client->sess.timeOfDeath > 1){ // this is the time at which he died
			if(ent->client->sess.timeOfDeath > timeofdeath && ent->s.number != rpgwinner){
				timeofdeath = ent->client->sess.timeOfDeath;
				deaths[deathcount] = ent->s.number;
				deathcount += 1;
			}else if(ent->client->sess.timeOfDeath == timeofdeath){// multiple
				// probably never happens
			}
		}
	}
	if(winnercount == 1){
		return winners[0];
	}else if(winnercount == 0 && deathcount >= 1){
		//return clients[irand(0, deaths-1)]; 
		return deaths[deathcount-1]; // Boe!Man 8/18/11: return the last person that got written to the integer, i.e. the one that lasted the longest.
	}else if(winnercount > 1){
		return winners[irand(0, winnercount-1)];
	}

	return -1; // fail
}

// Henk 30/03/11 -> New RPG give away code
int GetRpgWinner(void){
	int i;
	int winners[32];
	int winnercount = 0;
	int deaths[32];
	int deathcount = 0; // id
	int timeofdeath = 0; // time
	if(TeamCount1(TEAM_RED) < 1)
		return 100;
	for ( i = 0; i < level.numConnectedClients; i ++ )
	{
		gentity_t* ent = &g_entities[level.sortedClients[i]];
		if(ent->client->sess.team != TEAM_RED)
			continue;
		if(G_IsClientDead(ent->client))
			continue;
		if(ent->client->pers.connected != CON_CONNECTED)
			continue;

		if(ent->client->sess.timeOfDeath == 1){ // this guy really won the round
			winners[winnercount] = ent->s.number; // Boe!Man 8/18/11: Write it to a separate integer, or else it'll overwrite something from the death integer or vice versa..
			winnercount += 1;
		}else if(ent->client->sess.timeOfDeath == 0){ // weird case didn't die and won
			continue;
		}else if(ent->client->sess.timeOfDeath > 1){ // this is the time at which he died
			if(ent->client->sess.timeOfDeath > timeofdeath){
				timeofdeath = ent->client->sess.timeOfDeath;
				deaths[deathcount] = ent->s.number;
				deathcount += 1;
			}else if(ent->client->sess.timeOfDeath == timeofdeath){// multiple
				// probably never happens
			}
		}
	}

	if(winnercount == 1){
		return winners[0];
	}else if(winnercount == 0 && deathcount >= 1){
		// Boe!Man 8/18/11: No no no no! Don't return a random value, we need to sort out which client lasted the longest!
		//return clients[irand(0, deaths-1)];
		return deaths[deathcount-1]; // So return the last person that got written to the integer, i.e. the one that lasted the longest.
	}else if(winnercount > 1){
		return winners[irand(0, winnercount-1)];
	}
	return -1; // fail
}

// Henk 23/01/10 -> Will strip hiders and put weapon to WP_NONE(puts their hands up).
void StripHiders(void)
{
	int i;
	for ( i = 0; i < level.numConnectedClients; i ++ )
	{
		gentity_t* ent = &g_entities[level.sortedClients[i]];
		// Henk 26/01/10 -> Don't strip seekers..
		if(ent->client->sess.team != TEAM_RED) // better then TEAM_BLUE because specs are now also excluded
			continue;

		// Henk 26/01/10 -> Dead clients dun have to be stripped
		if(G_IsClientDead(ent->client))
			continue;

		ent->client->sess.timeOfDeath = 1;
		ent->client->ps.zoomFov = 0;	///if they are looking through a scope go to normal view
		ent->client->ps.pm_flags &= ~(PMF_GOGGLES_ON|PMF_ZOOM_FLAGS);
		ent->client->ps.stats[STAT_WEAPONS] = 0;
		ent->client->ps.stats[STAT_GOGGLES] = GOGGLES_NONE;
		memset ( ent->client->ps.ammo, 0, sizeof(ent->client->ps.ammo) );
		memset ( ent->client->ps.clip, 0, sizeof(ent->client->ps.clip) );
		ent->client->ps.weapon = WP_NONE;
		ent->client->ps.weaponstate = WEAPON_READY;
		ent->client->ps.weaponTime = 0;
		ent->client->ps.weaponAnimTime = 0;
		ent->client->sess.score += 1;
		ent->client->sess.kills += 1;
	}
}

// Henk 28/01/10 -> Add Eventeams
void EvenTeams_HS (gentity_t *adm, qboolean aet)
{
	int		counts[TEAM_NUM_TEAMS];
	int		diff = 0;
	int		highTeam, i, j, lastConnectedTime;
	gentity_t *lastConnected, *ent;
	clientSession_t	*sess;
	int	seekers, maxhiders, totalplayers;

	if(level.intermissiontime)
		return;

	// Boe!Man 1/19/11: If teams are locked don't proceed with evening the teams.
	if(level.blueLocked || level.redLocked){
		if(adm && adm->client){
			trap_SendServerCommand( adm - g_entities, va("print \"^3[Info] ^7Teams are locked.\n\"") );
		}else if (aet == qfalse){
			Com_Printf("Teams are locked.\n");
		}
		return;
	}

	counts[TEAM_BLUE] = TeamCount1(TEAM_BLUE);
	counts[TEAM_RED] = TeamCount1(TEAM_RED);
	totalplayers = counts[TEAM_BLUE]+counts[TEAM_RED];
	// Henk 29/01/10 -> New player balance code
	if(totalplayers <= 2){
		if(adm && adm->client)
			trap_SendServerCommand( adm - g_entities, va("print \"^3[Info] ^7You need atleast 3 players to use eventeams.\n\"") );
		else 
			Com_Printf("You need atleast 3 players to use eventeams.\n");
		return;
	}else if(totalplayers >= 3 && totalplayers <= 5){ // hiders = totalplayers-seekers
		seekers = 1;
	}else if(totalplayers >= 6 && totalplayers <= 10){ // hiders = totalplayers-seekers
		seekers = 2;
	}else if(totalplayers >= 11 && totalplayers <= 16){ // hiders = totalplayers-seekers
		seekers = 3;
	}else if(totalplayers >= 17 && totalplayers <= 22){ // hiders = totalplayers-seekers
		seekers = 4;
	}else if(totalplayers >= 23){
		seekers = 5;
	}
	maxhiders = totalplayers-seekers;
	
	if(counts[TEAM_BLUE] < seekers){ // too few seekers
		highTeam = TEAM_RED;
		diff = (seekers - counts[TEAM_BLUE]); // move diff to seekers team
	}else if(counts[TEAM_BLUE] > seekers){ // too many seekers
		highTeam = TEAM_BLUE;
		diff = (counts[TEAM_BLUE] - seekers); // move diff to hiders team
	}
	else {
		if(adm && adm->client)
			trap_SendServerCommand( adm - g_entities, va("print \"^3[Info] ^7Teams are as even as possible.\n\"") );
		else 
			Com_Printf("Teams are as even as possible.\n");
		return;
	}
	// if less than 2 players difference, you cant make it any more even
	if(diff < 0){
		if(adm && adm->client)
			trap_SendServerCommand( adm - g_entities, va("print \"^3[Info] ^7Teams are as even as possible.\n\"") );
		else 
			Com_Printf("Teams are as even as possible.\n");
		return;
	}

	for(i = 0; i < diff; i++){
		lastConnectedTime = 0;
		for ( j = 0; j < level.numConnectedClients ; j++ )	{
			ent = &g_entities[level.sortedClients[j]];
			sess = &ent->client->sess;

			if ( ent->client->pers.connected != CON_CONNECTED 
				&& ent->client->pers.connected != CON_CONNECTING )
				continue;
			if(sess->team != TEAM_RED && sess->team != TEAM_BLUE)
				continue;
			if(sess->team != highTeam)
				continue;

			if(ent->client->pers.enterTime > lastConnectedTime)	{
				lastConnectedTime = ent->client->pers.enterTime;
				lastConnected = ent;
			}
		} 
		
		// Boe!Man 7/13/12: Fix crash issue with auto eventeams too soon before entering the map.
		if(lastConnected == NULL){
			if(adm && adm->client){
				trap_SendServerCommand( adm - g_entities, va("print \"^3[Info] ^7You cannot even the teams this fast.\n\"") );
			}else if(!aet){
				Com_Printf("You cannot even the teams this fast.\n");
			}
			return;
		}

		TossClientItems( lastConnected ); // Henk 19/01/11 -> Fixed items not dropping with !et
		lastConnected->client->ps.stats[STAT_WEAPONS] = 0;
		G_StartGhosting( lastConnected );

		if(highTeam == TEAM_RED)
		lastConnected->client->sess.team = TEAM_BLUE;
		else
		lastConnected->client->sess.team = TEAM_RED;

		if (lastConnected->r.svFlags & SVF_BOT)	{
			char userinfo[MAX_INFO_STRING];
			trap_GetUserinfo( lastConnected->s.number, userinfo, sizeof( userinfo ) );
			Info_SetValueForKey( userinfo, "team", lastConnected->client->sess.team == TEAM_RED?"red":"blue");
			trap_SetUserinfo( lastConnected->s.number, userinfo );
		}

		lastConnected->client->pers.identity = NULL;
		ClientUserinfoChanged( lastConnected->s.number );		
		CalculateRanks();

		G_StopFollowing( lastConnected );
		G_StopGhosting( lastConnected );
		trap_UnlinkEntity ( lastConnected );
		ClientSpawn( lastConnected);
	}

	Boe_GlobalSound (G_SoundIndex("sound/misc/events/tut_lift02.mp3"));
			
	if(adm && adm->client) {
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7Eventeams by %s.\n\"", adm->client->pers.netname));
	} else	{
		if(aet == qfalse)
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7Eventeams.\n\""));
		else
		trap_SendServerCommand(-1, va("print\"^3[Auto Action] ^7Eventeams.\n\""));
	}
}
// End
