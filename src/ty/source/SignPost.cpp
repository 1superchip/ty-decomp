#include "ty/SignPost.h"
#include "ty/GameObjectManager.h"

DetourData detourData;

static SignPostDesc signPostDesc;
static SignPostDesc smallPostDesc;
static SignPostDesc crocSignDesc;
static SignPostDesc detourDesc;
static SignPostDesc constructionDesc;
static SignPostDesc directionDesc;
static SignPostDesc direction2Desc;
static SignPostDesc billboardDesc;

static ModuleInfo<TaSignPost> taSignPostModule;
static ModuleInfo<SignPost> signPostModule;

static bool bIsHidden = false;

void SignPost_LoadResources(KromeIni* pIni) {
    signPostDesc.Init(&signPostModule, "prop_0005_signpost", "SignPost", 1, 0);
    signPostDesc.Load(pIni);
    signPostDesc.type = SP_TYPE_0;
    signPostDesc.bDynamic = true;

    smallPostDesc.Init(&taSignPostModule, "Prop_0393_SignPost", "TASignPost", 1, 0);
    smallPostDesc.Load(pIni);
    smallPostDesc.type = SP_TYPE_1;

    crocSignDesc.Init(&signPostModule, "prop_0172_croc_sign", "CrocSign", 1, 0);
    crocSignDesc.Load(pIni);
    crocSignDesc.type = SP_TYPE_1;
    crocSignDesc.bDynamic = true;

    detourDesc.Init(&signPostModule, "prop_0014_detour_sign", "DetourSign", 1, 0);
    detourDesc.Load(pIni);
    detourDesc.type = SP_DETOUR;
    detourDesc.bDynamic = true;

    constructionDesc.Init(&signPostModule, "prop_0080_construct_sign", "ConstructionSign", 1, 0);
    constructionDesc.Load(pIni);
    constructionDesc.type = SP_CONSTRUCTION;
    constructionDesc.bDynamic = true;

    directionDesc.Init(&signPostModule, "prop_0124_arrow_01", "DirectionArrow", 1, 0);
    directionDesc.Load(pIni);
    directionDesc.type = SP_DIRECTION;
    directionDesc.bDynamic = true;

    direction2Desc.Init(&signPostModule, "prop_0400_arrow02", "DirectionArrow2", 1, 0);
    direction2Desc.Load(pIni);
    direction2Desc.type = SP_DIRECTION2;
    direction2Desc.bDynamic = false;

    billboardDesc.Init(&signPostModule, "prop_0581_billboard", "Billboard", 1, 0);
    billboardDesc.Load(pIni);
    billboardDesc.type = SP_BILLBOARD;
    billboardDesc.bDynamic = false;

    objectManager.AddDescriptor(&signPostDesc);
    objectManager.AddDescriptor(&smallPostDesc);
    objectManager.AddDescriptor(&crocSignDesc);
    objectManager.AddDescriptor(&detourDesc);
    objectManager.AddDescriptor(&constructionDesc);
    objectManager.AddDescriptor(&directionDesc);
    objectManager.AddDescriptor(&direction2Desc);
    objectManager.AddDescriptor(&billboardDesc);

    detourData.anim.Init("prop_0014_detour_sign");
}

void SignPost_HideAll(void) {
    bIsHidden = true;
}

void SignPost_ShowAll(void) {
    bIsHidden = false;
}
