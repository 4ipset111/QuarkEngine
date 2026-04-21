#include "headers/models.h"
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
    cube_asset.isProcedural = true;
    cube_asset.generator = [](int) { return LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f)); };
    assets.push_back(cube_asset);

    ModelAsset sphere_asset;
    sphere_asset.name = "Sphere";
    sphere_asset.type = SPHERE;
    sphere_asset.isProcedural = true;
    sphere_asset.generator = [](int seg) { return LoadModelFromMesh(GenMeshSphere(1.0f, seg, seg)); };
    assets.push_back(sphere_asset);

    ModelAsset cone_asset;
    cone_asset.name = "Cone";
    cone_asset.type = CONE;
    cone_asset.isProcedural = true;
    cone_asset.generator = [](int seg) { return LoadModelFromMesh(GenMeshCone(1.0f, 1.0f, seg)); };
    assets.push_back(cone_asset);

    ModelAsset cylinder_asset;
    cylinder_asset.name = "Cylinder";
    cylinder_asset.type = CYLINDER;
    cylinder_asset.isProcedural = true;
    cylinder_asset.generator = [](int seg) { return LoadModelFromMesh(GenMeshCylinder(1.0f, 1.0f, seg)); };
    assets.push_back(cylinder_asset);

    ModelAsset hemisphere_asset;
    hemisphere_asset.name = "HemiSphere";
    hemisphere_asset.type = HEMISPHERE;
    hemisphere_asset.isProcedural = true;
    hemisphere_asset.generator = [](int seg) { return LoadModelFromMesh(GenMeshHemiSphere(1.0f, seg, seg)); };
    assets.push_back(hemisphere_asset);

    ModelAsset torus_asset;
    torus_asset.name = "Torus";
    torus_asset.type = TORUS;
    torus_asset.isProcedural = true;
    torus_asset.generator = [](int seg) { return LoadModelFromMesh(GenMeshTorus(1.0f, 1.0f, seg, seg)); };
    assets.push_back(torus_asset);
}

void update_model(Entity* e) {
    if (!e || !e->asset || !e->asset->isProcedural || !e->asset->generator) return;
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
        if (!asset.isProcedural && asset.loadedModel.meshCount > 0 && asset.loadedModel.meshes) {
            void* mesh_ptr = static_cast<void*>(asset.loadedModel.meshes);
            if (released_meshes.insert(mesh_ptr).second) {
                UnloadModel(asset.loadedModel);
            }
        }
        asset.loadedModel = {0};
    }

    assets.clear();
}

void load_external_models() {
    namespace fs = std::filesystem;

    if (!fs::exists("assets")) fs::create_directories("assets");

    for (auto& entry : fs::directory_iterator("assets")) {
        if (!entry.is_regular_file()) continue;
        if (!is_model_file(entry.path())) continue;

        ModelAsset asset;
        asset.name = entry.path().filename().string();
        asset.isProcedural = false;
        asset.filepath = entry.path().string();

        asset.loadedModel = LoadModel(asset.filepath.c_str());

        assets.push_back(asset);
    }
}

void refresh_models() {
    namespace fs = std::filesystem;

    std::unordered_map<std::string, Model> old;

    for (auto& a : assets) {
        if (!a.isProcedural && a.loadedModel.meshCount > 0)
            old[a.name] = a.loadedModel;
    }

    std::vector<ModelAsset> next;

    for (auto& a : assets)
        if (a.isProcedural)
            next.push_back(a);

    for (auto& entry : fs::directory_iterator("assets")) {
        if (!is_model_file(entry.path())) continue;

        std::string name = entry.path().filename().string();

        ModelAsset a;
        a.name = name;
        a.isProcedural = false;
        a.filepath = entry.path().string();

        if (old.count(name)) {
            a.loadedModel = old[name];
            old.erase(name);
        } 
        
        else {
            a.loadedModel = LoadModel(a.filepath.c_str());
        }

        next.push_back(a);
    }

    for (auto& [_, m] : old) {
        UnloadModel(m);
    }

    assets = std::move(next);
}