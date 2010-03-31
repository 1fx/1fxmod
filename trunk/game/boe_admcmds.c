// Copyright (C) 2010 - Boe!Man.
//
// boe_admcmds.c - All the Admin Functions & commands for the Game Module go here.

#include "g_local.h"
#include "boe_local.h"

/*
==========
Boe_Adm_f
==========
*/

void Boe_adm_f ( gentity_t *ent )
{

	int		id, i, adm;
	char	arg1[MAX_STRING_TOKENS];
	char	arg2[MAX_STRING_TOKENS];
	char	arg3[MAX_STRING_TOKENS];
	gclient_t	*client;
	client = ent->client;
	trap_Argv( 1, arg1, sizeof( arg1 ) );
	trap_Argv( 2, arg2, sizeof( arg2 ) );
	
	adm = ent->client->sess.admin;
	if(!adm){
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You do not have admin powers!\n\""));
		return;
	}
	if (!Q_stricmp ( arg1, "?" )||!Q_stricmp ( arg1, "" ))
	{
	if (adm > 1){
		trap_SendServerCommand( ent-g_entities, va("print \" \n ^3Lvl   Commands         Arguments     Explanation\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \" ----------------------------------------------------------\n\""));
	if (adm >= g_kick.integer && g_kick.integer != 5){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   k   kick         <id> <reason> ^7[^3Kick a player^7]\n\"", g_kick.integer));
		}
	if (adm >= g_addbadmin.integer && g_addbadmin.integer != 5){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   ab  addbadmin    <id>          ^7[^3Add a Basic Admin^7]\n\"", g_addbadmin.integer));
		}
	if (adm >= g_addadmin.integer && g_addadmin.integer != 5){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   aa  addadmin     <id>          ^7[^3Add an Admin^7]\n\"", g_addadmin.integer));
		}
	// Boe!Man 1/4/10: Fix with using Tab in the Admin list.
	if (adm >= g_addsadmin.integer && g_addsadmin.integer != 5){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   as  addsadmin    <id>          ^7[^3Add a Server Admin^7]\n\"", g_addsadmin.integer));
		}
	if (adm >= g_ban.integer && g_ban.integer != 5){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   ba  ban          <id> <reason> ^7[^3Ban a player^7]\n\"", g_ban.integer));
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   uba unban        <ip>          ^7[^3Unban a banned IP^7]\n\"", g_ban.integer));
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   bl  banlist                    ^7[^3Shows the current banlist^7]\n\"", g_ban.integer));
		}
	if (adm >= g_subnetban.integer && g_subnetban.integer != 5){
		// Boe!Man 1/6/10: Reason added to the Subnetban command.
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   sb  subnetban    <id> <reason> ^7[^3Ban a players' subnet^7]\n\"", g_subnetban.integer));
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   sub subnetunban  <ip>          ^7[^3Unban a banned subnet^7]\n\"", g_subnetban.integer));
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   sbl subnetbanlist              ^7[^3Shows the current subnetbanlist^7]\n\"", g_subnetban.integer));
		}
	if (adm >= g_uppercut.integer && g_uppercut.integer != 5){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   uc  uppercut     <id>          ^7[^3Launch a player upwards^7]\n\"", g_uppercut.integer));
		}
	if (adm >= g_twist.integer && g_twist.integer != 5){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   tw  twist        <id>          ^7[^3Twist a player^7]\n\"", g_twist.integer));
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   utw untwist      <id>          ^7[^3Untwist a twisted player^7]\n\"", g_twist.integer));
		}
	if (adm >= g_runover.integer && g_runover.integer != 5){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   ro  runover      <id>          ^7[^3Boost a player backwards^7]\n\"", g_runover.integer));
		}
	if (adm >= g_maprestart.integer && g_maprestart.integer != 5){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   mr  maprestart   <id>          ^7[^3Restart the current map^7]\n\"", g_runover.integer));
		}
	if (adm >= g_flash.integer && g_flash.integer != 5){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   fl  flash        <id>          ^7[^3Flash a player^7]\n\"", g_flash.integer));
		}
	if (adm >= g_pop.integer && g_pop.integer != 5){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   p   pop          <id>          ^7[^3Pop a player^7]\n\"", g_pop.integer));
		}
	if (adm >= g_strip.integer && g_strip.integer != 5){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   st  strip        <id>          ^7[^3Remove weapons from a player^7]\n\"", g_strip.integer));
		}
	if (adm >= g_mute.integer && g_mute.integer != 5){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   m   mute         <id>          ^7[^3Mute a player^7]\n\"", g_mute.integer));
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   um  unmute       <id>          ^7[^3Unmute a player^7]\n\"", g_mute.integer));
		}
	if (adm >= g_plant.integer && g_plant.integer != 5){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   pl  plant        <id>          ^7[^3Plant a player in the ground^7]\n\"", g_plant.integer));
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   upl unplant      <id>          ^7[^3Unplant a planted player^7]\n\"", g_plant.integer));
		}
	if (adm >= g_burn.integer && g_burn.integer != 5){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   b   burn         <id>          ^7[^3Burn a player^7]\n\"", g_burn.integer));
		}
	if (adm >= g_eventeams.integer && g_eventeams.integer != 5){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   et  eventeams                  ^7[^3Make the teams even^7]\n\"", g_eventeams.integer));
		}
	if (adm >= g_333.integer && g_333.integer != 5){
		trap_SendServerCommand( ent-g_entities, va("print \" [^3%i^7]   333 333                        ^7[^3Enable/Disable 333 FPS jumps^7]\n\"", g_eventeams.integer));
		}
	trap_SendServerCommand( ent-g_entities, va("print \"    [^32^7] B-Admin          ^7[^33^7] Admin          ^7[^34^7] S-Admin\n\""));
	trap_SendServerCommand( ent-g_entities, va("print \" \n^7Use ^3[Page Up]^7 and ^3[Page Down]^7 keys to scroll.\n\""));
	}
	return;
	}
	if(!ent->client->sess.admin){
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Access denied: You don't have Admin powers!\n\""));
		return;
	}
	if (!Q_stricmp ( arg1, "kick" ) && ent->client->sess.admin >= g_kick.integer){
		id = Boe_ClientNumFromArg(ent, 2, "kick <id> <reason>", "Kick", qfalse, qfalse);
		if(id < 0) return;
		trap_Argv( 3, arg3, sizeof( arg3 ) );
		trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"%s\"\n", id, arg3));
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s was %sk%si%sc%sk%se%sd ^7by %s", level.time + 5000, g_entities[id].client->pers.netname, server_color1, server_color2, server_color3, server_color4, server_color5, server_color6, ent->client->pers.netname));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was kicked by %s.\n\"", g_entities[id].client->pers.netname,ent->client->pers.netname));
		return;
	}
	else if (!Q_stricmp ( arg1, "kick" ) && ent->client->sess.admin < g_kick.integer){
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\"", arg1));
		return;
	}
	if (!Q_stricmp ( arg1, "addbadmin" ) && ent->client->sess.admin >= g_addbadmin.integer){
		Boe_Add_bAdmin_f(2, ent);
		return;
	}
	else if (!Q_stricmp ( arg1, "addbadmin" ) && ent->client->sess.admin < g_addbadmin.integer){
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\"", arg1));
		return;
	}
	if (!Q_stricmp ( arg1, "addadmin" ) && ent->client->sess.admin >= g_addadmin.integer){
		Boe_Add_Admin_f(2, ent);
		return;
	}
	else if (!Q_stricmp ( arg1, "addadmin" ) && ent->client->sess.admin < g_addadmin.integer){
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\"", arg1));
		return;
	}
	if (!Q_stricmp ( arg1, "addsadmin" ) && ent->client->sess.admin >= g_addsadmin.integer){
		Boe_Add_sAdmin_f(2, ent);
		return;
	}
	else if (!Q_stricmp ( arg1, "addsadmin" ) && ent->client->sess.admin < g_addsadmin.integer){
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\"", arg1));
		return;
	}
	// Boe!Man 1/4/10
	if (!Q_stricmp ( arg1, "removeadmin" ) && ent->client->sess.admin >= g_removeadmin.integer){
		Boe_Remove_Admin_f(2, ent);
		return;
	}
	else if (!Q_stricmp ( arg1, "removeadmin" ) && ent->client->sess.admin < g_removeadmin.integer){
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\"", arg1));
		return;
	}
	if (!Q_stricmp ( arg1, "ban" ) && ent->client->sess.admin >= g_ban.integer){
		Boe_Ban_f(2, ent);
		return;
	}
	else if (!Q_stricmp ( arg1, "ban" ) && ent->client->sess.admin < g_ban.integer){
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\"", arg1));
		return;
	}
	if (!Q_stricmp ( arg1, "unban" ) && ent->client->sess.admin >= g_ban.integer){
		trap_Argv( 2, arg2, sizeof( arg2 ) );
		Boe_Unban(ent, arg2, qfalse);
		return;
	}
	else if (!Q_stricmp ( arg1, "unban" ) && ent->client->sess.admin < g_ban.integer){
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\"", arg1));
		return;
	}
	if (!Q_stricmp ( arg1, "subnetban" ) && ent->client->sess.admin >= g_subnetban.integer){
		Boe_subnetBan(2, ent);
		return;
	}
	else if (!Q_stricmp ( arg1, "subnetban" ) && ent->client->sess.admin < g_subnetban.integer){
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\"", arg1));
		return;
	}
	if (!Q_stricmp ( arg1, "subnetunban" ) && ent->client->sess.admin >= g_subnetban.integer){
		trap_Argv( 2, arg2, sizeof( arg2 ) );
		Boe_Unban(ent, arg2, qtrue);
		return;
	}
	else if (!Q_stricmp ( arg1, "subnetunban" ) && ent->client->sess.admin < g_subnetban.integer){
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\"", arg1));
		return;
	}
	if (!Q_stricmp ( arg1, "uppercut" ) && ent->client->sess.admin >= g_uppercut.integer){
		Boe_Uppercut(2, ent);
		return;
	}
	else if (!Q_stricmp ( arg1, "uc" ) && ent->client->sess.admin >= g_uppercut.integer){
		Boe_Uppercut(2, ent);
		return;
	}
	else if (!Q_stricmp ( arg1, "uppercut" ) && ent->client->sess.admin < g_uppercut.integer){
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\"", arg1));
		return;
	}
	else if (!Q_stricmp ( arg1, "uc" ) && ent->client->sess.admin < g_uppercut.integer){
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\"", arg1));
		return;
	}
	if ((!Q_stricmp ( arg1, "twist" )) && ent->client->sess.admin >= g_twist.integer) {
		Boe_Twist(2, ent);
		return;
	}
	else if ((!Q_stricmp ( arg1, "twist" )) && ent->client->sess.admin < g_twist.integer) {
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\"", arg1));
		return;
	}
	if ((!Q_stricmp ( arg1, "untwist" )) && ent->client->sess.admin >= g_twist.integer) {
		Boe_unTwist(2, ent);
		return;
	}
	else if ((!Q_stricmp ( arg1, "untwist" )) && ent->client->sess.admin < g_twist.integer) {
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\"", arg1));
		return;
	}
	if ((!Q_stricmp ( arg1, "respawn" )) && ent->client->sess.admin >= g_respawn.integer) {
		Boe_Respawn(2, ent);
		return;
	}
	else if ((!Q_stricmp ( arg1, "respawn" )) && ent->client->sess.admin < g_respawn.integer) {
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\"", arg1));
		return;
	}
	if ((!Q_stricmp ( arg1, "runover" )) && ent->client->sess.admin >= g_runover.integer) {
		Boe_Runover(2, ent);
		return;
	}
	else if ((!Q_stricmp ( arg1, "runover" )) && ent->client->sess.admin < g_runover.integer) {
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\"", arg1));
		return;
	}
	if ((!Q_stricmp ( arg1, "flash" )) && ent->client->sess.admin >= g_flash.integer) {
		Boe_Flash(2, ent);
		return;
	}
	else if ((!Q_stricmp ( arg1, "flash" )) && ent->client->sess.admin < g_flash.integer) {
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\"", arg1));
		return;
	}
	if ((!Q_stricmp ( arg1, "talk" ))) { // Boe!Man 1/17/10: No need to check for any level, as non-Admins are filtered out already..
		Cmd_Say_f (ent, ADM_TALK, qfalse); // Boe!Man 1/17/10: Sound will come later once the Admin Talk is triggered.
		return;
	}
	if (!Q_stricmp ( arg1, "maprestart" ) && ent->client->sess.admin >= g_maprestart.integer){
		trap_SendConsoleCommand( EXEC_APPEND, va("map_restart 5\n"));
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%sM%sa%sp %sr%se%sstart!", level.time + 5000, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		Boe_GlobalSound (G_SoundIndex("sound/misc/menus/invalid.wav"));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action^7] ^7Map restarted by %s.\n\"", ent->client->pers.netname));
		return;
	}
	else if (!Q_stricmp ( arg1, "maprestart" ) && ent->client->sess.admin < g_maprestart.integer){ // Boe!Man 1/24/10: Obviously we'd need to check for the g_maprestart command, and not the g_kick cmd.
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\"", arg1));
		return;
	}
	if ((!Q_stricmp ( arg1, "plant" )) && ent->client->sess.admin >= g_plant.integer) {
		Boe_Plant(2, ent);
		return;
	}
	else if ((!Q_stricmp ( arg1, "plant" )) && ent->client->sess.admin < g_plant.integer) {
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\"", arg1));
		return;
	}
	if ((!Q_stricmp ( arg1, "unplant" )) && ent->client->sess.admin >= g_plant.integer) {
		Boe_unPlant(2, ent);
		return;
	}
	else if ((!Q_stricmp ( arg1, "unplant" )) && ent->client->sess.admin < g_plant.integer) {
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\"", arg1));
		return;
	}
	if ((!Q_stricmp ( arg1, "pop" )) && ent->client->sess.admin >= g_pop.integer) {
		Boe_pop(2, ent);
		return;
	}
	else if ((!Q_stricmp ( arg1, "pop" )) && ent->client->sess.admin < g_pop.integer) {
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\"", arg1));
		return;
	}
	if ((!Q_stricmp ( arg1, "burn" )) && ent->client->sess.admin >= g_burn.integer) {
		Boe_Burn(2, ent);
		return;
	}
	else if ((!Q_stricmp ( arg1, "burn" )) && ent->client->sess.admin < g_burn.integer) {
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\"", arg1));
		return;
	}
	if ((!Q_stricmp ( arg1, "mute" )) && ent->client->sess.admin >= g_mute.integer) {
		Boe_Mute(2, ent, qtrue);
		return;
	}
	else if ((!Q_stricmp ( arg1, "mute" )) && ent->client->sess.admin < g_mute.integer) {
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\"", arg1));
		return;
	}
	if ((!Q_stricmp ( arg1, "unmute" )) && ent->client->sess.admin >= g_mute.integer) {
		Boe_Mute(2, ent, qfalse);
		return;
	}
	else if ((!Q_stricmp ( arg1, "unmute" )) && ent->client->sess.admin < g_mute.integer) {
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\"", arg1));
		return;
	}
	if ((!Q_stricmp ( arg1, "strip" )) && ent->client->sess.admin >= g_strip.integer) {
		Boe_Strip(2, ent);
		return;
	}
	else if ((!Q_stricmp ( arg1, "strip" )) && ent->client->sess.admin < g_strip.integer) {
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\"", arg1));
		return;
	}
	if ((!Q_stricmp ( arg1, "eventeams" )) && ent->client->sess.admin >= g_eventeams.integer) {
		EvenTeams(ent);
	}
	else if ((!Q_stricmp ( arg1, "eventeams" )) && ent->client->sess.admin < g_eventeams.integer) {
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\"", arg1));
		return;
	}
	if ((!Q_stricmp ( arg1, "banlist" )) && ent->client->sess.admin >= g_ban.integer) {
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Banlist]^7\n\""));
		Boe_Print_File( ent, g_banlist.string);
		return;
	}
	else if ((!Q_stricmp ( arg1, "banlist" )) && ent->client->sess.admin < g_ban.integer) {
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\"", arg1));
		return;
	}
	if ((!Q_stricmp ( arg1, "subnetbanlist" )) && ent->client->sess.admin >= g_subnetban.integer) {
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Subnet Banlist]^7\n\""));
		Boe_Print_File( ent, g_subnetbanlist.string);
		return;
	}
	else if ((!Q_stricmp ( arg1, "subnetbanlist" )) && ent->client->sess.admin < g_subnetban.integer) {
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\"", arg1));
		return;
	}
	if ((!Q_stricmp ( arg1, "forceteam" )) && ent->client->sess.admin >= g_forceteam.integer) {
		Adm_ForceTeam(ent);
		return;
	}
	else if ((!Q_stricmp ( arg1, "forceteam" )) && ent->client->sess.admin < g_forceteam.integer) {
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Your Admin level is too low to use this command.\n\"", arg1));
		return;
	}
	else
	{
		// Boe!Man 12/30/09: Putting two Info messages together.
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Unknown command %s. Usage: adm <command> <variable>\n\"", arg1));
		return;
	}
}

