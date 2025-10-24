#pragma once
#include "shared_ptr.hpp"
#include <exception>

namespace my_ptr {

// 自定义异常类
class bad_weak_ptr : public std::exception {
public:
    bad_weak_ptr() noexcept = default;
    const char* what() const noexcept override {
        return "bad_weak_ptr";
    }
};

template <typename T>
class weak_ptr {
public:
    using element_type = T;
private:
    element_type *ptr_;
    detail::control_block_base *ctrl_block_;

    template <typename U>
    friend class shared_ptr;
    template <typename U>
    friend class weak_ptr;

public:
    // 构造/析构
    constexpr weak_ptr() noexcept : ptr_(nullptr), ctrl_block_(nullptr) {}

    // 从shared_ptr构造
    template <typename U>
    weak_ptr(const shared_ptr<U>& other) noexcept : ptr_(other.ptr_), ctrl_block_(other.ctrl_block_) {
        if (ctrl_block_) {
            ctrl_block_->add_weak_ref();
        }
    }

    // 拷贝构造
    weak_ptr(const weak_ptr& other) noexcept : ptr_(other.ptr_), ctrl_block_(other.ctrl_block_) {
        if (ctrl_block_) {
            ctrl_block_->add_weak_ref();
        }
    }

    template <typename U>
    weak_ptr(const weak_ptr<U>& other) noexcept : ptr_(other.ptr_), ctrl_block_(other.ctrl_block_) {
        if (ctrl_block_) {
            ctrl_block_->add_weak_ref();
        }
    }
    // 移动构造
    weak_ptr(weak_ptr&& other) noexcept : ptr_(other.ptr_), ctrl_block_(other.ctrl_block_) {
        other.ptr_ = nullptr;
        other.ctrl_block_ = nullptr;
    }

    template <typename U>
    weak_ptr(weak_ptr<U>&& other) noexcept : ptr_(other.ptr_), ctrl_block_(other.ctrl_block_) {
        other.ptr_ = nullptr;
        other.ctrl_block_ = nullptr;
    }

    ~weak_ptr() {
        if (ctrl_block_) {
            ctrl_block_->release_weak();
        }
    }

    // 赋值操作
    weak_ptr& operator=(const weak_ptr& other) noexcept {
        weak_ptr(other).swap(*this);
        return *this;
    }

    template <typename U>
    weak_ptr& operator=(const weak_ptr<U>& other) noexcept {
        weak_ptr(other).swap(*this);
        return *this;
    }

    weak_ptr& operator=(weak_ptr&& other) noexcept {
        weak_ptr(std::move(other)).swap(*this);
        return *this;
    }

    template <typename U>
    weak_ptr& operator=(weak_ptr<U>&& other) noexcept {
        weak_ptr(std::move(other)).swap(*this);
        return *this;
    }

    template <typename U>
    weak_ptr& operator=(const shared_ptr<U>& other) noexcept {
        weak_ptr(other).swap(*this);
        return *this;
    }

    // 核心接口
    void reset() noexcept {
        weak_ptr().swap(*this);
    }

    void swap(weak_ptr& other) noexcept {
        std::swap(ptr_, other.ptr_);
        std::swap(ctrl_block_, other.ctrl_block_);
    }

    long use_count() const noexcept {
        return ctrl_block_ ? static_cast<long>(ctrl_block_->use_count()) : 0;
    }

    bool expired() const noexcept {
        return use_count() == 0;
    }

    shared_ptr<T> lock() const noexcept {
        if (ctrl_block_ && ctrl_block_->try_add_shared_ref()) {
            return shared_ptr<T>(ctrl_block_, ptr_);
        } 
        return shared_ptr<T>();
    }
};

// shared_ptr从weak_ptr构造的实现
template <typename T>
template <typename U>
shared_ptr<T>::shared_ptr(const weak_ptr<U>& other) : ptr_(other.ptr_), ctrl_block_(other.ctrl_block_) {
    if (ctrl_block_ && !ctrl_block_->try_add_shared_ref()) {
        throw bad_weak_ptr();
    }
}

// 非成员函数
template<typename T>
void swap(weak_ptr<T>& lhs, weak_ptr<T>& rhs) noexcept {
    lhs.swap(rhs);
}

} // namespace my_ptr