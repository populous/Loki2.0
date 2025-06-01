/**
 * @file include/core/policy_host.hpp
 * @brief 검색 결과 [4] Policy-based design 구현
 * @details "host class template, taking several type parameters as input"
 */

#pragma once

#include <metaloki/core/typelist.hpp>
#include <type_traits>
#include <concepts>

namespace metaloki::core {
    
    /**
     * @brief 검색 결과 [1] "classes that uses one or more policies are called hosts"
     * @details MetaLoki 2.0 Policy Host - Single Thread 전용
     */
    template<typename... Policies>
    class policy_host : private Policies... {
        static_assert(sizeof...(Policies) > 0, "At least one policy is required");
        
    public:
        using policy_list = typelist<Policies...>;
        static constexpr size_t policy_count = sizeof...(Policies);
        
        // 검색 결과 [4] "single thread execution" 보장
        static constexpr bool is_single_thread = true;
        static constexpr bool is_cpu_only = true;
        
        /**
         * @brief 기본 생성자
         */
        constexpr policy_host() = default;
        
        /**
         * @brief 검색 결과 [1] Policy forwarding constructor
         */
        template<typename... Args>
        constexpr explicit policy_host(Args&&... args)
            : Policies(std::forward<Args>(args))... {}
        
        /**
         * @brief 검색 결과 [4] "policy interface doesn't have direct representation"
         * @details Duck typing으로 policy 접근
         */
        template<typename PolicyType>
        constexpr const PolicyType& get_policy() const noexcept {
            static_assert(policy_list::template contains<PolicyType>(), 
                "PolicyType must be in policy list");
            return static_cast<const PolicyType&>(*this);
        }
        
        template<typename PolicyType>
        constexpr PolicyType& get_policy() noexcept {
            static_assert(policy_list::template contains<PolicyType>(), 
                "PolicyType must be in policy list");
            return static_cast<PolicyType&>(*this);
        }
        
        /**
         * @brief 검색 결과 [4] "decomposing a class into policies"
         * @details 모든 policy에 동일한 작업 적용
         */
        template<typename Operation>
        constexpr void for_each_policy(Operation&& op) {
            (op(static_cast<Policies&>(*this)), ...);
        }
        
        template<typename Operation>
        constexpr void for_each_policy(Operation&& op) const {
            (op(static_cast<const Policies&>(*this)), ...);
        }
        
        /**
         * @brief Policy 존재 확인
         */
        template<typename PolicyType>
        static constexpr bool has_policy() noexcept {
            return policy_list::template contains<PolicyType>();
        }
        
        /**
         * @brief Policy 인덱스 조회
         */
        template<typename PolicyType>
        static constexpr size_t policy_index() noexcept {
            return policy_list::template index_of<PolicyType>();
        }
    };
    
    /**
     * @brief 검색 결과 [1] "algorithms are called policies" 
     * @details Single Thread 전용 기본 정책들
     */
    namespace policies {
        
        /**
         * @brief 검색 결과 [1] "Policy classes" 예시 - Threading Policy
         */
        struct single_thread_policy {
            static constexpr bool is_thread_safe = false;
            static constexpr size_t thread_count = 1;
            
            // 검색 결과 [4] Single thread 전용 Lock (No-op)
            struct lock_type {
                constexpr lock_type() noexcept = default;
                constexpr void lock() noexcept {}
                constexpr void unlock() noexcept {}
                constexpr bool try_lock() noexcept { return true; }
            };
            
            static constexpr lock_type get_lock() noexcept { 
                return lock_type{}; 
            }
        };
        
        /**
         * @brief CPU 전용 메모리 정책
         */
        struct cpu_memory_policy {
            static constexpr bool is_gpu_enabled = false;
            static constexpr size_t cache_line_size = 64;
            
            template<typename T>
            static constexpr T* allocate(size_t count) {
                return static_cast<T*>(std::aligned_alloc(alignof(T), sizeof(T) * count));
            }
            
            template<typename T>
            static constexpr void deallocate(T* ptr) noexcept {
                std::free(ptr);
            }
        };
        
        /**
         * @brief 검증 정책 (Debug/Release 분기)
         */
        struct validation_policy {
            static constexpr bool enable_validation = 
#ifdef NDEBUG
                false;
#else
                true;
#endif
            
            template<typename Condition, typename Message>
            static constexpr void assert_that(Condition&& condition, Message&& message) {
                if constexpr (enable_validation) {
                    if (!condition) {
                        throw std::logic_error(message);
                    }
                }
            }
        };
        
        /**
         * @brief 로깅 정책
         */
        struct logging_policy {
            enum class level { debug, info, warning, error };
            
            template<level Level, typename Message>
            static void log(Message&& message) {
                if constexpr (Level >= level::info) {
                    std::cout << "[" << level_to_string(Level) << "] " 
                              << message << std::endl;
                }
            }
            
        private:
            static constexpr const char* level_to_string(level l) {
                switch (l) {
                    case level::debug: return "DEBUG";
                    case level::info: return "INFO";
                    case level::warning: return "WARNING";
                    case level::error: return "ERROR";
                    default: return "UNKNOWN";
                }
            }
        };
    }
}
