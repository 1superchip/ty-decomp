#include "types.h"
#include "common/Translation.h"
#include "common/Heap.h"
#include "common/FileSys.h"
#include "common/Str.h"

extern "C" int atoi(char*);
extern "C" int strcmpi(char*, char*);
extern "C" int strlen(char*);
extern "C" void memmove(char*, char*, int);
extern "C" u8 OSGetLanguage(void);
extern char gNullStr[];
extern int gDisplay[27];


static TranslationLanguage gCurrentLanguage = Language_NotSet;

static int gNmbrOfStrings;
static char* gpTranslationBuffer;
static char* pDOLanguage;
static int gLanguageDebugOption;

static Language gLanguageInfo[8] = {
	{"English (UK)", "English", "EN", "Translations.English.txt", 0},
	{"English (US)", "American", "EN", "Translations.American.txt", 0},
	{"Francais", "French", "FR", "Translations.French.txt", 0},
	{"Espa\xF1ol", "Spanish", "ES", "Translations.Spanish.txt", 0},
	{"Deutsch", "German", "DE", "Translations.German.txt", 0},
	{"Italiano", "Italian", "IT", "Translations.Italian.txt", 0},
	{"Nederlands", "Dutch", "NL", "Translations.Dutch.txt", 0},
	{"Japanese", "Japanese", "JP", "Translations.Japanese.txt", 0}
};

char* gpTranslation_StringArray[1025];
static char* gpEnumTagArray[1025];
static char* gpLanguageOptionStrings[9];
static int gLanguageOptionLanguages[9];

void Translation_InitModule(void) {
    int i;
    int maxSize = 0;
    int var_r25 = 0;
    for(i = 0; i < 8; i++) {
        if (!FileSys_Exists(gLanguageInfo[i].fileName, &gLanguageInfo[i].size)) {
            gLanguageInfo[i].size = 0;
        } else {
            gpLanguageOptionStrings[var_r25] = gLanguageInfo[i].languageName;
            gLanguageOptionLanguages[var_r25] = i;
            var_r25++;
            if (gLanguageInfo[i].size > maxSize) {
                maxSize = gLanguageInfo[i].size;
            }
        }
    }
    gpLanguageOptionStrings[var_r25] = NULL;
    gpTranslationBuffer = (char*)Heap_MemAlloc(maxSize + 1);
    gNmbrOfStrings = 0;
    gCurrentLanguage = Language_NotSet;
    pDOLanguage = NULL;
}

void Translation_DeinitModule(void) {
    Heap_MemFree(gpTranslationBuffer);
    gpTranslationBuffer = NULL;
    gCurrentLanguage = Language_NotSet;
}

TranslationLanguage Translation_GetLanguage(void) {
	return gCurrentLanguage;
}

TranslationLanguage Translation_GetDefaultLanguage(void) {
    TranslationLanguage defaultLanguage = Language_English;
    switch (OSGetLanguage()) {
    case 0:
        defaultLanguage = Language_English;
        break;
    case 1:
        defaultLanguage = Language_German;
        break;
    case 2:
        defaultLanguage = Language_French;
        break;
    case 3:
        defaultLanguage = Language_Spanish;
        break;
    case 4:
        defaultLanguage = Language_Italian;
        break;
    case 5:
        defaultLanguage = Language_Dutch;
        break;
    }
    if (defaultLanguage == Language_English || Translation_IsLanguageAvailable(defaultLanguage) == false) {
        if (gDisplay[0] == 1 && Translation_IsLanguageAvailable(Language_American) != false) {
            defaultLanguage = Language_American;
        } else {
            defaultLanguage = Language_English;
        }
    }
    return defaultLanguage;
}

inline void SetDebugOption(void) {
    int option = 0;
    gLanguageDebugOption = 0;
    while (gCurrentLanguage != gLanguageOptionLanguages[option]) {
        option++;
    }
    gLanguageDebugOption = option;
}

