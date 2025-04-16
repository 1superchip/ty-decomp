#ifndef IMMEDIATE_FSM_H
#define IMMEDIATE_FSM_H

// Could this be StateMachine<T>?

/// @brief Templated State Machine Manager
/// @tparam T Actor Type
template <typename T>
struct ImmediateFSM {

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

    int prevState;
    int newState;
    int mPrevPrevState;
    State* pStates;

    // Methods from Ty 2
    // Init
    // Deinit
    // Update
    // SetState
    // GetState
    // GetLastState
    // GetStateName

    inline void SetState(int state, bool bAlways) {
        if (bAlways || (prevState != state)) {
            newState = state;
        }
    }

    // This is CallStateDeinit?
    inline void Deinit(T* pActor) {
        if (prevState != -1) {
            if (pStates[prevState].func2) {
                (pActor->*pStates[prevState].func2)();
            }
        }

        prevState = -1;
    }

    inline void UnkFunc(T* pActor, bool bPrintStateChange /* parameter in debug build */) {
        if (newState != -1) {
            if (prevState != -1 && pStates[prevState].func2) {
                (pActor->*pStates[prevState].func2)();
            }

            mPrevPrevState = prevState;
            prevState = newState;
            newState = -1;

            if (prevState != -1 && pStates[prevState].func1) {
                (pActor->*pStates[prevState].func1)();
            }
        }
        
        if (prevState != -1 && pStates[prevState].func3) {
            (pActor->*pStates[prevState].func3)();
        }
    }
};

#endif // IMMEDIATE_FSM_H
