#include "ty/BoomerangManager.h"
#include "ty/global.h"
#include "ty/Ty.h"

StateMachine<BoomerangWeapon>::State boomerangLauncherStates[3] = {
    {
        // BLS_Idle
        NULL, 
        NULL, 
        &BoomerangWeapon::Idle
    },
    {
        // BLS_Firing
        &BoomerangWeapon::InitFiring, 
        &BoomerangWeapon::DeinitFiring, 
        &BoomerangWeapon::Firing
    },
    {
        // BLS_Catching
        &BoomerangWeapon::InitCatching, 
        &BoomerangWeapon::DeinitCatching, 
        &BoomerangWeapon::Catching
    }
};

void BoomerangWeapon::Init(BoomerangSide side, BoomerangDesc* pDesc, BoomerangManager* _pManager) {
    mSide = side;
    pManager = _pManager;
    pBoomerangDesc = pDesc;

    unkA4 = false;
    unkA5 = false;

    pBoomerang = NULL;

    mpRangs = (Boomerang*)Heap_MemAlloc(
        pBoomerangDesc->ammoCount * pBoomerangDesc->pModule->GetInstanceSize()
    );

    unkA7 = true;

    ammoQueue.Init(pBoomerangDesc->ammoCount);

    DescriptorIterator it = BeginAmmo();
    while (*it) {
        // Construct the boomerang object
        pBoomerangDesc->ConstructObject(
            (void*)static_cast<Boomerang*>(*it)
        );
        
        static_cast<Boomerang*>(*it)->Init(pBoomerangDesc, this);
        static_cast<Boomerang*>(*it)->unk56 = pManager->init.unkC;

        Boomerang** pNextRangPtr = ammoQueue.Add();
        Boomerang* pRang = static_cast<Boomerang*>(*it);
        *pNextRangPtr = pRang;

        it++;
    }

    if (!ammoQueue.IsEmpty()) {
        (*ammoQueue.GetCurr())->Load();
    }

    mFsm.Init(boomerangLauncherStates, 0);
}

void BoomerangWeapon::Deinit(void) {
    mFsm.Deinit(this);

    DescriptorIterator it = BeginAmmo();

    while (*it) {
        static_cast<Boomerang*>(*it)->Deinit();

        it++;
    }

    if (mpRangs) {
        Heap_MemFree(mpRangs);
    }

    mpRangs = NULL;

    ammoQueue.Deinit();
}

void BoomerangWeapon::Reset(void) {
    unkA7 = true;
    pBoomerang = NULL;

    mFsm.Init(boomerangLauncherStates, 0);

    ammoQueue.Reset();

    DescriptorIterator it = BeginAmmo();
    while (*it) {
        static_cast<Boomerang*>(*it)->Reset();

        Boomerang** pNextRangPtr = ammoQueue.Add();
        Boomerang* pRang = static_cast<Boomerang*>(*it);
        *pNextRangPtr = pRang;

        it++;
    }

    if (!ammoQueue.IsEmpty()) {
        (*ammoQueue.GetCurr())->Load();
    }
}

bool BoomerangWeapon::Fire(Vector* p, Vector* p1) {
    if (IsReady()) {
        unkA6 = false;

        pos.SetZero();
        if (p1) {
            pos = *p1;
            unkA6 = true;
        }

        direction = *p;

        mFsm.SetState(BLS_Firing, false);
        mFsm.UnkFunc(this, false);

        return true;
    } else {
        return false;
    }
}

void BoomerangWeapon::Draw(void) {
    if (!ammoQueue.IsEmpty()) {
        (*ammoQueue.GetCurr())->Draw();
    }
}

void BoomerangWeapon::DrawShadow(Vector* p) {
    if (!ammoQueue.IsEmpty()) {
        (*ammoQueue.GetCurr())->DrawShadow(p);
    }
}

void BoomerangWeapon::DrawReflection(void) {
    if (!ammoQueue.IsEmpty()) {
        (*ammoQueue.GetCurr())->DrawReflection();
    }
}

void BoomerangWeapon::Idle(void) {

}

void BoomerangWeapon::InitFiring(void) {
    if (pManager->init.pAnimScript) {
        pManager->StartThrowAnim(mSide);
        unkA4 = false;
    } else {
        unkA4 = true;
    }
}

void BoomerangWeapon::Firing(void) {
    if (unkA4) {
        mFsm.SetState(BLS_Idle, false);
    }
}

void BoomerangWeapon::DeinitFiring(void) {
    unkA8 = 12.0f;

    if (pManager->unkC8 == mSide) {
        pManager->unkC8 = BOOMERANG_SIDE_COUNT;
    }

    pManager->unk59 = false;

    DoFire();
}

