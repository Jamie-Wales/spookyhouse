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
        : obj{obj}
          , sphere(sphere) {
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

void prepareCollision(const std::shared_ptr<CollisionPacket> &colPacket,
                      const std::shared_ptr<physics::Object> &object) {
    colPacket->obj = object;
    colPacket->foundCollision = false;
    colPacket->nearestDistance = std::numeric_limits<float>::max();
    if (glm::length(object->velocity) > 0) {
        colPacket->normalizedVelocity = glm::normalize(object->velocity);
    } else {
        colPacket->normalizedVelocity = glm::vec3(0.1f);
    }
}

bool checkSphereCollision(const Sphere &sphere1, const Sphere &sphere2) {
    float radiusSum = sphere1.radius + sphere2.radius;
    float distance = glm::length(sphere1.center - sphere2.center);
    return distance <= radiusSum;
}

bool isFrontFacingTo(const glm::vec3 &direction, const glm::vec3 &normal) {
    return glm::dot(direction, normal) > 0;
}

bool checkVertexCollision(const vertex &vertex, const Sphere &sphere) {
    float distance = glm::length(vertex.position - sphere.center);
    return distance <= sphere.radius;
}

void checkVertexCollisions(const std::shared_ptr<CollisionPacket> &colPacket, const std::vector<vertex> & vertices, const glm::vec3& collisionNormal) {
    for (const auto& vertex : vertices) {
        if (isFrontFacingTo(collisionNormal, glm::normalize(vertex.position - colPacket->sphere.center))) {
            if (checkVertexCollision(vertex, colPacket->sphere)) {
                colPacket->foundCollision = true;
                colPacket->intersectionPoint = vertex.position;
                colPacket->nearestDistance = glm::length(vertex.position - colPacket->sphere.center);
                return;
            }
        }
    }
}


class Collider {
public:
    SweepAndPrune sweepAndPrune;

    void addModel(std::shared_ptr<Model> &model) {
        sweepAndPrune.addModel(model);
    }

    void addCamera(std::shared_ptr<Camera> &camera) {
        sweepAndPrune.addCamera(camera);
    }

    void removeObject(std::shared_ptr<Model> &model) {
        sweepAndPrune.removeModel(model);
    }

    void checkCollision(const std::shared_ptr<CollisionPacket> &colPacket, Sphere &secondMesh) {
        Sphere &collpacketSphere = colPacket->sphere;
    }

    std::vector<BroadCollision>
    broadCollide(std::shared_ptr<Model> &model) {
        sweepAndPrune.UpdateObject(model);
        auto col = sweepAndPrune.getTrueCollisions();
        sweepAndPrune.printTrueCollisions(col);
        return col;
    }

    std::vector<BroadCollision> broadCollide(std::shared_ptr<Camera> camera) {
        sweepAndPrune.updateObject(camera);
        auto col = sweepAndPrune.getTrueCollisions();
        return col;
    }

    void checkCollisions(const std::shared_ptr<CollisionPacket> &colPacket, Sphere &secondMesh) {
        if (checkSphereCollision(colPacket->sphere, secondMesh)) {
            colPacket->foundCollision = true;
            colPacket->intersectionPoint = (colPacket->sphere.center + secondMesh.center) / 2.0f;
            colPacket->nearestDistance = glm::length(colPacket->sphere.center - secondMesh.center);
        }
    }

    Sphere boundingBoxToSphere(const BoundingBox &box) {
        glm::vec3 center = (box.min+ box.max) * 0.5f;
        float radius = glm::length(box.max - center);
        return Sphere{center, radius};
    }

void resolveCollisions(
        std::vector<BroadCollision> broadphasePairs,
        std::shared_ptr<std::unordered_map<int, std::shared_ptr<physics::Object> > > objectMap, float dt) {

        for (auto &pair: broadphasePairs) {
            auto &objA = objectMap->at(pair.modelIdA);
            auto &objB = objectMap->at(pair.modelIdB);
            Sphere sphereA;
            Sphere sphereB;

            if (objA->isCamera) {
                sphereA = Sphere{objA->position, 0.5};
            } else {
                sphereA = boundingBoxToSphere(pair.firstMesh->boundingbox);
            }

            if (objB->isCamera) {
                sphereB = Sphere{objB->position, 0.1};
            } else {
                sphereB = boundingBoxToSphere(pair.secondMesh->boundingbox);
            }

            auto colPacketA = std::make_shared<CollisionPacket>(objA, sphereA);
            auto colPacketB = std::make_shared<CollisionPacket>(objB, sphereB);
            prepareCollision(colPacketA, objA);
            prepareCollision(colPacketB, objB);
            checkCollisions(colPacketA, sphereB);
            checkCollisions(colPacketB, sphereA);

            if (colPacketA->foundCollision || colPacketB->foundCollision) {
                glm::vec3 collisionNormal = glm::normalize(colPacketA->obj->position - colPacketB->obj->position);
                if (!colPacketA->foundCollision && !colPacketB->foundCollision) {
                    continue;
                }
                float relativeVelocity = glm::dot(colPacketB->obj->velocity - colPacketA->obj->velocity, collisionNormal);
                if (relativeVelocity < 0)
                    continue;

                float e = 1.0;
                float j = -(1 + e) * relativeVelocity / (1 / colPacketA->obj->mass + 1 / colPacketB->obj->mass);
                glm::vec3 impulse = j * collisionNormal;

                if (!colPacketA->obj->isStatic) {
                    colPacketA->obj->velocity -= impulse / colPacketA->obj->mass;
                }

                if (!colPacketB->obj->isStatic) {
                    colPacketB->obj->velocity += impulse / colPacketB->obj->mass;
                }

                float penetrationDepth = (sphereA.radius + sphereB.radius) - glm::distance(colPacketA->obj->position, colPacketB->obj->position);

                if (!colPacketA->obj->isStatic) {
                    colPacketA->obj->position += (penetrationDepth * collisionNormal) * 1.0f;
                }

                if (!colPacketB->obj->isStatic) {
                    colPacketB->obj->position -= (penetrationDepth * collisionNormal) * 1.0f;
                }
            }
        }
    }
};
#endif
