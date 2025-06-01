/**
 * @file examples/policy_host/basic_usage.cpp
 * @brief 검색 결과 [2] "practical examples" 기반 Policy Host 사용법
 */

#include <core/policy_host.hpp>
#include <core/policy_concepts.hpp>
#include <iostream>
#include <vector>
#include <chrono>

using namespace metaloki::core;
using namespace metaloki::core::policies;

/**
 * @brief 검색 결과 [2] "real-world scenario" - Factory Pattern with Policies
 */
template<ThreadingPolicy TPolicy, MemoryPolicy MPolicy, LoggingPolicy LPolicy>
class PolicyBasedFactory : public policy_host<TPolicy, MPolicy, LPolicy> {
    using base = policy_host<TPolicy, MPolicy, LPolicy>;
    
public:
    /**
     * @brief 검색 결과 [2] "object creation with policies"
     */
    template<typename T, typename... Args>
    std::unique_ptr<T> create(Args&&... args) {
        // Threading policy 적용
        auto lock = this->template get_policy<TPolicy>().get_lock();
        
        // Logging policy 적용  
        this->template get_policy<LPolicy>().template log<typename LPolicy::level::info>(
            "Creating object of type T");
        
        // Memory policy 적용
        auto* raw_ptr = this->template get_policy<MPolicy>().template allocate<T>(1);
        if (!raw_ptr) {
            this->template get_policy<LPolicy>().template log<typename LPolicy::level::error>(
                "Memory allocation failed");
            return nullptr;
        }
        
        // 객체 생성
        try {
            new(raw_ptr) T(std::forward<Args>(args)...);
            this->template get_policy<LPolicy>().template log<typename LPolicy::level::info>(
                "Object created successfully");
            
            return std::unique_ptr<T>(raw_ptr, [this](T* ptr) {
                ptr->~T();
                this->template get_policy<MPolicy>().template deallocate(ptr);
            });
        } catch (...) {
            this->template get_policy<MPolicy>().template deallocate(raw_ptr);
            this->template get_policy<LPolicy>().template log<typename LPolicy::level::error>(
                "Object construction failed");
            throw;
        }
    }
    
    /**
     * @brief 검색 결과 [2] "batch operations" 예시
     */
    template<typename T, typename... Args>
    std::vector<std::unique_ptr<T>> create_batch(size_t count, Args&&... args) {
        std::vector<std::unique_ptr<T>> result;
        result.reserve(count);
        
        this->template get_policy<LPolicy>().template log<typename LPolicy::level::info>(
            "Creating batch of " + std::to_string(count) + " objects");
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (size_t i = 0; i < count; ++i) {
            if (auto obj = create<T>(args...)) {
                result.push_back(std::move(obj));
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        this->template get_policy<LPolicy>().template log<typename LPolicy::level::info>(
            "Batch creation completed in " + std::to_string(duration.count()) + " μs");
        
        return result;
    }
};

/**
 * @brief 검색 결과 [2] "different policy combinations" 예시
 */
void example_policy_combinations() {
    std::cout << "=== Policy Combinations Examples ===" << std::endl;
    
    // Development 조합 (모든 기능 활성화)
    using DevFactory = PolicyBasedFactory<single_thread_policy, cpu_memory_policy, logging_policy>;
    
    // Production 조합 (성능 최적화)
    using ProdFactory = PolicyBasedFactory<single_thread_policy, cpu_memory_policy, no_logging_policy>;
    
    // Development factory 사용
    std::cout << "\n--- Development Factory ---" << std::endl;
    DevFactory dev_factory;
    
    auto dev_obj = dev_factory.create<std::string>("Hello, MetaLoki!");
    auto dev_batch = dev_factory.create_batch<int>(5, 42);
    
    std::cout << "Created string: " << *dev_obj << std::endl;
    std::cout << "Created batch size: " << dev_batch.size() << std::endl;
    
    // Production factory 사용 (로깅 없음)
    std::cout << "\n--- Production Factory ---" << std::endl;
    ProdFactory prod_factory;
    
    auto prod_obj = prod_factory.create<std::string>("Production String");
    auto prod_batch = prod_factory.create_batch<double>(1000, 3.14);
    
    std::cout << "Production batch size: " << prod_batch.size() << std::endl;
}

/**
 * @brief 검색 결과 [2] "custom policies" 예시
 */
namespace custom_policies {
    
    struct debug_memory_policy {
        static constexpr bool is_gpu_enabled = false;
        static constexpr size_t cache_line_size = 64;
        
        static inline size_t total_allocated = 0;
        static inline size_t allocation_count = 0;
        
        template<typename T>
        static T* allocate(size_t count) {
            total_allocated += sizeof(T) * count;
            allocation_count++;
            
            std::cout << "[DEBUG_MEMORY] Allocating " << sizeof(T) * count 
                      << " bytes (total: " << total_allocated << ", count: " 
                      << allocation_count << ")" << std::endl;
            
            return static_cast<T*>(std::aligned_alloc(alignof(T), sizeof(T) * count));
        }
        
        template<typename T>
        static void deallocate(T* ptr) noexcept {
            std::cout << "[DEBUG_MEMORY] Deallocating pointer: " << ptr << std::endl;
            std::free(ptr);
        }
        
        static void print_stats() {
            std::cout << "Memory Stats - Total: " << total_allocated 
                      << " bytes, Allocations: " << allocation_count << std::endl;
        }
    };
    
    struct performance_logging_policy {
        enum class level { debug, info, warning, error };
        
        template<level Level, typename Message>
        static void log(Message&& message) {
            auto now = std::chrono::high_resolution_clock::now();
            auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
                now.time_since_epoch()).count();
            
            std::cout << "[" << timestamp << "] " << message << std::endl;
        }
    };
}

void example_custom_policies() {
    std::cout << "\n=== Custom Policies Examples ===" << std::endl;
    
    using CustomFactory = PolicyBasedFactory<
        single_thread_policy,
        custom_policies::debug_memory_policy,
        custom_policies::performance_logging_policy
    >;
    
    CustomFactory custom_factory;
    
    // 메모리 디버깅과 성능 로깅이 활성화된 팩토리 사용
    auto obj1 = custom_factory.create<std::vector<int>>(100, 42);
    auto obj2 = custom_factory.create<std::string>("Custom Policy Test");
    
    // 메모리 통계 출력
    custom_policies::debug_memory_policy::print_stats();
}

int main() {
    try {
        example_policy_combinations();
        example_custom_policies();
        
        std::cout << "\n✅ All Policy Host examples completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