void BoomerangWeapon::InitCatching(void) {
    if (pManager->init.pAnimScript) {
        unkA5 = false;

        pManager->StartCatchAnim(mSide);
    } else {
        unkA5 = true;
    }
}

void BoomerangWeapon::Catching(void) {
    if (unkA5 && pBoomerang) {
        DoCatch(pBoomerang);

        pBoomerang = NULL;

        if (pManager->unkC8 == mSide) {
            pManager->unkC8 = BOOMERANG_SIDE_COUNT;
        }
    }

    if (pManager->unkC[mSide].Condition()) {
        mFsm.SetState(BLS_Idle, false);
    }
}

void BoomerangWeapon::DeinitCatching(void) {
    if (pManager->unkC8 == mSide) {
        pManager->unkC8 = BOOMERANG_SIDE_COUNT;
    }

    if (pBoomerang) {
        DoCatch(pBoomerang);

        pBoomerang = NULL;
    }
}

void BoomerangWeapon::DoFire(void) {
    // ASSERT(
    //     !ammoQueue.IsEmpty(),
    //     "BoomerangWeapon::DoFire No rang to fire!",
    //     "Source\\boomerangManager.cpp",
    //     436
    // );

    if (!ammoQueue.IsEmpty()) {
        pManager->numFired++;

        Boomerang* pRang = *ammoQueue.GetCurr();

        ammoQueue.UnkInline();

        pRang->Fire(&direction, unkA6 ? &pos : NULL);

        if (!ammoQueue.IsEmpty() && unkA7) {
            (*ammoQueue.GetCurr())->Load();
        }
    }
}

void BoomerangWeapon::DoCatch(Boomerang* pRang) {
    // ASSERT(
    //     pRang,
    //     "BoomerangWeapon::DoCatch: pRang NULL!",
    //     "Source\\boomerangManager.cpp",
    //     461
    // );

    pManager->numFired--;

    // ASSERT(
    //     pManager->numFired >= 0,
    //     "BoomerangWeapon::DoCatch: fire count out of sync!!",
    //     "Source\\boomerangManager.cpp",
    //     465
    // );

    pRang->Deactivate();

    *ammoQueue.Add() = pRang;

    if (pRang == *ammoQueue.GetCurr() && unkA7) {
        pRang->Load();
    }
}

DescriptorIterator BoomerangWeapon::BeginAmmo(void) {
    DescriptorIterator it;

    it.pCurr = (u8*)mpRangs;
    it.pEnd = (u8*)mpRangs + (
        pBoomerangDesc->ammoCount * pBoomerangDesc->pModule->GetInstanceSize()
    );
    
    return it;
}

void BoomerangWeapon::StartCatch(Boomerang* pRang) {
    if (mFsm.GetState() == BLS_Idle) {
        pBoomerang = pRang;
        mFsm.SetState(BLS_Catching, false);
        mFsm.UnkFunc(this, false);
    } else {
        DoCatch(pRang);
    }
}

bool BoomerangWeapon::IsOwnRang(Boomerang* pRang) {
    DescriptorIterator it = BeginAmmo();

    return (u8*)pRang >= it.pCurr && (u8*)pRang < it.pEnd;
}

void BoomerangWeapon::Update(Matrix* r4, Vector* r5) {
    catchMatrix = *r4;
    colour = *r5;

    DescriptorIterator it = BeginAmmo();

    while (*it) {
        ((Boomerang*)*it)->Update();

        it++;
    }

    mFsm.UnkFunc(this, false);
}

void BoomerangWeapon::Enable(void) {
    if (!ammoQueue.IsEmpty() && (*ammoQueue.GetCurr())->unk6C == BOOMERANG_STATE_0) {
        (*ammoQueue.GetCurr())->Load();
    }

    unkA7 = true;
}

void BoomerangWeapon::Disable(void) {
    if (!ammoQueue.IsEmpty() && (*ammoQueue.GetCurr())->unk6C != BOOMERANG_STATE_0) {
        (*ammoQueue.GetCurr())->Deactivate();
    }

    unkA7 = false;
}

void BoomerangWeapon::EnableParticles(void) {
    DescriptorIterator it = BeginAmmo();

    while (*it) {
        ((Boomerang*)*it)->bParticlesEnabled = true;

        it++;
    }
}

void BoomerangWeapon::DisableParticles(void) {
    DescriptorIterator it = BeginAmmo();

    while (*it) {
        ((Boomerang*)*it)->bParticlesEnabled = false;

        it++;
    }
}

