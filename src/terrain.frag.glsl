
#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 Tex;
in vec4 Color;
in vec4 FragPosLightSpace;

uniform vec3 lightPos;
uniform vec3 viewPos;
out vec4 FragColor;
uniform sampler2D gTextureHeight0;
uniform sampler2D gTextureHeight1;
uniform sampler2D gTextureHeight2;
uniform sampler2D gTextureHeight3;
uniform sampler2D shadowMap;
uniform float shininess;
uniform bool torch;

uniform float gHeight0 = 20.0;
uniform float gHeight1 = 30.0;
uniform float gHeight2 = 60.0;
uniform float gHeight3 = 80.0;

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
float ShadowCalculation(vec4 fragPosLightSpace);
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec4 CalcTexColor();

float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    const int halfkernelWidth = 3;
    for(int x = -halfkernelWidth; x <= halfkernelWidth; ++x)
    {
        for(int y = -halfkernelWidth; y <= halfkernelWidth; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.5;
        }
    }

    shadow /= ((halfkernelWidth*2+1)*(halfkernelWidth*2+1));

    if (projCoords.z > 1)
        shadow = 0.0;
    return shadow;
}

uniform DirLight dirLight;
uniform SpotLight spotLight;
void main() {
   vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result = CalcDirLight(dirLight, normal, viewDir);
    vec4 TexColor = CalcTexColor();
    if (torch) {
        result += CalcSpotLight(spotLight, normal, FragPos, viewDir);
    }
    FragColor = TexColor * vec4(result, 1.0);

}

vec4 CalcTexColor() {
    float Height = FragPos.y;
    if (Height < gHeight0) {
       return texture(gTextureHeight0, Tex);
    } else if (Height < gHeight1) {
       return mix(texture(gTextureHeight0, Tex), texture(gTextureHeight1, Tex), smoothstep(gHeight0, gHeight1, Height));
    } else if (Height < gHeight2) {
       return mix(texture(gTextureHeight1, Tex), texture(gTextureHeight2, Tex), smoothstep(gHeight1, gHeight2, Height));
    } else if (Height < gHeight3) {
       return mix(texture(gTextureHeight2, Tex), texture(gTextureHeight3, Tex), smoothstep(gHeight2, gHeight3, Height));
    } else {
       return texture(gTextureHeight3, Tex);
    }
}
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    vec3 ambient = light.ambient;
    vec3 diffuse = light.diffuse * diff;
    vec3 specular = light.specular * spec;
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    float ambient_scale = 0.5;
    vec3 ambient = light.ambient * ambient_scale;
    vec3 diffuse = light.diffuse * diff;
    vec3 specular = light.specular * spec;
    float shadow = ShadowCalculation(FragPosLightSpace);
    return (ambient + (1.0 - shadow) * (diffuse + specular));
}

