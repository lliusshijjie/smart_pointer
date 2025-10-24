#pragma once

// Standard library includes
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <memory>
#include <type_traits>
#include <utility>
#include <atomic>
#include <exception>
#include <cstddef>
#include <cassert>

// Project headers
#include "unique_ptr.hpp"
#include "shared_ptr.hpp"
#include "weak_ptr.hpp"
#include "detail/control_block.hpp"

namespace my_ptr {

// make_shared
template <typename T, typename... Args>
shared_ptr<T> make_shared(Args&&... args) {
    auto *ctrl_block = detail::make_inline_control_block<T>(std::forward<Args>(args)...);
    shared_ptr<T> result;
    result.ptr_ = static_cast<detail::inline_control_block<T>*>(ctrl_block)->get();
    result.ctrl_block_ = ctrl_block;
    return result;
}

// allocate_shared
template <typename T, typename Alloc, typename... Args>
shared_ptr<T> allocate_shared(const Alloc& alloc, Args&&... args) {
    using ControlBlockType = detail::inline_control_block<T>;
    
    // 分配内存
    using AllocTraits = std::allocator_traits<Alloc>;
    using RebindAlloc = typename AllocTraits::template rebind_alloc<ControlBlockType>;
    RebindAlloc rebound_alloc(alloc);
    
    ControlBlockType* ctrl_block = AllocTraits::allocate(rebound_alloc, 1);
    try {
        // 构造控制块和对象
        AllocTraits::construct(rebound_alloc, ctrl_block, std::forward<Args>(args)...);
    } catch (...) {
        AllocTraits::deallocate(rebound_alloc, ctrl_block, 1);
        throw;
    }
    
    shared_ptr<T> result;
    result.ptr_ = ctrl_block->get();
    result.ctrl_block_ = ctrl_block;
    return result;

}

}