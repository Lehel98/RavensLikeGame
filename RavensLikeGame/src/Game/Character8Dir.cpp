#include "Character8Dir.h"
#include <algorithm>
#include <cmath>

Character8Dir::Character8Dir(Texture& s, SpriteRenderer& r)
    : sheet(s), renderer(r)
{
    spriteSheetWidthPx = static_cast<float>(sheet.Width);
    spriteSheetHeightPx = static_cast<float>(sheet.Height);
    BuildFramesFromGivenCoords();
}

glm::vec2 Character8Dir::NormalizeVector(const glm::vec2& v)
{
    float len = std::sqrt(v.x * v.x + v.y * v.y);
    return (len > 0.0f) ? (v / len) : glm::vec2(0.0f);
}

int Character8Dir::DirectionFromMovement(const glm::vec2& v)
{
    constexpr float kZeroEpsilon = 1e-4f;

    const int ix = (v.x > kZeroEpsilon) - (v.x < -kZeroEpsilon);
    const int iy = (v.y > kZeroEpsilon) - (v.y < -kZeroEpsilon);

    if (ix == 0 && iy == 0) return -1;

    const int key = (iy + 1) * 3 + (ix + 1);
    return kQuantizedAxesToDirectionLookUpTable[key];
}

glm::vec4 Character8Dir::PixelRectToNormalizedUVRect(int x1, int y1, int x2, int y2, float sheetWidth, float sheetHeight)
{
    float u0 = (x1 - 1) / sheetWidth;
    float v0 = (y1 - 1) / sheetHeight;
    float u1 = (x2) / sheetWidth;
    float v1 = (y2) / sheetHeight;
    return glm::vec4(u0, v0, u1, v1);
}

void Character8Dir::BuildFramesFromGivenCoords()
{
    // A megadott koordináták (teljes sheet mérete: 125x51 px)
    // 1) FEL (N):
    uvFrames[N][0] = PixelRectToNormalizedUVRect(1, 1, 13, 16, spriteSheetWidthPx, spriteSheetHeightPx);
    uvFrames[N][1] = PixelRectToNormalizedUVRect(1, 18, 13, 34, spriteSheetWidthPx, spriteSheetHeightPx);
    uvFrames[N][2] = PixelRectToNormalizedUVRect(1, 36, 13, 51, spriteSheetWidthPx, spriteSheetHeightPx);

    // 2) JOBB-FEL (NE):
    uvFrames[NE][0] = PixelRectToNormalizedUVRect(17, 1, 29, 16, spriteSheetWidthPx, spriteSheetHeightPx);
    uvFrames[NE][1] = PixelRectToNormalizedUVRect(17, 18, 29, 34, spriteSheetWidthPx, spriteSheetHeightPx);
    uvFrames[NE][2] = PixelRectToNormalizedUVRect(17, 36, 29, 51, spriteSheetWidthPx, spriteSheetHeightPx);

    // 3) JOBB (E):
    uvFrames[E][0] = PixelRectToNormalizedUVRect(34, 1, 44, 16, spriteSheetWidthPx, spriteSheetHeightPx);
    uvFrames[E][1] = PixelRectToNormalizedUVRect(34, 18, 44, 34, spriteSheetWidthPx, spriteSheetHeightPx);
    uvFrames[E][2] = PixelRectToNormalizedUVRect(34, 36, 44, 51, spriteSheetWidthPx, spriteSheetHeightPx);

    // 4) JOBB-LE (SE):
    uvFrames[SE][0] = PixelRectToNormalizedUVRect(48, 1, 61, 16, spriteSheetWidthPx, spriteSheetHeightPx);
    uvFrames[SE][1] = PixelRectToNormalizedUVRect(48, 18, 61, 34, spriteSheetWidthPx, spriteSheetHeightPx);
    uvFrames[SE][2] = PixelRectToNormalizedUVRect(48, 36, 61, 51, spriteSheetWidthPx, spriteSheetHeightPx);

    // 5) LE (S):
    uvFrames[S][0] = PixelRectToNormalizedUVRect(65, 1, 77, 16, spriteSheetWidthPx, spriteSheetHeightPx);
    uvFrames[S][1] = PixelRectToNormalizedUVRect(65, 18, 77, 34, spriteSheetWidthPx, spriteSheetHeightPx);
    uvFrames[S][2] = PixelRectToNormalizedUVRect(65, 36, 77, 51, spriteSheetWidthPx, spriteSheetHeightPx);

    // 6) BAL-LE (SW):
    uvFrames[SW][0] = PixelRectToNormalizedUVRect(81, 1, 93, 16, spriteSheetWidthPx, spriteSheetHeightPx);
    uvFrames[SW][1] = PixelRectToNormalizedUVRect(81, 18, 93, 34, spriteSheetWidthPx, spriteSheetHeightPx);
    uvFrames[SW][2] = PixelRectToNormalizedUVRect(81, 36, 93, 51, spriteSheetWidthPx, spriteSheetHeightPx);

    // 7) BAL (W):
    uvFrames[W][0] = PixelRectToNormalizedUVRect(98, 1, 108, 16, spriteSheetWidthPx, spriteSheetHeightPx);
    uvFrames[W][1] = PixelRectToNormalizedUVRect(98, 18, 108, 34, spriteSheetWidthPx, spriteSheetHeightPx);
    uvFrames[W][2] = PixelRectToNormalizedUVRect(98, 36, 108, 51, spriteSheetWidthPx, spriteSheetHeightPx);

    // 8) BAL-FEL (NW):
    uvFrames[NW][0] = PixelRectToNormalizedUVRect(113, 1, 125, 16, spriteSheetWidthPx, spriteSheetHeightPx);
    uvFrames[NW][1] = PixelRectToNormalizedUVRect(113, 18, 125, 34, spriteSheetWidthPx, spriteSheetHeightPx);
    uvFrames[NW][2] = PixelRectToNormalizedUVRect(113, 36, 125, 51, spriteSheetWidthPx, spriteSheetHeightPx);
}

void Character8Dir::Update(const glm::vec2& move, float deltaTime)
{
    glm::vec2 dir = NormalizeVector(move);
    int newDir = DirectionFromMovement(dir);
    if (newDir >= 0) currentDir = newDir;

    if (newDir < 0) {
        currentFrame = 1;
        frameTime = 0.0f;
        return;
    }

    frameTime += deltaTime;
    while (frameTime >= frameDuration) {
        frameTime -= frameDuration;
        currentFrame = (currentFrame + 1) % kFramesPerDir;
    }
}

void Character8Dir::DrawPlayer(const glm::vec2& centerPosition, const glm::vec2& pictureSize)
{
    const glm::vec4 uv = uvFrames[currentDir][currentFrame];
    glm::vec2 drawPos = centerPosition - pictureSize * 0.5f;
    renderer.DrawSpriteRegion(sheet, drawPos, pictureSize, uv);
}