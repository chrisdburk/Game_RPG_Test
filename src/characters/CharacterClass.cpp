#include "characters/CharacterClass.h"

#include "characters/Fighter.h"

namespace classes {

namespace {
std::vector<std::unique_ptr<CharacterClass>> buildClasses() {
    std::vector<std::unique_ptr<CharacterClass>> list;
    list.push_back(std::make_unique<Fighter>());
    // Register new classes here, e.g. list.push_back(std::make_unique<Wizard>());
    return list;
}
}  // namespace

const std::vector<std::unique_ptr<CharacterClass>>& all() {
    static const std::vector<std::unique_ptr<CharacterClass>> registry = buildClasses();
    return registry;
}

const CharacterClass* find(const std::string& name) {
    for (const auto& cls : all()) {
        if (cls->name() == name) return cls.get();
    }
    return nullptr;
}

}  // namespace classes
