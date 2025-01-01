#pragma once

#include <vector>
#include <Object.h>
#include <string>

class Scene
{
private:
    std::string sceneName;
    std::vector<Object> Objects;
public:
    Scene();
    ~Scene();

    void draw();
    void addObject(const Object &obj);
};


