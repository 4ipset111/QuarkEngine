#include "headers/project.h"
#include "headers/entity.h"
#include "headers/lighting.h"
#include "headers/models.h"
#include "headers/tex.h"
#include "raylib.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;
using json = nlohmann::json;

static std::string color_to_hex(Color color) {
    char buf[12];
    snprintf(buf, sizeof(buf), "#%02X%02X%02X%02X", color.r, color.g, color.b, color.a);
    return buf;
}

static Color hex_to_color(const std::string& hex) {
    Color color = WHITE;
    if (hex.size() < 7) return color;

    color.r = (unsigned char)strtol(hex.substr(1, 2).c_str(), nullptr, 16);
    color.g = (unsigned char)strtol(hex.substr(3, 2).c_str(), nullptr, 16);
    color.b = (unsigned char)strtol(hex.substr(5, 2).c_str(), nullptr, 16);
    color.a = hex.size() >= 9 ? (unsigned char)strtol(hex.substr(7, 2).c_str(), nullptr, 16) : 255;
    return color;
}

void project_new(const std::string& folder_path, Scene& scene) {
    fs::create_directories(folder_path);
    fs::create_directories(folder_path + "/resources");

    json j;
    j["entities"] = json::array();
    
    std::ofstream f(folder_path + "/scene.json");
    f << j.dump(4);

    scene.release_resources();
}

void project_save(const std::string& folder_path, const Scene& scene) {
    fs::create_directories(folder_path + "/resources");

    json j;
    j["entities"] = json::array();

    for (const auto& e : scene.entities) {
        json ej;
        ej["name"]          = e.name;
        ej["type"]          = e.type;

        ej["position"] = json::array({ (float)e.position.x, (float)e.position.y, (float)e.position.z });
        ej["rotation"] = json::array({ (float)e.rotation.x, (float)e.rotation.y, (float)e.rotation.z });
        ej["scale"]    = json::array({ (float)e.scale.x,    (float)e.scale.y,    (float)e.scale.z    });

        ej["color"]         = color_to_hex(e.color);
        ej["outline_color"] = color_to_hex(e.outline_color);

        std::string tex_name = "None";
        for (const auto& opt : texture_options) {
            if (opt.texture.id == e.texture.id && opt.texture.id != 0)
                tex_name = opt.name;
        }

        ej["texture"]          = tex_name;
        ej["texture_stretch"]  = e.texture_stretch;
        ej["auto_uv"]          = e.auto_uv;
        ej["texture_repeat_u"] = e.texture_repeat_u;
        ej["texture_repeat_v"] = e.texture_repeat_v;
        ej["uv_scale_x"]       = e.uv_scale_vec.x;
        ej["uv_scale_y"]       = e.uv_scale_vec.y;
        ej["segments"]         = e.segments;

        ej["has_light"]        = e.has_light;
        ej["light_color"]      = color_to_hex(e.light.color);
        ej["light_intensity"]  = e.light.intensity;
        ej["light_range"]      = e.light.range;

        ej["asset_name"]       = e.asset ? e.asset->name : "";

        j["entities"].push_back(ej);
    }

    std::ofstream f(folder_path + "/scene.json");
    if (!f.is_open()) {
        TraceLog(LOG_ERROR, "Failed to open scene.json for writing: %s", folder_path.c_str());
        return;
    }
    f << j.dump(4);
    f.close();

    if (fs::exists("assets")) {
        for (auto& entry : fs::directory_iterator("assets")) {
            if (!entry.is_regular_file()) continue;
            fs::path dst = fs::path(folder_path) / "resources" / entry.path().filename();
            fs::copy_file(entry.path(), dst, fs::copy_options::overwrite_existing);
        }
    }
}

bool project_load(const std::string& folder_path, Scene& scene, Shader shader) {
    TraceLog(LOG_INFO, "project_load called: %s", folder_path.c_str());
    std::string json_path = folder_path + "/scene.json";
    if (!fs::exists(json_path)) return false;

    fs::create_directories("assets");
    std::string res_path = folder_path + "/resources";

    if (fs::exists(res_path)) {
        for (auto& entry : fs::directory_iterator(res_path)) {
            if (!entry.is_regular_file()) continue;
            fs::path dst = fs::path(res_path) / entry.path().filename();
            fs::copy_file(entry.path(), dst, fs::copy_options::overwrite_existing);
        }
    }

    refresh_textures(nullptr, folder_path);
    refresh_assets(folder_path);
    load_external_models(folder_path);
    scene.release_resources();

    std::ifstream f(json_path);
    json j;
    f >> j;

    for (auto& ej : j["entities"]) {
        Entity e;
        e.name = ej["name"].get<std::string>();
        e.type = (ObjectType)ej["type"].get<int>();
        e.id   = static_cast<int>(scene.entities.size());

        e.position = {
            ej["position"][0].get<float>(),
            ej["position"][1].get<float>(),
            ej["position"][2].get<float>()
        };

        e.rotation = {
            ej["rotation"][0].get<float>(),
            ej["rotation"][1].get<float>(),
            ej["rotation"][2].get<float>()
        };

        e.scale = {
            ej["scale"][0].get<float>(),
            ej["scale"][1].get<float>(),
            ej["scale"][2].get<float>()
        };

        e.color         = hex_to_color(ej["color"].get<std::string>());
        e.outline_color = hex_to_color(ej["outline_color"].get<std::string>());

        e.texture_stretch  = ej["texture_stretch"].get<bool>();
        e.auto_uv          = ej["auto_uv"].get<bool>();
        e.texture_repeat_u = ej["texture_repeat_u"].get<float>();
        e.texture_repeat_v = ej["texture_repeat_v"].get<float>();
        e.uv_scale_vec.x   = ej["uv_scale_x"].get<float>();
        e.uv_scale_vec.y   = ej["uv_scale_y"].get<float>();
        e.segments         = ej["segments"].get<int>();

        std::string asset_name = ej["asset_name"].get<std::string>();

        for (auto& a : assets) {
            if (a.name == asset_name) {
                e.asset = &a;
                e.type = a.type;

                if (a.is_procedural) {
                    e.model = a.generator(e.segments);
                } 
                
                else {
                    e.model = a.loaded_model;
                }

                store_uv(&e);
                break;
            }
        }

        std::string tex_name = ej["texture"].get<std::string>();
        e.texture = {0};

        for (auto& opt : texture_options) {
            if (opt.name == tex_name) {
                e.texture = opt.texture;
                break;
            }
        }

        for (int i = 0; i < e.model.materialCount; i++) {
            e.model.materials[i].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;

            if (e.texture.id != 0) {
                e.model.materials[i].maps[MATERIAL_MAP_DIFFUSE].texture = e.texture;
            } 
            
            else {
                e.model.materials[i].maps[MATERIAL_MAP_DIFFUSE].texture = {0};
            }

            e.model.materials[i].shader = shader;
        }

        e.shader_assigned = true;

        e.has_light = ej["has_light"].get<bool>();
        if (e.has_light) {
            e.light = create_lighting(e.position, hex_to_color(ej["light_color"].get<std::string>()));
            e.light.intensity = ej["light_intensity"].get<float>();
            e.light.range     = ej["light_range"].get<float>();

            int new_id = allocate_light_id();
            if (new_id != -1) {
                e.light.id = new_id;
                e.light.light = create_light_at_slot(new_id, LIGHT_POINT, e.position, Vector3Zero(), e.light.color, shader);
                e.light_created = true;
            } 
            
            else {
                e.has_light = false;
            }
        }

        scene.entities.push_back(e);
    }

    return true;
}