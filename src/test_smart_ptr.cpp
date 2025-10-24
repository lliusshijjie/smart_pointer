#include "../include/memory.hpp"

// ============================================================================
// 测试辅助宏
// ============================================================================
#define TEST_SECTION(name) std::cout << "--- Testing " << name << " ---\n"

// ============================================================================
// 测试用的类
// ============================================================================
struct TestClass {
    int value;
    static int instance_count;

    explicit TestClass(int v = 0) : value(v) {
        ++instance_count;
    }
    ~TestClass() {
        --instance_count;
    }
    TestClass(const TestClass& other) : value(other.value) {
        ++instance_count;
    }
    TestClass& operator=(const TestClass& other) {
        if (this != &other) {
            value = other.value;
        }
        return *this;
    }
};

int TestClass::instance_count = 0;

struct CustomDeleter {
    void operator()(TestClass* p) const {
        delete p;
    }
};


// ============================================================================
// 测试函数声明
// ============================================================================
bool test_unique_ptr_basics();
bool test_unique_ptr_move();
bool test_unique_ptr_modifiers();
bool test_unique_ptr_make_unique();
bool test_unique_ptr_custom_deleter();
bool test_unique_ptr_comparison();

bool test_shared_ptr_basics();
bool test_shared_ptr_copy();
bool test_shared_ptr_move();
bool test_shared_ptr_reset();
bool test_shared_ptr_make_shared();
bool test_shared_ptr_unique();
bool test_shared_ptr_comparison();

bool test_weak_ptr_basics();
bool test_weak_ptr_expired();
bool test_weak_ptr_copy_move();
bool test_weak_ptr_multiple();

bool test_integration_shared_weak();
bool test_integration_containers();

// ============================================================================
// main 函数
// ============================================================================
int main() {
    bool all_tests_passed = true;

    auto run_test = [&](const char* name, bool (*test_func)()) {
        TestClass::instance_count = 0;
        std::cout << "\n============================================================================\n";
        std::cout << "Running test suite: " << name << "\n";
        std::cout << "============================================================================\n";
        if (!test_func()) {
            all_tests_passed = false;
            std::cout << "xxx Test suite FAILED: " << name << " xxx\n";
        } else {
            std::cout << ">>> Test suite PASSED: " << name << " <<<\n";
        }
        assert(TestClass::instance_count == 0 && "Memory leak detected: instance_count is not zero after test!");
    };

    run_test("unique_ptr basics", test_unique_ptr_basics);
    run_test("unique_ptr move semantics", test_unique_ptr_move);
    run_test("unique_ptr modifiers", test_unique_ptr_modifiers);
    run_test("unique_ptr make_unique", test_unique_ptr_make_unique);
    run_test("unique_ptr custom deleter", test_unique_ptr_custom_deleter);
    run_test("unique_ptr comparison", test_unique_ptr_comparison);

    run_test("shared_ptr basics", test_shared_ptr_basics);
    run_test("shared_ptr copy semantics", test_shared_ptr_copy);
    run_test("shared_ptr move semantics", test_shared_ptr_move);
    run_test("shared_ptr reset", test_shared_ptr_reset);
    run_test("shared_ptr make_shared", test_shared_ptr_make_shared);
    run_test("shared_ptr unique", test_shared_ptr_unique);
    run_test("shared_ptr comparison", test_shared_ptr_comparison);

    run_test("weak_ptr basics", test_weak_ptr_basics);
    run_test("weak_ptr expired", test_weak_ptr_expired);
    run_test("weak_ptr copy and move", test_weak_ptr_copy_move);
    run_test("weak_ptr multiple pointers", test_weak_ptr_multiple);

    run_test("shared_ptr and weak_ptr interaction", test_integration_shared_weak);
    run_test("smart pointers in containers", test_integration_containers);

    std::cout << "\n\n----------------------------------------------------------------------------\n";
    if (all_tests_passed) {
        std::cout << "All tests passed successfully!\n";
    } else {
        std::cout << "Some tests failed.\n";
    }
    std::cout << "----------------------------------------------------------------------------\n";

    return all_tests_passed ? 0 : 1;
}

// ============================================================================
// unique_ptr 测试
// ============================================================================
bool test_unique_ptr_basics() {
    TEST_SECTION("unique_ptr basics");
    {
        my_ptr::unique_ptr<TestClass> ptr1(new TestClass(10));
        assert(ptr1);
        assert(ptr1->value == 10);
        assert(TestClass::instance_count == 1);

        TestClass* raw_ptr = ptr1.get();
        assert(raw_ptr != nullptr);
        assert(raw_ptr->value == 10);
    }
    assert(TestClass::instance_count == 0);
    std::cout << "success! unique_ptr basic functionality\n";
    return true;
}

