// Copyright (C) 2010 - Boe!Man.
//
// boe_utils.c - All the miscellaneous functions go here, e.g. tokens, motd, player menu, etc.

#include "g_local.h"
#include "boe_local.h"

// Temp decl until SQLite functions gets into one specific file.
#include <sys/types.h>
#include <sys/stat.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif

/*
================
RPM_WeaponMod
================
*/
void RPM_WeaponMod (void)
{
    void            *GP2, *group, *attackType;
    char            name[64], tmpStr[64];
    int             i, n, clipSize, numExtraClips, damage, splashRadius;
    int             *ammoMaxs;
    attackData_t    *attack;
    char            WpnFile[64];

    ammoMaxs = calloc(level.ammoMax * ATTACK_MAX, sizeof(int));

    // Henk 06/04/10 -> Different wpn files(H&S, Real Damage, Normal Damage)
    if(current_gametype.value == GT_HS){
        if(level.crossTheBridge){
            strcpy(WpnFile, "weaponfiles/ctb.wpn");
        }else{ // Boe!Man 5/28/11: Enable the loading of  custom weapon files.
            if(!*g_customWeaponFile.string || !Q_stricmp ( g_customWeaponFile.string, "none" ) ){
                strcpy(WpnFile, "weaponfiles/h&s.wpn");
            }else{
                strcpy(WpnFile, g_customWeaponFile.string);
            }
        }
    }else if(current_gametype.value == GT_HZ){
        if(!*g_customWeaponFile.string || !Q_stricmp ( g_customWeaponFile.string, "none" ) ){
            strcpy(WpnFile, "weaponfiles/zombies.wpn");
        }else{
            strcpy(WpnFile, g_customWeaponFile.string);
        }
    }else{
        // Boe!Man 5/28/11: Enable the loading of  custom weapon files.
        if(!*g_customWeaponFile.string || !Q_stricmp ( g_customWeaponFile.string, "none" ) ){
            if(g_instaGib.integer == 1){
                strcpy(WpnFile, "weaponfiles/rd.wpn");
            }else{
                strcpy(WpnFile, "weaponfiles/nd.wpn");
            }
        }else{
            strcpy(WpnFile, g_customWeaponFile.string);
        }
    }

    GP2 = trap_GP_ParseFile(WpnFile, qtrue, qfalse);
    if (!GP2)
    {
        //Com_Printf("^1Error in file: \"%s\" or file not found\n", WpnFile);
        G_LogPrintf("Error in file: \"%s\" or file not found\n", WpnFile);
        if(ammoMaxs != NULL){
            free(ammoMaxs);
        }
        return;
    }

    G_LogPrintf("Loading weapon mods from: %s\n", WpnFile);

    group = trap_GPG_GetSubGroups(GP2);

    while(group)
    {
        trap_GPG_FindPairValue(group, "name", "", name);

        for(i = 0; i < level.wpNumWeapons; i++)
        {
            if (Q_stricmp(bg_weaponNames[i], name) == 0)
            {
                for(n = ATTACK_NORMAL; n < ATTACK_MAX; n++)
                {
                    if(n == ATTACK_NORMAL)
                    {
                        attackType = trap_GPG_FindSubGroup(group, "attack");
                    }
                    else if(n == ATTACK_ALTERNATE)
                    {
                        attackType = trap_GPG_FindSubGroup(group, "altattack");
                    }

                    if ( NULL == attackType )
                    {
                        continue;
                    }

                    attack = &weaponData[i].attack[n];

                    trap_GPG_FindPairValue(attackType, "mp_clipSize||clipSize", "-1", tmpStr);
                    clipSize = atoi(tmpStr);
                    if(clipSize < 0) //If no value specified in the file use default vaule
                    {
                        clipSize = attack->clipSize;
                    }

                    trap_GPG_FindPairValue(attackType, "mp_radius||radius", "-1", tmpStr);
                    splashRadius = atoi(tmpStr);
                    if(splashRadius < 0) //If no value specified in the file use default vaule
                    {
                        splashRadius = attack->splashRadius;
                    }else
                        attack->splashRadius = splashRadius;

                    trap_GPG_FindPairValue(attackType, "mp_extraClips", "-1", tmpStr );
                    numExtraClips = atoi ( tmpStr );
                    if(numExtraClips < 0)
                    {
                        numExtraClips = attack->extraClips;
                    }

                    trap_GPG_FindPairValue(attackType, "mp_damage||damage", "-1", tmpStr);
                    damage = atoi(tmpStr);
                    if(damage < 0)
                    {
                        damage = attack->damage;
                    }

                    ammoMaxs[attack->ammoIndex] += clipSize * numExtraClips;

                    //if(g_weaponModFlags.integer & AMMO_MOD)
                    //{
                        attack->clipSize = clipSize;
                        attack->extraClips = numExtraClips;
                        ammoData[attack->ammoIndex].max = ammoMaxs[attack->ammoIndex];
                    //}

                    //if(g_weaponModFlags.integer & DAMAGE_MOD)
                    //{
                        attack->damage = damage;
                    //}
                }
            }
        }

        group = trap_GPG_GetNext(group);
    }

    trap_GP_Delete(&GP2);

    // Free allocated memory.
    if (ammoMaxs != NULL) {
        free(ammoMaxs);
    }
}

/*
===================
Boe_Motd by boe
3/30/10 - 10:58 AM
===================
*/

