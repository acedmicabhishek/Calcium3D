#include "BehaviorRegistry.h"

std::map<std::string, BehaviorRegistry::ScriptCreator>& BehaviorRegistry::GetCreators() {
    static std::map<std::string, ScriptCreator> creators;
    return creators;
}
