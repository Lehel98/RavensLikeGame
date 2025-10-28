#include "SpriteRenderer.h"
#include <glad/glad.h>
#include <gtc/matrix_transform.hpp>

SpriteRenderer::SpriteRenderer(const Shader& shader)
    : shader(shader)
{
    InitRenderData();
}

SpriteRenderer::~SpriteRenderer() {
    glDeleteVertexArrays(1, &quadVAO);
}

void SpriteRenderer::InitRenderData() {
    unsigned int VBO;
    float quadVertices[] = {
        // pos      // tex
        0.0f, 1.0f,  0.0f, 1.0f,
        1.0f, 0.0f,  1.0f, 0.0f,
        0.0f, 0.0f,  0.0f, 0.0f,

        0.0f, 1.0f,  0.0f, 1.0f,
        1.0f, 1.0f,  1.0f, 1.0f,
        1.0f, 0.0f,  1.0f, 0.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glBindVertexArray(quadVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void SpriteRenderer::DrawSprite(const Texture& texture, const glm::vec2& position, const glm::vec2& size, float rotation) {
    shader.Use();
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position, 0.0f));
    model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f));
    model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));
    model = glm::scale(model, glm::vec3(size, 1.0f));

    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, &model[0][0]);

    texture.Bind();
    glUniform1i(glGetUniformLocation(shader.ID, "sprite"), 0); // 🟢 fontos sor
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    texture.Unbind();
}