#ifndef INCLUDE_UTILS_VERTEX_H_
#define INCLUDE_UTILS_VERTEX_H_

#include "Terrain.h"
#include <glm/glm.hpp>
struct Vertex {
    glm::vec3 position;
    glm::vec2 tex;
    void init(Terrain& terrain, int x, int z)
    {
        position = glm::vec3(-z, -terrain.getHeight(x, z), -x);
    }
};

#endif // INCLUDE_UTILS_VERTEX_H_
