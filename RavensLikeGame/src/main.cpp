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

void CalculateDeltaTime(float& lastTime, float& deltaTime)
{
    float currentTime = glfwGetTime();
    deltaTime = currentTime - lastTime;
    lastTime = currentTime;
}

glm::vec2 CalculateMovement(const glm::vec2& position, float speed, float deltaTime)
{
    glm::vec2 movement(0.0f);

    if (Input::MoveUp)    movement.y -= 1.0f;
    if (Input::MoveDown)  movement.y += 1.0f;
    if (Input::MoveLeft)  movement.x -= 1.0f;
    if (Input::MoveRight) movement.x += 1.0f;
    
    if (glm::length(movement) > 0.0f)
        movement = glm::normalize(movement);

    return position + movement * speed * deltaTime;
}

void CheckCollision(const TileMap& map, glm::vec2& position, const glm::vec2& newPosition, float width, float height)
{
    glm::vec2 testPos = position;

    // --- X irány ---
    glm::vec2 testX = glm::vec2(newPosition.x, position.y);
    if (map.IsAreaWalkable(testX.x, testX.y, width, height))
        testPos.x = testX.x;

    // --- Y irány ---
    glm::vec2 testY = glm::vec2(testPos.x, newPosition.y);
    if (map.IsAreaWalkable(testY.x, testY.y, width, height))
        testPos.y = testY.y;

    position = testPos;
}

void UpdateCameraPosition(Camera& camera, const glm::vec2& playerPos, float deltaTime)
{
    glm::vec2 cameraTarget = playerPos - glm::vec2(Globals::WindowWidth / 2.0f, Globals::WindowHeight / 2.0f);
    static glm::vec2 cameraPos(0.0f);

    // 50.0f → smoothness faktor (kisebb = lassabban követ)
    cameraPos += (cameraTarget - cameraPos) * 50.0f * deltaTime;
    camera.SetPosition(cameraPos);
}

void DrawFrame(Shader& shader, Camera& camera, TileMap& map, SpriteRenderer& renderer,
    const Texture& playerTex, const glm::vec2& playerPosition, GLFWwindow* window)
{
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    shader.Use();
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, &camera.GetView()[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, &camera.GetProjection()[0][0]);

    map.Draw(renderer);
    renderer.DrawSprite(playerTex, playerPosition, glm::vec2(32.0f, 32.0f));

    glfwSwapBuffers(window);
}

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

    // Shaderek betöltése
    Shader shader(vertexShaderSource, fragmentShaderSource);
    Camera camera(Globals::WindowWidth, Globals::WindowHeight);
    SpriteRenderer renderer(shader);

    // Pálya betöltése
    TileMap map;
    if (!map.Load("assets/maps/map2.txt"))
    {
        std::cerr << "Map load failed!\n";
        return -1;
    }

    // Játékos textúra betöltése
    Texture playerTex;
    if (!playerTex.LoadFromFile("assets/textures/player/idle.png"))
    {
        std::cerr << "Player texture load failed!\n";
        return -1;
    }

    glm::vec2 playerPosition(64.0f, 64.0f);

    float lastTime = glfwGetTime();
	float deltaTime = 0.0f;

    // --- Fő játékhurok ---
    while (!glfwWindowShouldClose(window))
    {
        CalculateDeltaTime(lastTime, deltaTime);
        glfwPollEvents();

        CheckCollision(map, playerPosition, CalculateMovement(playerPosition, 200.0f, deltaTime), 32.0f, 32.0f);

        UpdateCameraPosition(camera, playerPosition, deltaTime);

        DrawFrame(shader, camera, map, renderer, playerTex, playerPosition, window);
    }

    playerTex.Delete();
    shader.Delete();
    glfwTerminate();
    return 0;
}