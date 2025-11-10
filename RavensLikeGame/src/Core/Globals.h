#pragma once
#include <GLFW/glfw3.h>

namespace Globals
{
    inline float CameraFollowSmoothness = 50.0f;

    constexpr unsigned int WindowWidth = 1200;
    constexpr unsigned int WindowHeight = 800;
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