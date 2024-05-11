
#ifndef INCLUDE_BOUNDINGBOX_H_
#define INCLUDE_BOUNDINGBOX_H_
#define GLM_ENABLE_EXPERIMENTAL

#include <assimp/aabb.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <vector>
class BoundingBox {
public:
    glm::vec3 position; // Center of the bounding box
    glm::vec3 min, max; // Min and max corners
    float pitch, yaw, roll = 0;
    glm::mat4 rotation = glm::mat4(1.0f);
    glm::vec3 extents; // Half-dimensions of the box
    glm::vec3 axis[3];
    BoundingBox() = default;
    BoundingBox(const aiAABB& aabb)
        : position((glm::vec3(aabb.mMin.x, aabb.mMin.y, aabb.mMin.z) + glm::vec3(aabb.mMax.x, aabb.mMax.y, aabb.mMax.z)) * 0.5f)
        , extents((glm::vec3(aabb.mMax.x, aabb.mMax.y, aabb.mMax.z) - glm::vec3(aabb.mMin.x, aabb.mMin.y, aabb.mMin.z)) * 0.5f)
        , axis { glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f) }
    {
    }

    BoundingBox(const glm::vec3& cameraPosition)
        : position(cameraPosition)
        , extents(glm::vec3(1.0f))
        , axis { glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f) }
    {
    }


    bool intersects(const BoundingBox &other, float scale = 0.0f) const {
        glm::vec3 scaledMin = min - glm::vec3(scale);
        glm::vec3 scaledMax = max + glm::vec3(scale);

        glm::vec3 otherScaledMin = other.min - glm::vec3(scale);
        glm::vec3 otherScaledMax = other.max + glm::vec3(scale);

        return (scaledMin.x <= otherScaledMax.x && scaledMax.x >= otherScaledMin.x) &&
               (scaledMin.y <= otherScaledMax.y && scaledMax.y >= otherScaledMin.y) &&
               (scaledMin.z <= otherScaledMax.z && scaledMax.z >= otherScaledMin.z);
    }
      bool intersects(const glm::vec3 &point) const {
        return (point.x >= min.x && point.x <= max.x) &&
               (point.y >= min.y && point.y <= max.y) &&
               (point.z >= min.z && point.z <= max.z);
    }

    void updateRotation()
    {
        rotation = glm::rotate(glm::mat4(1.0f), glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f))
            * glm::rotate(glm::mat4(1.0f), glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f))
            * glm::rotate(glm::mat4(1.0f), glm::radians(roll), glm::vec3(0.0f, 0.0f, 1.0f));
        for (int i = 0; i < 3; ++i) {
            axis[i] = glm::normalize(glm::mat3(rotation) * axis[i]);
        }
    }
    void transform(glm::mat4 mat)
    {
        position = glm::vec3(mat * glm::vec4(position, 1.0f));
        for (int i = 0; i < 3; ++i) {
            axis[i] = glm::normalize(glm::mat3(mat) * axis[i]);
        }
    }
    void rotate(float dPitch, float dYaw, float dRoll)
    {
        pitch += dPitch;
        yaw += dYaw;
        roll += dRoll;
        updateRotation();
    }

    void updateAABB()
    {
        auto corners = getCorners();
        min = corners[0];
        max = corners[0];
        for (const auto& corner : corners) {
            min = glm::min(min, corner);
            max = glm::max(max, corner);
        }
    }

    std::vector<glm::vec3> getCorners() const
    {
        std::vector<glm::vec3> corners(8);
        glm::vec3 vertex[8] = {
            glm::vec3(-extents.x, -extents.y, -extents.z),
            glm::vec3(extents.x, -extents.y, -extents.z),
            glm::vec3(extents.x, extents.y, -extents.z),
            glm::vec3(-extents.x, extents.y, -extents.z),
            glm::vec3(-extents.x, -extents.y, extents.z),
            glm::vec3(extents.x, -extents.y, extents.z),
            glm::vec3(extents.x, extents.y, extents.z),
            glm::vec3(-extents.x, extents.y, extents.z)
        };

        for (int i = 0; i < 8; i++) {
            corners[i] = position + vertex[i].x * axis[0] + vertex[i].y * axis[1] + vertex[i].z * axis[2];
        }
        return corners;
    }

    void translate(const glm::vec3 translation)
    {
        transform(glm::translate(glm::mat4(1.0f), translation));
    }

    void updateDifference(glm::vec3 diff)
    {
        diff = diff - position;
        position += diff;
        transform(glm::translate(glm::mat4(1.0f), diff));
    }

    void setCenterFromAABB(const glm::vec3& minVal, const glm::vec3& maxVal)
    {
        position = (minVal + maxVal) * 0.5f;
        min = minVal;
        max = maxVal;
    }
};
;

#endif // INCLUDE_INCLUDE_BOUNDINGBOX_H_
