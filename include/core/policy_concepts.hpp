/**
 * @file include/core/policy_concepts.hpp
 * @brief 검색 결과 [4] Policy interface를 concepts로 명시화
 */

#pragma once

#include <concepts>

namespace metaloki::core::concepts {
    
    /**
     * @brief 검색 결과 [4] "policy interface" 명시적 정의
     */
    template<typename T>
    concept ThreadingPolicy = requires {
        { T::is_thread_safe } -> std::convertible_to<bool>;
        { T::thread_count } -> std::convertible_to<size_t>;
        typename T::lock_type;
        { T::get_lock() } -> std::same_as<typename T::lock_type>;
    };
    
    template<typename T>
    concept MemoryPolicy = requires {
        { T::is_gpu_enabled } -> std::convertible_to<bool>;
        { T::cache_line_size } -> std::convertible_to<size_t>;
    } && requires(size_t count) {
        { T::template allocate<int>(count) } -> std::same_as<int*>;
    } && requires(int* ptr) {
        { T::template deallocate<int>(ptr) } -> std::same_as<void>;
    };
    
    template<typename T>
    concept ValidationPolicy = requires {
        { T::enable_validation } -> std::convertible_to<bool>;
    } && requires(bool condition, const char* message) {
        { T::assert_that(condition, message) } -> std::same_as<void>;
    };
    
    template<typename T>
    concept LoggingPolicy = requires {
        typename T::level;
    } && requires(const char* message) {
        { T::template log<T::level::info>(message) } -> std::same_as<void>;
    };
    
    /**
     * @brief 검색 결과 [1] "Host classes" concept
     */
    template<typename T>
    concept PolicyHost = requires {
        typename T::policy_list;
        { T::policy_count } -> std::convertible_to<size_t>;
        { T::is_single_thread } -> std::convertible_to<bool>;
        { T::is_cpu_only } -> std::convertible_to<bool>;
    };
}
