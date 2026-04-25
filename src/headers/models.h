#pragma once
#include "raylib.h"
#include "entity.h"
#include <functional>
#include <filesystem>

extern std::vector<ModelAsset> assets;

struct Scene;

void load_models();
void update_model(Entity* e);
void unload_models();
void load_external_models(std::string project_path);
void refresh_models(std::string project_path, Scene& scene);
bool is_model_file(const std::filesystem::path& p);