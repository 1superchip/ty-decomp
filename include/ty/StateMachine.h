#ifndef STATEMACHINE_H
#define STATEMACHINE_H

template <typename T>
struct StateMachine {

    typedef void(T::*StateFunc1)(void);
    typedef void(T::*StateFunc2)(void);
    typedef void(T::*StateFunc3)(void);
    typedef void(T::*StateFunc4)(void);
    typedef void(T::*StateFunc5)(void);

    struct State {
        StateFunc1  func1;
        StateFunc2  func2; // Deinit?
        StateFunc3  func3;
        StateFunc4  func4;
        StateFunc5  func5;
    };

    int unk0;
    int unk4;
    int unk8;
    State* mpStates;

    void Init(State* pStates, int arg2) {
        mpStates = pStates;
        unk0 = -1;
        unk8 = -1;
        unk4 = arg2;
    }

    void Deinit(T* pActor) {
        if (unk0 != -1 && mpStates[unk0].func2) {
            (pActor->*mpStates[unk0].func2)();
        }

        unk0 = -1;
    }

    inline void SetState(int state, bool bAlways) {
        if (bAlways || (unk0 != state)) {
            unk4 = state;
        }
    }

    int GetState(void) {
        return unk0;
    }

    int GetNextState(void) {
        return unk4;
    }

    int GetLastState(void) {
        return unk8;
    }

    inline void UnkFunc(T* pActor, bool bPrintStateChange /* parameter in debug build */) {
        if (unk4 != -1) {
            if (unk0 != -1 && mpStates[unk0].func2) {
                (pActor->*mpStates[unk0].func2)();
            }

            unk8 = unk0;
            unk0 = unk4;
            unk4 = -1;

            if (unk0 != -1 && mpStates[unk0].func1) {
                (pActor->*mpStates[unk0].func1)();
            }
        }
        
        if (unk0 != -1 && mpStates[unk0].func3) {
            (pActor->*mpStates[unk0].func3)();
        }
    }

    inline char* GetStateName(int index) {
        return NULL;
    }

};

#endif // STATEMACHINE_H
