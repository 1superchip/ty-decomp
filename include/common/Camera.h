#ifndef COMMON_CAMERA
#define COMMON_CAMERA

struct KeyMap {
    int unk0;
    char unk4;
    char unk5;
    char unk6[7];
    char unkD;
    int unk10;
    char unk14;
    char unk15;
    char unk16[7];
    char unk1D;
    char unk1E[7];
    char unk25;
    int unk28;
    char unk2C;
    char unk2D;
    char unk2E[7];
    char unk35;
    char unk36[7];
    char unk3D;
    int unk40;
    char unk44;
    char unk45;
    int unk48;
    char unk4C;
    char unk4D;
    char unk4E[7];
    char unk55;
    char unk56[7];
    char unk5D;
    int unk60;
    char unk64;
    char unk65;
    char unk66[7];
    char unk6D;
    char unk6E[7];
    char unk75;
    int unk78;
    char unk7C;
    char unk7D;
    char unk7E[7];
    char unk85;
    char unk86[7];
    char unk8D;
    int unk90;
    char unk94;
    char unk95;
    char unk96[7];
    char unk9D;
    char unk9E[7];
    char unkA5;
    int unkA8;
    char unkAC;
    char unkAD;
    char unkAE;
    char unkAF;
};

struct Camera {
    static void InitModule(void);
    static void DeinitModule(void);
    void Init(void);
    void Deinit(void);
    
    int unk0;
    int unk4;
    KeyMap* pMap;
    float unkC;
    float unk10;
    float unk14;
    float unk18;
    float unk1C;
    float unk20;
    float unk24;
    float unk28;
    float unk2C;
    float unk30;
    float unk34;
    float unk38;
    float unk3C;
    float unk40;
    float unk44;
    float unk48;
    float unk4C;
    
    static KeyMap keyMapDS;
};

#endif // COMMON_CAMERA
