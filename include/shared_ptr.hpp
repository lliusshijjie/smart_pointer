#pragma once
#include <type_traits>
#include <utility>
#include "detail/control_block.hpp"

namespace my_ptr {

// 前向声明
template <typename T>
class weak_ptr;

template <typename T>
class shared_ptr {
public:     
    using element_type = T;
    using weak_type = weak_ptr<T>;

    template<typename U, typename... Args>
    friend shared_ptr<U> make_shared(Args&&... args);

private:
    element_type *ptr_;
    detail::control_block_base *ctrl_block_;

    // 从控制块构造
    shared_ptr(detail::control_block_base *cb, element_type *p) noexcept
        : ptr_(p), ctrl_block_(cb) {}
    
    template <typename U>
    friend class shared_ptr;
    template <typename U>
    friend class weak_ptr;

public:
    // 构造/析构
    constexpr shared_ptr() noexcept : ptr_(nullptr), ctrl_block_(nullptr) {}
    constexpr shared_ptr(std::nullptr_t) noexcept : ptr_(nullptr), ctrl_block_(nullptr) {}

    template <typename U>
    explicit shared_ptr(U *ptr) 
    : ptr_(ptr), ctrl_block_(detail::make_control_block(ptr, detail::default_delete<U>())) {}
  
    template <typename U, typename Deleter>
    shared_ptr(U *ptr, Deleter&& deleter) : ptr_(ptr), ctrl_block_(detail::make_control_block(ptr, std::move<Deleter>(deleter))) {}

    // 拷贝构造
    shared_ptr(const shared_ptr& other) noexcept : ptr_(other.ptr_), ctrl_block_(other.ctrl_block_) { 
        if (ctrl_block_) {
            ctrl_block_->add_shared_ref();
        }
    }

    template <typename U>
    shared_ptr(const shared_ptr<U>& other) noexcept : ptr_(other.ptr_), ctrl_block_(other.ctrl_block_) {
        if (ctrl_block_) {
            ctrl_block_->add_shared_ref();
        }
    } 
    
    // 移动构造
    shared_ptr(shared_ptr&& other) noexcept 
        : ptr_(other.ptr_), ctrl_block_(other.ctrl_block_) {
        other.ptr_ = nullptr;
        other.ctrl_block_ = nullptr;
    }
    
    template<typename U>
    shared_ptr(shared_ptr<U>&& other) noexcept 
        : ptr_(other.ptr_), ctrl_block_(other.ctrl_block_) {
        other.ptr_ = nullptr;
        other.ctrl_block_ = nullptr;
    }

    // 从weak_ptr构造
    template <typename U>
    explicit shared_ptr(const weak_ptr<U>& other);

    ~shared_ptr() {
        if (ctrl_block_) {
            ctrl_block_->release_shared();
        }
    }

    // 赋值构造
    shared_ptr& operator=(const shared_ptr& other) noexcept {
        shared_ptr(other).swap(*this);
        return *this;
    }

    template <typename U>
    shared_ptr& operator=(const shared_ptr<U>& other) noexcept {
        shared_ptr(other).swap(*this);
        return *this;
    }

    shared_ptr& operator=(shared_ptr&& other) noexcept {
        shared_ptr(std::move(other)).swap(*this);
        return *this;
    }

    template <typename U>
    shared_ptr& operator=(shared_ptr<U>&& other) noexcept {
        shared_ptr(std::move(other)).swap(*this);
        return *this;
    }

    // 核心接口
    void reset() noexcept {
        shared_ptr().swap(*this);
    }

    template <typename U>
    void reset(U *ptr) {
        shared_ptr(ptr).swap(*this);
    }

    template <typename U, typename Deleter>
    void reset(U *ptr, Deleter deleter) {
        shared_ptr(ptr, std::move(deleter)).swap(*this);
    }

    void swap(shared_ptr& other) noexcept {
        std::swap(ptr_, other.ptr_);
        std::swap(ctrl_block_, other.ctrl_block_);
    }    

    // 访问器
    element_type *get() const noexcept { return ptr_; }
    element_type& operator*() const noexcept { return*ptr_; }
    element_type* operator->() const noexcept { return ptr_; }
    explicit operator bool() const noexcept { return ptr_ != nullptr; }
    
    long use_count() const noexcept { return ctrl_block_ ? static_cast<long>(ctrl_block_->use_count()) : 0;}
    bool unique() const noexcept { return use_count() == 1; }
};

// 非成员函数
template<typename T>
void swap(shared_ptr<T>& lhs, shared_ptr<T>& rhs) noexcept {
    lhs.swap(rhs);
}

template<typename T, typename U>
bool operator==(const shared_ptr<T>& lhs, const shared_ptr<U>& rhs) noexcept {
    return lhs.get() == rhs.get();
}

template<typename T>
bool operator==(const shared_ptr<T>& lhs, std::nullptr_t) noexcept {
    return !lhs;
}

template<typename T>
bool operator==(std::nullptr_t, const shared_ptr<T>& rhs) noexcept {
    return !rhs;
}

template<typename T>
bool operator!=(const shared_ptr<T>& lhs, std::nullptr_t) noexcept {
    return static_cast<bool>(lhs);
}

template<typename T>
bool operator!=(std::nullptr_t, const shared_ptr<T>& rhs) noexcept {
    return static_cast<bool>(rhs);
}

template<typename T, typename U>
bool operator!=(const shared_ptr<T>& lhs, const shared_ptr<U>& rhs) noexcept {
    return lhs.get() != rhs.get();
}

template<typename T>
bool operator<(const shared_ptr<T>& lhs, const shared_ptr<T>& rhs) noexcept {
    return lhs.get() < rhs.get();
}

// 工厂函数
template<typename T, typename... Args>
shared_ptr<T> make_shared(Args&&... args);

} // namespace my_ptr
