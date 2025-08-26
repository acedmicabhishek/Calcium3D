#include "Gizmo.h"
#include "Shader.h"
#include "Camera.h"
#include "Logger.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>

Gizmo::Gizmo() : isDragging(false), selectedAxis(NONE), currentMode(TRANSLATE),
                  arrowLength(2.0f), arrowRadius(0.05f), handleRadius(0.5f) {
    setupGizmo();
}

Gizmo::~Gizmo() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void Gizmo::setupGizmo() {
    
    std::vector<float> vertices;
    
    
    
    vertices.insert(vertices.end(), {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f});
    vertices.insert(vertices.end(), {arrowLength, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f});
    
    
    vertices.insert(vertices.end(), {arrowLength, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f});
    vertices.insert(vertices.end(), {arrowLength - 0.3f, 0.15f, 0.0f, 1.0f, 0.0f, 0.0f});
    vertices.insert(vertices.end(), {arrowLength, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f});
    vertices.insert(vertices.end(), {arrowLength - 0.3f, -0.15f, 0.0f, 1.0f, 0.0f, 0.0f});
    
    
    vertices.insert(vertices.end(), {0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f});
    vertices.insert(vertices.end(), {0.0f, arrowLength, 0.0f, 0.0f, 1.0f, 0.0f});
    
    vertices.insert(vertices.end(), {0.0f, arrowLength, 0.0f, 0.0f, 1.0f, 0.0f});
    vertices.insert(vertices.end(), {0.15f, arrowLength - 0.3f, 0.0f, 0.0f, 1.0f, 0.0f});
    vertices.insert(vertices.end(), {0.0f, arrowLength, 0.0f, 0.0f, 1.0f, 0.0f});
    vertices.insert(vertices.end(), {-0.15f, arrowLength - 0.3f, 0.0f, 0.0f, 1.0f, 0.0f});
    
    
    vertices.insert(vertices.end(), {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f});
    vertices.insert(vertices.end(), {0.0f, 0.0f, arrowLength, 0.0f, 0.0f, 1.0f});
    
    vertices.insert(vertices.end(), {0.0f, 0.0f, arrowLength, 0.0f, 0.0f, 1.0f});
    vertices.insert(vertices.end(), {0.15f, 0.0f, arrowLength - 0.3f, 0.0f, 0.0f, 1.0f});
    vertices.insert(vertices.end(), {0.0f, 0.0f, arrowLength, 0.0f, 0.0f, 1.0f});
    vertices.insert(vertices.end(), {-0.15f, 0.0f, arrowLength - 0.3f, 0.0f, 0.0f, 1.0f});

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Gizmo::Draw(Shader& shader, Camera& camera, const glm::vec3& position) {
    shader.use();
    
    
    float distance = glm::length(camera.Position - position);
    float gizmoScale = distance * 0.1f;
    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, glm::vec3(gizmoScale));
    
    
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = camera.GetProjectionMatrix();
    
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    
    
    glLineWidth(5.0f);
    
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, 18);
    glBindVertexArray(0);
    
    
    glLineWidth(1.0f);
}

void Gizmo::HandleMouse(GLFWwindow* window, Camera& camera, const glm::vec3& objectPosition, 
                        glm::vec3& newPosition, glm::vec3& newRotation, glm::vec3& newScale) {
    switch (currentMode) {
        case TRANSLATE:
            handleTranslate(window, camera, objectPosition, newPosition);
            break;
        case ROTATE:
            handleRotate(window, camera, objectPosition, newRotation);
            break;
        case SCALE:
            handleScale(window, camera, objectPosition, newScale);
            break;
    }
}

