#pragma once
#include <glm.hpp>
#include <array>
#include <vector>
#include "../Renderer/Texture.h"
#include "../Renderer/SpriteRenderer.h"
#include "../Core/Globals.h"

class Character8Direction {
public:
    static constexpr int kDirections = 8;
    static constexpr int kFramesPerDirection = 3;

    enum Directions { N = 0, NE = 1, E = 2, SE = 3, S = 4, SW = 5, W = 6, NW = 7 };

    Character8Direction(Texture& sheet, SpriteRenderer& renderer);

    void Update(const glm::vec2& movementDir, float deltaTime);
    void DrawPlayer(const glm::vec2& centerPosition, const glm::vec2& pictureSize);

    int GetCurrentDirection() const;
    glm::vec2 GetCurrentDirectionVector() const;

private:
    Texture& sheet;
    SpriteRenderer& renderer;

    float spriteSheetWidthPx = 1.0f, spriteSheetHeightPx = 1.0f;

    std::array<std::array<glm::vec4, kFramesPerDirection>, kDirections> uvFrames{};

    inline static constexpr std::array<int, 9> kQuantizedAxesToDirectionLookUpTable = {
        SW, S, SE, W, -1, E, NW, N, NE
    };

    int currentDirection = S;
    int currentFrame = 1;
    float frameTime = 0.0f;
    float frameDuration = 0.12f;  // 8–9 FPS körül

    static glm::vec2 NormalizeVector(const glm::vec2& v);
    static int DirectionFromMovement(const glm::vec2& v);

    static glm::vec4 PixelRectToNormalizedUVRect(int x1, int y1, int x2, int y2, float sheetWidth, float sheetHeight);

    void BuildFramesFromGivenCoords();
};