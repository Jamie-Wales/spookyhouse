#ifndef INCLUDE_RENDERER_H_
#define INCLUDE_RENDERER_H_

#include "Camera.h"
#include "Model.h"
#include <glm/ext/matrix_transform.hpp>
#include <initializer_list>
#include <memory>
#include <unordered_map>
#include <utility>

class Renderer {
private:
    std::unordered_map<unsigned int, std::unordered_map<int, std::shared_ptr<Model>>> renderQueue;
    std::unordered_map<unsigned int, std::shared_ptr<Shader>> shaders;
    glm::mat4& projection;
    glm::mat4 cameraMatrix;
    Terrain& terrain;

public:
    std::vector<glm::vec3> pointLightPositions = {
        glm::vec3(300, 200, 100),
        glm::vec3(200.0f, 69, 400),
    };
    glm::vec3 lightPos;
    std::shared_ptr<Camera> cam;
    glm::mat4 lightSpaceMatrix;

    Renderer(glm::mat4 projection, std::shared_ptr<Camera> cam, Terrain& terrain)
        : projection(projection)
        , cam(std::move(cam))
        , terrain(terrain)
    {
        lightPos = glm::vec3(-10.0f, 100.0f, -10.0f);
    }

    void enqueue(const Shader& shader, std::initializer_list<std::shared_ptr<Model>> model)
    {
        for (auto& m : model) {
            renderQueue[shader.ID][m->id] = m;
            shaders[shader.ID] = std::make_shared<Shader>(shader);
        }
    }

    void enqueue(const Shader& shader, const std::shared_ptr<Model>& model)
    {
        renderQueue[shader.ID][model->id] = model;
        shaders[shader.ID] = std::make_shared<Shader>(shader);
    }

    void lightingShader(Shader& lightingShader)
    {
        lightingShader.setVec3("lightPos", lightPos);
        lightingShader.setVec3("viewPos", cam->position);
        lightingShader.setFloat("material.shininess", 300.0f);

        // Set directional light properties
        lightingShader.setVec3("dirLight.direction", lightPos);
        lightingShader.setVec3("dirLight.ambient", 0.2f, 0.2f, 0.2f);
        lightingShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        lightingShader.setVec3("dirLight.specular", 0.8f, 0.8f, 0.8f);

        // Set first point light properties
        lightingShader.setVec3("pointLights[0].position", pointLightPositions[0]);
        lightingShader.setVec3("pointLights[0].ambient", 0.6f, 0.6f, 0.2f);
        lightingShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
        lightingShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("pointLights[0].constant", 1.0f);
        lightingShader.setFloat("pointLights[0].linear", 0.09f);
        lightingShader.setFloat("pointLights[0].quadratic", 0.032f);

        // Set second point light properties
        lightingShader.setVec3("pointLights[1].position", pointLightPositions[1]);
        lightingShader.setVec3("pointLights[1].ambient", 0.6f, 0.6f, 0.2f);
        lightingShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
        lightingShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("pointLights[1].constant", 1.0f);
        lightingShader.setFloat("pointLights[1].linear", 0.09f);
        lightingShader.setFloat("pointLights[1].quadratic", 0.032f);
    }

    void addModel(int shaderId, const std::shared_ptr<Model>& model)
    {
        renderQueue[shaderId][model->id] = model;
    }

    void removeModel(unsigned int shaderId, const std::shared_ptr<Model>& model)
    {
        renderQueue[shaderId].erase(model->id);
    }

    void renderShadowMap(Shader& shader)
    {
        for (auto& [key, value] : renderQueue) {
            for (auto [key, modelPtr] : value) {

                if (modelPtr->isInstanced)
                    continue;
                auto model = glm::mat4(1.0f);
                model = glm::translate(model, modelPtr->position);
                model = glm::rotate(model, glm::radians(modelPtr->yaw), glm::vec3(0, 1, 0));
                model = glm::rotate(model, glm::radians(modelPtr->pitch), glm::vec3(1, 0, 0));
                model = glm::rotate(model, glm::radians(modelPtr->roll), glm::vec3(0, 0, 1));
                shader.setMat4("model", model);
                modelPtr->draw(shader);
            }
        }
    }

    void renderAll()
    {
        for (auto& [key, value] : renderQueue) {
            auto shader = shaders.at(key);
            shader->use();

            this->lightingShader(*shader);
            shader->setMat4("projection", projection);
            shader->setMat4("view", cam->getCameraView());
            shader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
            for (auto [key, modelPtr] : value) {
                if (modelPtr->isInstanced) {
                    shader->setBool("isInstanced", true);
                    modelPtr->drawInstanced(*shader);
                } else {
                    shader->setBool("isInstanced", false);
                    auto model = glm::mat4(1.0f);
                    model = glm::translate(model, modelPtr->position);
                    model = glm::rotate(model, glm::radians(modelPtr->yaw), glm::vec3(0, 1, 0));
                    model = glm::rotate(model, glm::radians(modelPtr->pitch), glm::vec3(1, 0, 0));
                    model = glm::rotate(model, glm::radians(modelPtr->roll), glm::vec3(0, 0, 1));
                    shader->setMat4("model", model);
                    modelPtr->draw(*shader);
                }
            }
        }
    }

    void clear()
    {
        for (auto& [key, value] : renderQueue) {
            value.clear();
        }
        renderQueue.clear();
        shaders.clear();
    }
};

#endif // INCLUDE_INCLUDE_RENDERER_H_
