//
// Created by jamielinux on 2/17/24.
//

#include "Camera.h"
#include <cstdio>

void Camera::update()
{
    glm::vec3 updatedFront;
    updatedFront.x = cos(glm::radians(options.yaw)) * cos(glm::radians(options.pitch));
    updatedFront.y = sin(glm::radians(options.pitch));
    updatedFront.z = sin(glm::radians(options.yaw)) * cos(glm::radians(options.pitch));
    front = glm::normalize(updatedFront);
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}

void Camera::updatePosition(float dt)
{
    velocity = Approach(velocityTarget, velocity, dt * 30.0f);
    position += velocity * dt;
    velocity += velocity * dt;
}
void Camera::decrease(float deltaTime)
{
    options.velocity *= deltaTime;
    update();
}

glm::mat4 Camera::getCameraView()
{
    return glm::lookAt(position, position + front, up);
}

void Camera::processMouseScroll(float offset)
{
    options.zoom -= (float)offset;
    if (options.zoom < 1.0f)
        options.zoom = 1.0f;
    if (options.zoom > 60)
        options.zoom = 60;
}

void Camera::processMouseMovement(float x, float y)
{
    x *= options.mouseSensitivity;
    y *= options.mouseSensitivity;
    options.yaw += x;
    options.pitch += y;
    if (options.pitch > 89.0f)
        options.pitch = 89.0f;
    if (options.pitch < -89.0f)
        options.pitch = -89.0f;
    update();
}

void Camera::processKeyboard(Camera::Movement movement, float deltaTime, bool down)
{
    if (down) {
        switch (movement) {
        case Movement::FORWARD:
            velocityTarget = front * 30.0f * deltaTime;
            break;
        case Movement::BACKWARD:
            velocityTarget = -front * 30.0f * deltaTime;
            break;
        case Movement::LEFT:
            velocityTarget = -right * 30.0f * deltaTime;
            break;
        case Movement::RIGHT:
            velocityTarget = right * 30.0f * deltaTime;
            break;
        }

    } else {
        switch (movement) {
        case Movement::FORWARD:
            velocityTarget = glm::vec3(0.0);
            break;
        case Movement::BACKWARD:
            velocityTarget = glm::vec3(0.0);
            break;
        case Movement::LEFT:
            velocityTarget = glm::vec3(0.0);
            break;
        case Movement::RIGHT:
            velocityTarget = glm::vec3(0.0);
            break;
        }
    }
    update();
    updatePosition(deltaTime);
}
