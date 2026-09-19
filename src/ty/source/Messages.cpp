#include "types.h"
#include "ty/GameObjectManager.h"
#include "ty/Messages.h"
#include "ty/MessageMap.h"

extern "C" int stricmp(char*, char*);

#define GLOBAL_MESSAGE_COUNT (43)

char* globalMessageStrings[GLOBAL_MESSAGE_COUNT] = {
    "Activate", "Deactivate", // Activate = 10, Deactivate = 11
    "Enable", "Disable", // Enable = 11, Disable = 12
    "Show", "Hide", 
    "Open", "Close", 
    "Toggle", "Spawn",
    "Shatter", "Fall", 
    "ObjectiveIncrement", 
    "LearntToSwim", "LearntToDive", 
    "GotBothRangs", // Message ID: 25 (Index 15 here + 10)
    "WeatherLightning", "WeatherStormy", "WeatherRain", "WeatherSnow", "WeatherPlankton", "WeatherFoggy", // Starting Message ID: 26
    "Start", // Start = 32

    // Starting Message ID: 33
    "GotAquarang", "GotFlamerang", "GotFrostyrang", "GotSmasharang",
    "GotZappyrang", "GotZoomerang", "GotMultirang", "GotInfrarang", "GotMegarang",
    "GotKaboomarang", "GotChronorang", "GotDoomarang", 
    "RequestElementalRang", "RequestTechnoRang", "RequestTalisman", 
    "Abort", 
    "GotExtraHealth", // GotExtraHealth = 49
    "FallingOffGeoFluffy", // FallingOffGeoFluffy = 50
    "TimeAttackStart", "TimeAttackEnd"
};

char* mkMessageStrings[2] = {"MK_Destroy", "Reset"};

MessageMap mkMessageMap = {-2, 2, mkMessageStrings};
MessageMap globalMessageMap = {10, GLOBAL_MESSAGE_COUNT, globalMessageStrings};

void EventMessage::Init(void) {
    targetId = -1;
    message = 0;
}

bool EventMessage::LoadLine(KromeIniLine* pLine, const char* str) {
    static EventMessage *pCurrentEM = NULL;
    if (this == pCurrentEM) {
        if (stricmp(pLine->pFieldName, "Message") == 0) {
            char *pString;
            pLine->AsString(0, &pString);
            message = GameObject::GetMessageIdFromString(pString);
            return true;
        }

        if (LoadLevel_LoadInt(pLine, "TargetID", &targetId) != false) {
            return true;
        }

        pCurrentEM = NULL;
    }

    if (stricmp(pLine->pFieldName, (char*)str) == 0) {
        pCurrentEM = this;
        return true;
    }

    return false;
}

void EventMessage::Resolve(void) {
    if (targetId != -1) {
        pTargetObj = objectManager.GetObjectFromID(targetId);
    } else {
        pTargetObj = NULL;
    }
}

void EventMessage::Send(void) {
    MKMessage msg;
    msg.unk0 = message;
    if (message != 0 && pTargetObj != NULL) {
        pTargetObj->Message(&msg);
    }
}
