#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "Core/Globals.h"
#include "Core/Input.h"
#include "Core/UIRenderer.h"
#include "Game/Character8Direction.h"
#include "Renderer/Shader.h"
#include "Renderer/Camera.h"
#include "Renderer/Texture.h"
#include "Renderer/SpriteRenderer.h"
#include "Renderer/IsoRenderer.h"

struct DashState {
    bool active = false;     // éppen dash-ben van-e
    float time = 0.0f;      // eltelt idő a dash-ben
    float cooldown = 0.0f;      // mennyi van még hátra a cooldown-ból
    glm::vec2 start = { 0.0f, 0.0f };
    glm::vec2 end = { 0.0f, 0.0f };
    glm::vec2 direction = { 0.0f, 0.0f };  // normált irányvektor
};

// Egyszeri, megosztott erőforrások a rácsvonalakhoz
static GLuint sGridVAO = 0, sGridVBO = 0;
auto EnsureGridVAO = []() {
    if (sGridVAO) return;
    // Egység-rombussz (origó-középpontú), GL_LINE_LOOP-hoz 4 pont elég:
    // (-1,0) -> (0,1) -> (1,0) -> (0,-1)
    const float unitDiamond[8] = {
        -1.0f,  0.0f,
         0.0f,  1.0f,
         1.0f,  0.0f,
         0.0f, -1.0f
    };
    glGenVertexArrays(1, &sGridVAO);
    glGenBuffers(1, &sGridVBO);
    glBindVertexArray(sGridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sGridVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(unitDiamond), unitDiamond, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    };

void CenterWindowOnPrimary(GLFWwindow* window) {
    GLFWmonitor* mon = glfwGetPrimaryMonitor();
    if (!mon || !window) return;

    int ww, wh;
    glfwGetWindowSize(window, &ww, &wh);

    int ax = 0, ay = 0, aw = 0, ah = 0;
    if (glfwGetMonitorWorkarea) {
        glfwGetMonitorWorkarea(mon, &ax, &ay, &aw, &ah);
        int x = ax + (aw - ww) / 2;
        int y = ay + (ah - wh) / 2;
        glfwSetWindowPos(window, x, y);
        return;
    }

    int mx = 0, my = 0;
    glfwGetMonitorPos(mon, &mx, &my);
    const GLFWvidmode* mode = glfwGetVideoMode(mon);
    int x = mx + (mode->width - ww) / 2;
    int y = my + (mode->height - wh) / 2;
    glfwSetWindowPos(window, x, y);
}

void DrawWalkableOutlines(
    const IsoRenderer& iso,
    const std::vector<std::vector<int>>& map,
    Shader& lineShader,
    const glm::vec3& lineColor = glm::vec3(1.0f),
    float lineWidth = 1.0f)
{
    glDisable(GL_DEPTH_TEST);
    EnsureGridVAO();

    const int rows = static_cast<int>(map.size());
    const int cols = static_cast<int>(map[0].size());

    // Ugyanaz a P/V, mint a tile-oknál → semmilyen “úszás”
    lineShader.Use();
    glUniformMatrix4fv(glGetUniformLocation(lineShader.ID, "projection"), 1, GL_FALSE, &iso.GetProjection()[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(lineShader.ID, "view"), 1, GL_FALSE, &iso.GetView()[0][0]);

    // Ez a shaderedben azt jelenti: egyszínű rajz (ne textúrát mintázzon)
    glUniform1i(glGetUniformLocation(lineShader.ID, "useView"), 1);
    glUniform1i(glGetUniformLocation(lineShader.ID, "useColorOnly"), 1);
    glUniform3f(glGetUniformLocation(lineShader.ID, "spriteColor"),
        lineColor.r, lineColor.g, lineColor.b);

    glLineWidth(lineWidth);

    // Ugyanaz az origó és félméretek, mint az IsoRenderer-ben
    const float halfW = iso.ScaledWidth() * 0.5f;  // 693*scale/2
    const float halfH = iso.ScaledVisibleHeight() * 0.5f;  // 400*scale/2
    const glm::vec2 origin = iso.ComputeMapOrigin(rows, cols);

    const glm::vec2 worldBias(
        (Globals::kClampBiasTilesX - Globals::kClampBiasTilesY) * halfW,
        (Globals::kClampBiasTilesX + Globals::kClampBiasTilesY) * halfH
    );

    glBindVertexArray(sGridVAO);

    // Minden tile közepére: model = T(center) * S(halfW,halfH,1)
    for (int r = 0; r < rows; ++r)
    {
        for (int c = 0; c < cols; ++c)
        {
            // Tile-közép (ugyanaz a képlet, mint a csempéknél)
            const glm::vec2 center(
                origin.x + (c - r) * halfW,
                origin.y + (c + r) * halfH
            );

            glm::mat4 model(1.0f);
            model = glm::translate(model, glm::vec3(center + worldBias, 0.0f));
            model = glm::scale(model, glm::vec3(halfW, halfH, 1.0f));
            glUniformMatrix4fv(glGetUniformLocation(lineShader.ID, "model"), 1, GL_FALSE, &model[0][0]);

            glDrawArrays(GL_LINE_LOOP, 0, 4);
        }
    }

    glBindVertexArray(0);
}

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

    CenterWindowOnPrimary(window);

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
void UpdatePlayerMovement(GLFWwindow* window,
    Character8Direction& player,
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
    Character8Direction& player,
    const glm::vec2& playerPos,
    const glm::vec2& playerSize)
{
    glEnable(GL_DEPTH_TEST);
    // ugyanazok az uniformok, mint eddig a statikus sprite-hoz
    spriteShader.Use();
    glUniformMatrix4fv(glGetUniformLocation(spriteShader.ID, "projection"), 1, GL_FALSE, &camera.GetProjection()[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(spriteShader.ID, "view"), 1, GL_FALSE, &camera.GetView()[0][0]);
    glUniform1i(glGetUniformLocation(spriteShader.ID, "useView"), 1);
    glUniform1i(glGetUniformLocation(spriteShader.ID, "useColorOnly"), 0);
    glUniform1i(glGetUniformLocation(spriteShader.ID, "sprite"), 0);

    // a Character8Direction saját UV-t állít és rajzol; pozíciót/négyzetméretet tőled kap
    player.DrawPlayer(playerPos, playerSize);
}

void UpdateCameraFollow(Camera& camera, const glm::vec2& playerPos, float deltaTime)
{
    float smoothness = Globals::kCameraFollowSmoothness;
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

void BeginDash(const glm::vec2& direction, const glm::vec2& currentPosition, DashState& dash)
{
    dash.active = true;
    dash.time = 0.0f;
    dash.direction = (glm::length(direction) > 0.0f) ? glm::normalize(direction) : glm::vec2(0.0f, 1.0f);
    dash.start = currentPosition;
    dash.end = currentPosition + dash.direction * Globals::DashDistance;
}

glm::vec2 UpdateDash(DashState& dash, float deltaTime)
{
    dash.time += deltaTime;
    const float a = glm::clamp(dash.time / Globals::DashDurationInSeconds, 0.0f, 1.0f);
    glm::vec2 pos = dash.start + (dash.end - dash.start) * a;

    if (a >= 1.0f) {
        dash.active = false;
        dash.cooldown = Globals::DashCooldownInSeconds;
        pos = dash.end;
    }
    return pos;
}

void UpdatePlayerPosition(GLFWwindow* window,
    Character8Direction& player,
    glm::vec2& playerPosition,
    float playerSpeed,
    float deltaTime,
    DashState& dash,
    bool& dashKeyWasDown)
{
    if (dash.cooldown > 0.0f)
        dash.cooldown = std::max(0.0f, dash.cooldown - deltaTime);

    const bool dashKeyDown = (glfwGetKey(window, Globals::DashKey) == GLFW_PRESS);
    if (!dash.active && dash.cooldown <= 0.0f && dashKeyDown && !dashKeyWasDown)
    {
        glm::vec2 dashDirection = player.GetCurrentDirectionVector();
        // Ha valamiért 0 lenne (állt a karakter és nincs irány), nézzen felfelé alapból
        if (glm::dot(dashDirection, dashDirection) <= 0.0f)
            dashDirection = glm::vec2(0.0f, 1.0f);

        BeginDash(dashDirection, playerPosition, dash);
    }
    dashKeyWasDown = dashKeyDown;

    if (dash.active)
    {
        playerPosition = UpdateDash(dash, deltaTime);
        player.Update(dash.direction, deltaTime);
    }
    else
    {
        UpdatePlayerMovement(window, player, playerPosition, playerSpeed, deltaTime);
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

// L1 "rombusz peremre" való klampelés
glm::vec2 ClampLocalToDiamond(glm::vec2 local)
{
    const float halfW = Globals::kTileWalkableWidth * 0.5f;
    const float halfH = Globals::kTileWalkableHeight * 0.5f;

    float ax = std::abs(local.x) / halfW;
    float ay = std::abs(local.y) / halfH;
    float s = ax + ay;
    if (s <= 1.0f) return local;

    float scale = 1.0f / s;
    local.x = (local.x >= 0 ? 1.0f : -1.0f) * ax * scale * halfW;
    local.y = (local.y >= 0 ? 1.0f : -1.0f) * ay * scale * halfH;
    return local;
}

void ClampPlayerToMapBoundsDiamond(
    glm::vec2& playerCenterPosition,
    const glm::vec2& playerSize,
    const IsoRenderer& iso,
    int rows, int cols)
{
    // --- beállítások ---
    const float footH = Globals::kPlayerFootHitboxHeightPx;      // láb „magasság” a sprite alján
    const float halfHit = Globals::kPlayerFootHitboxWidthPx * 0.5f; // vízszintes fél-szélesség (pl. 6.5 a 13px-hez)

    // --- 1) sprite-középpont -> lábpont (világ) ---
    glm::vec2 feet = playerCenterPosition - glm::vec2(0.0f, playerSize.y * 0.5f - footH);

    // --- 2) előkészített méretek és origó (pontosan mint a rajzolásnál) ---
    const float halfW = iso.ScaledWidth() * 0.5f;          // 693 * scale / 2
    const float halfH = iso.ScaledVisibleHeight() * 0.5f;  // 400 * scale / 2
    const glm::vec2 O = iso.ComputeMapOrigin(rows, cols);

    // belső kis segéd-blokk (NEM függvény): egy konkrét lábpont klampelése a pálya „rombusz” határára
    auto clampOneSide = [&](float dx) -> glm::vec2 {
        const float X = (feet.x + dx) - O.x;
        const float Y = feet.y - O.y;

        float gx = 0.5f * ((X / halfW) + (Y / halfH));
        float gy = 0.5f * ((Y / halfH) - (X / halfW));

        gx -= Globals::kClampBiasTilesX; gy -= Globals::kClampBiasTilesY;
        gx = std::clamp(gx, -0.5f, static_cast<float>(cols) - 0.5f);
        gy = std::clamp(gy, -0.5f, static_cast<float>(rows) - 0.5f);
        gx += Globals::kClampBiasTilesX; gy += Globals::kClampBiasTilesY;

        const float Xc = (gx - gy) * halfW;
        const float Yc = (gx + gy) * halfH;
        return { O.x + Xc, O.y + Yc };
        };

    // --- 3) 13px széles talp: bal és jobb szélt külön klampeljük ---
    const glm::vec2 leftFeet = clampOneSide(-halfHit);
    const glm::vec2 rightFeet = clampOneSide(+halfHit);

    // --- 4) középre visszaátlagolunk, így a teljes 13px bent marad ---
    feet = 0.5f * (leftFeet + rightFeet);

    // --- 5) lábpont -> sprite-középpont ---
    playerCenterPosition = feet + glm::vec2(0.0f, playerSize.y * 0.5f - footH);
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

    // Shader betöltése fájlból
    std::string vertexCode = LoadShaderSource("assets/shaders/iso.vert");
    std::string fragmentCode = LoadShaderSource("assets/shaders/iso.frag");

    Shader isoShader(vertexCode.c_str(), fragmentCode.c_str());

    isoShader.Use();
    isoShader.SetInt("textureAtlas", 0);

    // Izometrikus renderer inicializálás
    IsoRenderer isoRenderer(isoShader, "assets/textures/tiles/tiles.png");
    
    glm::mat4 projection = glm::ortho(0.0f, (float)Globals::WindowWidth, 0.0f, (float)Globals::WindowHeight, -1.0f, 1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    isoRenderer.SetProjection(projection);
    isoRenderer.SetView(view);

    std::vector<std::vector<int>> mapData = {
        { 0, 1, 2, 3, 0, 3, 0, 3, 0, 3 },
        { 0, 2, 3, 0, 0, 0, 3, 0, 3, 0 },
        { 0, 3, 0, 1, 0, 3, 0, 3, 0, 3 },
        { 0, 0, 1, 2, 0, 0, 3, 0, 3, 0 },
        { 0, 3, 0, 3, 0, 3, 0, 3, 0, 3 }
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
    Character8Direction player(playerSheet, playerRenderer);

    Camera camera((float)Globals::WindowWidth, (float)Globals::WindowHeight);

    int centerTileX = static_cast<int>(mapData[0].size() / 2);
    int centerTileY = static_cast<int>(mapData.size() / 2);

    float worldX = (centerTileX - centerTileY) * (isoRenderer.ScaledWidth() * 0.5f);
    float worldY = (centerTileX + centerTileY) * (isoRenderer.ScaledVisibleHeight() * 0.5f);

    glm::vec2 playerPosition(worldX, worldY);
    glm::vec2 playerSize(32.0f, 32.0f);
    const float playerSpeed = 300.0f;

    DashState dash;
    bool dashKeyWasDown = false;

    int maxHealth = 100, currentHealth = 100;

    float lastTime = glfwGetTime();
    float deltaTime = 0.0f;

    const int mapWidth = static_cast<int>(mapData[0].size());
    const int mapHeight = static_cast<int>(mapData.size());

    while (!glfwWindowShouldClose(window))
    {
        CalculateDeltaTime(lastTime, deltaTime);
        glfwPollEvents();

        UpdatePlayerPosition(window, player, playerPosition, playerSpeed, deltaTime, dash, dashKeyWasDown);

        ClampPlayerToMapBoundsDiamond(playerPosition, playerSize, isoRenderer, mapHeight, mapWidth);

        UpdateCameraFollow(camera, playerPosition, deltaTime);
        isoRenderer.SetView(camera.GetView());

        DrainHealthOnKey(window, deltaTime, currentHealth);

        BeginFrame();
        RenderWorld(isoRenderer, mapData);

        //DrawWalkableOutlines(isoRenderer, mapData, uiShader, glm::vec3(1.0f), 1.0f);

        DrawPlayer(uiShader, camera, player, playerPosition, playerSize);

        RenderUI(uiRenderer, currentHealth, maxHealth);

        glfwSwapBuffers(window);
    }

    isoShader.Delete();
    uiShader.Delete();
    glfwTerminate();
    return 0;
}