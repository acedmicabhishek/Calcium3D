#ifndef C3D_INPUT_H
#define C3D_INPUT_H

#include <glm/glm.hpp>
#include <string>

namespace C3D {
    namespace Input {
        bool GetKey(int key);
        bool GetKeyDown(int key);
        bool GetKeyUp(int key);
        
        bool GetMouseButton(int button);
        glm::vec2 GetMousePos();
        float GetMouseX();
        float GetMouseY();
        
        void SetCursorMode(int mode);
        int GetCursorMode();
        
        bool IsButtonClicked(const std::string& name);
    }
}

#endif
