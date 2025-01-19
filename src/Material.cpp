#include "Material.h"

Material::Material()
{
    initializeOpenGLFunctions();
}

void Material::bindParameter(QSharedPointer<QOpenGLShaderProgram> shader, const QString &uniformName, const MaterialParameter &parameter, int &textureUnit)
{
    shader->setUniformValue((uniformName + ".value").toUtf8().constData(), parameter.value);
    shader->setUniformValue((uniformName + ".useMap").toUtf8().constData(), parameter.useMap);

    if (parameter.useMap && parameter.map)
    {
        glActiveTexture(GL_TEXTURE0 + textureUnit); // 激活纹理单元
        parameter.map->bind();
        shader->setUniformValue((uniformName + ".map").toUtf8().constData(), textureUnit);
        ++textureUnit;
    }
}

void Material::bindUniforms(QSharedPointer<QOpenGLShaderProgram> shader)
{
    if (!shader)
        return;

    int textureUnit = 0;
    bindParameter(shader, "uMaterial.baseColor", baseColor, textureUnit);
    bindParameter(shader, "uMaterial.metallic", metallic, textureUnit);
    bindParameter(shader, "uMaterial.roughness", roughness, textureUnit);
    bindParameter(shader, "uMaterial.normal", normal, textureUnit);
    bindParameter(shader, "uMaterial.ao", ao, textureUnit);
    bindParameter(shader, "uMaterial.height", height, textureUnit);
    bindParameter(shader, "uMaterial.emissive", emissive, textureUnit);
    bindParameter(shader, "uMaterial.opacity", opacity, textureUnit);
    bindParameter(shader, "uMaterial.translucency", translucency, textureUnit);
}