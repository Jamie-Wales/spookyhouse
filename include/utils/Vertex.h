#ifndef INCLUDE_UTILS_VERTEX_H_
#define INCLUDE_UTILS_VERTEX_H_

#include "Terrain.h"
#include <glm/glm.hpp>
struct Vertex {
    glm::vec3 position;
    glm::vec2 tex;
    glm::vec3 normal;

    glm::vec3 getNormal()
    {
        return normal;
    }
    float safeGetHeight(Terrain& terrain, int x, int z)
    {
        x = std::max(0, std::min(x, terrain.terrainSize - 1));
        z = std::max(0, std::min(z, terrain.terrainSize - 1));
        return terrain.getHeight(x, z);
    }
    void init(Terrain& terrain, int x, int z)
    {
        position = glm::vec3(x, -terrain.getHeight(x, z), z);
        float s = (float)terrain.terrainSize;
        float nZ = z / s;
        float nX = x / s;
        tex = glm::vec2(nZ * terrain.textureScale, nX * terrain.textureScale);

        float heightL = safeGetHeight(terrain, x - 1, z);
        float heightR = safeGetHeight(terrain, x + 1, z);
        float heightD = safeGetHeight(terrain, x, z - 1);
        float heightU = safeGetHeight(terrain, x, z + 1);

        glm::vec3 v1 = glm::vec3(2.0f, heightR - heightL, 0.0f);
        glm::vec3 v2 = glm::vec3(0.0f, heightU - heightD, 2.0f);

        glm::vec3 normal = glm::cross(v1, v2);
        normal = glm::normalize(normal);
        this->normal = normal;
    }
};

#endif // INCLUDE_UTILS_VERTEX_
