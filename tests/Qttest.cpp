#include <QApplication>
#include "MainWindow.h"
#include <Object.h>
#include <Scene.h>
#include <iostream>
#include <NavierStokesSolver.h>
#include <QSharedPointer>
#include <ObjectController.h>
#include <OpenGLWidget.h>

#include <NSController.h>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
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
    std::cout << "mainWindow initialized" << std::endl;

    auto gl = mainWindow.getOpenGLWidget();

    auto scene = QSharedPointer<Scene>::create();
    std::cout << "created scene" << std::endl;

    Mesh mesh(2, MeshType::SPHERE);

    // 创建两个网格对象
    auto obj = QSharedPointer<Object>::create();
    obj->setObjectType(ObjectType::STATIC);
    obj->loadFromMesh(mesh);
    obj->setDrawMode(DrawMode::WIREFRAME);
    obj->setShader("basic");

    // Mesh mesh2(10, CUBE);
    // auto obj2 = QSharedPointer<Object>::create();
    // obj2->setObjectType(ObjectType::STATIC);
    // obj2->loadFromMesh(mesh2);
    // obj2->setDrawMode(DrawMode::WIREFRAME);
    // // obj2->setScale(glm::vec3(2.0f, 1.0f, 0.5f));
    // obj2->setPosition(QVector3D(2.0f, 0.0f, 2.0f));
    // obj2->setShaderManager(gl->m_shaderManager);
    // obj2->setShader("basic");

    scene->addObject(obj);
    // scene->addObject(obj2);

    // 定义随机引擎
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
                obj->setPosition(QVector3D((float)(i) + 4 + randomOffsetX,
                                           (float)(j) + 4 + randomOffsetY,
                                           2.0f + k + randomOffsetZ));

                // 随机颜色
                float red = colorDis(gen) * 0.3f + 0.7f; // 偏蓝色调
                float green = colorDis(gen) * 0.5f + 0.5f;
                float blue = colorDis(gen) * 0.2f + 0.8f;
                obj->setColorBuffer({red, green, blue});

                scene->addObject(obj);
            }
        }
    }

    // // 创建NS对象
    // auto obj3 = QSharedPointer<Object>::create();
    // obj3->setPosition(QVector3D(-2.0f, 0.0f, 2.0f));
    // obj3->setShaderManager(gl->m_shaderManager);
    // obj3->setShader("basic");
    // QSharedPointer<NSController> nsController = QSharedPointer<NSController>::create(50, SPHERE, obj3);
    // scene->addObject(obj3);
    // scene->addController(nsController);

    // auto obj4 = std::make_shared<Object>();
    // std::shared_ptr<ObjectController> nsController2 = std::make_shared<NSController>(100, SPHERE, obj4);
    // obj4->setPosition(glm::vec3(-2.0f, 0.0f, 4.0f));
    // obj4->attachShader(shader);
    // scene->addObject(obj4);
    // scene->addController(nsController2);

    // 测试Blinn_Phong 光照
    auto obj5 = QSharedPointer<Object>::create();
    obj5->setPosition(QVector3D(4.0f, 0.0f, 0.0f));
    obj5->setShader("blinn_phong");
    QSharedPointer<NSController> nsController3 = QSharedPointer<NSController>::create(50, MeshType::SPHERE, obj5);
    scene->addObject(obj5);
    scene->addController(nsController3);

    auto obj6 = QSharedPointer<Object>::create();
    obj6->setObjectType(ObjectType::STATIC);
    obj6->setDrawMode(DrawMode::FILL);
    obj6->setPosition(QVector3D(2.0f, 0.0f, 0.0f));
    obj6->setColorBuffer(nsController3->m_colorBufferFront);
    Mesh m(50, MeshType::SPHERE);
    obj6->loadFromMesh(m);
    obj6->setShader("basic");
    scene->addObject(obj6);

    // 测试glb模型
    // auto obj7 = QSharedPointer<Object>::create();
    // obj7->setPosition(QVector3D(10.0f, 0.0f, 0.0f));
    // obj7->setShaderManager(gl->m_shaderManager);
    // obj7->setShader("blinn_phong");
    // obj7->setObjectType(ObjectType::FEM);
    // obj7->setDrawMode(DrawMode::WIREFRAME);
    // if (obj7->loadFromGLB(":/items/crates_and_barrels.glb"))
    //     std::cout << "success" << std::endl;
    // scene->addObject(obj7);

    // 测试Material
    auto obj8 = QSharedPointer<Object>(new Object);
    obj8->loadCube();
    obj8->setPosition(QVector3D(5.0f, 0.0f, 5.0f));
    obj8->setShader("blinn_phong");
    obj8->setDrawMode(DrawMode::FILL);
    obj8->setObjectType(ObjectType::MATERIAL);
    obj8->setMaterial("brick_wall");
    scene->addObject(obj8);

    auto obj9 = QSharedPointer<Object>(new Object);
    obj9->loadCube();
    obj9->setPosition(QVector3D(5.0f, 0.0f, 7.0f));
    obj9->setShader("pbr");
    obj9->setDrawMode(DrawMode::FILL);
    obj9->setObjectType(ObjectType::MATERIAL);
    obj9->setMaterial("SpaceBlanketFolds");
    scene->addObject(obj9);

    // 测试load_square
    Mesh square(10, MeshType::SQUARE);
    auto obj10 = QSharedPointer<Object>(new Object);
    obj10->loadFromMesh(square);
    obj10->setPosition(QVector3D(-2.1f, 0.0f, 0.0f));
    obj10->setShader("basic");
    obj10->setDrawMode(DrawMode::WIREFRAME);
    obj10->setObjectType(ObjectType::STATIC);
    scene->addObject(obj10);

    auto obj11 = QSharedPointer<Object>(new Object);
    obj11->setPosition(QVector3D(-4.2f, 0.0f, 0.0f));
    obj11->setShader("blinn_phong");
    QSharedPointer<NSController> nsController4 = QSharedPointer<NSController>::create(50, MeshType::SQUARE, obj11);
    scene->addObject(obj11);
    scene->addController(nsController4);

    // 测试load_hemisphere
    Mesh hemi(5, MeshType::HEMI_SPHERE);
    std::cout << "hemi!" << std::endl;
    auto obj12 = QSharedPointer<Object>(new Object);
    obj12->loadFromMesh(hemi);
    obj12->setPosition(QVector3D(-2.1f, 2.1f, 0.0f));
    obj12->setShader("basic");
    obj12->setDrawMode(DrawMode::WIREFRAME);
    obj12->setObjectType(ObjectType::STATIC);
    scene->addObject(obj12);

    auto obj13 = QSharedPointer<Object>(new Object);
    obj13->setPosition(QVector3D(-4.2f, 2.1f, 0.0f));
    obj13->setShader("blinn_phong");
    QSharedPointer<NSController> nsController5 = QSharedPointer<NSController>::create(50, MeshType::HEMI_SPHERE, obj13);
    scene->addObject(obj13);
    scene->addController(nsController5);

    gl->addScene(scene);
    gl->initializeScenes();
    return app.exec(); // 启动应用程序事件循环
}