/*
====================
Boe_ClientNumFromArg
====================
*/

int Boe_ClientNumFromArg (gentity_t *ent, int argNum, const char* usage, const char* action, qboolean aliveOnly, qboolean otheradmins)
{
	char	arg[8] = "\0";
	int		num = -1;

	trap_Argv( argNum, arg, sizeof( arg ) );

	if (arg[0] >= '0' && arg[0] <= '9')
	{
		num = atoi( arg );
	}
	else if(ent && ent->client)
	{
		// Boe!Man 1/4/10: Putting two Info messages together.
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Bad client slot: %s. ^7Usage: adm %s\n\"", arg, usage));
		return -1;
	}
	else
	{
		Com_Printf("Bad client slot: %s\n", arg);
		Com_Printf("Usage: rcon %s\n", usage);
		return -1;
	}

	if ( num < 0 || num >= g_maxclients.integer )
	{
		if(ent && ent->client)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Invalid client number %d.\n\"", num));
		}
		else
		{
			Com_Printf("Invalid client number %d.\n", num );
		}
		return -1;
	}

	if ( g_entities[num].client->pers.connected == CON_DISCONNECTED )
	{
		if(ent && ent->client)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7There is no client with the client number %d.\n\"", num));
		}
		else
		{
			Com_Printf("There is no client with the client number %d.\n", num);
		}
		return -1;
	}

	if(ent && ent->client)
	{
		if (g_entities[num].client->sess.admin ){
			// Boe!Man 1/4/10: Higher level Admin fix.
			if(g_entities[num].client->sess.admin > ent->client->sess.admin){
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You cannot %s higher level Admins.\n\"", action));
				return -1;
			}
		}
	}
	// Boe!Man 1/18/10: Using Admin commands on other Admins fix.
	if(otheradmins == qfalse){
		if(g_entities[num].client->sess.admin && otheradmins == qfalse){
			if(ent && ent->client){
			trap_SendServerCommand(ent->s.number, va("print\"^3[Info] ^7You cannot use this command on other Admins.\n\""));
			return -1;
			}
			else{
				Com_Printf("You cannot use this command on other Admins.\n");
				return -1;
			}
		}
	}

	if(aliveOnly)
	{
		if ( G_IsClientDead ( g_entities[num].client ) )
		{
			if(ent && ent->client)
			{
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You cannot %s dead Players.\n\"", action));
			}
			else
			{
				Com_Printf("You cannot %s dead Players.\n", action);
			}
			return -1;
		}
		if (G_IsClientSpectating(g_entities[num].client))
		{
			if(ent && ent->client)
			{
				trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You cannot %s a Spectator.\n\"", action));
			}
			else
			{
				Com_Printf("You cannot %s a Spectator.\n", action);
			}
			return - 1;
		}
	}

	return num;
}

