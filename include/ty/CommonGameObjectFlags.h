#ifndef COMMONGAMEOBJECTFLAGS_H
#define COMMONGAMEOBJECTFLAGS_H

#include "types.h"
#include "common/KromeIni.h"

// place these in the proper header
/*struct NameFlagPair {
    char* name;
    int flag;
};*/
bool LoadLevel_LoadBool(KromeIniLine*, char*, bool*);
bool LoadLevel_LoadFlags(KromeIniLine*, char*, NameFlagPair*, int, int*);

enum CommonGameObjFlags {
    GameObjFlags_Active     = 1,
    GameObjFlags_Enabled    = 2,
    GameObjFlags_Visible    = 4,
    GameObjFlags_All        = 7
};

struct CommonGameObjFlagsComponent {
    u16 flags;
    u16 defaultFlags;

    // sets defaultFlags to flags
    // this should be called after loading the component
    void SetDefaultFlags(void) {
        defaultFlags = flags;
    }

    void Reset(void) {
        flags = defaultFlags;
    }

    bool CheckFlags(CommonGameObjFlags objFlags) {
        return flags & objFlags;
    }

    void Clear(CommonGameObjFlags objFlags) {
        flags &= ~objFlags;
    }

    void Set(CommonGameObjFlags objFlags) {
        flags |= objFlags;
    }

    void Init(CommonGameObjFlags objFlags) {
        defaultFlags = objFlags;
        flags = objFlags;
    }

    bool LoadFlag(KromeIniLine* pLine, char* str, CommonGameObjFlags objFlags) {
        bool tmp = false;
        bool levelRet = LoadLevel_LoadBool(pLine, str, &tmp);
        if (levelRet != false) {
            if (tmp) {
                Set(objFlags);
            } else {
                Clear(objFlags);
            }

            return true;
        }

        return false;
    }

    bool LoadLine(KromeIniLine* pLine) {
        return LoadFlag(pLine, "bActive", GameObjFlags_Active) || 
            LoadFlag(pLine, "bEnabled", GameObjFlags_Enabled) ||
            LoadFlag(pLine, "bVisible", GameObjFlags_Visible);
    }
    
    void Message(MKMessage* pMsg) {
        switch (pMsg->unk0) {
            case MSG_Activate:
                Set(GameObjFlags_Active);
                break;
            case MSG_Deactivate:
                Clear(GameObjFlags_Active);
                break;
            case MSG_Show:
                Set(GameObjFlags_Visible);
                break;
            case MSG_Hide:
                Clear(GameObjFlags_Visible);
                break;
            case MSG_Enable:
                Set(GameObjFlags_Enabled);
                break;
            case MSG_Disable:
                Clear(GameObjFlags_Enabled);
                break;
        }
    }
};

#endif // COMMONGAMEOBJECTFLAGS_H
