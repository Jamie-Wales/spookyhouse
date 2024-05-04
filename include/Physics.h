#ifndef INCLUDE_PHYSICS_H_
#define INCLUDE_PHYSICS_H_

#include "Collider.h"
#include "Model.h"
#include "Terrain.h"
#include <functional>
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

public:
    PhysicsWorld() = default;

    Collider collider;
    std::vector<std::shared_ptr<physics::Object>> triggers = {};

    void addModel(std::shared_ptr<Model>& model, bool isDynamic, bool isTrigger, bool isCamera = false)
    {
        Object object;
        object.position = glm::vec3(model->position);
        object.velocity = glm::vec3(0);
        object.force = glm::vec3(0);
        object.mass = 1.0;
        object.model = model;
        object.camera = nullptr;
        object.isDynamic = isDynamic;
        object.isTrigger = isTrigger;
        object.isCamera = isCamera;
        objects[model->id] = std::make_shared<Object>(object);
        collider.addModel(model);
    }

    void addCamera(std::shared_ptr<Camera>& camera, bool isDynamic, bool isTrigger, bool isCamera = true)
    {
        Object object;
        object.position = glm::vec3(camera->position);
        object.velocity = glm::vec3(0);
        object.force = glm::vec3(0);
        object.mass = 1.0;
        object.model = nullptr;
        object.camera = camera;
        object.isTrigger = isTrigger;
        object.isCamera = isCamera;
        objects[camera->id] = std::make_shared<Object>(object);
        collider.addCamera(camera);
    }

    void removeObject(std::shared_ptr<Object> object)
    {
        objects.erase(object->model->id);
    }

    void updatePosition(int objId, glm::vec3 position)
    {

        bool found = false;
        for (auto& [id, object] : objects) {
            if (id == objId) {
                found = true;
                object->position = position;
            }
        }
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
    }

    void tick(float dt, Terrain& terrain)
    {
        for (auto& [id, object] : objects) {
            if (object->isDynamic) {
                object->force += object->mass * object->gravity;
                glm::vec3 acceleration = object->force / object->mass;
                object->velocity += acceleration * dt;
                object->velocity *= DAMPENING;
                object->position += object->velocity * dt;
                if (object->position.y < -terrain.GetHeightInterpolated(object->position.x, object->position.z)) {
                    object->position.y = -terrain.GetHeightInterpolated(object->position.x, object->position.z);
                    object->velocity.y = 0;
                }
            }

            std::vector<BroadCollision> broadCollisions;
            if (!object->isCamera) {
                if (object->position != object->model->boundingbox.position) {
                    object->model->boundingbox.updateDifference(object->position);
                    object->model->boundingbox.rotate(object->model->pitch, object->model->yaw, 0.0);
                }
                broadCollisions = collider.broadCollide(object->model);
            } else {
                broadCollisions = collider.broadCollide(object->camera);
            }

            for (auto& bc : broadCollisions) {
                auto objecta = objects[bc.modelIdA];
                auto objectb = objects[bc.modelIdB];
                if (objecta->isTrigger && objectb->isCamera) {
                    triggers.push_back(objecta);
                } else if (objectb->isTrigger && objecta->isCamera) {
                    triggers.push_back(objectb);
                }
            }

            std::shared_ptr<std::unordered_map<int, std::shared_ptr<physics::Object>>> pObjects = std::make_shared<std::unordered_map<int, std::shared_ptr<physics::Object>>>(
                objects);
            // collider.resolveCollisions(broadCollisions, pObjects, dt);
            if (!object->isCamera)
                // object->model->position = object->position;
                // object->force = glm::vec3(0);
                // object->velocity *= DECELERATION;

                if (glm::length(object->velocity) < ZERO_THRESHOLD) {
                    object->velocity = glm::vec3(0);
                }
        }
    }
};
}

#endif
