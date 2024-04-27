#ifndef INCLUDE_INCLUDE_TERRAIN_H_
#define INCLUDE_INCLUDE_TERRAIN_H_

#include "./utils/Array2D.h"
#include "Shader.h"
#include "utils/Texture.h"
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
    float textureScale;
    float FIRFilterSinglePoint(int x, int z, float preval, float filter);
    std::vector<std::shared_ptr<Texture>> textures = {};
    void ApplyFirFilter(float filter);
    glm::vec3 terposition;
    float getTerPosition(int x, int z);
    float getWorldHeight();
    Shader terrainShader = Shader("../src/terrain.vert.glsl", "../src/terrain.frag.glsl");
    explicit Terrain(float scale, std::initializer_list<const std::string> textureFiles, float textureScale);
    void LoadFromFile(const std::string& filename);
    void render();
    float getHeight(int x, int z) const;
    void populateBuffer();
    void CreateFaultFormation(int terrainSize, int iterations, float minHeight, float maxHeight);
    void faultFormationTerrain(int iterations, float minHeight, float maxHeight);

    float GetHeightInterpolated(float x, float z) const
    {
        float X0Z0Height = this->getHeight((int)x, (int)z);

        if (((int)x + 1 >= terrainSize) || ((int)z + 1 >= terrainSize)) {
            return X0Z0Height;
        }

        float X1Z0Height = this->getHeight((int)x + 1, (int)z);
        float X0Z1Height = this->getHeight((int)x, (int)z + 1);
        float X1Z1Height = this->getHeight((int)x + 1, (int)z + 1);

        float FactorX = x - floorf(x);

        float InterpolatedBottom = (X1Z0Height - X0Z0Height) * FactorX + X0Z0Height;
        float InterpolatedTop = (X1Z1Height - X0Z1Height) * FactorX + X0Z1Height;

        float FactorZ = z - floorf(z);

        float FinalHeight = (InterpolatedTop - InterpolatedBottom) * FactorZ + InterpolatedBottom;

        return FinalHeight;
    }

protected:
    Array2D<float> heightMap;

private:
    float scale = 1.0f;
};

#endif // INCLUDE_INCLUDE_TERRAIN_H_
