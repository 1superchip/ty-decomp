#include "ty/Mist.h"
#include "common/Model.h"
#include "common/Material.h"

extern struct GlobalVar {
	int unk[0x558 / 4];
} gb;
Vector* GameCamera_GetPos(void);

static Model* envMist;
static Material* envMat;

void Mist_Init(void) {
    if (gb.unk[0x558 / 4] != 0) {
        if (envMist == NULL) {
            envMist = Model::Create("env_mist", NULL);
            envMist->renderType = 3;
        }
        if (envMat == NULL) {
            envMat = Material::Create("mist_01");
        }
    }
}

void Mist_Deinit(void) {
    if (envMist != NULL) {
        envMist->Destroy();
        envMist = NULL;
    }
    if (envMat != NULL) {
        envMat->Destroy();
        envMat = NULL;
    }
}

void Mist_Update(void) {
    Vector trans;
    Vector scale;
    if (gb.unk[0x558 / 4] != 0) {
        envMist->matrices[0].SetIdentity();
        trans = *GameCamera_GetPos();
        trans.y = 0.0f;
        envMist->matrices[0].SetTranslation(&trans);
        scale.Set(3.0f, 1.5f, 3.0f);
        envMist->matrices[0].Scale(&envMist->matrices[0], &scale);
    }
}

void Mist_Draw(void) {
    if (gb.unk[0x558 / 4] != 0) {
		envMist->Draw(NULL);
	}
}