void Boe_Motd (gentity_t *ent)
{
    char    gmotd[1024] = "\0";
    char    motd[1024] = "\0";
    char    *s = motd;
    char    *gs = gmotd;
    char    name[36];
    char    *header1 = va("%s - %s\n", MODFULL_COLORED, __DATE__);

    strcpy(name, ent->client->pers.netname);

    Com_sprintf(gmotd, 1024, "%s%s%s\n%s\n%s\n%s\n%s\n%s\n",
        header1,
        MODDESC,
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
    G_Broadcast(motd, BROADCAST_MOTD, ent);
}

/*
===================
Boe_adminLog by boe
Original: 3/30/10 - 12:42 PM
Updated: 3/13/11 - 7:10 PM
===================
*/

void QDECL Boe_adminLog( const char *command, const char *by, const char *to, ...)
{
    char        string[1024] = "";
    int         len, i;
    va_list     argptr;
    va_list     argptr2 = "";
    qtime_t     q;
    fileHandle_t    f;
    char        temp[1024] = "";

    // Boe!Man 3/13/11: If they disabled the Admin Log, return.
    if(g_enableAdminLog.integer < 1){
        return;
    }

    trap_RealTime (&q);

    Com_sprintf( string, sizeof(string), "%02i/%02i/%i %02i:%02i ", 1+q.tm_mon,q.tm_mday, q.tm_year+1900,q.tm_hour,q.tm_min); // Boe!Man 3/13/11: Write the date.
    va_start( argptr, to );
    vsprintf( string + 17, command, argptr ); // Boe!Man 3/13/11: Append the command.
    va_end( argptr);
    // Boe!Man 3/13/11: Make sure the rest of the block gets filled with spaces.
    len = strlen(command);
    for(i = len + 17;i < 37; i++){
        string[i] = ' ';
    }
    string[37] = 'b';
    string[38] = 'y';
    string[39] = ' ';

    if((strstr(by, "RCON")) && (!strstr(by, "\\"))){
        vsprintf( string + 40, by, argptr2); // Boe!Man 3/13/11: Append RCON (as he did it).
    }else{
        strncpy(temp, by, sizeof(temp));
        vsprintf( string + 40, temp, argptr2); // Boe!Man 3/13/11: Append the Admin who did it.
    }
    va_end( argptr2 );
    // Boe!Man 3/13/11: Make sure the rest of the block gets filled with spaces.
    if(strlen(to) > 5){
        len = strlen(by);
        for(i = len + 40;i < 90; i++){
            string[i] = ' ';
        }
    }

    if(strlen(to) > 5){ // Boe!Man 3/13/11: Only append if the 'to' char contains something useful.
        string[90] = 't';
        string[91] = 'o';
        string[92] = ' ';

        va_start( argptr, to);
        strncpy(temp, to, sizeof(temp));
        vsprintf( string + 93, temp, argptr); // Boe!Man 3/13/11: Append the client who got it (can't be RCON).
        va_end( argptr );
    }

    trap_FS_FOpenFile(g_adminlog.string, &f, FS_APPEND_TEXT);

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
gspawn_t* G_SelectRandomSafeSpawnPoint ( team_t team, float safeDistance, gclient_t *client );

chatSounds_t chatSounds[MAX_BOE_CHATS];

char* defaultChatSounds[][2] =
{
    {"Call for reinforcements",         "sound/enemy/english/male/call_for_reinforcements.mp3"  },
    {"Advance",                         "sound/enemy/english/male/advance.mp3"                  },
    {"Awaiting orders",                 "sound/enemy/english/male/awaiting_orders.mp3"          },
    {"Check the perimiter",             "sound/enemy/english/male/check_peri.mp3"               },
    {"Go check that out",               "sound/enemy/english/male/check_that_out.mp3"           },
    {"He's dangerous",                  "sound/enemy/english/male/dangerous.mp3"                },
    {"Did you hear that",               "sound/enemy/english/male/did_you_hear.mp3"             },
    {"He's disappeared",                "sound/enemy/english/male/disappeared.mp3"              },
    {"Drop your weapon",                "sound/enemy/english/male/drop_your_weapon.mp3"         },
    {"Eliminate target",                "sound/enemy/english/male/eliminate.mp3"                },
    {"GET HIM",                         "sound/enemy/english/male/get_him.mp3"                  },
    {"I got that bastard",              "sound/enemy/english/male/got_bastard.mp3"              },
    {"Hold here",                       "sound/enemy/english/male/hold_here.mp3"                },
    {"Hurry",                           "sound/enemy/english/male/hurry.mp3"                    },
    {"I'm hit",                         "sound/npc/col8/blakely/imhit.mp3"                      },
    {"Investigate that area",           "sound/enemy/english/male/investigate_area.mp3"         },
    {"Keep looking",                    "sound/enemy/english/male/keep_looking.mp3"             },
    {"KILL HIM",                        "sound/enemy/english/male/kill_him"                     },
    {"I killed him",                    "sound/enemy/english/male/killed_him.mp3"               },
    {"I lost him",                      "sound/enemy/english/male/lost_him.mp3"                 },
    {"TAKING SERIOUS CASUALTIES",       "sound/enemy/english/male/man_down03.mp3"               },
    {"I need help here",                "sound/enemy/english/male/need_help.mp3"                },
    {"Medic Medic",                     "sound/enemy/english/male/medic_medic.mp3"              },
    {"Open Fire",                       "sound/enemy/english/male/open_fire.mp3"                },
    {"I'm out of Ammo",                 "sound/enemy/english/fmale/out_ammo.mp3"                },
    {"He's over here",                  "sound/enemy/english/male/over_here.mp3"                },
    {"Over there",                      "sound/enemy/english/male/over_there.mp3"               },
    {"Plug him",                        "sound/enemy/english/male/plug_him.mp3"                 },
    {"Position reached",                "sound/enemy/english/male/pos_reached.mp3"              },
    {"Secure the area",                 "sound/enemy/english/male/secure_the_area.mp3"          },
    {"We're getting slaughtered",       "sound/enemy/english/male/slaughtered.mp3"              },
    {"Somethings not right here",       "sound/enemy/english/male/something_not_right.mp3"      },
    {"Spread out",                      "sound/enemy/english/male/spread_out.mp3"               },
    {"Surround him",                    "sound/enemy/english/male/surround_him.mp3"             },
    {"Take cover",                      "sound/enemy/english/male/take_cover2.mp3"              },
    {"Take him out",                    "sound/enemy/english/male/take_him_out.mp3"             },
    {"Take Position",                   "sound/enemy/english/male/take_position.mp3"            },
    {"Take them out",                   "sound/enemy/english/male/take_them_out.mp3"            },
    {"Target has been eliminated",      "sound/enemy/english/male/target_eliminate.mp3"         },
    {"There he is",                     "sound/enemy/english/male/there_he_is.mp3"              },
    {"Taking alot of fire",             "sound/enemy/english/male/underfire03.mp3"              },
    {"WATCH OUT",                       "sound/enemy/english/male/watch_out.mp3"                },
    {"What are you doing here",         "sound/enemy/english/male/what_are_you_doing.mp3"       },
    {"What the...",                     "sound/enemy/english/male/what_the.mp3"                 },
    {"What was that",                   "sound/enemy/english/male/what_was_that.mp3"            },
    {"Whats happening here",            "sound/enemy/english/male/whats_happening_here.mp3"     },
    {"Who are you",                     "sound/enemy/english/male/who_are_you.mp3"              },
    {"You're not supposed to be here",  "sound/enemy/english/male/youre_not_supposed.mp3"       },
    {"Hey did you see that guy, He wet his pants ha haha ha ha..", "sound/npc/air1/guard01/01wetpants.mp3" },
    {"Hey honey..",                     "sound/npc/air1/guard05/01honey.mp3"                    },
    {"Huh I think i can help you",      "sound/npc/gladstone/arm1/03canhelp.mp3"                },
    {"If you look at me 1 more time, I swear I'll BLOW YOUR HEAD OFF", "sound/npc/air1/terrorists/blowheadoff.mp3" },
    {"How bout we see if you can dodge a BULLET", "sound/npc/air1/terrorists/dodgebullet.mp3"   },
    {"That was a close one",            "sound/npc/fritsch/closeone.mp3"                        },
    {"What are you lookin at",          "sound/npc/air1/terrorists/lookingat.mp3"               },
    {"You scared?, You should be",      "sound/npc/air1/terrorists/scared02.mp3"                },
    {"Leave me alone",                  "sound/npc/air1/woman01/01leaveme.mp3"                  },
    {"Please just leave me be..",       "sound/npc/air1/woman01/02please.mp3"                   },
    {"What? Please explain.. over",     "sound/npc/air4/hansen/04what.mp3"                      },
    {"Attention",                       "sound/npc/cem1/comm/01attention.mp3"                   },
    {"Someone call 911",                "sound/npc/civ/english/male/call_911.mp3"               },
    {"Call the police",                 "sound/npc/civ/english/male/callpolice.mp3"             },
    {"Comin thru..",                    "sound/npc/civ/english/male/coming_thru.mp3"            },
    {"NICE SHOT",                       "sound/npc/col8/washington/niceshot.mp3"                },
    {"Excuse me",                       "sound/npc/civ/english/male/excuse_me.mp3"              },
    {"Don't hurt me",                   "sound/npc/civ/english/male/dont_hurt.mp3"              },
    {"I don't understand",              "sound/npc/civ/english/male/dont_understand.mp3"        },
    {"I don't have any money",          "sound/npc/civ/english/male/no_money.mp3"               },
    {"Those men have guns",             "sound/npc/civ/english/male/thosemen.mp3"               },
    {"Take my wallet, Just don't hurt me", "sound/npc/civ/english/male/wallet.mp3"              },
    {"I'm unarmed",                     "sound/npc/civ/english/male/unarmed.mp3"                },
    {"Watchout",                        "sound/npc/civ/english/male/watchout.mp3"               },
    {"Pairup",                          "sound/npc/col2/butch/pairup.mp3"                       },
    {"Fanout",                          "sound/npc/col2/butch/fanout.mp3"                       },
    {"We got company, Take cover",      "sound/npc/col3/peterson/12company.mp3"                 },
    {"Enemy has been neutralized, base camp is secure. over", "sound/npc/col3/radio/02enemy.mp3" },
    {"BANG! Your dead",                 "sound/npc/col8/blakely/bang.mp3"                       },
    {"DAMN! that was close",            "sound/npc/col8/blakely/close.mp3"                      },
    {"GET DOWN",                        "sound/npc/col8/blakely/getdown.mp3"                    },
    {"Come get some",                   "sound/npc/col8/blakely/getsome.mp3"                    },
    {"Incoming",                        "sound/npc/col8/blakely/incoming.mp3"                   },
    {"Go cry to mama",                  "sound/npc/col8/blakely/mama.mp3"                       },
    {"We showed them",                  "sound/npc/col8/blakely/showed.mp3"                     },
    {"Take that",                       "sound/npc/col8/blakely/takethat.mp3"                   },
    {"That did it",                     "sound/npc/col8/blakely/thatdidit.mp3"                  },
    {"You want some of this",           "sound/npc/col8/blakely/wantsome.mp3"                   },
    {"Yeah",                            "sound/npc/col8/blakely/yeah.mp3"                       },
    {"WOOOO",                           "sound/npc/col8/blakely/woo.mp3"                        },
    {"Don't worry girls it'll all be over soon", "sound/npc/col8/peterson/01girls.mp3"          },
    {"Cover my left",                   "sound/npc/col8/peterson/coverleft.mp3"                 },
    {"Cover my right",                  "sound/npc/col8/peterson/coverright.mp3"                },
    {"Keep your eyes open boys",        "sound/npc/col8/peterson/eyesopen.mp3"                  },
    {"Get moving",                      "sound/npc/col8/peterson/getmoving02.mp3"               },
    {"Shudd-up man",                    "sound/npc/col8/washington/02shuddup.mp3"               },
    {"Eat lead sukka",                  "sound/npc/col8/washington/eatlead.mp3"                 },
    {"Kickin ass, and takin names",     "sound/npc/col8/washington/takingnames.mp3"             },
    {"How'd you like that",             "sound/npc/col8/washington/youlike.mp3"                 },
    {"WoOoOo",                          "sound/npc/col8/washington/woo.mp3"                     },
    {"O.K, lets do it",                 "sound/npc/col9/pilot/03letsdoit.mp3"                   },
    {"We can't take much more of this", "sound/npc/col9/pilot/canttake.mp3"                     },
    {NULL, NULL}
};

/*
==============
Boe_Tokens
==============
*/
void Boe_Tokens(gentity_t *ent, char *chatText, int mode, qboolean CheckSounds)
{
    int  i = 0, n = 0;
    qboolean playedSound = qfalse, ghost = qfalse;
    char location[64] = "\0";
    char newText[MAX_SAY_TEXT] = "\0";
    char *newTextp;
    char *chatTextS;
    char check;
    gentity_t *tent;
    qboolean text = qtrue;
    //gitem_t* item;

    if(ent->client->sess.mute)
    {
        return;
    }

    newTextp = newText;
    chatTextS = chatText;
    while(*chatText && newTextp < newText + MAX_SAY_TEXT )
    {
        chatText--;
        check = *chatText;
        chatText++;
        if (*chatText == '#' && check != '^')
        {
            chatText++;

            switch(*chatText)
            {
                case 'b':   // Boe!Man 5/9/10: Health bar in stat format.
                    Q_strcat(newText, MAX_SAY_TEXT, va("^1%s^2", Boe_BarStat(ent->health)));
                    chatText++;
                    continue;

                case 'B':   // Boe!Man 5/9/10: Armor bar in stat format.
                    Q_strcat(newText, MAX_SAY_TEXT, va("^-%s^2", Boe_BarStat(ent->client->ps.stats[STAT_ARMOR])));
                    chatText++;
                    continue;

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
                    if(ent->client->pers.statinfo.lasthurtby == -1 || !g_entities[ent->client->pers.statinfo.lasthurtby].client)
                        Q_strcat(newText, MAX_SAY_TEXT, va("none"));
                    else
                    Q_strcat(newText, MAX_SAY_TEXT, va("%s", g_entities[ent->client->pers.statinfo.lasthurtby].client->pers.netname));
                    chatText++;
                    continue;
                case 't':
                case 'T':
                    if(ent->client->pers.statinfo.lastclient_hurt == -1)
                        Q_strcat(newText, MAX_SAY_TEXT, va("none"));
                    else
                    Q_strcat(newText, MAX_SAY_TEXT, va("%s", g_entities[ent->client->pers.statinfo.lastclient_hurt].client->pers.netname));
                    chatText++;
                    continue;
                case 'r':
                case 'R':
                    if(current_gametype.value == GT_HS){
                    Q_strcat(newText, MAX_SAY_TEXT, va("%s", level.RPGloc));
                    chatText++;
                    }else
                        chatText++;
                    continue;
                case 'c':
                case 'C':
                    if(current_gametype.value == GT_HS){
                    Q_strcat(newText, MAX_SAY_TEXT, va("%s", level.M4loc));
                    chatText++;
                    }else
                        chatText++;
                    continue;
                case 'm':
                case 'M':
                    if(current_gametype.value == GT_HS){
                    Q_strcat(newText, MAX_SAY_TEXT, va("%s", level.MM1loc));
                    chatText++;
                    }else
                        chatText++;
                    continue;
                case '?':
                    if(current_gametype.value == GT_HS){
                    Q_strcat(newText, MAX_SAY_TEXT, va("%s", level.RandomNadeLoc));
                    chatText++;
                    }else
                        chatText++;
                    continue;
                case 'n':
                case 'N':
                        Q_strcat(newText, MAX_SAY_TEXT, va("%s", g_motd.string));
                        if ( ent->client->sess.voiceFloodCount >= g_voiceFloodCount.integer && g_voiceFloodCount.integer != 0) {
                            ent->client->sess.voiceFloodCount = 0;
                            ent->client->sess.voiceFloodTimer = 0;
                            ent->client->sess.voiceFloodPenalty = level.time + g_voiceFloodPenalty.integer * 1000;
                            trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Voice chat flooded, you will be able use voice chats again in %d seconds.\n\"", g_voiceFloodPenalty.integer ) );
                        }else if ( ent->client->sess.voiceFloodPenalty > level.time ) {
                            //trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Voice chat flooded, you will be able use voice chats again in %d seconds.\n\"", g_voiceFloodPenalty.integer ) );
                        }else if(!playedSound && strlen(g_motd.string) > 0){
                            // Boe!Man 7/3/13: Check if this doesn't happen twice, in order to avoid multiple sounds in one sentence.
                            ent->client->sess.voiceFloodCount++; // add one to floodcount as they could massively flood this
                            if( level.gametypeData->respawnType == RT_NONE){
                                if ( G_IsClientDead ( ent->client ) )
                                    ghost = qtrue;
                            }

                            if(ghost){
                                for (n = 0; n < level.numConnectedClients; n++){
                                    tent = &g_entities[level.sortedClients[n]];
                                    if (!tent || !tent->inuse || !tent->client)
                                        continue;
                                    if (!G_IsClientDead ( tent->client ) && !G_IsClientSpectating( tent->client))
                                        continue;

                                    if(level.nextSound)
                                        Boe_ClientSound(tent, chatSounds[level.nextSound].sound);
                                }
                            }else if(ent->client->sess.team == TEAM_SPECTATOR && current_gametype.value != GT_DM){
                                for (n = 0; n < level.numConnectedClients; n++){
                                    tent = &g_entities[level.sortedClients[n]];
                                    if (!tent || !tent->inuse || !tent->client)
                                        continue;
                                    if(tent->client->sess.team != TEAM_SPECTATOR && !G_IsClientDead(tent->client))
                                        continue;

                                    if(level.nextSound)
                                    Boe_ClientSound(tent, chatSounds[level.nextSound].sound);
                                }
                            }else if(level.nextSound){
                                Boe_GlobalSound(chatSounds[level.nextSound].sound);
                            }

                            // Check if the sound played, and if so, increment the voice flood count.
                            if (level.nextSound) {
                                ent->client->sess.voiceFloodCount++;
                                playedSound = qtrue;
                            }
                        }
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
        if(((*chatText == '@') || (*chatText == '!') || (*chatText == '&')) && !playedSound && CheckSounds){
        if (*chatText == '!' || *chatText == '&')
            text = qfalse;
        chatText++;
        if(*chatText >= '0' && *chatText <= '9'){ // Boe!Man 2/15/13: Only continue if there are indeed sounds after the '@', '!' or '&' char. If not, the client probably meant something else.
            chatText--;
            if ( g_voiceFloodCount.integer ) {
                if ( ent->client->sess.voiceFloodPenalty ) {
                    if ( ent->client->sess.voiceFloodPenalty > level.time ) {
                        // Boe!Man 11/5/12: Fix for no text when on flood penalty.
                        chatText++;
                        n = atoi(chatText) - 1;
                        if(n > -1 && n < MAX_BOE_CHATS && *chatSounds[n].text && text) {
                            Q_strcat(newText, MAX_SAY_TEXT, chatSounds[n].text);
                            chatText++;
                        }
                        while (*chatText >= '0' && *chatText <= '9'){
                            chatText++;
                        }
                        playedSound = qtrue;
                        continue;
                    }
                    ent->client->sess.voiceFloodPenalty = 0;
                }
                if ( ent->client->sess.voiceFloodCount >= g_voiceFloodCount.integer ) {
                    ent->client->sess.voiceFloodCount = 0;
                    ent->client->sess.voiceFloodTimer = 0;
                    ent->client->sess.voiceFloodPenalty = level.time + g_voiceFloodPenalty.integer * 1000;
                    // Boe!Man 12/20/09 - Update 12/22/09 [Yellow color instead of Red].
                    trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Voice chat flooded, you will be able use voice chats again in %d seconds.\n\"", g_voiceFloodPenalty.integer ) );
                    // Boe!Man 11/5/12: Fix for no text when on flood penalty.
                    chatText++;
                    n = atoi(chatText) - 1;
                    if(n > -1 && n < MAX_BOE_CHATS && *chatSounds[n].text && text) {
                        Q_strcat(newText, MAX_SAY_TEXT, chatSounds[n].text);
                        chatText++;
                    }
                    while (*chatText >= '0' && *chatText <= '9'){
                        chatText++;
                    }
                    playedSound = qtrue;
                    continue;
                }
                if (g_compMode.integer > 0 && cm_enabled.integer > 1){ // Boe!Man 11/20/10: Meaning the scrim already started..
                    if (cm_devents.integer == 1){
                        chatText++;
                        n = atoi(chatText) - 1;
                        if(n > -1 && n < MAX_BOE_CHATS && *chatSounds[n].text && text) {
                            Q_strcat(newText, MAX_SAY_TEXT, chatSounds[n].text);
                            chatText++;
                        }
                        while (*chatText >= '0' && *chatText <= '9'){
                            chatText++;
                        }
                        trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7Sounds are currently disabled in Competition Mode.\n\"" ) );
                        playedSound = qtrue;
                        continue;
                    }
                }
            }
            //if(*chatText == '!') { // fix me henk
            //  text = qfalse;
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
            else{
                return;
            }
            if(i) {
                if( level.gametypeData->respawnType == RT_NONE) {
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
                    ent->client->sess.voiceFloodCount++;
                    playedSound = qtrue;
                }
                else if(mode != SAY_TELL) {
                    if(ghost) {
                        for (n = 0; n < level.numConnectedClients; n++){
                            tent = &g_entities[level.sortedClients[n]];
                            if (!tent || !tent->inuse || !tent->client)
                                continue;
                            if (!G_IsClientDead ( tent->client ) && !G_IsClientSpectating( tent->client))
                                continue;
                            Boe_ClientSound(tent, i);
                        }
                    }else if(ent->client->sess.team == TEAM_SPECTATOR && current_gametype.value != GT_DM){
                        // Boe!Man 11/10/12: New code for specs (so alive clients can't hear them, only specs and ghosts). Do note that this should not work in DM (not a team based game).
                        for (n = 0; n < level.numConnectedClients; n++){
                            tent = &g_entities[level.sortedClients[n]];
                            if (!tent || !tent->inuse || !tent->client)
                                continue;
                            if(tent->client->sess.team != TEAM_SPECTATOR && !G_IsClientDead(tent->client))
                                continue;
                            Boe_ClientSound(tent, i);
                        }
                    }else{
                        Boe_GlobalSound(i);
                    }
                    ent->client->sess.voiceFloodCount++;
                    playedSound = qtrue;
                }
            }
                else {
                    return;
                }
                while (*chatText >= '0' && *chatText <= '9')
                    chatText++;
            }else{
                chatText--;
            }
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
int Boe_FindClosestTeammate(gentity_t *ent, qboolean bot)
{
    int         i;
    int         client = -1;
    gentity_t*  other;
    float       dist, bestDist = 99999;
    vec3_t      diff;

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
            if (!(other->r.svFlags & SVF_BOT))
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
int Boe_FindClosestEnemy(gentity_t *ent, qboolean bot)
{
    int         i;
    int         client = -1;
    gentity_t*  other;
    float       dist, bestDist = 99999;
    vec3_t      diff;

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
            if (!(other->r.svFlags & SVF_BOT))
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
    int         fileCount, filelen, i, numSounds, number;
    char        chatFiles[1024];
    char        *filePtr;
    char        *file;
    char        text[MAX_SAY_TEXT];
    char        sound[MAX_QPATH];
    char        numString[8];
    void        *GP2, *group;

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
                // Check if the global NEXT sound is present.
                if (strcmp(sound, "sound/next.mp3") == 0)
                    level.nextSound = number;

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

void Henk_CloseSound ( vec3_t origin, int soundIndex)
{
    gentity_t *tent;
    float radius = 100.0f;
    tent = G_TempEntity( origin, EV_GENERAL_SOUND ); //
    tent->r.svFlags |= SVF_BROADCAST;
    tent->s.time2 = (int) (radius * 1000.0f);
    G_AddEvent( tent, EV_GENERAL_SOUND, soundIndex );
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
    int soundIndex = 100;
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
    gentity_t   *missile;

    missile = G_Spawn();

    if(weapon == WP_M84_GRENADE || weapon == WP_M15_GRENADE){
        missile->r.singleClient = ent->client->ps.clientNum;
        missile->r.svFlags |= SVF_SINGLECLIENT;
    }
    else {
        missile->r.svFlags |= SVF_BROADCAST;
        missile->damage             = 1;
        missile->splashDamage       = 1;
    }
    missile->nextthink              = level.time + 1000;
    missile->think                  = G_ExplodeMissile;
    missile->s.eType                = ET_MISSILE;
    missile->s.weapon               = weapon;
    missile->r.ownerNum             = ent->s.number;
    missile->parent                 = ent;
    missile->classname              = "grenade";
    missile->splashRadius           = 500;
    missile->s.eFlags               = EF_BOUNCE_HALF | EF_BOUNCE;
    missile->clipmask               = MASK_SHOT | CONTENTS_MISSILECLIP;
    //missile->s.pos.trType         = TR_INTERPOLATE;
    missile->s.pos.trType           = TR_HEAVYGRAVITY;
    missile->s.pos.trTime           = level.time - 50;

    if(weapon == WP_ANM14_GRENADE){
        missile->splashRadius       = 150;
        missile->dflags             = DAMAGE_AREA_DAMAGE;
        missile->splashDamage       = 1;
        missile->methodOfDeath      = MOD_ANM14_GRENADE;
    }

    VectorCopy( start,  missile->s.pos.trBase );
    VectorCopy( dir,    missile->s.pos.trDelta );

    if ( ent->client->ps.pm_flags & PMF_JUMPING )
        VectorAdd( missile->s.pos.trDelta, ent->s.pos.trDelta, missile->s.pos.trDelta );

    SnapVector( missile->s.pos.trDelta );           // save net bandwidth

    VectorCopy (start, missile->r.currentOrigin);
    return missile;
}

/*
============
Boe_Players
============
*/

void Boe_Players(gentity_t *ent)
{
    char *admin;
    char *clan;
    qboolean hasClient;
    char client[12], id[9], ping[10], name[41];
    char *mute;
    int i, color;

    // Boe!Man 5/30/15: Only print the country column if countries are indeed enabled.
    if (g_checkCountry.integer > 0) {
        trap_SendServerCommand(ent - g_entities, va("print \"\n^3%-5s%-37s %-5s Coun Adm Cln Mut Ver\n\"", "Id#", "Name", "Ping"));
    }else{
        trap_SendServerCommand(ent - g_entities, va("print \"\n^3%-5s%-37s %-5s Adm Cln Mut Ver\n\"", "Id#", "Name", "Ping"));
    }

    trap_SendServerCommand( ent-g_entities, "print \"^7------------------------------------------------------------------------\n\"");

    // Loop through clients.
    for ( i = 0; i < MAX_CLIENTS; i ++ )
    {
        if(level.clients[i].pers.connected != CON_CONNECTED)
        {
            continue;
        }

        // Take note of player specifics.
        Q_strncpyz(id, va("[^3%d^7]", i), sizeof(id));
        Q_strncpyz(ping, va("[^3%d^7]", level.clients[i].ps.ping < 999 ? level.clients[i].ps.ping : 999), sizeof(ping));

        if (i == ent->s.number) {
            color = 5;
        }
        else {
            switch (level.clients[i].sess.team)
            {
            case TEAM_RED:
                color = 1;
                break;
            case TEAM_BLUE:
                color = 4;
                break;
            default:
                color = 7;
                break;
            }
        }
        // We use the found color for the name.
        Q_strncpyz(name, va("[^%d%s^7]", color, level.clients[i].pers.cleanName), sizeof(name));

        #ifdef _3DServer
        // 3D.
        if (level.clients[i].sess.admin == 2)
            admin = "[^3A^7]";
        else if (level.clients[i].sess.admin >= 3)
            admin = "[^3S^7]";
        else
            admin = "";
        #else
        // Regular.
        if (level.clients[i].sess.admin == 2)
            admin = "[^3B^7]";
        else if(level.clients[i].sess.admin == 3)
            admin = "[^3A^7]";
        else if (level.clients[i].sess.admin == 4)
            admin = "[^3S^7]";
        else
            admin = "";
        #endif // _3DServer

        if (level.clients[i].sess.clanMember == qtrue)
            clan = "[^3C^7]";
        else
            clan = "";

        if (level.clients[i].sess.mute)
            mute = "[^3M^7]";
        else
            mute = "";

        // We want to know if the client indeed has a valid client.
        hasClient = qfalse;

        #ifndef _GOLD
        if (level.clientMod == CL_RPM){
            if (level.clients[i].sess.rpmClient >= 0.1){
                hasClient = qtrue;
                Q_strncpyz(client, va("[^3%2.f^7]", level.clients[i].sess.rpmClient), sizeof(client));
            }
            else if (level.clients[i].sess.proClient >= 0.1){
                hasClient = qtrue;
                Q_strncpyz(client, va("[^3P%2.f^7]", level.clients[i].sess.proClient), sizeof(client));
            }
        }
        #else
        if (level.clients[i].sess.rocModClient){
            hasClient = qtrue;
            Q_strncpyz(client, "[^32.1c^7]", sizeof(client));
        }
        #endif // not _GOLD

        if (!hasClient) {
            Q_strncpyz(client, "[^3N/A^7]", sizeof(client));
        }

        // Print the line containing client specifics.
        if (g_checkCountry.integer > 0) {
            trap_SendServerCommand(ent - g_entities, va("print \"%-9s%-42s%-10s[^3%s^7] %-3s %-3s %-3s %s\n\"",
                id, name, ping, level.clients[i].sess.countryext, admin, clan, mute, client));
        }else{
            trap_SendServerCommand(ent - g_entities, va("print \"%-9s%-42s%-10s%-3s %-3s %-3s %s\n\"",
                id, name, ping, admin, clan, mute, client));
        }
    }

    trap_SendServerCommand( ent-g_entities, va("print \"\nUse ^3[Page Up] ^7and ^3[Page Down] ^7keys to scroll\n\n\""));
}

/*
=========
Boe_Stats
Updated 5/14/11 - 9:50 PM
=========
*/

void Boe_Stats ( gentity_t *ent )
{
    statinfo_t  *stat;
    float       client;
    char        *client0;
    char        arg1[32];
    char        *rate;
    char        *snaps;
    char        *ip;
    char        *player;
    char        *admin;
    char        *country;
    qboolean    client1 = qfalse;
    char        userinfo[MAX_INFO_STRING];
    int         idnum, n;
    char        *altname;
    float       accuracy = 0;
    qboolean    otherClient = qfalse;
    int i, numberofclients = 0;
    char string[1024] = "\0";
    char string1[64] = "\0";
    char        cleanName[MAX_NETNAME];
    int remain, remainS;

    trap_Argv( 1, arg1, sizeof( arg1 ) );  // Boe!Man 2/21/10: Getting the client ID.

    // Boe!Man 2/21/10: If no ID is entered, just display the current client stats.
    if ((arg1[0] < '0' || arg1[0] > '9') && !henk_ischar(arg1[0]))
    {
        stat = &ent->client->pers.statinfo;
        ip      = ent->client->pers.ip;
        player  = ent->client->pers.netname;
        trap_GetUserinfo( ent->s.number, userinfo, sizeof( userinfo ) );
        rate    = Info_ValueForKey ( userinfo, "rate" );
        snaps   = Info_ValueForKey ( userinfo, "snaps" );

        // Boe!Man 5/14/11: Check if the checking of countries is enabled.
        if(g_checkCountry.integer && level.countryInitialized){
            country = ent->client->sess.country;
        }

        idnum = ent->s.number;
        #ifndef _GOLD
        if (level.clientMod == CL_RPM){
            if (ent->client->sess.rpmClient >= 0.1){
                client = ent->client->sess.rpmClient;
            }else if (ent->client->sess.proClient >= 0.1){
                client = ent->client->sess.proClient;
            }else{
                client0 = "N/A";
                client1 = qtrue;
            }
        }else{
            client0 = "N/A";
            client1 = qtrue;
        }
        #else
        if (level.clientMod == CL_ROCMOD && ent->client->sess.rocModClient) {
            client = ent->client->sess.rocModClient;
            client0 = "ROCmod 2.1c";
            client1 = qtrue;
        }else{
            client0 = "N/A";
            client1 = qtrue;
        }
        #endif // not _GOLD

#ifdef _3DServer
        if (ent->client->sess.admin == 2){
            admin = "Admin";
        }else if (ent->client->sess.admin >= 3){
            admin = "S-Admin";
        }else{
            admin = "No";
        }
#else
        if (ent->client->sess.admin == 2){
            admin = "B-Admin";
        }
        else if (ent->client->sess.admin == 3){
            admin = "Admin";
        }
        else if (ent->client->sess.admin == 4){
            admin = "S-Admin";
        }
        else{
            admin = "No";
        }
#endif // _3DServer
    }
    // Boe!Man 2/21/10: If a ID is entered, we're going to display that users' status.
    else{
        if(henk_ischar(arg1[0])){
            memset(string, 0, sizeof(string));
            memset(string1, 0, sizeof(string1));
            numberofclients = 0;
            for(i=0;i<level.numConnectedClients;i++){
                Q_strncpyz(cleanName, g_entities[level.sortedClients[i]].client->pers.cleanName, sizeof(cleanName)); // Boe!Man 2/20/13: With NULL terminator.
                Q_strlwr(cleanName);
                //trap_SendServerCommand(-1, va("print\"^3[Debug] ^7%s comparing with %s.\n\"", g_entities[level.sortedClients[i]].client->pers.cleanName,numb));
                if(strstr(cleanName, Q_strlwr(arg1))){
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
            }else if(numberofclients == 0){
                idnum = -1;
            }
        }else{
            idnum = atoi (arg1);
        }
        otherClient = qtrue;
        // Boe!Man 2/21/10: The client number needs to be valid.
        if ( idnum < 0 || idnum >= g_maxclients.integer )
        {
            trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7You haven't entered a valid player ID/player name.\n\""));
            return;
        }
        // Boe!Man 2/21/10: The client needs to be connected.
        if ( g_entities[idnum].client->pers.connected == CON_DISCONNECTED )
        {
            trap_SendServerCommand( ent-g_entities, va("print \"^3[Info] ^7The player is not connected.\n\""));
            return;
        }
        // Boe!Man 2/21/10: We can continue..
        stat = &g_entities[idnum].client->pers.statinfo;
        ip      = g_entities[idnum].client->pers.ip;
        player  = g_entities[idnum].client->pers.netname;
        trap_GetUserinfo( g_entities[idnum].s.number, userinfo, sizeof( userinfo ) );
        rate    = Info_ValueForKey ( userinfo, "rate" );
        snaps   = Info_ValueForKey ( userinfo, "snaps" );

        // Boe!Man 5/14/11: Check if the checking of countries is enabled.
        if(g_checkCountry.integer && level.countryInitialized){
            country = g_entities[idnum].client->sess.country;
        }

        #ifndef _GOLD
        if (level.clientMod == CL_RPM){
            if (g_entities[idnum].client->sess.rpmClient >= 0.1){
                client = g_entities[idnum].client->sess.rpmClient;
            }else if (g_entities[idnum].client->sess.proClient >= 0.1){
                client = g_entities[idnum].client->sess.proClient;
            }else{
                client0 = "N/A";
                client1 = qtrue;
            }
        }else{
            client0 = "N/A";
            client1 = qtrue;
        }
        #else
        if (level.clientMod == CL_ROCMOD && g_entities[idnum].client->sess.rocModClient) {
            client = g_entities[idnum].client->sess.rocModClient;
            client0 = "ROCmod 2.1c";
            client1 = qtrue;
        }else{
            client0 = "N/A";
            client1 = qtrue;
        }
        #endif // not _GOLD

        if (g_entities[idnum].client->sess.admin == 2){
            admin = "B-Admin";
        }else if (g_entities[idnum].client->sess.admin == 3){
            admin = "Admin";
        }else if (g_entities[idnum].client->sess.admin == 4){
            admin = "S-Admin";
        }else{
            admin = "No";
        }
    }
    // Boe!Man 2/21/10: Print the stuff.
    // Boe!Man 6/2/10: Tier 0: Header - Start.
    trap_SendServerCommand( ent-g_entities, va("print \"\n^3Player statistics for ^7%s\n\"", player));
    trap_SendServerCommand( ent-g_entities, va("print \"-------------------------------------------------------\n"));
    if(g_aliasCheck.integer > 0){ // Boe!Man 12/13/10: Only show when the Aliases are enabled.
        trap_SendServerCommand( ent-g_entities, va("print \"%-18s", "[^3Aliases^7]"));
        if (otherClient == qfalse){
            Boe_printAliases(ent, ip, ent->client->pers.cleanName);
        }else{
            Boe_printAliases(ent, ip, g_entities[idnum].client->pers.cleanName);
        }
        trap_SendServerCommand( ent-g_entities, va("print \"\n%-18s%s\n", "[^3Admin^7]", admin));
    }else{
        trap_SendServerCommand( ent-g_entities, va("print \"%-18s%s\n", "[^3Admin^7]", admin));
    }

    // Boe!Man 5/20/12: Check if g_publicIPs is set to 1. If not, hide in stats (this will prevent IP abuse by other players).
    if (g_publicIPs.integer && (g_publicIPs.integer == 1 || ent->client->sess.admin >= g_publicIPs.integer)){
        trap_SendServerCommand( ent-g_entities, va("print \"%-18s%s\n", "[^3IP^7]", ip));
    }
    // Boe!Man 5/14/11: Check if the checking of countries is enabled.
    if(g_checkCountry.integer && level.countryInitialized){
        trap_SendServerCommand( ent-g_entities, va("print \"%-18s%s\n", "[^3Country^7]", country));
    }

    if(client1){
        trap_SendServerCommand( ent-g_entities, va("print \"%-18s%s\n", "[^3Client^7]", client0));
    }else{
        if(strlen(g_entities[idnum].client->sess.strClient) >= 2){
            trap_SendServerCommand( ent-g_entities, va("print \"%-18s%s\n", "[^3Client^7]", g_entities[idnum].client->sess.strClient));
        }else{
            if(client >= 0.1){
                trap_SendServerCommand( ent-g_entities, va("print \"%-18s%.1f\n", "[^3Client^7]", client));
            }else{
                #ifndef _GOLD
                if(g_entities[idnum].client->sess.proClient >= 0.1){
                    trap_SendServerCommand( ent-g_entities, va("print \"%18-sP%.1f\n", "[^3Client^7]", g_entities[idnum].client->sess.proClient));
                }else
                #endif // not _GOLD
                    trap_SendServerCommand( ent-g_entities, va("print \"%-18sN/A\n", "[^3Client^7]"));
            }
        }
    }
    trap_SendServerCommand( ent-g_entities, va("print \"%-18s%s\n", "[^3Rate^7]", rate));
    trap_SendServerCommand( ent-g_entities, va("print \"%-18s%s\n", "[^3Snaps^7]", snaps));

    if( g_entities[idnum].client->sess.mute){
        for(i = 0; i < MAX_CLIENTS; i++){
            if(level.mutedClients[i].used == qtrue){
                if(strstr(level.mutedClients[i].ip,  g_entities[idnum].client->pers.ip)){
                    remain = ((level.mutedClients[i].startTime+((level.mutedClients[i].time*60)*1000)-level.time)/1000)/60;
                    remainS = ((level.mutedClients[i].startTime+((level.mutedClients[i].time*60)*1000)-level.time)/1000);
                    trap_SendServerCommand(  ent-g_entities, va("print \"%-18s%i:%02i minutes remaining\n\n", "[^3Mute^7]", remain, remainS-(remain*60)));
                    break; // Boe!Man 2/15/13: Duplicate line fix and speed optimize.
                }
            }
        }
    }else{
        trap_SendServerCommand( ent-g_entities, va("print \"\n"));
    }

    // Boe!Man 6/2/10: Tier 0 - End.

    // Boe!Man 6/2/10: Tier 1 - Start.
    trap_SendServerCommand( ent-g_entities, va("print \"[^3Total kills^7] [^3Total death^7] [^3Damage done^7] [^3Damage take^7]\n"));
    trap_SendServerCommand( ent-g_entities, va("print \"%7d%14d%14d%14d\n\n", stat->kills, stat->deaths, stat->damageDone, stat->damageTaken));
    // Boe!Man 6/2/10: Tier 1 - End.

    // Boe!Man 6/2/10: Tier 2 - Start.
    trap_SendServerCommand( ent-g_entities, va("print \"[^3Hand^7] [^3Foot^7] [^3Arms^7] [^3Legs^7] [^3Head^7] [^3Neck^7] [^3Tors^7] [^3Wais^7]\n"));
    trap_SendServerCommand( ent-g_entities, va("print \"%4d%7d%7d%7d%7d%7d%7d%7d\n", stat->handhits, stat->foothits, stat->armhits, stat->leghits, stat->headhits, stat->neckhits, stat->torsohits, stat->waisthits));
    // Boe!Man 6/2/10: Tier 2 - End.

    // Boe!Man 6/2/10: Tier 3: Weapon Stats - Start.
    if(stat->shotcount)
        {
        trap_SendServerCommand( ent-g_entities, va("print \"\n%-22s%-13s%-13s%-13s[^3Accu^7]\n\"", "[^3Weapon^7]", "[^3Shot^7]", "[^3Hits^7]", "[^3Head^7]"));
        for(n = 0; n < level.wpNumWeapons; n++)
            {
            if(stat->weapon_shots[ATTACK_NORMAL * level.wpNumWeapons + n] <= 0 && stat->weapon_shots[ATTACK_ALTERNATE * level.wpNumWeapons + n] <=0)
                {
                    continue;
                }
                accuracy = 0;
                if(stat->weapon_shots[ATTACK_NORMAL * level.wpNumWeapons + n])
                    {
                        accuracy = (float)stat->weapon_hits[ATTACK_NORMAL * level.wpNumWeapons + n] / (float)stat->weapon_shots[ATTACK_NORMAL * level.wpNumWeapons + n] * 100;
                    }
                trap_SendServerCommand( ent-g_entities, va("print \"^3%14s^7%9d^7%9d^7%9d%7s%3.2f\n\"",
                bg_weaponNames[n],
                stat->weapon_shots[ATTACK_NORMAL * level.wpNumWeapons + n],
                stat->weapon_hits[ATTACK_NORMAL * level.wpNumWeapons + n],
                stat->weapon_headshots[ATTACK_NORMAL * level.wpNumWeapons + n],
                "^7",
                accuracy));

                if(stat->weapon_shots[ATTACK_ALTERNATE * level.wpNumWeapons + n])
                {
                    switch(n)
                    {
                    case WP_AK74_ASSAULT_RIFLE:
                        altname = "Bayonette";
                        break;

                    case WP_M4_ASSAULT_RIFLE:
                        altname = "M203";
                        break;

                    case WP_M590_SHOTGUN:
                        altname = "Bludgeon";
                        break;

                    case WP_M1911A1_PISTOL:
                    case WP_USSOCOM_PISTOL:
                        altname = "Pistol Whip";
                        break;

                    default:
                        altname = "none";
                        break;
                    }
                    if(Q_stricmp (altname, "none") != 0)
                        {
                        accuracy = 0;
                        if(stat->weapon_hits[ATTACK_ALTERNATE * level.wpNumWeapons + n])
                        {
                            accuracy = (float)stat->weapon_hits[ATTACK_ALTERNATE * level.wpNumWeapons + n] / (float)stat->weapon_shots[ATTACK_ALTERNATE * level.wpNumWeapons + n] * 100;
                        }
                trap_SendServerCommand( ent-g_entities, va("print \"^3%14s^7%9d^7%9d^7%9d%7s%3.2f\n\"",
                altname,
                stat->weapon_shots[ATTACK_ALTERNATE * level.wpNumWeapons + n],
                stat->weapon_hits[ATTACK_ALTERNATE * level.wpNumWeapons + n],
                stat->weapon_headshots[ATTACK_ALTERNATE * level.wpNumWeapons + n],
                "^7",
                accuracy));
                }
            }
        }
    }

    trap_SendServerCommand( ent-g_entities, va("print \"\nUse ^3[Page Up] ^7and ^3[Page Down] ^7keys to scroll\n\n\""));
}
//==============================================================================================================================
// END OF OLD 1FX. COMMANDS.

/*
==================
Boe_About by boe
3/30/10 - 2:19 PM
Updated 11/20/10 - 11:17 PM
==================
*/

void Boe_About( gentity_t *ent )
{
    // Boe!Man 3/30/10
    trap_SendServerCommand( ent-g_entities, "print \"\n^3Server settings\n\"");
    trap_SendServerCommand( ent-g_entities, "print \"--------------------------------------\n\"");
    trap_SendServerCommand( ent-g_entities, va("print \"%-25s " MODFULL "\n", "[^3Mod used^7]"));

    // Mod channel.
    #if defined _PRE
    trap_SendServerCommand(ent - g_entities, va("print \"%-25s Pre-release\n", "[^3Mod channel^7]"));
    #elif defined _NIGHTLY
    trap_SendServerCommand(ent - g_entities, va("print \"%-25s Nightly (master)\n", "[^3Mod channel^7]"));
    #else
    trap_SendServerCommand(ent - g_entities, va("print \"%-25s Release\n", "[^3Mod channel^7]"));
    #endif

    #ifdef _GOLD
    trap_SendServerCommand( ent-g_entities, va("print \"%-25s SoF2 v1.03 (Gold)\n", "[^3Mod port^7]"));
    #else
    trap_SendServerCommand( ent-g_entities, va("print \"%-25s SoF2 v1.00\n", "[^3Mod port^7]"));
    #endif // _GOLD

    #ifdef WIN32
    trap_SendServerCommand( ent-g_entities, va("print \"%-25s Windows (*.dll)\n", "[^3Host platform^7]"));
    #elif defined(__linux__)
    trap_SendServerCommand( ent-g_entities, va("print \"%-25s Linux (*.so)\n", "[^3Host platform^7]"));
    #elif defined(MACOS_X)
    trap_SendServerCommand( ent-g_entities, va("print \"%-25s Mac OS X (*.bundle)\n", "[^3Host platform^7]"));
    #endif // Host platform.

    trap_SendServerCommand( ent-g_entities, va("print \"%-25s %s\n", "[^3Instagib^7]", (g_instaGib.integer > 0) ? "Yes" : "No"));
    trap_SendServerCommand( ent-g_entities, va("print \"%-25s %s\n", "[^3Competition Mode^7]", (g_compMode.integer > 0 && cm_enabled.integer > 1) ? "Yes" : "No"));
    trap_SendServerCommand( ent-g_entities, va("print \"%-25s %i\n", "[^3Scorelimit^7]", g_scorelimit.integer));
    trap_SendServerCommand( ent-g_entities, va("print \"%-25s %i\n", "[^3Timelimit^7]", g_timelimit.integer));
    if(current_gametype.value == GT_CTF){
        trap_SendServerCommand( ent-g_entities, va("print \"%-25s %i\n", "[^3Respawn interval^7]", g_respawnInterval.integer));
    }

    #ifndef _GOLD
    trap_SendServerCommand( ent-g_entities, va("print \"%-25s Yes\n", "[^3Third person^7]", (g_allowthirdperson.integer > 0) ? "Yes" : "No"));
    #endif // not _GOLD
    trap_SendServerCommand( ent-g_entities, va("print \"%-25s %i\n", "[^3Speed^7]", g_speed.integer));
    trap_SendServerCommand( ent-g_entities, va("print \"%-25s %i\n", "[^3Gravity^7]", g_gravity.integer));
    trap_SendServerCommand( ent-g_entities, va("print \"%-25s %i\n", "[^3Total clients^7]", level.numConnectedClients));

    trap_SendServerCommand( ent-g_entities, "print \"\n^3Owner settings\n\"");
    trap_SendServerCommand( ent-g_entities, "print \"--------------------------------------\n\"");

    trap_SendServerCommand( ent-g_entities, va("print \"%-25s %s\n", "[^3Owner^7]", Owner.string));
    trap_SendServerCommand( ent-g_entities, va("print \"%-25s %s\n", "[^3Active clan^7]", (strstr(Clan.string, "0") || strlen(Clan.string) == 0) ? "No" : "Yes"));
    trap_SendServerCommand( ent-g_entities, va("print \"%-25s %s\n", "[^3Clan URL^7]", (strstr(ClanURL.string, "0") || strlen(ClanURL.string) == 0) ? "None" : ClanURL.string));
    trap_SendServerCommand( ent-g_entities, va("print \"%-25s %s\n", "[^3Hosted by^7]", HostedBy.string));

    trap_SendServerCommand( ent-g_entities, "print \"\nUse ^3[Page Up] ^7and ^3[Page Down] ^7keys to scroll\n\n\"");
}

/*
==================
Boe_BarStat
5/9/10 - 11:08 AM
==================
*/

char *Boe_BarStat(int stat)
{
    char    *bar;

    stat /= 10;

    if(stat >= 10)
    {
        bar = "----------";
        return bar;
    }

    switch(stat)
    {
        case 9:
            bar = "---------^0-";
            break;
        case 8:
            bar = "--------^0--";
            break;
        case 7:
            bar = "-------^0---";
            break;
        case 6:
            bar = "------^0----";
            break;
        case 5:
            bar = "-----^0-----";
            break;
        case 4:
            bar = "----^0------";
            break;
        case 3:
            bar = "---^0-------";
            break;
        case 2:
            bar= "--^0--------";
            break;
        case 1:
            bar = "-^0---------";
            break;
        default:
            bar = "^0----------";
    }
    return bar;
}

/*
================
Boe_serverMsg
6/2/10 - 9:54 PM
================
*/

void Boe_serverMsg (void)
{
    char    *message;

    level.serverMsgCount++;

    switch (level.serverMsgCount){
        case 1:
            message = server_message1.string;
            break;
        case 2:
            message = server_message2.string;
            break;
        case 3:
            message = server_message3.string;
            break;
        case 4:
            message = server_message4.string;
            break;
        case 5:
            message = server_message5.string;
            break;
        default:
            message = "";
            level.serverMsgCount = 0;
            level.serverMsg = level.time + (server_msgInterval.integer * 60000);
            break;
    }
    if ( message[0] == '\0' )
        return;

    level.serverMsg = level.time + (server_msgDelay.integer * 1000);
    trap_SendServerCommand(-1, va("chat -1 \"%s: %s\n\"", G_ColorizeMessage("\\Message"), message));
}

/*
================
Boe_calcMatchScores
11/18/10 - 3:46 PM
================
*/

void Boe_calcMatchScores (void)
{
    if(cm_enabled.integer == 2){ // Boe!Man 3/19/11: Can only be timelimit as the scorelimit won't use this function after one round. Calculate all.
        if (level.teamScores[TEAM_RED] > level.teamScores[TEAM_BLUE]){ // Red team won.
            G_Broadcast(va("%s ^7team wins the match with %i - %i!", server_redteamprefix.string, level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE]), BROADCAST_AWARDS, NULL);
            trap_SendServerCommand(-1, va("print\"^3[Info] ^7Red team wins the match with %i - %i.\n\"", level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE]));
            LogExit("Red team wins the match.");
        }else if(level.teamScores[TEAM_BLUE] > level.teamScores[TEAM_RED]){
            G_Broadcast(va("%s ^7team wins the match with %i - %i!", server_blueteamprefix.string, level.teamScores[TEAM_BLUE], level.teamScores[TEAM_RED]), BROADCAST_AWARDS, NULL);
            trap_SendServerCommand(-1, va("print\"^3[Info] ^7Blue team wins the match with %i - %i.\n\"", level.teamScores[TEAM_BLUE], level.teamScores[TEAM_RED]));
            LogExit("Blue team wins the match.");
        }else if(level.teamScores[TEAM_BLUE] == level.teamScores[TEAM_RED]){
            G_Broadcast(va("Match draw with %i - %i!", level.teamScores[TEAM_BLUE], level.teamScores[TEAM_RED]), BROADCAST_AWARDS, NULL);
            trap_SendServerCommand(-1, va("print\"^3[Info] ^7Match draw with %i - %i.\n\"", level.teamScores[TEAM_BLUE], level.teamScores[TEAM_RED]));
            LogExit("Match draw.");
        }
        return;
    }

    // Boe!Man 3/19/11: Else it's the scorelimit or timelimit that kicks in after two rounds.
    if (cm_sr.integer + level.teamScores[TEAM_RED] > cm_sb.integer + level.teamScores[TEAM_BLUE]){
        G_Broadcast(va("%s ^7team wins the match with %i - %i!", server_redteamprefix.string, level.teamScores[TEAM_RED] + cm_sr.integer, level.teamScores[TEAM_BLUE] + cm_sb.integer), BROADCAST_AWARDS, NULL);
        trap_SendServerCommand(-1, va("print\"^3[Info] ^7Red team wins the match with %i - %i.\n\"", level.teamScores[TEAM_RED] + cm_sr.integer, level.teamScores[TEAM_BLUE] + cm_sb.integer));
        LogExit("Red team wins the match.");
    }else if(cm_sb.integer + level.teamScores[TEAM_BLUE] > cm_sr.integer + level.teamScores[TEAM_RED]){
        G_Broadcast(va("%s ^7team wins the match with %i - %i!", server_blueteamprefix.string, level.teamScores[TEAM_BLUE] + cm_sb.integer, level.teamScores[TEAM_RED] + cm_sr.integer), BROADCAST_AWARDS, NULL);
        trap_SendServerCommand(-1, va("print\"^3[Info] ^7Blue team wins the match with %i - %i.\n\"", level.teamScores[TEAM_BLUE] + cm_sb.integer, level.teamScores[TEAM_RED] + cm_sr.integer));
        LogExit("Blue team wins the match.");
    }else if (cm_sb.integer + level.teamScores[TEAM_BLUE] == cm_sr.integer + level.teamScores[TEAM_RED]){
        G_Broadcast(va("Match draw with %i - %i!", level.teamScores[TEAM_BLUE] + cm_sb.integer, level.teamScores[TEAM_RED] + cm_sr.integer), BROADCAST_AWARDS, NULL);
        trap_SendServerCommand(-1, va("print\"^3[Info] ^7Match draw with %i - %i.\n\"", level.teamScores[TEAM_BLUE] + cm_sb.integer, level.teamScores[TEAM_RED] + cm_sr.integer));
        LogExit("Match draw.");
    }
}

/*
================
Boe_compTimelimitCheck
3/19/11 - 2:16 PM
================
*/

void Boe_compTimeLimitCheck (void)
{
    if (cm_enabled.integer == 2){
        if(cm_dr.integer == 1){ // Boe!Man 3/18/11: If dual rounds are enabled, make use of them and display the temporary stuff.
            if ( level.teamScores[TEAM_RED] > level.teamScores[TEAM_BLUE] ){
                G_Broadcast(va("%s ^7team wins the 1st round with %i - %i!", server_redteamprefix.string, level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE]), BROADCAST_AWARDS, NULL);
                trap_SendServerCommand(-1, va("print\"^3[Info] ^7Red team wins the 1st round with %i - %i.\n\"", level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE] ));
                // Boe!Man 11/18/10: Set the scores right (for logging purposes).
                if (cm_aswap.integer == 0){
                    trap_Cvar_Set("cm_sr", va("%i", level.teamScores[TEAM_RED]));
                    trap_Cvar_Set("cm_sb", va("%i", level.teamScores[TEAM_BLUE]));
                }else{
                    // Boe!Man 11/19/10: Log the scores the other way around as the teams will get swapped the next round.
                    trap_Cvar_Set("cm_sr", va("%i", level.teamScores[TEAM_BLUE]));
                    trap_Cvar_Set("cm_sb", va("%i", level.teamScores[TEAM_RED]));
                }
                LogExit("Red team wins the 1st round.");
            }else if ( level.teamScores[TEAM_BLUE] > level.teamScores[TEAM_RED] ){
                G_Broadcast(va("%s ^7team wins the 1st round with %i - %i!", server_blueteamprefix.string, level.teamScores[TEAM_BLUE], level.teamScores[TEAM_RED]), BROADCAST_AWARDS, NULL);
                trap_SendServerCommand(-1, va("print\"^3[Info] ^7Blue team wins the 1st round with %i - %i.\n\"", level.teamScores[TEAM_BLUE], level.teamScores[TEAM_RED] ));
                // Boe!Man 11/18/10: Set the scores right (for logging purposes).
                if (cm_aswap.integer == 0){
                    trap_Cvar_Set("cm_sr", va("%i", level.teamScores[TEAM_RED]));
                    trap_Cvar_Set("cm_sb", va("%i", level.teamScores[TEAM_BLUE]));
                }else{
                    // Boe!Man 11/19/10: Log the scores the other way around as the teams will get swapped the next round.
                    trap_Cvar_Set("cm_sr", va("%i", level.teamScores[TEAM_BLUE]));
                    trap_Cvar_Set("cm_sb", va("%i", level.teamScores[TEAM_RED]));
                }
                LogExit("Blue team wins the 1st round.");
            }else{ // Boe!Man 3/19/11: Tie is perfectly capable when a timelimit is set.
                G_Broadcast(va("\\Round draw with %i - %i!", level.teamScores[TEAM_BLUE], level.teamScores[TEAM_RED]), BROADCAST_AWARDS, NULL);
                trap_SendServerCommand(-1, va("print\"^3[Info] ^7Round draw with %i - %i.\n\"", level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE] ));
                // Boe!Man 11/18/10: Set the scores right (for logging purposes).
                trap_Cvar_Set("cm_sr", va("%i", level.teamScores[TEAM_RED]));
                trap_Cvar_Set("cm_sb", va("%i", level.teamScores[TEAM_BLUE]));
                LogExit("1st Round Draw.");
            }
            trap_Cvar_Set("cm_enabled", "21"); // Boe!Man 3/19/11: Display the screen again.
        }else{ // Boe!Man 3/19/11: They wanted to play a single round.
            Boe_calcMatchScores(); // Boe!Man 3/19/11: Calculate the match scores, could be a match draw or anything really.
            trap_Cvar_Set("cm_enabled", "5"); // Boe!Man 11/18/10: 5 - Scrim Ended.
        }
    }else if(g_compMode.integer > 0 && cm_enabled.integer == 4){ // Scrim ended, can ONLY BE DUAL ROUNDS.
        if (cm_sr.integer > cm_sb.integer){ // Round 1 scores.
            trap_SendServerCommand(-1, va("print\"^3[Info] ^7Red team won the 1st round with %i - %i.\n\"", cm_sr.integer, cm_sb.integer));
        }else if(cm_sb.integer > cm_sr.integer){
            trap_SendServerCommand(-1, va("print\"^3[Info] ^7Blue team won the 1st round with %i - %i.\n\"", cm_sb.integer, cm_sr.integer));
        }else{
            trap_SendServerCommand(-1, va("print\"^3[Info] ^7Round draw 1st round with %i - %i.\n\"", cm_sb.integer, cm_sr.integer));
        }

        if(level.teamScores[TEAM_RED] > level.teamScores[TEAM_BLUE]){ // Round 2 scores.
            trap_SendServerCommand(-1, va("print\"^3[Info] ^7Red team won the 2nd round with %i - %i.\n\"", level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE]));
        }else if(level.teamScores[TEAM_BLUE] > level.teamScores[TEAM_RED]){
            trap_SendServerCommand(-1, va("print\"^3[Info] ^7Blue team won the 2nd round with %i - %i.\n\"", level.teamScores[TEAM_BLUE], level.teamScores[TEAM_RED]));
        }else{
            trap_SendServerCommand(-1, va("print\"^3[Info] ^7Round draw 2nd round with %i - %i.\n\"", level.teamScores[TEAM_BLUE], level.teamScores[TEAM_RED]));
        }
        Boe_calcMatchScores(); // Boe!Man 3/19/11: Calculate the match scores, could be a match draw or anything really.
        trap_Cvar_Set("cm_enabled", "5"); // Boe!Man 11/18/10: 5 - Scrim Ended.
    }
    return;
}

