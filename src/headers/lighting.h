#pragma once
#include "raylib.h"
#include "rlights.h"
#include <string>

struct Lighting {
    int id = -1;

    Light light;
    Vector3 position;
    Vector3 target;

    Color color = WHITE;
    bool enabled;

    float intensity = 1.0f;
    float range = 5.0f;
};

Lighting create_lighting(Vector3 pos, Color color);
void update_lighting(Shader shader, Lighting& l);
int allocate_light_id();
