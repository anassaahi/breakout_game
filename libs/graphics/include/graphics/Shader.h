#pragma once
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

class Shader
{
public:
    unsigned int ID = 0;

    Shader() {}

    // Compile from source strings. gSrc may be nullptr.
    void Compile(const char* vSrc, const char* fSrc, const char* gSrc = nullptr)
    {
        unsigned int vertex   = compileStage(vSrc, GL_VERTEX_SHADER,   "VERTEX");
        unsigned int fragment = compileStage(fSrc, GL_FRAGMENT_SHADER, "FRAGMENT");
        unsigned int geometry = 0;
        if (gSrc)
            geometry = compileStage(gSrc, GL_GEOMETRY_SHADER, "GEOMETRY");

        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        if (gSrc) glAttachShader(ID, geometry);
        glLinkProgram(ID);
        checkErrors(ID, "PROGRAM");

        glDeleteShader(vertex);
        glDeleteShader(fragment);
        if (gSrc) glDeleteShader(geometry);
    }

    Shader& Use()
    {
        glUseProgram(ID);
        return *this;
    }

    // ── Uniform setters ──────────────────────────────────────────────────────
    void SetFloat(const char* name, float value)
    {
        glUniform1f(glGetUniformLocation(ID, name), value);
    }
    void SetInteger(const char* name, int value)
    {
        glUniform1i(glGetUniformLocation(ID, name), value);
    }
    void SetVector2f(const char* name, float x, float y)
    {
        glUniform2f(glGetUniformLocation(ID, name), x, y);
    }
    void SetVector2f(const char* name, const glm::vec2& v)
    {
        glUniform2f(glGetUniformLocation(ID, name), v.x, v.y);
    }
    void SetVector3f(const char* name, float x, float y, float z)
    {
        glUniform3f(glGetUniformLocation(ID, name), x, y, z);
    }
    void SetVector3f(const char* name, const glm::vec3& v)
    {
        glUniform3f(glGetUniformLocation(ID, name), v.x, v.y, v.z);
    }
    void SetVector4f(const char* name, float x, float y, float z, float w)
    {
        glUniform4f(glGetUniformLocation(ID, name), x, y, z, w);
    }
    void SetVector4f(const char* name, const glm::vec4& v)
    {
        glUniform4f(glGetUniformLocation(ID, name), v.x, v.y, v.z, v.w);
    }
    void SetMatrix4(const char* name, const glm::mat4& m)
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name), 1, GL_FALSE,
                           glm::value_ptr(m));
    }

private:
    unsigned int compileStage(const char* src, GLenum type, const char* label)
    {
        unsigned int id = glCreateShader(type);
        glShaderSource(id, 1, &src, nullptr);
        glCompileShader(id);
        checkErrors(id, label);
        return id;
    }

    void checkErrors(unsigned int object, const char* type)
    {
        int  success;
        char log[1024];
        if (std::string(type) != "PROGRAM") {
            glGetShaderiv(object, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(object, 1024, nullptr, log);
                std::cerr << "ERROR::SHADER::" << type
                          << "::COMPILE\n" << log << '\n';
            }
        } else {
            glGetProgramiv(object, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(object, 1024, nullptr, log);
                std::cerr << "ERROR::SHADER::PROGRAM::LINK\n" << log << '\n';
            }
        }
    }
};