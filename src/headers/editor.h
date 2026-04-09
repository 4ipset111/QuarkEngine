#pragma once
#include "scene.h"
#include "raylib.h"
#include "tex.h"
#include "models.h"

struct Editor {
    Scene scene;
    int selected_asset_index = -1;
    
    void draw_ui();
    void draw_assets_ui();
    void handle_input();
    void draw_entity_with_texture(Entity& e);
    void draw_gizmo(Camera3D camera);
};