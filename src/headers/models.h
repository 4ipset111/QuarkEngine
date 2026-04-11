#pragma once
#include "raylib.h"
#include "entity.h"
#include <functional>
#include <filesystem>

extern std::vector<ModelAsset> assets;

void load_models();
void update_model(Entity* e);
void unload_models();
void load_external_models();
void refresh_models();
bool is_model_file(const std::filesystem::path& p);