//
// Created by Jamie Wales on 21/04/2024.
//

#ifndef SPOOKY_OBJECT_H
#define SPOOKY_OBJECT_H

#include "BoundingBox.h"
#include "Model.h"
#include "PhysicsUtils.h"
#include <glm/glm.hpp>
#include <memory>

namespace physics {
    class Plane;

    class Object {
    public:
        int id;
        bool isCamera;
        glm::vec3 position;
        bool grounded = true;
        glm::vec3 velocity;
        glm::vec3 force;
        float mass;
        float gravity;
        bool isDynamic;
        bool isTrigger;
        bool isStatic;
        float pitch;
        float yaw;
        float roll;
        std::shared_ptr<Model> model;
        std::shared_ptr<Camera> camera;
        std::shared_ptr<BoundingBox> boundingBox;
        float height = 0.0f;
        std::shared_ptr<Plane> plane;

        Object()
            : id(id)
              , isCamera(isCamera)
              , position(0.0f, 0.0f, 0.0f)
              , velocity(0.0f, 0.0f, 0.0f)
              , force(0.0f, 0.0f, 0.0f)
              , mass(1.0f)
              , gravity(0.0f)
              , isDynamic(false)
              , isTrigger(false)
              , isStatic(true)
              , pitch(pitch)
              , yaw(yaw)
              , roll(roll)
              , height(height)
              , boundingBox()
              , plane(nullptr) {
        }

        virtual ~Object() = default;

        virtual void updateBB() {
            if (position != boundingBox->position || pitch != boundingBox->pitch || yaw != boundingBox->yaw || roll !=
                boundingBox->roll) {
                boundingBox->pitch = pitch;
                boundingBox->yaw = yaw;
                boundingBox->roll = roll;
                boundingBox->updateRotation();
                boundingBox->translate(position - boundingBox->position);
                boundingBox->updateAABB();
                boundingBox->position = position;
            }
        }

        virtual void updateModel() {
            model->position = position;
            model->pitch = pitch;
            model->yaw = yaw;
            model->roll = roll;
        }

        virtual void collide(std::shared_ptr<Object> other) {
            Sphere sphereA = boundingBoxToSphere(*boundingBox);
            Sphere sphereB = boundingBoxToSphere(*other->boundingBox);
            if (checkSphereCollision(sphereA, sphereB)) {
                glm::vec3 collisionNormal = glm::normalize(position - other->position);
                float relativeVelocity = glm::dot(other->velocity - velocity, collisionNormal);
                if (relativeVelocity < 0)
                    return;

                float e = 1.0f; // Coefficient of restitution
                float j = -(1 + e) * relativeVelocity / (1 / mass + 1 / other->mass);
                glm::vec3 impulse = j * collisionNormal;

                if (!isStatic) {
                    velocity -= impulse / mass;
                }

                if (!other->isStatic) {
                    other->velocity += impulse / other->mass;
                }
                float penetrationDepth = (sphereA.radius + sphereB.radius) - glm::distance(position, other->position);
                if (!isStatic) {
                    position += (penetrationDepth * collisionNormal) * 0.5f;
                }
                if (!other->isStatic) {
                    other->position -= (penetrationDepth * collisionNormal) * 0.5f;
                }
            }
        }

        Sphere boundingBoxToSphere(const BoundingBox &box) {
            glm::vec3 center = (box.min + box.max) * 0.5f;
            float radius = glm::length(box.max - center);
            return Sphere{center, radius};
        }

        bool checkSphereCollision(const Sphere &sphere1, const Sphere &sphere2) {
            float radiusSum = sphere1.radius + sphere2.radius;
            float distance = glm::length(sphere1.center - sphere2.center);
            return distance <= radiusSum;
        }
    };

    class Bullet : public Object {
    public:
        Bullet(glm::vec3 startPos, glm::vec3 vel, float life = 10.0f)
            : Object()
              , lifetime(life) {
            position = startPos;
            velocity = vel;
            mass = 1.0f;
            gravity = 0.0f;
            isDynamic = true;
            isTrigger = false;
            isStatic = false;
        }

        float lifetime;

        void collide(std::shared_ptr<Object> other) override {
            // Bullet-specific collision behavior
        }
    };

    class Cam : public Object {
    public:
        bool firstPerson;

        Cam()
            : Object()
              , firstPerson(false) {
            gravity = 0.0f;
            isDynamic = true;
            isStatic = false;
            isTrigger = false;
            height = 20.0f;
        }

        void collide(std::shared_ptr<Object> other) override {
            auto sphereA = Sphere{position, 1.0f};
            auto sphereB = boundingBoxToSphere(*other->boundingBox);
            if (checkSphereCollision(sphereA, sphereB)) {
                glm::vec3 collisionNormal = glm::normalize(position - other->position);
                float relativeVelocity = glm::dot(other->velocity - velocity, collisionNormal);
                if (relativeVelocity < 0)
                    return;

                float e = 1.0f; // Coefficient of restitution
                float j = -(1 + e) * relativeVelocity / (1 / mass + 1 / other->mass);
                glm::vec3 impulse = j * collisionNormal;

                if (!isStatic) {
                    velocity -= impulse / mass;
                }

                if (!other->isStatic) {
                    other->velocity += impulse / other->mass;
                }

                float penetrationDepth = (sphereA.radius + sphereB.radius) - glm::distance(position, other->position);

                if (!isStatic) {
                    position += (penetrationDepth * collisionNormal) * 0.5f;
                }

                if (!other->isStatic) {
                    other->position -= (penetrationDepth * collisionNormal) * 0.5f;
                }
            }
        }

        void updateBB() override {
            if (position != boundingBox->position || pitch != boundingBox->pitch || yaw != boundingBox->yaw || roll !=
                boundingBox->roll) {
                boundingBox->pitch = pitch;
                boundingBox->yaw = yaw;
                boundingBox->roll = roll;
                boundingBox->updateRotation();
                boundingBox->translate(position - boundingBox->position);
                boundingBox->updateAABB();
                boundingBox->position = position;
            }
        }

        void updateModel() override {
            camera->position = position;
            camera->options.yaw = pitch;
            camera->options.yaw = yaw;
        }
    };

    class Plane : public Object {
    public:
        float width;
        float height;
        glm::vec3 normal;
        bool isTerrain;

        Plane(glm::vec3 pos, float w, float h, glm::vec3 n)
            : Object()
              , width(w)
              , height(h)
              , normal(glm::normalize(n)) {
            position = pos;
            mass = 0.0f;
            isStatic = true;
            isDynamic = false;
            isTrigger = false;
        }

        void updateBB() override {
            // Update the bounding box for the plane if necessary
        }

        void updateModel() override {
            // Update the model properties for the plane if necessary
        }

        bool checkCollision(const std::shared_ptr<Object>& other) {
            // Basic collision detection with the plane (point-plane distance check)
            float distance = glm::dot(other->position - position, normal);
            return distance <= 0.0f; // Assuming the plane is infinitely thin
        }

        void resolveCollision(std::shared_ptr<Object> other) {
            if (checkCollision(other)) {
                glm::vec3 collisionNormal = normal;
                float relativeVelocity = glm::dot(other->velocity, collisionNormal);
                if (relativeVelocity < 0)
                    return;

                float e = 1.0f; // Coefficient of restitution
                float j = -(1 + e) * relativeVelocity / (1 / other->mass);
                glm::vec3 impulse = j * collisionNormal;

                if (!other->isStatic) {
                    other->velocity += impulse / other->mass;
                }
            }
        }
    };
}
#endif
