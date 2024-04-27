#version 330 core

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 InTex;
layout (location = 2) in vec3 aNormal;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;
uniform float minHeight;
uniform float maxHeight;

out vec4 Color;
out vec2 Tex;
out vec3 FragPos;
out vec3 Normal;
out vec3 aPos;

void main()
{
    gl_Position = projection * view * model * vec4(Position, 1.0);
    float DeltaHeight = maxHeight - minHeight ;
    float HeightRatio = (-Position.y - minHeight) / DeltaHeight;
    float c = HeightRatio * 0.8 + 0.2;
    Color = vec4(c, c, c, 1.0);
    Tex = InTex;
    FragPos = vec3(model * vec4(Position, 1.0));
    Normal = aNormal;
    aPos = Position;
}
