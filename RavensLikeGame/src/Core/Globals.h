#pragma once
#include <GLFW/glfw3.h>

namespace Globals
{
    inline constexpr float kCameraFollowSmoothness = 50.0f;
    inline constexpr float kPlayerFootHitboxHeightPx = 3.0f;
	inline constexpr float kPlayerFootHitboxWidthPx = 13.0f;
    inline constexpr float kTileWalkableWidth = 693.0f;
    inline constexpr float kTileWalkableHeight = 400.0f;

    inline constexpr float kClampBiasTilesX = 0.9f;
    inline constexpr float kClampBiasTilesY = 0.9f;

    constexpr unsigned int WindowWidth = 1600;
    constexpr unsigned int WindowHeight = 900;
    constexpr const char* WindowTitle = "RavensLikeGame";

    inline float DashDistance = 180.0f;
    inline float DashDurationInSeconds = 0.2f;
    inline float DashCooldownInSeconds = 3.0f;

    inline int KeyMoveUp = GLFW_KEY_W;
    inline int KeyMoveDown = GLFW_KEY_S;
    inline int KeyMoveLeft = GLFW_KEY_A;
    inline int KeyMoveRight = GLFW_KEY_D;
	inline int DashKey = GLFW_KEY_SPACE;
    inline int DecreaseHealth = GLFW_KEY_M;
}