void BoomerangWeapon::DisableSounds(void) {
    DescriptorIterator it = BeginAmmo();

    while (*it) {
        ((Boomerang*)*it)->StopSounds();

        it++;
    }
}

void BoomerangManager::Init(BoomerangManagerInit* pInitInfo) {
    init = *pInitInfo;

    mType = init.defaultType;

    if (init.pAnimScript) {
        mAnimEventDesc.Init(init.animEvents, 5, init.pAnimScript);
        mAnimEventManager.Init(&mAnimEventDesc);
    }

    for (int i = 0; i < BOOMERANG_SIDE_COUNT; i++) {
        mpWeapons[i] = (BoomerangWeapon*)Heap_MemAlloc(init.maxTypes * sizeof(BoomerangWeapon));
        unkC[i].pTemplate = NULL;

        if (init.pAnimScript) {
            unkC[i].Init(pInitInfo->pAnimScript);
        }

        for (int j = 0; j < init.maxTypes; j++) {
            mpWeapons[i][j].Init((BoomerangSide)i, pInitInfo->ppBoomerangDescs[j], this);
        }
    }

    Reset();
}

void BoomerangManager::Deinit(void) {
    for (int i = 0; i < BOOMERANG_SIDE_COUNT; i++) {
        if (unkC[i].pTemplate) {
            unkC[i].Deinit();
        }

        for (int j = 0; j < init.maxTypes; j++) {
            mpWeapons[i][j].Deinit();
        }

        Heap_MemFree(mpWeapons[i]);
    }
}

void BoomerangManager::Reset(void) {
    unkC8 = BOOMERANG_SIDE_COUNT;

    numFired = 0;

    bEnabled = true;

    unk59 = false;

    mNextType = mType;

    mCurrentSide = BOOMERANG_SIDE_LEFT;

    // Right boomerang is unlocked by default
    bShowRangs[BOOMERANG_SIDE_RIGHT] = true;
    bShowRangs[BOOMERANG_SIDE_LEFT]  = gb.mGameData.HasBothRangs();

    unk4C[BOOMERANG_SIDE_RIGHT] = init.unk18[BOOMERANG_SIDE_RIGHT];
    unk4C[BOOMERANG_SIDE_LEFT]  = init.unk18[BOOMERANG_SIDE_LEFT];
    
    for (int i = 0; i < BOOMERANG_SIDE_COUNT; i++) {
        for (int j = 0; j < init.maxTypes; j++) {
            mpWeapons[i][j].Reset();
        }
    }
}

void BoomerangManager::Update(Matrix* r4, Matrix* r5, Vector* r6) {
    if (&ty.mBoomerangManager == this && ty.mFsm.BiteState()) {
        unk4C[0] = init.unk18[0];
        unk4C[1] = init.unk18[1];
    }

    // If the next type isn't the same as the current type, numFired = 0, 
    // and both boomerang weapons aren't in the firing state
    // switch to the next type
    if (mNextType != mType && numFired == 0) {
        if (mpWeapons[BOOMERANG_SIDE_LEFT][mType].mFsm.GetState() != BLS_Firing &&
            mpWeapons[BOOMERANG_SIDE_RIGHT][mType].mFsm.GetState() != BLS_Firing) {
            
            for (int i = 0; i < BOOMERANG_SIDE_COUNT; i++) {
                mpWeapons[i][mType].Reset();
            }

            mType = mNextType;
            numFired = 0;
            unkC8 = BOOMERANG_SIDE_COUNT;

            if (mType != BR_Aquarang && mType != BR_Doomerang) {
                // Do not set the current boomerang in save data to 
                // Aquarang or Doomerang
                gb.mGameData.SetCurrentRang(mType);
            }

            for (int i = 0; i < BOOMERANG_SIDE_COUNT; i++) {
                mpWeapons[i][mType].Reset();
                if (!bShowRangs[i]) {
                    mpWeapons[i][mType].Disable();
                }
            }
        }
    }

    mpWeapons[BOOMERANG_SIDE_LEFT][mType].Update(r4, r6);
    mpWeapons[BOOMERANG_SIDE_RIGHT][mType].Update(r5, r6);
}

void BoomerangManager::Draw(void) {
    if (gb.mGameData.HasBoomerang(GetCurrentType()) && bEnabled) {
        for (int i = 0; i < BOOMERANG_SIDE_COUNT; i++) {
            if (bShowRangs[i]) {
                mpWeapons[i][mType].Draw();
            }
        }
    }
}

