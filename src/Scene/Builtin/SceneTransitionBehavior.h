#ifndef SCENE_TRANSITION_BEHAVIOR_H
#define SCENE_TRANSITION_BEHAVIOR_H

#include "../Behavior.h"
#include "../SceneManager.h"
#include <string>

class SceneTransitionBehavior : public Behavior {
public:
    std::string targetScene = "";
    std::string targetFlag = "";
    float duration = 1.0f;
    TransitionType transitionType = TransitionType::FadeBlack;
    bool triggerOnStart = false;

    void OnStart() override {
        if (triggerOnStart) {
            Trigger();
        }
    }

    void Trigger() {
        if (!targetScene.empty()) {
            TransitionToScene(targetScene, transitionType, duration);
        } else if (!targetFlag.empty()) {
            TransitionToFlag(targetFlag, transitionType, duration);
        }
    }
};

#endif
