#ifndef INCLUDE_UTILS_VERTEX_H_
#define INCLUDE_UTILS_VERTEX_H_

#include "Terrain.h"
#include <glm/glm.hpp>
struct Vertex {
    glm::vec3 position;
    void init(Terrain& terrain, int x, int z, float worldscale)
    {
        position = glm::vec3((-z) * worldscale, -terrain.getHeight(x, z), (-x) * worldscale);
    }
};

#endif // INCLUDE_UTILS_VERTEX_H_