bool test_unique_ptr_move() {
    TEST_SECTION("unique_ptr move semantics");
    {
        my_ptr::unique_ptr<TestClass> ptr1(new TestClass(20));
        assert(TestClass::instance_count == 1);

        my_ptr::unique_ptr<TestClass> ptr2(std::move(ptr1));
        assert(!ptr1);
        assert(ptr2);
        assert(ptr2->value == 20);
        assert(TestClass::instance_count == 1);

        my_ptr::unique_ptr<TestClass> ptr3;
        ptr3 = std::move(ptr2);
        assert(!ptr2);
        assert(ptr3);
        assert(ptr3->value == 20);
        assert(TestClass::instance_count == 1);
    }
    assert(TestClass::instance_count == 0);
    std::cout << "success! unique_ptr move semantics\n";
    return true;
}

bool test_unique_ptr_modifiers() {
    TEST_SECTION("unique_ptr modifiers (release, reset)");
    {
        my_ptr::unique_ptr<TestClass> ptr(new TestClass(30));
        assert(TestClass::instance_count == 1);

        TestClass* raw = ptr.release();
        assert(!ptr);
        assert(TestClass::instance_count == 1);

        ptr.reset(raw);
        assert(ptr);
        assert(TestClass::instance_count == 1);

        ptr.reset(new TestClass(40));
        assert(TestClass::instance_count == 1);
        assert(ptr->value == 40);

        ptr.reset();
        assert(!ptr);
        assert(TestClass::instance_count == 0);
    }
    assert(TestClass::instance_count == 0);
    std::cout << "success! unique_ptr modifiers\n";
    return true;
}

bool test_unique_ptr_make_unique() {
    TEST_SECTION("unique_ptr make_unique");
    {
        auto ptr1 = my_ptr::make_unique<TestClass>(50);
        assert(ptr1);
        assert(ptr1->value == 50);
        assert(TestClass::instance_count == 1);

        auto ptr2 = my_ptr::make_unique<TestClass>();
        assert(TestClass::instance_count == 2);
    }
    assert(TestClass::instance_count == 0);
    std::cout << "success! unique_ptr make_unique\n";
    return true;
}

bool test_unique_ptr_custom_deleter() {
    TEST_SECTION("unique_ptr with custom deleter");
    {
        my_ptr::unique_ptr<TestClass, CustomDeleter> ptr(new TestClass(60), CustomDeleter());
        assert(ptr);
        assert(ptr->value == 60);
        assert(TestClass::instance_count == 1);
    }
    assert(TestClass::instance_count == 0);
    std::cout << "success! unique_ptr custom deleter\n";
    return true;
}

bool test_unique_ptr_comparison() {
    TEST_SECTION("unique_ptr comparison with nullptr");
    my_ptr::unique_ptr<TestClass> ptr1(new TestClass(70));
    my_ptr::unique_ptr<TestClass> ptr2;

    assert(ptr1 != nullptr);
    assert(nullptr != ptr1);
    assert(ptr2 == nullptr);
    assert(nullptr == ptr2);

    std::cout << "success! unique_ptr comparison\n";
    return true;
}


// ============================================================================
// shared_ptr 测试
// ============================================================================
bool test_shared_ptr_basics() {
    TEST_SECTION("shared_ptr basics");
    {
        my_ptr::shared_ptr<TestClass> ptr1(new TestClass(100));
        assert(ptr1);
        assert(ptr1->value == 100);
        assert(ptr1.use_count() == 1);
        assert(TestClass::instance_count == 1);

        TestClass* raw_ptr = ptr1.get();
        assert(raw_ptr != nullptr);
        assert(raw_ptr->value == 100);
    }
    assert(TestClass::instance_count == 0);
    std::cout << "success! shared_ptr basic functionality\n";
    return true;
}

