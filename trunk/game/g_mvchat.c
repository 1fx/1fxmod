// Copyright (C) 2018 - Boe!Man, Henkie.
//
// g_mvchat.c - Multilingual voice chat (mvchat) sound system.

#include "g_local.h"
#include "boe_local.h"

//=============================================

#define MVCHAT_MAX_SOUNDS   1000

//
// mvchatLangInfo_t
//
// This structure contains the two letter country code,
// conform to the ISO-3166-2 spec. and the full
// country name in English.
//

typedef struct {
    const char          *langCode;                          // Two letter country code.
    const char          *langName;                          // Full country name.
} mvchatLangInfo_t;

//
// mvchatLangs
//
// This array contains all languages supported
// by the mvchat system.
//

static mvchatLangInfo_t mvchatLangs[] = {
    { "en", "english"   },
    { "es", "spanish"   },
    { "cn", "chinese"   },
    { "de", "german"    },
    { "ru", "russian"   }
};

#define MVCHAT_NUM_LANGS    (sizeof(mvchatLangs) / sizeof(mvchatLangs[0]))

//
// mvchatSoundLang_t
//
// This structure contains two pointers,
// to both the male and female sound.
// Either one may be NULL if unused.
//

typedef struct {
    const char          *maleSound;                         // The path to the male sound.
    const char          *femaleSound;                       // The path to the female sound.
} mvchatSoundLang_t;

//
// mvchatSound_t
//
// This structure contains all parsed sounds,
// the text to display and which sound
// is considered the default one.
//

typedef struct {
    const char          *text;                              // The text to display.
    mvchatSoundLang_t   *sounds[MVCHAT_NUM_LANGS];          // Array containing actual sounds per language.

    mvchatSoundLang_t   *defaultSoundLanguage;              // The default sound language sounds.
    mvchatSoundLang_t   *nextSoundLanguage;                 // Language of sound marked to be used as "next" sound (#n chat token).
} mvchatSound_t;

//=============================================

// The actual array of the parsed sounds.
static mvchatSound_t    *mvchatSounds[MVCHAT_MAX_SOUNDS]    = {0};

// Pointer to the sound marked to be used as "next" sound (#n chat token).
mvchatSound_t           *nextSound                          = NULL;

//=============================================

/*
================
mvchat_parseFiles

Parses the mvchat sound
files found.

Mvchat sound files should
be in the files/mvchats
directory.
================
*/