/*
=============
Boe_AddToList
=============
*/

int Boe_AddToList(const char *string, const char* file, const char* type, gentity_t* ent)
{
	int				len;
	fileHandle_t	f;
	len = trap_FS_FOpenFile( file, &f, FS_APPEND_TEXT );

    if (!f)
	{
		if(ent && ent->client)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"^1Error opening %s File\n\"", type));
		}
		else
		{
			Com_Printf("^1Error opening %s File\n", type);
		}
	    return -1;
	}

	if(len > 15000 - (strlen(string) + 1))
	{
		trap_FS_FCloseFile(f);

		if(ent && ent->client)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"^3List full, Unable to add %s.\n\"", type));
		}
		else
		{
			Com_Printf("^3List full, Unable to add %s.\n", type);
		}
		return 0;
	}
	trap_FS_Write(string, strlen(string), f);
	trap_FS_Write("\n", 1, f);
	trap_FS_FCloseFile(f);

	return 1;
}

/*
====================
Boe_Remove_from_list
====================
*/

int Boe_Remove_from_list ( char *key, const char *file, const char* type, gentity_t *ent, qboolean unban, qboolean removeAdmin, qboolean scoreCheck)
{
	int				len = 0, removed = 0;
	char			buf[15000] = "\0";
	char            listName[64] = "\0";
	char			ip[MAX_IP];
	char			*bufP, *listP;
	fileHandle_t	f;
	char			a[64] = "\0";
	int				count;

	while(*key == ' '){
		if(*key == '\0'){
			return 0;
		}
		key++;
	}
	Com_Printf("Remove: opening %s\n", file);
	len = trap_FS_FOpenFile( file, &f, FS_READ_TEXT);

	if (!f)	{
		Boe_FileError(ent, file);
		return -1;
	}

	if(len > 15000)	{
		len = 15000;
	}
	memset( buf, 0, sizeof(buf) );

	trap_FS_Read( buf, len, f );
	buf[len] = '\0';
	trap_FS_FCloseFile( f );

	bufP = buf;
	listP = listName;

	len = trap_FS_FOpenFile( file, &f, FS_WRITE_TEXT);

	while(*bufP) {
		while(*bufP != '\n' && *bufP != '\0') {
			*listP++ = *bufP++;
			if(unban && *bufP == '\\') {
				*listP = '\0';
				Q_strncpyz(ip, listName, MAX_IP);
			}
		}

		*listP = '\0';

		while (*bufP == '\n') {
			bufP++;
		}

		if(unban && !Q_stricmp(ip, key)) {
			removed = 1;
			listP = listName;
			continue;
		}
					
		if (removeAdmin){
			strcpy(a, va("%s:2", key ));
			if (!Q_stricmp(listName, a)) {
				removed = 1;
				listP = listName;
				continue;
			}
			strcpy(a, va("%s:3", key ));
			if (!Q_stricmp(listName, a)) {
				removed = 1;
				listP = listName;
				continue;
			}
			strcpy(a, va("%s:4", key ));
			if (!Q_stricmp(listName, a)) {
				removed = 1;
				listP = listName;
				continue;
			} 
		}else if(scoreCheck){
			if(strstr(listName, key)){
				removed = 1;
				listP = listName;
				continue;
			}
		}

		if(!Q_stricmp(listName, key)){
			removed = 1;
			listP = listName;
			continue;
		}

		trap_FS_Write( listName, strlen(listName), f);
		trap_FS_Write( "\n", 1, f);
		listP = listName;
	}
	trap_FS_FCloseFile( f );

	if(!removed){
		if(ent && ent->client) {
			trap_SendServerCommand( ent-g_entities, va("print \"^3%s ^7was not on the %s list\n\"", key, type));
		}
		else {
			Com_Printf("^3%s ^7was not on the %s list\n", key, type);
		}
	}
	return removed;
}

