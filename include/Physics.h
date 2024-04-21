#ifndef INCLUDE_PHYSICS_H_
#define INCLUDE_PHYSICS_H_

#include "Collider.h"
#include "Model.h"
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <initializer_list>
#include <memory>

#define DAMPENING 0.93
#define GRAVITY -9.8
#define DECELERATION 0.65
#define ZERO_THRESHOLD 0.00000000000001f

namespace physics {

struct Object {
    glm::vec3 position, velocity, force;
    float mass;
    std::shared_ptr<Model> model;
};
class PhysicsWorld {
private:
    std::unordered_map<int, std::shared_ptr<Object>> objects = {};
    glm::vec3 gravity = glm::vec3(0, GRAVITY, 0);
    Collider collider;

public:
    PhysicsWorld() {

    };

    void addModel(std::shared_ptr<Model>& model)
    {
        Object object;
        object.position = glm::vec3(0);
        object.velocity = glm::vec3(0);
        object.force = glm::vec3(0);
        object.mass = 1.0;
        object.model = model;
        auto pObj = std::make_shared<Object>(object);
        objects[model->id] = pObj;
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

    void tick(float dt)
    {
        for (auto& [id, object] : objects) {
            object->force += object->mass * gravity;
             glm::vec3 acceleration = object->force / object->mass;
             object->velocity += acceleration * dt;
             object->velocity *= DAMPENING;

             object->position += object->velocity;

             object->model->position += object->position * dt;
             if (object->model->position.y <= 0) {
                 object->position.y = 0;
                 object->model->position.y = 0;
                 object->velocity.y = 0;
             }



             object->force = glm::vec3(0);
             object->velocity *= DECELERATION;
             object->model->boundingbox.updateCorners(glm::translate(object->model->translation, object->model->position));
             collider.collide(object->model);

        }
    }
};

};

#endif
