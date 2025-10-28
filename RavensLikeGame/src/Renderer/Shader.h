#pragma once
#include <string>
#include <glad/glad.h>

class Shader
{
public:
    unsigned int ID;

    Shader(const char* vertexSource, const char* fragmentSource);
    void Use();
    void Delete();
};