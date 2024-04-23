#ifndef INCLUDE_COLLIDER_H_
#define INCLUDE_COLLIDER_H_

#include "Object.h"
#include "Resolver.h"
#include "SweepPrune.h"
#include <iostream>
#include <memory>
#include <vector>


struct Sphere {
    glm::vec3 center;
    float radius;
};

class CollisionPacket {
public:
    CollisionPacket(physics::Object obj, Sphere sphere) : obj(obj), sphere(sphere) {
        if (obj.velocity != glm::vec3(0)) {
            normalizedVelocity = glm::normalize(obj.velocity);
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

Sphere boundingBoxToSphere(const BoundingBox &box) {
    glm::vec3 center = (box.min + box.max) * 0.5f;
    float radius = glm::length(box.max - center);
    return Sphere{center, radius};
}

class Collider {
public:
    SweepAndPrune sweepAndPrune;

    void addModel(std::shared_ptr<Model> &model) {
        sweepAndPrune.addModel(model);
    }

    std::vector<std::pair<std::pair<std::shared_ptr<Model>, std::shared_ptr<Mesh>>, std::pair<std::shared_ptr<Model>, std::shared_ptr<Mesh>>>>
    broadCollide(std::shared_ptr<Model> &model) {
        sweepAndPrune.UpdateObject(model);
        sweepAndPrune.getTrueCollisions();

    }

    void resolveCollisions(
            std::vector<std::pair<std::pair<std::shared_ptr<Model>, std::shared_ptr<Mesh>>, std::pair<std::shared_ptr<Model>, std::shared_ptr<Mesh>>>> &broadphasePairs,
            std::unordered_map<int, physics::Object> &objectMap) {

        for (auto &pair: broadphasePairs) {
            Sphere sphere = boundingBoxToSphere(pair.first.second->boundingbox);
            std::shared_ptr<CollisionPacket> colPacketA = std::make_shared<CollisionPacket>(
                    objectMap[pair.first.first->id], sphere);
            Sphere sphere2 = boundingBoxToSphere(objectMap[pair.second.second->boundingbox)

            std::shared_ptr<CollisionPacket> colPacketA = std::make_shared<CollisionPacket>(
                    objectMap[pair.second.first->id], sphere2);

            checkCollisons(colPacketA, pair.second->
                    checkCollisions(colPacketB, pair.first->model);

        }

        if (colPacketA->foundCollision) {
            std::cout << colPacketA->r3Velocity.x << " " << colPacketA->r3Velocity.y << " " << colPacketA->r3Velocity.z
                      << std::endl;
            glm::vec3 displacementA = colPacketA->r3Position;
            std::cout << displacementA.x << " " << displacementA.y << " " << displacementA.z << std::endl;
            std::cout << pair.first->model->position.x + displacementA.x << " "
                      << pair.first->model->position.y + displacementA.y << " "
                      << pair.first->model->position.z + displacementA.z << std::endl;
            pair.first->model->position -= displacementA;
            pair.first->model->boundingbox.position -= displacementA;
            pair.first->velocity = displacementA;
            pair.first->position = displacementA;
        }

        if (colPacketB->foundCollision) {
            std::cout << colPacketB->r3Velocity.x << " " << colPacketB->r3Velocity.y << " " << colPacketB->r3Velocity.z
                      << std::endl;
            glm::vec3 displacementB = colPacketB->r3Position;
            std::cout << displacementB.x << " " << displacementB.y << " " << displacementB.z << std::endl;
            std::cout << pair.second->model->position.x + displacementB.x << " "
                      << pair.second->model->position.y + displacementB.y << " "
                      << pair.second->model->position.z + displacementB.z << std::endl;
            pair.second->model->position -= displacementB;
            pair.second->model->boundingbox.position -= displacementB;
            pair.second->velocity = displacementB;
            pair.first->position = displacementB;
        }
    }

    collisionCandidates

    .

    clear();

}

};

#endif // INCLUDE_COLLIDER_H_
