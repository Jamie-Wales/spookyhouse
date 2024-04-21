
#ifndef INCLUDE_BOUNDINGBOX_H_
#define INCLUDE_BOUNDINGBOX_H_
#include <assimp/aabb.h>
#include <glm/glm.hpp>
#include <vector>

struct BoundingBox {
    glm::vec3 min;
    glm::vec3 max;

    BoundingBox()
        : min(glm::vec3(0.0f))
        , max(glm::vec3(0.0f))
    {
    }
    BoundingBox(const aiAABB& aabb)
    {
        min = glm::vec3(aabb.mMin.x - 0.005, aabb.mMin.y- 0.005, aabb.mMin.z - 0.00);
        max = glm::vec3(aabb.mMax.x + 0.005, aabb.mMax.y + 0.005, aabb.mMax.z + 0.005);
    }

    std::vector<glm::vec3> getCorners() const
    {
        return {
            glm::vec3(min.x, min.y, min.z),
            glm::vec3(max.x, min.y, min.z),
            glm::vec3(min.x, max.y, min.z),
            glm::vec3(min.x, min.y, max.z),
            glm::vec3(max.x, max.y, max.z),
            glm::vec3(min.x, max.y, max.z),
            glm::vec3(max.x, min.y, max.z),
            glm::vec3(max.x, max.y, min.z)
        };
    }

    void updateCorners(glm::mat4 model)
    {
        std::vector<glm::vec3> corners = getCorners();
        for (auto& corner : corners) {
            glm::vec4 transformed = model * glm::vec4(corner, 1.0f);
            corner = glm::vec3(transformed);
        }
        min = max = corners[0];
        for (auto& corner : corners) {
            min = glm::min(min, corner);
            max = glm::max(max, corner);
        }
    }
};

#endif // INCLUDE_INCLUDE_BOUNDINGBOX_H_
