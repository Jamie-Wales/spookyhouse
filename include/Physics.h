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
#define GRAVITY 9.8f
#define DECELERATION 0.95f
#define ZERO_THRESHOLD 1e-30f

namespace physics {
    class PhysicsWorld {
        std::unordered_map<int, std::shared_ptr<Object> > objects = {};

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

        void addObject(std::shared_ptr<physics::Object> object) {
            objects[object->id] = object;
            collider.addObject(object);
        }

        void addModel(std::shared_ptr<Model> &model, bool isDynamic, bool isTrigger, bool isStatic) {
            auto object = std::make_shared<Object>();
            object->id = model->id;
            object->position = glm::vec3(model->position);
            object->velocity = glm::vec3(0.0f, 0.0f, 0.0f);
            object->force = glm::vec3(0.0f, 0.0f, 0.0f);
            object->mass = 1.0;
            object->model = model;
            object->gravity = 0;
            object->isDynamic = isDynamic;
            object->isTrigger = isTrigger;
            object->boundingBox = model->boundingbox;
            object->model = model;
            object->isStatic = isStatic;
            object->pitch = model->pitch;
            object->yaw = model->yaw;
            object->roll = model->roll;
            object->plane = nullptr;
            object->isCamera = false;
            addObject(object);
        }

        void addCamera(std::shared_ptr<Camera> camera, bool isDynamic, bool isTrigger, bool isStatic) {
            auto object = std::make_shared<Cam>();
            object->position = camera->position;
            object->velocity = glm::vec3(0);
            object->id = camera->id;
            object->force = glm::vec3(0);
            object->mass = 1.0f;
            object->gravity = camera->firstPerson ? GRAVITY : 0;
            object->isDynamic = isDynamic;
            object->isTrigger = isTrigger;
            object->isStatic = isStatic;
            object->plane = nullptr;
            object->isCamera = true;
            object->pitch = camera->options.pitch;
            object->yaw = camera->options.yaw;
            object->camera = camera;
            object->roll = 0.0f;
            object->boundingBox = camera->boundingBox;
            addObject(object);
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
                    float gravity = object->grounded ? 0 : object->gravity;
                    object->force.y -= object->mass * gravity;
                    glm::vec3 acceleration = object->force / object->mass;
                    object->velocity += acceleration * dt;


                    if (object->plane == nullptr) {
                        if (object->position.y < -terrain.GetHeightInterpolated(
                                object->position.x, object->position.z) + object->height) {
                            object->position.y = -terrain.GetHeightInterpolated(
                                                     object->position.x, object->position.z) +
                                                 object->height;
                            object->velocity.y = 0;
                            object->grounded = !object->grounded;
                        } else if (object->position.y > -terrain.GetHeightInterpolated(object->position.x, object->position.z) + object->height + 5.0f) {
                            object->grounded = !object->grounded;
                        }

                    } else {
                        float planeHeightAtPosition = object->plane->position.y;
                        if (object->position.y < planeHeightAtPosition) {
                            object->position.y = planeHeightAtPosition + object->height;
                            object->velocity.y = 0;
                            object->velocity.y = 0;
                            object->grounded = !object->grounded;
                        }
                    }
                    object->velocity *= DAMPENING;
                } else {
                    object->velocity = glm::vec3(0.0f);
                }

                if (object->isDynamic) {
                    object->position += object->velocity;
                }

                object->updateBB();
                std::vector<BroadCollision> broadCollisions;
                broadCollisions = collider.broadCollide(object);

                std::shared_ptr<std::unordered_map<int, std::shared_ptr<physics::Object> > > pObjects =
                        std::make_shared<std::unordered_map<int, std::shared_ptr<physics::Object> > >(objects);
                collider.resolveCollisions(broadCollisions, pObjects, dt);

                object->updateModel();
                object->force = glm::vec3(0.0f, 0.0f, 0.0f);
                if (object->isDynamic) {
                    object->velocity *= DECELERATION;
                }

                if (glm::length(object->velocity) < ZERO_THRESHOLD) {
                    object->velocity = glm::vec3(0.0f, 0.0f, 0.0f);
                }
            }
        }
    };
}
#endif
