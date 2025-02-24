#ifndef INCLUDE_COLLIDER_H_
#define INCLUDE_COLLIDER_H_

#include "BroadCollision.h"
#include "Object.h"
#include "PhysicsUtils.h"
#include "SweepPrune.h"
#include <memory>
#include <vector>

namespace physics {
class Collider {
public:
    SweepAndPrune sweepAndPrune;

    void addObject(std::shared_ptr<physics::Object> object) {
        sweepAndPrune.AddObject(object);
    }

    void removeObject(std::shared_ptr<Model>& model)
    {
        sweepAndPrune.removeModel(model);
    }


    std::vector<BroadCollision> broadCollide(std::shared_ptr<physics::Object> object)
    {

        sweepAndPrune.updateObject(object);
        auto col = sweepAndPrune.getTrueCollisions();
        sweepAndPrune.printTrueCollisions(col);
        return col;
    }

    Sphere boundingBoxToSphere(const BoundingBox& box)
    {
        glm::vec3 center = (box.min + box.max) * 0.5f;
        float radius = glm::length(box.max - center);
        return Sphere { center, radius };
    }

    void resolveCollisions(
        std::vector<BroadCollision> broadphasePairs,
        std::shared_ptr<std::unordered_map<int, std::shared_ptr<physics::Object>>> objectMap, float dt)
    {
        for (auto& pair : broadphasePairs) {
            auto& objA = objectMap->at(pair.modelIdA);
            auto& objB = objectMap->at(pair.modelIdB);

            objA->collide(objB);
            objB->collide(objA);
        }
    }
};
}
#endif
