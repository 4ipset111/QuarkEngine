#version 330

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform int useTexture;

out vec4 finalColor;

#define MAX_LIGHTS 4
#define LIGHT_DIRECTIONAL 0
#define LIGHT_POINT 1

struct Light {
    int enabled;
    int type;
    vec3 position;
    vec3 target;
    vec4 color;

    float intensity;
    float range;
};

uniform Light lights[MAX_LIGHTS];
uniform vec4 ambient;
uniform vec3 viewPos;

uniform vec3 emissionColor;
uniform float emissionPower;

void main()
{
    vec4 texelColor = vec4(1.0);

    if (useTexture == 1)
        texelColor = texture(texture0, fragTexCoord);
        
    vec3 normal = normalize(fragNormal);
    vec3 viewD = normalize(viewPos - fragPosition);
    vec3 lightAccum = vec3(0.0);
    vec3 specular = vec3(0.0);
    vec4 tint = colDiffuse * fragColor;

    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (lights[i].enabled == 1)
        {
            vec3 lightDir;
            if (lights[i].type == LIGHT_DIRECTIONAL)
                lightDir = -normalize(lights[i].target - lights[i].position);
            else
                lightDir = normalize(lights[i].position - fragPosition);

            float NdotL = max(dot(normal, lightDir), 0.0);
            float dist = length(lights[i].position - fragPosition);
            float attenuation = clamp(1.0 - (dist / lights[i].range), 0.0, 1.0);
            attenuation *= attenuation;
            lightAccum += lights[i].color.rgb * NdotL * attenuation * lights[i].intensity;

            if (NdotL > 0.0)
            {
                float spec = pow(max(dot(viewD, reflect(-lightDir, normal)), 0.0), 16.0);
                specular += spec * attenuation * lights[i].intensity;
            }
        }
    }

    vec3 ambientLight = ambient.rgb * 0.15;                          // always applied
    vec3 color = texelColor.rgb * tint.rgb * (ambientLight + lightAccum) + specular;
    color += emissionColor * emissionPower;

    finalColor = vec4(color, texelColor.a);
    finalColor = pow(finalColor, vec4(1.0/2.2));
}