#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D sprite;     // textúra
uniform vec3 spriteColor;     // egyszínű elemekhez (UI)
uniform bool useColorOnly;    // true = UI színezett elem, false = textúrázott elem

void main()
{
    if (useColorOnly)
    {
        // Egyszínű (pl. UI, health bar)
        FragColor = vec4(spriteColor, 1.0);
    }
    else
    {
        // Textúrázott (pl. pálya, karakter)
        FragColor = texture(sprite, TexCoord);
    }
}