#include "Animator.h"
#include "Camera.h"
#include "Model.h"
#include <memory>
class PlayerState {
public:
    enum class State {
        IDLE,
        RUNNING,
        GUN,
        FLYING,
        DUNGEON,
    };
    State state = State::IDLE;
    bool changed = false;
    Camera::Movement direction;
    std::shared_ptr<Model> left;
    std::shared_ptr<Model> right;
    std::shared_ptr<Model> gun;
    std::shared_ptr<Model> scope;
    std::shared_ptr<AnimationCycle> armLeft;
    std::shared_ptr<AnimationCycle> armRight;
    std::shared_ptr<AnimationCycle> fire;
    std::shared_ptr<AnimationCycle> fireScope;
    bool isShooting = false;

    PlayerState(std::shared_ptr<Model> left, std::shared_ptr<Model> right, std::shared_ptr<Model> gun, std::shared_ptr<Model> scope)
        : left(left)
        , right(right)
        , gun(gun)
        , scope(scope)
    {
        this->armLeft = armAnimation(left);
        this->armRight = armAnimation(right, true);
        this->fire = initGunHouseAnimation(gun);
        this->fireScope = initGunHouseAnimation(scope);
    };
    PlayerState() = default;
    void shoot()
    {
        fire->nextState();
        fireScope->nextState();
        isShooting = true;
    }
    void tick(float delta, std::shared_ptr<Camera> camera)
    {
        if (state == PlayerState::State::IDLE) {
            left->position.x = camera->position.x + camera->front.x * 2.0f + -camera->right.x * 1.0f;
            left->position.z = camera->position.z + camera->front.z * 2.0f + -camera->right.z * 1.0f;
            right->position.x = camera->position.x + camera->front.x * 2.0f + camera->right.x * 1.0f;
            right->position.z = camera->position.z + camera->front.z * 2.0f + camera->right.z * 1.0f;
            left->position.y = camera->position.y - 0.8f;
            right->position.y = camera->position.y - 0.8f;
            left->yaw = -camera->options.yaw;
            right->yaw = -camera->options.yaw;
        }
        if (state == PlayerState::State::RUNNING) {

            left->position.x = camera->position.x + camera->front.x * 2.0f + -camera->right.x * 1.0f;
            left->position.z = camera->position.z + camera->front.z * 2.0f + -camera->right.z * 1.0f;
            right->position.x = camera->position.x + camera->front.x * 2.0f + camera->right.x * 1.0f;
            right->position.z = camera->position.z + camera->front.z * 2.0f + camera->right.z * 1.0f;
            left->position.y = camera->position.y - 0.8f;
            right->position.y = camera->position.y - 0.8f;
            left->yaw = -camera->options.yaw;
            right->yaw = -camera->options.yaw;
            armLeft->update(delta);
            armRight->update(delta);
        }

        if (state == PlayerState::State::GUN) {
            gun->position.x = camera->position.x + camera->front.x * 2.0f + -camera->right.x * 1.0f;
            gun->position.z = camera->position.z + camera->front.z * 2.0f + -camera->right.x * 1.0f;
            gun->position.y += camera->position.y - gun->position.y - 0.8f;
            gun->yaw = -camera->options.yaw;
            scope->position.x = camera->position.x + camera->front.x * 2.0f + -camera->right.x * 1.0f;
            scope->position.z = camera->position.z + camera->front.z * 2.0f + -camera->right.x * 1.0f;
            scope->position.y += camera->position.y - scope->position.y - 0.8f;
            scope->yaw = -camera->options.yaw;
            fire->update(delta);
            fireScope->update(delta);
        }
    }
};
