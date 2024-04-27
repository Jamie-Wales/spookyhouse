#ifndef INCLUDE_INCLUDE_TERRAIN_H_
#define INCLUDE_INCLUDE_TERRAIN_H_

#include "./utils/Array2D.h"
#include "Shader.h"
#include <GL/glew.h>
struct TerrainPoint {
    int x = 0;
    int z = 0;

    void print()
    {
        std::cout << x << z << std::endl;
    }
    bool isEqual(TerrainPoint& p)
    {
        return ((x == p.x) && (z == p.z));
    }
};

class Terrain {
public:
    int width;
    int height;
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    int terrainSize;
    float minHeight;
    float maxHeight;
    float FIRFilterSinglePoint(int x, int z, float preval, float filter);
    void ApplyFirFilter(float filter);
    glm::vec3 terposition;
    float getTerPosition(int x, int z);
    Shader terrainShader = Shader("../src/terrain.vert.glsl", "../src/terrain.frag.glsl");
    explicit Terrain(float scale);
    void LoadFromFile(const std::string& filename);
    void render(glm::mat4 proj, glm::mat4 view) const;
    float getHeight(int x, int z);
    void populateBuffer();
    void CreateFaultFormation(int terrainSize, int iterations, float minHeight, float maxHeight);
    void faultFormationTerrain(int iterations, float minHeight, float maxHeight);

protected:
    Array2D<float> heightMap;

private:
    float scale = 1.0f;
};

#endif // INCLUDE_INCLUDE_TERRAIN_H_
