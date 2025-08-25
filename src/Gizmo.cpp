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
    // Create simple arrow shapes using lines for better performance
    std::vector<float> vertices;
    
    // X axis arrow (red) - make it thicker
    // Arrow shaft
    vertices.insert(vertices.end(), {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f});
    vertices.insert(vertices.end(), {arrowLength, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f});
    
    // Arrow head - make it larger and more visible
    vertices.insert(vertices.end(), {arrowLength, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f});
    vertices.insert(vertices.end(), {arrowLength - 0.3f, 0.15f, 0.0f, 1.0f, 0.0f, 0.0f});
    vertices.insert(vertices.end(), {arrowLength, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f});
    vertices.insert(vertices.end(), {arrowLength - 0.3f, -0.15f, 0.0f, 1.0f, 0.0f, 0.0f});
    
    // Y axis arrow (green) - make it thicker
    vertices.insert(vertices.end(), {0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f});
    vertices.insert(vertices.end(), {0.0f, arrowLength, 0.0f, 0.0f, 1.0f, 0.0f});
    
    vertices.insert(vertices.end(), {0.0f, arrowLength, 0.0f, 0.0f, 1.0f, 0.0f});
    vertices.insert(vertices.end(), {0.15f, arrowLength - 0.3f, 0.0f, 0.0f, 1.0f, 0.0f});
    vertices.insert(vertices.end(), {0.0f, arrowLength, 0.0f, 0.0f, 1.0f, 0.0f});
    vertices.insert(vertices.end(), {-0.15f, arrowLength - 0.3f, 0.0f, 0.0f, 1.0f, 0.0f});
    
    // Z axis arrow (blue) - make it thicker
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

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Gizmo::Draw(Shader& shader, Camera& camera, const glm::vec3& position) {
    shader.use();
    
    // Calculate gizmo scale based on distance from camera
    float distance = glm::length(camera.Position - position);
    float gizmoScale = distance * 0.1f; // Scale gizmo with distance
    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, glm::vec3(gizmoScale));
    
    // Get view and projection matrices from camera
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = camera.GetProjectionMatrix();
    
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    
    // Make lines thicker for better visibility
    glLineWidth(5.0f); // Increased from 3.0f to 5.0f for better visibility
    
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, 18); // 18 vertices for the arrows
    glBindVertexArray(0);
    
    // Reset line width
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
        
        // Get ray from camera
        glm::vec3 rayOrigin = camera.Position;
        glm::vec3 rayDirection = camera.GetRay(window);
        
        // Check which axis is closest with more generous hit detection
        float minDist = 1.0f; // Increased hit radius for easier selection
        
        // Check X axis with simple distance-based hit detection
        glm::vec3 xEnd = objectPosition + glm::vec3(arrowLength, 0.0f, 0.0f);
        float xDist = glm::distance(rayOrigin + rayDirection * glm::dot(xEnd - rayOrigin, rayDirection), xEnd);
        if (xDist < handleRadius) {
            selectedAxis = X;
            minDist = xDist;
            Logger::AddLog("Gizmo: X axis selected via mouse, distance: %.3f", xDist);
        }
        
        // Check Y axis
        glm::vec3 yEnd = objectPosition + glm::vec3(0.0f, arrowLength, 0.0f);
        float yDist = glm::distance(rayOrigin + rayDirection * glm::dot(yEnd - rayOrigin, rayDirection), yEnd);
        if (yDist < handleRadius && (selectedAxis == NONE || yDist < minDist)) {
            selectedAxis = Y;
            minDist = yDist;
            Logger::AddLog("Gizmo: Y axis selected via mouse, distance: %.3f", yDist);
        }
        
        // Check Z axis
        glm::vec3 zEnd = objectPosition + glm::vec3(0.0f, 0.0f, arrowLength);
        float zDist = glm::distance(rayOrigin + rayDirection * glm::dot(zEnd - rayOrigin, rayDirection), zEnd);
        if (zDist < handleRadius && (selectedAxis == NONE || zDist < minDist)) {
            selectedAxis = Z;
            minDist = zDist;
            Logger::AddLog("Gizmo: Z axis selected via mouse, distance: %.3f", zDist);
        }
        
        // If no specific axis was hit, default to X
        if (selectedAxis == NONE) {
            selectedAxis = X; // Default to X for free movement
            Logger::AddLog("Gizmo: No axis hit, defaulting to X");
        }
        
        Logger::AddLog("Gizmo: Starting drag on axis %d", selectedAxis);
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
        isDragging = false;
        Logger::AddLog("Gizmo: Mouse released, stopping drag");
        // Don't reset selectedAxis - keep it selected for next operation
        // selectedAxis = NONE; // REMOVED: Keep axis selected
        // Keep the current position when mouse is released
        newPosition = objectPosition;
    }
    
    if (isDragging && selectedAxis != NONE) {
        // Calculate movement based on mouse delta
        glm::vec2 mouseDelta = glm::vec2(mouseX, mouseY) - dragStartMousePos;
        float sensitivity = 0.01f;
        
        // Only apply movement if there's actual mouse movement
        if (glm::length(mouseDelta) > 0.1f) {
            // Start from the current object position and add movement
            newPosition = objectPosition;
            
            // Implement FREE 3D movement based on camera view
            glm::vec3 cameraRight = glm::normalize(glm::cross(camera.Orientation, camera.Up));
            glm::vec3 cameraUp = camera.Up;
            glm::vec3 cameraForward = camera.Orientation;
            
            // FREE MOVEMENT: Allow movement in all directions based on mouse movement
            // Horizontal mouse movement affects X and Z (forward/right)
            // Vertical mouse movement affects Y (up/down)
            newPosition += cameraRight * mouseDelta.x * sensitivity;      // X movement
            newPosition += cameraUp * (-mouseDelta.y * sensitivity);      // Y movement (inverted for natural feel)
            newPosition += cameraForward * mouseDelta.x * sensitivity * 0.5f; // Z movement (forward/back)
            
            // Update drag start position to prevent accumulation
            dragStartMousePos = glm::vec2(mouseX, mouseY);
        } else {
            // No mouse movement, keep current position
            newPosition = objectPosition;
        }
    } else {
        // Not dragging, keep current position
        newPosition = objectPosition;
    }
}

