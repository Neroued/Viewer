#include "ShaderManager.h"
#include <QFile>
#include <QDebug>

ShaderManager::ShaderManager()
{
    // 需要在 QOpenGLWidget::initializeGL() 或之后进行。
    initializeOpenGLFunctions();
}

ShaderManager::~ShaderManager()
{
    clear();
}

bool ShaderManager::loadShader(const QString &name,
                               const QString &vertexShaderPath,
                               const QString &fragmentShaderPath)
{
    // 如果已经存在同名着色器，直接返回 false
    if (m_shaders.contains(name))
    {
        qWarning() << "Shader with name" << name << "already exists.";
        return false;
    }

    // 创建一个新的 QOpenGLShaderProgram
    QSharedPointer<QOpenGLShaderProgram> program(new QOpenGLShaderProgram());

    // 编译顶点着色器
    if (!program->addShaderFromSourceFile(QOpenGLShader::Vertex, vertexShaderPath))
    {
        qWarning() << "Failed to compile vertex shader:" << program->log();
        return false;
    }

    // 编译片段着色器
    if (!program->addShaderFromSourceFile(QOpenGLShader::Fragment, fragmentShaderPath))
    {
        qWarning() << "Failed to compile fragment shader:" << program->log();
        return false;
    }

    // 链接 program
    if (!program->link())
    {
        qWarning() << "Failed to link shader program:" << program->log();
        return false;
    }

    // 如果全部成功，则存入 map
    m_shaders.insert(name, program);

    qDebug() << "Shader" << name << "loaded successfully.";
    return true;
}

QOpenGLShaderProgram *ShaderManager::getShader(const QString &name) const
{
    if (!m_shaders.contains(name))
    {
        return nullptr;
    }
    return m_shaders.value(name).data(); // 返回原始指针
}

void ShaderManager::removeShader(const QString &name)
{
    if (m_shaders.contains(name))
    {
        m_shaders.remove(name);
        qDebug() << "Shader" << name << "removed.";
    }
}

void ShaderManager::clear()
{
    m_shaders.clear();
    qDebug() << "All shaders removed.";
}