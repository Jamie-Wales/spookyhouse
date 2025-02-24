//
// Created by Jamie Wales on 21/04/2024.
//

#ifndef SPOOKY_RESOLVER_H
#define SPOOKY_RESOLVER_H

#include "Collider.h"
#include <glm/glm.hpp>

class Plane {
public:
    float equation[4];
    glm::vec3 origin;
    glm::vec3 normal;

    Plane(const glm::vec3& p1, const glm::vec3& p2,
        const glm::vec3& p3)
    {
        normal = glm::cross((p2 - p1), (p3 - p1));
        normal = glm::normalize(normal);
        origin = p1;
        equation[0] = normal.x;
        equation[1] = normal.y;
        equation[2] = normal.z;
        equation[3] = -(normal.x * origin.x + normal.y * origin.y
            + normal.z * origin.z);
    }

    [[nodiscard]] bool isFrontFacingTo(const glm::vec3& direction) const
    {
        return glm::dot(direction, normal) > 0;
    }

    [[nodiscard]] double signedDistanceTo(const glm::vec3& point) const
    {
        return glm::dot(point, normal) + equation[3];
    }
};
typedef unsigned int uint32;
#define in(a) ((uint32&)a)

bool checkTriangle(const glm::vec3& point,
    const glm::vec3& pa, const glm::vec3& pb, const glm::vec3& pc)
{
    glm::vec3 e10 = pb - pa;
    glm::vec3 e20 = pc - pa;
    float a = glm::dot(e10, e10);
    float b = glm::dot(e20, e10);
    float c = glm::dot(e20, e20);
    float ac_bb = (a * c) - (b * b);
    glm::vec3 vp(point.x - pa.x, point.y - pa.y, point.z - pa.z);
    float d = glm::dot(vp, e10);
    float e = glm::dot(vp, e20);
    float x = (d * c) - (e * b);
    float y = (e * a) - (d * b);
    float z = x + y - ac_bb;
    return ((in(z) & ~(in(x) | in(y))) & 0x80000000);
}

bool checkAABBCollision(const BoundingBox& box1, BoundingBox& box2)
{
    // add float for padding
    return (box1.min.x - 0.1 <= box2.max.x && box1.max.x + 0.1 >= box2.min.x) && (box1.min.y - 0.1 <= box2.max.y && box1.max.y + 0.1 >= box2.min.y) && (box1.min.z - 0.1 <= box2.max.z && box1.max.z + 0.1 >= box2.min.z);
}

void checkCollisions(const std::shared_ptr<CollisionPacket>& colPacket, std::shared_ptr<Model>& otherModel)
{
}

#endif
