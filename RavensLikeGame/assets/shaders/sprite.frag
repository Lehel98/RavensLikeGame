#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D sprite;     // atlasz
uniform vec3 spriteColor;     // színezés (UI-hoz)
uniform bool useColorOnly;    // UI: true → egyszínű
uniform bool useView;         // csak a vertex shader használja
uniform vec4 uvRect;          // (u0,v0,u1,v1) – al-téglalap az atlaszon

void main()
{
    if (useColorOnly)
    {
        FragColor = vec4(spriteColor, 1.0);
    }
    else
    {
        // a SpriteRenderer quad TexCoord-ja 0..1, ezt térképezzük rá az uvRect-re
        vec2 uv = mix(uvRect.xy, uvRect.zw, TexCoord);
        vec4 tex = texture(sprite, uv);
        if (tex.a < 0.01) discard; // tényleg átlátszó pixelek eldobása
        FragColor = tex;
    }
}