/*
================
Boe_mapEvents
6/10/11 - 12:35 PM
================
*/

void Boe_mapEvents (void){
    if(level.mapAction == 1){
        if (g_compMode.integer > 0 && cm_enabled.integer == 1){
            if(level.time >= level.mapSwitchCount + 2000){
                trap_SendConsoleCommand( EXEC_APPEND, va("map_restart 0\n"));
            }
        }
        else if (g_compMode.integer > 0 && cm_enabled.integer == 3){
            if(level.time >= level.mapSwitchCount + 2000){
                /*
                // Boe!Man 11/17/10: Is auto swap enabled? -- Update 1/24/11: Swap the teams when the round has just ended. -- Update 3/19/11: Swap the teams when the round has just started.. The temporary round.
                if (cm_aswap.integer > 0){
                    Boe_SwapTeams(NULL);
                }*/
                trap_SendConsoleCommand( EXEC_APPEND, va("map_restart 0\n"));
            }
        }
        else{
            if(level.time >= level.mapSwitchCount){
                if(level.mapSwitchCount2){
                    G_Broadcast(va("\\Map restart in %i!", level.mapSwitchCount2), BROADCAST_CMD, NULL);
                    level.mapSwitchCount2--;
                    level.mapSwitchCount = level.time + 1000;
                }else{
                    /*
                    if(current_gametype.value == GT_HS){
                        trap_SendConsoleCommand( EXEC_APPEND, va("g_gametype h&s\n"));
                    }else if(current_gametype.value == GT_HZ){
                        trap_SendConsoleCommand( EXEC_APPEND, va("g_gametype h&z\n"));
                    }*/
                trap_SendConsoleCommand( EXEC_APPEND, va("map_restart 0\n"));
                }
            }
        }
    }else if(level.mapAction == 2){
        if(level.time >= level.mapSwitchCount){
            if(level.mapSwitchCount2){
                G_Broadcast(va("%s ^7%s ^7in %i!", level.mapPrefix, level.mapSwitchName, level.mapSwitchCount2), BROADCAST_CMD, NULL);
                level.mapSwitchCount2--;
                level.mapSwitchCount = level.time + 1000;
            }else{
                if(strstr(level.mapPrefix, "D^")){
                    trap_SendConsoleCommand( EXEC_APPEND, va("devmap %s\n", level.mapSwitchName));
                }else{
                    trap_SendConsoleCommand( EXEC_APPEND, va("map %s\n", level.mapSwitchName));
                }

                // Boe!Man 7/15/13: Also re-set these values in case the map switch goes wrong.
                level.mapSwitch = qfalse;
                level.mapAction = 0;
            }
        }
    }else if(level.mapAction == 3){
        if(level.time >= level.mapSwitchCount + 3000){
            if(!RMG.integer){
                trap_Cvar_VariableStringBuffer ( "mapname", level.mapname, MAX_QPATH );
                trap_SendConsoleCommand( EXEC_APPEND, va("map %s\n", level.mapname));
            }else{
                // Boe!Man 11/5/12: It's a RMG map, make sure to call the rmgmap 0 command.
                trap_SendConsoleCommand( EXEC_APPEND, va("rmgmap 0\n"));
            }

            // Boe!Man 7/15/13: Also re-set these values in case the map switch goes wrong.
            level.mapSwitch = qfalse;
            level.mapAction = 0;
        }
    }
    else if(level.mapAction == 4){
        if(level.time >= level.mapSwitchCount){
            if(level.mapSwitchCount2){
                G_Broadcast(va("\\Mapcycle in %i!", level.mapSwitchCount2), BROADCAST_CMD, NULL);
                level.mapSwitchCount2--;
                level.mapSwitchCount = level.time + 1000;
            }else{
                trap_SendConsoleCommand( EXEC_APPEND, va("mapcycle\n"));

                // Boe!Man 7/15/13: Also re-set these values in case the map switch goes wrong.
                level.mapSwitch = qfalse;
                level.mapAction = 0;
            }
        }
    }
    return;
}

