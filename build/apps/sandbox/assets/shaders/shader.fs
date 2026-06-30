#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord; // Input from vertex shader

uniform sampler2D ourTexture; // The texture unit

void main()
{
    // Multiply the texture color by the vertex color to blend them
    FragColor = texture(ourTexture, TexCoord) * vec4(ourColor, 1.0);
}