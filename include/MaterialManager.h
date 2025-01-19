#pragma once

#include <QObject>
#include <QMap>
#include <QString>
#include <QSharedPointer>

#include "Material.h"

class MaterialManager : public QObject
{
    Q_OBJECT
public:
    static MaterialManager *instance(QObject *parent = nullptr);
    
    ~MaterialManager();

    // 创建材质
    QSharedPointer<Material> createMaterial(const QString &name);

    // 获取材质
    QSharedPointer<Material> getMaterial(const QString &name) const;

    bool loadFromJson(const QString &filePath);

    void clearAll();
private:
    MaterialManager(QObject *parent = nullptr);

    Q_DISABLE_COPY(MaterialManager);

    QMap<QString, QSharedPointer<Material>> m_materials;

    static MaterialManager *m_instance;

    void createDefaultMaterial();
    void loadTestMaterial();
};
