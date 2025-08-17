#include "Editor.h"

bool Editor::isEditMode = true;
Editor::SelectionMode Editor::selectionMode = Editor::TRANSLATE;
bool Editor::isFreeMovement = true;

void Editor::ToggleMode() {
    isEditMode = !isEditMode;
}

void Editor::ToggleFreeMovement() {
    isFreeMovement = !isFreeMovement;
}