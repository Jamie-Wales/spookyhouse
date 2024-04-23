#include "Terrain.h"
#include "./utils/Utilities.h"
#include "./utils/Vertex.h"
#include "Shader.h"
#include <GL/glew.h>

Terrain::Terrain(float scale)
    : scale(scale)
{
    LoadFromFile("../assets/heightmap.save");
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    int position = 0;
    size_t numfloats = 0;
    glEnableVertexAttribArray(position);
    glVertexAttribPointer(position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(numfloats * sizeof(float)));
    numfloats += 3;
    populateBuffer();
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Terrain::LoadFromFile(const std::string& filename)
{
    int fileSize = 0;
    char* data = readBinaryFile(*filename.c_str(), fileSize);
    if (data == nullptr) {
        return;
    }
    auto* ptr = reinterpret_cast<unsigned char*>(data);
    terrainSize = sqrt((fileSize / sizeof(float)));
    heightMap = Array2D<float>(terrainSize, terrainSize, (float*)ptr);
    height = terrainSize;
    width = terrainSize;
    delete[] data;
}

float Terrain::getHeight(int x, int z)
{
    return heightMap(x, z) * scale;
}

void Terrain::populateBuffer()
{
    std::vector<Vertex> vertices;
    vertices.resize(height * width);

    int index = 0;
    for (int x = 0; x < width; x++) {
        for (int z = 0; z < height; z++) {
            Vertex vertex;
            vertex.init(*this, x, z, scale);
            vertices[index] = vertex;
            index++;
        }
    }

    std::vector<unsigned int> indices;
    int numQuads = (width - 1) * (height - 1);
    indices.resize(numQuads * 6);
    int i = 0;
    for (int x = 0; x < width - 1; x++) {
        for (int z = 0; z < height - 1; z++) {
            unsigned int topLeft = (z * width) + x;
            unsigned int topRight = topLeft + 1;
            unsigned int bottomLeft = ((z + 1) * width) + x;
            unsigned int bottomRight = bottomLeft + 1;
            indices[i++] = topLeft;
            indices[i++] = bottomLeft;
            indices[i++] = topRight;
            indices[i++] = topRight;
            indices[i++] = bottomLeft;
            indices[i++] = bottomRight;
        }
    }
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
}

void Terrain::render(glm::mat4 proj, glm::mat4 view) const
{
    terrainShader.use();
    terrainShader.setMat4("projection", proj);
    terrainShader.setMat4("view", view);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, (width - 1) * (height - 1) * 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
