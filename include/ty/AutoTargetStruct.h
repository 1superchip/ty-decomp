#ifndef AUTOTARGETSTRUCT_H
#define AUTOTARGETSTRUCT_H

#include "common/Model.h"
#include "common/Vector.h"

enum TargetPriority {
    TP_1 = 1,
    TP_2 = 2,
    TP_3 = 3
};

struct AutoTargetSubStruct {
    float unk0;
    Vector unk4;
    int unk14;
};

struct AutoVisible {
    Vector* unk0[10];
    float unk28[10];
    int numItems;
    Vector unk54;
    
    void AddToList(Vector*, Model*);
};

struct AutoTargetStruct {
    AutoTargetSubStruct unk0[3];
    float unk48;
    float unk4C;
    float unk50;
    float unk54;
    float unk58;
    float unk5C;
    float unk60;
    Vector tyRot;
    AutoVisible unk74;
    AutoVisible unkD8;
    AutoVisible unk13C;
    Vector targetPos;
    Vector unk1B0;
    Vector unk1C0;
    Vector unk1D0;
    Model* unk1E0;
    Model* unk1E4; // targetedModel?
    Model* unk1E8;
    int unk1EC;
    int unk1F0;
    int targetPriority; // closestType?
    Vector unk1F8;
    int unk208;

    void Set(TargetPriority, Vector*, Vector*, Vector*, Model*);
    void SetNearestTargetEnemy(Vector*, Model*, Vector*);
    void Reset(void);

    void SetUnk208(int val) {
        unk208 = val;
    }

    Vector* GetTargetPos(void) {
        return unk1F0 ? &targetPos : NULL;
    }
};

#endif // AUTOTARGETSTRUCT_H
