
struct EventMessage {
    union {
        GameObject* pTargetObj;
        int target;
    };
	int message;
	
	void Init(void);
	bool LoadLine(KromeIniLine*, char const*);
	GameObject* Resolve(void);
	void Send(void);
};