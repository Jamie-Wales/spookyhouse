
#ifndef INCLUDE_BOUNDINGBOX_H_
#define INCLUDE_BOUNDINGBOX_H_

#include <assimp/aabb.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <vector>

struct BoundingBox {
    glm::vec3 min;
    glm::vec3 max;
    glm::vec3 position {};

    BoundingBox()
        : min(glm::vec3(0.0f))
        , max(glm::vec3(0.0f))
    {
    }

    BoundingBox(const aiAABB& aabb)
    {
        min = glm::vec3(aabb.mMin.x, aabb.mMin.y, aabb.mMin.z);
        max = glm::vec3(aabb.mMax.x, aabb.mMax.y, aabb.mMax.z);
        position = glm::vec3(0.0f);
    }

    [[nodiscard]] std::vector<glm::vec3> getCorners() const
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

    void copyPosition(glm::vec3 newPosition)
    {
        position = newPosition;
    }

    void updatePosition(const glm::vec3& newPosition)
    {
        min += newPosition;
        max += newPosition;
        position += newPosition;
    }
};

#endif // INCLUDE_INCLUDE_BOUNDINGBOX_H_
