#pragma once
#include <GLFW/glfw3.h>

namespace Globals
{
    constexpr unsigned int WindowWidth = 800;
    constexpr unsigned int WindowHeight = 600;
    constexpr const char* WindowTitle = "RavensLikeGame";

    inline int KeyMoveUp = GLFW_KEY_W;
    inline int KeyMoveDown = GLFW_KEY_S;
    inline int KeyMoveLeft = GLFW_KEY_A;
    inline int KeyMoveRight = GLFW_KEY_D;
}