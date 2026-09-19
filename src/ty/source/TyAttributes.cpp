#include "ty/TyAttributes.h"
#include "ty/GameObjectManager.h"

#include "ty/global.h"
#include "ty/Ty.h"

extern "C" void Sound_MusicDuckVolume(int, int, int);

static GameObjDesc attrDesc;
static ModuleInfo<TyAttributes> attrModule;

void TyAttributes_LoadResources(KromeIni* pIni) {
    attrDesc.Init(&attrModule, "TyAttributes", "TyAttributes", 1, 0);
    objectManager.AddDescriptor(&attrDesc);
}

void TyAttributes::LoadDone(void) {
    GameObject::LoadDone();
    objectManager.AddObject(this, NULL, NULL);
}

void TyAttributes::Message(MKMessage* pMsg) {
    bool bCollectedRang = false;

    switch (pMsg->unk0) {
        case MSG_LearntToSwim:
        case MSG_LearntToDive:
            gb.mGameData.SetLearntToSwim(true);
            gb.mGameData.SetLearntToDive(true);
            break;
        case MSG_GotBothRangs:
            gb.mGameData.SetBothRangs(true);
            ty.mBoomerangManager.SetHasBoth(true);
            ty.SetTwirlRangs();
            break;
        case MSG_GotAquarang:
            ty.mBoomerangManager.SetHasRang(BR_Aquarang, true);
            gb.mGameData.SetHasRang(BR_Aquarang, true);
            ty.mBoomerangManager.SetType(BR_Aquarang);
            bCollectedRang = true;
            break;
        case MSG_GotFlamerang:
            ty.mBoomerangManager.SetHasRang(BR_Flamerang, true);
            gb.mGameData.SetHasRang(BR_Flamerang, true);
            ty.mBoomerangManager.SetType(BR_Flamerang);
            ty.SetTwirlRangs();
            bCollectedRang = true;
            break;
        case MSG_GotFrostyrang:
            ty.mBoomerangManager.SetHasRang(BR_Frostyrang, true);
            gb.mGameData.SetHasRang(BR_Frostyrang, true);
            ty.mBoomerangManager.SetType(BR_Frostyrang);
            ty.SetTwirlRangs();
            bCollectedRang = true;
            break;
        case MSG_GotZappyrang:
            ty.mBoomerangManager.SetHasRang(BR_Zappyrang, true);
            gb.mGameData.SetHasRang(BR_Zappyrang, true);
            ty.mBoomerangManager.SetType(BR_Zappyrang);
            ty.SetTwirlRangs();
            bCollectedRang = true;
            break;
        case MSG_GotZoomerang:
            ty.mBoomerangManager.SetHasRang(BR_Zoomerang, true);
            gb.mGameData.SetHasRang(BR_Zoomerang, true);
            ty.mBoomerangManager.SetType(BR_Zoomerang);
            ty.SetTwirlRangs();
            bCollectedRang = true;
            break;
        case MSG_GotMultirang:
            ty.mBoomerangManager.SetHasRang(BR_Multirang, true);
            gb.mGameData.SetHasRang(BR_Multirang, true);
            ty.mBoomerangManager.SetType(BR_Multirang);
            ty.SetTwirlRangs();
            bCollectedRang = true;
            break;
        case MSG_GotInfrarang:
            ty.mBoomerangManager.SetHasRang(BR_Infrarang, true);
            gb.mGameData.SetHasRang(BR_Infrarang, true);
            ty.mBoomerangManager.SetType(BR_Infrarang);
            ty.SetTwirlRangs();
            bCollectedRang = true;
            break;
        case MSG_GotMegarang:
            ty.mBoomerangManager.SetHasRang(BR_Megarang, true);
            gb.mGameData.SetHasRang(BR_Megarang, true);
            ty.mBoomerangManager.SetType(BR_Megarang);
            ty.SetTwirlRangs();
            bCollectedRang = true;
            break;
        case MSG_GotKaboomarang:
            gb.mGameData.SetBothRangs(true);
            ty.mBoomerangManager.SetHasRang(BR_Kaboomerang, true);
            gb.mGameData.SetHasRang(BR_Kaboomerang, true);
            ty.mBoomerangManager.SetType(BR_Kaboomerang);
            ty.SetTwirlRangs();
            bCollectedRang = true;
            break;
        case MSG_GotChronorang:
            ty.mBoomerangManager.SetHasRang(BR_Chronorang, true);
            gb.mGameData.SetHasRang(BR_Chronorang, true);
            ty.mBoomerangManager.SetType(BR_Chronorang);
            ty.SetTwirlRangs();
            bCollectedRang = true;
            break;
        case MSG_GotDoomarang: {
            ty.mBoomerangManager.SetHasRang(BR_Doomerang, true);
            gb.mGameData.SetHasRang(BR_Doomerang, true);
            ty.mBoomerangManager.SetType(BR_Doomerang);
            ty.SetTwirlRangs();
            bCollectedRang = true;

            MKMessage msg = {MSG_Deactivate};

            objectManager.SendMessage(&msg, 2, NULL, 1.0f, false);

            GameObjDesc* pDesc = objectManager.FindDescriptor("EnemySpawner");

            if (pDesc) {
                DescriptorIterator it;
                it = pDesc->Begin();
                while (*it) {
                    (*it)->Message(&msg);
                    it++;
                }
            }
            break;
        }
        case MSG_GotExtraHealth:
            gb.mGameData.SetHasExtraHealth(true);
            break;
        case MSG_FallingOffGeoFluffy: {
            if (ty.TryChangeState(ty.mFsm.GetState() != TY_AS_28, TY_AS_28)) {
                ty.unk18DA = true;
            }
        }
    }

    if (bCollectedRang) {
        Sound_MusicDuckVolume(
            0, 
            SoundBank_Play(SFX_RangCollectionLP, NULL, 0), 
            15
        );
    }

    GameObject::Message(pMsg);
}
