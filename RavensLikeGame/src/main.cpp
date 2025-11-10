#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include "Core/Globals.h"
#include "Core/Input.h"
#include "Core/UIRenderer.h"
#include "Game/Character8Dir.h"
#include "Renderer/Shader.h"
#include "Renderer/Camera.h"
#include "Renderer/Texture.h"
#include "Renderer/SpriteRenderer.h"
#include "Renderer/IsoRenderer.h"

std::string LoadShaderSource(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open shader file: " << filePath << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

GLFWwindow* CreateGameWindow()
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    GLFWwindow* window = glfwCreateWindow(
        Globals::WindowWidth,
        Globals::WindowHeight,
        Globals::WindowTitle,
        nullptr,
        nullptr
    );

    /*GLFWwindow* window = glfwCreateWindow(
        mode->width,
        mode->height,
        Globals::WindowTitle,
        primaryMonitor,
        nullptr
    );*/

    if (!window)
    {
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, Input::KeyCallback);
    return window;
}

void CalculateDeltaTime(float& lastTime, float& deltaTime)
{
    float currentTime = glfwGetTime();
    deltaTime = currentTime - lastTime;
    lastTime = currentTime;
}

// --- 8 irányú input vektor olvasása (GLFW billentyűkből)
glm::vec2 ReadMovementVector(GLFWwindow* window)
{
    glm::vec2 m(0.0f);
    if (glfwGetKey(window, Globals::KeyMoveUp) == GLFW_PRESS) m.y += 1.0f;
    if (glfwGetKey(window, Globals::KeyMoveDown) == GLFW_PRESS) m.y -= 1.0f;
    if (glfwGetKey(window, Globals::KeyMoveLeft) == GLFW_PRESS) m.x -= 1.0f;
    if (glfwGetKey(window, Globals::KeyMoveRight) == GLFW_PRESS) m.x += 1.0f;
    if (glm::length(m) > 0.0f) m = glm::normalize(m);
    return m;
}

// --- Fizika + animáció együtt (rövid és átlátható)
void UpdatePlayer(GLFWwindow* window,
    Character8Dir& player,
    glm::vec2& playerPos,
    float playerSpeed,
    float dt)
{
    const glm::vec2 move = ReadMovementVector(window);
    playerPos += move * playerSpeed * dt;   // pozíció frissítés
    player.Update(move, dt);                // animáció frissítés
}

