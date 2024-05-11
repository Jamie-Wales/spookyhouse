//
// Created by Jamie Wales on 21/04/2024.
//

#ifndef SPOOKY_OBJECT_H
#define SPOOKY_OBJECT_H
#include "Camera.h"
#include "Model.h"
#include <glm/glm.hpp>
#include <memory>
namespace physics {

struct Object {
public:
    Object() = default;
    Object(glm::vec3 position, glm::vec3 velocity, glm::vec3 force, float mass, std::shared_ptr<Model> model, std::shared_ptr<Camera> camera, bool isDynamic, bool isTrigger, bool isCamera, bool isStatic)
        : position(position)
        , velocity(velocity)
        , force(force)
        , mass(mass)
        , model(model)
        , camera(camera)
        , isDynamic(isDynamic)
        , isTrigger(isTrigger)
        , isCamera(isCamera)
        , isStatic(isStatic)

    {
    }

    Object(std::shared_ptr<Object> object)
    {
        this->position = object->position;
        this->velocity = object->velocity;
        this->force = object->force;
        this->mass = object->mass;
        this->model = object->model;
        this->camera = object->camera;
        this->isDynamic = object->isDynamic;
        this->isTrigger = object->isTrigger;
        this->isCamera = object->isCamera;
        this->isStatic = object->isStatic;
        this->gravity = object->gravity;
    }
    Object(Object& object)
    {
        this->position = object.position;
        this->velocity = object.velocity;
        this->force = object.force;
        this->mass = object.mass;
        this->model = object.model;
        this->camera = object.camera;
        this->isDynamic = object.isDynamic;
        this->isTrigger = object.isTrigger;
        this->isCamera = object.isCamera;
        this->isStatic = object.isStatic;
        this->gravity = object.gravity;
    }

    glm::vec3 position, velocity, force;
    float mass = 1.0;

    std::shared_ptr<Model> model;
    std::shared_ptr<Camera> camera;
    bool isDynamic;
    bool isTrigger;
    bool isCamera;
    float gravity = -9.8f;
    ;
    bool isStatic;
};

}
#endif // SPOOKY_OBJECT_H
