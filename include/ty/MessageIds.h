#ifndef MESSAGEIDS_H
#define MESSAGEIDS_H

enum MESSAGE_ID {
    MSG_Resolve = 1,

    MSG_UNK_2 = 2, // Start? Think this may be a GOMSG, GOMSG_Start?

    // Platform attachment related messages
    MSG_UpdateAttachment = 3,
    MSG_AttachObject = 4,
    MSG_DetachObject = 5,

    MSG_UNK_6 = 6,
    MSG_UNK_7 = 7,

    MSG_BoomerangMsg = 8,
    MSG_ExplosionMsg = 9,

    // GOMSG_Last = 10,

    // Activate/Deactivate message ids
    MSG_Activate = 10,
    MSG_Deactivate = 11,

    // Enable/Disable message ids
    MSG_Enable = 12,
    MSG_Disable = 13,

    // Visibility message ids
    MSG_Show = 14,
    MSG_Hide = 15,

    MSG_UNK_16 = 16,
    MSG_UNK_17 = 17,
    MSG_UNK_18 = 18,
    MSG_UNK_19 = 19,

    MSG_UNK_20 = 20,

    MSG_ObjectiveIncrement = 22,

    MSG_GotBothRangs = 25,
    
    // Weather?
    MSG_UNK_26 = 26,
    MSG_UNK_27 = 27,
    MSG_UNK_28 = 28,
    MSG_UNK_29 = 29,
    MSG_UNK_30 = 30,
    MSG_UNK_31 = 31,

    MSG_Abort = 48,
};

#endif // MESSAGEIDS_H
