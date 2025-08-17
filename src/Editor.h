#ifndef EDITOR_H
#define EDITOR_H

class Editor {
public:
    enum EditorMode {
        EDIT_MODE,
        PLAY_MODE
    };

    enum SelectionMode {
        TRANSLATE,
        ROTATE,
        SCALE
    };

    static bool isEditMode;
    static SelectionMode selectionMode;
    static bool isFreeMovement;

    static void ToggleMode();
    static void ToggleFreeMovement();
};

#endif