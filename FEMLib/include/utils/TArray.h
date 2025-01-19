#pragma once

#include <NameSpace.h>
#include <sys_utils.h>
#include <stddef.h>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <cmath>
#include <initializer_list>

NAMESPACE_BEGIN(FEMLib)

template <typename T>
class TArray
{
public:
    // 构造函数
    TArray();                             // 无参数的构造函数，生成一个空的TArray
    TArray(size_t);                       // 生成指定大小的TArray
    TArray(size_t, T);                    // 生成指定大小且内部元素初始化为指定值的TArray
    TArray(const TArray<T> &other);       // 拷贝构造函数,即使用另一个现有的TArray来构造新的TArray
    TArray(std::initializer_list<T> init) // 支持{1,2,3}初始化的构造函数
        : size(init.size()), capacity(init.size()), data(new T[init.size()])
    {
        std::copy(init.begin(), init.end(), data);
    }

    // 析构函数，用于在对象生命周期结束时执行清理操作，这里要将data的地址释放
    ~TArray();

    // 运算符重载
    T &operator[](size_t i); // 重载[]运算符
    const T &operator[](size_t i) const;

    TArray &operator=(const TArray<T> &other); // 重载=赋值运算符

    // 重载算数运算符用来作为向量使用
    TArray<T> operator+(const TArray<T> &other);
    TArray<T> operator-(const TArray<T> &other); // 用于向量差 a-b
    TArray<T> operator-() const;                 // 用于 -a
    TArray<T> &operator+=(const TArray<T> &other);
    TArray<T> &operator-=(const TArray<T> &other);
    TArray<T> &operator*=(const T scalar);
    TArray<T> &operator/=(const T scalar);

    // 向量加法：result = this + other
    void add(const TArray<T> &other, TArray<T> &result) const;
    // 向量减法：result = this - other
    void subtract(const TArray<T> &other, TArray<T> &result) const;
    // 向量取负：result = -this
    void negate(TArray<T> &result) const;
    // 向量加法赋值：this += other
    void addInPlace(const TArray<T> &other);
    // 向量减法赋值：this -= other
    void subtractInPlace(const TArray<T> &other);
    // 向量数乘赋值：this *= scalar
    void scaleInPlace(const T scalar);
    // 向量除法赋值：this /= scalar
    void divideInPlace(const T scalar);

    // 声明为友元函数，需要显示声明为模板
    template <typename U>
    friend TArray<U> operator*(const TArray<U> &arr, const U scalar);
    template <typename U>
    friend TArray<U> operator*(const U scalar, const TArray<U> &arr);
    template <typename U>
    friend TArray<U> operator/(const TArray<U> &arr, const U scalar);
    template <typename U>
    friend std::ostream &operator<<(std::ostream &os, const TArray<U> &arr); // 通过声明友元的方式重载<<以使用cout输出

    // 方法
    size_t get_size() const;     // 获取TArray的元素个数
    size_t get_capacity() const; // 获取容量
    void push_back(const T &t);  // 向末尾增加一个元素,值为t
    void resize(size_t size);    // 修改Array的size
    T *erase(T *pos);            // 删除指定位置的元素
    T *erase(T *first, T *last); // 删除指定范围的元素
    void setAll(const T &value); // 将全部元素设置为value
    T sum();

    // 有关线性代数的方法
    T norm() const;
    T norm2() const;

    // begin() 与 end() 以支持迭代器
    T *begin() { return data; }
    T *end() { return data + size; }
    const T *begin() const { return data; }
    const T *end() const { return data + size; }

    // TArray中的成员
    size_t size;     // 当前array的元素个数
    size_t capacity; // 当前分配的容量
    T *data;         // 存储的T数组
};

typedef TArray<double> Vec;

// 构造函数
template <typename T>
TArray<T>::TArray()
    : size{0}, capacity{0}, data{nullptr}
{
}

template <typename T>
TArray<T>::TArray(size_t s)
    : size{s}, capacity{s}
{
    data = new T[s];
}

template <typename T>
TArray<T>::TArray(size_t s, T val)
    : size{s}, capacity{s}
{
    data = new T[s];
    for (int i = 0; i < s; ++i)
    {
        data[i] = val;
    }
}

template <typename T>
TArray<T>::TArray(const TArray<T> &other)        // 使用另一个TArray<T>对象来初始化
    : size{other.size}, capacity{other.capacity} // 同一个类的对象可以访问另一个对象的private参数，private和public是对于不同类而言的
{
    if (!other.data)
    {
        data = nullptr;
    }
    else
    {
        data = new T[capacity];
        std::copy(other.data, other.data + size, data);
    }
}

// 析构函数
template <typename T>
TArray<T>::~TArray()
{
    delete[] data;
}

// 运算符重载
template <typename T>
inline T &TArray<T>::operator[](size_t i)
{
    return data[i];
}

template <typename T>
inline const T &TArray<T>::operator[](size_t i) const
{
    return data[i];
}

