#include "types.h"
#include "ty/props/WeatherProp.h"

extern GameObjectManager objectManager;
bool LoadLevel_LoadInt(KromeIniLine*, char*, int*);
int Weather_GetType(void);
void Weather_Enable(bool);
void Weather_SetType(int);
void Weather_InitType(int);

int GameCamera_IsUnderwater(void);

int SoundBank_Play(int, Vector*, uint);
void SoundBank_Stop(int*);

static ModuleInfo<WeatherProp> module;
static GameObjDesc desc;
static WeatherProp* activeWeatherProp;

void WeatherProp_LoadResources(KromeIni* pIni) {
	desc.Init(&module, "Weather", "Weather", 1, 0);
	objectManager.AddDescriptor(&desc);
}

void WeatherProp_Init(void) {
    BeginStruct begin = desc.Begin();
    while (begin.GetPointers()) {
        WeatherProp* pProp = static_cast<WeatherProp*>(begin.GetPointers());
        Weather_InitType(pProp->type);
        begin.UpdatePointers();
    }
}

void WeatherProp::Init(GameObjDesc* pDesc) {
	GameObject::Init(pDesc);
	bEnabled = 1;
	bWater = 0;
	unk48 = 2;
	type = Weather_GetType();
	sound = -1;
}

void WeatherProp::Deinit(void) {
	Deactivate();
	GameObject::Deinit();
}

void WeatherProp::Reset(void) {
	Deactivate();
	type = unk50;
	bEnabled = unk40;
	GameObject::Reset();
}

bool WeatherProp::LoadLine(KromeIniLine* pLine) {
    if (LoadLevel_LoadInt(pLine, "bEnabled", &bEnabled) || LoadLevel_LoadInt(pLine, "bWater", &bWater)
        || LoadLevel_LoadInt(pLine, "type", &type)) {
        return true;
    }
    return GameObject::LoadLine(pLine);
}

void WeatherProp::LoadDone(void) {
	unk50 = type;
	unk40 = bEnabled;
	objectManager.AddObject(this, NULL, NULL);
}

void WeatherProp::Message(MKMessage * pMsg) {
	switch (pMsg->unk0) {
    case 2:
        Reset();
        break;
    case 10:
        if (bEnabled != false) {
            break;
        }
        bEnabled = 1;
        Activate();
        break;
    case 11:
        if (bEnabled == false) {
            break;
        }
        bEnabled = 0;
        Deactivate();
        break;
    case 26:
        SetType(1);
        break;
    case 27:
        SetType(2);
        break;
    case 28:
        SetType(3);
        break;
    case 29:
        SetType(4);
        break;
    case 30:
        SetType(5);
        break;
    case 31:
        SetType(6);
        break;
    case 0:
    case 1:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
        break;
    }
    GameObject::Message(pMsg);
}

void WeatherProp::Update(void) {
    if (GameCamera_IsUnderwater() != unk48) {
        unk48 = GameCamera_IsUnderwater();
        if (bWater == unk48) {
            Activate();
        } else {
            Deactivate();
        }
    }
}

void WeatherProp::Activate(void) {
    if (bEnabled != 0) {
        if (activeWeatherProp != NULL) {
			// deactive current active WeatherProp
            activeWeatherProp->Deactivate();
        }
        activeWeatherProp = this; // set active WeatherProp to this prop
        Weather_Enable(true);
        Weather_SetType(type);
        switch (type) {
            case 3:
                sound = SoundBank_Play(0x81, NULL, 0);
                break;
        }
    }
}

void WeatherProp::Deactivate(void) {
    if (activeWeatherProp == this) {
        Weather_Enable(false);
        SoundBank_Stop(&sound);
        activeWeatherProp = NULL;
    }
}

void WeatherProp::SetType(int newType) {
    type = newType;
    if (bEnabled != 0) {
        WeatherProp* active = activeWeatherProp;
        if (active != NULL) {
            Weather_Enable(false);
            SoundBank_Stop(&active->sound);
        }
        activeWeatherProp = this;
        Weather_Enable(true);
        Weather_SetType(type);
        switch (type) {
            case 3:
                sound = SoundBank_Play(0x81, NULL, 0);
                break;
        }
    }
}