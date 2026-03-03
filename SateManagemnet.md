# Calcium3D Core Architecture: State Management API

The Calcium3D engine uses a powerful, memory-efficient **Stack-Based State Machine** to manage game flow. This system strictly separates **State** (the logic and flow) from **Scene** (the 3D data and assets).

Instead of creating a new C++ class for every single game level, you define broad logic states (e.g., `StartScreen`, `GameplayScreen`, `PauseMenu`) and pass the specific level data as a payload when transitioning.

---

## 1. Defining a New State

To create a new state, you simply create a class that inherits from the `State` base class found in `<Core/State.h>`. 

A `State` has several lifecycle hooks you can override:

```cpp
#include "Core/State.h"
#include <any>

class MyCustomState : public State {
public:
    // Called once when the state is first booted up (pushed or changed to)
    void OnEnter(std::any payload) override {
        // You can extract payload data here, like a map path to load!
    }

    // Called every frame to run game or UI logic
    void Update(float deltaTime) override {
        // ... frame logic ...
    }

    // Called every frame to render UI or 2D overlays on top of the 3D scene
    void Render(glm::vec2 canvasSize, glm::vec2 baseScreenPos) override {
        // ... ImGui or 2D rendering ...
    }

    // Called when another state is Pushed ON TOP of this one (e.g. Pause Menu)
    void OnPause() override {
        // Freeze gameplay logic here
    }

    // Called when the state on top of this one is Popped, restoring focus to this state
    void OnResume() override {
        // Unfreeze gameplay logic here
    }

    // Called when the state is completely destroyed / swapped out
    void OnExit() override {
        // Cleanup memory here
    }
};
```

---

## 2. Example: Creating a Start Screen State

A main menu usually handles a 2D UI layout and waits for the user to press a "Play" button. You can load previously built UI layouts from the `UICreationEngine` using the built-in `LoadFromLayout()` function.

**`StartScreen.h`**
```cpp
#ifndef START_SCREEN_H
#define START_SCREEN_H

#include "Core/State.h"

class StartScreen : public State {
public:
    void Init() override; // Legacy initialization, called right after creation
    void Update(float deltaTime) override;
    void Render(glm::vec2 canvasSize, glm::vec2 baseScreenPos = {0,0}) override;
};

#endif
```

**`StartScreen.cpp`**
```cpp
#include "StartScreen.h"
#include "Core/StateManager.h"
#include "UI/UICreationEngine.h"
#include <imgui.h>

void StartScreen::Init() {
    // Automatically fetches the UI elements built in the Editor for "Start Screen"
    LoadFromLayout("Start Screen"); 
}

void StartScreen::Update(float deltaTime) {
    // If a button labeled "Play" is clicked in the UI, change to gameplay
    if (UICreationEngine::IsButtonClicked("Play")) {
        // Swap out the Start Screen entirely and boot up Gameplay!
        // You can pass the path to the first level as the payload.
        StateManager::ChangeState("Gameplay", std::string("assets/maps/level_1.scene"));
    }
}

void StartScreen::Render(glm::vec2 canvasSize, glm::vec2 baseScreenPos) {
    // Let the Engine render the 2D UI elements loaded from the layout
    for (auto& element : m_UIElements) {
        element.Render(canvasSize, baseScreenPos);
    }
}
```

---

## 3. Example: Creating the Gameplay State

The `GameplayScreen` is responsible for letting the 3D scene run. It usually renders nothing (so the 3D world is fully visible) or just renders an HUD (Heads Up Display). It also uses `OnEnter` to catch the payload passed from the `StartScreen`.

**`GameplayScreen.cpp`**
```cpp
#include "GameplayScreen.h"
#include "Core/StateManager.h"
#include "Core/Application.h"
#include <iostream>

void GameplayScreen::OnEnter(std::any payload) {
    // Check if the payload contains a scene path string
    if (payload.type() == typeid(std::string)) {
        std::string sceneToLoad = std::any_cast<std::string>(payload);
        std::cout << "Loading 3D World: " << sceneToLoad << std::endl;
        
        // Tell the Application/Scene layer to deserialize and load the .scene file
        // Application::Get().GetScene()->Load(sceneToLoad);
    }
}

void GameplayScreen::Update(float deltaTime) {
    // Handle in-game events. For example, pressing "Escape" opens the pause menu.
    if (InputManager::IsKeyPressed(GLFW_KEY_ESCAPE)) {
        // PUSH the Pause state ON TOP of Gameplay. 
        // Gameplay remains securely frozen in memory!
        StateManager::PushState("PauseMenu"); 
    }
}

void GameplayScreen::Render(glm::vec2 canvasSize, glm::vec2 baseScreenPos) {
    // Render an optional HUD, crosshair, or health bar here.
    // If you render nothing, the 3D world perfectly shows through!
}
```

---

## 4. Registering Your States (Factory Pattern)

Before the engine can transition to your states, you **must register them** with the `StateManager`. This is a one-time setup logic usually done when the Game or Editor first boots up (e.g., inside `RuntimeApplication::LoadProjectConfig()` or immediately in `Application::Init()`).

You register states using a Template Factory. This prevents you from ever having to manually write `new GameplayScreen()` or deal with raw pointer memory leaks.

```cpp
#include "Core/StateManager.h"
#include "StartScreen.h"
#include "GameplayScreen.h"

// Tell the engine what string maps to what C++ Class.
StateManager::RegisterState<StartScreen>("Start Screen");
StateManager::RegisterState<GameplayScreen>("Gameplay");
StateManager::RegisterState<PauseMenuState>("PauseMenu");

// Boot up the game by setting the initial state!
StateManager::ChangeState("Start Screen");
```

---

## 5. The Stack API Reference

You interact with the Game Flow exclusively through these static API functions provided by `<Core/StateManager.h>`:

### `StateManager::ChangeState(std::string name, std::any payload = {})`
*Destructive Transition.* 
Calls `OnExit()` on all currently running states, destroys them out of RAM, and boots up the requested state. Use this for major swaps like Main Menu ➜ Game, or Level 1 ➜ Level 2.

### `StateManager::PushState(std::string name, std::any payload = {})`
*Overlay Transition.*
Freezes the current state (calls `OnPause()`) and throws a new state on top of it. Use this for Pause Menus, Inventories, or temporary dialog boxes. 

### `StateManager::PopState()`
*Resume Transition.*
Destroys the top-most state (calls `OnExit()`), and thaws out the state directly underneath it (calls `OnResume()`). Use this when clicking "Resume Game" on a Pause Menu.

### `StateManager::GetCurrentStateName()`
Returns a `std::string` of the actively running state at the very top of the stack.