/*
=============
Boe_id
=============
*/
void Boe_id (int idnum)
{
	int             i = 0;
	char		    *ip = g_entities[idnum].client->pers.ip;
	char			*id = g_entities[idnum].client->pers.boe_id;

	if(g_entities[idnum].r.svFlags & SVF_BOT){
		Com_sprintf(id, MAX_BOE_ID, "bot\\%s", g_entities[idnum].client->pers.cleanName);
		return;
	}
	while(i < 6)
	{
		*id++ = *ip++;
		i++;
	}

	*id++ = '\\';
    *id = '\0';
	strcat(id, g_entities[idnum].client->pers.cleanName);
	return;
}

/*
==============
Boe_FileError
==============
*/
void Boe_FileError (gentity_t * ent, const char *file)
{
	if(ent && ent->client)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Error opening %s\n\"", file));
	}
	else
	{
		Com_Printf("^1Error opening %s\n", file);
	}
	return;
}

/*
================
Boe_Add_bAdmin_f
================
*/

void Boe_Add_bAdmin_f(int argNum, gentity_t *adm)
{
	int             idnum;
	char			*id;
	char			id2[64];

	idnum = Boe_ClientNumFromArg(adm, argNum, "addbadmin <idnumber>", "do this to", qfalse, qfalse);
	if(idnum < 0) return;

	id = g_entities[idnum].client->pers.boe_id;

	if(Boe_Remove_from_list(id, g_adminfile.string,	 "admin", NULL, qfalse, qtrue, qfalse));

	g_entities[idnum].client->sess.admin = 2;
	Q_strncpyz (id2, id, 64);
	strcat ( id2, ":2" );

	// Boe!Man 1/14/10
	if(adm && adm->client)	{
		if(Boe_AddToList(id2, g_adminfile.string, "Admin", NULL)) {
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s is now a %sB^7-%sA%sd%sm%si%sn", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s is now a B-Admin.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("%s - ADD B-ADMIN: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
		}
	}
	else {
		if(Boe_AddToList(id2, g_adminfile.string, "Admin", NULL)) {
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s is now a %sB^7-%sA%sd%sm%si%sn", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s is now a B-Admin.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("%s - ADD B-ADMIN: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
		}
	}
}

/*
===============
Boe_Add_Admin_f
===============
*/

void Boe_Add_Admin_f(int argNum, gentity_t *adm)
{
	int             idnum;
	char			*id;
	char			id2[64];

	idnum = Boe_ClientNumFromArg(adm, argNum, "addadmin <idnumber>", "do this to", qfalse, qfalse);
	if(idnum < 0) return;

	id = g_entities[idnum].client->pers.boe_id; 

	if(Boe_Remove_from_list(id, g_adminfile.string, "admin", NULL, qfalse, qtrue, qfalse));

	g_entities[idnum].client->sess.admin = 3;
	Q_strncpyz (id2, id, 64);
	strcat ( id2, ":3" );

	// Boe!Man 1/14/09
	if(adm && adm->client)	{
		if(Boe_AddToList(id2, g_adminfile.string, "Admin", NULL)) {
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s is now an %sA%sd%sm%si%sn", level.time + 5000, g_entities[idnum].client->pers.netname, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s is now an Admin.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("%s - ADD ADMIN: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
		}
	}
	else {
		if(Boe_AddToList(id2, g_adminfile.string, "Admin", NULL)) {
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s is now an %sA%sd%sm%si%sn", level.time + 5000, g_entities[idnum].client->pers.netname, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s is now an Admin.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("%s - ADD ADMIN: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
		}
	}
}

/*
================
Boe_Add_sAdmin_f
================
*/

void Boe_Add_sAdmin_f(int argNum, gentity_t *adm)
{
	int             idnum;
	char			*id;
	char			id2[64];

	idnum = Boe_ClientNumFromArg(adm, argNum, "addsadmin <idnumber>", "do this to", qfalse, qfalse);
	if(idnum < 0) return;

	id = g_entities[idnum].client->pers.boe_id; 

	if(Boe_Remove_from_list(id, g_adminfile.string, "admin", NULL, qfalse, qtrue, qfalse));

	g_entities[idnum].client->sess.admin = 4;
	Q_strncpyz (id2, id, 64);
	strcat ( id2, ":4" );

	// Boe!Man 1/14/10
	if(adm && adm->client)	{
		if(Boe_AddToList(id2, g_adminfile.string, "Admin", NULL)) {
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s is now a %sS^7-%sA%sd%sm%si%sn", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s is now a S-Admin.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("%s - ADD S-ADMIN: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
		}
	}
	else {
		if(Boe_AddToList(id2, g_adminfile.string, "Admin", NULL)) {
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s is now a %sS^7-%sA%sd%sm%si%sn", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s is now a S-Admin.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("%s - ADD S-ADMIN: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
		}
	}
}
		/*
		if(adm && adm->client)
			SC_adminLog (va("%s - AddAdmin: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
		else 
			SC_adminLog (va("%s - AddAdmin: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
		*/

/*
==================
Boe_Unban
==================
*/

