//
// Created by jamielinux on 2/17/24.
//

#pragma once

#include "BoundingBox.h"
#include "Terrain.h"
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>
#include <glm/vec3.hpp>

class Camera {
public:
    enum class Movement {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        IDLE
    };

    void alignWithCamera()
    {
        auto nposition = position;
        auto nfront = glm::normalize(front);

        nposition = nposition + front;
        boundingBox->min = nposition - 20.0f;
        boundingBox->max = nposition + 20.0f;
    }

    glm::mat4 getCameraView();

    int id;

    void processMouseScroll(float offset);

    void processMouseMovement(float x, float y);

    void processKeyboard(Camera::Movement movement, float deltaTime, bool down, std::shared_ptr<Terrain> terrain);

    std::shared_ptr<BoundingBox> boundingBox;

    void update();

    glm::vec3 Approach(glm::vec3 target, glm::vec3 current, float dt)
    {
        auto difference = target - current;

        if (glm::length(difference) > dt)
            return current + glm::normalize(difference) * dt;
        else if (glm::length(difference) < -dt)
            return current - glm::normalize(difference) * dt;
        return target;
    }

    float Approach(float target, float current, float delta)
    {
        float difference = target - current;
        if (difference > delta)
            return current + delta;
        if (difference < -delta)
            return current - delta;
        return target;
    }

    bool firstPerson;
    glm::vec3 position {};
    glm::vec3 front {};
    glm::vec3 up {};
    glm::vec3 worldUp {};
    glm::vec3 right {};

    glm::vec3 velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 velocityTarget = glm::vec3(0.0f, 0.0f, 0.0f);

    struct options {
        float yaw;
        float pitch;
        float maxSpeed;
        float accelerationRate;
        float mouseSensitivity;
        float velocity;
        float zoom;
    };

    void decrease(float deltaTime);

    void update(float deltaTime);

    void checkXpos(Terrain& terrain);

    void updateCameraVector();

    struct options options { };

    void updatePosition(float deltaTime, Terrain& terrain);

    Camera(int id)
    {
        this->id = id;
        options.maxSpeed = 15.0f;
        firstPerson = false;
        position = glm::vec3 { 1.0f, 100.0f, 1.0f };
        front = glm::vec3 { 0.0f, 0.0f, -1.0f };
        up = glm::vec3 { 0.0f, 1.0f, 0.0f };
        worldUp = up;
        right = glm::normalize(glm::cross(front, worldUp));
        options.pitch = 0.0;
        options.yaw = 90.0;
        options.mouseSensitivity = 0.1;
        options.zoom = 45.0f;
        options.accelerationRate = 1.0f;
        options.velocity = 0.0f;
        boundingBox = std::make_shared<BoundingBox>(position, options.pitch, options.yaw, 0.0);
        update();
    }

    Camera(int id, bool firstPerson)
    {
        this->id = id;
        this->firstPerson = firstPerson;
        position = glm::vec3 { 1.0f, 1.0, 3.0f };
        front = glm::vec3 { 0.0f, 0.0f, -1.0f };
        up = glm::vec3 { 0.0f, 1.0f, 0.0f };
        worldUp = up;
        right = glm::normalize(glm::cross(front, worldUp));
        options.pitch = 0.0;
        options.yaw = 90.0f;
        options.mouseSensitivity = 0.1;
        boundingBox = std::make_shared<BoundingBox>(position, options.pitch, options.yaw, 0.0);
        options.zoom = 45.0f;
        update();
    }

    Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch, float movementSpeed, float mouseSensitivity,
        float zoom)
        : position { position }
        , up { up }
    {
        options.pitch = pitch;
        options.yaw = yaw;
        options.mouseSensitivity = mouseSensitivity;
        options.zoom = zoom;
        update();
    }
};
