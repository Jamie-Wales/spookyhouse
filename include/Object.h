//
// Created by Jamie Wales on 21/04/2024.
//

#ifndef SPOOKY_OBJECT_H
#define SPOOKY_OBJECT_H
#include <glm/glm.hpp>
#include <memory>
#include "Model.h"
namespace physics {
    struct Object {
        glm::vec3 position, velocity, force;
        float mass;
        std::shared_ptr<Model> model;
    };

}
#endif //SPOOKY_OBJECT_H
