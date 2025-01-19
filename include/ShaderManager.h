#pragma once

#include <QObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QString>
#include <QMap>
#include <QSharedPointer>

class ShaderManager : public QObject, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    static ShaderManager *instance(QObject *parent = nullptr);
    ~ShaderManager();

    bool loadShader(const QString &name,
                    const QString &vertexShaderPath,
                    const QString &fragmentShaderPath);

    // 获取已经加载好的着色器指针
    QSharedPointer<QOpenGLShaderProgram> getShader(const QString &name) const;

    // 移除某个着色器（可选）
    void removeShader(const QString &name);

    // 清理所有着色器（可选）
    void clear();

private:
    ShaderManager(QObject *parent = nullptr);
    QMap<QString, QSharedPointer<QOpenGLShaderProgram>> m_shaders;

    static ShaderManager *m_instance;
};
