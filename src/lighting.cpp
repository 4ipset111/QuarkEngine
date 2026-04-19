#include "headers/lighting.h"

void update_lighting(Shader shader, Lighting& l) {
    l.light.position = l.position;
    l.light.target   = l.target;
    l.light.color    = l.color;
    l.light.enabled  = l.enabled;

    float intensity = l.intensity;
    float range = l.range;

    int intensity_loc = GetShaderLocation(shader, TextFormat("lights[%i].intensity", l.id));
    int range_loc = GetShaderLocation(shader, TextFormat("lights[%i].range", l.id));

    SetShaderValue(shader, intensity_loc, &intensity, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, range_loc, &range, SHADER_UNIFORM_FLOAT);

    UpdateLightValues(shader, l.light);
}

int allocate_light_id()
{
    static int light_id_counter = 0;
    return light_id_counter++;
}

Lighting create_lighting(Vector3 pos, Color color)
{
    Lighting l = {};
    l.position = pos;
    l.target = Vector3Zero();
    l.color = color;
    l.enabled = true;
    l.intensity = 1.0f;
    l.range = 5.0f;
    return l;
}
