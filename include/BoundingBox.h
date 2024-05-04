
#ifndef INCLUDE_BOUNDINGBOX_H_
#define INCLUDE_BOUNDINGBOX_H_
#define GLM_ENABLE_EXPERIMENTAL

#include <assimp/aabb.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <glm/gtx/rotate_vector.hpp>
#include <vector>

class BoundingBox {
public:
    float pitch, yaw, roll;
    glm::mat4 rotation = glm::mat4(1.0f);

    BoundingBox()
            : min(glm::vec3(0.0f)), max(glm::vec3(0.0f)), position(glm::vec3(0.0f)), pitch(0.0f), yaw(0.0f),
              roll(0.0f) {}

   BoundingBox(const glm::vec3 &minVal, const glm::vec3 &maxVal, float pitch = 0.0f, float yaw = 0.0f,
                float roll = 0.0f)
            : min(minVal), max(maxVal), position((minVal + maxVal) * 0.5f), pitch(pitch), yaw(yaw), roll(roll) {}


    BoundingBox(const aiAABB &aabb, glm::vec3 position, float pitch, float yaw, float roll = 0.0f) :
            pitch(pitch), yaw(yaw), roll(roll) {

        min = glm::vec3(aabb.mMin.x, aabb.mMin.y, aabb.mMin.z);
        max = glm::vec3(aabb.mMax.x, aabb.mMax.y, aabb.mMax.z);
        this->position = position;
        updateCorners();

    };

    glm::mat4 getTransformationMatrix() const {
        glm::mat4 trans = glm::translate(glm::mat4(1.0f), position);
        trans = glm::rotate(trans, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
        trans = glm::rotate(trans, glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
        trans = glm::rotate(trans, glm::radians(roll), glm::vec3(0.0f, 0.0f, 1.0f));
        return trans;
    }

    std::vector<glm::vec3> getCorners() {

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

    void updateCorners() {

        auto corners = getCorners();
        glm::mat4 trans = getTransformationMatrix();
        glm::vec3 newMin = glm::vec3(FLT_MAX);
        glm::vec3 newMax = glm::vec3(-FLT_MAX);
        for (auto &corner: corners) {
            corner = glm::vec3(trans * glm::vec4(corner, 1.0f));
            newMin = glm::min(newMin, corner);
            newMax = glm::max(newMax, corner);
        }
        min = newMin;
        max = newMax;
    }

    void translate(const glm::vec3 &translation) {
        position += translation;
        updateCorners();
    }

    void rotate(float dPitch, float dYaw, float dRoll) {
        float pdiff = pitch - dPitch;
        float ydiff = yaw - dYaw;
        min = glm::rotateX(min, pdiff);
        max = glm::rotateX(max, pdiff);
        min = glm::rotateY(min, ydiff);
        max = glm::rotateY(max, ydiff);
    }

    void translateInPlace(const glm::vec3 &translation) {
        position = translation;
        updateCorners();
    }


    void updateCameraPosition(glm::vec3 point) {
        min = point - 5.0f;
        max = point + 5.0f;
    }

    void copyPosition(glm::vec3 newPosition) {
        auto diff = max - min;
        position = newPosition;
        min = newPosition - diff;
        max = newPosition + diff;
    }

    void addPostion(const glm::vec3 &newPosition) {
        min += newPosition;
        max += newPosition;
        position += newPosition;
    }


    void updateDifference(const glm::vec3 newPosition) {
        glm::vec3 difference = newPosition - position;
        min += difference;
        max += difference;
        position = newPosition;
    }

    glm::vec3 position = glm::vec3(0.0);
    glm::vec3 min;
    glm::vec3 max;
};

#endif // INCLUDE_INCLUDE_BOUNDINGBOX_H_
