#include "Animator.h"
#include "Model.h"
#include "Renderer.h"
#include <memory>
class PlayerState {
public:
    enum class State {
        IDLE,
        RUNNING,
        GUN,
        FLYING,
    };
    State state = State::FLYING;
    bool changed = false;
    Camera::Movement direction;
    std::shared_ptr<Model> left;
    std::shared_ptr<Model> right;
    std::shared_ptr<Model> gun;
    std::shared_ptr<AnimationCycle> armLeft;
    std::shared_ptr<AnimationCycle> armRight;

    PlayerState(std::shared_ptr<Model> left, std::shared_ptr<Model> right, std::shared_ptr<Model> gun)
        : left(left)
        , right(right)
        , gun(gun)
    {
        this->armLeft = armAnimation(left);
        this->armRight = armAnimation(right, true);
    };
    PlayerState() = default;
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
    }
};
