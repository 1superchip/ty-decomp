#ifndef BEZIER_PATH_FOLLOWER_H
#define BEZIER_PATH_FOLLOWER_H

#include "ty/Spline.h"

struct BezierPathFollower {
    bool unk0;
    Spline mSpline;
    float unk14;
    int unk18;
    float unk1C;
    float unk20;
    bool unk24;
    float unk28;
    float unk2C;
    float unk30;
    float unk34;
    float unk38;
    Vector unk3C;
    Vector unk4C;

    void Init(Vector*, int, float, float, bool);
    void Reset(void);
    void Deinit(void);

    void DrawPath(float);
    void DrawVelocities(float);

    Vector GetNewPosition(Vector*, Vector*);
    bool AtLastPoint(void);
    void GetStartPositionAndRotation(Vector*, Vector*);
    Vector GetPreviousNode(void);
    Vector GetNextNode(void);
    float GetPositionAlongPath(Vector*);
    void MatchVelocityTo(Vector*, Vector*);
    Vector GetOnPathPosition(void);
};

#endif // BEZIER_PATH_FOLLOWER_H
