#ifndef SPHERE_H
#define SPHERE_H

#include <glad/glad.h>
#include <vector>
#include <cmath>

class Sphere
{
public:
    unsigned int VAO, VBO, EBO;
    unsigned int indexCount;

    Sphere(unsigned int xSegments = 50, unsigned int ySegments = 50)
    {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        const float PI = 3.14159265359f;

        // Generate sphere vertices with texture coordinates
        for (unsigned int y = 0; y <= ySegments; ++y)
        {
            for (unsigned int x = 0; x <= xSegments; ++x)
            {
                float xSegment = (float)x / (float)xSegments;
                float ySegment = (float)y / (float)ySegments;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                vertices.push_back(xPos);
                vertices.push_back(yPos);
                vertices.push_back(zPos);
                vertices.push_back(xPos); // Normal X
                vertices.push_back(yPos); // Normal Y
                vertices.push_back(zPos); // Normal Z
                vertices.push_back(xSegment); // Texture U
                vertices.push_back(ySegment); // Texture V
            }
        }

        // Generate indices
        for (unsigned int y = 0; y < ySegments; ++y)
        {
            for (unsigned int x = 0; x < xSegments; ++x)
            {
                indices.push_back(y * (xSegments + 1) + x);
                indices.push_back((y + 1) * (xSegments + 1) + x);
                indices.push_back((y + 1) * (xSegments + 1) + x + 1);

                indices.push_back(y * (xSegments + 1) + x);
                indices.push_back((y + 1) * (xSegments + 1) + x + 1);
                indices.push_back(y * (xSegments + 1) + x + 1);
            }
        }

        indexCount = indices.size();

        // Create buffers and arrays
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Normal attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // Texture coordinate attribute
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }

    void Draw()
    {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    ~Sphere()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }
};

#endif