void Boe_Unban(gentity_t *adm, char *ip, qboolean subnet)
{
	int		count = 0, count2 = 0;

	while (ip[count] != '\0'){
		if(ip[count] == ' '){
			count++;
			continue;
		}

		if(ip[count] == '.'){
			count++;
			count2++;
		}
		if(ip[count] < '0' || ip[count] > '9')	{
			trap_SendServerCommand( adm-g_entities, va("print \"^3%s ^7is an invalid ip address\n\"", ip));
			return;
		}
		count++;
	}

	if(!subnet){
		if(count2 < 3){
			trap_SendServerCommand( adm-g_entities, va("print \"^3%s ^7is an invalid ip address\n\"", ip));
			return;
		}
			if(Boe_Remove_from_list(ip, g_banlist.string, "Ban", adm, qtrue, qfalse, qfalse )){
				trap_SendServerCommand( adm-g_entities, va("print \"^3%s ^7has been Unbanned.\n\"", ip));
				/*
				if(adm && adm->client)
					SC_adminLog (va("%s - UNBAN: %s", adm->client->pers.cleanName, ip  )) ;
				else 
					SC_adminLog (va("%s - UNBAN: %s", "RCON", ip  )) ;
				*/
				return;
			}
	}
	else {
		if(count2 < 1){
			trap_SendServerCommand( adm-g_entities, va("print \"^3%s ^7is an invalid ip address\n\"", ip));
			return;
		}
		if(Boe_Remove_from_list(ip, g_subnetbanlist.string, "SubnetBan", adm, qtrue, qfalse, qfalse )){
			trap_SendServerCommand( adm-g_entities, va("print \"^3%s's Subnet ^7has been Unbanned.\n\"", ip));
			if(adm && adm->client)
				Boe_adminLog (va("%s - SUBNET UNBAN: %s", adm->client->pers.cleanName, ip  )) ;
			else 
				Boe_adminLog (va("%s - SUBNET UNBAN: %s", "RCON", ip  )) ;
			return;
		}
	}
}

/*
==================
Boe_NameListCheck
==================
*/

int Boe_NameListCheck (int num, const char *name, const char *file, gentity_t *ent, qboolean banCheck, qboolean admCheck, qboolean subnetCheck, qboolean scoreCheck)
{
	int             len;
	fileHandle_t	f;
	char            buf[15000];
	char			ip[MAX_IP];
	char			listName[64];
	char			*listP = listName;
	char			*bufP = buf;
	char			x[64] = "\0";
	char			a[64] = "\0";
	qboolean		banline = qfalse;
	int				realcount = 0;

	int count;
	ent = g_entities + num;
	len = trap_FS_FOpenFile( file, &f, FS_READ_TEXT); 

	if (!f) { 
		len = trap_FS_FOpenFile( file, &f, FS_APPEND_TEXT);   
		if (!f) {  
			Boe_FileError(ent, file);
			return -1;
		}
		trap_FS_FCloseFile( f ); 
		len = trap_FS_FOpenFile( file, &f, FS_READ_TEXT); 

		if (!f) { 
			Boe_FileError(ent, file);
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
			 if((banCheck || subnetCheck) && *bufP == '\\') { 
				 *listP = '\0';
				 Q_strncpyz(ip, listName, MAX_IP);
				 listP = listName;
				 banline = qtrue;
				 bufP++;
			 }
		}

		*listP = '\0';
		listP = listName;			
		if (admCheck){
			for(count=0;count<=20;count++){
			strcpy(a, va("%s:%i", name, count ));
			if (!Q_stricmp(listName, a)) {
				return count;
			}
			}
			strcpy(a, va("%s:2", name ));
			if (!Q_stricmp(listName, a)) {
				return 2;
			}
			strcpy(a, va("%s:3", name ));
			if (!Q_stricmp(listName, a)) {
				return 3;
			}
			strcpy(a, va("%s:4", name ));
			if (!Q_stricmp(listName, a)) {
				return 4;
			} 
		}
		else if(banCheck && banline){
			if(!Q_stricmp(ip, name))
				return 1;
		}
		else if (subnetCheck && banline){
			if(strlen(ip) > 3){
				Q_strncpyz (x, name, strlen(ip) + 1);
				if(!Q_stricmp(ip, x))
					return 1;
			}
		}else if(scoreCheck){
			for(count=0;count<=100;count++){ // FIX ME HENK
			strcpy(a, va("%s:%i", name, count ));
			if (!Q_stricmp(listName, a)) {
				realcount = count;
			}
			}	
			if(realcount != 0){
				return realcount;
			}
		}
		if (!Q_stricmp(listName, name) && !admCheck && !subnetCheck)	{
			return 1;
		}
		while(*bufP == '\n') {
			bufP++;
		}
	}	
	return 0;
}

/*
=================
Boe_subnetBan
=================
*/

void Boe_subnetBan (int argNum, gentity_t *adm)  
{
	gentity_t		*ent;
	char			ip[16];
	char			reason[16] = "\0";
	char			size[4];
	char			info[96];
	int				idnum, s;

	if(adm && adm->client) argNum = 2;
	else argNum = 1;

	idnum = Boe_ClientNumFromArg(adm, argNum, "subnetban <id> <size> <reason>", "subnetban", qfalse, qfalse);
	if(idnum < 0) return;
	ent = g_entities + idnum;

	trap_Argv( argNum + 1, size, sizeof( size ) );
	trap_Argv( argNum + 2, reason, sizeof( reason ) );
	s = atoi(size) + 1;
	
	if (!size[0] || s <= 4 ) s = 8;
	if (s > 16) s = 16;		

	Q_strncpyz(ip, ent->client->pers.ip, s);
	// Boe!Man 1/7/10: No more logging of reason in the ban file.
	strcpy(info, va("%s\\%s", ip, ent->client->pers.cleanName));
	
	if(Boe_AddToList(info, g_subnetbanlist.string, "Subnet ban", adm)){
		if(adm && adm->client)	{
			if(!*reason){
				trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"Subnetbanned by %s\"\n", idnum, adm->client->pers.netname)); }
			else if(reason != ""){
				trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"Subnetbanned by %s for: %s\"\n", idnum, adm->client->pers.netname, reason));}
			else {
				trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"Subnetbanned by %s\"\n", idnum, adm->client->pers.netname));}
			// trap_SendServerCommand( adm-g_entities, va("print \"%s's ^4SUBNET ^7WAS ^1BANNED^7!\n\"", ent->client->pers.netname));
			Boe_adminLog (va("%s - SUBNETBAN: %s", adm->client->pers.cleanName, ent->client->pers.cleanName  )) ;
		}
		else {
			if (!*reason){
				trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"Subnetbanned for: %s\"\n", idnum, reason));
			}
			else {
				trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"Subnetbanned!\"\n", idnum));
			}
			Boe_adminLog (va("%s - SUBNETBAN: %s", "RCON", ent->client->pers.cleanName  )) ;
			// Com_Printf("%s's ^4SUBNET ^7WAS ^1BANNED^7!\n", ent->client->pers.netname);
		}
	}
}

/*
==================
Boe_Remove_Admin_f
==================
*/

void Boe_Remove_Admin_f (int argNum, gentity_t *adm)

{
	int				idnum;
	char			*id;

	idnum = Boe_ClientNumFromArg(adm, argNum, "removeadmin <idnumer>", "do this to", qfalse, qtrue);
	if(idnum < 0) return;

	id = g_entities[idnum].client->pers.boe_id;
	
	// Boe!Man 1/6/10: Fix, succesfully writes the Admin out of the file.
	if(Boe_Remove_from_list(id, g_adminfile.string, "admin", NULL, qfalse, qtrue, qfalse)){
		// Boe: We'll log this later.
		//
		// SC_adminLog (va("%s - RemoveAdmin: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
	}
	g_entities[idnum].client->sess.admin = 0;

	if(adm && adm->client){
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7is no longer an %sA%sd%sm%si%sn", level.time + 5000, g_entities[idnum].client->pers.netname, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was removed as Admin by %s.\n\"", g_entities[idnum].client->pers.netname,adm->client->pers.netname));
		Boe_adminLog (va("%s - REMOVE ADMIN: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
	}
	else{
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7is no longer an %sA%sd%sm%si%sn", level.time + 5000, g_entities[idnum].client->pers.netname, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s ^7was removed as Admin.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("%s - REMOVE ADMIN: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
	}
}

