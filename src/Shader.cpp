#include <Shader.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <glm.hpp>
#include <glad/glad.h>

using std::string, std::ifstream, std::stringstream, std::cerr, std::endl, std::vector;

Shader::Shader()
    : m_programID(0)
{
}

Shader::~Shader()
{
    if (!m_programID)
    {
        glDeleteProgram(m_programID);
    }
}

bool Shader::compileShader(GLuint shaderID, const string &shaderSource)
// 对shaderID对应的着色器进行编译
{
    const char *CStr = shaderSource.c_str(); // 转换成c_str

    glShaderSource(shaderID, 1, &CStr, nullptr);
    glCompileShader(shaderID);

    GLint success = 0;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLint logLength = 0;
        glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &logLength);

        vector<GLchar> errorLog(logLength);
        glGetShaderInfoLog(shaderID, logLength, nullptr, errorLog.data());

        cerr << "Shader Compilation Error:\n"
             << errorLog.data() << endl;
        return false;
    }

    return true;
}

bool Shader::linkProgram(GLuint vertexShaderID, GLuint fragmentShaderID)
// 将编译好的vertexShader和fragmentShader链接
// 链接之后可以直接使用这个shader
{
    m_programID = glCreateProgram();
    if (m_programID == 0)
    {
        cerr << "Error: Failed to create shader program." << endl;
        return false;
    }

    glAttachShader(m_programID, vertexShaderID);
    glAttachShader(m_programID, fragmentShaderID);
    glLinkProgram(m_programID);

    GLint success = 0;
    glGetProgramiv(m_programID, GL_LINK_STATUS, &success);
    if (!success)
    {
        GLint logLength = 0;
        glGetProgramiv(m_programID, GL_INFO_LOG_LENGTH, &logLength);

        vector<GLchar> errorLog(logLength);
        glGetProgramInfoLog(m_programID, logLength, nullptr, errorLog.data());

        cerr << "Program Linking Error:\n"
             << errorLog.data() << endl;

        glDeleteProgram(m_programID);
        m_programID = 0;

        return false;
    }

    // 分离着色器对象，之后会删除这两个独立的着色器
    glDetachShader(m_programID, vertexShaderID);
    glDetachShader(m_programID, fragmentShaderID);

    return true;
}

bool Shader::readShaderFile(const string &filePath, string &outShaderCode)
{
    ifstream shaderFile(filePath);
    if (!shaderFile.is_open())
    {
        cerr << "Error: Unable to open shader file: " << filePath << endl;
        return false;
    }

    stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    shaderFile.close();

    outShaderCode = shaderStream.str();

    return true;
}

bool Shader::loadFromFile(const std::string &vertexPath, const std::string &fragmentPath)
{
    // 创建着色器程序
    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    if (vertexShaderID == 0 || fragmentShaderID == 0)
    {
        std::cerr << "Error: Failed to create shader objects." << std::endl;
        return false;
    }

    // 读取着色器代码
    string vertexShaderSource, fragmentShaderSource;
    if (!readShaderFile(vertexPath, vertexShaderSource))
    {
        glDeleteShader(vertexShaderID);
        glDeleteShader(fragmentShaderID);
        return false;
    }
    if (!readShaderFile(fragmentPath, fragmentShaderSource))
    {
        glDeleteShader(vertexShaderID);
        glDeleteShader(fragmentShaderID);
        return false;
    }

    // 编译着色器
    if (!compileShader(vertexShaderID, vertexShaderSource))
    {
        glDeleteShader(vertexShaderID);
        glDeleteShader(fragmentShaderID);
        return false;
    }
    if (!compileShader(fragmentShaderID, fragmentShaderSource))
    {
        glDeleteShader(vertexShaderID);
        glDeleteShader(fragmentShaderID);
        return false;
    }

    // 链接着色器
    if (!linkProgram(vertexShaderID, fragmentShaderID))
    {
        glDeleteShader(vertexShaderID);
        glDeleteShader(fragmentShaderID);
        return false;
    }
    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);

    return true;
}

void Shader::use() const
{
    if (m_programID)
    {
        glUseProgram(m_programID);
    }
    else
    {
        cerr << "Error: Attempted to use an uninitialized shader program." << endl;
    }
}

