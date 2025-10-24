#pragma once
#include <type_traits>
#include <utility>
#include "detail/default_delete.hpp"

namespace my_ptr {

// unique_ptr 基类模板
template <typename T, typename Deleter = detail::default_delete<T>>
class unique_ptr {
public:
    using element_type = T;
    using deleter_type = Deleter;
    using pointer = T*;

private:
    pointer ptr_;
    deleter_type deleter_;
    
    // 编译时检查删除器是否可用
    template <typename U>
    static constexpr bool check_deleter() {
        return std::is_nothrow_move_constructible_v<Deleter> &&
               std::is_nothrow_move_assignable_v<Deleter>;
    }

public:
    // 构造/析构
    constexpr unique_ptr() noexcept : ptr_(nullptr), deleter_() {}

    constexpr unique_ptr(std::nullptr_t) noexcept : ptr_(nullptr), deleter_() {}

    explicit unique_ptr(pointer ptr) noexcept : ptr_(ptr), deleter_() {}

    unique_ptr(pointer p, const deleter_type& deleter) noexcept 
        : ptr_(p), deleter_(deleter) {}

    unique_ptr(pointer p, deleter_type&& deleter) noexcept 
        : ptr_(p), deleter_(std::move(deleter)) {}

    // 移动构造
    unique_ptr(unique_ptr&& other) noexcept 
        : ptr_(other.ptr_), deleter_(std::move(other.deleter_)) {
        other.ptr_ = nullptr;
    }

   template<typename U, typename E,  typename = std::enable_if_t<
                std::is_convertible<typename unique_ptr<U, E>::pointer, pointer>::value &&
                std::is_assignable<deleter_type&, E&&>::value>>
    unique_ptr(unique_ptr<U, E>&& other) noexcept 
        : ptr_(other.ptr_), deleter_(std::move(other.deleter_)) {
        other.ptr_ = nullptr;
    }

    ~unique_ptr() {
        reset();
    }

    // 禁止拷贝
    unique_ptr(const unique_ptr&) = delete;
    unique_ptr& operator=(const unique_ptr&) = delete;

    // 移动赋值
    unique_ptr& operator=(unique_ptr&& other) noexcept {
        reset();
        ptr_ = other.ptr_;
        deleter_ = std::move(other.deleter_);
        other.ptr_= nullptr;
        return *this;
    }

    unique_ptr& operator=(std::nullptr_t) noexcept {
        reset();
        return *this;
    }

    // 核心接口
    pointer release() noexcept {
        pointer p = ptr_;
        ptr_ = nullptr;
        return p;
    }   

    void reset(pointer p = pointer()) noexcept {
        pointer old = ptr_;
        ptr_ = p;
        if (old) {
            deleter_(old);
        }
    }

    void swap(unique_ptr& other) noexcept {
        std::swap(ptr_, other.ptr_);
        std::swap(deleter_, other.deleter_);
    }

    // 访问器
    pointer get() const noexcept { return ptr_; }
    deleter_type& get_deleter() noexcept { return deleter_; }
    const deleter_type& get_deleter() const noexcept { return deleter_; }
    
    T& operator*() const noexcept { return *ptr_; }
    pointer operator->() const noexcept { return ptr_; }
    
    explicit operator bool() const noexcept { return ptr_ != nullptr; }

    // 数组操作符重载
    T& operator[](std::size_t idx) const noexcept { return ptr_[idx]; }
};

// 非成员函数
template <typename T, typename Deleter>
void swap(unique_ptr<T, Deleter>& lhs, unique_ptr<T, Deleter>& rhs) noexcept {
    lhs.swap(rhs);
}

template <typename T, typename Deleter>
bool operator==(const unique_ptr<T, Deleter>& ptr, std::nullptr_t) noexcept {
    return !ptr;
}

template <typename T, typename Deleter>
bool operator==(std::nullptr_t, const unique_ptr<T, Deleter>& ptr) noexcept {
    return !ptr;
}

template <typename T, typename Deleter>
bool operator!=(const unique_ptr<T, Deleter>&ptr, std::nullptr_t) noexcept {
    return static_cast<bool>(ptr);
}

template<typename T, typename Deleter>
bool operator!=(std::nullptr_t, const unique_ptr<T, Deleter>& ptr) noexcept {
    return static_cast<bool>(ptr);
}

// 工厂函数
template <typename T, typename... Args> 
unique_ptr<T> make_unique(Args&&... args) {
    return unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <typename T>
unique_ptr<T> make_unique_for_overwrite() {
    return unique_ptr<T>(new T);
} 

template <typename T>
unique_ptr<T[]> make_unique(std::size_t size) {
    return unique_ptr<T[]>(new T[size]);
}

template <typename T>
unique_ptr<T[]> make_unique_for_overwrite(std::size_t size) {
    return unique_ptr<T[]>(new T[size]);
}

} // namespace my_ptr