void Gizmo::handleRotate(GLFWwindow* window, Camera& camera, const glm::vec3& objectPosition, glm::vec3& newRotation) {
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !isDragging) {
        isDragging = true;
        dragStartMousePos = glm::vec2(mouseX, mouseY);
        
        // Get ray from camera
        glm::vec3 rayOrigin = camera.Position;
        glm::vec3 rayDirection = camera.GetRay(window);
        
        // Check which rotation ring is closest
        float minDist = 1.0f; // Increased for easier selection
        selectedAxis = NONE;
    
        // Check X rotation ring (YZ plane)
        glm::vec3 xRingCenter = objectPosition;
        glm::vec3 xRingNormal = glm::vec3(1.0f, 0.0f, 0.0f);
        float xDist = glm::abs(glm::dot(rayDirection, xRingNormal));
        if (xDist < minDist) { 
            selectedAxis = X; 
            minDist = xDist; 
            Logger::AddLog("Gizmo Rotate: X axis selected, dot: %.3f", xDist);
        }
        
        // Check Y rotation ring (XZ plane)
        glm::vec3 yRingNormal = glm::vec3(0.0f, 1.0f, 0.0f);
        float yDist = glm::abs(glm::dot(rayDirection, yRingNormal));
        if (yDist < minDist) { 
            selectedAxis = Y; 
            minDist = yDist; 
            Logger::AddLog("Gizmo Rotate: Y axis selected, dot: %.3f", yDist);
        }
        
        // Check Z rotation ring (XY plane)
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
        // Don't reset selectedAxis - keep it selected for next operation
        // selectedAxis = NONE; // REMOVED: Keep axis selected
        // Reset rotation when mouse is released to prevent continued rotation
        newRotation = glm::vec3(0.0f);
    }
    
    if (isDragging && selectedAxis != NONE) {
        // Calculate rotation based on mouse movement with increased sensitivity
        glm::vec2 mouseDelta = glm::vec2(mouseX, mouseY) - dragStartMousePos;
        float sensitivity = 0.1f; // Increased for faster rotation
        
        // Only apply rotation if there's actual mouse movement
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
            
            // Clamp rotation values to prevent excessive spinning
            newRotation.x = glm::clamp(newRotation.x, -45.0f, 45.0f);
            newRotation.y = glm::clamp(newRotation.y, -90.0f, 90.0f);
            newRotation.z = glm::clamp(newRotation.z, -45.0f, 45.0f);
            
            // Update drag start position to prevent accumulation
            dragStartMousePos = glm::vec2(mouseX, mouseY);
        } else {
            // No mouse movement, no rotation
            newRotation = glm::vec3(0.0f);
        }
    } else {
        // Not dragging, ensure no rotation
        newRotation = glm::vec3(0.0f);
    }
}

