#include <stdexcept>
#include <utility>
#include <memory>
#include <cassert>

// Исключение этого типа должно генерироватся при обращении к пустому optional
class BadOptionalAccess : public std::exception
{
public:
    using exception::exception;

    virtual const char *what() const noexcept override
    {
        return "Bad optional access";
    }
};

template <typename T>
class Optional
{
public:
    Optional()
    {
    }
    Optional(const T &value)
        : is_initialized_{true}
    {
        new (data_) T(value);
    }

    Optional(T &&value)
        : is_initialized_{true}
    {
        new (data_) T(std::move(value));
    }

    Optional(const Optional &other)
    {
        if (is_initialized_ && other.is_initialized_)
        {
            Value() = other.Value();
        }
        else if (is_initialized_)
        {
            is_initialized_ = false;
            Value().~T();
        }
        else if (other.is_initialized_)
        {
            is_initialized_ = true;
            new (data_) T(other.Value());
        }
    }

    Optional(Optional &&other)
    {
        new (data_) T(std::move(other.Value()));
        is_initialized_ = true;
        //        other.is_initialized_ = false;
    }

    Optional &operator=(const T &value)
    {
        is_initialized_ = true;
        new (data_) T(value);
        return *this;
    }
    Optional &operator=(T &&rhs)
    {
        is_initialized_ = true;
        new (data_) T(std::move(rhs));
        return *this;
    }

    Optional &operator=(const Optional &other)
    {
        if (this != &other)
        {
            if (is_initialized_ && other.is_initialized_)
            {
                Value() = other.Value();
            }
            else if (is_initialized_)
            {
                Value().~T();
                is_initialized_ = false;
            }
            else if (other.is_initialized_)
            {
                is_initialized_ = true;
                new (data_) T(other.Value());
            }
        }
        return *this;
    }

    Optional &swap(Optional &other)
    {
        if (is_initialized_ && other.is_initialized_)
        {
            std::swap(Value(), other.Value());
        }
        else if (is_initialized_)
        {
            other.Value() = std::move(Value());
            Value().~T();
            std::swap(is_initialized_, other.is_initialized_);
        }
        else if (other.is_initialized_)
        {
            new (data_) T(other.Value());
            other.Value().~T();
            std::swap(is_initialized_, other.is_initialized_);
        }
    }

    Optional &operator=(Optional &&other)
    {
        if (this != &other)
        {
            if (is_initialized_ && other.is_initialized_)
            {
                Value() = std::move(other.Value());
                other.is_initialized_ = false;
            }
            else if (is_initialized_)
            {
                Value().~T();
                is_initialized_ = false;
            }
            else if (other.is_initialized_)
            {
                is_initialized_ = true;
                new (data_) T(std::move(other.Value()));
                other.is_initialized_ = false;
            }
        }
        return *this;
    }

    ~Optional()
    {
        if (is_initialized_)
        {
            Value().~T(); // destroy the T
            is_initialized_ = false;
        }
    }

    bool HasValue() const
    {
        return is_initialized_;
    }

    // Операторы * и -> не должны делать никаких проверок на пустоту Optional.
    // Эти проверки остаются на совести программиста
    T &operator*()
    {
        return reinterpret_cast<T &>(*data_);
    }

    const T &operator*() const
    {
        return reinterpret_cast<const T &>(*data_);
    }

    T *operator->()
    {
        return reinterpret_cast<T *>(data_);
    }
    const T *operator->() const
    {
        return reinterpret_cast<const T *>(data_);
    }

    // Метод Value() генерирует исключение BadOptionalAccess, если Optional пуст
    T &Value()
    {
        if (!is_initialized_)
        {
            throw BadOptionalAccess{};
        }
        return reinterpret_cast<T &>(*data_);
    }
    const T &Value() const
    {
        if (!is_initialized_)
        {
            throw BadOptionalAccess{};
        }
        return reinterpret_cast<const T &>(*data_);
    }

    void Reset()
    {
        is_initialized_ = false;
    }

private:
    // alignas нужен для правильного выравнивания блока памяти
    alignas(T) char data_[sizeof(T)];
    bool is_initialized_ = false;
};
