#include "TextureManager.h"
#include <QImage>
#include <QDebug>

TextureManager *TextureManager::m_instance = nullptr;

TextureManager *TextureManager::instance(QObject *parent)
{
    if (m_instance)
    {
        if (parent && !m_instance->parent())
        {
            m_instance->setParent(parent);
        }
        return m_instance;
    }
    m_instance = new TextureManager(parent);
    return m_instance;
}

TextureManager::TextureManager(QObject *parent)
    : QObject(parent)
{
}

TextureManager::~TextureManager()
{
    clearAll();
}

QSharedPointer<QOpenGLTexture> TextureManager::getTexture(const QString &filePath)
{
    // 若已加载，直接返回
    if (m_textureCache.contains(filePath))
    {
        return m_textureCache.value(filePath);
    }

    // 加载图像
    QImage image(filePath);
    if (image.isNull())
    {
        qWarning() << "Failed to load texture from: " << filePath;
        return QSharedPointer<QOpenGLTexture>(nullptr);
    }

    // 创建QOpenGLTexture
    QSharedPointer<QOpenGLTexture> texture(new QOpenGLTexture(QOpenGLTexture::Target2D), [](QOpenGLTexture *ptr)
                                           { delete ptr; });

    texture->setData(image);

    texture->setWrapMode(QOpenGLTexture::Repeat);                       // 当uv坐标超出范围时重复贴图
    texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear); // 生成缩小时的贴图
    texture->setMagnificationFilter(QOpenGLTexture::Linear);            // 放大时的贴图

    // 存入缓存
    m_textureCache.insert(filePath, texture);

    qDebug() << "Texture loaded and cached:" << filePath;
    return texture;
}

void TextureManager::clearAll()
{
    // 遍历缓存并显式调用每个纹理的 destroy 方法
    for (auto it = m_textureCache.begin(); it != m_textureCache.end(); ++it)
    {
        if (it.value() && it.value()->isCreated())
        {
            it.value()->destroy();
        }
    }

    // 清空缓存
    m_textureCache.clear();

    qDebug() << "All textures cleared from TextureManager.";
}
