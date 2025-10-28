#include "TileMap.h"
#include <fstream>
#include <iostream>

bool TileMap::Load(const std::string& path, int width, int height) {
    mapWidth = width;
    mapHeight = height;
    tiles.resize(height, std::vector<int>(width));

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to load map: " << path << std::endl;
        return false;
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
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