template <typename T>
inline TArray<T> &TArray<T>::operator=(const TArray<T> &other)
{
    // this 是指向当前对象的指针
    if (this == &other)
    {
        return *this;
    }

    delete[] data;

    size = other.size;
    capacity = other.capacity;
    if (!other.data)
    {
        data = nullptr;
    }
    else
    {
        data = new T[capacity];
        std::copy(other.data, other.data + size, data);
    }

    return *this; // 返回当前对象的引用，以支持连续赋值
}

// 重载算数运算符用来作为向量使用

template <typename T>
inline TArray<T> TArray<T>::operator+(const TArray<T> &other)
{
    if (size != other.size)
    {
        throw std::invalid_argument("Size mismatch: Cannot add TArray objects of different sizes.");
    }

    TArray<T> res(size);
    for (size_t i = 0; i < size; ++i)
    {
        res.data[i] = data[i] + other.data[i];
    }
    return res;
}

template <typename T>
inline TArray<T> TArray<T>::operator-(const TArray<T> &other) // 用于向量差 a-b
{
    if (size != other.size)
    {
        throw std::invalid_argument("Size mismatch: Cannot subtract TArray objects of different sizes.");
    }

    TArray<T> res(size);
    for (size_t i = 0; i < size; ++i)
    {
        res.data[i] = data[i] - other.data[i];
    }
    return res;
}

template <typename T>
inline TArray<T> TArray<T>::operator-() const // 用于 -a
{
    TArray<T> res(size);
    for (size_t i = 0; i < size; ++i)
    {
        res.data[i] = -data[i];
    }
    return res;
}

template <typename T>
inline TArray<T> &TArray<T>::operator+=(const TArray<T> &other)
{
    if (size != other.size)
    {
        throw std::invalid_argument("Size mismatch: Cannot add TArray objects of different sizes.");
    }

    for (size_t i = 0; i < size; ++i)
    {
        data[i] += other.data[i];
    }
    return *this;
}

template <typename T>
inline TArray<T> &TArray<T>::operator-=(const TArray<T> &other)
{
    if (size != other.size)
    {
        throw std::invalid_argument("Size mismatch: Cannot subtract TArray objects of different sizes.");
    }

    for (size_t i = 0; i < size; ++i)
    {
        data[i] -= other.data[i];
    }
    return *this;
}

template <typename T>
inline TArray<T> &TArray<T>::operator*=(const T scalar)
{
    for (size_t i = 0; i < size; ++i)
    {
        data[i] *= scalar;
    }
    return *this;
}

template <typename T>
inline TArray<T> &TArray<T>::operator/=(const T scalar)
{
    if (scalar == 0)
    {
        throw std::domain_error("Division by zero: Cannot divide TArray by zero.");
    }

    for (size_t i = 0; i < size; ++i)
    {
        data[i] /= scalar;
    }
    return *this;
}

template <typename U>
inline TArray<U> operator*(const TArray<U> &v, const U scalar)
{
    TArray<U> res(v.size);
    for (size_t i = 0; i < v.size; ++i)
    {
        res[i] = scalar * v[i];
    }
    return res;
}

template <typename U>
inline TArray<U> operator*(const U scalar, const TArray<U> &v)
{
    TArray<U> res(v.size);
    for (size_t i = 0; i < v.size; ++i)
    {
        res[i] = scalar * v[i];
    }
    return res;
}

template <typename U>
inline TArray<U> operator/(const TArray<U> &v, const U scalar)
{
    if (scalar == 0)
    {
        throw std::domain_error("Division by zero: Cannot divide TArray by zero.");
    }

    TArray<U> res(v.size);
    for (size_t i = 0; i < v.size; ++i)
    {
        res[i] = v[i] / scalar;
    }
    return res;
}

template <typename U>
std::ostream &operator<<(std::ostream &os, const TArray<U> &arr)
{
    os << "[";
    for (size_t i = 0; i < arr.size; ++i)
    {
        os << arr.data[i];
        if (i < arr.size - 1)
        {
            os << ", ";
        }
    }
    os << "]";
    return os;
}

// 有关线性代数的方法
template <typename T>
T TArray<T>::norm() const
{
    T res = 0;
    for (size_t i = 0; i < size; ++i)
    {
        res += data[i] * data[i];
    }
    return std::sqrt(res);
}

template <typename T>
T TArray<T>::norm2() const
{
    T res = 0;
    for (size_t i = 0; i < size; ++i)
    {
        res += data[i] * data[i];
    }
    return res;
}

template <typename T>
T dot(const TArray<T> &a, const TArray<T> &b)
{
    if (a.size != b.size)
    {
        throw std::invalid_argument("Size mismatch: Cannot compute dot product for arrays of different sizes.");
    }

    T res = 0;
    for (size_t i = 0; i < a.size; ++i)
    {
        res += a[i] * b[i];
    }
    return res;
}

// 成员函数
template <typename T>
inline size_t TArray<T>::get_size() const { return size; }

template <typename T>
inline size_t TArray<T>::get_capacity() const { return capacity; }

template <typename T>
void TArray<T>::push_back(const T &t)
{
    // 首先检查容量是否足够
    if (size >= capacity)
    {
        size_t new_capacity = capacity ? 2 * capacity : 1;
        T *new_data = new T[new_capacity];

        for (size_t i = 0; i < size; ++i)
        {
            new_data[i] = std::move(data[i]);
        }

        delete[] data;
        data = new_data;
        capacity = new_capacity;
    }
    data[size++] = t;
}