/*
=========
Boe_Ban_f
=========
*/

void Boe_Ban_f (int argNum, gentity_t *adm)
{
	int				idnum;
	char            banid[128];
	char			reason[MAX_STRING_TOKENS] = "\0";

	idnum = Boe_ClientNumFromArg(adm, argNum, "ban <idnumber> <reason>", "ban", qfalse, qfalse);

	if(idnum < 0)
		return;

	trap_Argv( argNum + 1, reason, sizeof( reason ) );

	Com_sprintf (banid, sizeof(banid), "%s\\%s",
		g_entities[idnum].client->pers.ip,
		g_entities[idnum].client->pers.cleanName);

	if(Boe_AddToList(banid, g_banlist.string, "Ban", adm)){
		if(adm && adm->client)	{
			if(!*reason){
				trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"Banned by %s\"\n", idnum, adm->client->pers.netname));
				Boe_adminLog (va("%s - BAN: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;}
			else{
				trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"Banned by %s for: %s\"\n", idnum, adm->client->pers.netname, reason));
				trap_SendServerCommand( adm-g_entities, va("print \"%s was banned by %s!\n\"", g_entities[idnum].client->pers.netname, adm->client->pers.netname));
				Boe_adminLog (va("%s - BAN: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
			}
		}
		else{
			if(!*reason){
				trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"Banned!\"\n", idnum));
				Boe_adminLog (va("%s - RUNOVER: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;}
			else{
				trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%d\" \"Banned for: %s\"\n", idnum, reason));
				Com_Printf("%s was banned!\n", g_entities[idnum].client->pers.netname);
				Boe_adminLog (va("%s - BAN: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
			}
		}
	}
}

/*
============
Boe_Uppercut
============
*/

void Boe_Uppercut (int argNum, gentity_t *adm)
{
	gentity_t		*ent;
	int				idnum;
	char			*status;

	idnum = Boe_ClientNumFromArg(adm, argNum, "uppercut <idnumber>", "uppercut", qtrue, qtrue);
	if(idnum < 0) return;
	
	ent = g_entities + idnum;
	ent->client->ps.pm_flags |= PMF_JUMPING;
	ent->client->ps.groundEntityNum = ENTITYNUM_NONE;
	ent->client->ps.velocity[2] = 1000;
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
	
	if(g_entities[idnum].client->sess.lastIdentityChange)	{
		status = ".";
		g_entities[idnum].client->sess.lastIdentityChange = qfalse;
	}
	else {
		status = ".";
		g_entities[idnum].client->sess.lastIdentityChange = qtrue;
	}
	
	if(adm && adm->client){
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,^7%s was %su%sp%sp%se%sr%scut by %s", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was uppercut by %s.\n\"", g_entities[idnum].client->pers.netname,adm->client->pers.netname));
		Boe_adminLog (va("%s - UPPERCUT: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
	}
	else {
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,^7%s was %su%sp%sp%se%sr%scut", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s was uppercut.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("%s - UPPERCUT: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
	}
}

/*
=========
Boe_Twist
=========
*/

void Boe_Twist (int argNum, gentity_t *adm)
{
	gentity_t		*ent;
	int				idnum, i;
	vec3_t			lookdown;
	//char			*a, *b, *c;
	char			a[4], b[4], c[4];


	idnum = Boe_ClientNumFromArg(adm, argNum, "twist <idnumber>", "twist", qtrue, qtrue);
	if(idnum < 0)  return;

	ent = g_entities + idnum;

	trap_Argv( argNum + 1, c, sizeof(c) );
	trap_Argv( argNum + 2, b, sizeof(b) );
	trap_Argv( argNum + 3, a, sizeof(a) );
	if ( !c[0] ) {
	VectorSet(lookdown, 100, 0, 130);
	}
	else {
	VectorSet(lookdown, atoi(a), atoi(b), atoi(c));
	}

	SetClientViewAngle(ent, lookdown);

	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
	
	/*
	for (i = 0; i < level.numConnectedClients; i++)	{
		if (level.clients[level.sortedClients[i]].sess.admin){	
			if(adm != NULL)
				//CPx(level.sortedClients[i], va("print \"^7%s^7: ^3[%s] %s\n\"", adm->client->pers.netname, action, target));
				trap_SendServerCommand(level.sortedClients[i], va("print \"^7%s^7: ^3[Twisted] ^7%s\n\"", adm->client->pers.netname, g_entities[idnum].client->pers.netname));
			else 
				trap_SendServerCommand(level.sortedClients[i], va("print \"^7ADMIN^7: ^3[Twisted] ^7%s\n\"", g_entities[idnum].client->pers.netname));
		}
	}
	*/

	if(adm && adm->client) {
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,^7%s was %st%sw%si%ss%st%sed by %s", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was twisted by %s.\n\"", g_entities[idnum].client->pers.netname,adm->client->pers.netname));
		Boe_adminLog (va("%s - TWIST: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;}
	else {
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,^7%s was %st%sw%si%ss%st%sed", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s was twisted.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("%s - TWIST: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
	}
}

/*
===========
Boe_unTwist
===========
*/

void Boe_unTwist (int argNum, gentity_t *adm)
{
	gentity_t	*ent;
	int			idnum;
	vec3_t		lookdown;

	idnum = Boe_ClientNumFromArg(adm, argNum, "untwist <idnumber>", "untwist", qtrue, qtrue);
	if(idnum < 0) {	return;	}

	ent = g_entities + idnum;

	VectorSet(lookdown, 0, 0, 0);	
	SetClientViewAngle(ent, lookdown);
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));

	if(adm && adm->client){
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,^7%s was %st%sw%si%ss%st%sed by %s", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was untwisted by %s.\n\"", g_entities[idnum].client->pers.netname,adm->client->pers.netname));
		Boe_adminLog (va("%s - UNTWIST: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
	}
	else {
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,^7%s was %su%sn%st%sw%si%ssted", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s was untwisted.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("%s - UNTWIST: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
	}
}

/*
===========
Boe_Respawn
===========
*/

void Boe_Respawn (int argNum, gentity_t *adm)
{
	gentity_t		*ent;
	qboolean		ghost;
	int				idnum;
	char	*status;

	idnum = Boe_ClientNumFromArg(adm, argNum, "respawn <idnumber>", "respawn", qfalse, qtrue);
	if(idnum < 0) return;
	
	ent = g_entities + idnum;

	if ( ent->client->sess.ghost ){
			G_StopFollowing ( ent );
			ent->client->ps.pm_flags &= ~PMF_GHOST;
			ent->client->ps.pm_type = PM_NORMAL;
			ent->client->sess.ghost = qfalse;
			//if(ent->client->sess.ghostStartTime)
			//{
			//	ent->client->sess.totalSpectatorTime += level.time - ent->client->sess.ghostStartTime;
			//	ent->client->sess.ghostStartTime = 0;
			//}
	}else{
		TossClientItems(ent);
	}
	ent->client->sess.noTeamChange = qfalse;
	trap_UnlinkEntity (ent);
	ClientSpawn(ent);
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
	Boe_ClientSound(ent, G_SoundIndex("sound/ambience/vehicles/telephone_pole.mp3"));


	if(g_entities[idnum].client->sess.lastIdentityChange){
		status = ".";
		g_entities[idnum].client->sess.lastIdentityChange = qfalse;
	}
	else {
		status = ".";
		g_entities[idnum].client->sess.lastIdentityChange = qtrue;
	}
	
	if(adm && adm->client){
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,^7%s was %sr%se%ss%sp%sa%swned by %s", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s was respawned by %s.\n\"", g_entities[idnum].client->pers.netname,adm->client->pers.netname));
		Boe_adminLog (va("%s - RESPAWN: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
	}
	else {
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,^7%s was %sr%se%ss%sp%sa%swned", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s was respawned.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("%s - RESPAWN: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
	}
}

/*
======================
Boe_Runover
======================
*/
void Boe_Runover (int argNum, gentity_t *adm)
{
	gentity_t	*ent;
	vec3_t	dir;
	vec3_t  fireAngs;
	float   knockback = 400.0;
	int		idnum;
	int		anim = 0;
	
	idnum = Boe_ClientNumFromArg(adm, argNum, "runover <idnumber>", "runover", qtrue, qtrue);
	if(idnum < 0)
	{
		return;
	}

	ent = g_entities + idnum;

	VectorCopy(ent->client->ps.viewangles, fireAngs);
	AngleVectors( fireAngs, dir, NULL, NULL );	
	dir[0] *= -1.0;
	dir[1] *= -1.0;
	dir[2] = 0.0;
	VectorNormalize ( dir );

	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
	Boe_ClientSound(ent, G_SoundIndex("sound/ambience/vehicles/hit_scrape.mp3"));
	ent->client->ps.weaponstate = WEAPON_FIRING;
	ent->client->ps.velocity[2] = 20;
	ent->client->ps.weaponTime = 3000;

	if(adm && adm->client){
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sr%su%sn%so%sv%ser by %s", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was runover by %s.\n\"", g_entities[idnum].client->pers.netname,adm->client->pers.netname));
		Boe_adminLog (va("%s - RUNOVER: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
	}
	else{
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sr%su%sn%so%sv%ser", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s ^7was runover.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("%s - RUNOVER: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
	}	
	G_Damage (ent, NULL, NULL, NULL, NULL, 15, 0, MOD_CAR, HL_NONE );
	G_ApplyKnockback ( ent, dir, knockback );
}

/*
=========
Boe_Flash
=========
*/

void Boe_Flash (int argNum, gentity_t *adm)
{
	gentity_t		*ent;
	int				idnum;
	vec3_t dir;
	int it, nadeDir, weapon;
	float x, y;
	gentity_t *missile;
	idnum = Boe_ClientNumFromArg(adm, argNum, "flash <idnumber>", "flash", qtrue, qtrue);
	if(idnum < 0) return;
	ent = g_entities + idnum;
	weapon = WP_M84_GRENADE;
	nadeDir = 1;
	for( it = 0; it < 1; it++ ) {
		x = 100 * cos( DEG2RAD(nadeDir * it));  
		y = 100 * sin( DEG2RAD(nadeDir * it));
		VectorSet( dir, x, y, 100 );
		dir[2] = 300;	
		missile = NV_projectile( ent, ent->r.currentOrigin, dir, weapon, 0 );
		missile->nextthink = level.time + 250;
	}
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
	if(adm && adm->client){
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sf%sl%sa%ss%sh%sed by %s", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was flashed by %s.\n\"", g_entities[idnum].client->pers.netname,adm->client->pers.netname));
		Boe_adminLog (va("%s - FLASH: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
	}
	else{
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sf%sl%sa%ss%sh%sed", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s ^7was flashed.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("%s - FLASH: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
	}	
}

/*
=========
Boe_Plant
=========
*/

void Boe_Plant (int argNum, gentity_t *adm)
{
	gentity_t	*ent;
	int			idnum;
	vec3_t dir;
	int it, nadeDir, weapon;
	float x, y;
	gentity_t *missile;

	idnum = Boe_ClientNumFromArg(adm, argNum, "plant <idnumber>", "Plant", qtrue, qtrue);

	if(idnum < 0)
		return;

	ent = g_entities + idnum;

	if(ent->client->pers.planted){
		if(adm && adm->client)
			trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7%s ^7is already planted.\n\"", ent->client->pers.netname));
		else
			Com_Printf("^3%s is already Planted.\n", ent->client->pers.netname);
		return;
	}

	if ( ent->client->ps.pm_flags & PMF_DUCKED )
		ent->client->ps.origin[2] -=40;
	else
		ent->client->ps.origin[2] -= 65;

	VectorCopy( ent->client->ps.origin, ent->s.origin );
	ent->client->pers.planted = qtrue;

	// Boe!Man 1/29/10: All clients will need to hear the event sound, though only the victim will have to hear the wood breaking sound.
	Boe_ClientSound(ent, G_SoundIndex("sound/misc/confused/wood_break.mp3"));
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));

	if(adm && adm->client){
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sp%sl%sa%sn%st%sed by %s", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was planted by %s.\n\"", g_entities[idnum].client->pers.netname,adm->client->pers.netname));
		Boe_adminLog (va("%s - PLANT: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
	}
	else{
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sp%sl%sa%sn%st%sed", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s ^7was planted.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("%s - PLANT: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
	}
}

