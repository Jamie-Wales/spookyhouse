#ifndef INCLUDE_COLLIDER_H_
#define INCLUDE_COLLIDER_H_

#include "BroadCollision.h"
#include "Object.h"
#include "SweepPrune.h"
#include <memory>
#include <vector>

struct Sphere {
    glm::vec3 center;
    float radius;
};

class CollisionPacket {
public:
    CollisionPacket() = default;
    CollisionPacket(std::shared_ptr<physics::Object> obj, Sphere sphere)
        : obj { obj }
        , sphere(sphere)
    {
        if (obj->velocity != glm::vec3(0)) {
            normalizedVelocity = glm::normalize(obj->velocity);
        } else {
            normalizedVelocity = glm::vec3(0);
        }
    }

    std::shared_ptr<physics::Object> obj;
    Sphere sphere;
    glm::vec3 normalizedVelocity;
    bool foundCollision = false;
    float nearestDistance = std::numeric_limits<float>::max();
    glm::vec3 intersectionPoint;
};

Sphere boundingBoxToSphere(const BoundingBox& box)
{
    glm::vec3 center = (box.min + box.max) * 0.5f;
    float radius = glm::length(box.max - center);
    return Sphere { center, radius };
}
void prepareCollision(const std::shared_ptr<CollisionPacket>& colPacket, const std::shared_ptr<physics::Object>& object)
{
    colPacket->obj = object;
    colPacket->foundCollision = false;
    colPacket->nearestDistance = std::numeric_limits<float>::max();
    if (glm::length(object->velocity) > 0) {
        colPacket->normalizedVelocity = glm::normalize(object->velocity);
    } else {
        colPacket->normalizedVelocity = glm::vec3(0.0f);
    }
}
bool checkSphereCollision(const Sphere& sphere1, const Sphere& sphere2)
{
    float radiusSum = sphere1.radius + sphere2.radius;
    float distance = glm::length(sphere1.center - sphere2.center);
    return distance <= radiusSum;
}
class Collider {
public:
    SweepAndPrune sweepAndPrune;

    void addModel(std::shared_ptr<Model>& model)
    {
        sweepAndPrune.addModel(model);
    }

    void checkCollision(const std::shared_ptr<CollisionPacket>& colPacket, Sphere& secondMesh)
    {
        Sphere& collpacketSphere = colPacket->sphere;
    }
    std::vector<BroadCollision>
    broadCollide(std::shared_ptr<Model>& model)
    {
        sweepAndPrune.UpdateObject(model);
        return sweepAndPrune.getTrueCollisions();
    }

    void checkCollisions(const std::shared_ptr<CollisionPacket>& colPacket, Sphere& secondMesh)
    {
        if (checkSphereCollision(colPacket->sphere, secondMesh)) {
            colPacket->foundCollision = true;
            colPacket->intersectionPoint = (colPacket->sphere.center + secondMesh.center) / 2.0f;
            colPacket->nearestDistance = glm::length(colPacket->sphere.center - secondMesh.center);
        }
    }

    void
    resolveCollisions(
        std::vector<BroadCollision> broadphasePairs,
        std::shared_ptr<std::unordered_map<int, std::shared_ptr<physics::Object>>> objectMap, float dt)
    {
        for (auto& pair : broadphasePairs) {
            auto sphere = boundingBoxToSphere(pair.firstMesh->boundingbox);
            auto sphere2 = boundingBoxToSphere(pair.secondMesh->boundingbox);
            auto colPacketA = std::make_shared<CollisionPacket>();
            auto colPacketB = std::make_shared<CollisionPacket>();
            colPacketA->sphere = sphere;
            colPacketB->sphere = sphere2;
            prepareCollision(colPacketA, objectMap->at(pair.modelIdA));
            prepareCollision(colPacketB, objectMap->at(pair.modelIdB));
            checkCollisions(colPacketA, sphere2);
            checkCollisions(colPacketB, sphere);
            if (colPacketA->foundCollision || colPacketB->foundCollision) {
                glm::vec3 normal = glm::normalize(colPacketB->obj->position - colPacketA->obj->position);
                float relativeVelocity = glm::dot(colPacketB->obj->velocity - colPacketA->obj->velocity, normal);
                if (relativeVelocity < 0)
                    continue;

                float e = 1.0;
                float j = -(1 + e) * relativeVelocity / (1 / colPacketA->obj->mass + 1 / colPacketB->obj->mass);
                glm::vec3 impulse = j * normal;

                colPacketA->obj->velocity += impulse / colPacketA->obj->mass * dt;
                colPacketB->obj->velocity -= impulse / colPacketB->obj->mass * dt;

                float penetrationDepth = colPacketA->sphere.radius + colPacketB->sphere.radius - glm::length(colPacketA->obj->position - colPacketB->obj->position);
                glm::vec3 correction = penetrationDepth * 2.0f * normal;
                colPacketA->obj->position -= correction;
                colPacketB->obj->position += correction;
            }
        }
    }
};

#endif
