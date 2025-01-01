#include <Scene.h>
#include <Shader.h>

Scene::Scene(/* args */)
    : sceneName("Default Scene Name")
{
}

Scene::~Scene()
{
}

void Scene::draw()
{
    for (auto obj : Objects)
    {
        obj.draw();

    }
}