#ifndef INCLUDE_PHYSICS_H_
#define INCLUDE_PHYSICS_H_

#include "Collider.h"
#include "Model.h"
#include "Terrain.h"
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <unordered_map>
#include <vector>
#include <random>
#include <ctime>

#define DAMPENING 0.96f
#define GRAVITY 9.8f
#define DECELERATION 0.95f
#define ZERO_THRESHOLD 1e-30f

namespace physics {

class PhysicsWorld {
    std::unordered_map<int, std::shared_ptr<Object>> objects = {};
    std::vector<std::shared_ptr<Object>> enemies = {};  // Separate list for enemies
    std::default_random_engine generator;  // Random number generator

    struct EnemyMovement {
        glm::vec3 direction;
        float timeRemaining;
    };

    std::unordered_map<int, EnemyMovement> enemyMovements;  // Map to track enemy movement directions and times

public:
    Collider collider;
    std::vector<std::shared_ptr<Object>> triggers = {};
    std::vector<std::shared_ptr<Bullet>> bullets = {};

    PhysicsWorld() {
        generator.seed(static_cast<unsigned>(std::time(0)));  // Seed random number generator
    }

    void fireBullet(glm::vec3 position, glm::vec3 direction, float speed = 500.0f) {
        glm::vec3 velocity = glm::normalize(direction) * speed;
        std::shared_ptr<Bullet> bullet = std::make_shared<Bullet>(position, velocity);
        bullets.push_back(bullet);
    }

