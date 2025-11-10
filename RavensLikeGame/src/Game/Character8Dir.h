#pragma once
#include <glm.hpp>
#include <array>
#include <vector>
#include "../Renderer/Texture.h"
#include "../Renderer/SpriteRenderer.h"
#include "../Core/Globals.h"

class Character8Dir {
public:
    // 8 irány × 3 frame
    static constexpr int kDirs = 8;
    static constexpr int kFramesPerDir = 3;

    // direction indexek: 0=N,1=NE,2=E,3=SE,4=S,5=SW,6=W,7=NW
    enum Directions { N = 0, NE = 1, E = 2, SE = 3, S = 4, SW = 5, W = 6, NW = 7 };

    Character8Dir(Texture& sheet, SpriteRenderer& renderer);

    void Update(const glm::vec2& movementDir, float deltaTime);

    void DrawPlayer(const glm::vec2& centerPosition, const glm::vec2& pictureSize);

private:
    Texture& sheet;
    SpriteRenderer& renderer;

    float spriteSheetWidthPx = 1.0f, spriteSheetHeightPx = 1.0f;

    std::array<std::array<glm::vec4, kFramesPerDir>, kDirs> uvFrames{};

    inline static constexpr std::array<int, 9> kQuantizedAxesToDirectionLookUpTable = {
        SW, S, SE, W, -1, E, NW, N, NE
    };

    int currentDir = S;           // kezdetben lefelé néz
    int currentFrame = 1;         // középső frame
    float frameTime = 0.0f;
    float frameDuration = 0.12f;  // 8–9 FPS körül

    static glm::vec2 NormalizeVector(const glm::vec2& v);
    static int DirectionFromMovement(const glm::vec2& v);

    static glm::vec4 PixelRectToNormalizedUVRect(int x1, int y1, int x2, int y2, float sheetWidth, float sheetHeight);

    void BuildFramesFromGivenCoords();
};