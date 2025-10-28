#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "Core/Globals.h"
#include "Core/Input.h"
#include "Renderer/Shader.h"
#include "Renderer/Camera.h"
#include "Renderer/Texture.h"
#include "Renderer/SpriteRenderer.h"
#include "Game/TileMap.h"

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoord = aTexCoord;
    gl_Position = projection * view * model * vec4(aPos, 0.0, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D sprite;

void main()
{
    FragColor = texture(sprite, TexCoord);
}
)";

int main()
{
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(
        Globals::WindowWidth, Globals::WindowHeight,
        Globals::WindowTitle, nullptr, nullptr
    );

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, Input::KeyCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        return -1;

    // Shaderek bet�lt�se
    Shader shader(vertexShaderSource, fragmentShaderSource);
    Camera camera(Globals::WindowWidth, Globals::WindowHeight);
    SpriteRenderer renderer(shader);

    // P�lya bet�lt�se
    TileMap map;
    if (!map.Load("assets/maps/test_map.txt", 8, 5))
    {
        std::cerr << "Map load failed!\n";
        return -1;
    }

    // J�t�kos text�ra bet�lt�se
    Texture playerTex;
    if (!playerTex.LoadFromFile("assets/textures/player/idle.png"))
    {
        std::cerr << "Player texture load failed!\n";
        return -1;
    }

    glm::vec2 playerPos(100.0f, 100.0f);

    float lastTime = glfwGetTime();

    // --- F� j�t�khurok ---
    while (!glfwWindowShouldClose(window))
    {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glfwPollEvents();

        // --- J�t�kos mozgat�sa ---
        float speed = 200.0f;
        glm::vec2 movement(0.0f);

        if (Input::MoveUp)    movement.y -= 1.0f;
        if (Input::MoveDown)  movement.y += 1.0f;
        if (Input::MoveLeft)  movement.x -= 1.0f;
        if (Input::MoveRight) movement.x += 1.0f;

        // Normalize movement (nehogy �tl�san gyorsabban mozogjon)
        if (glm::length(movement) > 0.0f)
            movement = glm::normalize(movement);

        playerPos += movement * speed * deltaTime;

        // --- Kamera k�vet�s ---
        glm::vec2 cameraTarget = playerPos - glm::vec2(Globals::WindowWidth / 2, Globals::WindowHeight / 2);
        static glm::vec2 cameraPos(0.0f);
        cameraPos += (cameraTarget - cameraPos) * 5.0f * deltaTime; // 5.0 = mozgat�s "simas�g�nak" m�rt�ke
        camera.SetPosition(cameraPos);

        // --- Rajzol�s ---
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.Use();
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, &camera.GetView()[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, &camera.GetProjection()[0][0]);

        map.Draw(renderer);
        renderer.DrawSprite(playerTex, playerPos, glm::vec2(64.0f, 64.0f));

        glfwSwapBuffers(window);
    }

    playerTex.Delete();
    shader.Delete();
    glfwTerminate();
    return 0;
}