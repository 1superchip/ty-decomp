#include "common/Translation.h"
#include "common/DiscErrors.h"
#include "common/demoinit.h"
#include "common/system_extras.h"
#include "types.h"

#include "Dolphin/os.h"

extern "C" int strlen(char*);
extern "C" void strncpy(char*, char*, int);
extern "C" char* strchr(char*, char);
void XFONTClearScreen(u32);
void XFONTPrintf(short, short, char*, ...);
extern u8 __ctype_map[];

struct LanguageErrorStrings {
    char* errors[5];
};

LanguageErrorStrings discErrStrings[8] = {
    {
        "An error has occurred. Turn off the power and refer to the Nintendo GameCube(tm) Instruction Booklet for further instructions.",
        "Please insert the\nTY The Tasmanian Tiger\nGame Disc.",
        "The Disc Cover is open. If you want to continue the game, please close the Disc Cover.",
        "The Game Disc could not be read. Please read the Nintendo GameCube(tm) Instruction Booklet for more information.",
        "This is not the \nTY The Tasmanian Tiger\n Game Disc. Please insert the\nTY The Tasmanian Tiger\nGame Disc."
    },
    {
        "An error has occurred. Turn off the power and refer to the Nintendo GameCube(tm) Instruction Booklet for further instructions.",
        "Please insert the\nTY The Tasmanian Tiger\nGame Disc.",
        "The Disc Cover is open. If you want to continue the game, please close the Disc Cover.",
        "The Game Disc could not be read. Please read the Nintendo GameCube(tm) Instruction Booklet for more information.",
        "This is not the \nTY The Tasmanian Tiger\n Game Disc. Please insert the\nTY The Tasmanian Tiger\nGame Disc."
    },
    {
        "Une erreur est survenue. Eteignez la console et r\xE9\x66\xE9rez-vous au manuel d'utilisation du Nintendo GameCube(tm) pour de plus amples informations.",
        "Veuillez ins\xE9rer le disque\nTY le tigre de Tasmanie.",
        "Le couvercle est ouvert. Pour continuer \xE0 jouer, veuillez le fermer.",
        "La lecture du disque a \xE9\x63hou\xE9. Veuillez vous r\xE9\x66\xE9rer au manuel d'utilisation du Nintendo GameCube(tm) pour de plus amples informations.",
        "Le disque ins\xE9r\xE9 n'est pas celui de\nTY le tigre de Tasmanie. Veuillez ins\xE9rer le disque\nTY le tigre de Tasmanie."
    },
    {
        "Ha ocurrido un error. Apaga y mira el manual de instrucciones de Nintendo GameCube(tm) para m\xE1s informaci\xF3n.",
        "Por favor, inserta el disco del juego de\nTY el Tigre de Tasmania.",
        "La Tapa est\xE1 abierta. Si quieres conrinuar jugando, cierra la Tapa.",
        "No se puede leer el disco. Por favor, lee el manual de instrucciones de Nintendo GameCube(tm) para m\xE1s informaci\xF3n.",
        "\xC9ste no es el disco de\nTY el Tigre de Tasmania. Por favor, inserta el disco de\nTY el Tigre de Tasmania."
    },
    {
        "Ein Fehler ist aufgetreten. Bitte den Nintendo GameCube(tm) ausschalten und die Bedienungsanleitung lesen, um weitere Informationen zu erhalten.",
        "Bitte die TY The Tasmanian Tiger Game Disc einlegen.",
        "Der Disc-Deckel ist ge\xF6\x66\x66net. Bitte den Disc-Deckel schliessen, um mit dem Spiel fortzufahren",
        "Diese Game Disc kann nicht gelesen werden. Bitte Bedienungsanleitung lesen, um weitere Informationen zu erhalten.",
        "Dies ist nicht die\nTY The Tasmanian Tiger Game Disc. Bitte die\nTY The Tasmanian Tiger Game Disc einlegen."
    },
    {
        "Si \xE8 verificato un errore. Spegni (OFF) e controlla il manuale d'istruzioni del Nintendo GameCube(tm) per ulteriori indicazioni.",
        "Inserisci il disco di\nTY la Tigre della Tasmania.",
        "Il coperchio del disco \xE8 aperto. Se vuoi proseguire nel gioco, chiudi il coperchio del disco.",
        "Impossibile leggere il disco di gioco. Consulta il manuale d'istruzioni del Nintendo GameCube(tm) per ulteriori indicazioni.",
        "Questo non \xE8 il disco di\nTY la Tigre della Tasmania. Inserisci il disco di\nTY la Tigre della Tasmania."
    },
    {
        "An error has occurred. Turn off the power and refer to the Nintendo GameCube(tm) Instruction Booklet for further instructions.",
        "Please insert the\nTY The Tasmanian Tiger\nGame Disc.",
        "The Disc Cover is open. If you want to continue the game, please close the Disc Cover.",
        "The Game Disc could not be read. Please read the Nintendo GameCube(tm) Instruction Booklet for more information.",
        "This is not the \nTY The Tasmanian Tiger\n Game Disc. Please insert the\nTY The Tasmanian Tiger\nGame Disc."
    },
    {
        "An error has occurred. Turn off the power and refer to the Nintendo GameCube(tm) Instruction Booklet for further instructions.",
        "Please insert the\nTY The Tasmanian Tiger\nGame Disc.",
        "The Disc Cover is open. If you want to continue the game, please close the Disc Cover.",
        "The Game Disc could not be read. Please read the Nintendo GameCube(tm) Instruction Booklet for more information.",
        "This is not the \nTY The Tasmanian Tiger\n Game Disc. Please insert the\nTY The Tasmanian Tiger\nGame Disc."
    }
};

