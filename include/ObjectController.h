#pragma once

/* 对Object的状态进行更新的Controller
 * 每个ObjectController绑定一个Object
 * 这是一个基类，通过继承可以派生不同类型的ObjectController
 */

#include <QSharedPointer>

class Object;

class ObjectController
{
public:
    virtual ~ObjectController() = default;
    virtual void update(double dt) = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void reset() = 0;
};