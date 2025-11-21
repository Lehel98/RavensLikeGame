#include "IsoRenderer.h"
#include <glad/glad.h>
#include <stb_image.h>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "../Core/Globals.h"
#include <iostream>
#include <algorithm>

IsoRenderer::IsoRenderer(Shader& shader, const std::string& texturePath)
    : shader(shader)
{
    LoadTexture(texturePath);
    InitRenderData();

    projection = glm::mat4(1.0f);
    view = glm::mat4(1.0f);

    const float atlasPixelWidth = static_cast<float>(kTileWidth * kTileCount);
    for (int i = 0; i < kTileCount; ++i) {
        const float uMin = (i * kTileWidth) / atlasPixelWidth;
        const float uMax = ((i + 1) * kTileWidth) / atlasPixelWidth;
        tileUvRects[i] = glm::vec4(uMin, 0.0f, uMax, 1.0f);
    }
}

IsoRenderer::~IsoRenderer()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteTextures(1, &textureID);
}

void IsoRenderer::SetProjection(const glm::mat4& proj)
{
    projection = proj;
}

void IsoRenderer::SetView(const glm::mat4& v)
{
    view = v;
}

void IsoRenderer::LoadTexture(const std::string& path)
{
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 4);
    if (!data) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
}

void IsoRenderer::InitRenderData()
{
    const float width = ScaledWidth();
    const float height = ScaledHeight();

    float vertices[] = {
        // positions       // texCoords
        0.0f, 0.0f,         0.0f, 0.0f,
        width, 0.0f,    1.0f, 0.0f,
        width, height, 1.0f, 1.0f,
        0.0f, height,   0.0f, 1.0f
    };
    unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

    unsigned int ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void IsoRenderer::DrawTile(int tileIndex, const glm::vec2& worldPos)
{
    shader.SetVec4("uvRect", tileUvRects[tileIndex]);

    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(worldPos.x, worldPos.y, 0.0f));
    shader.SetMat4("model", model);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void IsoRenderer::DrawMap(const std::vector<std::vector<int>>& mapData)
{
    const int rows = static_cast<int>(mapData.size());
    const int cols = static_cast<int>(mapData[0].size());

    const float width = ScaledWidth();
    const float visH = ScaledVisibleHeight();
    const float halfW = width * 0.5f;
    const float halfVisH = visH * 0.5f;

    const glm::vec2 origin = ComputeMapOrigin(rows, cols);

    shader.Use();
    shader.SetMat4("projection", projection);
    shader.SetMat4("view", view);

    glBindTexture(GL_TEXTURE_2D, textureID);
    glBindVertexArray(vao);

    const int maxS = (rows - 1) + (cols - 1);
    for (int s = maxS; s >= 0; --s) {
        int xStart = std::max(0, s - (rows - 1));
        int xEnd = std::min(cols - 1, s);
        for (int x = xEnd; x >= xStart; --x) {
            int y = s - x;
            int tile = mapData[y][x];
            if (tile < 0) continue;

            // tető (apex) helye
            const float apexX = origin.x + (x - y) * halfW;
            const float apexY = origin.y + (x + y) * halfVisH;

            // quad bal-felső sarka (innen rajzol a quad lefelé)
            const glm::vec2 topLeft(apexX - halfW, apexY);

            DrawTile(tile, topLeft);
        }
    }

    glBindVertexArray(0);
}

glm::vec2 IsoRenderer::ComputeMapOrigin(int rows, int cols) const
{
    const float width = ScaledWidth();
    const float visH = ScaledVisibleHeight();

    const float mapW = (cols + rows) * (width * 0.5f);
    const float mapH = (cols + rows) * (visH * 0.5f);

    return glm::vec2(
        Globals::WindowWidth * 0.5f - mapW * 0.5f + width * 0.5f,
        Globals::WindowHeight * 0.5f - mapH * 0.5f
    );
}