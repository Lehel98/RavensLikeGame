#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "Core/Globals.h"
#include "Core/Input.h"
#include "Renderer/Shader.h"
#include "Renderer/Camera.h"
#include "Renderer/Texture.h"
#include "Renderer/SpriteRenderer.h"

float vertices[] = {
     0.0f,  0.5f, 0.0f,
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f
};

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

    GLFWwindow* window = glfwCreateWindow(Globals::WindowWidth, Globals::WindowHeight, Globals::WindowTitle, nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, Input::KeyCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        return -1;

    Shader shader(vertexShaderSource, fragmentShaderSource);
    Camera camera(Globals::WindowWidth, Globals::WindowHeight);
    SpriteRenderer renderer(shader);

    Texture tex;
    if (!tex.LoadFromFile("assets/textures/my_sprite.png"))
        std::cerr << "Texture load failed\n";

    float lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glfwPollEvents();

        float speed = 200.0f;
        glm::vec2 movement(0.0f);
        if (Input::MoveUp)    movement.y += 1.0f;
        if (Input::MoveDown)  movement.y -= 1.0f;
        if (Input::MoveLeft)  movement.x -= 1.0f;
        if (Input::MoveRight) movement.x += 1.0f;

        // Normalize movement (nehogy átlósan gyorsabban mozogjon)
        if (glm::length(movement) > 0.0f)
            movement = glm::normalize(movement);

        static glm::vec2 pos = glm::vec2(100.0f, 100.0f);
        pos += movement * speed * deltaTime;

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.Use();
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, &camera.GetView()[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, &camera.GetProjection()[0][0]);

        renderer.DrawSprite(tex, pos, glm::vec2(64.0f, 64.0f));

        glfwSwapBuffers(window);
    }

    tex.Delete();
    shader.Delete();
    glfwTerminate();
    return 0;
}