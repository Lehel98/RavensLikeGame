#pragma once
#include "Shader.h"
#include <array>
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

    float ScaledWidth()  const { return kTileWidth * kTileScale; }
    float ScaledHeight()  const { return kTileHeight * kTileScale; }
    float ScaledVisibleHeight() const { return kTileVisibleHeight * kTileScale; }

    float GetHalfTileWidth()  const { return ScaledWidth() * 0.5f; }
    float GetHalfTileHeight() const { return ScaledVisibleHeight() * 0.5f; }

private:
    std::array<glm::vec4, 4> tileUvRects;

    Shader& shader;
    unsigned int vao, vbo;
    unsigned int textureID;

    static constexpr float kTileWidth = 693;
    static constexpr float kTileHeight = 560;
    static constexpr float kTileVisibleHeight = 400;
    static constexpr int kTileCount = 4;
    static constexpr float kTileScale = 0.5f;

    glm::mat4 projection;
    glm::mat4 view;

    void DrawTile(int tileIndex, const glm::vec2& worldPos);
    void LoadTexture(const std::string& path);
    void InitRenderData();
};