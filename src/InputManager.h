#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <unordered_map>

class InputManager {
public:
    static void Init(GLFWwindow* window);
    static void Update();

    static bool IsKeyPressed(int key);
    static bool IsKeyJustPressed(int key);
    static bool IsMouseButtonPressed(int button);
    
    static glm::vec2 GetMousePosition();
    static float GetMouseX();
    static float GetMouseY();
    
    
    static void SetCursorMode(int mode); 
    static int GetCursorMode();

private:
    static GLFWwindow* m_Window;
    static std::unordered_map<int, bool> m_KeyStates;
    static std::unordered_map<int, bool> m_KeyJustPressed;
    
    static double m_MouseX;
    static double m_MouseY;
};

#endif
