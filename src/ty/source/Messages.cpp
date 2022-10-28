#include "types.h"
#include "ty/GameObjectManager.h"
#include "ty/Messages.h"
#include "ty/MessageMap.h"

extern "C" int stricmp(char*, char*);
bool LoadLevel_LoadInt(KromeIniLine *, char *, int *);

extern GameObjectManager objectManager;

char* globalMessageStrings[43] = {"Activate", "Deactivate", "Enable", "Disable", "Show", "Hide", "Open", "Close", 
"Toggle", "Spawn", "Shatter", "Fall", "ObjectiveIncrement", "LearntToSwim", "LearntToDive", "GotBothRangs", 
"WeatherLightning", "WeatherStormy", "WeatherRain", "WeatherSnow", "WeatherPlankton", "WeatherFoggy", "Start", 
"GotAquarang", "GotFlamerang", "GotFrostyrang", "GotSmasharang", "GotZappyrang", "GotZoomerang", "GotMultirang", "GotInfrarang", 
"GotMegarang", "GotKaboomarang", "GotChronorang", "GotDoomarang", "RequestElementalRang", "RequestTechnoRang", "RequestTalisman", 
"Abort", "GotExtraHealth", "FallingOffGeoFluffy", "TimeAttackStart", "TimeAttackEnd"};

char* mkMessageStrings[2] = {"MK_Destroy", "Reset"};

MessageMap mkMessageMap = {-2, 2, mkMessageStrings};
MessageMap globalMessageMap = {10, 43, globalMessageStrings};

void EventMessage::Init(void) {
	target = -1;
	message = 0;
}

bool EventMessage::LoadLine(KromeIniLine *pLine, char const *str) {
    static EventMessage *pCurrentEM = NULL;
    if (this == pCurrentEM) {
        if (stricmp(pLine->pFieldName, "Message") == 0) {
            char *pString;
            pLine->AsString(0, &pString);
            message = GameObject::GetMessageIdFromString(pString);
            return true;
        }
        if (LoadLevel_LoadInt(pLine, "TargetID", &target) != false) {
            return true;
        }
        pCurrentEM = NULL;
    }
    if (stricmp(pLine->pFieldName, (char *)str) == 0) {
        pCurrentEM = this;
        return true;
    }
    return false;
}

GameObject* EventMessage::Resolve(void) {
    if (target != -1) {
        GameObject* object = objectManager.GetObjectFromID(target);
        pTargetObj = object;
        return object;
    }
    target = 0;
    return (GameObject*)this; // ???
}

void EventMessage::Send(void) {
    MKMessage msg;
    msg.unk0 = message;
    if (message == 0) {
        return;
    }
    if (pTargetObj != NULL) {
        pTargetObj->Message(&msg);
    }
}
