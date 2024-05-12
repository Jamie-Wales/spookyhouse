// PhysicsWorld.h
#ifndef INCLUDE_PHYSICS_H_
#define INCLUDE_PHYSICS_H_

#include "Collider.h"
#include "Model.h"
#include "Terrain.h"
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <unordered_map>

#define DAMPENING 0.96f
#define GRAVITY -9.8f
#define DECELERATION 0.95f
#define ZERO_THRESHOLD 1e-30f

namespace physics {
    class PhysicsWorld {
        std::unordered_map<int, std::shared_ptr<Object> > objects = {};
        glm::vec3 gravity = glm::vec3(0, GRAVITY, 0);

    public:
        Collider collider;
        std::vector<std::shared_ptr<Object> > triggers = {};
        std::vector<std::shared_ptr<Bullet> > bullets = {};

        void fireBullet(glm::vec3 position, glm::vec3 direction, float speed = 500.0f) {
            glm::vec3 velocity = glm::normalize(direction) * speed;
            std::shared_ptr<Bullet> bullet = std::make_shared<Bullet>(position, velocity);
            bullets.push_back(bullet);
        }

        std::vector<std::shared_ptr<Object> > getTriggers(const BoundingBox &cameraBoundingBox) {
            std::vector<std::shared_ptr<Object> > triggers = {};
            for (auto &[id, object]: objects) {
                if (object->isTrigger) {
                    BoundingBox enlargedBox = *object->boundingBox;
                    if (enlargedBox.intersects(cameraBoundingBox, 10.0f)) {
                        triggers.push_back(object);
                    }
                }
            }
            return triggers;
        }

        void addObject(bool isModel, std::shared_ptr<physics::Object> object) {
            if (object->isCamera) {
                objects[object->id] = object;
                collider.addCamera(object);
            } else {
                objects[object->id] = object;
                collider.addObject(object);
            }
        }

        void addModel(std::shared_ptr<Model> &model, bool isDynamic, bool isTrigger, bool isStatic) {
            auto object = std::make_shared<Object>();
            object-> id = model->id;
            object->position = glm::vec3(model->position);
            object->velocity = glm::vec3(0);
            object->force = glm::vec3(0);
            object->mass = 10000.0;
            object->model = model;
            object->gravity = GRAVITY;
            object->isDynamic = isDynamic;
            object->isTrigger = isTrigger;
            object->boundingBox = model->boundingbox;
            object->model = model;
            object->isStatic = isStatic;
            object->pitch = model->pitch;
            object->yaw = model->yaw;
            object->roll = model->roll;
            object->isCamera = false;
            addObject(true, object);
        }

        void addCamera(std::shared_ptr<Camera> camera, bool isDynamic, bool isTrigger, bool isStatic) {
            auto object = std::make_shared<Cam>();
            object->position = camera->position;
            object->velocity = glm::vec3(0);
            object-> id = camera->id;
            object->force = glm::vec3(0);
            object->mass = 1.0f;
            object->gravity = camera->firstPerson ? GRAVITY : 0.0f;
            object->isDynamic = isDynamic;
            object->isTrigger = isTrigger;
            object->isStatic = isStatic;
            object->isCamera = true;
            object->pitch = camera->options.pitch;
            object->yaw = camera->options.yaw;
            object->camera = camera;
            object->roll = 0.0f;
            object->boundingBox = camera->boundingBox;
            addObject(false, object);
        }

        void cameraIsDynamic(unsigned int cameraid) {
            for (auto &[id, object]: objects) {
                if (id == cameraid) {
                    object->isDynamic = !object->isDynamic;
                }
            }
        }

        void removeObject(const std::shared_ptr<Object> &object) {
            objects.erase(object->model->id);
        }

        void updatePyr(int objId, float pitch, float yaw, float roll) {
            auto obj = objects.at(objId);
            obj->pitch = pitch;
            obj->yaw = yaw;
            obj->roll = roll;
        }

        void updateAll(int objId, glm::vec3 position, float pitch, float yaw, float roll) {
            auto obj = objects.at(objId);
            obj->position = position;
            obj->pitch = pitch;
            obj->yaw = yaw;
            obj->roll = roll;
        }
        void updatePosition(int objId, glm::vec3 position) {
            objects.at(objId)->position = position;
        }

        void applyForce(int objId, glm::vec3 force) {
            bool found = false;
            for (auto &[id, object]: objects) {
                if (id == objId) {
                    found = true;
                    object->force += force;
                }
            }
        }

        void tick(float dt, Terrain &terrain) {
            std::vector<std::shared_ptr<Bullet> > toRemove = {};
            auto it = bullets.begin();
            while (it != bullets.end()) {
                auto &bullet = *it;
                bool hit = false;
                glm::vec3 nextPosition;
                for (int i = 0; i < 5; i++) {
                    nextPosition = bullet->position + (bullet->velocity * dt);
                    for (auto &[id, object]: objects) {
                        if (object->boundingBox->intersects(nextPosition)) {
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

            for (auto &bullet: toRemove) {
                bullets.erase(std::remove(bullets.begin(), bullets.end(), bullet), bullets.end());
            }

            for (auto &[id, object]: objects) {
                if (object->isDynamic) {
                    if (object->gravity > 0) {
                        object->force += object->mass * object->gravity;
                    }
                    glm::vec3 acceleration = object->force / object->mass;
                    object->velocity += acceleration * dt;
                    object->velocity *= DAMPENING;
                    object->position += object->velocity * dt;

                    if (object->planeHeight == 0) {
                        if (object->position.y < -terrain.GetHeightInterpolated(
                                object->position.x, object->position.z)) {
                            object->position.y = -terrain.GetHeightInterpolated(
                                                     object->position.x, object->position.z) + 2.0f;
                            object->velocity.y = 0;
                        }
                    }
                } else {
                    object->velocity = glm::vec3(0.0f);
                }


                object->updateBB();
                std::vector<BroadCollision> broadCollisions;
                broadCollisions = collider.broadCollide(object);

                std::shared_ptr<std::unordered_map<int, std::shared_ptr<physics::Object> > > pObjects =
                        std::make_shared<std::unordered_map<int, std::shared_ptr<physics::Object> > >(objects);
                collider.resolveCollisions(broadCollisions, pObjects, dt);

                object->updateModel();
                object->force = glm::vec3(0.0f);
                object->velocity *= DECELERATION;

                if (glm::length(object->velocity) < ZERO_THRESHOLD) {
                    object->velocity = glm::vec3(0.0f);
                }
            }
        }
    };
}
#endif
