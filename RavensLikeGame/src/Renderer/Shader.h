#pragma once
#include <string>
#include <glad/glad.h>
#include <glm.hpp>

class Shader
{
public:
    unsigned int ID;

    Shader(const char* vertexSource, const char* fragmentSource);
    void Use();
    void Delete();

    void SetMat4(const std::string& name, const glm::mat4& mat) const;
    void SetVec4(const std::string& name, const glm::vec4& vec) const;
    void SetInt(const std::string& name, int value) const;
};