/*
================
Boe_displayTokens
8/25/11 - 2:17 PM
================
*/

void Boe_displayTokens ( gentity_t *ent )
{
    // Header.
    trap_SendServerCommand( ent-g_entities, "print \"\n^3Key      Explanation\n\"");
    trap_SendServerCommand( ent-g_entities, "print \"--------------------------------------\n\"");
    // Tokens body.
    trap_SendServerCommand( ent-g_entities, va("print \"^7%-8s [^3Health in bar format^7]\n", "#b"));
    trap_SendServerCommand( ent-g_entities, va("print \"^7%-8s [^3Armor in bar format^7]\n", "#B"));
    trap_SendServerCommand( ent-g_entities, va("print \"^7%-8s [^3Health colored^7]\n", "#h"));
    trap_SendServerCommand( ent-g_entities, va("print \"^7%-8s [^3Health non-colored^7]\n", "#H"));
    trap_SendServerCommand( ent-g_entities, va("print \"^7%-8s [^3Armor colored^7]\n", "#a"));
    trap_SendServerCommand( ent-g_entities, va("print \"^7%-8s [^3Armor non-colored^7]\n", "#A"));
    trap_SendServerCommand( ent-g_entities, va("print \"^7%-12s [^3Last player that hurt you^7]\n", "#d^1/^7#D"));
    trap_SendServerCommand( ent-g_entities, va("print \"^7%-12s [^3Last player that you hurt^7]\n", "#t^1/^7#T"));
    trap_SendServerCommand( ent-g_entities, va("print \"^7%-12s [^3Shows the server g_motd (usually next map)^7]\n", "#n^1/^7#N"));
    if(current_gametype.value != GT_DM){ // Don't show closest team-mate in DM (there is none).
        trap_SendServerCommand( ent-g_entities, va("print \"^7%-12s [^3Shows your closest team-mate name^7]\n", "#f^1/^7#F"));
    }
    trap_SendServerCommand( ent-g_entities, va("print \"^7%-12s [^3Shows your closest enemy name^7]\n", "#e^1/^7#E"));
    trap_SendServerCommand( ent-g_entities, va("print \"^7%-12s [^3Shows your current location^7]\n", "#l^1/^7#L"));
    trap_SendServerCommand( ent-g_entities, va("print \"^7%-12s [^3Shows the last connected player name^7]\n", "#z^1/^7#Z"));

    // H&S tokens (only show them when it's actually H&S).
    if(current_gametype.value == GT_HS){
        trap_SendServerCommand( ent-g_entities, va("print \"\n^7%-12s [^3Shows players' name that holds RPG or its location^7]\n", "#r^1/^7#R"));
        trap_SendServerCommand( ent-g_entities, va("print \"^7%-12s [^3Shows players' name that holds M4 or its location^7]\n", "#c^1/^7#C"));
        trap_SendServerCommand( ent-g_entities, va("print \"^7%-12s [^3Shows players' name that holds MM1 or its location^7]\n", "#m^1/^7#M"));
        trap_SendServerCommand( ent-g_entities, va("print \"^7%-8s [^3Shows players' name that holds '? grenade' or its location^7]\n", "#?"));
    }

    trap_SendServerCommand( ent-g_entities, "print \"\nUse ^3[Page Up] ^7and ^3[Page Down] ^7keys to scroll\n\n\"");

    return;
}

