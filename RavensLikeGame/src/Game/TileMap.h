#pragma once
#include <vector>
#include <string>
#include <glm.hpp>
#include "../Renderer/Texture.h"
#include "../Renderer/SpriteRenderer.h"

class TileMap {
public:
    bool Load(const std::string& path, int width, int height);
    void Draw(SpriteRenderer& renderer);
private:
    std::vector<std::vector<int>> tiles;
    Texture tileTextures[4];
    int mapWidth, mapHeight;
    float tileSize = 64.0f;
};