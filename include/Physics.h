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
    struct Bullet {
        glm::vec3 position;
        glm::vec3 velocity;
        float lifetime;

        Bullet(glm::vec3 startPos, glm::vec3 vel, float life = 10.0f)
            : position(startPos)
            , velocity(vel)
            , lifetime(life)
        {
        }
    };

private:
    std::unordered_map<int, std::shared_ptr<Object>> objects = {};
    glm::vec3 gravity = glm::vec3(0, GRAVITY, 0);

public:
    PhysicsWorld() = default;

    Collider collider;
    std::vector<std::shared_ptr<physics::Object>> triggers = {};
    std::vector<std::shared_ptr<Bullet>> bullets = {};

    void fireBullet(glm::vec3 position, glm::vec3 direction, float speed = 500.0f)
    {
        glm::vec3 velocity = glm::normalize(direction) * speed;
        std::shared_ptr<Bullet> bullet = std::make_shared<Bullet>(position, velocity);
        bullets.push_back(bullet);
    }

    std::vector<std::shared_ptr<physics::Object>> getTriggers(const BoundingBox& cameraBoundingBox)
    {
        std::vector<std::shared_ptr<physics::Object>> triggers = {};
        for (auto& [id, object] : objects) {
            if (object->isTrigger) {
                std::cout << object->model->id << std::endl;
                BoundingBox enlargedBox = object->model->boundingbox;
                if (enlargedBox.intersects(cameraBoundingBox, 10.0f)) {
                    triggers.push_back(object);
                }
            }
        }
        return triggers;
    }

    void addModel(std::shared_ptr<Model>& model, bool isDynamic, bool isTrigger, bool isStatic)
    {
        Object object;
        object.position = glm::vec3(model->position);
        object.velocity = glm::vec3(0);
        object.force = glm::vec3(0);
        object.mass = 10000.0;
        object.model = model;
        object.camera = nullptr;
        object.gravity = -9.8f;
        object.isDynamic = isDynamic;
        object.isTrigger = isTrigger;
        object.isStatic = isStatic;
        object.isCamera = false;
        objects[model->id] = std::make_shared<Object>(object);
        collider.addModel(model);
    }

    void addCamera(std::shared_ptr<Camera>& camera, bool isDynamic, bool isTrigger, bool isStatic)
    {
        Object object;
        object.position = glm::vec3(camera->position);
        object.velocity = glm::vec3(0);
        object.force = glm::vec3(0);
        object.mass = 1.0;
        object.model = nullptr;
        object.camera = camera;
        object.gravity = GRAVITY;
        object.isTrigger = isTrigger;
        object.isDynamic = isDynamic;
        object.isCamera = true;
        objects[camera->id] = std::make_shared<Object>(object);
        collider.addCamera(camera);
    }

    void cameraIsDynamic(unsigned int cameraid)
    {
        for (auto& [id, object] : objects) {
            if (id == cameraid) {
                object->isDynamic = !object->isDynamic;
            }
        }
    }

    void removeObject(const std::shared_ptr<Object>& object)
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
        std::vector<std::shared_ptr<Bullet>> toRemove = {};
        auto it = bullets.begin();
        while (it != bullets.end()) {
            auto& bullet = *it;
            bool hit = false;
            glm::vec3 nextPosition;
            for (int i = 0; i < 5; i++) {
                nextPosition = bullet->position + (bullet->velocity * dt);
                for (auto& [id, object] : objects) {
                    if (object->isCamera)
                        continue;
                    if (object->model->boundingbox.intersects(nextPosition)) {
                        std::cout << "MODEL HIT" << object->model->id;
                        toRemove.push_back(bullet);
                        hit = true;
                        break;
                    }
                }
                if (hit)
                    break;
            }

            if (bullet->lifetime <= 0) {
                it = bullets.erase(it);
            } else {
                bullet->position = nextPosition;
                bullet->lifetime -= dt;
                ++it;
            }
        }

        for (auto& bullet : toRemove) {
            bullets.erase(std::remove(bullets.begin(), bullets.end(), bullet), bullets.end());
        }
        for (auto& [id, object] : objects) {
            if (object->isDynamic) {

                if (object->isCamera) {
                    std::cout << object->gravity << std::endl;
                }
                if (object->gravity > 0) {
                    object->force += object->mass * object->gravity;
                }
                glm::vec3 acceleration = object->force / object->mass;
                object->velocity += acceleration * dt;
                object->velocity *= DAMPENING;
                object->position += object->velocity * dt;
                if (object->isCamera && object->camera->firstPerson) {
                    if (object->position.y < -terrain.getTerPosition(object->position.x, object->position.z) + 20.0f) {
                        //     object->position.y = glm::mix(object->position.y,
                        //        -terrain.getTerPosition(
                        //           object->position.x, object->position.z)
                        //          + 20.0f,
                        //     0.1f);
                    }
                }
                if (object->position.y < -terrain.GetHeightInterpolated(object->position.x, object->position.z)) {
                    object->position.y = -terrain.GetHeightInterpolated(object->position.x, object->position.z) + 2.0f;
                    object->velocity.y = 0;
                }
            } else {
                object->velocity = glm::vec3(0.0);
            }

            std::vector<BroadCollision> broadCollisions;
            if (!object->isCamera) {

                if (object->model->boundingbox.pitch != object->model->pitch || object->model->boundingbox.yaw != object->model->yaw || object->model->position != object->position) {
                    object->model->boundingbox.pitch = object->model->pitch;
                    object->model->boundingbox.yaw = object->model->yaw;
                    object->model->boundingbox.roll = object->model->roll;
                    object->model->boundingbox.updateRotation();

                    object->model->boundingbox.translate(
                        object->position - object->model->boundingbox.position);
                    object->model->boundingbox.updateAABB();
                }
            } else {
                if (object->camera->boundingBox.pitch != object->camera->options.pitch || object->camera->boundingBox.yaw != object->camera->options.yaw || object->camera->position != object->camera->boundingBox.position) {
                    object->camera->boundingBox.pitch = object->camera->options.pitch;
                    object->camera->boundingBox.yaw = object->camera->options.yaw;
                    object->camera->boundingBox.roll = 0.0f;
                    object->camera->boundingBox.updateRotation();
                    object->camera->boundingBox.translate(
                        object->camera->position - object->camera->boundingBox.position);
                    object->camera->boundingBox.updateAABB();
                }
                broadCollisions = collider.broadCollide(object->camera);
            }

            std::shared_ptr<std::unordered_map<int, std::shared_ptr<physics::Object>>> pObjects = std::make_shared<std::unordered_map<int, std::shared_ptr<physics::Object>>>(
                objects);
            collider.resolveCollisions(broadCollisions, pObjects, dt);
            if (object->isCamera) {
                object->camera->position = object->position;
            } else {
                object->model->position = object->position;
            }

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
