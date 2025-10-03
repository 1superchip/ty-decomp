#ifndef STATEMACHINE_H
#define STATEMACHINE_H

template <typename T>
struct StateMachine {

    typedef void(T::*InitFunc)(void);
    typedef void(T::*DeinitFunc)(void);
    typedef void(T::*StateFunc3)(void);
    typedef void(T::*DrawFunc)(void);
    typedef void(T::*StateFunc5)(void);

    struct State {
        InitFunc    Init;
        DeinitFunc  Deinit;
        StateFunc3  func3;
        DrawFunc    Draw;
        StateFunc5  func5;
    };

    int currentState;
    int nextState;
    int lastState;
    State* mpStates;

    void Init(State* pStates, int arg2) {
        mpStates = pStates;
        currentState = -1;
        lastState = -1;
        nextState = arg2;
    }

    void Init(State* pStates, int state, T* pActor) {
        mpStates = pStates;
        currentState = state;
        lastState = -1;
        nextState = -1;

        // Init new current state
        if (mpStates[currentState].Init) {
            (pActor->*mpStates[currentState].Init)();
        }
    }

    /// @brief Calls the current state's Deinit method
    /// @param pActor 
    void Deinit(T* pActor) {
        if (currentState != -1 && mpStates[currentState].Deinit) {
            (pActor->*mpStates[currentState].Deinit)();
        }

        currentState = -1;
    }

    /// @brief Calls the current state's Draw method
    /// @param pActor 
    void DrawState(T* pActor) {
        if (currentState != -1 && mpStates[currentState].Draw) {
            (pActor->*mpStates[currentState].Draw)();
        }
    }

    /// @brief Sets the next state to state
    /// @param state Next state
    /// @param bAlways Whether or not to set the next state if state is equal to the current state
    inline void SetState(int state, bool bAlways) {
        if (bAlways || (currentState != state)) {
            nextState = state;
        }
    }

    int GetState(void) {
        return currentState;
    }

    int GetNextState(void) {
        return nextState;
    }

    int GetLastState(void) {
        return lastState;
    }

    /// @brief Updates the state machine
    /// @param pActor 
    /// @param bPrintStateChange Whether or not to print state changes
    inline void Update(T* pActor, bool bPrintStateChange /* parameter in debug build */) {
        if (nextState != -1) {
            // Deinit old current state
            if (currentState != -1 && mpStates[currentState].Deinit) {
                (pActor->*mpStates[currentState].Deinit)();
            }

            lastState = currentState;
            currentState = nextState;
            nextState = -1;

            // Init new current state
            if (currentState != -1 && mpStates[currentState].Init) {
                (pActor->*mpStates[currentState].Init)();
            }
        }
        
        if (currentState != -1 && mpStates[currentState].func3) {
            (pActor->*mpStates[currentState].func3)();
        }
    }

    inline char* GetStateName(int index) {
        return NULL;
    }
};

#endif // STATEMACHINE_H
