#ifndef TYATTRIBUTES_H
#define TYATTRIBUTES_H

#include "ty/GameObject.h"

void TyAttributes_LoadResources(KromeIni* pIni);

struct TyAttributes : GameObject {
    virtual void LoadDone(void);
    virtual void Message(MKMessage* pMsg);
};

#endif // TYATTRIBUTES_H