void Gizmo::handleScale(GLFWwindow* window, Camera& camera, const glm::vec3& objectPosition, glm::vec3& newScale) {
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !isDragging) {
        isDragging = true;
        dragStartMousePos = glm::vec2(mouseX, mouseY);
        
        // Get ray from camera
        glm::vec3 rayOrigin = camera.Position;
        glm::vec3 rayDirection = camera.GetRay(window);
        
        // Check which scale handle is closest
        float minDist = 1.0f; // Increased for easier selection
        selectedAxis = NONE;
    
        // Check X scale handle
        glm::vec3 xHandle = objectPosition + glm::vec3(arrowLength, 0.0f, 0.0f);
        float xDist = glm::distance(rayOrigin + rayDirection * glm::dot(xHandle - rayOrigin, rayDirection), xHandle);
        if (xDist < minDist) { 
            selectedAxis = X; 
            minDist = xDist; 
            Logger::AddLog("Gizmo Scale: X axis selected, distance: %.3f", xDist);
        }
        
        // Check Y scale handle
        glm::vec3 yHandle = objectPosition + glm::vec3(0.0f, arrowLength, 0.0f);
        float yDist = glm::distance(rayOrigin + rayDirection * glm::dot(yHandle - rayOrigin, rayDirection), yHandle);
        if (yDist < minDist) { 
            selectedAxis = Y; 
            minDist = yDist; 
            Logger::AddLog("Gizmo Scale: Y axis selected, distance: %.3f", yDist);
        }
        
        // Check Z scale handle
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
        // Don't reset selectedAxis - keep it selected for next operation
        // selectedAxis = NONE; // REMOVED: Keep axis selected
        // Reset scale when mouse is released to prevent continued scaling
        newScale = glm::vec3(0.0f);
    }
    
    if (isDragging && selectedAxis != NONE) {
        // Calculate scale based on mouse movement
        glm::vec2 mouseDelta = glm::vec2(mouseX, mouseY) - dragStartMousePos;
        float sensitivity = 0.005f; // Reduced sensitivity for more precise scaling
        
        // Only apply scaling if there's actual mouse movement
        if (glm::length(mouseDelta) > 0.1f) {
            // Start with current scale and apply delta
            newScale = glm::vec3(0.0f); // This will be the scale delta
            
            // UNIFORM SCALING: Scale all axes together based on mouse movement
            float scaleDelta = (mouseDelta.x + mouseDelta.y) * sensitivity;
            
            switch (selectedAxis) {
                case X:
                    // Scale X axis, but also apply uniform scaling
                    newScale.x = scaleDelta;
                    newScale.y = scaleDelta * 0.5f; // Apply some uniform scaling
                    newScale.z = scaleDelta * 0.5f; // Apply some uniform scaling
                    break;
                case Y:
                    // Scale Y axis, but also apply uniform scaling
                    newScale.x = scaleDelta * 0.5f; // Apply some uniform scaling
                    newScale.y = scaleDelta;
                    newScale.z = scaleDelta * 0.5f; // Apply some uniform scaling
                    break;
                case Z:
                    // Scale Z axis, but also apply uniform scaling
                    newScale.x = scaleDelta * 0.5f; // Apply some uniform scaling
                    newScale.y = scaleDelta * 0.5f; // Apply some uniform scaling
                    newScale.z = scaleDelta;
                    break;
            }
            
            // Update drag start position to prevent accumulation
            dragStartMousePos = glm::vec2(mouseX, mouseY);
        } else {
            // No mouse movement, no scaling
            newScale = glm::vec3(0.0f);
        }
    } else {
        // Not dragging, ensure no scaling
        newScale = glm::vec3(0.0f);
    }
}

bool Gizmo::rayIntersectsAxis(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, 
                              const glm::vec3& axisStart, const glm::vec3& axisEnd, 
                              float axisRadius, float& t) {
    // Calculate axis direction and length
    glm::vec3 axisDir = glm::normalize(axisEnd - axisStart);
    float axisLength = glm::length(axisEnd - axisStart);
    
    // Calculate perpendicular distance from ray to axis
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
    
    // Check if intersection is within axis bounds
    if (t_axis < 0.0f || t_axis > axisLength) return false;
    
    // Calculate closest point on axis
    glm::vec3 closestPoint = axisStart + t_axis * axisDir;
    
    // Calculate distance from ray to closest point
    float dist = glm::length(rayOrigin + s * rayDirection - closestPoint);
    
    if (dist <= axisRadius && s > 0.0f) {
        t = s;
        return true;
    }
    
    return false;
}