#ifndef INCLUDE_PHYSICS_H_
#define INCLUDE_PHYSICS_H_

#include "Collider.h"
#include "Model.h"
#include "Terrain.h"
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <unordered_map>

#define DAMPENING 0.96
#define GRAVITY -9.8
#define DECELERATION 0.95
#define ZERO_THRESHOLD 0.000000000000000000000000000001

namespace physics {

class PhysicsWorld {
private:
    std::unordered_map<int, std::shared_ptr<Object>> objects = {};
    glm::vec3 gravity = glm::vec3(0, GRAVITY, 0);
    Collider collider;

public:
    PhysicsWorld() = default;

    void addModel(std::shared_ptr<Model>& model)
    {
        Object object;
        object.position = glm::vec3(model->position);
        object.velocity = glm::vec3(0);
        object.force = glm::vec3(0);
        object.mass = 1.0;
        object.model = model;
        objects[model->id] = std::make_shared<Object>(object);
        collider.addModel(model);
    }

    void removeObject(std::shared_ptr<Object> object)
    {
        objects.erase(object->model->id);
    }

    void applyForce(int objId, glm::vec3 force)
    {
        bool found = false;
        for (auto& [id, object] : objects) {
            if (id == objId) {
                found = true;
                object->force += force;
            }
        }

        if (!found) {
            std::cerr << "Object not found" << std::endl;
        }
    }

    void tick(float dt, Terrain& terrain)
    {
        for (auto& [id, object] : objects) {
            object->force += object->mass * gravity;
            glm::vec3 acceleration = object->force / object->mass;
            object->velocity += acceleration * dt;
            object->velocity *= DAMPENING;
            object->position += object->velocity * dt;
            if (object->position.y < 0) {
                object->position.y = 0;
                object->velocity.y = 0;
            }
            object->model->boundingbox.updateDifference(object->position);
            auto broadCollisions = collider.broadCollide(object->model);
            std::shared_ptr<std::unordered_map<int, std::shared_ptr<physics::Object>>> pObjects = std::make_shared<std::unordered_map<int, std::shared_ptr<physics::Object>>>(objects);
            collider.resolveCollisions(broadCollisions, pObjects, dt);
            object->model->position = object->position;
            object->force = glm::vec3(0);
            object->velocity *= DECELERATION;

            if (glm::length(object->velocity) < ZERO_THRESHOLD) {
                object->velocity = glm::vec3(0);
            }
        }
    }
};
}

#endif
