#include "../Scene/Behavior.h"
#include "../Scene/BehaviorRegistry.h"
#include "../Scene/Scene.h"
#include "../Core/InputManager.h"
#include <GLFW/glfw3.h>
#include <iostream>

// ============================================
// Built-in: WASD Movement + Jump
// Attach to any object to control it with WASD
// ============================================
class PlayerMovement : public Behavior {
public:
    float speed = 5.0f;
    float jumpForce = 8.0f;
    bool grounded = true;

    void OnStart() override {
        std::cout << "[PlayerMovement] Attached to: " << gameObject->name << std::endl;
    }

    void OnUpdate(float dt) override {
        if (!gameObject) return;

        glm::vec3 movement(0.0f);
        if (InputManager::IsKeyPressed(GLFW_KEY_W)) movement.z -= 1.0f;
        if (InputManager::IsKeyPressed(GLFW_KEY_S)) movement.z += 1.0f;
        if (InputManager::IsKeyPressed(GLFW_KEY_A)) movement.x -= 1.0f;
        if (InputManager::IsKeyPressed(GLFW_KEY_D)) movement.x += 1.0f;

        // Jump
        if (InputManager::IsKeyPressed(GLFW_KEY_SPACE) && grounded) {
            gameObject->velocity.y = jumpForce;
            grounded = false;
        }

        // Ground check (simple)
        if (gameObject->position.y <= -0.5f) {
            gameObject->position.y = -0.5f;
            gameObject->velocity.y = 0.0f;
            grounded = true;
        }

        if (glm::length(movement) > 0.1f) {
            movement = glm::normalize(movement) * speed * dt;
            gameObject->position += movement;
        }
    }
};

REGISTER_BEHAVIOR(PlayerMovement)

// ============================================
// Built-in: Spin / Rotate Object
// ============================================
class SpinBehavior : public Behavior {
public:
    float spinSpeed = 90.0f;

    void OnUpdate(float dt) override {
        if (!gameObject) return;
        float angle = glm::radians(spinSpeed * dt);
        gameObject->rotation = glm::rotate(gameObject->rotation, angle, glm::vec3(0.0f, 1.0f, 0.0f));
    }
};

REGISTER_BEHAVIOR(SpinBehavior)

// ============================================
// Built-in: Bobbing (up/down float)
// ============================================
class BobbingBehavior : public Behavior {
public:
    float amplitude = 0.5f;
    float frequency = 2.0f;
    float baseY = 0.0f;
    float timer = 0.0f;

    void OnStart() override {
        if (gameObject) baseY = gameObject->position.y;
    }

    void OnUpdate(float dt) override {
        if (!gameObject) return;
        timer += dt;
        gameObject->position.y = baseY + sin(timer * frequency) * amplitude;
    }
};

REGISTER_BEHAVIOR(BobbingBehavior)
