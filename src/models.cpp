#include "headers/models.h"
#include "headers/scene.h"
#include <unordered_set>
#include <filesystem>

std::vector<ModelAsset> assets;

bool is_model_file(const std::filesystem::path& p) {
    std::string ext = p.extension().string();
    for (auto& c : ext) c = (char)tolower(c);

    return ext == ".obj" || ext == ".glb" || ext == ".gltf" || ext == ".fbx" || ext == ".iqm";
}

void load_models() {
    ModelAsset cube_asset;
    cube_asset.name = "Cube";
    cube_asset.type = CUBE;
    cube_asset.is_procedural = true;
    cube_asset.generator = [](int) { return LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f)); };
    assets.push_back(cube_asset);

    ModelAsset sphere_asset;
    sphere_asset.name = "Sphere";
    sphere_asset.type = SPHERE;
    sphere_asset.is_procedural = true;
    sphere_asset.generator = [](int seg) { return LoadModelFromMesh(GenMeshSphere(1.0f, seg, seg)); };
    assets.push_back(sphere_asset);

    ModelAsset cone_asset;
    cone_asset.name = "Cone";
    cone_asset.type = CONE;
    cone_asset.is_procedural = true;
    cone_asset.generator = [](int seg) { return LoadModelFromMesh(GenMeshCone(1.0f, 1.0f, seg)); };
    assets.push_back(cone_asset);

    ModelAsset cylinder_asset;
    cylinder_asset.name = "Cylinder";
    cylinder_asset.type = CYLINDER;
    cylinder_asset.is_procedural = true;
    cylinder_asset.generator = [](int seg) { return LoadModelFromMesh(GenMeshCylinder(1.0f, 1.0f, seg)); };
    assets.push_back(cylinder_asset);

    ModelAsset hemisphere_asset;
    hemisphere_asset.name = "HemiSphere";
    hemisphere_asset.type = HEMISPHERE;
    hemisphere_asset.is_procedural = true;
    hemisphere_asset.generator = [](int seg) { return LoadModelFromMesh(GenMeshHemiSphere(1.0f, seg, seg)); };
    assets.push_back(hemisphere_asset);

    ModelAsset torus_asset;
    torus_asset.name = "Torus";
    torus_asset.type = TORUS;
    torus_asset.is_procedural = true;
    torus_asset.generator = [](int seg) { return LoadModelFromMesh(GenMeshTorus(1.0f, 1.0f, seg, seg)); };
    assets.push_back(torus_asset);
}

void update_model(Entity* e) {
    if (!e || !e->asset || !e->asset->is_procedural || !e->asset->generator) return;
    if (e->model.meshCount > 0) UnloadModel(e->model);

    int max_seg = 125;
    if (e->type == SPHERE || e->type == HEMISPHERE) max_seg = 100;

    if (e->segments < 3)        e->segments = 3;
    if (e->segments > max_seg)  e->segments = max_seg;

    e->model = e->asset->generator(e->segments);
}

void unload_models() {
    std::unordered_set<void*> released_meshes;

    for (auto& asset : assets) {
        if (!asset.is_procedural && asset.loaded_model.meshCount > 0 && asset.loaded_model.meshes) {
            void* mesh_ptr = static_cast<void*>(asset.loaded_model.meshes);
            if (released_meshes.insert(mesh_ptr).second) {
                UnloadModel(asset.loaded_model);
            }
        }
        asset.loaded_model = {0};
    }

    assets.clear();
}

void load_external_models(std::string project_path) {
    namespace fs = std::filesystem;

    fs::path resource_dir = fs::path(project_path) / "resources";
    if (!fs::exists(resource_dir)) fs::create_directories(resource_dir);

    for (auto& entry : fs::directory_iterator(resource_dir)) {
        if (!entry.is_regular_file()) continue;
        if (!is_model_file(entry.path())) continue;

        ModelAsset asset;
        asset.name = entry.path().filename().string();
        asset.is_procedural = false;
        asset.filepath = entry.path().string();

        asset.loaded_model = LoadModel(asset.filepath.c_str());

        assets.push_back(asset);
    }
}

void refresh_models(std::string project_path, Scene& scene) {
    namespace fs = std::filesystem;

    std::unordered_map<std::string, Model> old;

    for (auto& a : assets) {
        if (!a.is_procedural && a.loaded_model.meshCount > 0)
            old[a.name] = a.loaded_model;
    }

    std::vector<ModelAsset> next;

    for (auto& a : assets)
        if (a.is_procedural)
            next.push_back(a);

    fs::path resource_dir = fs::path(project_path) / "resources";
    if (!fs::exists(resource_dir)) fs::create_directories(resource_dir);

    for (auto& entry : fs::directory_iterator(resource_dir)) {
        if (!is_model_file(entry.path())) continue;

        std::string name = entry.path().filename().string();

        ModelAsset a;
        a.name = name;
        a.is_procedural = false;
        a.filepath = entry.path().string();

        if (old.count(name)) {
            a.loaded_model = old[name];
            old.erase(name);
        } 
        
        else {
            a.loaded_model = LoadModel(a.filepath.c_str());
        }

        next.push_back(a);
    }

    for (auto& [_, m] : old) {
        UnloadModel(m);
    }

    assets = std::move(next);

    for (auto& entity : scene.entities) {
        if (!entity.asset_name.empty()) {
            entity.asset = nullptr;
            for (auto& a : assets) {
                if (a.name == entity.asset_name) {
                    entity.asset = &a;
                    break;
                }
            }
        }
    }
}