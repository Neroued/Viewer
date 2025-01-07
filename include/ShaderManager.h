#pragma once

#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QString>
#include <QMap>
#include <QSharedPointer>


class ShaderManager : protected QOpenGLFunctions
{
public:
    ShaderManager();
    ~ShaderManager();

    bool loadShader(const QString &name,
                    const QString &vertexShaderPath,
                    const QString &fragmentShaderPath);

    // 获取已经加载好的着色器指针
    QOpenGLShaderProgram *getShader(const QString &name) const;

    // 移除某个着色器（可选）
    void removeShader(const QString &name);

    // 清理所有着色器（可选）
    void clear();

private:
    QMap<QString, QSharedPointer<QOpenGLShaderProgram>> m_shaders;
};