#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} vs_out;

uniform bool isInstanced;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;
uniform mat4 aInstanceMatrix;

void main() {
    if (isInstanced) {
        vs_out.FragPos = vec3(aInstanceMatrix * vec4(aPos, 1.0));
        vs_out.Normal = mat3(transpose(inverse(aInstanceMatrix))) * aNormal;
        vs_out.TexCoords = aTexCoords;
        vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
        gl_Position = projection * view * aInstanceMatrix * vec4(aPos, 1.0f); 
    } else {
        vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
        vs_out.Normal = mat3(transpose(inverse(model))) * aNormal;
        vs_out.TexCoords = aTexCoords;
        vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
        gl_Position = projection * view * model * vec4(aPos, 1.0);
    }
}
