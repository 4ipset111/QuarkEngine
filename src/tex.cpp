#include "headers/tex.h"

std::vector<TextureOption> texture_options;

void load_textures() {
    texture_options.clear();
    texture_options.push_back({ "None", {0} });

    for (const auto& entry : std::filesystem::directory_iterator("assets")) {
        if (!entry.is_regular_file()) continue;
        std::string path = entry.path().string();

        if (path.substr(path.size() - 4) == ".png") {
            Texture2D tex = LoadTexture(path.c_str());
            texture_options.push_back({ entry.path().filename().string(), tex });
        }
    }
}

void apply_texture_repeat(Entity &e) {
    for (int m = 0; m < e.model.meshCount; m++) {
        Mesh &mesh = e.model.meshes[m];
        if (!mesh.texcoords) continue;

        for (int i = 0; i < mesh.vertexCount; i++) {
            float u, v;

            if (e.auto_uv) {
                Vector3 pos = {
                    mesh.vertices[i*3+0],
                    mesh.vertices[i*3+1],
                    mesh.vertices[i*3+2]
                };

                Vector3 normal = {
                    mesh.normals[i*3+0],
                    mesh.normals[i*3+1],
                    mesh.normals[i*3+2]
                };

                float ax = fabs(normal.x);
                float ay = fabs(normal.y);
                float az = fabs(normal.z);

                float sx = e.scale.x;
                float sy = e.scale.y;
                float sz = e.scale.z;

                if (ay > ax && ay > az) {
                    u = pos.x * sx;
                    v = pos.z * sz;
                } 
                
                else if (ax > az) {
                    u = pos.z * sz;
                    v = pos.y * sy;
                } 
                
                else {
                    u = pos.x * sx;
                    v = pos.y * sy;
                }

                u *= e.uv_scale_vec.x;
                v *= e.uv_scale_vec.y;
            } else {
                if (m >= e.original_texcoords.size()) continue;
                auto& base = e.original_texcoords[m];

                u = base[i*2+0] * e.texture_repeat_u * e.scale.x;
                v = base[i*2+1] * e.texture_repeat_v * e.scale.y;
            }

            mesh.texcoords[i*2+0] = u;
            mesh.texcoords[i*2+1] = v;
        }

        UpdateMeshBuffer(mesh, 1, mesh.texcoords, mesh.vertexCount * 2 * sizeof(float), 0);
    }
}

void store_uv(Entity* e) {
    e->original_texcoords.clear();

    for (int m = 0; m < e->model.meshCount; m++) {
        Mesh& mesh = e->model.meshes[m];

        if (!mesh.texcoords) {
            e->original_texcoords.push_back({});
            continue;
        }

        std::vector<float> uv(mesh.vertexCount * 2);
        memcpy(uv.data(), mesh.texcoords, uv.size() * sizeof(float));

        e->original_texcoords.push_back(uv);
    }
}

void draw_entity_with_texture(Entity& e) {
    if (e.texture.id != 0) {
        for (int i = 0; i < e.model.materialCount; i++)
            e.model.materials[i].maps[MATERIAL_MAP_DIFFUSE].texture = e.texture;

        if (e.texture_stretch) {
            for (int m = 0; m < e.model.meshCount; m++) {
                Mesh& mesh = e.model.meshes[m];
                if (!mesh.texcoords || m >= e.original_texcoords.size()) continue;

                memcpy(mesh.texcoords, e.original_texcoords[m].data(), mesh.vertexCount * 2 * sizeof(float));
                UpdateMeshBuffer(mesh, 1, mesh.texcoords, mesh.vertexCount * 2 * sizeof(float), 0);
            }
        } 
        
        else {
            apply_texture_repeat(e);
        }
    }

    rlPushMatrix();
    rlTranslatef(e.position.x, e.position.y, e.position.z);

    rlRotatef(e.rotation.x, 1, 0, 0);
    rlRotatef(e.rotation.y, 0, 1, 0);
    rlRotatef(e.rotation.z, 0, 0, 1);
    
    rlScalef(e.scale.x, e.scale.y, e.scale.z);
    DrawModel(e.model, {0,0,0}, 1.0f, e.color);
    DrawModelWires(e.model, {0,0,0}, 1.0f, e.outline_color);
    rlPopMatrix();
}