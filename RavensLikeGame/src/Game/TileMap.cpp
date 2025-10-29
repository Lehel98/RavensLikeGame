#include "TileMap.h"
#include <fstream>
#include <iostream>

bool TileMap::Load(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to load map: " << path << std::endl;
        return false;
    }

    file >> mapHeight >> mapWidth;

    if (mapHeight <= 0 || mapWidth <= 0) {
        std::cerr << "Invalid map size in file: " << path << std::endl;
        return false;
    }

    tiles.resize(mapHeight, std::vector<int>(mapWidth));

    for (int y = 0; y < mapHeight; y++) {
        for (int x = 0; x < mapWidth; x++) {
            file >> tiles[y][x];
        }
    }

    tileTextures[0].LoadFromFile("assets/textures/tiles/grass.png");
    tileTextures[1].LoadFromFile("assets/textures/tiles/dirt.png");
    tileTextures[2].LoadFromFile("assets/textures/tiles/water.png");
    tileTextures[3].LoadFromFile("assets/textures/tiles/rock.png");

    return true;
}

void TileMap::Draw(SpriteRenderer& renderer) {
    for (int y = 0; y < mapHeight; y++) {
        for (int x = 0; x < mapWidth; x++) {
            int id = tiles[y][x];
            if (id >= 0 && id < 4)
                renderer.DrawSprite(tileTextures[id],
                    glm::vec2(x * tileSize, y * tileSize),
                    glm::vec2(tileSize, tileSize));
        }
    }
}

bool TileMap::IsWalkable(float worldX, float worldY) const
{
    int tileX = static_cast<int>(worldX / tileSize);
    int tileY = static_cast<int>(worldY / tileSize);

    if (tileX < 0 || tileY < 0 || tileX >= mapWidth || tileY >= mapHeight)
        return false; // pályán kívül nem járható

    int tileID = tiles[tileY][tileX];
    return walkable[tileID];
}

bool TileMap::IsAreaWalkable(float x, float y, float width, float height) const
{
    // Négy sarok vizsgálata
    glm::vec2 corners[4] = {
        { x, y },
        { x + width, y },
        { x, y + height },
        { x + width, y + height }
    };

    for (auto& c : corners)
    {
        if (!IsWalkable(c.x, c.y))
            return false;
    }

    return true;
}