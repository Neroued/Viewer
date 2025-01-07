#pragma once

#include <QOpenGLShaderProgram>
#include <QString>
#include <QDebug>

/**
 * @brief 直接继承 QOpenGLShaderProgram
 *        提供与原先类似的 API，如 loadFromFile() / use() 等
 */
class Shader : public QOpenGLShaderProgram
{
    Q_OBJECT
public:
    explicit Shader(QObject *parent = nullptr)
        : QOpenGLShaderProgram(parent)
    {
    }

    ~Shader() override = default;

    /**
     * @brief 加载并编译顶点、片段着色器，然后 link
     * @param vertexPath    顶点着色器文件路径
     * @param fragmentPath  片段着色器文件路径
     * @return 成功/失败
     */
    bool loadFromFile(const QString &vertexPath, const QString &fragmentPath)
    {
        // 添加顶点着色器
        if (!addShaderFromSourceFile(QOpenGLShader::Vertex, vertexPath))
        {
            qWarning() << "[MyShader] Vertex shader compile error:" << log();
            return false;
        }
        // 添加片段着色器
        if (!addShaderFromSourceFile(QOpenGLShader::Fragment, fragmentPath))
        {
            qWarning() << "[MyShader] Fragment shader compile error:" << log();
            return false;
        }
        // 链接
        if (!link())
        {
            qWarning() << "[MyShader] Program link error:" << log();
            return false;
        }
        return true;
    }

    /**
     * @brief 相当于 glUseProgram
     */
    void use()
    {
        bind();
    }
};
