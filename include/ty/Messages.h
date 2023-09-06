#ifndef MESSAGES_H
#define MESSAGES_H

#include "ty/GameObject.h"

struct EventMessage {
    union {
        GameObject* pTargetObj;
        int targetId; // this is used initially and then never used once the message has been resolved
    };
	int message;
	
	void Init(void);
	bool LoadLine(KromeIniLine*, char const*);
	void Resolve(void);
	void Send(void);
};

#endif // MESSAGES_H