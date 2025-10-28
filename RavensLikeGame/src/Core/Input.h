#pragma once
#include <GLFW/glfw3.h>
#include "Globals.h"

namespace Input
{
    inline bool MoveUp = false;
    inline bool MoveDown = false;
    inline bool MoveLeft = false;
    inline bool MoveRight = false;

    inline void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (key == Globals::KeyMoveUp)
            MoveUp = (action != GLFW_RELEASE);
        if (key == Globals::KeyMoveDown)
            MoveDown = (action != GLFW_RELEASE);
        if (key == Globals::KeyMoveLeft)
            MoveLeft = (action != GLFW_RELEASE);
        if (key == Globals::KeyMoveRight)
            MoveRight = (action != GLFW_RELEASE);

        // ESC: kilépés
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
    }
}