#include "InputManager.h"

GLFWwindow* InputManager::m_Window = nullptr;
std::unordered_map<int, bool> InputManager::m_KeyStates;
std::unordered_map<int, bool> InputManager::m_KeyJustPressed;
double InputManager::m_MouseX = 0.0;
double InputManager::m_MouseY = 0.0;

void InputManager::Init(GLFWwindow* window) {
    m_Window = window;
}

void InputManager::Update() {
    if (!m_Window) return;

    
    m_KeyJustPressed.clear();
    
    glfwGetCursorPos(m_Window, &m_MouseX, &m_MouseY);
}

bool InputManager::IsKeyPressed(int key) {
    if (!m_Window) return false;
    return glfwGetKey(m_Window, key) == GLFW_PRESS;
}

bool InputManager::IsKeyJustPressed(int key) {
    bool current = glfwGetKey(m_Window, key) == GLFW_PRESS;
    bool previous = m_KeyStates[key];
    m_KeyStates[key] = current;
    
    return current && !previous;
}

bool InputManager::IsMouseButtonPressed(int button) {
    if (!m_Window) return false;
    return glfwGetMouseButton(m_Window, button) == GLFW_PRESS;
}

glm::vec2 InputManager::GetMousePosition() {
    return glm::vec2((float)m_MouseX, (float)m_MouseY);
}

float InputManager::GetMouseX() {
    return (float)m_MouseX;
}

float InputManager::GetMouseY() {
    return (float)m_MouseY;
}

void InputManager::SetCursorMode(int mode) {
    if (!m_Window) return;
    glfwSetInputMode(m_Window, GLFW_CURSOR, mode);
}

int InputManager::GetCursorMode() {
    if (!m_Window) return 0;
    return glfwGetInputMode(m_Window, GLFW_CURSOR);
}
