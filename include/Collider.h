#ifndef INCLUDE_COLLIDER_H_
#define INCLUDE_COLLIDER_H_

#include "Object.h"
#include "Resolver.h"
#include "SweepPrune.h"
#include <iostream>
#include <memory>
#include <vector>

class Collider {
public:
    SweepAndPrune sweepAndPrune;
    std::vector<Pair> broadphasePairs;
    std::vector<std::pair<std::shared_ptr<physics::Object>&, std::shared_ptr<physics::Object>&>> collisionCandidates;

    void addModel(std::shared_ptr<Model>& model)
    {
        sweepAndPrune.addModel(model);
    }

    void broadCollide(std::shared_ptr<Model>& model)
    {
        sweepAndPrune.UpdateObject(model);
       broadphasePairs =  sweepAndPrune.getTrueCollisions();

    }

    void resolveCollisions()
    {
        for (auto& pair : collisionCandidates) {
            std::shared_ptr<CollisionPacket> colPacketA = std::make_shared<CollisionPacket>();
            prepareCollision(colPacketA, pair.first);

            std::shared_ptr<CollisionPacket> colPacketB = std::make_shared<CollisionPacket>();
            prepareCollision(colPacketB, pair.second);

            checkCollisions(colPacketA, pair.second->model);
            checkCollisions(colPacketB, pair.first->model);

            if (colPacketA->foundCollision) {
                std::cout << colPacketA->r3Velocity.x << " " << colPacketA->r3Velocity.y << " " << colPacketA->r3Velocity.z << std::endl;
                glm::vec3 displacementA = colPacketA->r3Position - pair.first->model->position;
                std::cout << displacementA.x << " " << displacementA.y << " " << displacementA.z << std::endl;
                std::cout << pair.first->model->position.x + displacementA.x << " " << pair.first->model->position.y + displacementA.y << " " << pair.first->model->position.z + displacementA.z << std::endl;
                pair.first->model->position = displacementA;
                pair.first->model->boundingbox.position = displacementA;
                pair.first->velocity = glm::vec3(0.0);
                pair.first->position = glm::vec3(0.0);

            }

            if (colPacketB->foundCollision) {
                std::cout << colPacketB->r3Velocity.x << " " << colPacketB->r3Velocity.y << " " << colPacketB->r3Velocity.z << std::endl;
                glm::vec3 displacementB = colPacketB->r3Position - pair.second->model->position;
                std::cout << displacementB.x << " " << displacementB.y << " " << displacementB.z << std::endl;
                std::cout << pair.second->model->position.x + displacementB.x << " " << pair.second->model->position.y + displacementB.y << " " << pair.second->model->position.z + displacementB.z << std::endl;
                pair.second->model->position = displacementB;
                pair.second->model->boundingbox.position = displacementB;
                pair.second->velocity = glm::vec3(0.0);
                pair.first->velocity = glm::vec3(0.0);
                pair.first->position = glm::vec3(0.0);

            }
        }
    }
};

#endif // INCLUDE_COLLIDER_H_
