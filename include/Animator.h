#ifndef INCLUDE_INCLUDE_TRANFORMATION_H_
#define INCLUDE_INCLUDE_TRANFORMATION_H_
#include "Model.h"
#include <functional>
#include <glm/common.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <memory>
#include <utility>
#include <utils/Spline.h>
#include <vector>

class Animation {
    virtual void animate(float delta, std::shared_ptr<Model> model) = 0;
};

class BaseAnimation : public Animation {
public:
    float endTime;
    float currentTime;
    int index = 0;
    std::function<void(float, std::shared_ptr<Model>)> animation;
    BaseAnimation(float end, std::function<void(float, std::shared_ptr<Model>)> animation)
        : endTime(end)
        , animation(std::move(animation))
    {
        currentTime = 0;
    }

    [[nodiscard]] bool isActive() const
    {
        return currentTime <= endTime;
    };

    void animate(float delta, std::shared_ptr<Model> model) override
    {
        if (!isActive())
            return;
        animation(delta, model);
        currentTime += delta;
    }
};

class State {
public:
    std::shared_ptr<Model> model;
    std::shared_ptr<BaseAnimation> animation;
    bool autoTransition;
    State(std::shared_ptr<Model> m, std::shared_ptr<BaseAnimation> a, bool autoTransition)
        : model(std::move(m))
        , animation(std::move(a))
        , autoTransition(autoTransition)

    {
    }

    void animate(float delta) const
    {
        if (animation->isActive() == false)
            return;
        animation->animate(delta, model);
    }
};

class AnimationCycle {
private:
    std::vector<std::shared_ptr<State>> states;
    int currentIndex = 0; // Current state index

public:
    AnimationCycle() = default;

    void addState(const std::shared_ptr<State>& state)
    {
        states.push_back(state);
    }

    bool active()
    {
        return states[currentIndex]->animation->isActive();
    }

    void nextState()
    {
        if (states.empty() || states[currentIndex]->animation->isActive())
            return;
        currentIndex = (currentIndex + 1) % states.size();
        auto& currentAnimation = states[currentIndex]->animation;
        currentAnimation->currentTime = 0;
    }

    void update(float delta)
    {
        if (states.empty()) {
            return;
        } else if ((!states[currentIndex]->animation->isActive()) && states[currentIndex]->autoTransition) {
            nextState();
        }

        auto currentState = states[currentIndex];
        currentState->animation->animate(delta, currentState->model);
    }
};

std::shared_ptr<AnimationCycle> initDoorAnimation(const std::shared_ptr<Model>& cartDoor, const std::shared_ptr<Model>& pipe, const std::shared_ptr<Spline>& spline, std::shared_ptr<Model> cart, std::vector<std::shared_ptr<Model>>& splineModels)
{
    BaseAnimation open(0, [](float delta, const std::shared_ptr<Model>& model) {
        return;
    });
    BaseAnimation closed(0, [](float delta, const std::shared_ptr<Model>& model) {
        return;
    });
    BaseAnimation opening(2, [](float delta, const std::shared_ptr<Model>& model) {
        model->position += glm::vec3(0, 0.09, 0) * delta;
    });
    BaseAnimation closing(2, [](float delta, const std::shared_ptr<Model>& model) {
        model->position += glm::vec3(0, -0.09, 0) * delta;
    });

    BaseAnimation openingLeft(2, [](float delta, const std::shared_ptr<Model>& model) {
        model->position += glm::vec3(0.05 * delta, 0, 0);
    });
    BaseAnimation closingLeft(2, [](float delta, const std::shared_ptr<Model>& model) {
        model->position += glm::vec3(-0.05 * delta, 0, 0);
    });

    BaseAnimation splineAnimation(2.0, [spline, splineModels](float delta, const std::shared_ptr<Model>& model) {
        if (spline->isAtEnd())
            return;
        model->position += spline->current();
        for (auto& splineModel : splineModels) {
            splineModel->position += spline->current();
        }
        spline->addDelta(delta);
    });

    State closedState(cartDoor, std::make_shared<BaseAnimation>(closed), false);
    State openingState(cartDoor, std::make_shared<BaseAnimation>(opening), true);
    State pipeOpeningState(pipe, std::make_shared<BaseAnimation>(openingLeft), true);
    State openedState(cartDoor, std::make_shared<BaseAnimation>(open), false);
    State closingState(cartDoor, std::make_shared<BaseAnimation>(closing), true);
    State pipeClosingState(pipe, std::make_shared<BaseAnimation>(closingLeft), true);
    State splineState(std::move(cart), std::make_shared<BaseAnimation>(splineAnimation), true);

    auto closedStatePtr = std::make_shared<State>(closedState);
    auto pipeOpeningStatePtr = std::make_shared<State>(pipeOpeningState);
    auto openingStatePtr = std::make_shared<State>(openingState);
    auto openedStatePtr = std::make_shared<State>(openedState);
    auto closingStatePtr = std::make_shared<State>(closingState);
    auto pipeClosingStatePtr = std::make_shared<State>(pipeClosingState);
    auto doorAnimation = std::make_shared<AnimationCycle>();
    auto splineAnimationPtr = std::make_shared<State>(splineState);
    doorAnimation->addState(closedStatePtr);
    doorAnimation->addState(pipeOpeningStatePtr);
    doorAnimation->addState(openingStatePtr);
    doorAnimation->addState(openedStatePtr);
    doorAnimation->addState(closingStatePtr);
    doorAnimation->addState(pipeClosingStatePtr);
    doorAnimation->addState(splineAnimationPtr);
    return doorAnimation;
}