/*
===========
Boe_unPlant
===========
*/

void Boe_unPlant (int argNum, gentity_t *adm)
{
	gentity_t	*ent;
	int			idnum;
	vec3_t dir;
	int it, nadeDir, weapon;
	float x, y;
	gentity_t *missile;

	idnum = Boe_ClientNumFromArg(adm, argNum, "unplant <idnumber>", "unplant", qtrue, qtrue);

	if(idnum < 0){return;}
	ent = g_entities + idnum;

	if(!ent->client->pers.planted){
		if(adm && adm->client)
			trap_SendServerCommand( adm-g_entities, va("print \"^3[Info] ^7%s ^7is not planted.\n\"", ent->client->pers.netname));
		 else
			Com_Printf("^3%s is not planted.\n", ent->client->pers.netname);
		return;
	}

	if(ent->client->pers.planted){
		ent->client->ps.origin[2] += 65;
		VectorCopy( ent->client->ps.origin, ent->s.origin );
		ent->client->pers.planted = qfalse;
	}

	Boe_ClientSound(ent, G_SoundIndex("sound/misc/confused/wood_break.mp3"));
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));

	if(adm && adm->client){
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %su%sn%sp%sl%sa%snted by %s", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was unplanted by %s.\n\"", g_entities[idnum].client->pers.netname,adm->client->pers.netname));
		Boe_adminLog (va("%s - UNPLANT: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
	}
	else{
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %su%sn%sp%sl%sa%snted", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s ^7was unplanted.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("%s - UNPLANT: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
	}
}

/*
=======
Boe_pop
=======
*/

void Boe_pop (int argNum, gentity_t *adm)
{
	gentity_t		*ent;
	int				idnum;
	int				anim = 0;

	idnum = Boe_ClientNumFromArg(adm, argNum, "pop <idnumber>", "pop", qtrue, qtrue);
	if(idnum < 0){return;}
	ent = g_entities + idnum;
	
	Boe_ClientSound(ent, G_SoundIndex("sound/npc/air1/guard02/laughs.mp3"));
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));

	if(adm && adm->client){
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sp%so%sp%sp%se%sd by %s", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was popped by %s.\n\"", g_entities[idnum].client->pers.netname,adm->client->pers.netname));
		Boe_adminLog (va("%s - POP: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
	}
	else{
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sp%so%sp%sp%se%sd", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s ^7was popped.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("%s - POP: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
	}
	// Boe!Man 1/30/10: We use g_damage since the animations won't work.
	G_Damage (ent, NULL, NULL, NULL, NULL, 10000, 0, MOD_POP, HL_HEAD|HL_FOOT_RT|HL_FOOT_LT|HL_LEG_UPPER_RT|HL_LEG_UPPER_LT|HL_HAND_RT|HL_HAND_LT|HL_WAIST|HL_CHEST|HL_NECK);
}

