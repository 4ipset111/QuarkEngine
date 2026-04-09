#include "headers/scene.h"

void Scene::add_object(Model model, ObjectType type) {
    Entity e(static_cast<int>(entities.size()));
    e.model = model;
    e.type = type;
    entities.push_back(e);
    selected = static_cast<int>(entities.size()) - 1;
}

Entity* Scene::get_selected() {
    if (selected < 0 || selected >= static_cast<int>(entities.size())) return nullptr;
    return &entities[selected];
}