/*
================
Boe_checkRoof
6/3/12 - 3:53 PM
================
*/

void Boe_checkRoof ( gentity_t *ent )
{
    // Boe!Man 11/22/13: Only proceed if this is true.
    if(!level.noLRActive[1] || !level.noLRMWEntFound[1]){
        return;
    }

    // Boe!Man 6/3/12: Do this with an interval. It's a shame to be cocky about something this small, so save resources.
    if(ent->client->sess.noroofCheckTime > level.time){
        return;
    }

    // Boe!Man 6/3/12: He must be alive.
    if(G_IsClientDead (ent->client)){
        if(ent->client->sess.isOnRoof){ // Well, since he's dead now, reset this..
            ent->client->sess.isOnRoof = qfalse;
            ent->client->sess.isOnRoofTime = 0;
        }
        ent->client->sess.noroofCheckTime = level.time + 1000;
        return;
    }

    // Boe!Man 6/3/12: Check for the player.
    if(!level.noLROpened[1]){ // Roof must be opened, else don't check this.
        if(!ent->client->sess.isOnRoof){ // Player ISN'T on roof, last time we checked.
            if(ent->r.currentOrigin[2] >= level.noLR[1][2]){ // Well he is now. Check for the timeout.
                if(!level.noLR[1][1]){ // 0 or less.. Meaning, instant pop. No need for further checks.
                    G_Damage(ent, NULL, NULL, NULL, NULL, 10000, 0, MOD_TRIGGER_HURT, 0);
                    trap_SendServerCommand(-1, va("print\"^3[Info] ^7%s ^7was killed for not leaving the roof.\n\"", ent->client->pers.cleanName));
                }else{
                    ent->client->sess.isOnRoof = qtrue; // The server owner specified a timer. So, first, the player initialised this process by being on roof.
                    G_Broadcast(va("\\Leave the roof within ^1%.0f ^7seconds!", level.noLR[1][1]), BROADCAST_GAME, ent);
                    ent->client->sess.isOnRoofTime = 1;
                }
            }
        }else{ // Player IS on roof.
            if(ent->r.currentOrigin[2] < level.noLR[1][2]){ // He left the roof.
                ent->client->sess.isOnRoof = qfalse;
                ent->client->sess.isOnRoofTime = 0;
                G_Broadcast("\\You're no longer on the roof!", BROADCAST_GAME, ent);
            }else{ // He's still on the roof.
                if(level.noLR[1][1] == ent->client->sess.isOnRoofTime){ // Well, he waited it out. Pop him.
                    G_Damage(ent, NULL, NULL, NULL, NULL, 10000, 0, MOD_TRIGGER_HURT, 0);
                    trap_SendServerCommand(-1, va("print\"^3[Info] ^7%s ^7was killed for not leaving the roof.\n\"", ent->client->pers.netname));
                    ent->client->sess.isOnRoof = qfalse;
                    ent->client->sess.isOnRoofTime = 0;
                }else{ // Give him another warning.
                    G_Broadcast(va("\\Leave the roof within ^1%.0f ^7seconds!", level.noLR[1][1] - ent->client->sess.isOnRoofTime), BROADCAST_GAME, ent);
                    ent->client->sess.isOnRoofTime += 1;
                }
            }
        }
    }

    ent->client->sess.noroofCheckTime = level.time + 1000;

    return;
}