void BoomerangManager::DrawShadow(Vector* p) {
    if (bEnabled && gb.mGameData.HasBoomerang(mType)) {
        for (int i = 0; i < BOOMERANG_SIDE_COUNT; i++) {
            if (bShowRangs[i]) {
                mpWeapons[i][mType].DrawShadow(p);
            }
        }
    }

    Doomerang* pDoomerang = Boomerang_GetInAirDoomerang();
    if (pDoomerang) {
        Vector doomerangLightPos = *pDoomerang->pModel->matrices[0].Row3();
        doomerangLightPos.y += 1000.0f;

        Vector add = *View::GetCurrent()->unk48.Row2();
        add.Scale(-1000.0f);
        doomerangLightPos.Add(&add);

        pDoomerang->DrawShadow(&doomerangLightPos);
    }
}

void BoomerangManager::DrawReflection(void) {
    if (bEnabled && gb.mGameData.HasBoomerang(mType)) {
        for (int i = 0; i < BOOMERANG_SIDE_COUNT; i++) {
            if (bShowRangs[i]) {
                mpWeapons[i][mType].DrawReflection();
            }
        }
    }
}

bool BoomerangWeapon::IsReady(void) {
    return unkA7 && (mFsm.GetState() == BLS_Idle || (mFsm.GetState() == BLS_Catching && !pBoomerang)) && 
        (!ammoQueue.IsEmpty() && (pManager->unkC8 == BOOMERANG_SIDE_COUNT || pManager->unk59));
}

/// @brief 
/// @param p 
/// @param p1 
/// @param bFireBoth Whether or not both boomerangs should be fired
/// @return 
bool BoomerangManager::Fire(Vector* p, Vector* p1, bool bFireBoth) {
    bool ret = false;

    if (mType == mNextType && bEnabled && gb.mGameData.HasBoomerang(mType)) {
        if (bFireBoth && AreBothReady()) {
            // if both boomerangs need to be fired and both are ready
            // attempt to fire both
            unk59 = bFireBoth;
            ret = mpWeapons[BOOMERANG_SIDE_LEFT][mType].Fire(p, p1) &&
                mpWeapons[BOOMERANG_SIDE_RIGHT][mType].Fire(p, p1);
        } else if (gb.mGameData.HasBothRangs()) {
            // if both rangs are unlocked, we need to fire the next side
            if (mpWeapons[(mCurrentSide + 1) % BOOMERANG_SIDE_COUNT][mType].IsReady()) {
                // if the next side is ready, then fire it
                ret = mpWeapons[(mCurrentSide + 1) % BOOMERANG_SIDE_COUNT][mType].Fire(p, p1);
                mCurrentSide = (BoomerangSide)((mCurrentSide + 1) % BOOMERANG_SIDE_COUNT);
            } else if (mpWeapons[mCurrentSide][mType].IsReady()) {
                // if the next side isn't ready but the current side is
                // fire the current side
                ret = mpWeapons[mCurrentSide][mType].Fire(p, p1);
            }
        } else {
            // If both rangs aren't unlocked, then fire the right side
            ret = mpWeapons[BOOMERANG_SIDE_RIGHT][mType].Fire(p, p1);
        }
    }

    return ret;
}

void BoomerangManager::StartThrowAnim(BoomerangSide side) {
    if (unk59) {
        unkC8 = BOOMERANG_SIDE_LEFT;
        unk4C[side] = init.unk18[side];
        unk4C[BOOMERANG_SIDE_LEFT] = init.pAnim;
        unkC[side].SetAnim(init.mAnims.unk4);
    } else {
        if (unkC8 == 2) {
            unkC8 = side;
            int nextSide = (side + 1) % BOOMERANG_SIDE_COUNT;
            if (unk4C[nextSide] == init.pAnim) {
                unk4C[nextSide] = init.unk18[nextSide];
            }
        }

        unk4C[side] = side == unkC8 ? init.pAnim : init.unk18[side];

        unkC[side].SetAnim(init.mAnims.unk8[side]);
    }
}

void BoomerangManager::StartCatchAnim(BoomerangSide side) {
    bool r6 = false;
    if (unkC8 == BOOMERANG_SIDE_COUNT && init.mAnims.unk0) {
        r6 = true;
        int newIndex = (side + 1) % BOOMERANG_SIDE_COUNT;
        if (unk4C[newIndex] == init.pAnim) {
            unk4C[newIndex] = init.unk18[newIndex];
        }
    }

    unk4C[side] = r6 ? init.pAnim : init.unk18[side];

    unkC[side].SetAnim(init.mAnims.unk10[side]);
}

bool BoomerangManager::IsOwnRang(Boomerang* pRang) {
    for (int i = 0; i < BOOMERANG_SIDE_COUNT; i++) {
        if (mpWeapons[i][mType].IsOwnRang(pRang)) {
            return true;
        }
    }

    return false;
}

