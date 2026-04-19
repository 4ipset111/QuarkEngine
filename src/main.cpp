#include "raylib.h"
#include "rlImGui.h"
#include "imgui.h"
#include "raymath.h"
#include "rlgl.h"
#include "headers/editor.h"
#include "headers/camera.h"

static Entity make_entity_from_asset(Scene& scene, ModelAsset* asset) {
    Entity entity;
    entity.id = static_cast<int>(scene.entities.size());
    entity.type = asset->type;
    entity.asset = asset;
    entity.segments = 16;
    entity.name = scene.make_default_name_for(entity);

    if (asset->isProcedural) {
        entity.model = asset->generator(entity.segments);
        store_uv(&entity);
    } else {
        entity.model = asset->loadedModel;
    }

    entity.texture = {0};
    return entity;
}

int main() {
    if (!std::filesystem::exists("assets")) std::filesystem::create_directory("assets");

    InitWindow(1280, 720, "Quark Engine");

    SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));
    rlImGuiSetup(true);
    SetExitKey(0);

    Editor editor;
    FlyCamera camera;

    Shader shader = LoadShader("assets/lighting.vs", "assets/lighting.fs");

    int emission_color_loc = GetShaderLocation(shader, "emissionColor");
    int emission_power_loc = GetShaderLocation(shader, "emissionPower");

    load_models();
    load_textures();

    for (auto& asset : assets) {
        if (asset.type != SPHERE) continue;

        Entity light_entity = make_entity_from_asset(editor.scene, &asset);
        light_entity.has_light = true;
        light_entity.name = editor.scene.make_default_name_for(light_entity);
        light_entity.position = { 2.0f, 3.0f, 2.0f };
        light_entity.scale = { 0.2f, 0.2f, 0.2f };
        light_entity.color = WHITE;
        light_entity.light = create_lighting(light_entity.position, WHITE);
        light_entity.light.range = 12.0f;
        light_entity.light.intensity = 1.5f;
        light_entity.light.id = allocate_light_id();
        light_entity.light.light = CreateLight(LIGHT_POINT, light_entity.position, Vector3Zero(), light_entity.light.color, shader);
        light_entity.light_created = true;

        editor.scene.entities.push_back(light_entity);
        break;
    }

    while (!WindowShouldClose()) {
        SetWindowTitle(TextFormat("Quark Engine / FPS: %d", GetFPS()));

        BeginDrawing();
        ClearBackground(DARKGRAY);

        rlImGuiBegin();

        editor.draw_gizmo(camera.get_camera());
        camera.update();
        editor.handle_input();

        BeginMode3D(camera.get_camera());

        DrawGrid(20, 1.0f);

        BeginShaderMode(shader);

        for (auto& e : editor.scene.entities)
        {
            if (!e.shader_assigned)
            {
                for (int i = 0; i < e.model.materialCount; i++)
                    e.model.materials[i].shader = shader;
                e.shader_assigned = true;
            }

            if (e.has_light && e.light_created)
                update_lighting(shader, e.light);

            if (!e.has_light && e.light_created)
            {
                e.light.enabled = false;
                update_lighting(shader, e.light);
                e.light_created = false;
            }

            if (e.has_light)
            {
                Vector3 emission = {
                    e.color.r / 255.0f,
                    e.color.g / 255.0f,
                    e.color.b / 255.0f
                };

                float power = 2.0f;

                SetShaderValue(shader, emission_color_loc, &emission, SHADER_UNIFORM_VEC3);
                SetShaderValue(shader, emission_power_loc, &power, SHADER_UNIFORM_FLOAT);
            }
            else
            {
                Vector3 zero = {0, 0, 0};
                float power = 0.0f;

                SetShaderValue(shader, emission_color_loc, &zero, SHADER_UNIFORM_VEC3);
                SetShaderValue(shader, emission_power_loc, &power, SHADER_UNIFORM_FLOAT);
            }

            draw_entity_with_texture(e);
        }
        
        EndShaderMode();

        for (auto& l : editor.scene.lightings)
        {
            if (l.enabled) DrawSphere(l.position, 0.2f, l.color);
        }

        EndMode3D();

        editor.draw_ui(shader);

        rlImGuiEnd();

        EndDrawing();
    }

    editor.scene.release_resources();
    unload_models();
    unload_textures();

    rlImGuiShutdown();
    CloseWindow();
}
