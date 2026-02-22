#ifndef BEHAVIOR_REGISTRY_H
#define BEHAVIOR_REGISTRY_H

#include <string>
#include <map>
#include <functional>
#include <memory>
#include "Behavior.h"

class BehaviorRegistry {
public:
    using ScriptCreator = std::function<std::unique_ptr<Behavior>()>;

    static void Register(const std::string& name, ScriptCreator creator) {
        GetCreators()[name] = creator;
    }

    static std::unique_ptr<Behavior> Create(const std::string& name) {
        auto& creators = GetCreators();
        if (creators.find(name) != creators.end()) {
            return creators[name]();
        }
        return nullptr;
    }

    static std::vector<std::string> GetAvailableScripts() {
        std::vector<std::string> scripts;
        for (auto const& [name, creator] : GetCreators()) {
            scripts.push_back(name);
        }
        return scripts;
    }

private:
    static std::map<std::string, ScriptCreator>& GetCreators() {
        static std::map<std::string, ScriptCreator> creators;
        return creators;
    }
};


#define REGISTER_BEHAVIOR(T) \
    namespace { \
        struct T##Registrar { \
            T##Registrar() { \
                BehaviorRegistry::Register(#T, []() { return std::make_unique<T>(); }); \
            } \
        } T##_registrar; \
    }

#endif
