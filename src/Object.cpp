#include "Object.h"
#include <glm/gtx/norm.hpp>
#include <iostream>

namespace physics {
    Object::Object()
        : id(0)
          , isCamera(false)
          , position(0.0f, 0.0f, 0.0f)
          , grounded(true)
          , velocity(0.0f, 0.0f, 0.0f)
          , force(0.0f, 0.0f, 0.0f)
          , mass(1.0f)
          , gravity(0.0f)
          , isDynamic(false)
          , isTrigger(false)
          , isStatic(true)
          , pitch(0.0f)
          , yaw(0.0f)
          , roll(0.0f)
          , height(0.0f)
          , boundingBox(nullptr)
          , plane(nullptr) {
    }

    Object::~Object() = default;

    void Object::updateBB() {
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

    void Object::updateModel() {
        model->position = position;
        model->pitch = pitch;
        model->yaw = yaw;
        model->roll = roll;
    }

    void Object::collide(std::shared_ptr<Object> other) {
        if (auto bullet = std::dynamic_pointer_cast<Bullet>(other)) {
            collideWithBullet(bullet);
        } else if (auto cam = std::dynamic_pointer_cast<Cam>(other)) {
            collideWithCam(cam);
        } else if (auto plane = std::dynamic_pointer_cast<Plane>(other)) {
            collideWithPlane(plane);
        }
    }

    void Object::collideWithBullet(std::shared_ptr<Bullet> bullet) {
        /* Default behavior */
    }

    void Object::collideWithCam(std::shared_ptr<Cam> cam) {
        /* Default behavior */
    }

    void Object::collideWithPlane(std::shared_ptr<Plane> plane) {
        /* Default behavior */
    }

    Sphere Object::boundingBoxToSphere(const BoundingBox &box) {
        glm::vec3 center = (box.min + box.max) * 0.5f;
        float radius = glm::length(box.max - center);
        return Sphere{center, radius};
    }

    bool Object::checkSphereCollision(const Sphere &sphere1, const Sphere &sphere2) {
        float radiusSum = sphere1.radius + sphere2.radius;
        float distance = glm::length(sphere1.center - sphere2.center);
        return distance <= radiusSum;
    }

    Bullet::Bullet(glm::vec3 startPos, glm::vec3 vel, float life)
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

    void Bullet::collide(std::shared_ptr<Object> other) {
        other->collideWithBullet(std::make_shared<Bullet>(*this));
    }

    void Bullet::collideWithCam(std::shared_ptr<Cam> cam) {
        // Bullet-specific collision with Cam
        std::cout << "Bullet collided with Cam\n";
    }

    void Bullet::collideWithPlane(std::shared_ptr<Plane> plane) {
        // Bullet-specific collision with Plane
        std::cout << "Bullet collided with Plane\n";
    }

    Cam::Cam()
        : Object()
          , firstPerson(false) {
        gravity = 0.0f;
        isDynamic = true;
        isStatic = false;
        isTrigger = false;
        height = 20.0f;
    }

    void Cam::collide(std::shared_ptr<Object> other) {
        other->collideWithCam(std::make_shared<Cam>(*this));
    }

    void Cam::collideWithBullet(std::shared_ptr<Bullet> bullet) {
        // Cam-specific collision with Bullet
        std::cout << "Cam collided with Bullet\n";
    }


    bool planeCollision(Plane &plane, glm::vec3 point) {
        float distance = glm::dot(point - plane.position, plane.normal);
        return distance <= 1.0f; // Assuming the plane is infinitely thin
    }

    void Cam::collideWithPlane(std::shared_ptr<Plane> plane) {
        Sphere sphereA = Sphere{position, 1.0f};
        if (checkSphereCollision(boundingBoxToSphere(*plane->boundingBox), sphereA)) {
            if (this->camera->firstPerson) {
                this->plane = plane;
                grounded = true;
                position.y = plane->height;
                position.y += height;
                velocity.y = 0;
            }
        }
    }

    void Cam::updateBB() {
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

    void Cam::updateModel() {
        camera->position = position;
        camera->options.pitch = pitch;
        camera->options.yaw = yaw;
    }

    Plane::Plane(glm::vec3 pos, float w, float h, glm::vec3 n)
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

    void Plane::updateBB() {
        boundingBox->updateAABB();
    }

    void Plane::updateModel() {
        // Update the model properties for the plane if necessary
    }

    bool Plane::checkCollision(const std::shared_ptr<Object> &other) {
        float distance = glm::dot(other->position - position, normal);
        return distance <= 1.0f; // Assuming the plane is infinitely thin
    }

    void Plane::collide(std::shared_ptr<Object> other) {
        other->collideWithPlane(std::make_shared<Plane>(*this));
    }

    void Plane::collideWithBullet(std::shared_ptr<Bullet> bullet) {
        // Plane-specific collision with Bullet
        std::cout << "Plane collided with Bullet\n";
    }

    void Plane::collideWithCam(std::shared_ptr<Cam> cam) {
        // Plane-specific collision with Cam
        std::cout << "Plane collided with Cam\n";
    }
} // namespace physics
