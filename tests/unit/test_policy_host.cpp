/**
 * @file tests/unit/test_policy_host.cpp
 * @brief 검색 결과 [3] "unit testing" 기반 Policy Host 테스트
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <core/policy_host.hpp>
#include <core/policy_concepts.hpp>
#include <string>
#include <vector>

using namespace core;
using namespace core::policies;

/**
 * @brief 검색 결과 [3] "mock objects" - 테스트용 Mock Policies
 */
namespace test_policies {
    
    struct mock_threading_policy {
        static constexpr bool is_thread_safe = true;
        static constexpr size_t thread_count = 1;
        
        struct lock_type {
            bool locked = false;
            void lock() { locked = true; }
            void unlock() { locked = false; }
            bool try_lock() { return !locked; }
        };
        
        static lock_type get_lock() { return lock_type{}; }
    };
    
    struct mock_memory_policy {
        static constexpr bool is_gpu_enabled = false;
        static constexpr size_t cache_line_size = 32;
        
        static inline size_t allocation_count = 0;
        
        template<typename T>
        static T* allocate(size_t count) {
            allocation_count++;
            return static_cast<T*>(std::malloc(sizeof(T) * count));
        }
        
        template<typename T>
        static void deallocate(T* ptr) noexcept {
            std::free(ptr);
        }
        
        static void reset_count() { allocation_count = 0; }
    };
    
    struct mock_validation_policy {
        static constexpr bool enable_validation = true;
        static inline size_t assertion_count = 0;
        
        template<typename Condition, typename Message>
        static void assert_that(Condition&& condition, Message&&) {
            assertion_count++;
            if (!condition) {
                throw std::logic_error("Assertion failed");
            }
        }
        
        static void reset_count() { assertion_count = 0; }
    };
}

TEST_SUITE("Policy Host Basic Tests") {
    
    TEST_CASE("Policy Host Creation and Basic Properties") {
        using TestHost = policy_host<single_thread_policy, cpu_memory_policy>;
        
        TestHost host;
        
        // 기본 속성 확인
        CHECK(TestHost::policy_count == 2);
        CHECK(TestHost::is_single_thread == true);
        CHECK(TestHost::is_cpu_only == true);
        
        // Policy 존재 확인
        CHECK(TestHost::has_policy<single_thread_policy>() == true);
        CHECK(TestHost::has_policy<cpu_memory_policy>() == true);
        CHECK(TestHost::has_policy<validation_policy>() == false);
        
        // Policy 인덱스 확인
        CHECK(TestHost::policy_index<single_thread_policy>() == 0);
        CHECK(TestHost::policy_index<cpu_memory_policy>() == 1);
    }
    
    TEST_CASE("Policy Access and Usage") {
        using TestHost = policy_host<test_policies::mock_threading_policy, 
                                   test_policies::mock_memory_policy>;
        
        TestHost host;
        
        // Policy 접근
        auto& thread_policy = host.get_policy<test_policies::mock_threading_policy>();
        auto& memory_policy = host.get_policy<test_policies::mock_memory_policy>();
        
        // Threading policy 테스트
        auto lock = thread_policy.get_lock();
        CHECK(lock.locked == false);
        lock.lock();
        CHECK(lock.locked == true);
        lock.unlock();
        CHECK(lock.locked == false);
        
        // Memory policy 테스트
        test_policies::mock_memory_policy::reset_count();
        
        auto* ptr = memory_policy.allocate<int>(10);
        CHECK(ptr != nullptr);
        CHECK(test_policies::mock_memory_policy::allocation_count == 1);
        
        memory_policy.deallocate(ptr);
    }
    
    TEST_CASE("For Each Policy Iteration") {
        using TestHost = policy_host<test_policies::mock_threading_policy,
                                   test_policies::mock_memory_policy,
                                   test_policies::mock_validation_policy>;
        
        TestHost host;
        
        // Policy 순회 테스트
        int visit_count = 0;
        host.for_each_policy([&visit_count](auto& policy) {
            visit_count++;
            // 각 policy가 올바른 타입인지 확인
            (void)policy; // 사용하지 않는 변수 경고 제거
        });
        
        CHECK(visit_count == 3);
        
        // const 버전 테스트
        const TestHost& const_host = host;
        int const_visit_count = 0;
        const_host.for_each_policy([&const_visit_count](const auto& policy) {
            const_visit_count++;
            (void)policy;
        });
        
        CHECK(const_visit_count == 3);
    }
}

TEST_SUITE("Policy Concepts Tests") {
    
    TEST_CASE("Policy Concept Compliance") {
        // Threading Policy Concept
        STATIC_CHECK(concepts::ThreadingPolicy<single_thread_policy>);
        STATIC_CHECK(concepts::ThreadingPolicy<test_policies::mock_threading_policy>);
        
        // Memory Policy Concept  
        STATIC_CHECK(concepts::MemoryPolicy<cpu_memory_policy>);
        STATIC_CHECK(concepts::MemoryPolicy<test_policies::mock_memory_policy>);
        
        // Validation Policy Concept
        STATIC_CHECK(concepts::ValidationPolicy<validation_policy>);
        STATIC_CHECK(concepts::ValidationPolicy<test_policies::mock_validation_policy>);
        
        // Logging Policy Concept
        STATIC_CHECK(concepts::LoggingPolicy<logging_policy>);
        
        // Policy Host Concept
        using TestHost = policy_host<single_thread_policy, cpu_memory_policy>;
        STATIC_CHECK(concepts::PolicyHost<TestHost>);
    }
}

TEST_SUITE("Policy Host Integration Tests") {
    
    TEST_CASE("Policy Host with Validation") {
        using TestHost = policy_host<single_thread_policy,
                                   cpu_memory_policy,
                                   test_policies::mock_validation_policy>;
        
        TestHost host;
        auto& validation = host.get_policy<test_policies::mock_validation_policy>();
        
        test_policies::mock_validation_policy::reset_count();
        
        // 성공 케이스
        CHECK_NOTHROW(validation.assert_that(true, "Should pass"));
        CHECK(test_policies::mock_validation_policy::assertion_count == 1);
        
        // 실패 케이스
        CHECK_THROWS_AS(validation.assert_that(false, "Should fail"), std::logic_error);
        CHECK(test_policies::mock_validation_policy::assertion_count == 2);
    }
    
    TEST_CASE("Complex Policy Combination") {
        using ComplexHost = policy_host<
            test_policies::mock_threading_policy,
            test_policies::mock_memory_policy,
            test_policies::mock_validation_policy,
            logging_policy
        >;
        
        ComplexHost host;
        
        // 모든 policy가 올바르게 작동하는지 확인
        CHECK(ComplexHost::policy_count == 4);
        CHECK(host.has_policy<test_policies::mock_threading_policy>());
        CHECK(host.has_policy<test_policies::mock_memory_policy>());
        CHECK(host.has_policy<test_policies::mock_validation_policy>());
        CHECK(host.has_policy<logging_policy>());
        
        // 실제 사용 시나리오
        auto lock = host.get_policy<test_policies::mock_threading_policy>().get_lock();
        auto* ptr = host.get_policy<test_policies::mock_memory_policy>().allocate<double>(5);
        
        host.get_policy<test_policies::mock_validation_policy>().assert_that(
            ptr != nullptr, "Memory allocation should succeed");
        
        host.get_policy<logging_policy>().log<logging_policy::level::info>(
            "Integration test completed");
        
        host.get_policy<test_policies::mock_memory_policy>().deallocate(ptr);
    }
}
