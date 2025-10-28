#pragma once
#include "Shader.h"
#include "Texture.h"
#include <glm.hpp>

class SpriteRenderer
{
public:
    SpriteRenderer(const Shader& shader);
    ~SpriteRenderer();

    void DrawSprite(const Texture& texture, const glm::vec2& position, const glm::vec2& size, float rotation = 0.0f);

private:
    Shader shader;
    unsigned int quadVAO;

    void InitRenderData();
};