#ifndef GAMEENUMS_H
#define GAMEENUMS_H

// File for commonly used enums

enum LevelNumber {
    LN_First            = 0, 

    LN_RAINBOW_CLIFFS   = 0,  // z1
    LN_1                = 1,  // z2
    LN_2                = 2,  // z3
    LN_3                = 3,  // z4

    LN_TWO_UP           = 4,  // a1
    LN_WALK_IN_THE_PARK = 5,  // a2
    LN_SHIP_REX         = 6,  // a3
    LN_BULLS_PEN        = 7,  // a4

    LN_BRIDGE_RIVER_TY  = 8,  // b1
    LN_SNOW_WORRIES     = 9,  // b2
    LN_OUTBACK_SAFARI   = 10, // b3
    LN_11               = 11, // b4

    LN_LYRE_LYRE        = 12, // c1
    LN_BLACK_STUMP      = 13, // c2
    LN_REX_MARKS_SPOT   = 14, // c3
    LN_FLUFFYS_FJORD    = 15, // c4

    LN_16               = 16, // d1
    LN_CASS_CREST       = 17, // d2
    LN_18               = 18, // d3
    LN_CRIKEYS_COVE     = 19, // d4

    LN_CASS_PASS        = 20, // e1
    LN_BONUS_WORLD_DAY  = 21, // e2
    LN_BONUS_WORLD_NIGHT= 22, // e3
    LN_FINAL_BATTLE     = 23, // e4

    TOTAL_LEVEL_MAX     = 24,
};

enum ZoneNumber {
    ZN_0        = 0,
    ZN_1        = 1,
    ZN_2        = 2,
    ZN_3        = 3,
    ZN_4        = 4,
    ZN_5        = 5,
    TOTAL_ZONES = 6
};

enum TalismanType {
    TALISMAN_TYPE_0 = 0,
    TALISMAN_TYPE_1 = 1,
    TALISMAN_TYPE_2 = 2,
    TALISMAN_TYPE_3 = 3,
    TALISMAN_TYPE_4 = 4,
    TALISMAN_TYPE_5 = 5
};

enum ElementType {
    ELEMENT_FIRE        = 0,
    ELEMENT_ICE         = 1,
    ELEMENT_AIR         = 2,
    ELEMENT_RAINBOW     = 3,
    ELEMENT_EARTH       = 4,
    ELEMENT_MAX         = 5,
};

enum ThunderEggType {
    ThunderEggType_0 = 0,
    ThunderEggType_1 = 1,
    TE_Max = 8,
};

enum GoldenCogType {
    GC_Max = 10,
};

#endif // GAMEENUMS_H
