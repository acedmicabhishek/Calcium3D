#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include "State.h"
#include "Logger.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>
#include <mutex>
#include <any>
#include <map>



enum class GameState {
    START_SCREEN = 0,
    GAMEPLAY = 1,
    EXIT = 2,
    PAUSE = 3,
    SETTINGS = 4,
    CUSTOM = 5 
};

class StateManager {
public:
    template<typename T>
    static void RegisterState(const std::string& name) {
        m_Registry[name] = []() -> std::unique_ptr<State> {
            return std::make_unique<T>();
        };
    }

    static void PushState(const std::string& name, std::any payload = {}) {
        if (m_Registry.find(name) == m_Registry.end()) {
            Logger::AddLog("[ERROR] Failed to push state: %s not registered.", name.c_str());
            return;
        }

        if (!m_StateStack.empty()) {
            m_StateStack.back()->OnPause();
        }

        m_StateStack.push_back(m_Registry[name]());
        m_StateStack.back()->SetStateName(name);
        m_StateStack.back()->Init(); 
        m_StateStack.back()->OnEnter(payload);
        m_CurrentStateName = name;
    }

    static void PopState() {
        if (m_StateStack.empty()) return;

        m_StateStack.back()->OnExit();
        m_StateStack.pop_back();

        if (!m_StateStack.empty()) {
            m_StateStack.back()->OnResume();
        } else {
            m_CurrentStateName = "";
        }
    }

    static void ChangeState(const std::string& name, std::any payload = {}) {
        if (m_Registry.find(name) == m_Registry.end()) {
            Logger::AddLog("[ERROR] Failed to change state: %s not registered.", name.c_str());
            return;
        }

        m_StackMutex.lock(); 
        while (!m_StateStack.empty()) {
            m_StateStack.at(m_StateStack.size() - 1)->OnExit();
            m_StateStack.pop_back();
        }

        m_StateStack.push_back(m_Registry[name]());
        m_StateStack.back()->SetStateName(name);
        m_StateStack.back()->Init();
        m_StateStack.back()->OnEnter(payload);
        m_CurrentStateName = name;
        
        
        for (auto const& [id, n] : m_StateNames) {
            if (n == name) {
                m_PreviousState = m_CurrentState;
                m_CurrentState = id;
                break;
            }
        }
        m_StackMutex.unlock();
        
        Logger::AddLog("[State] Changed to native logic: %s", name.c_str());
    }

    static void Update(float dt) {
        if (!m_StateStack.empty()) {
            m_StateStack.back()->Update(dt);
        }
    }

    static void Render(glm::vec2 canvasSize, glm::vec2 basePos = {0,0}) {
        if (!m_StateStack.empty()) {
            m_StateStack.back()->Render(canvasSize, basePos);
        }
    }

    static std::string GetCurrentStateName() { return m_CurrentStateName; }
    
    static std::vector<std::string> GetRegisteredStateNames() {
        std::vector<std::string> names;
        for (const auto& pair : m_Registry) {
            names.push_back(pair.first);
        }
        return names;
    }
    
    
    
    
    static void Init() {
        if (m_Initialized) return;
        m_StateNames[0] = "Start Screen";
        m_StateNames[1] = "Gameplay";
        m_StateNames[2] = "Exit";
        m_StateNames[3] = "Pause";
        m_StateNames[4] = "Settings";
        m_Initialized = true;
    }

    static void RegisterState(int id, const std::string& name) {
        m_StateNames[id] = name;
    }

    static void ChangeState(int id, std::any payload = {}) {
        if (m_StateNames.find(id) == m_StateNames.end()) {
            Logger::AddLog("[ERROR] Failed to change state: ID %d not mapped to any string logic.", id);
            return;
        }
        
        m_PreviousState = m_CurrentState;
        m_CurrentState = id;
        ChangeState(m_StateNames[id], payload);
    }
    
    static void ChangeState(GameState state, std::any payload = {}) {
        ChangeState((int)state, payload);
    }

    static int GetState() { return m_CurrentState; }
    static int GetPreviousState() { return m_PreviousState; }
    static bool IsState(int state) { return m_CurrentState == state; }
    static bool IsState(GameState state) { return IsState((int)state); }
    static std::string GetStateName(int id) {
        if (m_StateNames.find(id) != m_StateNames.end()) return m_StateNames[id];
        return "Unknown State";
    }
    static const std::map<int, std::string>& GetAllStates() { return m_StateNames; }

private:
    static std::unordered_map<std::string, std::function<std::unique_ptr<State>()>> m_Registry;
    static std::vector<std::unique_ptr<State>> m_StateStack;
    static std::string m_CurrentStateName;

    
    static int m_CurrentState;
    static int m_PreviousState;
    static std::map<int, std::string> m_StateNames;
    static bool m_Initialized;
    static std::mutex m_StackMutex;
};


inline std::unordered_map<std::string, std::function<std::unique_ptr<State>()>> StateManager::m_Registry;
inline std::vector<std::unique_ptr<State>> StateManager::m_StateStack;
inline std::string StateManager::m_CurrentStateName = "";

inline int StateManager::m_CurrentState = 0;
inline int StateManager::m_PreviousState = 0;
inline std::map<int, std::string> StateManager::m_StateNames;
inline bool StateManager::m_Initialized = false;
inline std::mutex StateManager::m_StackMutex;

using GameStateManager = StateManager;

#endif
