#pragma once

#include <QObject>
#include <QOpenGLTexture>
#include <QMap>
#include <QString>
#include <QSharedPointer>

// 纹理管理器，统一管理各种纹理
class TextureManager : public QObject
{
    Q_OBJECT
public:
    static TextureManager *instance(QObject *parent = nullptr); // 单例模式

    ~TextureManager();

    QSharedPointer<QOpenGLTexture> getTexture(const QString &filePath);

    void clearAll();

private:
    explicit TextureManager(QObject *parent = nullptr);

    // 禁止拷贝或赋值
    Q_DISABLE_COPY(TextureManager);

    // 缓存： key=文件路径, value=纹理指针
    QMap<QString, QSharedPointer<QOpenGLTexture>> m_textureCache;

    static TextureManager *m_instance;
};
