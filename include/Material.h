#pragma once

#include <QString>
#include <QVector4D>
#include <QSharedPointer>
#include <QOpenGLTexture>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>

// 一个通用的材质属性结构
struct MaterialParameter
{
    // 对于标量，如 Metallic, 使用 value.x
    // 对于颜色，如 BaseColor, 使用前三位或全部 (RGB/RGBA)
    QVector4D value = QVector4D(1.0f, 1.0f, 1.0f, 1.0f);

    // 是否使用贴图
    bool useMap = false;

    // 贴图
    QSharedPointer<QOpenGLTexture> map = nullptr;

    MaterialParameter() = default;
    ~MaterialParameter() = default;

    void setMap(QSharedPointer<QOpenGLTexture> t)
    {
        if (t)
        {
            map = t;
            useMap = true;
        }
    }
};

// 专为pbr光照使用的材质
// 使用MaterialManager管理
class Material : protected QOpenGLFunctions
{
public:
    Material();
    ~Material() = default;

    MaterialParameter baseColor;    // 颜色纹理
    MaterialParameter metallic;     // 金属度
    MaterialParameter roughness;    // 粗糙度
    MaterialParameter normal;       // 法线贴图
    MaterialParameter ao;           // 环境光遮蔽 Ambient Occlusion
    MaterialParameter height;       // 高度，用于视差映射
    MaterialParameter emissive;     // 自发光
    MaterialParameter opacity;      // 不透明度
    MaterialParameter translucency; // 半透性

    QString m_name;

    void bindUniforms(QSharedPointer<QOpenGLShaderProgram> shader);

private:
    void bindParameter(QSharedPointer<QOpenGLShaderProgram> shader, const QString &uniformName, const MaterialParameter &parameter, int &textureUnit);
};