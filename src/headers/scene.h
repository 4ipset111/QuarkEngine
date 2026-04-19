#pragma once
#include <vector>
#include "entity.h"
#include "lighting.h"
#include <memory>

struct Scene {
    std::vector<Entity> entities;
    std::vector<Lighting> lightings;

    int selected = -1;

    Entity* get_selected();
    void release_resources();
};
