/*
    默认删除器
*/
#pragma once
#include "traits.hpp"

namespace my_ptr {
namespace detail {

// 默认删除器
template <typename T>
struct default_delete {
    constexpr default_delete() noexcept = default;

    template <typename U, typename = enable_if_t<std::is_convertible_v<U*, T*>>>
    default_delete(const default_delete<U>&) noexcept {}

    void operator() (T *ptr) const noexcept {
        static_assert(sizeof(T) > 0, "default_delete can not delete incomplete type");
        delete ptr;
    }  
};

// 数组特化
template <typename T>
struct default_delete<T[]> {
    constexpr default_delete() noexcept = default;

    template <typename U, typename = enable_if_t<std::is_convertible<U(*)[], T(*)[]>::value>>
    default_delete(const default_delete<U[]>&) noexcept {}

    void operator() (T *ptr) const noexcept {
        static_assert(sizeof(T) > 0, "default_delete can not delete incomplete type");
        delete[] ptr;
    }
};


} // namespace detail
} // namespace my_ptr