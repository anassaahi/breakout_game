#pragma once
#include <glad/glad.h>

// A 2D texture — wraps an OpenGL texture object.
// Load pixel data via Generate(), then Bind() before drawing.
class Texture2D
{
public:
    unsigned int ID;          // OpenGL texture object handle
    unsigned int Width, Height;

    // Texture format
    unsigned int Internal_Format; // format stored on GPU  (e.g. GL_RGB)
    unsigned int Image_Format;    // format of source data (e.g. GL_RGB)

    // Texture wrapping / filtering
    unsigned int Wrap_S;          // GL_REPEAT, GL_CLAMP_TO_EDGE …
    unsigned int Wrap_T;
    unsigned int Filter_Min;      // GL_LINEAR, GL_NEAREST …
    unsigned int Filter_Max;

    Texture2D()
        : Width(0), Height(0),
          Internal_Format(GL_RGB), Image_Format(GL_RGB),
          Wrap_S(GL_REPEAT), Wrap_T(GL_REPEAT),
          Filter_Min(GL_LINEAR), Filter_Max(GL_LINEAR)
    {
        glGenTextures(1, &ID);
    }

    // Upload pixel data and set GL parameters.
    // Call once after construction; data may be nullptr for an empty texture.
    void Generate(unsigned int width, unsigned int height, unsigned char* data)
    {
        Width  = width;
        Height = height;

        glBindTexture(GL_TEXTURE_2D, ID);
        glTexImage2D(GL_TEXTURE_2D, 0, Internal_Format,
                     width, height, 0,
                     Image_Format, GL_UNSIGNED_BYTE, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, Wrap_S);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, Wrap_T);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Filter_Min);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Filter_Max);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Bind() const
    {
        glBindTexture(GL_TEXTURE_2D, ID);
    }
};