/*
================
Boe_firstDigitOfInt
7/4/12 - 6:49 PM
Returns the first digit of the given integer.
================
*/

int Boe_firstDigitOfInt(int i)
{
    if(!i){
        return -1;
    }

    while(i >= 10){
        i /= 10;
    }

    return i;
}

/*
================
Boe_parseCustomCommandArgs
7/27/12 - 2:40 PM
Function that replaces arguments in the actual CustomCommand action.
================
*/

char *Boe_parseCustomCommandArgs(gentity_t *ent, char *in, qboolean shortCmd)
{
    static char *buf;
    static char out[512] = "\0";
    char        buf2[5] = "\0";
    char        arg2[1];
    char        arg[MAX_STRING_TOKENS];
    int         pos = 0; // Position of out.
    int         argc;

    #ifdef _DEBUG
    Com_Printf("Boe_parseCustomCommandArgs: in: %s\n", in); // Debug.
    #endif

    if (shortCmd) {
        argc = G_GetChatArgumentCount();
    }

    buf = in;
    while ( *buf ){
        if(*buf == '%'){ // Check for the identifier of the arg.
            if(strlen(buf) >= 5){ // Can't go out of bounds here, so check if there are at least 5 characters to process.
                strncpy(buf2, buf, 5);
                if(strstr(buf2, "%arg") && henk_isdigit(buf2[4])){ // Can't really go wrong here..
                    memset(arg, 0, sizeof(arg)); // Reset memory here.
                    arg2[0] = buf2[4]; // Copy the argument string to the new buffer, including a terminator.
                    arg2[1] = '\0';
                    if (!shortCmd || shortCmd && !argc){
                        trap_Argv(henk_atoi(arg2) + 1, arg, sizeof(arg)); // Fetch arg.
                    }else{
                        if (argc && argc >= henk_atoi(arg2)) {
                            Q_strncpyz(arg, G_GetChatArgument(henk_atoi(arg2)), sizeof(arg));
                        }else{
                            arg[0] = '\0';
                        }
                    }

                    #ifdef _DEBUG
                    Com_Printf("Argument %i: %s\n", henk_atoi(arg2), arg);
                    #endif
                    if (strlen(arg) == 0) {
                        trap_SendServerCommand(ent - g_entities, "print \"^3[Info] ^7Error: You need to append additional arguments to this command!\n\"");
                        return NULL;
                    }
                    Q_strcat(out, sizeof(out), arg); // Append the argument.

                    // Set the new positions correctly (so we won't get any left overs or overwriting).
                    pos += (strlen(arg)-1);
                    buf += 4;
                }else{ // If any of this fails, just grab the char and move on.
                    out[pos] = *buf;
                }
            }else{ // Same here..
                out[pos] = *buf;
            }
        }else{ // Same..
            out[pos] = *buf;
        }

        // (Re-)Set pointers.
        pos += 1;
        buf++;
    }

    buf = out;

    #ifdef _DEBUG
    Com_Printf("Boe_parseCustomCommandArgs: out: %s\n", buf); // Debug.
    #endif

    return buf;
}

/*
============
G_GetChatArgumentCount

Get argument count
from chat buffer.
============
*/

int G_GetChatArgumentCount()
{
    char text[MAX_SAY_TEXT];
    char *text2;
    int argc = 0;

    // Fetch the argument containing the full buffer.
    trap_Argv(1, text, sizeof(text));
    text2 = text;

    // Loop through text, find first character.
    while (text2 != NULL && *text2 == ' ') {
        *text2++;
    }

    if (!text2 || strlen(text2) == 0) {
        // No real argument present.
        return 0;
    }

    while (text2 != NULL && strlen(text2) != 0) {
        text2 = strstr(text2, " ");

        // No more arguments found, return.
        if (text2 == NULL)
            break;

        // Get rid of extra spaces.
        while (text2 != NULL && *text2 == ' ') {
            *text2++;
        }

        if (text2 == NULL || strlen(text2) == 0) {
            // No real argument present.
            break;
        }

        argc++;
    }

    return argc;
}

/*
============
G_GetChatArgument

Get argument from chat
buffer.
============
*/

char *G_GetChatArgument(int argNum)
{
    static char newArg[MAX_SAY_TEXT];
    char text[MAX_SAY_TEXT];
    char *text2, *end;
    int argc = 0;

    // Reset buffer.
    memset(newArg, 0, sizeof(newArg));

    // Fetch the argument containing the full buffer.
    trap_Argv(1, text, sizeof(text));
    text2 = text;

    // Argument must be present.
    if (G_GetChatArgumentCount() < argNum) {
        return "";
    }

    // Loop through text, find first character.
    while (text2 != NULL && *text2 == ' '){
        *text2++;
    }

    while (argc < argNum) {
        text2 = strstr(text2, " ");

        // Get rid of extra spaces.
        while (text2 && *text2 == ' ') {
            *text2++;
        }

        argc++;
    }

    // Check if there are more arguments after this one, or if it's the last one.
    end = strstr(text2, " ");
    if (end == NULL) {
        Q_strncpyz(newArg, text2, sizeof(newArg));
    }else{
        text2[end - text2] = '\0';
        Q_strncpyz(newArg, text2, sizeof(newArg));
    }

    // Remove colors from arg.
    RemoveColorEscapeSequences(newArg);

    return newArg;
}

/*
============
Boe_Howto
By boe, 10/5/10 7:27 PM
============
*/

void Boe_Howto ( gentity_t *ent )
{
    if (level.crossTheBridge){
        trap_SendServerCommand( ent-g_entities, "print \"\n^3You're playing: Cross The Bridge\n\"");
        trap_SendServerCommand( ent-g_entities, "print \"-------------------------------------------------------\n\n");
        trap_SendServerCommand( ent-g_entities, "print \"[^3General^7]\n\"");
        trap_SendServerCommand( ent-g_entities, "print \"This Mini-Game's purpose is for the hiders to cross the\n\"");
        trap_SendServerCommand( ent-g_entities, "print \"bridge. The seekers will be spawned strategically around\n\"");
        trap_SendServerCommand( ent-g_entities, "print \"the bridge. Seekers will get spawned in either a chopper\n\"");
        trap_SendServerCommand( ent-g_entities, "print \"(with AK74) or a watchtower (with sniper). Hiders will\n\"");
        trap_SendServerCommand( ent-g_entities, "print \"get 4 smokes: they must prevent the seekers killing them,\n\"");
        trap_SendServerCommand( ent-g_entities, "print \"these smokes will help them block their vision and crossing\n\"");
        trap_SendServerCommand( ent-g_entities, "print \"safely.\n\n\"");
        trap_SendServerCommand( ent-g_entities, "print \"\n[^3Rewards^7]\n\"");
        trap_SendServerCommand( ent-g_entities, "print \"Seekers will be awarded one point for each hider they kill.\n\"");
        trap_SendServerCommand( ent-g_entities, "print \"Hiders will get one point for crossing safely and a bonus\n\"");
        trap_SendServerCommand( ent-g_entities, "print \"point if they cross within 20 seconds.\n\n\"");
        trap_SendServerCommand( ent-g_entities, "print \"Have fun playing!\n");

        trap_SendServerCommand( ent-g_entities, "print \"\nUse ^3[Page Up] ^7and ^3[Page Down] ^7keys to scroll\n\n\"");
    }
}

// Boe!Man 5/27/13: Misc. SQLite functions.
int process_ddl_row2(void * pData, int nColumns,
        char **values, char **columns)
{
        sqlite3     *db;

        if (nColumns != 1)
                return 1; // Error

        db = (sqlite3*)pData;
        sqlite3_exec(db, values[0], NULL, NULL, NULL);

        return 0;
}

int process_dml_row2(void *pData, int nColumns,
        char **values, char **columns)
{
        sqlite3     *db;
        char *stmt;

        if (nColumns != 1)
                return 1; // Error

        db = (sqlite3*)pData;

        stmt = sqlite3_mprintf("insert into main.%q "
                "select * from %s.%q", values[0], tempName, values[0]);
        sqlite3_exec(db, stmt, NULL, NULL, NULL);
        sqlite3_free(stmt);

        return 0;
}
// End Boe!Man 5/27/13

/*
================
Boe_userdataIntegrity
12/8/12 - 9:49 AM
Function that checks userdata prior to having a finished initgame.
As of 5/27/13, this function also loads the database data into memory.
================
*/

