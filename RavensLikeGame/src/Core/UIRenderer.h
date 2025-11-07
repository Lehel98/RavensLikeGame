#pragma once
#include "../Renderer/Shader.h"
#include "../Renderer/Camera.h"
#include <glm.hpp>

class UIRenderer {
public:
    UIRenderer(Shader& shader);
    ~UIRenderer();

    void DrawHealthBar(int currentHealth, int maxHealth);

private:
    // VAO = Vertex Array Object
	// VBO = Vertex Buffer Object
    unsigned int vaoHealthBarBorder, vboHealthBarBorder;
    unsigned int vaoHealthBarBackground, vboHealthBarBackground;
    unsigned int vaoHealthBarFill, vboHealthBarFill;
	const float defaultOffset = 15.0f;
	const float innerBarWidth = 300.0f;
    const float innerBarHeight = 25.0f;
	const float uiLayerDepth = -0.5f;
    Shader& shader;

    void InitRenderData();
    void InitHealthBarBorderVAO();
    void InitHealthBarBackgroundVAO();
    void InitHealthBarForegroundVAO();
    void SetupUIRendering(float screenWidth, float screenHeight);
    void DrawHealthBarBorder(const glm::vec2& position);
    void DrawHealthBarBackground(const glm::vec2& position);
    void DrawHealthBarForeground(const glm::vec2& position, float ratio);
    void RestoreDefaultShaderState();
};