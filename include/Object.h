//
// Created by Jamie Wales on 21/04/2024.
//

#ifndef SPOOKY_OBJECT_H
#define SPOOKY_OBJECT_H
#include "Model.h"
#include <glm/glm.hpp>
#include <memory>
namespace physics {

struct Object {
public:
    bool sphere = true;
    bool camera = false;
    Object() = default;
    Object(glm::vec3 position, glm::vec3 velocity, glm::vec3 force, float mass, std::shared_ptr<Model> model)
        : position(position)
        , velocity(velocity)
        , force(force)
        , mass(mass)
        , model(model)
    {
    }

    Object(std::shared_ptr<Object> object)
    {
        this->position = object->position;
        this->velocity = object->velocity;
        this->force = object->force;
        this->mass = object->mass;
        this->model = object->model;
    }
    Object(Object& object)
    {
        this->position = object.position;
        this->velocity = object.velocity;
        this->force = object.force;
        this->mass = object.mass;
        this->model = object.model;
    }

    glm::vec3 position, velocity, force;
    float mass;
    std::shared_ptr<Model> model;
};

}
#endif // SPOOKY_OBJECT_H