void mvchat_parseFiles()
{
    char            mvchatFiles[4096];
    char            *mvchatPtr;
    int             mvchatCount;
    int             i, x;
    int             fnameLen;

    // Variables used for statistics.
    int             mvchatSoundNumberCount = 0;
    int             mvchatSoundFileCount = 0;
    int             maleSoundsParsed[MVCHAT_NUM_LANGS];
    int             femaleSoundsParsed[MVCHAT_NUM_LANGS];

    Com_Printf("------- MVCHAT sound system -------\n");
    Com_Printf("Multilingual voice chat sound system.\n\n");

    // Zero-initialize statistics variables.
    mvchatSoundNumberCount = 0;
    mvchatSoundFileCount = 0;
    memset(maleSoundsParsed, 0, sizeof(int) * MVCHAT_NUM_LANGS);
    memset(femaleSoundsParsed, 0, sizeof(int) * MVCHAT_NUM_LANGS);

    // Get the available .mvchat files.
    mvchatCount = trap_FS_GetFileList("files/mvchats", ".mvchat", mvchatFiles, sizeof(mvchatFiles));

    // There is no need to continue here in the
    // absence of mvchat files.
    if(!mvchatCount){
        Com_Printf(S_COLOR_YELLOW "WARNING: No .mvchat files found to parse!\n");
        return;
    }

    //
    // Start iterating through the .mvchat files.
    //
    Com_Printf("Parsing .mvchat files.\n");
    mvchatPtr = mvchatFiles;

    for(i = 0; i < mvchatCount; i++, mvchatPtr += fnameLen + 1){
        // GP2 mvchat file variables.
        TGenericParser2 mvchatFile;
        TGPGroup        mvchatGroup;

        // Determine the length of the .mvchat file name.
        // We use this to increment the mvchatPtr pointer in
        // case of error or success.
        fnameLen = strlen(mvchatPtr);

        // Start parsing the .mvchat file.
        // A voice chat file is required to be Generic Parser 2 compatible.
        mvchatFile = trap_GP_ParseFile(va("files/mvchats/%s", mvchatPtr), qtrue, qfalse);
        Com_Printf("... %s\n", mvchatPtr);
        if(!mvchatFile){
            Com_Printf(S_COLOR_RED "Unable to parse the voice chat file!\n");
            Com_Printf(S_COLOR_RED "Skipping this file.\n");
            continue;
        }

        //
        // File successfully initialized by GP2.
        // Iterate through all sounds found in this file.
        //
        mvchatGroup = trap_GPG_GetSubGroups(mvchatFile);
        while(mvchatGroup){
            char                mvchatGroupName[32];
            mvchatSound_t       *mvchatSound;
            int                 sNum;
            char                sNumText[4];
            char                sText[MAX_SAY_TEXT];

            // Get the base information for this sound.
            // Store a copy of the GP2 group name.
            trap_GPG_GetName(mvchatGroup, mvchatGroupName);

            // Get the sound number and text.
            trap_GPG_FindPairValue(mvchatGroup, "number", "0", sNumText);
            trap_GPG_FindPairValue(mvchatGroup, "text", "", sText);

            // Convert the sound number to an integer, so we can check
            // if it is properly within range.
            sNum = atoi(sNumText);
            if(sNum < 1 || sNum > MVCHAT_MAX_SOUNDS - 1){
                // The number is out of range.
                Com_Printf(S_COLOR_YELLOW "WARNING: Sound block %s: Number out of range, is %d, must be 1-%d.\n",
                    mvchatGroupName, sNum, MVCHAT_MAX_SOUNDS - 1);

                // Advance to the next sound in this file.
                mvchatGroup = trap_GPG_GetNext(mvchatGroup);
                continue;
            }

            // Is the sound number already initialized?
            if(mvchatSounds[sNum] != NULL){
                // The number is already used once.
                Com_Printf(S_COLOR_YELLOW "WARNING: Sound block %s: Trying to use sound number %d more than once!\n",
                    mvchatGroupName, sNum);

                // Advance to the next sound in this file.
                mvchatGroup = trap_GPG_GetNext(mvchatGroup);
                continue;
            }

            // The initial check seems OK.
            // We're going to initialize this sound now.
            mvchatSound
                = mvchatSounds[sNum]
                = trap_VM_LocalAlloc(sizeof(mvchatSound_t));
            mvchatSoundNumberCount++;

            // Zero-initialize the sounds pointer array with the
            // number of languages available.
            memset(mvchatSound->sounds, 0, sizeof(mvchatSoundLang_t) * MVCHAT_NUM_LANGS);

            // Allocate some space for the text of this sound.
            mvchatSound->text = trap_VM_LocalStringAlloc(sText);

            // Mandatory info is now stored.
            // Iterate through the available languages to use for this sound.
            qboolean soundFound = qfalse;
            for(x = 0; x < MVCHAT_NUM_LANGS; x++){
                mvchatLangInfo_t    *currLang = &mvchatLangs[x];
                TGPGroup            langSound;
                TGPGroup            langSoundGender;
                char                sSound[MAX_QPATH];
                char                sOption[4];

                // Is the current language we're trying to parse available?
                langSound = trap_GPG_FindSubGroup(mvchatGroup, currLang->langCode);
                if(!langSound){
                    // Language not specified, moving on.
                    continue;
                }

                // Look for the male sound.
                langSoundGender = trap_GPG_FindSubGroup(langSound, "male");
                if(langSoundGender){
                    // Male sound found.
                    // Grab the sound, if specified.
                    trap_GPG_FindPairValue(langSoundGender, "sound", "", sSound);
                    if(sSound[0] != 0){
                        // Valid sound. Create an entry for this language.
                        // Initialize the sound language entry if necessary.
                        if(mvchatSound->sounds[x] == NULL){
                            mvchatSound->sounds[x] = trap_VM_LocalAlloc(sizeof(mvchatSoundLang_t));
                        }
                        // Copy this sound.
                        mvchatSound->sounds[x]->maleSound = trap_VM_LocalStringAlloc(sSound);
                        soundFound = qtrue;

                        // Increase statistics.
                        mvchatSoundFileCount++;
                        maleSoundsParsed[x]++;
                    }
                }

                // Look for the female sound.
                langSoundGender = trap_GPG_FindSubGroup(langSound, "female");
                if(langSoundGender){
                    // Female sound found.
                    // Grab the sound, if specified.
                    trap_GPG_FindPairValue(langSoundGender, "sound", "", sSound);
                    if(sSound[0] != 0){
                        // Valid sound. Create an entry for this language.
                        // Initialize the sound language entry if necessary.
                        if(mvchatSound->sounds[x] == NULL){
                            mvchatSound->sounds[x] = trap_VM_LocalAlloc(sizeof(mvchatSoundLang_t));
                        }

                        // Copy this sound.
                        mvchatSound->sounds[x]->femaleSound = trap_VM_LocalStringAlloc(sSound);
                        soundFound = qtrue;

                        // Increase statistics.
                        mvchatSoundFileCount++;
                        femaleSoundsParsed[x]++;
                    }
                }

                // Is this the default language?
                trap_GPG_FindPairValue(langSound, "default", "no", sOption);
                if(Q_stricmp(sOption, "yes") == 0){
                    mvchatSound->defaultSoundLanguage = mvchatSound->sounds[x];
                }

                // Is this the next sound for the #n chat token?
                trap_GPG_FindPairValue(langSound, "next", "no", sOption);
                if(Q_stricmp(sOption, "yes") == 0){
                    mvchatSound->nextSoundLanguage = mvchatSound->sounds[x];
                    nextSound = mvchatSound;
                }
            }

            // Detect if no sounds were parsed from this group.
            if(!soundFound){
                // No (valid) sound was found for this sound block.
                Com_Printf(S_COLOR_YELLOW "WARNING: Sound block %s: No valid sound file specified!\n", mvchatGroupName);
                mvchatSounds[sNum] = NULL;
            // Always make sure there's a default sound language set.
            }else if(mvchatSound->defaultSoundLanguage == NULL){
                // Iterate through all available sound languages.
                // Take first language with a sound available - doesn't matter which.
                // We *always* need a default language set.
                for(x = 0; x < MVCHAT_NUM_LANGS; x++){
                    if(!mvchatSound->sounds[x]){
                        continue;
                    }

                    if(mvchatSound->sounds[x]->maleSound || mvchatSound->sounds[x]->femaleSound){
                        mvchatSound->defaultSoundLanguage = mvchatSound->sounds[x];
                        break;
                    }
                }
            }

            // Advance to the next sound in this file.
            mvchatGroup = trap_GPG_GetNext(mvchatGroup);
        }
    }

    // All files are parsed.
    Com_Printf("\n");

    //
    // Print the amount of sounds found, among the files they hold.
    //
    Com_Printf("Using %d sounds containing %d sound files.\n", mvchatSoundNumberCount, mvchatSoundFileCount);

    //
    // Iterate through languages and print statistical information.
    //
    for(i = 0; i < MVCHAT_NUM_LANGS; i++){
        Com_Printf("... %s (male: %d, female: %d)\n", mvchatLangs[i].langName, maleSoundsParsed[i], femaleSoundsParsed[i]);
    }

    Com_Printf("-----------------------------------\n");
}

