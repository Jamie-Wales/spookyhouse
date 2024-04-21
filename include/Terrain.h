#ifndef INCLUDE_INCLUDE_TERRAIN_H_
#define INCLUDE_INCLUDE_TERRAIN_H_

#include "./utils/Array2D.h"
#include "Shader.h"
#include <GL/glew.h>
class Terrain {
public:
    int width;
    int height;
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    Shader terrainShader = Shader("../src/terrain.vert.glsl", "../src/terrain.frag.glsl");
    Terrain(float scale);
    void LoadFromFile(const std::string& filename);
    void render(glm::mat4 proj, glm::mat4 view) const;
    float getHeight(int x, int z);
    void populateBuffer();

protected:
    int terrainSize;
    Array2D<float> heightMap;

private:
    float scale = 1.0f;
};

#endif // INCLUDE_INCLUDE_TERRAIN_H_
