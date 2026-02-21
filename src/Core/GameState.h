#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <memory>
#include <vector>

enum class GameState {
    START_SCREEN = 0,
    GAMEPLAY = 1,
    EXIT = 2,
    PAUSE = 3,
    SETTINGS = 4
};

class GameStateManager {
public:
    static void SetState(GameState newState) {
        m_PreviousState = m_CurrentState;
        m_CurrentState = newState;
    }

    static GameState GetState() { return m_CurrentState; }
    static GameState GetPreviousState() { return m_PreviousState; }

    static bool IsState(GameState state) { return m_CurrentState == state; }

private:
    static GameState m_CurrentState;
    static GameState m_PreviousState;
};


inline GameState GameStateManager::m_CurrentState = GameState::START_SCREEN;
inline GameState GameStateManager::m_PreviousState = GameState::START_SCREEN;

#endif
