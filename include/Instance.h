#ifndef INCLUDE_INCLUDE_INSTANCE_H_
#define INCLUDE_INCLUDE_INSTANCE_H_

#include "Terrain.h"
#include <glm/ext/matrix_transform.hpp>

void initTreeTranslations(int amount, float offset, float radius, std::vector<glm::mat4>& translations, glm::vec3 housePosition, Terrain& terrain)
{
    for (int i = 0; i < amount; i++) {
        glm::mat4 model = glm::mat4(1.0f);
        float angle = (float)i / (float)amount * 360.0f;
        angle = glm::radians(angle);

        float xDisplacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float zDisplacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;

        float x = sin(angle) * radius + xDisplacement;
        float z = cos(angle) * radius + zDisplacement;

        x += housePosition.x;
        z += housePosition.z;

        if (x < 0.0f || x > terrain.terrainSize || z < 0.0f || z > terrain.terrainSize)
            continue;
        model = glm::translate(model, glm::vec3(x, -terrain.GetHeightInterpolated(x, z), z));
        translations[i] = model;
    }
}

void initMultiTree(int amount, int numModels, float offset, float radius, std::vector<std::vector<glm::mat4>>& translations, glm::vec3 housePosition, Terrain& terrain)
{
    translations.resize(numModels);
    for (int i = 0; i < amount; i++) {
        int randomTreeIndex = rand() % numModels;
        glm::mat4 model = glm::mat4(1.0f);
        float angle = (float)i / (float)amount * 360.0f;
        angle = glm::radians(angle);

        float xDisplacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float zDisplacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;

        float x = sin(angle) * radius + xDisplacement;
        float z = cos(angle) * radius + zDisplacement;

        x += housePosition.x;
        z += housePosition.z;

        if (x < 0.0f || x > terrain.terrainSize || z < 0.0f || z > terrain.terrainSize)
            continue;
        model = glm::translate(model, glm::vec3(x, -terrain.GetHeightInterpolated(x, z), z));
        translations[randomTreeIndex].push_back(model);
    }
}
/*
unsigned int buffer;
glGenBuffers(1, &buffer);
glBindBuffer(GL_ARRAY_BUFFER, buffer);
glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), translations.data(), GL_STATIC_DRAW);

for (unsigned int i = 0; i < pModel->meshes.size(); i++) {
    unsigned int VAO = pModel->meshes[i].VAO;
    glBindVertexArray(VAO);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);
    glBindVertexArray(0);
}
}
*/

#endif
