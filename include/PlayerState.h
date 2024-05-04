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
    std::shared_ptr<Model> left;
    std::shared_ptr<Model> right;
    std::shared_ptr<Model> gun;
    PlayerState(std::shared_ptr<Model> left, std::shared_ptr<Model> right, std::shared_ptr<Model> gun)
        : left(left)
        , right(right)
        , gun(gun) {};
    PlayerState() = default;
    void tick(float delta, std::shared_ptr<Camera> camera)
    {
        if (state == PlayerState::State::RUNNING || state == PlayerState::State::IDLE) {
            left->position.x = camera->position.x + camera->front.x * 2.0f + -camera->right.x * 1.0f;
            left->position.z = camera->position.z + camera->front.z * 2.0f + -camera->right.z * 1.0f;
            right->position.x = camera->position.x + camera->front.x * 2.0f + camera->right.x * 1.0f;
            right->position.z = camera->position.z + camera->front.z * 2.0f + camera->right.z * 1.0f;
            left->position.y = camera->position.y - left->position.y - 0.8f;
            right->position.y = camera->position.y - right->position.y - 0.8f;
            right->yaw = -camera->options.yaw;
            left->yaw = -camera->options.yaw;
        }
    }
};
