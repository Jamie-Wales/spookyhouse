#ifndef CUBE_H
#define CUBE_H

#include "BoundingBox.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

class Cube {
public:
    GLuint VAO, VBO, EBO;

    explicit Cube(const BoundingBox& box)
    {
        std::vector<glm::vec3> vertices;

        std::vector<unsigned int> indices = {
            0, 1, 2, 2, 1, 3, // Front face
            1, 5, 3, 5, 7, 3, // Right face
            5, 4, 7, 4, 6, 7, // Back face
            4, 0, 6, 0, 2, 6, // Left face
            2, 3, 6, 3, 7, 6, // Top face
            0, 4, 1, 1, 4, 5 // Bottom face
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }

    ~Cube()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

    void draw()
    {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
};

#endif // CUBE_H
