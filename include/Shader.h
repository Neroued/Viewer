#pragma once
#define GLM_ENABLE_EXPERIMENTAL

#include <string>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

class Shader
{
public:
    Shader();
    ~Shader();

    Shader(const Shader &) = delete;
    Shader &operator=(const Shader &) = delete;

    bool loadFromFile(const std::string &vertexPath, const std::string &fragmentPath);
    void use() const;
    GLuint getProgramID() const { return m_programID; }

private:
    GLuint m_programID;

    bool compileShader(GLuint shaderID, const std::string &shaderSource);
    bool linkProgram(GLuint vertexShaderID, GLuint fragmentShaderID);
    bool readShaderFile(const std::string &filePath, std::string &outShaderCode);

public:
    template <typename T>
    void setUniform(const std::string &name, const T &value) const
    {
        GLint location = glGetUniformLocation(m_programID, name.c_str());
        if (location == -1)
        {
            std::cerr << "Warning: Uniform '" << name << "' not found!" << std::endl;
            return;
        }
        uploadUniform(location, value);
    }

    static void uploadUniform(GLint location, bool value)
    {
        glUniform1i(location, static_cast<int>(value));
    }

    static void uploadUniform(GLint location, int value)
    {
        glUniform1i(location, value);
    }

    static void uploadUniform(GLint location, float value)
    {
        glUniform1f(location, value);
    }

    static void uploadUniform(GLint location, const glm::vec2 &value)
    {
        glUniform2fv(location, 1, glm::value_ptr(value));
    }

    static void uploadUniform(GLint location, const glm::vec3 &value)
    {
        glUniform3fv(location, 1, glm::value_ptr(value));
    }

    static void uploadUniform(GLint location, const glm::vec4 &value)
    {
        glUniform4fv(location, 1, glm::value_ptr(value));
    }

    static void uploadUniform(GLint location, const glm::mat4 &value)
    {
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
    }
};