#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include "Core/Globals.h"
#include "Core/Input.h"
#include "Core/UIRenderer.h"
#include "Renderer/Shader.h"
#include "Renderer/Camera.h"
#include "Renderer/Texture.h"
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

void UpdateCameraPosition(Camera& camera, const glm::vec2& playerPos, float deltaTime)
{
    glm::vec2 cameraTarget = playerPos - glm::vec2(Globals::WindowWidth / 2.0f, Globals::WindowHeight / 2.0f);
    static glm::vec2 cameraPos(0.0f);

    // 50.0f → smoothness faktor (kisebb = lassabban követ)
    cameraPos += (cameraTarget - cameraPos) * 50.0f * deltaTime;
    camera.SetPosition(cameraPos);
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
        { 0, 1, 2 },
        { 1, 2, 0 },
        { 2, 0, 1 }
    };

    Shader uiShader(
        LoadShaderSource("assets/shaders/sprite.vert").c_str(),
        LoadShaderSource("assets/shaders/sprite.frag").c_str());
    UIRenderer uiRenderer(uiShader);
    int maxHealth = 100, currentHealth = 100;

    float lastTime = glfwGetTime();
    float deltaTime = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        CalculateDeltaTime(lastTime, deltaTime);
        glfwPollEvents();

        static float mHoldAccum = 0.0f;

        const float kDrainIntervalSeconds = 0.10f;

        if (glfwGetKey(window, Globals::DecreaseHealth) == GLFW_PRESS) {
            mHoldAccum += deltaTime;
            while (mHoldAccum >= kDrainIntervalSeconds && currentHealth > 0) {
                --currentHealth;
                mHoldAccum -= kDrainIntervalSeconds;
            }
        }
        else {
            mHoldAccum = 0.0f;
        }

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        isoRenderer.DrawMap(mapData);

        glDisable(GL_DEPTH_TEST);
        uiRenderer.DrawHealthBar(currentHealth, maxHealth);

        glfwSwapBuffers(window);
    }

    isoShader.Delete();
    uiShader.Delete();
    glfwTerminate();
    return 0;
}