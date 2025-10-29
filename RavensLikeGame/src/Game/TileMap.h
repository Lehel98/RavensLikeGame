#pragma once
#include <vector>
#include <string>
#include <glm.hpp>
#include "../Renderer/Texture.h"
#include "../Renderer/SpriteRenderer.h"

class TileMap {
public:
    bool Load(const std::string& path);
    void Draw(SpriteRenderer& renderer);
    bool IsWalkable(float worldX, float worldY) const;
    bool IsAreaWalkable(float x, float y, float width, float height) const;
private:
    std::vector<std::vector<int>> tiles;
    Texture tileTextures[4];
    bool walkable[4] = { true, true, false, false };
    int mapWidth = 0, mapHeight = 0;
    float tileSize = 64.0f;
};