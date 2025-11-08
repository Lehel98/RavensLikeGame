#pragma once
#include "Shader.h"
#include <glm.hpp>
#include <string>
#include <vector>

class IsoRenderer
{
public:
    IsoRenderer(Shader& shader, const std::string& texturePath);
    ~IsoRenderer();

    void DrawMap(const std::vector<std::vector<int>>& mapData);

    void SetProjection(const glm::mat4& projection);
    void SetView(const glm::mat4& view);

private:
    Shader& shader;
    unsigned int vao, vbo;
    unsigned int textureID;

    const float tileWidth = 693;
    const float tileHeight = 560;
    const float tileVisibleHeight = 400;
    const int tileCount = 3;

    glm::mat4 projection;
    glm::mat4 view;

    void DrawTile(int tileIndex, const glm::vec2& worldPos);
    void LoadTexture(const std::string& path);
    void InitRenderData();
};
