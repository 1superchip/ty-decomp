#include "ty/props/Picture.h"
#include "ty/GameObjectManager.h"
#include "ty/global.h"

extern void SoundBank_Play(int, Vector*, uint);
extern View* GameCamera_View(void);
extern GameObject* pHero;

static StaticPropDescriptor pictureDesc;
static ModuleInfo<Picture> pictureModule;

void Picture_LoadResources(KromeIni* pIni) {
    pictureDesc.Init(&pictureModule, "Prop_0590_PickupFrame", "PictureFrame", 1, 1);
    pictureDesc.Load(pIni);
    objectManager.AddDescriptor(&pictureDesc);
}

/// @brief Creates a new Picture object
/// @param pPos Position of the object
/// @return New Picture object
Picture* Picture::Create(Vector* pPos) {
    Picture* pPic = (Picture*)Heap_MemAlloc(sizeof(Picture));
    pictureDesc.pModule->ConstructObject(pPic);
    pPic->Init(&pictureDesc);
    pPic->pModel->matrices[0].SetIdentity();
    pPic->pModel->matrices[0].SetTranslation(pPos);
    objectManager.AddObject(pPic, pPic->pModel);
    pPic->unk80 = false;
    pPic->unk68 = pPos->y;
    pPic->mQuadratic.Init();
    pPic->mFrameNumber = -1;
    pPic->pos = *pPic->GetPos();
    pPic->Reset();
    return pPic;
}

// should this be static?
void Picture::Destroy(void) {
    objectManager.RemoveObject(this);
    Deinit();
    Heap_MemFree(this);
}

void Picture::Init(GameObjDesc* pDesc) {
    StaticProp::Init(pDesc);
    bShow = false;
    bInitialised = true;
    unk80 = true;
    unk83 = false;
    mQuadraticTime = -1.0f;
    mFrameNumber = -1;
    unk74 = 0.0f;
    angle = RandomFR(&gb.mRandSeed, 0.0f, 2.0f * PI);
}

bool Picture::LoadLine(KromeIniLine* pLine) {
    return StaticProp::LoadLine(pLine) || LoadLevel_LoadInt(pLine, "FrameNumber", &mFrameNumber);
}

void Picture::LoadDone(void) {
    StaticProp::LoadDone();
    if (mFrameNumber >= 0) {
        bShow = gb.mGameData.GetHasGalleryImage(mFrameNumber);
    }
    unk68 = StaticProp::GetPos()->y;
    pos = *StaticProp::GetPos();
}

void Picture::Reset(void) {
    GameObject::Reset();
    pModel->matrices[0].SetIdentity();
    pModel->matrices[0].SetTranslation(&pos);
    if (mFrameNumber >= 0) {
        bShow = gb.mGameData.GetHasGalleryImage(mFrameNumber);
    }
    bInitialised = unk80;
    unk83 = false;
    mQuadraticTime = -1.0f;
    unk74 = 0.0f;
    angle = RandomFR(&gb.mRandSeed, 0.0f, 2 * PI);
}

void Picture::Update(void) {
    if (!bInitialised) return; // if not initialised, don't update
    if (bShow) {
        if (pModel->matrices[0].Row0()->MagSquared() > 0.01f) {
            pModel->matrices[0].Row3()->x = pHero->pLocalToWorld->Row3()->x;
            pModel->matrices[0].Row3()->z = pHero->pLocalToWorld->Row3()->z;
            pModel->matrices[0].Scale(0.9f);
        } else {
            pModel->matrices[0].Scale(0.0f);
        }
        return;
    }
    Vector temp;
    Vector heroPos = *pHero->GetPos();
    heroPos.y += 50.0f;
    temp.Sub(&heroPos, GetPos());
    float distToHero = temp.MagSquared();
    if (distToHero < 60000.0f || unk83) {
        unk83 = true;
        if (distToHero < 5000.0f) {
            bShow = true;
            if (mFrameNumber > -1) {
                gb.mGameData.SetHasGalleryImage(mFrameNumber);
                SoundBank_Play(0xB7, NULL, 0);
            }
            return;
        }
        unk74 += temp.Normalise() * 0.01f * 0.15f;
        temp.Scale(unk74);
        GetPos()->Add(&temp);
        if (unk74 > 50.0f) {
            bShow = true;
            if (mFrameNumber > -1) {
                gb.mGameData.SetHasGalleryImage(mFrameNumber);
                SoundBank_Play(0xB7, NULL, 0);
            }
            return;
        }
    } else {
        angle += 0.081f;
        if (angle >= 2.0f * PI) {
            angle -= 2.0f * PI;
        }
        GetPos()->y = _table_sinf(angle) * 5.0f + unk68;
    }
    // Update Quadratic equation
    if (mQuadraticTime >= 0.0f && mQuadraticTime < 1.0f) {
        if (mQuadraticTime < 1.0f) {
            mQuadraticTime += unk70;
        } else {
            mQuadraticTime = 1.0f;
        }
        mQuadratic.Update(mQuadraticTime);
        GetPos()->Copy(&mQuadratic.pos);
    }
    // Copy GameCamera rotation so it always faces the camera / screen
    pModel->matrices[0].CopyRotation(&GameCamera_View()->unk48);
}

void Draw_AddPostDrawElement(void*, void (*drawFunc)(void*), float, bool);

void Picture::Draw(void) {
    if (!bInitialised || (bShow && pModel->matrices[0].Row0()->MagSquared() < 0.01f)) {
        return;
    }
    Draw_AddPostDrawElement((void*)this, Picture::PostDraw, distSquared, GetDrawFlag());
}

void Picture::PostDraw(void* pObj) {
    Picture* self = static_cast<Picture*>(pObj);
    self->pModel->Draw(NULL);
}

/// @brief Initialises the Picture object
/// @param frameNumber Frame Number for the save data
/// @param pPos Position of Object
void Picture::Spawn(int frameNumber, Vector* pPos) {
    CollisionResult cr;
    if (bInitialised) return;
    mFrameNumber = frameNumber;
    GetPos()->Copy(pPos);
    Vector mid = *GetPos();
    Vector end = mid;
    mid.y += 50.0f;
    end.y -= 2000.0f;
    if (Collision_RayCollide(&mid, &end, &cr, COLLISION_MODE_ALL, 0)) {
        end.y = cr.pos.y + 25.0f;
        unk68 = cr.pos.y + 25.0f;
    }
    mQuadratic.SetPoints(GetPos(), &mid, &end);
    bInitialised = true;
    mQuadraticTime = 0.0f;
    unk70 = Max<float>(200.0f / (GetPos()->y - end.y), 1.0f) * gDisplay.updateFreq;
}
