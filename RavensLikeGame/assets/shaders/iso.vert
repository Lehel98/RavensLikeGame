#version 330 core
layout (location = 0) in vec2 aPos;   // quad pozíció (pixelekben)
layout (location = 1) in vec2 aTex;   // quad UV [0..1]

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec4 uvRect;                  // x=u0, y=v0, z=u1, w=v1

out vec2 TexCoord;

void main()
{
    // quad UV → atlas UV
    TexCoord = mix(uvRect.xy, uvRect.zw, aTex);
    gl_Position = projection * view * model * vec4(aPos, 0.0, 1.0);
}