    std::vector<std::shared_ptr<Object>> getTriggers(const BoundingBox &cameraBoundingBox) {
        std::vector<std::shared_ptr<Object>> triggers = {};
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

    void addEnemy(std::shared_ptr<physics::Object> enemy) {
        enemies.push_back(enemy);
        addObject(enemy);  // Also add to the general objects list
        chooseNewDirection(enemy->id);  // Initialize with a random direction
    }

    void addPlane(std::shared_ptr<Model> plane) {
        std::shared_ptr<Plane> p = std::make_shared<Plane>(plane->position, plane->position.x, plane->position.z,
                                                           glm::vec3(0.0f, 1.0f, 0.0f));
        p->id = plane->id;
        p->position = plane->position;
        p->height = plane->position.y;
        p->isDynamic = true;
        p->isTrigger = false;
        p->isStatic = true;
        p->velocity = glm::vec3(0.0f);
        p->boundingBox = plane->boundingbox;
        p->pitch = plane->pitch;
        p->yaw = plane->yaw;
        p->roll = plane->roll;
        p->gravity = 0;
        p->mass = 0;
        addObject(p);
    }

    void addModel(std::shared_ptr<Model> &model, bool isDynamic, bool isTrigger, bool isStatic, float height, float mass, float gravity) {
        auto object = std::make_shared<Object>();
        object->id = model->id;
        object->position = glm::vec3(model->position);
        object->velocity = glm::vec3(0.0f, 0.0f, 0.0f);
        object->force = glm::vec3(0.0f, 0.0f, 0.0f);
        object->mass = mass;
        object->model = model;
        object->gravity = gravity;
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
        object->height = height;
        addObject(object);
    }

    void addEnemy(std::shared_ptr<Model> &model, float height, float mass, float gravity) {
        auto enemy = std::make_shared<Object>();
        enemy->id = model->id;
        enemy->position = glm::vec3(model->position);
        enemy->velocity = glm::vec3(0.0f, 0.0f, 0.0f);
        enemy->force = glm::vec3(0.0f, 0.0f, 0.0f);
        enemy->mass = mass;
        enemy->model = model;
        enemy->gravity = gravity;
        enemy->isDynamic = true;
        enemy->isTrigger = false;
        enemy->boundingBox = model->boundingbox;
        enemy->model = model;
        enemy->isStatic = false;
        enemy->pitch = model->pitch;
        enemy->yaw = model->yaw;
        enemy->roll = model->roll;
        enemy->plane = nullptr;
        enemy->isCamera = false;
        enemy->height = height;
        addEnemy(enemy);
    }

    void addCamera(std::shared_ptr<Camera> camera, bool isDynamic, bool isTrigger, bool isStatic) {
        auto object = std::make_shared<Cam>();
        object->position = camera->position;
        object->velocity = glm::vec3(0);
        object->id = camera->id;
        object->height = 20.0f;
        object->force = glm::vec3(0);
        object->mass = 1.0f;
        object->gravity = camera->firstPerson ? GRAVITY : 0;
        object->isDynamic = isDynamic;
        object->isTrigger = isTrigger;
        object->isStatic = isStatic;
        object->plane = nullptr;
        object->isCamera = true;
        object->grounded = object->firstPerson ? true : false;
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

    void chooseNewDirection(int enemyId) {
        std::uniform_real_distribution<float> directionDist(-1.0f, 1.0f);
        std::uniform_real_distribution<float> timeDist(2.0f, 5.0f);  // Random time between 2 and 5 seconds

        glm::vec3 direction(directionDist(generator), 0.0f, directionDist(generator));
        direction = glm::normalize(direction);  // Normalize to get a unit vector

        float time = timeDist(generator);

        enemyMovements[enemyId] = {direction, time};
    }

    void updateEnemyMovements(float dt, Terrain &terrain) {
        for (auto &enemy : enemies) {
            if (enemy->isDynamic && enemy->model) {
                auto &movement = enemyMovements[enemy->id];
                movement.timeRemaining -= dt;

                if (movement.timeRemaining <= 0.0f) {
                    // Choose a new direction and time
                    chooseNewDirection(enemy->id);
                } else {
                    // Apply the current direction as force
                    glm::vec3 force = movement.direction * 100.0f;  // Scale the direction to get a reasonable force
                    glm::vec3 nextPosition = enemy->position + force * dt;

                    // Boundary checks
                    if (nextPosition.x < 0) {
                        enemy->position.x = 0;
                        movement.direction.x = -movement.direction.x;  // Reverse the direction
                    } else if (nextPosition.x > terrain.terrainSize) {
                        enemy->position.x = terrain.terrainSize;
                        movement.direction.x = -movement.direction.x;  // Reverse the direction
                    }

                    if (nextPosition.z < 0) {
                        enemy->position.z = 0;
                        movement.direction.z = -movement.direction.z;  // Reverse the direction
                    } else if (nextPosition.z > terrain.terrainSize) {
                        enemy->position.z = terrain.terrainSize;
                        movement.direction.z = -movement.direction.z;  // Reverse the direction
                    }

                    applyForce(enemy->id, force);

                    // Update the yaw to face the movement direction
                    enemy->yaw = glm::degrees(std::atan2(movement.direction.z, movement.direction.x));
                }
            }
        }
    }

    void tick(float dt, Terrain &terrain) {
        std::vector<std::shared_ptr<Bullet>> toRemove = {};
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

        updateEnemyMovements(dt, terrain);

        for (auto &[id, object]: objects) {
            if (object->isDynamic) {
                float gravity = object->grounded ? 0 : object->gravity;
                object->force.y -= object->mass * gravity;
                glm::vec3 acceleration = object->force / object->mass;
                object->velocity += acceleration * dt;

                if (object->plane == nullptr) {
                    float terrainHeight = terrain.GetHeightInterpolated(object->position.x, object->position.z);
                    if (object->position.y < -terrainHeight + object->height) {
                        object->position.y = -terrainHeight + object->height;
                        object->velocity.y = 0;
                        object->grounded = true;
                    } else if (object->position.y > -terrainHeight + object->height + 5.0f) {
                        object->grounded = false;
                    }
                } else {
                    float planeHeightAtPosition = object->plane->height;
                    if (object->position.y < planeHeightAtPosition) {
                        object->position.y = planeHeightAtPosition + object->height;
                        object->velocity.y = 0;
                        object->grounded = true;
                    } else if (object->position.y > planeHeightAtPosition + object->height + 2.0f ||
                               object->position.x > object->plane->boundingBox->max.x + 2.0f ||
                               object->position.x < object->plane->boundingBox->min.x - 2.0f ||
                               object->position.z > object->plane->boundingBox->max.z + 2.0f ||
                               object->position.z < object->plane->boundingBox->min.z - 2.0f) {
                        object->grounded = false;
                        object->plane = nullptr;
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

            std::shared_ptr<std::unordered_map<int, std::shared_ptr<physics::Object>>> pObjects = std::make_shared
                    <std::unordered_map<int, std::shared_ptr<physics::Object>>>(objects);
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

}  // namespace physics

#endif  // INCLUDE_PHYSICS_H_
