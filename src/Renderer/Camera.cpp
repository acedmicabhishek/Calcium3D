#include "Camera.h"
#ifndef C3D_RUNTIME
#include "Editor.h"
#endif
#include "InputManager.h"

Camera::Camera(int width, int height, glm::vec3 position)
    : m_cameraEnabled(true) {
  Camera::width = width;
  Camera::height = height;
  Position = position;
}

void Camera::Matrix(float FOVdeg, float nearPlane, float farPlane,
                    Shader &shader, const char *uniform) {

  glm::mat4 view = glm::mat4(1.0f);
  glm::mat4 projection = glm::mat4(1.0f);

  view = glm::lookAt(Position, Position + Orientation, Up);

  projection = glm::perspective(glm::radians(FOVdeg), (float)width / height,
                                nearPlane, farPlane);

  glUniformMatrix4fv(glGetUniformLocation(shader.ID, uniform), 1, GL_FALSE,
                     glm::value_ptr(projection * view));
}

void Camera::UpdateSize(int width, int height) {
  this->width = width;
  this->height = height;
}

void Camera::Inputs(GLFWwindow *window, float deltaTime, bool forceFreeMove) {

#ifndef C3D_RUNTIME
  if (Editor::isEditMode || forceFreeMove) {

    if (InputManager::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_2)) {
      if (!m_cameraEnabled) {
        InputManager::SetCursorMode(GLFW_CURSOR_DISABLED);
        m_cameraEnabled = true;
        firstClick = true;
      }
    } else {
      if (m_cameraEnabled) {
        InputManager::SetCursorMode(GLFW_CURSOR_NORMAL);
        m_cameraEnabled = false;
        lastMouseUpdated = false;
      }
    }
  } else {

    if (m_cameraEnabled) {
      InputManager::SetCursorMode(GLFW_CURSOR_NORMAL);
      m_cameraEnabled = false;
      lastMouseUpdated = false;
    }
  }
#else
  if (!m_cameraEnabled) {
    InputManager::SetCursorMode(GLFW_CURSOR_DISABLED);
    m_cameraEnabled = true;
    firstClick = true;
  }
#endif

  if (!m_cameraEnabled) {
    return;
  }

  float currentSpeed = speed * deltaTime * 60.0f;
  float sprintMultiplier =
      (InputManager::IsKeyPressed(GLFW_KEY_LEFT_SHIFT)) ? 4.0f : 1.0f;
  float moveSpeed = currentSpeed * sprintMultiplier;

  if (InputManager::IsKeyPressed(GLFW_KEY_W)) {
    Position += moveSpeed * Orientation;
  }
  if (InputManager::IsKeyPressed(GLFW_KEY_A)) {
    Position += moveSpeed * -glm::normalize(glm::cross(Orientation, Up));
  }
  if (InputManager::IsKeyPressed(GLFW_KEY_S)) {
    Position += moveSpeed * -Orientation;
  }
  if (InputManager::IsKeyPressed(GLFW_KEY_D)) {
    Position += moveSpeed * glm::normalize(glm::cross(Orientation, Up));
  }
  if (InputManager::IsKeyPressed(GLFW_KEY_SPACE)) {
    Position += moveSpeed * Up;
  }
  if (InputManager::IsKeyPressed(GLFW_KEY_LEFT_CONTROL)) {
    Position += moveSpeed * -Up;
  }

  
  if (m_cameraEnabled) {
    static double lastX = 0, lastY = 0;
    double mouseX = InputManager::GetMouseX();
    double mouseY = InputManager::GetMouseY();

    if (firstClick || !lastMouseUpdated) {
      lastX = mouseX;
      lastY = mouseY;
      firstClick = false;
      lastMouseUpdated = true;
    }

    float deltaX = (float)(mouseX - lastX);
    float deltaY = (float)(mouseY - lastY);

    lastX = mouseX;
    lastY = mouseY;

    
    
    float rotX = deltaY * (sensitivity * 0.5f) / height;
    float rotY = deltaX * (sensitivity * 0.5f) / width;

    yaw += rotY;
    pitch -= rotX;

    if (pitch > 89.0f)
      pitch = 89.0f;
    if (pitch < -89.0f)
      pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    Orientation = glm::normalize(direction);
  }
}

glm::mat4 Camera::GetViewMatrix() {
  return glm::lookAt(Position, Position + Orientation, Up);
}

glm::mat4 Camera::GetProjectionMatrix() {
  return glm::perspective(glm::radians(FOV), (float)width / height, nearPlane,
                          farPlane);
}

glm::vec3 Camera::GetRay(GLFWwindow *window) {
  double mouseX, mouseY;
  glfwGetCursorPos(window, &mouseX, &mouseY);

  float x = (2.0f * mouseX) / width - 1.0f;
  float y = 1.0f - (2.0f * mouseY) / height;
  float z = 1.0f;
  glm::vec3 ray_nds = glm::vec3(x, y, z);

  glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);

  glm::vec4 ray_eye = inverse(GetProjectionMatrix()) * ray_clip;
  ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

  glm::vec3 ray_wor = (inverse(GetViewMatrix()) * ray_eye);
  ray_wor = normalize(ray_wor);

  return ray_wor;
}