void Gizmo::handleTranslate(GLFWwindow* window, Camera& camera, const glm::vec3& objectPosition, glm::vec3& newPosition) {
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !isDragging) {
        isDragging = true;
        objectStartPos = objectPosition;
        dragStartMousePos = glm::vec2(mouseX, mouseY);
        
        
        glm::vec3 rayOrigin = camera.Position;
        glm::vec3 rayDirection = camera.GetRay(window);
        
        float minDist = 1.0f;
        
        
        glm::vec3 xEnd = objectPosition + glm::vec3(arrowLength, 0.0f, 0.0f);
        float xDist = glm::distance(rayOrigin + rayDirection * glm::dot(xEnd - rayOrigin, rayDirection), xEnd);
        if (xDist < handleRadius) {
            selectedAxis = X;
            minDist = xDist;
            Logger::AddLog("Gizmo: X axis selected via mouse, distance: %.3f", xDist);
        }
        
        
        glm::vec3 yEnd = objectPosition + glm::vec3(0.0f, arrowLength, 0.0f);
        float yDist = glm::distance(rayOrigin + rayDirection * glm::dot(yEnd - rayOrigin, rayDirection), yEnd);
        if (yDist < handleRadius && (selectedAxis == NONE || yDist < minDist)) {
            selectedAxis = Y;
            minDist = yDist;
            Logger::AddLog("Gizmo: Y axis selected via mouse, distance: %.3f", yDist);
        }
        
        
        glm::vec3 zEnd = objectPosition + glm::vec3(0.0f, 0.0f, arrowLength);
        float zDist = glm::distance(rayOrigin + rayDirection * glm::dot(zEnd - rayOrigin, rayDirection), zEnd);
        if (zDist < handleRadius && (selectedAxis == NONE || zDist < minDist)) {
            selectedAxis = Z;
            minDist = zDist;
            Logger::AddLog("Gizmo: Z axis selected via mouse, distance: %.3f", zDist);
        }
        
        
        if (selectedAxis == NONE) {
            selectedAxis = X; 
            Logger::AddLog("Gizmo: No axis hit, defaulting to X");
        }
        
        Logger::AddLog("Gizmo: Starting drag on axis %d", selectedAxis);
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
        isDragging = false;
        Logger::AddLog("Gizmo: Mouse released, stopping drag");
        
        
        
        newPosition = objectPosition;
    }
    
    if (isDragging && selectedAxis != NONE) {
        
        glm::vec2 mouseDelta = glm::vec2(mouseX, mouseY) - dragStartMousePos;
        float sensitivity = 0.01f;
        
        if (glm::length(mouseDelta) > 0.1f) {
            newPosition = objectPosition;
            
            
            glm::vec3 cameraRight = glm::normalize(glm::cross(camera.Orientation, camera.Up));
            glm::vec3 cameraUp = camera.Up;
            glm::vec3 cameraForward = camera.Orientation;
            
            newPosition += cameraRight * mouseDelta.x * sensitivity;      
            newPosition += cameraUp * (-mouseDelta.y * sensitivity);      
            newPosition += cameraForward * mouseDelta.x * sensitivity * 0.5f; 
            
            
            dragStartMousePos = glm::vec2(mouseX, mouseY);
        } else {
            
            newPosition = objectPosition;
        }
    } else {
        
        newPosition = objectPosition;
    }
}

void Gizmo::handleRotate(GLFWwindow* window, Camera& camera, const glm::vec3& objectPosition, glm::vec3& newRotation) {
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !isDragging) {
        isDragging = true;
        dragStartMousePos = glm::vec2(mouseX, mouseY);
        
        
        glm::vec3 rayOrigin = camera.Position;
        glm::vec3 rayDirection = camera.GetRay(window);
        
        float minDist = 1.0f;
        selectedAxis = NONE;
    
        
        glm::vec3 xRingCenter = objectPosition;
        glm::vec3 xRingNormal = glm::vec3(1.0f, 0.0f, 0.0f);
        float xDist = glm::abs(glm::dot(rayDirection, xRingNormal));
        if (xDist < minDist) { 
            selectedAxis = X; 
            minDist = xDist; 
            Logger::AddLog("Gizmo Rotate: X axis selected, dot: %.3f", xDist);
        }
        
        
        glm::vec3 yRingNormal = glm::vec3(0.0f, 1.0f, 0.0f);
        float yDist = glm::abs(glm::dot(rayDirection, yRingNormal));
        if (yDist < minDist) { 
            selectedAxis = Y; 
            minDist = yDist; 
            Logger::AddLog("Gizmo Rotate: Y axis selected, dot: %.3f", yDist);
        }
        
        
        glm::vec3 zRingNormal = glm::vec3(0.0f, 0.0f, 1.0f);
        float zDist = glm::abs(glm::dot(rayDirection, zRingNormal));
        if (zDist < minDist) { 
            selectedAxis = Z; 
            minDist = zDist; 
            Logger::AddLog("Gizmo Rotate: Z axis selected, dot: %.3f", zDist);
        }
        
        if (selectedAxis != NONE) {
            Logger::AddLog("Gizmo Rotate: Starting drag on axis %d", selectedAxis);
        }
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
        isDragging = false;
        Logger::AddLog("Gizmo Rotate: Mouse released, stopping rotation");
        newRotation = glm::vec3(0.0f);
    }
    
    if (isDragging && selectedAxis != NONE) {
        glm::vec2 mouseDelta = glm::vec2(mouseX, mouseY) - dragStartMousePos;
        float sensitivity = 0.1f;
        
        if (glm::length(mouseDelta) > 0.1f) {
            newRotation = glm::vec3(0.0f);
            switch (selectedAxis) {
                case X:
                    newRotation.x = mouseDelta.y * sensitivity;
                    break;
                case Y:
                    newRotation.y = mouseDelta.x * sensitivity;
                    break;
                case Z:
                    newRotation.z = mouseDelta.x * sensitivity;
                    break;
            }
            
            
            newRotation.x = glm::clamp(newRotation.x, -45.0f, 45.0f);
            newRotation.y = glm::clamp(newRotation.y, -90.0f, 90.0f);
            newRotation.z = glm::clamp(newRotation.z, -45.0f, 45.0f);
            
            
            dragStartMousePos = glm::vec2(mouseX, mouseY);
        } else {
            
            newRotation = glm::vec3(0.0f);
        }
    } else {
        
        newRotation = glm::vec3(0.0f);
    }
}