static int nCurrErr = -1;

void DiscErr_SetError(int error) {
    nCurrErr = error;
}

bool DiscErr_IsError(void) {
    return nCurrErr != -1;
}

void DiscErr_ClearError(void) {
    nCurrErr = -1;
}

char* DiscErr_GetErrorString(void) {
    if (nCurrErr <= -1 || nCurrErr > 5) {
        return NULL;
    }
    
    if (Translation_GetLanguage() == -1) {
        int defaultLanguage = 0;
        switch (OSGetLanguage()) {
            case OS_LANG_ENGLISH:
                defaultLanguage = LANGUAGE_ENGLISH;
                break;
            case OS_LANG_GERMAN:
                defaultLanguage = Language_German;
                break;
            case OS_LANG_FRENCH:
                defaultLanguage = Language_French;
                break;
            case OS_LANG_SPANISH:
                defaultLanguage = Language_Spanish;
                break;
            case OS_LANG_ITALIAN:
                defaultLanguage = Language_Italian;
                break;
            case OS_LANG_DUTCH:
                defaultLanguage = Language_Dutch;
                break;
        }

        return discErrStrings[defaultLanguage].errors[nCurrErr];
    }

    return discErrStrings[Translation_GetLanguage()].errors[nCurrErr];
}

void DiscErr_DrawErrorString(void) {
    DiscErr_DrawString(DiscErr_GetErrorString());
}

#define CTYPE_PUNCT  0x8
#define CTYPE_DIGIT  0x10
#define CTYPE_XDIGIT 0x20
#define CTYPE_LOWER  0x40
#define CTYPE_UPPER  0x80

void DiscErr_DrawString(char* pErrStr) {
    char buf[32];

    DEMODoneRender();
    DEMOBeforeRender();
    XFONTClearScreen(7);

    char* r31 = pErrStr;
    char* r30;
    int r29 = 14;

    while (true) {
        int slen = strlen(r31);
        if (slen > 0x1e) {
            r30 = r31 + 0x1e;
            while (((__ctype_map[*r30 & 0xff] & (CTYPE_UPPER | CTYPE_LOWER | CTYPE_DIGIT)) || 
                    (__ctype_map[*r30 & 0xff] & (CTYPE_PUNCT)) || (u8)*r30 > 0xC0) && r30 > r31) {
                r30--;
            }
        } else {
            r30 = r31 + slen;
        }

        char* f = strchr(r31, '\n');
        if (f != NULL && f < r30) {
            r30 = f;
        }

        int r26 = r30 - r31;
        strncpy(buf, r31, r26);
        buf[r26] = '\0';

        strupr(buf);

        int buflen = (uint)strlen(buf) / 2;

        XFONTPrintf(0x10 - buflen, r29, buf);

        if (r31 + slen <= r30) {
            break;
        }

        r29 += 2;
        r31 = r30 + 1;
        if (r31 == NULL || *r31 == '\0') {
            break;
        }
    }
    
    DCFlushRange(DEMOGetCurrentBuffer(), 0x84000);
}