/*
================
mvchat_chatDetermineSound

Parses a full chat line. Start point
is the sound indicator.

Fills up the chatParse struct passed
on to this function. The chatParse
pointer must be valid and zero-initialized.

Sound is determined from either the
language specified with optional
selection for the gender.

Either the specified lanauge (+ gender)
sound is used or the closest language or
gender sound available is used.
================
*/

void mvchat_chatDetermineSound(mvchat_ChatParse_t *chatParse, char *chatText, TIdentity *identity)
{
    int                 i, num;
    char                buf[MAX_SAY_TEXT];
    mvchatSoundLang_t   *soundLang;
    const char          *sound;
    int                 lCode               = -1;
    int                 skinGender          = -1;
    char                sNum[MAX_SAY_TEXT]  = {0};
    char                langCode[3]         = {0};
    char                genderCode[3]       = {0};
    qboolean            langOptionValid     = qfalse;
    qboolean            genderOptionValid   = qfalse;

    // The chatParse pointer passed must be valid.
    if(chatParse == NULL){
        return;
    }

    // Check the first character, is the user trying to play
    // a sound without the text being broadcasted?
    if(chatText[0] == '&' || chatText[0] == '!'){
        chatParse->displayNoText = qtrue;
    }

    // Copy the chat text to the temporary buffer.
    Q_strncpyz(buf, chatText + 1, sizeof(buf));

    //
    // Determine the number of the sound.
    //

    // Iterate through the chat.
    for(i = 0; i < strlen(buf); i++){
        // Determine number.
        if(buf[i] >= '0' && buf[i] <= '9'){
            continue;
        }
        break;
    }

    // If no number was detected in the chat, this is not a valid sound request.
    if(i == 0){
        return;
    }

    // Got the number.
    strncpy(sNum, buf, i);

    // Iterate through the rest of the chat to get additional sound options.
    for( ; i < strlen(buf); i++){
        // Check if they added a country code.
        if(langCode[0] == 0){
            langCode[0] = buf[i];
        }else if(langCode[1] == 0){
            langCode[1] = buf[i];
        // Check if the gender is appended.
        }else if(genderCode[0] == 0){
            if(buf[i] == ':'){
                genderCode[0] = buf[i];
            }else{
                // No need to continue if the next character is anything else.
                break;
            }
        }else if(genderCode[1] == 0){
            genderCode[1] = buf[i];
        }else{
            // End of available options.
            break;
        }
    }

    //
    // Check if the sound requested is legit.
    //

    // Is the number out of bounds?
    num = atoi(sNum);
    if(num < 0 || num > MVCHAT_MAX_SOUNDS - 1){
        return;
    }
    // Is the number not used on the server?
    if(mvchatSounds[num] == NULL){
        return;
    }

    //
    // Is the language code valid?
    //
    if(langCode[0] != 0){
        for(i = 0; i < MVCHAT_NUM_LANGS; i++){
            if(Q_stricmp(mvchatLangs[i].langCode, langCode) == 0){
                lCode = i;
                langOptionValid = qtrue;
                break;
            }
        }
    }

    //
    // Determine sound gender.
    //

    // Do we have a set gender code and was the previously
    // determined language code valid?
    if(lCode != -1 && genderCode[1] != 0){
        // Try to determine the gender from the code specified.
        if(Q_stricmp(":m", genderCode) == 0){
            skinGender = GENDER_MALE;
            genderOptionValid = qtrue;
        }else if(Q_stricmp(":f", genderCode) == 0){
            skinGender = GENDER_FEMALE;
            genderOptionValid = qtrue;
        }
    }

    // Try auto detection based on skin if not set.
    if(skinGender == -1){
        // Is the identity set properly?
        if(identity && identity->mCharacter){
            // Female part of the model name?
            if(strstr(identity->mCharacter->mModel, "female")){
                skinGender = GENDER_FEMALE;
            }else{
                skinGender = GENDER_MALE;
            }
        }else{
            // Default to male if the identity is unset.
            skinGender = GENDER_MALE;
        }
    }

    //
    // Determine sound.
    //

    // Do we not have a set language code?
    if(lCode == -1 || mvchatSounds[num]->sounds[lCode] == NULL){
        // Take default language sound.
        soundLang = mvchatSounds[num]->defaultSoundLanguage;
    }else{
        // Take the sounds for the specified language.
        soundLang = mvchatSounds[num]->sounds[lCode];
    }

    // Sound available for our requested gender?
    if(skinGender == GENDER_MALE){
        if(soundLang->maleSound){
            sound = soundLang->maleSound;
        }else{
            sound = soundLang->femaleSound;
        }
    }else{
        if(soundLang->femaleSound){
            sound = soundLang->femaleSound;
        }else{
            sound = soundLang->maleSound;
        }
    }

    //
    // Set the remaining information for this parsed chat.
    //

    // Determine how many characters to strip from the chat.
    if(langOptionValid){
        chatParse->stripChars += 2;
    }
    if(genderOptionValid){
        chatParse->stripChars += 2;
    }
    chatParse->stripChars += strlen(sNum) + 1;

    // Set remaining base information.
    chatParse->text = mvchatSounds[num]->text;
    chatParse->shouldSoundPlay = qtrue;
    chatParse->soundIndex = G_SoundIndex((char *)sound);
}

