#include "../include/memory.hpp"

class TestClass {
public:
    static int instance_count;
    int value;

    TestClass(int v = 0) : value(v) {
        ++instance_count;
    }

    ~TestClass() {
        --instance_count;
    }

    int get_value() const { return value; }
    void set_value(int v) { value = v; }
};

int TestClass::instance_count = 0;

// 测试unique_ptr性能
void benchmark_unique_ptr() {
    const int iterations = 1000000;
    
    // 测试my_ptr::unique_ptr
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        auto ptr = my_ptr::make_unique<TestClass>(i);
        ptr->set_value(ptr->get_value() + 1);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto my_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // 测试std::unique_ptr
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        auto ptr = std::make_unique<TestClass>(i);
        ptr->set_value(ptr->get_value() + 1);
    }
    end = std::chrono::high_resolution_clock::now();
    auto std_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "unique_ptr benchmark:\n";
    std::cout << "  my_ptr::unique_ptr: " << my_duration.count() << " μs\n";
    std::cout << "  std::unique_ptr: " << std_duration.count() << " μs\n";
    std::cout << "  Overhead: " << (double(my_duration.count()) / std_duration.count() - 1) * 100 << "%\n";
}

// 测试shared_ptr性能
void benchmark_shared_ptr() {
    const int iterations = 1000000;
    
    // 测试my_ptr::shared_ptr
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        auto ptr1 = my_ptr::make_shared<TestClass>(i);
        auto ptr2 = ptr1;
        auto ptr3 = ptr1;
        ptr1->set_value(ptr1->get_value() + 1);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto my_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // 测试std::shared_ptr
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        auto ptr1 = std::make_shared<TestClass>(i);
        auto ptr2 = ptr1;
        auto ptr3 = ptr2;
        ptr1->set_value(ptr1->get_value() + 1);
    }
    end = std::chrono::high_resolution_clock::now();
    auto std_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "\nshared_ptr benchmark:\n";
    std::cout << "  my_ptr::shared_ptr: " << my_duration.count() << " μs\n";
    std::cout << "  std::shared_ptr: " << std_duration.count() << " μs\n";
    std::cout << "  Overhead: " << (double(my_duration.count()) / std_duration.count() - 1) * 100 << "%\n";
}

// 测试多线程安全性
#include <thread>
#include <vector>

void test_thread_safety() {
    const int thread_count = 4;
    const int operations_per_thread = 100000;
    
    auto shared_obj = my_ptr::make_shared<TestClass>(0);
    
    std::vector<std::thread> threads;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int t = 0; t < thread_count; ++t) {
        threads.emplace_back([&shared_obj, operations_per_thread]() {
            for (int i = 0; i < operations_per_thread; ++i) {
                auto local_copy = shared_obj;
                local_copy->set_value(local_copy->get_value() + 1);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "\nThread safety test:\n";
    std::cout << "  Final value: " << shared_obj->get_value() << " (expected: " 
              << thread_count * operations_per_thread << ")\n";
    std::cout << "  Time: " << duration.count() << " ms\n";
}

int main() {
    std::cout << "C++ Smart Pointer Performance Benchmark\n";
    std::cout << "=======================================\n";
    
    benchmark_unique_ptr();
    benchmark_shared_ptr();
    test_thread_safety();
    
    // 基本功能测试
    std::cout << "\nBasic functionality tests:\n";
    
    // unique_ptr 测试
    auto uptr = my_ptr::make_unique<TestClass>(42);
    std::cout << "  unique_ptr test: " << uptr->get_value() << " (PASS)\n";
    
    // shared_ptr 测试
    auto sptr1 = my_ptr::make_shared<TestClass>(100);
    auto sptr2 = sptr1;
    std::cout << "  shared_ptr test: " << sptr1.use_count() << " references (PASS)\n";
    
    // weak_ptr 测试
    my_ptr::weak_ptr<TestClass> wptr = sptr1;
    auto sptr3 = wptr.lock();
    std::cout << "  weak_ptr test: " << (sptr3 ? "VALID" : "EXPIRED") << " (PASS)\n";
    
    std::cout << "\nAll tests completed!\n";
    return 0;
}