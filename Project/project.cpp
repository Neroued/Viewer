#include <QApplication>
#include <MainWindow.h>
#include <Scene.h>
#include <Object.h>
#include <QSharedPointer>
#include <NSController.h>
#include <OpenGLWidget.h>
#include <QVector3D>
#include <random>

using namespace FEMLib;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QSurfaceFormat format;
    format.setVersion(4, 6);                        // 指定 OpenGL 版本，例如 3.3
    format.setProfile(QSurfaceFormat::CoreProfile); // 使用核心配置（或 setProfile(QSurfaceFormat::CompatibilityProfile)）
    format.setDepthBufferSize(24);                  // 设置深度缓冲大小
    format.setSamples(4);                           // 设置多重采样级别（4x MSAA）
    QSurfaceFormat::setDefaultFormat(format);       // 应用为默认格式

    MainWindow mainWindow;
    mainWindow.resize(800, 600); // 设置窗口大小
    mainWindow.show();           // 显示窗口
    mainWindow.setWindowTitle("Project 1");

    auto gl = mainWindow.getOpenGLWidget();

    // First scene: Sphere
    auto scene1 = QSharedPointer<Scene>::create("Sphere");
    gl->addScene(scene1);

    Mesh sphere(10, MeshType::SPHERE); // Example mesh
    auto sphereMeshObj = QSharedPointer<Object>::create();
    sphereMeshObj->loadFromMesh(sphere);
    sphereMeshObj->setDrawMode(DrawMode::WIREFRAME);
    sphereMeshObj->setObjectType(ObjectType::STATIC);
    sphereMeshObj->setShader("basic");
    sphereMeshObj->setPosition(QVector3D(-1.1f, 0.0f, 0.0f));
    scene1->addObject(sphereMeshObj);

    auto sphereNSObj = QSharedPointer<Object>::create(); // Example NS solver Object and Controller
    sphereNSObj->setShader("blinn_phong");
    sphereNSObj->setPosition(QVector3D(1.1f, 0.0f, 0.0f));
    auto sphereNSctrl = QSharedPointer<NSController>::create(50, MeshType::SPHERE, sphereNSObj);
    scene1->addObject(sphereNSObj);
    scene1->addController(sphereNSctrl);

    // Second scene: Square
    auto scene2 = QSharedPointer<Scene>::create("Square");
    gl->addScene(scene2);

    Mesh square(10, MeshType::SQUARE); // Example mesh
    auto squareMeshObj = QSharedPointer<Object>::create();
    squareMeshObj->loadFromMesh(square);
    squareMeshObj->setDrawMode(DrawMode::WIREFRAME);
    squareMeshObj->setObjectType(ObjectType::STATIC);
    squareMeshObj->setShader("basic");
    squareMeshObj->setPosition(QVector3D(-1.1f, 0.0f, 0.0f));
    scene2->addObject(squareMeshObj);

    auto squareNSObj = QSharedPointer<Object>::create(); // Example NS solver Object and Controller
    squareNSObj->setShader("blinn_phong");
    squareNSObj->setPosition(QVector3D(1.1f, 0.0f, 0.0f));
    auto squareNSctrl = QSharedPointer<NSController>::create(50, MeshType::SQUARE, squareNSObj);
    scene2->addObject(squareNSObj);
    scene2->addController(squareNSctrl);

    // Third scene: HemiSphere
    auto scene3 = QSharedPointer<Scene>::create("HemiSphere");
    gl->addScene(scene3);

    Mesh hemi(10, MeshType::HEMI_SPHERE); // Example mesh
    auto hemiMeshObj = QSharedPointer<Object>::create();
    hemiMeshObj->loadFromMesh(hemi);
    hemiMeshObj->setDrawMode(DrawMode::WIREFRAME);
    hemiMeshObj->setObjectType(ObjectType::STATIC);
    hemiMeshObj->setShader("basic");
    hemiMeshObj->setPosition(QVector3D(-1.1f, 0.0f, 0.0f));
    scene3->addObject(hemiMeshObj);

    auto hemiNSObj = QSharedPointer<Object>::create(); // Example NS solver Object and Controller
    hemiNSObj->setShader("blinn_phong");
    hemiNSObj->setPosition(QVector3D(1.1f, 0.0f, 0.0f));
    auto hemiNSctrl = QSharedPointer<NSController>::create(50, MeshType::HEMI_SPHERE, hemiNSObj);
    scene3->addObject(hemiNSObj);
    scene3->addController(hemiNSctrl);

    // Fourth scene: Materials
    auto scene4 = QSharedPointer<Scene>::create("Materials");
    scene4->getCamera().setPosition(QVector3D(-1.1f, 1.0f, -3.0f));
    scene4->getCamera().rotate(0.0f, 180.0f, 1.0f);
    gl->addScene(scene4);

    auto cube1 = QSharedPointer<Object>::create();
    cube1->loadCube();
    cube1->setPosition(QVector3D(-1.1f, 0.0f, 0.0f));
    cube1->setShader("blinn_phong");
    cube1->setDrawMode(DrawMode::FILL);
    cube1->setObjectType(ObjectType::MATERIAL);
    cube1->setMaterial("brick_wall");
    scene4->addObject(cube1);

    auto cube2 = QSharedPointer<Object>::create();
    cube2->loadCube();
    cube2->setPosition(QVector3D(1.1f, 0.0f, 0.0f));
    cube2->setShader("pbr");
    cube2->setDrawMode(DrawMode::FILL);
    cube2->setObjectType(ObjectType::MATERIAL);
    cube2->setMaterial("SpaceBlanketFolds");
    scene4->addObject(cube2);

    // Fifth scene: A lot of Objects
    auto scene5 = QSharedPointer<Scene>::create("A lot of Objects");
    gl->addScene(scene5);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> positionDis(-0.2f, 0.2f); // 随机位置偏移
    std::uniform_real_distribution<float> scaleDis(0.1f, 0.3f);     // 随机缩放
    std::uniform_real_distribution<float> colorDis(0.0f, 1.0f);     // 随机颜色

    Mesh mesh_alot(15, MeshType::SPHERE);
    int n = 10;
    for (int k = 0; k < n; ++k)
    {
        for (int i = 0; i < n; ++i)
        {
            for (int j = 0; j < n; ++j)
            {
                auto obj = QSharedPointer<Object>::create();
                obj->setObjectType(ObjectType::STATIC);
                obj->loadFromMesh(mesh_alot);
                obj->setDrawMode(DrawMode::FILL);
                obj->setShader("blinn_phong");

                // 随机缩放
                float scaleVariation = scaleDis(gen);
                obj->setScale({scaleVariation, scaleVariation, scaleVariation});

                // 随机位置偏移
                float randomOffsetX = positionDis(gen);
                float randomOffsetY = positionDis(gen);
                float randomOffsetZ = positionDis(gen);
                obj->setPosition(QVector3D((float)(i) + randomOffsetX,
                                           (float)(j) + randomOffsetY,
                                           2.0f + k + randomOffsetZ));

                // 随机颜色
                float red = colorDis(gen) * 0.3f + 0.7f; // 偏蓝色调
                float green = colorDis(gen) * 0.5f + 0.5f;
                float blue = colorDis(gen) * 0.2f + 0.8f;
                obj->setColorBuffer({red, green, blue});

                scene5->addObject(obj);
            }
        }
    }

    gl->initializeScenes();
    return app.exec();
}