/*
================
mvchat_chatGetNextSound

Gets the sound representing the
next sound (#n chat token).

The gender of the skin is also
taken in account, however
if the gender is unavailable
the sound of the opposite
gender is then used instead
(this is usually male).

Returns the sound index to
the determined sound file.
================
*/

int mvchat_chatGetNextSound(TIdentity *identity)
{
    int                 skinGender;
    mvchatSoundLang_t   *soundLang;
    const char          *sound;

    // Don't continue if there's not a next sound specified
    // in any of the mvchat files.
    if(nextSound == NULL){
        return -1;
    }

    // Check if the player has a male or female skin.
    // Is the identity set properly?
    if(identity && identity->mCharacter){
        // Female part of the model name?
        if(strstr(identity->mCharacter->mModel, "female")){
            skinGender = GENDER_FEMALE;
        }else{
            skinGender = GENDER_MALE;
        }
    }else{
        // Default to male if the identity is unset.
        skinGender = GENDER_MALE;
    }

    // Return sound depending on skin set, or the sound bound
    // to the other gender if unavailable.
    soundLang = nextSound->nextSoundLanguage;
    if(skinGender == GENDER_MALE){
        if(soundLang->maleSound){
            sound = soundLang->maleSound;
        }else{
            sound = soundLang->femaleSound;
        }
    }else{
        if(soundLang->femaleSound){
            sound = soundLang->femaleSound;
        }else{
            sound = soundLang->maleSound;
        }
    }

    // Return the sound index to this sound file.
    return G_SoundIndex((char *)sound);
}
