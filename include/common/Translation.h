#ifndef COMMON_TRANSLATION
#define COMMON_TRANSLATION

struct Language {
	char* localName;
	char* languageName;
	char* languageCode;
	char* fileName;
	int size;
};

enum TranslationLanguage {
    Language_NotSet = -1,
	LANGUAGE_ENGLISH = 0,
	Language_American = 1,
	Language_French = 2,
	Language_Spanish = 3,
	Language_German = 4,
	Language_Italian = 5,
	Language_Dutch = 6,
    LANGUAGE_NMBR_OF_LANGUAGES = 8,
};

enum Translation_Platform {
    Platform_NotSet,
    Platform_PS2,
    Platform_GCN,
    Platform_XBOX
};

void Translation_InitModule(void);
void Translation_DeinitModule(void);
TranslationLanguage Translation_GetLanguage(void);
TranslationLanguage Translation_GetDefaultLanguage(void);
void Translation_SetLanguage(TranslationLanguage);
bool Translation_IsLanguageAvailable(TranslationLanguage);
char* Translation_GetLanguageName(TranslationLanguage);
char* Translation_GetLanguageLocalName(TranslationLanguage);
char* Translation_GetLanguageCode(TranslationLanguage);

#define MAX_TRANSLATION_STRINGS (0x400)

extern char* gpTranslation_StringArray[1025];

#endif // COMMON_TRANSLATION
