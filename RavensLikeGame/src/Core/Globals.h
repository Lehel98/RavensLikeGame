#pragma once
#include <GLFW/glfw3.h>

namespace Globals
{
    // Ablak m�retek
    constexpr unsigned int WindowWidth = 800;
    constexpr unsigned int WindowHeight = 600;
    constexpr const char* WindowTitle = "RavensLikeGame";

    // Alap billenty�k (k�s�bb ak�r konfigur�lhat�ak lesznek)
    inline int KeyMoveUp = GLFW_KEY_W;
    inline int KeyMoveDown = GLFW_KEY_S;
    inline int KeyMoveLeft = GLFW_KEY_A;
    inline int KeyMoveRight = GLFW_KEY_D;
}