void BoomerangManager::UpdateAnimation(Model* pModel) {

    int r31 = unk4C[0] != init.pAnim;
    
    for (int i = 0; i < BOOMERANG_SIDE_COUNT; i++) {
        int index = (r31 + i) % BOOMERANG_SIDE_COUNT;

        if (!unkC[index].Condition()) {
            unkC[index].ApplyNode(pModel->pAnimation, (int)unk4C[index]);
        }

        unkC[index].Animate();

        mAnimEventManager.Update(&unkC[index]);

        if (mAnimEventManager.CheckUnk4Flags(index)) {
            mpWeapons[index][mType].unkA4 = true;
        }

        if (mAnimEventManager.CheckUnk4Flags(index + 2)) {
            mpWeapons[index][mType].unkA5 = true;
        }

        if (mAnimEventManager.CheckUnk4Flags(4) && unk59) {
            mpWeapons[BOOMERANG_SIDE_LEFT][mType].unkA4 = true;
            mpWeapons[BOOMERANG_SIDE_RIGHT][mType].unkA4 = true;
        }
    }
}

void BoomerangManager::SetType(BoomerangType newType) {
    // ASSERT(
    //     newType >= BR_Standard && newType < (BoomerangType)init.maxTypes,
    //     "new boomerang type outside of available rang range",
    //     "Source\\boomerangManager.cpp",
    //     957
    // );

    mNextType = newType;
}

void BoomerangManager::SetHasBoth(bool arg1) {
    if (arg1) {
        Show(BOOMERANG_SIDE_LEFT);
    } else {
        Hide(BOOMERANG_SIDE_LEFT);
    }
}

bool BoomerangManager::HasFired(void) {
    for (int i = 0; i < BOOMERANG_SIDE_COUNT; i++) {
        if (numFired > 0 || mpWeapons[i][mType].mFsm.GetState() != BLS_Idle) {
            return true;
        }
    }

    return false;
}

void BoomerangManager::SetHasRang(BoomerangType rangType, bool arg2) {
    // ASSERT(
    //     rangType < BR_Max,
    //     "BoomerangManager::SetHasRang: Out of range!",
    //     "Source\\boomerangManager.cpp",
    //     1008
    // );
}


bool BoomerangManager::IsReady(BoomerangSide side) {
    return bEnabled && gb.mGameData.HasBoomerang(mType) && mpWeapons[side][mType].IsReady();
}

void BoomerangManager::Show(BoomerangSide side) {
    bShowRangs[side] = true;
    bShowRangs[BOOMERANG_SIDE_LEFT] = gb.mGameData.HasBothRangs();

    if (bShowRangs[side]) {
        mpWeapons[side][mType].Enable();
    }
}

void BoomerangManager::Hide(BoomerangSide side) {
    bShowRangs[side] = false;

    if (!bShowRangs[side]) {
        mpWeapons[side][mType].Disable();
    }
}

void BoomerangManager::Disable(void) {
    bEnabled = false;
}

void BoomerangManager::Enable(void) {
    bEnabled = true;
}

void BoomerangManager::SetAnims(BoomerangManagerAnims* pAnims) {
    init.mAnims = *pAnims;
}

/// @brief Enables particles for all boomerangs. Only use if the manager has 12 types.
void BoomerangManager::EnableBoomerangParticles(void) {
    // This assumes there are 12 boomerang weapons
    // but there may not be

    for (int i = 0; i < BR_Max; i++) { // Should be using init.maxTypes
        mpWeapons[BOOMERANG_SIDE_LEFT][i].EnableParticles();
        mpWeapons[BOOMERANG_SIDE_RIGHT][i].EnableParticles();
    }
}

/// @brief Disables particles for all boomerangs. Only use if the manager has 12 types.
void BoomerangManager::DisableBoomerangParticles(void) {
    // This assumes there are 12 boomerang weapons
    // but there may not be

    for (int i = 0; i < BR_Max; i++) { // Should be using init.maxTypes
        mpWeapons[BOOMERANG_SIDE_LEFT][i].DisableParticles();
        mpWeapons[BOOMERANG_SIDE_RIGHT][i].DisableParticles();
    }
}

/// @brief Disables sounds for the currently selected boomerangs
void BoomerangManager::DisableBoomerangSounds(void) {
    mpWeapons[BOOMERANG_SIDE_LEFT][mType].DisableSounds();
    mpWeapons[BOOMERANG_SIDE_RIGHT][mType].DisableSounds();
}