template <typename T>
void TArray<T>::resize(size_t s)
{
    if (s > capacity)
    {
        T *new_data = new T[s];

        for (size_t i = 0; i < size; ++i)
        {
            new_data[i] = std::move(data[i]);
        }

        delete[] data;
        data = new_data;
        capacity = s;
    }
    size = s;
}

// 删除单个元素
template <typename T>
T *TArray<T>::erase(T *pos)
{
    if (pos >= data && pos < data + size)
    {
        // 将 pos 之后的元素前移一个位置
        std::move(pos + 1, data + size, pos);
        --size;
    }
    return pos;
}

// 删除范围内的元素
template <typename T>
T *TArray<T>::erase(T *first, T *last)
{
    if (first >= data && last <= data + size && first <= last)
    {
        // 将 last 之后的元素前移
        auto new_end = std::move(last, data + size, first);
        size -= (last - first); // 更新 size
    }
    return first;
}

template <typename T>
inline void TArray<T>::add(const TArray<T> &other, TArray<T> &result) const
{
    if (size != other.size || size != result.size)
    {
        throw std::invalid_argument("Size mismatch: Cannot add TArray objects of different sizes.");
    }

    for (size_t i = 0; i < size; ++i)
    {
        result.data[i] = data[i] + other.data[i];
    }
}

template <typename T>
inline void TArray<T>::subtract(const TArray<T> &other, TArray<T> &result) const
{
    if (size != other.size || size != result.size)
    {
        throw std::invalid_argument("Size mismatch: Cannot subtract TArray objects of different sizes.");
    }

    for (size_t i = 0; i < size; ++i)
    {
        result.data[i] = data[i] - other.data[i];
    }
}

template <typename T>
inline void TArray<T>::negate(TArray<T> &result) const
{
    if (size != result.size)
    {
        throw std::invalid_argument("Size mismatch: Result TArray size must match.");
    }

    for (size_t i = 0; i < size; ++i)
    {
        result.data[i] = -data[i];
    }
}

template <typename T>
inline void TArray<T>::addInPlace(const TArray<T> &other)
{
    if (size != other.size)
    {
        throw std::invalid_argument("Size mismatch: Cannot add TArray objects of different sizes.");
    }

    for (size_t i = 0; i < size; ++i)
    {
        data[i] += other.data[i];
    }
}

template <typename T>
inline void TArray<T>::subtractInPlace(const TArray<T> &other)
{
    if (size != other.size)
    {
        throw std::invalid_argument("Size mismatch: Cannot subtract TArray objects of different sizes.");
    }

    for (size_t i = 0; i < size; ++i)
    {
        data[i] -= other.data[i];
    }
}

template <typename T>
inline void TArray<T>::scaleInPlace(const T scalar)
{
    for (size_t i = 0; i < size; ++i)
    {
        data[i] *= scalar;
    }
}

template <typename T>
inline void TArray<T>::divideInPlace(const T scalar)
{
    if (scalar == 0)
    {
        throw std::domain_error("Division by zero: Cannot divide TArray by zero.");
    }

    for (size_t i = 0; i < size; ++i)
    {
        data[i] /= scalar;
    }
}

template <typename U>
inline void scale(const TArray<U> &v, const U scalar, TArray<U> &result)
{
    if (v.size != result.size)
    {
        throw std::invalid_argument("Size mismatch: Result TArray size must match.");
    }

    for (size_t i = 0; i < v.size; ++i)
    {
        result.data[i] = scalar * v.data[i];
    }
}

template <typename U>
inline void divide(const TArray<U> &v, const U scalar, TArray<U> &result)
{
    if (scalar == 0)
    {
        throw std::domain_error("Division by zero: Cannot divide TArray by zero.");
    }

    if (v.size != result.size)
    {
        throw std::invalid_argument("Size mismatch: Result TArray size must match.");
    }

    for (size_t i = 0; i < v.size; ++i)
    {
        result.data[i] = v.data[i] / scalar;
    }
}

template <typename T>
inline void TArray<T>::setAll(const T &value)
{
    for (size_t i = 0; i < size; ++i)
    {
        data[i] = value;
    }
}

template <typename T>
inline void blas_axpby(const T &a, const TArray<T> &x, const T &b, const TArray<T> &y, TArray<T> &out)
// out  = ax + by
{
    for (size_t i = 0; i < x.size; ++i)
    {
        out.data[i] = a * x.data[i] + b * y.data[i];
    }
}

template <typename T>
inline void blas_axpy(const T &a, const TArray<T> &x, TArray<T> &y)
// y = ax + y
{
    for (size_t i = 0; i < x.size; ++i)
    {
        y.data[i] += a * x.data[i];
    }
}

template <typename T>
T TArray<T>::sum()
{
    T s = 0;
    for (size_t i = 0; i < size; ++i)
    {
        s += data[i];
    }
    return s;
}

NAMESPACE_END