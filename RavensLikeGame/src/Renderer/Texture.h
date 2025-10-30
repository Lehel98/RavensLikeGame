#pragma once
#include <string>
#include <glad/glad.h>

class Texture
{
public:
    unsigned int ID;
    int Width, Height, Channels;

    Texture();
    ~Texture();

    bool LoadFromFile(const std::string& path);
    void Bind(unsigned int unit = 0) const;
    void Unbind() const;
    void Delete();
};