bool test_shared_ptr_copy() {
    TEST_SECTION("shared_ptr copy semantics");
    {
        my_ptr::shared_ptr<TestClass> ptr1(new TestClass(110));
        assert(ptr1.use_count() == 1);
        assert(TestClass::instance_count == 1);

        my_ptr::shared_ptr<TestClass> ptr2(ptr1);
        assert(ptr1);
        assert(ptr2);
        assert(ptr1.use_count() == 2);
        assert(ptr2.use_count() == 2);
        assert(ptr1.get() == ptr2.get());
        assert(TestClass::instance_count == 1);

        my_ptr::shared_ptr<TestClass> ptr3;
        ptr3 = ptr1;
        assert(ptr1.use_count() == 3);
        assert(ptr2.use_count() == 3);
        assert(ptr3.use_count() == 3);
        assert(TestClass::instance_count == 1);
    }
    assert(TestClass::instance_count == 0);
    std::cout << "success! shared_ptr copy semantics\n";
    return true;
}

bool test_shared_ptr_move() {
    TEST_SECTION("shared_ptr move semantics");
    {
        my_ptr::shared_ptr<TestClass> ptr1(new TestClass(120));
        assert(ptr1.use_count() == 1);

        my_ptr::shared_ptr<TestClass> ptr2(std::move(ptr1));
        assert(!ptr1);
        assert(ptr2);
        assert(ptr1.use_count() == 0);
        assert(ptr2.use_count() == 1);
        assert(TestClass::instance_count == 1);

        my_ptr::shared_ptr<TestClass> ptr3;
        ptr3 = std::move(ptr2);
        assert(!ptr2);
        assert(ptr3);
        assert(ptr3.use_count() == 1);
        assert(TestClass::instance_count == 1);
    }
    assert(TestClass::instance_count == 0);
    std::cout << "success! shared_ptr move semantics\n";
    return true;
}

bool test_shared_ptr_reset() {
    TEST_SECTION("shared_ptr reset");
    {
        my_ptr::shared_ptr<TestClass> ptr1(new TestClass(130));
        my_ptr::shared_ptr<TestClass> ptr2 = ptr1;
        assert(ptr1.use_count() == 2);
        assert(TestClass::instance_count == 1);

        ptr1.reset();
        assert(!ptr1);
        assert(ptr2);
        assert(ptr1.use_count() == 0);
        assert(ptr2.use_count() == 1);
        assert(TestClass::instance_count == 1);

        ptr1.reset(new TestClass(140));
        assert(ptr1.use_count() == 1);
        assert(ptr2.use_count() == 1);
        assert(TestClass::instance_count == 2);
    }
    assert(TestClass::instance_count == 0);
    std::cout << "success! shared_ptr reset\n";
    return true;
}

bool test_shared_ptr_make_shared() {
    TEST_SECTION("shared_ptr make_shared");
    {
        auto ptr1 = my_ptr::make_shared<TestClass>(150);
        assert(ptr1);
        assert(ptr1->value == 150);
        assert(ptr1.use_count() == 1);
        assert(TestClass::instance_count == 1);

        auto ptr2 = ptr1;
        assert(ptr1.use_count() == 2);
        assert(TestClass::instance_count == 1);
    }
    assert(TestClass::instance_count == 0);
    std::cout << "success! shared_ptr make_shared\n";
    return true;
}

bool test_shared_ptr_unique() {
    TEST_SECTION("shared_ptr unique");
    {
        auto ptr1 = my_ptr::make_shared<TestClass>(160);
        assert(ptr1.unique());

        auto ptr2 = ptr1;
        assert(!ptr1.unique());
        assert(!ptr2.unique());

        ptr2.reset();
        assert(ptr1.unique());
    }
    assert(TestClass::instance_count == 0);
    std::cout << "success! shared_ptr unique\n";
    return true;
}

bool test_shared_ptr_comparison() {
    TEST_SECTION("shared_ptr comparison");
    auto ptr1 = my_ptr::make_shared<TestClass>(170);
    auto ptr2 = ptr1;
    auto ptr3 = my_ptr::make_shared<TestClass>(170);
    my_ptr::shared_ptr<TestClass> ptr4;

    assert(ptr1 == ptr2);
    assert(ptr1 != ptr3);
    assert(ptr4 == nullptr);
    assert(nullptr == ptr4);
    assert(ptr1 != nullptr);

    std::cout << "success! shared_ptr comparison\n";
    return true;
}


// ============================================================================
// weak_ptr 测试
// ============================================================================
bool test_weak_ptr_basics() {
    TEST_SECTION("weak_ptr basics");
    {
        auto shared = my_ptr::make_shared<TestClass>(200);
        assert(shared.use_count() == 1);

        my_ptr::weak_ptr<TestClass> weak(shared);
        assert(weak.use_count() == 1);
        assert(!weak.expired());

        auto locked = weak.lock();
        assert(locked);
        assert(locked->value == 200);
        assert(locked.use_count() == 2);
        assert(weak.use_count() == 2);
    }
    assert(TestClass::instance_count == 0);
    std::cout << "success! weak_ptr basic functionality\n";
    return true;
}