void Boe_userdataIntegrity(void)
{
    sqlite3         *db;
    sqlite3_stmt    *stmt;
    int              rc;
    qboolean         dbOkay;
    struct stat st = {0};

    Com_Printf("Checking userdata integrity...\n");

    // Check bans.db first. Prior to doing that, check if the folder exists. If not, create it.
    // After that, the first thing we check is if the database exists on one of the two locations.
    if(!level.altPath){
        if(stat("./users", &st) == -1){
            #ifdef _WIN32
                #ifdef __GNUC__
                mkdir("./users");
                #elif _MSC_VER
                _mkdir(".\\users");
                #endif
            #elif __linux__
            mkdir("./users", 0755);
            #endif
        }

        rc = sqlite3_open_v2("./users/bans.db", &db, SQLITE_OPEN_READWRITE, NULL);
    }else{
        if(stat(va("%s/users/", level.altString), &st) == -1){
            #ifdef _WIN32
            #ifdef __GNUC__
            mkdir(va("%s/users", level.altString));
            #elif _MSC_VER
            _mkdir(va("%s\\users", level.altString));
            #endif
            #elif __linux__
            mkdir(va("%s/users", level.altString), 0755);
            #endif
        }

        rc = sqlite3_open_v2(va("%s/users/bans.db", level.altString), &db, SQLITE_OPEN_READWRITE, NULL);
    }

    dbOkay = qfalse;

    if(rc){
        // The database cannot be found. We try to create it.
        if(!level.altPath){
            rc = sqlite3_open_v2("./users/bans.db", &db, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL);
        }else{
            rc = sqlite3_open_v2(va("%s/users/bans.db", level.altString), &db, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL);
        }

        if(rc){
            G_LogPrintf("^1Fatal Error: ^7bans database: %s\n", sqlite3_errmsg(db));
            // Boe!Man 5/27/13: This is bad, drop with a fatal error.
            Com_Error(ERR_FATAL, "Bans database: %s", sqlite3_errmsg(db));
        }else{
            dbOkay = qtrue;
        }
    }else{
        dbOkay = qtrue;
    }

    if(dbOkay){
        // The database should be opened by now, see if it needs maintenance.
        if(sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS bans(IP VARCHAR(24), name VARCHAR(36), by VARCHAR(36), reason VARCHAR(128))", 0, 0, 0) != SQLITE_OK){
            G_LogPrintf("^1Error: ^7bans database: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            // Boe!Man 5/27/13: This is bad, drop with a fatal error.
            Com_Error(ERR_FATAL, "Bans database: %s", sqlite3_errmsg(db));
            return;
        }

        if(sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS subnetbans(IP VARCHAR(8), name VARCHAR(36), by VARCHAR(36), reason VARCHAR(128))", 0, 0, 0) != SQLITE_OK){
            G_LogPrintf("^1Error: ^7bans database: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            // Boe!Man 5/27/13: This is bad, drop with a fatal error.
            Com_Error(ERR_FATAL, "Bans database: %s", sqlite3_errmsg(db));
            return;
        }

        // Boe!Man 5/27/13: No errors, load the database into memory.
        sqlite3_open(":memory:", &bansDb);

        sqlite3_exec(db, "BEGIN", NULL, NULL, NULL);
        sqlite3_exec(db, "SELECT sql FROM sqlite_master WHERE sql NOT NULL", &process_ddl_row2, bansDb, NULL);
        sqlite3_exec(db, "COMMIT", NULL, NULL, NULL);

        // Boe!Man 12/12/12: Close the bans database.
        sqlite3_close(db);

        // Boe!Man 5/27/13: Attach the database.
        if(!level.altPath){
            sqlite3_exec(bansDb, "ATTACH DATABASE './users/bans.db' as bans", NULL, NULL, NULL);
        }else{
            sqlite3_exec(bansDb, va("ATTACH DATABASE './%s/users/bans.db' as bans", level.altString), NULL, NULL, NULL);
        }
        Q_strncpyz(tempName, "bans", sizeof(tempName));

        // Boe!Man 5/17/13: Copy the data from the backup to the in-memory database.
        sqlite3_exec(bansDb, "BEGIN", NULL, NULL, NULL);
        sqlite3_exec(bansDb, "SELECT name FROM bans.sqlite_master WHERE type='table'", &process_dml_row2, bansDb, NULL);
        sqlite3_exec(bansDb, "COMMIT", NULL, NULL, NULL);
    }

    // Now we check the alias database.
    // The first thing we check is if the database exists on one of the two locations.
    if(!level.altPath){
        rc = sqlite3_open_v2("./users/aliases.db", &db, SQLITE_OPEN_READWRITE, NULL);
    }else{
        rc = sqlite3_open_v2(va("%s/users/aliases.db", level.altString), &db, SQLITE_OPEN_READWRITE, NULL);
    }

    dbOkay = qfalse;

    if(rc){
        // The database cannot be found. We try to create it.
        if(!level.altPath){
            rc = sqlite3_open_v2("./users/aliases.db", &db, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL);
        }else{
            rc = sqlite3_open_v2(va("%s/users/aliases.db", level.altString), &db, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL);
        }

        if(rc){
            G_LogPrintf("^1Error: ^7aliases database: %s\n", sqlite3_errmsg(db));
            // Boe!Man 5/27/13: This is bad, drop with a fatal error.
            Com_Error(ERR_FATAL, "Aliases database: %s", sqlite3_errmsg(db));
        }else{
            dbOkay = qtrue;
        }
    }else{
        dbOkay = qtrue;
    }

    if(dbOkay){
        sqlite3_exec(db, "PRAGMA synchronous = OFF", NULL, NULL, NULL);
        sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL);

        // The database should be opened by now, see if it needs maintenance.
        if(sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS aliases_index('ID' INTEGER PRIMARY KEY NOT NULL, 'IP' varchar(24) NOT NULL)", 0, 0, 0) != SQLITE_OK){
            G_LogPrintf("^1Error: ^7aliases database: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            // Boe!Man 5/27/13: This is bad, drop with a fatal error.
            Com_Error(ERR_FATAL, "Aliases database: %s", sqlite3_errmsg(db));
            return;
        }

        if(sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS aliases_names('ID' INTEGER, 'name' varchar(36) collate nocase)", 0, 0, 0) != SQLITE_OK){
            G_LogPrintf("^1Error: ^7aliases database: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            // Boe!Man 5/27/13: This is bad, drop with a fatal error.
            Com_Error(ERR_FATAL, "Aliases database: %s", sqlite3_errmsg(db));
            return;
        }

        // Boe!Man 1/2/13: Check if we can delete all data from the tables if the index exceeds the (user) defined limit.
        sqlite3_prepare(db, "SELECT count(ID) from aliases_index", -1, &stmt, 0);
        if(sqlite3_step(stmt) != SQLITE_DONE){
            // Boe!Man 1/2/13: Delete all data from the alias database if we hit the limit.
            if(sqlite3_column_int(stmt, 0) > sql_aliasFlushCount.integer){
                if(sqlite3_exec(db, "DELETE FROM aliases_index", 0, 0, 0) != SQLITE_OK){
                    G_LogPrintf("^1Error: ^7aliases database: %s\n", sqlite3_errmsg(db));
                    sqlite3_finalize(stmt);
                    sqlite3_close(db);
                    // Boe!Man 5/27/13: This is bad, drop with a fatal error.
                    Com_Error(ERR_FATAL, "Aliases database: %s", sqlite3_errmsg(db));
                    return;
                }
                if(sqlite3_exec(db, "DELETE FROM aliases_names", 0, 0, 0) != SQLITE_OK){
                    G_LogPrintf("^1Error: ^7aliases database: %s\n", sqlite3_errmsg(db));
                    sqlite3_finalize(stmt);
                    sqlite3_close(db);
                    // Boe!Man 5/27/13: This is bad, drop with a fatal error.
                    Com_Error(ERR_FATAL, "Aliases database: %s", sqlite3_errmsg(db));
                    return;
                }
                Com_Printf("Emptied aliases database due to the data exceeding the limit..\n");
            }
        }
        sqlite3_finalize(stmt);
        sqlite3_exec(db, "COMMIT", NULL, NULL, NULL);

        // Boe!Man 5/27/13: No errors, load the database into memory.
        sqlite3_open(":memory:", &aliasesDb);

        sqlite3_exec(db, "BEGIN", NULL, NULL, NULL);
        sqlite3_exec(db, "SELECT sql FROM sqlite_master WHERE sql NOT NULL", &process_ddl_row2, aliasesDb, NULL);
        sqlite3_exec(db, "COMMIT", NULL, NULL, NULL);

        // Boe!Man 12/30/12: Close the aliases database.
        sqlite3_close(db);

        // Boe!Man 5/27/13: Attach the database.
        if(!level.altPath){
            sqlite3_exec(aliasesDb, "ATTACH DATABASE './users/aliases.db' as aliases", NULL, NULL, NULL);
        }else{
            sqlite3_exec(aliasesDb, va("ATTACH DATABASE './%s/users/aliases.db' as aliases", level.altString), NULL, NULL, NULL);
        }
        Q_strncpyz(tempName, "aliases", sizeof(tempName));

        // Boe!Man 5/17/13: Copy the data from the backup to the in-memory database.
        sqlite3_exec(aliasesDb, "BEGIN", NULL, NULL, NULL);
        sqlite3_exec(aliasesDb, "SELECT name FROM aliases.sqlite_master WHERE type='table'", &process_dml_row2, aliasesDb, NULL);
        sqlite3_exec(aliasesDb, "COMMIT", NULL, NULL, NULL);
    }

    // Boe!Man 2/4/13: Also check the users.db, which handles Admins, Pass Admins and Clan Members.
    // The first thing we check is if the database exists on one of the two locations.
    if(!level.altPath){
        rc = sqlite3_open_v2("./users/users.db", &db, SQLITE_OPEN_READWRITE, NULL);
    }else{
        rc = sqlite3_open_v2(va("%s/users/users.db", level.altString), &db, SQLITE_OPEN_READWRITE, NULL);
    }

    dbOkay = qfalse;

    if(rc){
        // The database cannot be found. We try to create it.
        if(!level.altPath){
            rc = sqlite3_open_v2("./users/users.db", &db, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL);
        }else{
            rc = sqlite3_open_v2(va("%s/users/users.db", level.altString), &db, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL);
        }

        if(rc){
            G_LogPrintf("^1Error: ^7users database: %s\n", sqlite3_errmsg(db));
            // Boe!Man 5/27/13: This is bad, drop with a fatal error.
            Com_Error(ERR_FATAL, "Users database: %s", sqlite3_errmsg(db));
        }else{
            dbOkay = qtrue;
        }
    }else{
        dbOkay = qtrue;
    }

    if(dbOkay){
        // The database should be opened by now, see if it needs maintenance.
        if(sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS admins('IP' VARCHAR(24), 'name' VARCHAR(36) collate nocase, 'by' VARCHAR(36), 'level' INTEGER NOT NULL)", 0, 0, 0) != SQLITE_OK){
            G_LogPrintf("^1Error: ^7users database: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            // Boe!Man 5/27/13: This is bad, drop with a fatal error.
            Com_Error(ERR_FATAL, "Users database: %s", sqlite3_errmsg(db));
            return;
        }

        if (sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS passadmins('name' varchar(36) collate nocase, 'by' VARCHAR(36), 'level' INTEGER NOT NULL, 'pass' VARCHAR(36))", 0, 0, 0) != SQLITE_OK){
            G_LogPrintf("^1Error: ^7users database: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            Com_Error(ERR_FATAL, "Users database: %s", sqlite3_errmsg(db));
            return;
        }

        if(sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS clanmembers('IP' VARCHAR(24), 'name' VARCHAR(36) collate nocase, 'by' VARCHAR(36))", 0, 0, 0) != SQLITE_OK){
            G_LogPrintf("^1Error: ^7users database: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            // Boe!Man 5/27/13: This is bad, drop with a fatal error.
            Com_Error(ERR_FATAL, "Users database: %s", sqlite3_errmsg(db));
            return;
        }

        // Boe!Man 5/27/13: No errors, load the database into memory.
        sqlite3_open(":memory:", &usersDb);

        sqlite3_exec(db, "BEGIN", NULL, NULL, NULL);
        sqlite3_exec(db, "SELECT sql FROM sqlite_master WHERE sql NOT NULL", &process_ddl_row2, usersDb, NULL);
        sqlite3_exec(db, "COMMIT", NULL, NULL, NULL);

        // Boe!Man 12/12/12: Close the users database.
        sqlite3_close(db);

        // Boe!Man 5/27/13: Attach the database.
        if(!level.altPath){
            sqlite3_exec(usersDb, "ATTACH DATABASE './users/users.db' as users", NULL, NULL, NULL);
        }else{
            sqlite3_exec(usersDb, va("ATTACH DATABASE './%s/users/users.db' as users", level.altString), NULL, NULL, NULL);
        }
        Q_strncpyz(tempName, "users", sizeof(tempName));

        // Boe!Man 5/17/13: Copy the data from the backup to the in-memory database.
        sqlite3_exec(usersDb, "BEGIN", NULL, NULL, NULL);
        sqlite3_exec(usersDb, "SELECT name FROM users.sqlite_master WHERE type='table'", &process_dml_row2, usersDb, NULL);
        sqlite3_exec(usersDb, "COMMIT", NULL, NULL, NULL);
    }

    Com_Printf("Succesfully finished checking userdata integrity.\n");
    dbsLoaded = qtrue;
}

/*
================
Boe_checkAlias
12/30/12 - 12:05 PM
Function that checks if the name used is already an alias.
================
*/

qboolean Boe_checkAlias(char *ip, char *name2)
{
    char            name[MAX_NETNAME]; // name2 but without unsupported characters.
    sqlite3         *db;
    sqlite3_stmt    *stmt;

    Q_strncpyz(name, name2, sizeof(name)); // Boe!Man 12/30/12: Copy buffer and check for unsupported characters.
    Boe_convertNonSQLChars(name);

    db = aliasesDb;

    sqlite3_prepare(db, va("SELECT ID from aliases_names WHERE ID=(SELECT ID from aliases_index WHERE IP='%s' LIMIT 1) AND name='%s'", ip, name), -1, &stmt, 0);
    if(sqlite3_step(stmt) == SQLITE_DONE){ // He wasn't found on the aliases table. Return false.
        sqlite3_finalize(stmt);
        return qfalse;
    }else{ // He's on it. Return true.
        sqlite3_finalize(stmt);
        return qtrue;
    }

    return qfalse;
}

/*
================
Boe_addAlias
1/1/13 - 1:55 PM
Function that adds an alias to the database.
================
*/

void Boe_addAlias(char *ip, char *name2)
{
    char            name[MAX_NETNAME]; // name2 but without unsupported characters.
    int             indexnr, acount;
    sqlite3         *db;
    sqlite3_stmt    *stmt;

    Q_strncpyz(name, name2, sizeof(name)); // Boe!Man 12/30/12: Copy buffer and check for unsupported characters.
    Boe_convertNonSQLChars(name);

    db = aliasesDb;

    sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL);

    sqlite3_prepare(db, va("SELECT ID from aliases_index WHERE IP='%s' LIMIT 1", ip), -1, &stmt, 0);
    if(sqlite3_step(stmt) == SQLITE_DONE){ // It wasn't found on the main table, we can safely assume this guy isn't on it.
        sqlite3_finalize(stmt);
        if(sqlite3_exec(db, va("INSERT INTO aliases_index (ID, IP) values (?, '%s')", ip), 0, 0, 0) != SQLITE_OK){
            G_LogPrintf("^1Error: ^7aliases database: %s\n", sqlite3_errmsg(db));
            return;
        }
        // Try again.
        sqlite3_prepare(db, va("SELECT ID from aliases_index WHERE IP='%s' LIMIT 1", ip), -1, &stmt, 0);
        sqlite3_step(stmt);
        indexnr = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
    }else{
        // Boe!Man 1/4/13: He should be on it.
        indexnr = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
    }

    // Boe!Man 1/1/13: Now insert the data onto the next table.
    // First we check how many aliases there are.
    sqlite3_prepare(db, va("SELECT DISTINCT count(ID) from aliases_names WHERE ID='%i'", indexnr), -1, &stmt, 0);
    if(sqlite3_step(stmt) != SQLITE_DONE){
        acount = sqlite3_column_int(stmt, 0);
        if(acount >= g_aliasCount.integer){ // If this number is higher then the allowed aliases, delete some.
            if(sqlite3_exec(db, va("DELETE FROM aliases_names WHERE ID='%i' AND ROWID=(SELECT ROWID FROM aliases_names WHERE ID='%i' LIMIT 1)", indexnr, indexnr), 0, 0, 0) != SQLITE_OK){
                G_LogPrintf("^1Error: ^7aliases database: %s\n", sqlite3_errmsg(db));
                sqlite3_finalize(stmt);
                return;
            }
        }
    }
    sqlite3_finalize(stmt);

    // Now insert new name into table.
    if(sqlite3_exec(db, va("INSERT INTO aliases_names (ID, name) values (%i, '%s')", indexnr, name), 0, 0, 0) != SQLITE_OK){
        G_LogPrintf("^1Error: ^7aliases database: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_exec(db, "COMMIT", NULL, NULL, NULL);
}

/*
================
Boe_printAliases
1/3/13 - 10:18 AM
Prints all aliases to the screen. Can only be called via stats.
================
*/

void Boe_printAliases(gentity_t *ent, char *ip, char *name2)
{
    sqlite3         *db;
    sqlite3_stmt    *stmt;
    int             count;
    char            name[MAX_NETNAME];
    char            names[1024];

    count = 0;
    Q_strncpyz(name, name2, sizeof(name)); // Boe!Man 12/30/12: Copy buffer and check for unsupported characters.
    Boe_convertNonSQLChars(name);

    db = aliasesDb;

    sqlite3_prepare(db, va("SELECT name,ROWID from aliases_names WHERE ID=(SELECT ID from aliases_index WHERE IP='%s' LIMIT 1) AND name!='%s' ORDER BY ROWID DESC LIMIT %i", ip, name, g_aliasCount.integer), -1, &stmt, 0);
    while(sqlite3_step(stmt) != SQLITE_DONE){
        if(!count){
            Com_sprintf(names, sizeof(names), "%s", sqlite3_column_text(stmt, 0));
        }else{
            Com_sprintf(names+strlen(names), sizeof(names), "%-14s %s", "\n", sqlite3_column_text(stmt, 0));
        }
        count++;
    }

    sqlite3_finalize(stmt);

    if(!count){
        trap_SendServerCommand( ent-g_entities, va("print \"None\""));
    }else{
        names[strlen(names)] = '\0'; // NULL terminate the char array.
        trap_SendServerCommand( ent-g_entities, va("print \"%s\"", names));
    }

    memset(name, 0, sizeof(name));
    memset(names, 0, sizeof(names));
}

/*
================
Boe_SQLStats
1/30/13 - 12:36 PM
Prints a sheet containing information regarding SQLite statistics.
Accessible from RCON only. Mainly here for debug purposes.
================
*/

void Boe_SQLStats(void)
{
    int     mem;
    float   memInM;

    mem = sqlite3_memory_used();
    memInM = mem;
    memInM = memInM/1024/1024;

    Com_Printf("^3[SQLite3 Statistics]\n");
    Com_Printf("--------------------------------------\n");

    // Boe!Man 1/30/13: Hard-coded statistics, i.e. things that never change.
    #ifdef WIN32
    Com_Printf("%-25s Windows\n", "[^3Host platform^7]");
    #elif defined(__linux__)
    Com_Printf("%-25s Linux\n", "[^3Host platform^7]");
    #elif defined(MACOS_X)
    Com_Printf("%-25s Mac OS X\n", "[^3Host platform^7]");
    #endif // _WIN32

    #if defined(__linux__) && (defined(__GNUC__) && __GNUC__ < 3)
    Com_Printf("%-25s memsys5\n", "[^3Memory allocator^7]");
    Com_Printf("%-25s 41943040B (40M)\n", "[^3Heap limit^7]");
    #else
    Com_Printf("%-25s Native\n", "[^3Memory allocator^7]");
    Com_Printf("%-25s None defined\n", "[^3Heap limit^7]");
    #endif // __linux__ && GNUC < 3
    Com_Printf("%-25s %iB (%.2fM)\n", "[^3Mem in use^7]", mem, memInM);

    Com_Printf("\nUse ^3[Page Up] ^7and ^3[Page Down] ^7keys to scroll\n\n");
}

/*
================
Boe_convertNonSQLChars
2/6/13 - 1:38 PM
Checks for malformed (incompatible) characters in query values (such as names).
================
*/

void Boe_convertNonSQLChars(char *input)
{
    char    *s;

    s = input;
    while (*s) {
        // Boe!Man 2/6/13
        // Unsupported characters in a query:
        // ==================================
        // Single quote ('): 39 on the ASCII table.
        // Backslash (\): 92 on the ASCII table.
        if(*s == 39 || *s == 92){
            *s = 32; // If found, convert to space (32 on ASCII table).
        }
        s++;
    }
}

/*
================
Boe_checkAdmin
2/12/13 - 7:27 PM
Function that checks if the client is an Admin.
================
*/

int Boe_checkAdmin(char *ip, char *name2)
{
    char            name[MAX_NETNAME]; // name2 but without unsupported characters.
    sqlite3         *db;
    sqlite3_stmt    *stmt;
    int             level2;

    G_ClientCleanName(name2, name, sizeof(name), qfalse); // Boe!Man 2/12/13: Get the cleanName first.
    Boe_convertNonSQLChars(name);

    db = usersDb;

    sqlite3_prepare(db, va("SELECT level from admins WHERE IP='%s' AND name='%s'", ip, name), -1, &stmt, 0);
    if(sqlite3_step(stmt) == SQLITE_DONE){ // He wasn't found on the admin table.
        sqlite3_finalize(stmt);
        return 0;
    }else{ // He's on it. Return his level.
        level2 = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        return level2;
    }
}

/*
================
Boe_checkPassAdmin
2/12/13 - 7:43 PM
Function that checks if the client is a Passworded Admin.
================
*/

int Boe_checkPassAdmin(char *name2, char *pass)
{
    char            name[MAX_NETNAME]; // name2 but without unsupported characters.
    sqlite3         *db;
    sqlite3_stmt    *stmt;
    int             level2;

    G_ClientCleanName(name2, name, sizeof(name), qtrue); // Boe!Man 2/12/13: Get the cleanName first.
    Boe_convertNonSQLChars(name);
    Boe_convertNonSQLChars(pass);

    db = usersDb;

    sqlite3_prepare(db, va("SELECT level from passadmins WHERE name='%s' AND pass='%s'", name, pass), -1, &stmt, 0);
    if(sqlite3_step(stmt) == SQLITE_DONE){ // He wasn't found on the admin table.
        level2 = 0;
    }else{
        level2 = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return level2;
}

/*
================
Boe_checkPassAdmin2
8/25/14 - 7:49 PM
Function that checks if the client was already added once as Passworded Admin.
================
*/

qboolean Boe_checkPassAdmin2(char *name2)
{
    char            name[MAX_NETNAME]; // name2 but without unsupported characters.
    sqlite3         *db;
    sqlite3_stmt    *stmt;

    G_ClientCleanName(name2, name, sizeof(name), qtrue); // Boe!Man 2/12/13: Get the cleanName first.
    Boe_convertNonSQLChars(name);

    db = usersDb;

    sqlite3_prepare(db, va("SELECT ROWID from passadmins WHERE name='%s'", name), -1, &stmt, 0);
    if (sqlite3_step(stmt) == SQLITE_DONE){ // He wasn't found on the admin table.
        sqlite3_finalize(stmt);
        return qfalse;
    }

    sqlite3_finalize(stmt);
    return qtrue;
}

/*
================
Boe_addPasswordToDatabase
8/25/14 - 7:10 PM
Function that adds a (new) password to the database.
================
*/

void Boe_addPasswordToDatabase(char *ip, char *name2, char *pass)
{
    char            name[MAX_NETNAME]; // name2 but without unsupported characters.
    sqlite3         *db;

    G_ClientCleanName(name2, name, sizeof(name), qtrue); // Boe!Man 2/12/13: Get the cleanName first.
    Boe_convertNonSQLChars(name);
    Boe_convertNonSQLChars(pass);

    db = usersDb;
    if (sqlite3_exec(db, va("UPDATE passadmins set 'pass'='%s' WHERE name='%s'", pass, name), 0, 0, 0) != SQLITE_OK){
        // This should never happen.
        G_LogPrintf("^1Error: ^7users database: %s\n", sqlite3_errmsg(db));
    }
}

/*
================
Boe_checkClanMember
2/12/13 - 7:34 PM
Function that checks if the client is a clan member.
================
*/

qboolean Boe_checkClanMember(char *ip, char *name2)
{
    char            name[MAX_NETNAME]; // name2 but without unsupported characters.
    sqlite3         *db;
    sqlite3_stmt    *stmt;

    G_ClientCleanName(name2, name, sizeof(name), qtrue); // Boe!Man 2/12/13: Get the cleanName first.
    Boe_convertNonSQLChars(name);

    db = usersDb;

    sqlite3_prepare(db, va("SELECT name from clanmembers WHERE IP='%s' AND name='%s'", ip, name), -1, &stmt, 0);
    if(sqlite3_step(stmt) == SQLITE_DONE){ // He wasn't found on the admin table.
        sqlite3_finalize(stmt);
        return qfalse;
    }else{ // He's on it. Return true.
        sqlite3_finalize(stmt);
        return qtrue;
    }
}

/*
================
Boe_unloadUserdataDbs
5/27/13 - 12:55 PM
Unloads userdata in-memory databases to free up memory.
================
*/

void Boe_unloadUserdataDbs(void)
{
    if(!dbsLoaded)
        return;

    Boe_backupInMemoryDbs("users.db", usersDb);
    sqlite3_exec(usersDb, "DETACH DATABASE users", NULL, NULL, NULL);
    sqlite3_close(usersDb);

    Boe_backupInMemoryDbs("aliases.db", aliasesDb);
    sqlite3_exec(aliasesDb, "DETACH DATABASE aliases", NULL, NULL, NULL);
    sqlite3_close(aliasesDb);

    Boe_backupInMemoryDbs("bans.db", bansDb);
    sqlite3_exec(bansDb, "DETACH DATABASE bans", NULL, NULL, NULL);
    sqlite3_close(bansDb);

    Com_Printf("Unloaded userdata databases.\n");
}

/*
================
Boe_backupInMemoryDbs
5/27/13 - 1:46 PM
Backs up in-memory databases to disk.
================
*/

void Boe_backupInMemoryDbs(char *fileName, sqlite3 *db)
{
    sqlite3_backup  *pBackup;    // Boe!Man 5/27/13: Backup handle used to copy data.
    sqlite3         *pFile;
    int             rc;

    if(!level.altPath){
        rc = sqlite3_open(va("./users/%s", fileName), &pFile);
    }else{
        rc = sqlite3_open(va("%s/users/%s", level.altString, fileName), &pFile);
    }

    if(rc){
        G_LogPrintf("Critical error backing up in-memory database %s: %s\n", fileName, sqlite3_errmsg(pFile));
        return;
    }

    // Boe!Man 7/1/13: Fixed the backup being very slow when databases are relatively full, can especially be noted when restarting the map.
    sqlite3_exec(pFile, "PRAGMA synchronous = OFF", NULL, NULL, NULL);
    sqlite3_exec(pFile, "BEGIN TRANSACTION", NULL, NULL, NULL);

    pBackup = sqlite3_backup_init(pFile, "main", db, "main");

    if(pBackup){
        sqlite3_backup_step(pBackup, -1);
        // Boe!Man 5/27/13: Release resources allocated by backup_init().
        sqlite3_backup_finish(pBackup);
    }

    rc = sqlite3_errcode(pFile);
    if(rc){
        G_LogPrintf("SQLite3 error while backing up data in %s: %s\n", fileName, sqlite3_errmsg(pFile));
    }

    sqlite3_exec(pFile, "COMMIT", NULL, NULL, NULL);
    sqlite3_close(pFile);
}

/*
================
Boe_SQLTableClear
8/21/13 - 11:18 AM
Clears a userdata table.
================
*/

void Boe_SQLTableClear(void)
{
    char    arg[MAX_STRING_TOKENS];
    sqlite3 *db;
    int     rc;

    // Fetch the argument so we can determine what list the user wishes to clear.
    trap_Argv(1, arg, sizeof(arg));
    Q_strlwr(arg);

    if(strstr(arg, "subnetbanlist")){
        if(sqlite3_exec(bansDb, "DELETE FROM subnetbans", 0, 0, 0) != SQLITE_OK){
            G_LogPrintf("^1Error: ^7Bans database: %s\n", sqlite3_errmsg(bansDb));
        }else{
            sqlite3_exec(bansDb, "VACUUM", NULL, NULL, NULL);
            Com_Printf("Successfully cleared all subnetbans from the database!\n");
        }
    }else if(strstr(arg, "banlist")){
        if(sqlite3_exec(bansDb, "DELETE FROM bans", 0, 0, 0) != SQLITE_OK){
            G_LogPrintf("^1Error: ^7Bans database: %s\n", sqlite3_errmsg(bansDb));
        }else{
            sqlite3_exec(bansDb, "VACUUM", NULL, NULL, NULL);
            Com_Printf("Successfully cleared all bans from the database!\n");
        }
    }else if(strstr(arg, "adminlist")){
        if(sqlite3_exec(usersDb, "DELETE FROM admins", 0, 0, 0) != SQLITE_OK){
            G_LogPrintf("^1Error: ^7Users database: %s\n", sqlite3_errmsg(usersDb));
        }else{
            sqlite3_exec(usersDb, "VACUUM", NULL, NULL, NULL);
            Com_Printf("Successfully cleared all admins from the database!\n");
        }
    }else if(strstr(arg, "passlist")){
        if(sqlite3_exec(usersDb, "DELETE FROM passadmins", 0, 0, 0) != SQLITE_OK){
            G_LogPrintf("^1Error: ^7Users database: %s\n", sqlite3_errmsg(usersDb));
        }else{
            sqlite3_exec(usersDb, "VACUUM", NULL, NULL, NULL);
            Com_Printf("Successfully cleared all passadmins from the database!\n");
        }
    }else if(strstr(arg, "clanlist")){
        if(sqlite3_exec(usersDb, "DELETE FROM clanmembers", 0, 0, 0) != SQLITE_OK){
            G_LogPrintf("^1Error: ^7Users database: %s\n", sqlite3_errmsg(usersDb));
        }else{
            sqlite3_exec(usersDb, "VACUUM", NULL, NULL, NULL);
            Com_Printf("Successfully cleared all clanmembers from the database!\n");
        }
    }else if(strstr(arg, "aliases")){
        if(sqlite3_exec(aliasesDb, "DELETE FROM aliases_index", 0, 0, 0) != SQLITE_OK){
            G_LogPrintf("^1Error: ^7Aliases database: %s\n", sqlite3_errmsg(aliasesDb));
        }else{
            if(sqlite3_exec(aliasesDb, "DELETE FROM aliases_names", 0, 0, 0) != SQLITE_OK){
                G_LogPrintf("^1Error: ^7Aliases database: %s\n", sqlite3_errmsg(aliasesDb));
            }else{
                sqlite3_exec(aliasesDb, "VACUUM", NULL, NULL, NULL);
                Com_Printf("Successfully cleared all aliases from the database!\n");
            }
        }
    }else if(strstr(arg, "scores")){
        if(!level.altPath){
            rc = sqlite3_open_v2("./users/scores.db", &db, SQLITE_OPEN_READWRITE, NULL);
        }else{
            rc = sqlite3_open_v2(va("%s/users/scores.db", level.altString), &db, SQLITE_OPEN_READWRITE, NULL);
        }

        if(rc){
            G_LogPrintf("^1Error: ^7scores database: %s\n", sqlite3_errmsg(db));
        }

        sqlite3_exec(db, "PRAGMA writable_schema = 1", NULL, NULL, NULL);
        if(sqlite3_exec(db, "delete from sqlite_master where type = 'table'", 0, 0, 0) != SQLITE_OK){
            G_LogPrintf("^1Error: ^7Scores database: %s\n", sqlite3_errmsg(db));
        }
        sqlite3_exec(db, "PRAGMA writable_schema = 0", NULL, NULL, NULL);
        sqlite3_exec(db, "VACUUM", NULL, NULL, NULL);
        if(sqlite3_exec(db, "PRAGMA INTEGRITY_CHECK", NULL, NULL, NULL) == SQLITE_OK){
            Com_Printf("Successfully cleared the scores database!\n");
        }else{
            G_LogPrintf("^1Error: ^7Scores database was not successfully cleared (did not pass integrity check).\n");
        }

        sqlite3_close(db);
    }else{
        Com_Printf("^3Info: ^7Invalid choice: %s. Valid choices are: subnetbanlist, banlist, adminlist, passlist, clanlist, aliases, scores.\n", arg);
    }
}

/*
================
logCrash
10/8/13 - 5:43 PM
Writes soft-crash information (triggered on Com_Error) to the crashlog file.
================
*/

void logCrash(void)
{
    fileHandle_t    crashFile;
    char            crashReason[128];
    char            text[148];
    qtime_t         q;

    // Boe!Man 1/2/14: Check if the server crashed.
    trap_Cvar_VariableStringBuffer("com_errorMessage", crashReason, sizeof(crashReason));
    if(crashReason == NULL || strlen(crashReason) == 0){
        return;
    }

    // Boe!Man 11/22/10: Appending the date & time.
    trap_RealTime(&q);

    Com_sprintf(text, sizeof(text), "%02i/%02i/%i %02i:%02i - %s\n", 1+q.tm_mon,q.tm_mday, q.tm_year+1900,q.tm_hour,q.tm_min, crashReason);

    // Boe!Man 11/22/10: Open and write to the crashinfo file.
    trap_FS_FOpenFile("logs/crashlog.txt", &crashFile, FS_APPEND_TEXT);
    if(!crashFile){
        return;
    }

    trap_FS_Write(text, strlen(text), crashFile);
    trap_FS_FCloseFile(crashFile);
}
