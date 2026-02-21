#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <memory>
#include <vector>

#include <map>
#include <string>

enum class GameState {
    START_SCREEN = 0,
    GAMEPLAY = 1,
    EXIT = 2,
    PAUSE = 3,
    SETTINGS = 4,
    CUSTOM = 5 
};

class GameStateManager {
public:
    static void Init() {
        if (m_Initialized) return;
        m_StateNames[0] = "Start Screen";
        m_StateNames[1] = "Gameplay";
        m_StateNames[2] = "Exit";
        m_StateNames[3] = "Pause";
        m_StateNames[4] = "Settings";
        m_Initialized = true;
    }

    static void SetState(int newState) {
        m_PreviousState = m_CurrentState;
        m_CurrentState = newState;
    }

    static void SetState(GameState state) { SetState((int)state); }

    static int GetState() { return m_CurrentState; }
    static int GetPreviousState() { return m_PreviousState; }

    static bool IsState(int state) { return m_CurrentState == state; }
    static bool IsState(GameState state) { return IsState((int)state); }

    static void RegisterState(int id, const std::string& name) {
        m_StateNames[id] = name;
    }

    static std::string GetStateName(int id) {
        if (m_StateNames.find(id) != m_StateNames.end()) return m_StateNames[id];
        return "Unknown State";
    }

    static const std::map<int, std::string>& GetAllStates() { return m_StateNames; }

private:
    static int m_CurrentState;
    static int m_PreviousState;
    static std::map<int, std::string> m_StateNames;
    static bool m_Initialized;
};


inline int GameStateManager::m_CurrentState = 0;
inline int GameStateManager::m_PreviousState = 0;
inline std::map<int, std::string> GameStateManager::m_StateNames;
inline bool GameStateManager::m_Initialized = false;

#endif
