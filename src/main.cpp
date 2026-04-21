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

void ApplyCustomImGuiTheme()
{
    ImGuiStyle& style = ImGui::GetStyle();

    // ====== SHAPES ======
    style.WindowRounding = 0.0f;
    style.FrameRounding  = 0.0f;
    style.PopupRounding  = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.TabRounding = 0.0f;

    style.FrameBorderSize = 1.0f;
    style.WindowBorderSize = 1.0f;

    style.FramePadding = ImVec2(6, 3);
    style.ItemSpacing = ImVec2(6, 4);

    ImVec4* colors = style.Colors;

    // ====== GLOBAL ======
    colors[ImGuiCol_Text]           = ImVec4(0.80f, 0.82f, 0.85f, 1.00f); // #c9cdd1
    colors[ImGuiCol_WindowBg]       = ImVec4(0.16f, 0.17f, 0.18f, 1.00f); // #2a2c2f
    colors[ImGuiCol_ChildBg]        = ImVec4(0.14f, 0.15f, 0.16f, 1.00f);
    colors[ImGuiCol_PopupBg]        = ImVec4(0.20f, 0.21f, 0.23f, 1.00f); // #32353a

    // ====== BORDERS ======
    colors[ImGuiCol_Border]         = ImVec4(0.27f, 0.28f, 0.30f, 1.00f); // #44484d
    colors[ImGuiCol_Separator]      = ImVec4(0.24f, 0.25f, 0.27f, 1.00f);

    // ====== FRAMES (inputs, edits) ======
    colors[ImGuiCol_FrameBg]        = ImVec4(0.14f, 0.15f, 0.16f, 1.00f); // #24272a
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.23f, 0.25f, 0.27f, 1.00f); // hover
    colors[ImGuiCol_FrameBgActive]  = ImVec4(0.78f, 0.52f, 0.18f, 1.00f); // #c8842d

    // ====== TITLE / MENUBAR ======
    colors[ImGuiCol_TitleBg]        = ImVec4(0.19f, 0.20f, 0.22f, 1.00f); // #31343a
    colors[ImGuiCol_TitleBgActive]  = ImVec4(0.20f, 0.21f, 0.23f, 1.00f);
    colors[ImGuiCol_MenuBarBg]      = ImVec4(0.19f, 0.20f, 0.22f, 1.00f);

    // ====== BUTTONS ======
    colors[ImGuiCol_Button]         = ImVec4(0.30f, 0.32f, 0.35f, 1.00f); // ~#51565c
    colors[ImGuiCol_ButtonHovered]  = ImVec4(0.36f, 0.38f, 0.41f, 1.00f); // hover
    colors[ImGuiCol_ButtonActive]   = ImVec4(0.24f, 0.26f, 0.28f, 1.00f); // pressed

    // ====== HEADERS (Tree, Selectable) ======
    colors[ImGuiCol_Header]         = ImVec4(0.16f, 0.17f, 0.18f, 1.00f);
    colors[ImGuiCol_HeaderHovered]  = ImVec4(0.23f, 0.25f, 0.27f, 1.00f);
    colors[ImGuiCol_HeaderActive]   = ImVec4(0.78f, 0.52f, 0.18f, 1.00f); // selection

    // ====== SELECTION ======
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.78f, 0.52f, 0.18f, 0.35f);

    // ====== SCROLLBAR ======
    colors[ImGuiCol_ScrollbarBg]    = ImVec4(0.18f, 0.20f, 0.22f, 1.00f); // #2f3337
    colors[ImGuiCol_ScrollbarGrab]  = ImVec4(0.33f, 0.36f, 0.38f, 1.00f); // #555b62
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.43f, 0.46f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.50f, 0.53f, 0.56f, 1.00f);

    // ====== TABS ======
    colors[ImGuiCol_Tab]            = ImVec4(0.20f, 0.21f, 0.23f, 1.00f);
    colors[ImGuiCol_TabHovered]     = ImVec4(0.30f, 0.32f, 0.35f, 1.00f);
    colors[ImGuiCol_TabActive]      = ImVec4(0.24f, 0.26f, 0.28f, 1.00f);

    // ====== CHECKBOX ======
    colors[ImGuiCol_CheckMark]      = ImVec4(0.78f, 0.52f, 0.18f, 1.00f);

    // ====== RESIZE GRIP ======
    colors[ImGuiCol_ResizeGrip]         = ImVec4(0.30f, 0.32f, 0.35f, 1.00f);
    colors[ImGuiCol_ResizeGripHovered]  = ImVec4(0.40f, 0.43f, 0.46f, 1.00f);
    colors[ImGuiCol_ResizeGripActive]   = ImVec4(0.78f, 0.52f, 0.18f, 1.00f);
}

int main() {
    if (!std::filesystem::exists("assets")) std::filesystem::create_directory("assets");

    InitWindow(1280, 720, "Quark Engine");

    SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));
    rlImGuiSetup(true);
    
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    io.Fonts->AddFontFromFileTTF("fonts/Roboto-Regular.ttf", 16.0f);
    io.Fonts->Build();

    ApplyCustomImGuiTheme();
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