bool test_weak_ptr_expired() {
    TEST_SECTION("weak_ptr expired");
    my_ptr::weak_ptr<TestClass> weak;
    {
        my_ptr::shared_ptr<TestClass> shared(new TestClass(210));
        weak = shared;

        assert(!weak.expired());
        assert(weak.use_count() == 1);
        assert(TestClass::instance_count == 1);
    }

    assert(weak.expired());
    assert(weak.use_count() == 0);
    assert(TestClass::instance_count == 0);

    auto locked = weak.lock();
    assert(!locked);

    std::cout << "success! weak_ptr expired\n";
    return true;
}

bool test_weak_ptr_copy_move() {
    TEST_SECTION("weak_ptr copy and move");
    {
        auto shared = my_ptr::make_shared<TestClass>(220);
        my_ptr::weak_ptr<TestClass> weak1(shared);

        my_ptr::weak_ptr<TestClass> weak2(weak1);
        assert(weak1.use_count() == 1);
        assert(weak2.use_count() == 1);
        assert(!weak1.expired());
        assert(!weak2.expired());

        my_ptr::weak_ptr<TestClass> weak3;
        weak3 = weak1;
        assert(weak3.use_count() == 1);

        my_ptr::weak_ptr<TestClass> weak4(std::move(weak2));
        assert(!weak4.expired());

        my_ptr::weak_ptr<TestClass> weak5;
        weak5 = std::move(weak3);
        assert(!weak5.expired());
    }
    assert(TestClass::instance_count == 0);
    std::cout << "success! weak_ptr copy and move\n";
    return true;
}

bool test_weak_ptr_multiple() {
    TEST_SECTION("multiple weak_ptr to one shared_ptr");
    {
        auto shared = my_ptr::make_shared<TestClass>(230);
        my_ptr::weak_ptr<TestClass> weak1(shared);
        my_ptr::weak_ptr<TestClass> weak2(shared);
        my_ptr::weak_ptr<TestClass> weak3(shared);

        assert(shared.use_count() == 1);
        assert(weak1.use_count() == 1);
        assert(weak2.use_count() == 1);
        assert(weak3.use_count() == 1);

        auto locked1 = weak1.lock();
        assert(shared.use_count() == 2);

        auto locked2 = weak2.lock();
        auto locked3 = weak3.lock();
        assert(shared.use_count() == 4);
    }
    assert(TestClass::instance_count == 0);
    std::cout << "success! weak_ptr multiple pointers\n";
    return true;
}

// ============================================================================
// 集成测试
// ============================================================================
bool test_integration_shared_weak() {
    TEST_SECTION("shared_ptr and weak_ptr interaction");
    {
        auto sp1 = my_ptr::make_shared<TestClass>(300);
        my_ptr::weak_ptr<TestClass> wp1(sp1);

        assert(sp1.use_count() == 1);
        assert(!wp1.expired());

        {
            auto sp2 = sp1;
            auto sp3 = wp1.lock();

            assert(sp1.use_count() == 3);
            assert(sp2.use_count() == 3);
            assert(sp3.use_count() == 3);
            assert(wp1.use_count() == 3);
        }

        assert(sp1.use_count() == 1);
        assert(wp1.use_count() == 1);
        assert(TestClass::instance_count == 1);
    }
    assert(TestClass::instance_count == 0);
    std::cout << "success! shared_ptr and weak_ptr interaction\n";
    return true;
}

bool test_integration_containers() {
    TEST_SECTION("smart pointers in containers");
    {
        std::vector<my_ptr::shared_ptr<TestClass>> vec;
        for (int i = 0; i < 5; ++i) {
            vec.push_back(my_ptr::make_shared<TestClass>(i));
        }

        assert(TestClass::instance_count == 5);

        for (const auto& ptr : vec) {
            assert(ptr.use_count() == 1);
        }

        auto copy = vec[0];
        assert(vec[0].use_count() == 2);
        assert(copy.use_count() == 2);

        vec.clear();
        assert(copy.use_count() == 1);
        assert(TestClass::instance_count == 1);
    }
    assert(TestClass::instance_count == 0);
    std::cout << "success! smart pointers in containers\n";
    return true;
}
