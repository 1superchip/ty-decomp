
struct EventMessage {
    union {
        GameObject* pTargetObj;
        int target; // this is used initially and then never used once the message has been resolved
    };
	int message;
	
	void Init(void);
	bool LoadLine(KromeIniLine*, char const*);
	void Resolve(void);
	void Send(void);
};