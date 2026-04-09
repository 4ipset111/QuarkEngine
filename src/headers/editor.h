#pragma once
#include "scene.h"
#include "raylib.h"

struct Editor {
    Scene scene;

    void draw_ui();
    void load_models();
    void load_textures();
    void handle_input();
    void update_model(Entity* e);
    void draw_entity_with_texture(Entity& e);
    void draw_gizmo(Camera3D camera);
};