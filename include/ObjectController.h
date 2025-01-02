#pragma once

/* 对Object的状态进行更新的Controller
 * 每个ObjectController绑定一个Object
 * 这是一个基类，通过继承可以派生不同类型的ObjectController
 */

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