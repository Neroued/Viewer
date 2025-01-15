#include <MaterialManager.h>
#include <TextureManager.h>

#include <QFile>
#include <QDebug>

MaterialManager *MaterialManager::m_instance = nullptr;

MaterialManager *MaterialManager::instance(QObject *parent)
{
    if (m_instance)
    {
        if (parent && m_instance->parent())
        {
            m_instance->setParent(parent);
        }
        return m_instance;
    }
    m_instance = new MaterialManager(parent);
    return m_instance;
}

MaterialManager::MaterialManager(QObject *parent)
    : QObject(parent)
{
    createDefaultMaterial();
    loadTestMaterial();
}

MaterialManager::~MaterialManager()
{
    clearAll();
}

QSharedPointer<Material> MaterialManager::createMaterial(const QString &name)
{
    // 如果已存在同名材质，直接返回
    if (m_materials.contains(name))
    {
        return m_materials.value(name);
    }

    // 否则创建新的
    QSharedPointer<Material> mat(new Material());
    mat->m_name = name;

    m_materials.insert(name, mat);

    return mat;
}

QSharedPointer<Material> MaterialManager::getMaterial(const QString &name) const
{
    if (m_materials.contains(name))
    {
        return m_materials.value(name);
    }
    return QSharedPointer<Material>(nullptr);
}

void MaterialManager::clearAll()
{
    m_materials.clear();
    qDebug() << "All materials cleared.";
}

void MaterialManager::createDefaultMaterial()
{
    QSharedPointer<Material> defaultMat(new Material());

    defaultMat->baseColor.value = QVector4D(1.0f, 1.0f, 1.0f, 1.0f); // 白色
    defaultMat->baseColor.useMap = false;

    // 金属度 (非金属，标量)
    defaultMat->metallic.value = QVector4D(0.0f, 0.0f, 0.0f, 0.0f);
    defaultMat->metallic.useMap = false;

    // 粗糙度 (适中，标量)
    defaultMat->roughness.value = QVector4D(0.5f, 0.5f, 0.5f, 0.5f);
    defaultMat->roughness.useMap = false;

    // 法线贴图 (无)
    defaultMat->normal.value = QVector4D(0.0f, 0.0f, 0.0f, 0.0f);
    defaultMat->normal.useMap = false;

    // 环境光遮蔽 (无遮蔽)
    defaultMat->ao.value = QVector4D(1.0f, 1.0f, 1.0f, 1.0f);
    defaultMat->ao.useMap = false;

    // 高度 (无视差)
    defaultMat->height.value = QVector4D(0.0f, 0.0f, 0.0f, 0.0f);
    defaultMat->height.useMap = false;

    // 自发光 (无自发光)
    defaultMat->emissive.value = QVector4D(0.0f, 0.0f, 0.0f, 0.0f);
    defaultMat->emissive.useMap = false;

    // 不透明度 (完全不透明)
    defaultMat->opacity.value = QVector4D(1.0f, 1.0f, 1.0f, 1.0f);
    defaultMat->opacity.useMap = false;

    // 半透性 (无半透性)
    defaultMat->translucency.value = QVector4D(0.0f, 0.0f, 0.0f, 0.0f);
    defaultMat->translucency.useMap = false;

    m_materials.insert("default", defaultMat);
}

void MaterialManager::loadTestMaterial()
{
    QSharedPointer<Material> mat(new Material());

    mat->baseColor.map = TextureManager::instance()->getTexture(":/items/brickwall.jpg");
    mat->baseColor.useMap = true;

    // 金属度 (非金属，标量)
    mat->metallic.value = QVector4D(0.0f, 0.0f, 0.0f, 0.0f);
    mat->metallic.useMap = false;

    // 粗糙度 (适中，标量)
    mat->roughness.value = QVector4D(0.5f, 0.5f, 0.5f, 0.5f);
    mat->roughness.useMap = false;

    // 法线贴图
    mat->normal.map = TextureManager::instance()->getTexture(":/items/brickwall_normal.jpg");
    mat->normal.useMap = true;

    // 环境光遮蔽 (无遮蔽)
    mat->ao.value = QVector4D(1.0f, 1.0f, 1.0f, 1.0f);
    mat->ao.useMap = false;

    // 高度 (无视差)
    mat->height.value = QVector4D(0.0f, 0.0f, 0.0f, 0.0f);
    mat->height.useMap = false;

    // 自发光 (无自发光)
    mat->emissive.value = QVector4D(0.0f, 0.0f, 0.0f, 0.0f);
    mat->emissive.useMap = false;

    // 不透明度 (完全不透明)
    mat->opacity.value = QVector4D(1.0f, 1.0f, 1.0f, 1.0f);
    mat->opacity.useMap = false;

    // 半透性 (无半透性)
    mat->translucency.value = QVector4D(0.0f, 0.0f, 0.0f, 0.0f);
    mat->translucency.useMap = false;

    m_materials.insert("brick_wall", mat);

    QSharedPointer<Material> mat2(new Material());

    mat2->baseColor.map = TextureManager::instance()->getTexture(":/items/SpaceBlanketFolds/baseColor.tif");
    mat2->baseColor.useMap = true;

    // 金属度 (非金属，标量)
    mat2->metallic.map = TextureManager::instance()->getTexture(":/items/SpaceBlanketFolds/metallic.tif");
    mat2->metallic.useMap = true;

    // 粗糙度 (适中，标量)
    mat2->roughness.map = TextureManager::instance()->getTexture(":/items/SpaceBlanketFolds/roughness.tif");
    mat2->roughness.useMap = true;

    // 法线贴图
    mat2->normal.map = TextureManager::instance()->getTexture(":/items/SpaceBlanketFolds/normal.tif");
    mat2->normal.useMap = true;

    // 环境光遮蔽 (无遮蔽)
    mat2->ao.map = TextureManager::instance()->getTexture(":/items/SpaceBlanketFolds/ao.tif");
    mat2->ao.useMap = true;

    // 高度 (无视差)
    mat2->height.map = TextureManager::instance()->getTexture(":/items/SpaceBlanketFolds/height.tif");
    mat2->height.useMap = true;

    // 自发光 (无自发光)
    mat2->emissive.value = QVector4D(0.0f, 0.0f, 0.0f, 0.0f);
    mat2->emissive.useMap = false;

    // 不透明度 (完全不透明)
    mat2->opacity.value = QVector4D(1.0f, 1.0f, 1.0f, 1.0f);
    mat2->opacity.useMap = false;

    // 半透性 (无半透性)
    mat2->translucency.value = QVector4D(0.0f, 0.0f, 0.0f, 0.0f);
    mat2->translucency.useMap = false;

    m_materials.insert("SpaceBlanketFolds", mat2);
}