std::shared_ptr<AnimationCycle> armAnimation(std::shared_ptr<Model>& leftArm, bool leftright = false)
{
    BaseAnimation open(0.2, [](float delta, const std::shared_ptr<Model>& model) {
        model->roll -= 50.0f * delta;
    });
    BaseAnimation closed(0.2, [](float delta, const std::shared_ptr<Model>& model) {
        model->roll += 50.0f * delta;
    });

    State closedState(leftArm, std::make_shared<BaseAnimation>(closed), true);
    State openedState(leftArm, std::make_shared<BaseAnimation>(open), true);
    std::shared_ptr<State> openedStatePtr = std::make_shared<State>(openedState);
    std::shared_ptr<State> closedStatePtr = std::make_shared<State>(closedState);
    std::shared_ptr<AnimationCycle> animation = std::make_shared<AnimationCycle>();
    if (leftright) {
        animation->addState(closedStatePtr);
        animation->addState(openedStatePtr);
    } else {
        animation->addState(openedStatePtr);
        animation->addState(closedStatePtr);
    }
    return animation;
}

std::shared_ptr<AnimationCycle> initGunHouseAnimation(const std::shared_ptr<Model>& gun)
{

    BaseAnimation waiting(0, [](float delta, const std::shared_ptr<Model>& model) {
        return;
    });
    BaseAnimation open(0.1, [](float delta, const std::shared_ptr<Model>& model) {
        model->roll -= 5.0f * (delta * 30);
        model->position.x += 0.1;
    });
    BaseAnimation closed(0.1, [](float delta, const std::shared_ptr<Model>& model) {
        model->roll += 5.0f * (delta * 30);
        model->position.x += 0.1;
    });

    State waitingState(gun, std::make_shared<BaseAnimation>(waiting), false);
    State closedState(gun, std::make_shared<BaseAnimation>(closed), true);
    State openedState(gun, std::make_shared<BaseAnimation>(open), true);
    std::shared_ptr<State> waitingStatePtr = std::make_shared<State>(waitingState);
    std::shared_ptr<State> closedStatePtr = std::make_shared<State>(closedState);
    std::shared_ptr<State> openedStatePtr = std::make_shared<State>(openedState);
    std::shared_ptr<AnimationCycle> doorAnimation = std::make_shared<AnimationCycle>();
    doorAnimation->addState(waitingStatePtr);
    doorAnimation->addState(closedStatePtr);
    doorAnimation->addState(openedStatePtr);
    return doorAnimation;
}

#endif