void Translation_SetLanguage(TranslationLanguage language) {
    gNmbrOfStrings = 0;
    Translation_Platform platformId = Platform_NotSet;
    Translation_Platform currentPlatform = Platform_GCN;
    gCurrentLanguage = language;
    FileSys_Load(gLanguageInfo[language].fileName, NULL, gpTranslationBuffer, -1);
    gpTranslationBuffer[gLanguageInfo[language].size] = '\0';
    for (int idx = 0; idx < gLanguageInfo[language].size; idx++) {
        if (gpTranslationBuffer[idx] == '\r') {
            gpTranslationBuffer[idx] = '\n';
        }
    }
    char *translationBuffer = gpTranslationBuffer;
    goto start;
    while (translationBuffer < (gpTranslationBuffer + gLanguageInfo[language].size)) {
    start:
        while (translationBuffer[0] == ' ') {
            translationBuffer++;
        }
        if (translationBuffer[0] == '\n') {
            translationBuffer++;
        } else {
            if (translationBuffer[0] == '#') {
                while (translationBuffer[0] != '\0' && translationBuffer[0] != '\n') {
                    translationBuffer++;
                }
            } else {
                gpEnumTagArray[gNmbrOfStrings] = translationBuffer;
                while (translationBuffer[0] > ' ') {
                    translationBuffer++;
                }
                *translationBuffer = '\0';
                translationBuffer++;
                if (*gpEnumTagArray[gNmbrOfStrings] == '[') {
                    if (strcmpi(gpEnumTagArray[gNmbrOfStrings], "[]") == 0) {
                        platformId = Platform_NotSet;
                    } else if (strcmpi(gpEnumTagArray[gNmbrOfStrings], "[PS2]") == 0) {
                        platformId = Platform_PS2;
                    } else if (strcmpi(gpEnumTagArray[gNmbrOfStrings], "[GCN]") == 0) {
                        platformId = Platform_GCN;
                    } else if (strcmpi(gpEnumTagArray[gNmbrOfStrings], "[XBOX]") == 0) {
                        platformId = Platform_XBOX;
                    }
                    while (translationBuffer[0] != '\0' && translationBuffer[0] != '\n') {
                        translationBuffer++;
                    }
                } else {
                    // not set or GCN
					// load strings for set platform
                    if (platformId == Platform_NotSet || platformId == currentPlatform) {
                        char *found = Str_FindChar(gpEnumTagArray[gNmbrOfStrings], '=');
                        if (found != NULL) {
                            int value = atoi(found + 1);
                            while (gNmbrOfStrings < value) {
                                gpEnumTagArray[gNmbrOfStrings + 1] = gpEnumTagArray[gNmbrOfStrings];
                                gpEnumTagArray[gNmbrOfStrings] = gNullStr;
                                gNmbrOfStrings++;
                            }
                            *found = '\0';
                        }
                        while (translationBuffer[0] == ' ' || translationBuffer[0] == '\t') {
                            translationBuffer++;
                        }
                        if (translationBuffer[0] == '\0' || translationBuffer[0] == '\n') {
                            gpTranslation_StringArray[gNmbrOfStrings++] = gNullStr;
                        } else {
                            gpTranslation_StringArray[gNmbrOfStrings++] = translationBuffer;
                            while (*translationBuffer != '\0' && *translationBuffer != '\n') {
                                if (*translationBuffer == '\\' && *(translationBuffer + 1) == 'n') {
                                    memmove(translationBuffer, translationBuffer + 1,
                                            strlen(translationBuffer));
                                    *translationBuffer = '\n';
                                }
                                translationBuffer++;
                            }
                            char *buffer = translationBuffer - 1;
                            *translationBuffer++ = '\0';
                            while (buffer >= gpTranslation_StringArray[gNmbrOfStrings - 1] - 1) {
                                if (*buffer != ' ' && *buffer != '\t') {
                                    break;
                                }
                                *buffer = '\0';
                                buffer--;
                            }
                        }
                    }
                }
            }
        }
    }
    SetDebugOption();
}

bool Translation_IsLanguageAvailable(TranslationLanguage language) {
    return gLanguageInfo[language].size;
}

char* Translation_GetLanguageName(TranslationLanguage language) {
	return gLanguageInfo[language].languageName;
}

char* Translation_GetLanguageLocalName(TranslationLanguage language) {
	return gLanguageInfo[language].localName;
}

char* Translation_GetLanguageCode(TranslationLanguage language) {
	return gLanguageInfo[language].languageCode;
}