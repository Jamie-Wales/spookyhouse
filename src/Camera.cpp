//
// Created by jamielinux on 2/17/24.
//

#include "Camera.h"

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

void Camera::updatePosition(float dt, Terrain& terrain)
{
    velocity = Approach(velocityTarget, velocity, dt * 500.0f);
    position += velocity * dt;
    checkXpos(terrain);
    velocity += velocity * dt;
}
void Camera::decrease(float deltaTime)
{
    options.velocity *= deltaTime;
    update();
}
void Camera::checkXpos(Terrain& terrain)
{
    if (this->position.y != -terrain.getTerPosition(this->position.x, this->position.z) + 20.0f) {
        this->position.y = glm::mix(this->position.y, -terrain.getTerPosition(this->position.x, this->position.z) + 20.0f, 0.1f);
    }
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

    if (options.yaw > 360.0f) {
        options.yaw = 0.0f;

    } else if (options.yaw < -360.0f) {
        options.yaw = 0.0f;
    }

    if (options.pitch > 50.0f)
        options.pitch = 50.0f;
    if (options.pitch < -50.0f)
        options.pitch = -50.0f;
    update();
}

void Camera::processKeyboard(Camera::Movement movement, float deltaTime, bool down, Terrain& terrain)
{
    if (firstPerson) {
        if (down) {
            switch (movement) {
            case Movement::FORWARD:
                velocityTarget.x = front.x * 1000.0f * deltaTime;
                velocityTarget.y = 0.0f;
                velocityTarget.z = front.z * 1000.0f * deltaTime;
                break;
            case Movement::BACKWARD:

                velocityTarget.x = -front.x * 1000.0f * deltaTime;
                velocityTarget.y = 0.0f;
                velocityTarget.z = -front.z * 1000.0f * deltaTime;
                break;
            case Movement::LEFT:
                velocityTarget.x = -right.x * 1000.0f * deltaTime;
                velocityTarget.y = 0.0f;
                velocityTarget.z = -right.z * 1000.0f * deltaTime;
                break;
            case Movement::RIGHT:
                velocityTarget.x = right.x * 1000.0f * deltaTime;
                velocityTarget.y = 0.0f;
                velocityTarget.z = right.z * 1000.0f * deltaTime;
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
    } else {
        if (down) {
            switch (movement) {
            case Movement::FORWARD:
                velocityTarget = front * 1000.0f * deltaTime;
                break;
            case Movement::BACKWARD:
                velocityTarget = -front * 1000.0f * deltaTime;
                break;
            case Movement::LEFT:
                velocityTarget = -right * 1000.0f * deltaTime;
                break;
            case Movement::RIGHT:
                velocityTarget = right * 1000.0f * deltaTime;
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
    }
    update();
    updatePosition(deltaTime, terrain);
}
