#ifndef SIGNPOST_H
#define SIGNPOST_H

#include "ty/props/StaticProp.h"
#include "common/MKAnimScript.h"

struct DetourData {
    MKAnimScript anim;
    char padding[0x8];
};

extern DetourData detourData;

void SignPost_LoadResources(KromeIni* pIni);
void SignPost_HideAll(void);
void SignPost_ShowAll(void);

// May not have been an enum
enum SignPostType {
    SP_TYPE_0       = 0,
    SP_TYPE_1       = 1,
    SP_DETOUR       = 2,
    SP_CONSTRUCTION = 3,
    SP_DIRECTION2   = 4,
    SP_DIRECTION    = 5,
    SP_BILLBOARD    = 6
};

struct SignPostDesc : StaticPropDescriptor {
    SignPostType type;
};

enum SignPostState {

};

struct SignPost : StaticProp {

    SignPostDesc* GetDesc(void) {
        return descr_cast<SignPostDesc*>(pDescriptor);
    }
};

struct TaSignPost : SignPost {

};

#endif // SIGNPOST_H
