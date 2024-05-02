

#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 Tex;
in vec4 Color;
in vec3 aPos;
uniform vec3 viewPos;

out vec4 FragColor;

uniform sampler2D gTextureHeight0;
uniform sampler2D gTextureHeight1;
uniform sampler2D gTextureHeight2;
uniform sampler2D gTextureHeight3;

uniform float gHeight0 = 20.0;
uniform float gHeight1 = 30.0;
uniform float gHeight2 = 60.0;
uniform float gHeight3 = 80.0;

uniform vec3 lightDir; 
uniform vec3 lightColor; 


vec4 CalcTexColor();
vec3 CalcLighting(vec3 normal, vec3 lightDir, vec3 viewDir);

void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(FragPos- viewPos);
    vec3 lightDirection = normalize(lightDir);

    vec4 TexColor = CalcTexColor(); 
    vec3 lighting = CalcLighting(norm, lightDirection, viewDir);

    FragColor = vec4(TexColor) * vec4(lighting, 1.0);
}

vec4 CalcTexColor() {
    float Height = -aPos.y;
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

vec3 CalcLighting(vec3 normal, vec3 lightDirection, vec3 viewDirection) {
float diff = max(dot(normal, lightDirection), 0.0);
    return lightColor * diff;
}

