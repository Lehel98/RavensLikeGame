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

    const glm::mat4& GetProjection() const { return projection; }
    const glm::mat4& GetView() const { return view; }

    void SetProjection(const glm::mat4& projection);
    void SetView(const glm::mat4& view);

    glm::vec2 ComputeMapOrigin(int rows, int cols) const;

    float ScaledWidth()  const { return tileWidth * tileScale; }
    float ScaledHeight()  const { return tileHeight * tileScale; }
    float ScaledVisibleHeight() const { return tileVisibleHeight * tileScale; }

    float GetHalfTileWidth()  const { return ScaledWidth() * 0.5f; }
    float GetHalfTileHeight() const { return ScaledVisibleHeight() * 0.5f; }

private:
    Shader& shader;
    unsigned int vao, vbo;
    unsigned int textureID;

    const float tileWidth = 693;
    const float tileHeight = 560;
    const float tileVisibleHeight = 400;
    const int tileCount = 3;

    const float tileScale = 0.5f;

    glm::mat4 projection;
    glm::mat4 view;

    void DrawTile(int tileIndex, const glm::vec2& worldPos);
    void LoadTexture(const std::string& path);
    void InitRenderData();
};
