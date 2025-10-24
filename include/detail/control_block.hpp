/*
    控制块实现 
*/

#pragma once
#include <atomic>
#include <type_traits>
#include <utility>
#include "default_delete.hpp"

namespace my_ptr {
namespace detail {

// 控制块基类
class control_block_base {
protected:
    std::atomic<size_t> shared_count_;
    std::atomic<size_t> weak_count_;

public:
    control_block_base() noexcept 
        : shared_count_(1), weak_count_(1) {}

    virtual ~control_block_base() = default;

    virtual void dispose() noexcept = 0; // 销毁托管对象
    virtual void destroy() noexcept = 0; // 销毁控制块对象

    void add_shared_ref() noexcept {
        shared_count_.fetch_add(1, std::memory_order_acq_rel);
    }

    void release_shared() noexcept {
        if (shared_count_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            dispose();
            release_weak();
        }
    }

    void add_weak_ref() noexcept {
        weak_count_.fetch_add(1, std::memory_order_acq_rel);
    }

    void release_weak() noexcept {
        if (weak_count_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            destroy();
        }
    }

    // 尝试增加共享对象引用计数
    bool try_add_shared_ref() noexcept {
        size_t count = shared_count_.load(std::memory_order_acquire);
        while (count != 0) {
            if (shared_count_.compare_exchange_weak(count, count + 1, std::memory_order_acq_rel, std::memory_order_relaxed)) {
                return true;
            }
        }
        return false;
    }

    size_t use_count() const noexcept {
        return shared_count_.load(std::memory_order_relaxed);
    }
};

// 分离式控制块（指针和删除器单独实现）
template <typename T, typename Deleter>
class separate_control_block : public control_block_base {
private:
    T *ptr_;
    Deleter deleter_;

public:
    separate_control_block(T *ptr, Deleter deleter) noexcept
        : control_block_base(), ptr_(ptr), deleter_(std::move(deleter)) {}
    
    void dispose() noexcept override {
        if (ptr_) {
            // std::exchange(ptr_, nullptr);
            deleter_(ptr_);
            ptr_ = nullptr;
        }
    }

    void destroy() noexcept override {
        delete this;
    }
};

// 内联控制块（make_shared优化，对象和控制块一起分配）
template <typename T>
class inline_control_block : public control_block_base {
private:
    alignas(alignof(T)) unsigned char storage_[sizeof(T)];

    T *get_ptr() noexcept {
        return reinterpret_cast<T*>(storage_);
    }

    const T *get_ptr() const noexcept {
        return reinterpret_cast<const T*>(storage_);
    }

public:
    template <typename... Args>
    inline_control_block(Args&&... args) : control_block_base() {
        new (storage_) T(std::forward<Args>(args)...);
    }

    void dispose() noexcept override {
        get_ptr()->~T();
    }

    void destroy() noexcept override {
        delete this;
    }

    T* get() noexcept {
        return get_ptr();
    }

    const T* get() const noexcept {
        return get_ptr();
    }
};

// 控制块工厂函数
template <typename T, typename Deleter>
control_block_base* make_control_block(T *ptr, Deleter&& deleter) {
    return new separate_control_block<T, std::decay_t<Deleter>>(ptr, std::forward<Deleter>(deleter));
}

template <typename T, typename... Args>
control_block_base* make_inline_control_block(Args&&... args) {
    return new inline_control_block<T>(std::forward<Args>(args)...);
}

} // namespace detail
} // namespace my_ptr