void Gizmo::handleScale(GLFWwindow* window, Camera& camera, const glm::vec3& objectPosition, glm::vec3& newScale) {
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !isDragging) {
        isDragging = true;
        dragStartMousePos = glm::vec2(mouseX, mouseY);
        
        
        glm::vec3 rayOrigin = camera.Position;
        glm::vec3 rayDirection = camera.GetRay(window);
        
        
        float minDist = 1.0f;
        selectedAxis = NONE;
    
        
        glm::vec3 xHandle = objectPosition + glm::vec3(arrowLength, 0.0f, 0.0f);
        float xDist = glm::distance(rayOrigin + rayDirection * glm::dot(xHandle - rayOrigin, rayDirection), xHandle);
        if (xDist < minDist) { 
            selectedAxis = X; 
            minDist = xDist; 
            Logger::AddLog("Gizmo Scale: X axis selected, distance: %.3f", xDist);
        }
        
        
        glm::vec3 yHandle = objectPosition + glm::vec3(0.0f, arrowLength, 0.0f);
        float yDist = glm::distance(rayOrigin + rayDirection * glm::dot(yHandle - rayOrigin, rayDirection), yHandle);
        if (yDist < minDist) { 
            selectedAxis = Y; 
            minDist = yDist; 
            Logger::AddLog("Gizmo Scale: Y axis selected, distance: %.3f", yDist);
        }
        
        
        glm::vec3 zHandle = objectPosition + glm::vec3(0.0f, 0.0f, arrowLength);
        float zDist = glm::distance(rayOrigin + rayDirection * glm::dot(zHandle - rayOrigin, rayDirection), zHandle);
        if (zDist < minDist) { 
            selectedAxis = Z; 
            minDist = zDist; 
            Logger::AddLog("Gizmo Scale: Z axis selected, distance: %.3f", zDist);
        }
        
        if (selectedAxis != NONE) {
            Logger::AddLog("Gizmo Scale: Starting drag on axis %d", selectedAxis);
        }
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
        isDragging = false;
        Logger::AddLog("Gizmo Scale: Mouse released, stopping scaling");
        
        
        
        newScale = glm::vec3(0.0f);
    }
    
    if (isDragging && selectedAxis != NONE) {
        
        glm::vec2 mouseDelta = glm::vec2(mouseX, mouseY) - dragStartMousePos;
        float sensitivity = 0.005f; 
        
        
        if (glm::length(mouseDelta) > 0.1f) {
            
            newScale = glm::vec3(0.0f); 
            
            
            float scaleDelta = (mouseDelta.x + mouseDelta.y) * sensitivity;
            
            switch (selectedAxis) {
                case X:
                    
                    newScale.x = scaleDelta;
                    newScale.y = scaleDelta * 0.5f; 
                    newScale.z = scaleDelta * 0.5f; 
                    break;
                case Y:
                    
                    newScale.x = scaleDelta * 0.5f; 
                    newScale.y = scaleDelta;
                    newScale.z = scaleDelta * 0.5f; 
                    break;
                case Z:
                    
                    newScale.x = scaleDelta * 0.5f; 
                    newScale.y = scaleDelta * 0.5f; 
                    newScale.z = scaleDelta;
                    break;
            }
            
            
            dragStartMousePos = glm::vec2(mouseX, mouseY);
        } else {
            
            newScale = glm::vec3(0.0f);
        }
    } else {
        
        newScale = glm::vec3(0.0f);
    }
}

bool Gizmo::rayIntersectsAxis(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, 
                              const glm::vec3& axisStart, const glm::vec3& axisEnd, 
                              float axisRadius, float& t) {
    
    glm::vec3 axisDir = glm::normalize(axisEnd - axisStart);
    float axisLength = glm::length(axisEnd - axisStart);
    
    
    glm::vec3 w0 = rayOrigin - axisStart;
    float a = glm::dot(rayDirection, rayDirection);
    float b = glm::dot(rayDirection, axisDir);
    float c = glm::dot(axisDir, axisDir);
    float d = glm::dot(rayDirection, w0);
    float e = glm::dot(axisDir, w0);
    
    float denominator = a * c - b * b;
    if (abs(denominator) < 1e-6) return false;
    
    float s = (b * e - c * d) / denominator;
    float t_axis = (a * e - b * d) / denominator;
    
    
    if (t_axis < 0.0f || t_axis > axisLength) return false;
    
    
    glm::vec3 closestPoint = axisStart + t_axis * axisDir;
    
    
    float dist = glm::length(rayOrigin + s * rayDirection - closestPoint);
    
    if (dist <= axisRadius && s > 0.0f) {
        t = s;
        return true;
    }
    
    return false;
}