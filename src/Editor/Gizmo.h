#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>


class Shader;
class Camera;

class Gizmo {
public:
    enum TransformMode {
        TRANSLATE,
        ROTATE,
        SCALE
    };

    enum Axis {
        NONE = 0,
        X = 1,
        Y = 2,
        Z = 3
    };

    Gizmo();
    ~Gizmo();

    void Draw(Shader& shader, Camera& camera, const glm::vec3& position);
    void HandleMouse(GLFWwindow* window, Camera& camera, const glm::vec3& objectPosition, 
                     glm::vec3& newPosition, glm::vec3& newRotation, glm::vec3& newScale);
    
    void SetMode(TransformMode mode) { currentMode = mode; }
    TransformMode GetMode() const { return currentMode; }
    
    bool IsDragging() const { return isDragging; }
    Axis GetSelectedAxis() const { return selectedAxis; }

private:
    void setupGizmo();
    void createArrow(const glm::vec3& direction, const glm::vec3& color, std::vector<float>& vertices);
    
    bool rayIntersectsAxis(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, 
                           const glm::vec3& axisStart, const glm::vec3& axisEnd, 
                           float axisRadius, float& t);
    
    void handleTranslate(GLFWwindow* window, Camera& camera, const glm::vec3& objectPosition, glm::vec3& newPosition);
    void handleRotate(GLFWwindow* window, Camera& camera, const glm::vec3& objectPosition, glm::vec3& newRotation);
    void handleScale(GLFWwindow* window, Camera& camera, const glm::vec3& objectPosition, glm::vec3& newScale);

    
    GLuint VAO, VBO;
    
    
    bool isDragging;
    Axis selectedAxis;
    TransformMode currentMode;
    
    
    glm::vec3 dragStartWorldPos;
    glm::vec3 objectStartPos;
    glm::vec2 dragStartMousePos;
    
    
    float arrowLength;
    float arrowRadius;
    float handleRadius;
};