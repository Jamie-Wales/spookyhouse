#ifndef OBJECT_H
#define OBJECT_H

#include <glm/glm.hpp>
#include <memory>
#include "BoundingBox.h"
#include "Camera.h"
#include "Model.h"
#include "PhysicsUtils.h"

namespace physics {
    class Plane;
class Bullet;
    class Cam;
    class Object {
public:
    int id;
    bool isCamera;
    glm::vec3 position;
    bool grounded;
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
    float height;
    std::shared_ptr<Plane> plane;

    Object();
    virtual ~Object();

    virtual void updateBB();
    virtual void updateModel();

    virtual void collide(std::shared_ptr<Object> other);

    virtual void collideWithBullet(std::shared_ptr<Bullet> bullet);
    virtual void collideWithCam(std::shared_ptr<Cam> cam);
    virtual void collideWithPlane(std::shared_ptr<Plane> plane);

    Sphere boundingBoxToSphere(const BoundingBox& box);
    bool checkSphereCollision(const Sphere& sphere1, const Sphere& sphere2);
};

class Bullet : public Object {
public:
    Bullet(glm::vec3 startPos, glm::vec3 vel, float life = 10.0f);

    float lifetime;

    void collide(std::shared_ptr<Object> other) override;
    void collideWithCam(std::shared_ptr<Cam> cam) override;
    void collideWithPlane(std::shared_ptr<Plane> plane) override;
};

class Cam : public Object {
public:
    bool firstPerson;

    Cam();

    void collide(std::shared_ptr<Object> other) override;
    void collideWithBullet(std::shared_ptr<Bullet> bullet) override;
    void collideWithPlane(std::shared_ptr<Plane> plane) override;

    void updateBB() override;
    void updateModel() override;
};

class Plane : public Object {
public:
    float width;
    float height;
    glm::vec3 normal;
    bool isTerrain;

    Plane(glm::vec3 pos, float w, float h, glm::vec3 n);

    void updateBB() override;
    void updateModel() override;

    bool checkCollision(const std::shared_ptr<Object>& other);
    void resolveCollision(std::shared_ptr<Object> other);

    void collide(std::shared_ptr<Object> other) override;
    void collideWithBullet(std::shared_ptr<Bullet> bullet) override;
    void collideWithCam(std::shared_ptr<Cam> cam) override;
};

} // namespace physics

#endif // OBJECT_H
