#version 410 core
uniform vec4 color; // Add this line for uniform color
out vec4 frcol;

void main()
{
    frcol = color; // Use the uniform color
}