// --- Játékos kirajzolása (sprite shader beállítás + render)
void DrawPlayer(Shader& spriteShader,
    const Camera& camera,
    Character8Dir& player,
    const glm::vec2& playerPos,
    const glm::vec2& playerSize)
{
    // ugyanazok az uniformok, mint eddig a statikus sprite-hoz
    spriteShader.Use();
    glUniformMatrix4fv(glGetUniformLocation(spriteShader.ID, "projection"), 1, GL_FALSE, &camera.GetProjection()[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(spriteShader.ID, "view"), 1, GL_FALSE, &camera.GetView()[0][0]);
    glUniform1i(glGetUniformLocation(spriteShader.ID, "useView"), 1);
    glUniform1i(glGetUniformLocation(spriteShader.ID, "useColorOnly"), 0);
    glUniform1i(glGetUniformLocation(spriteShader.ID, "sprite"), 0);

    // a Character8Dir saját UV-t állít és rajzol; pozíciót/négyzetméretet tőled kap
    player.DrawPlayer(playerPos, playerSize);
}

void UpdateCameraFollow(Camera& camera, const glm::vec2& playerPos, float deltaTime, float smoothness = Globals::CameraFollowSmoothness)
{
    const glm::vec2 halfViewport(Globals::WindowWidth * 0.5f, Globals::WindowHeight * 0.5f);
    const glm::vec2 target = playerPos - halfViewport;

    if (smoothness <= 0.0f) {
        camera.SetPosition(target);
        return;
    }

    static glm::vec2 camPos = glm::vec2(0.0f);
    camPos += (target - camPos) * smoothness * deltaTime;
    camera.SetPosition(camPos);
}

void DrainHealthOnKey(GLFWwindow* window, float deltaTime, int& currentHealth,
    int key = Globals::DecreaseHealth, float intervalSec = 0.10f)
{
    static float accum = 0.0f;
    if (glfwGetKey(window, key) == GLFW_PRESS) {
        accum += deltaTime;
        while (accum >= intervalSec && currentHealth > 0) {
            --currentHealth;
            accum -= intervalSec;
        }
    }
    else {
        accum = 0.0f;
    }
}

void BeginFrame()
{
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderWorld(IsoRenderer& isoRenderer, const std::vector<std::vector<int>>& mapData)
{
    glEnable(GL_DEPTH_TEST);
    isoRenderer.DrawMap(mapData);
}

void RenderUI(UIRenderer& ui, int currentHealth, int maxHealth)
{
    glDisable(GL_DEPTH_TEST);
    ui.DrawHealthBar(currentHealth, maxHealth);
}

int main()
{
    if (!glfwInit())
        return -1;

    GLFWwindow* window = CreateGameWindow();
    if (!window)
        return -1;

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        return -1;

    // Engedélyezzük az átlátszóságot (alpha blending)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearDepth(1.0);

    // --- Shader betöltése fájlból ---
    std::string vertexCode = LoadShaderSource("assets/shaders/iso.vert");
    std::string fragmentCode = LoadShaderSource("assets/shaders/iso.frag");

    Shader isoShader(vertexCode.c_str(), fragmentCode.c_str());

    isoShader.Use();
    isoShader.SetInt("textureAtlas", 0);

    // --- Izometrikus renderer inicializálás ---
    IsoRenderer isoRenderer(isoShader, "assets/textures/tiles/tiles.png");
    
    glm::mat4 projection = glm::ortho(0.0f, (float)Globals::WindowWidth, 0.0f, (float)Globals::WindowHeight, -1.0f, 1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    isoRenderer.SetProjection(projection);
    isoRenderer.SetView(view);

    std::vector<std::vector<int>> mapData = {
        { 0, 1, 2, 0 },
        { 1, 2, 0, 0 },
        { 2, 0, 1, 0 },
        { 0, 0, 0, 0 }
    };

    Shader uiShader(
        LoadShaderSource("assets/shaders/sprite.vert").c_str(),
        LoadShaderSource("assets/shaders/sprite.frag").c_str());
    UIRenderer uiRenderer(uiShader);

    Texture playerSheet;
    if (!playerSheet.LoadFromFile("assets/textures/player/characters.png")) {
        std::cerr << "Player texture load failed!\n";
        return -1;
    }
    SpriteRenderer playerRenderer(uiShader);
    Character8Dir player(playerSheet, playerRenderer);

    Camera camera((float)Globals::WindowWidth, (float)Globals::WindowHeight);
    glm::vec2 playerPosition(Globals::WindowWidth * 0.5f, Globals::WindowHeight * 0.5f);
    glm::vec2 playerSize(32.0f, 32.0f);
    const float playerSpeed = 300.0f;

    int maxHealth = 100, currentHealth = 100;

    float lastTime = glfwGetTime();
    float deltaTime = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        CalculateDeltaTime(lastTime, deltaTime);
        glfwPollEvents();

        UpdatePlayer(window, player, playerPosition, playerSpeed, deltaTime);

        UpdateCameraFollow(camera, playerPosition, deltaTime);
        isoRenderer.SetView(camera.GetView());

        DrainHealthOnKey(window, deltaTime, currentHealth);

        BeginFrame();
        RenderWorld(isoRenderer, mapData);

        DrawPlayer(uiShader, camera, player, playerPosition, playerSize);

        RenderUI(uiRenderer, currentHealth, maxHealth);

        glfwSwapBuffers(window);
    }

    isoShader.Delete();
    uiShader.Delete();
    glfwTerminate();
    return 0;
}