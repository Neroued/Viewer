#pragma once

#include <memory>
#include <Object.h>

class ObjectController
{
public:
    virtual ~ObjectController() = default;
    virtual void update(double dt) = 0;

    void bindObject(std::shared_ptr<Object> obj) { m_object = obj; }

private:
    std::shared_ptr<Object> m_object;
};