/*
========
Boe_Burn
========
*/
void Boe_Burn (int argNum, gentity_t *adm)
{
	gentity_t		*ent;
	int				idnum;
	gentity_t *tent; 

	idnum = Boe_ClientNumFromArg(adm, argNum, "burn <idnumber>", "burn", qtrue, qtrue);
	if(idnum < 0) return;
	ent = g_entities + idnum;
	ent->client->sess.burnSeconds = 6;

	tent = G_TempEntity( g_entities[ent->s.number].r.currentOrigin, EV_EXPLOSION_HIT_FLESH ); 
	tent->s.eventParm = 0; 
	tent->s.otherEntityNum2 = g_entities[ent->s.number].s.number; 
	tent->s.time = WP_ANM14_GRENADE + ((((int)g_entities[ent->s.number].s.apos.trBase[YAW]&0x7FFF) % 360) << 16); 
	VectorCopy ( g_entities[ent->s.number].r.currentOrigin, tent->s.angles ); 
	SnapVector ( tent->s.angles ); 

	Boe_ClientSound(ent, G_SoundIndex("/sound/weapons/incendiary_grenade/incen01.mp3"));
	Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));

	if(adm && adm->client){
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sb%su%sr%sn%se%sd by %s", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was burned by %s.\n\"", g_entities[idnum].client->pers.netname,adm->client->pers.netname));
		Boe_adminLog (va("%s - BURN: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
	}
	else{
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sb%su%sr%sn%se%sd", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s ^7was burned.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("%s - BURN: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
	}
}

void Adm_ForceTeam(gentity_t *adm)
{
	char		str[MAX_TOKEN_CHARS];
	int			idnum;

	// find the player
	idnum = Boe_ClientNumFromArg(adm, 2, "forceteam <idnumber>", "forceteam", qfalse, qtrue);
	if(idnum < 0) return;

	// set the team
	trap_Argv( 3, str, sizeof( str ) );
	SetTeam( &g_entities[idnum], str, NULL );
}

/*
========
Boe_Mute
========
*/

void Boe_Mute (int argNum, gentity_t *adm, qboolean mute)
{
	int		idnum;
	char	*status;
	char	*status2;

	idnum = Boe_ClientNumFromArg(adm, argNum, "mute/unmute <idnumber>", "mute/unmute", qfalse, qfalse);

	if(idnum < 0) return;

	if (mute == qtrue){
		if(g_entities[idnum].client->sess.mute == qtrue){
			trap_SendServerCommand(adm-g_entities, va("print \"^3[Info] ^7This client is already muted!\n\""));}
		else{
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			g_entities[idnum].client->sess.mute = qtrue;
			if(adm && adm->client){
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sm%su%st%se%sd by %s", level.time + 5000, g_entities[idnum].client->pers.netname, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
				trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was muted by %s.\n\"", g_entities[idnum].client->pers.netname, adm->client->pers.netname));
				Boe_adminLog (va("%s - MUTE: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
			}
			else{
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %sm%su%st%se%sd", level.time + 5000, g_entities[idnum].client->pers.netname, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
				trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s ^7was muted.\n\"", g_entities[idnum].client->pers.netname));
				Boe_adminLog (va("%s - MUTE: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
			}
		}
	}else{
		if(g_entities[idnum].client->sess.mute == qfalse){
			trap_SendServerCommand(adm-g_entities, va("print \"^3[Info] ^7This client is not muted!\n\""));}
		else{
			Boe_GlobalSound(G_SoundIndex("sound/misc/menus/click.wav"));
			g_entities[idnum].client->sess.mute = qfalse;
			if(adm && adm->client){
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %su%sn%sm%su%st%sed by %s", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
				trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was unmuted by %s.\n\"", g_entities[idnum].client->pers.netname, adm->client->pers.netname));
				Boe_adminLog (va("%s - UNMUTE: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
			}
			else{
				trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %su%sn%sm%su%st%sed", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
				trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s ^7was unmuted.\n\"", g_entities[idnum].client->pers.netname));
				Boe_adminLog (va("%s - UNMUTE: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
			}
		}
	}
}

/*
=========
Boe_Strip
=========
*/

void Boe_Strip (int argNum, gentity_t *adm)
{
	gentity_t	*ent;
	gclient_t	*client;
	int			idnum;
	int			idle;

	idnum = Boe_ClientNumFromArg(adm, argNum, "strip <idnumber>", "strip", qtrue, qtrue);
	if(idnum < 0)
		return;

	ent    = g_entities + idnum;
	client = ent->client;
	
	ent->client->ps.zoomFov = 0;
	ent->client->ps.pm_flags &= ~(PMF_GOGGLES_ON|PMF_ZOOM_FLAGS);
	client->ps.stats[STAT_WEAPONS] = 0;
	client->ps.stats[STAT_GOGGLES] = GOGGLES_NONE;  
	memset ( client->ps.ammo, 0, sizeof(client->ps.ammo) );
	memset ( client->ps.clip, 0, sizeof(client->ps.clip) );
	client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_KNIFE );
	client->ps.clip[ATTACK_NORMAL][WP_KNIFE]=weaponData[WP_KNIFE].attack[ATTACK_NORMAL].clipSize;
	client->ps.firemode[WP_KNIFE] = BG_FindFireMode ( WP_KNIFE, ATTACK_NORMAL, WP_FIREMODE_AUTO );
	client->ps.weapon = WP_KNIFE;
	BG_GetInviewAnim(client->ps.weapon,"idle",&idle);
	client->ps.weaponAnimId = idle;
	client->ps.weaponstate = WEAPON_READY;
	client->ps.weaponTime = 0;
	client->ps.weaponAnimTime = 0;
	client->ps.stats[STAT_OUTFIT_GRENADE] = bg_itemlist[bg_outfittingGroups[-1][client->pers.outfitting.items[-1]]].giTag;

	if(adm && adm->client){
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %ss%st%sr%si%sp%sped by %s", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string, adm->client->pers.netname));
		trap_SendServerCommand(-1, va("print\"^3[Admin Action] ^7%s ^7was stripped by %s.\n\"", g_entities[idnum].client->pers.netname,adm->client->pers.netname));
		Boe_adminLog (va("%s - STRIP: %s", adm->client->pers.cleanName, g_entities[idnum].client->pers.cleanName  )) ;
	}
	else{
		trap_SetConfigstring ( CS_GAMETYPE_MESSAGE, va("%i,@^7%s ^7was %ss%st%sr%si%sp%sped", level.time + 5000, g_entities[idnum].client->pers.netname, server_color1.string, server_color2.string, server_color3.string, server_color4.string, server_color5.string, server_color6.string));
		trap_SendServerCommand(-1, va("print\"^3[Rcon Action] ^7%s ^7was stripped.\n\"", g_entities[idnum].client->pers.netname));
		Boe_adminLog (va("%s - STRIP: %s", "RCON", g_entities[idnum].client->pers.cleanName  )) ;
	}
}
