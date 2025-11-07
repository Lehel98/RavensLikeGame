#include "UIRenderer.h"
#include "../Core/Globals.h"
#include <glad/glad.h>

UIRenderer::UIRenderer(Shader& shader) : shader(shader) {
    InitRenderData();
}

UIRenderer::~UIRenderer() {
    glDeleteVertexArrays(1, &vaoHealthBarBorder);
    glDeleteBuffers(1, &vboHealthBarBorder);

    glDeleteVertexArrays(1, &vaoHealthBarBackground);
    glDeleteBuffers(1, &vboHealthBarBackground);

    glDeleteVertexArrays(1, &vaoHealthBarFill);
    glDeleteBuffers(1, &vboHealthBarFill);
}

void UIRenderer::InitRenderData()
{
    InitHealthBarBorderVAO();
    InitHealthBarBackgroundVAO();
    InitHealthBarForegroundVAO();
}

void UIRenderer::InitHealthBarBorderVAO()
{
	float offset = defaultOffset + 5.0f;
    const float whiteW = 308.0f;
    const float whiteH = 33.0f;

    float whiteVertices[] = {
        offset,        0.0f,
        0.0f,         whiteH,
        whiteW,       whiteH,
        whiteW + offset, 0.0f
    };

    glGenVertexArrays(1, &vaoHealthBarBorder);
    glGenBuffers(1, &vboHealthBarBorder);
    glBindVertexArray(vaoHealthBarBorder);
    glBindBuffer(GL_ARRAY_BUFFER, vboHealthBarBorder);
    glBufferData(GL_ARRAY_BUFFER, sizeof(whiteVertices), whiteVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void UIRenderer::InitHealthBarBackgroundVAO()
{
	float normalizedOffset = defaultOffset / 300.0f;
    float unitParallelogram[] = {
        normalizedOffset, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f + normalizedOffset, 0.0f
    };

    glGenVertexArrays(1, &vaoHealthBarBackground);
    glGenBuffers(1, &vboHealthBarBackground);
    glBindVertexArray(vaoHealthBarBackground);
    glBindBuffer(GL_ARRAY_BUFFER, vboHealthBarBackground);
    glBufferData(GL_ARRAY_BUFFER, sizeof(unitParallelogram), unitParallelogram, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void UIRenderer::InitHealthBarForegroundVAO()
{
    glGenVertexArrays(1, &vaoHealthBarFill);
    glGenBuffers(1, &vboHealthBarFill);
    glBindVertexArray(vaoHealthBarFill);
    glBindBuffer(GL_ARRAY_BUFFER, vboHealthBarFill);
    glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void UIRenderer::DrawHealthBar(int currentHealth, int maxHealth)
{
    float ratio = glm::clamp((float)currentHealth / (float)maxHealth, 0.0f, 1.0f);

    float screenWidth = Globals::WindowWidth;
    float screenHeight = Globals::WindowHeight;
    glm::vec2 whitePos(screenWidth - innerBarWidth - 36.5f, 16.0f);
    glm::vec2 grayPos(screenWidth - innerBarWidth - 30.0f, 20.0f);

    SetupUIRendering(screenWidth, screenHeight);

    DrawHealthBarBorder(whitePos);
    DrawHealthBarBackground(grayPos);

    if (currentHealth > 0 && currentHealth <= maxHealth)
        DrawHealthBarForeground(grayPos, ratio);

    RestoreDefaultShaderState();
}

void UIRenderer::SetupUIRendering(float screenWidth, float screenHeight)
{
    shader.Use();
    glUniform1i(glGetUniformLocation(shader.ID, "useView"), false);
    glUniform1i(glGetUniformLocation(shader.ID, "useColorOnly"), true);

    glm::mat4 projection = glm::ortho(0.0f, screenWidth, 0.0f, screenHeight, -1.0f, 1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, &view[0][0]);
}

void UIRenderer::DrawHealthBarBorder(const glm::vec2& position)
{
    glBindVertexArray(vaoHealthBarBorder);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position, uiLayerDepth));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, &model[0][0]);

    glm::vec3 borderColor = glm::vec3(1.0f, 1.0f, 1.0f);
    glUniform3fv(glGetUniformLocation(shader.ID, "spriteColor"), 1, &borderColor[0]);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}

void UIRenderer::DrawHealthBarBackground(const glm::vec2& position)
{
    glBindVertexArray(vaoHealthBarBackground);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position, uiLayerDepth - 0.01f));
    model = glm::scale(model, glm::vec3(innerBarWidth, innerBarHeight, 1.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, &model[0][0]);

    glm::vec3 backgroundColor = glm::vec3(0.3f, 0.3f, 0.3f);
    glUniform3fv(glGetUniformLocation(shader.ID, "spriteColor"), 1, &backgroundColor[0]);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}

void UIRenderer::DrawHealthBarForeground(const glm::vec2& position, float ratio)
{
    float visibleWidth = innerBarWidth * ratio;
    float missingWidth = innerBarWidth - visibleWidth;

    float greenVertices[] = {
        missingWidth + defaultOffset, 0.0f,
        missingWidth, innerBarHeight,
        innerBarWidth, innerBarHeight,
        innerBarWidth + defaultOffset, 0.0f
    };

    glBindBuffer(GL_ARRAY_BUFFER, vboHealthBarFill);
    glBufferData(GL_ARRAY_BUFFER, sizeof(greenVertices), nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(greenVertices), greenVertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(vaoHealthBarFill);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position, uiLayerDepth - 0.02f));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, &model[0][0]);

    glm::vec3 foregroundColor = glm::vec3(0.0f, 1.0f, 0.0f);
    glUniform3fv(glGetUniformLocation(shader.ID, "spriteColor"), 1, &foregroundColor[0]);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}

void UIRenderer::RestoreDefaultShaderState()
{
    glUniform1i(glGetUniformLocation(shader.ID, "useView"), true);
    glUniform1i(glGetUniformLocation(shader.ID, "useColorOnly"), false);
}